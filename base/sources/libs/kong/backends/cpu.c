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

static char *type_string_simd1(type_id type) {
	if (type == float_id) {
		return "float";
	}
	if (type == float2_id) {
		return "kore_float2";
	}
	if (type == float3_id) {
		return "kore_float3";
	}
	if (type == float4_id) {
		return "kore_float4";
	}
	if (type == float4x4_id) {
		return "kore_matrix4x4";
	}
	if (type == int2_id) {
		return "kore_int2";
	}
	if (type == int3_id) {
		return "kore_int3";
	}
	if (type == int4_id) {
		return "kore_int4";
	}
	if (type == uint_id) {
		return "uint32_t";
	}
	if (type == uint2_id) {
		return "kore_uint2";
	}
	if (type == uint3_id) {
		return "kore_uint3";
	}
	if (type == uint4_id) {
		return "kore_uint4";
	}
	return get_name(get_type(type)->name);
}

static char *type_string_simd4(type_id type) {
	if (type == float_id) {
		return "kore_float32x4";
	}
	if (type == float2_id) {
		return "kore_float2x4";
	}
	if (type == float3_id) {
		return "kore_float3x4";
	}
	if (type == float4_id) {
		return "kore_float4x4";
	}
	if (type == float4x4_id) {
		return "kore_matrix4x4";
	}
	if (type == int_id) {
		return "kore_int32x4";
	}
	if (type == int2_id) {
		return "kore_int2x4";
	}
	if (type == int3_id) {
		return "kore_int3x4";
	}
	if (type == int4_id) {
		return "kore_int4x4";
	}
	if (type == uint_id) {
		return "kore_uint32x4";
	}
	if (type == uint2_id) {
		return "kore_uint2x4";
	}
	if (type == uint3_id) {
		return "kore_uint3x4";
	}
	if (type == uint4_id) {
		return "kore_uint4x4";
	}
	return get_name(get_type(type)->name);
}

static char *type_string(type_id type, uint8_t simd_width) {
	if (simd_width == 4) {
		return type_string_simd4(type);
	}
	else if (simd_width == 1) {
		return type_string_simd1(type);
	}
	else {
		debug_context context = {0};
		error(context, "Unsupported simd width %i.", simd_width);
		return "type string error";
	}
}

static void write_code(char *code, char *header_code, char *directory, const char *filename, const char *name) {
	char full_filename[512];

	{
		sprintf(full_filename, "%s/%s.h", directory, filename);
		FILE *file = fopen(full_filename, "wb");
		fprintf(file, "#include <kong.h>\n\n");
		fprintf(file, "#include <stddef.h>\n");
		fprintf(file, "#include <stdint.h>\n\n");

		fprintf(file, "%s", header_code);

		fprintf(file, "void %s(uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z);\n\n", name);

		fclose(file);
	}

	{
		sprintf(full_filename, "%s/%s.c", directory, filename);

		FILE *file = fopen(full_filename, "wb");
		fprintf(file, "#include \"%s.h\"\n\n", filename);

		fprintf(file, "#include <kore3/math/vector.h>\n");
		fprintf(file, "#include <kore3/util/cpucompute.h>\n\n");

		fprintf(file, "%s", code);

		fclose(file);
	}
}

static void write_types(char *code, size_t *offset, function *main, uint8_t simd_width) {
	type_id types[256];
	size_t  types_size = 0;
	find_referenced_types(main, types, &types_size);

	for (size_t i = 0; i < types_size; ++i) {
		type *t = get_type(types[i]);

		if (!t->built_in && !has_attribute(&t->attributes, add_name("pipe"))) {
			*offset += sprintf(&code[*offset], "struct %s {\n", get_name(t->name));

			for (size_t j = 0; j < t->members.size; ++j) {
				*offset += sprintf(&code[*offset], "\t%s %s;\n", type_string(t->members.m[j].type.type, simd_width), get_name(t->members.m[j].name));
			}

			*offset += sprintf(&code[*offset], "};\n\n");
		}
	}
}

static void write_globals(char *code, size_t *offset, char *header_code, size_t *header_offset, function *main) {
	global_array globals = {0};

	find_referenced_globals(main, &globals);

	for (size_t i = 0; i < globals.size; ++i) {
		global *g = get_global(globals.globals[i]);

		type   *t         = get_type(g->type);
		type_id base_type = t->array_size > 0 ? t->base : g->type;

		if (base_type == sampler_type_id) {
			*offset += sprintf(&code[*offset], "SamplerState _%" PRIu64 ";\n\n", g->var_index);
		}
		else if (base_type == tex2d_type_id) {
			if (has_attribute(&g->attributes, add_name("write"))) {
				*offset += sprintf(&code[*offset], "RWTexture2D<float4> _%" PRIu64 ";\n\n", g->var_index);
			}
			else {
				if (t->array_size > 0 && t->array_size == UINT32_MAX) {
					*offset += sprintf(&code[*offset], "Texture2D<float4> _%" PRIu64 "[];\n\n", g->var_index);
				}
				else {
					*offset += sprintf(&code[*offset], "Texture2D<float4> _%" PRIu64 ";\n\n", g->var_index);
				}
			}
		}
		else if (base_type == tex2darray_type_id) {
			*offset += sprintf(&code[*offset], "Texture2DArray<float4> _%" PRIu64 ";\n\n", g->var_index);
		}
		else if (base_type == texcube_type_id) {
			*offset += sprintf(&code[*offset], "TextureCube<float4> _%" PRIu64 ";\n\n", g->var_index);
		}
		else if (base_type == bvh_type_id) {
			*offset += sprintf(&code[*offset], "RaytracingAccelerationStructure  _%" PRIu64 ";\n\n", g->var_index);
		}
		else if (base_type == float_id) {
			*offset += sprintf(&code[*offset], "static const float _%" PRIu64 " = %f;\n\n", g->var_index, g->value.value.floats[0]);
		}
		else if (base_type == float2_id) {
			*offset += sprintf(&code[*offset], "static const kore_float2 _%" PRIu64 " = float2(%f, %f);\n\n", g->var_index, g->value.value.floats[0],
			                   g->value.value.floats[1]);
		}
		else if (base_type == float3_id) {
			*offset += sprintf(&code[*offset], "static const kore_float3 _%" PRIu64 " = float3(%f, %f, %f);\n\n", g->var_index, g->value.value.floats[0],
			                   g->value.value.floats[1], g->value.value.floats[2]);
		}
		else if (base_type == float4_id) {
			if (t->array_size > 0) {
				*header_offset += sprintf(&header_code[*header_offset], "void set_%s(kore_float4 *value);\n\n", get_name(g->name));

				*offset += sprintf(&code[*offset], "static kore_float4 *_%llu;\n\n", g->var_index);
				*offset += sprintf(&code[*offset], "void set_%s(kore_float4 *value) {\n", get_name(g->name));
				*offset += sprintf(&code[*offset], "\t_%" PRIu64 " = value;\n", g->var_index);
				*offset += sprintf(&code[*offset], "}\n\n");
			}
			else {
				*offset += sprintf(&code[*offset], "static const float4 _%" PRIu64 " = float4(%f, %f, %f, %f);\n\n", g->var_index, g->value.value.floats[0],
				                   g->value.value.floats[1], g->value.value.floats[2], g->value.value.floats[3]);
			}
		}
		else {
			*header_offset += sprintf(&header_code[*header_offset], "void set_%s(%s_type *value);\n\n", get_name(g->name), get_name(g->name));

			*offset += sprintf(&code[*offset], "static %s_type *_%" PRIu64 ";\n\n", get_name(g->name), g->var_index);
			*offset += sprintf(&code[*offset], "void set_%s(%s_type *value) {\n", get_name(g->name), get_name(g->name));
			*offset += sprintf(&code[*offset], "\t_%" PRIu64 " = value;\n", g->var_index);
			*offset += sprintf(&code[*offset], "}\n\n");
		}
	}
}

static const char *type_to_mini(type_ref t) {
	if (t.type == int_id) {
		return "_i1";
	}
	else if (t.type == int2_id) {
		return "_i2";
	}
	else if (t.type == int3_id) {
		return "_i3";
	}
	else if (t.type == int4_id) {
		return "_i4";
	}
	else if (t.type == uint_id) {
		return "_u1";
	}
	else if (t.type == uint2_id) {
		return "_u2";
	}
	else if (t.type == uint3_id) {
		return "_u3";
	}
	else if (t.type == uint4_id) {
		return "_u4";
	}
	else if (t.type == float_id) {
		return "_f1";
	}
	else if (t.type == float2_id) {
		return "_f2";
	}
	else if (t.type == float3_id) {
		return "_f3";
	}
	else if (t.type == float4_id) {
		return "_f4";
	}
	else {
		debug_context context = {0};
		error(context, "Unknown parameter type");
		return "error";
	}
}

static void write_functions(char *code, const char *name, size_t *offset, function *main, uint8_t simd_width) {
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

		int indentation = 1;

		if (f == main) {
			attribute *threads_attribute = find_attribute(&f->attributes, add_name("threads"));
			if (threads_attribute == NULL || threads_attribute->paramters_count != 3) {
				debug_context context = {0};
				error(context, "Compute function requires a threads attribute with three parameters");
			}

			*offset += sprintf(&code[*offset], "void %s(uint32_t workgroup_count_x, uint32_t workgroup_count_y, uint32_t workgroup_count_z) {\n", name);

			*offset += sprintf(&code[*offset], "\tuint32_t local_size_x = %i;\n\tuint32_t local_size_y = %i;\n\tuint32_t local_size_z = %i;\n",
			                   (int)threads_attribute->parameters[0], (int)threads_attribute->parameters[1], (int)threads_attribute->parameters[2]);

			indent(code, offset, indentation);
			*offset += sprintf(&code[*offset], "for (uint32_t workgroup_index_z = 0; workgroup_index_z < workgroup_count_z; ++workgroup_index_z) {\n");
			++indentation;

			indent(code, offset, indentation);
			*offset += sprintf(&code[*offset], "for (uint32_t workgroup_index_y = 0; workgroup_index_y < workgroup_count_y; ++workgroup_index_y) {\n");
			++indentation;

			indent(code, offset, indentation);
			*offset += sprintf(&code[*offset], "for (uint32_t workgroup_index_x = 0; workgroup_index_x < workgroup_count_x; ++workgroup_index_x) {\n");
			++indentation;

			if (simd_width == 4) {
				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "for (uint32_t local_index_z = 0; local_index_z < local_size_z; ++local_index_z) {\n");
				++indentation;

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "for (uint32_t local_index_y = 0; local_index_y < local_size_y; ++local_index_y) {\n");
				++indentation;

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "for (uint32_t local_index_x = 0; local_index_x < local_size_x; local_index_x += 4) {\n");
				++indentation;

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "kore_uint3x4 group_id;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_id.x = kore_uint32x4_load_all(workgroup_index_x);\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_id.y = kore_uint32x4_load_all(workgroup_index_y);\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_id.z = kore_uint32x4_load_all(workgroup_index_z);\n\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "kore_uint3x4 group_thread_id;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset],
				                   "group_thread_id.x = kore_uint32x4_load(local_index_x, local_index_x + 1, local_index_x + 2, local_index_x + 3);\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_thread_id.y = kore_uint32x4_load_all(local_index_y);\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_thread_id.z = kore_uint32x4_load_all(local_index_z);\n\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "kore_uint3x4 dispatch_thread_id;\n");

				indent(code, offset, indentation);
				*offset += sprintf(
				    &code[*offset],
				    "dispatch_thread_id.x = kore_uint32x4_add(kore_uint32x4_mul(group_id.x, kore_uint32x4_load_all(workgroup_count_x)), group_thread_id.x);\n");

				indent(code, offset, indentation);
				*offset += sprintf(
				    &code[*offset],
				    "dispatch_thread_id.y = kore_uint32x4_add(kore_uint32x4_mul(group_id.y, kore_uint32x4_load_all(workgroup_count_y)), group_thread_id.y);\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "dispatch_thread_id.z = kore_uint32x4_add(kore_uint32x4_mul(group_id.z, "
				                                   "kore_uint32x4_load_all(workgroup_count_z)), group_thread_id.z);\n\n");

				indent(code, offset, indentation);
				*offset +=
				    sprintf(&code[*offset],
				            "kore_uint32x4 group_index = kore_uint32x4_add(kore_uint32x4_mul(group_thread_id.z, "
				            "kore_uint32x4_mul(kore_uint32x4_load_all(workgroup_count_x), kore_uint32x4_load_all(workgroup_count_y))), "
				            "kore_uint32x4_add(kore_uint32x4_mul(group_thread_id.y, kore_uint32x4_load_all(workgroup_count_x)), group_thread_id.x));\n\n");
			}
			else if (simd_width == 1) {
				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "for (uint32_t local_index_z = 0; local_index_z < local_size_z; ++local_index_z) {\n");
				++indentation;

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "for (uint32_t local_index_y = 0; local_index_y < local_size_y; ++local_index_y) {\n");
				++indentation;

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "for (uint32_t local_index_x = 0; local_index_x < local_size_x; ++local_index_x) {\n");
				++indentation;

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "kore_uint3 group_id;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_id.x = workgroup_index_x;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_id.y = workgroup_index_y;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_id.z = workgroup_index_z;\n\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "kore_uint3 group_thread_id;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_thread_id.x = local_index_x;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_thread_id.y = local_index_y;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "group_thread_id.z = local_index_z;\n\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "kore_uint3 dispatch_thread_id;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "dispatch_thread_id.x = group_id.x * workgroup_count_x + group_thread_id.x;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "dispatch_thread_id.y = group_id.y * workgroup_count_y + group_thread_id.y;\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "dispatch_thread_id.z = group_id.z * workgroup_count_z + group_thread_id.z;\n\n");

				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "uint32_t group_index = group_thread_id.z * workgroup_count_x * workgroup_count_y + group_thread_id.y * "
				                                   "workgroup_count_x + group_thread_id.x;\n\n");
			}
		}
		else {
			*offset += sprintf(&code[*offset], "%s %s(", type_string(f->return_type.type, simd_width), get_name(f->name));
			for (uint8_t parameter_index = 0; parameter_index < f->parameters_size; ++parameter_index) {
				if (parameter_index == 0) {
					*offset += sprintf(&code[*offset], "%s _%" PRIu64, type_string(f->parameter_types[parameter_index].type, simd_width),
					                   parameter_ids[parameter_index]);
				}
				else {
					*offset += sprintf(&code[*offset], ", %s _%" PRIu64, type_string(f->parameter_types[parameter_index].type, simd_width),
					                   parameter_ids[parameter_index]);
				}
			}
			*offset += sprintf(&code[*offset], ") {\n");
		}

		size_t index = 0;
		while (index < size) {
			opcode *o = (opcode *)&data[index];
			switch (o->type) {
			case OPCODE_ADD: {
				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_add", type_string(o->op_binary.result.type.type, simd_width),
				                   o->op_binary.result.index);
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.left.type));
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.right.type));
				*offset += sprintf(&code[*offset], "_x%i(_%" PRIu64 ", _%" PRIu64 ");\n", simd_width, o->op_binary.left.index, o->op_binary.right.index);
				break;
			}
			case OPCODE_SUB: {
				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_sub", type_string(o->op_binary.result.type.type, simd_width),
				                   o->op_binary.result.index);
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.left.type));
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.right.type));
				*offset += sprintf(&code[*offset], "_x%i(_%" PRIu64 ", _%" PRIu64 ");\n", simd_width, o->op_binary.left.index, o->op_binary.right.index);
				break;
			}
			case OPCODE_MULTIPLY: {
				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_mult", type_string(o->op_binary.result.type.type, simd_width),
				                   o->op_binary.result.index);
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.left.type));
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.right.type));
				*offset += sprintf(&code[*offset], "_x%i(_%" PRIu64 ", _%" PRIu64 ");\n", simd_width, o->op_binary.left.index, o->op_binary.right.index);
				break;
			}
			case OPCODE_DIVIDE: {
				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_div", type_string(o->op_binary.result.type.type, simd_width),
				                   o->op_binary.result.index);
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.left.type));
				*offset += sprintf(&code[*offset], "%s", type_to_mini(o->op_binary.right.type));
				*offset += sprintf(&code[*offset], "_x%i(_%" PRIu64 ", _%" PRIu64 ");\n", simd_width, o->op_binary.left.index, o->op_binary.right.index);
				break;
			}
			case OPCODE_LOAD_FLOAT_CONSTANT:
				indent(code, offset, indentation);
				if (simd_width == 1) {
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = %ff;\n", type_string(o->op_load_float_constant.to.type.type, simd_width),
					                   o->op_load_float_constant.to.index, o->op_load_float_constant.number);
				}
				else if (simd_width == 4) {
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_float32x4_load_all(%ff);\n",
					                   type_string(o->op_load_float_constant.to.type.type, simd_width), o->op_load_float_constant.to.index,
					                   o->op_load_float_constant.number);
				}
				break;
			case OPCODE_LOAD_INT_CONSTANT:
				indent(code, offset, indentation);
				if (simd_width == 1) {
					cstyle_write_opcode(code, offset, o, type_string_simd1, &indentation);
				}
				else if (simd_width == 4) {
					*offset +=
					    sprintf(&code[*offset], "%s _%" PRIu64 " = kore_int32x4_load_all(%i);\n", type_string(o->op_load_int_constant.to.type.type, simd_width),
					            o->op_load_int_constant.to.index, o->op_load_int_constant.number);
				}
				break;
			case OPCODE_CALL: {
				if (o->op_call.func == add_name("group_id")) {
					check(o->op_call.parameters_size == 0, context, "group_id can not have a parameter");
					indent(code, offset, indentation);
					*offset +=
					    sprintf(&code[*offset], "%s _%" PRIu64 " = group_id;\n", type_string(o->op_call.var.type.type, simd_width), o->op_call.var.index);
				}
				else if (o->op_call.func == add_name("group_thread_id")) {
					check(o->op_call.parameters_size == 0, context, "group_thread_id can not have a parameter");
					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = group_thread_id;\n", type_string(o->op_call.var.type.type, simd_width),
					                   o->op_call.var.index);
				}
				else if (o->op_call.func == add_name("dispatch_thread_id")) {
					check(o->op_call.parameters_size == 0, context, "dispatch_thread_id can not have a parameter");
					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = dispatch_thread_id;\n", type_string(o->op_call.var.type.type, simd_width),
					                   o->op_call.var.index);
				}
				else if (o->op_call.func == add_name("group_index")) {
					check(o->op_call.parameters_size == 0, context, "group_index can not have a parameter");
					indent(code, offset, indentation);
					*offset +=
					    sprintf(&code[*offset], "%s _%" PRIu64 " = group_index;\n", type_string(o->op_call.var.type.type, simd_width), o->op_call.var.index);
				}
				else {
					const char *function_name = get_name(o->op_call.func);
					if (o->op_call.func == add_name("float")) {
						function_name = "create_float";
					}
					else if (o->op_call.func == add_name("float2")) {
						function_name = "create_float2";
					}
					else if (o->op_call.func == add_name("float3")) {
						function_name = "create_float3";
					}
					else if (o->op_call.func == add_name("float4")) {
						function_name = "create_float4";
					}

					indent(code, offset, indentation);

					*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_%s", type_string(o->op_call.var.type.type, simd_width),
					                   o->op_call.var.index, function_name);

					for (uint8_t parameter_index = 0; parameter_index < o->op_call.parameters_size; ++parameter_index) {
						variable v = o->op_call.parameters[parameter_index];
						*offset += sprintf(&code[*offset], "%s", type_to_mini(v.type));
					}

					*offset += sprintf(&code[*offset], "_x%i(", simd_width);

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

				type *s = get_type(o->op_load_access_list.from.type.type);

				for (size_t i = 0; i < o->op_load_access_list.access_list_size; ++i) {
					switch (o->op_load_access_list.access_list[i].kind) {
					case ACCESS_ELEMENT:
						*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = _%" PRIu64, type_string(o->op_load_access_list.to.type.type, simd_width),
						                   o->op_load_access_list.to.index, o->op_load_access_list.from.index);
						*offset += sprintf(&code[*offset], "[_%" PRIu64 "]", o->op_load_access_list.access_list[i].access_element.index.index);
						break;
					case ACCESS_MEMBER:
						*offset += sprintf(&code[*offset], ".%s", get_name(o->op_load_access_list.access_list[i].access_member.name));

						if (simd_width == 1) {
							*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = _%" PRIu64, type_string(o->op_load_access_list.to.type.type, simd_width),
							                   o->op_load_access_list.to.index, o->op_load_access_list.from.index);

							if (global_var_index != 0) {
								*offset += sprintf(&code[*offset], ".%s", get_name(o->op_load_access_list.access_list[i].access_member.name));
							}
							else {
								*offset += sprintf(&code[*offset], ".%s", get_name(o->op_load_access_list.access_list[i].access_member.name));
							}
						}
						else if (simd_width == 4) {
							if (global_var_index != 0) {
								*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_float32x4_load_all(_%" PRIu64,
								                   type_string(o->op_load_access_list.to.type.type, simd_width), o->op_load_access_list.to.index,
								                   o->op_load_access_list.from.index);
								*offset += sprintf(&code[*offset], "->%s", get_name(o->op_load_access_list.access_list[i].access_member.name));
							}
							else {
								*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = _%" PRIu64, type_string(o->op_load_access_list.to.type.type, simd_width),
								                   o->op_load_access_list.to.index, o->op_load_access_list.from.index);
								*offset += sprintf(&code[*offset], ".%s", get_name(o->op_load_access_list.access_list[i].access_member.name));
							}
						}

						break;
					case ACCESS_SWIZZLE: {
						swizzle *swizzle = &o->op_load_access_list.access_list[i].access_swizzle.swizzle;

						if (o->op_load_access_list.from.type.type == uint2_id) {
							assert(swizzle->size == 1); // TODO

							if (swizzle->size == 1 && swizzle->indices[0] == 0) {
								*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_swizzle_x_u2_x%i(_%" PRIu64 ");\n",
								                   type_string(o->op_load_access_list.to.type.type, simd_width), o->op_load_access_list.to.index, simd_width,
								                   o->op_load_access_list.from.index);
							}
							else if (swizzle->size == 1 && swizzle->indices[0] == 1) {
								*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_swizzle_y_u2_x%i(_%" PRIu64 ");\n",
								                   type_string(o->op_load_access_list.to.type.type, simd_width), o->op_load_access_list.to.index, simd_width,
								                   o->op_load_access_list.from.index);
							}
							else {
								assert(false); // TODO
							}
						}
						else if (o->op_load_access_list.from.type.type == uint3_id) {
							assert(swizzle->size == 1); // TODO

							if (swizzle->size == 2 && swizzle->indices[0] == 0 && swizzle->indices[1] == 1) {
								*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_swizzle_xy_u3_x%i(_%" PRIu64 ");\n",
								                   type_string(o->op_load_access_list.to.type.type, simd_width), o->op_load_access_list.to.index, simd_width,
								                   o->op_load_access_list.from.index);
							}
							else if (swizzle->size == 1 && swizzle->indices[0] == 0) {
								*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_swizzle_x_u3_x%i(_%" PRIu64 ");\n",
								                   type_string(o->op_load_access_list.to.type.type, simd_width), o->op_load_access_list.to.index, simd_width,
								                   o->op_load_access_list.from.index);
							}
							else if (swizzle->size == 1 && swizzle->indices[0] == 1) {
								*offset += sprintf(&code[*offset], "%s _%" PRIu64 " = kore_cpu_compute_swizzle_y_u3_x%i(_%" PRIu64 ");\n",
								                   type_string(o->op_load_access_list.to.type.type, simd_width), o->op_load_access_list.to.index, simd_width,
								                   o->op_load_access_list.from.index);
							}
							else {
								assert(false); // TODO
							}
						}

						break;
					}
					}

					s = get_type(o->op_load_access_list.access_list[i].type);
				}

				*offset += sprintf(&code[*offset], ";\n");

				break;
			}
			case OPCODE_STORE_ACCESS_LIST:
			case OPCODE_SUB_AND_STORE_ACCESS_LIST:
			case OPCODE_ADD_AND_STORE_ACCESS_LIST:
			case OPCODE_DIVIDE_AND_STORE_ACCESS_LIST:
			case OPCODE_MULTIPLY_AND_STORE_ACCESS_LIST: {
				if (simd_width == 1) {
					cstyle_write_opcode(code, offset, o, type_string_simd1, &indentation);
				}
				else if (simd_width == 4) {
					for (int simd_index = 0; simd_index < 4; ++simd_index) {
						indent(code, offset, indentation);
						*offset += sprintf(&code[*offset], "_%" PRIu64, o->op_store_access_list.to.index);

						type *s = get_type(o->op_store_access_list.to.type.type);

						for (size_t i = 0; i < o->op_store_access_list.access_list_size; ++i) {
							switch (o->op_store_access_list.access_list[i].kind) {
							case ACCESS_ELEMENT:
								*offset += sprintf(&code[*offset], "[kore_int32x4_get(_%" PRIu64 ", %i)]",
								                   o->op_store_access_list.access_list[i].access_element.index.index, simd_index);
								break;
							case ACCESS_MEMBER:
								*offset += sprintf(&code[*offset], ".%s", get_name(o->op_store_access_list.access_list[i].access_member.name));
								break;
							case ACCESS_SWIZZLE: {
								char swizzle[4];

								for (uint32_t swizzle_index = 0; swizzle_index < o->op_store_access_list.access_list[i].access_swizzle.swizzle.size;
								     ++swizzle_index) {
									swizzle[swizzle_index] = "xyzw"[o->op_store_access_list.access_list[i].access_swizzle.swizzle.indices[swizzle_index]];
								}
								swizzle[o->op_store_access_list.access_list[i].access_swizzle.swizzle.size] = 0;

								*offset += sprintf(&code[*offset], ".%s", swizzle);

								break;
							}
							}

							s = get_type(o->op_store_access_list.access_list[i].type);
						}

						switch (o->type) {
						case OPCODE_STORE_ACCESS_LIST:
							*offset += sprintf(&code[*offset], " = _%" PRIu64 ";\n", o->op_store_access_list.from.index);

							*offset += sprintf(&code[*offset],
							                   " = kore_cpu_compute_create_float4(kore_float32x4_get(_%" PRIu64 ".x, %i), kore_float32x4_get(_%" PRIu64
							                   ".y, %i), kore_float32x4_get(_%" PRIu64 ".z, %i), kore_float32x4_get(_%" PRIu64 ".w, %i));\n",
							                   o->op_store_access_list.from.index, simd_index, o->op_store_access_list.from.index, simd_index,
							                   o->op_store_access_list.from.index, simd_index, o->op_store_access_list.from.index, simd_index);
							break;
						case OPCODE_SUB_AND_STORE_ACCESS_LIST:
							*offset += sprintf(&code[*offset], " -= _%" PRIu64 ";\n", o->op_store_access_list.from.index);
							break;
						case OPCODE_ADD_AND_STORE_ACCESS_LIST:
							*offset += sprintf(&code[*offset], " += _%" PRIu64 ";\n", o->op_store_access_list.from.index);
							break;
						case OPCODE_DIVIDE_AND_STORE_ACCESS_LIST:
							*offset += sprintf(&code[*offset], " /= _%" PRIu64 ";\n", o->op_store_access_list.from.index);
							break;
						case OPCODE_MULTIPLY_AND_STORE_ACCESS_LIST:
							*offset += sprintf(&code[*offset], " *= _%" PRIu64 ";\n", o->op_store_access_list.from.index);
							break;
						default:
							assert(false);
							break;
						}
					}
				}
				break;
			}
			case OPCODE_RETURN: {
				if (o->size > offsetof(opcode, op_return)) {
					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "return _%" PRIu64 ";\n", o->op_return.var.index);
				}
				else {
					indent(code, offset, indentation);
					*offset += sprintf(&code[*offset], "return;\n");
				}
				break;
			}
			default:
				if (simd_width == 1) {
					cstyle_write_opcode(code, offset, o, type_string_simd1, &indentation);
				}
				else if (simd_width == 4) {
					cstyle_write_opcode(code, offset, o, type_string_simd4, &indentation);
				}
				break;
			}

			index += o->size;
		}

		if (f == main) {
			for (int i = 0; i < 6; ++i) {
				--indentation;
				indent(code, offset, indentation);
				*offset += sprintf(&code[*offset], "}\n");
			}

			*offset += sprintf(&code[*offset], "}\n\n");
		}
		else {
			*offset += sprintf(&code[*offset], "}\n\n");
		}
	}
}

static void cpu_export_compute(char *directory, function *main) {
	uint8_t simd_width = 4;

	debug_context context = {0};

	char *code = (char *)calloc(1024 * 1024, 1);
	check(code != NULL, context, "Could not allocate code string");

	size_t offset = 0;

	char *header_code = (char *)calloc(1024 * 1024, 1);
	check(header_code != NULL, context, "Could not allocate code string");

	size_t header_offset = 0;

	assert(main->parameters_size == 0);

	write_types(code, &offset, main, simd_width);

	write_globals(code, &offset, header_code, &header_offset, main);

	char *name = get_name(main->name);

	char func_name[256];
	sprintf(func_name, "%s_on_cpu", name);

	write_functions(code, func_name, &offset, main, simd_width);

	char filename[512];
	sprintf(filename, "kong_cpu_%s", name);

	write_code(code, header_code, directory, filename, func_name);
}

void cpu_export(char *directory) {
	function *compute_shaders[256];
	size_t    compute_shaders_size = 0;

	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);
		if (has_attribute(&f->attributes, add_name("compute")) && has_attribute(&f->attributes, add_name("cpu"))) {
			compute_shaders[compute_shaders_size] = f;
			compute_shaders_size += 1;
		}
	}

	for (size_t i = 0; i < compute_shaders_size; ++i) {
		cpu_export_compute(directory, compute_shaders[i]);
	}
}
