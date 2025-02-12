#include "pipeline.h"

#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>
#include <kinc/log.h>
#include <kinc/backend/system_microsoft.h>

void kinc_g5_internal_setConstants(kinc_g5_command_list_t *commandList, kinc_g5_pipeline_t *pipeline) {
	commandList->impl._commandList->lpVtbl->SetGraphicsRootSignature(commandList->impl._commandList, globalRootSignature);

	if (pipeline->impl.textures > 0) {
		kinc_g5_internal_set_textures(commandList);
	}
}

void kinc_g5_internal_set_compute_constants(kinc_g5_command_list_t *commandList) {

	commandList->impl._commandList->lpVtbl->SetComputeRootSignature(commandList->impl._commandList, globalComputeRootSignature);

	//if (pipeline->impl.textures > 0) {
		kinc_g5_internal_set_textures(commandList);
	//}
}

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipe) {
	kinc_g5_internal_pipeline_init(pipe);
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipe) {
	if (pipe->impl.pso != NULL) {
		pipe->impl.pso->lpVtbl->Release(pipe->impl.pso);
		pipe->impl.pso = NULL;
	}
}

#define MAX_SHADER_THING 32

static ShaderConstant findConstant(kinc_g5_shader_t *shader, const char *name) {
	if (shader != NULL) {
		for (int i = 0; i < MAX_SHADER_THING; ++i) {
			if (strcmp(shader->impl.constants[i].name, name) == 0) {
				return shader->impl.constants[i];
			}
		}
	}

	ShaderConstant constant;
	constant.name[0] = 0;
	constant.offset = -1;
	constant.size = 0;
	return constant;
}

static ShaderTexture findTexture(kinc_g5_shader_t *shader, const char *name) {
	for (int i = 0; i < MAX_SHADER_THING; ++i) {
		if (strcmp(shader->impl.textures[i].name, name) == 0) {
			return shader->impl.textures[i];
		}
	}

	ShaderTexture texture;
	texture.name[0] = 0;
	texture.texture = -1;
	return texture;
}

static ShaderAttribute findAttribute(kinc_g5_shader_t *shader, const char *name) {
	for (int i = 0; i < MAX_SHADER_THING; ++i) {
		if (strcmp(shader->impl.attributes[i].name, name) == 0) {
			return shader->impl.attributes[i];
		}
	}

	ShaderAttribute attribute;
	attribute.name[0] = 0;
	attribute.attribute = -1;
	return attribute;
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(struct kinc_g5_pipeline *pipe, const char *name) {
	kinc_g5_constant_location_t location;

	{
		ShaderConstant constant = findConstant(pipe->vertexShader, name);
		location.impl.vertexOffset = constant.offset;
		location.impl.vertexSize = constant.size;
	}

	{
		ShaderConstant constant = findConstant(pipe->fragmentShader, name);
		location.impl.fragmentOffset = constant.offset;
		location.impl.fragmentSize = constant.size;
	}

	location.impl.computeOffset = 0;
	location.impl.computeSize = 0;

	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t unit;
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	ShaderTexture vertexTexture = findTexture(pipe->vertexShader, name);
	if (vertexTexture.texture != -1) {
		unit.stages[KINC_G5_SHADER_TYPE_VERTEX] = vertexTexture.texture;
	}
	else {
		ShaderTexture fragmentTexture = findTexture(pipe->fragmentShader, name);
		unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] = fragmentTexture.texture;
	}
	return unit;
}

static D3D12_BLEND convert_blend_factor(kinc_g5_blending_factor_t factor) {
	switch (factor) {
	case KINC_G5_BLEND_ONE:
		return D3D12_BLEND_ONE;
	case KINC_G5_BLEND_ZERO:
		return D3D12_BLEND_ZERO;
	case KINC_G5_BLEND_SOURCE_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case KINC_G5_BLEND_DEST_ALPHA:
		return D3D12_BLEND_DEST_ALPHA;
	case KINC_G5_BLEND_INV_SOURCE_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case KINC_G5_BLEND_INV_DEST_ALPHA:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case KINC_G5_BLEND_SOURCE_COLOR:
		return D3D12_BLEND_SRC_COLOR;
	case KINC_G5_BLEND_DEST_COLOR:
		return D3D12_BLEND_DEST_COLOR;
	case KINC_G5_BLEND_INV_SOURCE_COLOR:
		return D3D12_BLEND_INV_SRC_COLOR;
	case KINC_G5_BLEND_INV_DEST_COLOR:
		return D3D12_BLEND_INV_DEST_COLOR;
	case KINC_G5_BLEND_CONSTANT:
		return D3D12_BLEND_BLEND_FACTOR;
	case KINC_G5_BLEND_INV_CONSTANT:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	default:
		assert(false);
		return D3D12_BLEND_ONE;
	}
}

static D3D12_BLEND_OP convert_blend_operation(kinc_g5_blending_operation_t op) {
	switch (op) {
	case KINC_G5_BLENDOP_ADD:
		return D3D12_BLEND_OP_ADD;
	case KINC_G5_BLENDOP_SUBTRACT:
		return D3D12_BLEND_OP_SUBTRACT;
	case KINC_G5_BLENDOP_REVERSE_SUBTRACT:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case KINC_G5_BLENDOP_MIN:
		return D3D12_BLEND_OP_MIN;
	case KINC_G5_BLENDOP_MAX:
		return D3D12_BLEND_OP_MAX;
	default:
		assert(false);
		return D3D12_BLEND_OP_ADD;
	}
}

static D3D12_CULL_MODE convert_cull_mode(kinc_g5_cull_mode_t cullMode) {
	switch (cullMode) {
	case KINC_G5_CULL_MODE_CLOCKWISE:
		return D3D12_CULL_MODE_FRONT;
	case KINC_G5_CULL_MODE_COUNTERCLOCKWISE:
		return D3D12_CULL_MODE_BACK;
	case KINC_G5_CULL_MODE_NEVER:
	default:
		return D3D12_CULL_MODE_NONE;
	}
}

static D3D12_COMPARISON_FUNC convert_compare_mode(kinc_g5_compare_mode_t compare) {
	switch (compare) {
	default:
	case KINC_G5_COMPARE_MODE_ALWAYS:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	case KINC_G5_COMPARE_MODE_NEVER:
		return D3D12_COMPARISON_FUNC_NEVER;
	case KINC_G5_COMPARE_MODE_EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case KINC_G5_COMPARE_MODE_NOT_EQUAL:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case KINC_G5_COMPARE_MODE_LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	case KINC_G5_COMPARE_MODE_LESS_EQUAL:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case KINC_G5_COMPARE_MODE_GREATER:
		return D3D12_COMPARISON_FUNC_GREATER;
	case KINC_G5_COMPARE_MODE_GREATER_EQUAL:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	}
}

static DXGI_FORMAT convert_format(kinc_g5_render_target_format_t format) {
	switch (format) {
	case KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT:
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static void set_blend_state(D3D12_BLEND_DESC *blend_desc, kinc_g5_pipeline_t *pipe, int target) {
	blend_desc->RenderTarget[target].BlendEnable = pipe->blend_source != KINC_G5_BLEND_ONE || pipe->blend_destination != KINC_G5_BLEND_ZERO ||
	                                               pipe->alpha_blend_source != KINC_G5_BLEND_ONE || pipe->alpha_blend_destination != KINC_G5_BLEND_ZERO;
	blend_desc->RenderTarget[target].SrcBlend = convert_blend_factor(pipe->blend_source);
	blend_desc->RenderTarget[target].DestBlend = convert_blend_factor(pipe->blend_destination);
	blend_desc->RenderTarget[target].BlendOp = convert_blend_operation(pipe->blend_operation);
	blend_desc->RenderTarget[target].SrcBlendAlpha = convert_blend_factor(pipe->alpha_blend_source);
	blend_desc->RenderTarget[target].DestBlendAlpha = convert_blend_factor(pipe->alpha_blend_destination);
	blend_desc->RenderTarget[target].BlendOpAlpha = convert_blend_operation(pipe->alpha_blend_operation);
	blend_desc->RenderTarget[target].RenderTargetWriteMask =
	    (((pipe->colorWriteMaskRed[target] ? D3D12_COLOR_WRITE_ENABLE_RED : 0) | (pipe->colorWriteMaskGreen[target] ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0)) |
	     (pipe->colorWriteMaskBlue[target] ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0)) |
	    (pipe->colorWriteMaskAlpha[target] ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
	int vertexAttributeCount = pipe->inputLayout->size;

	D3D12_INPUT_ELEMENT_DESC *vertexDesc = (D3D12_INPUT_ELEMENT_DESC *)alloca(sizeof(D3D12_INPUT_ELEMENT_DESC) * vertexAttributeCount);
	ZeroMemory(vertexDesc, sizeof(D3D12_INPUT_ELEMENT_DESC) * vertexAttributeCount);

	for (int i = 0; i < pipe->inputLayout->size; ++i) {
		vertexDesc[i].SemanticName = "TEXCOORD";
		vertexDesc[i].SemanticIndex = findAttribute(pipe->vertexShader, pipe->inputLayout->elements[i].name).attribute;
		vertexDesc[i].InputSlot = 0;
		vertexDesc[i].AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		vertexDesc[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertexDesc[i].InstanceDataStepRate = 0;

		switch (pipe->inputLayout->elements[i].data) {
		case KINC_G4_VERTEX_DATA_F32_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_F32_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_F32_3X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_F32_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_I8_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R8_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U8_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R8_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R8_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R8_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I8_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U8_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I8_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U8_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I16_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R16_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U16_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R16_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R16_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R16_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I16_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U16_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I16_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U16_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_UNORM;
			break;
		case KINC_G4_VERTEX_DATA_I32_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R32_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I32_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I32_3X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_3X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_UINT;
			break;
		case KINC_G4_VERTEX_DATA_I32_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		case KINC_G4_VERTEX_DATA_U32_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_UINT;
			break;
		default:
			break;
		}
	}

	HRESULT hr = S_OK;
	pipe->impl.textures = pipe->fragmentShader->impl.texturesCount;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {0};
	psoDesc.VS.BytecodeLength = pipe->vertexShader->impl.length;
	psoDesc.VS.pShaderBytecode = pipe->vertexShader->impl.data;
	psoDesc.PS.BytecodeLength = pipe->fragmentShader->impl.length;
	psoDesc.PS.pShaderBytecode = pipe->fragmentShader->impl.data;
	psoDesc.pRootSignature = globalRootSignature;
	psoDesc.NumRenderTargets = pipe->colorAttachmentCount;
	for (int i = 0; i < pipe->colorAttachmentCount; ++i) {
		psoDesc.RTVFormats[i] = convert_format(pipe->colorAttachment[i]);
	}
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	psoDesc.InputLayout.NumElements = vertexAttributeCount;
	psoDesc.InputLayout.pInputElementDescs = vertexDesc;

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = convert_cull_mode(pipe->cullMode);
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
	    FALSE,
	    FALSE,
	    D3D12_BLEND_ONE,
	    D3D12_BLEND_ZERO,
	    D3D12_BLEND_OP_ADD,
	    D3D12_BLEND_ONE,
	    D3D12_BLEND_ZERO,
	    D3D12_BLEND_OP_ADD,
	    D3D12_LOGIC_OP_NOOP,
	    D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
		psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
	}

	bool independentBlend = false;
	for (int i = 1; i < 8; ++i) {
		if (pipe->colorWriteMaskRed[0] != pipe->colorWriteMaskRed[i] || pipe->colorWriteMaskGreen[0] != pipe->colorWriteMaskGreen[i] ||
		    pipe->colorWriteMaskBlue[0] != pipe->colorWriteMaskBlue[i] || pipe->colorWriteMaskAlpha[0] != pipe->colorWriteMaskAlpha[i]) {
			independentBlend = true;
			break;
		}
	}

	set_blend_state(&psoDesc.BlendState, pipe, 0);
	if (independentBlend) {
		psoDesc.BlendState.IndependentBlendEnable = true;
		for (int i = 1; i < 8; ++i) {
			set_blend_state(&psoDesc.BlendState, pipe, i);
		}
	}

	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS};
	psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
	psoDesc.DepthStencilState.BackFace = defaultStencilOp;

	psoDesc.DepthStencilState.DepthEnable = pipe->depthMode != KINC_G5_COMPARE_MODE_ALWAYS;
	psoDesc.DepthStencilState.DepthWriteMask = pipe->depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = convert_compare_mode(pipe->depthMode);
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	hr = device->lpVtbl->CreateGraphicsPipelineState(device , &psoDesc, &IID_ID3D12PipelineState, &pipe->impl.pso);
	if (hr != S_OK) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not create pipeline.");
	}
}
