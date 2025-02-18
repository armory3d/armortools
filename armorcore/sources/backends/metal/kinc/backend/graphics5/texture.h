#pragma once

#include <kinc/graphics5/texture.h>

typedef struct {
	int index;
	bool vertex;
} TextureUnit5Impl;

typedef struct {
	void *_tex;
	void *data;
	bool has_mipmaps;
} Texture5Impl;
