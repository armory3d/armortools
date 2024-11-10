#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics5/rendertarget.h>
#include <kinc/log.h>

void kinc_g5_render_target_init_with_multisampling(kinc_g5_render_target_t *renderTarget, int width, int height, kinc_g5_render_target_format_t format,
                                                   int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	renderTarget->texWidth = renderTarget->width = width;
	renderTarget->texHeight = renderTarget->height = height;
	renderTarget->framebuffer_index = -1;
	kinc_g4_render_target_init_with_multisampling(&renderTarget->impl.target, width, height, (kinc_g4_render_target_format_t)format, depthBufferBits,
	                                              stencilBufferBits, samples_per_pixel);
}

void kinc_g5_render_target_init_framebuffer_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                               int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	target->framebuffer_index = 0;
	target->width = target->texWidth = width;
	target->height = target->texHeight = height;
}

void kinc_g5_render_target_init_cube_with_multisampling(kinc_g5_render_target_t *target, int cubeMapSize, kinc_g5_render_target_format_t format,
                                                        int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	target->texWidth = target->width = cubeMapSize;
	target->texHeight = target->height = cubeMapSize;
	target->isCubeMap = true;
	target->framebuffer_index = -1;
	kinc_g4_render_target_init_cube_with_multisampling(&target->impl.target, cubeMapSize, (kinc_g4_render_target_format_t)format, depthBufferBits,
	                                                   stencilBufferBits, samples_per_pixel);
}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *renderTarget) {
	kinc_g4_render_target_destroy(&renderTarget->impl.target);
}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *renderTarget, kinc_g5_render_target_t *source) {
	kinc_g4_render_target_set_depth_stencil_from(&renderTarget->impl.target, &source->impl.target);
}

// void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *renderTarget, uint8_t *data) {
//     kinc_g4_render_target_get_pixels(&renderTarget->impl, data);
// }

// void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *renderTarget, int levels) {
//     kinc_g4_render_target_generate_mipmaps(&renderTarget->impl, levels);
// }
