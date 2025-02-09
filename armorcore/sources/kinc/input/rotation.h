#pragma once

#include <kinc/global.h>

/*! \file rotation.h
    \brief Provides support for rotation-sensors.
*/

void kinc_rotation_set_callback(void (*value)(float /*x*/, float /*y*/, float /*z*/));

void kinc_internal_on_rotation(float x, float y, float z);

#ifdef KINC_IMPLEMENTATION_INPUT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <memory.h>
#include <stddef.h>

static void (*rotation_callback)(float /*x*/, float /*y*/, float /*z*/) = NULL;

void kinc_rotation_set_callback(void (*value)(float /*x*/, float /*y*/, float /*z*/)) {
	rotation_callback = value;
}

void kinc_internal_on_rotation(float x, float y, float z) {
	if (rotation_callback != NULL) {
		rotation_callback(x, y, z);
	}
}

#endif
