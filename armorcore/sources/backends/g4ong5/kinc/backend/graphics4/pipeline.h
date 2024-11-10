#pragma once

#include <kinc/graphics5/constantlocation.h>
#include <kinc/graphics5/pipeline.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	// PipelineStateImpl();
	kinc_g5_pipeline_t _pipeline;
} kinc_g4_pipeline_impl_t;

typedef struct {
	kinc_g5_constant_location_t _location;
} kinc_g4_constant_location_impl_t;

typedef struct {
	int nothing;
} Kinc_G4_AttributeLocationImpl;

#ifdef __cplusplus
}
#endif
