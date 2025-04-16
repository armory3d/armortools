#pragma once

#include <iron_gpu.h>
#include <iron_math.h>

struct iron_gpu_buffer;

typedef struct {
	struct iron_gpu_buffer *current_index_buffer;
} gpu_command_list_impl_t;

struct iron_gpu_shader;

typedef struct {
	struct iron_gpu_shader *vertex_shader;
	struct iron_gpu_shader *fragment_shader;
	void *_pipeline;
	void *_pipelineDepth;
	void *_reflection;
	void *_depthStencil;
	void *_depthStencilNone;
} gpu_pipeline_impl_t;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
} gpu_constant_location_impl_t;

typedef struct iron_gpu_sampler_impl {
	void *sampler;
} gpu_sampler_impl_t;

typedef struct {
	char name[1024];
	void *mtlFunction;
	char *source;
	int length;
} gpu_shader_impl_t;

typedef struct {
	void *_raytracingPipeline;
} gpu_raytrace_pipeline_impl_t;

typedef struct {
	void *_accelerationStructure;
} gpu_raytrace_acceleration_structure_impl_t;

typedef struct {
	void *_tex;
	void *data;
	bool has_mipmaps;

	void *_texReadback;
	void *_depthTex;
} gpu_texture_impl_t;

typedef struct {
	int myCount;
	int myStride;
	void *mtlBuffer;
	bool gpuMemory;
	int lastStart;
	int lastCount;

	void *metal_buffer;
	int count;
	bool gpu_memory;
	int last_start;
	int last_count;

	void *_buffer;
	int mySize;
} gpu_buffer_impl_t;
