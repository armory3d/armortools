#include <kinc/graphics5/rendertarget.h>
#include <kinc/log.h>

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits) {

}

void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits) {}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *renderTarget) {}

void kinc_g5_render_target_set_depth_from(kinc_g5_render_target_t *renderTarget, kinc_g5_render_target_t *source) {}

void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *renderTarget, uint8_t *data) {}

void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *renderTarget, int levels) {}
