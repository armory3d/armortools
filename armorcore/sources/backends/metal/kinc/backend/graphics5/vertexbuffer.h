#pragma once

#include <kinc/graphics5/vertexstructure.h>

typedef struct {
	int myCount;
	int myStride;
	void *mtlBuffer;
	bool gpuMemory;
	int lastStart;
	int lastCount;
} VertexBuffer5Impl;
