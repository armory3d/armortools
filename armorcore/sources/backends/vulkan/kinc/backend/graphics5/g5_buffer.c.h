#include "vulkan.h"
#include "g5_buffer.h"
#include <kinc/g5_buffer.h>

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int vertexCount, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.myCount = vertexCount;
	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g5_vertex_element_t element = structure->elements[i];
		buffer->impl.myStride += kinc_g5_vertex_data_size(element.data);
	}
	buffer->impl.structure = *structure;

	VkBufferCreateInfo buf_info = {0};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = vertexCount * buffer->impl.myStride;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	if (kinc_g5_supports_raytracing()) {
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
	if (kinc_g5_supports_raytracing()) {
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

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

bool kinc_g5_transpose_mat = true;

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

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

static void unset(kinc_g5_index_buffer_t *buffer) {
}

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int indexCount, bool gpuMemory) {
	buffer->impl.count = indexCount;

	VkBufferCreateInfo buf_info = {0};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = indexCount * sizeof(uint32_t);
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if (kinc_g5_supports_raytracing()) {
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
	if (kinc_g5_supports_raytracing()) {
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
