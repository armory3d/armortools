
// Simple text processing tool for converting shaders at runtime.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "iron_string.h"

#include "../../sources/libs/kong/analyzer.h"
#include "../../sources/libs/kong/compiler.h"
#include "../../sources/libs/kong/disasm.h"
#include "../../sources/libs/kong/errors.h"
#include "../../sources/libs/kong/functions.h"
#include "../../sources/libs/kong/globals.h"
#include "../../sources/libs/kong/log.h"
#include "../../sources/libs/kong/names.h"
#include "../../sources/libs/kong/parser.h"
#include "../../sources/libs/kong/tokenizer.h"
#include "../../sources/libs/kong/typer.h"
#include "../../sources/libs/kong/types.h"
#include "../../sources/libs/kong/backends/hlsl.h"
#include "../../sources/libs/kong/backends/metal.h"
#include "../../sources/libs/kong/backends/spirv.h"
#include "../../sources/libs/kong/backends/wgsl.h"

#ifdef _WIN32
#include <d3d11.h>
#include <D3Dcompiler.h>

static void write_attrib(char *file, int *output_len, const char *attrib, int index) {
	if (index > -1) {
		strcpy(file + (*output_len), attrib);
		(*output_len) += strlen(attrib);
		file[(*output_len)] = 0;
		(*output_len) += 1;
		file[(*output_len)] = index;
		(*output_len) += 1;
	}
}

char *hlsl_to_bin(char *source, char *shader_type, char *to) {
	char *type;
	if (string_equals(shader_type, "vert")) {
		type = "vs_5_0";
	}
	else {
		type = "ps_5_0";
	}

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	// UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;
	UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
	HRESULT hr = D3DCompile(source, strlen(source) + 1, NULL, NULL, NULL, "main", type, flags, 0, &shader_buffer, &error_message);
	if (hr != S_OK) {
		printf("%s\n", (char *)error_message->lpVtbl->GetBufferPointer(error_message));
		return NULL;
	}

	ID3D11ShaderReflection *reflector = NULL;
	D3DReflect(shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer), &IID_ID3D11ShaderReflection, (void **)&reflector);

	int len = shader_buffer->lpVtbl->GetBufferSize(shader_buffer);
	char *file = malloc(len * 2);
	int output_len = 0;

	bool has_col = strstr(source, " col:") != NULL || strstr(source, " col :") != NULL;
	bool has_nor = strstr(source, " nor:") != NULL || strstr(source, " nor :") != NULL;
	bool has_pos = strstr(source, " pos:") != NULL || strstr(source, " pos :") != NULL;
	bool has_tex = strstr(source, " tex:") != NULL || strstr(source, " tex :") != NULL;

	int icol = -1;
	int inor = -1;
	int ipos = -1;
	int itex = -1;

	int index = 0;
	if (has_col) icol = index++;
	if (has_nor) inor = index++;
	if (has_pos) ipos = index++;
	if (has_tex) itex = index++;

	file[output_len] = (char)index;
	output_len += 1;

	write_attrib(file, &output_len, "col", icol);
	write_attrib(file, &output_len, "nor", inor);
	write_attrib(file, &output_len, "pos", ipos);
	write_attrib(file, &output_len, "tex", itex);

	D3D11_SHADER_DESC desc;
	reflector->lpVtbl->GetDesc(reflector, &desc);

	file[output_len] = desc.BoundResources;
	output_len += 1;
	for (int i = 0; i < desc.BoundResources; ++i) {
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflector->lpVtbl->GetResourceBindingDesc(reflector, i, &bindDesc);
		strcpy(file + output_len, bindDesc.Name);
		output_len += strlen(bindDesc.Name);
		file[output_len] = 0;
		output_len += 1;
		file[output_len] = bindDesc.BindPoint;
		output_len += 1;
	}

	ID3D11ShaderReflectionConstantBuffer *constants = reflector->lpVtbl->GetConstantBufferByName(reflector, "$Globals");
	D3D11_SHADER_BUFFER_DESC buffer_desc;
	hr = constants->lpVtbl->GetDesc(constants, &buffer_desc);
	if (hr == S_OK) {
		file[output_len] = buffer_desc.Variables;
		output_len += 1;
		for (int i = 0; i < buffer_desc.Variables; ++i) {
			ID3D11ShaderReflectionVariable *variable = constants->lpVtbl->GetVariableByIndex(constants, i);
			D3D11_SHADER_VARIABLE_DESC variable_desc;
			hr = variable->lpVtbl->GetDesc(variable, &variable_desc);
			if (hr == S_OK) {
				strcpy(file + output_len, variable_desc.Name);
				output_len += strlen(variable_desc.Name);
				file[output_len] = 0;
				output_len += 1;

				*(uint32_t *)(file + output_len) = variable_desc.StartOffset;
				output_len += 4;

				*(uint32_t *)(file + output_len) = variable_desc.Size;
				output_len += 4;

				D3D11_SHADER_TYPE_DESC type_desc;
				ID3D11ShaderReflectionType *type = variable->lpVtbl->GetType(variable);
				hr = type->lpVtbl->GetDesc(type, &type_desc);
				if (hr == S_OK) {
					file[output_len] = type_desc.Columns;
					output_len += 1;
					file[output_len] = type_desc.Rows;
					output_len += 1;
				}
				else {
					file[output_len] = 0;
					output_len += 1;
					file[output_len] = 0;
					output_len += 1;
				}
			}
		}
	}
	else {
		file[output_len] = 0;
		output_len += 1;
	}

	memcpy(file + output_len, (char *)shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer));
	output_len += shader_buffer->lpVtbl->GetBufferSize(shader_buffer);

	shader_buffer->lpVtbl->Release(shader_buffer);
	reflector->lpVtbl->Release(reflector);

	FILE *fp = fopen(to, "wb");
	fwrite(file, 1, output_len, fp);
	fclose(fp);
	free(file);
}

void hlslbin(const char *from, const char *to) {
	FILE *fp = fopen(from, "rb");
	fseek(fp , 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	char *source = malloc(size + 1);
	source[size] = 0;
	fread(source, size, 1, fp);
	fclose(fp);

	char *type;
	if (strstr(from, "vert.")) {
		type = "vert";
	}
	else  {
		type = "frag";
	}

	hlsl_to_bin(source, type, to);
}
#endif

extern uint64_t next_variable_id;
extern size_t allocated_globals_size;
extern function_id next_function_index;
extern global_id globals_size;
extern name_id names_index;
extern size_t sets_count;
extern type_id next_type_index;

void kong_compile(const char *from, const char *to) {
	FILE *fp = fopen(from, "rb");
	fseek(fp , 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	char *data = malloc(size + 1);
	data[size] = 0;
	fread(data, size, 1, fp);
	fclose(fp);

	next_variable_id = 1;
	allocated_globals_size = 0;
	next_function_index = 0;
	globals_size = 0;
	names_index = 1;
	sets_count = 0;
	next_type_index = 0;
	names_init();
	types_init();
	functions_init();
	globals_init();
	tokens tokens = tokenize(from, data);
	parse(from, &tokens);
	resolve_types();
	allocate_globals();
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		compile_function_block(&get_function(i)->code, get_function(i)->block);
	}
	analyze();

	char output[512];
	strcpy(output, to);

	#ifdef _WIN32

	int i = string_last_index_of(to, "\\");
	output[i] = '\0';

	strcat(output, "\\..\\..\\temp");
	hlsl_export(output, API_DIRECT3D11, false);

	char filename[512];
	strcpy(filename, to);
	char *file = &filename[i + 1];
	i = string_index_of(file, ".");
	file[i] = '\0';

	strcat(output, "\\");
	strcat(output, file);

	char from_[512];
	strcpy(from_, output);
	strcat(from_, "_frag.hlsl");

	char to_[512];
	strcpy(to_, to);
	to_[strlen(to_) - 4] = '\0';
	strcat(to_, "frag.d3d11");

	hlslbin(from_, to_);

	strcpy(from_, output);
	strcat(from_, "_vert.hlsl");

	strcpy(to_, to);
	to_[strlen(to_) - 4] = '\0';
	strcat(to_, "vert.d3d11");

	hlslbin(from_, to_);

	#else

	int i = string_last_index_of(to, "/");
	output[i] = '\0';

	// metal_export(output);
	spirv_export(output);

	#endif
}

int ashader(char *shader_lang, char *from, char *to) {
	// shader_lang == hlsl || msl || spirv
	kong_compile(from, to);
	return 0;
}
