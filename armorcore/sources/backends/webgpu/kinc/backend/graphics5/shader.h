#pragma once

#include <webgpu/webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

struct WGPUShaderModuleImpl;

typedef struct {
	WGPUShaderModule module;
} Shader5Impl;

#ifdef __cplusplus
}
#endif
