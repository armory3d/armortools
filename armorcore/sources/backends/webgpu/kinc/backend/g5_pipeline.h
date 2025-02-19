#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPURenderPipeline pipeline;
} PipelineState5Impl;

typedef struct {
	WGPUComputePipeline pipeline;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
	int computeOffset;
} ConstantLocation5Impl;

typedef struct kinc_g5_sampler_impl {
	int a;
} kinc_g5_sampler_impl_t;

struct WGPUShaderModuleImpl;

typedef struct {
	WGPUShaderModule module;
} Shader5Impl;
