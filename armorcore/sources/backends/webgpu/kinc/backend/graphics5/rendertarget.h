#pragma once

#include <webgpu/webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	WGPUTexture texture;
} RenderTarget5Impl;

#ifdef __cplusplus
}
#endif
