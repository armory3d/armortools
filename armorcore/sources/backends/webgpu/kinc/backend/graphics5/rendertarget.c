#include <kinc/graphics5/rendertarget.h>
#include <kinc/log.h>

void kinc_g5_render_target_init_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                   int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	
}

void kinc_g5_render_target_init_framebuffer_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                               int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {}

void kinc_g5_render_target_init_cube_with_multisampling(kinc_g5_render_target_t *render_target, int cubeMapSize, kinc_g5_render_target_format_t format,
                                                        int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *renderTarget) {}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *renderTarget, kinc_g5_render_target_t *source) {}

void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *renderTarget, uint8_t *data) {}

void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *renderTarget, int levels) {}
