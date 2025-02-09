#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics5/texture.h>
#include <kinc/graphics5/shader.h>

/*! \file textureunit.h
    \brief Provides a texture-unit-struct which is used for setting textures in a shader.
*/

typedef struct kinc_g5_texture_unit {
	int stages[KINC_G5_SHADER_TYPE_COUNT];
} kinc_g5_texture_unit_t;

bool kinc_g5_texture_unit_equals(kinc_g5_texture_unit_t *unit1, kinc_g5_texture_unit_t *unit2);
