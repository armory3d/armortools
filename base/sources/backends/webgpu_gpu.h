#pragma once

#include <iron_gpu.h>
#include <iron_math.h>
#include <webgpu/webgpu.h>

typedef struct {
	WGPUBuffer buffer;
	int count;
	int stride;
} gpu_buffer_impl_t;

typedef struct {
	WGPUTexture texture;
} gpu_texture_impl_t;

typedef struct {
	WGPURenderPipeline pipeline;
} gpu_pipeline_impl_t;

typedef struct {
	int vertexOffset;
} gpu_constant_location_impl_t;

typedef struct iron_gpu_sampler_impl {
	int a;
} gpu_sampler_impl_t;

struct WGPUShaderModuleImpl;

typedef struct {
	WGPUShaderModule module;
} gpu_shader_impl_t;

typedef struct {
	WGPUCommandEncoder encoder;
	WGPURenderPassEncoder pass;
	int indexCount;
} gpu_command_list_impl_t;
