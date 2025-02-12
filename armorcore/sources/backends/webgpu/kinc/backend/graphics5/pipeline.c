#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/constantlocation.h>
#include <assert.h>
#include <string.h>

extern WGPUDevice device;

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipe) {
	kinc_g5_internal_pipeline_init(pipe);
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipe, const char* name) {
	kinc_g5_constant_location_t location;
	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t unit;
	return unit;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
	WGPUColorTargetState csDesc;
	memset(&csDesc, 0, sizeof(csDesc));
	csDesc.format = WGPUTextureFormat_BGRA8Unorm;
	csDesc.writeMask = WGPUColorWriteMask_All;
	WGPUBlendState blend;
	memset(&blend, 0, sizeof(blend));
	blend.color.operation = WGPUBlendOperation_Add;
	blend.color.srcFactor = WGPUBlendFactor_One;
	blend.color.dstFactor = WGPUBlendFactor_Zero;
	blend.alpha.operation = WGPUBlendOperation_Add;
	blend.alpha.srcFactor = WGPUBlendFactor_One;
	blend.alpha.dstFactor = WGPUBlendFactor_Zero;
	csDesc.blend = &blend;

	WGPUPipelineLayoutDescriptor plDesc;
	memset(&plDesc, 0, sizeof(plDesc));
	plDesc.bindGroupLayoutCount = 0;
	plDesc.bindGroupLayouts = NULL;

	WGPUVertexAttribute vaDesc[8];
	memset(&vaDesc[0], 0, sizeof(vaDesc[0]) * 8);
	uint64_t offset = 0;
	for (int i = 0; i < pipe->inputLayout->size; ++i) {
		vaDesc[i].shaderLocation = i;
		vaDesc[i].offset = offset;
		offset += kinc_g4_vertex_data_size(pipe->inputLayout->elements[i].data);
		switch (pipe->inputLayout->elements[i].data) {
		case KINC_G4_VERTEX_DATA_NONE:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_F32_1X:
			vaDesc[i].format = WGPUVertexFormat_Float32;
			break;
		case KINC_G4_VERTEX_DATA_F32_2X:
			vaDesc[i].format = WGPUVertexFormat_Float32x2;
			break;
		case KINC_G4_VERTEX_DATA_F32_3X:
			vaDesc[i].format = WGPUVertexFormat_Float32x3;
			break;
		case KINC_G4_VERTEX_DATA_F32_4X:
			vaDesc[i].format = WGPUVertexFormat_Float32x4;
			break;
		case KINC_G4_VERTEX_DATA_F32_4X4:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_I8_1X:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_U8_1X:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_I8_2X:
			vaDesc[i].format = WGPUVertexFormat_Sint8x2;
			break;
		case KINC_G4_VERTEX_DATA_U8_2X:
			vaDesc[i].format = WGPUVertexFormat_Uint8x2 ;
			break;
		case KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Snorm8x2;
			break;
		case KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Unorm8x2;
			break;
		case KINC_G4_VERTEX_DATA_I8_4X:
			vaDesc[i].format = WGPUVertexFormat_Sint8x4;
			break;
		case KINC_G4_VERTEX_DATA_U8_4X:
			vaDesc[i].format = WGPUVertexFormat_Uint8x4;
			break;
		case KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Snorm8x4;
			break;
		case KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Unorm8x4;
			break;
		case KINC_G4_VERTEX_DATA_I16_1X:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_U16_1X:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Undefined;
			assert(false);
			break;
		case KINC_G4_VERTEX_DATA_I16_2X:
			vaDesc[i].format = WGPUVertexFormat_Sint16x2;
			break;
		case KINC_G4_VERTEX_DATA_U16_2X:
			vaDesc[i].format = WGPUVertexFormat_Uint16x2;
			break;
		case KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x2;
			break;
		case KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Unorm16x2;
			break;
		case KINC_G4_VERTEX_DATA_I16_4X:
			vaDesc[i].format = WGPUVertexFormat_Sint16x4;
			break;
		case KINC_G4_VERTEX_DATA_U16_4X:
			vaDesc[i].format = WGPUVertexFormat_Uint16x4;
			break;
		case KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x4;
			break;
		case KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Unorm16x4;
			break;
		case KINC_G4_VERTEX_DATA_I32_1X:
			vaDesc[i].format = WGPUVertexFormat_Sint32;
			break;
		case KINC_G4_VERTEX_DATA_U32_1X:
			vaDesc[i].format = WGPUVertexFormat_Uint32;
			break;
		case KINC_G4_VERTEX_DATA_I32_2X:
			vaDesc[i].format = WGPUVertexFormat_Sint32x2;
			break;
		case KINC_G4_VERTEX_DATA_U32_2X:
			vaDesc[i].format = WGPUVertexFormat_Uint32x2;
			break;
		case KINC_G4_VERTEX_DATA_I32_3X:
			vaDesc[i].format = WGPUVertexFormat_Sint32x3;
			break;
		case KINC_G4_VERTEX_DATA_U32_3X:
			vaDesc[i].format = WGPUVertexFormat_Uint32x3;
			break;
		case KINC_G4_VERTEX_DATA_I32_4X:
			vaDesc[i].format = WGPUVertexFormat_Sint32x4;
			break;
		case KINC_G4_VERTEX_DATA_U32_4X:
			vaDesc[i].format = WGPUVertexFormat_Uint32x4;
			break;
		}
	}

	WGPUVertexBufferLayout vbDesc;
	memset(&vbDesc, 0, sizeof(vbDesc));
	vbDesc.arrayStride = offset;
	vbDesc.attributeCount = pipe->inputLayout->size;
	vbDesc.attributes = &vaDesc[0];

	WGPUVertexState vsDest;
	memset(&vsDest, 0, sizeof(vsDest));

	vsDest.module = pipe->vertexShader->impl.module;
	vsDest.entryPoint = "main";

	vsDest.bufferCount = 1;
	vsDest.buffers = &vbDesc;

	WGPUFragmentState fragmentDest;
	memset(&fragmentDest, 0, sizeof(fragmentDest));

	fragmentDest.module = pipe->fragmentShader->impl.module;
	fragmentDest.entryPoint = "main";

	fragmentDest.targetCount = 1;
	fragmentDest.targets = &csDesc;

	WGPUPrimitiveState rsDesc;
	memset(&rsDesc, 0, sizeof(rsDesc));
	rsDesc.topology = WGPUPrimitiveTopology_TriangleList;
	rsDesc.stripIndexFormat = WGPUIndexFormat_Uint32;
	rsDesc.frontFace = WGPUFrontFace_CW;
	rsDesc.cullMode = WGPUCullMode_None;

	WGPUMultisampleState multisample;
	memset(&multisample, 0, sizeof(multisample));
	multisample.count = 1;
	multisample.mask = 0xffffffff;
	multisample.alphaToCoverageEnabled = false;

	WGPURenderPipelineDescriptor rpDesc;
	memset(&rpDesc, 0, sizeof(rpDesc));
	rpDesc.layout = wgpuDeviceCreatePipelineLayout(device, &plDesc);
	rpDesc.fragment = &fragmentDest;
	rpDesc.vertex = vsDest;
	rpDesc.multisample = multisample;
	rpDesc.primitive = rsDesc;
	pipe->impl.pipeline = wgpuDeviceCreateRenderPipeline(device, &rpDesc);
}
