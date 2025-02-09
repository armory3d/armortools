#pragma once

#include <kinc/graphics5/texture.h>
#include <kinc/graphics5/textureunit.h>
#include <kinc/image.h>

typedef struct {
	kinc_g5_texture_unit_t _unit;
} kinc_g4_texture_unit_impl_t;

typedef struct {
	kinc_g5_texture_t _texture;
	bool _uploaded;
} kinc_g4_texture_impl_t;
