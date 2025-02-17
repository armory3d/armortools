#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/log.h>

extern kinc_g5_command_list_t commandList;

void kinc_g4_render_target_get_pixels(kinc_g5_render_target_t *render_target, uint8_t *data) {
	kinc_g5_command_list_get_render_target_pixels(&commandList, render_target, data);
}

void kinc_g5_render_target_generate_mipmaps(kinc_g5_render_target_t *render_target, int levels) {}
