
// Simple text processing tool for converting shaders at runtime.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "iron_string.h"
#ifdef __linux__
int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
#endif

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
	else if (string_equals(shader_type, "frag")) {
		type = "ps_5_0";
	}
	else {
		type = "gs_5_0";
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

	bool has_bone = strstr(source, " bone:") != NULL;
	bool has_col = strstr(source, " col:") != NULL;
	bool has_nor = strstr(source, " nor:") != NULL;
	bool has_pos = strstr(source, " pos:") != NULL;
	bool has_tex = strstr(source, " tex:") != NULL;

	int ibone = -1;
	int icol = -1;
	int inor = -1;
	int ipos = -1;
	int itex = -1;
	int iweight = -1;

	int index = 0;
	if (has_bone) ibone = index++;
	if (has_col) icol = index++;
	if (has_nor) inor = index++;
	if (has_pos) ipos = index++;
	if (has_tex) itex = index++;
	if (has_bone) iweight = index++;

	file[output_len] = (char)index;
	output_len += 1;

	write_attrib(file, &output_len, "bone", ibone);
	write_attrib(file, &output_len, "col", icol);
	write_attrib(file, &output_len, "nor", inor);
	write_attrib(file, &output_len, "pos", ipos);
	write_attrib(file, &output_len, "tex", itex);
	write_attrib(file, &output_len, "weight", iweight);

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
	if (strstr(from, ".vert.")) {
		type = "vert";
	}
	else if (strstr(from, ".frag.")) {
		type = "frag";
	}
	else {
		type = "geom";
	}

	hlsl_to_bin(source, type, to);
}
#endif

static char out[128 * 1024];
static int out_pos = 0;
static char line[1024];
static char tmp[1024];

static char *buffer;
static int buffer_size;
static int buffer_pos = 0;

typedef struct var {
	char name[64];
	char type[16];
} var_t;

static char defines[32][64];
static int defines_count;
static bool is_defined[8];
static int is_defined_index;

static char *numbers[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static var_t inputs[8];
static var_t outputs[8];
static var_t uniforms[32];
static var_t samplers[32];
static int inputs_count;
static int outputs_count;
static int uniforms_count;
static int samplers_count;

static char *shader_type;

const char *version_glsl = "#version 450\n";
const char *version_essl = "#version 300 es\n";
const char *precision_essl = "precision highp float;\nprecision mediump int;\n";

const char *header_glsl = "#define GLSL\n\
#define mul(a, b) b * a\n\
#define atan2(x, y) atan(y, x)\n\
#define OUT(t, v) out t v\n\
";

const char *header_hlsl = "#define HLSL\n\
#define sampler2D Texture2D\n\
#define sampler3D Texture3D\n\
#define texture(tex, coord) tex.Sample(tex ## _sampler, coord)\n\
#define textureLod(tex, coord, lod) tex.SampleLevel(tex ## _sampler, coord, lod)\n\
#define texelFetch(tex, coord, lod) tex.Load(float3(coord.xy, lod))\n\
uint2 _GetDimensions(Texture2D tex, uint lod) { uint x, y; tex.GetDimensions(x, y); return uint2(x, y); }\n\
#define textureSize _GetDimensions\n\
#define mod(a, b) (a % b)\n\
#define vec2 float2\n\
#define vec3 float3\n\
#define vec4 float4\n\
#define ivec2 int2\n\
#define ivec3 int3\n\
#define ivec4 int4\n\
#define mat2 float2x2\n\
#define mat3 float3x3\n\
#define mat4 float4x4\n\
#define dFdx ddx\n\
#define dFdy ddy\n\
#define inversesqrt rsqrt\n\
#define fract frac\n\
#define mix lerp\n\
#define OUT(t, v) out t v\n\
";

const char *header_msl = "// my_main\n\
#define METAL\n\
#include <metal_stdlib>\n\
#include <simd/simd.h>\n\
using namespace metal;\n\
#define sampler2D texture2d<float>\n\
#define sampler3D texture3d<float>\n\
#define texture(tex, coord) tex.sample(tex ## _sampler, coord)\n\
#define textureLod(tex, coord, lod) tex.sample(tex ## _sampler, coord, level(lod))\n\
#define texelFetch(tex, coord, lod) tex.read(uint2(coord), uint(lod))\n\
float2 _getDimensions(texture2d<float> tex, uint lod) { return float2(tex.get_width(lod), tex.get_height(lod)); }\n\
#define textureSize _getDimensions\n\
#define mod(a, b) fmod(a, b)\n\
#define vec2 float2\n\
#define vec3 float3\n\
#define vec4 float4\n\
#define ivec2 int2\n\
#define ivec3 int3\n\
#define ivec4 int4\n\
#define mat2 float2x2\n\
#define mat3 float3x3\n\
#define mat4 float4x4\n\
#define dFdx dfdx\n\
#define dFdy dfdy\n\
#define inversesqrt rsqrt\n\
#define mul(a, b) b * a\n\
#define discard discard_fragment()\n\
#define OUT(t, v) thread t &v\n\
class my_class {\n\
public:\n\
";

static char *read_line() {
	int i = 0;
	while (true) {
		if (buffer[buffer_pos] == '\0') {
			return NULL;
		}

		line[i] = buffer[buffer_pos];
		i++;
		buffer_pos++;

		#ifdef _WIN32
		if (line[i - 1] == '\r') {
			line[i - 1] = '\n';
			buffer_pos++; // Skip \n
			break;
		}
		#endif

		if (line[i - 1] == '\n') {
			break;
		}
	}

	line[i] = '\0';
	return &line[0];
}

static void w(char *str) {
	strcpy(out + out_pos, str);
	out_pos += strlen(str);
}

static bool is_undefined() {
	for (int i = 0; i <= is_defined_index; ++i) {
		if (!is_defined[i]) {
			return true;
		}
	}
	return false;
}

static void write_includes(char *file, int off) {
	char *_buffer = buffer;
	int _pos = buffer_pos;
	buffer_pos = off;

	FILE *fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	buffer = malloc(size + 1);
	buffer[size] = '\0';
	fread(buffer, size, 1, fp);
	fclose(fp);

	while (true) {
		char *line = read_line();
		if (line == NULL) {
			break;
		}

		if (starts_with(line, "#include ")) {
			char *rel = line + 10; // #include "
			rel[strlen(rel) - 1 - 1] = '\0'; // trailing "

			char tmp[512];
			int last = string_last_index_of(file, "/") + 1;
			strncpy(tmp, file, last);
			tmp[last] = '\0';
			strcat(tmp, rel);

			write_includes(tmp, 0);
			line[0] = '\0';
		}

		if (starts_with(line, "#define ")) {
			strcpy(defines[defines_count++], line + 8);
		}

		if (starts_with(line, "#ifdef ")) {
			line += 7;
			is_defined_index++;
			is_defined[is_defined_index] = false;
			for (int i = 0; i < defines_count; ++i) {
				if (string_equals(line, defines[i])) {
					is_defined[is_defined_index] = true;
					break;
				}
			}
			continue;
		}

		if (starts_with(line, "#else")) {
			is_defined[is_defined_index] = !is_defined[is_defined_index];
			continue;
		}

		if (starts_with(line, "#endif")) {
			is_defined_index--;
			continue;
		}

		if (starts_with(line, "//") || is_undefined()) {
			continue;
		}

		w(line);
	}

	free(buffer);
	buffer = _buffer;
	buffer_pos = _pos;
}

static void write_header(char *shader_lang) {
	if (string_equals(shader_lang, "glsl") ||
		string_equals(shader_lang, "essl") ||
		string_equals(shader_lang, "spirv")
	) {
		if (string_equals(shader_lang, "essl")) {
			w(version_essl);
			w(precision_essl);
		}
		else {
			w(version_glsl);
		}
		w(header_glsl);

		if (string_equals(shader_lang, "spirv")) {
			strcpy(defines[defines_count++], "SPIRV\n");
		}
		else {
			strcpy(defines[defines_count++], "GLSL\n");
		}
	}
	else if (string_equals(shader_lang, "hlsl")) {
		w(header_hlsl);
		strcpy(defines[defines_count++], "HLSL\n");
	}
	else if (string_equals(shader_lang, "msl")) {
		w(header_msl);
		strcpy(defines[defines_count++], "METAL\n");
	}
}

static int sort_vars(const void *_a, const void *_b) {
	var_t *a = _a;
	var_t *b = _b;
	return strcmp(a->name, b->name);
}

static void read_vars(char *line, var_t *vars, int *vars_count, char *start) {
	*vars_count = 0;

	while (starts_with(line, start)) {
		memset(&vars[*vars_count], 0, sizeof(var_t));
		line = line + strlen(start);

		int pos = string_index_of(line, " ");
		strncat(vars[*vars_count].type, line, pos);
		line += pos + 1;

		pos = string_index_of(line, ";");
		strncat(vars[*vars_count].name, line, pos);

		(*vars_count)++;
		line = read_line();

		while (starts_with(line, "#")) {
			w(line);
			line = read_line();
		}
	}

	qsort(vars, *vars_count, sizeof(var_t), sort_vars);
}

static void init_buffer() {
	buffer = malloc(128 * 1024);
	strcpy(buffer, out);
	buffer_size = strlen(buffer);
	buffer_pos = 0;
	out[0] = '\0';
	out_pos = 0;

	inputs_count = 0;
	outputs_count = 0;
	uniforms_count = 0;
	samplers_count = 0;
}

static void write_hlsl_return() {
	w("\tshader_output stage_output;\n");
	if (string_equals(shader_type, "vert")) {
		w("\tgl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n");
		w("\tstage_output.gl_Position = gl_Position;\n");
	}
	for (int i = 0; i < outputs_count; ++i) {
		w("\tstage_output.");
		w(outputs[i].name);
		if (out[out_pos - 1] == ']') {
			out_pos -= 3; // Erase [i]
		}
		w(" = ");
		w(outputs[i].name);
		if (out[out_pos - 1] == ']') {
			out_pos -= 3;
		}
		w(";\n");
	}
	w("\treturn stage_output;\n");
}

static char *write_hlsl_inputs(char *line) {
	read_vars(line, inputs, &inputs_count, "in ");

	w("struct shader_input {\n");
	for (int i = 0; i < inputs_count; ++i) {
		w("\t");
		w(inputs[i].type);
		w(" ");
		w(inputs[i].name);
		w(": TEXCOORD");
		w(numbers[i]);
		w(";\n");
	}

	if (string_index_of(buffer, "gl_VertexID") > -1) {
		w("\tuint gl_VertexID: SV_VertexID;\n");
	}
	if (string_index_of(buffer, "gl_InstanceID") > -1) {
		w("\tuint gl_InstanceID: SV_InstanceID;\n");
	}

	w("};\n");

	return line;
}

static char *write_hlsl_outputs(char *line) {
	read_vars(line, outputs, &outputs_count, "out ");

	w("struct shader_output {\n");
	for (int i = 0; i < outputs_count; ++i) {
		w("\t");
		w(outputs[i].type);
		w(" ");
		w(outputs[i].name);

		if (string_equals(shader_type, "vert")) {
			w(": TEXCOORD");
		}
		else {
			w(": SV_TARGET");
		}

		w(numbers[i]);
		w(";\n");
	}

	if (string_equals(shader_type, "vert")) {
		w("\tvec4 gl_Position: SV_POSITION;\n");
	}

	w("};\n");

	return line;
}

static void to_hlsl() {
	init_buffer();

	while (true) {
		char *line = read_line();
		if (line == NULL) {
			break;
		}

		if (starts_with(line, "in ")) {
			line = write_hlsl_inputs(line);

			for (int i = 0; i < inputs_count; ++i) {
				w("static ");
				w(inputs[i].type);
				w(" ");
				w(inputs[i].name);
				w(";\n");
			}

			if (string_index_of(buffer, "gl_VertexID") > -1) {
				w("static uint gl_VertexID;\n");
			}
			if (string_index_of(buffer, "gl_InstanceID") > -1) {
				w("static uint gl_InstanceID;\n");
			}
		}

		if (starts_with(line, "out ")) {
			line = write_hlsl_outputs(line);

			if (string_equals(shader_type, "vert")) {
				w("static vec4 gl_Position;\n");
			}

			for (int i = 0; i < outputs_count; ++i) {
				w("static ");
				w(outputs[i].type);
				w(" ");
				w(outputs[i].name);
				w(";\n");
			}
		}

		if (starts_with(line, "uniform sampler")) {
			int len = strlen(line);
			int last = string_last_index_of(line, " ");
			strcpy(tmp, "SamplerState ");
			strncat(tmp, line + last + 1, len - last - 3); // - ";\n"
			strcat(tmp, "_sampler;\n");
			w(tmp);
		}

		if (starts_with(line, "const ")) {
			w("static ");
		}

		if (starts_with(line, "vec") ||
			starts_with(line, "float ") ||
			starts_with(line, "int ")
		) {
			if (ends_with(line, ";\n")) {
				w("static ");
			}
		}

		if (ends_with(line, "return;\n")) {
			write_hlsl_return();
			line = "";
		}

		if (starts_with(line, "void main")) {
			w("shader_output main(shader_input stage_input) {\n");
			line = "";

			for (int i = 0; i < inputs_count; ++i) {
				w("\t");
				w(inputs[i].name);
				w(" = stage_input.");
				w(inputs[i].name);
				w(";\n");
			}

			if (string_index_of(buffer, "gl_VertexID") > -1) {
				w("\tgl_VertexID = stage_input.gl_VertexID;\n");
			}

			if (string_index_of(buffer, "gl_InstanceID") > -1) {
				w("\tgl_InstanceID = stage_input.gl_InstanceID;\n");
			}
		}

		if (starts_with(line, "}") && buffer_pos == buffer_size) {
			write_hlsl_return();
		}

		w(line);
	}

	free(buffer);
}

static char *write_msl_inputs(char *line) {
	read_vars(line, inputs, &inputs_count, "in ");

	w("struct shader_input {\n");

	for (int i = 0; i < inputs_count; ++i) {
		w("\t");
		w(inputs[i].type);
		w(" ");
		w(inputs[i].name);
		if (string_equals(shader_type, "vert")) {
			w(" [[attribute(");
		}
		else {
			w(" [[user(locn");
		}
		w(numbers[i]);
		w(")]];\n");
	}

	w("};\n");

	return line;
}

static char *write_msl_outputs(char *line) {
	read_vars(line, outputs, &outputs_count, "out ");

	w("struct shader_output {\n");
	for (int i = 0; i < outputs_count; ++i) {
		int pos = string_index_of(outputs[i].name, "[");
		if (pos > -1) {
			int n = outputs[i].name[pos + 1] - '0';
			for (int j = 0; j < n; ++j) {
				w("\t");
				w(outputs[i].type);
				w(" ");
				// w(outputs[i].name);
				w("frag_color");
				w(numbers[j]);
				w(" [[color(");
				w(numbers[j]);
				w(")]];\n");
			}
		}
		else {
			w("\t");
			w(outputs[i].type);
			w(" ");
			w(outputs[i].name);
			if (string_equals(shader_type, "vert")) {
				w(" [[user(locn");
				w(numbers[i]);
				w(")]];\n");
			}
			else {
				w(" [[color(0)]];\n");
			}
		}
	}

	if (string_equals(shader_type, "vert")) {
		w("\tvec4 gl_Position [[position]];\n");
	}

	w("};\n");

	return line;
}

static char *write_msl_uniforms(char *line) {
	read_vars(line, uniforms, &uniforms_count, "uniform ");

	w("struct shader_uniforms {\n");

	for (int i = 0; i < uniforms_count; ++i) {

		if (starts_with(uniforms[i].type, "sampler")) {
			strcpy(samplers[samplers_count].type, uniforms[i].type);
			strcpy(samplers[samplers_count].name, uniforms[i].name);
			samplers_count++;
			continue;
		}

		w("\t");
		w(uniforms[i].type);
		w(" ");
		w(uniforms[i].name);
		w(";\n");
	}

	w("};\n");

	return line;
}

static void write_msl_return() {
	w("\tshader_output stage_output;\n");
	if (string_equals(shader_type, "vert")) {
		w("\tgl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n");
		w("\tstage_output.gl_Position = gl_Position;\n");
	}
	for (int i = 0; i < outputs_count; ++i) {
		int pos = string_index_of(outputs[i].name, "[");
		if (pos > -1) {
			int n = outputs[i].name[pos + 1] - '0';
			char tmp[64];
			strcpy(tmp, outputs[i].name);
			tmp[pos] = '\0';
			for (int j = 0; j < n; ++j) {
				w("\tstage_output.");
				w(tmp);
				w(numbers[j]);
				w(" = ");
				w(tmp);
				w("[");
				w(numbers[j]);
				w("];\n");
			}
		}
		else {
			w("\tstage_output.");
			w(outputs[i].name);
			w(" = ");
			w(outputs[i].name);
			w(";\n");
		}
	}
	w("\treturn stage_output;\n");
}

static void to_msl() {
	init_buffer();
	char main_header[1024];
	char main_body[1024];
	strcpy(main_header, "#undef texture\n");
	strcpy(main_body, "\tmy_class c;\n\treturn c.my_main(stage_input");

	while (true) {

		char *line = read_line();
		if (line == NULL) {
			break;
		}

		if (starts_with(line, "in ")) {
			line = write_msl_inputs(line);

			for (int i = 0; i < inputs_count; ++i) {
				w(inputs[i].type);
				w(" ");
				w(inputs[i].name);
				w(";\n");
			}

			if (string_index_of(buffer, "gl_VertexID") > -1) {
				w("uint gl_VertexID;\n");
			}
			if (string_index_of(buffer, "gl_InstanceID") > -1) {
				w("uint gl_InstanceID;\n");
			}
		}

		if (starts_with(line, "out ")) {
			line = write_msl_outputs(line);

			if (string_equals(shader_type, "vert")) {
				w("vec4 gl_Position;\n");
			}

			for (int i = 0; i < outputs_count; ++i) {
				w(outputs[i].type);
				w(" ");
				w(outputs[i].name);
				w(";\n");
			}
		}

		if (starts_with(line, "uniform ")) {
			line = write_msl_uniforms(line);

			for (int i = 0; i < uniforms_count; ++i) {
				w(uniforms[i].type);
				w(" ");
				w(uniforms[i].name);
				w(";\n");

				if (starts_with(uniforms[i].type, "sampler")) {
					w("sampler ");
					w(uniforms[i].name);
					w("_sampler;\n");
				}
			}
		}

		if (starts_with(line, "void main")) {

			if (string_equals(shader_type, "vert")) {
				strcat(main_header, "vertex ");
			}
			else {
				strcat(main_header, "fragment ");
			}

			w("shader_output my_main(shader_input stage_input");
			strcat(main_header, "my_class::shader_output my_main(my_class::shader_input stage_input [[stage_in]]");

			if (uniforms_count > 0) {
				w(", constant shader_uniforms& uniforms");
				strcat(main_header, ", constant my_class::shader_uniforms& uniforms [[buffer(");
				if (string_equals(shader_type, "vert")) {
					strcat(main_header, "1");
				}
				else {
					strcat(main_header, "0");
				}
				strcat(main_header, ")]]");
				strcat(main_body, ", uniforms");
			}

			if (samplers_count > 0) {
				for (int i = 0; i < samplers_count; ++i) {
					w(", ");
					w(samplers[i].type);
					w(" ");
					w(samplers[i].name);

					w(", sampler ");
					w(samplers[i].name);
					w("_sampler");

					strcat(main_header, ", ");
					strcat(main_header, samplers[i].type);
					strcat(main_header, " ");
					strcat(main_header, samplers[i].name);
					strcat(main_header, " [[texture(");
					strcat(main_header, numbers[i]);
					strcat(main_header, ")]]");

					strcat(main_header, ", sampler ");
					strcat(main_header, samplers[i].name);
					strcat(main_header, "_sampler [[sampler(");
					strcat(main_header, numbers[i]);
					strcat(main_header, ")]]");

					strcat(main_body, ", ");
					strcat(main_body, samplers[i].name);
					strcat(main_body, ", ");
					strcat(main_body, samplers[i].name);
					strcat(main_body, "_sampler");
				}
			}

			if (string_equals(shader_type, "vert")) {
				if (string_index_of(buffer, "gl_VertexID") > -1) {
					w(", uint gl_VertexID");
					strcat(main_header, ", uint gl_VertexID [[vertex_id]]");
					strcat(main_body, ", gl_VertexID");
				}
				if (string_index_of(buffer, "gl_InstanceID") > -1) {
					w(", uint gl_InstanceID");
					strcat(main_header, ", uint gl_InstanceID [[instance_id]]");
					strcat(main_body, ", gl_InstanceID");
				}
			}

			w(") {\n");
			strcat(main_header, ") {\n");
			strcat(main_body, ");\n}\n");

			line = "";

			for (int i = 0; i < inputs_count; ++i) {
				w("\t");
				w(inputs[i].name);
				w(" = stage_input.");
				w(inputs[i].name);
				w(";\n");
			}

			if (string_index_of(buffer, "gl_VertexID") > -1) {
				w("\tthis->gl_VertexID = gl_VertexID;\n");
			}

			if (string_index_of(buffer, "gl_InstanceID") > -1) {
				w("\tthis->gl_InstanceID = gl_InstanceID;\n");
			}

			for (int i = 0; i < uniforms_count; ++i) {
				int pos = string_index_of(uniforms[i].name, "[");
				if (pos > -1) {
					char tmp[64];
					strcpy(tmp, uniforms[i].name);
					tmp[pos] = '\0';

					int n = uniforms[i].name[pos + 1] - '0';
					for (int j = 0; j < n; ++j) {
						w("\tthis->");
						w(tmp);
						w("[");
						w(numbers[j]);
						w("] = uniforms.");
						w(tmp);
						w("[");
						w(numbers[j]);
						w("];\n");
					}
				}
				else if (starts_with(uniforms[i].type, "sampler")) {
					w("\tthis->");
					w(uniforms[i].name);
					w(" = ");
					w(uniforms[i].name);
					w(";\n");

					w("\tthis->");
					w(uniforms[i].name);
					w("_sampler = ");
					w(uniforms[i].name);
					w("_sampler;\n");
				}
				else {
					w("\tthis->");
					w(uniforms[i].name);
					w(" = uniforms.");
					w(uniforms[i].name);
					w(";\n");
				}
			}
		}

		if (starts_with(line, "}") && buffer_pos == buffer_size) {
			write_msl_return();
		}

		w(line);
	}

	w("};\n"); // class my_class
	w(main_header);
	w(main_body);

	free(buffer);
}

int ashader(char *shader_lang, char *from, char *to) {
	// shader_lang == glsl || essl || hlsl || msl || spirv
	shader_type = string_index_of(from, ".vert") != -1 ? "vert" : "frag";

	#ifdef _WIN32
	char from_[512];
	strcpy(from_, from);
	int len = strlen(from_);
	for (int i = 0; i < len; ++i) {
		if (from_[i] == '\\') {
			from_[i] = '/';
		}
	}
	from = from_;
	#endif

	out[0] = '\0';
	out_pos = 0;
	defines_count = 0;
	is_defined[0] = true;
	is_defined_index = 0;
	write_header(shader_lang);

	write_includes(from, 13); // Skip #version 450\n

	if (string_equals(shader_lang, "glsl") ||
		string_equals(shader_lang, "essl")
	) {
		FILE *fp = fopen(to, "wb");
		fwrite(out, 1, strlen(out), fp);
		fclose(fp);
	}

	#ifdef __linux__
	else if (string_equals(shader_lang, "spirv")) {
		char *buf = malloc(1024 * 1024);
		int buf_len;
		krafix_compile(out, buf, &buf_len, "spirv", "linux", shader_type, -1);

		FILE *fp = fopen(to, "wb");
		fwrite(buf, 1, buf_len, fp);
		fclose(fp);
	}
	#endif

	else if (string_equals(shader_lang, "hlsl")) {
		to_hlsl();

		// FILE *fp = fopen(to, "wb");
		// fwrite(out, 1, strlen(out), fp); // Write .hlsl
		// fclose(fp);

		#ifdef _WIN32
		char to_[512];
		strcpy(to_, to);
		to_[strlen(to_) - 4] = '\0';
		strcat(to_, "d3d11");
		hlsl_to_bin(out, shader_type, to_);
		#endif
	}

	else if (string_equals(shader_lang, "msl")) {
		to_msl();

		char to_[512];
		strcpy(to_, to);
		to_[strlen(to_) - 3] = '\0';
		strcat(to_, "metal");

		FILE *fp = fopen(to_, "wb");
		fwrite(out, 1, strlen(out), fp);
		fclose(fp);
	}

	return 0;
}
