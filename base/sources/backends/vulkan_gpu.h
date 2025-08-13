#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

typedef struct gpu_pipeline_impl {
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
} gpu_pipeline_impl_t;

typedef struct {
	char *source;
	int length;
} gpu_shader_impl_t;

typedef struct {
	VkImage image;
	VkDeviceMemory mem;
	VkImageView view;
	bool has_storage_bit;
} gpu_texture_impl_t;

typedef struct {
	VkBuffer buf;
	VkDeviceMemory mem;
} gpu_buffer_impl_t;

typedef struct {
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	VkDescriptorSet descriptor_set;
	VkDescriptorSetLayout descriptor_set_layout;
	VkBuffer raygen_shader_binding_table;
	VkBuffer miss_shader_binding_table;
	VkBuffer hit_shader_binding_table;
} gpu_raytrace_pipeline_impl_t;

typedef struct {
	VkAccelerationStructureKHR top_level_acceleration_structure;
	VkAccelerationStructureKHR bottom_level_acceleration_structure[16];
	uint64_t top_level_acceleration_structure_handle;
	uint64_t bottom_level_acceleration_structure_handle[16];

	VkBuffer bottom_level_buffer[16];
	VkDeviceMemory bottom_level_mem[16];
	VkBuffer top_level_buffer;
	VkDeviceMemory top_level_mem;
	VkBuffer instances_buffer;
	VkDeviceMemory instances_mem;
} gpu_raytrace_acceleration_structure_impl_t;
