#include <kinc/graphics4/texture.h>

#include "ogl.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/image.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

#include "OpenGL.h"

#include <stdlib.h>

#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D 0x806F
#endif

#ifndef GL_RGBA16F_EXT
#define GL_RGBA16F_EXT 0x881A
#endif

#ifndef GL_RGBA32F_EXT
#define GL_RGBA32F_EXT 0x8814
#endif

#ifndef GL_R16F_EXT
#define GL_R16F_EXT 0x822D
#endif

#ifndef GL_R32F_EXT
#define GL_R32F_EXT 0x822E
#endif

#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT 0x140B
#endif

#ifndef GL_RED
#define GL_RED GL_LUMINANCE
#endif

#ifndef GL_R8
#define GL_R8 GL_RED
#endif

#ifndef GL_RGBA8
#define GL_RGBA8 GL_RGBA
#endif

#ifndef GL_KHR_texture_compression_astc_ldr
#define GL_KHR_texture_compression_astc_ldr 1

#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR 0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR 0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR 0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR 0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR 0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR 0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR 0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR 0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR 0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR 0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR 0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR 0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR 0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR 0x93DD
#endif

static int convertFormat(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_BGRA32:
#ifdef GL_BGRA
		return GL_BGRA;
#else
		return GL_RGBA;
#endif
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_RGBA64:
	case KINC_IMAGE_FORMAT_RGBA128:
	default:
		return GL_RGBA;
	case KINC_IMAGE_FORMAT_RGB24:
		return GL_RGB;
	case KINC_IMAGE_FORMAT_A32:
	case KINC_IMAGE_FORMAT_A16:
	case KINC_IMAGE_FORMAT_GREY8:
		return GL_RED;
	}
}

static int convertInternalFormat(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return GL_RGBA32F_EXT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return GL_RGBA16F_EXT;
	case KINC_IMAGE_FORMAT_RGBA32:
		return GL_RGBA8;
	default:
#ifdef KINC_IOS
		return GL_RGBA;
#else
		// #ifdef GL_BGRA
		// return GL_BGRA;
		// #else
		return GL_RGBA;
// #endif
#endif
	case KINC_IMAGE_FORMAT_RGB24:
		return GL_RGB;
	case KINC_IMAGE_FORMAT_A32:
		return GL_R32F_EXT;
	case KINC_IMAGE_FORMAT_A16:
		return GL_R16F_EXT;
	case KINC_IMAGE_FORMAT_GREY8:
#ifdef KINC_IOS
		return GL_RED;
#else
		return GL_R8;
#endif
	}
}

static int convertType(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
	case KINC_IMAGE_FORMAT_RGBA64:
	case KINC_IMAGE_FORMAT_A32:
	case KINC_IMAGE_FORMAT_A16:
		return GL_FLOAT;
	case KINC_IMAGE_FORMAT_RGBA32:
	default:
		return GL_UNSIGNED_BYTE;
	}
}

static int astcFormat(uint8_t blockX, uint8_t blockY) {
	switch (blockX) {
	case 4:
		switch (blockY) {
		case 4:
			return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
		}
	case 5:
		switch (blockY) {
		case 4:
			return GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
		case 5:
			return GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
		}
	case 6:
		switch (blockY) {
		case 5:
			return GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
		case 6:
			return GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
		}
	case 8:
		switch (blockY) {
		case 5:
			return GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
		case 6:
			return GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
		case 8:
			return GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
		}
	case 10:
		switch (blockY) {
		case 5:
			return GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
		case 6:
			return GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
		case 8:
			return GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
		case 10:
			return GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
		}
	case 12:
		switch (blockY) {
		case 10:
			return GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
		case 12:
			return GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
		}
	}
	return 0;
}

/*static int pow2(int pow) {
    int ret = 1;
    for (int i = 0; i < pow; ++i) ret *= 2;
    return ret;
}

static int getPower2(int i) {
    for (int power = 0;; ++power)
        if (pow2(power) >= i) return pow2(power);
}*/

static void convertImageToPow2(kinc_image_format_t format, uint8_t *from, int fw, int fh, uint8_t *to, int tw, int th) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA32:
		for (int y = 0; y < th; ++y) {
			for (int x = 0; x < tw; ++x) {
				to[tw * 4 * y + x * 4 + 0] = 0;
				to[tw * 4 * y + x * 4 + 1] = 0;
				to[tw * 4 * y + x * 4 + 2] = 0;
				to[tw * 4 * y + x * 4 + 3] = 0;
			}
		}
		for (int y = 0; y < fh; ++y) {
			for (int x = 0; x < fw; ++x) {
				to[tw * 4 * y + x * 4 + 0] = from[y * fw * 4 + x * 4 + 0];
				to[tw * 4 * y + x * 4 + 1] = from[y * fw * 4 + x * 4 + 1];
				to[tw * 4 * y + x * 4 + 2] = from[y * fw * 4 + x * 4 + 2];
				to[tw * 4 * y + x * 4 + 3] = from[y * fw * 4 + x * 4 + 3];
			}
		}
		break;
	case KINC_IMAGE_FORMAT_GREY8:
		for (int y = 0; y < th; ++y) {
			for (int x = 0; x < tw; ++x) {
				to[tw * y + x] = 0;
			}
		}
		for (int y = 0; y < fh; ++y) {
			for (int x = 0; x < fw; ++x) {
				to[tw * y + x] = from[y * fw + x];
			}
		}
		break;
	case KINC_IMAGE_FORMAT_RGB24:
	case KINC_IMAGE_FORMAT_RGBA128:
	case KINC_IMAGE_FORMAT_RGBA64:
	case KINC_IMAGE_FORMAT_A32:
	case KINC_IMAGE_FORMAT_A16:
	case KINC_IMAGE_FORMAT_BGRA32:
		break;
	}
}

void kinc_g4_texture_init_from_image(kinc_g4_texture_t *texture, kinc_image_t *image) {
	texture->format = image->format;
	bool toPow2;
#ifdef KINC_IOS
	texture->tex_width = image->width;
	texture->tex_height = image->height;
	toPow2 = false;
#else
	if (kinc_g4_supports_non_pow2_textures()) {
		texture->tex_width = image->width;
		texture->tex_height = image->height;
		toPow2 = false;
	}
	else {
		texture->tex_width = getPower2(image->width);
		texture->tex_height = getPower2(image->height);
		toPow2 = !(texture->tex_width == image->width && texture->tex_height == image->height);
	}
#endif
	texture->tex_depth = 1;

	uint8_t *conversionBuffer = NULL;

	switch (image->compression) {
	case KINC_IMAGE_COMPRESSION_NONE:
		if (toPow2) {
			conversionBuffer = (uint8_t *)malloc(texture->tex_width * texture->tex_height * kinc_image_format_sizeof(image->format));
			convertImageToPow2(image->format, (uint8_t *)image->data, image->width, image->height, conversionBuffer, texture->tex_width, texture->tex_height);
		}
		break;
	case KINC_IMAGE_COMPRESSION_PVRTC:
		texture->tex_width = kinc_maxi(texture->tex_width, texture->tex_height);
		texture->tex_height = kinc_maxi(texture->tex_width, texture->tex_height);
		if (texture->tex_width < 8)
			texture->tex_width = 8;
		if (texture->tex_height < 8)
			texture->tex_height = 8;
		break;
	default:
		texture->tex_width = image->width;
		texture->tex_height = image->height;
		break;
	}

#ifdef KINC_ANDROID
	texture->impl.external_oes = false;
#endif

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCheckErrors();
	glGenTextures(1, &texture->impl.texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, texture->impl.texture);
	glCheckErrors();

	int convertedType = convertType(image->format);
	bool isHdr = convertedType == GL_FLOAT;

	switch (image->compression) {
	case KINC_IMAGE_COMPRESSION_PVRTC:
#ifdef KINC_IOS
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, texture->tex_width, texture->tex_height, 0,
		                       texture->tex_width * texture->tex_height / 2, image->data);
#endif
		break;
	case KINC_IMAGE_COMPRESSION_ASTC: {
		uint8_t blockX = image->internal_format >> 8;
		uint8_t blockY = image->internal_format & 0xff;
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, astcFormat(blockX, blockY), texture->tex_width, texture->tex_height, 0, image->data_size, image->data);
		break;
	}
	case KINC_IMAGE_COMPRESSION_DXT5:
#ifdef KINC_WINDOWS
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, texture->tex_width, texture->tex_height, 0, image->data_size, image->data);
#endif
		break;
	case KINC_IMAGE_COMPRESSION_NONE: {
		void *texdata = image->data;
		if (!isHdr && toPow2) {
			texdata = conversionBuffer;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, convertInternalFormat(image->format), texture->tex_width, texture->tex_height, 0, convertFormat(image->format),
		             convertedType, texdata);
		glCheckErrors();
		break;
	}
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (toPow2) {
		free(conversionBuffer);
		conversionBuffer = NULL;
	}

	/*if (!readable) {
	    if (isHdr) {
	        free(texture->image.hdrData);
	        texture->image.hdrData = NULL;
	    }
	    else {
	        free(texture->image.data);
	        texture->image.data = NULL;
	    }
	}

	if (readable && texture->image.compression != KINC_IMAGE_COMPRESSION_NONE) {
	    kinc_log(KINC_LOG_LEVEL_WARNING, "Compressed images can not be readable.");
	}*/
}

void kinc_g4_texture_init_from_image3d(kinc_g4_texture_t *texture, kinc_image_t *image) {
	texture->format = image->format;
#ifndef KINC_OPENGL_ES // Requires GLES 3.0
	texture->tex_width = image->width;
	texture->tex_height = image->height;
	texture->tex_depth = image->depth;

#ifdef KINC_ANDROID
	external_oes = false;
#endif

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCheckErrors();
	glGenTextures(1, &texture->impl.texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_3D, texture->impl.texture);
	glCheckErrors();

	int convertedType = convertType(image->format);
	// bool isHdr = convertedType == GL_FLOAT;

	void *texdata = image->data;
	glTexImage3D(GL_TEXTURE_3D, 0, convertInternalFormat(image->format), texture->tex_width, texture->tex_height, texture->tex_depth, 0,
	             convertFormat(image->format), convertedType, texdata);
	glCheckErrors();

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();

	/*if (!readable) {
	    if (isHdr) {
	        free(texture->image.hdrData);
	        texture->image.hdrData = NULL;
	    }
	    else {
	        free(texture->image.data);
	        texture->image.data = NULL;
	    }
	}

	if (texture->image.compression != KINC_IMAGE_COMPRESSION_NONE) {
	    kinc_log(KINC_LOG_LEVEL_WARNING, "Compressed images can not be 3D.");
	}*/
#endif
}

void kinc_g4_texture_init(kinc_g4_texture_t *texture, int width, int height, kinc_image_format_t format) {
#ifdef KINC_IOS
	texture->tex_width = width;
	texture->tex_height = height;
#else
	if (kinc_g4_supports_non_pow2_textures()) {
		texture->tex_width = width;
		texture->tex_height = height;
	}
	else {
		texture->tex_width = getPower2(width);
		texture->tex_height = getPower2(height);
	}
#endif
	texture->tex_depth = 1;
	texture->format = format;
	// conversionBuffer = new u8[texWidth * texHeight * 4];

#ifdef KINC_ANDROID
	texture->impl.external_oes = false;
#endif

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCheckErrors();
	glGenTextures(1, &texture->impl.texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, texture->impl.texture);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();

	if (convertType(format) == GL_FLOAT) {
		glTexImage2D(GL_TEXTURE_2D, 0, convertInternalFormat(format), texture->tex_width, texture->tex_height, 0, convertFormat(format), GL_FLOAT, NULL);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, convertInternalFormat(format), texture->tex_width, texture->tex_height, 0, convertFormat(format), GL_UNSIGNED_BYTE,
		             NULL);
	}
	glCheckErrors();
}

void kinc_g4_texture_init3d(kinc_g4_texture_t *texture, int width, int height, int depth, kinc_image_format_t format) {
#ifndef KINC_OPENGL_ES
	texture->tex_width = width;
	texture->tex_height = height;
	texture->tex_depth = depth;
	texture->format = format;

	glGenTextures(1, &texture->impl.texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_3D, texture->impl.texture);
	glCheckErrors();

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();

	glTexImage3D(GL_TEXTURE_3D, 0, convertInternalFormat(format), width, height, depth, 0, convertFormat(format), GL_UNSIGNED_BYTE, NULL);
	glCheckErrors();
#endif
}

#ifdef KINC_ANDROID
void kinc_g4_texture_init_from_id(kinc_g4_texture_t *texture, unsigned texid) {
	texture->impl.texture = texid;
	texture->impl.external_oes = true;
	texture->tex_width = 1023;
	texture->tex_height = 684;
	texture->format = KINC_IMAGE_FORMAT_RGBA32;
}
#endif

void kinc_g4_texture_destroy(kinc_g4_texture_t *texture) {
	glDeleteTextures(1, &texture->impl.texture);
	glFlush();
}

void Kinc_G4_Internal_TextureSet(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit) {
	GLenum target = texture->tex_depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		if (unit.stages[i] >= 0) {
			glActiveTexture(GL_TEXTURE0 + unit.stages[i]);
		}
	}
	glCheckErrors();
#ifdef KINC_ANDROID
	if (texture->impl.external_oes) {
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture->impl.texture);
		glCheckErrors();
	}
	else {
		glBindTexture(target, texture->impl.texture);
		glCheckErrors();
	}
#else
	glBindTexture(target, texture->impl.texture);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Kinc_G4_Internal_TextureAddressingU(unit));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Kinc_G4_Internal_TextureAddressingV(unit));
#endif
}

void Kinc_G4_Internal_TextureImageSet(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit) {
#if defined(KINC_WINDOWS) || (defined(KINC_LINUX) && defined(GL_VERSION_4_4))
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		if (unit.stages[i] >= 0) {
			glBindImageTexture(unit.stages[i], texture->impl.texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, convertInternalFormat(texture->format));
		}
	}
	glCheckErrors();
#endif
}

int kinc_g4_texture_stride(kinc_g4_texture_t *texture) {
	return texture->tex_width * kinc_image_format_sizeof(texture->format);
}

static uint8_t *lock_cache = NULL;

unsigned char *kinc_g4_texture_lock(kinc_g4_texture_t *texture) {
	if (lock_cache == NULL) {
		lock_cache = (uint8_t *)malloc(4096 * 4096 * 4);
	}
	return lock_cache; //**(texture->image.data ? texture->image.data : (uint8_t *)texture->image.hdrData);
}

/*void Texture::unlock() {
    if (conversionBuffer != nullptr) {
        convertImageToPow2(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
        glBindTexture(GL_TEXTURE_2D, texture);
#ifndef GL_LUMINANCE
#define GL_LUMINANCE GL_RED
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, texWidth, texHeight, 0, (format == Image::RGBA32) ? GL_RGBA :
GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
    }
}*/

void kinc_g4_texture_unlock(kinc_g4_texture_t *texture) {
	GLenum target = texture->tex_depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	void *texdata = lock_cache;
	glBindTexture(target, texture->impl.texture);
	glCheckErrors();
	if (texture->tex_depth > 1) {
#ifndef KINC_OPENGL_ES
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, texture->tex_width, texture->tex_height, texture->tex_depth, convertFormat(texture->format),
		                convertType(texture->format), texdata);
#endif
	}
	else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->tex_width, texture->tex_height, convertFormat(texture->format), convertType(texture->format), texdata);
	}
	glCheckErrors();
}

void kinc_g4_texture_clear(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {
#ifdef GL_VERSION_4_4
	static float clearColor[4];
	clearColor[0] = ((color & 0x00ff0000) >> 16) / 255.0f;
	clearColor[1] = ((color & 0x0000ff00) >> 8) / 255.0f;
	clearColor[2] = (color & 0x000000ff) / 255.0f;
	clearColor[3] = ((color & 0xff000000) >> 24) / 255.0f;
	GLenum target = depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(target, texture->impl.texture);
	glClearTexSubImage(texture->impl.texture, 0, x, y, z, width, height, depth, convertFormat(texture->format), convertType(texture->format), clearColor);
#endif
}

#if defined(KINC_IOS) || defined(KINC_MACOS)
void kinc_g4_texture_upload(kinc_g4_texture_t *texture, uint8_t *data, int stride) {
	glBindTexture(GL_TEXTURE_2D, texture->impl.texture);
	glCheckErrors();
	glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->tex_width, texture->tex_height, convertFormat(texture->format), GL_UNSIGNED_BYTE, data);
	glCheckErrors();
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
#endif

void kinc_g4_texture_generate_mipmaps(kinc_g4_texture_t *texture, int levels) {
	GLenum target = texture->tex_depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(target, texture->impl.texture);
	glCheckErrors();
	glGenerateMipmap(target);
	glCheckErrors();
}

void kinc_g4_texture_set_mipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level) {
	int convertedType = convertType(mipmap->format);
	// bool isHdr = convertedType == GL_FLOAT;
	GLenum target = texture->tex_depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(target, texture->impl.texture);
	glCheckErrors();
	glTexImage2D(target, level, convertInternalFormat(mipmap->format), mipmap->width, mipmap->height, 0, convertFormat(mipmap->format), convertedType,
	             mipmap->data);
	glCheckErrors();
}
