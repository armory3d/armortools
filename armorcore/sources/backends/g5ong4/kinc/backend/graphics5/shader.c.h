#include <kinc/graphics4/shader.h>
#include <kinc/graphics5/shader.h>

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type) {
	kinc_g4_shader_init(&shader->impl.shader, source, length, (kinc_g4_shader_type_t)type);
}

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {
	kinc_g4_shader_destroy(&shader->impl.shader);
}
