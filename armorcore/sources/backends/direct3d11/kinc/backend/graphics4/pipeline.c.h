#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/libs/stb_sprintf.h>
#include <kinc/log.h>

kinc_g4_pipeline_t *currentPipeline = NULL;
float currentBlendFactor[4] = {0, 0, 0, 0};
bool needPipelineRebind = true;

static D3D11_CULL_MODE convert_cull_mode(kinc_g4_cull_mode_t cullMode) {
	switch (cullMode) {
	case KINC_G4_CULL_CLOCKWISE:
		return D3D11_CULL_BACK;
	case KINC_G4_CULL_COUNTER_CLOCKWISE:
		return D3D11_CULL_FRONT;
	case KINC_G4_CULL_NOTHING:
		return D3D11_CULL_NONE;
	default:
		assert(false);
		return D3D11_CULL_NONE;
	}
}

static D3D11_BLEND convert_blend_factor(kinc_g4_blending_factor_t factor) {
	switch (factor) {
	case KINC_G4_BLEND_ONE:
		return D3D11_BLEND_ONE;
	case KINC_G4_BLEND_ZERO:
		return D3D11_BLEND_ZERO;
	case KINC_G4_BLEND_SOURCE_ALPHA:
		return D3D11_BLEND_SRC_ALPHA;
	case KINC_G4_BLEND_DEST_ALPHA:
		return D3D11_BLEND_DEST_ALPHA;
	case KINC_G4_BLEND_INV_SOURCE_ALPHA:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case KINC_G4_BLEND_INV_DEST_ALPHA:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case KINC_G4_BLEND_SOURCE_COLOR:
		return D3D11_BLEND_SRC_COLOR;
	case KINC_G4_BLEND_DEST_COLOR:
		return D3D11_BLEND_DEST_COLOR;
	case KINC_G4_BLEND_INV_SOURCE_COLOR:
		return D3D11_BLEND_INV_SRC_COLOR;
	case KINC_G4_BLEND_INV_DEST_COLOR:
		return D3D11_BLEND_INV_DEST_COLOR;
	default:
		assert(false);
		return D3D11_BLEND_SRC_ALPHA;
	}
}

static D3D11_BLEND_OP convert_blend_operation(kinc_g4_blending_operation_t operation) {
	switch (operation) {
	case KINC_G4_BLENDOP_ADD:
		return D3D11_BLEND_OP_ADD;
	case KINC_G4_BLENDOP_SUBTRACT:
		return D3D11_BLEND_OP_SUBTRACT;
	case KINC_G4_BLENDOP_REVERSE_SUBTRACT:
		return D3D11_BLEND_OP_REV_SUBTRACT;
	case KINC_G4_BLENDOP_MIN:
		return D3D11_BLEND_OP_MIN;
	case KINC_G4_BLENDOP_MAX:
		return D3D11_BLEND_OP_MAX;
	default:
		assert(false);
		return D3D11_BLEND_OP_ADD;
	}
}

static D3D11_STENCIL_OP get_stencil_action(kinc_g4_stencil_action_t action) {
	switch (action) {
	default:
	case KINC_G4_STENCIL_KEEP:
		return D3D11_STENCIL_OP_KEEP;
	case KINC_G4_STENCIL_ZERO:
		return D3D11_STENCIL_OP_ZERO;
	case KINC_G4_STENCIL_REPLACE:
		return D3D11_STENCIL_OP_REPLACE;
	case KINC_G4_STENCIL_INCREMENT:
		return D3D11_STENCIL_OP_INCR;
	case KINC_G4_STENCIL_INCREMENT_WRAP:
		return D3D11_STENCIL_OP_INCR_SAT;
	case KINC_G4_STENCIL_DECREMENT:
		return D3D11_STENCIL_OP_DECR;
	case KINC_G4_STENCIL_DECREMENT_WRAP:
		return D3D11_STENCIL_OP_DECR_SAT;
	case KINC_G4_STENCIL_INVERT:
		return D3D11_STENCIL_OP_INVERT;
	}
}

void kinc_internal_set_constants(void) {
	if (currentPipeline->vertex_shader->impl.constantsSize > 0) {
		dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)currentPipeline->impl.vertexConstantBuffer, 0, NULL, vertexConstants, 0, 0);
		dx_ctx.context->lpVtbl->VSSetConstantBuffers(dx_ctx.context, 0, 1, &currentPipeline->impl.vertexConstantBuffer);
	}
	if (currentPipeline->fragment_shader->impl.constantsSize > 0) {
		dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)currentPipeline->impl.fragmentConstantBuffer, 0, NULL, fragmentConstants, 0,
		                                          0);
		dx_ctx.context->lpVtbl->PSSetConstantBuffers(dx_ctx.context, 0, 1, &currentPipeline->impl.fragmentConstantBuffer);
	}
	if (currentPipeline->geometry_shader != NULL && currentPipeline->geometry_shader->impl.constantsSize > 0) {
		dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)currentPipeline->impl.geometryConstantBuffer, 0, NULL, geometryConstants, 0,
		                                          0);
		dx_ctx.context->lpVtbl->GSSetConstantBuffers(dx_ctx.context, 0, 1, &currentPipeline->impl.geometryConstantBuffer);
	}
	if (currentPipeline->tessellation_control_shader != NULL && currentPipeline->tessellation_control_shader->impl.constantsSize > 0) {
		dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)currentPipeline->impl.tessControlConstantBuffer, 0, NULL,
		                                          tessControlConstants, 0, 0);
		dx_ctx.context->lpVtbl->HSSetConstantBuffers(dx_ctx.context, 0, 1, &currentPipeline->impl.tessControlConstantBuffer);
	}
	if (currentPipeline->tessellation_evaluation_shader != NULL && currentPipeline->tessellation_evaluation_shader->impl.constantsSize > 0) {
		dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)currentPipeline->impl.tessEvalConstantBuffer, 0, NULL, tessEvalConstants, 0,
		                                          0);
		dx_ctx.context->lpVtbl->DSSetConstantBuffers(dx_ctx.context, 0, 1, &currentPipeline->impl.tessEvalConstantBuffer);
	}
}

void kinc_g4_pipeline_init(struct kinc_g4_pipeline *state) {
	memset(state, 0, sizeof(struct kinc_g4_pipeline));
	kinc_g4_internal_pipeline_set_defaults(state);
	state->impl.d3d11inputLayout = NULL;
	state->impl.fragmentConstantBuffer = NULL;
	state->impl.vertexConstantBuffer = NULL;
	state->impl.geometryConstantBuffer = NULL;
	state->impl.tessEvalConstantBuffer = NULL;
	state->impl.tessControlConstantBuffer = NULL;
	state->impl.depthStencilState = NULL;
	state->impl.rasterizerState = NULL;
	state->impl.rasterizerStateScissor = NULL;
	state->impl.blendState = NULL;
}

void kinc_g4_pipeline_destroy(struct kinc_g4_pipeline *state) {
	if (state->impl.d3d11inputLayout != NULL) {
		state->impl.d3d11inputLayout->lpVtbl->Release(state->impl.d3d11inputLayout);
		state->impl.d3d11inputLayout = NULL;
	}
	if (state->impl.fragmentConstantBuffer != NULL) {
		state->impl.fragmentConstantBuffer->lpVtbl->Release(state->impl.fragmentConstantBuffer);
		state->impl.fragmentConstantBuffer = NULL;
	}
	if (state->impl.vertexConstantBuffer != NULL) {
		state->impl.vertexConstantBuffer->lpVtbl->Release(state->impl.vertexConstantBuffer);
		state->impl.vertexConstantBuffer = NULL;
	}
	if (state->impl.geometryConstantBuffer != NULL) {
		state->impl.geometryConstantBuffer->lpVtbl->Release(state->impl.geometryConstantBuffer);
		state->impl.geometryConstantBuffer = NULL;
	}
	if (state->impl.tessEvalConstantBuffer != NULL) {
		state->impl.tessEvalConstantBuffer->lpVtbl->Release(state->impl.tessEvalConstantBuffer);
		state->impl.tessEvalConstantBuffer = NULL;
	}
	if (state->impl.tessControlConstantBuffer != NULL) {
		state->impl.tessControlConstantBuffer->lpVtbl->Release(state->impl.tessControlConstantBuffer);
		state->impl.tessControlConstantBuffer = NULL;
	}
	if (state->impl.depthStencilState != NULL) {
		state->impl.depthStencilState->lpVtbl->Release(state->impl.depthStencilState);
		state->impl.depthStencilState = NULL;
	}
	if (state->impl.rasterizerState != NULL) {
		state->impl.rasterizerState->lpVtbl->Release(state->impl.rasterizerState);
		state->impl.rasterizerState = NULL;
	}
	if (state->impl.rasterizerStateScissor != NULL) {
		state->impl.rasterizerStateScissor->lpVtbl->Release(state->impl.rasterizerStateScissor);
		state->impl.rasterizerStateScissor = NULL;
	}
	if (state->impl.blendState != NULL) {
		state->impl.blendState->lpVtbl->Release(state->impl.blendState);
		state->impl.blendState = NULL;
	}
}

void kinc_internal_set_rasterizer_state(struct kinc_g4_pipeline *pipeline, bool scissoring) {
	if (scissoring && pipeline->impl.rasterizerStateScissor != NULL)
		dx_ctx.context->lpVtbl->RSSetState(dx_ctx.context, pipeline->impl.rasterizerStateScissor);
	else if (pipeline->impl.rasterizerState != NULL)
		dx_ctx.context->lpVtbl->RSSetState(dx_ctx.context, pipeline->impl.rasterizerState);
}

void kinc_internal_set_pipeline(struct kinc_g4_pipeline *pipeline, bool scissoring) {
	dx_ctx.context->lpVtbl->OMSetDepthStencilState(dx_ctx.context, pipeline->impl.depthStencilState, pipeline->stencil_reference_value);
	UINT sampleMask = 0xffffffff;
	dx_ctx.context->lpVtbl->OMSetBlendState(dx_ctx.context, pipeline->impl.blendState, currentBlendFactor, sampleMask);
	kinc_internal_set_rasterizer_state(pipeline, scissoring);

	dx_ctx.context->lpVtbl->VSSetShader(dx_ctx.context, (ID3D11VertexShader *)pipeline->vertex_shader->impl.shader, NULL, 0);
	dx_ctx.context->lpVtbl->PSSetShader(dx_ctx.context, (ID3D11PixelShader *)pipeline->fragment_shader->impl.shader, NULL, 0);

	dx_ctx.context->lpVtbl->GSSetShader(dx_ctx.context,
	                                    pipeline->geometry_shader != NULL ? (ID3D11GeometryShader *)pipeline->geometry_shader->impl.shader : NULL, NULL, 0);
	dx_ctx.context->lpVtbl->HSSetShader(
	    dx_ctx.context, pipeline->tessellation_control_shader != NULL ? (ID3D11HullShader *)pipeline->tessellation_control_shader->impl.shader : NULL, NULL, 0);
	dx_ctx.context->lpVtbl->DSSetShader(
	    dx_ctx.context, pipeline->tessellation_evaluation_shader != NULL ? (ID3D11DomainShader *)pipeline->tessellation_evaluation_shader->impl.shader : NULL,
	    NULL, 0);

	dx_ctx.context->lpVtbl->IASetInputLayout(dx_ctx.context, pipeline->impl.d3d11inputLayout);
}

void kinc_internal_pipeline_rebind() {
	if (currentPipeline != NULL && needPipelineRebind) {
		kinc_internal_set_pipeline(currentPipeline, kinc_internal_scissoring);
		needPipelineRebind = false;
	}
}

static kinc_internal_shader_constant_t *findConstant(kinc_internal_shader_constant_t *constants, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (constants[i].hash == hash) {
			return &constants[i];
		}
	}
	return NULL;
}

static kinc_internal_hash_index_t *findTextureUnit(kinc_internal_hash_index_t *units, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (units[i].hash == hash) {
			return &units[i];
		}
	}
	return NULL;
}

void kinc_g4_pipeline_get_constant_locations(kinc_g4_pipeline_t *state, kinc_g4_constant_location_t *vertex_locations,
                                             kinc_g4_constant_location_t *fragment_locations, int *vertex_sizes, int *fragment_sizes, int *max_vertex,
                                             int *max_fragment) {

	// *max_vertex = state->vertex_shader->impl.constantsSize;
	// *max_fragment = state->fragment_shader->impl.constantsSize;
	// if (vertex_locations != null && fragment_locations != null) {
	// 	for (int i = 0; i < state->vertex_shader->impl.constantsSize; i++) {
	// 		kinc_internal_shader_constant_t *constant = state->vertex_shader->impl.constants[i];
	// 		vertex_location[i].impl.vertexOffset = constant->offset;
	// 		vertex_location[i].impl.vertexSize = constant->size;
	// 		vertex_location[i].impl.vertexColumns = constant->columns;
	// 		vertex_location[i].impl.vertexRows = constant->rows;
	// 		vertex_sizes[i] = constant->size;
	// 	}

	// 	for (int i = 0; i < state->fragment_shader->impl.constantsSize; i++) {
	// 		kinc_internal_shader_constant_t *constant = state->fragment_shader->impl.constants[i];
	// 		fragment_location[i].impl.vertexOffset = constant->offset;
	// 		fragment_location[i].impl.vertexSize = constant->size;
	// 		fragment_location[i].impl.vertexColumns = constant->columns;
	// 		fragment_location[i].impl.vertexRows = constant->rows;
	// 		fragment_sizes[i] = constant->size;
	// 	}
	// }
}

kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(struct kinc_g4_pipeline *state, const char *name) {
	kinc_g4_constant_location_t location = {0};

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	kinc_internal_shader_constant_t *constant = findConstant(state->vertex_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.vertexOffset = 0;
		location.impl.vertexSize = 0;
		location.impl.vertexColumns = 0;
		location.impl.vertexRows = 0;
	}
	else {
		location.impl.vertexOffset = constant->offset;
		location.impl.vertexSize = constant->size;
		location.impl.vertexColumns = constant->columns;
		location.impl.vertexRows = constant->rows;
	}

	constant = findConstant(state->fragment_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.fragmentOffset = 0;
		location.impl.fragmentSize = 0;
		location.impl.fragmentColumns = 0;
		location.impl.fragmentRows = 0;
	}
	else {
		location.impl.fragmentOffset = constant->offset;
		location.impl.fragmentSize = constant->size;
		location.impl.fragmentColumns = constant->columns;
		location.impl.fragmentRows = constant->rows;
	}

	constant = state->geometry_shader == NULL ? NULL : findConstant(state->geometry_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.geometryOffset = 0;
		location.impl.geometrySize = 0;
		location.impl.geometryColumns = 0;
		location.impl.geometryRows = 0;
	}
	else {
		location.impl.geometryOffset = constant->offset;
		location.impl.geometrySize = constant->size;
		location.impl.geometryColumns = constant->columns;
		location.impl.geometryRows = constant->rows;
	}

	constant = state->tessellation_control_shader == NULL ? NULL : findConstant(state->tessellation_control_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.tessControlOffset = 0;
		location.impl.tessControlSize = 0;
		location.impl.tessControlColumns = 0;
		location.impl.tessControlRows = 0;
	}
	else {
		location.impl.tessControlOffset = constant->offset;
		location.impl.tessControlSize = constant->size;
		location.impl.tessControlColumns = constant->columns;
		location.impl.tessControlRows = constant->rows;
	}

	constant = state->tessellation_evaluation_shader == NULL ? NULL : findConstant(state->tessellation_evaluation_shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.tessEvalOffset = 0;
		location.impl.tessEvalSize = 0;
		location.impl.tessEvalColumns = 0;
		location.impl.tessEvalRows = 0;
	}
	else {
		location.impl.tessEvalOffset = constant->offset;
		location.impl.tessEvalSize = constant->size;
		location.impl.tessEvalColumns = constant->columns;
		location.impl.tessEvalRows = constant->rows;
	}

	if (location.impl.vertexSize == 0 && location.impl.fragmentSize == 0 && location.impl.geometrySize == 0 && location.impl.tessControlSize == 0 &&
	    location.impl.tessEvalSize == 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}

	return location;
}

kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(struct kinc_g4_pipeline *state, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63)
		len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_g4_texture_unit_t unit;
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	kinc_internal_hash_index_t *fragmentUnit = findTextureUnit(state->fragment_shader->impl.textures, hash);
	if (fragmentUnit != NULL) {
		unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] = fragmentUnit->index + unitOffset;
	}

	kinc_internal_hash_index_t *vertexUnit = findTextureUnit(state->vertex_shader->impl.textures, hash);
	if (vertexUnit != NULL) {
		unit.stages[KINC_G4_SHADER_TYPE_VERTEX] = vertexUnit->index + unitOffset;
	}

	return unit;
}

static char stringCache[1024];
static int stringCacheIndex = 0;

static void setVertexDesc(D3D11_INPUT_ELEMENT_DESC *vertexDesc, int attributeIndex, int index, int stream, bool instanced, int subindex) {
	if (subindex < 0) {
		vertexDesc->SemanticName = "TEXCOORD";
		vertexDesc->SemanticIndex = attributeIndex;
	}
	else {
		// SPIRV_CROSS uses TEXCOORD_0_0,... for split up matrices
		int stringStart = stringCacheIndex;
		strcpy(&stringCache[stringCacheIndex], "TEXCOORD");
		stringCacheIndex += (int)strlen("TEXCOORD");
		sprintf(&stringCache[stringCacheIndex], "%i", attributeIndex);
		stringCacheIndex += (int)strlen(&stringCache[stringCacheIndex]);
		strcpy(&stringCache[stringCacheIndex], "_");
		stringCacheIndex += 2;
		vertexDesc->SemanticName = &stringCache[stringStart];
		vertexDesc->SemanticIndex = subindex;
	}
	vertexDesc->InputSlot = stream;
	vertexDesc->AlignedByteOffset = (index == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc->InputSlotClass = instanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc->InstanceDataStepRate = instanced ? 1 : 0;
}

#define usedCount 32

static int getAttributeLocation(kinc_internal_hash_index_t *attributes, const char *name, bool *used) {
	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	for (int i = 0; i < 64; ++i) {
		if (attributes[i].hash == hash) {
			return attributes[i].index;
		}
	}

	for (int i = 0; i < usedCount; ++i) {
		if (!used[i]) {
			used[i] = true;
			return i;
		}
	}

	return 0;
}

static void createRenderTargetBlendDesc(struct kinc_g4_pipeline *pipe, D3D11_RENDER_TARGET_BLEND_DESC *rtbd, int targetNum) {
	rtbd->BlendEnable = pipe->blend_source != KINC_G4_BLEND_ONE || pipe->blend_destination != KINC_G4_BLEND_ZERO ||
	                    pipe->alpha_blend_source != KINC_G4_BLEND_ONE || pipe->alpha_blend_destination != KINC_G4_BLEND_ZERO;
	rtbd->SrcBlend = convert_blend_factor(pipe->blend_source);
	rtbd->DestBlend = convert_blend_factor(pipe->blend_destination);
	rtbd->BlendOp = convert_blend_operation(pipe->blend_operation);
	rtbd->SrcBlendAlpha = convert_blend_factor(pipe->alpha_blend_source);
	rtbd->DestBlendAlpha = convert_blend_factor(pipe->alpha_blend_destination);
	rtbd->BlendOpAlpha = convert_blend_operation(pipe->alpha_blend_operation);
	rtbd->RenderTargetWriteMask = (((pipe->color_write_mask_red[targetNum] ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
	                                (pipe->color_write_mask_green[targetNum] ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0)) |
	                               (pipe->color_write_mask_blue[targetNum] ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0)) |
	                              (pipe->color_write_mask_alpha[targetNum] ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);
}

void kinc_g4_pipeline_compile(struct kinc_g4_pipeline *state) {
	if (state->vertex_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = (UINT)get_multiple_of_16(state->vertex_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &state->impl.vertexConstantBuffer));
	}
	if (state->fragment_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = (UINT)get_multiple_of_16(state->fragment_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &state->impl.fragmentConstantBuffer));
	}
	if (state->geometry_shader != NULL && state->geometry_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = (UINT)get_multiple_of_16(state->geometry_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &state->impl.geometryConstantBuffer));
	}
	if (state->tessellation_control_shader != NULL && state->tessellation_control_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = (UINT)get_multiple_of_16(state->tessellation_control_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &state->impl.tessControlConstantBuffer));
	}
	if (state->tessellation_evaluation_shader != NULL && state->tessellation_evaluation_shader->impl.constantsSize > 0) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = (UINT)get_multiple_of_16(state->tessellation_evaluation_shader->impl.constantsSize);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &state->impl.tessEvalConstantBuffer));
	}

	int all = 0;
	for (int stream = 0; state->input_layout[stream] != NULL; ++stream) {
		for (int index = 0; index < state->input_layout[stream]->size; ++index) {
			if (state->input_layout[stream]->elements[index].data == KINC_G4_VERTEX_DATA_F32_4X4) {
				all += 4;
			}
			else {
				all += 1;
			}
		}
	}

	bool used[usedCount];
	for (int i = 0; i < usedCount; ++i)
		used[i] = false;
	for (int i = 0; i < 64; ++i) {
		used[state->vertex_shader->impl.attributes[i].index] = true;
	}
	stringCacheIndex = 0;
	D3D11_INPUT_ELEMENT_DESC *vertexDesc = (D3D11_INPUT_ELEMENT_DESC *)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * all);

	int i = 0;
	for (int stream = 0; state->input_layout[stream] != NULL; ++stream) {
		for (int index = 0; index < state->input_layout[stream]->size; ++index) {
			switch (state->input_layout[stream]->elements[index].data) {
			case KINC_G4_VERTEX_DATA_F32_1X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_F32_2X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_F32_3X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_F32_4X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I8_1X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U8_1X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8_UNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I8_2X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U8_2X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8_UNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I8_4X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U8_4X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I16_1X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U16_1X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16_UNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I16_2X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U16_2X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16_UNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I16_4X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U16_4X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_UNORM;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I32_1X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U32_1X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I32_2X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U32_2X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I32_3X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U32_3X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_I32_4X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_SINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_U32_4X:
				setVertexDesc(&vertexDesc[i],
				              getAttributeLocation(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name, used), index,
				              stream, state->input_layout[stream]->instanced, -1);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_UINT;
				++i;
				break;
			case KINC_G4_VERTEX_DATA_F32_4X4: {
				char name[101];
				strcpy(name, state->input_layout[stream]->elements[index].name);
				strcat(name, "_");
				size_t length = strlen(name);
				sprintf(&name[length], "%i", 0);
				name[length + 1] = 0;
				int attributeLocation = getAttributeLocation(state->vertex_shader->impl.attributes, name, used);

				for (int i2 = 0; i2 < 4; ++i2) {
					setVertexDesc(&vertexDesc[i], attributeLocation, index + i2, stream, state->input_layout[stream]->instanced, i2);
					vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					++i;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateInputLayout(dx_ctx.device, vertexDesc, all, state->vertex_shader->impl.data,
	                                                               state->vertex_shader->impl.length, &state->impl.d3d11inputLayout));
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.DepthEnable = state->depth_mode != KINC_G4_COMPARE_ALWAYS;
		desc.DepthWriteMask = state->depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = get_comparison(state->depth_mode);

		desc.StencilEnable = state->stencil_front_mode != KINC_G4_COMPARE_ALWAYS || state->stencil_back_mode != KINC_G4_COMPARE_ALWAYS ||
		                     state->stencil_front_both_pass != KINC_G4_STENCIL_KEEP || state->stencil_back_both_pass != KINC_G4_STENCIL_KEEP ||
		                     state->stencil_front_depth_fail != KINC_G4_STENCIL_KEEP || state->stencil_back_depth_fail != KINC_G4_STENCIL_KEEP ||
		                     state->stencil_front_fail != KINC_G4_STENCIL_KEEP || state->stencil_back_fail != KINC_G4_STENCIL_KEEP;
		desc.StencilReadMask = state->stencil_read_mask;
		desc.StencilWriteMask = state->stencil_write_mask;
		desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = get_comparison(state->stencil_front_mode);
		desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = get_stencil_action(state->stencil_front_depth_fail);
		desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = get_stencil_action(state->stencil_front_both_pass);
		desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = get_stencil_action(state->stencil_front_fail);
		desc.BackFace.StencilFunc = desc.BackFace.StencilFunc = get_comparison(state->stencil_back_mode);
		desc.BackFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = get_stencil_action(state->stencil_back_depth_fail);
		desc.BackFace.StencilPassOp = desc.BackFace.StencilPassOp = get_stencil_action(state->stencil_back_both_pass);
		desc.BackFace.StencilFailOp = desc.BackFace.StencilFailOp = get_stencil_action(state->stencil_back_fail);

		dx_ctx.device->lpVtbl->CreateDepthStencilState(dx_ctx.device, &desc, &state->impl.depthStencilState);
	}

	{
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.CullMode = convert_cull_mode(state->cull_mode);
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = TRUE;
		rasterDesc.DepthBias = 0;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.ScissorEnable = FALSE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;

		dx_ctx.device->lpVtbl->CreateRasterizerState(dx_ctx.device, &rasterDesc, &state->impl.rasterizerState);
		rasterDesc.ScissorEnable = TRUE;
		dx_ctx.device->lpVtbl->CreateRasterizerState(dx_ctx.device, &rasterDesc, &state->impl.rasterizerStateScissor);

		// We need d3d11_3 for conservative raster
		// D3D11_RASTERIZER_DESC2 rasterDesc;
		// rasterDesc.ConservativeRaster = conservativeRasterization ? D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		// dx_ctx.device->CreateRasterizerState2(&rasterDesc, &rasterizerState);
		// rasterDesc.ScissorEnable = TRUE;
		// dx_ctx.device->CreateRasterizerState2(&rasterDesc, &rasterizerStateScissor);
	}

	{
		bool independentBlend = false;
		for (int i = 1; i < 8; ++i) {
			if (state->color_write_mask_red[0] != state->color_write_mask_red[i] || state->color_write_mask_green[0] != state->color_write_mask_green[i] ||
			    state->color_write_mask_blue[0] != state->color_write_mask_blue[i] || state->color_write_mask_alpha[0] != state->color_write_mask_alpha[i]) {
				independentBlend = true;
				break;
			}
		}

		D3D11_BLEND_DESC blendDesc;
		memset(&blendDesc, 0, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = independentBlend;

		D3D11_RENDER_TARGET_BLEND_DESC rtbd[8];
		memset(&rtbd, 0, sizeof(rtbd));
		createRenderTargetBlendDesc(state, &rtbd[0], 0);
		blendDesc.RenderTarget[0] = rtbd[0];
		if (independentBlend) {
			for (int i = 1; i < 8; ++i) {
				createRenderTargetBlendDesc(state, &rtbd[i], i);
				blendDesc.RenderTarget[i] = rtbd[i];
			}
		}

		dx_ctx.device->lpVtbl->CreateBlendState(dx_ctx.device, &blendDesc, &state->impl.blendState);
	}
}
