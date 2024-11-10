#pragma once

#include <webgpu/webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	WGPUCommandEncoder encoder;
	WGPURenderPassEncoder pass;
	int indexCount;
} CommandList5Impl;

#ifdef __cplusplus
}
#endif
