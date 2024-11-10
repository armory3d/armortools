#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/math/core.h>

// extern kinc_g5_pipeline_t *currentProgram;

void kinc_g5_internal_destroy_window(int window) {
	kinc_g4_internal_destroy_window(window);
}

void kinc_g5_internal_destroy() {
	kinc_g4_internal_destroy();
}

void kinc_g5_internal_init() {
	kinc_g4_internal_init();
}

void kinc_g5_internal_init_window(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	kinc_g4_internal_init_window(window, depthBufferBits, stencilBufferBits, vsync);
}

// void kinc_g5_draw_indexed_vertices_instanced(int instanceCount) {}

// void kinc_g5_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {}

// void kinc_g5_draw_indexed_vertices_instanced_from_to_from(int instanceCount, int start, int count, int vertex_offset) {}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {}

void kinc_g5_end(int window) {}

bool kinc_g5_swap_buffers() {
	return kinc_g4_swap_buffers();
}

void kinc_g5_flush() {}

int kinc_g5_max_bound_textures(void) {
	return kinc_g4_max_bound_textures();
}

bool kinc_g5_supports_raytracing() {
	return false;
}

bool kinc_g5_supports_instanced_rendering() {
	return kinc_g4_supports_instanced_rendering();
}

bool kinc_g5_supports_compute_shaders() {
	return kinc_g4_supports_compute_shaders();
}

bool kinc_g5_supports_blend_constants() {
	return kinc_g4_supports_blend_constants();
}

bool kinc_g5_supports_non_pow2_textures() {
	return kinc_g4_supports_non_pow2_textures();
}

bool kinc_g5_render_targets_inverted_y() {
	return kinc_g4_render_targets_inverted_y();
}