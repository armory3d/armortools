#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics4/pipeline.h>
#include <kinc/backend/graphics4/shader.h>

/*! \file constantlocation.h
    \brief Provides the constant_location-struct which is used for setting constants/uniforms in a shader.
*/

typedef struct kinc_g4_constant_location {
	kinc_g4_constant_location_impl_t impl;
} kinc_g4_constant_location_t;
