#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics5/pipeline.h>

/*! \file constantlocation.h
    \brief Provides the constant_location-struct which is used for setting constants/uniforms in a shader.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_constant_location {
	ConstantLocation5Impl impl;
} kinc_g5_constant_location_t;

#ifdef __cplusplus
}
#endif
