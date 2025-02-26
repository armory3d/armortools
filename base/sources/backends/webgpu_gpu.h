#pragma once

#include <iron_gpu.h>
#include <iron_math.h>

#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPUBuffer buffer;
	int count;
	int stride;
} VertexBuffer5Impl;

typedef struct {
	WGPUBuffer buffer;
} ConstantBuffer5Impl;

typedef struct {
	WGPUBuffer buffer;
	int count;
} IndexBuffer5Impl;


#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPUTexture texture;
} Texture5Impl;

typedef struct {
	WGPUTexture texture;
} RenderTarget5Impl;



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



#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPUCommandEncoder encoder;
	WGPURenderPassEncoder pass;
	int indexCount;
} CommandList5Impl;



#pragma once

typedef struct kinc_g5_compute_shader_impl {
	int nothing;
} kinc_g5_compute_shader_impl;

