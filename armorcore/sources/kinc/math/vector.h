#pragma once

#include "core.h"

/*! \file vector.h
    \brief Provides basic vector types.
*/

typedef struct kinc_vector2 {
	float x;
	float y;
} kinc_vector2_t;

typedef struct kinc_vector3 {
	float x;
	float y;
	float z;
} kinc_vector3_t;

typedef struct kinc_vector4 {
	float x;
	float y;
	float z;
	float w;
} kinc_vector4_t;
