#include "rendertarget.h"

void kinc_g5_render_target_init(kinc_g5_render_target_t *render_target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits) {
	kinc_g5_render_target_init_with_multisampling(render_target, width, height, format, depthBufferBits, 1);
}

void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *render_target, int width, int height, kinc_g5_render_target_format_t format,
                                            int depthBufferBits) {
	kinc_g5_render_target_init_framebuffer_with_multisampling(render_target, width, height, format, depthBufferBits, 1);
}

void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *render_target, int cubeMapSize, kinc_g5_render_target_format_t format, int depthBufferBits) {
	kinc_g5_render_target_init_cube_with_multisampling(render_target, cubeMapSize, format, depthBufferBits, 1);
}
