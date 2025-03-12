#pragma once

#include <iron_gpu.h>
#include <iron_math.h>

struct iron_gpu_index_buffer;

typedef struct {
	struct iron_gpu_index_buffer *current_index_buffer;
} CommandList5Impl;

struct iron_gpu_shader;

typedef struct {
	struct iron_gpu_shader *vertex_shader;
	struct iron_gpu_shader *fragment_shader;
	void *_pipeline;
	void *_pipelineDepth;
	void *_reflection;
	void *_depthStencil;
	void *_depthStencilNone;
} PipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
} ConstantLocation5Impl;

typedef struct iron_gpu_sampler_impl {
	void *sampler;
} iron_gpu_sampler_impl_t;

typedef struct {
	char name[1024];
	void *mtlFunction;
} Shader5Impl;

typedef struct {
	void *_raytracingPipeline;
} iron_gpu_raytrace_pipeline_impl_t;

typedef struct {
	void *_accelerationStructure;
} iron_gpu_raytrace_acceleration_structure_impl_t;

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
