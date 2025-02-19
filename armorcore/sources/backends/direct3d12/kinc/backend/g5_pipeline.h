#pragma once

#include <stdint.h>
#include <stdbool.h>

struct kinc_g5_shader;

struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;

typedef struct {
	struct ID3D12PipelineState *pso;
	int textures;
} PipelineState5Impl;

typedef struct {
	struct ID3D12PipelineState *pso;
	int textures;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	uint32_t vertexSize;
	int fragmentOffset;
	uint32_t fragmentSize;
	int computeOffset;
	uint32_t computeSize;
	int geometryOffset;
	uint32_t geometrySize;
	int tessEvalOffset;
	uint32_t tessEvalSize;
	int tessControlOffset;
	uint32_t tessControlSize;
} ConstantLocation5Impl;

typedef struct {
	int nothing;
} AttributeLocation5Impl;

struct kinc_g5_pipeline;
struct kinc_g5_command_list;

void kinc_g5_internal_setConstants(struct kinc_g5_command_list *commandList, struct kinc_g5_pipeline *pipeline);

typedef struct kinc_g5_sampler_impl {
	struct ID3D12DescriptorHeap *sampler_heap;
} kinc_g5_sampler_impl_t;

typedef struct {
	char name[64];
	uint32_t offset;
	uint32_t size;
} ShaderConstant;

typedef struct {
	char name[64];
	int attribute;
} ShaderAttribute;

typedef struct {
	char name[64];
	int texture;
} ShaderTexture;

typedef struct {
	ShaderConstant constants[32];
	int constantsSize;
	ShaderAttribute attributes[32];
	ShaderTexture textures[32];
	int texturesCount;
	void *shader;
	uint8_t *data;
	int length;
} Shader5Impl;

typedef struct {
	uint32_t hash;
	uint32_t index;
} kinc_internal_hash_index_t;

uint32_t kinc_internal_hash_name(unsigned char *str);
