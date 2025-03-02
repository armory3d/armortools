
#ifndef NDEBUG
#define VALIDATE
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iron_gpu.h>
#include <iron_math.h>
#include <iron_system.h>
#include "vulkan_gpu.h"

static VkSemaphore framebuffer_available;
static VkSemaphore relay_semaphore;
static bool wait_for_relay = false;
static void command_list_should_wait_for_framebuffer(void);
static VkDescriptorSetLayout compute_descriptor_layout;

// djb2
uint32_t kinc_internal_hash_name(unsigned char *str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = hash * 33 ^ c;
	}
	return hash;
}

struct vk_funs vk = {0};
struct vk_context vk_ctx = {0};

void kinc_vulkan_get_instance_extensions(const char **extensions, int *index, int max);
VkBool32 kinc_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
VkResult kinc_vulkan_create_surface(VkInstance instance, VkSurfaceKHR *surface);
void kinc_g5_internal_resize(int, int);

#define GET_INSTANCE_PROC_ADDR(instance, entrypoint)                                                                                                           \
	{                                                                                                                                                          \
		vk.fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(instance, "vk" #entrypoint);                                                             \
		if (vk.fp##entrypoint == NULL) {                                                                                                                       \
			kinc_error("vkGetInstanceProcAddr failed to find vk" #entrypoint);                                                                         \
		}                                                                                                                                                      \
	}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void create_descriptor_layout(void);
static void create_compute_descriptor_layout(void);
void set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);

kinc_g5_texture_t *vulkan_textures[16] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

kinc_g5_render_target_t *vulkan_render_targets[16] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

VkSampler vulkan_samplers[16] = {
	VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
	VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
    VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
	VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE
};

static bool began = false;
static VkPhysicalDeviceProperties gpu_props;
static VkQueueFamilyProperties *queue_props;
static uint32_t graphics_queue_node_index;
static VkPhysicalDeviceMemoryProperties memory_properties;
static uint32_t queue_count;

#define MAX_PRESENT_MODES 256
static VkPresentModeKHR present_modes[MAX_PRESENT_MODES];

static VkBool32 vkDebugUtilsMessengerCallbackEXT(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *pcallback_data,
	void *puser_data) {

	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		kinc_error("Vulkan ERROR: Code %d : %s", pcallback_data->messageIdNumber, pcallback_data->pMessage);
	}
	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		kinc_log("Vulkan WARNING: Code %d : %s", pcallback_data->messageIdNumber, pcallback_data->pMessage);
	}
	return VK_FALSE;
}

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

void kinc_internal_resize(int width, int height) {
	struct vk_window *window = &vk_ctx.windows[0];
	if (window->width != width || window->height != height) {
		window->resized = true;
		window->width = width;
		window->height = height;
	}

	kinc_g5_internal_resize(width, height);
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

	VkSwapchainKHR oldSwapchain = cleanup_swapchain();
	if (window->surface_destroyed) {
		vk.fpDestroySwapchainKHR(vk_ctx.device, oldSwapchain, NULL);
		oldSwapchain = VK_NULL_HANDLE;
		vk.fpDestroySurfaceKHR(vk_ctx.instance, window->surface, NULL);
		VkResult err = kinc_vulkan_create_surface(vk_ctx.instance, &window->surface);
		assert(!err);
		window->width = kinc_window_width();
		window->height = kinc_window_height();
		window->surface_destroyed = false;
	}

	// Check the surface capabilities and formats
	VkSurfaceCapabilitiesKHR surfCapabilities = {0};
	VkResult err = vk.fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_ctx.gpu, window->surface, &surfCapabilities);
	assert(!err);

	uint32_t present_mode_count;
	err = vk.fpGetPhysicalDeviceSurfacePresentModesKHR(vk_ctx.gpu, window->surface, &present_mode_count, NULL);
	present_mode_count = present_mode_count > MAX_PRESENT_MODES ? MAX_PRESENT_MODES : present_mode_count;
	assert(!err);
	err = vk.fpGetPhysicalDeviceSurfacePresentModesKHR(vk_ctx.gpu, window->surface, &present_mode_count, present_modes);
	assert(!err);

	VkExtent2D swapchainExtent;
	// width and height are either both -1, or both not -1.
	if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = window->width;
		swapchainExtent.height = window->height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
		window->width = surfCapabilities.currentExtent.width;
		window->height = surfCapabilities.currentExtent.height;
	}

	VkPresentModeKHR swapchainPresentMode = window->vsynced ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
	if ((surfCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount)) {
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR preTransform = {0};
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchain_info = {0};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.pNext = NULL;
	swapchain_info.surface = window->surface;
	swapchain_info.minImageCount = desiredNumberOfSwapchainImages;
	swapchain_info.imageFormat = window->format.format;
	swapchain_info.imageColorSpace = window->format.colorSpace;
	swapchain_info.imageExtent.width = swapchainExtent.width;
	swapchain_info.imageExtent.height = swapchainExtent.height;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.preTransform = preTransform;

	if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	else if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	else if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	else if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	}
	else {
		kinc_error("No supported composite alpha, this should not happen.\nPlease go complain to the writers of your Vulkan driver.");
		exit(1);
	}

	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.queueFamilyIndexCount = 0;
	swapchain_info.pQueueFamilyIndices = NULL;
	swapchain_info.presentMode = swapchainPresentMode;
	swapchain_info.oldSwapchain = oldSwapchain;
	swapchain_info.clipped = true;

	err = vk.fpCreateSwapchainKHR(vk_ctx.device, &swapchain_info, NULL, &window->swapchain);
	assert(!err);

	// If we just re-created an existing swapchain, we should destroy the old
	// swapchain at this point.
	// Note: destroying the swapchain also cleans up all its associated
	// presentable images once the platform is done with them.
	if (oldSwapchain != VK_NULL_HANDLE) {
		vk.fpDestroySwapchainKHR(vk_ctx.device, oldSwapchain, NULL);
	}

	err = vk.fpGetSwapchainImagesKHR(vk_ctx.device, window->swapchain, &window->image_count, NULL);
	assert(!err);

	window->images = (VkImage *)malloc(window->image_count * sizeof(VkImage));
	assert(window->images);

	err = vk.fpGetSwapchainImagesKHR(vk_ctx.device, window->swapchain, &window->image_count, window->images);
	assert(!err);

	window->views = (VkImageView *)malloc(window->image_count * sizeof(VkImageView));
	assert(window->views);

	for (uint32_t i = 0; i < window->image_count; i++) {
		VkImageViewCreateInfo color_attachment_view = {0};
		color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_attachment_view.pNext = NULL;
		color_attachment_view.format = window->format.format;
		color_attachment_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_attachment_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_attachment_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_attachment_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_attachment_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_attachment_view.subresourceRange.baseMipLevel = 0;
		color_attachment_view.subresourceRange.levelCount = 1;
		color_attachment_view.subresourceRange.baseArrayLayer = 0;
		color_attachment_view.subresourceRange.layerCount = 1;
		color_attachment_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_attachment_view.flags = 0;

		// Render loop will expect image to have been used before and in
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		// layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image
		// to that state
		set_image_layout(window->images[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		color_attachment_view.image = window->images[i];

		err = vkCreateImageView(vk_ctx.device, &color_attachment_view, NULL, &window->views[i]);
		assert(!err);
	}

	window->current_image = 0;

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;

	if (window->depth_bits > 0) {
		VkImageCreateInfo image = {0};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = NULL;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = depth_format;
		image.extent.width = window->width;
		image.extent.height = window->height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image.flags = 0;

		VkMemoryAllocateInfo mem_alloc = {0};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkImageViewCreateInfo view = {0};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.pNext = NULL;
		view.image = VK_NULL_HANDLE;
		view.format = depth_format;
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.levelCount = 1;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;
		view.flags = 0;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;

		VkMemoryRequirements mem_reqs = {0};
		bool pass;

		err = vkCreateImage(vk_ctx.device, &image, NULL, &window->depth.image);
		assert(!err);

		vkGetImageMemoryRequirements(vk_ctx.device, window->depth.image, &mem_reqs);

		mem_alloc.allocationSize = mem_reqs.size;
		pass = memory_type_from_properties(mem_reqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);
		assert(pass);

		err = vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &window->depth.memory);
		assert(!err);

		err = vkBindImageMemory(vk_ctx.device, window->depth.image, window->depth.memory, 0);
		assert(!err);

		set_image_layout(window->depth.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		view.image = window->depth.image;
		err = vkCreateImageView(vk_ctx.device, &view, NULL, &window->depth.view);
		assert(!err);
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

	VkAttachmentReference color_reference = {0};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {0};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = window->depth_bits > 0 ? &depth_reference : NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

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

	VkRenderPassCreateInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = window->depth_bits > 0 ? 2 : 1;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 2;
	rp_info.pDependencies = dependencies;

	err = vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->framebuffer_render_pass);
	assert(!err);

	VkImageView attachmentViews[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};

	if (window->depth_bits > 0) {
		attachmentViews[1] = window->depth.view;
	}

	VkFramebufferCreateInfo fb_info = {0};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = window->framebuffer_render_pass;
	fb_info.attachmentCount = window->depth_bits > 0 ? 2 : 1;
	fb_info.pAttachments = attachmentViews;
	fb_info.width = window->width;
	fb_info.height = window->height;
	fb_info.layers = 1;

	window->framebuffers = (VkFramebuffer *)malloc(window->image_count * sizeof(VkFramebuffer));
	assert(window->framebuffers);

	for (uint32_t i = 0; i < window->image_count; i++) {
		attachmentViews[0] = window->views[i];
		err = vkCreateFramebuffer(vk_ctx.device, &fb_info, NULL, &window->framebuffers[i]);
		assert(!err);
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

	VkAttachmentReference color_reference = {0};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

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

	VkRenderPassCreateInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = 1;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 2;
	rp_info.pDependencies = dependencies;

	VkResult err = vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->rendertarget_render_pass);
	assert(!err);

	// depthBufferBits > 0
	attachments[1].format = VK_FORMAT_D16_UNORM;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].flags = 0;

	VkAttachmentReference depth_reference = {0};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	subpass.pDepthStencilAttachment = &depth_reference;

	rp_info.attachmentCount = 2;

	err = vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &window->rendertarget_render_pass_with_depth);
	assert(!err);
}

void destroy_render_target_pass(struct vk_window *window) {
	vkDestroyRenderPass(vk_ctx.device, window->rendertarget_render_pass, NULL);
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
			kinc_error("Failed to find extension %s", wanted_extensions[i]);
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

void kinc_g5_internal_init() {
	VkResult err;
	uint32_t instance_layer_count = 0;

	static const char *wanted_instance_layers[64];
	int wanted_instance_layer_count = 0;

	err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
	assert(!err);

	if (instance_layer_count > 0) {
		VkLayerProperties *instance_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * instance_layer_count);
		err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
		assert(!err);

#ifdef VALIDATE
		vk_ctx.validation_found = find_layer(instance_layers, instance_layer_count, "VK_LAYER_KHRONOS_validation");
		if (vk_ctx.validation_found) {
			kinc_log("Running with Vulkan validation layers enabled.");
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
	kinc_vulkan_get_instance_extensions(wanted_instance_extensions, &wanted_instance_extension_count, ARRAY_SIZE(wanted_instance_extensions));

	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	assert(!err);
	VkExtensionProperties *instance_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instance_extension_count);
	err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
	assert(!err);
	bool missing_instance_extensions =
	    check_extensions(wanted_instance_extensions, wanted_instance_extension_count, instance_extensions, instance_extension_count);

	if (missing_instance_extensions) {
		kinc_error("");
	}

#ifdef VALIDATE
	// this extension should be provided by the validation layers
	if (vk_ctx.validation_found) {
		wanted_instance_extensions[wanted_instance_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}
#endif

	VkApplicationInfo app = {0};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = NULL;
	app.pApplicationName = kinc_application_name();
	app.applicationVersion = 0;
	app.pEngineName = "Kore";
	app.engineVersion = 0;

#ifdef KINC_ANDROID
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


	err = vkCreateInstance(&info, NULL, &vk_ctx.instance);

	if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
		kinc_error("Vulkan driver is incompatible");
	}
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
		kinc_error("Vulkan extension not found");
	}
	else if (err) {
		kinc_error("Can not create Vulkan instance");
	}

	uint32_t gpu_count;

	err = vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, NULL);
	assert(!err && gpu_count > 0);

	if (gpu_count > 0) {
		VkPhysicalDevice *physical_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		err = vkEnumeratePhysicalDevices(vk_ctx.instance, &gpu_count, physical_devices);
		assert(!err);

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
				VkBool32 queue_supports_present = kinc_vulkan_get_physical_device_presentation_support(gpu, i);
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

			// Score the device in order to compare it to others.
			// Higher score = better.
			float score = 0.0;
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(gpu, &properties);
			switch (properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score += 10;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score += 7;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				score += 5;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				score += 1;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				// CPU gets a score of zero
				break;
			case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
				break;
			}

			if (vk_ctx.gpu == VK_NULL_HANDLE || score > best_score) {
				vk_ctx.gpu = gpu;
				best_score = score;
			}
		}

		if (vk_ctx.gpu == VK_NULL_HANDLE) {
			kinc_error("No Vulkan device that supports presentation found");
		}

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(vk_ctx.gpu, &properties);
		kinc_log("Chosen Vulkan device: %s", properties.deviceName);
		free(physical_devices);
	}
	else {
		kinc_error("No Vulkan device found");
	}

	static const char *wanted_device_layers[64];
	int wanted_device_layer_count = 0;

	uint32_t device_layer_count = 0;
	err = vkEnumerateDeviceLayerProperties(vk_ctx.gpu, &device_layer_count, NULL);
	assert(!err);

	if (device_layer_count > 0) {
		VkLayerProperties *device_layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * device_layer_count);
		err = vkEnumerateDeviceLayerProperties(vk_ctx.gpu, &device_layer_count, device_layers);
		assert(!err);

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
	// Allows negative viewport height to flip viewport
	wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;

	if (kinc_g5_raytrace_supported()) {
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
		wanted_device_extensions[wanted_device_extension_count++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;
	}

	uint32_t device_extension_count = 0;

	err = vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, NULL);
	assert(!err);

	VkExtensionProperties *device_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * device_extension_count);
	err = vkEnumerateDeviceExtensionProperties(vk_ctx.gpu, NULL, &device_extension_count, device_extensions);
	assert(!err);

	bool missing_device_extensions = check_extensions(wanted_device_extensions, wanted_device_extension_count, device_extensions, device_extension_count);
	free(device_extensions);

	if (missing_device_extensions) {
		exit(1);
	}

#ifdef VALIDATE
	if (vk_ctx.validation_found) {
		GET_INSTANCE_PROC_ADDR(vk_ctx.instance, CreateDebugUtilsMessengerEXT);

		VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo = {0};
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dbgCreateInfo.flags = 0;
		dbgCreateInfo.pfnUserCallback = vkDebugUtilsMessengerCallbackEXT;
		dbgCreateInfo.pUserData = NULL;
		dbgCreateInfo.pNext = NULL;
		dbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		dbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		err = vk.fpCreateDebugUtilsMessengerEXT(vk_ctx.instance, &dbgCreateInfo, NULL, &vk_ctx.debug_messenger);
		assert(!err);
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
	assert(queue_count >= 1);

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *supportsPresent = (VkBool32 *)malloc(queue_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < queue_count; i++) {
		supportsPresent[i] = kinc_vulkan_get_physical_device_presentation_support(vk_ctx.gpu, i);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queue_count; i++) {
		if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (graphicsQueueNodeIndex == UINT32_MAX) {
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX) {
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (uint32_t i = 0; i < queue_count; ++i) {
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	free(supportsPresent);

	// Generate error if could not find both a graphics and a present queue
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
		kinc_error("Graphics or present queue not found");
	}

	if (graphicsQueueNodeIndex != presentQueueNodeIndex) {
		kinc_error("Graphics and present queue do not match");
	}

	graphics_queue_node_index = graphicsQueueNodeIndex;

	{
		float queue_priorities[1] = {0.0};
		VkDeviceQueueCreateInfo queue = {0};
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.pNext = NULL;
		queue.queueFamilyIndex = graphics_queue_node_index;
		queue.queueCount = 1;
		queue.pQueuePriorities = queue_priorities;

		VkDeviceCreateInfo deviceinfo = {0};
		deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceinfo.pNext = NULL;
		deviceinfo.queueCreateInfoCount = 1;
		deviceinfo.pQueueCreateInfos = &queue;

		deviceinfo.enabledLayerCount = wanted_device_layer_count;
		deviceinfo.ppEnabledLayerNames = (const char *const *)wanted_device_layers;

		deviceinfo.enabledExtensionCount = wanted_device_extension_count;
		deviceinfo.ppEnabledExtensionNames = (const char *const *)wanted_device_extensions;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineExt = {0};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR rayTracingAccelerationStructureExt = {0};
		VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressExt = {0};
		if (kinc_g5_raytrace_supported()) {
			rayTracingPipelineExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			rayTracingPipelineExt.pNext = NULL;
			rayTracingPipelineExt.rayTracingPipeline = VK_TRUE;

			rayTracingAccelerationStructureExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			rayTracingAccelerationStructureExt.pNext = &rayTracingPipelineExt;
			rayTracingAccelerationStructureExt.accelerationStructure = VK_TRUE;

			bufferDeviceAddressExt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
			bufferDeviceAddressExt.pNext = &rayTracingAccelerationStructureExt;
			bufferDeviceAddressExt.bufferDeviceAddress = VK_TRUE;

			deviceinfo.pNext = &bufferDeviceAddressExt;
		}

		err = vkCreateDevice(vk_ctx.gpu, &deviceinfo, NULL, &vk_ctx.device);
		assert(!err);
	}

	vkGetDeviceQueue(vk_ctx.device, graphics_queue_node_index, 0, &vk_ctx.queue);

	vkGetPhysicalDeviceMemoryProperties(vk_ctx.gpu, &memory_properties);

	VkCommandPoolCreateInfo cmd_pool_info = {0};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = graphics_queue_node_index;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	err = vkCreateCommandPool(vk_ctx.device, &cmd_pool_info, NULL, &vk_ctx.cmd_pool);

	create_descriptor_layout();
	create_compute_descriptor_layout();
	assert(!err);

	VkSemaphoreCreateInfo semInfo = {0};
	semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semInfo.pNext = NULL;
	semInfo.flags = 0;

	err = vkCreateSemaphore(vk_ctx.device, &semInfo, NULL, &framebuffer_available);
	assert(!err);

	err = vkCreateSemaphore(vk_ctx.device, &semInfo, NULL, &relay_semaphore);
	assert(!err);
}

void kinc_g5_internal_destroy() {}

void kinc_vulkan_init_window() {
	// this function is used in the android backend
	struct vk_window *window = &vk_ctx.windows[0];

	// delay swapchain/surface recreation
	// otherwise trouble ensues due to G4onG5 backend ending the command list in kinc_g4_begin
	window->resized = true;
	window->surface_destroyed = true;
}

void kinc_g5_internal_init_window(int depthBufferBits, bool vsync) {
	struct vk_window *window = &vk_ctx.windows[0];

	window->depth_bits = depthBufferBits;
	window->vsynced = vsync;

	VkResult err = kinc_vulkan_create_surface(vk_ctx.instance, &window->surface);
	assert(!err);

	VkBool32 surface_supported;
	err = vkGetPhysicalDeviceSurfaceSupportKHR(vk_ctx.gpu, graphics_queue_node_index, window->surface, &surface_supported);
	assert(!err);
	assert(surface_supported);

	uint32_t formatCount;
	err = vk.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_ctx.gpu, window->surface, &formatCount, NULL);
	assert(!err);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	err = vk.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_ctx.gpu, window->surface, &formatCount, surfFormats);
	assert(!err);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		window->format = surfFormats[0];
	}
	else {
		assert(formatCount >= 1);
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
	window->width = kinc_window_width();
	window->height = kinc_window_height();
	create_swapchain();
	create_render_target_render_pass(window);

	began = false;
	kinc_g5_begin(NULL);
}

void kinc_g5_internal_destroy_window() {
	struct vk_window *window = &vk_ctx.windows[0];
	VkSwapchainKHR swapchain = cleanup_swapchain();
	destroy_render_target_pass(window);
	vk.fpDestroySwapchainKHR(vk_ctx.device, swapchain, NULL);
	vk.fpDestroySurfaceKHR(vk_ctx.instance, window->surface, NULL);
}

bool kinc_g5_swap_buffers() {
	return true;
}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget) {
	struct vk_window *window = &vk_ctx.windows[0];

	if (began) {
		return;
	}

	if (window->resized) {
		vkDeviceWaitIdle(vk_ctx.device);
		create_swapchain();
	}

	// Get the index of the next available swapchain image:
	command_list_should_wait_for_framebuffer();
	VkResult err = -1;
	do {
		err = vk.fpAcquireNextImageKHR(vk_ctx.device, window->swapchain, UINT64_MAX, framebuffer_available, VK_NULL_HANDLE, &window->current_image);
		if (err == VK_ERROR_SURFACE_LOST_KHR || err == VK_ERROR_OUT_OF_DATE_KHR) {
			window->surface_destroyed = (err == VK_ERROR_SURFACE_LOST_KHR);
			create_swapchain();
		}
		else {
			assert(err == VK_SUCCESS || err == VK_SUBOPTIMAL_KHR);
			began = true;
			if (renderTarget != NULL) {
				renderTarget->impl.framebuffer = window->framebuffers[window->current_image];
			}
			return;
		}
	}
	while (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR);
}

void kinc_g5_end() {
	VkPresentInfoKHR present = {0};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &vk_ctx.windows[0].swapchain;
	present.pImageIndices = &vk_ctx.windows[0].current_image;
	present.pWaitSemaphores = &relay_semaphore;
	present.waitSemaphoreCount = 1;
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
	else {
		assert(err == VK_SUCCESS);
	}

	reuse_descriptor_sets();
	reuse_compute_descriptor_sets();
	began = false;
}

void kinc_g5_flush() {
	vkDeviceWaitIdle(vk_ctx.device);
}

bool kinc_vulkan_internal_get_size(int *width, int *height) {
	// this is exclusively used by the Android backend at the moment
	if (vk_ctx.windows[0].surface) {
		VkSurfaceCapabilitiesKHR capabilities;
		VkResult err = vk.fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_ctx.gpu, vk_ctx.windows[0].surface, &capabilities);
		assert(!err);
		*width = capabilities.currentExtent.width;
		*height = capabilities.currentExtent.height;
		return true;
	}
	else {
		return false;
	}
}

bool kinc_g5_raytrace_supported() {
#ifdef KINC_ANDROID
	return false;
#else
	return true;
#endif
}

int kinc_g5_max_bound_textures(void) {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(vk_ctx.gpu, &props);
	return props.limits.maxPerStageDescriptorSamplers;
}


















extern kinc_g5_texture_t *vulkan_textures[16];
extern kinc_g5_render_target_t *vulkan_render_targets[16];
VkDescriptorSet getDescriptorSet(void);
static VkDescriptorSet get_compute_descriptor_set(void);
bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
void setImageLayout(VkCommandBuffer _buffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

VkRenderPassBeginInfo currentRenderPassBeginInfo;
VkPipeline currentVulkanPipeline;
kinc_g5_render_target_t *currentRenderTargets[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static bool onBackBuffer = false;
static uint32_t lastVertexConstantBufferOffset = 0;
static uint32_t lastFragmentConstantBufferOffset = 0;
static uint32_t lastComputeConstantBufferOffset = 0;
static kinc_g5_pipeline_t *current_pipeline = NULL;
static kinc_g5_compute_shader *current_compute_shader = NULL;
static int mrtIndex = 0;
static VkFramebuffer mrtFramebuffer[16];
static VkRenderPass mrtRenderPass[16];

static bool in_render_pass = false;

static void endPass(kinc_g5_command_list_t *list) {
	if (in_render_pass) {
		vkCmdEndRenderPass(list->impl._buffer);
		in_render_pass = false;
	}

	for (int i = 0; i < 16; ++i) {
		vulkan_textures[i] = NULL;
		vulkan_render_targets[i] = NULL;
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

void set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout) {
	VkResult err;

	if (vk_ctx.setup_cmd == VK_NULL_HANDLE) {
		VkCommandBufferAllocateInfo cmd = {0};
		cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd.pNext = NULL;
		cmd.commandPool = vk_ctx.cmd_pool;
		cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd.commandBufferCount = 1;

		err = vkAllocateCommandBuffers(vk_ctx.device, &cmd, &vk_ctx.setup_cmd);
		assert(!err);

		VkCommandBufferBeginInfo cmd_buf_info = {0};
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext = NULL;
		cmd_buf_info.flags = 0;
		cmd_buf_info.pInheritanceInfo = NULL;

		err = vkBeginCommandBuffer(vk_ctx.setup_cmd, &cmd_buf_info);
		assert(!err);
	}

	VkImageMemoryBarrier image_memory_barrier = {0};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.pNext = NULL;
	image_memory_barrier.srcAccessMask = 0;
	image_memory_barrier.dstAccessMask = 0;
	image_memory_barrier.oldLayout = old_image_layout;
	image_memory_barrier.newLayout = new_image_layout;
	image_memory_barrier.image = image;
	image_memory_barrier.subresourceRange.aspectMask = aspectMask;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;

	if (old_image_layout != VK_IMAGE_LAYOUT_UNDEFINED) {
		image_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		// Make sure anything that was copying from this image has completed
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		// Make sure any Copy or CPU writes to image are flushed
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}

	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}

	vkCmdPipelineBarrier(vk_ctx.setup_cmd, srcStageMask, dstStageMask, 0, 0, NULL, 0, NULL, 1, &image_memory_barrier);
}

void setup_init_cmd() {
	if (vk_ctx.setup_cmd == VK_NULL_HANDLE) {
		VkCommandBufferAllocateInfo cmd = {0};
		cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd.pNext = NULL;
		cmd.commandPool = vk_ctx.cmd_pool;
		cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd.commandBufferCount = 1;

		VkResult err = vkAllocateCommandBuffers(vk_ctx.device, &cmd, &vk_ctx.setup_cmd);
		assert(!err);

		VkCommandBufferBeginInfo cmd_buf_info = {0};
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext = NULL;
		cmd_buf_info.flags = 0;
		cmd_buf_info.pInheritanceInfo = NULL;

		err = vkBeginCommandBuffer(vk_ctx.setup_cmd, &cmd_buf_info);
		assert(!err);
	}
}

void flush_init_cmd() {
	VkResult err;

	if (vk_ctx.setup_cmd == VK_NULL_HANDLE)
		return;

	err = vkEndCommandBuffer(vk_ctx.setup_cmd);
	assert(!err);

	const VkCommandBuffer cmd_bufs[] = {vk_ctx.setup_cmd};
	VkFence nullFence = {VK_NULL_HANDLE};
	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = NULL;
	submit_info.pWaitDstStageMask = NULL;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = cmd_bufs;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = NULL;

	err = vkQueueSubmit(vk_ctx.queue, 1, &submit_info, nullFence);
	assert(!err);

	err = vkQueueWaitIdle(vk_ctx.queue);
	assert(!err);

	vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, cmd_bufs);
	vk_ctx.setup_cmd = VK_NULL_HANDLE;
}

void set_viewport_and_scissor(kinc_g5_command_list_t *list) {
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));

	if (currentRenderTargets[0] == NULL || currentRenderTargets[0]->framebuffer_index >= 0) {
		viewport.x = 0;
		viewport.y = (float)kinc_window_height();
		viewport.width = (float)kinc_window_width();
		viewport.height = -(float)kinc_window_height();
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		scissor.extent.width = kinc_window_width();
		scissor.extent.height = kinc_window_height();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
	}
	else {
		viewport.x = 0;
		viewport.y = (float)currentRenderTargets[0]->height;
		viewport.width = (float)currentRenderTargets[0]->width;
		viewport.height = -(float)currentRenderTargets[0]->height;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		scissor.extent.width = currentRenderTargets[0]->width;
		scissor.extent.height = currentRenderTargets[0]->height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
	}

	vkCmdSetViewport(list->impl._buffer, 0, 1, &viewport);
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);
}

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {
	VkCommandBufferAllocateInfo cmd = {0};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = vk_ctx.cmd_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	VkResult err = vkAllocateCommandBuffers(vk_ctx.device, &cmd, &list->impl._buffer);
	assert(!err);

	VkFenceCreateInfo fenceInfo = {0};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	err = vkCreateFence(vk_ctx.device, &fenceInfo, NULL, &list->impl.fence);
	assert(!err);

	list->impl._indexCount = 0;
}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {
	vkFreeCommandBuffers(vk_ctx.device, vk_ctx.cmd_pool, 1, &list->impl._buffer);
	vkDestroyFence(vk_ctx.device, list->impl.fence, NULL);
}

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
	VkResult err = vkWaitForFences(vk_ctx.device, 1, &list->impl.fence, VK_TRUE, UINT64_MAX);
	assert(!err);

	vkResetCommandBuffer(list->impl._buffer, 0);
	VkCommandBufferBeginInfo cmd_buf_info = {0};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

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

	VkRenderPassBeginInfo rp_begin = {0};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = vk_ctx.windows[0].framebuffer_render_pass;
	rp_begin.framebuffer = vk_ctx.windows[0].framebuffers[vk_ctx.windows[0].current_image];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = vk_ctx.windows[0].width;
	rp_begin.renderArea.extent.height = vk_ctx.windows[0].height;
	rp_begin.clearValueCount = vk_ctx.windows[0].depth_bits > 0 ? 2 : 1;
	rp_begin.pClearValues = clear_values;

	err = vkBeginCommandBuffer(list->impl._buffer, &cmd_buf_info);
	assert(!err);

	VkImageMemoryBarrier prePresentBarrier = {0};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.pNext = NULL;
	prePresentBarrier.srcAccessMask = 0;
	prePresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	prePresentBarrier.subresourceRange.baseMipLevel = 0;
	prePresentBarrier.subresourceRange.levelCount = 1;
	prePresentBarrier.subresourceRange.baseArrayLayer = 0;
	prePresentBarrier.subresourceRange.layerCount = 1;

	prePresentBarrier.image = vk_ctx.windows[0].images[vk_ctx.windows[0].current_image];
	VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
	vkCmdPipelineBarrier(list->impl._buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1,
	                     pmemory_barrier);

	vkCmdBeginRenderPass(list->impl._buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	currentRenderPassBeginInfo = rp_begin;
	in_render_pass = true;

	set_viewport_and_scissor(list);

	onBackBuffer = true;

	for (int i = 0; i < mrtIndex; ++i) {
		vkDestroyFramebuffer(vk_ctx.device, mrtFramebuffer[i], NULL);
		vkDestroyRenderPass(vk_ctx.device, mrtRenderPass[i], NULL);
	}
	mrtIndex = 0;
}

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {
	vkCmdEndRenderPass(list->impl._buffer);

	VkImageMemoryBarrier prePresentBarrier = {0};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.pNext = NULL;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	prePresentBarrier.subresourceRange.baseMipLevel = 0;
	prePresentBarrier.subresourceRange.levelCount = 1;
	prePresentBarrier.subresourceRange.baseArrayLayer = 0;
	prePresentBarrier.subresourceRange.layerCount = 1;

	prePresentBarrier.image = vk_ctx.windows[0].images[vk_ctx.windows[0].current_image];
	VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
	vkCmdPipelineBarrier(list->impl._buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);

	VkResult err = vkEndCommandBuffer(list->impl._buffer);
	assert(!err);
}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth) {
	VkClearRect clearRect = {0};
	clearRect.rect.offset.x = 0;
	clearRect.rect.offset.y = 0;
	clearRect.rect.extent.width = renderTarget->width;
	clearRect.rect.extent.height = renderTarget->height;
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;

	int count = 0;
	VkClearAttachment attachments[2];
	if (flags & KINC_G5_CLEAR_COLOR) {
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
	if ((flags & KINC_G5_CLEAR_DEPTH) && renderTarget->impl.depthBufferBits > 0) {
		attachments[count].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachments[count].clearValue.depthStencil.depth = depth;
		attachments[count].clearValue.depthStencil.stencil = 0;
		count++;
	}
	vkCmdClearAttachments(list->impl._buffer, count, attachments, 1, &clearRect);
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(list, 0, list->impl._indexCount);
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count) {
	vkCmdDrawIndexed(list->impl._buffer, count, 1, start, 0, 0);
}

void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
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

void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = x;
	scissor.offset.y = y;
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);
}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list) {
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	if (currentRenderTargets[0] == NULL || currentRenderTargets[0]->framebuffer_index >= 0) {
		scissor.extent.width = kinc_window_width();
		scissor.extent.height = kinc_window_height();
	}
	else {
		scissor.extent.width = currentRenderTargets[0]->width;
		scissor.extent.height = currentRenderTargets[0]->height;
	}
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);
}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	current_pipeline = pipeline;
	lastVertexConstantBufferOffset = 0;
	lastFragmentConstantBufferOffset = 0;

	if (onBackBuffer) {
		currentVulkanPipeline = current_pipeline->impl.framebuffer_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.framebuffer_pipeline);
	}
	else {
		currentVulkanPipeline = current_pipeline->impl.rendertarget_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.rendertarget_pipeline);
	}
}

void kinc_g5_command_list_set_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *vertexBuffer) {
	VkBuffer buffers[1];
	VkDeviceSize offsets[1];
	buffers[0] = vertexBuffer->impl.buf;
	offsets[0] = (VkDeviceSize)(0);
	vkCmdBindVertexBuffers(list->impl._buffer, 0, 1, buffers, offsets);
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *indexBuffer) {
	list->impl._indexCount = kinc_g5_index_buffer_count(indexBuffer);
	vkCmdBindIndexBuffer(list->impl._buffer, indexBuffer->impl.buf, 0, VK_INDEX_TYPE_UINT32);
}

void kinc_internal_restore_render_target(kinc_g5_command_list_t *list, struct kinc_g5_render_target *target) {
	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.x = 0;
	viewport.y = (float)kinc_window_height();
	viewport.width = (float)kinc_window_width();
	viewport.height = -(float)kinc_window_height();
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(list->impl._buffer, 0, 1, &viewport);
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = kinc_window_width();
	scissor.extent.height = kinc_window_height();
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);

	if (onBackBuffer && in_render_pass) {
		return;
	}

	endPass(list);

	currentRenderTargets[0] = NULL;
	onBackBuffer = true;

	VkClearValue clear_values[2];
	memset(clear_values, 0, sizeof(VkClearValue) * 2);
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].depthStencil.depth = 1.0;
	clear_values[1].depthStencil.stencil = 0;
	VkRenderPassBeginInfo rp_begin = {0};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = vk_ctx.windows[0].framebuffer_render_pass;
	rp_begin.framebuffer = vk_ctx.windows[0].framebuffers[vk_ctx.windows[0].current_image];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = kinc_window_width();
	rp_begin.renderArea.extent.height = kinc_window_height();
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;
	vkCmdBeginRenderPass(list->impl._buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	currentRenderPassBeginInfo = rp_begin;
	in_render_pass = true;

	if (current_pipeline != NULL) {
		currentVulkanPipeline = current_pipeline->impl.framebuffer_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.framebuffer_pipeline);
	}
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {
	for (int i = 0; i < count; ++i) {
		currentRenderTargets[i] = targets[i];
	}
	for (int i = count; i < 8; ++i) {
		currentRenderTargets[i] = NULL;
	}

	if (targets[0]->framebuffer_index >= 0) {
		kinc_internal_restore_render_target(list, targets[0]);
		return;
	}

	endPass(list);

	onBackBuffer = false;

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

	VkRenderPassBeginInfo rp_begin = {0};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = targets[0]->width;
	rp_begin.renderArea.extent.height = targets[0]->height;
	rp_begin.clearValueCount = count + 1;
	rp_begin.pClearValues = clear_values;

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

		VkAttachmentReference depth_reference = {0};
		depth_reference.attachment = count;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {0};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = NULL;
		subpass.colorAttachmentCount = count;
		subpass.pColorAttachments = color_references;
		subpass.pResolveAttachments = NULL;
		subpass.pDepthStencilAttachment = targets[0]->impl.depthBufferBits > 0 ? &depth_reference : NULL;
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
		rp_info.attachmentCount = targets[0]->impl.depthBufferBits > 0 ? count + 1 : count;
		rp_info.pAttachments = attachments;
		rp_info.subpassCount = 1;
		rp_info.pSubpasses = &subpass;
		rp_info.dependencyCount = 2;
		rp_info.pDependencies = dependencies;

		VkResult err = vkCreateRenderPass(vk_ctx.device, &rp_info, NULL, &mrtRenderPass[mrtIndex]);
		assert(!err);

		VkImageView attachmentsViews[9];
		for (int i = 0; i < count; ++i) {
			attachmentsViews[i] = targets[i]->impl.sourceView;
		}
		if (targets[0]->impl.depthBufferBits > 0) {
			attachmentsViews[count] = targets[0]->impl.depthView;
		}

		VkFramebufferCreateInfo fbufCreateInfo = {0};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		fbufCreateInfo.renderPass = mrtRenderPass[mrtIndex];
		fbufCreateInfo.attachmentCount = targets[0]->impl.depthBufferBits > 0 ? count + 1 : count;
		fbufCreateInfo.pAttachments = attachmentsViews;
		fbufCreateInfo.width = targets[0]->width;
		fbufCreateInfo.height = targets[0]->height;
		fbufCreateInfo.layers = 1;

		err = vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &mrtFramebuffer[mrtIndex]);
		assert(!err);

		rp_begin.renderPass = mrtRenderPass[mrtIndex];
		rp_begin.framebuffer = mrtFramebuffer[mrtIndex];
		mrtIndex++;
	}

	vkCmdBeginRenderPass(list->impl._buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	currentRenderPassBeginInfo = rp_begin;
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
		currentVulkanPipeline = current_pipeline->impl.rendertarget_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.rendertarget_pipeline);
	}
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {
	VkFormat format = render_target->impl.format;
	int format_bytes_size = format_size(format);

	// Create readback buffer
	if (!render_target->impl.readbackBufferCreated) {
		VkBufferCreateInfo buf_info = {0};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.pNext = NULL;
		buf_info.size = render_target->width * render_target->height * format_bytes_size;
		buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		buf_info.flags = 0;
		vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &render_target->impl.readbackBuffer);

		VkMemoryRequirements mem_reqs = {0};
		vkGetBufferMemoryRequirements(vk_ctx.device, render_target->impl.readbackBuffer, &mem_reqs);

		VkMemoryAllocateInfo mem_alloc;
		memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;
		mem_alloc.allocationSize = mem_reqs.size;
		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
		vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &render_target->impl.readbackMemory);
		vkBindBufferMemory(vk_ctx.device, render_target->impl.readbackBuffer, render_target->impl.readbackMemory, 0);

		render_target->impl.readbackBufferCreated = true;
	}

	vkCmdEndRenderPass(list->impl._buffer);
	setImageLayout(list->impl._buffer, render_target->impl.sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
	vkCmdCopyImageToBuffer(list->impl._buffer, render_target->impl.sourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, render_target->impl.readbackBuffer, 1,
	                       &region);

	setImageLayout(list->impl._buffer, render_target->impl.sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vkCmdBeginRenderPass(list->impl._buffer, &currentRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	in_render_pass = true;

	kinc_g5_command_list_end(list);
	kinc_g5_command_list_execute(list);
	kinc_g5_command_list_wait_for_execution_to_finish(list);
	kinc_g5_command_list_begin(list);

	// Read buffer
	void *p;
	vkMapMemory(vk_ctx.device, render_target->impl.readbackMemory, 0, VK_WHOLE_SIZE, 0, (void **)&p);
	memcpy(data, p, render_target->width * render_target->height * format_bytes_size);
	vkUnmapMemory(vk_ctx.device, render_target->impl.readbackMemory);
}

void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {
	// render-passes are used to transition render-targets
}

void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {
	// render-passes are used to transition render-targets
}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	lastVertexConstantBufferOffset = offset;
}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	lastFragmentConstantBufferOffset = offset;

	VkDescriptorSet descriptor_set = getDescriptorSet();
	uint32_t offsets[2] = {lastVertexConstantBufferOffset, lastFragmentConstantBufferOffset};
	vkCmdBindDescriptorSets(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline->impl.pipeline_layout, 0, 1, &descriptor_set, 2, offsets);
}

void kinc_g5_command_list_set_compute_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	lastComputeConstantBufferOffset = offset;

	VkDescriptorSet descriptor_set = get_compute_descriptor_set();
	uint32_t offsets[2] = {lastComputeConstantBufferOffset, lastComputeConstantBufferOffset};
	vkCmdBindDescriptorSets(list->impl._buffer, VK_PIPELINE_BIND_POINT_COMPUTE, current_compute_shader->impl.pipeline_layout, 0, 1, &descriptor_set, 2,
	                        offsets);
}

static bool wait_for_framebuffer = false;

static void command_list_should_wait_for_framebuffer(void) {
	wait_for_framebuffer = true;
}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {
	// Make sure the previous execution is done, so we can reuse the fence
	// Not optimal of course
	VkResult err = vkWaitForFences(vk_ctx.device, 1, &list->impl.fence, VK_TRUE, UINT64_MAX);
	assert(!err);
	vkResetFences(vk_ctx.device, 1, &list->impl.fence);

	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;

	VkSemaphore semaphores[2] = {framebuffer_available, relay_semaphore};
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

	err = vkQueueSubmit(vk_ctx.queue, 1, &submit_info, list->impl.fence);
	assert(!err);
}

void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list) {
	VkResult err = vkWaitForFences(vk_ctx.device, 1, &list->impl.fence, VK_TRUE, UINT64_MAX);
	assert(!err);
}

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	vulkan_textures[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = texture;
	vulkan_render_targets[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = NULL;
}

void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		vulkan_samplers[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = sampler->impl.sampler;
	}
}

void kinc_g5_command_list_set_texture_from_render_target(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *target) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		target->impl.stage = unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT];
		vulkan_render_targets[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = target;
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		target->impl.stage = unit.stages[KINC_G5_SHADER_TYPE_VERTEX];
		vulkan_render_targets[unit.stages[KINC_G5_SHADER_TYPE_VERTEX]] = target;
	}
	vulkan_textures[target->impl.stage] = NULL;
}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *target) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		target->impl.stage_depth = unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT];
		vulkan_render_targets[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = target;
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		target->impl.stage_depth = unit.stages[KINC_G5_SHADER_TYPE_VERTEX];
		vulkan_render_targets[unit.stages[KINC_G5_SHADER_TYPE_VERTEX]] = target;
	}
	vulkan_textures[target->impl.stage_depth] = NULL;
}

void kinc_g5_command_list_set_compute_shader(kinc_g5_command_list_t *list, kinc_g5_compute_shader *shader) {
	current_compute_shader = shader;
	vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->impl.pipeline);
	//**vkCmdBindDescriptorSets(list->impl._buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->impl.pipeline_layout, 0, 1, &shader->impl.descriptor_set, 0, 0);
}

void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z) {
	if (in_render_pass) {
		vkCmdEndRenderPass(list->impl._buffer);
		in_render_pass = false;
	}

	vkCmdDispatch(list->impl._buffer, x, y, z);

	int render_target_count = 0;
	for (int i = 0; i < 8; ++i) {
		if (currentRenderTargets[i] == NULL) {
			break;
		}
		++render_target_count;
	}
	if (render_target_count > 0) {
		kinc_g5_command_list_set_render_targets(list, currentRenderTargets, render_target_count);
	}
}




















static void parse_shader(uint32_t *shader_source, int shader_length, kinc_internal_named_number *locations, kinc_internal_named_number *textureBindings, kinc_internal_named_number *uniformOffsets);
static VkShaderModule create_shader_module(const void *code, size_t size);
static VkDescriptorPool compute_descriptor_pool;

static void create_compute_descriptor_layout(void) {
	VkDescriptorSetLayoutBinding layoutBindings[18];
	memset(layoutBindings, 0, sizeof(layoutBindings));

	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	layoutBindings[0].pImmutableSamplers = NULL;

	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	layoutBindings[1].pImmutableSamplers = NULL;

	for (int i = 2; i < 18; ++i) {
		layoutBindings[i].binding = i;
		layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		layoutBindings[i].descriptorCount = 1;
		layoutBindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[i].pImmutableSamplers = NULL;
	}

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {0};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 18;
	descriptor_layout.pBindings = layoutBindings;

	VkResult err = vkCreateDescriptorSetLayout(vk_ctx.device, &descriptor_layout, NULL, &compute_descriptor_layout);
	assert(!err);

	VkDescriptorPoolSize typeCounts[2];
	memset(typeCounts, 0, sizeof(typeCounts));

	typeCounts[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	typeCounts[0].descriptorCount = 2 * 1024;

	typeCounts[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	typeCounts[1].descriptorCount = 16 * 1024;

	VkDescriptorPoolCreateInfo pool_info = {0};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.pNext = NULL;
	pool_info.maxSets = 1024;
	pool_info.poolSizeCount = 2;
	pool_info.pPoolSizes = typeCounts;

	err = vkCreateDescriptorPool(vk_ctx.device, &pool_info, NULL, &compute_descriptor_pool);
	assert(!err);
}

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *_data, int length) {
	memset(shader->impl.locations, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(shader->impl.offsets, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(shader->impl.texture_bindings, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	parse_shader((uint32_t *)_data, length, shader->impl.locations, shader->impl.texture_bindings, shader->impl.offsets);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {0};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = &compute_descriptor_layout;

	VkResult err = vkCreatePipelineLayout(vk_ctx.device, &pPipelineLayoutCreateInfo, NULL, &shader->impl.pipeline_layout);
	assert(!err);

	VkComputePipelineCreateInfo pipeline_info = {0};

	memset(&pipeline_info, 0, sizeof(pipeline_info));
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.layout = shader->impl.pipeline_layout;

	VkPipelineShaderStageCreateInfo shader_stage;
	memset(&shader_stage, 0, sizeof(VkPipelineShaderStageCreateInfo));

	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader->impl.shader_module = create_shader_module(_data, (size_t)length);
	shader_stage.module = shader->impl.shader_module;
	shader_stage.pName = "main";

	pipeline_info.stage = shader_stage;

	err = vkCreateComputePipelines(vk_ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &shader->impl.pipeline);
	assert(!err);

	vkDestroyShaderModule(vk_ctx.device, shader->impl.shader_module, NULL);
}

void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader) {}

kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_constant_location_t location = {0};

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	return location;
}

kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63)
		len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_g5_texture_unit_t unit;
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}
	unit.stages[KINC_G5_SHADER_TYPE_COMPUTE] = 0;

	return unit;
}



























VkDescriptorSetLayout desc_layout;
extern kinc_g5_texture_t *vulkan_textures[16];
extern kinc_g5_render_target_t *vulkan_render_targets[16];
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

static VkShaderModule prepare_vs(VkShaderModule *vert_shader_module, kinc_g5_shader_t *vertex_shader) {
	*vert_shader_module = create_shader_module(vertex_shader->impl.source, vertex_shader->impl.length);
	return *vert_shader_module;
}

static VkShaderModule prepare_fs(VkShaderModule *frag_shader_module, kinc_g5_shader_t *fragment_shader) {
	*frag_shader_module = create_shader_module(fragment_shader->impl.source, fragment_shader->impl.length);
	return *frag_shader_module;
}

static VkFormat convert_format(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case KINC_IMAGE_FORMAT_R32:
		return VK_FORMAT_R32_SFLOAT;
	case KINC_IMAGE_FORMAT_R16:
		return VK_FORMAT_R16_SFLOAT;
	case KINC_IMAGE_FORMAT_R8:
		return VK_FORMAT_R8_UNORM;
	case KINC_IMAGE_FORMAT_RGBA32:
	default:
		return VK_FORMAT_B8G8R8A8_UNORM;
	}
}

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline) {
	kinc_g5_internal_pipeline_init(pipeline);
	kinc_g5_internal_pipeline_set_defaults(pipeline);
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

static VkCullModeFlagBits convert_cull_mode(kinc_g5_cull_mode_t cull_mode) {
	switch (cull_mode) {
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
	parse_shader((uint32_t *)pipeline->vertex_shader->impl.source, pipeline->vertex_shader->impl.length, pipeline->impl.vertexLocations,
	             pipeline->impl.vertexTextureBindings, pipeline->impl.vertexOffsets);
	parse_shader((uint32_t *)pipeline->fragment_shader->impl.source, pipeline->fragment_shader->impl.length, pipeline->impl.fragmentLocations,
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
	int vertexAttributeCount = pipeline->input_layout->size;
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
	for (int i = 0; i < pipeline->input_layout->size; ++i) {
		kinc_g5_vertex_element_t element = pipeline->input_layout->elements[i];

		vi_attrs[attr].binding = 0;
		vi_attrs[attr].location = find_number(pipeline->impl.vertexLocations, element.name);
		vi_attrs[attr].offset = offset;
		offset += kinc_g5_vertex_data_size(element.data);

		switch (element.data) {
		case KINC_G5_VERTEX_DATA_F32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_SFLOAT;
			break;
		case KINC_G5_VERTEX_DATA_F32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_SFLOAT;
			break;
		case KINC_G5_VERTEX_DATA_F32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case KINC_G5_VERTEX_DATA_F32_4X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case KINC_G5_VERTEX_DATA_I8_1X:
			vi_attrs[attr].format = VK_FORMAT_R8_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U8_1X:
			vi_attrs[attr].format = VK_FORMAT_R8_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I8_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8_SNORM;
			break;
		case KINC_G5_VERTEX_DATA_U8_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8_UNORM;
			break;
		case KINC_G5_VERTEX_DATA_I8_2X:
			vi_attrs[attr].format = VK_FORMAT_R8G8_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U8_2X:
			vi_attrs[attr].format = VK_FORMAT_R8G8_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I8_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8_SNORM;
			break;
		case KINC_G5_VERTEX_DATA_U8_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8_UNORM;
			break;
		case KINC_G5_VERTEX_DATA_I8_4X:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U8_4X:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I8_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_SNORM;
			break;
		case KINC_G5_VERTEX_DATA_U8_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case KINC_G5_VERTEX_DATA_I16_1X:
			vi_attrs[attr].format = VK_FORMAT_R16_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U16_1X:
			vi_attrs[attr].format = VK_FORMAT_R16_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I16_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16_SNORM;
			break;
		case KINC_G5_VERTEX_DATA_U16_1X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16_UNORM;
			break;
		case KINC_G5_VERTEX_DATA_I16_2X:
			vi_attrs[attr].format = VK_FORMAT_R16G16_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U16_2X:
			vi_attrs[attr].format = VK_FORMAT_R16G16_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I16_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16_SNORM;
			break;
		case KINC_G5_VERTEX_DATA_U16_2X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16_UNORM;
			break;
		case KINC_G5_VERTEX_DATA_I16_4X:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U16_4X:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I16_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_SNORM;
			break;
		case KINC_G5_VERTEX_DATA_U16_4X_NORMALIZED:
			vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_UNORM;
			break;
		case KINC_G5_VERTEX_DATA_I32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U32_1X:
			vi_attrs[attr].format = VK_FORMAT_R32_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U32_2X:
			vi_attrs[attr].format = VK_FORMAT_R32G32_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U32_3X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32_UINT;
			break;
		case KINC_G5_VERTEX_DATA_I32_4X:
			vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SINT;
			break;
		case KINC_G5_VERTEX_DATA_U32_4X:
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
		att_state[i].blendEnable = pipeline->blend_source != KINC_G5_BLEND_ONE ||
								   pipeline->blend_destination != KINC_G5_BLEND_ZERO ||
		                           pipeline->alpha_blend_source != KINC_G5_BLEND_ONE ||
								   pipeline->alpha_blend_destination != KINC_G5_BLEND_ZERO;
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
	ds.depthTestEnable = pipeline->depth_mode != KINC_G5_COMPARE_MODE_ALWAYS;
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

	err = vkCreateGraphicsPipelines(vk_ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->impl.framebuffer_pipeline);
	assert(!err);

	VkAttachmentDescription attachments[9];
	for (int i = 0; i < pipeline->color_attachment_count; ++i) {
		attachments[i].format = convert_format(pipeline->color_attachment[i]);
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

	VkAttachmentReference depth_reference = {0};
	depth_reference.attachment = pipeline->color_attachment_count;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = pipeline->color_attachment_count;
	subpass.pColorAttachments = color_references;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = pipeline->depth_attachment_bits > 0 ? &depth_reference : NULL;
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
	rp_info.attachmentCount = pipeline->depth_attachment_bits > 0 ? pipeline->color_attachment_count + 1 : pipeline->color_attachment_count;
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

void create_descriptor_layout(void) {
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
		if (vulkan_textures[i] != NULL) {
			texture_count++;
		}
		else if (vulkan_render_targets[i] != NULL) {
			texture_count++;
		}
	}

	bool uniform_buffer = vk_ctx.vertex_uniform_buffer != NULL && vk_ctx.fragment_uniform_buffer != NULL;

	return 1 | (texture_count << 1) | ((uniform_buffer ? 1 : 0) << 8);
}

int calc_compute_descriptor_id(void) {
	int texture_count = 0;
	for (int i = 0; i < 16; ++i) {
		if (vulkan_textures[i] != NULL) {
			texture_count++;
		}
		else if (vulkan_render_targets[i] != NULL) {
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
		if (vulkan_textures[i] != NULL) {
			tex_descs[i].sampler = vulkan_samplers[i];
			tex_descs[i].imageView = vulkan_textures[i]->impl.view;
			texture_count++;
		}
		else if (vulkan_render_targets[i] != NULL) {
			tex_descs[i].sampler = vulkan_samplers[i];
			if (vulkan_render_targets[i]->impl.stage_depth == i) {
				tex_descs[i].imageView = vulkan_render_targets[i]->impl.depthView;
				vulkan_render_targets[i]->impl.stage_depth = -1;
			}
			else {
				tex_descs[i].imageView = vulkan_render_targets[i]->impl.sourceView;
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

	if (vulkan_textures[0] != NULL || vulkan_render_targets[0] != NULL) {
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
		if (vulkan_textures[i] != NULL) {
			assert(vulkan_samplers[i] != VK_NULL_HANDLE);
			tex_desc[i].sampler = vulkan_samplers[i];
			tex_desc[i].imageView = vulkan_textures[i]->impl.view;
			texture_count++;
		}
		else if (vulkan_render_targets[i] != NULL) {
			tex_desc[i].sampler = vulkan_samplers[i];
			if (vulkan_render_targets[i]->impl.stage_depth == i) {
				tex_desc[i].imageView = vulkan_render_targets[i]->impl.depthView;
				vulkan_render_targets[i]->impl.stage_depth = -1;
			}
			else {
				tex_desc[i].imageView = vulkan_render_targets[i]->impl.sourceView;
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

	if (vulkan_textures[0] != NULL || vulkan_render_targets[0] != NULL) {
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
		if (vulkan_textures[i] != NULL) {
			tex_descs[i].sampler = vulkan_samplers[i];
			tex_descs[i].imageView = vulkan_textures[i]->impl.view;
			texture_count++;
		}
		else if (vulkan_render_targets[i] != NULL) {
			tex_descs[i].sampler = vulkan_samplers[i];
			if (vulkan_render_targets[i]->impl.stage_depth == i) {
				tex_descs[i].imageView = vulkan_render_targets[i]->impl.depthView;
				vulkan_render_targets[i]->impl.stage_depth = -1;
			}
			else {
				tex_descs[i].imageView = vulkan_render_targets[i]->impl.sourceView;
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

	if (vulkan_textures[0] != NULL || vulkan_render_targets[0] != NULL) {
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
		if (vulkan_textures[i] != NULL) {
			// assert(vulkan_samplers[i] != VK_NULL_HANDLE);
			tex_desc[i].sampler = VK_NULL_HANDLE; // vulkan_samplers[i];
			tex_desc[i].imageView = vulkan_textures[i]->impl.view;
			texture_count++;
		}
		else if (vulkan_render_targets[i] != NULL) {
			tex_desc[i].sampler = vulkan_samplers[i];
			if (vulkan_render_targets[i]->impl.stage_depth == i) {
				tex_desc[i].imageView = vulkan_render_targets[i]->impl.depthView;
				vulkan_render_targets[i]->impl.stage_depth = -1;
			}
			else {
				tex_desc[i].imageView = vulkan_render_targets[i]->impl.sourceView;
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

	if (vulkan_textures[0] != NULL || vulkan_render_targets[0] != NULL) {
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

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type) {
	shader->impl.length = (int)length;
	shader->impl.id = 0;
	shader->impl.source = (char *)malloc(length + 1);
	for (int i = 0; i < length; ++i) {
		shader->impl.source[i] = ((char *)source)[i];
	}
	shader->impl.source[length] = 0;
}

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {
	free(shader->impl.source);
	shader->impl.source = NULL;
}

static VkCompareOp convert_compare_mode(kinc_g5_compare_mode_t compare);

static VkSamplerAddressMode convert_addressing(kinc_g5_texture_addressing_t mode) {
	switch (mode) {
	case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case KINC_G5_TEXTURE_ADDRESSING_BORDER:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	default:
		assert(false);
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

static VkSamplerMipmapMode convert_mipmap_mode(kinc_g5_mipmap_filter_t filter) {
	switch (filter) {
	case KINC_G5_MIPMAP_FILTER_NONE:
	case KINC_G5_MIPMAP_FILTER_POINT:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case KINC_G5_MIPMAP_FILTER_LINEAR:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	default:
		assert(false);
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
}

static VkFilter convert_texture_filter(kinc_g5_texture_filter_t filter) {
	switch (filter) {
	case KINC_G5_TEXTURE_FILTER_POINT:
		return VK_FILTER_NEAREST;
	case KINC_G5_TEXTURE_FILTER_LINEAR:
		return VK_FILTER_LINEAR;
	case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
		return VK_FILTER_LINEAR; // ?
	default:
		assert(false);
		return VK_FILTER_NEAREST;
	}
}

void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options) {
	VkSamplerCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;

	info.addressModeU = convert_addressing(options->u_addressing);
	info.addressModeV = convert_addressing(options->v_addressing);
	info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	info.mipmapMode = convert_mipmap_mode(options->mipmap_filter);

	info.magFilter = convert_texture_filter(options->magnification_filter);
	info.minFilter = convert_texture_filter(options->minification_filter);

	info.anisotropyEnable =
	    (options->magnification_filter == KINC_G5_TEXTURE_FILTER_ANISOTROPIC || options->minification_filter == KINC_G5_TEXTURE_FILTER_ANISOTROPIC);
	info.maxAnisotropy = 1;

	info.maxLod = 32;
	info.minLod = 0;

	vkCreateSampler(vk_ctx.device, &info, NULL, &sampler->impl.sampler);
}

void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler) {
	vkDestroySampler(vk_ctx.device, sampler->impl.sampler, NULL);
}



























































bool use_staging_buffer = false;

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
void set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);

static void prepare_texture_image(uint8_t *tex_colors, uint32_t width, uint32_t height, Texture5Impl *tex_obj, VkImageTiling tiling,
                                  VkImageUsageFlags usage, VkFlags required_props, VkDeviceSize *deviceSize, VkFormat tex_format) {
	VkResult err;
	bool pass;

	tex_obj->width = width;
	tex_obj->height = height;

	VkImageCreateInfo image_create_info = {0};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext = NULL;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = tex_format;
	image_create_info.extent.width = width;
	image_create_info.extent.height = height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = tiling;
	image_create_info.usage = usage;
	image_create_info.flags = 0;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	VkMemoryAllocateInfo mem_alloc = {0};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements mem_reqs;

	err = vkCreateImage(vk_ctx.device, &image_create_info, NULL, &tex_obj->image);
	assert(!err);

	vkGetImageMemoryRequirements(vk_ctx.device, tex_obj->image, &mem_reqs);

	*deviceSize = mem_alloc.allocationSize = mem_reqs.size;
	pass = memory_type_from_properties(mem_reqs.memoryTypeBits, required_props, &mem_alloc.memoryTypeIndex);
	assert(pass);

	// allocate memory
	err = vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &tex_obj->mem);
	assert(!err);

	// bind memory
	err = vkBindImageMemory(vk_ctx.device, tex_obj->image, tex_obj->mem, 0);
	assert(!err);

	if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && tex_colors != NULL) {
		VkImageSubresource subres = {0};
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = 0;
		subres.arrayLayer = 0;

		VkSubresourceLayout layout;
		uint8_t *data;

		vkGetImageSubresourceLayout(vk_ctx.device, tex_obj->image, &subres, &layout);

		err = vkMapMemory(vk_ctx.device, tex_obj->mem, 0, mem_alloc.allocationSize, 0, (void **)&data);
		assert(!err);

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
	// setting the image layout does not reference the actual memory so no need to add a mem ref
}

static void destroy_texture_image(Texture5Impl *tex_obj) {
	// clean up staging resources
	vkDestroyImage(vk_ctx.device, tex_obj->image, NULL);
	vkFreeMemory(vk_ctx.device, tex_obj->mem, NULL);
}

static VkFormat convert_image_format(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case KINC_IMAGE_FORMAT_R8:
		return VK_FORMAT_R8_UNORM;
	case KINC_IMAGE_FORMAT_R16:
		return VK_FORMAT_R16_SFLOAT;
	case KINC_IMAGE_FORMAT_R32:
		return VK_FORMAT_R32_SFLOAT;
	case KINC_IMAGE_FORMAT_BGRA32:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case KINC_IMAGE_FORMAT_RGBA32:
		return VK_FORMAT_R8G8B8A8_UNORM;
	default:
		return VK_FORMAT_B8G8R8A8_UNORM;
	}
}

static int format_byte_size(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_R16:
		return 2;
	case KINC_IMAGE_FORMAT_R8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_R32:
	default:
		return 4;
	}
}

static void update_stride(kinc_g5_texture_t *texture) {
	VkImageSubresource subres = {0};
	subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subres.mipLevel = 0;
	subres.arrayLayer = 0;

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(vk_ctx.device, texture->impl.image, &subres, &layout);

	texture->impl.stride = (int)layout.rowPitch;
}

void kinc_g5_texture_init_from_bytes(kinc_g5_texture_t *texture, void *data, int width, int height, kinc_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->_uploaded = false;
	texture->format = format;
	texture->data = data;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;
	VkResult err;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !use_staging_buffer) {
		// Device can texture using linear textures
		prepare_texture_image((uint8_t *)data, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_LINEAR,
		                      VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize,
		                      tex_format);

		flush_init_cmd();
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
		// Must use staging buffer to copy linear texture to optimized
		Texture5Impl staging_texture;

		memset(&staging_texture, 0, sizeof(staging_texture));
		prepare_texture_image((uint8_t *)data, (uint32_t)width, (uint32_t)height, &staging_texture, VK_IMAGE_TILING_LINEAR,
		                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize, tex_format);
		prepare_texture_image((uint8_t *)data, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_OPTIMAL,
		                      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/),
		                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->impl.deviceSize, tex_format);
		set_image_layout(staging_texture.image, VK_IMAGE_ASPECT_COLOR_BIT, staging_texture.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, texture->impl.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageCopy copy_region = {0};
		copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.srcSubresource.mipLevel = 0;
		copy_region.srcSubresource.baseArrayLayer = 0;
		copy_region.srcSubresource.layerCount = 1;
		copy_region.srcOffset.x = copy_region.srcOffset.y = copy_region.srcOffset.z = 0;
		copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.dstSubresource.mipLevel = 0;
		copy_region.dstSubresource.baseArrayLayer = 0;
		copy_region.dstSubresource.layerCount = 1;
		copy_region.dstOffset.x = copy_region.dstOffset.y = copy_region.dstOffset.z = 0;
		copy_region.extent.width = (uint32_t)staging_texture.width;
		copy_region.extent.height = (uint32_t)staging_texture.height;
		copy_region.extent.depth = 1;

		vkCmdCopyImage(vk_ctx.setup_cmd, staging_texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->impl.image,
		               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

		set_image_layout(texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->impl.imageLayout);

		flush_init_cmd();

		destroy_texture_image(&staging_texture);
	}
	else {
		assert(!"No support for B8G8R8A8_UNORM as texture image format");
	}

	update_stride(texture);

	VkImageViewCreateInfo view = {0};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = tex_format;
	view.components.r = VK_COMPONENT_SWIZZLE_R;
	view.components.g = VK_COMPONENT_SWIZZLE_G;
	view.components.b = VK_COMPONENT_SWIZZLE_B;
	view.components.a = VK_COMPONENT_SWIZZLE_A;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.flags = 0;

	view.image = texture->impl.image;
	err = vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.view);
	assert(!err);
}

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->_uploaded = true;
	texture->format = format;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;
	VkResult err;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	// Device can texture using linear textures
	prepare_texture_image(NULL, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_LINEAR,
	                      VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize,
	                      tex_format);

	flush_init_cmd();

	update_stride(texture);

	VkImageViewCreateInfo view = {0};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = tex_format;
	view.components.r = VK_COMPONENT_SWIZZLE_R;
	view.components.g = VK_COMPONENT_SWIZZLE_G;
	view.components.b = VK_COMPONENT_SWIZZLE_B;
	view.components.a = VK_COMPONENT_SWIZZLE_A;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.flags = 0;

	view.image = texture->impl.image;
	err = vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.view);
	assert(!err);
}

void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->width = width;
	texture->height = height;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;
	VkResult err;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	// Device can texture using linear textures
	prepare_texture_image(NULL, (uint32_t)width, (uint32_t)height, &texture->impl, VK_IMAGE_TILING_OPTIMAL,
	                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->impl.deviceSize,
	                      tex_format);

	flush_init_cmd();

	update_stride(texture);

	VkImageViewCreateInfo view = {0};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = tex_format;
	view.components.r = VK_COMPONENT_SWIZZLE_R;
	view.components.g = VK_COMPONENT_SWIZZLE_G;
	view.components.b = VK_COMPONENT_SWIZZLE_B;
	view.components.a = VK_COMPONENT_SWIZZLE_A;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.flags = 0;

	view.image = texture->impl.image;
	err = vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.view);
	assert(!err);
}

void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {
	vkDestroyImageView(vk_ctx.device, texture->impl.view, NULL);
	destroy_texture_image(&texture->impl);
}

void kinc_g5_internal_texture_set(kinc_g5_texture_t *texture, int unit) {}

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	return texture->impl.stride;
}

uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture) {
	void *data;

	VkResult err = vkMapMemory(vk_ctx.device, texture->impl.mem, 0, texture->impl.deviceSize, 0, &data);
	assert(!err);

	return (uint8_t *)data;
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *texture) {
	vkUnmapMemory(vk_ctx.device, texture->impl.mem);
	texture->_uploaded = false;
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

extern kinc_g5_command_list_t commandList;

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level) {
    // VkBuffer staging_buffer;
    // VkDeviceMemory staging_buffer_mem;

    // VkBufferCreateInfo buffer_info = {0};
    // buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    // buffer_info.size = mipmap->width * mipmap->height * format_size(mipmap->format);
    // buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    // buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // vkCreateBuffer(vk_ctx.device, &buffer_info, NULL, &staging_buffer);

    // VkMemoryRequirements mem_req;
    // vkGetBufferMemoryRequirements(vk_ctx.device, staging_buffer, &mem_req);

    // VkMemoryAllocateInfo alloc_info = {0};
    // alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    // alloc_info.allocationSize = mem_req.size;

	// memory_type_from_properties(mem_req.memoryTypeBits,
	// 	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_info.memoryTypeIndex);

    // vkAllocateMemory(vk_ctx.device, &alloc_info, NULL, &staging_buffer_mem);
    // vkBindBufferMemory(vk_ctx.device, staging_buffer, staging_buffer_mem, 0);

    // void *mapped_data;
    // vkMapMemory(vk_ctx.device, staging_buffer_mem, 0, buffer_info.size, 0, &mapped_data);
    // memcpy(mapped_data, mipmap->data, (size_t)buffer_info.size);
    // vkUnmapMemory(vk_ctx.device, staging_buffer_mem);

	// setup_init_cmd();
	// setImageLayout(vk_ctx.setup_cmd, texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	// flush_init_cmd();

    // VkBufferImageCopy region = {0};
    // region.bufferOffset = 0;
    // region.bufferRowLength = 0;
    // region.bufferImageHeight = 0;
    // region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // region.imageSubresource.mipLevel = level;
    // region.imageSubresource.baseArrayLayer = 0;
    // region.imageSubresource.layerCount = 1;
    // region.imageOffset = (VkOffset3D){0, 0, 0};
    // region.imageExtent = (VkExtent3D){
    //     (uint32_t)mipmap->width,
    //     (uint32_t)mipmap->height,
    //     1
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

	// setup_init_cmd();
	// setImageLayout(vk_ctx.setup_cmd, texture->impl.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	// flush_init_cmd();

    // vkFreeMemory(vk_ctx.device, staging_buffer_mem, NULL);
    // vkDestroyBuffer(vk_ctx.device, staging_buffer, NULL);

    // texture->_uploaded = true;
}

extern uint32_t swapchainImageCount;
extern kinc_g5_texture_t *vulkan_textures[16];
extern kinc_g5_render_target_t *vulkan_render_targets[16];

void setup_init_cmd();

void setImageLayout(VkCommandBuffer _buffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout) {
	VkImageMemoryBarrier imageMemoryBarrier = {0};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (oldImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (oldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		if (oldImageLayout != VK_IMAGE_LAYOUT_UNDEFINED)
			imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		if (oldImageLayout != VK_IMAGE_LAYOUT_UNDEFINED)
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		if (oldImageLayout != VK_IMAGE_LAYOUT_UNDEFINED)
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	vkCmdPipelineBarrier(_buffer, srcStageFlags, dstStageFlags, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
}

static void render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits, int framebuffer_index) {
	target->framebuffer_index = framebuffer_index;
	target->width = width;
	target->height = height;
	target->impl.format = convert_format(format);
	target->impl.depthBufferBits = depthBufferBits;
	target->impl.stage = 0;
	target->impl.stage_depth = -1;
	target->impl.readbackBufferCreated = false;

	if (framebuffer_index < 0) {
		{
			VkFormatProperties formatProperties;
			VkResult err;

			vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, target->impl.format, &formatProperties);
			assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);

			VkImageCreateInfo image = {0};
			image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image.pNext = NULL;
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = target->impl.format;
			image.extent.width = width;
			image.extent.height = height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = 1;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			image.flags = 0;

			VkImageViewCreateInfo colorImageView = {0};
			colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorImageView.pNext = NULL;
			colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorImageView.format = target->impl.format;
			colorImageView.flags = 0;
			colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorImageView.subresourceRange.baseMipLevel = 0;
			colorImageView.subresourceRange.levelCount = 1;
			colorImageView.subresourceRange.baseArrayLayer = 0;
			colorImageView.subresourceRange.layerCount = 1;

			err = vkCreateImage(vk_ctx.device, &image, NULL, &target->impl.sourceImage);
			assert(!err);

			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(vk_ctx.device, target->impl.sourceImage, &memoryRequirements);

			VkMemoryAllocateInfo allocationInfo = {0};
			allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocationInfo.pNext = NULL;
			allocationInfo.memoryTypeIndex = 0;
			allocationInfo.allocationSize = memoryRequirements.size;
			bool pass = memory_type_from_properties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocationInfo.memoryTypeIndex);
			assert(pass);

			err = vkAllocateMemory(vk_ctx.device, &allocationInfo, NULL, &target->impl.sourceMemory);
			assert(!err);

			err = vkBindImageMemory(vk_ctx.device, target->impl.sourceImage, target->impl.sourceMemory, 0);
			assert(!err);

			setup_init_cmd();
			setImageLayout(vk_ctx.setup_cmd, target->impl.sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
			               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			flush_init_cmd();

			colorImageView.image = target->impl.sourceImage;
			err = vkCreateImageView(vk_ctx.device, &colorImageView, NULL, &target->impl.sourceView);
			assert(!err);
		}

		if (depthBufferBits > 0) {
			// const VkFormat depth_format = VK_FORMAT_D16_UNORM;
			const VkFormat depth_format = VK_FORMAT_D24_UNORM_S8_UINT;
			VkImageCreateInfo image = {0};
			image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image.pNext = NULL;
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = depth_format;
			image.extent.width = width;
			image.extent.height = height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = 1;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			image.flags = 0;

			VkResult err = vkCreateImage(vk_ctx.device, &image, NULL, &target->impl.depthImage);
			assert(!err);

			VkMemoryAllocateInfo mem_alloc = {0};
			mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			mem_alloc.pNext = NULL;
			mem_alloc.allocationSize = 0;
			mem_alloc.memoryTypeIndex = 0;

			VkImageViewCreateInfo view = {0};
			view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.pNext = NULL;
			view.image = target->impl.depthImage;
			view.format = depth_format;
			view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			view.subresourceRange.baseMipLevel = 0;
			view.subresourceRange.levelCount = 1;
			view.subresourceRange.baseArrayLayer = 0;
			view.subresourceRange.layerCount = 1;
			view.flags = 0;
			view.viewType = VK_IMAGE_VIEW_TYPE_2D;

			VkMemoryRequirements mem_reqs = {0};
			bool pass;

			/* get memory requirements for this object */
			vkGetImageMemoryRequirements(vk_ctx.device, target->impl.depthImage, &mem_reqs);

			/* select memory size and type */
			mem_alloc.allocationSize = mem_reqs.size;
			pass = memory_type_from_properties(mem_reqs.memoryTypeBits, 0, /* No requirements */ &mem_alloc.memoryTypeIndex);
			assert(pass);

			/* allocate memory */
			err = vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &target->impl.depthMemory);
			assert(!err);

			/* bind memory */
			err = vkBindImageMemory(vk_ctx.device, target->impl.depthImage, target->impl.depthMemory, 0);
			assert(!err);

			setup_init_cmd();
			setImageLayout(vk_ctx.setup_cmd, target->impl.depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
			               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			flush_init_cmd();

			/* create image view */
			err = vkCreateImageView(vk_ctx.device, &view, NULL, &target->impl.depthView);
			assert(!err);
		}

		VkImageView attachments[2];
		attachments[0] = target->impl.sourceView;

		if (depthBufferBits > 0) {
			attachments[1] = target->impl.depthView;
		}

		VkFramebufferCreateInfo fbufCreateInfo = {0};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		if (framebuffer_index >= 0) {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].framebuffer_render_pass;
		}
		else if (depthBufferBits > 0) {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass_with_depth;
		}
		else {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass;
		}
		fbufCreateInfo.attachmentCount = depthBufferBits > 0 ? 2 : 1;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = width;
		fbufCreateInfo.height = height;
		fbufCreateInfo.layers = 1;

		VkResult err = vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &target->impl.framebuffer);
		assert(!err);
	}
}

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, -1);
	target->width = width;
	target->height = height;
	target->state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
}

static int framebuffer_count = 0;

void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, framebuffer_count);
	framebuffer_count += 1;
}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target) {
	if (target->framebuffer_index >= 0) {
		framebuffer_count -= 1;
	}
	else {
		vkDestroyFramebuffer(vk_ctx.device, target->impl.framebuffer, NULL);
		if (target->impl.depthBufferBits > 0) {
			vkDestroyImageView(vk_ctx.device, target->impl.depthView, NULL);
			vkDestroyImage(vk_ctx.device, target->impl.depthImage, NULL);
			vkFreeMemory(vk_ctx.device, target->impl.depthMemory, NULL);
		}
		vkDestroyImageView(vk_ctx.device, target->impl.sourceView, NULL);
		vkDestroyImage(vk_ctx.device, target->impl.sourceImage, NULL);
		vkFreeMemory(vk_ctx.device, target->impl.sourceMemory, NULL);
	}
}

void kinc_g5_render_target_set_depth_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source) {
	target->impl.depthImage = source->impl.depthImage;
	target->impl.depthMemory = source->impl.depthMemory;
	target->impl.depthView = source->impl.depthView;
	target->impl.depthBufferBits = source->impl.depthBufferBits;

	// vkDestroyFramebuffer(vk_ctx.device, target->impl.framebuffer, nullptr);

	{
		VkImageView attachments[2];
		attachments[0] = target->impl.sourceView;

		if (target->impl.depthBufferBits > 0) {
			attachments[1] = target->impl.depthView;
		}

		VkFramebufferCreateInfo fbufCreateInfo = {0};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = NULL;
		if (target->impl.depthBufferBits > 0) {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass_with_depth;
		}
		else {
			fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass;
		}
		fbufCreateInfo.attachmentCount = target->impl.depthBufferBits > 0 ? 2 : 1;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = target->width;
		fbufCreateInfo.height = target->height;
		fbufCreateInfo.layers = 1;

		VkResult err = vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &target->impl.framebuffer);
		assert(!err);
	}
}
































bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int vertexCount, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.myCount = vertexCount;
	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g5_vertex_element_t element = structure->elements[i];
		buffer->impl.myStride += kinc_g5_vertex_data_size(element.data);
	}

	VkBufferCreateInfo buf_info = {0};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = vertexCount * buffer->impl.myStride;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	if (kinc_g5_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	buf_info.flags = 0;

	memset(&buffer->impl.mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = 0;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements mem_reqs = {0};
	VkResult err;
	bool pass;

	buffer->impl.buf = NULL;
	buffer->impl.mem = NULL;

	err = vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &buffer->impl.buf);
	assert(!err);

	vkGetBufferMemoryRequirements(vk_ctx.device, buffer->impl.buf, &mem_reqs);
	assert(!err);

	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);
	assert(pass);

	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
	if (kinc_g5_raytrace_supported()) {
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		buffer->impl.mem_alloc.pNext = &memory_allocate_flags_info;
	}

	err = vkAllocateMemory(vk_ctx.device, &buffer->impl.mem_alloc, NULL, &buffer->impl.mem);
	assert(!err);

	err = vkBindBufferMemory(vk_ctx.device, buffer->impl.buf, buffer->impl.mem, 0);
	assert(!err);
}

static void unset_vertex_buffer(kinc_g5_vertex_buffer_t *buffer) {

}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {
	unset_vertex_buffer(buffer);
	vkFreeMemory(vk_ctx.device, buffer->impl.mem, NULL);
	vkDestroyBuffer(vk_ctx.device, buffer->impl.buf, NULL);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_lock(buffer, 0, buffer->impl.myCount);
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count) {
	VkResult err =
	    vkMapMemory(vk_ctx.device, buffer->impl.mem, start * buffer->impl.myStride, count * buffer->impl.myStride, 0, (void **)&buffer->impl.data);
	assert(!err);
	return buffer->impl.data;
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
}

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer) {
	return 0;
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}

bool kinc_g5_transpose_mat = true;

static void createUniformBuffer(VkBuffer *buf, VkMemoryAllocateInfo *mem_alloc, VkDeviceMemory *mem, VkDescriptorBufferInfo *buffer_info, int size) {
	VkBufferCreateInfo buf_info;
	memset(&buf_info, 0, sizeof(buf_info));
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	if (kinc_g5_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	buf_info.size = size;
	VkResult err = vkCreateBuffer(vk_ctx.device, &buf_info, NULL, buf);
	assert(!err);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(vk_ctx.device, *buf, &mem_reqs);

	mem_alloc->sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc->pNext = NULL;
	mem_alloc->allocationSize = mem_reqs.size;
	mem_alloc->memoryTypeIndex = 0;

	bool pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc->memoryTypeIndex);
	assert(pass);

	err = vkAllocateMemory(vk_ctx.device, mem_alloc, NULL, mem);
	assert(!err);

	err = vkBindBufferMemory(vk_ctx.device, *buf, *mem, 0);
	assert(!err);

	buffer_info->buffer = *buf;
	buffer_info->offset = 0;
	buffer_info->range = size;
}

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;

	createUniformBuffer(&buffer->impl.buf, &buffer->impl.mem_alloc, &buffer->impl.mem, &buffer->impl.buffer_info, size);

	// buffer hack
	if (vk_ctx.vertex_uniform_buffer == NULL) {
		vk_ctx.vertex_uniform_buffer = &buffer->impl.buf;
	}
	else if (vk_ctx.fragment_uniform_buffer == NULL) {
		vk_ctx.fragment_uniform_buffer = &buffer->impl.buf;
	}
	else if (vk_ctx.compute_uniform_buffer == NULL) {
		vk_ctx.compute_uniform_buffer = &buffer->impl.buf;
	}

	void *p;
	VkResult err = vkMapMemory(vk_ctx.device, buffer->impl.mem, 0, buffer->impl.mem_alloc.allocationSize, 0, (void **)&p);
	assert(!err);
	memset(p, 0, buffer->impl.mem_alloc.allocationSize);
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {
	vkFreeMemory(vk_ctx.device, buffer->impl.mem, NULL);
	vkDestroyBuffer(vk_ctx.device, buffer->impl.buf, NULL);
}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {
	VkResult err = vkMapMemory(vk_ctx.device, buffer->impl.mem, start, count, 0, (void **)&buffer->data);
	assert(!err);
}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
	buffer->data = NULL;
}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}

static void unset(kinc_g5_index_buffer_t *buffer) {
}

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int indexCount, bool gpuMemory) {
	buffer->impl.count = indexCount;

	VkBufferCreateInfo buf_info = {0};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = indexCount * sizeof(uint32_t);
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if (kinc_g5_raytrace_supported()) {
		buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buf_info.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	buf_info.flags = 0;

	memset(&buffer->impl.mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = 0;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	buffer->impl.buf = NULL;
	buffer->impl.mem = NULL;

	VkResult err = vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &buffer->impl.buf);
	assert(!err);

	VkMemoryRequirements mem_reqs = {0};
	vkGetBufferMemoryRequirements(vk_ctx.device, buffer->impl.buf, &mem_reqs);

	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	bool pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);
	assert(pass);

	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {0};
	if (kinc_g5_raytrace_supported()) {
		memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		buffer->impl.mem_alloc.pNext = &memory_allocate_flags_info;
	}

	err = vkAllocateMemory(vk_ctx.device, &buffer->impl.mem_alloc, NULL, &buffer->impl.mem);
	assert(!err);

	err = vkBindBufferMemory(vk_ctx.device, buffer->impl.buf, buffer->impl.mem, 0);
	assert(!err);
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	unset(buffer);
	vkFreeMemory(vk_ctx.device, buffer->impl.mem, NULL);
	vkDestroyBuffer(vk_ctx.device, buffer->impl.buf, NULL);
}

static int kinc_g5_internal_index_buffer_stride(kinc_g5_index_buffer_t *buffer) {
	return 4;
}

void *kinc_g5_index_buffer_lock_all(kinc_g5_index_buffer_t *buffer) {
	return kinc_g5_index_buffer_lock(buffer, 0, kinc_g5_index_buffer_count(buffer));
}

void *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer, int start, int count) {
	uint8_t *data;
	VkResult err = vkMapMemory(vk_ctx.device, buffer->impl.mem, 0, buffer->impl.mem_alloc.allocationSize, 0, (void **)&data);
	assert(!err);
	return &data[start * kinc_g5_internal_index_buffer_stride(buffer)];
}

void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.mem);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
	kinc_g5_index_buffer_unlock_all(buffer);
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}










































#ifndef KINC_ANDROID

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
static kinc_g5_raytrace_acceleration_structure_t *accel;
static kinc_g5_raytrace_pipeline_t *pipeline;
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

void kinc_g5_raytrace_pipeline_init(kinc_g5_raytrace_pipeline_t *pipeline, kinc_g5_command_list_t *command_list, void *ray_shader, int ray_shader_size,
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

void kinc_g5_raytrace_pipeline_destroy(kinc_g5_raytrace_pipeline_t *pipeline) {
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

void kinc_g5_raytrace_acceleration_structure_init(kinc_g5_raytrace_acceleration_structure_t *accel) {
	_vkGetBufferDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetBufferDeviceAddressKHR");
	_vkCreateAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkCreateAccelerationStructureKHR");
	_vkGetAccelerationStructureDeviceAddressKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetAccelerationStructureDeviceAddressKHR");
	_vkGetAccelerationStructureBuildSizesKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkGetAccelerationStructureBuildSizesKHR");

	vb_count = 0;
	instances_count = 0;
}

void kinc_g5_raytrace_acceleration_structure_add(kinc_g5_raytrace_acceleration_structure_t *accel, kinc_g5_vertex_buffer_t *_vb, kinc_g5_index_buffer_t *_ib,
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

void _kinc_g5_raytrace_acceleration_structure_destroy_bottom(kinc_g5_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkDestroyAccelerationStructureKHR");
	for (int i = 0; i < vb_count_last; ++i) {
		_vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.bottom_level_acceleration_structure[i], NULL);
		vkFreeMemory(vk_ctx.device, accel->impl.bottom_level_mem[i], NULL);
		vkDestroyBuffer(vk_ctx.device, accel->impl.bottom_level_buffer[i], NULL);
	}
}

void _kinc_g5_raytrace_acceleration_structure_destroy_top(kinc_g5_raytrace_acceleration_structure_t *accel) {
	_vkDestroyAccelerationStructureKHR = (void *)vkGetDeviceProcAddr(vk_ctx.device, "vkDestroyAccelerationStructureKHR");
	_vkDestroyAccelerationStructureKHR(vk_ctx.device, accel->impl.top_level_acceleration_structure, NULL);
	vkFreeMemory(vk_ctx.device, accel->impl.top_level_mem, NULL);
	vkDestroyBuffer(vk_ctx.device, accel->impl.top_level_buffer, NULL);
	vkFreeMemory(vk_ctx.device, accel->impl.instances_mem, NULL);
	vkDestroyBuffer(vk_ctx.device, accel->impl.instances_buffer, NULL);
}

void kinc_g5_raytrace_acceleration_structure_build(kinc_g5_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list,
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
			_kinc_g5_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_kinc_g5_raytrace_acceleration_structure_destroy_top(accel);
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

			vertex_data_device_address.deviceAddress = get_buffer_device_address(vb[i]->impl.buf);
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

		vb_full = _vb_full->impl.buf;
		vb_full_mem = _vb_full->impl.mem;

		#else

		vb_full = vb[0]->impl.buf;
		vb_full_mem = vb[0]->impl.mem;

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

void kinc_g5_raytrace_acceleration_structure_destroy(kinc_g5_raytrace_acceleration_structure_t *accel) {
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

void kinc_g5_raytrace_set_textures(kinc_g5_render_target_t *_texpaint0, kinc_g5_render_target_t *_texpaint1, kinc_g5_render_target_t *_texpaint2, kinc_g5_texture_t *_texenv, kinc_g5_texture_t *_texsobol, kinc_g5_texture_t *_texscramble, kinc_g5_texture_t *_texrank) {
	texpaint0 = _texpaint0;
	texpaint1 = _texpaint1;
	texpaint2 = _texpaint2;
	texenv = _texenv;
	texsobol = _texsobol;
	texscramble = _texscramble;
	texrank = _texrank;
}

void kinc_g5_raytrace_set_acceleration_structure(kinc_g5_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_g5_raytrace_set_pipeline(kinc_g5_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_g5_raytrace_set_target(kinc_g5_render_target_t *_output) {
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
		fbufCreateInfo.renderPass = vk_ctx.windows[0].rendertarget_render_pass;
		fbufCreateInfo.attachmentCount = 1;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = _output->width;
		fbufCreateInfo.height = _output->height;
		fbufCreateInfo.layers = 1;
		vkCreateFramebuffer(vk_ctx.device, &fbufCreateInfo, NULL, &_output->impl.framebuffer);
	}
	output = _output;
}

void kinc_g5_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
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
	texenvimage_descriptor.imageView = texenv->impl.view;
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
	texsobolimage_descriptor.imageView = texsobol->impl.view;
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
	texscrambleimage_descriptor.imageView = texscramble->impl.view;
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
	texrankimage_descriptor.imageView = texrank->impl.view;
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
	                   output->width, output->height, 1);

	vkCmdBeginRenderPass(command_list->impl._buffer, &currentRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

#endif
