#pragma once

#include <iron_gpu.h>
#include <iron_math.h>
#include <webgpu/webgpu.h>

typedef struct {
	WGPUBuffer buffer;
	int        stride;
} gpu_buffer_impl_t;

typedef struct {
	WGPUTexture texture;
} gpu_texture_impl_t;

typedef struct {
	WGPURenderPipeline pipeline;
} gpu_pipeline_impl_t;

struct WGPUShaderModuleImpl;

typedef struct {
	WGPUShaderModule module;
} gpu_shader_impl_t;
