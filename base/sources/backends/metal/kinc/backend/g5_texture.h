#pragma once

#include <iron_gpu.h>

typedef struct {
	int index;
	bool vertex;
} TextureUnit5Impl;

typedef struct {
	void *_tex;
	void *data;
	bool has_mipmaps;
} Texture5Impl;

typedef struct {
	void *_tex;
	void *_texReadback;
	void *_depthTex;
} RenderTarget5Impl;
