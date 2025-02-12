#include "vulkan.h"
#include "shader.h"
#include "vertexbuffer.h"
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/vertexbuffer.h>

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int vertexCount, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.myCount = vertexCount;
	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g5_vertex_element_t element = structure->elements[i];
		buffer->impl.myStride += kinc_g4_vertex_data_size(element.data);
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

	memset(&buffer->impl.vertices, 0, sizeof(buffer->impl.vertices));

	err = vkCreateBuffer(vk_ctx.device, &buf_info, NULL, &buffer->impl.vertices.buf);
	assert(!err);

	vkGetBufferMemoryRequirements(vk_ctx.device, buffer->impl.vertices.buf, &mem_reqs);
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

	err = vkAllocateMemory(vk_ctx.device, &buffer->impl.mem_alloc, NULL, &buffer->impl.vertices.mem);
	assert(!err);

	err = vkBindBufferMemory(vk_ctx.device, buffer->impl.vertices.buf, buffer->impl.vertices.mem, 0);
	assert(!err);
}

static void unset_vertex_buffer(kinc_g5_vertex_buffer_t *buffer) {

}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {
	unset_vertex_buffer(buffer);
	vkFreeMemory(vk_ctx.device, buffer->impl.vertices.mem, NULL);
	vkDestroyBuffer(vk_ctx.device, buffer->impl.vertices.buf, NULL);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_lock(buffer, 0, buffer->impl.myCount);
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count) {
	VkResult err =
	    vkMapMemory(vk_ctx.device, buffer->impl.vertices.mem, start * buffer->impl.myStride, count * buffer->impl.myStride, 0, (void **)&buffer->impl.data);
	assert(!err);
	return buffer->impl.data;
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.vertices.mem);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count) {
	vkUnmapMemory(vk_ctx.device, buffer->impl.vertices.mem);
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
