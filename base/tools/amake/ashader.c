
// kong wrapper

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

	FILE *fp = fopen(to, "wb");
	int len = shader_buffer->lpVtbl->GetBufferSize(shader_buffer);
	fwrite((char *)shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), 1, len, fp);
	fclose(fp);
	shader_buffer->lpVtbl->Release(shader_buffer);
}
#endif

extern uint64_t next_variable_id;
extern size_t allocated_globals_size;
extern function_id next_function_index;
extern global_id globals_size;
extern name_id names_index;
extern size_t sets_count;
extern type_id next_type_index;
void hlsl_export2(char **vs, char **fs, api_kind d3d, bool debug);
extern size_t vertex_inputs_size;
extern size_t fragment_inputs_size;
extern size_t vertex_functions_size;
extern size_t fragment_functions_size;

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
	vertex_inputs_size = 0;
	fragment_inputs_size = 0;
	vertex_functions_size = 0;
	fragment_functions_size = 0;
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

	#ifdef _WIN32

	char *vs;
	char *fs;
	hlsl_export2(&vs, &fs, API_DIRECT3D11, false);

	////
	// int i = string_last_index_of(to, "\\");
	// char filename[512];
	// strcpy(filename, to);
	// char *filebase = &filename[i + 1];
	// int j = string_index_of(filebase, ".");
	// filebase[j] = '\0';

	// char tmp[512];
	// strcpy(tmp, to);
	// tmp[i] = '\0';
	// strcat(tmp, "\\..\\..\\temp\\");
	// strcat(tmp, filebase);
	// strcat(tmp, ".vert.hlsl");
	// fp = fopen(tmp, "wb");
	// fwrite(vs, 1, strlen(vs), fp);
	// fclose(fp);
	// tmp[i] = '\0';
	// strcat(tmp, "\\..\\..\\temp\\");
	// strcat(tmp, filebase);
	// strcat(tmp, ".frag.hlsl");
	// fp = fopen(tmp, "wb");
	// fwrite(fs, 1, strlen(fs), fp);
	// fclose(fp);
	////

	char to_[512];
	strcpy(to_, to);
	to_[strlen(to_) - 4] = '\0';
	strcat(to_, "vert.d3d11");
	hlsl_to_bin(vs, "vert", to_);

	strcpy(to_, to);
	to_[strlen(to_) - 4] = '\0';
	strcat(to_, "frag.d3d11");
	hlsl_to_bin(fs, "frag", to_);

	#elif defined(__APPLE__)

	char *metal = metal_export("");

	int i = string_last_index_of(to, "/");
	char filename[512];
	strcpy(filename, to);
	char *filebase = &filename[i + 1];
	int j = string_index_of(filebase, ".");
	filebase[j] = '\0';

	char to_[512];
	strcpy(to_, to);
	to_[strlen(to_) - 5] = '\0';
	strcat(to_, "vert.metal");

	fp = fopen(to_, "wb");
	fwrite("//>", 1, 3, fp);
	fwrite(filebase, 1, strlen(filebase), fp);
	fwrite("_vert\n", 1, 6, fp);
	fwrite(metal, 1, strlen(metal), fp);
	fclose(fp);

	strcpy(to_, to);
	to_[strlen(to_) - 5] = '\0';
	strcat(to_, "frag.metal");

	fp = fopen(to_, "wb");
	fwrite("//>", 1, 3, fp);
	fwrite(filebase, 1, strlen(filebase), fp);
	fwrite("_frag\n", 1, 6, fp);
	fclose(fp);

	#else

	// transform(TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE);

	// int i = string_last_index_of(to, "/");
	// output[i] = '\0';
	// spirv_export2(output);

	#endif
}

int ashader(char *shader_lang, char *from, char *to) {
	// shader_lang == hlsl || metal || spirv
	kong_compile(from, to);
	return 0;
}
