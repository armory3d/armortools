#include "pipeline.h"

void kinc_g5_internal_pipeline_init(kinc_g5_pipeline_t *pipe) {
	pipe->inputLayout = NULL;

	pipe->vertexShader = NULL;
	pipe->fragmentShader = NULL;

	pipe->cullMode = KINC_G5_CULL_MODE_NEVER;

	pipe->depthWrite = false;
	pipe->depthMode = KINC_G5_COMPARE_MODE_ALWAYS;

	pipe->blend_source = KINC_G5_BLEND_ONE;
	pipe->blend_destination = KINC_G5_BLEND_ZERO;
	pipe->blend_operation = KINC_G5_BLENDOP_ADD;
	pipe->alpha_blend_source = KINC_G5_BLEND_ONE;
	pipe->alpha_blend_destination = KINC_G5_BLEND_ZERO;
	pipe->alpha_blend_operation = KINC_G5_BLENDOP_ADD;

	for (int i = 0; i < 8; ++i) {
		pipe->colorWriteMaskRed[i] = true;
	}
	for (int i = 0; i < 8; ++i) {
		pipe->colorWriteMaskGreen[i] = true;
	}
	for (int i = 0; i < 8; ++i) {
		pipe->colorWriteMaskBlue[i] = true;
	}
	for (int i = 0; i < 8; ++i) {
		pipe->colorWriteMaskAlpha[i] = true;
	}

	pipe->colorAttachmentCount = 1;
	for (int i = 0; i < 8; ++i) {
		pipe->colorAttachment[i] = KINC_G5_RENDER_TARGET_FORMAT_32BIT;
	}

	pipe->depthAttachmentBits = 0;
}
