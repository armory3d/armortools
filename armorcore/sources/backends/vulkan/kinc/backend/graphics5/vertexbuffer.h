#pragma once

#include <kinc/graphics5/vertexstructure.h>

#include "MiniVulkan.h"

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
	int instanceDataStepRate;
	struct Vertices vertices;
} VertexBuffer5Impl;
