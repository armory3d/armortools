#pragma once

#ifdef KINC_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g4_render_target;

typedef struct {
	HDC deviceContext;
	HGLRC glContext;

	int depthBufferBits;

	int framebuffer;
	unsigned vertexArray;
	struct kinc_g4_render_target renderTarget;
} Kinc_Internal_OpenGLWindow;

extern Kinc_Internal_OpenGLWindow Kinc_Internal_windows[10];

void Kinc_Internal_initWindowsGLContext(int window, int depthBufferBits, int stencilBufferBits);
void Kinc_Internal_blitWindowContent(int window);
void Kinc_Internal_setWindowRenderTarget(int window);
void Kinc_Internal_resizeWindowRenderTarget(int window, int width, int height);
#ifdef __cplusplus
}
#endif
#endif
