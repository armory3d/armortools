#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11Buffer;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11BlendState;

typedef struct {
	// PipelineStateImpl();
	//~PipelineStateImpl();
	struct ID3D11InputLayout *d3d11inputLayout;
	struct ID3D11Buffer *fragmentConstantBuffer;
	struct ID3D11Buffer *vertexConstantBuffer;
	struct ID3D11Buffer *geometryConstantBuffer;
	struct ID3D11Buffer *tessEvalConstantBuffer;
	struct ID3D11Buffer *tessControlConstantBuffer;
	struct ID3D11DepthStencilState *depthStencilState;
	struct ID3D11RasterizerState *rasterizerState;
	struct ID3D11RasterizerState *rasterizerStateScissor;
	struct ID3D11BlendState *blendState;
	// void set(Graphics4::PipelineState* pipeline, bool scissoring);
	// void setRasterizerState(bool scissoring);
	// static void setConstants();
} kinc_g4_pipeline_impl_t;

void kinc_internal_set_constants(void);

#ifdef __cplusplus
}
#endif
