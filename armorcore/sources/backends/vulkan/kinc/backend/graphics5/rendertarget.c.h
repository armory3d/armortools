#include "vulkan.h"
#include "rendertarget.h"
#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/log.h>

extern uint32_t swapchainImageCount;
extern kinc_g5_texture_t *vulkanTextures[16];
extern kinc_g5_render_target_t *vulkanRenderTargets[16];

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
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

static void render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits, int framebuffer_index) {
	target->width = width;
	target->height = height;
	target->framebuffer_index = framebuffer_index;
	target->texWidth = width;
	target->texHeight = height;
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
			fbufCreateInfo.renderPass = vk_ctx.windows[vk_ctx.current_window].framebuffer_render_pass;
		}
		else if (depthBufferBits > 0) {
			fbufCreateInfo.renderPass = vk_ctx.windows[vk_ctx.current_window].rendertarget_render_pass_with_depth;
		}
		else {
			fbufCreateInfo.renderPass = vk_ctx.windows[vk_ctx.current_window].rendertarget_render_pass;
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

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, -1);
}

static int framebuffer_count = 0;

void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits) {
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
			fbufCreateInfo.renderPass = vk_ctx.windows[vk_ctx.current_window].rendertarget_render_pass_with_depth;
		}
		else {
			fbufCreateInfo.renderPass = vk_ctx.windows[vk_ctx.current_window].rendertarget_render_pass;
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
