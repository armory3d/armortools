#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPUCommandEncoder encoder;
	WGPURenderPassEncoder pass;
	int indexCount;
} CommandList5Impl;
