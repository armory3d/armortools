#pragma once

#include <kinc/graphics5/vertexstructure.h>
#include "minivulkan.h"

struct Vertices {
	VkBuffer buf;
	VkDeviceMemory mem;
};

typedef struct {
	float *data;
	int myCount;
	int myStride;
	unsigned bufferId;
	kinc_g5_vertex_structure_t structure;
	VkMemoryAllocateInfo mem_alloc;
	struct Vertices vertices;
} VertexBuffer5Impl;
