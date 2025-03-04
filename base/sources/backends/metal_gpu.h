#pragma once

#include <iron_gpu.h>
#include <iron_math.h>

#pragma once

struct kinc_g5_index_buffer;

typedef struct {
	struct kinc_g5_index_buffer *current_index_buffer;
} CommandList5Impl;

typedef struct kinc_g5_compute_shader_impl {
	char name[1024];
	void *_function;
	void *_pipeline;
	void *_reflection;
} kinc_g5_compute_shader_impl;

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

typedef struct {
	void *_raytracingPipeline;
} kinc_g5_raytrace_pipeline_impl_t;

typedef struct {
	void *_accelerationStructure;
} kinc_g5_raytrace_acceleration_structure_impl_t;

typedef struct {
	int index;
	bool vertex;
} TextureUnit5Impl;

typedef struct {
	void *_tex;
	void *data;
	bool has_mipmaps;

	void *_texReadback;
	void *_depthTex;
} Texture5Impl;

typedef struct {
	int myCount;
	int myStride;
	void *mtlBuffer;
	bool gpuMemory;
	int lastStart;
	int lastCount;
} VertexBuffer5Impl;

typedef struct {
	void *_buffer;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

typedef struct {
	void *metal_buffer;
	int count;
	bool gpu_memory;
	int last_start;
	int last_count;
} IndexBuffer5Impl;
