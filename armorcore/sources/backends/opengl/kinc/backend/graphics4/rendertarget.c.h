#include <kinc/graphics4/rendertarget.h>

#include "ogl.h"

#include <kinc/backend/graphics4/OpenGL.h>

#include <kinc/graphics4/graphics.h>
#include <kinc/log.h>
#include <kinc/system.h>

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

extern bool Kinc_Internal_SupportsDepthTexture;

static int pow2(int pow) {
	int ret = 1;
	for (int i = 0; i < pow; ++i)
		ret *= 2;
	return ret;
}

static int getPower2(int i) {
	for (int power = 0;; ++power)
		if (pow2(power) >= i)
			return pow2(power);
}

#ifdef KINC_OPENGL_ES
extern int gles_version;
#endif

bool kinc_opengl_internal_nonPow2RenderTargetsSupported() {
#ifdef KINC_OPENGL_ES
	return gles_version >= 3;
#else
	return true;
#endif
}

static void setupDepthStencil(kinc_g4_render_target_t *renderTarget, GLenum texType, int depthBufferBits, int stencilBufferBits, int width, int height) {
	if (depthBufferBits > 0 && stencilBufferBits > 0) {
		renderTarget->impl._hasDepth = true;
#if defined(KINC_OPENGL_ES) && !defined(KINC_RASPBERRY_PI) && !defined(KINC_EMSCRIPTEN)
		GLenum internalFormat = GL_DEPTH24_STENCIL8_OES;
#elif defined(KINC_OPENGL_ES)
		GLenum internalFormat = 0x88F0; // GL_DEPTH24_STENCIL8_OES
#else
		GLenum internalFormat;
		if (depthBufferBits == 24)
			internalFormat = GL_DEPTH24_STENCIL8;
		else
			internalFormat = GL_DEPTH32F_STENCIL8;
#endif
		// Renderbuffer
		// 		glGenRenderbuffers(1, &_depthRenderbuffer);
		// 		glCheckErrors();
		// 		glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
		// 		glCheckErrors();
		// 		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		// 		glCheckErrors();
		// #ifdef KINC_OPENGL_ES
		// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// #else
		// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// #endif
		// 		glCheckErrors();
		// Texture
		glGenTextures(1, &renderTarget->impl._depthTexture);
		glCheckErrors();
		glBindTexture(texType, renderTarget->impl._depthTexture);
		glCheckErrors();
		glTexImage2D(texType, 0, internalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
		glCheckErrors();
		glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glCheckErrors();
		glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->impl._framebuffer);
		glCheckErrors();
#ifdef KINC_OPENGL_ES
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texType, renderTarget->impl._depthTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, texType, renderTarget->impl._depthTexture, 0);
#else
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texType, renderTarget->impl._depthTexture, 0);
#endif
		glCheckErrors();
	}
	else if (depthBufferBits > 0) {
		renderTarget->impl._hasDepth = true;
		if (!Kinc_Internal_SupportsDepthTexture) {
			// Renderbuffer
			glGenRenderbuffers(1, &renderTarget->impl._depthTexture);
			glCheckErrors();
			glBindRenderbuffer(GL_RENDERBUFFER, renderTarget->impl._depthTexture);
			glCheckErrors();
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
			glCheckErrors();
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTarget->impl._depthTexture);
			glCheckErrors();
		}
		else {
			// Texture
			glGenTextures(1, &renderTarget->impl._depthTexture);
			glCheckErrors();
			glBindTexture(texType, renderTarget->impl._depthTexture);
			glCheckErrors();
#if defined(KINC_EMSCRIPTEN) || defined(KINC_WASM)
			GLint format = GL_DEPTH_COMPONENT16;
#else
			GLint format = depthBufferBits == 16 ? GL_DEPTH_COMPONENT16 : GL_DEPTH_COMPONENT;
#endif
			glTexImage2D(texType, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
			glCheckErrors();
			glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glCheckErrors();
			glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->impl._framebuffer);
			glCheckErrors();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texType, renderTarget->impl._depthTexture, 0);
			glCheckErrors();
		}
	}
}

void kinc_g4_render_target_init_with_multisampling(kinc_g4_render_target_t *renderTarget, int width, int height, kinc_g4_render_target_format_t format,
                                                   int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	renderTarget->width = width;
	renderTarget->height = height;
	renderTarget->isCubeMap = false;
	renderTarget->isDepthAttachment = false;

	renderTarget->impl._hasDepth = false;

	if (kinc_opengl_internal_nonPow2RenderTargetsSupported()) {
		renderTarget->texWidth = width;
		renderTarget->texHeight = height;
	}
	else {
		renderTarget->texWidth = getPower2(width);
		renderTarget->texHeight = getPower2(height);
	}

	renderTarget->impl.format = (int)format;

	glGenTextures(1, &renderTarget->impl._texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, renderTarget->impl._texture);
	glCheckErrors();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheckErrors();

	switch (format) {
	case KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT:
#ifdef KINC_OPENGL_ES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_EXT, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#endif
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
#ifdef KINC_OPENGL_ES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_EXT, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#endif
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH:
#ifdef KINC_OPENGL_ES
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, renderTarget->texWidth, renderTarget->texHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED:
#ifdef KINC_IOS
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
#endif
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F_EXT, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RED, GL_HALF_FLOAT, 0);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F_EXT, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RED, GL_FLOAT, 0);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	glCheckErrors();
	glGenFramebuffers(1, &renderTarget->impl._framebuffer);
	glCheckErrors();
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->impl._framebuffer);
	glCheckErrors();

	setupDepthStencil(renderTarget, GL_TEXTURE_2D, depthBufferBits, stencilBufferBits, renderTarget->texWidth, renderTarget->texHeight);

	if (format == KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderTarget->impl._texture, 0);
#ifndef KINC_OPENGL_ES
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
#endif
	}
	else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget->impl._texture, 0);
	}
	glCheckErrors();
	// GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	// glDrawBuffers(1, drawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, 0);
	glCheckErrors();
}

void kinc_g4_render_target_init_cube_with_multisampling(kinc_g4_render_target_t *renderTarget, int cubeMapSize, kinc_g4_render_target_format_t format,
                                                        int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	renderTarget->width = cubeMapSize;
	renderTarget->height = cubeMapSize;
	renderTarget->isCubeMap = true;
	renderTarget->isDepthAttachment = false;

	renderTarget->impl._hasDepth = false;

	if (kinc_opengl_internal_nonPow2RenderTargetsSupported()) {
		renderTarget->texWidth = renderTarget->width;
		renderTarget->texHeight = renderTarget->height;
	}
	else {
		renderTarget->texWidth = getPower2(renderTarget->width);
		renderTarget->texHeight = getPower2(renderTarget->height);
	}

	renderTarget->impl.format = (int)format;

	glGenTextures(1, &renderTarget->impl._texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_CUBE_MAP, renderTarget->impl._texture);
	glCheckErrors();

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheckErrors();

	switch (format) {
	case KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT:
#ifdef KINC_OPENGL_ES
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F_EXT, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#else
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#endif
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
#ifdef KINC_OPENGL_ES
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F_EXT, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#else
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#endif
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH:
#ifdef KINC_OPENGL_ES
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16, renderTarget->texWidth, renderTarget->texHeight, 0, GL_DEPTH_COMPONENT,
			             GL_UNSIGNED_INT, 0);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, renderTarget->texWidth, renderTarget->texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	glCheckErrors();
	glGenFramebuffers(1, &renderTarget->impl._framebuffer);
	glCheckErrors();
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->impl._framebuffer);
	glCheckErrors();

	setupDepthStencil(renderTarget, GL_TEXTURE_CUBE_MAP, depthBufferBits, stencilBufferBits, renderTarget->texWidth, renderTarget->texHeight);

	if (format == KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH) {
		renderTarget->isDepthAttachment = true;
#ifndef KINC_OPENGL_ES
		glDrawBuffer(GL_NONE);
		glCheckErrors();
		glReadBuffer(GL_NONE);
		glCheckErrors();
#endif
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glCheckErrors();
}

void kinc_g4_render_target_destroy(kinc_g4_render_target_t *renderTarget) {
	{
		GLuint textures[] = {renderTarget->impl._texture};
		glDeleteTextures(1, textures);
	}
	if (renderTarget->impl._hasDepth) {
		GLuint textures[] = {renderTarget->impl._depthTexture};
		glDeleteTextures(1, textures);
	}
	GLuint framebuffers[] = {renderTarget->impl._framebuffer};
	glDeleteFramebuffers(1, framebuffers);
}

void kinc_g4_render_target_use_color_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {
	glActiveTexture(GL_TEXTURE0 + unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
	glCheckErrors();
	glBindTexture(renderTarget->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTarget->impl._texture);
	glCheckErrors();
}

void kinc_g4_render_target_use_depth_as_texture(kinc_g4_render_target_t *renderTarget, kinc_g4_texture_unit_t unit) {
	glActiveTexture(GL_TEXTURE0 + unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
	glCheckErrors();
	glBindTexture(renderTarget->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTarget->impl._depthTexture);
	glCheckErrors();
}

void kinc_g4_render_target_set_depth_stencil_from(kinc_g4_render_target_t *renderTarget, kinc_g4_render_target_t *source) {
	renderTarget->impl._depthTexture = source->impl._depthTexture;
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->impl._framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderTarget->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTarget->impl._depthTexture,
	                       0);
}

void kinc_g4_render_target_get_pixels(kinc_g4_render_target_t *renderTarget, uint8_t *data) {
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->impl._framebuffer);
	switch ((kinc_g4_render_target_format_t)renderTarget->impl.format) {
	case KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		glReadPixels(0, 0, renderTarget->texWidth, renderTarget->texHeight, GL_RGBA, GL_FLOAT, data);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		glReadPixels(0, 0, renderTarget->texWidth, renderTarget->texHeight, GL_RGBA, GL_HALF_FLOAT, data);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED:
		glReadPixels(0, 0, renderTarget->texWidth, renderTarget->texHeight, GL_RED, GL_UNSIGNED_BYTE, data);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		glReadPixels(0, 0, renderTarget->texWidth, renderTarget->texHeight, GL_RED, GL_HALF_FLOAT, data);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		glReadPixels(0, 0, renderTarget->texWidth, renderTarget->texHeight, GL_RED, GL_FLOAT, data);
		break;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		glReadPixels(0, 0, renderTarget->texWidth, renderTarget->texHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
}

void kinc_g4_render_target_generate_mipmaps(kinc_g4_render_target_t *renderTarget, int levels) {
	glBindTexture(GL_TEXTURE_2D, renderTarget->impl._texture);
	glCheckErrors();
	glGenerateMipmap(GL_TEXTURE_2D);
	glCheckErrors();
}
