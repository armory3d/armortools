#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define IRON_INTERNAL_NAMED_NUMBER_COUNT 32

typedef struct {
	char name[256];
	uint32_t number;
} iron_internal_named_number;

typedef struct {
	uint32_t hash;
	uint32_t index;
} iron_internal_hash_index_t;

struct indexed_name {
	uint32_t id;
	char *name;
};

struct indexed_index {
	uint32_t id;
	uint32_t value;
};

struct descriptor_set {
	int id;
	bool in_use;
	VkDescriptorImageInfo tex_desc[16];
	VkDescriptorSet set;
};

typedef struct inst {
	iron_matrix4x4_t m;
	int i;
} inst_t;

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

#ifdef VALIDATE
	bool validation_found;
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

extern struct vk_funs vk;
extern struct vk_context vk_ctx;
extern void flush_init_cmd(void);
extern void reuse_descriptor_sets(void);

typedef struct {
	int _indexCount;
	VkCommandBuffer _buffer;
	VkFence fence;
} gpu_command_list_impl_t;

typedef struct gpu_pipeline_impl {
	const char **textures;
	int *textureValues;
	int textureCount;

	VkPipeline framebuffer_pipeline;
	VkPipeline rendertarget_pipeline;
	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	iron_internal_named_number vertexLocations[IRON_INTERNAL_NAMED_NUMBER_COUNT];
	iron_internal_named_number fragmentLocations[IRON_INTERNAL_NAMED_NUMBER_COUNT];
	iron_internal_named_number vertexTextureBindings[IRON_INTERNAL_NAMED_NUMBER_COUNT];
	iron_internal_named_number fragmentTextureBindings[IRON_INTERNAL_NAMED_NUMBER_COUNT];
	iron_internal_named_number vertexOffsets[IRON_INTERNAL_NAMED_NUMBER_COUNT];
	iron_internal_named_number fragmentOffsets[IRON_INTERNAL_NAMED_NUMBER_COUNT];

	VkPipelineLayout pipeline_layout;
} gpu_pipeline_impl_t;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
} gpu_constant_location_impl_t;

typedef struct {
	unsigned id;
	char *source;
	int length;
} gpu_shader_impl_t;

typedef struct iron_gpu_sampler_impl {
	VkSampler sampler;
} gpu_sampler_impl_t;

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

} gpu_texture_impl_t;

typedef struct {
	int myCount;

	VkBuffer buf;
	VkDeviceMemory mem;
	VkMemoryAllocateInfo mem_alloc;

	float *data;
	int myStride;
	unsigned bufferId;

	int lastStart;
	int lastCount;
	int mySize;
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
