#pragma once

#include <kinc/graphics5/texture.h>
#include <kinc/graphics5/textureunit.h>
#include <kinc/image.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_g5_texture_unit_t _unit;
} kinc_g4_texture_unit_impl_t;

typedef struct {
	/*TextureImpl();
	TextureImpl(int width, int height, Image::Format format, bool readable);
	TextureImpl(int width, int height, int depth, Image::Format format, bool readable);
	~TextureImpl();
	void unmipmap();
	void unset();*/

	kinc_g5_texture_t _texture;
	bool _uploaded;
} kinc_g4_texture_impl_t;

#ifdef __cplusplus
}
#endif
