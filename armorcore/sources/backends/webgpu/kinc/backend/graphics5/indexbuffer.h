#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPUBuffer buffer;
	int count;
} IndexBuffer5Impl;
