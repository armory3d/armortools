#include "glsl.h"

#include "../analyzer.h"
#include "../compiler.h"
#include "../errors.h"
#include "../functions.h"
#include "../parser.h"
#include "../shader_stage.h"
#include "../types.h"
#include "cstyle.h"
#include "util.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *type_string(type_id type) {
	if (type == float_id) {
		return "float";
	}
	if (type == float2_id) {
		return "vec2";
	}
	if (type == float3_id) {
		return "vec3";
	}
	if (type == float4_id) {
		return "vec4";
	}
	if (type == float4x4_id) {
		return "mat4";
	}
	return get_name(get_type(type)->name);
}

static void write_code(char *glsl, char *directory, const char *filename, const char *name) {
	char full_filename[512];

	{
		sprintf(full_filename, "%s/%s.h", directory, filename);
		FILE *file = fopen(full_filename, "wb");
		fprintf(file, "#include <stddef.h>\n\n");
		fprintf(file, "extern const char *%s;\n", name);
		fprintf(file, "extern size_t %s_size;\n", name);
		fclose(file);
	}

	{
		sprintf(full_filename, "%s/%s.c", directory, filename);

		FILE *file = fopen(full_filename, "wb");
		fprintf(file, "#include \"%s.h\"\n\n", filename);

		fprintf(file, "const char *%s = \"", name);

		size_t length = strlen(glsl);

		for (size_t i = 0; i < length; ++i) {
			if (glsl[i] == '\n') {
				fprintf(file, "\\n");
			}
			else if (glsl[i] == '\r') {
				fprintf(file, "\\r");
			}
			else if (glsl[i] == '\t') {
				fprintf(file, "\\t");
			}
			else if (glsl[i] == '"') {
				fprintf(file, "\\\"");
			}
			else {
				fprintf(file, "%c", glsl[i]);
			}
		}

		fprintf(file, "\";\n\n");

		fprintf(file, "size_t %s_size = %zu;\n\n", name, length);

		fprintf(file, "/*\n%s*/\n", glsl);

		fclose(file);
	}
}

static void write_types(char *glsl, size_t *offset, shader_stage stage, type_id input, type_id output, function *main) {
	type_id types[256];
	size_t  types_size = 0;
	find_referenced_types(main, types, &types_size);

	for (size_t i = 0; i < types_size; ++i) {
		type *t = get_type(types[i]);

		if (!t->built_in && !has_attribute(&t->attributes, add_name("pipe"))) {
			if (stage == SHADER_STAGE_VERTEX && types[i] == input) {
				for (size_t j = 0; j < t->members.size; ++j) {
					*offset += sprintf(&glsl[*offset], "layout(location = %zu) in %s %s_%s;\n", j, type_string(t->members.m[j].type.type), get_name(t->name),
					                   get_name(t->members.m[j].name));
				}
			}
			else if (stage == SHADER_STAGE_VERTEX && types[i] == output) {
				for (size_t j = 0; j < t->members.size; ++j) {
					if (j != 0) {
						*offset += sprintf(&glsl[*offset], "out %s %s_%s;\n", type_string(t->members.m[j].type.type), get_name(t->name),
						                   get_name(t->members.m[j].name));
					}
				}
			}
			else if (stage == SHADER_STAGE_FRAGMENT && types[i] == input) {
				for (size_t j = 0; j < t->members.size; ++j) {
					if (j != 0) {
						*offset += sprintf(&glsl[*offset], "in %s %s_%s;\n", type_string(t->members.m[j].type.type), get_name(t->name),
						                   get_name(t->members.m[j].name));
					}
				}
			}
		}
	}

	*offset += sprintf(&glsl[*offset], "\n");

	for (size_t i = 0; i < types_size; ++i) {
		type *t = get_type(types[i]);

		if (!t->built_in && !has_attribute(&t->attributes, add_name("pipe"))) {
			*offset += sprintf(&glsl[*offset], "struct %s {\n", get_name(t->name));

			for (size_t j = 0; j < t->members.size; ++j) {
				*offset += sprintf(&glsl[*offset], "\t%s %s;\n", type_string(t->members.m[j].type.type), get_name(t->members.m[j].name));
			}

			*offset += sprintf(&glsl[*offset], "};\n\n");
		}
	}
}

static int global_register_indices[512];

static void write_globals(char *glsl, size_t *offset, function *main) {
	global_array globals = {0};

	find_referenced_globals(main, &globals);

	for (size_t i = 0; i < globals.size; ++i) {
		global *g = get_global(globals.globals[i]);
		// int register_index = global_register_indices[globals[i]];

		if (g->type == sampler_type_id) {
		}
		else if (g->type == tex2d_type_id) {
			*offset += sprintf(&glsl[*offset], "uniform sampler2D _%" PRIu64 ";\n\n", g->var_index);
		}
		else if (g->type == texcube_type_id) {
			*offset += sprintf(&glsl[*offset], "uniform samplerCube _%" PRIu64 ";\n\n", g->var_index);
		}
		else if (g->type == float_id) {
		}
		else {
			*offset += sprintf(&glsl[*offset], "layout(std140) uniform _%" PRIu64 " {\n", g->var_index);
			type *t = get_type(g->type);
			for (size_t i = 0; i < t->members.size; ++i) {
				*offset +=
				    sprintf(&glsl[*offset], "\t%s _%" PRIu64 "_%s;\n", type_string(t->members.m[i].type.type), g->var_index, get_name(t->members.m[i].name));
			}
			*offset += sprintf(&glsl[*offset], "};\n\n");
		}
	}
}

static void write_functions(char *code, size_t *offset, shader_stage stage, type_id input, type_id output, function *main, bool flip) {
	function *functions[256];
	size_t    functions_size = 0;

	functions[functions_size] = main;
	functions_size += 1;

	find_referenced_functions(main, functions, &functions_size);

	for (size_t i = 0; i < functions_size; ++i) {
		function *f = functions[i];

		debug_context context = {0};
		check(f->block != NULL, context, "Function has no block");

		uint8_t *data = f->code.o;
		size_t   size = f->code.size;

		uint64_t parameter_ids[256] = {0};
		for (uint8_t parameter_index = 0; parameter_index < f->parameters_size; ++parameter_index) {
			for (size_t i = 0; i < f->block->block.vars.size; ++i) {
				if (f->parameter_names[parameter_index] == f->block->block.vars.v[i].name) {
					parameter_ids[parameter_index] = f->block->block.vars.v[i].variable_id;
					break;
				}
			}
		}

		for (uint8_t parameter_index = 0; parameter_index < f->parameters_size; ++parameter_index) {
			check(parameter_ids[parameter_index] != 0, context, "Parameter not found");
		}

		if (f == main) {
			if (stage == SHADER_STAGE_VERTEX) {
				*offset += sprintf(&code[*offset], "void main() {\n");
			}
			else if (stage == SHADER_STAGE_FRAGMENT) {
				if (get_type(f->return_type.type)->array_size > 0) {
					*offset += sprintf(&code[*offset], "out vec4 _kong_colors[%i];\n\n", get_type(f->return_type.type)->array_size);
					*offset += sprintf(&code[*offset], "void main() {\n");
				}
				else {
					*offset += sprintf(&code[*offset], "out vec4 _kong_color;\n\n");
					*offset += sprintf(&code[*offset], "void main() {\n");
				}
			}
			else if (stage == SHADER_STAGE_COMPUTE) {
				attribute *threads_attribute = find_attribute(&f->attributes, add_name("threads"));
				if (threads_attribute == NULL || threads_attribute->paramters_count != 3) {
					debug_context context = {0};
					error(context, "Compute function requires a threads attribute with three parameters");
				}

				*offset += sprintf(&code[*offset], "layout (local_size_x = %i, local_size_y = %i, local_size_z = %i) in;\n\n",
				                   (int)threads_attribute->parameters[0], (int)threads_attribute->parameters[1], (int)threads_attribute->parameters[2]);
				*offset += sprintf(&code[*offset], "void main() {\n");
			}
			else {
				debug_context context = {0};
				error(context, "Unsupported shader stage");
			}
		}
		else {
			*offset += sprintf(&code[*offset], "%s %s(", type_string(f->return_type.type), get_name(f->name));
			for (uint8_t parameter_index = 0; parameter_index < f->parameters_size; ++parameter_index) {
				if (parameter_index == 0) {
					*offset += sprintf(&code[*offset], "%s _%" PRIu64, type_string(f->parameter_types[parameter_index].type), parameter_ids[parameter_index]);
				}
				else {
					*offset += sprintf(&code[*offset], ", %s _%" PRIu64, type_string(f->parameter_types[parameter_index].type), parameter_ids[parameter_index]);
				}
			}
			*offset += sprintf(&code[*offset], ") {\n");
		}

		int indentation = 1;

		size_t index = 0;
		while (index < size) {
			opcode *o = (opcode *)&data[index];
			switch (o->type) {
			case OPCODE_CALL: {
				if (o->op_call.func == add_name("sample")) {
					debug_context context = {0};
					check(o->op_call.parameters_size == 3, context, "sample requires three parameters");
					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = texture(_%" PRIu64 ", _%" PRIu64 ");\n", type_string(o->op_call.var.type.type),
					                   o->op_call.var.index, o->op_call.parameters[0].index, o->op_call.parameters[2].index);
				}
				else if (o->op_call.func == add_name("sample_lod")) {
					debug_context context = {0};
					check(o->op_call.parameters_size == 4, context, "sample_lod requires four parameters");
					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = textureLod(_%" PRIu64 ", _%" PRIu64 ", _%" PRIu64 ");\n",
					                   type_string(o->op_call.var.type.type), o->op_call.var.index, o->op_call.parameters[0].index,
					                   o->op_call.parameters[2].index, o->op_call.parameters[3].index);
				}
				else if (o->op_call.func == add_name("group_id")) {
					check(o->op_call.parameters_size == 0, context, "group_id can not have a parameter");
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = gl_WorkGroupID;\n", type_string(o->op_call.var.type.type), o->op_call.var.index);
				}
				else if (o->op_call.func == add_name("group_thread_id")) {
					check(o->op_call.parameters_size == 0, context, "group_thread_id can not have a parameter");
					*offset +=
					    sprintf(&code[*offset], "%s _%" PRIu64 " = gl_LocalInvocationID;\n", type_string(o->op_call.var.type.type), o->op_call.var.index);
				}
				else if (o->op_call.func == add_name("dispatch_thread_id")) {
					check(o->op_call.parameters_size == 0, context, "dispatch_thread_id can not have a parameter");
					*offset +=
					    sprintf(&code[*offset], "%s _%" PRIu64 " = gl_GlobalInvocationID;\n", type_string(o->op_call.var.type.type), o->op_call.var.index);
				}
				else if (o->op_call.func == add_name("group_index")) {
					check(o->op_call.parameters_size == 0, context, "group_index can not have a parameter");
					*offset +=
					    sprintf(&code[*offset], "%s _%" PRIu64 " = gl_LocalInvocationIndex;\n", type_string(o->op_call.var.type.type), o->op_call.var.index);
				}
				else {
					const char *function_name = get_name(o->op_call.func);
					if (o->op_call.func == add_name("float2")) {
						function_name = "vec2";
					}
					else if (o->op_call.func == add_name("float3")) {
						function_name = "vec3";
					}
					else if (o->op_call.func == add_name("float4")) {
						function_name = "vec4";
					}

					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = %s(", type_string(o->op_call.var.type.type), o->op_call.var.index, function_name);
					if (o->op_call.parameters_size > 0) {
						*offset += sprintf(&code[*offset], "_%" PRIu64, o->op_call.parameters[0].index);
						for (uint8_t i = 1; i < o->op_call.parameters_size; ++i) {
							*offset += sprintf(&code[*offset], ", _%" PRIu64, o->op_call.parameters[i].index);
						}
					}
					*offset += sprintf(&code[*offset], ");\n");
				}
				break;
			}
			case OPCODE_LOAD_ACCESS_LIST: {
				uint64_t global_var_index = 0;
				for (global_id j = 0; get_global(j) != NULL && get_global(j)->type != NO_TYPE; ++j) {
					global *g = get_global(j);
					if (o->op_load_access_list.from.index == g->var_index) {
						global_var_index = g->var_index;
						break;
					}
				}

				indent(code, offset, indentation);

				if (f == main && o->op_load_access_list.from.type.type == input) {
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = %s", type_string(o->op_load_access_list.to.type.type),
					                   o->op_load_access_list.to.index, type_string(o->op_load_access_list.from.type.type));
				}
				else {
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = _%" PRIu64, type_string(o->op_load_access_list.to.type.type),
					                   o->op_load_access_list.to.index, o->op_load_access_list.from.index);
				}

				type *s = get_type(o->op_load_access_list.from.type.type);

				for (size_t i = 0; i < o->op_load_access_list.access_list_size; ++i) {
					switch (o->op_load_access_list.access_list[i].kind) {
					case ACCESS_ELEMENT:
						*offset += sprintf(&code[*offset], "[_%" PRIu64 "]", o->op_load_access_list.access_list[i].access_element.index.index);
						break;
					case ACCESS_MEMBER:
						if (global_var_index != 0 || (f == main && o->op_load_access_list.from.type.type == input && i == 0)) {
							*offset += sprintf(&code[*offset], "_%s", get_name(o->op_load_access_list.access_list[i].access_member.name));
						}
						else {
							*offset += sprintf(&code[*offset], ".%s", get_name(o->op_load_access_list.access_list[i].access_member.name));
						}
						break;
					case ACCESS_SWIZZLE: {
						char swizzle[4];

						for (uint32_t swizzle_index = 0; swizzle_index < o->op_load_access_list.access_list[i].access_swizzle.swizzle.size; ++swizzle_index) {
							swizzle[swizzle_index] = "xyzw"[o->op_load_access_list.access_list[i].access_swizzle.swizzle.indices[swizzle_index]];
						}
						swizzle[o->op_load_access_list.access_list[i].access_swizzle.swizzle.size] = 0;

						*offset += sprintf(&code[*offset], ".%s", swizzle);
						break;
					}
					}

					s = get_type(o->op_load_access_list.access_list[i].type);
				}

				*offset += sprintf(&code[*offset], ";\n");

				break;
			}
			case OPCODE_RETURN: {
				if (o->size > offsetof(opcode, op_return)) {
					if (f == main && stage == SHADER_STAGE_VERTEX) {
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "{\n");

						type *t = get_type(f->return_type.type);

						indent(code, offset, indentation + 1);
						*offset += sprintf(&code[*offset], "gl_Position.x = _%" PRIu64 ".%s.x;\n", o->op_return.var.index, get_name(t->members.m[0].name));
						indent(code, offset, indentation + 1);
						if (flip) {
							*offset +=
							    sprintf(&code[*offset], "gl_Position.y = -1.0 * _%" PRIu64 ".%s.y;\n", o->op_return.var.index, get_name(t->members.m[0].name));
						}
						else {
							*offset += sprintf(&code[*offset], "gl_Position.y = _%" PRIu64 ".%s.y;\n", o->op_return.var.index, get_name(t->members.m[0].name));
						}
						indent(code, offset, indentation + 1);
						*offset +=
						    sprintf(&code[*offset], "gl_Position.z = (_%" PRIu64 ".%s.z * 2.0) - _%" PRIu64 ".%s.w; // OpenGL clip space z is from -w to w\n",
						            o->op_return.var.index, get_name(t->members.m[0].name), o->op_return.var.index, get_name(t->members.m[0].name));
						indent(code, offset, indentation + 1);
						*offset += sprintf(&code[*offset], "gl_Position.w = _%" PRIu64 ".%s.w;\n", o->op_return.var.index, get_name(t->members.m[0].name));

						for (size_t j = 1; j < t->members.size; ++j) {
							indent(code, offset, indentation + 1);
							*offset += sprintf(&code[*offset], "%s_%s = _%" PRIu64 ".%s;\n", get_name(t->name), get_name(t->members.m[j].name),
							                   o->op_return.var.index, get_name(t->members.m[j].name));
						}

						indent(code, offset, indentation + 1);
						*offset += sprintf(&code[*offset], "return;\n");
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "}\n");
					}
					else if (f == main && stage == SHADER_STAGE_FRAGMENT && get_type(f->return_type.type)->array_size > 0) {
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "{\n");
						for (uint32_t j = 0; j < get_type(f->return_type.type)->array_size; ++j) {
							indent(code, offset, indentation + 1);
							*offset += sprintf(&code[*offset], "_kong_colors[%i] = _%" PRIu64 "[%i];\n", j, o->op_return.var.index, j);
						}
						indent(code, offset, indentation + 1);
						*offset += sprintf(&code[*offset], "return;\n");
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "}\n");
					}
					else if (f == main && stage == SHADER_STAGE_FRAGMENT) {
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "{\n");
						indent(code, offset, indentation + 1);
						*offset += sprintf(&code[*offset], "_kong_color = _%" PRIu64 ";\n", o->op_return.var.index);
						indent(code, offset, indentation + 1);
						*offset += sprintf(&code[*offset], "return;\n");
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "}\n");
					}
					else {
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "return _%" PRIu64 ";\n", o->op_return.var.index);
					}
				}
				else {
					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "return;\n");
				}
				break;
			}
			default:
				cstyle_write_opcode(code, offset, o, type_string, &indentation);
				break;
			}

			index += o->size;
		}

		*offset += sprintf(&code[*offset], "}\n\n");
	}
}

static void glsl_export_vertex(char *directory, function *main, bool flip) {
	char         *glsl    = (char *)calloc(1024 * 1024, 1);
	debug_context context = {0};
	check(glsl != NULL, context, "Could not allocate glsl string");

	size_t offset = 0;

	assert(main->parameters_size > 0);
	type_id vertex_input  = main->parameter_types[0].type;
	type_id vertex_output = main->return_type.type;

	check(vertex_input != NO_TYPE, context, "vertex input missing");
	check(vertex_output != NO_TYPE, context, "vertex output missing");

	offset += sprintf(&glsl[offset], "#version 330\n\n");

	write_types(glsl, &offset, SHADER_STAGE_VERTEX, vertex_input, vertex_output, main);

	write_globals(glsl, &offset, main);

	write_functions(glsl, &offset, SHADER_STAGE_VERTEX, vertex_input, vertex_output, main, flip);

	char *name = get_name(main->name);

	char filename[512];
	if (flip) {
		sprintf(filename, "kong_%s_flip", name);
	}
	else {
		sprintf(filename, "kong_%s", name);
	}

	char var_name[256];
	if (flip) {
		sprintf(var_name, "%s_flip_code", name);
	}
	else {
		sprintf(var_name, "%s_code", name);
	}

	write_code(glsl, directory, filename, var_name);
}

static void glsl_export_fragment(char *directory, function *main) {
	char         *glsl    = (char *)calloc(1024 * 1024, 1);
	debug_context context = {0};
	check(glsl != NULL, context, "Could not allocate glsl string");

	size_t offset = 0;

	assert(main->parameters_size > 0);
	type_id pixel_input = main->parameter_types[0].type;

	check(pixel_input != NO_TYPE, context, "fragment input missing");

	offset += sprintf(&glsl[offset], "#version 330\n\n");

	write_types(glsl, &offset, SHADER_STAGE_FRAGMENT, pixel_input, NO_TYPE, main);

	write_globals(glsl, &offset, main);

	write_functions(glsl, &offset, SHADER_STAGE_FRAGMENT, pixel_input, NO_TYPE, main, false);

	char *name = get_name(main->name);

	char filename[512];
	sprintf(filename, "kong_%s", name);

	char var_name[256];
	sprintf(var_name, "%s_code", name);

	write_code(glsl, directory, filename, var_name);
}

static void glsl_export_compute(char *directory, function *main) {
	char         *glsl    = (char *)calloc(1024 * 1024, 1);
	debug_context context = {0};
	check(glsl != NULL, context, "Could not allocate glsl string");

	size_t offset = 0;

	assert(main->parameters_size == 0);

	offset += sprintf(&glsl[offset], "#version 330\n\n");

	write_types(glsl, &offset, SHADER_STAGE_COMPUTE, NO_TYPE, NO_TYPE, main);

	write_globals(glsl, &offset, main);

	write_functions(glsl, &offset, SHADER_STAGE_COMPUTE, NO_TYPE, NO_TYPE, main, false);

	char *name = get_name(main->name);

	char filename[512];
	sprintf(filename, "kong_%s", name);

	char var_name[256];
	sprintf(var_name, "%s_code", name);

	write_code(glsl, directory, filename, var_name);
}

void glsl_export(char *directory) {
	int cbuffer_index = 0;
	int texture_index = 0;
	int sampler_index = 0;

	memset(global_register_indices, 0, sizeof(global_register_indices));

	for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
		global *g = get_global(i);
		if (g->type == sampler_type_id) {
			global_register_indices[i] = sampler_index;
			sampler_index += 1;
		}
		else if (g->type == tex2d_type_id || g->type == texcube_type_id) {
			global_register_indices[i] = texture_index;
			texture_index += 1;
		}
		else if (g->type == float_id) {
		}
		else {
			global_register_indices[i] = cbuffer_index;
			cbuffer_index += 1;
		}
	}

	function *vertex_shaders[256];
	size_t    vertex_shaders_size = 0;

	function *fragment_shaders[256];
	size_t    fragment_shaders_size = 0;

	for (type_id i = 0; get_type(i) != NULL; ++i) {
		type *t = get_type(i);
		if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
			name_id vertex_shader_name   = NO_NAME;
			name_id fragment_shader_name = NO_NAME;

			for (size_t j = 0; j < t->members.size; ++j) {
				if (t->members.m[j].name == add_name("vertex")) {
					vertex_shader_name = t->members.m[j].value.identifier;
				}
				else if (t->members.m[j].name == add_name("fragment")) {
					fragment_shader_name = t->members.m[j].value.identifier;
				}
			}

			debug_context context = {0};
			check(vertex_shader_name != NO_NAME, context, "vertex shader missing");
			check(fragment_shader_name != NO_NAME, context, "fragment shader missing");

			for (function_id i = 0; get_function(i) != NULL; ++i) {
				function *f = get_function(i);
				if (f->name == vertex_shader_name) {
					vertex_shaders[vertex_shaders_size] = f;
					vertex_shaders_size += 1;
				}
				else if (f->name == fragment_shader_name) {
					fragment_shaders[fragment_shaders_size] = f;
					fragment_shaders_size += 1;
				}
			}
		}
	}

	function *compute_shaders[256];
	size_t    compute_shaders_size = 0;

	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);
		if (has_attribute(&f->attributes, add_name("compute"))) {
			compute_shaders[compute_shaders_size] = f;
			compute_shaders_size += 1;
		}
	}

	for (size_t i = 0; i < vertex_shaders_size; ++i) {
		glsl_export_vertex(directory, vertex_shaders[i], false);
		glsl_export_vertex(directory, vertex_shaders[i], true);
	}

	for (size_t i = 0; i < fragment_shaders_size; ++i) {
		glsl_export_fragment(directory, fragment_shaders[i]);
	}

	for (size_t i = 0; i < compute_shaders_size; ++i) {
		glsl_export_compute(directory, compute_shaders[i]);
	}
}
