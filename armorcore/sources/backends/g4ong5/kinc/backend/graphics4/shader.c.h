#include <kinc/graphics4/shader.h>

void kinc_g4_shader_init(kinc_g4_shader_t *shader, const void *_data, size_t length, kinc_g4_shader_type_t type) {
	kinc_g5_shader_init(&shader->impl._shader, _data, length, (kinc_g5_shader_type_t)type);
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

#ifdef KINC_VULKAN
	const char *target = "spirv";
#elif defined(KINC_METAL)
	const char *target = "metal";
#else
	const char *target = "d3d11";
#endif

	int errors = krafix_compile(source, output, &length, target, system, type == KINC_G4_SHADER_TYPE_FRAGMENT ? "frag" : "vert", -1);
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
	kinc_g5_shader_destroy(&shader->impl._shader);
}
