#include "ogl.h"

#include <kinc/graphics4/shader.h>

#include <stdlib.h>
#include <string.h>

void kinc_g4_shader_init(kinc_g4_shader_t *shader, const void *data, size_t length, kinc_g4_shader_type_t type) {
	shader->impl.length = length;
	shader->impl._glid = 0;
	char *source = (char *)malloc(length + 1);
	memcpy(source, data, length);
	source[length] = 0;
	shader->impl.source = source;
}

#ifdef KRAFIX_LIBRARY
extern int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
#endif

int kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type) {
#ifdef KRAFIX_LIBRARY
	char *output = malloc(1024 * 1024);
	int length;
#ifdef KINC_WINDOWS
	const char *system = "windows";
#elif defined(KINC_MACOS)
	const char *system = "macos";
#elif defined(KINC_LINUX)
	const char *system = "linux";
#elif defined(KINC_ANDROID)
	const char *system = "android";
#elif defined(KINC_IOS)
	const char *system = "ios";
#endif
	int errors = krafix_compile(source, output, &length, "glsl", system, type == KINC_G4_SHADER_TYPE_FRAGMENT ? "frag" : "vert", -1);
	if (errors > 0) {
		return errors;
	}
	kinc_g4_shader_init(shader, output, length, type);
	return 0;
#else
	return 0;
#endif
}

void kinc_g4_shader_destroy(kinc_g4_shader_t *shader) {
	free((void *)shader->impl.source);
	shader->impl.source = NULL;
	if (shader->impl._glid != 0) {
		glDeleteShader(shader->impl._glid);
	}
}
