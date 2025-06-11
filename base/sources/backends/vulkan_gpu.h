#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define MAXIMUM_WINDOWS 1

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
	struct vk_depth depth;
};

struct vk_context {
	VkInstance instance;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkCommandBuffer setup_cmd;
	VkCommandPool cmd_pool;
	VkQueue queue;
	struct vk_window windows[MAXIMUM_WINDOWS];
	VkBuffer *uniform_buffer;
#ifdef VALIDATE
	bool validation_found;
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

typedef struct {
	int _indexCount;
	VkCommandBuffer _buffer;
	VkFence fence;
} gpu_command_list_impl_t;

typedef struct gpu_pipeline_impl {
	const char **textures;
	int *textureValues;
	int textureCount;
	VkPipeline pipeline;
	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;
	VkPipelineLayout pipeline_layout;
} gpu_pipeline_impl_t;

typedef struct {
	int vertexOffset;
} gpu_constant_location_impl_t;

typedef struct {
	unsigned id;
	char *source;
	int length;
} gpu_shader_impl_t;

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
