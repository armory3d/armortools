#pragma once
#include "webgpu.h"

typedef struct {
	WGPUBuffer buf;
	void      *mem;
	int        start;
} gpu_buffer_impl_t;

typedef struct {
	WGPUTexture     texture;
	WGPUTextureView view;
} gpu_texture_impl_t;

typedef struct {
	WGPURenderPipeline pipeline;
	WGPURenderPipeline pipeline_depth;
	WGPUPipelineLayout pipeline_layout;
	WGPUPipelineLayout pipeline_layout_depth;
} gpu_pipeline_impl_t;

typedef struct {
	void  *source;
	size_t length;
} gpu_shader_impl_t;

typedef struct {
	int empty;
} gpu_raytrace_pipeline_impl_t;

typedef struct {
	int empty;
} gpu_raytrace_acceleration_structure_impl_t;
