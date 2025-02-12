#include "vulkan.h"
#include "raytrace.h"

#ifndef KINC_ANDROID

#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/raytrace.h>
#include <kinc/graphics5/vertexbuffer.h>

extern VkRenderPassBeginInfo currentRenderPassBeginInfo;
extern VkFramebuffer *framebuffers;
extern uint32_t current_buffer;
bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

static const int INDEX_RAYGEN = 0;
static const int INDEX_MISS = 1;
static const int INDEX_CLOSEST_HIT = 2;
static const char *raygen_shader_name = "raygeneration";
static const char *closesthit_shader_name = "closesthit";
static const char *miss_shader_name = "miss";

typedef struct inst {
	kinc_matrix4x4_t m;
	int i;
} inst_t;

static VkDescriptorPool raytrace_descriptor_pool;
static kinc_raytrace_acceleration_structure_t *accel;
static kinc_raytrace_pipeline_t *pipeline;
static kinc_g5_render_target_t *output = NULL;
static kinc_g5_render_target_t *texpaint0;
static kinc_g5_render_target_t *texpaint1;
static kinc_g5_render_target_t *texpaint2;
static kinc_g5_texture_t *texenv;
static kinc_g5_texture_t *texsobol;
static kinc_g5_texture_t *texscramble;
static kinc_g5_texture_t *texrank;
static kinc_g5_vertex_buffer_t *vb[16];
static kinc_g5_vertex_buffer_t *vb_last[16];
static kinc_g5_index_buffer_t *ib[16];
static int vb_count = 0;
static int vb_count_last = 0;
static inst_t instances[1024];
static int instances_count = 0;
static VkBuffer vb_full = VK_NULL_HANDLE;
static VkBuffer ib_full = VK_NULL_HANDLE;
static VkDeviceMemory vb_full_mem = VK_NULL_HANDLE;
static VkDeviceMemory ib_full_mem = VK_NULL_HANDLE;

static PFN_vkCreateRayTracingPipelinesKHR _vkCreateRayTracingPipelinesKHR = NULL;
static PFN_vkGetRayTracingShaderGroupHandlesKHR _vkGetRayTracingShaderGroupHandlesKHR = NULL;
static PFN_vkGetBufferDeviceAddressKHR _vkGetBufferDeviceAddressKHR = NULL;
static PFN_vkCreateAccelerationStructureKHR _vkCreateAccelerationStructureKHR = NULL;
static PFN_vkGetAccelerationStructureDeviceAddressKHR _vkGetAccelerationStructureDeviceAddressKHR = NULL;
static PFN_vkGetAccelerationStructureBuildSizesKHR _vkGetAccelerationStructureBuildSizesKHR = NULL;
static PFN_vkCmdBuildAccelerationStructuresKHR _vkCmdBuildAccelerationStructuresKHR = NULL;
static PFN_vkDestroyAccelerationStructureKHR _vkDestroyAccelerationStructureKHR = NULL;
static PFN_vkCmdTraceRaysKHR _vkCmdTraceRaysKHR = NULL;

void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, kinc_g5_command_list_t *command_list, void *ray_shader, int ray_shader_size,
                                 kinc_g5_constant_buffer_t *constant_buffer) {
	output = NULL;
	pipeline->_constant_buffer = constant_buffer;

	{
		VkDescriptorSetLayoutBinding acceleration_structure_layout_binding = {0};
		acceleration_structure_layout_binding.binding = 0;
		acceleration_structure_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		acceleration_structure_layout_binding.descriptorCount = 1;
		acceleration_structure_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding result_image_layout_binding = {0};
		result_image_layout_binding.binding = 10;
		result_image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		result_image_layout_binding.descriptorCount = 1;
		result_image_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding uniform_buffer_binding = {0};
		uniform_buffer_binding.binding = 11;
		uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_buffer_binding.descriptorCount = 1;
		uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding ib_binding = {0};
		ib_binding.binding = 1;
		ib_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		ib_binding.descriptorCount = 1;
		ib_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding vb_binding = {0};
		vb_binding.binding = 2;
		vb_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vb_binding.descriptorCount = 1;
		vb_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding tex0_binding = {0};
		tex0_binding.binding = 3;
		tex0_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		tex0_binding.descriptorCount = 1;
		tex0_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding tex1_binding = {0};
		tex1_binding.binding = 4;
		tex1_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		tex1_binding.descriptorCount = 1;
		tex1_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding tex2_binding = {0};
		tex2_binding.binding = 5;
		tex2_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		tex2_binding.descriptorCount = 1;
		tex2_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding texenv_binding = {0};
		texenv_binding.binding = 6;
		texenv_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texenv_binding.descriptorCount = 1;
		texenv_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding texsobol_binding = {0};
		texsobol_binding.binding = 7;
		texsobol_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texsobol_binding.descriptorCount = 1;
		texsobol_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding texscramble_binding = {0};
		texscramble_binding.binding = 8;
		texscramble_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texscramble_binding.descriptorCount = 1;
		texscramble_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding texrank_binding = {0};
		texrank_binding.binding = 9;
		texrank_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texrank_binding.descriptorCount = 1;
		texrank_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

		VkDescriptorSetLayoutBinding bindings[12] = {
			acceleration_structure_layout_binding,
			result_image_layout_binding,
			uniform_buffer_binding,
			vb_binding,
			ib_binding,
			tex0_binding,
			tex1_binding,
			tex2_binding,
			texenv_binding,
			texsobol_binding,
			texscramble_binding,
			texrank_binding
		};

		VkDescriptorSetLayoutCreateInfo layout_info = {0};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.pNext = NULL;
		layout_info.bindingCount = 12;
		layout_info.pBindings = &bindings[0];
		vkCreateDescriptorSetLayout(vk_ctx.device, &layout_info, NULL, &pipeline->impl.descriptor_set_layout);

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext = NULL;
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &pipeline->impl.descriptor_set_layout;

		vkCreatePipelineLayout(vk_ctx.device, &pipeline_layout_create_info, NULL, &pipeline->impl.pipeline_layout);

		VkShaderModuleCreateInfo module_create_info = {0};
		memset(&module_create_info, 0, sizeof(VkShaderModuleCreateInfo));
		module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		module_create_info.codeSize = ray_shader_size;
		module_create_info.pCode = (const uint32_t *)ray_shader;
		module_create_info.pNext = NULL;
		module_create_info.flags = 0;
		VkShaderModule shader_module;
		vkCreateShaderModule(vk_ctx.device, &module_create_info, NULL, &shader_module);

		VkPipelineShaderStageCreateInfo shader_stages[3];
		shader_stages[INDEX_RAYGEN].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[INDEX_RAYGEN].pNext = NULL;
		shader_stages[INDEX_RAYGEN].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		shader_stages[INDEX_RAYGEN].module = shader_module;
		shader_stages[INDEX_RAYGEN].pName = raygen_shader_name;
		shader_stages[INDEX_RAYGEN].flags = 0;
		shader_stages[INDEX_RAYGEN].pSpecializationInfo = NULL;

		shader_stages[INDEX_MISS].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[INDEX_MISS].pNext = NULL;
		shader_stages[INDEX_MISS].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		shader_stages[INDEX_MISS].module = shader_module;
		shader_stages[INDEX_MISS].pName = miss_shader_name;
		shader_stages[INDEX_MISS].flags = 0;
		shader_stages[INDEX_MISS].pSpecializationInfo = NULL;

		shader_stages[INDEX_CLOSEST_HIT].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[INDEX_CLOSEST_HIT].pNext = NULL;
		shader_stages[INDEX_CLOSEST_HIT].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		shader_stages[INDEX_CLOSEST_HIT].module = shader_module;
		shader_stages[INDEX_CLOSEST_HIT].pName = closesthit_shader_name;
		shader_stages[INDEX_CLOSEST_HIT].flags = 0;
		shader_stages[INDEX_CLOSEST_HIT].pSpecializationInfo = NULL;

		VkRayTracingShaderGroupCreateInfoKHR groups[3];
		groups[INDEX_RAYGEN].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		groups[INDEX_RAYGEN].pNext = NULL;
		groups[INDEX_RAYGEN].generalShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].closestHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].anyHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].intersectionShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_RAYGEN].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		groups[INDEX_RAYGEN].generalShader = INDEX_RAYGEN;

		groups[INDEX_MISS].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		groups[INDEX_MISS].pNext = NULL;
		groups[INDEX_MISS].generalShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].closestHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].anyHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].intersectionShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_MISS].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		groups[INDEX_MISS].generalShader = INDEX_MISS;

		groups[INDEX_CLOSEST_HIT].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		groups[INDEX_CLOSEST_HIT].pNext = NULL;
		groups[INDEX_CLOSEST_HIT].generalShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].closestHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].anyHitShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].intersectionShader = VK_SHADER_UNUSED_KHR;
		groups[INDEX_CLOSEST_HIT].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		groups[INDEX_CLOSEST_HIT].closestHitShader = INDEX_CLOSEST_HIT;

		VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_create_info = {0};
		raytracing_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		raytracing_pipeline_create_info.pNext = NULL;
		raytracing_pipeline_create_info.flags = 0;
		raytracing_pipeline_create_info.stageCount = 3;
		raytracing_pipeline_create_info.pStages = &shader_stages[0];
		raytracing_pipeline_create_info.groupCount = 3;
		raytracing_pipeline_create_info.pGroups = &groups[0];
		raytracing_pipeline_create_info.maxPipelineRayRecursionDepth = 1;
		raytracing_pipeline_create_info.layout = pipeline->impl.pipeline_layout;
		_vkCreateRayTracingPipelinesKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCreateRayTracingPipelinesKHR");
		_vkCreateRayTracingPipelinesKHR(vk_ctx.device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &raytracing_pipeline_create_info, NULL, &pipeline->impl.pipeline);
	}

	{
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_pipeline_properties;
		ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		ray_tracing_pipeline_properties.pNext = NULL;
		VkPhysicalDeviceProperties2 device_properties = {0};
		device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		device_properties.pNext = &ray_tracing_pipeline_properties;
		vkGetPhysicalDeviceProperties2(vk_ctx.gpu, &device_properties);

		_vkGetRayTracingShaderGroupHandlesKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetRayTracingShaderGroupHandlesKHR");
		uint32_t handle_size = ray_tracing_pipeline_properties.shaderGroupHandleSize;
		uint32_t handle_size_aligned =
		    (ray_tracing_pipeline_properties.shaderGroupHandleSize + ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1) &
		    ~(ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1);

		VkBufferCreateInfo buf_info = {0};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.pNext = NULL;
		buf_info.size = handle_size;
		buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.flags = 0;

		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &pipeline->impl.raygen_shader_binding_table);
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &pipeline->impl.hit_shader_binding_table);
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &pipeline->impl.miss_shader_binding_table);

		uint8_t shader_handle_storage[1024];
		_vkGetRayTracingShaderGroupHandlesKHR(vk_ctx.device, pipeline->impl.pipeline, 0, 3, handle_size_aligned * 3, shader_handle_storage);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memory_allocate_info = {0};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.pNext = &memory_allocate_flags_info;

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(vk_ctx.device, pipeline->impl.raygen_shader_binding_table, &mem_reqs);
		memory_allocate_info.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		                            &memory_allocate_info.memoryTypeIndex);

		VkDeviceMemory mem;
		void *data;
		vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &mem);
		vkBindBufferMemory(vk_ctx.device, pipeline->impl.raygen_shader_binding_table, mem, 0);
		vkMapMemory(vk_ctx.device, mem, 0, handle_size, 0, (void **)&data);
		memcpy(data, shader_handle_storage, handle_size);
		vkUnmapMemory(vk_ctx.device, mem);

		vkGetBufferMemoryRequirements(vk_ctx.device, pipeline->impl.miss_shader_binding_table, &mem_reqs);
		memory_allocate_info.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memory_allocate_info.memoryTypeIndex);

		vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &mem);
		vkBindBufferMemory(vk_ctx.device, pipeline->impl.miss_shader_binding_table, mem, 0);
		vkMapMemory(vk_ctx.device, mem, 0, handle_size, 0, (void **)&data);
		memcpy(data, shader_handle_storage + handle_size_aligned, handle_size);
		vkUnmapMemory(vk_ctx.device, mem);

		vkGetBufferMemoryRequirements(vk_ctx.device, pipeline->impl.hit_shader_binding_table, &mem_reqs);
		memory_allocate_info.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memory_allocate_info.memoryTypeIndex);

		vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &mem);
		vkBindBufferMemory(vk_ctx.device, pipeline->impl.hit_shader_binding_table, mem, 0);
		vkMapMemory(vk_ctx.device, mem, 0, handle_size, 0, (void **)&data);
		memcpy(data, shader_handle_storage + handle_size_aligned * 2, handle_size);
		vkUnmapMemory(vk_ctx.device, mem);
	}

	{
		VkDescriptorPoolSize type_counts[12];
		memset(type_counts, 0, sizeof(type_counts));

		type_counts[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		type_counts[0].descriptorCount = 1;

		type_counts[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		type_counts[1].descriptorCount = 1;

		type_counts[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		type_counts[2].descriptorCount = 1;

		type_counts[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		type_counts[3].descriptorCount = 1;

		type_counts[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		type_counts[4].descriptorCount = 1;

		type_counts[5].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		type_counts[5].descriptorCount = 1;

		type_counts[6].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		type_counts[6].descriptorCount = 1;

		type_counts[7].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		type_counts[7].descriptorCount = 1;

		type_counts[8].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		type_counts[8].descriptorCount = 1;

		type_counts[9].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		type_counts[9].descriptorCount = 1;

		type_counts[10].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		type_counts[10].descriptorCount = 1;

		type_counts[11].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		type_counts[11].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {0};
		descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.pNext = NULL;
		descriptor_pool_create_info.maxSets = 1024;
		descriptor_pool_create_info.poolSizeCount = 12;
		descriptor_pool_create_info.pPoolSizes = type_counts;

		vkCreateDescriptorPool(vk_ctx.device, &descriptor_pool_create_info, NULL, &raytrace_descriptor_pool);

		VkDescriptorSetAllocateInfo alloc_info = {0};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.pNext = NULL;
		alloc_info.descriptorPool = raytrace_descriptor_pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &pipeline->impl.descriptor_set_layout;
		vkAllocateDescriptorSets(vk_ctx.device, &alloc_info, &pipeline->impl.descriptor_set);
	}
}

void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline) {
	vkDestroyPipeline(vk_ctx.device, pipeline->impl.pipeline, NULL);
	vkDestroyPipelineLayout(vk_ctx.device, pipeline->impl.pipeline_layout, NULL);
	vkDestroyDescriptorSetLayout(vk_ctx.device, pipeline->impl.descriptor_set_layout, NULL);
}

uint64_t get_buffer_device_address(VkBuffer buffer) {
	VkBufferDeviceAddressInfoKHR buffer_device_address_info = {0};
	buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	buffer_device_address_info.buffer = buffer;
	_vkGetBufferDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetBufferDeviceAddressKHR");
	return _vkGetBufferDeviceAddressKHR(vk_ctx.device, &buffer_device_address_info);
}

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel) {
	_vkGetBufferDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetBufferDeviceAddressKHR");
	_vkCreateAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCreateAccelerationStructureKHR");
	_vkGetAccelerationStructureDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetAccelerationStructureDeviceAddressKHR");
	_vkGetAccelerationStructureBuildSizesKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetAccelerationStructureBuildSizesKHR");

	vb_count = 0;
	instances_count = 0;
}

void kinc_raytrace_acceleration_structure_add(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_vertex_buffer_t *_vb, kinc_g5_index_buffer_t *_ib,
	kinc_matrix4x4_t _transform) {

	int vb_i = -1;
	for (int i = 0; i < vb_count; ++i) {
		if (_vb == vb[i]) {
			vb_i = i;
			break;
		}
	}
	if (vb_i == -1) {
		vb_i = vb_count;
		vb[vb_count] = _vb;
		ib[vb_count] = _ib;
		vb_count++;
	}

	inst_t inst = { .i = vb_i, .m =  _transform };
	instances[instances_count] = inst;
	instances_count++;
}

void _kinc_raytrace_acceleration_structure_destroy_bottom(kinc_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkDestroyAccelerationStructureKHR");
	for (int i = 0; i < vb_count_last; ++i) {
		_vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.bottom_level_acceleration_structure[i], NULL);
		vkFreeMemory(vk_ctx.device, accel->impl.bottom_level_mem[i], NULL);
		vkDestroyBuffer(vk_ctx.device, accel->impl.bottom_level_buffer[i], NULL);
	}
}

void _kinc_raytrace_acceleration_structure_destroy_top(kinc_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkDestroyAccelerationStructureKHR");
	_vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.top_level_acceleration_structure, NULL);
	vkFreeMemory(vk_ctx.device, accel->impl.top_level_mem, NULL);
	vkDestroyBuffer(vk_ctx.device, accel->impl.top_level_buffer, NULL);
	vkFreeMemory(vk_ctx.device, accel->impl.instances_mem, NULL);
	vkDestroyBuffer(vk_ctx.device, accel->impl.instances_buffer, NULL);
}

void kinc_raytrace_acceleration_structure_build(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list,
	kinc_g5_vertex_buffer_t *_vb_full, kinc_g5_index_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_kinc_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_kinc_raytrace_acceleration_structure_destroy_top(accel);
	}

	vb_count_last = vb_count;

	if (vb_count == 0) {
		return;
	}

	// Bottom level
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {

			uint32_t prim_count = ib[i]->impl.count / 3;
			uint32_t vert_count = vb[i]->impl.myCount;

			VkDeviceOrHostAddressConstKHR vertex_data_device_address = {0};
			VkDeviceOrHostAddressConstKHR index_data_device_address = {0};

			vertex_data_device_address.deviceAddress = get_buffer_device_address(vb[i]->impl.vertices.buf);
			index_data_device_address.deviceAddress = get_buffer_device_address(ib[i]->impl.buf);

			VkAccelerationStructureGeometryKHR acceleration_geometry = {0};
			acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			acceleration_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			acceleration_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16A16_SNORM;
			acceleration_geometry.geometry.triangles.vertexData.deviceAddress = vertex_data_device_address.deviceAddress;
			acceleration_geometry.geometry.triangles.vertexStride = vb[i]->impl.myStride;
			acceleration_geometry.geometry.triangles.maxVertex = vb[i]->impl.myCount;
			acceleration_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			acceleration_geometry.geometry.triangles.indexData.deviceAddress = index_data_device_address.deviceAddress;

			VkAccelerationStructureBuildGeometryInfoKHR acceleration_structure_build_geometry_info = {0};
			acceleration_structure_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			acceleration_structure_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			acceleration_structure_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			acceleration_structure_build_geometry_info.geometryCount = 1;
			acceleration_structure_build_geometry_info.pGeometries = &acceleration_geometry;

			VkAccelerationStructureBuildSizesInfoKHR acceleration_build_sizes_info = {0};
			acceleration_build_sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
			_vkGetAccelerationStructureBuildSizesKHR(vk_ctx.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &acceleration_structure_build_geometry_info,
													&prim_count, &acceleration_build_sizes_info);

			VkBufferCreateInfo buffer_create_info = {0};
			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.size = acceleration_build_sizes_info.accelerationStructureSize;
			buffer_create_info.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VkBuffer bottom_level_buffer = VK_NULL_HANDLE;
			vkCreateBuffer(vk_ctx.device, &buffer_create_info, NULL, &bottom_level_buffer);

			VkMemoryRequirements memory_requirements2;
			vkGetBufferMemoryRequirements(vk_ctx.device, bottom_level_buffer, &memory_requirements2);

			VkMemoryAllocateFlagsInfo memory_allocate_flags_info2 = {0};
			memory_allocate_flags_info2.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			memory_allocate_flags_info2.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

			VkMemoryAllocateInfo memory_allocate_info = {0};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.pNext = &memory_allocate_flags_info2;
			memory_allocate_info.allocationSize = memory_requirements2.size;
			memory_type_from_properties(memory_requirements2.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
			VkDeviceMemory bottom_level_mem;
			vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &bottom_level_mem);
			vkBindBufferMemory(vk_ctx.device, bottom_level_buffer, bottom_level_mem, 0);

			VkAccelerationStructureCreateInfoKHR acceleration_create_info = {0};
			acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			acceleration_create_info.buffer = bottom_level_buffer;
			acceleration_create_info.size = acceleration_build_sizes_info.accelerationStructureSize;
			_vkCreateAccelerationStructureKHR(vk_ctx.device, &acceleration_create_info, NULL, &accel->impl.bottom_level_acceleration_structure[i]);

			VkBuffer scratch_buffer = VK_NULL_HANDLE;
			VkDeviceMemory scratch_memory = VK_NULL_HANDLE;

			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.size = acceleration_build_sizes_info.buildScratchSize;
			buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			vkCreateBuffer(vk_ctx.device, &buffer_create_info, NULL, &scratch_buffer);

			VkMemoryRequirements memory_requirements;
			vkGetBufferMemoryRequirements(vk_ctx.device, scratch_buffer, &memory_requirements);

			VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
			memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.pNext = &memory_allocate_flags_info;
			memory_allocate_info.allocationSize = memory_requirements.size;
			memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
			vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &scratch_memory);
			vkBindBufferMemory(vk_ctx.device, scratch_buffer, scratch_memory, 0);

			VkBufferDeviceAddressInfoKHR buffer_device_address_info = {0};
			buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			buffer_device_address_info.buffer = scratch_buffer;
			uint64_t scratch_buffer_device_address = _vkGetBufferDeviceAddressKHR(vk_ctx.device, &buffer_device_address_info);

			VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info = {0};
			acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			acceleration_build_geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			acceleration_build_geometry_info.dstAccelerationStructure = accel->impl.bottom_level_acceleration_structure[i];
			acceleration_build_geometry_info.geometryCount = 1;
			acceleration_build_geometry_info.pGeometries = &acceleration_geometry;
			acceleration_build_geometry_info.scratchData.deviceAddress = scratch_buffer_device_address;

			VkAccelerationStructureBuildRangeInfoKHR acceleration_build_range_info = {0};
			acceleration_build_range_info.primitiveCount = prim_count;
			acceleration_build_range_info.primitiveOffset = 0x0;
			acceleration_build_range_info.firstVertex = 0;
			acceleration_build_range_info.transformOffset = 0x0;

			const VkAccelerationStructureBuildRangeInfoKHR *acceleration_build_infos[1] = {&acceleration_build_range_info};

			{
				VkCommandBufferAllocateInfo cmd_buf_allocate_info = {0};
				cmd_buf_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cmd_buf_allocate_info.commandPool = vk_ctx.cmd_pool;
				cmd_buf_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				cmd_buf_allocate_info.commandBufferCount = 1;

				VkCommandBuffer command_buffer;
				vkAllocateCommandBuffers(vk_ctx.device, &cmd_buf_allocate_info, &command_buffer);

				VkCommandBufferBeginInfo command_buffer_info = {0};
				command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				vkBeginCommandBuffer(command_buffer, &command_buffer_info);

				_vkCmdBuildAccelerationStructuresKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCmdBuildAccelerationStructuresKHR");
				_vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &acceleration_build_geometry_info, &acceleration_build_infos[0]);

				vkEndCommandBuffer(command_buffer);

				VkSubmitInfo submit_info = {0};
				submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit_info.commandBufferCount = 1;
				submit_info.pCommandBuffers = &command_buffer;

				VkFenceCreateInfo fence_info = {0};
				fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fence_info.flags = 0;

				VkFence fence;
				vkCreateFence(vk_ctx.device, &fence_info, NULL, &fence);

				VkResult result = vkQueueSubmit(vk_ctx.queue, 1, &submit_info, fence);
				assert(!result);
				vkWaitForFences(vk_ctx.device, 1, &fence, VK_TRUE, 100000000000);
				vkDestroyFence(vk_ctx.device, fence, NULL);
				vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, &command_buffer);
			}

			VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info = {0};
			acceleration_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			acceleration_device_address_info.accelerationStructure = accel->impl.bottom_level_acceleration_structure[i];

			accel->impl.bottom_level_acceleration_structure_handle[i] = _vkGetAccelerationStructureDeviceAddressKHR(vk_ctx.device, &acceleration_device_address_info);

			vkFreeMemory(vk_ctx.device, scratch_memory, NULL);
			vkDestroyBuffer(vk_ctx.device, scratch_buffer, NULL);

			accel->impl.bottom_level_buffer[i] = bottom_level_buffer;
			accel->impl.bottom_level_mem[i] = bottom_level_mem;
		}
	}

	// Top level

	{
		VkBufferCreateInfo buf_info = {0};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.pNext = NULL;
		buf_info.size = instances_count * sizeof(VkAccelerationStructureInstanceKHR);
		buf_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		buf_info.flags = 0;

		VkMemoryAllocateInfo mem_alloc;
		memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkBuffer instances_buffer;
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &instances_buffer);

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(vk_ctx.device, instances_buffer, &mem_reqs);

		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		mem_alloc.pNext = &memory_allocate_flags_info;

		VkDeviceMemory instances_mem;
		vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &instances_mem);

		vkBindBufferMemory(vk_ctx.device, instances_buffer, instances_mem, 0);
		void *data;
		vkMapMemory(vk_ctx.device, instances_mem, 0, sizeof(VkAccelerationStructureInstanceKHR), 0, (void **)&data);

		for (int i = 0; i < instances_count; ++i) {
			VkTransformMatrixKHR transform_matrix = {
				instances[i].m.m[0],
				instances[i].m.m[4],
				instances[i].m.m[8],
				instances[i].m.m[12],
				instances[i].m.m[1],
				instances[i].m.m[5],
				instances[i].m.m[9],
				instances[i].m.m[13],
				instances[i].m.m[2],
				instances[i].m.m[6],
				instances[i].m.m[10],
				instances[i].m.m[14]
			};
			VkAccelerationStructureInstanceKHR instance = {0};
			instance.transform = transform_matrix;

			int ib_off = 0;
			for (int j = 0; j < instances[i].i; ++j) {
				ib_off += ib[j]->impl.count * 4;
			}
			instance.instanceCustomIndex = ib_off;

			instance.mask = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = accel->impl.bottom_level_acceleration_structure_handle[instances[i].i];
			memcpy(data + i * sizeof(VkAccelerationStructureInstanceKHR), &instance, sizeof(VkAccelerationStructureInstanceKHR));
		}

		vkUnmapMemory(vk_ctx.device, instances_mem);

		VkDeviceOrHostAddressConstKHR instance_data_device_address = {0};
		instance_data_device_address.deviceAddress = get_buffer_device_address(instances_buffer);

		VkAccelerationStructureGeometryKHR acceleration_geometry = {0};
		acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		acceleration_geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		acceleration_geometry.geometry.instances.arrayOfPointers = VK_FALSE;
		acceleration_geometry.geometry.instances.data.deviceAddress = instance_data_device_address.deviceAddress;

		VkAccelerationStructureBuildGeometryInfoKHR acceleration_structure_build_geometry_info = {0};
		acceleration_structure_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		acceleration_structure_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		acceleration_structure_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		acceleration_structure_build_geometry_info.geometryCount = 1;
		acceleration_structure_build_geometry_info.pGeometries = &acceleration_geometry;

		VkAccelerationStructureBuildSizesInfoKHR acceleration_build_sizes_info = {0};
		acceleration_build_sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		uint32_t instance_count = instances_count;

		_vkGetAccelerationStructureBuildSizesKHR(vk_ctx.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &acceleration_structure_build_geometry_info,
		                                         &instance_count, &acceleration_build_sizes_info);

		VkBufferCreateInfo buffer_create_info = {0};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = acceleration_build_sizes_info.accelerationStructureSize;
		buffer_create_info.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer top_level_buffer = VK_NULL_HANDLE;
		vkCreateBuffer(vk_ctx.device, &buffer_create_info, NULL, &top_level_buffer);

		VkMemoryRequirements memory_requirements2;
		vkGetBufferMemoryRequirements(vk_ctx.device, top_level_buffer, &memory_requirements2);

		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memory_allocate_info = {0};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.pNext = &memory_allocate_flags_info;
		memory_allocate_info.allocationSize = memory_requirements2.size;
		memory_type_from_properties(memory_requirements2.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
		VkDeviceMemory top_level_mem;
		vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &top_level_mem);
		vkBindBufferMemory(vk_ctx.device, top_level_buffer, top_level_mem, 0);

		VkAccelerationStructureCreateInfoKHR acceleration_create_info = {0};
		acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		acceleration_create_info.buffer = top_level_buffer;
		acceleration_create_info.size = acceleration_build_sizes_info.accelerationStructureSize;
		_vkCreateAccelerationStructureKHR(vk_ctx.device, &acceleration_create_info, NULL, &accel->impl.top_level_acceleration_structure);

		VkBuffer scratch_buffer = VK_NULL_HANDLE;
		VkDeviceMemory scratch_memory = VK_NULL_HANDLE;

		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = acceleration_build_sizes_info.buildScratchSize;
		buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(vk_ctx.device, &buffer_create_info, NULL, &scratch_buffer);

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(vk_ctx.device, scratch_buffer, &memory_requirements);

		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.pNext = &memory_allocate_flags_info;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
		vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &scratch_memory);
		vkBindBufferMemory(vk_ctx.device, scratch_buffer, scratch_memory, 0);

		VkBufferDeviceAddressInfoKHR buffer_device_address_info = {0};
		buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		buffer_device_address_info.buffer = scratch_buffer;
		uint64_t scratch_buffer_device_address = _vkGetBufferDeviceAddressKHR(vk_ctx.device, &buffer_device_address_info);

		VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info = {0};
		acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		acceleration_build_geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		acceleration_build_geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;
		acceleration_build_geometry_info.dstAccelerationStructure = accel->impl.top_level_acceleration_structure;
		acceleration_build_geometry_info.geometryCount = 1;
		acceleration_build_geometry_info.pGeometries = &acceleration_geometry;
		acceleration_build_geometry_info.scratchData.deviceAddress = scratch_buffer_device_address;

		VkAccelerationStructureBuildRangeInfoKHR acceleration_build_range_info = {0};
		acceleration_build_range_info.primitiveCount = instances_count;
		acceleration_build_range_info.primitiveOffset = 0x0;
		acceleration_build_range_info.firstVertex = 0;
		acceleration_build_range_info.transformOffset = 0x0;

		const VkAccelerationStructureBuildRangeInfoKHR *acceleration_build_infos[1] = {&acceleration_build_range_info};

		{
			VkCommandBufferAllocateInfo cmd_buf_allocate_info = {0};
			cmd_buf_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd_buf_allocate_info.commandPool = vk_ctx.cmd_pool;
			cmd_buf_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd_buf_allocate_info.commandBufferCount = 1;

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(vk_ctx.device, &cmd_buf_allocate_info, &command_buffer);

			VkCommandBufferBeginInfo command_buffer_info = {0};
			command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			vkBeginCommandBuffer(command_buffer, &command_buffer_info);

			_vkCmdBuildAccelerationStructuresKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCmdBuildAccelerationStructuresKHR");
			_vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &acceleration_build_geometry_info, &acceleration_build_infos[0]);

			vkEndCommandBuffer(command_buffer);

			VkSubmitInfo submit_info = {0};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer;

			VkFenceCreateInfo fence_info = {0};
			fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fence_info.flags = 0;

			VkFence fence;
			vkCreateFence(vk_ctx.device, &fence_info, NULL, &fence);

			VkResult result = vkQueueSubmit(vk_ctx.queue, 1, &submit_info, fence);
			assert(!result);
			vkWaitForFences(vk_ctx.device, 1, &fence, VK_TRUE, 100000000000);
			vkDestroyFence(vk_ctx.device, fence, NULL);

			vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, &command_buffer);
		}

		VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info = {0};
		acceleration_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		acceleration_device_address_info.accelerationStructure = accel->impl.top_level_acceleration_structure;

		accel->impl.top_level_acceleration_structure_handle = _vkGetAccelerationStructureDeviceAddressKHR(vk_ctx.device, &acceleration_device_address_info);

		vkFreeMemory(vk_ctx.device, scratch_memory, NULL);
		vkDestroyBuffer(vk_ctx.device, scratch_buffer, NULL);

		accel->impl.top_level_buffer = top_level_buffer;
		accel->impl.top_level_mem = top_level_mem;
		accel->impl.instances_buffer = instances_buffer;
		accel->impl.instances_mem = instances_mem;
	}

	{
		// if (vb_full != NULL) {
		// 	vkFreeMemory(vk_ctx.device, vb_full_mem, NULL);
		// 	vkDestroyBuffer(vk_ctx.device, vb_full, NULL);
		// }

		// VkBufferCreateInfo buf_info = {0};
		// buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// buf_info.pNext = NULL;
		// buf_info.size = vert_count * vb[0]->impl.myStride;
		// buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		// buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		// buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		// buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		// buf_info.flags = 0;

		// VkMemoryAllocateInfo mem_alloc = {0};
		// mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		// mem_alloc.pNext = NULL;
		// mem_alloc.allocationSize = 0;
		// mem_alloc.memoryTypeIndex = 0;

		// vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &vb_full);

		// VkMemoryRequirements mem_reqs = {0};
		// vkGetBufferMemoryRequirements(vk_ctx.device, vb_full, &mem_reqs);

		// mem_alloc.allocationSize = mem_reqs.size;
		// memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		// VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
		// memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		// memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		// mem_alloc.pNext = &memory_allocate_flags_info;

		// vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &vb_full_mem);
		// vkBindBufferMemory(vk_ctx.device, vb_full, vb_full_mem, 0);

		// float *data;
		// vkMapMemory(vk_ctx.device, vb_full_mem, 0, vert_count * vb[0]->impl.myStride, 0, (void **)&data);
		// vkUnmapMemory(vk_ctx.device, vb_full_mem);

		////

		#ifdef is_forge

		vb_full = _vb_full->impl.vertices.buf;
		vb_full_mem = _vb_full->impl.vertices.mem;

		#else

		vb_full = vb[0]->impl.vertices.buf;
		vb_full_mem = vb[0]->impl.vertices.mem;

		#endif
	}

	{
		// if (ib_full != NULL) {
		// 	vkFreeMemory(vk_ctx.device, ib_full_mem, NULL);
		// 	vkDestroyBuffer(vk_ctx.device, ib_full, NULL);
		// }

		// VkBufferCreateInfo buf_info = {0};
		// buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// buf_info.pNext = NULL;
		// buf_info.size = prim_count * 3 * sizeof(uint32_t);
		// buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		// buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		// buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		// buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		// buf_info.flags = 0;

		// VkMemoryAllocateInfo mem_alloc = {0};
		// mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		// mem_alloc.pNext = NULL;
		// mem_alloc.allocationSize = 0;
		// mem_alloc.memoryTypeIndex = 0;

		// vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &ib_full);

		// VkMemoryRequirements mem_reqs = {0};
		// vkGetBufferMemoryRequirements(vk_ctx.device, ib_full, &mem_reqs);

		// mem_alloc.allocationSize = mem_reqs.size;
		// memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		// VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
		// memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		// memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		// mem_alloc.pNext = &memory_allocate_flags_info;

		// vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &ib_full_mem);
		// vkBindBufferMemory(vk_ctx.device, ib_full, ib_full_mem, 0);

		// uint8_t *data;
		// vkMapMemory(vk_ctx.device, ib_full_mem, 0, mem_alloc.allocationSize, 0, (void **)&data);
		// for (int i = 0; i < instances_count; ++i) {
		// 	memcpy(data, ib[i]->impl., sizeof(VkAccelerationStructureInstanceKHR));
		// }
		// vkUnmapMemory(vk_ctx.device, ib_full_mem);

		////

		#ifdef is_forge

		ib_full = _ib_full->impl.buf;
		ib_full_mem = _ib_full->impl.mem;

		#else

		ib_full = ib[0]->impl.buf;
		ib_full_mem = ib[0]->impl.mem;

		#endif
	}
}

void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel) {
	// _vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkDestroyAccelerationStructureKHR");
	// for (int i = 0; i < vb_count; ++i) {
	// 	_vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.bottom_level_acceleration_structure[i], NULL);
	// 	vkFreeMemory(vk_ctx.device, accel->impl.bottom_level_mem[i], NULL);
	// 	vkDestroyBuffer(vk_ctx.device, accel->impl.bottom_level_buffer[i], NULL);
	// }
	// _vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.top_level_acceleration_structure, NULL);
	// vkFreeMemory(vk_ctx.device, accel->impl.top_level_mem, NULL);
	// vkDestroyBuffer(vk_ctx.device, accel->impl.top_level_buffer, NULL);
	// vkFreeMemory(vk_ctx.device, accel->impl.instances_mem, NULL);
	// vkDestroyBuffer(vk_ctx.device, accel->impl.instances_buffer, NULL);
}

void kinc_raytrace_set_textures(kinc_g5_render_target_t *_texpaint0, kinc_g5_render_target_t *_texpaint1, kinc_g5_render_target_t *_texpaint2, kinc_g5_texture_t *_texenv, kinc_g5_texture_t *_texsobol, kinc_g5_texture_t *_texscramble, kinc_g5_texture_t *_texrank) {
	texpaint0 = _texpaint0;
	texpaint1 = _texpaint1;
	texpaint2 = _texpaint2;
	texenv = _texenv;
	texsobol = _texsobol;
	texscramble = _texscramble;
	texrank = _texrank;
}

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_raytrace_set_target(kinc_g5_render_target_t *_output) {
	if (_output != output) {
		vkDestroyImage(vk_ctx.device, _output->impl.sourceImage, NULL);

		VkImageCreateInfo image = {0};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = NULL;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = _output->impl.format;
		image.extent.width = _output->width;
		image.extent.height = _output->height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		image.flags = 0;

		vkCreateImage(vk_ctx.device, &image, NULL, &_output->impl.sourceImage);

		vkBindImageMemory(vk_ctx.device, _output->impl.sourceImage, _output->impl.sourceMemory, 0);

		VkImageViewCreateInfo colorImageView = {0};
		colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageView.pNext = NULL;
		colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageView.format = _output->impl.format;
		colorImageView.flags = 0;
		colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageView.subresourceRange.baseMipLevel = 0;
		colorImageView.subresourceRange.levelCount = 1;
		colorImageView.subresourceRange.baseArrayLayer = 0;
		colorImageView.subresourceRange.layerCount = 1;
		colorImageView.image = _output->impl.sourceImage;
		vkCreateImageView(vk_ctx.device, &colorImageView, NULL, &_output->impl.sourceView);

		VkImageView attachments[1];
		attachments[0] = _output->impl.sourceView;

		VkFramebufferCreateInfo fbufCreateInfo = {0};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		fbufCreateInfo.renderPass = vk_ctx.windows[vk_ctx.current_window].rendertarget_render_pass;
		fbufCreateInfo.attachmentCount = 1;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = _output->width;
		fbufCreateInfo.height = _output->height;
		fbufCreateInfo.layers = 1;
		vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &_output->impl.framebuffer);
	}
	output = _output;
}

void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	VkWriteDescriptorSetAccelerationStructureKHR descriptor_acceleration_structure_info = {0};
	descriptor_acceleration_structure_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	descriptor_acceleration_structure_info.accelerationStructureCount = 1;
	descriptor_acceleration_structure_info.pAccelerationStructures = &accel->impl.top_level_acceleration_structure;

	VkWriteDescriptorSet acceleration_structure_write = {0};
	acceleration_structure_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	acceleration_structure_write.pNext = &descriptor_acceleration_structure_info;
	acceleration_structure_write.dstSet = pipeline->impl.descriptor_set;
	acceleration_structure_write.dstBinding = 0;
	acceleration_structure_write.descriptorCount = 1;
	acceleration_structure_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

	VkDescriptorImageInfo image_descriptor = {0};
	image_descriptor.imageView = output->impl.sourceView;
	image_descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkDescriptorBufferInfo buffer_descriptor = {0};
	buffer_descriptor.buffer = pipeline->_constant_buffer->impl.buf;
	buffer_descriptor.range = VK_WHOLE_SIZE;
	buffer_descriptor.offset = 0;

	VkWriteDescriptorSet result_image_write = {0};
	result_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	result_image_write.pNext = NULL;
	result_image_write.dstSet = pipeline->impl.descriptor_set;
	result_image_write.dstBinding = 10;
	result_image_write.descriptorCount = 1;
	result_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	result_image_write.pImageInfo = &image_descriptor;

	VkWriteDescriptorSet uniform_buffer_write = {0};
	uniform_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniform_buffer_write.pNext = NULL;
	uniform_buffer_write.dstSet = pipeline->impl.descriptor_set;
	uniform_buffer_write.dstBinding = 11;
	uniform_buffer_write.descriptorCount = 1;
	uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_buffer_write.pBufferInfo = &buffer_descriptor;

	VkDescriptorBufferInfo ib_descriptor = {0};
	ib_descriptor.buffer = ib_full;
	ib_descriptor.range = VK_WHOLE_SIZE;
	ib_descriptor.offset = 0;

	VkWriteDescriptorSet ib_write = {0};
	ib_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ib_write.pNext = NULL;
	ib_write.dstSet = pipeline->impl.descriptor_set;
	ib_write.dstBinding = 1;
	ib_write.descriptorCount = 1;
	ib_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ib_write.pBufferInfo = &ib_descriptor;

	VkDescriptorBufferInfo vb_descriptor = {0};
	vb_descriptor.buffer = vb_full;
	vb_descriptor.range = VK_WHOLE_SIZE;
	vb_descriptor.offset = 0;

	VkWriteDescriptorSet vb_write = {0};
	vb_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vb_write.pNext = NULL;
	vb_write.dstSet = pipeline->impl.descriptor_set;
	vb_write.dstBinding = 2;
	vb_write.descriptorCount = 1;
	vb_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vb_write.pBufferInfo = &vb_descriptor;

	VkDescriptorImageInfo tex0image_descriptor = {0};
	tex0image_descriptor.imageView = texpaint0->impl.sourceView;
	tex0image_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet tex0_image_write = {0};
	tex0_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	tex0_image_write.pNext = NULL;
	tex0_image_write.dstSet = pipeline->impl.descriptor_set;
	tex0_image_write.dstBinding = 3;
	tex0_image_write.descriptorCount = 1;
	tex0_image_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	tex0_image_write.pImageInfo = &tex0image_descriptor;

	VkDescriptorImageInfo tex1image_descriptor = {0};
	tex1image_descriptor.imageView = texpaint1->impl.sourceView;
	tex1image_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet tex1_image_write = {0};
	tex1_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	tex1_image_write.pNext = NULL;
	tex1_image_write.dstSet = pipeline->impl.descriptor_set;
	tex1_image_write.dstBinding = 4;
	tex1_image_write.descriptorCount = 1;
	tex1_image_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	tex1_image_write.pImageInfo = &tex1image_descriptor;

	VkDescriptorImageInfo tex2image_descriptor = {0};
	tex2image_descriptor.imageView = texpaint2->impl.sourceView;
	tex2image_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet tex2_image_write = {0};
	tex2_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	tex2_image_write.pNext = NULL;
	tex2_image_write.dstSet = pipeline->impl.descriptor_set;
	tex2_image_write.dstBinding = 5;
	tex2_image_write.descriptorCount = 1;
	tex2_image_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	tex2_image_write.pImageInfo = &tex2image_descriptor;

	VkDescriptorImageInfo texenvimage_descriptor = {0};
	texenvimage_descriptor.imageView = texenv->impl.texture.view;
	texenvimage_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texenv_image_write = {0};
	texenv_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	texenv_image_write.pNext = NULL;
	texenv_image_write.dstSet = pipeline->impl.descriptor_set;
	texenv_image_write.dstBinding = 6;
	texenv_image_write.descriptorCount = 1;
	texenv_image_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	texenv_image_write.pImageInfo = &texenvimage_descriptor;

	VkDescriptorImageInfo texsobolimage_descriptor = {0};
	texsobolimage_descriptor.imageView = texsobol->impl.texture.view;
	texsobolimage_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texsobol_image_write = {0};
	texsobol_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	texsobol_image_write.pNext = NULL;
	texsobol_image_write.dstSet = pipeline->impl.descriptor_set;
	texsobol_image_write.dstBinding = 7;
	texsobol_image_write.descriptorCount = 1;
	texsobol_image_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	texsobol_image_write.pImageInfo = &texsobolimage_descriptor;

	VkDescriptorImageInfo texscrambleimage_descriptor = {0};
	texscrambleimage_descriptor.imageView = texscramble->impl.texture.view;
	texscrambleimage_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texscramble_image_write = {0};
	texscramble_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	texscramble_image_write.pNext = NULL;
	texscramble_image_write.dstSet = pipeline->impl.descriptor_set;
	texscramble_image_write.dstBinding = 8;
	texscramble_image_write.descriptorCount = 1;
	texscramble_image_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	texscramble_image_write.pImageInfo = &texscrambleimage_descriptor;

	VkDescriptorImageInfo texrankimage_descriptor = {0};
	texrankimage_descriptor.imageView = texrank->impl.texture.view;
	texrankimage_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texrank_image_write = {0};
	texrank_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	texrank_image_write.pNext = NULL;
	texrank_image_write.dstSet = pipeline->impl.descriptor_set;
	texrank_image_write.dstBinding = 9;
	texrank_image_write.descriptorCount = 1;
	texrank_image_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	texrank_image_write.pImageInfo = &texrankimage_descriptor;

	VkWriteDescriptorSet write_descriptor_sets[12] = {
		acceleration_structure_write,
		result_image_write,
		uniform_buffer_write,
		vb_write,
		ib_write,
		tex0_image_write,
		tex1_image_write,
		tex2_image_write,
		texenv_image_write,
		texsobol_image_write,
		texscramble_image_write,
		texrank_image_write
	};
	vkUpdateDescriptorSets(vk_ctx.device, 12, write_descriptor_sets, 0, VK_NULL_HANDLE);

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_pipeline_properties;
	ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	ray_tracing_pipeline_properties.pNext = NULL;
	VkPhysicalDeviceProperties2 device_properties = {0};
	device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	device_properties.pNext = &ray_tracing_pipeline_properties;
	vkGetPhysicalDeviceProperties2(vk_ctx.gpu, &device_properties);

	// Setup the strided buffer regions pointing to the shaders in our shader binding table
	const uint32_t handle_size_aligned =
	    (ray_tracing_pipeline_properties.shaderGroupHandleSize + ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1) &
	    ~(ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1);

	VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry = {0};
	raygen_shader_sbt_entry.deviceAddress = get_buffer_device_address(pipeline->impl.raygen_shader_binding_table);
	raygen_shader_sbt_entry.stride = handle_size_aligned;
	raygen_shader_sbt_entry.size = handle_size_aligned;

	VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry = {0};
	miss_shader_sbt_entry.deviceAddress = get_buffer_device_address(pipeline->impl.miss_shader_binding_table);
	miss_shader_sbt_entry.stride = handle_size_aligned;
	miss_shader_sbt_entry.size = handle_size_aligned;

	VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry = {0};
	hit_shader_sbt_entry.deviceAddress = get_buffer_device_address(pipeline->impl.hit_shader_binding_table);
	hit_shader_sbt_entry.stride = handle_size_aligned;
	hit_shader_sbt_entry.size = handle_size_aligned;

	VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry = {0};

	vkCmdEndRenderPass(command_list->impl._buffer);

	// Dispatch the ray tracing commands
	vkCmdBindPipeline(command_list->impl._buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline);
	vkCmdBindDescriptorSets(command_list->impl._buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline_layout, 0, 1,
	                        &pipeline->impl.descriptor_set, 0, 0);

	_vkCmdTraceRaysKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCmdTraceRaysKHR");
	_vkCmdTraceRaysKHR(command_list->impl._buffer, &raygen_shader_sbt_entry, &miss_shader_sbt_entry, &hit_shader_sbt_entry, &callable_shader_sbt_entry,
	                   output->texWidth, output->texHeight, 1);

	vkCmdBeginRenderPass(command_list->impl._buffer, &currentRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

#endif
