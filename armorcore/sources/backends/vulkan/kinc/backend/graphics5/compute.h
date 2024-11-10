#pragma once

#include <kinc/backend/graphics5/ShaderHash.h>

#include "MiniVulkan.h"

#include "named_number.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_compute_shader_impl {
	kinc_internal_named_number locations[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number texture_bindings[KINC_INTERNAL_NAMED_NUMBER_COUNT];
	kinc_internal_named_number offsets[KINC_INTERNAL_NAMED_NUMBER_COUNT];

	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
	VkShaderModule shader_module;
} kinc_g5_compute_shader_impl;

#ifdef __cplusplus
}
#endif
