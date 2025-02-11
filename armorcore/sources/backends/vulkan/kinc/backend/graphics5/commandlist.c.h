#include "kinc/graphics5/sampler.h"
#include "vulkan.h"

#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/compute.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/log.h>
#include <kinc/window.h>
#include <vulkan/vulkan_core.h>

extern kinc_g5_texture_t *vulkanTextures[16];
extern kinc_g5_render_target_t *vulkanRenderTargets[16];
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
static kinc_g5_pipeline_t *currentPipeline = NULL;
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
		vulkanTextures[i] = NULL;
		vulkanRenderTargets[i] = NULL;
	}
}

static int formatSize(VkFormat format) {
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
		viewport.y = (float)kinc_window_height(vk_ctx.current_window);
		viewport.width = (float)kinc_window_width(vk_ctx.current_window);
		viewport.height = -(float)kinc_window_height(vk_ctx.current_window);
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		scissor.extent.width = kinc_window_width(vk_ctx.current_window);
		scissor.extent.height = kinc_window_height(vk_ctx.current_window);
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
	if (vk_ctx.windows[vk_ctx.current_window].depth_bits > 0) {
		clear_values[1].depthStencil.depth = 1.0;
		clear_values[1].depthStencil.stencil = 0;
	}

	VkRenderPassBeginInfo rp_begin = {0};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = vk_ctx.windows[vk_ctx.current_window].framebuffer_render_pass;
	rp_begin.framebuffer = vk_ctx.windows[vk_ctx.current_window].framebuffers[vk_ctx.windows[vk_ctx.current_window].current_image];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = vk_ctx.windows[vk_ctx.current_window].width;
	rp_begin.renderArea.extent.height = vk_ctx.windows[vk_ctx.current_window].height;
	rp_begin.clearValueCount = vk_ctx.windows[vk_ctx.current_window].depth_bits > 0 ? 2 : 1;
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

	prePresentBarrier.image = vk_ctx.windows[vk_ctx.current_window].images[vk_ctx.windows[vk_ctx.current_window].current_image];
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

	prePresentBarrier.image = vk_ctx.windows[vk_ctx.current_window].images[vk_ctx.windows[vk_ctx.current_window].current_image];
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
		scissor.extent.width = kinc_window_width(vk_ctx.current_window);
		scissor.extent.height = kinc_window_height(vk_ctx.current_window);
	}
	else {
		scissor.extent.width = currentRenderTargets[0]->width;
		scissor.extent.height = currentRenderTargets[0]->height;
	}
	vkCmdSetScissor(list->impl._buffer, 0, 1, &scissor);
}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	currentPipeline = pipeline;
	lastVertexConstantBufferOffset = 0;
	lastFragmentConstantBufferOffset = 0;

	if (onBackBuffer) {
		currentVulkanPipeline = currentPipeline->impl.framebuffer_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->impl.framebuffer_pipeline);
	}
	else {
		currentVulkanPipeline = currentPipeline->impl.rendertarget_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->impl.rendertarget_pipeline);
	}
}

void kinc_g5_command_list_set_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *vertexBuffer) {
	VkBuffer buffers[1];
	VkDeviceSize offsets[1];
	buffers[0] = vertexBuffer->impl.vertices.buf;
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
	viewport.y = (float)kinc_window_height(vk_ctx.current_window);
	viewport.width = (float)kinc_window_width(vk_ctx.current_window);
	viewport.height = -(float)kinc_window_height(vk_ctx.current_window);
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(list->impl._buffer, 0, 1, &viewport);
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = kinc_window_width(vk_ctx.current_window);
	scissor.extent.height = kinc_window_height(vk_ctx.current_window);
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
	rp_begin.renderPass = vk_ctx.windows[vk_ctx.current_window].framebuffer_render_pass;
	rp_begin.framebuffer = vk_ctx.windows[vk_ctx.current_window].framebuffers[vk_ctx.windows[vk_ctx.current_window].current_image];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = kinc_window_width(vk_ctx.current_window);
	rp_begin.renderArea.extent.height = kinc_window_height(vk_ctx.current_window);
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;
	vkCmdBeginRenderPass(list->impl._buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	currentRenderPassBeginInfo = rp_begin;
	in_render_pass = true;

	if (currentPipeline != NULL) {
		currentVulkanPipeline = currentPipeline->impl.framebuffer_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->impl.framebuffer_pipeline);
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
			rp_begin.renderPass = vk_ctx.windows[vk_ctx.current_window].rendertarget_render_pass_with_depth;
		}
		else {
			rp_begin.renderPass = vk_ctx.windows[vk_ctx.current_window].rendertarget_render_pass;
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

	if (currentPipeline != NULL) {
		currentVulkanPipeline = currentPipeline->impl.rendertarget_pipeline;
		vkCmdBindPipeline(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->impl.rendertarget_pipeline);
	}
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {
	VkFormat format = render_target->impl.format;
	int formatByteSize = formatSize(format);

	// Create readback buffer
	if (!render_target->impl.readbackBufferCreated) {
		VkBufferCreateInfo buf_info = {0};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.pNext = NULL;
		buf_info.size = render_target->width * render_target->height * formatByteSize;
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
	memcpy(data, p, render_target->texWidth * render_target->texHeight * formatByteSize);
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
	vkCmdBindDescriptorSets(list->impl._buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->impl.pipeline_layout, 0, 1, &descriptor_set, 2, offsets);
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
	vulkanTextures[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = texture;
	vulkanRenderTargets[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = NULL;
}

void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		vulkanSamplers[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = sampler->impl.sampler;
	}
}

void kinc_g5_command_list_set_texture_from_render_target(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *target) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		target->impl.stage = unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT];
		vulkanRenderTargets[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = target;
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		target->impl.stage = unit.stages[KINC_G5_SHADER_TYPE_VERTEX];
		vulkanRenderTargets[unit.stages[KINC_G5_SHADER_TYPE_VERTEX]] = target;
	}
	vulkanTextures[target->impl.stage] = NULL;
}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *target) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		target->impl.stage_depth = unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT];
		vulkanRenderTargets[unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]] = target;
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		target->impl.stage_depth = unit.stages[KINC_G5_SHADER_TYPE_VERTEX];
		vulkanRenderTargets[unit.stages[KINC_G5_SHADER_TYPE_VERTEX]] = target;
	}
	vulkanTextures[target->impl.stage_depth] = NULL;
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
