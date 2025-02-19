#pragma once

#include <kinc/g5.h>
#include "minivulkan.h"

typedef struct {
	float *data;
	int myCount;
	int myStride;
	unsigned bufferId;
	kinc_g5_vertex_structure_t structure;
	VkMemoryAllocateInfo mem_alloc;

	VkBuffer buf;
	VkDeviceMemory mem;

} VertexBuffer5Impl;

typedef struct {
	VkBuffer buf;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryAllocateInfo mem_alloc;
	VkDeviceMemory mem;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

typedef struct {
	int count;
	VkBuffer buf;
	VkDeviceMemory mem;
	VkMemoryAllocateInfo mem_alloc;
} IndexBuffer5Impl;
