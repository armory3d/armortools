#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>
#include <kinc/log.h>
#import <Metal/Metal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

id getMetalDevice(void);
id getMetalEncoder(void);

static MTLBlendFactor convert_blending_factor(kinc_g5_blending_factor_t factor) {
	switch (factor) {
	case KINC_G5_BLEND_ONE:
		return MTLBlendFactorOne;
	case KINC_G5_BLEND_ZERO:
		return MTLBlendFactorZero;
	case KINC_G5_BLEND_SOURCE_ALPHA:
		return MTLBlendFactorSourceAlpha;
	case KINC_G5_BLEND_DEST_ALPHA:
		return MTLBlendFactorDestinationAlpha;
	case KINC_G5_BLEND_INV_SOURCE_ALPHA:
		return MTLBlendFactorOneMinusSourceAlpha;
	case KINC_G5_BLEND_INV_DEST_ALPHA:
		return MTLBlendFactorOneMinusDestinationAlpha;
	case KINC_G5_BLEND_SOURCE_COLOR:
		return MTLBlendFactorSourceColor;
	case KINC_G5_BLEND_DEST_COLOR:
		return MTLBlendFactorDestinationColor;
	case KINC_G5_BLEND_INV_SOURCE_COLOR:
		return MTLBlendFactorOneMinusSourceColor;
	case KINC_G5_BLEND_INV_DEST_COLOR:
		return MTLBlendFactorOneMinusDestinationColor;
	case KINC_G5_BLEND_CONSTANT:
		return MTLBlendFactorBlendColor;
	case KINC_G5_BLEND_INV_CONSTANT:
		return MTLBlendFactorOneMinusBlendColor;
	}
}

static MTLBlendOperation convert_blending_operation(kinc_g5_blending_operation_t op) {
	switch (op) {
	case KINC_G5_BLENDOP_ADD:
		return MTLBlendOperationAdd;
	case KINC_G5_BLENDOP_SUBTRACT:
		return MTLBlendOperationSubtract;
	case KINC_G5_BLENDOP_REVERSE_SUBTRACT:
		return MTLBlendOperationReverseSubtract;
	case KINC_G5_BLENDOP_MIN:
		return MTLBlendOperationMin;
	case KINC_G5_BLENDOP_MAX:
		return MTLBlendOperationMax;
	}
}

static MTLCompareFunction convert_compare_mode(kinc_g5_compare_mode_t compare) {
	switch (compare) {
	case KINC_G5_COMPARE_MODE_ALWAYS:
		return MTLCompareFunctionAlways;
	case KINC_G5_COMPARE_MODE_NEVER:
		return MTLCompareFunctionNever;
	case KINC_G5_COMPARE_MODE_EQUAL:
		return MTLCompareFunctionEqual;
	case KINC_G5_COMPARE_MODE_NOT_EQUAL:
		return MTLCompareFunctionNotEqual;
	case KINC_G5_COMPARE_MODE_LESS:
		return MTLCompareFunctionLess;
	case KINC_G5_COMPARE_MODE_LESS_EQUAL:
		return MTLCompareFunctionLessEqual;
	case KINC_G5_COMPARE_MODE_GREATER:
		return MTLCompareFunctionGreater;
	case KINC_G5_COMPARE_MODE_GREATER_EQUAL:
		return MTLCompareFunctionGreaterEqual;
	}
}

static MTLCullMode convert_cull_mode(kinc_g5_cull_mode_t cull) {
	switch (cull) {
	case KINC_G5_CULL_MODE_CLOCKWISE:
		return MTLCullModeFront;
	case KINC_G5_CULL_MODE_COUNTERCLOCKWISE:
		return MTLCullModeBack;
	case KINC_G5_CULL_MODE_NEVER:
		return MTLCullModeNone;
	}
}

static MTLPixelFormat convert_render_target_format(kinc_g5_render_target_format_t format) {
	switch (format) {
	case KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return MTLPixelFormatRGBA32Float;
	case KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return MTLPixelFormatRGBA16Float;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return MTLPixelFormatR32Float;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return MTLPixelFormatR16Float;
	case KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED:
		return MTLPixelFormatR8Unorm;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT:
	default:
		return MTLPixelFormatBGRA8Unorm;
	}
}

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline) {
	memset(&pipeline->impl, 0, sizeof(pipeline->impl));
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline) {
	pipeline->impl._reflection = NULL;
	pipeline->impl._depthStencil = NULL;

	id<MTLRenderPipelineState> pipe = (__bridge_transfer id<MTLRenderPipelineState>)pipeline->impl._pipeline;
	pipe = nil;
	pipeline->impl._pipeline = NULL;

	MTLRenderPipelineReflection *reflection = (__bridge_transfer MTLRenderPipelineReflection *)pipeline->impl._reflection;
	reflection = nil;
	pipeline->impl._reflection = NULL;

	id<MTLRenderPipelineState> pipeDepth = (__bridge_transfer id<MTLRenderPipelineState>)pipeline->impl._pipelineDepth;
	pipeDepth = nil;
	pipeline->impl._pipelineDepth = NULL;

	id<MTLDepthStencilState> depthStencil = (__bridge_transfer id<MTLDepthStencilState>)pipeline->impl._depthStencil;
	depthStencil = nil;
	pipeline->impl._depthStencil = NULL;

	id<MTLDepthStencilState> depthStencilNone = (__bridge_transfer id<MTLDepthStencilState>)pipeline->impl._depthStencilNone;
	depthStencilNone = nil;
	pipeline->impl._depthStencilNone = NULL;
}

static int findAttributeIndex(NSArray<MTLVertexAttribute *> *attributes, const char *name) {
	for (MTLVertexAttribute *attribute in attributes) {
		if (strcmp(name, [[attribute name] UTF8String]) == 0) {
			return (int)[attribute attributeIndex];
		}
	}
	return -1;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline) {
	MTLRenderPipelineDescriptor *renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = (__bridge id<MTLFunction>)pipeline->vertexShader->impl.mtlFunction;
	renderPipelineDesc.fragmentFunction = (__bridge id<MTLFunction>)pipeline->fragmentShader->impl.mtlFunction;
	for (int i = 0; i < pipeline->colorAttachmentCount; ++i) {
		renderPipelineDesc.colorAttachments[i].pixelFormat = convert_render_target_format(pipeline->colorAttachment[i]);
		renderPipelineDesc.colorAttachments[i].blendingEnabled =
		    pipeline->blend_source != KINC_G5_BLEND_ONE || pipeline->blend_destination != KINC_G5_BLEND_ZERO ||
		    pipeline->alpha_blend_source != KINC_G5_BLEND_ONE || pipeline->alpha_blend_destination != KINC_G5_BLEND_ZERO;
		renderPipelineDesc.colorAttachments[i].sourceRGBBlendFactor = convert_blending_factor(pipeline->blend_source);
		renderPipelineDesc.colorAttachments[i].destinationRGBBlendFactor = convert_blending_factor(pipeline->blend_destination);
		renderPipelineDesc.colorAttachments[i].rgbBlendOperation = convert_blending_operation(pipeline->blend_operation);
		renderPipelineDesc.colorAttachments[i].sourceAlphaBlendFactor = convert_blending_factor(pipeline->alpha_blend_source);
		renderPipelineDesc.colorAttachments[i].destinationAlphaBlendFactor = convert_blending_factor(pipeline->alpha_blend_destination);
		renderPipelineDesc.colorAttachments[i].alphaBlendOperation = convert_blending_operation(pipeline->alpha_blend_operation);
		renderPipelineDesc.colorAttachments[i].writeMask =
		    (pipeline->colorWriteMaskRed[i] ? MTLColorWriteMaskRed : 0) | (pipeline->colorWriteMaskGreen[i] ? MTLColorWriteMaskGreen : 0) |
		    (pipeline->colorWriteMaskBlue[i] ? MTLColorWriteMaskBlue : 0) | (pipeline->colorWriteMaskAlpha[i] ? MTLColorWriteMaskAlpha : 0);
	}
	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
	renderPipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;

	float offset = 0;
	MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];

	for (int i = 0; i < pipeline->inputLayout->size; ++i) {
		int index = findAttributeIndex(renderPipelineDesc.vertexFunction.vertexAttributes, pipeline->inputLayout->elements[i].name);

		if (index < 0) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Could not find vertex attribute %s\n", pipeline->inputLayout->elements[i].name);
		}

		if (index >= 0) {
			vertexDescriptor.attributes[index].bufferIndex = 0;
			vertexDescriptor.attributes[index].offset = offset;
		}

		offset += kinc_g4_vertex_data_size(pipeline->inputLayout->elements[i].data);
		if (index >= 0) {
			switch (pipeline->inputLayout->elements[i].data) {
			case KINC_G4_VERTEX_DATA_NONE:
				assert(false);
				break;
			case KINC_G4_VERTEX_DATA_F32_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat;
				break;
			case KINC_G4_VERTEX_DATA_F32_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat2;
				break;
			case KINC_G4_VERTEX_DATA_F32_3X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat3;
				break;
			case KINC_G4_VERTEX_DATA_F32_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat4;
				break;
			case KINC_G4_VERTEX_DATA_F32_4X4:
				assert(false);
				break;
			case KINC_G4_VERTEX_DATA_I8_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatChar;
				break;
			case KINC_G4_VERTEX_DATA_U8_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUChar;
				break;
			case KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatCharNormalized;
				break;
			case KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUCharNormalized;
				break;
			case KINC_G4_VERTEX_DATA_I8_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatChar2;
				break;
			case KINC_G4_VERTEX_DATA_U8_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUChar2;
				break;
			case KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatChar2Normalized;
				break;
			case KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUChar2Normalized;
				break;
			case KINC_G4_VERTEX_DATA_I8_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatChar4;
				break;
			case KINC_G4_VERTEX_DATA_U8_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUChar4;
				break;
			case KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatChar4Normalized;
				break;
			case KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUChar4Normalized;
				break;
			case KINC_G4_VERTEX_DATA_I16_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShort;
				break;
			case KINC_G4_VERTEX_DATA_U16_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUShort;
				break;
			case KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShortNormalized;
				break;
			case KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUShortNormalized;
				break;
			case KINC_G4_VERTEX_DATA_I16_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShort2;
				break;
			case KINC_G4_VERTEX_DATA_U16_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUShort2;
				break;
			case KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShort2Normalized;
				break;
			case KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUShort2Normalized;
				break;
			case KINC_G4_VERTEX_DATA_I16_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShort4;
				break;
			case KINC_G4_VERTEX_DATA_U16_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUShort4;
				break;
			case KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShort4Normalized;
				break;
			case KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUShort4Normalized;
				break;
			case KINC_G4_VERTEX_DATA_I32_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatInt;
				break;
			case KINC_G4_VERTEX_DATA_U32_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUInt;
				break;
			case KINC_G4_VERTEX_DATA_I32_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatInt2;
				break;
			case KINC_G4_VERTEX_DATA_U32_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUInt2;
				break;
			case KINC_G4_VERTEX_DATA_I32_3X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatInt3;
				break;
			case KINC_G4_VERTEX_DATA_U32_3X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUInt3;
				break;
			case KINC_G4_VERTEX_DATA_I32_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatInt4;
				break;
			case KINC_G4_VERTEX_DATA_U32_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUInt4;
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	vertexDescriptor.layouts[0].stride = offset;
	vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

	renderPipelineDesc.vertexDescriptor = vertexDescriptor;

	NSError *errors = nil;
	MTLRenderPipelineReflection *reflection = nil;
	id<MTLDevice> device = getMetalDevice();

	pipeline->impl._pipeline = (__bridge_retained void *)[device newRenderPipelineStateWithDescriptor:renderPipelineDesc
	                                                                                          options:MTLPipelineOptionBufferTypeInfo
	                                                                                       reflection:&reflection
	                                                                                            error:&errors];
	if (errors != nil)
		NSLog(@"%@", [errors localizedDescription]);
	assert(pipeline->impl._pipeline && !errors);

	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	renderPipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	pipeline->impl._pipelineDepth = (__bridge_retained void *)[device newRenderPipelineStateWithDescriptor:renderPipelineDesc
	                                                                                               options:MTLPipelineOptionBufferTypeInfo
	                                                                                            reflection:&reflection
	                                                                                                 error:&errors];
	if (errors != nil)
		NSLog(@"%@", [errors localizedDescription]);
	assert(pipeline->impl._pipelineDepth && !errors);

	pipeline->impl._reflection = (__bridge_retained void *)reflection;

	MTLDepthStencilDescriptor *depthStencilDescriptor = [MTLDepthStencilDescriptor new];
	depthStencilDescriptor.depthCompareFunction = convert_compare_mode(pipeline->depthMode);
	depthStencilDescriptor.depthWriteEnabled = pipeline->depthWrite;
	pipeline->impl._depthStencil = (__bridge_retained void *)[device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

	depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
	depthStencilDescriptor.depthWriteEnabled = false;
	pipeline->impl._depthStencilNone = (__bridge_retained void *)[device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
}

bool kinc_internal_current_render_target_has_depth(void);

void kinc_g5_internal_pipeline_set(kinc_g5_pipeline_t *pipeline) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	if (kinc_internal_current_render_target_has_depth()) {
		id<MTLRenderPipelineState> pipe = (__bridge id<MTLRenderPipelineState>)pipeline->impl._pipelineDepth;
		[encoder setRenderPipelineState:pipe];
		id<MTLDepthStencilState> depthStencil = (__bridge id<MTLDepthStencilState>)pipeline->impl._depthStencil;
		[encoder setDepthStencilState:depthStencil];
	}
	else {
		id<MTLRenderPipelineState> pipe = (__bridge id<MTLRenderPipelineState>)pipeline->impl._pipeline;
		[encoder setRenderPipelineState:pipe];
		id<MTLDepthStencilState> depthStencil = (__bridge id<MTLDepthStencilState>)pipeline->impl._depthStencilNone;
		[encoder setDepthStencilState:depthStencil];
	}
	[encoder setFrontFacingWinding:MTLWindingClockwise];
	[encoder setCullMode:convert_cull_mode(pipeline->cullMode)];
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name) {
	if (strcmp(name, "bias") == 0) {
		name = "bias0";
	}

	kinc_g5_constant_location_t location;
	location.impl.vertexOffset = -1;
	location.impl.fragmentOffset = -1;
	location.impl.computeOffset = -1;

	MTLRenderPipelineReflection *reflection = (__bridge MTLRenderPipelineReflection *)pipeline->impl._reflection;

	for (MTLArgument *arg in reflection.vertexArguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType *structObj = [arg bufferStructType];
				for (MTLStructMember *member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.vertexOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	for (MTLArgument *arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeBuffer && [[arg name] isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType *structObj = [arg bufferStructType];
				for (MTLStructMember *member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.fragmentOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name) {
	kinc_g5_texture_unit_t unit = {0};
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	MTLRenderPipelineReflection *reflection = (__bridge MTLRenderPipelineReflection *)pipeline->impl._reflection;
	for (MTLArgument *arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] = (int)[arg index];
			break;
		}
	}

	for (MTLArgument *arg in reflection.vertexArguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.stages[KINC_G5_SHADER_TYPE_VERTEX] = (int)[arg index];
			break;
		}
	}

	return unit;
}
