#include "OpenGL.h"
#include "ogl.h"
#ifdef KINC_EGL
#define EGL_NO_PLATFORM_SPECIFIC_TYPES
#include <EGL/egl.h>
#endif

#include <kinc/backend/graphics4/vertexbuffer.h>

#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/vertexbuffer.h>

#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/math/core.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include "OpenGLWindow.h"

#ifdef KINC_WINDOWS
#include <kinc/backend/Windows.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef KINC_IOS
#include <OpenGLES/ES2/glext.h>
#endif

#ifdef KINC_WINDOWS
#include <GL/wglew.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

#ifndef GL_MAX_COLOR_ATTACHMENTS
#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif
#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE 0x884C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#endif
#ifndef GL_COMPARE_REF_TO_TEXTURE
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#endif

#if !defined(KINC_IOS) && !defined(KINC_ANDROID)
bool Kinc_Internal_ProgramUsesTessellation;
#endif
bool Kinc_Internal_SupportsConservativeRaster = false;
bool Kinc_Internal_SupportsDepthTexture = true;

#if defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18
void *glesVertexAttribDivisor;

GL_APICALL void (*GL_APIENTRY glesGenQueries)(GLsizei n, GLuint *ids);
GL_APICALL void (*GL_APIENTRY glesDeleteQueries)(GLsizei n, const GLuint *ids);
GL_APICALL void (*GL_APIENTRY glesBeginQuery)(GLenum target, GLuint id);
GL_APICALL void (*GL_APIENTRY glesEndQuery)(GLenum target);
GL_APICALL void (*GL_APIENTRY glesGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params);
#endif

#if defined(KINC_WINDOWS) && !defined(NDEBUG)
static void __stdcall debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	kinc_log(KINC_LOG_LEVEL_INFO, "OpenGL: %s", message);
}
#endif

#ifdef KINC_WINDOWS
static HINSTANCE instance = 0;
#endif

static int currentWindow = 0;

static kinc_g4_texture_filter_t minFilters[32];
static kinc_g4_mipmap_filter_t mipFilters[32];

static int _renderTargetWidth;
static int _renderTargetHeight;
static bool renderToBackbuffer;

static int maxColorAttachments;

static kinc_g4_pipeline_t *lastPipeline = NULL;

#if defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18
static void *glesDrawBuffers;
static void *glesDrawElementsInstanced;
#endif

static int texModesU[256];
static int texModesV[256];

void kinc_internal_resize(int window, int width, int height) {
#ifdef KINC_WINDOWS
	Kinc_Internal_resizeWindowRenderTarget(window, width, height);
#endif
	glViewport(0, 0, width, height);
}

void kinc_internal_change_framebuffer(int window, kinc_framebuffer_options_t *frame) {
#ifdef KINC_WINDOWS
	if (window == 0) {
		if (wglSwapIntervalEXT != NULL)
			wglSwapIntervalEXT(frame->vertical_sync);
	}
#endif
}

#ifdef KINC_EGL
static EGLDisplay egl_display = EGL_NO_DISPLAY;
static EGLContext egl_context = EGL_NO_CONTEXT;
static EGLConfig egl_config = NULL;

struct {
	EGLSurface surface;
} kinc_egl_windows[16] = {0};

EGLDisplay kinc_egl_get_display(void);
EGLNativeWindowType kinc_egl_get_native_window(EGLDisplay, EGLConfig, int);
void kinc_egl_init();
void kinc_egl_init_window(int window);
void kinc_egl_destroy_window(int window);
#endif

#ifdef KINC_EGL
#define EGL_CHECK_ERROR()                                                                                                                                      \
	{                                                                                                                                                          \
		EGLint error = eglGetError();                                                                                                                          \
		if (error != EGL_SUCCESS) {                                                                                                                            \
			kinc_log(KINC_LOG_LEVEL_ERROR, "EGL Error at line %i: %i", __LINE__, error);                                                                       \
			kinc_debug_break();                                                                                                                                \
			exit(1);                                                                                                                                           \
		}                                                                                                                                                      \
	}
#endif

void kinc_g4_internal_destroy() {
#ifdef KINC_EGL
	if (egl_display != EGL_NO_DISPLAY) {
		eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		for (int i = 0; i < 16; i++) {
			if (kinc_egl_windows[i].surface != EGL_NO_SURFACE) {
				eglDestroySurface(egl_display, kinc_egl_windows[i].surface);
				kinc_egl_windows[i].surface = EGL_NO_SURFACE;
			}
		}
		if (egl_context != EGL_NO_CONTEXT) {
			eglDestroyContext(egl_display, egl_context);
			egl_context = EGL_NO_CONTEXT;
		}
		eglTerminate(egl_display);
	}

	egl_display = EGL_NO_DISPLAY;
#endif
}

void kinc_g4_internal_destroy_window(int window) {
#ifdef KINC_EGL
	kinc_egl_destroy_window(window);
#endif
#ifdef KINC_WINDOWS
	if (Kinc_Internal_windows[window].glContext) {
		assert(wglMakeCurrent(NULL, NULL));
		assert(wglDeleteContext(Kinc_Internal_windows[window].glContext));
		Kinc_Internal_windows[window].glContext = NULL;
	}

	HWND windowHandle = kinc_windows_window_handle(window);

	if (Kinc_Internal_windows[window].deviceContext != NULL) {
		ReleaseDC(windowHandle, Kinc_Internal_windows[window].deviceContext);
		Kinc_Internal_windows[window].deviceContext = NULL;
	}
#endif
}

#ifdef CreateWindow
#undef CreateWindow
#endif

void kinc_g4_internal_init() {
#ifdef KINC_EGL
#if !defined(KINC_OPENGL_ES)
	eglBindAPI(EGL_OPENGL_API);
#else
	eglBindAPI(EGL_OPENGL_ES_API);
#endif
	kinc_egl_init();
#endif

	for (int i = 0; i < 32; ++i) {
		minFilters[i] = KINC_G4_TEXTURE_FILTER_LINEAR;
		mipFilters[i] = KINC_G4_MIPMAP_FILTER_NONE;
	}

	for (int i = 0; i < 256; ++i) {
		texModesU[i] = GL_CLAMP_TO_EDGE;
		texModesV[i] = GL_CLAMP_TO_EDGE;
	}

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &kinc_internal_opengl_max_vertex_attribute_arrays);
}

#ifdef KINC_EGL
EGLDisplay kinc_egl_get_display(void);
EGLNativeWindowType kinc_egl_get_native_window(EGLDisplay, EGLConfig, int);
#endif

extern bool kinc_internal_opengl_force_16bit_index_buffer;

#ifdef KINC_OPENGL_ES
int gles_version = 2;
#endif

void kinc_g4_internal_init_window(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
#ifdef KINC_WINDOWS
	Kinc_Internal_initWindowsGLContext(windowId, depthBufferBits, stencilBufferBits);
#endif
#ifdef KINC_EGL
	kinc_egl_init_window(windowId);
#endif

#ifdef KINC_WINDOWS
	if (windowId == 0) {
		if (wglSwapIntervalEXT != NULL)
			wglSwapIntervalEXT(vsync);
	}
#endif

	renderToBackbuffer = true;

#if defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18
	glesDrawBuffers = (void *)eglGetProcAddress("glDrawBuffers");
	glesDrawElementsInstanced = (void *)eglGetProcAddress("glDrawElementsInstanced");
	glesVertexAttribDivisor = (void *)eglGetProcAddress("glVertexAttribDivisor");

	glesGenQueries = (void *)eglGetProcAddress("glGenQueries");
	glesDeleteQueries = (void *)eglGetProcAddress("glDeleteQueries");
	glesBeginQuery = (void *)eglGetProcAddress("glBeginQuery");
	glesEndQuery = (void *)eglGetProcAddress("glEndQuery");
	glesGetQueryObjectuiv = (void *)eglGetProcAddress("glGetQueryObjectuiv");
#endif

#if defined(KINC_WINDOWS) && !defined(NDEBUG)
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debugCallback, NULL);
#endif

#ifndef KINC_OPENGL_ES
	int extensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensions);
	if (glGetError() != GL_NO_ERROR) {
		for (int i = 0; i < extensions; ++i) {
			const char *extension = (const char *)glGetStringi(GL_EXTENSIONS, i);
			if (extension != NULL && strcmp(extension, "GL_NV_conservative_raster") == 0) {
				Kinc_Internal_SupportsConservativeRaster = true;
			}
		}
	}
	maxColorAttachments = 8;
#endif

#ifdef KINC_OPENGL_ES
	{
		int major = -1;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glCheckErrors();
		gles_version = major;
		char *exts = (char *)glGetString(GL_EXTENSIONS);

		Kinc_Internal_SupportsDepthTexture = major >= 3 || (exts != NULL && strstr(exts, "GL_OES_depth_texture") != NULL);
		maxColorAttachments = 4;
		kinc_internal_opengl_force_16bit_index_buffer = major < 3 && strstr(exts, "GL_OES_element_index_uint") == NULL;
	}
#else
	kinc_internal_opengl_force_16bit_index_buffer = false;
#endif

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);

	lastPipeline = NULL;

#ifndef KINC_OPENGL_ES
	int major = -1, minor = -1;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	if (major < 0 || minor < 0) {
		const GLubyte *version = glGetString(GL_VERSION);
		if (version != NULL) {
			major = version[0] - '0';
		}
		else {
			major = 2;
		}
		minor = 0;
	}
	int gl_version = major * 100 + minor * 10;
#endif

#if defined(KINC_LINUX) || defined(KINC_MACOS)
	if (gl_version >= 300) {
		unsigned vertexArray;
		glGenVertexArrays(1, &vertexArray);
		glCheckErrors();
		glBindVertexArray(vertexArray);
		glCheckErrors();
	}
#endif
}

bool kinc_window_vsynced(int window) {
#ifdef KINC_WINDOWS
	return wglGetSwapIntervalEXT();
#else
	return true;
#endif
}

void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value) {
	glUniform1i(location.impl.location, value ? 1 : 0);
	glCheckErrors();
}

void kinc_g4_set_int(kinc_g4_constant_location_t location, int value) {
	glUniform1i(location.impl.location, value);
	glCheckErrors();
}

void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2) {
	glUniform2i(location.impl.location, value1, value2);
	glCheckErrors();
}

void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3) {
	glUniform3i(location.impl.location, value1, value2, value3);
	glCheckErrors();
}

void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4) {
	glUniform4i(location.impl.location, value1, value2, value3, value4);
	glCheckErrors();
}

void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count) {
	switch (location.impl.type) {
	case GL_INT_VEC2:
		glUniform2iv(location.impl.location, count / 2, values);
		break;
	case GL_INT_VEC3:
		glUniform3iv(location.impl.location, count / 3, values);
		break;
	case GL_INT_VEC4:
		glUniform4iv(location.impl.location, count / 4, values);
		break;
	default:
		glUniform1iv(location.impl.location, count, values);
		break;
	}
	glCheckErrors();
}

void kinc_g4_set_float(kinc_g4_constant_location_t location, float value) {
	glUniform1f(location.impl.location, value);
	glCheckErrors();
}

void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2) {
	glUniform2f(location.impl.location, value1, value2);
	glCheckErrors();
}

void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3) {
	glUniform3f(location.impl.location, value1, value2, value3);
	glCheckErrors();
}

void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4) {
	glUniform4f(location.impl.location, value1, value2, value3, value4);
	glCheckErrors();
}

void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count) {
	switch (location.impl.type) {
	case GL_FLOAT_VEC2:
		glUniform2fv(location.impl.location, count / 2, values);
		break;
	case GL_FLOAT_VEC3:
		glUniform3fv(location.impl.location, count / 3, values);
		break;
	case GL_FLOAT_VEC4:
		glUniform4fv(location.impl.location, count / 4, values);
		break;
	case GL_FLOAT_MAT4:
		glUniformMatrix4fv(location.impl.location, count / 16, false, values);
		break;
	default:
		glUniform1fv(location.impl.location, count, values);
		break;
	}
	glCheckErrors();
}

void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value) {
	glUniformMatrix4fv(location.impl.location, 1, GL_FALSE, value->m);
	glCheckErrors();
}

void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value) {
	glUniformMatrix3fv(location.impl.location, 1, GL_FALSE, value->m);
	glCheckErrors();
}

kinc_g4_index_buffer_t *Kinc_Internal_CurrentIndexBuffer;

void kinc_g4_draw_indexed_vertices() {
	kinc_g4_draw_indexed_vertices_from_to(0, kinc_g4_index_buffer_count(Kinc_Internal_CurrentIndexBuffer));
}

void kinc_g4_draw_indexed_vertices_from_to(int start, int count) {
	bool sixteen = Kinc_Internal_CurrentIndexBuffer->impl.format == KINC_G4_INDEX_BUFFER_FORMAT_16BIT || kinc_internal_opengl_force_16bit_index_buffer;
	GLenum type = sixteen ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	void *_start = sixteen ? (void *)(start * sizeof(uint16_t)) : (void *)(start * sizeof(uint32_t));

#ifndef KINC_OPENGL_ES
	if (Kinc_Internal_ProgramUsesTessellation) {
		glDrawElements(GL_PATCHES, count, type, _start);
		glCheckErrors();
	}
	else {
#endif
		glDrawElements(GL_TRIANGLES, count, type, _start);
		glCheckErrors();
#ifndef KINC_OPENGL_ES
	}
#endif
}

void kinc_g4_draw_indexed_vertices_from_to_from(int start, int count, int vertex_offset) {
	bool sixteen = Kinc_Internal_CurrentIndexBuffer->impl.format == KINC_G4_INDEX_BUFFER_FORMAT_16BIT || kinc_internal_opengl_force_16bit_index_buffer;
	GLenum type = sixteen ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	void *_start = sixteen ? (void *)(start * sizeof(uint16_t)) : (void *)(start * sizeof(uint32_t));
#ifdef KINC_OPENGL_ES
	glDrawElements(GL_TRIANGLES, count, type, _start);
	glCheckErrors();
#else
	if (Kinc_Internal_ProgramUsesTessellation) {
		glDrawElementsBaseVertex(GL_PATCHES, count, type, _start, vertex_offset);
		glCheckErrors();
	}
	else {
		glDrawElementsBaseVertex(GL_TRIANGLES, count, type, _start, vertex_offset);
		glCheckErrors();
	}
#endif
}

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount) {
	kinc_g4_draw_indexed_vertices_instanced_from_to(instanceCount, 0, kinc_g4_index_buffer_count(Kinc_Internal_CurrentIndexBuffer));
}

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {
	bool sixteen = Kinc_Internal_CurrentIndexBuffer->impl.format == KINC_G4_INDEX_BUFFER_FORMAT_16BIT || kinc_internal_opengl_force_16bit_index_buffer;
	GLenum type = sixteen ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	void *_start = sixteen ? (void *)(start * sizeof(uint16_t)) : (void *)(start * sizeof(uint32_t));
#if defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18
	((void (*)(GLenum, GLsizei, GLenum, void *, GLsizei))glesDrawElementsInstanced)(GL_TRIANGLES, count, type, _start, instanceCount);
#elif defined(KINC_OPENGL_ES) && defined(KINC_WASM)
	glDrawElementsInstanced(GL_TRIANGLES, count, type, _start, instanceCount);
	glCheckErrors();
#elif !defined(KINC_OPENGL_ES)
	if (Kinc_Internal_ProgramUsesTessellation) {
		glDrawElementsInstanced(GL_PATCHES, count, type, _start, instanceCount);
		glCheckErrors();
	}
	else {
		glDrawElementsInstanced(GL_TRIANGLES, count, type, _start, instanceCount);
		glCheckErrors();
	}
#endif
}

#ifdef KINC_MACOS
void swapBuffersMac(int window);
#endif

#ifdef KINC_IOS
void swapBuffersiOS();
#endif

#ifdef KINC_EGL
static EGLint egl_major = 0;
static EGLint egl_minor = 0;
static int egl_depth_size = 0;

void kinc_egl_init() {
	egl_display = kinc_egl_get_display();
	eglInitialize(egl_display, &egl_major, &egl_minor);
	EGL_CHECK_ERROR()

	// clang-format off
	const EGLint attribs[] = {
		#if !defined(KINC_OPENGL_ES)
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		#else
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		#endif
		EGL_SURFACE_TYPE, 	 EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 		 8,
		EGL_GREEN_SIZE, 	 8,
		EGL_RED_SIZE, 		 8,
		EGL_DEPTH_SIZE, 	 24,
		EGL_STENCIL_SIZE, 	 8,
		EGL_NONE,
	};
	// clang-format on
	egl_depth_size = 24;

	EGLint num_configs = 0;
	eglChooseConfig(egl_display, attribs, &egl_config, 1, &num_configs);
	EGL_CHECK_ERROR()

	if (!num_configs) {
		// clang-format off
		const EGLint attribs[] = {
			#if !defined(KINC_OPENGL_ES)
			EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
			#else
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			#endif
			EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 		 8,
			EGL_GREEN_SIZE, 	 8,
			EGL_RED_SIZE, 		 8,
			EGL_DEPTH_SIZE, 	 16,
			EGL_STENCIL_SIZE, 	 8,
			EGL_NONE,
		};
		// clang-format on
		eglChooseConfig(egl_display, attribs, &egl_config, 1, &num_configs);
		EGL_CHECK_ERROR()
		egl_depth_size = 16;
	}

	if (!num_configs) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Unable to choose EGL config");
	}

#if !defined(KINC_OPENGL_ES)
	EGLint gl_versions[][2] = {{4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0}, {3, 3}, {3, 2}, {3, 1}, {3, 0}, {2, 1}, {2, 0}};
	bool gl_initialized = false;
	for (int i = 0; i < sizeof(gl_versions) / sizeof(EGLint) / 2; ++i) {
		{
			EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION,
			                           gl_versions[i][0],
			                           EGL_CONTEXT_MINOR_VERSION,
			                           gl_versions[i][1],
			                           EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE,
			                           EGL_TRUE,
			                           EGL_NONE};
			egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, contextAttribs);
			EGLint error = eglGetError();
			if (error == EGL_SUCCESS) {
				gl_initialized = true;
				kinc_log(KINC_LOG_LEVEL_INFO, "Using OpenGL version %i.%i (forward-compatible).", gl_versions[i][0], gl_versions[i][1]);
				break;
			}
		}

		{
			EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, gl_versions[i][0], EGL_CONTEXT_MINOR_VERSION, gl_versions[i][1], EGL_NONE};
			egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, contextAttribs);
			EGLint error = eglGetError();
			if (error == EGL_SUCCESS) {
				gl_initialized = true;
				kinc_log(KINC_LOG_LEVEL_INFO, "Using OpenGL version %i.%i.", gl_versions[i][0], gl_versions[i][1]);
				break;
			}
		}
	}

	if (!gl_initialized) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not create OpenGL-context.");
		exit(1);
	}
#else
	EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_NONE};
	egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, contextAttribs);
	EGL_CHECK_ERROR()
#endif
}

int kinc_egl_width(int window) {
	EGLint w = 0;
	eglQuerySurface(egl_display, kinc_egl_windows[window].surface, EGL_WIDTH, &w);
	return w;
}

int kinc_egl_height(int window) {
	EGLint h = 0;
	eglQuerySurface(egl_display, kinc_egl_windows[window].surface, EGL_HEIGHT, &h);
	return h;
}

void kinc_egl_init_window(int window) {
	EGLSurface egl_surface = eglCreateWindowSurface(egl_display, egl_config, kinc_egl_get_native_window(egl_display, egl_config, window), NULL);
	EGL_CHECK_ERROR()
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
	EGL_CHECK_ERROR()
	kinc_egl_windows[window].surface = egl_surface;
}

void kinc_egl_destroy_window(int window) {
	eglMakeCurrent(egl_display, kinc_egl_windows[window].surface, kinc_egl_windows[window].surface, egl_context);
	EGL_CHECK_ERROR()
	eglDestroySurface(egl_display, kinc_egl_windows[window].surface);
	EGL_CHECK_ERROR()
	kinc_egl_windows[window].surface = NULL;
}
#endif

bool kinc_g4_swap_buffers() {
#ifdef KINC_WINDOWS
	for (int i = 9; i >= 0; --i) {
		if (Kinc_Internal_windows[i].deviceContext != NULL) {
			wglMakeCurrent(Kinc_Internal_windows[i].deviceContext, Kinc_Internal_windows[i].glContext);
			if (i != 0) {
				Kinc_Internal_blitWindowContent(i);
			}
			SwapBuffers(Kinc_Internal_windows[i].deviceContext);
		}
	}
#elif defined(KINC_EGL)
	for (int window = 15; window >= 0; --window) {
		if (kinc_egl_windows[window].surface) {
			eglMakeCurrent(egl_display, kinc_egl_windows[window].surface, kinc_egl_windows[window].surface, egl_context);
			EGL_CHECK_ERROR()
			if (!eglSwapBuffers(egl_display, kinc_egl_windows[window].surface)) {
				EGLint error = eglGetError();
				if (error == EGL_BAD_SURFACE) {
					kinc_log(KINC_LOG_LEVEL_WARNING, "Recreating surface.");
					kinc_egl_init_window(window);
				}
				else if (error == EGL_CONTEXT_LOST || error == EGL_BAD_CONTEXT) {
					kinc_log(KINC_LOG_LEVEL_ERROR, "Context lost.");
					return false;
				}
			}
			EGL_CHECK_ERROR()
		}
	}
#elif defined(KINC_MACOS)
	swapBuffersMac(0);
#elif defined(KINC_IOS)
	swapBuffersiOS();
#endif
	return true;
}

#ifdef KINC_IOS
int kinc_ios_gl_framebuffer = -1;

void beginGL();
#endif

void kinc_g4_begin(int window) {
	currentWindow = window;

#ifdef KINC_EGL
	eglMakeCurrent(egl_display, kinc_egl_windows[window].surface, kinc_egl_windows[window].surface, egl_context);
	EGL_CHECK_ERROR()
#endif
#ifdef KINC_IOS
	beginGL();
#endif
	kinc_g4_restore_render_target();
	glCheckErrors();
#ifdef KINC_ANDROID
	// if rendered to a texture, strange things happen if the backbuffer is not cleared
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void kinc_g4_viewport(int x, int y, int width, int height) {
	glViewport(x, _renderTargetHeight - y - height, width, height);
}

static bool scissor_on = false;

void kinc_g4_scissor(int x, int y, int width, int height) {
	scissor_on = true;
	glEnable(GL_SCISSOR_TEST);
	if (renderToBackbuffer) {
		glScissor(x, _renderTargetHeight - y - height, width, height);
	}
	else {
		glScissor(x, y, width, height);
	}
}

void kinc_g4_disable_scissor() {
	scissor_on = false;
	glDisable(GL_SCISSOR_TEST);
}

void kinc_g4_end(int windowId) {
	currentWindow = 0;
	glCheckErrors();
}

void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil) {
	glColorMask(true, true, true, true);
	glCheckErrors();
	glClearColor(((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	             ((color & 0xff000000) >> 24) / 255.0f);
	glCheckErrors();
	if (flags & KINC_G4_CLEAR_DEPTH) {
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glCheckErrors();
	}
#ifdef KINC_OPENGL_ES
	glClearDepthf(depth);
#else
	glClearDepth(depth);
#endif
	glCheckErrors();
	glStencilMask(0xff);
	glCheckErrors();
	glClearStencil(stencil);
	glCheckErrors();
	GLbitfield oglflags = ((flags & KINC_G4_CLEAR_COLOR) ? GL_COLOR_BUFFER_BIT : 0) | ((flags & KINC_G4_CLEAR_DEPTH) ? GL_DEPTH_BUFFER_BIT : 0) |
	                      ((flags & KINC_G4_CLEAR_STENCIL) ? GL_STENCIL_BUFFER_BIT : 0);

	if (scissor_on) {
		glDisable(GL_SCISSOR_TEST);
	}

	glClear(oglflags);
	glCheckErrors();

	if (scissor_on) {
		glEnable(GL_SCISSOR_TEST);
	}

	if (lastPipeline != NULL) {
		kinc_g4_set_pipeline(lastPipeline);
	}
}

void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **vertexBuffers, int count) {
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		offset += kinc_internal_g4_vertex_buffer_set(vertexBuffers[i], offset);
	}
}

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *indexBuffer) {
	kinc_internal_g4_index_buffer_set(indexBuffer);
}

void Kinc_G4_Internal_TextureImageSet(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void Kinc_G4_Internal_TextureSet(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	Kinc_G4_Internal_TextureSet(texture, unit);
}

void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	Kinc_G4_Internal_TextureImageSet(texture, unit);
}

int kinc_g4_max_bound_textures(void) {
	int units;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &units);
	return units;
}

static void setTextureAddressingInternal(GLenum target, kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	glActiveTexture(GL_TEXTURE0 + unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
	GLenum texDir;
	switch (dir) {
	case KINC_G4_TEXTURE_DIRECTION_U:
		texDir = GL_TEXTURE_WRAP_S;
		break;
	case KINC_G4_TEXTURE_DIRECTION_V:
		texDir = GL_TEXTURE_WRAP_T;
		break;
	case KINC_G4_TEXTURE_DIRECTION_W:
#ifndef KINC_OPENGL_ES
		texDir = GL_TEXTURE_WRAP_R;
#endif
		break;
	}
	switch (addressing) {
	case KINC_G4_TEXTURE_ADDRESSING_CLAMP:
		glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
		if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
			texModesU[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_CLAMP_TO_EDGE;
		}
		else {
			texModesV[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_CLAMP_TO_EDGE;
		}
		break;
	case KINC_G4_TEXTURE_ADDRESSING_REPEAT:
		glTexParameteri(target, texDir, GL_REPEAT);
		if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
			texModesU[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_REPEAT;
		}
		else {
			texModesV[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_REPEAT;
		}
		break;
	case KINC_G4_TEXTURE_ADDRESSING_BORDER:
		// unsupported
		glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
		if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
			texModesU[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_CLAMP_TO_EDGE;
		}
		else {
			texModesV[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_CLAMP_TO_EDGE;
		}
		break;
	case KINC_G4_TEXTURE_ADDRESSING_MIRROR:
		// unsupported
		glTexParameteri(target, texDir, GL_REPEAT);
		if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
			texModesU[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_REPEAT;
		}
		else {
			texModesV[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = GL_REPEAT;
		}
		break;
	}
	glCheckErrors();
}

int Kinc_G4_Internal_TextureAddressingU(kinc_g4_texture_unit_t unit) {
	return texModesU[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]];
}

int Kinc_G4_Internal_TextureAddressingV(kinc_g4_texture_unit_t unit) {
	return texModesV[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]];
}

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	setTextureAddressingInternal(GL_TEXTURE_2D, unit, dir, addressing);
}

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
#ifndef KINC_OPENGL_ES
	setTextureAddressingInternal(GL_TEXTURE_3D, unit, dir, addressing);
#endif
}

static void setTextureMagnificationFilterInternal(GLenum target, kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	glActiveTexture(GL_TEXTURE0 + texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
	glCheckErrors();
	switch (filter) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case KINC_G4_TEXTURE_FILTER_LINEAR:
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
	glCheckErrors();
}

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	setTextureMagnificationFilterInternal(GL_TEXTURE_2D, texunit, filter);
}

void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
#ifndef KINC_OPENGL_ES
	setTextureMagnificationFilterInternal(GL_TEXTURE_3D, texunit, filter);
#endif
}

static void setMinMipFilters(GLenum target, int unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
	glCheckErrors();
	switch (minFilters[unit]) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		switch (mipFilters[unit]) {
		case KINC_G4_MIPMAP_FILTER_NONE:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case KINC_G4_MIPMAP_FILTER_POINT:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			break;
		case KINC_G4_MIPMAP_FILTER_LINEAR:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			break;
		}
		break;
	case KINC_G4_TEXTURE_FILTER_LINEAR:
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
		switch (mipFilters[unit]) {
		case KINC_G4_MIPMAP_FILTER_NONE:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case KINC_G4_MIPMAP_FILTER_POINT:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			break;
		case KINC_G4_MIPMAP_FILTER_LINEAR:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;
		}
		if (minFilters[unit] == KINC_G4_TEXTURE_FILTER_ANISOTROPIC) {
			float maxAniso = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
			glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
		}
		break;
	}
	glCheckErrors();
}

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	minFilters[texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
}

void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	minFilters[texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = filter;
#ifndef KINC_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
#endif
}

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	mipFilters[texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
}

void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	mipFilters[texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]] = filter;
#ifndef KINC_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]);
#endif
}

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t texunit, bool enabled) {
	if (texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;
	if (enabled) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void kinc_g4_set_texture_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, Kinc_G4_Internal_StencilFunc(mode));
}

void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t texunit, bool enabled) {
	if (texunit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;
	if (enabled) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
	else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void kinc_g4_set_cubemap_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, Kinc_G4_Internal_StencilFunc(mode));
}

void kinc_g4_set_texture_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
}

void kinc_g4_set_cubemap_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
}

void kinc_g4_set_texture_lod(kinc_g4_texture_unit_t unit, float lod_min_clamp, float lod_max_clamp) {
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, lod_min_clamp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, lod_max_clamp);
}

void kinc_g4_set_cubemap_lod(kinc_g4_texture_unit_t unit, float lod_min_clamp, float lod_max_clamp) {
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, lod_min_clamp);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, lod_max_clamp);
}

void kinc_g4_set_render_targets(kinc_g4_render_target_t **targets, int count) {
	glBindFramebuffer(GL_FRAMEBUFFER, targets[0]->impl._framebuffer);
	glCheckErrors();
#ifndef KINC_OPENGL_ES
	if (targets[0]->isCubeMap)
		glFramebufferTexture(GL_FRAMEBUFFER, targets[0]->isDepthAttachment ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, targets[0]->impl._texture,
		                     0); // Layered
#endif
	glViewport(0, 0, targets[0]->width, targets[0]->height);
	_renderTargetWidth = targets[0]->width;
	_renderTargetHeight = targets[0]->height;
	renderToBackbuffer = false;
	glCheckErrors();

	if (count > 1) {
		for (int i = 0; i < count; ++i) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, targets[i]->impl._texture, 0);
			glCheckErrors();
		}

		GLenum buffers[16];
		for (int i = 0; i < count; ++i)
			buffers[i] = GL_COLOR_ATTACHMENT0 + i;
#if defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18
		((void (*)(GLsizei, GLenum *))glesDrawBuffers)(count, buffers);
#elif !defined(KINC_OPENGL_ES) || defined(KINC_EMSCRIPTEN) || defined(KINC_WASM)
		glDrawBuffers(count, buffers);
#endif
		glCheckErrors();
	}

	for (int i = count; i < maxColorAttachments; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
		glCheckErrors();
	}
}

void kinc_g4_set_render_target_face(kinc_g4_render_target_t *texture, int face) {
	glBindFramebuffer(GL_FRAMEBUFFER, texture->impl._framebuffer);
	glCheckErrors();
	glFramebufferTexture2D(GL_FRAMEBUFFER, texture->isDepthAttachment ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
	                       texture->impl._texture, 0);
	glViewport(0, 0, texture->width, texture->height);
	_renderTargetWidth = texture->width;
	_renderTargetHeight = texture->height;
	renderToBackbuffer = false;
	glCheckErrors();
}

void kinc_g4_restore_render_target() {
#ifdef KINC_IOS
	glBindFramebuffer(GL_FRAMEBUFFER, kinc_ios_gl_framebuffer);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
#endif
	glCheckErrors();
	int w = kinc_window_width(currentWindow);
	int h = kinc_window_height(currentWindow);
	glViewport(0, 0, w, h);
	_renderTargetWidth = w;
	_renderTargetHeight = h;
	renderToBackbuffer = true;
	glCheckErrors();
#ifdef KINC_WINDOWS
	Kinc_Internal_setWindowRenderTarget(currentWindow);
#endif
}

void kinc_g4_flush() {
	glFlush();
	glCheckErrors();
}

void kinc_g4_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_internal_set_pipeline(pipeline);
	lastPipeline = pipeline;
}

void kinc_g4_set_blend_constant(float r, float g, float b, float a) {
	glBlendColor(r, g, b, a);
}

void kinc_g4_set_stencil_reference_value(int value) {
	glStencilFuncSeparate(GL_FRONT, Kinc_G4_Internal_StencilFunc(lastPipeline->stencil_front_mode), value, lastPipeline->stencil_read_mask);
	glStencilFuncSeparate(GL_BACK, Kinc_G4_Internal_StencilFunc(lastPipeline->stencil_back_mode), value, lastPipeline->stencil_read_mask);
}

int Kinc_G4_Internal_StencilFunc(kinc_g4_compare_mode_t mode) {
	switch (mode) {
	case KINC_G4_COMPARE_ALWAYS:
		return GL_ALWAYS;
	case KINC_G4_COMPARE_EQUAL:
		return GL_EQUAL;
	case KINC_G4_COMPARE_GREATER:
		return GL_GREATER;
	case KINC_G4_COMPARE_GREATER_EQUAL:
		return GL_GEQUAL;
	case KINC_G4_COMPARE_LESS:
		return GL_LESS;
	case KINC_G4_COMPARE_LESS_EQUAL:
		return GL_LEQUAL;
	case KINC_G4_COMPARE_NEVER:
		return GL_NEVER;
	case KINC_G4_COMPARE_NOT_EQUAL:
		return GL_NOTEQUAL;
	}

	return 0;
}

extern bool kinc_internal_gl_has_compute;

bool kinc_g4_supports_instanced_rendering() {
#if defined(KINC_OPENGL_ES) && defined(KINC_ANDROID)
#if KINC_ANDROID_API >= 18
	return glesDrawElementsInstanced != NULL;
#else
	return false;
#endif
#else
	return true;
#endif
}

bool kinc_g4_supports_compute_shaders() {
	return kinc_internal_gl_has_compute;
}

bool kinc_g4_supports_blend_constants() {
	return true;
}

bool kinc_g4_supports_non_pow2_textures() {
	// we use OpenGL 2.0+, which should supports NPOT textures.
	// in practice certain very old hardware doesn't,
	// but detecting that is not practical
	return true;
}

bool kinc_g4_render_targets_inverted_y(void) {
	return true;
}
