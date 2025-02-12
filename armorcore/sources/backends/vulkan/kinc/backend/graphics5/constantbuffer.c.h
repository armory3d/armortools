#include "vulkan.h"
#include <kinc/graphics5/constantbuffer.h>

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

bool kinc_g5_transposeMat = true;

static void createUniformBuffer(VkBuffer *buf, VkMemoryAllocateInfo *mem_alloc, VkDeviceMemory *mem, VkDescriptorBufferInfo *buffer_info, int size) {
	VkBufferCreateInfo buf_info;
	memset(&buf_info, 0, sizeof(buf_info));
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	if (kinc_g5_supports_raytracing()) {
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
