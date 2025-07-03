
#ifndef NDEBUG
#define VALIDATE
#endif

#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <iron_gpu.h>
#include <iron_math.h>
#include <iron_system.h>
#include "vulkan_gpu.h"

#define MAX_DESCRIPTOR_SETS 1024

typedef struct descriptor_set {
	int id;
	bool in_use;
	VkDescriptorImageInfo tex_desc[16];
	VkDescriptorSet set;
} descriptor_set_t;

bool gpu_transpose_mat = true;

static gpu_texture_t *current_textures[16] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static VkSemaphore framebuffer_available_semaphore;
static VkSemaphore rendering_finished_semaphore;
static VkFence fence;
static gpu_pipeline_t *current_pipeline = NULL;
static VkDescriptorSetLayout descriptor_layout;
static VkDescriptorPool descriptor_pool;
static descriptor_set_t descriptor_sets[MAX_DESCRIPTOR_SETS] = {0};
static int descriptor_sets_count = 0;
static VkRenderingInfo current_rendering_info;
static VkRenderingAttachmentInfo current_color_attachment_infos[8];
static VkRenderingAttachmentInfo current_depth_attachment_info;
static VkPhysicalDeviceMemoryProperties memory_properties;
static VkSampler immutable_sampler;
static int index_count = 0;
static VkCommandBuffer command_buffer;

static VkInstance instance;
static VkPhysicalDevice gpu;
static VkDevice device;
static VkCommandPool cmd_pool;
static VkQueue queue;
static VkBuffer *uniform_buffer;
#ifdef VALIDATE
static bool validation_found;
static VkDebugUtilsMessengerEXT debug_messenger;
#endif

static bool window_surface_destroyed;
static int window_depth_bits;
static bool window_vsynced;
static VkSurfaceKHR window_surface;
static VkSurfaceFormatKHR window_format;
static VkSwapchainKHR window_swapchain;
static uint32_t framebuffer_count;
static bool framebuffer_acquired = false;

void iron_vulkan_get_instance_extensions(const char **extensions, int *index);
VkBool32 iron_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physical_device, uint32_t queue_family_index);
VkResult iron_vulkan_create_surface(VkInstance instance, VkSurfaceKHR *surface);

static VkFormat convert_image_format(gpu_texture_format_t format) {
	switch (format) {
	case GPU_TEXTURE_FORMAT_RGBA128:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case GPU_TEXTURE_FORMAT_RGBA64:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case GPU_TEXTURE_FORMAT_R8:
		return VK_FORMAT_R8_UNORM;
	case GPU_TEXTURE_FORMAT_R16:
		return VK_FORMAT_R16_SFLOAT;
	case GPU_TEXTURE_FORMAT_R32:
		return VK_FORMAT_R32_SFLOAT;
	case GPU_TEXTURE_FORMAT_RGBA32:
		// return VK_FORMAT_R8G8B8A8_UNORM;
		return VK_FORMAT_B8G8R8A8_UNORM;
	case GPU_TEXTURE_FORMAT_D32:
		return VK_FORMAT_D32_SFLOAT;
	default:
		return VK_FORMAT_B8G8R8A8_UNORM;
	}
}

static int format_size(VkFormat format) {
	switch (format) {
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return 16;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return 8;
	case VK_FORMAT_R16_SFLOAT:
		return 2;
	case VK_FORMAT_R8_UNORM:
		return 1;
	default:
		return 4;
	}
}

static VkCullModeFlagBits convert_cull_mode(gpu_cull_mode_t cull_mode) {
	switch (cull_mode) {
	case GPU_CULL_MODE_CLOCKWISE:
		return VK_CULL_MODE_BACK_BIT;
	case GPU_CULL_MODE_COUNTERCLOCKWISE:
		return VK_CULL_MODE_FRONT_BIT;
	case GPU_CULL_MODE_NEVER:
	default:
		return VK_CULL_MODE_NONE;
	}
}

static VkCompareOp convert_compare_mode(gpu_compare_mode_t compare) {
	switch (compare) {
	default:
	case GPU_COMPARE_MODE_ALWAYS:
		return VK_COMPARE_OP_ALWAYS;
	case GPU_COMPARE_MODE_NEVER:
		return VK_COMPARE_OP_NEVER;
	case GPU_COMPARE_MODE_LESS:
		return VK_COMPARE_OP_LESS;
	}
}

static VkBlendFactor convert_blend_factor(gpu_blending_factor_t factor) {
	switch (factor) {
	case GPU_BLEND_ONE:
		return VK_BLEND_FACTOR_ONE;
	case GPU_BLEND_ZERO:
		return VK_BLEND_FACTOR_ZERO;
	case GPU_BLEND_SOURCE_ALPHA:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case GPU_BLEND_DEST_ALPHA:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case GPU_BLEND_INV_SOURCE_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case GPU_BLEND_INV_DEST_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	}
}

static VkBlendOp convert_blend_operation(gpu_blending_operation_t op) {
	switch (op) {
	case GPU_BLENDOP_ADD:
		return VK_BLEND_OP_ADD;
	}
}

static VkImageLayout convert_texture_state(gpu_texture_state_t state) {
	switch (state) {
	case GPU_TEXTURE_STATE_SHADER_RESOURCE:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case GPU_TEXTURE_STATE_RENDER_TARGET:
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case GPU_TEXTURE_STATE_PRESENT:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
}

static VkBool32 vk_debug_utils_messenger_callback_ext(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT *pcallback_data,
	void *puser_data) {
	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		iron_error("Vulkan ERROR: Code %d : %s", pcallback_data->messageIdNumber, pcallback_data->pMessage);
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		iron_log("Vulkan WARNING: Code %d : %s", pcallback_data->messageIdNumber, pcallback_data->pMessage);
	}
	return VK_FALSE;
}

static bool check_extensions(const char **wanted_extensions, int wanted_extension_count, VkExtensionProperties *extensions, int extension_count) {
	bool *found_extensions = calloc(wanted_extension_count, 1);
	for (int i = 0; i < extension_count; i++) {
		for (int i2 = 0; i2 < wanted_extension_count; i2++) {
			if (strcmp(wanted_extensions[i2], extensions[i].extensionName) == 0) {
				found_extensions[i2] = true;
			}
		}
	}

	bool missing_extensions = false;
	for (int i = 0; i < wanted_extension_count; i++) {
		if (!found_extensions[i]) {
			iron_error("Failed to find extension %s", wanted_extensions[i]);
			missing_extensions = true;
		}
	}
	free(found_extensions);
	return missing_extensions;
}

static bool find_layer(VkLayerProperties *layers, int layer_count, const char *wanted_layer) {
	for (int i = 0; i < layer_count; i++) {
		if (strcmp(wanted_layer, layers[i].layerName) == 0) {
			return true;
		}
	}
	return false;
}

static void memory_type_from_properties(uint32_t type_bits, VkFlags requirements_mask, uint32_t *type_index) {
	for (uint32_t i = 0; i < 32; i++) {
		if ((type_bits & 1) == 1) {
			if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*type_index = i;
			}
		}
		type_bits >>= 1;
	}
}

static VkAccessFlags access_mask(VkImageLayout layout) {
	// .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	// .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	// .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
	// .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
	if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		return VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		return VK_ACCESS_MEMORY_READ_BIT;
	}
	return 0;
}

void gpu_barrier(gpu_texture_t *render_target, gpu_texture_state_t state_after) {
	if (render_target->state == state_after) {
		return;
	}

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = access_mask(convert_texture_state(render_target->state)),
		.dstAccessMask = access_mask(convert_texture_state(state_after)),
		.oldLayout = convert_texture_state(render_target->state),
		.newLayout = convert_texture_state(state_after),
		.image = render_target->impl.image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	// if (render_target->format == GPU_TEXTURE_FORMAT_D32) {
	// 	VkImageMemoryBarrier barrier = {
	// 		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	// 		.pNext = NULL,
	// 		.srcAccessMask = access_mask(convert_texture_state(render_target->state)),
	// 		.dstAccessMask = access_mask(convert_texture_state(state_after)),
	// 		.oldLayout = convert_texture_state(render_target->state) == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : convert_texture_state(render_target->state),
	// 		.newLayout = convert_texture_state(state_after) == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : convert_texture_state(state_after),
	// 		.image = render_target->impl.image,
	// 		.subresourceRange = {
	// 			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
	// 			.baseMipLevel = 0,
	// 			.levelCount = 1,
	// 			.baseArrayLayer = 0,
	// 			.layerCount = 1,
	// 		},
	// 	};
	// 	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	// }
	// else {
	// }

	render_target->state = state_after;
}

static void set_image_layout(VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_layout, VkImageLayout new_layout) {
	if (gpu_in_use) {
		vkCmdEndRendering(command_buffer);
	}

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.image = image,
		.subresourceRange.aspectMask = aspect_mask,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
	};

	if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	}
	if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
		barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	if (gpu_in_use) {
		vkCmdBeginRendering(command_buffer, &current_rendering_info);
	}
}

void create_descriptor_layout(void) {
	VkDescriptorSetLayoutBinding bindings[18];
	memset(bindings, 0, sizeof(bindings));

	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = NULL;

	VkSamplerCreateInfo sampler_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1.0f,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias = 0.0f,
		.minLod = 0.0f,
		.maxLod = 0.0f,
	};
	vkCreateSampler(device, &sampler_info, NULL, &immutable_sampler);

	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = &immutable_sampler;

	for (int i = 2; i < 18; ++i) {
		bindings[i].binding = i;
		bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[i].pImmutableSamplers = NULL;
	}

	VkDescriptorSetLayoutCreateInfo layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.bindingCount = 18,
		.pBindings = bindings,
	};

	vkCreateDescriptorSetLayout(device, &layout_create_info, NULL, &descriptor_layout);

	VkDescriptorPoolSize type_counts[3];
	memset(type_counts, 0, sizeof(type_counts));

	type_counts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	type_counts[0].descriptorCount = 1 * 1024;

	type_counts[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	type_counts[1].descriptorCount = 1 * 1024;

	type_counts[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	type_counts[2].descriptorCount = 16 * 1024;

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.maxSets = 1024,
		.poolSizeCount = 3,
		.pPoolSizes = type_counts,
	};

	vkCreateDescriptorPool(device, &pool_info, NULL, &descriptor_pool);
}

VkSwapchainKHR cleanup_swapchain() {
	// for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
	// 	gpu_texture_destroy(&framebuffers[i]);
	// }
	VkSwapchainKHR chain = window_swapchain;
	window_swapchain = VK_NULL_HANDLE;
	return chain;
}

void gpu_render_target_init2(gpu_texture_t *target, int width, int height, gpu_texture_format_t format, int framebuffer_index) {
	target->width = width;
	target->height = height;
	target->data = NULL;
	target->impl.format = convert_image_format(format);
	target->impl.stage = 0;
	target->impl.readback_buffer_created = false;
	target->uploaded = true;
	target->state = (framebuffer_index >= 0) ? GPU_TEXTURE_STATE_PRESENT : GPU_TEXTURE_STATE_SHADER_RESOURCE;

	if (framebuffer_index >= 0) {
		return;
	}

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(gpu, target->impl.format, &format_properties);

	VkImageCreateInfo image = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = target->impl.format,
		.extent.width = width,
		.extent.height = height,
		.extent.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.flags = 0,
	};

	if (format == GPU_TEXTURE_FORMAT_D32) {
		image.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else {
		image.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	VkImageViewCreateInfo color_image_view = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = target->impl.format,
		.flags = 0,
		.subresourceRange.aspectMask = format == GPU_TEXTURE_FORMAT_D32 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
	};

	vkCreateImage(device, &image, NULL, &target->impl.image);
	VkMemoryRequirements memory_reqs;
	vkGetImageMemoryRequirements(device, target->impl.image, &memory_reqs);

	VkMemoryAllocateInfo allocation_nfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.memoryTypeIndex = 0,
		.allocationSize = memory_reqs.size,
	};
	memory_type_from_properties(memory_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocation_nfo.memoryTypeIndex);
	vkAllocateMemory(device, &allocation_nfo, NULL, &target->impl.mem);
	vkBindImageMemory(device, target->impl.image, target->impl.mem, 0);
	set_image_layout(target->impl.image, format == GPU_TEXTURE_FORMAT_D32 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	color_image_view.image = target->impl.image;
	vkCreateImageView(device, &color_image_view, NULL, &target->impl.view);
}

static void create_swapchain() {
	VkSwapchainKHR old_swapchain = cleanup_swapchain();
	if (window_surface_destroyed) {
		vkDestroySwapchainKHR(device, old_swapchain, NULL);
		old_swapchain = VK_NULL_HANDLE;
		vkDestroySurfaceKHR(instance, window_surface, NULL);
		iron_vulkan_create_surface(instance, &window_surface);
		window_surface_destroyed = false;
	}

	VkSurfaceCapabilitiesKHR caps = {0};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, window_surface, &caps);

	VkPresentModeKHR present_modes[256];
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, window_surface, &present_mode_count, NULL);
	present_mode_count = present_mode_count > 256 ? 256 : present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, window_surface, &present_mode_count, present_modes);

	uint32_t image_count = caps.minImageCount;// + 1;
	if ((caps.maxImageCount > 0) && (image_count > caps.maxImageCount)) {
		image_count = caps.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR pre_transform = {0};
	if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		pre_transform = caps.currentTransform;
	}

	// Fetch newest window size
	iron_internal_handle_messages();

	VkSwapchainCreateInfoKHR swapchain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.surface = window_surface,
		.minImageCount = image_count,
		.imageFormat = window_format.format,
		.imageColorSpace = window_format.colorSpace,
		.imageExtent.width = iron_window_width(),
		.imageExtent.height = iron_window_height(),
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = pre_transform,
	};

	if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	else if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	else if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	else if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	}

	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.queueFamilyIndexCount = 0;
	swapchain_info.pQueueFamilyIndices = NULL;
	swapchain_info.presentMode = window_vsynced ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
	swapchain_info.oldSwapchain = old_swapchain;
	swapchain_info.clipped = true;

	vkCreateSwapchainKHR(device, &swapchain_info, NULL, &window_swapchain);

	if (old_swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, old_swapchain, NULL);
	}

	vkGetSwapchainImagesKHR(device, window_swapchain, &framebuffer_count, NULL);
	VkImage *window_images = (VkImage *)malloc(framebuffer_count * sizeof(VkImage));
	vkGetSwapchainImagesKHR(device, window_swapchain, &framebuffer_count, window_images);

	for (uint32_t i = 0; i < framebuffer_count; i++) {
		framebuffers[i].impl.image = window_images[i];
		VkImageViewCreateInfo color_attachment_view = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.format = window_format.format,
			.components.r = VK_COMPONENT_SWIZZLE_R,
			.components.g = VK_COMPONENT_SWIZZLE_G,
			.components.b = VK_COMPONENT_SWIZZLE_B,
			.components.a = VK_COMPONENT_SWIZZLE_A,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.flags = 0,
		};
		set_image_layout(window_images[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		color_attachment_view.image = window_images[i];
		vkCreateImageView(device, &color_attachment_view, NULL, &framebuffers[i].impl.view);
	}

	framebuffer_index = 0;

	if (window_depth_bits > 0) {
		VkImageCreateInfo image = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D16_UNORM,
			.extent.width = iron_window_width(),
			.extent.height = iron_window_height(),
			.extent.depth = 1,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.flags = 0,
		};

		VkMemoryAllocateInfo mem_alloc = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = NULL,
			.allocationSize = 0,
			.memoryTypeIndex = 0,
		};

		VkImageViewCreateInfo view = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.image = VK_NULL_HANDLE,
			.format = VK_FORMAT_D16_UNORM,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
			.flags = 0,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
		};

		VkMemoryRequirements mem_reqs = {0};
		vkCreateImage(device, &image, NULL, &framebuffer_depth.impl.image);
		vkGetImageMemoryRequirements(device, framebuffer_depth.impl.image, &mem_reqs);
		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);
		vkAllocateMemory(device, &mem_alloc, NULL, &framebuffer_depth.impl.mem);
		vkBindImageMemory(device, framebuffer_depth.impl.image, framebuffer_depth.impl.mem, 0);
		set_image_layout(framebuffer_depth.impl.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		view.image = framebuffer_depth.impl.image;
		vkCreateImageView(device, &view, NULL, &framebuffer_depth.impl.view);
	}
}

static void acquire_next_image() {
	VkResult err = vkAcquireNextImageKHR(device, window_swapchain, UINT64_MAX, framebuffer_available_semaphore, VK_NULL_HANDLE, &framebuffer_index);
	if (err == VK_ERROR_SURFACE_LOST_KHR || err == VK_ERROR_OUT_OF_DATE_KHR) {
		window_surface_destroyed = (err == VK_ERROR_SURFACE_LOST_KHR);

		gpu_in_use = false;
		create_swapchain();
		gpu_in_use = true;
		acquire_next_image();

		for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
			// gpu_texture_destroy(&framebuffers[i]);
			// gpu_render_target_init2(&framebuffers[i], iron_window_width(), iron_window_height(), GPU_TEXTURE_FORMAT_RGBA32, i);
			framebuffers[i].width = iron_window_width();
			framebuffers[i].height = iron_window_height();
		}
	}
}

void gpu_resize_internal(int width, int height) {
	// Newest window size is fetched in create_swapchain
}

void gpu_init_internal(int depth_buffer_bits, bool vsync) {
	uint32_t instance_layer_count = 0;

	static const char *wanted_instance_layers[64];
	int wanted_instance_layer_count = 0;

	vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);

	if (instance_layer_count > 0) {
		VkLayerProperties *instance_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * instance_layer_count);
		vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);

#ifdef VALIDATE
		validation_found = find_layer(instance_layers, instance_layer_count, "VK_LAYER_KHRONOS_validation");
		if (validation_found) {
			iron_log("Running with Vulkan validation layers enabled.");
			wanted_instance_layers[wanted_instance_layer_count++] = "VK_LAYER_KHRONOS_validation";
		}
#endif

		free(instance_layers);
	}

	static const char *wanted_instance_extensions[64];
	int wanted_instance_extension_count = 0;
	uint32_t instance_extension_count = 0;
	wanted_instance_extensions[wanted_instance_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
	wanted_instance_extensions[wanted_instance_extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
	iron_vulkan_get_instance_extensions(wanted_instance_extensions, &wanted_instance_extension_count);

	vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	VkExtensionProperties *instance_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
	vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
	bool missing_instance_extensions = check_extensions(wanted_instance_extensions, wanted_instance_extension_count, instance_extensions, instance_extension_count);

	if (missing_instance_extensions) {
		iron_error("");
	}

#ifdef VALIDATE
	// this extension should be provided by the validation layers
	if (validation_found) {
		wanted_instance_extensions[wanted_instance_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}
#endif

	VkApplicationInfo app = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = iron_application_name(),
		.applicationVersion = 0,
		.pEngineName = "Iron",
		.engineVersion = 0,
		.apiVersion = VK_API_VERSION_1_3,
	};

	VkInstanceCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = NULL;
	info.pApplicationInfo = &app;
#ifdef VALIDATE
	if (validation_found) {
		info.enabledLayerCount = wanted_instance_layer_count;
		info.ppEnabledLayerNames = (const char *const *)wanted_instance_layers;
	}
	else
#endif
	{
		info.enabledLayerCount = 0;
		info.ppEnabledLayerNames = NULL;
	}
	info.enabledExtensionCount = wanted_instance_extension_count;
	info.ppEnabledExtensionNames = (const char *const *)wanted_instance_extensions;

	VkResult err = vkCreateInstance(&info, NULL, &instance);
	if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
		iron_error("Vulkan driver is incompatible");
	}
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
		iron_error("Vulkan extension not found");
	}
	else if (err) {
		iron_error("Can not create Vulkan instance");
	}

	uint32_t gpu_count;
	vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);

	if (gpu_count > 0) {
		VkPhysicalDevice *physical_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		vkEnumeratePhysicalDevices(instance, &gpu_count, physical_devices);

		float best_score = 0.0;
		for (uint32_t gpu_idx = 0; gpu_idx < gpu_count; gpu_idx++) {
			VkPhysicalDevice current_gpu = physical_devices[gpu_idx];
			uint32_t queue_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(current_gpu, &queue_count, NULL);
			VkQueueFamilyProperties *queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
			vkGetPhysicalDeviceQueueFamilyProperties(current_gpu, &queue_count, queue_props);
			bool can_present = false;
			bool can_render = false;
			for (uint32_t i = 0; i < queue_count; i++) {
				VkBool32 queue_supports_present = iron_vulkan_get_physical_device_presentation_support(current_gpu, i);
				if (queue_supports_present) {
					can_present = true;
				}
				VkQueueFamilyProperties queue_properties = queue_props[i];
				uint32_t flags = queue_properties.queueFlags;
				if ((flags & VK_QUEUE_GRAPHICS_BIT) != 0) {
					can_render = true;
				}
			}
			if (!can_present || !can_render) {
				continue;
			}

			float score = 0.0;
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(current_gpu, &properties);
			switch (properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score = 2;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score = 1;
				break;
			}

			if (gpu == VK_NULL_HANDLE || score > best_score) {
				gpu = current_gpu;
				best_score = score;
			}
		}

		if (gpu == VK_NULL_HANDLE) {
			iron_error("No Vulkan device that supports presentation found");
		}

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(gpu, &properties);
		iron_log("Chosen Vulkan device: %s", properties.deviceName);
		free(physical_devices);
	}
	else {
		iron_error("No Vulkan device found");
	}

	static const char *wanted_device_layers[64];
	int wanted_device_layer_count = 0;

	uint32_t device_layer_count = 0;
	vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, NULL);

	if (device_layer_count > 0) {
		VkLayerProperties *device_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * device_layer_count);
		vkEnumerateDeviceLayerProperties(gpu, &device_layer_count, device_layers);

#ifdef VALIDATE
		validation_found = find_layer(device_layers, device_layer_count, "VK_LAYER_KHRONOS_validation");
		if (validation_found) {
			wanted_device_layers[wanted_device_layer_count++] = "VK_LAYER_KHRONOS_validation";
		}
#endif

		free(device_layers);
	}

	const char *wanted_device_extensions[64];
	int wanted_device_extension_count = 0;

	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	if (gpu_raytrace_supported()) {
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
	}

	uint32_t device_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(gpu, NULL, &device_extension_count, NULL);

	VkExtensionProperties *device_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * device_extension_count);
	vkEnumerateDeviceExtensionProperties(gpu, NULL, &device_extension_count, device_extensions);

	bool missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);
	free(device_extensions);

	if (missing_device_extensions) {
		exit(1);
	}

#ifdef VALIDATE
	if (validation_found) {
		VkDebugUtilsMessengerCreateInfoEXT create_info = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.flags = 0,
			.pfnUserCallback = vk_debug_utils_messenger_callback_ext,
			.pUserData = NULL,
			.pNext = NULL,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		};
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		vkCreateDebugUtilsMessengerEXT(instance, &create_info, NULL, &debug_messenger);
	}
#endif

	uint32_t queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);

	VkQueueFamilyProperties *queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);

	VkBool32 *supports_present = (VkBool32 *)malloc(queue_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < queue_count; i++) {
		supports_present[i] = iron_vulkan_get_physical_device_presentation_support(gpu, i);
	}

	uint32_t graphics_queue_node_index = UINT32_MAX;
	uint32_t present_queue_node_index = UINT32_MAX;
	for (uint32_t i = 0; i < queue_count; i++) {
		if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (graphics_queue_node_index == UINT32_MAX) {
				graphics_queue_node_index = i;
			}

			if (supports_present[i] == VK_TRUE) {
				graphics_queue_node_index = i;
				present_queue_node_index = i;
				break;
			}
		}
	}
	if (present_queue_node_index == UINT32_MAX) {
		for (uint32_t i = 0; i < queue_count; ++i) {
			if (supports_present[i] == VK_TRUE) {
				present_queue_node_index = i;
				break;
			}
		}
	}
	free(supports_present);

	if (graphics_queue_node_index == UINT32_MAX || present_queue_node_index == UINT32_MAX) {
		iron_error("Graphics or present queue not found");
	}

	if (graphics_queue_node_index != present_queue_node_index) {
		iron_error("Graphics and present queue do not match");
	}

	{
		float queue_priorities[1] = { 0.0 };
		VkDeviceQueueCreateInfo queue = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.queueFamilyIndex = graphics_queue_node_index,
			.queueCount = 1,
			.pQueuePriorities = queue_priorities,
		};

		VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
			.dynamicRendering = VK_TRUE,
		};

		VkDeviceCreateInfo deviceinfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &dynamic_rendering_features,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queue,
			.enabledLayerCount = wanted_device_layer_count,
			.ppEnabledLayerNames = (const char *const *)wanted_device_layers,
			.enabledExtensionCount = wanted_device_extension_count,
			.ppEnabledExtensionNames = (const char *const *)wanted_device_extensions,
		};

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_pipeline_ext = {0};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR raytracing_acceleration_structure_ext = {0};
		VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_ext = {0};
		if (gpu_raytrace_supported()) {
			raytracing_pipeline_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			raytracing_pipeline_ext.pNext = deviceinfo.pNext;
			raytracing_pipeline_ext.rayTracingPipeline = VK_TRUE;

			raytracing_acceleration_structure_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			raytracing_acceleration_structure_ext.pNext = &raytracing_pipeline_ext;
			raytracing_acceleration_structure_ext.accelerationStructure = VK_TRUE;

			buffer_device_address_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
			buffer_device_address_ext.pNext = &raytracing_acceleration_structure_ext;
			buffer_device_address_ext.bufferDeviceAddress = VK_TRUE;

			deviceinfo.pNext = &buffer_device_address_ext;
		}

		vkCreateDevice(gpu, &deviceinfo, NULL, &device);
	}

	vkGetDeviceQueue(device, graphics_queue_node_index, 0, &queue);
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
	VkCommandPoolCreateInfo cmd_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.queueFamilyIndex = graphics_queue_node_index,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};

	vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmd_pool);
	create_descriptor_layout();

	VkSemaphoreCreateInfo sem_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};

	vkCreateSemaphore(device, &sem_info, NULL, &framebuffer_available_semaphore);
	vkCreateSemaphore(device, &sem_info, NULL, &rendering_finished_semaphore);

	window_depth_bits = depth_buffer_bits;
	window_vsynced = vsync;

	iron_vulkan_create_surface(instance, &window_surface);

	VkBool32 surface_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpu, graphics_queue_node_index, window_surface, &surface_supported);

	VkSurfaceFormatKHR surf_formats[256];
	uint32_t formatCount = sizeof(surf_formats) / sizeof(surf_formats[0]);
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, window_surface, &formatCount, surf_formats);

	if (formatCount == 1 && surf_formats[0].format == VK_FORMAT_UNDEFINED) {
		window_format = surf_formats[0];
	}
	else {
		bool found = false;
		for (uint32_t i = 0; i < formatCount; ++i) {
			if (surf_formats[i].format != VK_FORMAT_B8G8R8A8_SRGB) {
				window_format = surf_formats[i];
				found = true;
				break;
			}
		}
		if (!found) {
			window_format = surf_formats[0];
		}
	}

	VkCommandBufferAllocateInfo cmd = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = cmd_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	vkAllocateCommandBuffers(device, &cmd, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};
	vkBeginCommandBuffer(command_buffer, &begin_info);

	gpu_create_framebuffers(depth_buffer_bits);

	create_swapchain();

	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	vkCreateFence(device, &fence_info, NULL, &fence);
}

void gpu_destroy() {
	vkFreeCommandBuffers(device, cmd_pool, 1, &command_buffer);
	vkDestroyFence(device, fence, NULL);
	VkSwapchainKHR swapchain = cleanup_swapchain();
	vkDestroySwapchainKHR(device, swapchain, NULL);
	vkDestroySurfaceKHR(instance, window_surface, NULL);
}

void iron_vulkan_init_window() {
	// This function is used in the android backend
	window_surface_destroyed = true;
}

bool iron_vulkan_internal_get_size(int *width, int *height) {
	// This function is used in the android backend
	if (window_surface) {
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, window_surface, &capabilities);
		*width = capabilities.currentExtent.width;
		*height = capabilities.currentExtent.height;
		return true;
	}
	return false;
}

int gpu_max_bound_textures(void) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(gpu, &props);
	return props.limits.maxPerStageDescriptorSamplers;
}

static void set_viewport_and_scissor() {
	VkViewport viewport = {
		.x = 0,
		.y = (float)current_render_targets[0]->height,
		.width = (float)current_render_targets[0]->width,
		.height = -(float)current_render_targets[0]->height,
		.minDepth = (float)0.0f,
		.maxDepth = (float)1.0f,
	};
	VkRect2D scissor = {
		.extent.width = current_render_targets[0]->width,
		.extent.height = current_render_targets[0]->height,
		.offset.x = 0,
		.offset.y = 0,
	};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void gpu_begin_internal(gpu_texture_t **targets, int count, gpu_texture_t * depth_buffer, unsigned flags, unsigned color, float depth) {
	if (!framebuffer_acquired) {
		acquire_next_image();
		framebuffer_acquired = true;
	}

	gpu_texture_t *target = current_render_targets[0];

	VkRect2D render_area = {
		.offset = {0, 0}
	};
	render_area.extent.width = target->width;
	render_area.extent.height = target->height;

	VkClearValue clear_value;
	memset(&clear_value, 0, sizeof(VkClearValue));
	clear_value.color.float32[0] = ((color & 0x00ff0000) >> 16) / 255.0f;
	clear_value.color.float32[1] = ((color & 0x0000ff00) >> 8 ) / 255.0f;
	clear_value.color.float32[2] = ((color & 0x000000ff)      ) / 255.0f;
	clear_value.color.float32[3] = ((color & 0xff000000) >> 24) / 255.0f;

	for (size_t i = 0; i < current_render_targets_count; ++i) {
		current_color_attachment_infos[i] = (VkRenderingAttachmentInfo){
			.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext              = NULL,
			.imageView          = current_render_targets[i]->impl.view,
			.imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.resolveMode        = VK_RESOLVE_MODE_NONE,
			.resolveImageView   = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.loadOp             = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = clear_value,
		};
	}

	if (depth_buffer != NULL) {
		current_depth_attachment_info = (VkRenderingAttachmentInfo) {
			.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext              = NULL,
			.imageView          = depth_buffer->impl.view,
			.imageLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.resolveMode        = VK_RESOLVE_MODE_NONE,
			.resolveImageView   = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.loadOp             = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue         = 1.0,
		};
	}

	current_rendering_info = (VkRenderingInfo) {
		.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.pNext                = NULL,
		.flags                = 0,
		.renderArea           = render_area,
		.layerCount           = 1,
		.viewMask             = 0,
		.colorAttachmentCount = (uint32_t)current_render_targets_count,
		.pColorAttachments    = current_color_attachment_infos,
		.pDepthAttachment     = depth_buffer == NULL ? VK_NULL_HANDLE : &current_depth_attachment_info,
		.pStencilAttachment   = VK_NULL_HANDLE,
	};
	vkCmdBeginRendering(command_buffer, &current_rendering_info);

	set_viewport_and_scissor();

	if (current_pipeline != NULL) {
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.pipeline);
	}

	if (flags != GPU_CLEAR_NONE) {
		int count = 0;
		VkClearAttachment attachments[2];
		if (flags & GPU_CLEAR_COLOR) {
			VkClearColorValue clearColor = {0};
			clearColor.float32[0] = ((color & 0x00ff0000) >> 16) / 255.0f;
			clearColor.float32[1] = ((color & 0x0000ff00) >> 8) / 255.0f;
			clearColor.float32[2] = (color & 0x000000ff) / 255.0f;
			clearColor.float32[3] = ((color & 0xff000000) >> 24) / 255.0f;
			attachments[count].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			attachments[count].colorAttachment = 0;
			attachments[count].clearValue.color = clearColor;
			count++;
		}
		if (flags & GPU_CLEAR_DEPTH) {
			attachments[count].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			attachments[count].clearValue.depthStencil.depth = depth;
			attachments[count].clearValue.depthStencil.stencil = 0;
			count++;
		}
		VkClearRect clearRect = {
			.rect.offset.x = 0,
			.rect.offset.y = 0,
			.rect.extent.width = targets[0]->width,
			.rect.extent.height = targets[0]->height,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
		vkCmdClearAttachments(command_buffer, count, attachments, 1, &clearRect);
	}
}

void gpu_end_internal() {
	vkCmdEndRendering(command_buffer);

	for (int i = 0; i < current_render_targets_count; ++i) {
		gpu_barrier(current_render_targets[i],
			current_render_targets[i] == &framebuffers[framebuffer_index] ? GPU_TEXTURE_STATE_PRESENT : GPU_TEXTURE_STATE_SHADER_RESOURCE);
	}
	current_render_targets_count = 0;
}

void gpu_wait() {
	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void gpu_present() {
	vkEndCommandBuffer(command_buffer);
	vkResetFences(device, 1, &fence);

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &rendering_finished_semaphore,
	};

	VkSemaphore wait_semaphores[1] = {framebuffer_available_semaphore};
	VkPipelineStageFlags wait_stages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	vkQueueSubmit(queue, 1, &submit_info, fence);

	VkPresentInfoKHR present = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.swapchainCount = 1,
		.pSwapchains = &window_swapchain,
		.pImageIndices = &framebuffer_index,
		.pWaitSemaphores = &rendering_finished_semaphore,
		.waitSemaphoreCount = 1,
	};
	vkQueuePresentKHR(queue, &present);

	for (int i = 0; i < descriptor_sets_count; ++i) {
		descriptor_sets[i].in_use = false;
	}

	gpu_wait();
	framebuffer_acquired = false;

	vkResetCommandBuffer(command_buffer, 0);
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};
	vkBeginCommandBuffer(command_buffer, &begin_info);
}

void gpu_draw_internal() {
	vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
}

void gpu_viewport(int x, int y, int width, int height) {
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.x = (float)x;
	viewport.y = y + (float)height;
	viewport.width = (float)width;
	viewport.height = (float)-height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
}

void gpu_scissor(int x, int y, int width, int height) {
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = x;
	scissor.offset.y = y;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void gpu_disable_scissor() {
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = current_render_targets[0]->width;
	scissor.extent.height = current_render_targets[0]->height;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void gpu_set_pipeline(gpu_pipeline_t *pipeline) {
	current_pipeline = pipeline;
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.pipeline);
}

void gpu_set_vertex_buffer(gpu_buffer_t *buffer) {
	VkBuffer buffers[1];
	VkDeviceSize offsets[1];
	buffers[0] = buffer->impl.buf;
	offsets[0] = (VkDeviceSize)(0);
	vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
}

void gpu_set_index_buffer(gpu_buffer_t *buffer) {
	index_count = gpu_index_buffer_count(buffer);
	vkCmdBindIndexBuffer(command_buffer, buffer->impl.buf, 0, VK_INDEX_TYPE_UINT32);
}

void gpu_upload_texture(gpu_texture_t *texture) {
}

void gpu_get_render_target_pixels(gpu_texture_t *render_target, uint8_t *data) {
	VkFormat format = render_target->impl.format;
	int format_bytes_size = format_size(format);

	if (!render_target->impl.readback_buffer_created) {
		VkBufferCreateInfo buf_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = NULL,
			.size = render_target->width * render_target->height * format_bytes_size,
			.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.flags = 0,
		};
		vkCreateBuffer(device, &buf_info, NULL, &render_target->impl.readback_buffer);

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(device, render_target->impl.readback_buffer, &mem_reqs);

		VkMemoryAllocateInfo mem_alloc;
		memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;
		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
		vkAllocateMemory(device, &mem_alloc, NULL, &render_target->impl.readback_memory);
		vkBindBufferMemory(device, render_target->impl.readback_buffer, render_target->impl.readback_memory, 0);

		render_target->impl.readback_buffer_created = true;
	}

	vkCmdEndRendering(command_buffer);

	set_image_layout(render_target->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = render_target->width;
	region.bufferImageHeight = render_target->height;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = 0;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageOffset.z = 0;
	region.imageExtent.width = (uint32_t)render_target->width;
	region.imageExtent.height = (uint32_t)render_target->height;
	region.imageExtent.depth = 1;
	vkCmdCopyImageToBuffer(command_buffer, render_target->impl.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, render_target->impl.readback_buffer, 1, &region);

	set_image_layout(render_target->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	gpu_wait();

	// Read buffer
	void *p;
	vkMapMemory(device, render_target->impl.readback_memory, 0, VK_WHOLE_SIZE, 0, (void **)&p);
	memcpy(data, p, render_target->width * render_target->height * format_bytes_size);
	vkUnmapMemory(device, render_target->impl.readback_memory);
}

static int calc_descriptor_id(void) {
	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (current_textures[i] != NULL) {
			texture_count++;
		}
	}
	bool uniform_buffer = true;
	return 1 | (texture_count << 1) | ((uniform_buffer ? 1 : 0) << 8);
}

static int write_tex_descs(VkDescriptorImageInfo *tex_descs) {
	memset(tex_descs, 0, sizeof(VkDescriptorImageInfo) * 16);

	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (current_textures[i] != NULL) {
			tex_descs[i].imageView = current_textures[i]->impl.view;
			tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			texture_count++;
		}
	}
	return texture_count;
}

static void update_textures(descriptor_set_t *set) {
	memset(&set->tex_desc, 0, sizeof(set->tex_desc));
	int texture_count = write_tex_descs(set->tex_desc);
	VkWriteDescriptorSet writes[16];
	memset(&writes, 0, sizeof(writes));

	for (int i = 0; i < 16; ++i) {
		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set->set;
		writes[i].dstBinding = i + 2;
		writes[i].descriptorCount = 1;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		writes[i].pImageInfo = &set->tex_desc[i];
	}

	if (current_textures[0] != NULL) {
		vkUpdateDescriptorSets(device, texture_count, writes, 0, NULL);
	}
}

static VkDescriptorSet get_descriptor_set() {
	int id = calc_descriptor_id();
	for (int i = 0; i < descriptor_sets_count; ++i) {
		if (descriptor_sets[i].id == id) {
			if (!descriptor_sets[i].in_use) {
				descriptor_sets[i].in_use = true;
				update_textures(&descriptor_sets[i]);
				return descriptor_sets[i].set;
			}
		}
	}

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptor_layout,
	};
	VkDescriptorSet descriptor_set;
	vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set);

	VkDescriptorBufferInfo buffer_descs[1];
	memset(&buffer_descs, 0, sizeof(buffer_descs));
	buffer_descs[0].buffer = *uniform_buffer;
	buffer_descs[0].offset = 0;
	buffer_descs[0].range = 256 * sizeof(float);

	VkDescriptorImageInfo tex_desc[16];
	memset(&tex_desc, 0, sizeof(tex_desc));

	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (current_textures[i] != NULL) {
			tex_desc[i].imageView = current_textures[i]->impl.view;
			texture_count++;
		}
		tex_desc[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	VkWriteDescriptorSet writes[17];
	memset(&writes, 0, sizeof(writes));

	int write_count = 0;
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = descriptor_set;
	writes[0].dstBinding = 0;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writes[0].pBufferInfo = &buffer_descs[0];
	write_count++;

	for (int i = 0; i < 16; ++i) {
		if (current_textures[i] != NULL) {
			writes[write_count].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[write_count].dstSet = descriptor_set;
			writes[write_count].dstBinding = i + 2;
			writes[write_count].descriptorCount = 1;
			writes[write_count].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			writes[write_count].pImageInfo = &tex_desc[i];
			write_count++;
		}
	}

	vkUpdateDescriptorSets(device, write_count, writes, 0, NULL);
	descriptor_sets[descriptor_sets_count].id = id;
	descriptor_sets[descriptor_sets_count].in_use = true;
	descriptor_sets[descriptor_sets_count].set = descriptor_set;
	write_tex_descs(descriptor_sets[descriptor_sets_count].tex_desc);
	descriptor_sets_count += 1;
	return descriptor_set;
}

void gpu_set_constant_buffer(gpu_buffer_t *buffer, int offset, size_t size) {
	VkDescriptorSet descriptor_set = get_descriptor_set();
	uint32_t offsets[1] = {offset};
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.pipeline_layout, 0, 1, &descriptor_set, 1, offsets);
}

void gpu_set_texture(int unit, gpu_texture_t *texture) {
	texture->impl.stage = unit;
	current_textures[unit] = texture;
}

void gpu_pipeline_init(gpu_pipeline_t *pipeline) {
	gpu_internal_pipeline_init(pipeline);
}

void gpu_pipeline_destroy(gpu_pipeline_t *pipeline) {
	vkDestroyPipeline(device, pipeline->impl.pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline->impl.pipeline_layout, NULL);
}

static VkShaderModule create_shader_module(const void *code, size_t size) {
	VkShaderModuleCreateInfo module_create_info;
	module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_create_info.pNext = NULL;
	module_create_info.codeSize = size;
	module_create_info.pCode = (const uint32_t *)code;
	module_create_info.flags = 0;
	VkShaderModule module;
	vkCreateShaderModule(device, &module_create_info, NULL, &module);
	return module;
}

void gpu_pipeline_compile(gpu_pipeline_t *pipeline) {
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_layout,
	};
	vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL, &pipeline->impl.pipeline_layout);

	VkGraphicsPipelineCreateInfo pipeline_info = {0};
	VkPipelineInputAssemblyStateCreateInfo ia = {0};
	VkPipelineRasterizationStateCreateInfo rs = {0};
	VkPipelineColorBlendStateCreateInfo cb = {0};
	VkPipelineDepthStencilStateCreateInfo ds = {0};
	VkPipelineViewportStateCreateInfo vp = {0};
	VkPipelineMultisampleStateCreateInfo ms = {0};
	VkDynamicState dynamic_state[2];
	VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {0};

	memset(dynamic_state, 0, sizeof(dynamic_state));
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.pDynamicStates = dynamic_state;

	memset(&pipeline_info, 0, sizeof(pipeline_info));
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.layout = pipeline->impl.pipeline_layout;

	VkVertexInputBindingDescription vi_bindings[1];
	int vertexAttributeCount = pipeline->input_layout->size;
	VkVertexInputAttributeDescription vi_attrs[vertexAttributeCount];

	VkPipelineVertexInputStateCreateInfo vi = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = vi_bindings,
		.vertexAttributeDescriptionCount = vertexAttributeCount,
		.pVertexAttributeDescriptions = vi_attrs,
	};

	uint32_t attr = 0;
	uint32_t offset = 0;
	for (int i = 0; i < pipeline->input_layout->size; ++i) {
		gpu_vertex_element_t element = pipeline->input_layout->elements[i];
		vi_attrs[attr].binding = 0;
		vi_attrs[attr].location = i;
		vi_attrs[attr].offset = offset;
		offset += gpu_vertex_data_size(element.data);

		switch (element.data) {
		case GPU_VERTEX_DATA_F32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_SFLOAT;
			break;
		case GPU_VERTEX_DATA_F32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_SFLOAT;
			break;
		case GPU_VERTEX_DATA_F32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case GPU_VERTEX_DATA_F32_4X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case GPU_VERTEX_DATA_I16_2X_NORM:
			vi_attrs[attr].format = VK_FORMAT_R16G16_SNORM;
			break;
		case GPU_VERTEX_DATA_I16_4X_NORM:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_SNORM;
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
	rs.cullMode = convert_cull_mode(pipeline->cull_mode);
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.depthClampEnable = VK_FALSE;
	rs.rasterizerDiscardEnable = VK_FALSE;
	rs.depthBiasEnable = VK_FALSE;
	rs.lineWidth = 1.0f;

	memset(&cb, 0, sizeof(cb));
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState att_state[8];
	memset(att_state, 0, sizeof(att_state));
	for (int i = 0; i < pipeline->color_attachment_count; ++i) {
		att_state[i].colorWriteMask =
			(pipeline->color_write_mask_red[i] ? VK_COLOR_COMPONENT_R_BIT : 0) |
			(pipeline->color_write_mask_green[i] ? VK_COLOR_COMPONENT_G_BIT : 0) |
			(pipeline->color_write_mask_blue[i] ? VK_COLOR_COMPONENT_B_BIT : 0) |
			(pipeline->color_write_mask_alpha[i] ? VK_COLOR_COMPONENT_A_BIT : 0);
		att_state[i].blendEnable = pipeline->blend_source != GPU_BLEND_ONE ||
								   pipeline->blend_destination != GPU_BLEND_ZERO ||
								   pipeline->alpha_blend_source != GPU_BLEND_ONE ||
								   pipeline->alpha_blend_destination != GPU_BLEND_ZERO;
		att_state[i].srcColorBlendFactor = convert_blend_factor(pipeline->blend_source);
		att_state[i].dstColorBlendFactor = convert_blend_factor(pipeline->blend_destination);
		att_state[i].colorBlendOp = convert_blend_operation(pipeline->blend_operation);
		att_state[i].srcAlphaBlendFactor = convert_blend_factor(pipeline->alpha_blend_source);
		att_state[i].dstAlphaBlendFactor = convert_blend_factor(pipeline->alpha_blend_destination);
		att_state[i].alphaBlendOp = convert_blend_operation(pipeline->alpha_blend_operation);
	}
	cb.attachmentCount = pipeline->color_attachment_count;
	cb.pAttachments = att_state;

	memset(&vp, 0, sizeof(vp));
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	dynamic_state[dynamic_state_create_info.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount = 1;
	dynamic_state[dynamic_state_create_info.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	memset(&ds, 0, sizeof(ds));
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = pipeline->depth_mode != GPU_COMPARE_MODE_ALWAYS;
	ds.depthWriteEnable = pipeline->depth_write;
	ds.depthCompareOp = convert_compare_mode(pipeline->depth_mode);
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
	pipeline->impl.vert_shader_module = create_shader_module(pipeline->vertex_shader->impl.source, pipeline->vertex_shader->impl.length);
	shaderStages[0].module = pipeline->impl.vert_shader_module;
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipeline->impl.frag_shader_module = create_shader_module(pipeline->fragment_shader->impl.source, pipeline->fragment_shader->impl.length);
	shaderStages[1].module = pipeline->impl.frag_shader_module;
	shaderStages[1].pName = "main";

	pipeline_info.pVertexInputState = &vi;
	pipeline_info.pInputAssemblyState = &ia;
	pipeline_info.pRasterizationState = &rs;
	pipeline_info.pColorBlendState = &cb;
	pipeline_info.pMultisampleState = &ms;
	pipeline_info.pViewportState = &vp;
	pipeline_info.pDepthStencilState = &ds;
	pipeline_info.pStages = shaderStages;
	pipeline_info.pDynamicState = &dynamic_state_create_info;

	VkFormat color_attachment_formats[8];
	for (int i = 0; i < pipeline->color_attachment_count; ++i) {
		color_attachment_formats[i] = convert_image_format(pipeline->color_attachment[i]);
	}

	VkPipelineRenderingCreateInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = pipeline->color_attachment_count,
		.pColorAttachmentFormats = color_attachment_formats,
		.depthAttachmentFormat = pipeline->depth_attachment_bits > 0 ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_UNDEFINED,
	};
	pipeline_info.pNext = &rendering_info;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->impl.pipeline);
	vkDestroyShaderModule(device, pipeline->impl.frag_shader_module, NULL);
	vkDestroyShaderModule(device, pipeline->impl.vert_shader_module, NULL);
}

void gpu_shader_init(gpu_shader_t *shader, const void *source, size_t length, gpu_shader_type_t type) {
	shader->impl.length = (int)length;
	shader->impl.id = 0;
	shader->impl.source = (char *)malloc(length + 1);
	for (int i = 0; i < length; ++i) {
		shader->impl.source[i] = ((char *)source)[i];
	}
	shader->impl.source[length] = 0;
}

void gpu_shader_destroy(gpu_shader_t *shader) {
	free(shader->impl.source);
	shader->impl.source = NULL;
}

static void prepare_texture_image(uint8_t *tex_colors, uint32_t width, uint32_t height, gpu_texture_impl_t *tex_obj, VkImageTiling tiling,
								  VkImageUsageFlags usage, VkFlags required_props, VkDeviceSize *deviceSize, VkFormat tex_format) {

	VkImageCreateInfo image_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = tex_format,
		.extent.width = width,
		.extent.height = height,
		.extent.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = tiling,
		.usage = usage,
		.flags = 0,
		.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
	};

	VkMemoryAllocateInfo mem_alloc = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = 0,
		.memoryTypeIndex = 0,
	};

	vkCreateImage(device, &image_create_info, NULL, &tex_obj->image);
	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(device, tex_obj->image, &mem_reqs);
	*deviceSize = mem_alloc.allocationSize = mem_reqs.size;
	memory_type_from_properties(mem_reqs.memoryTypeBits, required_props, &mem_alloc.memoryTypeIndex);
	vkAllocateMemory(device, &mem_alloc, NULL, &tex_obj->mem);
	vkBindImageMemory(device, tex_obj->image, tex_obj->mem, 0);

	if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && tex_colors != NULL) {
		VkImageSubresource subres = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.arrayLayer = 0,
		};

		VkSubresourceLayout layout;
		uint8_t *data;
		vkGetImageSubresourceLayout(device, tex_obj->image, &subres, &layout);
		vkMapMemory(device, tex_obj->mem, 0, mem_alloc.allocationSize, 0, (void **)&data);

		if (tex_format == VK_FORMAT_R8_UNORM) {
			for (uint32_t y = 0; y < height; y++) {
				for (uint32_t x = 0; x < width; x++) {
					data[y * layout.rowPitch + x] = tex_colors[y * width + x];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R32_SFLOAT) {
			uint32_t *data32 = (uint32_t *)data;
			uint32_t *tex_colors32 = (uint32_t *)tex_colors;
			for (uint32_t y = 0; y < height; y++) {
				for (uint32_t x = 0; x < width; x++) {
					data32[y * (layout.rowPitch / 4) + x] = tex_colors32[y * width + x];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R16_SFLOAT) {
			uint16_t *data16 = (uint16_t *)data;
			uint16_t *tex_colors16 = (uint16_t *)tex_colors;
			for (uint32_t y = 0; y < height; y++) {
				for (uint32_t x = 0; x < width; x++) {
					data16[y * (layout.rowPitch / 4) + x] = tex_colors16[y * width + x];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R32G32B32A32_SFLOAT) {
			uint32_t *data32 = (uint32_t *)data;
			uint32_t *tex_colors32 = (uint32_t *)tex_colors;
			for (uint32_t y = 0; y < height; y++) {
				for (uint32_t x = 0; x < width; x++) {
					data32[y * (layout.rowPitch / 4) + x * 4 + 0] = tex_colors32[y * width * 4 + x * 4 + 0];
					data32[y * (layout.rowPitch / 4) + x * 4 + 1] = tex_colors32[y * width * 4 + x * 4 + 1];
					data32[y * (layout.rowPitch / 4) + x * 4 + 2] = tex_colors32[y * width * 4 + x * 4 + 2];
					data32[y * (layout.rowPitch / 4) + x * 4 + 3] = tex_colors32[y * width * 4 + x * 4 + 3];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R16G16B16A16_SFLOAT) {
			uint16_t *data16 = (uint16_t *)data;
			uint16_t *tex_colors16 = (uint16_t *)tex_colors;
			for (uint32_t y = 0; y < height; y++) {
				for (uint32_t x = 0; x < width; x++) {
					data16[y * (layout.rowPitch / 4) + x * 4 + 0] = tex_colors16[y * width * 4 + x * 4 + 0];
					data16[y * (layout.rowPitch / 4) + x * 4 + 1] = tex_colors16[y * width * 4 + x * 4 + 1];
					data16[y * (layout.rowPitch / 4) + x * 4 + 2] = tex_colors16[y * width * 4 + x * 4 + 2];
					data16[y * (layout.rowPitch / 4) + x * 4 + 3] = tex_colors16[y * width * 4 + x * 4 + 3];
				}
			}
		}
		else if (tex_format == VK_FORMAT_B8G8R8A8_UNORM) {
			for (uint32_t y = 0; y < height; y++) {
				// uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
				for (uint32_t x = 0; x < width; x++) {
					data[y * layout.rowPitch + x * 4 + 0] = tex_colors[y * width * 4 + x * 4 + 2];
					data[y * layout.rowPitch + x * 4 + 1] = tex_colors[y * width * 4 + x * 4 + 1];
					data[y * layout.rowPitch + x * 4 + 2] = tex_colors[y * width * 4 + x * 4 + 0];
					data[y * layout.rowPitch + x * 4 + 3] = tex_colors[y * width * 4 + x * 4 + 3];
					// row[x] = tex_colors[(x & 1) ^ (y & 1)];
				}
			}
		}
		else {
			for (uint32_t y = 0; y < height; y++) {
				// uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
				for (uint32_t x = 0; x < width; x++) {
					data[y * layout.rowPitch + x * 4 + 0] = tex_colors[y * width * 4 + x * 4 + 0];
					data[y * layout.rowPitch + x * 4 + 1] = tex_colors[y * width * 4 + x * 4 + 1];
					data[y * layout.rowPitch + x * 4 + 2] = tex_colors[y * width * 4 + x * 4 + 2];
					data[y * layout.rowPitch + x * 4 + 3] = tex_colors[y * width * 4 + x * 4 + 3];
					// row[x] = tex_colors[(x & 1) ^ (y & 1)];
				}
			}
		}

		vkUnmapMemory(device, tex_obj->mem);
	}

	if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
		tex_obj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else {
		tex_obj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	set_image_layout(tex_obj->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, tex_obj->imageLayout);
}

static void update_stride(gpu_texture_t *texture) {
	VkImageSubresource subres = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0,
		.arrayLayer = 0,
	};
	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(device, texture->impl.image, &subres, &layout);
	texture->impl.stride = (int)layout.rowPitch;
}

void gpu_texture_init_from_bytes(gpu_texture_t *texture, void *data, int width, int height, gpu_texture_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->uploaded = false;
	texture->format = format;
	texture->data = data;
	texture->impl.stage = 0;
	texture->state = GPU_TEXTURE_STATE_SHADER_RESOURCE;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(gpu, tex_format, &props);

	if (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
		prepare_texture_image((uint8_t *)data, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_LINEAR,
							  VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize, tex_format);
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
		gpu_texture_impl_t staging_texture;

		memset(&staging_texture, 0, sizeof(staging_texture));
		prepare_texture_image((uint8_t *)data, (uint32_t)width, (uint32_t)height, &staging_texture, VK_IMAGE_TILING_LINEAR,
							  VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize, tex_format);
		prepare_texture_image((uint8_t *)data, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_OPTIMAL,
							  (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/),
							  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->impl.deviceSize, tex_format);

		set_image_layout(staging_texture.image, VK_IMAGE_ASPECT_COLOR_BIT, staging_texture.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, texture->impl.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageCopy copy_region = {
			.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.srcSubresource.mipLevel = 0,
			.srcSubresource.baseArrayLayer = 0,
			.srcSubresource.layerCount = 1,
			.srcOffset.x = 0,
			.srcOffset.y = 0,
			.srcOffset.z = 0,
			.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.dstSubresource.mipLevel = 0,
			.dstSubresource.baseArrayLayer = 0,
			.dstSubresource.layerCount = 1,
			.dstOffset.x = 0,
			.dstOffset.y = 0,
			.dstOffset.z = 0,
			.extent.width = (uint32_t)width,
			.extent.height = (uint32_t)height,
			.extent.depth = 1,
		};

		vkCmdCopyImage(command_buffer, staging_texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->impl.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
		set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->impl.imageLayout);
		vkDestroyImage(device, staging_texture.image, NULL);
		vkFreeMemory(device, staging_texture.mem, NULL);
	}

	update_stride(texture);

	VkImageViewCreateInfo view = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.image = VK_NULL_HANDLE,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = tex_format,
		.components.r = VK_COMPONENT_SWIZZLE_R,
		.components.g = VK_COMPONENT_SWIZZLE_G,
		.components.b = VK_COMPONENT_SWIZZLE_B,
		.components.a = VK_COMPONENT_SWIZZLE_A,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
		.flags = 0,
	};

	view.image = texture->impl.image;
	vkCreateImageView(device, &view, NULL, &texture->impl.view);
}

void gpu_texture_init(gpu_texture_t *texture, int width, int height, gpu_texture_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->uploaded = true;
	texture->format = format;
	texture->data = NULL;
	texture->impl.stage = 0;
	texture->state = GPU_TEXTURE_STATE_SHADER_RESOURCE;

	VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(gpu, tex_format, &props);

	prepare_texture_image(NULL, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_LINEAR,
						  VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize, tex_format);

	update_stride(texture);

	VkImageViewCreateInfo view = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = tex_format,
		.components.r = VK_COMPONENT_SWIZZLE_R,
		.components.g = VK_COMPONENT_SWIZZLE_G,
		.components.b = VK_COMPONENT_SWIZZLE_B,
		.components.a = VK_COMPONENT_SWIZZLE_A,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
		.flags = 0,
		.image = texture->impl.image,
	};

	vkCreateImageView(device, &view, NULL, &texture->impl.view);
}

void gpu_texture_destroy(gpu_texture_t *target) {
	if (target->impl.image != NULL) {
		vkDestroyImage(device, target->impl.image, NULL);
		vkFreeMemory(device, target->impl.mem, NULL);
	}
	if (target->impl.view != NULL) {
		vkDestroyImageView(device, target->impl.view, NULL);
	}
}

int gpu_texture_stride(gpu_texture_t *texture) {
	return texture->impl.stride;
}

void gpu_texture_generate_mipmaps(gpu_texture_t *texture, int levels) {}

void gpu_texture_set_mipmap(gpu_texture_t *texture, gpu_texture_t *mipmap, int level) {
	// VkBuffer staging_buffer;
	// VkDeviceMemory staging_buffer_mem;

	// VkBufferCreateInfo buffer_info = {
		// .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		// .size = mipmap->width * mipmap->height * format_size(mipmap->format),
		// .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		// .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	// };

	// vkCreateBuffer(device, &buffer_info, NULL, &staging_buffer);

	// VkMemoryRequirements mem_req;
	// vkGetBufferMemoryRequirements(device, staging_buffer, &mem_req);

	// VkMemoryAllocateInfo alloc_info = {
		// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		// .allocationSize = mem_req.size,
	// };

	// memory_type_from_properties(mem_req.memoryTypeBits,
	// 	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_info.memoryTypeIndex);

	// vkAllocateMemory(device, &alloc_info, NULL, &staging_buffer_mem);
	// vkBindBufferMemory(device, staging_buffer, staging_buffer_mem, 0);

	// void *mapped_data;
	// vkMapMemory(device, staging_buffer_mem, 0, buffer_info.size, 0, &mapped_data);
	// memcpy(mapped_data, mipmap->data, (size_t)buffer_info.size);
	// vkUnmapMemory(device, staging_buffer_mem);

	// set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// VkBufferImageCopy region = {
		// .bufferOffset = 0,
		// .bufferRowLength = 0,
		// .bufferImageHeight = 0,
		// .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		// .imageSubresource.mipLevel = level,
		// .imageSubresource.baseArrayLayer = 0,
		// .imageSubresource.layerCount = 1,
		// .imageOffset = (VkOffset3D){0, 0, 0},
		// .imageExtent = (VkExtent3D){(uint32_t)mipmap->width, (uint32_t)mipmap->height, 1},
	// };

	// vkCmdCopyBufferToImage(
	//     command_buffer,
	//     staging_buffer,
	//     texture->impl.image,
	//     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//     1,
	//     &region
	// );

	// set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// vkFreeMemory(device, staging_buffer_mem, NULL);
	// vkDestroyBuffer(device, staging_buffer, NULL);

	// texture->uploaded = true;
}

void gpu_render_target_init(gpu_texture_t *target, int width, int height, gpu_texture_format_t format) {
	gpu_render_target_init2(target, width, height, format, -1);
}

void gpu_vertex_buffer_init(gpu_buffer_t *buffer, int count, gpu_vertex_structure_t *structure) {
	buffer->count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		gpu_vertex_element_t element = structure->elements[i];
		buffer->impl.stride += gpu_vertex_data_size(element.data);
	}

	VkBufferCreateInfo buf_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.size = count * buffer->impl.stride,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.flags = 0,
	};

	if (gpu_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	memset(&buffer->impl.mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = 0;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	vkCreateBuffer(device, &buf_info, NULL, &buffer->impl.buf);
	VkMemoryRequirements mem_reqs = {0};
	vkGetBufferMemoryRequirements(device, buffer->impl.buf, &mem_reqs);

	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);

	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
	if (gpu_raytrace_supported()) {
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		buffer->impl.mem_alloc.pNext = &memory_allocate_flags_info;
	}

	vkAllocateMemory(device, &buffer->impl.mem_alloc, NULL, &buffer->impl.mem);
	vkBindBufferMemory(device, buffer->impl.buf, buffer->impl.mem, 0);
}

float *gpu_vertex_buffer_lock(gpu_buffer_t *buffer) {
	vkMapMemory(device, buffer->impl.mem, 0, buffer->count * buffer->impl.stride, 0, (void **)&buffer->impl.data);
	return buffer->impl.data;
}

void gpu_vertex_buffer_unlock(gpu_buffer_t *buffer) {
	vkUnmapMemory(device, buffer->impl.mem);
}

int gpu_vertex_buffer_count(gpu_buffer_t *buffer) {
	return buffer->count;
}

int gpu_vertex_buffer_stride(gpu_buffer_t *buffer) {
	return buffer->impl.stride;
}

void gpu_constant_buffer_init(gpu_buffer_t *buffer, int size) {
	buffer->impl.count = size;
	buffer->data = NULL;

	VkBufferCreateInfo buf_info;
	memset(&buf_info, 0, sizeof(buf_info));
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	if (gpu_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	buf_info.size = size;
	vkCreateBuffer(device, &buf_info, NULL, &buffer->impl.buf);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, buffer->impl.buf, &mem_reqs);

	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);
	vkAllocateMemory(device, &buffer->impl.mem_alloc, NULL, &buffer->impl.mem);
	vkBindBufferMemory(device, buffer->impl.buf, buffer->impl.mem, 0);

	if (uniform_buffer == NULL) {
		uniform_buffer = &buffer->impl.buf;
	}

	void *p;
	vkMapMemory(device, buffer->impl.mem, 0, buffer->impl.mem_alloc.allocationSize, 0, (void **)&p);
	memset(p, 0, buffer->impl.mem_alloc.allocationSize);
	vkUnmapMemory(device, buffer->impl.mem);
}

void gpu_constant_buffer_destroy(gpu_buffer_t *buffer) {
	vkFreeMemory(device, buffer->impl.mem, NULL);
	vkDestroyBuffer(device, buffer->impl.buf, NULL);
}

void gpu_constant_buffer_lock(gpu_buffer_t *buffer, int start, int count) {
	vkMapMemory(device, buffer->impl.mem, start, count, 0, (void **)&buffer->data);
}

void gpu_constant_buffer_unlock(gpu_buffer_t *buffer) {
	vkUnmapMemory(device, buffer->impl.mem);
	buffer->data = NULL;
}

int gpu_constant_buffer_size(gpu_buffer_t *buffer) {
	return buffer->impl.count;
}

void gpu_index_buffer_init(gpu_buffer_t *buffer, int count) {
	buffer->count = count;

	VkBufferCreateInfo buf_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.size = count * sizeof(uint32_t),
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.flags = 0,
	};

	if (gpu_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	memset(&buffer->impl.mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = 0;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	vkCreateBuffer(device, &buf_info, NULL, &buffer->impl.buf);

	VkMemoryRequirements mem_reqs = {0};
	vkGetBufferMemoryRequirements(device, buffer->impl.buf, &mem_reqs);

	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);

	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
	if (gpu_raytrace_supported()) {
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		buffer->impl.mem_alloc.pNext = &memory_allocate_flags_info;
	}

	vkAllocateMemory(device, &buffer->impl.mem_alloc, NULL, &buffer->impl.mem);
	vkBindBufferMemory(device, buffer->impl.buf, buffer->impl.mem, 0);
}

void gpu_buffer_destroy(gpu_buffer_t *buffer) {
	vkFreeMemory(device, buffer->impl.mem, NULL);
	vkDestroyBuffer(device, buffer->impl.buf, NULL);
}

void *gpu_index_buffer_lock(gpu_buffer_t *buffer) {
	uint8_t *data;
	vkMapMemory(device, buffer->impl.mem, 0, buffer->impl.mem_alloc.allocationSize, 0, (void **)&data);
	return data;
}

void gpu_index_buffer_unlock(gpu_buffer_t *buffer) {
	vkUnmapMemory(device, buffer->impl.mem);
}

int gpu_index_buffer_count(gpu_buffer_t *buffer) {
	return buffer->count;
}

static const int INDEX_RAYGEN = 0;
static const int INDEX_MISS = 1;
static const int INDEX_CLOSEST_HIT = 2;
static const char *raygen_shader_name = "raygeneration";
static const char *closesthit_shader_name = "closesthit";
static const char *miss_shader_name = "miss";

typedef struct inst {
	iron_matrix4x4_t m;
	int i;
} inst_t;

static VkDescriptorPool raytrace_descriptor_pool;
static gpu_raytrace_acceleration_structure_t *accel;
static gpu_raytrace_pipeline_t *pipeline;
static gpu_texture_t *output = NULL;
static gpu_texture_t *texpaint0;
static gpu_texture_t *texpaint1;
static gpu_texture_t *texpaint2;
static gpu_texture_t *texenv;
static gpu_texture_t *texsobol;
static gpu_texture_t *texscramble;
static gpu_texture_t *texrank;
static gpu_buffer_t *vb[16];
static gpu_buffer_t *vb_last[16];
static gpu_buffer_t *ib[16];
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

bool gpu_raytrace_supported() {
	#ifdef IRON_ANDROID
	return false; // Use VK_KHR_ray_query
	#else
	return true;
	#endif
}

void gpu_raytrace_pipeline_init(gpu_raytrace_pipeline_t *pipeline, void *ray_shader, int ray_shader_size, gpu_buffer_t *constant_buffer) {
	output = NULL;
	pipeline->_constant_buffer = constant_buffer;

	{
		VkDescriptorSetLayoutBinding acceleration_structure_layout_binding = {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding result_image_layout_binding = {
			.binding = 10,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding uniform_buffer_binding = {
			.binding = 11,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding ib_binding = {
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding vb_binding = {
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding tex0_binding = {
			.binding = 3,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding tex1_binding = {
			.binding = 4,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding tex2_binding = {
			.binding = 5,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding texenv_binding = {
			.binding = 6,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding texsobol_binding = {
			.binding = 7,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding texscramble_binding = {
			.binding = 8,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

		VkDescriptorSetLayoutBinding texrank_binding = {
			.binding = 9,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
		};

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

		VkDescriptorSetLayoutCreateInfo layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.bindingCount = 12,
			.pBindings = &bindings[0],
		};
		vkCreateDescriptorSetLayout(device, &layout_info, NULL, &pipeline->impl.descriptor_set_layout);

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.setLayoutCount = 1,
			.pSetLayouts = &pipeline->impl.descriptor_set_layout,
		};

		vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL, &pipeline->impl.pipeline_layout);

		VkShaderModuleCreateInfo module_create_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = ray_shader_size,
			.pCode = (const uint32_t *)ray_shader,
			.pNext = NULL,
			.flags = 0,
		};
		VkShaderModule shader_module;
		vkCreateShaderModule(device, &module_create_info, NULL, &shader_module);

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

		VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_create_info = {
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
			.pNext = NULL,
			.flags = 0,
			.stageCount = 3,
			.pStages = &shader_stages[0],
			.groupCount = 3,
			.pGroups = &groups[0],
			.maxPipelineRayRecursionDepth = 1,
			.layout = pipeline->impl.pipeline_layout,
		};
		_vkCreateRayTracingPipelinesKHR = (void *)vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR");
		_vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &raytracing_pipeline_create_info, NULL, &pipeline->impl.pipeline);
	}

	{
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_pipeline_properties;
		ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		ray_tracing_pipeline_properties.pNext = NULL;
		VkPhysicalDeviceProperties2 device_properties = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			.pNext = &ray_tracing_pipeline_properties,
		};
		vkGetPhysicalDeviceProperties2(gpu, &device_properties);

		_vkGetRayTracingShaderGroupHandlesKHR = (void *)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR");
		uint32_t handle_size = ray_tracing_pipeline_properties.shaderGroupHandleSize;
		uint32_t handle_size_aligned =
			(ray_tracing_pipeline_properties.shaderGroupHandleSize + ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1) &
			~(ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1);

		VkBufferCreateInfo buf_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = NULL,
			.size = handle_size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			.flags = 0,
		};

		vkCreateBuffer(device, &buf_info, NULL, &pipeline->impl.raygen_shader_binding_table);
		vkCreateBuffer(device, &buf_info, NULL, &pipeline->impl.hit_shader_binding_table);
		vkCreateBuffer(device, &buf_info, NULL, &pipeline->impl.miss_shader_binding_table);

		uint8_t shader_handle_storage[1024];
		_vkGetRayTracingShaderGroupHandlesKHR(device, pipeline->impl.pipeline, 0, 3, handle_size_aligned * 3, shader_handle_storage);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		};

		VkMemoryAllocateInfo memory_allocate_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &memory_allocate_flags_info,
		};

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(device, pipeline->impl.raygen_shader_binding_table, &mem_reqs);
		memory_allocate_info.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
									&memory_allocate_info.memoryTypeIndex);

		VkDeviceMemory mem;
		void *data;
		vkAllocateMemory(device, &memory_allocate_info, NULL, &mem);
		vkBindBufferMemory(device, pipeline->impl.raygen_shader_binding_table, mem, 0);
		vkMapMemory(device, mem, 0, handle_size, 0, (void **)&data);
		memcpy(data, shader_handle_storage, handle_size);
		vkUnmapMemory(device, mem);

		vkGetBufferMemoryRequirements(device, pipeline->impl.miss_shader_binding_table, &mem_reqs);
		memory_allocate_info.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memory_allocate_info.memoryTypeIndex);

		vkAllocateMemory(device, &memory_allocate_info, NULL, &mem);
		vkBindBufferMemory(device, pipeline->impl.miss_shader_binding_table, mem, 0);
		vkMapMemory(device, mem, 0, handle_size, 0, (void **)&data);
		memcpy(data, shader_handle_storage + handle_size_aligned, handle_size);
		vkUnmapMemory(device, mem);

		vkGetBufferMemoryRequirements(device, pipeline->impl.hit_shader_binding_table, &mem_reqs);
		memory_allocate_info.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memory_allocate_info.memoryTypeIndex);

		vkAllocateMemory(device, &memory_allocate_info, NULL, &mem);
		vkBindBufferMemory(device, pipeline->impl.hit_shader_binding_table, mem, 0);
		vkMapMemory(device, mem, 0, handle_size, 0, (void **)&data);
		memcpy(data, shader_handle_storage + handle_size_aligned * 2, handle_size);
		vkUnmapMemory(device, mem);
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

		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = NULL,
			.maxSets = 1024,
			.poolSizeCount = 12,
			.pPoolSizes = type_counts,
		};

		vkCreateDescriptorPool(device, &descriptor_pool_create_info, NULL, &raytrace_descriptor_pool);

		VkDescriptorSetAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = NULL,
			.descriptorPool = raytrace_descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &pipeline->impl.descriptor_set_layout,
		};
		vkAllocateDescriptorSets(device, &alloc_info, &pipeline->impl.descriptor_set);
	}
}

void gpu_raytrace_pipeline_destroy(gpu_raytrace_pipeline_t *pipeline) {
	vkDestroyPipeline(device, pipeline->impl.pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline->impl.pipeline_layout, NULL);
	vkDestroyDescriptorSetLayout(device, pipeline->impl.descriptor_set_layout, NULL);
}

uint64_t get_buffer_device_address(VkBuffer buffer) {
	VkBufferDeviceAddressInfoKHR buffer_device_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = buffer,
	};
	_vkGetBufferDeviceAddressKHR = (void *)vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR");
	return _vkGetBufferDeviceAddressKHR(device, &buffer_device_address_info);
}

void gpu_raytrace_acceleration_structure_init(gpu_raytrace_acceleration_structure_t *accel) {
	_vkGetBufferDeviceAddressKHR = (void *)vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR");
	_vkCreateAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
	_vkGetAccelerationStructureDeviceAddressKHR = (void *)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR");
	_vkGetAccelerationStructureBuildSizesKHR = (void *)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR");

	vb_count = 0;
	instances_count = 0;
}

void gpu_raytrace_acceleration_structure_add(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *_vb, gpu_buffer_t *_ib,
	iron_matrix4x4_t _transform) {

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

void _gpu_raytrace_acceleration_structure_destroy_bottom(gpu_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
	for (int i = 0; i < vb_count_last; ++i) {
		_vkDestroyAccelerationStructureKHR(device, accel->impl.bottom_level_acceleration_structure[i], NULL);
		vkFreeMemory(device, accel->impl.bottom_level_mem[i], NULL);
		vkDestroyBuffer(device, accel->impl.bottom_level_buffer[i], NULL);
	}
}

void _gpu_raytrace_acceleration_structure_destroy_top(gpu_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
	_vkDestroyAccelerationStructureKHR(device, accel->impl.top_level_acceleration_structure, NULL);
	vkFreeMemory(device, accel->impl.top_level_mem, NULL);
	vkDestroyBuffer(device, accel->impl.top_level_buffer, NULL);
	vkFreeMemory(device, accel->impl.instances_mem, NULL);
	vkDestroyBuffer(device, accel->impl.instances_buffer, NULL);
}

void gpu_raytrace_acceleration_structure_build(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *_vb_full, gpu_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_gpu_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_gpu_raytrace_acceleration_structure_destroy_top(accel);
	}

	vb_count_last = vb_count;

	if (vb_count == 0) {
		return;
	}

	// Bottom level
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {

			uint32_t prim_count = ib[i]->count / 3;
			uint32_t vert_count = vb[i]->count;

			VkDeviceOrHostAddressConstKHR vertex_data_device_address = {0};
			VkDeviceOrHostAddressConstKHR index_data_device_address = {0};

			vertex_data_device_address.deviceAddress = get_buffer_device_address(vb[i]->impl.buf);
			index_data_device_address.deviceAddress = get_buffer_device_address(ib[i]->impl.buf);

			VkAccelerationStructureGeometryKHR acceleration_geometry = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
				.geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16A16_SNORM,
				.geometry.triangles.vertexData.deviceAddress = vertex_data_device_address.deviceAddress,
				.geometry.triangles.vertexStride = vb[i]->impl.stride,
				.geometry.triangles.maxVertex = vb[i]->count,
				.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32,
				.geometry.triangles.indexData.deviceAddress = index_data_device_address.deviceAddress,
			};

			VkAccelerationStructureBuildGeometryInfoKHR acceleration_structure_build_geometry_info = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
				.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
				.geometryCount = 1,
				.pGeometries = &acceleration_geometry,
			};

			VkAccelerationStructureBuildSizesInfoKHR acceleration_build_sizes_info = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
			};
			_vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &acceleration_structure_build_geometry_info,
													&prim_count, &acceleration_build_sizes_info);

			VkBufferCreateInfo buffer_create_info = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = acceleration_build_sizes_info.accelerationStructureSize,
				.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			};
			VkBuffer bottom_level_buffer = VK_NULL_HANDLE;
			vkCreateBuffer(device, &buffer_create_info, NULL, &bottom_level_buffer);

			VkMemoryRequirements memory_requirements2;
			vkGetBufferMemoryRequirements(device, bottom_level_buffer, &memory_requirements2);

			VkMemoryAllocateFlagsInfo memory_allocate_flags_info2 = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
				.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
			};

			VkMemoryAllocateInfo memory_allocate_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = &memory_allocate_flags_info2,
				.allocationSize = memory_requirements2.size,
			};
			memory_type_from_properties(memory_requirements2.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
			VkDeviceMemory bottom_level_mem;
			vkAllocateMemory(device, &memory_allocate_info, NULL, &bottom_level_mem);
			vkBindBufferMemory(device, bottom_level_buffer, bottom_level_mem, 0);

			VkAccelerationStructureCreateInfoKHR acceleration_create_info = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
				.buffer = bottom_level_buffer,
				.size = acceleration_build_sizes_info.accelerationStructureSize,
			};
			_vkCreateAccelerationStructureKHR(device, &acceleration_create_info, NULL, &accel->impl.bottom_level_acceleration_structure[i]);

			VkBuffer scratch_buffer = VK_NULL_HANDLE;
			VkDeviceMemory scratch_memory = VK_NULL_HANDLE;

			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.size = acceleration_build_sizes_info.buildScratchSize;
			buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			vkCreateBuffer(device, &buffer_create_info, NULL, &scratch_buffer);

			VkMemoryRequirements memory_requirements;
			vkGetBufferMemoryRequirements(device, scratch_buffer, &memory_requirements);

			VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
				.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
			};

			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.pNext = &memory_allocate_flags_info;
			memory_allocate_info.allocationSize = memory_requirements.size;
			memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
			vkAllocateMemory(device, &memory_allocate_info, NULL, &scratch_memory);
			vkBindBufferMemory(device, scratch_buffer, scratch_memory, 0);

			VkBufferDeviceAddressInfoKHR buffer_device_address_info = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
				.buffer = scratch_buffer,
			};
			uint64_t scratch_buffer_device_address = _vkGetBufferDeviceAddressKHR(device, &buffer_device_address_info);

			VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
				.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
				.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
				.dstAccelerationStructure = accel->impl.bottom_level_acceleration_structure[i],
				.geometryCount = 1,
				.pGeometries = &acceleration_geometry,
				.scratchData.deviceAddress = scratch_buffer_device_address,
			};

			VkAccelerationStructureBuildRangeInfoKHR acceleration_build_range_info = {
				.primitiveCount = prim_count,
				.primitiveOffset = 0x0,
				.firstVertex = 0,
				.transformOffset = 0x0,
			};

			const VkAccelerationStructureBuildRangeInfoKHR *acceleration_build_infos[1] = {&acceleration_build_range_info};

			{
				VkCommandBufferAllocateInfo cmd_buf_allocate_info = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
					.commandPool = cmd_pool,
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = 1,
				};

				VkCommandBuffer command_buffer;
				vkAllocateCommandBuffers(device, &cmd_buf_allocate_info, &command_buffer);

				VkCommandBufferBeginInfo command_buffer_info = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				};
				vkBeginCommandBuffer(command_buffer, &command_buffer_info);

				_vkCmdBuildAccelerationStructuresKHR = (void *)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
				_vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &acceleration_build_geometry_info, &acceleration_build_infos[0]);

				vkEndCommandBuffer(command_buffer);

				VkSubmitInfo submit_info = {
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					.commandBufferCount = 1,
					.pCommandBuffers = &command_buffer,
				};

				VkFenceCreateInfo fence_info = {
					.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.flags = 0,
				};

				VkFence fence;
				vkCreateFence(device, &fence_info, NULL, &fence);

				vkQueueSubmit(queue, 1, &submit_info, fence);
				vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000);
				vkDestroyFence(device, fence, NULL);
				vkFreeCommandBuffers(device, cmd_pool, 1, &command_buffer);
			}

			VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
				.accelerationStructure = accel->impl.bottom_level_acceleration_structure[i],
			};

			accel->impl.bottom_level_acceleration_structure_handle[i] = _vkGetAccelerationStructureDeviceAddressKHR(device, &acceleration_device_address_info);

			vkFreeMemory(device, scratch_memory, NULL);
			vkDestroyBuffer(device, scratch_buffer, NULL);

			accel->impl.bottom_level_buffer[i] = bottom_level_buffer;
			accel->impl.bottom_level_mem[i] = bottom_level_mem;
		}
	}

	// Top level

	{
		VkBufferCreateInfo buf_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = NULL,
			.size = instances_count * sizeof(VkAccelerationStructureInstanceKHR),
			.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			.flags = 0,
		};

		VkMemoryAllocateInfo mem_alloc;
		memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkBuffer instances_buffer;
		vkCreateBuffer(device, &buf_info, NULL, &instances_buffer);

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(device, instances_buffer, &mem_reqs);

		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		};
		mem_alloc.pNext = &memory_allocate_flags_info;

		VkDeviceMemory instances_mem;
		vkAllocateMemory(device, &mem_alloc, NULL, &instances_mem);

		vkBindBufferMemory(device, instances_buffer, instances_mem, 0);
		void *data;
		vkMapMemory(device, instances_mem, 0, sizeof(VkAccelerationStructureInstanceKHR), 0, (void **)&data);

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
			VkAccelerationStructureInstanceKHR instance = {
				.transform = transform_matrix,
			};

			int ib_off = 0;
			for (int j = 0; j < instances[i].i; ++j) {
				ib_off += ib[j]->count * 4;
			}
			instance.instanceCustomIndex = ib_off;

			instance.mask = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = accel->impl.bottom_level_acceleration_structure_handle[instances[i].i];
			memcpy(data + i * sizeof(VkAccelerationStructureInstanceKHR), &instance, sizeof(VkAccelerationStructureInstanceKHR));
		}

		vkUnmapMemory(device, instances_mem);

		VkDeviceOrHostAddressConstKHR instance_data_device_address = {
			.deviceAddress = get_buffer_device_address(instances_buffer),
		};

		VkAccelerationStructureGeometryKHR acceleration_geometry = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
			.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
			.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.geometry.instances.arrayOfPointers = VK_FALSE,
			.geometry.instances.data.deviceAddress = instance_data_device_address.deviceAddress,
		};

		VkAccelerationStructureBuildGeometryInfoKHR acceleration_structure_build_geometry_info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.geometryCount = 1,
			.pGeometries = &acceleration_geometry,
		};

		VkAccelerationStructureBuildSizesInfoKHR acceleration_build_sizes_info = {
			acceleration_build_sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		};

		uint32_t instance_count = instances_count;

		_vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &acceleration_structure_build_geometry_info,
												 &instance_count, &acceleration_build_sizes_info);

		VkBufferCreateInfo buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = acceleration_build_sizes_info.accelerationStructureSize,
			.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		VkBuffer top_level_buffer = VK_NULL_HANDLE;
		vkCreateBuffer(device, &buffer_create_info, NULL, &top_level_buffer);

		VkMemoryRequirements memory_requirements2;
		vkGetBufferMemoryRequirements(device, top_level_buffer, &memory_requirements2);

		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memory_allocate_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &memory_allocate_flags_info,
			.allocationSize = memory_requirements2.size,
		};
		memory_type_from_properties(memory_requirements2.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
		VkDeviceMemory top_level_mem;
		vkAllocateMemory(device, &memory_allocate_info, NULL, &top_level_mem);
		vkBindBufferMemory(device, top_level_buffer, top_level_mem, 0);

		VkAccelerationStructureCreateInfoKHR acceleration_create_info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.buffer = top_level_buffer,
			.size = acceleration_build_sizes_info.accelerationStructureSize,
		};
		_vkCreateAccelerationStructureKHR(device, &acceleration_create_info, NULL, &accel->impl.top_level_acceleration_structure);

		VkBuffer scratch_buffer = VK_NULL_HANDLE;
		VkDeviceMemory scratch_memory = VK_NULL_HANDLE;

		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = acceleration_build_sizes_info.buildScratchSize;
		buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(device, &buffer_create_info, NULL, &scratch_buffer);

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(device, scratch_buffer, &memory_requirements);

		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.pNext = &memory_allocate_flags_info;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
		vkAllocateMemory(device, &memory_allocate_info, NULL, &scratch_memory);
		vkBindBufferMemory(device, scratch_buffer, scratch_memory, 0);

		VkBufferDeviceAddressInfoKHR buffer_device_address_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = scratch_buffer,
		};
		uint64_t scratch_buffer_device_address = _vkGetBufferDeviceAddressKHR(device, &buffer_device_address_info);

		VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = VK_NULL_HANDLE,
			.dstAccelerationStructure = accel->impl.top_level_acceleration_structure,
			.geometryCount = 1,
			.pGeometries = &acceleration_geometry,
			.scratchData.deviceAddress = scratch_buffer_device_address,
		};

		VkAccelerationStructureBuildRangeInfoKHR acceleration_build_range_info = {
			.primitiveCount = instances_count,
			.primitiveOffset = 0x0,
			.firstVertex = 0,
			.transformOffset = 0x0,
		};

		const VkAccelerationStructureBuildRangeInfoKHR *acceleration_build_infos[1] = {&acceleration_build_range_info};

		{
			VkCommandBufferAllocateInfo cmd_buf_allocate_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = cmd_pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(device, &cmd_buf_allocate_info, &command_buffer);

			VkCommandBufferBeginInfo command_buffer_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			};
			vkBeginCommandBuffer(command_buffer, &command_buffer_info);

			_vkCmdBuildAccelerationStructuresKHR = (void *)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
			_vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &acceleration_build_geometry_info, &acceleration_build_infos[0]);

			vkEndCommandBuffer(command_buffer);

			VkSubmitInfo submit_info = {
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers = &command_buffer,
			};

			VkFenceCreateInfo fence_info = {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = 0,
			};

			VkFence fence;
			vkCreateFence(device, &fence_info, NULL, &fence);

			vkQueueSubmit(queue, 1, &submit_info, fence);
			vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000);
			vkDestroyFence(device, fence, NULL);

			vkFreeCommandBuffers(device, cmd_pool, 1, &command_buffer);
		}

		VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = accel->impl.top_level_acceleration_structure,
		};

		accel->impl.top_level_acceleration_structure_handle = _vkGetAccelerationStructureDeviceAddressKHR(device, &acceleration_device_address_info);

		vkFreeMemory(device, scratch_memory, NULL);
		vkDestroyBuffer(device, scratch_buffer, NULL);

		accel->impl.top_level_buffer = top_level_buffer;
		accel->impl.top_level_mem = top_level_mem;
		accel->impl.instances_buffer = instances_buffer;
		accel->impl.instances_mem = instances_mem;
	}

	{
		// if (vb_full != NULL) {
		// 	vkFreeMemory(device, vb_full_mem, NULL);
		// 	vkDestroyBuffer(device, vb_full, NULL);
		// }

		// VkBufferCreateInfo buf_info = {
			// .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			// .pNext = NULL,
			// .size = vert_count * vb[0]->stride,
			// .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			// .usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			// .usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			// .usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			// .flags = 0,
		// };

		// VkMemoryAllocateInfo mem_alloc = {
			// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			// .pNext = NULL,
			// .allocationSize = 0,
			// .memoryTypeIndex = 0,
		// };

		// vkCreateBuffer(device, &buf_info, NULL, &vb_full);

		// VkMemoryRequirements mem_reqs = {0};
		// vkGetBufferMemoryRequirements(device, vb_full, &mem_reqs);

		// mem_alloc.allocationSize = mem_reqs.size;
		// memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		// VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			// .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		// };
		// mem_alloc.pNext = &memory_allocate_flags_info;

		// vkAllocateMemory(device, &mem_alloc, NULL, &vb_full_mem);
		// vkBindBufferMemory(device, vb_full, vb_full_mem, 0);

		// float *data;
		// vkMapMemory(device, vb_full_mem, 0, vert_count * vb[0]->stride, 0, (void **)&data);
		// vkUnmapMemory(device, vb_full_mem);

		////

		#ifdef is_forge

		vb_full = _vb_full->impl.buf;
		vb_full_mem = _vb_full->impl.mem;

		#else

		vb_full = vb[0]->impl.buf;
		vb_full_mem = vb[0]->impl.mem;

		#endif
	}

	{
		// if (ib_full != NULL) {
		// 	vkFreeMemory(device, ib_full_mem, NULL);
		// 	vkDestroyBuffer(device, ib_full, NULL);
		// }

		// VkBufferCreateInfo buf_info = {
			// .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			// .pNext = NULL,
			// .size = prim_count * 3 * sizeof(uint32_t),
			// .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			// .usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			// .usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			// .usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			// .flags = 0,
		// };

		// VkMemoryAllocateInfo mem_alloc = {
			// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			// .pNext = NULL,
			// .allocationSize = 0,
			// .memoryTypeIndex = 0,
		// };

		// vkCreateBuffer(device, &buf_info, NULL, &ib_full);

		// VkMemoryRequirements mem_reqs = {0};
		// vkGetBufferMemoryRequirements(device, ib_full, &mem_reqs);

		// mem_alloc.allocationSize = mem_reqs.size;
		// memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		// VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			// .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		// };
		// mem_alloc.pNext = &memory_allocate_flags_info;

		// vkAllocateMemory(device, &mem_alloc, NULL, &ib_full_mem);
		// vkBindBufferMemory(device, ib_full, ib_full_mem, 0);

		// uint8_t *data;
		// vkMapMemory(device, ib_full_mem, 0, mem_alloc.allocationSize, 0, (void **)&data);
		// for (int i = 0; i < instances_count; ++i) {
		// 	memcpy(data, ib[i]->impl., sizeof(VkAccelerationStructureInstanceKHR));
		// }
		// vkUnmapMemory(device, ib_full_mem);

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

void gpu_raytrace_acceleration_structure_destroy(gpu_raytrace_acceleration_structure_t *accel) {
	// _vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
	// for (int i = 0; i < vb_count; ++i) {
	// 	_vkDestroyAccelerationStructureKHR(device, accel->impl.bottom_level_acceleration_structure[i], NULL);
	// 	vkFreeMemory(device, accel->impl.bottom_level_mem[i], NULL);
	// 	vkDestroyBuffer(device, accel->impl.bottom_level_buffer[i], NULL);
	// }
	// _vkDestroyAccelerationStructureKHR(device, accel->impl.top_level_acceleration_structure, NULL);
	// vkFreeMemory(device, accel->impl.top_level_mem, NULL);
	// vkDestroyBuffer(device, accel->impl.top_level_buffer, NULL);
	// vkFreeMemory(device, accel->impl.instances_mem, NULL);
	// vkDestroyBuffer(device, accel->impl.instances_buffer, NULL);
}

void gpu_raytrace_set_textures(gpu_texture_t *_texpaint0, gpu_texture_t *_texpaint1, gpu_texture_t *_texpaint2, gpu_texture_t *_texenv, gpu_texture_t *_texsobol, gpu_texture_t *_texscramble, gpu_texture_t *_texrank) {
	texpaint0 = _texpaint0;
	texpaint1 = _texpaint1;
	texpaint2 = _texpaint2;
	texenv = _texenv;
	texsobol = _texsobol;
	texscramble = _texscramble;
	texrank = _texrank;
}

void gpu_raytrace_set_acceleration_structure(gpu_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void gpu_raytrace_set_pipeline(gpu_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void gpu_raytrace_set_target(gpu_texture_t *_output) {
	if (_output != output) {
		vkDestroyImage(device, _output->impl.image, NULL);

		VkImageCreateInfo image = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = _output->impl.format,
			.extent.width = _output->width,
			.extent.height = _output->height,
			.extent.depth = 1,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			.flags = 0,
		};

		vkCreateImage(device, &image, NULL, &_output->impl.image);

		vkBindImageMemory(device, _output->impl.image, _output->impl.mem, 0);

		VkImageViewCreateInfo color_image_view = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = _output->impl.format,
			.flags = 0,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
			.image = _output->impl.image,
		};
		vkCreateImageView(device, &color_image_view, NULL, &_output->impl.view);
	}
	output = _output;
}

void gpu_raytrace_dispatch_rays() {
	VkWriteDescriptorSetAccelerationStructureKHR descriptor_acceleration_structure_info = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
		.accelerationStructureCount = 1,
		.pAccelerationStructures = &accel->impl.top_level_acceleration_structure,
	};

	VkWriteDescriptorSet acceleration_structure_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = &descriptor_acceleration_structure_info,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
	};

	VkDescriptorImageInfo image_descriptor = {
		.imageView = output->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkDescriptorBufferInfo buffer_descriptor = {
		.buffer = pipeline->_constant_buffer->impl.buf,
		.range = VK_WHOLE_SIZE,
		.offset = 0,
	};

	VkWriteDescriptorSet result_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 10,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.pImageInfo = &image_descriptor,
	};

	VkWriteDescriptorSet uniform_buffer_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 11,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &buffer_descriptor,
	};

	VkDescriptorBufferInfo ib_descriptor = {
		.buffer = ib_full,
		.range = VK_WHOLE_SIZE,
		.offset = 0,
	};

	VkWriteDescriptorSet ib_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &ib_descriptor,
	};

	VkDescriptorBufferInfo vb_descriptor = {
		.buffer = vb_full,
		.range = VK_WHOLE_SIZE,
		.offset = 0,
	};

	VkWriteDescriptorSet vb_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 2,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &vb_descriptor,
	};

	VkDescriptorImageInfo tex0image_descriptor = {
		.imageView = texpaint0->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet tex0_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 3,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &tex0image_descriptor,
	};

	VkDescriptorImageInfo tex1image_descriptor = {
		.imageView = texpaint1->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet tex1_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 4,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &tex1image_descriptor,
	};

	VkDescriptorImageInfo tex2image_descriptor = {
		.imageView = texpaint2->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet tex2_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 5,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &tex2image_descriptor,
	};

	VkDescriptorImageInfo texenvimage_descriptor = {
		.imageView = texenv->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet texenv_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 6,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &texenvimage_descriptor,
	};

	VkDescriptorImageInfo texsobolimage_descriptor = {
		.imageView = texsobol->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet texsobol_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 7,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &texsobolimage_descriptor,
	};

	VkDescriptorImageInfo texscrambleimage_descriptor = {
		.imageView = texscramble->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet texscramble_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 8,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &texscrambleimage_descriptor,
	};

	VkDescriptorImageInfo texrankimage_descriptor = {
		.imageView = texrank->impl.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet texrank_image_write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = pipeline->impl.descriptor_set,
		.dstBinding = 9,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &texrankimage_descriptor,
	};

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
	vkUpdateDescriptorSets(device, 12, write_descriptor_sets, 0, VK_NULL_HANDLE);

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_pipeline_properties;
	ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	ray_tracing_pipeline_properties.pNext = NULL;
	VkPhysicalDeviceProperties2 device_properties = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = &ray_tracing_pipeline_properties,
	};
	vkGetPhysicalDeviceProperties2(gpu, &device_properties);

	// Setup the strided buffer regions pointing to the shaders in our shader binding table
	const uint32_t handle_size_aligned =
		(ray_tracing_pipeline_properties.shaderGroupHandleSize + ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1) &
		~(ray_tracing_pipeline_properties.shaderGroupHandleAlignment - 1);

	VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry = {
		.deviceAddress = get_buffer_device_address(pipeline->impl.raygen_shader_binding_table),
		.stride = handle_size_aligned,
		.size = handle_size_aligned,
	};

	VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry = {
		.deviceAddress = get_buffer_device_address(pipeline->impl.miss_shader_binding_table),
		.stride = handle_size_aligned,
		.size = handle_size_aligned,
	};

	VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry = {
		.deviceAddress = get_buffer_device_address(pipeline->impl.hit_shader_binding_table),
		.stride = handle_size_aligned,
		.size = handle_size_aligned,
	};

	VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry = {0};

	vkCmdEndRendering(command_buffer);

	// Dispatch the ray tracing commands
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline_layout, 0, 1,
							&pipeline->impl.descriptor_set, 0, 0);

	_vkCmdTraceRaysKHR = (void *)vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
	_vkCmdTraceRaysKHR(command_buffer, &raygen_shader_sbt_entry, &miss_shader_sbt_entry, &hit_shader_sbt_entry, &callable_shader_sbt_entry,
					   output->width, output->height, 1);

	vkCmdBeginRendering(command_buffer, &current_rendering_info);
}
