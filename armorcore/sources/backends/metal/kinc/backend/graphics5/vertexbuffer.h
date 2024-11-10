#pragma once

#include <kinc/graphics5/vertexstructure.h>

typedef struct {
	// void unset();
	int myCount;
	int myStride;
	void *mtlBuffer;
	bool gpuMemory;
	int lastStart;
	int lastCount;
	// static Graphics5::VertexBuffer* current;
} VertexBuffer5Impl;
