#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define KINC_INTERNAL_NAMED_NUMBER_COUNT 32

typedef struct {
	char name[256];
	uint32_t number;
} kinc_internal_named_number;

typedef struct {
	uint32_t hash;
	uint32_t index;
} kinc_internal_hash_index_t;

uint32_t kinc_internal_hash_name(unsigned char *str);

struct vk_funs {
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkDestroySurfaceKHR fpDestroySurfaceKHR;

	PFN_vkCreateDebugUtilsMessengerEXT fpCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT fpDestroyDebugUtilsMessengerEXT;

	PFN_vkQueuePresentKHR fpQueuePresentKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
};

struct vk_depth {
	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
};

struct vk_window {
	int width;
	int height;

	bool resized;
	bool surface_destroyed;

	int depth_bits;

	bool vsynced;

	uint32_t current_image;

	VkSurfaceKHR surface;
	VkSurfaceFormatKHR format;

	VkSwapchainKHR swapchain;
	uint32_t image_count;
	VkImage *images;
	VkImageView *views;
	VkFramebuffer *framebuffers;

	VkRenderPass framebuffer_render_pass;
	VkRenderPass rendertarget_render_pass;
	VkRenderPass rendertarget_render_pass_with_depth;

	struct vk_depth depth;
};

#define MAXIMUM_WINDOWS 1

struct vk_context {
	VkInstance instance;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memory_properties;

	VkCommandBuffer setup_cmd;
	VkCommandPool cmd_pool;
	VkQueue queue;

	struct vk_window windows[MAXIMUM_WINDOWS];

	// buffer hack
	VkBuffer *vertex_uniform_buffer;
	VkBuffer *fragment_uniform_buffer;
	VkBuffer *compute_uniform_buffer;

#ifdef VALIDATE
	bool validation_found;
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

extern struct vk_funs vk;
extern struct vk_context vk_ctx;

extern void flush_init_cmd(void);
extern void reuse_descriptor_sets(void);
extern void reuse_compute_descriptor_sets(void);

typedef struct {
	int _indexCount;
	VkCommandBuffer _buffer;
	VkFence fence;
} CommandList5Impl;

typedef struct kinc_g5_compute_shader_impl {
	kinc_internal_named_number locations[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number texture_bindings[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number offsets[KINC_INTERNAL_NAMED_NUMBER_COUNT];

	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
	VkShaderModule shader_module;
} kinc_g5_compute_shader_impl;

typedef struct PipelineState5Impl_s {
	const char **textures;
	int *textureValues;
	int textureCount;

	VkPipeline framebuffer_pipeline;
	VkPipeline rendertarget_pipeline;
	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	kinc_internal_named_number vertexLocations[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number fragmentLocations[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number vertexTextureBindings[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number fragmentTextureBindings[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number vertexOffsets[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number fragmentOffsets[KINC_INTERNAL_NAMED_NUMBER_COUNT];

	VkPipelineLayout pipeline_layout;
} PipelineState5Impl;

typedef struct ComputePipelineState5Impl_t {
	int a;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
	int computeOffset;
} ConstantLocation5Impl;

typedef struct {
	int nothing;
} AttributeLocation5Impl;

typedef struct {
	unsigned id;
	char *source;
	int length;
} Shader5Impl;

typedef struct kinc_g5_sampler_impl {
	VkSampler sampler;
} kinc_g5_sampler_impl_t;

typedef struct {
	VkImageLayout imageLayout;
	VkDeviceSize deviceSize;
	int stride;

	VkImage image;
	VkDeviceMemory mem;
	VkImageView view;

	VkImage depthImage;
	VkDeviceMemory depthMemory;
	VkImageView depthView;
	int depthBufferBits;

	VkFramebuffer framebuffer;
	VkFormat format;

	VkBuffer readback_buffer;
	VkDeviceMemory readback_memory;
	bool readback_buffer_created;

	int stage;
	int stage_depth;

} Texture5Impl;

typedef struct {
	float *data;
	int myCount;
	int myStride;
	unsigned bufferId;
	VkMemoryAllocateInfo mem_alloc;

	VkBuffer buf;
	VkDeviceMemory mem;
} VertexBuffer5Impl;

typedef struct {
	VkBuffer buf;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryAllocateInfo mem_alloc;
	VkDeviceMemory mem;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

typedef struct {
	int count;
	VkBuffer buf;
	VkDeviceMemory mem;
	VkMemoryAllocateInfo mem_alloc;
} IndexBuffer5Impl;

#ifndef KINC_ANDROID

typedef struct {
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	VkDescriptorSet descriptor_set;
	VkDescriptorSetLayout descriptor_set_layout;
	VkBuffer raygen_shader_binding_table;
	VkBuffer miss_shader_binding_table;
	VkBuffer hit_shader_binding_table;
} kinc_g5_raytrace_pipeline_impl_t;

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
} kinc_g5_raytrace_acceleration_structure_impl_t;

#endif
