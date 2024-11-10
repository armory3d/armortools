#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_shader;

struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;

typedef struct {
	struct ID3D12PipelineState *pso;
#ifdef KINC_DXC
	// struct ID3D12RootSignature *rootSignature;
	int vertexConstantsSize;
	int fragmentConstantsSize;
#endif
	int textures;
	// ID3D11InputLayout* inputLayout;
	// ID3D11Buffer* fragmentConstantBuffer;
	// ID3D11Buffer* vertexConstantBuffer;
	// ID3D11Buffer* geometryConstantBuffer;
	// ID3D11Buffer* tessEvalConstantBuffer;
	// ID3D11Buffer* tessControlConstantBuffer;

	// static void setConstants(ID3D12GraphicsCommandList *commandList, Graphics5::PipelineState *pipeline);
} PipelineState5Impl;

typedef struct {
	struct ID3D12PipelineState *pso;
#ifdef KINC_DXC
	struct ID3D12RootSignature *rootSignature;
	int vertexConstantsSize;
	int fragmentConstantsSize;
#endif
	int textures;
	// ID3D11InputLayout* inputLayout;
	// ID3D11Buffer* fragmentConstantBuffer;
	// ID3D11Buffer* vertexConstantBuffer;
	// ID3D11Buffer* geometryConstantBuffer;
	// ID3D11Buffer* tessEvalConstantBuffer;
	// ID3D11Buffer* tessControlConstantBuffer;

	// static void setConstants(ID3D12GraphicsCommandList *commandList, Graphics5::PipelineState *pipeline);
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

#ifdef __cplusplus
}
#endif
