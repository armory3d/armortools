#include "vulkan.h"
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>
#include <vulkan/vulkan_core.h>
#include <assert.h>

VkDescriptorSetLayout desc_layout;
extern kinc_g5_texture_t *vulkanTextures[16];
extern kinc_g5_render_target_t *vulkanRenderTargets[16];
extern uint32_t swapchainImageCount;
extern uint32_t current_buffer;
bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

static VkDescriptorPool descriptor_pool;

static bool has_number(kinc_internal_named_number *named_numbers, const char *name) {
	for (int i = 0; i < KINC_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (strcmp(named_numbers[i].name, name) == 0) {
			return true;
		}
	}
	return false;
}

static uint32_t find_number(kinc_internal_named_number *named_numbers, const char *name) {
	for (int i = 0; i < KINC_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (strcmp(named_numbers[i].name, name) == 0) {
			return named_numbers[i].number;
		}
	}
	return -1;
}

static void set_number(kinc_internal_named_number *named_numbers, const char *name, uint32_t number) {
	for (int i = 0; i < KINC_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (strcmp(named_numbers[i].name, name) == 0) {
			named_numbers[i].number = number;
			return;
		}
	}

	for (int i = 0; i < KINC_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (named_numbers[i].name[0] == 0) {
			strcpy(named_numbers[i].name, name);
			named_numbers[i].number = number;
			return;
		}
	}

	assert(false);
}

struct indexed_name {
	uint32_t id;
	char *name;
};

struct indexed_index {
	uint32_t id;
	uint32_t value;
};

#define MAX_THINGS 256
static struct indexed_name names[MAX_THINGS];
static uint32_t names_size = 0;
static struct indexed_name memberNames[MAX_THINGS];
static uint32_t memberNames_size = 0;
static struct indexed_index locs[MAX_THINGS];
static uint32_t locs_size = 0;
static struct indexed_index bindings[MAX_THINGS];
static uint32_t bindings_size = 0;
static struct indexed_index offsets[MAX_THINGS];
static uint32_t offsets_size = 0;

static void add_name(uint32_t id, char *name) {
	names[names_size].id = id;
	names[names_size].name = name;
	++names_size;
}

static char *find_name(uint32_t id) {
	for (uint32_t i = 0; i < names_size; ++i) {
		if (names[i].id == id) {
			return names[i].name;
		}
	}
	return NULL;
}

static void add_member_name(uint32_t id, char *name) {
	memberNames[memberNames_size].id = id;
	memberNames[memberNames_size].name = name;
	++memberNames_size;
}

static char *find_member_name(uint32_t id) {
	for (uint32_t i = 0; i < memberNames_size; ++i) {
		if (memberNames[i].id == id) {
			return memberNames[i].name;
		}
	}
	return NULL;
}

static void add_location(uint32_t id, uint32_t location) {
	locs[locs_size].id = id;
	locs[locs_size].value = location;
	++locs_size;
}

static void add_binding(uint32_t id, uint32_t binding) {
	bindings[bindings_size].id = id;
	bindings[bindings_size].value = binding;
	++bindings_size;
}

static void add_offset(uint32_t id, uint32_t offset) {
	offsets[offsets_size].id = id;
	offsets[offsets_size].value = offset;
	++offsets_size;
}

static void parse_shader(uint32_t *shader_source, int shader_length, kinc_internal_named_number *locations, kinc_internal_named_number *textureBindings,
                         kinc_internal_named_number *uniformOffsets) {
	names_size = 0;
	memberNames_size = 0;
	locs_size = 0;
	bindings_size = 0;
	offsets_size = 0;

	uint32_t *spirv = (uint32_t *)shader_source;
	int spirvsize = shader_length / 4;
	int index = 0;

	uint32_t magicNumber = spirv[index++];
	uint32_t version = spirv[index++];
	uint32_t generator = spirv[index++];
	uint32_t bound = spirv[index++];
	index++;

	while (index < spirvsize) {
		int wordCount = spirv[index] >> 16;
		uint32_t opcode = spirv[index] & 0xffff;

		uint32_t *operands = wordCount > 1 ? &spirv[index + 1] : NULL;
		uint32_t length = wordCount - 1;

		switch (opcode) {
		case 5: { // OpName
			uint32_t id = operands[0];
			char *string = (char *)&operands[1];
			add_name(id, string);
			break;
		}
		case 6: { // OpMemberName
			uint32_t type = operands[0];
			char *name = find_name(type);
			if (name != NULL && strcmp(name, "_k_global_uniform_buffer_type") == 0) {
				uint32_t member = operands[1];
				char *string = (char *)&operands[2];
				add_member_name(member, string);
			}
			break;
		}
		case 71: { // OpDecorate
			uint32_t id = operands[0];
			uint32_t decoration = operands[1];
			if (decoration == 30) { // location
				uint32_t location = operands[2];
				add_location(id, location);
			}
			if (decoration == 33) { // binding
				uint32_t binding = operands[2];
				add_binding(id, binding);
			}
			break;
		}
		case 72: { // OpMemberDecorate
			uint32_t type = operands[0];
			char *name = find_name(type);
			if (name != NULL && strcmp(name, "_k_global_uniform_buffer_type") == 0) {
				uint32_t member = operands[1];
				uint32_t decoration = operands[2];
				if (decoration == 35) { // offset
					uint32_t offset = operands[3];
					add_offset(member, offset);
				}
			}
			break;
		}
		}

		index += wordCount;
	}

	for (uint32_t i = 0; i < locs_size; ++i) {
		char *name = find_name(locs[i].id);
		if (name != NULL) {
			set_number(locations, name, locs[i].value);
		}
	}

	for (uint32_t i = 0; i < bindings_size; ++i) {
		char *name = find_name(bindings[i].id);
		if (name != NULL) {
			set_number(textureBindings, name, bindings[i].value);
		}
	}

	for (uint32_t i = 0; i < offsets_size; ++i) {
		char *name = find_member_name(offsets[i].id);
		if (name != NULL) {
			set_number(uniformOffsets, name, offsets[i].value);
		}
	}
}

static VkShaderModule create_shader_module(const void *code, size_t size) {
	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.codeSize = size;
	moduleCreateInfo.pCode = (const uint32_t *)code;
	moduleCreateInfo.flags = 0;

	VkShaderModule module;
	VkResult err = vkCreateShaderModule(vk_ctx.device, &moduleCreateInfo, NULL, &module);
	assert(!err);

	return module;
}

static VkShaderModule prepare_vs(VkShaderModule *vert_shader_module, kinc_g5_shader_t *vertexShader) {
	*vert_shader_module = create_shader_module(vertexShader->impl.source, vertexShader->impl.length);
	return *vert_shader_module;
}

static VkShaderModule prepare_fs(VkShaderModule *frag_shader_module, kinc_g5_shader_t *fragmentShader) {
	*frag_shader_module = create_shader_module(fragmentShader->impl.source, fragmentShader->impl.length);
	return *frag_shader_module;
}

static VkFormat convert_format(kinc_g5_render_target_format_t format) {
	switch (format) {
	case KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return VK_FORMAT_R16_SFLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED:
		return VK_FORMAT_R8_UNORM;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT:
	default:
		return VK_FORMAT_B8G8R8A8_UNORM;
	}
}

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline) {
	kinc_g5_internal_pipeline_init(pipeline);
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline) {
	vkDestroyPipeline(vk_ctx.device, pipeline->impl.framebuffer_pipeline, NULL);
	vkDestroyPipeline(vk_ctx.device, pipeline->impl.rendertarget_pipeline, NULL);
	vkDestroyPipelineLayout(vk_ctx.device, pipeline->impl.pipeline_layout, NULL);
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name) {
	kinc_g5_constant_location_t location;
	location.impl.vertexOffset = -1;
	location.impl.fragmentOffset = -1;
	location.impl.computeOffset = -1;
	if (has_number(pipeline->impl.vertexOffsets, name)) {
		location.impl.vertexOffset = find_number(pipeline->impl.vertexOffsets, name);
	}
	if (has_number(pipeline->impl.fragmentOffsets, name)) {
		location.impl.fragmentOffset = find_number(pipeline->impl.fragmentOffsets, name);
	}
	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name) {
	kinc_g5_texture_unit_t unit;
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	int number = find_number(pipeline->impl.vertexTextureBindings, name);
	assert(number == -1 || number >= 2); // something wrong with the SPIR-V when this triggers

	if (number >= 0) {
		unit.stages[KINC_G5_SHADER_TYPE_VERTEX] = number - 2;
	}
	else {
		number = find_number(pipeline->impl.fragmentTextureBindings, name);
		if (number >= 0) {
			unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] = number - 2;
		}
	}

	return unit;
}

static VkCullModeFlagBits convert_cull_mode(kinc_g5_cull_mode_t cullMode) {
	switch (cullMode) {
	case KINC_G5_CULL_MODE_CLOCKWISE:
		return VK_CULL_MODE_BACK_BIT;
	case KINC_G5_CULL_MODE_COUNTERCLOCKWISE:
		return VK_CULL_MODE_FRONT_BIT;
	case KINC_G5_CULL_MODE_NEVER:
	default:
		return VK_CULL_MODE_NONE;
	}
}

static VkCompareOp convert_compare_mode(kinc_g5_compare_mode_t compare) {
	switch (compare) {
	default:
	case KINC_G5_COMPARE_MODE_ALWAYS:
		return VK_COMPARE_OP_ALWAYS;
	case KINC_G5_COMPARE_MODE_NEVER:
		return VK_COMPARE_OP_NEVER;
	case KINC_G5_COMPARE_MODE_EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case KINC_G5_COMPARE_MODE_NOT_EQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case KINC_G5_COMPARE_MODE_LESS:
		return VK_COMPARE_OP_LESS;
	case KINC_G5_COMPARE_MODE_LESS_EQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case KINC_G5_COMPARE_MODE_GREATER:
		return VK_COMPARE_OP_GREATER;
	case KINC_G5_COMPARE_MODE_GREATER_EQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	}
}

static VkBlendFactor convert_blend_factor(kinc_g5_blending_factor_t factor) {
	switch (factor) {
	case KINC_G5_BLEND_ONE:
		return VK_BLEND_FACTOR_ONE;
	case KINC_G5_BLEND_ZERO:
		return VK_BLEND_FACTOR_ZERO;
	case KINC_G5_BLEND_SOURCE_ALPHA:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case KINC_G5_BLEND_DEST_ALPHA:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case KINC_G5_BLEND_INV_SOURCE_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case KINC_G5_BLEND_INV_DEST_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case KINC_G5_BLEND_SOURCE_COLOR:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case KINC_G5_BLEND_DEST_COLOR:
		return VK_BLEND_FACTOR_DST_COLOR;
	case KINC_G5_BLEND_INV_SOURCE_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case KINC_G5_BLEND_INV_DEST_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case KINC_G5_BLEND_CONSTANT:
		return VK_BLEND_FACTOR_CONSTANT_COLOR;
	case KINC_G5_BLEND_INV_CONSTANT:
		return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	default:
		assert(false);
		return VK_BLEND_FACTOR_ONE;
	}
}

static VkBlendOp convert_blend_operation(kinc_g5_blending_operation_t op) {
	switch (op) {
	case KINC_G5_BLENDOP_ADD:
		return VK_BLEND_OP_ADD;
	case KINC_G5_BLENDOP_SUBTRACT:
		return VK_BLEND_OP_SUBTRACT;
	case KINC_G5_BLENDOP_REVERSE_SUBTRACT:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	case KINC_G5_BLENDOP_MIN:
		return VK_BLEND_OP_MIN;
	case KINC_G5_BLENDOP_MAX:
		return VK_BLEND_OP_MAX;
	default:
		assert(false);
		return VK_BLEND_OP_ADD;
	}
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline) {
	memset(pipeline->impl.vertexLocations, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.vertexOffsets, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentLocations, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentOffsets, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.vertexTextureBindings, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentTextureBindings, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	parse_shader((uint32_t *)pipeline->vertexShader->impl.source, pipeline->vertexShader->impl.length, pipeline->impl.vertexLocations,
	             pipeline->impl.vertexTextureBindings, pipeline->impl.vertexOffsets);
	parse_shader((uint32_t *)pipeline->fragmentShader->impl.source, pipeline->fragmentShader->impl.length, pipeline->impl.fragmentLocations,
	             pipeline->impl.fragmentTextureBindings, pipeline->impl.fragmentOffsets);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {0};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = &desc_layout;

	VkResult err = vkCreatePipelineLayout(vk_ctx.device, &pPipelineLayoutCreateInfo, NULL, &pipeline->impl.pipeline_layout);
	assert(!err);

	VkGraphicsPipelineCreateInfo pipeline_info = {0};

	VkPipelineInputAssemblyStateCreateInfo ia = {0};
	VkPipelineRasterizationStateCreateInfo rs = {0};
	VkPipelineColorBlendStateCreateInfo cb = {0};
	VkPipelineDepthStencilStateCreateInfo ds = {0};
	VkPipelineViewportStateCreateInfo vp = {0};
	VkPipelineMultisampleStateCreateInfo ms = {0};
	VkDynamicState dynamicStateEnables[2];
	VkPipelineDynamicStateCreateInfo dynamicState = {0};

	memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
	memset(&dynamicState, 0, sizeof(dynamicState));
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables;

	memset(&pipeline_info, 0, sizeof(pipeline_info));
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.layout = pipeline->impl.pipeline_layout;

	VkVertexInputBindingDescription vi_bindings[1];
	int vertexAttributeCount = pipeline->inputLayout->size;
	VkVertexInputAttributeDescription vi_attrs[vertexAttributeCount];

	VkPipelineVertexInputStateCreateInfo vi = {0};
	memset(&vi, 0, sizeof(vi));
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.pNext = NULL;
	vi.vertexBindingDescriptionCount = 1;
	vi.pVertexBindingDescriptions = vi_bindings;
	vi.vertexAttributeDescriptionCount = vertexAttributeCount;
	vi.pVertexAttributeDescriptions = vi_attrs;

	uint32_t attr = 0;
	uint32_t offset = 0;
	for (int i = 0; i < pipeline->inputLayout->size; ++i) {
		kinc_g5_vertex_element_t element = pipeline->inputLayout->elements[i];

		vi_attrs[attr].binding = 0;
		vi_attrs[attr].location = find_number(pipeline->impl.vertexLocations, element.name);
		vi_attrs[attr].offset = offset;
		offset += kinc_g4_vertex_data_size(element.data);

		switch (element.data) {
		case KINC_G4_VERTEX_DATA_F32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_SFLOAT;
			break;
		case KINC_G4_VERTEX_DATA_F32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_SFLOAT;
			break;
		case KINC_G4_VERTEX_DATA_F32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case KINC_G4_VERTEX_DATA_F32_4X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case KINC_G4_VERTEX_DATA_I8_1X:
			vi_attrs[attr].format = VK_FORMAT_R8_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U8_1X:
			vi_attrs[attr].format = VK_FORMAT_R8_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I8_2X:
			vi_attrs[attr].format = VK_FORMAT_R8G8_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U8_2X:
			vi_attrs[attr].format = VK_FORMAT_R8G8_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I8_4X:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U8_4X:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I16_1X:
			vi_attrs[attr].format = VK_FORMAT_R16_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U16_1X:
			vi_attrs[attr].format = VK_FORMAT_R16_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I16_2X:
			vi_attrs[attr].format = VK_FORMAT_R16G16_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U16_2X:
			vi_attrs[attr].format = VK_FORMAT_R16G16_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I16_4X:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U16_4X:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I32_4X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_4X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_UINT;
			break;
		default:
			assert(false);
			break;
		}
		attr++;
	}
	vi_bindings[0].binding = 0;
	vi_bindings[0].stride = offset;
	vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	memset(&ia, 0, sizeof(ia));
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	memset(&rs, 0, sizeof(rs));
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = convert_cull_mode(pipeline->cullMode);
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.depthClampEnable = VK_FALSE;
	rs.rasterizerDiscardEnable = VK_FALSE;
	rs.depthBiasEnable = VK_FALSE;
	rs.lineWidth = 1.0f;

	memset(&cb, 0, sizeof(cb));
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState att_state[8];
	memset(att_state, 0, sizeof(att_state));
	for (int i = 0; i < pipeline->colorAttachmentCount; ++i) {
		att_state[i].colorWriteMask =
		    (pipeline->colorWriteMaskRed[i] ? VK_COLOR_COMPONENT_R_BIT : 0) | (pipeline->colorWriteMaskGreen[i] ? VK_COLOR_COMPONENT_G_BIT : 0) |
		    (pipeline->colorWriteMaskBlue[i] ? VK_COLOR_COMPONENT_B_BIT : 0) | (pipeline->colorWriteMaskAlpha[i] ? VK_COLOR_COMPONENT_A_BIT : 0);
		att_state[i].blendEnable = pipeline->blend_source != KINC_G5_BLEND_ONE || pipeline->blend_destination != KINC_G5_BLEND_ZERO ||
		                           pipeline->alpha_blend_source != KINC_G5_BLEND_ONE || pipeline->alpha_blend_destination != KINC_G5_BLEND_ZERO;
		att_state[i].srcColorBlendFactor = convert_blend_factor(pipeline->blend_source);
		att_state[i].dstColorBlendFactor = convert_blend_factor(pipeline->blend_destination);
		att_state[i].colorBlendOp = convert_blend_operation(pipeline->blend_operation);
		att_state[i].srcAlphaBlendFactor = convert_blend_factor(pipeline->alpha_blend_source);
		att_state[i].dstAlphaBlendFactor = convert_blend_factor(pipeline->alpha_blend_destination);
		att_state[i].alphaBlendOp = convert_blend_operation(pipeline->alpha_blend_operation);
	}
	cb.attachmentCount = pipeline->colorAttachmentCount;
	cb.pAttachments = att_state;

	memset(&vp, 0, sizeof(vp));
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount = 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	memset(&ds, 0, sizeof(ds));
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = pipeline->depthMode != KINC_G5_COMPARE_MODE_ALWAYS;
	ds.depthWriteEnable = pipeline->depthWrite;
	ds.depthCompareOp = convert_compare_mode(pipeline->depthMode);
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.back.failOp = VK_STENCIL_OP_KEEP;
	ds.back.passOp = VK_STENCIL_OP_KEEP;
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
	ds.stencilTestEnable = VK_FALSE;
	ds.front = ds.back;

	memset(&ms, 0, sizeof(ms));
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pSampleMask = NULL;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	pipeline_info.stageCount = 2;
	VkPipelineShaderStageCreateInfo shaderStages[2];
	memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = prepare_vs(&pipeline->impl.vert_shader_module, pipeline->vertexShader);
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = prepare_fs(&pipeline->impl.frag_shader_module, pipeline->fragmentShader);
	shaderStages[1].pName = "main";

	pipeline_info.pVertexInputState = &vi;
	pipeline_info.pInputAssemblyState = &ia;
	pipeline_info.pRasterizationState = &rs;
	pipeline_info.pColorBlendState = &cb;
	pipeline_info.pMultisampleState = &ms;
	pipeline_info.pViewportState = &vp;
	pipeline_info.pDepthStencilState = &ds;
	pipeline_info.pStages = shaderStages;
	pipeline_info.renderPass = vk_ctx.windows[vk_ctx.current_window].framebuffer_render_pass;
	pipeline_info.pDynamicState = &dynamicState;

	err = vkCreateGraphicsPipelines(vk_ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->impl.framebuffer_pipeline);
	assert(!err);

	VkAttachmentDescription attachments[9];
	for (int i = 0; i < pipeline->colorAttachmentCount; ++i) {
		attachments[i].format = convert_format(pipeline->colorAttachment[i]);
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachments[i].flags = 0;
	}

	if (pipeline->depthAttachmentBits > 0) {
		attachments[pipeline->colorAttachmentCount].format = VK_FORMAT_D16_UNORM;
		attachments[pipeline->colorAttachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[pipeline->colorAttachmentCount].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[pipeline->colorAttachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[pipeline->colorAttachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[pipeline->colorAttachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[pipeline->colorAttachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[pipeline->colorAttachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[pipeline->colorAttachmentCount].flags = 0;
	}

	VkAttachmentReference color_references[8];
	for (int i = 0; i < pipeline->colorAttachmentCount; ++i) {
		color_references[i].attachment = i;
		color_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentReference depth_reference = {0};
	depth_reference.attachment = pipeline->colorAttachmentCount;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = pipeline->colorAttachmentCount;
	subpass.pColorAttachments = color_references;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = pipeline->depthAttachmentBits > 0 ? &depth_reference : NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkSubpassDependency dependencies[2];
	memset(&dependencies, 0, sizeof(dependencies));

	// TODO: For multi-targets-rendering
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = pipeline->depthAttachmentBits > 0 ? pipeline->colorAttachmentCount + 1 : pipeline->colorAttachmentCount;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 2;
	rp_info.pDependencies = dependencies;

	VkRenderPass render_pass;
	err = vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &render_pass);
	assert(!err);

	pipeline_info.renderPass = render_pass;

	err = vkCreateGraphicsPipelines(vk_ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->impl.rendertarget_pipeline);
	assert(!err);

	vkDestroyShaderModule(vk_ctx.device, pipeline->impl.frag_shader_module, NULL);
	vkDestroyShaderModule(vk_ctx.device, pipeline->impl.vert_shader_module, NULL);
}

void createDescriptorLayout(void) {
	VkDescriptorSetLayoutBinding layoutBindings[18];
	memset(layoutBindings, 0, sizeof(layoutBindings));

	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = NULL;

	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[1].pImmutableSamplers = NULL;

	for (int i = 2; i < 18; ++i) {
		layoutBindings[i].binding = i;
		layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[i].descriptorCount = 1;
		layoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[i].pImmutableSamplers = NULL;
	}

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {0};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 18;
	descriptor_layout.pBindings = layoutBindings;

	VkResult err = vkCreateDescriptorSetLayout(vk_ctx.device, &descriptor_layout, NULL, &desc_layout);
	assert(!err);

	VkDescriptorPoolSize typeCounts[2];
	memset(typeCounts, 0, sizeof(typeCounts));

	typeCounts[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	typeCounts[0].descriptorCount = 2 * 1024;

	typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	typeCounts[1].descriptorCount = 16 * 1024;

	VkDescriptorPoolCreateInfo pool_info = {0};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.pNext = NULL;
	pool_info.maxSets = 1024;
	pool_info.poolSizeCount = 2;
	pool_info.pPoolSizes = typeCounts;

	err = vkCreateDescriptorPool(vk_ctx.device, &pool_info, NULL, &descriptor_pool);
	assert(!err);
}

int calc_descriptor_id(void) {
	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkanTextures[i] != NULL) {
			texture_count++;
		}
		else if (vulkanRenderTargets[i] != NULL) {
			texture_count++;
		}
	}

	bool uniform_buffer = vk_ctx.vertex_uniform_buffer != NULL && vk_ctx.fragment_uniform_buffer != NULL;

	return 1 | (texture_count << 1) | ((uniform_buffer ? 1 : 0) << 8);
}

int calc_compute_descriptor_id(void) {
	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkanTextures[i] != NULL) {
			texture_count++;
		}
		else if (vulkanRenderTargets[i] != NULL) {
			texture_count++;
		}
	}

	bool uniform_buffer = vk_ctx.compute_uniform_buffer != NULL;

	return 1 | (texture_count << 1) | ((uniform_buffer ? 1 : 0) << 8);
}

#define MAX_DESCRIPTOR_SETS 1024

struct destriptor_set {
	int id;
	bool in_use;
	VkDescriptorImageInfo tex_desc[16];
	VkDescriptorSet set;
};

static struct destriptor_set descriptor_sets[MAX_DESCRIPTOR_SETS] = {0};
static int descriptor_sets_count = 0;

static int write_tex_descs(VkDescriptorImageInfo *tex_descs) {
	memset(tex_descs, 0, sizeof(VkDescriptorImageInfo) * 16);

	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkanTextures[i] != NULL) {
			tex_descs[i].sampler = vulkanSamplers[i];
			tex_descs[i].imageView = vulkanTextures[i]->impl.texture.view;
			texture_count++;
		}
		else if (vulkanRenderTargets[i] != NULL) {
			tex_descs[i].sampler = vulkanSamplers[i];
			if (vulkanRenderTargets[i]->impl.stage_depth == i) {
				tex_descs[i].imageView = vulkanRenderTargets[i]->impl.depthView;
				vulkanRenderTargets[i]->impl.stage_depth = -1;
			}
			else {
				tex_descs[i].imageView = vulkanRenderTargets[i]->impl.sourceView;
			}
			texture_count++;
		}
		tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	return texture_count;
}

static bool textures_changed(struct destriptor_set *set) {
	VkDescriptorImageInfo tex_desc[16];

	write_tex_descs(tex_desc);

	return memcmp(&tex_desc, &set->tex_desc, sizeof(tex_desc)) != 0;
}

static void update_textures(struct destriptor_set *set) {
	memset(&set->tex_desc, 0, sizeof(set->tex_desc));

	int texture_count = write_tex_descs(set->tex_desc);

	VkWriteDescriptorSet writes[16];
	memset(&writes, 0, sizeof(writes));

	for (int i = 0; i < 16; ++i) {
		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set->set;
		writes[i].dstBinding = i + 2;
		writes[i].descriptorCount = 1;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[i].pImageInfo = &set->tex_desc[i];
	}

	if (vulkanTextures[0] != NULL || vulkanRenderTargets[0] != NULL) {
		vkUpdateDescriptorSets(vk_ctx.device, texture_count, writes, 0, NULL);
	}
}

void reuse_descriptor_sets(void) {
	for (int i = 0; i < descriptor_sets_count; ++i) {
		descriptor_sets[i].in_use = false;
	}
}

VkDescriptorSet getDescriptorSet() {
	int id = calc_descriptor_id();
	for (int i = 0; i < descriptor_sets_count; ++i) {
		if (descriptor_sets[i].id == id) {
			if (!descriptor_sets[i].in_use) {
				descriptor_sets[i].in_use = true;
				update_textures(&descriptor_sets[i]);
				return descriptor_sets[i].set;
			}
			else {
				// if (!textures_changed(&descriptor_sets[i])) {
				// 	return descriptor_sets[i].set;
				// }
			}
		}
	}

	VkDescriptorSetAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &desc_layout;
	VkDescriptorSet descriptor_set;
	VkResult err = vkAllocateDescriptorSets(vk_ctx.device, &alloc_info, &descriptor_set);
	assert(!err);

	VkDescriptorBufferInfo buffer_descs[2];

	memset(&buffer_descs, 0, sizeof(buffer_descs));

	if (vk_ctx.vertex_uniform_buffer != NULL) {
		buffer_descs[0].buffer = *vk_ctx.vertex_uniform_buffer;
	}
	buffer_descs[0].offset = 0;
	buffer_descs[0].range = 256 * sizeof(float);

	if (vk_ctx.fragment_uniform_buffer != NULL) {
		buffer_descs[1].buffer = *vk_ctx.fragment_uniform_buffer;
	}
	buffer_descs[1].offset = 0;
	buffer_descs[1].range = 256 * sizeof(float);

	VkDescriptorImageInfo tex_desc[16];
	memset(&tex_desc, 0, sizeof(tex_desc));

	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkanTextures[i] != NULL) {
			assert(vulkanSamplers[i] != VK_NULL_HANDLE);
			tex_desc[i].sampler = vulkanSamplers[i];
			tex_desc[i].imageView = vulkanTextures[i]->impl.texture.view;
			texture_count++;
		}
		else if (vulkanRenderTargets[i] != NULL) {
			tex_desc[i].sampler = vulkanSamplers[i];
			if (vulkanRenderTargets[i]->impl.stage_depth == i) {
				tex_desc[i].imageView = vulkanRenderTargets[i]->impl.depthView;
				vulkanRenderTargets[i]->impl.stage_depth = -1;
			}
			else {
				tex_desc[i].imageView = vulkanRenderTargets[i]->impl.sourceView;
			}
			texture_count++;
		}
		tex_desc[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}

	VkWriteDescriptorSet writes[18];
	memset(&writes, 0, sizeof(writes));

	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = descriptor_set;
	writes[0].dstBinding = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	writes[0].pBufferInfo = &buffer_descs[0];

	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet = descriptor_set;
	writes[1].dstBinding = 1;
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	writes[1].pBufferInfo = &buffer_descs[1];

	for (int i = 2; i < 18; ++i) {
		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = descriptor_set;
		writes[i].dstBinding = i;
		writes[i].descriptorCount = 1;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[i].pImageInfo = &tex_desc[i - 2];
	}

	if (vulkanTextures[0] != NULL || vulkanRenderTargets[0] != NULL) {
		if (vk_ctx.vertex_uniform_buffer != NULL && vk_ctx.fragment_uniform_buffer != NULL) {
			vkUpdateDescriptorSets(vk_ctx.device, 2 + texture_count, writes, 0, NULL);
		}
		else {
			vkUpdateDescriptorSets(vk_ctx.device, texture_count, writes + 2, 0, NULL);
		}
	}
	else {
		if (vk_ctx.vertex_uniform_buffer != NULL && vk_ctx.fragment_uniform_buffer != NULL) {
			vkUpdateDescriptorSets(vk_ctx.device, 2, writes, 0, NULL);
		}
	}

	assert(descriptor_sets_count + 1 < MAX_DESCRIPTOR_SETS);
	descriptor_sets[descriptor_sets_count].id = id;
	descriptor_sets[descriptor_sets_count].in_use = true;
	descriptor_sets[descriptor_sets_count].set = descriptor_set;
	write_tex_descs(descriptor_sets[descriptor_sets_count].tex_desc);
	descriptor_sets_count += 1;

	return descriptor_set;
}

static struct destriptor_set compute_descriptor_sets[MAX_DESCRIPTOR_SETS] = {0};
static int compute_descriptor_sets_count = 0;

static int write_compute_tex_descs(VkDescriptorImageInfo *tex_descs) {
	memset(tex_descs, 0, sizeof(VkDescriptorImageInfo) * 16);

	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkanTextures[i] != NULL) {
			tex_descs[i].sampler = vulkanSamplers[i];
			tex_descs[i].imageView = vulkanTextures[i]->impl.texture.view;
			texture_count++;
		}
		else if (vulkanRenderTargets[i] != NULL) {
			tex_descs[i].sampler = vulkanSamplers[i];
			if (vulkanRenderTargets[i]->impl.stage_depth == i) {
				tex_descs[i].imageView = vulkanRenderTargets[i]->impl.depthView;
				vulkanRenderTargets[i]->impl.stage_depth = -1;
			}
			else {
				tex_descs[i].imageView = vulkanRenderTargets[i]->impl.sourceView;
			}
			texture_count++;
		}
		tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	return texture_count;
}

static bool compute_textures_changed(struct destriptor_set *set) {
	VkDescriptorImageInfo tex_desc[16];

	write_compute_tex_descs(tex_desc);

	return memcmp(&tex_desc, &set->tex_desc, sizeof(tex_desc)) != 0;
}

static void update_compute_textures(struct destriptor_set *set) {
	memset(&set->tex_desc, 0, sizeof(set->tex_desc));

	int texture_count = write_compute_tex_descs(set->tex_desc);

	VkWriteDescriptorSet writes[16];
	memset(&writes, 0, sizeof(writes));

	for (int i = 0; i < 16; ++i) {
		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set->set;
		writes[i].dstBinding = i + 2;
		writes[i].descriptorCount = 1;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writes[i].pImageInfo = &set->tex_desc[i];
	}

	if (vulkanTextures[0] != NULL || vulkanRenderTargets[0] != NULL) {
		vkUpdateDescriptorSets(vk_ctx.device, texture_count, writes, 0, NULL);
	}
}

void reuse_compute_descriptor_sets(void) {
	for (int i = 0; i < compute_descriptor_sets_count; ++i) {
		compute_descriptor_sets[i].in_use = false;
	}
}

static VkDescriptorSet get_compute_descriptor_set() {
	int id = calc_compute_descriptor_id();
	for (int i = 0; i < compute_descriptor_sets_count; ++i) {
		if (compute_descriptor_sets[i].id == id) {
			if (!compute_descriptor_sets[i].in_use) {
				compute_descriptor_sets[i].in_use = true;
				update_compute_textures(&compute_descriptor_sets[i]);
				return compute_descriptor_sets[i].set;
			}
			else {
				if (!compute_textures_changed(&compute_descriptor_sets[i])) {
					return compute_descriptor_sets[i].set;
				}
			}
		}
	}

	VkDescriptorSetAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &compute_descriptor_layout;
	VkDescriptorSet descriptor_set;
	VkResult err = vkAllocateDescriptorSets(vk_ctx.device, &alloc_info, &descriptor_set);
	assert(!err);

	VkDescriptorBufferInfo buffer_descs[2];

	memset(&buffer_descs, 0, sizeof(buffer_descs));

	if (vk_ctx.compute_uniform_buffer != NULL) {
		buffer_descs[0].buffer = *vk_ctx.compute_uniform_buffer;
	}
	buffer_descs[0].offset = 0;
	buffer_descs[0].range = 256 * sizeof(float);

	if (vk_ctx.compute_uniform_buffer != NULL) {
		buffer_descs[1].buffer = *vk_ctx.compute_uniform_buffer;
	}
	buffer_descs[1].offset = 0;
	buffer_descs[1].range = 256 * sizeof(float);

	VkDescriptorImageInfo tex_desc[16];
	memset(&tex_desc, 0, sizeof(tex_desc));

	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkanTextures[i] != NULL) {
			// assert(vulkanSamplers[i] != VK_NULL_HANDLE);
			tex_desc[i].sampler = VK_NULL_HANDLE; // vulkanSamplers[i];
			tex_desc[i].imageView = vulkanTextures[i]->impl.texture.view;
			texture_count++;
		}
		else if (vulkanRenderTargets[i] != NULL) {
			tex_desc[i].sampler = vulkanSamplers[i];
			if (vulkanRenderTargets[i]->impl.stage_depth == i) {
				tex_desc[i].imageView = vulkanRenderTargets[i]->impl.depthView;
				vulkanRenderTargets[i]->impl.stage_depth = -1;
			}
			else {
				tex_desc[i].imageView = vulkanRenderTargets[i]->impl.sourceView;
			}
			texture_count++;
		}
		tex_desc[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}

	VkWriteDescriptorSet writes[18];
	memset(&writes, 0, sizeof(writes));

	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = descriptor_set;
	writes[0].dstBinding = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	writes[0].pBufferInfo = &buffer_descs[0];

	writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet = descriptor_set;
	writes[1].dstBinding = 1;
	writes[1].descriptorCount = 1;
	writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	writes[1].pBufferInfo = &buffer_descs[1];

	for (int i = 2; i < 18; ++i) {
		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = descriptor_set;
		writes[i].dstBinding = i;
		writes[i].descriptorCount = 1;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writes[i].pImageInfo = &tex_desc[i - 2];
	}

	if (vulkanTextures[0] != NULL || vulkanRenderTargets[0] != NULL) {
		if (vk_ctx.compute_uniform_buffer != NULL) {
			vkUpdateDescriptorSets(vk_ctx.device, 2 + texture_count, writes, 0, NULL);
		}
		else {
			vkUpdateDescriptorSets(vk_ctx.device, texture_count, writes + 2, 0, NULL);
		}
	}
	else {
		if (vk_ctx.compute_uniform_buffer != NULL) {
			vkUpdateDescriptorSets(vk_ctx.device, 2, writes, 0, NULL);
		}
	}

	assert(compute_descriptor_sets_count + 1 < MAX_DESCRIPTOR_SETS);
	compute_descriptor_sets[compute_descriptor_sets_count].id = id;
	compute_descriptor_sets[compute_descriptor_sets_count].in_use = true;
	compute_descriptor_sets[compute_descriptor_sets_count].set = descriptor_set;
	write_tex_descs(compute_descriptor_sets[compute_descriptor_sets_count].tex_desc);
	compute_descriptor_sets_count += 1;

	return descriptor_set;
}
