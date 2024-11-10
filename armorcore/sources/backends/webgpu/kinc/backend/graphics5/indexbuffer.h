#pragma once

#include <webgpu/webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	WGPUBuffer buffer;
	int count;
	int format;
} IndexBuffer5Impl;

#ifdef __cplusplus
}
#endif
