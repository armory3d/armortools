#pragma once

#include <kinc/global.h>

#include <kinc/graphics4/shader.h>

/*! \file textureunit.h
    \brief Provides a texture-unit-struct which is used for setting textures in a shader.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_unit {
	int stages[KINC_G4_SHADER_TYPE_COUNT];
} kinc_g4_texture_unit_t;

#ifdef __cplusplus
}
#endif
