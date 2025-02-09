#include "rendertarget.h"

void kinc_g4_render_target_init(kinc_g4_render_target_t *renderTarget, int width, int height, kinc_g4_render_target_format_t format, int depthBufferBits) {
	kinc_g4_render_target_init_with_multisampling(renderTarget, width, height, format, depthBufferBits, 1);
}
