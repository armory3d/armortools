#include "rendertarget.h"

void kinc_g4_render_target_init(kinc_g4_render_target_t *renderTarget, int width, int height, kinc_g4_render_target_format_t format, int depthBufferBits) {
	kinc_g4_render_target_init_with_multisampling(renderTarget, width, height, format, depthBufferBits, 1);
}

void kinc_g4_render_target_init_cube(kinc_g4_render_target_t *renderTarget, int cubeMapSize, kinc_g4_render_target_format_t format, int depthBufferBits) {
	kinc_g4_render_target_init_cube_with_multisampling(renderTarget, cubeMapSize, format, depthBufferBits, 1);
}