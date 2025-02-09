#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPUBuffer buffer;
	int count;
	int format;
} IndexBuffer5Impl;
