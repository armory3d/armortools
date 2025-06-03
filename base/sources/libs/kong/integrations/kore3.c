#include "kore3.h"

#include "../analyzer.h"
#include "../backends/util.h"
#include "../compiler.h"
#include "../errors.h"
#include "../functions.h"
#include "../parser.h"
#include "../sets.h"
#include "../types.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static char *type_string(type_id type) {
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
	if (type == float3x3_id) {
		return "kore_matrix3x3";
	}
	if (type == float4x4_id) {
		return "kore_matrix4x4";
	}
	if (type == int_id) {
		return "int32_t";
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

static const char *structure_type(type_id type, api_kind api) {
	if (api == API_DIRECT3D11) {
		if (type == float_id) {
			return "KORE_D3D11_VERTEX_FORMAT_FLOAT32";
		}
		if (type == float2_id) {
			return "KORE_D3D11_VERTEX_FORMAT_FLOAT32X2";
		}
		if (type == float3_id) {
			return "KORE_D3D11_VERTEX_FORMAT_FLOAT32X3";
		}
		if (type == float4_id) {
			return "KORE_D3D11_VERTEX_FORMAT_FLOAT32X4";
		}
	}
	else if (api == API_DIRECT3D12) {
		if (type == float_id) {
			return "KORE_D3D12_VERTEX_FORMAT_FLOAT32";
		}
		if (type == float2_id) {
			return "KORE_D3D12_VERTEX_FORMAT_FLOAT32X2";
		}
		if (type == float3_id) {
			return "KORE_D3D12_VERTEX_FORMAT_FLOAT32X3";
		}
		if (type == float4_id) {
			return "KORE_D3D12_VERTEX_FORMAT_FLOAT32X4";
		}
	}
	else if (api == API_METAL) {
		if (type == float_id) {
			return "KORE_METAL_VERTEX_FORMAT_FLOAT32";
		}
		if (type == float2_id) {
			return "KORE_METAL_VERTEX_FORMAT_FLOAT32X2";
		}
		if (type == float3_id) {
			return "KORE_METAL_VERTEX_FORMAT_FLOAT32X3";
		}
		if (type == float4_id) {
			return "KORE_METAL_VERTEX_FORMAT_FLOAT32X4";
		}
	}
	else if (api == API_VULKAN) {
		if (type == float_id) {
			return "KORE_VULKAN_VERTEX_FORMAT_FLOAT32";
		}
		if (type == float2_id) {
			return "KORE_VULKAN_VERTEX_FORMAT_FLOAT32X2";
		}
		if (type == float3_id) {
			return "KORE_VULKAN_VERTEX_FORMAT_FLOAT32X3";
		}
		if (type == float4_id) {
			return "KORE_VULKAN_VERTEX_FORMAT_FLOAT32X4";
		}
	}
	else if (api == API_WEBGPU) {
		if (type == float_id) {
			return "KORE_WEBGPU_VERTEX_FORMAT_FLOAT32";
		}
		if (type == float2_id) {
			return "KORE_WEBGPU_VERTEX_FORMAT_FLOAT32X2";
		}
		if (type == float3_id) {
			return "KORE_WEBGPU_VERTEX_FORMAT_FLOAT32X3";
		}
		if (type == float4_id) {
			return "KORE_WEBGPU_VERTEX_FORMAT_FLOAT32X4";
		}
	}
	else if (api == API_OPENGL) {
		if (type == float_id) {
			return "KORE_OPENGL_VERTEX_FORMAT_FLOAT32";
		}
		if (type == float2_id) {
			return "KORE_OPENGL_VERTEX_FORMAT_FLOAT32X2";
		}
		if (type == float3_id) {
			return "KORE_OPENGL_VERTEX_FORMAT_FLOAT32X3";
		}
		if (type == float4_id) {
			return "KORE_OPENGL_VERTEX_FORMAT_FLOAT32X4";
		}
	}
	debug_context context = {0};
	error(context, "Unknown type for vertex structure");
	return "UNKNOWN";
}

static const char *convert_compare_mode(int mode) {
	switch (mode) {
	case 0:
		return "KORE_GPU_COMPARE_FUNCTION_ALWAYS";
	case 1:
		return "KORE_GPU_COMPARE_FUNCTION_NEVER";
	case 2:
		return "KORE_GPU_COMPARE_FUNCTION_EQUAL";
	case 3:
		return "KORE_GPU_COMPARE_FUNCTION_NOT_EQUAL";
	case 4:
		return "KORE_GPU_COMPARE_FUNCTION_LESS";
	case 5:
		return "KORE_GPU_COMPARE_FUNCTION_LESS_EQUAL";
	case 6:
		return "KORE_GPU_COMPARE_FUNCTION_GREATER";
	case 7:
		return "KORE_GPU_COMPARE_FUNCTION_GREATER_EQUAL";
	default: {
		debug_context context = {0};
		error(context, "Unknown compare mode");
		return "UNKNOWN";
	}
	}
}

static const char *convert_texture_format(int format) {
	switch (format) {
	case 0:
		return "KORE_GPU_TEXTURE_FORMAT_R8_UNORM";
	case 1:
		return "KORE_GPU_TEXTURE_FORMAT_R8_SNORM";
	case 2:
		return "KORE_GPU_TEXTURE_FORMAT_R8_UINT";
	case 3:
		return "KORE_GPU_TEXTURE_FORMAT_R8_SINT";
	case 4:
		return "KORE_GPU_TEXTURE_FORMAT_R16_UINT";
	case 5:
		return "KORE_GPU_TEXTURE_FORMAT_R16_SINT";
	case 6:
		return "KORE_GPU_TEXTURE_FORMAT_R16_FLOAT";
	case 7:
		return "KORE_GPU_TEXTURE_FORMAT_RG8_UNORM";
	case 8:
		return "KORE_GPU_TEXTURE_FORMAT_RG8_SNORM";
	case 9:
		return "KORE_GPU_TEXTURE_FORMAT_RG8_UINT";
	case 10:
		return "KORE_GPU_TEXTURE_FORMAT_RG8_SINT";
	case 11:
		return "KORE_GPU_TEXTURE_FORMAT_R32_UINT";
	case 12:
		return "KORE_GPU_TEXTURE_FORMAT_R32_SINT";
	case 13:
		return "KORE_GPU_TEXTURE_FORMAT_R32_FLOAT";
	case 14:
		return "KORE_GPU_TEXTURE_FORMAT_RG16_UINT";
	case 15:
		return "KORE_GPU_TEXTURE_FORMAT_RG16_SINT";
	case 16:
		return "KORE_GPU_TEXTURE_FORMAT_RG16_FLOAT";
	case 17:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM";
	case 18:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM_SRGB";
	case 19:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA8_SNORM";
	case 20:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA8_UINT";
	case 21:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA8_SINT";
	case 22:
		return "KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM";
	case 23:
		return "KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM_SRGB";
	case 24:
		return "KORE_GPU_TEXTURE_FORMAT_RGB9E5U_FLOAT";
	case 25:
		return "KORE_GPU_TEXTURE_FORMAT_RGB10A2_UINT";
	case 26:
		return "KORE_GPU_TEXTURE_FORMAT_RGB10A2_UNORM";
	case 27:
		return "KORE_GPU_TEXTURE_FORMAT_RG11B10U_FLOAT";
	case 28:
		return "KORE_GPU_TEXTURE_FORMAT_RG32_UINT";
	case 29:
		return "KORE_GPU_TEXTURE_FORMAT_RG32_SINT";
	case 30:
		return "KORE_GPU_TEXTURE_FORMAT_RG32_FLOAT";
	case 31:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA16_UINT";
	case 32:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA16_SINT";
	case 33:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA16_FLOAT";
	case 34:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA32_UINT";
	case 35:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA32_SINT";
	case 36:
		return "KORE_GPU_TEXTURE_FORMAT_RGBA32_FLOAT";
	case 37:
		return "KORE_GPU_TEXTURE_FORMAT_DEPTH16_UNORM";
	case 38:
		return "KORE_GPU_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8";
	case 39:
		return "KORE_GPU_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8";
	case 40:
		return "KORE_GPU_TEXTURE_FORMAT_DEPTH32FLOAT";
	case 41:
		return "KORE_GPU_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24";
	default: {
		debug_context context = {0};
		error(context, "Unknown texture format");
		return "UNKNOWN";
	}
	}
}

static char blend_mode[64];

static const char *convert_blend_mode(int mode, const char *api) {
	switch (mode) {
	case 0:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_ZERO", api);
		return blend_mode;
	case 1:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_ONE", api);
		return blend_mode;
	case 2:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_SRC", api);
		return blend_mode;
	case 3:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_ONE_MINUS_SRC", api);
		return blend_mode;
	case 4:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_SRC_ALPHA", api);
		return blend_mode;
	case 5:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA", api);
		return blend_mode;
	case 6:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_DST", api);
		return blend_mode;
	case 7:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_ONE_MINUS_DST", api);
		return blend_mode;
	case 8:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_DST_ALPHA", api);
		return blend_mode;
	case 9:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_ONE_MINUS_DST_ALPHA", api);
		return blend_mode;
	case 10:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_SRC_ALPHA_SATURATED", api);
		return blend_mode;
	case 11:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_CONSTANT", api);
		return blend_mode;
	case 12:
		sprintf(blend_mode, "KORE_%s_BLEND_FACTOR_ONE_MINUS_CONSTANT", api);
		return blend_mode;
	default: {
		debug_context context = {0};
		error(context, "Unknown blend mode");
		return "UNKNOWN";
	}
	}
}

static char blend_op[64];

static const char *convert_blend_op(int op, const char *api) {
	switch (op) {
	case 0:
		sprintf(blend_op, "KORE_%s_BLEND_OPERATION_ADD", api);
		return blend_op;
	case 1:
		sprintf(blend_op, "KORE_%s_BLEND_OPERATION_SUBTRACT", api);
		return blend_op;
	case 2:
		sprintf(blend_op, "KORE_%s_BLEND_OPERATION_REVERSE_SUBTRACT", api);
		return blend_op;
	case 3:
		sprintf(blend_op, "KORE_%s_BLEND_OPERATION_MIN", api);
		return blend_op;
	case 4:
		sprintf(blend_op, "KORE_%s_BLEND_OPERATION_MAX", api);
		return blend_op;
	default: {
		debug_context context = {0};
		error(context, "Unknown blend op");
		return "UNKNOWN";
	}
	}
}

static member *find_member(type *t, char *name) {
	for (size_t j = 0; j < t->members.size; ++j) {
		if (t->members.m[j].name == add_name(name)) {
			return &t->members.m[j];
		}
	}

	return NULL;
}

static void write_root_signature(FILE *output, descriptor_set *all_descriptor_sets[256], size_t all_descriptor_sets_count) {
	uint32_t cbv_index     = 0;
	uint32_t srv_index     = 0;
	uint32_t uav_index     = 0;
	uint32_t sampler_index = 0;

	uint32_t table_count = 0;

	for (size_t set_index = 0; set_index < all_descriptor_sets_count; ++set_index) {
		descriptor_set *set = all_descriptor_sets[set_index];

		bool has_sampler = false;
		bool has_other   = false;

		for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
			global *g = get_global(set->globals.globals[global_index]);

			if (is_cbv_srv_uav(g->type)) {
				has_other = true;
			}
			else if (is_sampler(g->type)) {
				has_sampler = true;
			}

			if (has_other && has_sampler) {
				break;
			}
		}

		if (has_other && has_sampler) {
			table_count += 2;
		}
		else if (has_other || has_sampler) {
			table_count += 1;
		}
	}

	fprintf(output, "\tD3D12_ROOT_PARAMETER params[%i] = {0};\n", table_count);

	uint32_t table_index = 0;

	for (size_t set_index = 0; set_index < all_descriptor_sets_count; ++set_index) {
		descriptor_set *set = all_descriptor_sets[set_index];

		bool has_sampler = false;
		bool has_other   = false;

		for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
			global *g = get_global(set->globals.globals[global_index]);

			if (is_cbv_srv_uav(g->type)) {
				has_other = true;
			}
			else if (is_sampler(g->type)) {
				has_sampler = true;
			}

			if (has_other && has_sampler) {
				break;
			}
		}

		if (has_other) {
			size_t count = 0;

			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);

				if (is_cbv_srv_uav(g->type)) {
					count += 1;
				}
			}

			fprintf(output, "\n\tD3D12_DESCRIPTOR_RANGE ranges%i[%zu] = {0};\n", table_index, count);

			size_t range_index = 0;
			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g        = get_global(set->globals.globals[global_index]);
				bool    writable = set->globals.writable[global_index];

				if (!get_type(g->type)->built_in) {
					fprintf(output, "\tranges%i[%zu].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;\n", table_index, range_index);
					fprintf(output, "\tranges%i[%zu].BaseShaderRegister = %i;\n", table_index, range_index, cbv_index);
					fprintf(output, "\tranges%i[%zu].NumDescriptors = 1;\n", table_index, range_index);
					fprintf(output, "\tranges%i[%zu].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;\n", table_index, range_index);

					cbv_index += 1;

					range_index += 1;
				}
				else if (is_texture(g->type)) {
					if (writable) {
						fprintf(output, "\tranges%i[%zu].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;\n", table_index, range_index);
						fprintf(output, "\tranges%i[%zu].BaseShaderRegister = %i;\n", table_index, range_index, uav_index);
						fprintf(output, "\tranges%i[%zu].NumDescriptors = 1;\n", table_index, range_index);
						fprintf(output, "\tranges%i[%zu].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;\n", table_index,
						        range_index);

						uav_index += 1;
					}
					else {
						fprintf(output, "\tranges%i[%zu].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;\n", table_index, range_index);
						fprintf(output, "\tranges%i[%zu].BaseShaderRegister = %i;\n", table_index, range_index, srv_index);
						fprintf(output, "\tranges%i[%zu].NumDescriptors = 1;\n", table_index, range_index);
						fprintf(output, "\tranges%i[%zu].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;\n", table_index,
						        range_index);

						srv_index += 1;
					}

					range_index += 1;
				}
				else if (g->type == bvh_type_id) {
					fprintf(output, "\tranges%i[%zu].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;\n", table_index, range_index);
					fprintf(output, "\tranges%i[%zu].BaseShaderRegister = %i;\n", table_index, range_index, srv_index);
					fprintf(output, "\tranges%i[%zu].NumDescriptors = 1;\n", table_index, range_index);
					fprintf(output, "\tranges%i[%zu].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;\n", table_index, range_index);

					srv_index += 1;

					range_index += 1;
				}
			}

			fprintf(output, "\n\tparams[%i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;\n", table_index);
			fprintf(output, "\tparams[%i].DescriptorTable.NumDescriptorRanges = %zu;\n", table_index, count);
			fprintf(output, "\tparams[%i].DescriptorTable.pDescriptorRanges = ranges%i;\n", table_index, table_index);

			table_index += 1;
		}

		if (has_sampler) {
			size_t count = 0;

			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);
				if (is_sampler(g->type)) {
					count += 1;
				}
			}

			fprintf(output, "\n\tD3D12_DESCRIPTOR_RANGE ranges%i[%zu] = {0};\n", table_index, count);

			size_t range_index = 0;
			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);

				if (is_sampler(g->type)) {
					fprintf(output, "\tranges%i[%zu].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;\n", table_index, range_index);
					fprintf(output, "\tranges%i[%zu].BaseShaderRegister = %i;\n", table_index, range_index, sampler_index);
					fprintf(output, "\tranges%i[%zu].NumDescriptors = 1;\n", table_index, range_index);
					fprintf(output, "\tranges%i[%zu].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;\n", table_index, range_index);
					sampler_index += 1;
				}
			}

			fprintf(output, "\n\tparams[%i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;\n", table_index);
			fprintf(output, "\tparams[%i].DescriptorTable.NumDescriptorRanges = %zu;\n", table_index, count);
			fprintf(output, "\tparams[%i].DescriptorTable.pDescriptorRanges = ranges%i;\n", table_index, table_index);

			table_index += 1;
		}
	}

	fprintf(output, "\n\tD3D12_ROOT_SIGNATURE_DESC desc = {0};\n");
	fprintf(output, "\tdesc.NumParameters = %i;\n", table_count);
	fprintf(output, "\tdesc.pParameters = params;\n");

	fprintf(output, "\n\tID3D12RootSignature* root_signature;\n");
	fprintf(output, "\tID3DBlob *blob;\n");
	fprintf(output, "\tD3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, NULL);\n");
	fprintf(
	    output,
	    "\tdevice->d3d12.device->lpVtbl->CreateRootSignature(device->d3d12.device, 0, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), "
	    "&IID_ID3D12RootSignature, &root_signature);\n");
	fprintf(output, "\tblob->lpVtbl->Release(blob);\n");

	fprintf(output, "\n\treturn root_signature;\n");
}

static void to_upper(char *from, char *to) {
	size_t from_size = strlen(from);
	for (size_t i = 0; i < from_size; ++i) {
		if (from[i] >= 'a' && from[i] <= 'z') {
			to[i] = from[i] + ('A' - 'a');
		}
		else {
			to[i] = from[i];
		}
	}
	to[from_size] = 0;
}

static int global_register_indices[512];

void kore3_export(char *directory, api_kind api) {
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);
		find_used_capabilities(f);
	}

	memset(global_register_indices, 0, sizeof(global_register_indices));

	char *api_short = NULL;
	char *api_long  = NULL;
	char *api_caps  = NULL;
	switch (api) {
	case API_DIRECT3D11:
		api_short = "d3d11";
		api_long  = "direct3d11";
		api_caps  = "D3D11";
		break;
	case API_DIRECT3D12:
		api_short = "d3d12";
		api_long  = "direct3d12";
		api_caps  = "D3D12";
		break;
	case API_METAL:
		api_short = "metal";
		api_long  = "metal";
		api_caps  = "METAL";
		break;
	case API_OPENGL:
		api_short = "opengl";
		api_long  = "opengl";
		api_caps  = "OPENGL";
		break;
	case API_VULKAN:
		api_short = "vulkan";
		api_long  = "vulkan";
		api_caps  = "VULKAN";
		break;
	case API_WEBGPU:
		api_short = "webgpu";
		api_long  = "webgpu";
		api_caps  = "WEBGPU";
		break;
	default:
		assert(false);
		break;
	}

	if (api == API_WEBGPU) {
		int binding_index = 0;

		for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
			global *g = get_global(i);
			if (g->type == sampler_type_id) {
				global_register_indices[i] = binding_index;
				binding_index += 1;
			}
			else if (is_texture(g->type)) {
				global_register_indices[i] = binding_index;
				binding_index += 1;
			}
			else if (g->type == float_id) {
			}
			else {
				global_register_indices[i] = binding_index;
				binding_index += 1;
			}
		}
	}
	else {
		int cbuffer_index = 0;
		int texture_index = 0;
		int sampler_index = 0;

		for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
			global *g = get_global(i);
			if (g->type == sampler_type_id) {
				global_register_indices[i] = sampler_index;
				sampler_index += 1;
			}
			else if (is_texture(g->type)) {
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
	}

	type_id vertex_inputs[256]              = {0};
	size_t  vertex_input_slots[256]         = {0};
	bool    vertex_inputs_per_instance[256] = {0};
	size_t  vertex_inputs_size              = 0;

	for (type_id i = 0; get_type(i) != NULL; ++i) {
		type *t = get_type(i);
		if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
			name_id vertex_shader_name = NO_NAME;
			name_id mesh_shader_name   = NO_NAME;

			for (size_t j = 0; j < t->members.size; ++j) {
				if (t->members.m[j].name == add_name("vertex")) {
					debug_context context = {0};
					check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "vertex expects an identifier");
					vertex_shader_name = t->members.m[j].value.identifier;
				}
				if (t->members.m[j].name == add_name("mesh")) {
					debug_context context = {0};
					check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "mesh expects an identifier");
					mesh_shader_name = t->members.m[j].value.identifier;
				}
			}

			debug_context context = {0};
			check(vertex_shader_name != NO_NAME || mesh_shader_name != NO_NAME, context, "No vertex or mesh shader name found");

			if (vertex_shader_name != NO_NAME) {
				for (function_id i = 0; get_function(i) != NULL; ++i) {
					function *f = get_function(i);
					if (f->name == vertex_shader_name) {
						check(f->parameters_size > 0, context, "Vertex function requires at least one parameter");

						for (size_t input_index = 0; input_index < f->parameters_size; ++input_index) {
							vertex_inputs[vertex_inputs_size]              = f->parameter_types[input_index].type;
							vertex_input_slots[vertex_inputs_size]         = input_index;
							vertex_inputs_per_instance[vertex_inputs_size] = f->parameter_attributes[input_index] == add_name("per_instance");
							vertex_inputs_size += 1;
						}

						break;
					}
				}
			}
		}
	}

	descriptor_set *sets[256];
	size_t          sets_count = 0;

	{
		char filename[512];
		sprintf(filename, "%s/%s", directory, "kong.h");

		FILE *output = fopen(filename, "wb");

		fprintf(output, "#ifndef KONG_INTEGRATION_HEADER\n");
		fprintf(output, "#define KONG_INTEGRATION_HEADER\n\n");

		fprintf(output, "#include <kore3/gpu/device.h>\n");
		fprintf(output, "#include <kore3/gpu/sampler.h>\n");
		fprintf(output, "#include <kore3/%s/descriptorset_structs.h>\n", api_long);
		fprintf(output, "#include <kore3/%s/pipeline_structs.h>\n", api_long);
		fprintf(output, "#include <kore3/math/matrix.h>\n");
		fprintf(output, "#include <kore3/math/vector.h>\n\n");

		fprintf(output, "#ifdef __cplusplus\n");
		fprintf(output, "extern \"C\" {\n");
		fprintf(output, "#endif\n");

		fprintf(output, "\nvoid kong_init(kore_gpu_device *device);\n\n");

		for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
			global *g = get_global(i);

			type_id base_type = get_type(g->type)->array_size > 0 ? get_type(g->type)->base : g->type;

			if (is_texture(g->type)) {
				fprintf(output, "uint32_t %s_texture_usage_flags(void);\n", get_name(g->name));
			}
			else if (!get_type(base_type)->built_in) {
				type *t = get_type(base_type);

				char name[256];
				if (t->name != NO_NAME) {
					strcpy(name, get_name(t->name));
				}
				else {
					strcpy(name, get_name(g->name));
					strcat(name, "_type");
				}

				fprintf(output, "typedef struct %s {\n", name);
				for (size_t j = 0; j < t->members.size; ++j) {
					fprintf(output, "\t%s %s;\n", type_string(t->members.m[j].type.type), get_name(t->members.m[j].name));
					if (t->members.m[j].type.type == float3x3_id) {
						fprintf(output, "\tfloat pad%zu[3];\n", j);
					}
				}
				fprintf(output, "} %s;\n\n", name);

				bool is_root_constant = false;
				for (size_t set_index = 0; set_index < g->sets_count; ++set_index) {
					if (g->sets[set_index]->name == add_name("root_constants")) {
						is_root_constant = true;
						break;
					}
				}

				if (is_root_constant) {
					fprintf(output, "void kong_set_root_constants_%s(kore_gpu_command_list *list, %s *constants);\n", get_name(g->name), name);
				}
				else {
					fprintf(output, "uint32_t %s_buffer_usage_flags(void);\n", name);
					fprintf(output, "void %s_buffer_create(kore_gpu_device *device, kore_gpu_buffer *buffer, uint32_t count);\n", name);
					fprintf(output, "void %s_buffer_destroy(kore_gpu_buffer *buffer);\n", name);
					fprintf(output, "%s *%s_buffer_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count);\n", name, name);
					fprintf(output, "%s *%s_buffer_try_to_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count);\n", name, name);
					fprintf(output, "void %s_buffer_unlock(kore_gpu_buffer *buffer);\n", name);
				}
			}
		}

		fprintf(output, "\n");

		for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
			global *g = get_global(i);
			for (size_t set_index = 0; set_index < g->sets_count; ++set_index) {
				descriptor_set *set = g->sets[set_index];

				if (set == NULL) {
					continue;
				}

				bool found = false;
				for (size_t set_index = 0; set_index < sets_count; ++set_index) {
					if (sets[set_index] == set) {
						found = true;
						break;
					}
				}

				if (!found) {
					sets[sets_count] = set;
					sets_count += 1;
				}
			}
		}

		for (size_t set_index = 0; set_index < sets_count; ++set_index) {
			descriptor_set *set = sets[set_index];

			if (set->name == add_name("root_constants")) {
				continue;
			}

			fprintf(output, "typedef struct %s_parameters {\n", get_name(set->name));
			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);

				if (!get_type(g->type)->built_in) {
					fprintf(output, "\tkore_gpu_buffer *%s;\n", get_name(g->name));
				}
				else if (is_texture(g->type)) {
					type *t = get_type(g->type);
					if (t->array_size == UINT32_MAX) {
						fprintf(output, "\tkore_gpu_texture_view *%s;\n", get_name(g->name));
						fprintf(output, "\tsize_t %s_count;\n", get_name(g->name));
					}
					else {
						fprintf(output, "\tkore_gpu_texture_view %s;\n", get_name(g->name));
					}
				}
				else if (is_sampler(g->type)) {
					fprintf(output, "\tkore_gpu_sampler *%s;\n", get_name(g->name));
				}
				else if (g->type == bvh_type_id) {
					fprintf(output, "\tkore_gpu_raytracing_hierarchy *%s;\n", get_name(g->name));
				}
				else {
					fprintf(output, "\tkore_gpu_buffer *%s;\n", get_name(g->name));
				}
			}

			fprintf(output, "} %s_parameters;\n\n", get_name(set->name));

			fprintf(output, "typedef struct %s_set {\n", get_name(set->name));
			fprintf(output, "\tkore_%s_descriptor_set set;\n\n", api_short);

			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);

				if (!get_type(g->type)->built_in) {
					fprintf(output, "\tkore_gpu_buffer *%s;\n", get_name(g->name));
				}
				else if (g->type == bvh_type_id) {
					fprintf(output, "\tkore_gpu_raytracing_hierarchy *%s;\n", get_name(g->name));
				}
				else if (is_texture(g->type)) {
					type *t = get_type(g->type);
					if (t->array_size == UINT32_MAX) {
						fprintf(output, "\tkore_gpu_texture_view *%s;\n", get_name(g->name));
						fprintf(output, "\tsize_t %s_count;\n", get_name(g->name));
					}
					else {
						fprintf(output, "\tkore_gpu_texture_view %s;\n", get_name(g->name));
						if (api == API_METAL) {
							fprintf(output, "\tvoid *%s_view;\n", get_name(g->name));
						}
					}
				}
				else if (is_sampler(g->type)) {
					fprintf(output, "\tkore_gpu_sampler *%s;\n", get_name(g->name));
				}
				else {
					fprintf(output, "\tkore_gpu_buffer *%s;\n", get_name(g->name));
				}
			}
			fprintf(output, "} %s_set;\n\n", get_name(set->name));

			fprintf(output, "void kong_create_%s_set(kore_gpu_device *device, const %s_parameters *parameters, %s_set *set);\n", get_name(set->name),
			        get_name(set->name), get_name(set->name));

			fprintf(output, "void kong_set_descriptor_set_%s(kore_gpu_command_list *list, %s_set *set", get_name(set->name), get_name(set->name));
			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);

				if (!get_type(g->type)->built_in) {
					if (has_attribute(&g->attributes, add_name("indexed"))) {
						fprintf(output, ", uint32_t %s_index", get_name(g->name));
					}
				}
			}
			fprintf(output, ");\n\n");

			char upper_set_name[256];
			to_upper(get_name(set->name), upper_set_name);

			fprintf(output, "typedef struct %s_set_update {\n", get_name(set->name));
			fprintf(output, "\tenum {\n");
			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);
				char    upper_definition_name[256];
				to_upper(get_name(g->name), upper_definition_name);

				if (!get_type(g->type)->built_in) {
					fprintf(output, "\t\t%s_SET_UPDATE_%s,\n", upper_set_name, upper_definition_name);
				}
				else if (g->type == bvh_type_id) {
					fprintf(output, "\t\t%s_SET_UPDATE_%s,\n", upper_set_name, upper_definition_name);
				}
				else if (is_texture(g->type)) {
					// type *t = get_type(get_global(d.global)->type);
					fprintf(output, "\t\t%s_SET_UPDATE_%s,\n", upper_set_name, upper_definition_name);
				}
			}
			fprintf(output, "\t} kind;\n");

			fprintf(output, "\tunion {\n");
			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);

				if (!get_type(g->type)->built_in) {
					fprintf(output, "\t\tkore_gpu_buffer *%s;\n", get_name(g->name));
				}
				else if (g->type == bvh_type_id) {
					fprintf(output, "\t\tkore_gpu_raytracing_hierarchy *%s;\n", get_name(g->name));
				}
				else if (is_texture(g->type)) {
					type *t = get_type(g->type);
					if (t->array_size == UINT32_MAX) {
						fprintf(output, "\t\tstruct {\n");
						fprintf(output, "\t\t\tkore_gpu_texture_view *%s;\n", get_name(g->name));
						fprintf(output, "\t\t\tuint32_t *%s_indices;\n", get_name(g->name));
						fprintf(output, "\t\t\tsize_t %s_count;\n", get_name(g->name));
						fprintf(output, "\t\t} %s;\n", get_name(g->name));
					}
					else {
						fprintf(output, "\t\tkore_gpu_texture_view %s;\n", get_name(g->name));
					}
				}
			}
			fprintf(output, "\t};\n");

			fprintf(output, "} %s_set_update;\n\n", get_name(set->name));

			fprintf(output, "void kong_update_%s_set(%s_set *set, %s_set_update *updates, uint32_t updates_count);\n", get_name(set->name), get_name(set->name),
			        get_name(set->name));
		}

		fprintf(output, "\n");

		for (size_t i = 0; i < vertex_inputs_size; ++i) {
			type *t = get_type(vertex_inputs[i]);

			fprintf(output, "typedef struct %s {\n", get_name(t->name));
			for (size_t j = 0; j < t->members.size; ++j) {
				fprintf(output, "\t%s %s;\n", type_string(t->members.m[j].type.type), get_name(t->members.m[j].name));
			}
			fprintf(output, "} %s;\n\n", get_name(t->name));

			fprintf(output, "typedef struct %s_buffer {\n", get_name(t->name));
			fprintf(output, "\tkore_gpu_buffer buffer;\n");
			fprintf(output, "\tsize_t count;\n");
			fprintf(output, "} %s_buffer;\n\n", get_name(t->name));

			fprintf(output, "uint32_t kong_%s_buffer_usage_flags(void);\n", get_name(t->name));
			fprintf(output, "void kong_create_buffer_%s(kore_gpu_device * device, size_t count, %s_buffer *buffer);\n", get_name(t->name), get_name(t->name));
			fprintf(output, "void kong_destroy_buffer_%s(%s_buffer *buffer);\n", get_name(t->name), get_name(t->name));
			fprintf(output, "%s *kong_%s_buffer_lock(%s_buffer *buffer);\n", get_name(t->name), get_name(t->name), get_name(t->name));
			fprintf(output, "%s *kong_%s_buffer_try_to_lock(%s_buffer *buffer);\n", get_name(t->name), get_name(t->name), get_name(t->name));
			fprintf(output, "void kong_%s_buffer_unlock(%s_buffer *buffer);\n", get_name(t->name), get_name(t->name));
			fprintf(output, "void kong_set_vertex_buffer_%s(kore_gpu_command_list *list, %s_buffer *buffer);\n\n", get_name(t->name), get_name(t->name));
		}

		for (type_id i = 0; get_type(i) != NULL; ++i) {
			type *t = get_type(i);
			if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
				fprintf(output, "void kong_set_render_pipeline_%s(kore_gpu_command_list *list);\n\n", get_name(t->name));
			}
		}

		for (function_id i = 0; get_function(i) != NULL; ++i) {
			function *f = get_function(i);
			if (has_attribute(&f->attributes, add_name("compute"))) {
				fprintf(output, "void kong_set_compute_shader_%s(kore_gpu_command_list *list);\n\n", get_name(f->name));
			}
		}

		for (type_id i = 0; get_type(i) != NULL; ++i) {
			type *t = get_type(i);
			if (!t->built_in && has_attribute(&t->attributes, add_name("raypipe"))) {
				fprintf(output, "void kong_set_ray_pipeline_%s(kore_gpu_command_list *list);\n\n", get_name(t->name));
			}
		}

		fprintf(output, "#ifdef __cplusplus\n");
		fprintf(output, "}\n");
		fprintf(output, "#endif\n\n");

		fprintf(output, "#endif\n");

		fclose(output);
	}

	{
		char filename[512];
		if (api == API_METAL) {
			sprintf(filename, "%s/%s", directory, "kong.m");
		}
		else {
			sprintf(filename, "%s/%s", directory, "kong.c");
		}

		FILE *output = fopen(filename, "wb");

		fprintf(output, "#include \"kong.h\"\n\n");

		if (api == API_METAL) {
			// Code is added directly to the Xcode project instead
		}
		else if (api == API_WEBGPU) {
			fprintf(output, "#include \"wgsl.h\"\n");
		}
		else {
			for (type_id i = 0; get_type(i) != NULL; ++i) {
				type *t = get_type(i);
				if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
					for (size_t j = 0; j < t->members.size; ++j) {
						debug_context context = {0};
						if (t->members.m[j].name == add_name("vertex")) {
							check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "vertex expects an identifier");
							fprintf(output, "#include \"kong_%s.h\"\n", get_name(t->members.m[j].value.identifier));
							if (api == API_OPENGL) {
								fprintf(output, "#include \"kong_%s_flip.h\"\n", get_name(t->members.m[j].value.identifier));
							}
						}
						else if (t->members.m[j].name == add_name("fragment")) {
							check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "fragment expects an identifier");
							fprintf(output, "#include \"kong_%s.h\"\n", get_name(t->members.m[j].value.identifier));
						}
					}
				}
			}

			for (function_id i = 0; get_function(i) != NULL; ++i) {
				function *f = get_function(i);
				if (has_attribute(&f->attributes, add_name("compute"))) {
					fprintf(output, "#include \"kong_%s.h\"\n", get_name(f->name));
				}
			}
		}

		fprintf(output, "\n#include <kore3/%s/buffer_functions.h>\n", api_long);
		fprintf(output, "#include <kore3/%s/commandlist_functions.h>\n", api_long);
		fprintf(output, "#include <kore3/%s/device_functions.h>\n", api_long);
		fprintf(output, "#include <kore3/%s/descriptorset_functions.h>\n", api_long);
		fprintf(output, "#include <kore3/%s/pipeline_functions.h>\n", api_long);
		fprintf(output, "#include <kore3/%s/texture_functions.h>\n", api_long);
		fprintf(output, "#include <kore3/util/align.h>\n\n");
		fprintf(output, "#include <assert.h>\n");
		fprintf(output, "#include <stdlib.h>\n\n");

		if (api == API_METAL) {
			fprintf(output, "#import <MetalKit/MTKView.h>\n\n");
		}

		for (size_t i = 0; i < vertex_inputs_size; ++i) {
			type *t = get_type(vertex_inputs[i]);

			fprintf(output, "uint32_t kong_%s_buffer_usage_flags(void) {\n", get_name(t->name));
			fprintf(output, "\treturn KORE_%s_BUFFER_USAGE_VERTEX;\n", api_caps);
			fprintf(output, "}\n\n");

			fprintf(output, "void kong_create_buffer_%s(kore_gpu_device * device, size_t count, %s_buffer *buffer) {\n", get_name(t->name), get_name(t->name));
			fprintf(output, "\tkore_gpu_buffer_parameters parameters;\n");
			fprintf(output, "\tparameters.size = count * sizeof(%s);\n", get_name(t->name));
			fprintf(output, "\tparameters.usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | kong_%s_buffer_usage_flags();\n", get_name(t->name));
			fprintf(output, "\tkore_gpu_device_create_buffer(device, &parameters, &buffer->buffer);\n");
			fprintf(output, "\tbuffer->count = count;\n");
			fprintf(output, "}\n\n");

			fprintf(output, "void kong_destroy_buffer_%s(%s_buffer *buffer) {\n", get_name(t->name), get_name(t->name));
			fprintf(output, "\tkore_%s_buffer_destroy(&buffer->buffer);\n", api_short);
			fprintf(output, "}\n\n");

			fprintf(output, "%s *kong_%s_buffer_lock(%s_buffer *buffer) {\n", get_name(t->name), get_name(t->name), get_name(t->name));
			fprintf(output, "\treturn (%s *)kore_%s_buffer_lock_all(&buffer->buffer);\n", get_name(t->name), api_short);
			fprintf(output, "}\n\n");

			fprintf(output, "%s *kong_%s_buffer_try_to_lock(%s_buffer *buffer) {\n", get_name(t->name), get_name(t->name), get_name(t->name));
			fprintf(output, "\treturn (%s *)kore_%s_buffer_try_to_lock_all(&buffer->buffer);\n", get_name(t->name), api_short);
			fprintf(output, "}\n\n");

			fprintf(output, "void kong_%s_buffer_unlock(%s_buffer *buffer) {\n", get_name(t->name), get_name(t->name));
			fprintf(output, "\tkore_%s_buffer_unlock(&buffer->buffer);\n", api_short);
			fprintf(output, "}\n\n");

			fprintf(output, "void kong_set_vertex_buffer_%s(kore_gpu_command_list *list, %s_buffer *buffer) {\n", get_name(t->name), get_name(t->name));
			fprintf(output, "\tkore_%s_command_list_set_vertex_buffer(list, %zu, &buffer->buffer.%s, 0, buffer->count * sizeof(%s), sizeof(%s));\n", api_short,
			        vertex_input_slots[i], api_short, get_name(t->name), get_name(t->name));
			fprintf(output, "}\n\n");
		}

		if (api != API_WEBGPU) {
			for (size_t set_index = 0; set_index < sets_count; ++set_index) {
				descriptor_set *set = sets[set_index];
				if (api == API_METAL) {
					fprintf(output, "static uint32_t %s_vertex_table_index = UINT32_MAX;\n\n", get_name(set->name));
					fprintf(output, "static uint32_t %s_fragment_table_index = UINT32_MAX;\n\n", get_name(set->name));
					fprintf(output, "static uint32_t %s_compute_table_index = UINT32_MAX;\n\n", get_name(set->name));
				}
				else if (api == API_VULKAN) {
					if (set->name != add_name("root_constants")) {
						fprintf(output, "static uint32_t %s_table_index = UINT32_MAX;\n\n", get_name(set->name));
					}
				}
				else {
					fprintf(output, "static uint32_t %s_table_index = UINT32_MAX;\n\n", get_name(set->name));
				}
			}
		}

		for (type_id i = 0; get_type(i) != NULL; ++i) {
			type *t = get_type(i);
			if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
				fprintf(output, "static kore_%s_render_pipeline %s;\n\n", api_short, get_name(t->name));

				name_id vertex_shader_name   = NO_NAME;
				name_id fragment_shader_name = NO_NAME;

				function *vertex_function   = NULL;
				function *fragment_function = NULL;

				global_array globals = {0};

				for (global_id i = 0; get_global(i) != NULL; ++i) {
					global *g = get_global(i);
					if (g->type == sampler_type_id) {
					}
					else if (is_texture(g->type)) {
					}
					else if (g->type == float_id) {
					}
					else if (!get_type(g->type)->built_in) {
						fprintf(output, "static uint32_t _%" PRIu64 "_uniform_block_index;\n", g->var_index);
					}
				}

				for (size_t j = 0; j < t->members.size; ++j) {
					if (t->members.m[j].name == add_name("vertex")) {
						vertex_shader_name = t->members.m[j].value.identifier;
					}
					if (t->members.m[j].name == add_name("fragment")) {
						fragment_shader_name = t->members.m[j].value.identifier;
					}
				}

				assert(vertex_shader_name != NO_NAME);
				assert(fragment_shader_name != NO_NAME);

				for (function_id i = 0; get_function(i) != NULL; ++i) {
					function *f = get_function(i);

					if (f->name == vertex_shader_name) {
						vertex_function = f;
					}

					if (f->name == fragment_shader_name) {
						fragment_function = f;
					}

					if (vertex_function != NULL && fragment_function != NULL) {
						break;
					}
				}

				assert(vertex_function != NULL);
				assert(fragment_function != NULL);

				if (api == API_OPENGL) {
					find_referenced_globals(vertex_function, &globals);
					find_referenced_globals(fragment_function, &globals);

					for (uint32_t i = 0; i < globals.size; ++i) {
						global *g = get_global(globals.globals[i]);
						if (g->type == sampler_type_id) {
						}
						else if (is_texture(g->type)) {
						}
						else if (g->type == float_id) {
						}
						else {
							fprintf(output, "static uint32_t %s_%" PRIu64 "_uniform_block_index;\n", get_name(t->name), g->var_index);
						}
					}
				}

				fprintf(output, "void kong_set_render_pipeline_%s(kore_gpu_command_list *list) {\n", get_name(t->name));
				fprintf(output, "\tkore_%s_command_list_set_render_pipeline(list, &%s);\n", api_short, get_name(t->name));

				if (api == API_OPENGL) {
					for (uint32_t i = 0; i < globals.size; ++i) {
						global *g = get_global(globals.globals[i]);
						if (g->type == sampler_type_id) {
						}
						else if (is_texture(g->type)) {
						}
						else if (g->type == float_id) {
						}
						else {
							fprintf(output, "\t_%" PRIu64 "_uniform_block_index = %s_%" PRIu64 "_uniform_block_index;\n\n", g->var_index, get_name(t->name),
							        g->var_index);
						}
					}
				}

				if (api != API_WEBGPU) {
					descriptor_set_group *group = find_descriptor_set_group_for_pipe_type(t);

					if (api == API_VULKAN) {
						size_t index = 0;
						for (size_t group_index = 0; group_index < group->size; ++group_index) {
							if (group->values[group_index]->name != add_name("root_constants")) {
								fprintf(output, "\t%s_table_index = %zu;\n", get_name(group->values[group_index]->name), index);
								index += 1;
							}
						}
					}
					else {
						for (size_t group_index = 0; group_index < group->size; ++group_index) {
							if (api == API_METAL) {
								fprintf(output, "\t%s_vertex_table_index = %zu;\n", get_name(group->values[group_index]->name),
								        group_index + vertex_function->parameters_size);
								fprintf(output, "\t%s_fragment_table_index = %zu;\n", get_name(group->values[group_index]->name), group_index + 1);
							}
							else {
								fprintf(output, "\t%s_table_index = %zu;\n", get_name(group->values[group_index]->name), group_index);
							}
						}
					}
				}

				fprintf(output, "}\n\n");
			}
		}

		for (type_id i = 0; get_type(i) != NULL; ++i) {
			type *t = get_type(i);
			if (!t->built_in && has_attribute(&t->attributes, add_name("raypipe"))) {
				fprintf(output, "static kore_%s_ray_pipeline %s;\n\n", api_short, get_name(t->name));
				fprintf(output, "void kong_set_ray_pipeline_%s(kore_gpu_command_list *list) {\n", get_name(t->name));
				fprintf(output, "\tkore_d3d12_command_list_set_ray_pipeline(list, &%s);\n", get_name(t->name));

				descriptor_set_group *group = find_descriptor_set_group_for_pipe_type(t);
				for (size_t group_index = 0; group_index < group->size; ++group_index) {
					if (api == API_METAL) {
						fprintf(output, "\t%s_compute_table_index = %zu;\n", get_name(group->values[group_index]->name), group_index);
					}
					else {
						fprintf(output, "\t%s_table_index = %zu;\n", get_name(group->values[group_index]->name), group_index);
					}
				}

				fprintf(output, "}\n\n");
			}
		}

		global *root_constants_global         = NULL;
		char    root_constants_type_name[256] = {0};

		for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
			global *g = get_global(i);

			type_id base_type = get_type(g->type)->array_size > 0 ? get_type(g->type)->base : g->type;

			if (is_texture(g->type)) {
				fprintf(output, "uint32_t %s_texture_usage_flags(void) {\n", get_name(g->name));
				fprintf(output, "\tuint32_t usage = 0u;\n");
				if (api == API_DIRECT3D12) {
					if (global_has_usage(i, GLOBAL_USAGE_TEXTURE_SAMPLE)) {
						fprintf(output, "\tusage |= KORE_D3D12_TEXTURE_USAGE_SRV;\n");
					}
					if (global_has_usage(i, GLOBAL_USAGE_TEXTURE_READ) || global_has_usage(i, GLOBAL_USAGE_TEXTURE_WRITE)) {
						fprintf(output, "\tusage |= KORE_D3D12_TEXTURE_USAGE_UAV;\n");
					}
				}
				else if (api == API_VULKAN || api == API_WEBGPU || api == API_OPENGL) {
					if (global_has_usage(i, GLOBAL_USAGE_TEXTURE_SAMPLE)) {
						fprintf(output, "\tusage |= KORE_%s_TEXTURE_USAGE_SAMPLED;\n", api_caps);
					}
					if (global_has_usage(i, GLOBAL_USAGE_TEXTURE_READ) || global_has_usage(i, GLOBAL_USAGE_TEXTURE_WRITE)) {
						fprintf(output, "\tusage |= KORE_%s_TEXTURE_USAGE_STORAGE;\n", api_caps);
					}
				}
				else {
					if (global_has_usage(i, GLOBAL_USAGE_TEXTURE_SAMPLE)) {
						fprintf(output, "\tusage |= KORE_%s_TEXTURE_USAGE_SAMPLE;\n", api_caps);
					}
					if (global_has_usage(i, GLOBAL_USAGE_TEXTURE_READ) || global_has_usage(i, GLOBAL_USAGE_TEXTURE_WRITE)) {
						fprintf(output, "\tusage |= KORE_%s_TEXTURE_USAGE_READ_WRITE;\n", api_caps);
					}
				}
				fprintf(output, "\treturn usage;\n");
				fprintf(output, "}\n\n");
			}
			else if (!get_type(base_type)->built_in) {
				type *t = get_type(g->type);

				char type_name[256];
				if (t->name != NO_NAME) {
					strcpy(type_name, get_name(t->name));
				}
				else {
					strcpy(type_name, get_name(g->name));
					strcat(type_name, "_type");
				}

				bool is_root_constant = false;

				for (size_t set_index = 0; set_index < g->sets_count; ++set_index) {
					if (g->sets[set_index]->name == add_name("root_constants")) {
						is_root_constant = true;
						break;
					}
				}

				if (is_root_constant) {
					root_constants_global = g;
					strcpy(root_constants_type_name, type_name);
				}
				else {
					if (api == API_WEBGPU) {
						fprintf(output, "uint32_t %s_buffer_usage_flags(void) {\n", type_name);
						fprintf(output, "\treturn KORE_WEBGPU_BUFFER_USAGE_UNIFORM;\n");
						fprintf(output, "}\n\n");
					}
					else if (api == API_DIRECT3D12) {
						fprintf(output, "uint32_t %s_buffer_usage_flags(void) {\n", type_name);
						fprintf(output, "\treturn KORE_D3D12_BUFFER_USAGE_CBV;\n");
						fprintf(output, "}\n\n");
					}
					else if (api == API_OPENGL) {
						fprintf(output, "uint32_t %s_buffer_usage_flags(void) {\n", type_name);
						fprintf(output, "\treturn KORE_OPENGL_BUFFER_USAGE_UNIFORM;\n");
						fprintf(output, "}\n\n");
					}
					else {
						fprintf(output, "uint32_t %s_buffer_usage_flags(void) {\n", type_name);
						fprintf(output, "\treturn 0u;\n");
						fprintf(output, "}\n\n");
					}

					fprintf(output, "void %s_buffer_create(kore_gpu_device *device, kore_gpu_buffer *buffer, uint32_t count) {\n", type_name);
					fprintf(output, "\tkore_gpu_buffer_parameters parameters;\n");
					fprintf(output, "\tparameters.size = align_pow2(%i, 256) * count;\n", struct_size(g->type));
					fprintf(output, "\tparameters.usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | %s_buffer_usage_flags();\n", type_name);
					fprintf(output, "\tkore_gpu_device_create_buffer(device, &parameters, buffer);\n");
					fprintf(output, "}\n\n");

					fprintf(output, "void %s_buffer_destroy(kore_gpu_buffer *buffer) {\n", type_name);
					fprintf(output, "\tkore_gpu_buffer_destroy(buffer);\n");
					fprintf(output, "}\n\n");

					fprintf(output, "%s *%s_buffer_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count) {\n", type_name, type_name);
					fprintf(output,
					        "\treturn (%s *)kore_gpu_buffer_lock(buffer, index * align_pow2((int)sizeof(%s), 256), count * align_pow2((int)sizeof(%s), "
					        "256));\n",
					        type_name, type_name, type_name);
					fprintf(output, "}\n\n");

					fprintf(output, "%s *%s_buffer_try_to_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count) {\n", type_name, type_name);
					fprintf(output,
					        "\treturn (%s *)kore_gpu_buffer_try_to_lock(buffer, index * align_pow2((int)sizeof(%s), 256), count * "
					        "align_pow2((int)sizeof(%s), "
					        "256));\n",
					        type_name, type_name, type_name);
					fprintf(output, "}\n\n");

					fprintf(output, "void %s_buffer_unlock(kore_gpu_buffer *buffer) {\n", type_name);

					bool has_matrices = false;
					for (size_t j = 0; j < t->members.size; ++j) {
						if (t->members.m[j].type.type == float4x4_id || t->members.m[j].type.type == float3x3_id) {
							has_matrices = true;
							break;
						}
					}

					if (has_matrices) {
						fprintf(output, "\t%s *data = (%s *)buffer->%s.locked_data;\n", type_name, type_name, api_short);
						// adjust matrices
						for (size_t j = 0; j < t->members.size; ++j) {
							if (t->members.m[j].type.type == float4x4_id && (api != API_METAL && api != API_VULKAN && api != API_WEBGPU && api != API_OPENGL)) {
								fprintf(output, "\tkore_matrix4x4_transpose(&data->%s);\n", get_name(t->members.m[j].name));
							}
							else if (t->members.m[j].type.type == float3x3_id &&
							         (api == API_METAL || api == API_VULKAN || api == API_WEBGPU || api == API_OPENGL)) {
								fprintf(output, "\t{\n");
								fprintf(output, "\t\tkore_matrix3x3 m = data->%s;\n", get_name(t->members.m[j].name));
								fprintf(output, "\t\tfloat *m_data = (float *)&data->%s;\n", get_name(t->members.m[j].name));
								fprintf(output, "\t\tm_data[0] = m.m[0];\n");
								fprintf(output, "\t\tm_data[1] = m.m[3];\n");
								fprintf(output, "\t\tm_data[2] = m.m[6];\n");
								fprintf(output, "\t\tm_data[3] = 0.0f;\n");
								fprintf(output, "\t\tm_data[4] = m.m[1];\n");
								fprintf(output, "\t\tm_data[5] = m.m[4];\n");
								fprintf(output, "\t\tm_data[6] = m.m[7];\n");
								fprintf(output, "\t\tm_data[7] = 0.0f;\n");
								fprintf(output, "\t\tm_data[8] = m.m[2];\n");
								fprintf(output, "\t\tm_data[9] = m.m[5];\n");
								fprintf(output, "\t\tm_data[10] = m.m[8];\n");
								fprintf(output, "\t\tm_data[11] = 0.0f;\n");
								fprintf(output, "\t}\n");
							}
							else if (t->members.m[j].type.type == float3x3_id) {
								fprintf(output, "\t{\n");
								fprintf(output, "\t\tkore_matrix3x3 m = data->%s;\n", get_name(t->members.m[j].name));
								fprintf(output, "\t\tfloat *m_data = (float *)&data->%s;\n", get_name(t->members.m[j].name));
								fprintf(output, "\t\tm_data[0] = m.m[0];\n");
								fprintf(output, "\t\tm_data[1] = m.m[1];\n");
								fprintf(output, "\t\tm_data[2] = m.m[2];\n");
								fprintf(output, "\t\tm_data[3] = 0.0f;\n");
								fprintf(output, "\t\tm_data[4] = m.m[3];\n");
								fprintf(output, "\t\tm_data[5] = m.m[4];\n");
								fprintf(output, "\t\tm_data[6] = m.m[5];\n");
								fprintf(output, "\t\tm_data[7] = 0.0f;\n");
								fprintf(output, "\t\tm_data[8] = m.m[6];\n");
								fprintf(output, "\t\tm_data[9] = m.m[7];\n");
								fprintf(output, "\t\tm_data[10] = m.m[8];\n");
								fprintf(output, "\t\tm_data[11] = 0.0f;\n");
								fprintf(output, "\t}\n");
							}
						}
					}

					fprintf(output, "\tkore_gpu_buffer_unlock(buffer);\n");
					fprintf(output, "}\n\n");
				}
			}
		}

		for (size_t set_index = 0; set_index < sets_count; ++set_index) {
			descriptor_set *set = sets[set_index];

			if (set->name == add_name("root_constants")) {
				assert(root_constants_global != NULL);

				fprintf(output, "void kong_set_root_constants_%s(kore_gpu_command_list *list, %s *constants) {\n", get_name(root_constants_global->name),
				        root_constants_type_name);
				if (api == API_METAL) {
					fprintf(output,
					        "\tkore_%s_command_list_set_root_constants(list, %s_vertex_table_index, %s_fragment_table_index, %s_compute_table_index, "
					        "constants, %i);\n",
					        api_short, get_name(set->name), get_name(set->name), get_name(set->name), struct_size(root_constants_global->type));
				}
				else if (api == API_VULKAN) {
					fprintf(output, "\tkore_%s_command_list_set_root_constants(list, constants, %i);\n", api_short, struct_size(root_constants_global->type));
				}
				else {
					fprintf(output, "\tkore_%s_command_list_set_root_constants(list, %s_table_index, constants, %i);\n", api_short, get_name(set->name),
					        struct_size(root_constants_global->type));
				}
				fprintf(output, "}\n\n");

				continue;
			}

			if (api == API_VULKAN) {
				fprintf(output, "extern VkDescriptorSetLayout %s_set_layout;\n\n", get_name(set->name));
			}
			else if (api == API_WEBGPU) {
				fprintf(output, "extern WGPUBindGroupLayout %s_set_layout;\n\n", get_name(set->name));
			}

			fprintf(output, "void kong_create_%s_set(kore_gpu_device *device, const %s_parameters *parameters, %s_set *set) {\n", get_name(set->name),
			        get_name(set->name), get_name(set->name));

			if (api == API_DIRECT3D12) {
				size_t other_count    = 0;
				size_t sampler_count  = 0;
				size_t dynamic_count  = 0;
				size_t bindless_count = 0;

				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							dynamic_count += 1;
						}
						else {
							other_count += 1;
						}
					}
					else if (is_texture(g->type)) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							bindless_count += 1;
						}
						else {
							other_count += 1;
						}
					}
					else if (is_sampler(g->type)) {
						sampler_count += 1;
					}
					else {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							dynamic_count += 1;
						}
						else {
							other_count += 1;
						}
					}
				}

				fprintf(output, "\tkore_%s_device_create_descriptor_set(device, %zu, %zu, %zu, %zu, &set->set);\n", api_short, other_count, dynamic_count,
				        bindless_count, sampler_count);

				size_t other_index   = 0;
				size_t sampler_index = 0;

				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g            = get_global(set->globals.globals[global_index]);
					bool    writable     = set->globals.writable[global_index];
					type_id base_type_id = get_type(g->type)->base != NO_TYPE ? get_type(g->type)->base : g->type;

					if (!get_type(g->type)->built_in) {
						if (!has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\tkore_%s_descriptor_set_set_buffer_view_cbv(device, &set->set, parameters->%s, %zu);\n", api_short,
							        get_name(g->name), other_index);
							other_index += 1;
						}
						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
					}
					else if (base_type_id == bvh_type_id) {
						fprintf(output, "\tkore_%s_descriptor_set_set_bvh_view_srv(device, &set->set, parameters->%s, %zu);\n", api_short, get_name(g->name),
						        other_index);
						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
					else if (base_type_id == tex2d_type_id) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							fprintf(output, "\tset->%s = (kore_gpu_texture_view *)malloc(sizeof(kore_gpu_texture_view) * parameters->textures_count);\n",
							        get_name(g->name));
							fprintf(output, "\tassert(set->%s != NULL);\n", get_name(g->name));
							fprintf(output, "\tfor (size_t index = 0; index < parameters->textures_count; ++index) {\n");
							fprintf(output,
							        "\t\tkore_%s_descriptor_set_set_texture_view_srv(device, set->set.bindless_descriptor_allocation.offset + (uint32_t)index, "
							        "&parameters->%s[index]);\n",
							        api_short, get_name(g->name));
							fprintf(output, "\t\tset->%s[index] = parameters->%s[index];\n", get_name(g->name), get_name(g->name));
							fprintf(output, "\t}\n");

							fprintf(output, "\tset->%s_count = parameters->%s_count;\n", get_name(g->name), get_name(g->name));
						}
						else {
							if (writable) {
								fprintf(output, "\tkore_%s_descriptor_set_set_texture_view_uav(device, &set->set, &parameters->%s, %zu);\n", api_short,
								        get_name(g->name), other_index);
							}
							else {
								fprintf(output,
								        "\tkore_%s_descriptor_set_set_texture_view_srv(device, set->set.descriptor_allocation.offset + %zu, "
								        "&parameters->%s);\n",
								        api_short, other_index, get_name(g->name));
							}

							fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));

							other_index += 1;
						}
					}
					else if (base_type_id == tex2darray_type_id) {
						if (writable) {
							debug_context context = {0};
							error(context, "Texture arrays can not be writable");
						}

						fprintf(output, "\tkore_%s_descriptor_set_set_texture_array_view_srv(device, &set->set, &parameters->%s, %zu);\n", api_short,
						        get_name(g->name), other_index);

						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
					else if (base_type_id == texcube_type_id) {
						if (writable) {
							debug_context context = {0};
							error(context, "Cube maps can not be writable");
						}
						fprintf(output, "\tkore_%s_descriptor_set_set_texture_cube_view_srv(device, &set->set, &parameters->%s, %zu);\n", api_short,
						        get_name(g->name), other_index);

						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
					else if (is_sampler(g->type)) {
						fprintf(output, "\tkore_%s_descriptor_set_set_sampler(device, &set->set, parameters->%s, %zu);\n", api_short, get_name(g->name),
						        sampler_index);
						sampler_index += 1;
					}
					else {
						if (!has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\tkore_%s_descriptor_set_set_buffer_view_uav(device, &set->set, parameters->%s, %zu);\n", api_short,
							        get_name(g->name), other_index);
							other_index += 1;
						}
						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
					}
				}
				fprintf(output, "}\n\n");
			}
			else if (api == API_METAL) {
				fprintf(output, "\tid<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;\n\n");

				size_t index = 0;
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);

					if (!get_type(g->type)->built_in && !has_attribute(&g->attributes, add_name("indexed"))) {
						fprintf(output, "\tMTLArgumentDescriptor* descriptor%zu = [MTLArgumentDescriptor argumentDescriptor];\n", index);
						fprintf(output, "\tdescriptor%zu.index = %zu;\n", index, index);
						fprintf(output, "\tdescriptor%zu.dataType = MTLDataTypePointer;\n\n", index);
						index += 1;
					}
					else if (is_texture(g->type)) {
						fprintf(output, "\tMTLArgumentDescriptor* descriptor%zu = [MTLArgumentDescriptor argumentDescriptor];\n", index);
						fprintf(output, "\tdescriptor%zu.index = %zu;\n", index, index);
						fprintf(output, "\tdescriptor%zu.dataType = MTLDataTypeTexture;\n\n", index);
						index += 1;
					}
					else if (is_sampler(g->type)) {
						fprintf(output, "\tMTLArgumentDescriptor* descriptor%zu = [MTLArgumentDescriptor argumentDescriptor];\n", index);
						fprintf(output, "\tdescriptor%zu.index = %zu;\n", index, index);
						fprintf(output, "\tdescriptor%zu.dataType = MTLDataTypeSampler;\n\n", index);
						index += 1;
					}
				}

				fprintf(output, "\tid<MTLArgumentEncoder> argument_encoder = [metal_device newArgumentEncoderWithArguments: @[");

				bool first = true;
				index      = 0;

				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);
					if (!has_attribute(&g->attributes, add_name("indexed"))) {
						if (first) {
							fprintf(output, "descriptor%zu", index);
							first = false;
						}
						else {
							fprintf(output, ", descriptor%zu", index);
						}
						index += 1;
					}
				}
				fprintf(output, "]];\n\n");

				fprintf(output, "\tkore_metal_device_create_descriptor_set_buffer(device, [argument_encoder encodedLength], &set->set.argument_buffer);\n\n");

				fprintf(output, "\tid<MTLBuffer> metal_argument_buffer = (__bridge id<MTLBuffer>)set->set.argument_buffer.metal.buffer;\n\n");
				fprintf(output, "\t[argument_encoder setArgumentBuffer:metal_argument_buffer offset:0];\n\n");

				index = 0;
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);

					fprintf(output, "\t{\n");

					if (!get_type(g->type)->built_in && !has_attribute(&g->attributes, add_name("indexed"))) {
						fprintf(output, "\t\tid<MTLBuffer> buffer = (__bridge id<MTLBuffer>)parameters->%s->metal.buffer;\n", get_name(g->name));
						fprintf(output, "\t\t[argument_encoder setBuffer: buffer offset: 0 atIndex: %zu];\n", index);
						index += 1;
					}
					else if (is_texture(g->type)) {
						fprintf(output, "\t\tid<MTLTexture> texture = (__bridge id<MTLTexture>)parameters->%s.texture->metal.texture;\n", get_name(g->name));

						if (g->type == tex2d_type_id) {
							fprintf(output,
							        "\t\tid<MTLTexture> view = [texture newTextureViewWithPixelFormat:texture.pixelFormat textureType:MTLTextureType2D "
							        "levels:NSMakeRange(parameters->%s.base_mip_level, parameters->%s.mip_level_count) "
							        "slices:NSMakeRange(parameters->%s.base_array_layer, parameters->%s.array_layer_count)];\n",
							        get_name(g->name), get_name(g->name), get_name(g->name), get_name(g->name));
						}
						else if (g->type == tex2darray_type_id) {
							fprintf(output,
							        "\t\tid<MTLTexture> view = [texture newTextureViewWithPixelFormat:texture.pixelFormat textureType:MTLTextureType2DArray "
							        "levels:NSMakeRange(parameters->%s.base_mip_level, parameters->%s.mip_level_count) "
							        "slices:NSMakeRange(parameters->%s.base_array_layer, parameters->%s.array_layer_count)];\n",
							        get_name(g->name), get_name(g->name), get_name(g->name), get_name(g->name));
						}
						else if (g->type == texcube_type_id) {
							fprintf(output,
							        "\t\tid<MTLTexture> view = [texture newTextureViewWithPixelFormat:texture.pixelFormat textureType:MTLTextureTypeCube "
							        "levels:NSMakeRange(parameters->%s.base_mip_level, parameters->%s.mip_level_count) "
							        "slices:NSMakeRange(parameters->%s.base_array_layer, parameters->%s.array_layer_count)];\n",
							        get_name(g->name), get_name(g->name), get_name(g->name), get_name(g->name));
						}

						fprintf(output, "\t\t[argument_encoder setTexture: view atIndex: %zu];\n", index);
						fprintf(output, "\t\tset->%s_view = (__bridge_retained void*)view;\n", get_name(g->name));
						index += 1;
					}
					else if (is_sampler(g->type)) {
						fprintf(output, "\t\tid<MTLSamplerState> sampler = (__bridge id<MTLSamplerState>)parameters->%s->metal.sampler;\n", get_name(g->name));
						fprintf(output, "\t\t[argument_encoder setSamplerState: sampler atIndex: %zu];\n", index);
						index += 1;
					}

					fprintf(output, "\t\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));

					fprintf(output, "\t}\n\n");
				}

				fprintf(output, "}\n\n");
			}
			else if (api == API_VULKAN) {
				size_t other_count    = 0;
				size_t dynamic_count  = 0;
				size_t bindless_count = 0;

				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							dynamic_count += 1;
						}
						else {
							other_count += 1;
						}
					}
					else if (is_texture(g->type)) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							bindless_count += 1;
						}
						else {
							other_count += 1;
						}
					}
					else if (is_sampler(g->type)) {
						other_count += 1;
					}
					else {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							dynamic_count += 1;
						}
						else {
							other_count += 1;
						}
					}
				}

				fprintf(output, "\tkore_vulkan_device_create_descriptor_set(device, &%s_set_layout, &set->set);\n", get_name(set->name));

				size_t other_index = 0;

				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g            = get_global(set->globals.globals[global_index]);
					bool    readable     = set->globals.readable[global_index];
					bool    writable     = set->globals.writable[global_index];
					type_id base_type_id = get_type(g->type)->base != NO_TYPE ? get_type(g->type)->base : g->type;

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\tkore_%s_descriptor_set_set_dynamic_uniform_buffer_descriptor(device, &set->set, parameters->%s, %u, %zu);\n",
							        api_short, get_name(g->name), struct_size(g->type), other_index);
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_set_uniform_buffer_descriptor(device, &set->set, parameters->%s, %zu);\n", api_short,
							        get_name(g->name), other_index);
						}
						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
					else if (base_type_id == bvh_type_id) {
						fprintf(output, "\tkore_%s_descriptor_set_set_bvh_view_srv(device, &set->set, parameters->%s, %zu);\n", api_short, get_name(g->name),
						        other_index);
						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
					else if (base_type_id == tex2d_type_id) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							fprintf(output, "\tset->%s = (kore_gpu_texture_view *)malloc(sizeof(kore_gpu_texture_view) * parameters->textures_count);\n",
							        get_name(g->name));
							fprintf(output, "\tassert(set->%s != NULL);\n", get_name(g->name));
							fprintf(output, "\tfor (size_t index = 0; index < parameters->textures_count; ++index) {\n");
							fprintf(output,
							        "\t\tkore_%s_descriptor_set_set_texture_view_srv(device, set->set.bindless_descriptor_allocation.offset + (uint32_t)index, "
							        "&parameters->%s[index]);\n",
							        api_short, get_name(g->name));
							fprintf(output, "\t\tset->%s[index] = parameters->%s[index];\n", get_name(g->name), get_name(g->name));
							fprintf(output, "\t}\n");

							fprintf(output, "\tset->%s_count = parameters->%s_count;\n", get_name(g->name), get_name(g->name));
						}
						else {
							if (readable | writable) {
								fprintf(output, "\tkore_vulkan_descriptor_set_set_storage_image_descriptor(device, &set->set, &parameters->%s, %zu);\n",
								        get_name(g->name), other_index);
							}
							else {
								fprintf(output, "\tkore_vulkan_descriptor_set_set_sampled_image_descriptor(device, &set->set, &parameters->%s, %zu);\n",
								        get_name(g->name), other_index);
							}

							fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));

							other_index += 1;
						}
					}
					else if (base_type_id == tex2darray_type_id) {
						if (writable) {
							debug_context context = {0};
							error(context, "Texture arrays can not be writable");
						}

						fprintf(output, "\tkore_vulkan_descriptor_set_set_sampled_image_array_descriptor(device, &set->set, &parameters->%s, %zu);\n",
						        get_name(g->name), other_index);

						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
					else if (base_type_id == texcube_type_id) {
						if (writable) {
							debug_context context = {0};
							error(context, "Cube maps can not be writable");
						}
						fprintf(output, "\tkore_vulkan_descriptor_set_set_sampled_cube_image_descriptor(device, &set->set, &parameters->%s, %zu);\n",
						        get_name(g->name), other_index);

						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
					else if (is_sampler(g->type)) {
						fprintf(output, "\tkore_%s_descriptor_set_set_sampler(device, &set->set, parameters->%s, %zu);\n", api_short, get_name(g->name),
						        other_index);
						other_index += 1;
					}
					else {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\tkore_%s_descriptor_set_set_dynamic_storage_buffer_descriptor(device, &set->set, parameters->%s, %zu);\n",
							        api_short, get_name(g->name), other_index);
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_set_storage_buffer_descriptor(device, &set->set, parameters->%s, %zu);\n", api_short,
							        get_name(g->name), other_index);
						}
						fprintf(output, "\tset->%s = parameters->%s;\n", get_name(g->name), get_name(g->name));
						other_index += 1;
					}
				}
				fprintf(output, "}\n\n");
			}
			else if (api == API_WEBGPU) {
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);

					if (is_texture(g->type)) {
						fprintf(output, "\tWGPUTextureViewDescriptor texture_view_descriptor%zu = {\n", global_index);
						fprintf(output, "\t\t.format = kore_webgpu_convert_texture_format(parameters->%s.texture->webgpu.format),\n", get_name(g->name));
						if (g->type == tex2darray_type_id) {
							fprintf(output, "\t\t.dimension = WGPUTextureViewDimension_2DArray,\n");
						}
						else if (g->type == texcube_type_id) {
							fprintf(output, "\t\t.dimension = WGPUTextureViewDimension_Cube,\n");
						}
						else {
							fprintf(output, "\t\t.dimension = WGPUTextureViewDimension_2D,\n");
						}
						fprintf(output, "\t\t.baseArrayLayer  = parameters->%s.base_array_layer,\n", get_name(g->name));
						fprintf(output, "\t\t.arrayLayerCount = parameters->%s.array_layer_count,\n", get_name(g->name));
						fprintf(output, "\t\t.baseMipLevel    = parameters->%s.base_mip_level,\n", get_name(g->name));
						fprintf(output, "\t\t.mipLevelCount   = parameters->%s.mip_level_count,\n", get_name(g->name));
						fprintf(output, "\t};\n\n");
					}
				}

				size_t index = 0;

				fprintf(output, "\tconst WGPUBindGroupEntry entries[] = {\n");

				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);

					if (!get_type(g->type)->built_in) {
						fprintf(output, "\t\t{\n");
						fprintf(output, "\t\t\t.binding = %zu,\n", index);
						fprintf(output, "\t\t\t.buffer  = parameters->%s->webgpu.buffer,\n", get_name(g->name));
						fprintf(output, "\t\t\t.offset  = 0,\n");
						fprintf(output, "\t\t\t.size    = align_pow2(%u, 256),\n", struct_size(g->type));
						fprintf(output, "\t\t},\n");

						index += 1;
					}
					else if (is_texture(g->type)) {
						fprintf(output, "\t\t{\n");
						fprintf(output, "\t\t\t.binding     = %zu,\n", index);
						fprintf(output, "\t\t\t.textureView = wgpuTextureCreateView(parameters->%s.texture->webgpu.texture, &texture_view_descriptor%zu),\n",
						        get_name(g->name), global_index);
						fprintf(output, "\t\t},\n");

						index += 1;
					}
					else if (is_sampler(g->type)) {
						fprintf(output, "\t\t{\n");
						fprintf(output, "\t\t\t.binding = %zu,\n", index);
						fprintf(output, "\t\t\t.sampler = parameters->%s->webgpu.sampler,\n", get_name(g->name));
						fprintf(output, "\t\t},\n");

						index += 1;
					}
				}

				fprintf(output, "\t};\n\n");

				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);
					fprintf(output, "\tset->%s = parameters->%s;\n\n", get_name(g->name), get_name(g->name));
				}

				fprintf(output, "\tWGPUBindGroupDescriptor bind_group_descriptor = {\n");
				fprintf(output, "\t\t.layout     = %s_set_layout,\n", get_name(set->name));
				fprintf(output, "\t\t.entries    = entries,\n");
				fprintf(output, "\t\t.entryCount = %zu,\n", set->globals.size);
				fprintf(output, "\t};\n");

				fprintf(output, "\tWGPUBindGroup group = wgpuDeviceCreateBindGroup(device->webgpu.device, &bind_group_descriptor);\n\n");

				fprintf(output, "\tkore_webgpu_device_create_descriptor_set(device, group, &set->set);\n");

				fprintf(output, "}\n\n");
			}
			else if (api == API_OPENGL) {
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);
					fprintf(output, "\tset->%s = parameters->%s;\n\n", get_name(g->name), get_name(g->name));
				}

				fprintf(output, "}\n\n");
			}

			fprintf(output, "void kong_update_%s_set(%s_set *set, %s_set_update *updates, uint32_t updates_count) {\n", get_name(set->name),
			        get_name(set->name), get_name(set->name));
			fprintf(output, "}\n\n");

			fprintf(output, "void kong_set_descriptor_set_%s(kore_gpu_command_list *list, %s_set *set", get_name(set->name), get_name(set->name));
			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g = get_global(set->globals.globals[global_index]);

				if (!get_type(g->type)->built_in) {
					if (has_attribute(&g->attributes, add_name("indexed"))) {
						fprintf(output, ", uint32_t %s_index", get_name(g->name));
					}
				}
			}
			fprintf(output, ") {\n");

			if (api == API_DIRECT3D12) {
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g        = get_global(set->globals.globals[global_index]);
					bool    writable = set->globals.writable[global_index];

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output,
							        "\tkore_%s_descriptor_set_prepare_cbv_buffer(list, set->%s, %s_index * align_pow2((int)%i, 256), "
							        "align_pow2((int)%i, 256));\n",
							        api_short, get_name(g->name), get_name(g->name), struct_size(g->type), struct_size(g->type));
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_prepare_cbv_buffer(list, set->%s, 0, UINT32_MAX);\n", api_short, get_name(g->name));
						}
					}
					else if (is_texture(g->type)) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							fprintf(output, "\tfor (size_t index = 0; index < set->%s_count; ++index) {\n", get_name(g->name));
							fprintf(output, "\t\tkore_%s_descriptor_set_prepare_srv_texture(list, &set->%s[index]);\n", api_short, get_name(g->name));
							fprintf(output, "\t}\n");
						}
						else {
							if (writable) {
								fprintf(output, "\tkore_%s_descriptor_set_prepare_uav_texture(list, &set->%s);\n", api_short, get_name(g->name));
							}
							else {
								fprintf(output, "\tkore_%s_descriptor_set_prepare_srv_texture(list, &set->%s);\n", api_short, get_name(g->name));
							}
						}
					}
					else if (!is_sampler(g->type) && g->type != bvh_type_id) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output,
							        "\tkore_%s_descriptor_set_prepare_cbv_buffer(list, set->%s, %s_index * align_pow2((int)%i, 256), "
							        "align_pow2((int)%i, 256));\n",
							        api_short, get_name(g->name), get_name(g->name), struct_size(g->type), struct_size(g->type));
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_prepare_uav_buffer(list, set->%s, 0, UINT32_MAX);\n", api_short, get_name(g->name));
						}
					}
				}
			}
			else if (api == API_METAL) {
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g        = get_global(set->globals.globals[global_index]);
					bool    writable = set->globals.writable[global_index];

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output,
							        "\tkore_%s_descriptor_set_prepare_buffer(list, set->%s, %s_index * align_pow2((int)%i, 256), "
							        "align_pow2((int)%i, 256));\n",
							        api_short, get_name(g->name), get_name(g->name), struct_size(g->type), struct_size(g->type));
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_prepare_buffer(list, set->%s, 0, UINT32_MAX);\n", api_short, get_name(g->name));
						}
					}
					else if (is_texture(g->type)) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							fprintf(output, "\tfor (size_t index = 0; index < set->%s_count; ++index) {\n", get_name(g->name));
							fprintf(output, "\t\tkore_%s_descriptor_set_prepare_srv_texture(list, &set->%s[index]);\n", api_short, get_name(g->name));
							fprintf(output, "\t}\n");
						}
						else {
							if (writable) {
								fprintf(output, "\tkore_%s_descriptor_set_prepare_texture(list, set->%s_view, true);\n", api_short, get_name(g->name));
							}
							else {
								fprintf(output, "\tkore_%s_descriptor_set_prepare_texture(list, set->%s_view, false);\n", api_short, get_name(g->name));
							}
						}
					}
					else if (!is_sampler(g->type) && g->type != bvh_type_id) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output,
							        "\tkore_%s_descriptor_set_prepare_cbv_buffer(list, set->%s, %s_index * align_pow2((int)%i, 256), "
							        "align_pow2((int)%i, 256));\n",
							        api_short, get_name(g->name), get_name(g->name), struct_size(g->type), struct_size(g->type));
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_prepare_uav_buffer(list, set->%s, 0, UINT32_MAX);\n", api_short, get_name(g->name));
						}
					}
				}
			}
			else if (api == API_VULKAN) {
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g        = get_global(set->globals.globals[global_index]);
					bool    writable = set->globals.writable[global_index];

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output,
							        "\tkore_%s_descriptor_set_prepare_buffer(list, set->%s, %s_index * align_pow2((int)%i, 256), "
							        "align_pow2((int)%i, 256));\n",
							        api_short, get_name(g->name), get_name(g->name), struct_size(g->type), struct_size(g->type));
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_prepare_buffer(list, set->%s, 0, UINT32_MAX);\n", api_short, get_name(g->name));
						}
					}
					else if (is_texture(g->type)) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							fprintf(output, "\tfor (size_t index = 0; index < set->%s_count; ++index) {\n", get_name(g->name));
							fprintf(output, "\t\tkore_%s_descriptor_set_prepare_srv_texture(list, &set->%s[index]);\n", api_short, get_name(g->name));
							fprintf(output, "\t}\n");
						}
						else {
							if (writable) {
								fprintf(output, "\tkore_%s_descriptor_set_prepare_texture(list, &set->%s, true);\n", api_short, get_name(g->name));
							}
							else {
								fprintf(output, "\tkore_%s_descriptor_set_prepare_texture(list, &set->%s, false);\n", api_short, get_name(g->name));
							}
						}
					}
					else if (!is_sampler(g->type) && g->type != bvh_type_id) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\tkore_vulkan_descriptor_set_prepare_buffer(list, set->%s);\n", get_name(g->name));
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_prepare_uav_buffer(list, set->%s, 0, UINT32_MAX);\n", api_short, get_name(g->name));
						}
					}
				}
			}
			else if (api == API_WEBGPU) {
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g        = get_global(set->globals.globals[global_index]);
					bool    writable = set->globals.writable[global_index];

					if (!get_type(g->type)->built_in) {
						fprintf(output, "\tkore_%s_descriptor_set_prepare_buffer(list, set->%s);\n", api_short, get_name(g->name));
					}
				}
			}
			else if (api == API_OPENGL) {
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g        = get_global(set->globals.globals[global_index]);
					bool    writable = set->globals.writable[global_index];

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output,
							        "\tkore_opengl_command_list_set_uniform_buffer(list, set->%s, _%" PRIu64
							        "_uniform_block_index, %s_index * align_pow2((int)%i, 256), "
							        "align_pow2((int)%i, 256));\n",
							        get_name(g->name), g->var_index, get_name(g->name), struct_size(g->type), struct_size(g->type));
						}
						else {
							fprintf(output,
							        "\tkore_opengl_command_list_set_uniform_buffer(list, set->%s, _%" PRIu64
							        "_uniform_block_index, 0, align_pow2((int)%i, 256));\n",
							        get_name(g->name), g->var_index, struct_size(g->type));
						}
					}
					else if (is_texture(g->type)) {
						type *t = get_type(g->type);
						if (t->array_size == UINT32_MAX) {
							fprintf(output, "\tfor (size_t index = 0; index < set->%s_count; ++index) {\n", get_name(g->name));
							fprintf(output, "\t\tkore_%s_descriptor_set_prepare_srv_texture(list, &set->%s[index]);\n", api_short, get_name(g->name));
							fprintf(output, "\t}\n");
						}
						else {
							if (writable) {
								fprintf(output, "\tkore_%s_command_list_set_texture(list, &set->%s, true);\n", api_short, get_name(g->name));
							}
							else {
								fprintf(output, "\tkore_%s_command_list_set_texture(list, &set->%s, false);\n", api_short, get_name(g->name));
							}
						}
					}
					else if (!is_sampler(g->type) && g->type != bvh_type_id) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\tkore_vulkan_descriptor_set_prepare_buffer(list, set->%s);\n", get_name(g->name));
						}
						else {
							fprintf(output, "\tkore_%s_descriptor_set_prepare_uav_buffer(list, set->%s, 0, UINT32_MAX);\n", api_short, get_name(g->name));
						}
					}
				}
			}

			fprintf(output, "\n");

			{
				uint32_t dynamic_count = 0;
				for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
					global *g = get_global(set->globals.globals[global_index]);

					if (!get_type(g->type)->built_in) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							dynamic_count += 1;
						}
					}
				}

				if (dynamic_count > 0) {
					fprintf(output, "\tkore_gpu_buffer *dynamic_buffers[%i];\n", dynamic_count);

					fprintf(output, "\tuint32_t dynamic_offsets[%i];\n", dynamic_count);

					fprintf(output, "\tuint32_t dynamic_sizes[%i];\n", dynamic_count);

					uint32_t dynamic_index = 0;
					for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
						global *g = get_global(set->globals.globals[global_index]);

						if (!get_type(g->type)->built_in) {
							if (has_attribute(&g->attributes, add_name("indexed"))) {
								fprintf(output, "\tdynamic_buffers[%i] = set->%s;\n", dynamic_index, get_name(g->name));
								fprintf(output, "\tdynamic_offsets[%i] = %s_index * align_pow2((int)%i, 256);\n", dynamic_index, get_name(g->name),
								        struct_size(g->type));
								fprintf(output, "\tdynamic_sizes[%i] = align_pow2((int)%i, 256);\n", dynamic_index, struct_size(g->type));
								dynamic_index += 1;
							}
						}
					}
				}

				if (api == API_DIRECT3D12) {
					fprintf(output, "\n\tkore_%s_command_list_set_descriptor_table(list, %s_table_index, &set->set", api_short, get_name(set->name));
					if (dynamic_count > 0) {
						fprintf(output, ", dynamic_buffers, dynamic_offsets, dynamic_sizes");
					}
					else {
						fprintf(output, ", NULL, NULL, NULL");
					}
					fprintf(output, ");\n");
				}
				else if (api == API_METAL) {
					fprintf(output,
					        "\n\tkore_metal_command_list_set_descriptor_set(list, %s_vertex_table_index, %s_fragment_table_index, %s_compute_table_index, "
					        "&set->set",
					        get_name(set->name), get_name(set->name), get_name(set->name));
					if (dynamic_count > 0) {
						fprintf(output, ", dynamic_buffers, dynamic_offsets, dynamic_sizes, %u", dynamic_count);
					}
					else {
						fprintf(output, ", NULL, NULL, NULL, 0");
					}
					fprintf(output, ");\n");
				}
				else if (api == API_VULKAN) {
					fprintf(output, "\n\tkore_vulkan_command_list_set_descriptor_set(list, %s_table_index, &set->set", get_name(set->name));
					if (dynamic_count > 0) {
						fprintf(output, ", %u, dynamic_offsets", dynamic_count);
					}
					else {
						fprintf(output, ", 0, NULL");
					}
					fprintf(output, ");\n");
				}
				else if (api == API_WEBGPU) {
					bool   found            = false;
					size_t global_set_index = 0;

					for (; global_set_index < get_sets_count(); ++global_set_index) {
						if (get_set(global_set_index) == set) {
							found = true;
							break;
						}
					}

					assert(found);

					fprintf(output, "\n\tkore_webgpu_command_list_set_bind_group(list, %zu, &set->set", global_set_index);
					if (dynamic_count > 0) {
						fprintf(output, ", %u, dynamic_offsets", dynamic_count);
					}
					else {
						fprintf(output, ", 0, NULL");
					}
					fprintf(output, ");\n");
				}
				else if (api == API_OPENGL) {
					fprintf(output, "\tkore_%s_command_list_set_descriptor_table(list, %s_table_index, &set->set", api_short, get_name(set->name));
					if (dynamic_count > 0) {
						fprintf(output, ", dynamic_buffers, dynamic_offsets, dynamic_sizes");
					}
					else {
						fprintf(output, ", NULL, NULL, NULL");
					}
					fprintf(output, ");\n");
				}
				fprintf(output, "}\n\n");
			}
		}

		if (api != API_METAL && api != API_OPENGL) {
			for (type_id i = 0; get_type(i) != NULL; ++i) {
				type *t = get_type(i);
				if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
					for (size_t j = 0; j < t->members.size; ++j) {
						if (t->members.m[j].name == add_name("vertex") || t->members.m[j].name == add_name("fragment")) {
							debug_context context = {0};
							check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "vertex or fragment expects an identifier");
							fprintf(output, "static kore_%s_shader %s;\n", api_short, get_name(t->members.m[j].value.identifier));
						}
					}
				}
			}
		}

		for (function_id i = 0; get_function(i) != NULL; ++i) {
			function *f = get_function(i);
			if (has_attribute(&f->attributes, add_name("compute"))) {
				fprintf(output, "static kore_%s_compute_pipeline %s;\n", api_short, get_name(f->name));
				fprintf(output, "void kong_set_compute_shader_%s(kore_gpu_command_list *list) {\n", get_name(f->name));
				if (api == API_METAL) {
					attribute *threads_attribute = find_attribute(&f->attributes, add_name("threads"));
					if (threads_attribute == NULL || threads_attribute->paramters_count != 3) {
						debug_context context = {0};
						error(context, "Compute function requires a threads attribute with three parameters");
					}

					fprintf(output, "\tkore_%s_command_list_set_compute_pipeline(list, &%s, %u, %u, %u);\n", api_short, get_name(f->name),
					        (uint32_t)threads_attribute->parameters[0], (uint32_t)threads_attribute->parameters[1], (uint32_t)threads_attribute->parameters[2]);
				}
				else {
					fprintf(output, "\tkore_%s_command_list_set_compute_pipeline(list, &%s);\n", api_short, get_name(f->name));
				}

				if (api != API_WEBGPU) {
					descriptor_set_group *group = find_descriptor_set_group_for_function(f);
					if (api == API_VULKAN) {
						size_t index = 0;
						for (size_t group_index = 0; group_index < group->size; ++group_index) {
							if (group->values[group_index]->name != add_name("root_constants")) {
								fprintf(output, "\t%s_table_index = %zu;\n", get_name(group->values[group_index]->name), index);
								++index;
							}
						}
					}
					else {
						for (size_t group_index = 0; group_index < group->size; ++group_index) {
							if (api == API_METAL) {
								fprintf(output, "\t%s_compute_table_index = %zu;\n", get_name(group->values[group_index]->name), group_index);
							}
							else {
								fprintf(output, "\t%s_table_index = %zu;\n", get_name(group->values[group_index]->name), group_index);
							}
						}
					}
				}

				fprintf(output, "}\n\n");
			}
		}

		if (api == API_OPENGL) {
			fprintf(output, "\nuint32_t kore_opengl_find_uniform_block_index(unsigned program, const char *name);\n");
		}

		if (api == API_DIRECT3D12) {
			fprintf(output, "struct ID3D12RootSignature;");

			for (type_id i = 0; get_type(i) != NULL; ++i) {
				type *t = get_type(i);
				if (!t->built_in && has_attribute(&t->attributes, add_name("raypipe"))) {
					fprintf(output, "struct ID3D12RootSignature *kong_create_%s_root_signature(kore_gpu_device *device);", get_name(t->name));
				}
			}
		}

		if (api == API_VULKAN) {
			fprintf(output, "\nvoid create_descriptor_set_layouts(kore_gpu_device *device);\n");
		}
		else if (api == API_WEBGPU) {
			fprintf(output, "\nvoid create_bind_group_layouts(kore_gpu_device *device);\n");
		}

		fprintf(output, "\nvoid kong_init(kore_gpu_device *device) {\n");

		if (api == API_VULKAN) {
			fprintf(output, "\tcreate_descriptor_set_layouts(device);\n\n");
		}
		else if (api == API_WEBGPU) {
			fprintf(output, "\tcreate_bind_group_layouts(device);\n\n");
		}

		for (type_id i = 0; get_type(i) != NULL; ++i) {
			type *t = get_type(i);
			if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
				fprintf(output, "\tkore_%s_render_pipeline_parameters %s_parameters = {0};\n\n", api_short, get_name(t->name));

				name_id vertex_shader_name        = NO_NAME;
				name_id amplification_shader_name = NO_NAME;
				name_id mesh_shader_name          = NO_NAME;
				name_id fragment_shader_name      = NO_NAME;

				int blend_source            = 1;
				int blend_destination       = 0;
				int blend_operation         = 0;
				int alpha_blend_source      = 1;
				int alpha_blend_destination = 0;
				int alpha_blend_operation   = 0;

				for (size_t j = 0; j < t->members.size; ++j) {
					if (t->members.m[j].name == add_name("vertex")) {
						if (api == API_METAL || api == API_WEBGPU) {
							fprintf(output, "\t%s_parameters.vertex.shader.function_name = \"%s\";\n", get_name(t->name),
							        get_name(t->members.m[j].value.identifier));
						}
						else {
							fprintf(output, "\t%s_parameters.vertex.shader.data = %s_code;\n", get_name(t->name), get_name(t->members.m[j].value.identifier));
							fprintf(output, "\t%s_parameters.vertex.shader.size = %s_code_size;\n\n", get_name(t->name),
							        get_name(t->members.m[j].value.identifier));
							if (api == API_OPENGL) {
								fprintf(output, "\t%s_parameters.vertex.shader.flip_data = %s_flip_code;\n", get_name(t->name),
								        get_name(t->members.m[j].value.identifier));
								fprintf(output, "\t%s_parameters.vertex.shader.flip_size = %s_flip_code_size;\n\n", get_name(t->name),
								        get_name(t->members.m[j].value.identifier));
							}
						}
						vertex_shader_name = t->members.m[j].value.identifier;
					}
					else if (t->members.m[j].name == add_name("amplification")) {
						amplification_shader_name = t->members.m[j].value.identifier;
					}
					else if (t->members.m[j].name == add_name("mesh")) {
						mesh_shader_name = t->members.m[j].value.identifier;
					}
					else if (t->members.m[j].name == add_name("fragment")) {
						if (api == API_METAL || api == API_WEBGPU) {
							fprintf(output, "\t%s_parameters.fragment.shader.function_name = \"%s\";\n", get_name(t->name),
							        get_name(t->members.m[j].value.identifier));
						}
						else {
							fprintf(output, "\t%s_parameters.fragment.shader.data = %s_code;\n", get_name(t->name), get_name(t->members.m[j].value.identifier));
							fprintf(output, "\t%s_parameters.fragment.shader.size = %s_code_size;\n\n", get_name(t->name),
							        get_name(t->members.m[j].value.identifier));
							if (api == API_OPENGL) {
								fprintf(output, "\t%s_parameters.fragment.shader.flip_data = NULL;\n", get_name(t->name));
								fprintf(output, "\t%s_parameters.fragment.shader.flip_size = 0;\n\n", get_name(t->name));
							}
						}
						fragment_shader_name = t->members.m[j].value.identifier;
					}
					// else if (t->members.m[j].name == add_name("depth_write")) {
					//	debug_context context = {0};
					//	check(t->members.m[j].value.kind == TOKEN_BOOLEAN, context, "depth_write expects a bool");
					//	fprintf(output, "\t%s.depth_write = %s;\n\n", get_name(t->name), t->members.m[j].value.boolean ? "true" : "false");
					// }
					// else if (t->members.m[j].name == add_name("depth_mode")) {
					//	debug_context context = {0};
					//	check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "depth_mode expects an identifier");
					//	global *g = find_global(t->members.m[j].value.identifier);
					//	fprintf(output, "\t%s.depth_mode = %s;\n\n", get_name(t->name), convert_compare_mode(g->value.value.ints[0]));
					//}
					else if (t->members.m[j].name == add_name("blend_source")) {
						debug_context context = {0};
						check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "blend_source expects an identifier");
						global *g    = find_global(t->members.m[j].value.identifier);
						blend_source = g->value.value.ints[0];
					}
					else if (t->members.m[j].name == add_name("blend_destination")) {
						debug_context context = {0};
						check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "blend_destination expects an identifier");
						global *g         = find_global(t->members.m[j].value.identifier);
						blend_destination = g->value.value.ints[0];
					}
					else if (t->members.m[j].name == add_name("blend_operation")) {
						debug_context context = {0};
						check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "blend_operation expects an identifier");
						global *g       = find_global(t->members.m[j].value.identifier);
						blend_operation = g->value.value.ints[0];
					}
					else if (t->members.m[j].name == add_name("alpha_blend_source")) {
						debug_context context = {0};
						check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "alpha_blend_source expects an identifier");
						global *g          = find_global(t->members.m[j].value.identifier);
						alpha_blend_source = g->value.value.ints[0];
					}
					else if (t->members.m[j].name == add_name("alpha_blend_destination")) {
						debug_context context = {0};
						check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "alpha_blend_destination expects an identifier");
						global *g               = find_global(t->members.m[j].value.identifier);
						alpha_blend_destination = g->value.value.ints[0];
					}
					else if (t->members.m[j].name == add_name("alpha_blend_operation")) {
						debug_context context = {0};
						check(t->members.m[j].value.kind == TOKEN_IDENTIFIER, context, "alpha_blend_operation expects an identifier");
						global *g             = find_global(t->members.m[j].value.identifier);
						alpha_blend_operation = g->value.value.ints[0];
					}
					// else {
					//	debug_context context = {0};
					//	error(context, "Unsupported pipe member %s", get_name(t->members.m[j].name));
					// }
				}

				{
					debug_context context = {0};
					check(vertex_shader_name != NO_NAME || mesh_shader_name != NO_NAME, context, "No vertex or mesh shader name found");
					check(fragment_shader_name != NO_NAME, context, "No fragment shader name found");
				}

				function *vertex_function   = NULL;
				function *fragment_function = NULL;

				type_id vertex_inputs[64]   = {0};
				bool    instanced[64]       = {0};
				size_t  vertex_inputs_count = 0;

				if (vertex_shader_name != NO_NAME) {
					for (function_id i = 0; get_function(i) != NULL; ++i) {
						function *f = get_function(i);
						if (f->name == vertex_shader_name) {
							vertex_function       = f;
							debug_context context = {0};
							check(f->parameters_size > 0, context, "Vertex function requires at least one parameter");
							for (size_t input_index = 0; input_index < f->parameters_size; ++input_index) {
								vertex_inputs[input_index] = f->parameter_types[input_index].type;
								if (f->parameter_attributes[input_index] == add_name("per_instance")) {
									instanced[input_index] = true;
								}
							}
							vertex_inputs_count = f->parameters_size;
							break;
						}
					}
				}

				for (function_id i = 0; get_function(i) != NULL; ++i) {
					function *f = get_function(i);
					if (f->name == fragment_shader_name) {
						fragment_function = f;
						break;
					}
				}

				{
					debug_context context = {0};
					check(vertex_shader_name == NO_NAME || vertex_function != NULL, context, "Vertex function not found");
					check(fragment_function != NULL, context, "Fragment function not found");
					check(vertex_function == NULL || (vertex_inputs_count > 0 && vertex_inputs[0] != NO_TYPE), context, "No vertex input found");
				}

				if (vertex_function != NULL) {
					size_t location = 0;

					for (size_t input_index = 0; input_index < vertex_inputs_count; ++input_index) {
						type  *vertex_type = get_type(vertex_inputs[input_index]);
						size_t offset      = 0;

						for (size_t j = 0; j < vertex_type->members.size; ++j) {
							fprintf(output, "\t%s_parameters.vertex.buffers[%zu].attributes[%zu].format = %s;\n", get_name(t->name), input_index, j,
							        structure_type(vertex_type->members.m[j].type.type, api));
							fprintf(output, "\t%s_parameters.vertex.buffers[%zu].attributes[%zu].offset = %zu;\n", get_name(t->name), input_index, j, offset);
							if (api == API_OPENGL) {
								fprintf(output, "\t%s_parameters.vertex.buffers[%zu].attributes[%zu].name = \"%s_%s\";\n", get_name(t->name), input_index, j,
								        get_name(vertex_type->name), get_name(vertex_type->members.m[j].name));
							}
							else {
								fprintf(output, "\t%s_parameters.vertex.buffers[%zu].attributes[%zu].shader_location = %zu;\n", get_name(t->name), input_index,
								        j, location);
							}

							offset += base_type_size(vertex_type->members.m[j].type.type);
							location += 1;
						}
						fprintf(output, "\t%s_parameters.vertex.buffers[%zu].attributes_count = %zu;\n", get_name(t->name), input_index,
						        vertex_type->members.size);
						fprintf(output, "\t%s_parameters.vertex.buffers[%zu].array_stride = %zu;\n", get_name(t->name), input_index, offset);

						char step_mode[64];
						if (instanced[input_index]) {
							sprintf(step_mode, "KORE_%s_VERTEX_STEP_MODE_INSTANCE", api_caps);
						}
						else {
							sprintf(step_mode, "KORE_%s_VERTEX_STEP_MODE_VERTEX", api_caps);
						}
						fprintf(output, "\t%s_parameters.vertex.buffers[%zu].step_mode = %s;\n", get_name(t->name), input_index, step_mode);
					}
					fprintf(output, "\t%s_parameters.vertex.buffers_count = %zu;\n\n", get_name(t->name), vertex_inputs_count);
				}

				fprintf(output, "\t%s_parameters.primitive.topology = KORE_%s_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;\n", get_name(t->name), api_caps);
				fprintf(output, "\t%s_parameters.primitive.strip_index_format = KORE_GPU_INDEX_FORMAT_UINT16;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.primitive.front_face = KORE_%s_FRONT_FACE_CW;\n", get_name(t->name), api_caps);
				fprintf(output, "\t%s_parameters.primitive.cull_mode = KORE_%s_CULL_MODE_NONE;\n", get_name(t->name), api_caps);
				fprintf(output, "\t%s_parameters.primitive.unclipped_depth = false;\n\n", get_name(t->name));

				member *depth_stencil_format = find_member(t, "depth_stencil_format");
				if (depth_stencil_format != NULL) {
					debug_context context = {0};
					check(depth_stencil_format->value.kind == TOKEN_IDENTIFIER, context, "depth_stencil_format expects an identifier");
					global *g = find_global(depth_stencil_format->value.identifier);
					fprintf(output, "\t%s_parameters.depth_stencil.format = %s;\n", get_name(t->name), convert_texture_format(g->value.value.ints[0]));
				}
				else {
					fprintf(output, "\t%s_parameters.depth_stencil.format = KORE_GPU_TEXTURE_FORMAT_UNDEFINED;\n", get_name(t->name));
				}

				member *depth_write = find_member(t, "depth_write");
				if (depth_write != NULL) {
					debug_context context = {0};
					check(depth_write->value.kind == TOKEN_BOOLEAN, context, "depth_write expects a bool");
					fprintf(output, "\t%s_parameters.depth_stencil.depth_write_enabled = %s;\n", get_name(t->name),
					        depth_write->value.boolean ? "true" : "false");
				}
				else {
					fprintf(output, "\t%s_parameters.depth_stencil.depth_write_enabled = false;\n", get_name(t->name));
				}

				member *depth_compare = find_member(t, "depth_compare");
				if (depth_compare != NULL) {
					debug_context context = {0};
					check(depth_compare->value.kind == TOKEN_IDENTIFIER, context, "depth_compare expects an identifier");
					global *g = find_global(depth_compare->value.identifier);
					fprintf(output, "\t%s_parameters.depth_stencil.depth_compare = %s;\n", get_name(t->name), convert_compare_mode(g->value.value.ints[0]));
				}
				else {
					fprintf(output, "\t%s_parameters.depth_stencil.depth_compare = KORE_GPU_COMPARE_FUNCTION_ALWAYS;\n", get_name(t->name));
				}

				fprintf(output, "\t%s_parameters.depth_stencil.stencil_front.compare = KORE_GPU_COMPARE_FUNCTION_ALWAYS;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.depth_stencil.stencil_front.fail_op = KORE_%s_STENCIL_OPERATION_KEEP;\n", get_name(t->name), api_caps);
				fprintf(output, "\t%s_parameters.depth_stencil.stencil_front.depth_fail_op = KORE_%s_STENCIL_OPERATION_KEEP;\n", get_name(t->name), api_caps);
				fprintf(output, "\t%s_parameters.depth_stencil.stencil_front.pass_op = KORE_%s_STENCIL_OPERATION_KEEP;\n", get_name(t->name), api_caps);

				fprintf(output, "\t%s_parameters.depth_stencil.stencil_back.compare = KORE_GPU_COMPARE_FUNCTION_ALWAYS;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.depth_stencil.stencil_back.fail_op = KORE_%s_STENCIL_OPERATION_KEEP;\n", get_name(t->name), api_caps);
				fprintf(output, "\t%s_parameters.depth_stencil.stencil_back.depth_fail_op = KORE_%s_STENCIL_OPERATION_KEEP;\n", get_name(t->name), api_caps);
				fprintf(output, "\t%s_parameters.depth_stencil.stencil_back.pass_op = KORE_%s_STENCIL_OPERATION_KEEP;\n", get_name(t->name), api_caps);

				fprintf(output, "\t%s_parameters.depth_stencil.stencil_read_mask = 0xffffffff;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.depth_stencil.stencil_write_mask = 0xffffffff;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.depth_stencil.depth_bias = 0;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.depth_stencil.depth_bias_slope_scale = 0.0f;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.depth_stencil.depth_bias_clamp = 0.0f;\n\n", get_name(t->name));

				fprintf(output, "\t%s_parameters.multisample.count = 1;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.multisample.mask = 0xffffffff;\n", get_name(t->name));
				fprintf(output, "\t%s_parameters.multisample.alpha_to_coverage_enabled = false;\n\n", get_name(t->name));

				if (get_type(fragment_function->return_type.type)->array_size > 0) {
					fprintf(output, "\t%s_parameters.fragment.targets_count = %i;\n", get_name(t->name),
					        get_type(fragment_function->return_type.type)->array_size);
					for (uint32_t i = 0; i < get_type(fragment_function->return_type.type)->array_size; ++i) {
						char format_name[32];
						sprintf(format_name, "format%i", i);
						member *format = find_member(t, format_name);
						if (format == NULL && i == 0) {
							format = find_member(t, "format");
						}
						if (format != NULL) {
							debug_context context = {0};
							check(format->value.kind == TOKEN_IDENTIFIER, context, "format expects an identifier");

							if (strcmp(get_name(format->value.identifier), "framebuffer_format") == 0) {
								fprintf(output, "\t%s_parameters.fragment.targets[%i].format = kore_gpu_device_framebuffer_format(device);\n",
								        get_name(t->name), i);
							}
							else {
								global *g = find_global(format->value.identifier);
								fprintf(output, "\t%s_parameters.fragment.targets[%i].format = %s;\n", get_name(t->name), i,
								        convert_texture_format(g->value.value.ints[0]));
							}
						}
						else {
							fprintf(output, "\t%s_parameters.fragment.targets[%i].format = KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM;\n", get_name(t->name), i);
						}

						fprintf(output, "\t%s_parameters.fragment.targets[%i].write_mask = 0xf;\n\n", get_name(t->name), i);
					}
					fprintf(output, "\n");
				}
				else {
					member *format = find_member(t, "format");
					if (format == NULL) {
						format = find_member(t, "format0");
					}

					if (format == NULL) {
						fprintf(output, "\t%s_parameters.fragment.targets_count = 0;\n", get_name(t->name));
					}
					else {
						fprintf(output, "\t%s_parameters.fragment.targets_count = 1;\n", get_name(t->name));

						debug_context context = {0};
						check(format->value.kind == TOKEN_IDENTIFIER, context, "format expects an identifier");

						if (strcmp(get_name(format->value.identifier), "framebuffer_format") == 0) {
							fprintf(output, "\t%s_parameters.fragment.targets[0].format = kore_gpu_device_framebuffer_format(device);\n", get_name(t->name));
						}
						else {
							global *g = find_global(format->value.identifier);
							fprintf(output, "\t%s_parameters.fragment.targets[0].format = %s;\n", get_name(t->name),
							        convert_texture_format(g->value.value.ints[0]));
						}

						fprintf(output, "\t%s_parameters.fragment.targets[0].write_mask = 0xf;\n\n", get_name(t->name));
					}
				}

				uint32_t target_count = get_type(fragment_function->return_type.type)->array_size;

				if (target_count == 0) {
					member *format = find_member(t, "format");
					if (format == NULL) {
						format = find_member(t, "format0");
					}

					if (format != NULL) {
						target_count = 1;
					}
				}

				for (uint32_t target_index = 0; target_index < target_count; ++target_index) {
					fprintf(output, "\t%s_parameters.fragment.targets[%u].blend.color.src_factor = %s;\n", get_name(t->name), target_index,
					        convert_blend_mode(blend_source, api_caps));
					fprintf(output, "\t%s_parameters.fragment.targets[%u].blend.color.dst_factor = %s;\n", get_name(t->name), target_index,
					        convert_blend_mode(blend_destination, api_caps));
					fprintf(output, "\t%s_parameters.fragment.targets[%u].blend.color.operation = %s;\n", get_name(t->name), target_index,
					        convert_blend_op(blend_operation, api_caps));
					fprintf(output, "\t%s_parameters.fragment.targets[%u].blend.alpha.src_factor = %s;\n", get_name(t->name), target_index,
					        convert_blend_mode(alpha_blend_source, api_caps));
					fprintf(output, "\t%s_parameters.fragment.targets[%u].blend.alpha.dst_factor = %s;\n", get_name(t->name), target_index,
					        convert_blend_mode(alpha_blend_destination, api_caps));
					fprintf(output, "\t%s_parameters.fragment.targets[%u].blend.alpha.operation = %s;\n\n", get_name(t->name), target_index,
					        convert_blend_op(alpha_blend_operation, api_caps));
				}

				if (api == API_VULKAN) {
					descriptor_set_group *group = find_descriptor_set_group_for_pipe_type(t);

					fprintf(output, "\t{\n");

					if (group->size == 0) {
						fprintf(output, "\t\tkore_%s_render_pipeline_init(&device->%s, &%s, &%s_parameters, NULL, 0, 0);\n", api_short, api_short,
						        get_name(t->name), get_name(t->name));
					}
					else {
						size_t root_constants_size = 0;
						size_t group_size          = group->size;

						for (size_t layout_index = 0; layout_index < group->size; ++layout_index) {
							if (group->values[layout_index]->name == add_name("root_constants")) {
								--group_size;
							}
						}

						fprintf(output, "\t\tVkDescriptorSetLayout layouts[%zu];\n", group_size);

						size_t layout_index = 0;
						for (size_t i = 0; i < group->size; ++i) {
							if (group->values[i]->name == add_name("root_constants")) {
								for (size_t global_index = 0; global_index < group->values[i]->globals.size; ++global_index) {
									global *g = get_global(group->values[i]->globals.globals[global_index]);
									root_constants_size += struct_size(g->type);
								}

								continue;
							}

							fprintf(output, "\t\tlayouts[%zu] = %s_set_layout;\n", layout_index, get_name(group->values[i]->name));
							++layout_index;
						}

						fprintf(output, "\t\tkore_%s_render_pipeline_init(&device->%s, &%s, &%s_parameters, layouts, %zu, %zu);\n", api_short, api_short,
						        get_name(t->name), get_name(t->name), group_size, root_constants_size);
					}

					fprintf(output, "\t}\n");
				}
				else if (api == API_WEBGPU) {

					descriptor_set_group *group = find_descriptor_set_group_for_pipe_type(t);

					fprintf(output, "\t{\n");

					if (group->size == 0) {
						fprintf(output, "\t\tkore_webgpu_render_pipeline_init(&device->webgpu, &%s, &%s_parameters, NULL, 0);\n", get_name(t->name),
						        get_name(t->name));
					}
					else {

						fprintf(output, "\t\tWGPUBindGroupLayout layouts[%zu];\n", get_sets_count());

						for (size_t layout_index = 0; layout_index < get_sets_count(); ++layout_index) {
							bool found = false;
							for (size_t layout_in_group_index = 0; layout_in_group_index < group->size; ++layout_in_group_index) {
								if (get_set(layout_index) == group->values[layout_in_group_index]) {
									found = true;
									break;
								}
							}

							if (found) {
								fprintf(output, "\t\tlayouts[%zu] = %s_set_layout;\n", layout_index, get_name(get_set(layout_index)->name));
							}
							else {
								fprintf(output, "\t\tlayouts[%zu] = NULL;\n", layout_index);
							}
						}

						fprintf(output, "\t\tkore_webgpu_render_pipeline_init(&device->webgpu, &%s, &%s_parameters, layouts, %zu);\n", get_name(t->name),
						        get_name(t->name), get_sets_count());
					}

					fprintf(output, "\t}\n");
				}
				else {
					fprintf(output, "\tkore_%s_render_pipeline_init(&device->%s, &%s, &%s_parameters);\n\n", api_short, api_short, get_name(t->name),
					        get_name(t->name));
				}

				if (api == API_OPENGL) {
					global_array globals = {0};

					find_referenced_globals(vertex_function, &globals);
					find_referenced_globals(fragment_function, &globals);

					for (global_id i = 0; i < globals.size; ++i) {
						global *g = get_global(globals.globals[i]);
						if (g->type == sampler_type_id) {
						}
						else if (is_texture(g->type)) {
						}
						else if (g->type == float_id) {
						}
						else {
							fprintf(output, "\t%s_%" PRIu64 "_uniform_block_index = kore_opengl_find_uniform_block_index(%s.program, \"_%" PRIu64 "\");\n\n",
							        get_name(t->name), g->var_index, get_name(t->name), g->var_index);
						}
					}
				}
			}
		}

		for (function_id i = 0; get_function(i) != NULL; ++i) {
			function *f = get_function(i);
			if (has_attribute(&f->attributes, add_name("compute"))) {
				fprintf(output, "\tkore_%s_compute_pipeline_parameters %s_parameters;\n", api_short, get_name(f->name));
				if (api == API_METAL || api == API_WEBGPU) {
					fprintf(output, "\t%s_parameters.shader.function_name = \"%s\";\n", get_name(f->name), get_name(f->name));
				}
				else {
					fprintf(output, "\t%s_parameters.shader.data = %s_code;\n", get_name(f->name), get_name(f->name));
					fprintf(output, "\t%s_parameters.shader.size = %s_code_size;\n", get_name(f->name), get_name(f->name));
				}
				if (api == API_VULKAN) {
					descriptor_set_group *group = find_descriptor_set_group_for_function(f);

					fprintf(output, "\t{\n");

					if (group->size == 0) {
						fprintf(output, "\t\tkore_%s_compute_pipeline_init(&device->%s, &%s, &%s_parameters, NULL, 0, 0);\n", api_short, api_short,
						        get_name(f->name), get_name(f->name));
					}
					else {
						size_t root_constants_size = 0;
						size_t group_size          = group->size;

						for (size_t layout_index = 0; layout_index < group->size; ++layout_index) {
							if (group->values[layout_index]->name == add_name("root_constants")) {
								--group_size;
							}
						}

						fprintf(output, "\t\tVkDescriptorSetLayout layouts[%zu];\n", group_size);

						size_t layout_index = 0;
						for (size_t i = 0; i < group->size; ++i) {
							if (group->values[i]->name == add_name("root_constants")) {
								for (size_t global_index = 0; global_index < group->values[i]->globals.size; ++global_index) {
									global *g = get_global(group->values[i]->globals.globals[global_index]);
									root_constants_size += struct_size(g->type);
								}

								continue;
							}

							fprintf(output, "\t\tlayouts[%zu] = %s_set_layout;\n", layout_index, get_name(group->values[i]->name));
							++layout_index;
						}

						fprintf(output, "\t\tkore_%s_compute_pipeline_init(&device->%s, &%s, &%s_parameters, layouts, %zu, %zu);\n", api_short, api_short,
						        get_name(f->name), get_name(f->name), group_size, root_constants_size);
					}

					fprintf(output, "\t}\n");
				}
				else if (api == API_WEBGPU) {
					descriptor_set_group *group = find_descriptor_set_group_for_function(f);

					fprintf(output, "\t{\n");

					if (group->size == 0) {
						fprintf(output, "\t\tkore_webgpu_render_pipeline_init(&device->webgpu, &%s, &%s_parameters, NULL, 0);\n", get_name(f->name),
						        get_name(f->name));
					}
					else {
						fprintf(output, "\t\tWGPUBindGroupLayout layouts[%zu];\n", get_sets_count());

						for (size_t layout_index = 0; layout_index < get_sets_count(); ++layout_index) {
							bool found = false;
							for (size_t layout_in_group_index = 0; layout_in_group_index < group->size; ++layout_in_group_index) {
								if (get_set(layout_index) == group->values[layout_in_group_index]) {
									found = true;
									break;
								}
							}

							if (found) {
								fprintf(output, "\t\tlayouts[%zu] = %s_set_layout;\n", layout_index, get_name(get_set(layout_index)->name));
							}
							else {
								fprintf(output, "\t\tlayouts[%zu] = NULL;\n", layout_index);
							}
						}

						fprintf(output, "\t\tkore_webgpu_compute_pipeline_init(&device->webgpu, &%s, &%s_parameters, layouts, %zu);\n", get_name(f->name),
						        get_name(f->name), get_sets_count());
					}

					fprintf(output, "\t}\n");
				}
				else {
					fprintf(output, "\tkore_%s_compute_pipeline_init(&device->%s, &%s, &%s_parameters);\n", api_short, api_short, get_name(f->name),
					        get_name(f->name));
				}
			}
		}

		for (type_id i = 0; get_type(i) != NULL; ++i) {
			type *t = get_type(i);
			if (!t->built_in && has_attribute(&t->attributes, add_name("raypipe"))) {
				fprintf(output, "\tkore_%s_ray_pipeline_parameters %s_parameters = {0};\n\n", api_short, get_name(t->name));

				name_id gen_shader_name          = NO_NAME;
				name_id miss_shader_name         = NO_NAME;
				name_id closest_shader_name      = NO_NAME;
				name_id intersection_shader_name = NO_NAME;
				name_id any_shader_name          = NO_NAME;

				for (size_t j = 0; j < t->members.size; ++j) {
					if (t->members.m[j].name == add_name("gen")) {
						gen_shader_name = t->members.m[j].value.identifier;
					}
					else if (t->members.m[j].name == add_name("miss")) {
						miss_shader_name = t->members.m[j].value.identifier;
					}
					else if (t->members.m[j].name == add_name("closest")) {
						closest_shader_name = t->members.m[j].value.identifier;
					}
					else if (t->members.m[j].name == add_name("intersection")) {
						intersection_shader_name = t->members.m[j].value.identifier;
					}
					else if (t->members.m[j].name == add_name("any")) {
						any_shader_name = t->members.m[j].value.identifier;
					}
				}

				{
					debug_context context = {0};
					check(gen_shader_name != NO_NAME, context, "No ray gen shader name found");
					check(miss_shader_name != NO_NAME, context, "No miss shader name found");
					check(closest_shader_name != NO_NAME, context, "No closest hit shader name found");
				}

				fprintf(output, "\t%s_parameters.gen_shader_name = \"%s\";\n", get_name(t->name), get_name(gen_shader_name));
				fprintf(output, "\t%s_parameters.miss_shader_name = \"%s\";\n", get_name(t->name), get_name(miss_shader_name));
				fprintf(output, "\t%s_parameters.closest_shader_name = \"%s\";\n", get_name(t->name), get_name(closest_shader_name));
				if (intersection_shader_name != NO_NAME) {
					fprintf(output, "\t%s_parameters.intersection_shader_name = \"%s\";\n", get_name(t->name), get_name(intersection_shader_name));
				}
				if (any_shader_name != NO_NAME) {
					fprintf(output, "\t%s_parameters.any_shader_name = \"%s\";\n", get_name(t->name), get_name(any_shader_name));
				}

				fprintf(output, "\n\tkore_%s_ray_pipeline_init(device, &%s, &%s_parameters, kong_create_%s_root_signature(device));\n\n", api_short,
				        get_name(t->name), get_name(t->name), get_name(t->name));
			}
		}

		fprintf(output, "}\n");

		fclose(output);
	}

	if (api == API_DIRECT3D12) {
		char filename[512];
		sprintf(filename, "%s/%s", directory, "kong_ray_root_signatures.c");

		FILE *output = fopen(filename, "wb");

		fprintf(output, "#include <kore3/gpu/device.h>\n\n");
		fprintf(output, "#include <d3d12.h>\n\n");

		for (type_id i = 0; get_type(i) != NULL; ++i) {
			type *t = get_type(i);
			if (!t->built_in && has_attribute(&t->attributes, add_name("raypipe"))) {
				fprintf(output, "ID3D12RootSignature *kong_create_%s_root_signature(kore_gpu_device *device) {\n", get_name(t->name));
				write_root_signature(output, sets, sets_count);
				fprintf(output, "}\n");
			}
		}

		fclose(output);
	}
	else if (api == API_VULKAN) {
		char filename[512];
		sprintf(filename, "%s/%s", directory, "kong_descriptor_sets.c");

		FILE *output = fopen(filename, "wb");

		fprintf(output, "#include <kore3/gpu/device.h>\n\n");

		fprintf(output, "#include <assert.h>\n\n");

		for (size_t set_index = 0; set_index < sets_count; ++set_index) {
			descriptor_set *set = sets[set_index];

			fprintf(output, "VkDescriptorSetLayout %s_set_layout;\n", get_name(set->name));
		}

		fprintf(output, "\n");

		fprintf(output, "void create_descriptor_set_layouts(kore_gpu_device *device) {\n");

		for (size_t set_index = 0; set_index < sets_count; ++set_index) {
			descriptor_set *set = sets[set_index];

			fprintf(output, "\t{\n");

			fprintf(output, "\t\tVkDescriptorSetLayoutBinding layout_bindings[%zu] = {\n", set->globals.size);

			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g        = get_global(set->globals.globals[global_index]);
				bool    readable = set->globals.readable[global_index];
				bool    writable = set->globals.writable[global_index];

				if (g->type == tex2d_type_id) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (readable | writable) {
						fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,\n");
					}
					else {
						fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,\n");
					}
					fprintf(output, "\t\t\t\t.descriptorCount = 1,\n");
					fprintf(output, "\t\t\t\t.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT,\n");
					fprintf(output, "\t\t\t\t.pImmutableSamplers = NULL,\n");
					fprintf(output, "\t\t\t},\n");
				}
				else if (g->type == tex2darray_type_id) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (readable | writable) {
						fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,\n");
					}
					else {
						fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,\n");
					}
					fprintf(output, "\t\t\t\t.descriptorCount = 1,\n");
					fprintf(output, "\t\t\t\t.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT,\n");
					fprintf(output, "\t\t\t\t.pImmutableSamplers = NULL,\n");
					fprintf(output, "\t\t\t},\n");
				}
				else if (g->type == texcube_type_id) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (readable | writable) {
						fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,\n");
					}
					else {
						fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,\n");
					}
					fprintf(output, "\t\t\t\t.descriptorCount = 1,\n");
					fprintf(output, "\t\t\t\t.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT,\n");
					fprintf(output, "\t\t\t\t.pImmutableSamplers = NULL,\n");
					fprintf(output, "\t\t\t},\n");
				}
				else if (is_sampler(g->type)) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,\n");
					fprintf(output, "\t\t\t\t.descriptorCount = 1,\n");
					fprintf(output, "\t\t\t\t.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT,\n");
					fprintf(output, "\t\t\t\t.pImmutableSamplers = NULL,\n");
					fprintf(output, "\t\t\t},\n");
				}
				else if (!get_type(g->type)->built_in) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (writable) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,\n");
						}
						else {
							fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,\n");
						}
					}
					else {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,\n");
						}
						else {
							fprintf(output, "\t\t\t\t.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,\n");
						}
					}
					fprintf(output, "\t\t\t\t.descriptorCount = 1,\n");
					fprintf(output, "\t\t\t\t.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT,\n");
					fprintf(output, "\t\t\t\t.pImmutableSamplers = NULL,\n");
					fprintf(output, "\t\t\t},\n");
				}
			}

			fprintf(output, "\t\t};\n");

			fprintf(output, "\n");

			fprintf(output, "\t\tVkDescriptorSetLayoutCreateInfo layout_create_info = {\n");
			fprintf(output, "\t\t\t.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,\n");
			fprintf(output, "\t\t\t.pNext = NULL,\n");
			fprintf(output, "\t\t\t.bindingCount = %zu,\n", set->globals.size);
			fprintf(output, "\t\t\t.pBindings = layout_bindings,\n");
			fprintf(output, "\t\t};\n");

			fprintf(output, "\t\tVkResult result = vkCreateDescriptorSetLayout(device->vulkan.device, &layout_create_info, NULL, &%s_set_layout);\n",
			        get_name(set->name));
			fprintf(output, "\t\tassert(result == VK_SUCCESS);\n");

			fprintf(output, "\t}\n");
		}

		fprintf(output, "}\n");

		fclose(output);
	}
	else if (api == API_WEBGPU) {
		char filename[512];
		sprintf(filename, "%s/%s", directory, "kong_bind_groups.c");

		FILE *output = fopen(filename, "wb");

		fprintf(output, "#include <kore3/gpu/device.h>\n\n");

		fprintf(output, "#include <assert.h>\n\n");

		for (size_t set_index = 0; set_index < sets_count; ++set_index) {
			descriptor_set *set = sets[set_index];

			fprintf(output, "WGPUBindGroupLayout %s_set_layout;\n", get_name(set->name));
		}

		fprintf(output, "\n");

		fprintf(output, "void create_bind_group_layouts(kore_gpu_device *device) {\n");

		for (size_t set_index = 0; set_index < sets_count; ++set_index) {
			descriptor_set *set = sets[set_index];

			fprintf(output, "\t{\n");

			fprintf(output, "\t\tWGPUBindGroupLayoutEntry layout_entries[%zu] = {\n", set->globals.size);

			for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
				global *g        = get_global(set->globals.globals[global_index]);
				bool    writable = set->globals.writable[global_index];

				if (g->type == tex2d_type_id) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (writable) {
						fprintf(output, "\t\t\t\t.storageTexture = {.viewDimension = WGPUTextureViewDimension_2D, .format = WGPUTextureFormat_RGBA32Float, "
						                ".access = WGPUStorageTextureAccess_WriteOnly},\n");
					}
					else {
						fprintf(output, "\t\t\t\t.texture = {.sampleType = WGPUTextureSampleType_Float, .viewDimension = WGPUTextureViewDimension_2D},\n");
					}
					if (writable) {
						fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					}
					else {
						fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					}
					fprintf(output, "\t\t\t},\n");
				}
				else if (g->type == tex2darray_type_id) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (writable) {
						fprintf(output,
						        "\t\t\t\t.storageTexture = {.viewDimension = WGPUTextureViewDimension_2DArray, .format = WGPUTextureFormat_RGBA32Float, "
						        ".access = WGPUStorageTextureAccess_WriteOnly},\n");
					}
					else {
						fprintf(output, "\t\t\t\t.texture = {.sampleType = WGPUTextureSampleType_Float, .viewDimension = WGPUTextureViewDimension_2DArray},\n");
					}
					if (writable) {
						fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					}
					else {
						fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					}
					fprintf(output, "\t\t\t},\n");
				}
				else if (g->type == texcube_type_id) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (writable) {
						fprintf(output, "\t\t\t\t.storageTexture = {.viewDimension = WGPUTextureViewDimension_Cube, .format = WGPUTextureFormat_RGBA32Float, "
						                ".access = WGPUStorageTextureAccess_WriteOnly},\n");
					}
					else {
						fprintf(output, "\t\t\t\t.texture = {.sampleType = WGPUTextureSampleType_Float, .viewDimension = WGPUTextureViewDimension_Cube},\n");
					}
					if (writable) {
						fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					}
					else {
						fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					}
					fprintf(output, "\t\t\t},\n");
				}
				else if (is_sampler(g->type)) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					fprintf(output, "\t\t\t\t.sampler = {.type = WGPUSamplerBindingType_Filtering},\n");
					fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					fprintf(output, "\t\t\t},\n");
				}
				else if (!get_type(g->type)->built_in) {
					fprintf(output, "\t\t\t{\n");
					fprintf(output, "\t\t\t\t.binding = %zu,\n", global_index);
					if (writable) {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\t\t\t\t.buffer = {.type = WGPUBufferBindingType_Storage, .hasDynamicOffset = true},\n");
						}
						else {
							fprintf(output, "\t\t\t\t.buffer = {.type = WGPUBufferBindingType_Storage},\n");
						}
					}
					else {
						if (has_attribute(&g->attributes, add_name("indexed"))) {
							fprintf(output, "\t\t\t\t.buffer = {.type = WGPUBufferBindingType_Uniform, .hasDynamicOffset = true},\n");
						}
						else {
							fprintf(output, "\t\t\t\t.buffer = {.type = WGPUBufferBindingType_Uniform},\n");
						}
					}
					fprintf(output, "\t\t\t\t.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute,\n");
					fprintf(output, "\t\t\t},\n");
				}
			}

			fprintf(output, "\t\t};\n");

			fprintf(output, "\n");

			fprintf(output, "\t\tWGPUBindGroupLayoutDescriptor bind_group_layout_descriptor = {\n");
			fprintf(output, "\t\t\t.entryCount = %zu,\n", set->globals.size);
			fprintf(output, "\t\t\t.entries = layout_entries,\n");
			fprintf(output, "\t\t};\n");

			fprintf(output, "\t\t%s_set_layout = wgpuDeviceCreateBindGroupLayout(device->webgpu.device, &bind_group_layout_descriptor);\n",
			        get_name(set->name));

			fprintf(output, "\t}\n");
		}

		fprintf(output, "}\n");

		fclose(output);
	}
}
