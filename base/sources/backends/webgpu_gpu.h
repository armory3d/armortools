#pragma once
#include "webgpu.h"

typedef struct {
	WGPUBuffer buf;
	WGPUBuffer temp_buf;
} gpu_buffer_impl_t;

typedef struct {
	WGPUTexture     texture;
	WGPUTextureView view;
} gpu_texture_impl_t;

typedef struct {
	WGPURenderPipeline pipeline;
	WGPUPipelineLayout pipeline_layout;
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
