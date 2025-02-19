#pragma once

#include <stdint.h>

struct kinc_g5_shader;

struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;

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
