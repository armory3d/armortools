
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

#define MAX_THINGS 256
#define MAX_DESCRIPTOR_SETS 1024
#define MAX_PRESENT_MODES 256
#define GET_INSTANCE_PROC_ADDR(instance, entrypoint)                                                                                                           \
	{                                                                                                                                                          \
		vk.fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(instance, "vk" #entrypoint);                                                             \
		if (vk.fp##entrypoint == NULL) {                                                                                                                       \
			iron_error("vkGetInstanceProcAddr failed to find vk" #entrypoint);                                                                         \
		}                                                                                                                                                      \
	}

bool iron_gpu_transpose_mat = true;

static iron_gpu_texture_t *vulkan_textures[16] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static VkSampler vulkan_samplers[16] = {
	VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
	VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
    VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
	VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE
};

static iron_gpu_texture_t *current_render_targets[8] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static VkSemaphore framebuffer_available;
static VkSemaphore relay_semaphore;
static bool wait_for_relay = false;
static int framebuffer_count = 0;
static VkRenderPassBeginInfo current_render_pass_begin_info;
static VkPipeline current_vulkan_pipeline;
static uint32_t last_vertex_constant_buffer_offset = 0;
static uint32_t last_fragment_constant_buffer_offset = 0;
static iron_gpu_pipeline_t *current_pipeline = NULL;
static int mrt_index = 0;
static VkFramebuffer mrt_framebuffer[16];
static VkRenderPass mrt_render_pass[16];
static bool on_back_buffer = false;
static bool in_render_pass = false;
static bool wait_for_framebuffer = false;
static VkDescriptorSetLayout desc_layout;
static VkDescriptorPool descriptor_pool;
static struct descriptor_set descriptor_sets[MAX_DESCRIPTOR_SETS] = {0};
static int descriptor_sets_count = 0;

static struct indexed_name names[MAX_THINGS];
static uint32_t names_size = 0;
static struct indexed_name member_names[MAX_THINGS];
static uint32_t member_names_size = 0;
static struct indexed_index locs[MAX_THINGS];
static uint32_t locs_size = 0;
static struct indexed_index bindings[MAX_THINGS];
static uint32_t bindings_size = 0;
static struct indexed_index offsets[MAX_THINGS];
static uint32_t offsets_size = 0;

static bool began = false;
static VkPhysicalDeviceProperties gpu_props;
static VkQueueFamilyProperties *queue_props;
static uint32_t graphics_queue_node_index;
static VkPhysicalDeviceMemoryProperties memory_properties;
static uint32_t queue_count;
static VkPresentModeKHR present_modes[MAX_PRESENT_MODES];

struct vk_funs vk = {0};
struct vk_context vk_ctx = {0};

void iron_vulkan_get_instance_extensions(const char **extensions, int *index);
VkBool32 iron_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physical_device, uint32_t queue_family_index);
VkResult iron_vulkan_create_surface(VkInstance instance, VkSurfaceKHR *surface);
void iron_gpu_internal_resize(int, int);

VkBool32 vkDebugUtilsMessengerCallbackEXT(
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

void memory_type_from_properties(uint32_t type_bits, VkFlags requirements_mask, uint32_t *type_index) {
	for (uint32_t i = 0; i < 32; i++) {
		if ((type_bits & 1) == 1) {
			if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*type_index = i;
			}
		}
		type_bits >>= 1;
	}
}

void setup_init_cmd() {
	if (vk_ctx.setup_cmd == VK_NULL_HANDLE) {
		VkCommandBufferAllocateInfo cmd = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = vk_ctx.cmd_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		vkAllocateCommandBuffers(vk_ctx.device, &cmd, &vk_ctx.setup_cmd);

		VkCommandBufferBeginInfo cmd_buf_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = 0,
			.pInheritanceInfo = NULL,
		};

		vkBeginCommandBuffer(vk_ctx.setup_cmd, &cmd_buf_info);
	}
}

void flush_init_cmd() {
	if (vk_ctx.setup_cmd == VK_NULL_HANDLE) {
		return;
	}

	vkEndCommandBuffer(vk_ctx.setup_cmd);

	const VkCommandBuffer cmd_bufs[] = { vk_ctx.setup_cmd };
	VkFence null_fence = { VK_NULL_HANDLE };
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = NULL,
		.pWaitDstStageMask = NULL,
		.commandBufferCount = 1,
		.pCommandBuffers = cmd_bufs,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = NULL,
	};

	vkQueueSubmit(vk_ctx.queue, 1, &submit_info, null_fence);
	vkQueueWaitIdle(vk_ctx.queue);

	vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, cmd_bufs);
	vk_ctx.setup_cmd = VK_NULL_HANDLE;
}

void set_image_layout(VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_layout, VkImageLayout new_layout) {
	setup_init_cmd();

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
	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	vkCmdPipelineBarrier(vk_ctx.setup_cmd, src_stage_mask, dst_stage_mask, 0, 0, NULL, 0, NULL, 1, &barrier);
	flush_init_cmd();
}

void create_descriptor_layout(void) {
	VkDescriptorSetLayoutBinding bindings[18];
	memset(bindings, 0, sizeof(bindings));

	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = NULL;

	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = NULL;

	for (int i = 2; i < 18; ++i) {
		bindings[i].binding = i;
		bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		bindings[i].pImmutableSamplers = NULL;
	}

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.bindingCount = 18,
		.pBindings = bindings,
	};

	vkCreateDescriptorSetLayout(vk_ctx.device, &descriptor_layout, NULL, &desc_layout);

	VkDescriptorPoolSize type_counts[2];
	memset(type_counts, 0, sizeof(type_counts));

	type_counts[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	type_counts[0].descriptorCount = 2 * 1024;

	type_counts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	type_counts[1].descriptorCount = 16 * 1024;

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.maxSets = 1024,
		.poolSizeCount = 2,
		.pPoolSizes = type_counts,
	};

	vkCreateDescriptorPool(vk_ctx.device, &pool_info, NULL, &descriptor_pool);
}

void iron_internal_resize(int width, int height) {
	struct vk_window *window = &vk_ctx.windows[0];
	if (window->width != width || window->height != height) {
		window->resized = true;
		window->width = width;
		window->height = height;
	}
	iron_gpu_internal_resize(width, height);
}

VkSwapchainKHR cleanup_swapchain() {
	struct vk_window *window = &vk_ctx.windows[0];

	if (window->framebuffer_render_pass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(vk_ctx.device, window->framebuffer_render_pass, NULL);
	}

	if (window->framebuffers) {
		for (uint32_t i = 0; i < window->image_count; i++) {
			vkDestroyFramebuffer(vk_ctx.device, window->framebuffers[i], NULL);
		}
		free(window->framebuffers);
		window->framebuffers = NULL;
	}

	if (window->depth.image != VK_NULL_HANDLE) {
		vkDestroyImageView(vk_ctx.device, window->depth.view, NULL);
		vkDestroyImage(vk_ctx.device, window->depth.image, NULL);
		vkFreeMemory(vk_ctx.device, window->depth.memory, NULL);
		window->depth.image = VK_NULL_HANDLE;
		window->depth.memory = VK_NULL_HANDLE;
		window->depth.view = VK_NULL_HANDLE;
	}

	if (window->images) {
		for (uint32_t i = 0; i < window->image_count; i++) {
			vkDestroyImageView(vk_ctx.device, window->views[i], NULL);
		}
		free(window->images);
		free(window->views);
		window->images = NULL;
		window->views = NULL;
	}

	VkSwapchainKHR chain = window->swapchain;
	window->swapchain = VK_NULL_HANDLE;
	return chain;
}

void create_swapchain() {
	struct vk_window *window = &vk_ctx.windows[0];

	VkSwapchainKHR old_swapchain = cleanup_swapchain();
	if (window->surface_destroyed) {
		vk.fpDestroySwapchainKHR(vk_ctx.device, old_swapchain, NULL);
		old_swapchain = VK_NULL_HANDLE;
		vk.fpDestroySurfaceKHR(vk_ctx.instance, window->surface, NULL);
		iron_vulkan_create_surface(vk_ctx.instance, &window->surface);
		window->width = iron_window_width();
		window->height = iron_window_height();
		window->surface_destroyed = false;
	}

	// Check the surface capabilities and formats
	VkSurfaceCapabilitiesKHR caps = {0};
	vk.fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_ctx.gpu, window->surface, &caps);

	uint32_t present_mode_count;
	vk.fpGetPhysicalDeviceSurfacePresentModesKHR(vk_ctx.gpu, window->surface, &present_mode_count, NULL);
	present_mode_count = present_mode_count > MAX_PRESENT_MODES ? MAX_PRESENT_MODES : present_mode_count;
	vk.fpGetPhysicalDeviceSurfacePresentModesKHR(vk_ctx.gpu, window->surface, &present_mode_count, present_modes);

	VkExtent2D swapchain_extent;
	if (caps.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchain_extent.width = window->width;
		swapchain_extent.height = window->height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchain_extent = caps.currentExtent;
		window->width = caps.currentExtent.width;
		window->height = caps.currentExtent.height;
	}

	VkPresentModeKHR swapchain_present_mode = window->vsynced ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t image_count = caps.minImageCount + 1;
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

	VkSwapchainCreateInfoKHR swapchain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.surface = window->surface,
		.minImageCount = image_count,
		.imageFormat = window->format.format,
		.imageColorSpace = window->format.colorSpace,
		.imageExtent.width = swapchain_extent.width,
		.imageExtent.height = swapchain_extent.height,
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
	swapchain_info.presentMode = swapchain_present_mode;
	swapchain_info.oldSwapchain = old_swapchain;
	swapchain_info.clipped = true;

	vk.fpCreateSwapchainKHR(vk_ctx.device, &swapchain_info, NULL, &window->swapchain);

	if (old_swapchain != VK_NULL_HANDLE) {
		vk.fpDestroySwapchainKHR(vk_ctx.device, old_swapchain, NULL);
	}

	vk.fpGetSwapchainImagesKHR(vk_ctx.device, window->swapchain, &window->image_count, NULL);
	window->images = (VkImage *)malloc(window->image_count * sizeof(VkImage));

	vk.fpGetSwapchainImagesKHR(vk_ctx.device, window->swapchain, &window->image_count, window->images);
	window->views = (VkImageView *)malloc(window->image_count * sizeof(VkImageView));

	for (uint32_t i = 0; i < window->image_count; i++) {
		VkImageViewCreateInfo color_attachment_view = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.format = window->format.format,
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
		set_image_layout(window->images[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		color_attachment_view.image = window->images[i];
		vkCreateImageView(vk_ctx.device, &color_attachment_view, NULL, &window->views[i]);
	}

	window->current_image = 0;

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	if (window->depth_bits > 0) {
		VkImageCreateInfo image = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = NULL,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = depth_format,
			.extent.width = window->width,
			.extent.height = window->height,
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
			.format = depth_format,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
			.flags = 0,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
		};

		VkMemoryRequirements mem_reqs = {0};
		vkCreateImage(vk_ctx.device, &image, NULL, &window->depth.image);
		vkGetImageMemoryRequirements(vk_ctx.device, window->depth.image, &mem_reqs);
		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);
		vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &window->depth.memory);
		vkBindImageMemory(vk_ctx.device, window->depth.image, window->depth.memory, 0);
		set_image_layout(window->depth.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		view.image = window->depth.image;
		vkCreateImageView(vk_ctx.device, &view, NULL, &window->depth.view);
	}

	VkAttachmentDescription attachments[2];
	attachments[0].format = window->format.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].flags = 0;

	if (window->depth_bits > 0) {
		attachments[1].format = depth_format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;
	}

	VkAttachmentReference color_reference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference depth_reference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.flags = 0,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_reference,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = window->depth_bits > 0 ? &depth_reference : NULL,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL,
	};

	VkSubpassDependency dependencies[2];
	memset(&dependencies, 0, sizeof(dependencies));

	// for the frame-buffer
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo rp_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.attachmentCount = window->depth_bits > 0 ? 2 : 1,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 2,
		.pDependencies = dependencies,
	};

	vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->framebuffer_render_pass);

	VkImageView attachment_views[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};

	if (window->depth_bits > 0) {
		attachment_views[1] = window->depth.view;
	}

	VkFramebufferCreateInfo fb_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = NULL,
		.renderPass = window->framebuffer_render_pass,
		.attachmentCount = window->depth_bits > 0 ? 2 : 1,
		.pAttachments = attachment_views,
		.width = window->width,
		.height = window->height,
		.layers = 1,
	};

	window->framebuffers = (VkFramebuffer *)malloc(window->image_count * sizeof(VkFramebuffer));
	for (uint32_t i = 0; i < window->image_count; i++) {
		attachment_views[0] = window->views[i];
		vkCreateFramebuffer(vk_ctx.device, &fb_info, NULL, &window->framebuffers[i]);
	}

	flush_init_cmd();
}

void create_render_target_render_pass(struct vk_window *window) {
	VkAttachmentDescription attachments[2];
	attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM; // target->impl.format; // TODO
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[0].flags = 0;

	VkAttachmentReference color_reference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.flags = 0,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_reference,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = NULL,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL,
	};

	// for render-targets
	VkSubpassDependency dependencies[2];
	memset(&dependencies, 0, sizeof(dependencies));

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

	VkRenderPassCreateInfo rp_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.attachmentCount = 1,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 2,
		.pDependencies = dependencies,
	};

	vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->rendertarget_render_pass);

	// depthBufferBits > 0
	{
		attachments[1].format = VK_FORMAT_D16_UNORM;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;
		VkAttachmentReference depth_reference = {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		subpass.pDepthStencilAttachment = &depth_reference;
		rp_info.attachmentCount = 2;
		vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->rendertarget_render_pass_with_depth);
	}
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

void iron_gpu_internal_init() {
	uint32_t instance_layer_count = 0;

	static const char *wanted_instance_layers[64];
	int wanted_instance_layer_count = 0;

	vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);

	if (instance_layer_count > 0) {
		VkLayerProperties *instance_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * instance_layer_count);
		vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);

#ifdef VALIDATE
		vk_ctx.validation_found = find_layer(instance_layers, instance_layer_count, "VK_LAYER_KHRONOS_validation");
		if (vk_ctx.validation_found) {
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
	bool missing_instance_extensions =
	    check_extensions(wanted_instance_extensions, wanted_instance_extension_count, instance_extensions, instance_extension_count);

	if (missing_instance_extensions) {
		iron_error("");
	}

#ifdef VALIDATE
	// this extension should be provided by the validation layers
	if (vk_ctx.validation_found) {
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
	};

#ifdef IRON_ANDROID
	app.apiVersion = VK_API_VERSION_1_0; // VK_API_VERSION_1_1
#else
	app.apiVersion = VK_API_VERSION_1_2; // VKRT
#endif

	VkInstanceCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = NULL;
	info.pApplicationInfo = &app;
#ifdef VALIDATE
	if (vk_ctx.validation_found) {
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

	VkResult err = vkCreateInstance(&info, NULL, &vk_ctx.instance);
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
	vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, NULL);

	if (gpu_count > 0) {
		VkPhysicalDevice *physical_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, physical_devices);

		// The device with the highest score is chosen.
		float best_score = 0.0;
		for (uint32_t gpu_idx = 0; gpu_idx < gpu_count; gpu_idx++) {
			VkPhysicalDevice gpu = physical_devices[gpu_idx];
			uint32_t queue_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);
			VkQueueFamilyProperties *queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);
			bool can_present = false;
			bool can_render = false;
			for (uint32_t i = 0; i < queue_count; i++) {
				VkBool32 queue_supports_present = iron_vulkan_get_physical_device_presentation_support(gpu, i);
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
			vkGetPhysicalDeviceProperties(gpu, &properties);
			switch (properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score = 2;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score = 1;
				break;
			}

			if (vk_ctx.gpu == VK_NULL_HANDLE || score > best_score) {
				vk_ctx.gpu = gpu;
				best_score = score;
			}
		}

		if (vk_ctx.gpu == VK_NULL_HANDLE) {
			iron_error("No Vulkan device that supports presentation found");
		}

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(vk_ctx.gpu, &properties);
		iron_log("Chosen Vulkan device: %s", properties.deviceName);
		free(physical_devices);
	}
	else {
		iron_error("No Vulkan device found");
	}

	static const char *wanted_device_layers[64];
	int wanted_device_layer_count = 0;

	uint32_t device_layer_count = 0;
	vkEnumerateDeviceLayerProperties(vk_ctx.gpu, &device_layer_count, NULL);

	if (device_layer_count > 0) {
		VkLayerProperties *device_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * device_layer_count);
		vkEnumerateDeviceLayerProperties(vk_ctx.gpu, &device_layer_count, device_layers);

#ifdef VALIDATE
		vk_ctx.validation_found = find_layer(device_layers, device_layer_count, "VK_LAYER_KHRONOS_validation");
		if (vk_ctx.validation_found) {
			wanted_device_layers[wanted_device_layer_count++] = "VK_LAYER_KHRONOS_validation";
		}
#endif

		free(device_layers);
	}

	const char *wanted_device_extensions[64];
	int wanted_device_extension_count = 0;

	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	if (iron_gpu_raytrace_supported()) {
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
	}

	uint32_t device_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, NULL);

	VkExtensionProperties *device_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * device_extension_count);
	vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, device_extensions);

	bool missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);
	free(device_extensions);

	if (missing_device_extensions) {
		exit(1);
	}

#ifdef VALIDATE
	if (vk_ctx.validation_found) {
		GET_INSTANCE_PROC_ADDR(vk_ctx.instance, CreateDebugUtilsMessengerEXT);

		VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.flags = 0,
			.pfnUserCallback = vkDebugUtilsMessengerCallbackEXT,
			.pUserData = NULL,
			.pNext = NULL,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		};
		vk.fpCreateDebugUtilsMessengerEXT(vk_ctx.instance, &dbgCreateInfo, NULL, &vk_ctx.debug_messenger);
	}
#endif

	// Having these GIPA queries of vk_ctx.device extension entry points both
	// BEFORE and AFTER vkCreateDevice is a good test for the loader
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, CreateSwapchainKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, DestroySwapchainKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, DestroySurfaceKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, GetSwapchainImagesKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, AcquireNextImageKHR);
	GET_INSTANCE_PROC_ADDR(vk_ctx.instance, QueuePresentKHR);

	vkGetPhysicalDeviceProperties(vk_ctx.gpu, &gpu_props);

	// Query with NULL data to get count
	vkGetPhysicalDeviceQueueFamilyProperties(vk_ctx.gpu, &queue_count, NULL);

	queue_props = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(vk_ctx.gpu, &queue_count, queue_props);

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *supports_present = (VkBool32 *)malloc(queue_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < queue_count; i++) {
		supports_present[i] = iron_vulkan_get_physical_device_presentation_support(vk_ctx.gpu, i);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
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
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (uint32_t i = 0; i < queue_count; ++i) {
			if (supports_present[i] == VK_TRUE) {
				present_queue_node_index = i;
				break;
			}
		}
	}
	free(supports_present);

	// Generate error if could not find both a graphics and a present queue
	if (graphics_queue_node_index == UINT32_MAX || present_queue_node_index == UINT32_MAX) {
		iron_error("Graphics or present queue not found");
	}

	if (graphics_queue_node_index != present_queue_node_index) {
		iron_error("Graphics and present queue do not match");
	}

	graphics_queue_node_index = graphics_queue_node_index;

	{
		float queue_priorities[1] = { 0.0 };
		VkDeviceQueueCreateInfo queue = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.queueFamilyIndex = graphics_queue_node_index,
			.queueCount = 1,
			.pQueuePriorities = queue_priorities,
		};

		VkDeviceCreateInfo deviceinfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = NULL,
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
		if (iron_gpu_raytrace_supported()) {
			raytracing_pipeline_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			raytracing_pipeline_ext.pNext = NULL;
			raytracing_pipeline_ext.rayTracingPipeline = VK_TRUE;

			raytracing_acceleration_structure_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			raytracing_acceleration_structure_ext.pNext = &raytracing_pipeline_ext;
			raytracing_acceleration_structure_ext.accelerationStructure = VK_TRUE;

			buffer_device_address_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
			buffer_device_address_ext.pNext = &raytracing_acceleration_structure_ext;
			buffer_device_address_ext.bufferDeviceAddress = VK_TRUE;

			deviceinfo.pNext = &buffer_device_address_ext;
		}

		vkCreateDevice(vk_ctx.gpu, &deviceinfo, NULL, &vk_ctx.device);
	}

	vkGetDeviceQueue(vk_ctx.device, graphics_queue_node_index, 0, &vk_ctx.queue);
	vkGetPhysicalDeviceMemoryProperties(vk_ctx.gpu, &memory_properties);
	VkCommandPoolCreateInfo cmd_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.queueFamilyIndex = graphics_queue_node_index,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	};

	vkCreateCommandPool(vk_ctx.device, &cmd_pool_info, NULL, &vk_ctx.cmd_pool);
	create_descriptor_layout();

	VkSemaphoreCreateInfo sem_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};

	vkCreateSemaphore(vk_ctx.device, &sem_info, NULL, &framebuffer_available);
	vkCreateSemaphore(vk_ctx.device, &sem_info, NULL, &relay_semaphore);
}

void iron_gpu_internal_destroy() {}

void iron_vulkan_init_window() {
	// this function is used in the android backend
	struct vk_window *window = &vk_ctx.windows[0];

	// delay swapchain/surface recreation
	// otherwise trouble ensues due to G4onG5 backend ending the command list in gpu_begin
	window->resized = true;
	window->surface_destroyed = true;
}

void iron_gpu_internal_init_window(int depthBufferBits, bool vsync) {
	struct vk_window *window = &vk_ctx.windows[0];

	window->depth_bits = depthBufferBits;
	window->vsynced = vsync;

	iron_vulkan_create_surface(vk_ctx.instance, &window->surface);

	VkBool32 surface_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(vk_ctx.gpu, graphics_queue_node_index, window->surface, &surface_supported);

	uint32_t formatCount;
	vk.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_ctx.gpu, window->surface, &formatCount, NULL);

	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	vk.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_ctx.gpu, window->surface, &formatCount, surfFormats);

	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		window->format = surfFormats[0];
	}
	else {
		bool found = false;
		for (uint32_t i = 0; i < formatCount; ++i) {
			// VK_FORMAT_B8G8R8A8_SRGB causes gamma-correction, making things bright
			if (surfFormats[i].format != VK_FORMAT_B8G8R8A8_SRGB) {
				window->format = surfFormats[i];
				found = true;
				break;
			}
		}
		if (!found) {
			window->format = surfFormats[0];
		}
	}
	free(surfFormats);
	window->width = iron_window_width();
	window->height = iron_window_height();
	create_swapchain();
	create_render_target_render_pass(window);

	began = false;
	iron_gpu_begin(NULL);
}

void iron_gpu_internal_destroy_window() {
	struct vk_window *window = &vk_ctx.windows[0];
	VkSwapchainKHR swapchain = cleanup_swapchain();
	vkDestroyRenderPass(vk_ctx.device, window->rendertarget_render_pass, NULL);
	vk.fpDestroySwapchainKHR(vk_ctx.device, swapchain, NULL);
	vk.fpDestroySurfaceKHR(vk_ctx.instance, window->surface, NULL);
}

bool iron_gpu_swap_buffers() {
	return true;
}

void iron_gpu_begin(iron_gpu_texture_t *renderTarget) {
	struct vk_window *window = &vk_ctx.windows[0];

	if (began) {
		return;
	}

	if (window->resized) {
		vkDeviceWaitIdle(vk_ctx.device);
		create_swapchain();
	}

	// Get the index of the next available swapchain image:
	wait_for_framebuffer = true;
	VkResult err = -1;
	do {
		err = vk.fpAcquireNextImageKHR(vk_ctx.device, window->swapchain, UINT64_MAX, framebuffer_available, VK_NULL_HANDLE, &window->current_image);
		if (err == VK_ERROR_SURFACE_LOST_KHR || err == VK_ERROR_OUT_OF_DATE_KHR) {
			window->surface_destroyed = (err == VK_ERROR_SURFACE_LOST_KHR);
			create_swapchain();
		}
		else {
			began = true;
			if (renderTarget != NULL) {
				renderTarget->impl.framebuffer = window->framebuffers[window->current_image];
			}
			return;
		}
	}
	while (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR);
}

void reuse_descriptor_sets(void) {
	for (int i = 0; i < descriptor_sets_count; ++i) {
		descriptor_sets[i].in_use = false;
	}
}

void iron_gpu_end() {
	VkPresentInfoKHR present = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.swapchainCount = 1,
		.pSwapchains = &vk_ctx.windows[0].swapchain,
		.pImageIndices = &vk_ctx.windows[0].current_image,
		.pWaitSemaphores = &relay_semaphore,
		.waitSemaphoreCount = 1,
	};
	wait_for_relay = false;

	VkResult err = vk.fpQueuePresentKHR(vk_ctx.queue, &present);
	if (err == VK_ERROR_SURFACE_LOST_KHR) {
		vkDeviceWaitIdle(vk_ctx.device);
		vk_ctx.windows[0].surface_destroyed = true;
		create_swapchain();
	}
	else if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		vkDeviceWaitIdle(vk_ctx.device);
		create_swapchain();
	}

	reuse_descriptor_sets();
	began = false;
}

void iron_gpu_flush() {
	vkDeviceWaitIdle(vk_ctx.device);
}

bool iron_vulkan_internal_get_size(int *width, int *height) {
	// this is exclusively used by the Android backend at the moment
	if (vk_ctx.windows[0].surface) {
		VkSurfaceCapabilitiesKHR capabilities;
		vk.fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_ctx.gpu, vk_ctx.windows[0].surface, &capabilities);
		*width = capabilities.currentExtent.width;
		*height = capabilities.currentExtent.height;
		return true;
	}
	else {
		return false;
	}
}

bool iron_gpu_raytrace_supported() {
#ifdef IRON_ANDROID
	return false;
#else
	return true;
#endif
}

int iron_gpu_max_bound_textures(void) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(vk_ctx.gpu, &props);
	return props.limits.maxPerStageDescriptorSamplers;
}


static void end_pass(iron_gpu_command_list_t *list) {
	if (in_render_pass) {
		vkCmdEndRenderPass(list->impl._buffer);
		in_render_pass = false;
	}

	for (int i = 0; i < 16; ++i) {
		vulkan_textures[i] = NULL;
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

void set_viewport_and_scissor(iron_gpu_command_list_t *list) {
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));

	if (current_render_targets[0] == NULL || current_render_targets[0]->framebuffer_index >= 0) {
		viewport.x = 0;
		viewport.y = (float)iron_window_height();
		viewport.width = (float)iron_window_width();
		viewport.height = -(float)iron_window_height();
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		scissor.extent.width = iron_window_width();
		scissor.extent.height = iron_window_height();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
	}
	else {
		viewport.x = 0;
		viewport.y = (float)current_render_targets[0]->height;
		viewport.width = (float)current_render_targets[0]->width;
		viewport.height = -(float)current_render_targets[0]->height;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		scissor.extent.width = current_render_targets[0]->width;
		scissor.extent.height = current_render_targets[0]->height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
	}

	vkCmdSetViewport(list->impl._buffer, 0, 1, &viewport);
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);
}

void iron_gpu_command_list_init(iron_gpu_command_list_t *list) {
	VkCommandBufferAllocateInfo cmd = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = vk_ctx.cmd_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	vkAllocateCommandBuffers(vk_ctx.device, &cmd, &list->impl._buffer);

	VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	vkCreateFence(vk_ctx.device, &fenceInfo, NULL, &list->impl.fence);

	list->impl._indexCount = 0;
}

void iron_gpu_command_list_destroy(iron_gpu_command_list_t *list) {
	vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, &list->impl._buffer);
	vkDestroyFence(vk_ctx.device, list->impl.fence, NULL);
}

void iron_gpu_command_list_begin(iron_gpu_command_list_t *list) {
	vkWaitForFences(vk_ctx.device, 1, &list->impl.fence, VK_TRUE, UINT64_MAX);

	vkResetCommandBuffer(list->impl._buffer, 0);
	VkCommandBufferBeginInfo cmd_buf_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};

	VkClearValue clear_values[2];
	memset(clear_values, 0, sizeof(VkClearValue) * 2);
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;
	if (vk_ctx.windows[0].depth_bits > 0) {
		clear_values[1].depthStencil.depth = 1.0;
		clear_values[1].depthStencil.stencil = 0;
	}

	VkRenderPassBeginInfo rp_begin = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = vk_ctx.windows[0].framebuffer_render_pass,
		.framebuffer = vk_ctx.windows[0].framebuffers[vk_ctx.windows[0].current_image],
		.renderArea.offset.x = 0,
		.renderArea.offset.y = 0,
		.renderArea.extent.width = vk_ctx.windows[0].width,
		.renderArea.extent.height = vk_ctx.windows[0].height,
		.clearValueCount = vk_ctx.windows[0].depth_bits > 0 ? 2 : 1,
		.pClearValues = clear_values,
	};

	vkBeginCommandBuffer(list->impl._buffer, &cmd_buf_info);

	VkImageMemoryBarrier prePresentBarrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
		.image = vk_ctx.windows[0].images[vk_ctx.windows[0].current_image],
	};

	VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
	vkCmdPipelineBarrier(list->impl._buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1,
	                     pmemory_barrier);

	vkCmdBeginRenderPass(list->impl._buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	current_render_pass_begin_info = rp_begin;
	in_render_pass = true;

	set_viewport_and_scissor(list);

	on_back_buffer = true;

	for (int i = 0; i < mrt_index; ++i) {
		vkDestroyFramebuffer(vk_ctx.device, mrt_framebuffer[i], NULL);
		vkDestroyRenderPass(vk_ctx.device, mrt_render_pass[i], NULL);
	}
	mrt_index = 0;
}

void iron_gpu_command_list_end(iron_gpu_command_list_t *list) {
	vkCmdEndRenderPass(list->impl._buffer);

	VkImageMemoryBarrier prePresentBarrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
	};

	prePresentBarrier.image = vk_ctx.windows[0].images[vk_ctx.windows[0].current_image];
	VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
	vkCmdPipelineBarrier(list->impl._buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);

	vkEndCommandBuffer(list->impl._buffer);
}

void iron_gpu_command_list_clear(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget, unsigned flags, unsigned color, float depth) {
	VkClearRect clearRect = {
		.rect.offset.x = 0,
		.rect.offset.y = 0,
		.rect.extent.width = renderTarget->width,
		.rect.extent.height = renderTarget->height,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	int count = 0;
	VkClearAttachment attachments[2];
	if (flags & IRON_GPU_CLEAR_COLOR) {
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
	if ((flags & IRON_GPU_CLEAR_DEPTH) && renderTarget->impl.depthBufferBits > 0) {
		attachments[count].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachments[count].clearValue.depthStencil.depth = depth;
		attachments[count].clearValue.depthStencil.stencil = 0;
		count++;
	}
	vkCmdClearAttachments(list->impl._buffer, count, attachments, 1, &clearRect);
}

void iron_gpu_command_list_render_target_to_framebuffer_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {}

void iron_gpu_command_list_framebuffer_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {}

void iron_gpu_command_list_draw_indexed_vertices(iron_gpu_command_list_t *list) {
	iron_gpu_command_list_draw_indexed_vertices_from_to(list, 0, list->impl._indexCount);
}

void iron_gpu_command_list_draw_indexed_vertices_from_to(iron_gpu_command_list_t *list, int start, int count) {
	vkCmdDrawIndexed(list->impl._buffer, count, 1, start, 0, 0);
}

void iron_gpu_command_list_viewport(iron_gpu_command_list_t *list, int x, int y, int width, int height) {
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.x = (float)x;
	viewport.y = y + (float)height;
	viewport.width = (float)width;
	viewport.height = (float)-height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(list->impl._buffer, 0, 1, &viewport);
}

void iron_gpu_command_list_scissor(iron_gpu_command_list_t *list, int x, int y, int width, int height) {
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = x;
	scissor.offset.y = y;
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);
}

void iron_gpu_command_list_disable_scissor(iron_gpu_command_list_t *list) {
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	if (current_render_targets[0] == NULL || current_render_targets[0]->framebuffer_index >= 0) {
		scissor.extent.width = iron_window_width();
		scissor.extent.height = iron_window_height();
	}
	else {
		scissor.extent.width = current_render_targets[0]->width;
		scissor.extent.height = current_render_targets[0]->height;
	}
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);
}

void iron_gpu_command_list_set_pipeline(iron_gpu_command_list_t *list, struct iron_gpu_pipeline *pipeline) {
	current_pipeline = pipeline;
	last_vertex_constant_buffer_offset = 0;
	last_fragment_constant_buffer_offset = 0;

	if (on_back_buffer) {
		current_vulkan_pipeline = current_pipeline->impl.framebuffer_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.framebuffer_pipeline);
	}
	else {
		current_vulkan_pipeline = current_pipeline->impl.rendertarget_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.rendertarget_pipeline);
	}
}

void iron_gpu_command_list_set_vertex_buffer(iron_gpu_command_list_t *list, gpu_vertex_buffer_impl_t *vertexBuffer) {
	VkBuffer buffers[1];
	VkDeviceSize offsets[1];
	buffers[0] = vertexBuffer->buf;
	offsets[0] = (VkDeviceSize)(0);
	vkCmdBindVertexBuffers(list->impl._buffer, 0, 1, buffers, offsets);
}

void iron_gpu_command_list_set_index_buffer(iron_gpu_command_list_t *list, struct iron_gpu_index_buffer *indexBuffer) {
	list->impl._indexCount = iron_gpu_index_buffer_count(indexBuffer);
	vkCmdBindIndexBuffer(list->impl._buffer, indexBuffer->impl.buf, 0, VK_INDEX_TYPE_UINT32);
}

void iron_internal_restore_render_target(iron_gpu_command_list_t *list, struct iron_gpu_texture *target) {
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.x = 0;
	viewport.y = (float)iron_window_height();
	viewport.width = (float)iron_window_width();
	viewport.height = -(float)iron_window_height();
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(list->impl._buffer, 0, 1, &viewport);
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = iron_window_width();
	scissor.extent.height = iron_window_height();
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);

	if (on_back_buffer && in_render_pass) {
		return;
	}

	end_pass(list);

	current_render_targets[0] = NULL;
	on_back_buffer = true;

	VkClearValue clear_values[2];
	memset(clear_values, 0, sizeof(VkClearValue) * 2);
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].depthStencil.depth = 1.0;
	clear_values[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rp_begin = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = vk_ctx.windows[0].framebuffer_render_pass,
		.framebuffer = vk_ctx.windows[0].framebuffers[vk_ctx.windows[0].current_image],
		.renderArea.offset.x = 0,
		.renderArea.offset.y = 0,
		.renderArea.extent.width = iron_window_width(),
		.renderArea.extent.height = iron_window_height(),
		.clearValueCount = 2,
		.pClearValues = clear_values,
	};

	vkCmdBeginRenderPass(list->impl._buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	current_render_pass_begin_info = rp_begin;
	in_render_pass = true;

	if (current_pipeline != NULL) {
		current_vulkan_pipeline = current_pipeline->impl.framebuffer_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.framebuffer_pipeline);
	}
}

void iron_gpu_command_list_set_render_targets(iron_gpu_command_list_t *list, struct iron_gpu_texture **targets, int count) {
	for (int i = 0; i < count; ++i) {
		current_render_targets[i] = targets[i];
	}
	for (int i = count; i < 8; ++i) {
		current_render_targets[i] = NULL;
	}

	if (targets[0]->framebuffer_index >= 0) {
		iron_internal_restore_render_target(list, targets[0]);
		return;
	}

	end_pass(list);

	on_back_buffer = false;

	VkClearValue clear_values[9];
	memset(clear_values, 0, sizeof(VkClearValue));
	for (int i = 0; i < count; ++i) {
		clear_values[i].color.float32[0] = 0.0f;
		clear_values[i].color.float32[1] = 0.0f;
		clear_values[i].color.float32[2] = 0.0f;
		clear_values[i].color.float32[3] = 1.0f;
	}
	clear_values[count].depthStencil.depth = 1.0;
	clear_values[count].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rp_begin = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderArea.offset.x = 0,
		.renderArea.offset.y = 0,
		.renderArea.extent.width = targets[0]->width,
		.renderArea.extent.height = targets[0]->height,
		.clearValueCount = count + 1,
		.pClearValues = clear_values,
	};

	if (count == 1) {
		if (targets[0]->impl.depthBufferBits > 0) {
			rp_begin.renderPass = vk_ctx.windows[0].rendertarget_render_pass_with_depth;
		}
		else {
			rp_begin.renderPass = vk_ctx.windows[0].rendertarget_render_pass;
		}
		rp_begin.framebuffer = targets[0]->impl.framebuffer;
	}
	else {
		VkAttachmentDescription attachments[9];
		for (int i = 0; i < count; ++i) {
			attachments[i].format = targets[i]->impl.format;
			attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			attachments[i].flags = 0;
		}

		if (targets[0]->impl.depthBufferBits > 0) {
			attachments[count].format = VK_FORMAT_D16_UNORM;
			attachments[count].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[count].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[count].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[count].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[count].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments[count].flags = 0;
		}

		VkAttachmentReference color_references[8];
		for (int i = 0; i < count; ++i) {
			color_references[i].attachment = i;
			color_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference depth_reference = {
			.attachment = count,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		VkSubpassDescription subpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.flags = 0,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = count,
			.pColorAttachments = color_references,
			.pResolveAttachments = NULL,
			.pDepthStencilAttachment = targets[0]->impl.depthBufferBits > 0 ? &depth_reference : NULL,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL,
		};

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

		VkRenderPassCreateInfo rp_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = NULL,
			.attachmentCount = targets[0]->impl.depthBufferBits > 0 ? count + 1 : count,
			.pAttachments = attachments,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 2,
			.pDependencies = dependencies,
		};

		vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &mrt_render_pass[mrt_index]);

		VkImageView attachmentsViews[9];
		for (int i = 0; i < count; ++i) {
			attachmentsViews[i] = targets[i]->impl.view;
		}
		if (targets[0]->impl.depthBufferBits > 0) {
			attachmentsViews[count] = targets[0]->impl.depthView;
		}

		VkFramebufferCreateInfo fbufCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.renderPass = mrt_render_pass[mrt_index],
			.attachmentCount = targets[0]->impl.depthBufferBits > 0 ? count + 1 : count,
			.pAttachments = attachmentsViews,
			.width = targets[0]->width,
			.height = targets[0]->height,
			.layers = 1,
		};

		vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &mrt_framebuffer[mrt_index]);

		rp_begin.renderPass = mrt_render_pass[mrt_index];
		rp_begin.framebuffer = mrt_framebuffer[mrt_index];
		mrt_index++;
	}

	vkCmdBeginRenderPass(list->impl._buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	current_render_pass_begin_info = rp_begin;
	in_render_pass = true;

	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.x = 0;
	viewport.y = (float)targets[0]->height;
	viewport.width = (float)targets[0]->width;
	viewport.height = -(float)targets[0]->height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(list->impl._buffer, 0, 1, &viewport);

	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = targets[0]->width;
	scissor.extent.height = targets[0]->height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);

	if (current_pipeline != NULL) {
		current_vulkan_pipeline = current_pipeline->impl.rendertarget_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.rendertarget_pipeline);
	}
}

void iron_gpu_command_list_upload_index_buffer(iron_gpu_command_list_t *list, struct iron_gpu_index_buffer *buffer) {}

void iron_gpu_command_list_upload_vertex_buffer(iron_gpu_command_list_t *list, struct iron_gpu_vertex_buffer *buffer) {}

void iron_gpu_command_list_upload_texture(iron_gpu_command_list_t *list, struct iron_gpu_texture *texture) {}

void iron_gpu_command_list_get_render_target_pixels(iron_gpu_command_list_t *list, iron_gpu_texture_t *render_target, uint8_t *data) {
	VkFormat format = render_target->impl.format;
	int format_bytes_size = format_size(format);

	// Create readback buffer
	if (!render_target->impl.readback_buffer_created) {
		VkBufferCreateInfo buf_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = NULL,
			.size = render_target->width * render_target->height * format_bytes_size,
			.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.flags = 0,
		};
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &render_target->impl.readback_buffer);

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(vk_ctx.device, render_target->impl.readback_buffer, &mem_reqs);

		VkMemoryAllocateInfo mem_alloc;
		memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;
		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
		vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &render_target->impl.readback_memory);
		vkBindBufferMemory(vk_ctx.device, render_target->impl.readback_buffer, render_target->impl.readback_memory, 0);

		render_target->impl.readback_buffer_created = true;
	}

	vkCmdEndRenderPass(list->impl._buffer);

	set_image_layout(render_target->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

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
	vkCmdCopyImageToBuffer(list->impl._buffer, render_target->impl.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, render_target->impl.readback_buffer, 1,
	                       &region);

	set_image_layout(render_target->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	vkCmdBeginRenderPass(list->impl._buffer, &current_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	in_render_pass = true;

	iron_gpu_command_list_end(list);
	iron_gpu_command_list_execute(list);
	iron_gpu_command_list_wait_for_execution_to_finish(list);
	iron_gpu_command_list_begin(list);

	// Read buffer
	void *p;
	vkMapMemory(vk_ctx.device, render_target->impl.readback_memory, 0, VK_WHOLE_SIZE, 0, (void **)&p);
	memcpy(data, p, render_target->width * render_target->height * format_bytes_size);
	vkUnmapMemory(vk_ctx.device, render_target->impl.readback_memory);
}

void iron_gpu_command_list_texture_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {
	// render-passes are used to transition render-targets
}

void iron_gpu_command_list_render_target_to_texture_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {
	// render-passes are used to transition render-targets
}

void iron_gpu_command_list_set_vertex_constant_buffer(iron_gpu_command_list_t *list, struct iron_gpu_constant_buffer *buffer, int offset, size_t size) {
	last_vertex_constant_buffer_offset = offset;
}

int calc_descriptor_id(void) {
	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkan_textures[i] != NULL) {
			texture_count++;
		}
	}

	bool uniform_buffer = vk_ctx.vertex_uniform_buffer != NULL && vk_ctx.fragment_uniform_buffer != NULL;

	return 1 | (texture_count << 1) | ((uniform_buffer ? 1 : 0) << 8);
}

static int write_tex_descs(VkDescriptorImageInfo *tex_descs) {
	memset(tex_descs, 0, sizeof(VkDescriptorImageInfo) * 16);

	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkan_textures[i] != NULL) {
			tex_descs[i].sampler = vulkan_samplers[i];
			if (vulkan_textures[i]->impl.stage_depth == i) {
				tex_descs[i].imageView = vulkan_textures[i]->impl.depthView;
				vulkan_textures[i]->impl.stage_depth = -1;
			}
			else {
				tex_descs[i].imageView = vulkan_textures[i]->impl.view;
			}
			texture_count++;
		}
		tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	return texture_count;
}

static bool textures_changed(struct descriptor_set *set) {
	VkDescriptorImageInfo tex_desc[16];
	write_tex_descs(tex_desc);
	return memcmp(&tex_desc, &set->tex_desc, sizeof(tex_desc)) != 0;
}

static void update_textures(struct descriptor_set *set) {
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

	if (vulkan_textures[0] != NULL) {
		vkUpdateDescriptorSets(vk_ctx.device, texture_count, writes, 0, NULL);
	}
}

VkDescriptorSet get_descriptor_set() {
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

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &desc_layout,
	};
	VkDescriptorSet descriptor_set;
	vkAllocateDescriptorSets(vk_ctx.device, &alloc_info, &descriptor_set);

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
		if (vulkan_textures[i] != NULL) {
			tex_desc[i].sampler = vulkan_samplers[i];
			if (vulkan_textures[i]->impl.stage_depth == i) {
				tex_desc[i].imageView = vulkan_textures[i]->impl.depthView;
				vulkan_textures[i]->impl.stage_depth = -1;
			}
			else {
				tex_desc[i].imageView = vulkan_textures[i]->impl.view;
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

	if (vulkan_textures[0] != NULL) {
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

	descriptor_sets[descriptor_sets_count].id = id;
	descriptor_sets[descriptor_sets_count].in_use = true;
	descriptor_sets[descriptor_sets_count].set = descriptor_set;
	write_tex_descs(descriptor_sets[descriptor_sets_count].tex_desc);
	descriptor_sets_count += 1;

	return descriptor_set;
}

void iron_gpu_command_list_set_fragment_constant_buffer(iron_gpu_command_list_t *list, struct iron_gpu_constant_buffer *buffer, int offset, size_t size) {
	last_fragment_constant_buffer_offset = offset;

	VkDescriptorSet descriptor_set = get_descriptor_set();
	uint32_t offsets[2] = {last_vertex_constant_buffer_offset, last_fragment_constant_buffer_offset};
	vkCmdBindDescriptorSets(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.pipeline_layout, 0, 1, &descriptor_set, 2, offsets);
}

void iron_gpu_command_list_execute(iron_gpu_command_list_t *list) {
	// Make sure the previous execution is done, so we can reuse the fence
	// Not optimal of course
	vkWaitForFences(vk_ctx.device, 1, &list->impl.fence, VK_TRUE, UINT64_MAX);
	vkResetFences(vk_ctx.device, 1, &list->impl.fence);

	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
	};

	VkSemaphore semaphores[2] = { framebuffer_available, relay_semaphore };
	VkPipelineStageFlags dst_stage_flags[2] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
	if (wait_for_framebuffer) {
		submit_info.pWaitSemaphores = semaphores;
		submit_info.pWaitDstStageMask = dst_stage_flags;
		submit_info.waitSemaphoreCount = wait_for_relay ? 2 : 1;
		wait_for_framebuffer = false;
	}
	else if (wait_for_relay) {
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &semaphores[1];
		submit_info.pWaitDstStageMask = &dst_stage_flags[1];
	}

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &list->impl._buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &relay_semaphore;
	wait_for_relay = true;

	vkQueueSubmit(vk_ctx.queue, 1, &submit_info, list->impl.fence);
}

void iron_gpu_command_list_wait_for_execution_to_finish(iron_gpu_command_list_t *list) {
	vkWaitForFences(vk_ctx.device, 1, &list->impl.fence, VK_TRUE, UINT64_MAX);
}

void iron_gpu_command_list_set_sampler(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_sampler_t *sampler) {
	if (unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] >= 0) {
		vulkan_samplers[unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT]] = sampler->impl.sampler;
	}
}

void iron_gpu_command_list_set_texture(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *texture) {
	if (unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] >= 0) {
		texture->impl.stage = unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT];
		vulkan_textures[unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT]] = texture;
	}
	else if (unit.stages[IRON_GPU_SHADER_TYPE_VERTEX] >= 0) {
		texture->impl.stage = unit.stages[IRON_GPU_SHADER_TYPE_VERTEX];
		vulkan_textures[unit.stages[IRON_GPU_SHADER_TYPE_VERTEX]] = texture;
	}
}

void iron_gpu_command_list_set_texture_from_render_target_depth(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *target) {
	if (unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] >= 0) {
		target->impl.stage_depth = unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT];
		vulkan_textures[unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT]] = target;
	}
	else if (unit.stages[IRON_GPU_SHADER_TYPE_VERTEX] >= 0) {
		target->impl.stage_depth = unit.stages[IRON_GPU_SHADER_TYPE_VERTEX];
		vulkan_textures[unit.stages[IRON_GPU_SHADER_TYPE_VERTEX]] = target;
	}
}

static bool has_number(iron_internal_named_number *named_numbers, const char *name) {
	for (int i = 0; i < IRON_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (strcmp(named_numbers[i].name, name) == 0) {
			return true;
		}
	}
	return false;
}

static uint32_t find_number(iron_internal_named_number *named_numbers, const char *name) {
	for (int i = 0; i < IRON_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (strcmp(named_numbers[i].name, name) == 0) {
			return named_numbers[i].number;
		}
	}
	return -1;
}

static void set_number(iron_internal_named_number *named_numbers, const char *name, uint32_t number) {
	for (int i = 0; i < IRON_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (strcmp(named_numbers[i].name, name) == 0) {
			named_numbers[i].number = number;
			return;
		}
	}

	for (int i = 0; i < IRON_INTERNAL_NAMED_NUMBER_COUNT; ++i) {
		if (named_numbers[i].name[0] == 0) {
			strcpy(named_numbers[i].name, name);
			named_numbers[i].number = number;
			return;
		}
	}
}

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
	member_names[member_names_size].id = id;
	member_names[member_names_size].name = name;
	++member_names_size;
}

static char *find_member_name(uint32_t id) {
	for (uint32_t i = 0; i < member_names_size; ++i) {
		if (member_names[i].id == id) {
			return member_names[i].name;
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

static void parse_shader(uint32_t *shader_source, int shader_length, iron_internal_named_number *locations, iron_internal_named_number *textureBindings,
                         iron_internal_named_number *uniformOffsets) {
	names_size = 0;
	member_names_size = 0;
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
	vkCreateShaderModule(vk_ctx.device, &moduleCreateInfo, NULL, &module);

	return module;
}

static VkShaderModule prepare_vs(VkShaderModule *vert_shader_module, iron_gpu_shader_t *vertex_shader) {
	*vert_shader_module = create_shader_module(vertex_shader->impl.source, vertex_shader->impl.length);
	return *vert_shader_module;
}

static VkShaderModule prepare_fs(VkShaderModule *frag_shader_module, iron_gpu_shader_t *fragment_shader) {
	*frag_shader_module = create_shader_module(fragment_shader->impl.source, fragment_shader->impl.length);
	return *frag_shader_module;
}

static VkFormat convert_image_format(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case IRON_IMAGE_FORMAT_RGBA64:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case IRON_IMAGE_FORMAT_R8:
		return VK_FORMAT_R8_UNORM;
	case IRON_IMAGE_FORMAT_R16:
		return VK_FORMAT_R16_SFLOAT;
	case IRON_IMAGE_FORMAT_R32:
		return VK_FORMAT_R32_SFLOAT;
	case IRON_IMAGE_FORMAT_BGRA32:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case IRON_IMAGE_FORMAT_RGBA32:
		return VK_FORMAT_R8G8B8A8_UNORM;
	default:
		return VK_FORMAT_B8G8R8A8_UNORM;
	}
}

void iron_gpu_pipeline_init(iron_gpu_pipeline_t *pipeline) {
	iron_gpu_internal_pipeline_init(pipeline);
	iron_gpu_internal_pipeline_set_defaults(pipeline);
}

void iron_gpu_pipeline_destroy(iron_gpu_pipeline_t *pipeline) {
	vkDestroyPipeline(vk_ctx.device, pipeline->impl.framebuffer_pipeline, NULL);
	vkDestroyPipeline(vk_ctx.device, pipeline->impl.rendertarget_pipeline, NULL);
	vkDestroyPipelineLayout(vk_ctx.device, pipeline->impl.pipeline_layout, NULL);
}

iron_gpu_constant_location_t iron_gpu_pipeline_get_constant_location(iron_gpu_pipeline_t *pipeline, const char *name) {
	iron_gpu_constant_location_t location;
	location.impl.vertexOffset = -1;
	location.impl.fragmentOffset = -1;
	if (has_number(pipeline->impl.vertexOffsets, name)) {
		location.impl.vertexOffset = find_number(pipeline->impl.vertexOffsets, name);
	}
	if (has_number(pipeline->impl.fragmentOffsets, name)) {
		location.impl.fragmentOffset = find_number(pipeline->impl.fragmentOffsets, name);
	}
	return location;
}

iron_gpu_texture_unit_t iron_gpu_pipeline_get_texture_unit(iron_gpu_pipeline_t *pipeline, const char *name) {
	iron_gpu_texture_unit_t unit;
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	int number = find_number(pipeline->impl.vertexTextureBindings, name);
	if (number >= 0) {
		unit.stages[IRON_GPU_SHADER_TYPE_VERTEX] = number - 2;
	}
	else {
		number = find_number(pipeline->impl.fragmentTextureBindings, name);
		if (number >= 0) {
			unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] = number - 2;
		}
	}

	return unit;
}

static VkCullModeFlagBits convert_cull_mode(iron_gpu_cull_mode_t cull_mode) {
	switch (cull_mode) {
	case IRON_GPU_CULL_MODE_CLOCKWISE:
		return VK_CULL_MODE_BACK_BIT;
	case IRON_GPU_CULL_MODE_COUNTERCLOCKWISE:
		return VK_CULL_MODE_FRONT_BIT;
	case IRON_GPU_CULL_MODE_NEVER:
	default:
		return VK_CULL_MODE_NONE;
	}
}

static VkCompareOp convert_compare_mode(iron_gpu_compare_mode_t compare) {
	switch (compare) {
	default:
	case IRON_GPU_COMPARE_MODE_ALWAYS:
		return VK_COMPARE_OP_ALWAYS;
	case IRON_GPU_COMPARE_MODE_NEVER:
		return VK_COMPARE_OP_NEVER;
	case IRON_GPU_COMPARE_MODE_EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case IRON_GPU_COMPARE_MODE_NOT_EQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case IRON_GPU_COMPARE_MODE_LESS:
		return VK_COMPARE_OP_LESS;
	case IRON_GPU_COMPARE_MODE_LESS_EQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case IRON_GPU_COMPARE_MODE_GREATER:
		return VK_COMPARE_OP_GREATER;
	case IRON_GPU_COMPARE_MODE_GREATER_EQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	}
}

static VkBlendFactor convert_blend_factor(iron_gpu_blending_factor_t factor) {
	switch (factor) {
	case IRON_GPU_BLEND_ONE:
		return VK_BLEND_FACTOR_ONE;
	case IRON_GPU_BLEND_ZERO:
		return VK_BLEND_FACTOR_ZERO;
	case IRON_GPU_BLEND_SOURCE_ALPHA:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case IRON_GPU_BLEND_DEST_ALPHA:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case IRON_GPU_BLEND_INV_SOURCE_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case IRON_GPU_BLEND_INV_DEST_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case IRON_GPU_BLEND_SOURCE_COLOR:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case IRON_GPU_BLEND_DEST_COLOR:
		return VK_BLEND_FACTOR_DST_COLOR;
	case IRON_GPU_BLEND_INV_SOURCE_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case IRON_GPU_BLEND_INV_DEST_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case IRON_GPU_BLEND_CONSTANT:
		return VK_BLEND_FACTOR_CONSTANT_COLOR;
	case IRON_GPU_BLEND_INV_CONSTANT:
		return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	}
}

static VkBlendOp convert_blend_operation(iron_gpu_blending_operation_t op) {
	switch (op) {
	case IRON_GPU_BLENDOP_ADD:
		return VK_BLEND_OP_ADD;
	case IRON_GPU_BLENDOP_SUBTRACT:
		return VK_BLEND_OP_SUBTRACT;
	case IRON_GPU_BLENDOP_REVERSE_SUBTRACT:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	case IRON_GPU_BLENDOP_MIN:
		return VK_BLEND_OP_MIN;
	case IRON_GPU_BLENDOP_MAX:
		return VK_BLEND_OP_MAX;
	}
}

void iron_gpu_pipeline_compile(iron_gpu_pipeline_t *pipeline) {
	memset(pipeline->impl.vertexLocations, 0, sizeof(iron_internal_named_number) * IRON_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.vertexOffsets, 0, sizeof(iron_internal_named_number) * IRON_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentLocations, 0, sizeof(iron_internal_named_number) * IRON_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentOffsets, 0, sizeof(iron_internal_named_number) * IRON_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.vertexTextureBindings, 0, sizeof(iron_internal_named_number) * IRON_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentTextureBindings, 0, sizeof(iron_internal_named_number) * IRON_INTERNAL_NAMED_NUMBER_COUNT);
	parse_shader((uint32_t *)pipeline->vertex_shader->impl.source, pipeline->vertex_shader->impl.length, pipeline->impl.vertexLocations,
	             pipeline->impl.vertexTextureBindings, pipeline->impl.vertexOffsets);
	parse_shader((uint32_t *)pipeline->fragment_shader->impl.source, pipeline->fragment_shader->impl.length, pipeline->impl.fragmentLocations,
	             pipeline->impl.fragmentTextureBindings, pipeline->impl.fragmentOffsets);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.setLayoutCount = 1,
		.pSetLayouts = &desc_layout,
	};

	vkCreatePipelineLayout(vk_ctx.device, &pPipelineLayoutCreateInfo, NULL, &pipeline->impl.pipeline_layout);

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
		iron_gpu_vertex_element_t element = pipeline->input_layout->elements[i];

		vi_attrs[attr].binding = 0;
		vi_attrs[attr].location = find_number(pipeline->impl.vertexLocations, element.name);
		vi_attrs[attr].offset = offset;
		offset += iron_gpu_vertex_data_size(element.data);

		switch (element.data) {
		case IRON_GPU_VERTEX_DATA_F32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_SFLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_F32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_SFLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_F32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_F32_4X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_U8_4X_NORM:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case IRON_GPU_VERTEX_DATA_I16_2X_NORM:
			vi_attrs[attr].format = VK_FORMAT_R16G16_SNORM;
			break;
		case IRON_GPU_VERTEX_DATA_I16_4X_NORM:
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
		att_state[i].blendEnable = pipeline->blend_source != IRON_GPU_BLEND_ONE ||
								   pipeline->blend_destination != IRON_GPU_BLEND_ZERO ||
		                           pipeline->alpha_blend_source != IRON_GPU_BLEND_ONE ||
								   pipeline->alpha_blend_destination != IRON_GPU_BLEND_ZERO;
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
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount = 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	memset(&ds, 0, sizeof(ds));
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = pipeline->depth_mode != IRON_GPU_COMPARE_MODE_ALWAYS;
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
	shaderStages[0].module = prepare_vs(&pipeline->impl.vert_shader_module, pipeline->vertex_shader);
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = prepare_fs(&pipeline->impl.frag_shader_module, pipeline->fragment_shader);
	shaderStages[1].pName = "main";

	pipeline_info.pVertexInputState = &vi;
	pipeline_info.pInputAssemblyState = &ia;
	pipeline_info.pRasterizationState = &rs;
	pipeline_info.pColorBlendState = &cb;
	pipeline_info.pMultisampleState = &ms;
	pipeline_info.pViewportState = &vp;
	pipeline_info.pDepthStencilState = &ds;
	pipeline_info.pStages = shaderStages;
	pipeline_info.renderPass = vk_ctx.windows[0].framebuffer_render_pass;
	pipeline_info.pDynamicState = &dynamicState;

	vkCreateGraphicsPipelines(vk_ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->impl.framebuffer_pipeline);

	VkAttachmentDescription attachments[9];
	for (int i = 0; i < pipeline->color_attachment_count; ++i) {
		attachments[i].format = convert_image_format(pipeline->color_attachment[i]);
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachments[i].flags = 0;
	}

	if (pipeline->depth_attachment_bits > 0) {
		attachments[pipeline->color_attachment_count].format = VK_FORMAT_D16_UNORM;
		attachments[pipeline->color_attachment_count].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[pipeline->color_attachment_count].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[pipeline->color_attachment_count].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[pipeline->color_attachment_count].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[pipeline->color_attachment_count].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[pipeline->color_attachment_count].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[pipeline->color_attachment_count].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[pipeline->color_attachment_count].flags = 0;
	}

	VkAttachmentReference color_references[8];
	for (int i = 0; i < pipeline->color_attachment_count; ++i) {
		color_references[i].attachment = i;
		color_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentReference depth_reference = {
		.attachment = pipeline->color_attachment_count,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.flags = 0,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = pipeline->color_attachment_count,
		.pColorAttachments = color_references,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = pipeline->depth_attachment_bits > 0 ? &depth_reference : NULL,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL,
	};

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

	VkRenderPassCreateInfo rp_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.attachmentCount = pipeline->depth_attachment_bits > 0 ? pipeline->color_attachment_count + 1 : pipeline->color_attachment_count,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 2,
		.pDependencies = dependencies,
	};

	VkRenderPass render_pass;
	vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &render_pass);

	pipeline_info.renderPass = render_pass;

	vkCreateGraphicsPipelines(vk_ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->impl.rendertarget_pipeline);

	vkDestroyShaderModule(vk_ctx.device, pipeline->impl.frag_shader_module, NULL);
	vkDestroyShaderModule(vk_ctx.device, pipeline->impl.vert_shader_module, NULL);
}

void iron_gpu_shader_init(iron_gpu_shader_t *shader, const void *source, size_t length, iron_gpu_shader_type_t type) {
	shader->impl.length = (int)length;
	shader->impl.id = 0;
	shader->impl.source = (char *)malloc(length + 1);
	for (int i = 0; i < length; ++i) {
		shader->impl.source[i] = ((char *)source)[i];
	}
	shader->impl.source[length] = 0;
}

void iron_gpu_shader_destroy(iron_gpu_shader_t *shader) {
	free(shader->impl.source);
	shader->impl.source = NULL;
}

static VkSamplerAddressMode convert_addressing(iron_gpu_texture_addressing_t mode) {
	switch (mode) {
	case IRON_GPU_TEXTURE_ADDRESSING_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case IRON_GPU_TEXTURE_ADDRESSING_BORDER:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case IRON_GPU_TEXTURE_ADDRESSING_CLAMP:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case IRON_GPU_TEXTURE_ADDRESSING_MIRROR:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	default:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

static VkSamplerMipmapMode convert_mipmap_mode(iron_gpu_mipmap_filter_t filter) {
	switch (filter) {
	case IRON_GPU_MIPMAP_FILTER_NONE:
	case IRON_GPU_MIPMAP_FILTER_POINT:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case IRON_GPU_MIPMAP_FILTER_LINEAR:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	default:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
}

static VkFilter convert_texture_filter(iron_gpu_texture_filter_t filter) {
	switch (filter) {
	case IRON_GPU_TEXTURE_FILTER_POINT:
		return VK_FILTER_NEAREST;
	case IRON_GPU_TEXTURE_FILTER_LINEAR:
		return VK_FILTER_LINEAR;
	case IRON_GPU_TEXTURE_FILTER_ANISOTROPIC:
		return VK_FILTER_LINEAR; // ?
	default:
		return VK_FILTER_NEAREST;
	}
}

void iron_gpu_sampler_init(iron_gpu_sampler_t *sampler, const iron_gpu_sampler_options_t *options) {
	VkSamplerCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.addressModeU = convert_addressing(options->u_addressing),
		.addressModeV = convert_addressing(options->v_addressing),
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipmapMode = convert_mipmap_mode(options->mipmap_filter),
		.magFilter = convert_texture_filter(options->magnification_filter),
		.minFilter = convert_texture_filter(options->minification_filter),
		.anisotropyEnable = (options->magnification_filter == IRON_GPU_TEXTURE_FILTER_ANISOTROPIC || options->minification_filter == IRON_GPU_TEXTURE_FILTER_ANISOTROPIC),
		.maxAnisotropy = 1,
		.maxLod = 32,
		.minLod = 0,
	};

	vkCreateSampler(vk_ctx.device, &info, NULL, &sampler->impl.sampler);
}

void iron_gpu_sampler_destroy(iron_gpu_sampler_t *sampler) {
	vkDestroySampler(vk_ctx.device, sampler->impl.sampler, NULL);
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

	VkMemoryRequirements mem_reqs;

	vkCreateImage(vk_ctx.device, &image_create_info, NULL, &tex_obj->image);

	vkGetImageMemoryRequirements(vk_ctx.device, tex_obj->image, &mem_reqs);

	*deviceSize = mem_alloc.allocationSize = mem_reqs.size;
	memory_type_from_properties(mem_reqs.memoryTypeBits, required_props, &mem_alloc.memoryTypeIndex);

	// allocate memory
	vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &tex_obj->mem);

	// bind memory
	vkBindImageMemory(vk_ctx.device, tex_obj->image, tex_obj->mem, 0);

	if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && tex_colors != NULL) {
		VkImageSubresource subres = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.arrayLayer = 0,
		};

		VkSubresourceLayout layout;
		uint8_t *data;

		vkGetImageSubresourceLayout(vk_ctx.device, tex_obj->image, &subres, &layout);

		vkMapMemory(vk_ctx.device, tex_obj->mem, 0, mem_alloc.allocationSize, 0, (void **)&data);

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

		vkUnmapMemory(vk_ctx.device, tex_obj->mem);
	}

	if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
		tex_obj->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	else {
		tex_obj->imageLayout = VK_IMAGE_LAYOUT_GENERAL; // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	set_image_layout(tex_obj->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, tex_obj->imageLayout);
}

static void update_stride(iron_gpu_texture_t *texture) {
	VkImageSubresource subres = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0,
		.arrayLayer = 0,
	};

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(vk_ctx.device, texture->impl.image, &subres, &layout);

	texture->impl.stride = (int)layout.rowPitch;
}

void iron_gpu_texture_init_from_bytes(iron_gpu_texture_t *texture, void *data, int width, int height, iron_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->_uploaded = false;
	texture->format = format;
	texture->data = data;
	texture->impl.stage = 0;
	texture->impl.stage_depth = -1;
	texture->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	texture->framebuffer_index = -1;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	if (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
		// Device can texture using linear textures
		prepare_texture_image((uint8_t *)data, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_LINEAR,
		                      VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize,
		                      tex_format);

		flush_init_cmd();
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
		// Must use staging buffer to copy linear texture to optimized
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

		vkCmdCopyImage(vk_ctx.setup_cmd, staging_texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->impl.image,
		               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

		set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->impl.imageLayout);

		vkDestroyImage(vk_ctx.device, staging_texture.image, NULL);
		vkFreeMemory(vk_ctx.device, staging_texture.mem, NULL);

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
	vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.view);
}

void iron_gpu_texture_init(iron_gpu_texture_t *texture, int width, int height, iron_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->_uploaded = true;
	texture->format = format;
	texture->data = NULL;
	texture->impl.stage = 0;
	texture->impl.stage_depth = -1;
	texture->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	texture->framebuffer_index = -1;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	// Device can texture using linear textures
	prepare_texture_image(NULL, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_LINEAR,
	                      VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize,
	                      tex_format);

	flush_init_cmd();

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

	vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.view);
}

void iron_gpu_texture_destroy(iron_gpu_texture_t *target) {
	if (target->framebuffer_index >= 0) {
		framebuffer_count -= 1;
	}
	else {
		if (target->impl.framebuffer != NULL) {
			vkDestroyFramebuffer(vk_ctx.device, target->impl.framebuffer, NULL);
		}

		if (target->impl.depthBufferBits > 0) {
			vkDestroyImageView(vk_ctx.device, target->impl.depthView, NULL);
			vkDestroyImage(vk_ctx.device, target->impl.depthImage, NULL);
			vkFreeMemory(vk_ctx.device, target->impl.depthMemory, NULL);
		}
		vkDestroyImageView(vk_ctx.device, target->impl.view, NULL);
		vkDestroyImage(vk_ctx.device, target->impl.image, NULL);
		vkFreeMemory(vk_ctx.device, target->impl.mem, NULL);
	}
}

void iron_gpu_internal_texture_set(iron_gpu_texture_t *texture, int unit) {}

int iron_gpu_texture_stride(iron_gpu_texture_t *texture) {
	return texture->impl.stride;
}

void iron_gpu_texture_generate_mipmaps(iron_gpu_texture_t *texture, int levels) {}

void iron_gpu_texture_set_mipmap(iron_gpu_texture_t *texture, iron_gpu_texture_t *mipmap, int level) {
    // VkBuffer staging_buffer;
    // VkDeviceMemory staging_buffer_mem;

    // VkBufferCreateInfo buffer_info = {
		// .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		// .size = mipmap->width * mipmap->height * format_size(mipmap->format),
		// .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		// .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	// };

    // vkCreateBuffer(vk_ctx.device, &buffer_info, NULL, &staging_buffer);

    // VkMemoryRequirements mem_req;
    // vkGetBufferMemoryRequirements(vk_ctx.device, staging_buffer, &mem_req);

    // VkMemoryAllocateInfo alloc_info = {
		// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		// .allocationSize = mem_req.size,
	// };

	// memory_type_from_properties(mem_req.memoryTypeBits,
	// 	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_info.memoryTypeIndex);

    // vkAllocateMemory(vk_ctx.device, &alloc_info, NULL, &staging_buffer_mem);
    // vkBindBufferMemory(vk_ctx.device, staging_buffer, staging_buffer_mem, 0);

    // void *mapped_data;
    // vkMapMemory(vk_ctx.device, staging_buffer_mem, 0, buffer_info.size, 0, &mapped_data);
    // memcpy(mapped_data, mipmap->data, (size_t)buffer_info.size);
    // vkUnmapMemory(vk_ctx.device, staging_buffer_mem);

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

	// setup_init_cmd();
    // vkCmdCopyBufferToImage(
    //     vk_ctx.setup_cmd,
    //     staging_buffer,
    //     texture->impl.image,
    //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //     1,
    //     &region
    // );
	// flush_init_cmd();

	// set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // vkFreeMemory(vk_ctx.device, staging_buffer_mem, NULL);
    // vkDestroyBuffer(vk_ctx.device, staging_buffer, NULL);

    // texture->_uploaded = true;
}

static void render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depth_bits, int framebuffer_index) {
	target->framebuffer_index = framebuffer_index;
	target->width = width;
	target->height = height;
	target->data = NULL;
	target->impl.format = convert_image_format(format);
	target->impl.depthBufferBits = depth_bits;
	target->impl.stage = 0;
	target->impl.stage_depth = -1;
	target->impl.readback_buffer_created = false;

	if (framebuffer_index < 0) {
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, target->impl.format, &formatProperties);

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
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				.flags = 0,
			};

			VkImageViewCreateInfo colorImageView = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = NULL,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = target->impl.format,
				.flags = 0,
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.baseMipLevel = 0,
				.subresourceRange.levelCount = 1,
				.subresourceRange.baseArrayLayer = 0,
				.subresourceRange.layerCount = 1,
			};

			vkCreateImage(vk_ctx.device, &image, NULL, &target->impl.image);

			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(vk_ctx.device, target->impl.image, &memoryRequirements);

			VkMemoryAllocateInfo allocationInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.memoryTypeIndex = 0,
				.allocationSize = memoryRequirements.size,
			};
			memory_type_from_properties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocationInfo.memoryTypeIndex);

			vkAllocateMemory(vk_ctx.device, &allocationInfo, NULL, &target->impl.mem);

			vkBindImageMemory(vk_ctx.device, target->impl.image, target->impl.mem, 0);

			set_image_layout(target->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
			               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			colorImageView.image = target->impl.image;
			vkCreateImageView(vk_ctx.device, &colorImageView, NULL, &target->impl.view);
		}

		if (depth_bits > 0) {
			// const VkFormat depth_format = VK_FORMAT_D16_UNORM;
			const VkFormat depth_format = VK_FORMAT_D24_UNORM_S8_UINT;
			VkImageCreateInfo image = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = NULL,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = depth_format,
				.extent.width = width,
				.extent.height = height,
				.extent.depth = 1,
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.flags = 0,
			};

			vkCreateImage(vk_ctx.device, &image, NULL, &target->impl.depthImage);

			VkMemoryAllocateInfo mem_alloc = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = 0,
				.memoryTypeIndex = 0,
			};

			VkImageViewCreateInfo view = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = NULL,
				.image = target->impl.depthImage,
				.format = depth_format,
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.subresourceRange.baseMipLevel = 0,
				.subresourceRange.levelCount = 1,
				.subresourceRange.baseArrayLayer = 0,
				.subresourceRange.layerCount = 1,
				.flags = 0,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
			};

			VkMemoryRequirements mem_reqs = {0};

			vkGetImageMemoryRequirements(vk_ctx.device, target->impl.depthImage, &mem_reqs);

			mem_alloc.allocationSize = mem_reqs.size;
			memory_type_from_properties(mem_reqs.memoryTypeBits, 0, /* No requirements */ &mem_alloc.memoryTypeIndex);

			vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &target->impl.depthMemory);

			vkBindImageMemory(vk_ctx.device, target->impl.depthImage, target->impl.depthMemory, 0);

			set_image_layout(target->impl.depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
			               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			vkCreateImageView(vk_ctx.device, &view, NULL, &target->impl.depthView);
		}

		VkImageView attachments[2];
		attachments[0] = target->impl.view;

		if (depth_bits > 0) {
			attachments[1] = target->impl.depthView;
		}

		VkFramebufferCreateInfo fbufCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.attachmentCount = depth_bits > 0 ? 2 : 1,
			.pAttachments = attachments,
			.width = width,
			.height = height,
			.layers = 1,
		};
		if (framebuffer_index >= 0) {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].framebuffer_render_pass;
		}
		else if (depth_bits > 0) {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass_with_depth;
		}
		else {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass;
		}

		vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &target->impl.framebuffer);
	}
}

void iron_gpu_render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depth_bits) {
	render_target_init(target, width, height, format, depth_bits, -1);
	target->width = width;
	target->height = height;
	target->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
	target->_uploaded = true;
}

void iron_gpu_render_target_init_framebuffer(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depth_bits) {
	render_target_init(target, width, height, format, depth_bits, framebuffer_count);
	framebuffer_count += 1;
}

void iron_gpu_render_target_set_depth_from(iron_gpu_texture_t *target, iron_gpu_texture_t *source) {
	target->impl.depthImage = source->impl.depthImage;
	target->impl.depthMemory = source->impl.depthMemory;
	target->impl.depthView = source->impl.depthView;
	target->impl.depthBufferBits = source->impl.depthBufferBits;

	// vkDestroyFramebuffer(vk_ctx.device, target->impl.framebuffer, nullptr);

	{
		VkImageView attachments[2];
		attachments[0] = target->impl.view;

		if (target->impl.depthBufferBits > 0) {
			attachments[1] = target->impl.depthView;
		}

		VkFramebufferCreateInfo fbufCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.attachmentCount = target->impl.depthBufferBits > 0 ? 2 : 1,
			.pAttachments = attachments,
			.width = target->width,
			.height = target->height,
			.layers = 1,
		};
		if (target->impl.depthBufferBits > 0) {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass_with_depth;
		}
		else {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass;
		}

		vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &target->impl.framebuffer);
	}
}

void iron_gpu_vertex_buffer_init(gpu_vertex_buffer_impl_t *buffer, int vertexCount, iron_gpu_vertex_structure_t *structure, bool gpuMemory) {
	buffer->myCount = vertexCount;
	buffer->myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		iron_gpu_vertex_element_t element = structure->elements[i];
		buffer->myStride += iron_gpu_vertex_data_size(element.data);
	}

	VkBufferCreateInfo buf_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.size = vertexCount * buffer->myStride,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.flags = 0,
	};

	if (iron_gpu_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	memset(&buffer->mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->mem_alloc.pNext = NULL;
	buffer->mem_alloc.allocationSize = 0;
	buffer->mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements mem_reqs = {0};

	buffer->buf = NULL;
	buffer->mem = NULL;

	vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &buffer->buf);

	vkGetBufferMemoryRequirements(vk_ctx.device, buffer->buf, &mem_reqs);

	buffer->mem_alloc.allocationSize = mem_reqs.size;
	memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->mem_alloc.memoryTypeIndex);

	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
	if (iron_gpu_raytrace_supported()) {
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		buffer->mem_alloc.pNext = &memory_allocate_flags_info;
	}

	vkAllocateMemory(vk_ctx.device, &buffer->mem_alloc, NULL, &buffer->mem);
	vkBindBufferMemory(vk_ctx.device, buffer->buf, buffer->mem, 0);
}

static void unset_vertex_buffer(gpu_vertex_buffer_impl_t *buffer) {

}

void iron_gpu_vertex_buffer_destroy(gpu_vertex_buffer_impl_t *buffer) {
	unset_vertex_buffer(buffer);
	vkFreeMemory(vk_ctx.device, buffer->mem, NULL);
	vkDestroyBuffer(vk_ctx.device, buffer->buf, NULL);
}

float *iron_gpu_vertex_buffer_lock_all(gpu_vertex_buffer_impl_t *buffer) {
	return iron_gpu_vertex_buffer_lock(buffer, 0, buffer->myCount);
}

float *iron_gpu_vertex_buffer_lock(gpu_vertex_buffer_impl_t *buffer, int start, int count) {
	vkMapMemory(vk_ctx.device, buffer->mem, start * buffer->myStride, count * buffer->myStride, 0, (void **)&buffer->data);
	return buffer->data;
}

void iron_gpu_vertex_buffer_unlock_all(gpu_vertex_buffer_impl_t *buffer) {
	vkUnmapMemory(vk_ctx.device, buffer->mem);
}

void iron_gpu_vertex_buffer_unlock(gpu_vertex_buffer_impl_t *buffer, int count) {
	vkUnmapMemory(vk_ctx.device, buffer->mem);
}

int iron_gpu_internal_vertex_buffer_set(gpu_vertex_buffer_impl_t *buffer) {
	return 0;
}

int iron_gpu_vertex_buffer_count(gpu_vertex_buffer_impl_t *buffer) {
	return buffer->myCount;
}

int iron_gpu_vertex_buffer_stride(gpu_vertex_buffer_impl_t *buffer) {
	return buffer->myStride;
}

static void create_uniform_buffer(VkBuffer *buf, VkMemoryAllocateInfo *mem_alloc, VkDeviceMemory *mem, VkDescriptorBufferInfo *buffer_info, int size) {
	VkBufferCreateInfo buf_info;
	memset(&buf_info, 0, sizeof(buf_info));
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	if (iron_gpu_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	buf_info.size = size;
	vkCreateBuffer(vk_ctx.device, &buf_info, NULL, buf);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(vk_ctx.device, *buf, &mem_reqs);

	mem_alloc->sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc->pNext = NULL;
	mem_alloc->allocationSize = mem_reqs.size;
	mem_alloc->memoryTypeIndex = 0;

	memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc->memoryTypeIndex);
	vkAllocateMemory(vk_ctx.device, mem_alloc, NULL, mem);
	vkBindBufferMemory(vk_ctx.device, *buf, *mem, 0);

	buffer_info->buffer = *buf;
	buffer_info->offset = 0;
	buffer_info->range = size;
}

void iron_gpu_constant_buffer_init(iron_gpu_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;

	create_uniform_buffer(&buffer->impl.buf, &buffer->impl.mem_alloc, &buffer->impl.mem, &buffer->impl.buffer_info, size);

	// buffer hack
	if (vk_ctx.vertex_uniform_buffer == NULL) {
		vk_ctx.vertex_uniform_buffer = &buffer->impl.buf;
	}
	else if (vk_ctx.fragment_uniform_buffer == NULL) {
		vk_ctx.fragment_uniform_buffer = &buffer->impl.buf;
	}

	void *p;
	vkMapMemory(vk_ctx.device, buffer->impl.mem, 0, buffer->impl.mem_alloc.allocationSize, 0, (void **)&p);
	memset(p, 0, buffer->impl.mem_alloc.allocationSize);
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
}

void iron_gpu_constant_buffer_destroy(iron_gpu_constant_buffer_t *buffer) {
	vkFreeMemory(vk_ctx.device, buffer->impl.mem, NULL);
	vkDestroyBuffer(vk_ctx.device, buffer->impl.buf, NULL);
}

void iron_gpu_constant_buffer_lock_all(iron_gpu_constant_buffer_t *buffer) {
	iron_gpu_constant_buffer_lock(buffer, 0, iron_gpu_constant_buffer_size(buffer));
}

void iron_gpu_constant_buffer_lock(iron_gpu_constant_buffer_t *buffer, int start, int count) {
	vkMapMemory(vk_ctx.device, buffer->impl.mem, start, count, 0, (void **)&buffer->data);
}

void iron_gpu_constant_buffer_unlock(iron_gpu_constant_buffer_t *buffer) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
	buffer->data = NULL;
}

int iron_gpu_constant_buffer_size(iron_gpu_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}

void iron_gpu_index_buffer_init(iron_gpu_index_buffer_t *buffer, int indexCount, bool gpuMemory) {
	buffer->impl.count = indexCount;

	VkBufferCreateInfo buf_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.size = indexCount * sizeof(uint32_t),
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		.flags = 0,
	};

	if (iron_gpu_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	memset(&buffer->impl.mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = 0;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	buffer->impl.buf = NULL;
	buffer->impl.mem = NULL;

	vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &buffer->impl.buf);

	VkMemoryRequirements mem_reqs = {0};
	vkGetBufferMemoryRequirements(vk_ctx.device, buffer->impl.buf, &mem_reqs);

	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);

	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
	if (iron_gpu_raytrace_supported()) {
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		buffer->impl.mem_alloc.pNext = &memory_allocate_flags_info;
	}

	vkAllocateMemory(vk_ctx.device, &buffer->impl.mem_alloc, NULL, &buffer->impl.mem);
	vkBindBufferMemory(vk_ctx.device, buffer->impl.buf, buffer->impl.mem, 0);
}

void iron_gpu_index_buffer_destroy(iron_gpu_index_buffer_t *buffer) {
	// unset(buffer);
	vkFreeMemory(vk_ctx.device, buffer->impl.mem, NULL);
	vkDestroyBuffer(vk_ctx.device, buffer->impl.buf, NULL);
}

static int iron_gpu_internal_index_buffer_stride(iron_gpu_index_buffer_t *buffer) {
	return 4;
}

void *iron_gpu_index_buffer_lock_all(iron_gpu_index_buffer_t *buffer) {
	return iron_gpu_index_buffer_lock(buffer, 0, iron_gpu_index_buffer_count(buffer));
}

void *iron_gpu_index_buffer_lock(iron_gpu_index_buffer_t *buffer, int start, int count) {
	uint8_t *data;
	vkMapMemory(vk_ctx.device, buffer->impl.mem, 0, buffer->impl.mem_alloc.allocationSize, 0, (void **)&data);
	return &data[start * iron_gpu_internal_index_buffer_stride(buffer)];
}

void iron_gpu_index_buffer_unlock_all(iron_gpu_index_buffer_t *buffer) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
}

void iron_gpu_index_buffer_unlock(iron_gpu_index_buffer_t *buffer, int count) {
	iron_gpu_index_buffer_unlock_all(buffer);
}

int iron_gpu_index_buffer_count(iron_gpu_index_buffer_t *buffer) {
	return buffer->impl.count;
}

static const int INDEX_RAYGEN = 0;
static const int INDEX_MISS = 1;
static const int INDEX_CLOSEST_HIT = 2;
static const char *raygen_shader_name = "raygeneration";
static const char *closesthit_shader_name = "closesthit";
static const char *miss_shader_name = "miss";

static VkDescriptorPool raytrace_descriptor_pool;
static iron_gpu_raytrace_acceleration_structure_t *accel;
static iron_gpu_raytrace_pipeline_t *pipeline;
static iron_gpu_texture_t *output = NULL;
static iron_gpu_texture_t *texpaint0;
static iron_gpu_texture_t *texpaint1;
static iron_gpu_texture_t *texpaint2;
static iron_gpu_texture_t *texenv;
static iron_gpu_texture_t *texsobol;
static iron_gpu_texture_t *texscramble;
static iron_gpu_texture_t *texrank;
static gpu_vertex_buffer_impl_t *vb[16];
static gpu_vertex_buffer_impl_t *vb_last[16];
static iron_gpu_index_buffer_t *ib[16];
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

void iron_gpu_raytrace_pipeline_init(iron_gpu_raytrace_pipeline_t *pipeline, iron_gpu_command_list_t *command_list, void *ray_shader, int ray_shader_size,
                                 iron_gpu_constant_buffer_t *constant_buffer) {
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
		vkCreateDescriptorSetLayout(vk_ctx.device, &layout_info, NULL, &pipeline->impl.descriptor_set_layout);

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.setLayoutCount = 1,
			.pSetLayouts = &pipeline->impl.descriptor_set_layout,
		};

		vkCreatePipelineLayout(vk_ctx.device, &pipeline_layout_create_info, NULL, &pipeline->impl.pipeline_layout);

		VkShaderModuleCreateInfo module_create_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = ray_shader_size,
			.pCode = (const uint32_t *)ray_shader,
			.pNext = NULL,
			.flags = 0,
		};
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
		_vkCreateRayTracingPipelinesKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCreateRayTracingPipelinesKHR");
		_vkCreateRayTracingPipelinesKHR(vk_ctx.device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &raytracing_pipeline_create_info, NULL, &pipeline->impl.pipeline);
	}

	{
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_pipeline_properties;
		ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		ray_tracing_pipeline_properties.pNext = NULL;
		VkPhysicalDeviceProperties2 device_properties = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			.pNext = &ray_tracing_pipeline_properties,
		};
		vkGetPhysicalDeviceProperties2(vk_ctx.gpu, &device_properties);

		_vkGetRayTracingShaderGroupHandlesKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetRayTracingShaderGroupHandlesKHR");
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

		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &pipeline->impl.raygen_shader_binding_table);
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &pipeline->impl.hit_shader_binding_table);
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &pipeline->impl.miss_shader_binding_table);

		uint8_t shader_handle_storage[1024];
		_vkGetRayTracingShaderGroupHandlesKHR(vk_ctx.device, pipeline->impl.pipeline, 0, 3, handle_size_aligned * 3, shader_handle_storage);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		};

		VkMemoryAllocateInfo memory_allocate_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &memory_allocate_flags_info,
		};

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

		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = NULL,
			.maxSets = 1024,
			.poolSizeCount = 12,
			.pPoolSizes = type_counts,
		};

		vkCreateDescriptorPool(vk_ctx.device, &descriptor_pool_create_info, NULL, &raytrace_descriptor_pool);

		VkDescriptorSetAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = NULL,
			.descriptorPool = raytrace_descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &pipeline->impl.descriptor_set_layout,
		};
		vkAllocateDescriptorSets(vk_ctx.device, &alloc_info, &pipeline->impl.descriptor_set);
	}
}

void iron_gpu_raytrace_pipeline_destroy(iron_gpu_raytrace_pipeline_t *pipeline) {
	vkDestroyPipeline(vk_ctx.device, pipeline->impl.pipeline, NULL);
	vkDestroyPipelineLayout(vk_ctx.device, pipeline->impl.pipeline_layout, NULL);
	vkDestroyDescriptorSetLayout(vk_ctx.device, pipeline->impl.descriptor_set_layout, NULL);
}

uint64_t get_buffer_device_address(VkBuffer buffer) {
	VkBufferDeviceAddressInfoKHR buffer_device_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = buffer,
	};
	_vkGetBufferDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetBufferDeviceAddressKHR");
	return _vkGetBufferDeviceAddressKHR(vk_ctx.device, &buffer_device_address_info);
}

void iron_gpu_raytrace_acceleration_structure_init(iron_gpu_raytrace_acceleration_structure_t *accel) {
	_vkGetBufferDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetBufferDeviceAddressKHR");
	_vkCreateAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCreateAccelerationStructureKHR");
	_vkGetAccelerationStructureDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetAccelerationStructureDeviceAddressKHR");
	_vkGetAccelerationStructureBuildSizesKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetAccelerationStructureBuildSizesKHR");

	vb_count = 0;
	instances_count = 0;
}

void iron_gpu_raytrace_acceleration_structure_add(iron_gpu_raytrace_acceleration_structure_t *accel, struct iron_gpu_vertex_buffer *_vb, iron_gpu_index_buffer_t *_ib,
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

void _iron_gpu_raytrace_acceleration_structure_destroy_bottom(iron_gpu_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkDestroyAccelerationStructureKHR");
	for (int i = 0; i < vb_count_last; ++i) {
		_vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.bottom_level_acceleration_structure[i], NULL);
		vkFreeMemory(vk_ctx.device, accel->impl.bottom_level_mem[i], NULL);
		vkDestroyBuffer(vk_ctx.device, accel->impl.bottom_level_buffer[i], NULL);
	}
}

void _iron_gpu_raytrace_acceleration_structure_destroy_top(iron_gpu_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkDestroyAccelerationStructureKHR");
	_vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.top_level_acceleration_structure, NULL);
	vkFreeMemory(vk_ctx.device, accel->impl.top_level_mem, NULL);
	vkDestroyBuffer(vk_ctx.device, accel->impl.top_level_buffer, NULL);
	vkFreeMemory(vk_ctx.device, accel->impl.instances_mem, NULL);
	vkDestroyBuffer(vk_ctx.device, accel->impl.instances_buffer, NULL);
}

void iron_gpu_raytrace_acceleration_structure_build(iron_gpu_raytrace_acceleration_structure_t *accel, iron_gpu_command_list_t *command_list,
	struct iron_gpu_vertex_buffer *_vb_full, iron_gpu_index_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_iron_gpu_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_iron_gpu_raytrace_acceleration_structure_destroy_top(accel);
	}

	vb_count_last = vb_count;

	if (vb_count == 0) {
		return;
	}

	// Bottom level
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {

			uint32_t prim_count = ib[i]->impl.count / 3;
			uint32_t vert_count = vb[i]->myCount;

			VkDeviceOrHostAddressConstKHR vertex_data_device_address = {0};
			VkDeviceOrHostAddressConstKHR index_data_device_address = {0};

			vertex_data_device_address.deviceAddress = get_buffer_device_address(vb[i]->buf);
			index_data_device_address.deviceAddress = get_buffer_device_address(ib[i]->impl.buf);

			VkAccelerationStructureGeometryKHR acceleration_geometry = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
				.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
				.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
				.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
				.geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16A16_SNORM,
				.geometry.triangles.vertexData.deviceAddress = vertex_data_device_address.deviceAddress,
				.geometry.triangles.vertexStride = vb[i]->myStride,
				.geometry.triangles.maxVertex = vb[i]->myCount,
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
			_vkGetAccelerationStructureBuildSizesKHR(vk_ctx.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &acceleration_structure_build_geometry_info,
													&prim_count, &acceleration_build_sizes_info);

			VkBufferCreateInfo buffer_create_info = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = acceleration_build_sizes_info.accelerationStructureSize,
				.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			};
			VkBuffer bottom_level_buffer = VK_NULL_HANDLE;
			vkCreateBuffer(vk_ctx.device, &buffer_create_info, NULL, &bottom_level_buffer);

			VkMemoryRequirements memory_requirements2;
			vkGetBufferMemoryRequirements(vk_ctx.device, bottom_level_buffer, &memory_requirements2);

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
			vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &bottom_level_mem);
			vkBindBufferMemory(vk_ctx.device, bottom_level_buffer, bottom_level_mem, 0);

			VkAccelerationStructureCreateInfoKHR acceleration_create_info = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
				.buffer = bottom_level_buffer,
				.size = acceleration_build_sizes_info.accelerationStructureSize,
			};
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

			VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
				.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
			};

			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.pNext = &memory_allocate_flags_info;
			memory_allocate_info.allocationSize = memory_requirements.size;
			memory_type_from_properties(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
			vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &scratch_memory);
			vkBindBufferMemory(vk_ctx.device, scratch_buffer, scratch_memory, 0);

			VkBufferDeviceAddressInfoKHR buffer_device_address_info = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
				.buffer = scratch_buffer,
			};
			uint64_t scratch_buffer_device_address = _vkGetBufferDeviceAddressKHR(vk_ctx.device, &buffer_device_address_info);

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
					.commandPool = vk_ctx.cmd_pool,
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = 1,
				};

				VkCommandBuffer command_buffer;
				vkAllocateCommandBuffers(vk_ctx.device, &cmd_buf_allocate_info, &command_buffer);

				VkCommandBufferBeginInfo command_buffer_info = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				};
				vkBeginCommandBuffer(command_buffer, &command_buffer_info);

				_vkCmdBuildAccelerationStructuresKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCmdBuildAccelerationStructuresKHR");
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
				vkCreateFence(vk_ctx.device, &fence_info, NULL, &fence);

				vkQueueSubmit(vk_ctx.queue, 1, &submit_info, fence);
				vkWaitForFences(vk_ctx.device, 1, &fence, VK_TRUE, 100000000000);
				vkDestroyFence(vk_ctx.device, fence, NULL);
				vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, &command_buffer);
			}

			VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info = {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
				.accelerationStructure = accel->impl.bottom_level_acceleration_structure[i],
			};

			accel->impl.bottom_level_acceleration_structure_handle[i] = _vkGetAccelerationStructureDeviceAddressKHR(vk_ctx.device, &acceleration_device_address_info);

			vkFreeMemory(vk_ctx.device, scratch_memory, NULL);
			vkDestroyBuffer(vk_ctx.device, scratch_buffer, NULL);

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
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &instances_buffer);

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(vk_ctx.device, instances_buffer, &mem_reqs);

		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		};
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
			VkAccelerationStructureInstanceKHR instance = {
				.transform = transform_matrix,
			};

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

		_vkGetAccelerationStructureBuildSizesKHR(vk_ctx.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &acceleration_structure_build_geometry_info,
		                                         &instance_count, &acceleration_build_sizes_info);

		VkBufferCreateInfo buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = acceleration_build_sizes_info.accelerationStructureSize,
			.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		VkBuffer top_level_buffer = VK_NULL_HANDLE;
		vkCreateBuffer(vk_ctx.device, &buffer_create_info, NULL, &top_level_buffer);

		VkMemoryRequirements memory_requirements2;
		vkGetBufferMemoryRequirements(vk_ctx.device, top_level_buffer, &memory_requirements2);

		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memory_allocate_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &memory_allocate_flags_info,
			.allocationSize = memory_requirements2.size,
		};
		memory_type_from_properties(memory_requirements2.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_allocate_info.memoryTypeIndex);
		VkDeviceMemory top_level_mem;
		vkAllocateMemory(vk_ctx.device, &memory_allocate_info, NULL, &top_level_mem);
		vkBindBufferMemory(vk_ctx.device, top_level_buffer, top_level_mem, 0);

		VkAccelerationStructureCreateInfoKHR acceleration_create_info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.buffer = top_level_buffer,
			.size = acceleration_build_sizes_info.accelerationStructureSize,
		};
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

		VkBufferDeviceAddressInfoKHR buffer_device_address_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = scratch_buffer,
		};
		uint64_t scratch_buffer_device_address = _vkGetBufferDeviceAddressKHR(vk_ctx.device, &buffer_device_address_info);

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
				.commandPool = vk_ctx.cmd_pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};

			VkCommandBuffer command_buffer;
			vkAllocateCommandBuffers(vk_ctx.device, &cmd_buf_allocate_info, &command_buffer);

			VkCommandBufferBeginInfo command_buffer_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			};
			vkBeginCommandBuffer(command_buffer, &command_buffer_info);

			_vkCmdBuildAccelerationStructuresKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCmdBuildAccelerationStructuresKHR");
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
			vkCreateFence(vk_ctx.device, &fence_info, NULL, &fence);

			vkQueueSubmit(vk_ctx.queue, 1, &submit_info, fence);
			vkWaitForFences(vk_ctx.device, 1, &fence, VK_TRUE, 100000000000);
			vkDestroyFence(vk_ctx.device, fence, NULL);

			vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, &command_buffer);
		}

		VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = accel->impl.top_level_acceleration_structure,
		};

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

		// VkBufferCreateInfo buf_info = {
			// .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			// .pNext = NULL,
			// .size = vert_count * vb[0]->myStride,
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

		// vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &vb_full);

		// VkMemoryRequirements mem_reqs = {0};
		// vkGetBufferMemoryRequirements(vk_ctx.device, vb_full, &mem_reqs);

		// mem_alloc.allocationSize = mem_reqs.size;
		// memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		// VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			// .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		// };
		// mem_alloc.pNext = &memory_allocate_flags_info;

		// vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &vb_full_mem);
		// vkBindBufferMemory(vk_ctx.device, vb_full, vb_full_mem, 0);

		// float *data;
		// vkMapMemory(vk_ctx.device, vb_full_mem, 0, vert_count * vb[0]->myStride, 0, (void **)&data);
		// vkUnmapMemory(vk_ctx.device, vb_full_mem);

		////

		#ifdef is_forge

		vb_full = _vb_full->buf;
		vb_full_mem = _vb_full->mem;

		#else

		vb_full = vb[0]->buf;
		vb_full_mem = vb[0]->mem;

		#endif
	}

	{
		// if (ib_full != NULL) {
		// 	vkFreeMemory(vk_ctx.device, ib_full_mem, NULL);
		// 	vkDestroyBuffer(vk_ctx.device, ib_full, NULL);
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

		// vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &ib_full);

		// VkMemoryRequirements mem_reqs = {0};
		// vkGetBufferMemoryRequirements(vk_ctx.device, ib_full, &mem_reqs);

		// mem_alloc.allocationSize = mem_reqs.size;
		// memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);

		// VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {
			// .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			// .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		// };
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

void iron_gpu_raytrace_acceleration_structure_destroy(iron_gpu_raytrace_acceleration_structure_t *accel) {
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

void iron_gpu_raytrace_set_textures(iron_gpu_texture_t *_texpaint0, iron_gpu_texture_t *_texpaint1, iron_gpu_texture_t *_texpaint2, iron_gpu_texture_t *_texenv, iron_gpu_texture_t *_texsobol, iron_gpu_texture_t *_texscramble, iron_gpu_texture_t *_texrank) {
	texpaint0 = _texpaint0;
	texpaint1 = _texpaint1;
	texpaint2 = _texpaint2;
	texenv = _texenv;
	texsobol = _texsobol;
	texscramble = _texscramble;
	texrank = _texrank;
}

void iron_gpu_raytrace_set_acceleration_structure(iron_gpu_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void iron_gpu_raytrace_set_pipeline(iron_gpu_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void iron_gpu_raytrace_set_target(iron_gpu_texture_t *_output) {
	if (_output != output) {
		vkDestroyImage(vk_ctx.device, _output->impl.image, NULL);

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

		vkCreateImage(vk_ctx.device, &image, NULL, &_output->impl.image);

		vkBindImageMemory(vk_ctx.device, _output->impl.image, _output->impl.mem, 0);

		VkImageViewCreateInfo colorImageView = {
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
		vkCreateImageView(vk_ctx.device, &colorImageView, NULL, &_output->impl.view);

		VkImageView attachments[1];
		attachments[0] = _output->impl.view;

		VkFramebufferCreateInfo fbufCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.renderPass = vk_ctx.windows[0].rendertarget_render_pass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = _output->width,
			.height = _output->height,
			.layers = 1,
		};
		vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &_output->impl.framebuffer);
	}
	output = _output;
}

void iron_gpu_raytrace_dispatch_rays(iron_gpu_command_list_t *command_list) {
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
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
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
	vkUpdateDescriptorSets(vk_ctx.device, 12, write_descriptor_sets, 0, VK_NULL_HANDLE);

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_pipeline_properties;
	ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	ray_tracing_pipeline_properties.pNext = NULL;
	VkPhysicalDeviceProperties2 device_properties = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = &ray_tracing_pipeline_properties,
	};
	vkGetPhysicalDeviceProperties2(vk_ctx.gpu, &device_properties);

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

	vkCmdEndRenderPass(command_list->impl._buffer);

	// Dispatch the ray tracing commands
	vkCmdBindPipeline(command_list->impl._buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline);
	vkCmdBindDescriptorSets(command_list->impl._buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->impl.pipeline_layout, 0, 1,
	                        &pipeline->impl.descriptor_set, 0, 0);

	_vkCmdTraceRaysKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCmdTraceRaysKHR");
	_vkCmdTraceRaysKHR(command_list->impl._buffer, &raygen_shader_sbt_entry, &miss_shader_sbt_entry, &hit_shader_sbt_entry, &callable_shader_sbt_entry,
	                   output->width, output->height, 1);

	vkCmdBeginRenderPass(command_list->impl._buffer, &current_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}
