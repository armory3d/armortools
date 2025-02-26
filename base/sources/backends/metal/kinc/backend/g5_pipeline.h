#pragma once

struct kinc_g5_shader;

typedef struct {
	struct kinc_g5_shader *vertex_shader;
	struct kinc_g5_shader *fragment_shader;
	void *_pipeline;
	void *_pipelineDepth;
	void *_reflection;
	void *_depthStencil;
	void *_depthStencilNone;
} PipelineState5Impl;

typedef struct {
	int a;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
	int computeOffset;
} ConstantLocation5Impl;

typedef struct kinc_g5_sampler_impl {
	void *sampler;
} kinc_g5_sampler_impl_t;

typedef struct {
	char name[1024];
	void *mtlFunction;
} Shader5Impl;
