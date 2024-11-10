#include "texture.h"

#include <kinc/graphics5/texture.h>
#include <kinc/log.h>

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	kinc_g4_texture_init(&texture->impl.texture, width, height, format);
}
void kinc_g5_texture_init3d(kinc_g5_texture_t *texture, int width, int height, int depth, kinc_image_format_t format) {
	kinc_g4_texture_init3d(&texture->impl.texture, width, height, depth, format);
}
void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image) {
	kinc_g4_texture_init_from_image(&texture->impl.texture, image);
}
void kinc_g5_texture_init_from_encoded_data(kinc_g5_texture_t *texture, void *data, int size, const char *format, bool readable) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "kinc_g5_texture_init_from_encoded_data not implemented");
}
void kinc_g5_texture_init_from_data(kinc_g5_texture_t *texture, void *data, int width, int height, int format, bool readable) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "kinc_g5_texture_init_from_data not implemented");
}
void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "kinc_g5_texture_init_non_sampled_access not implemented");
}
void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {
	kinc_g4_texture_destroy(&texture->impl.texture);
}

uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture) {
	return kinc_g4_texture_lock(&texture->impl.texture);
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *texture) {
	kinc_g4_texture_unlock(&texture->impl.texture);
}

void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {
	kinc_g4_texture_clear(&texture->impl.texture, x, y, z, width, height, depth, color);
}

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	return kinc_g4_texture_stride(&texture->impl.texture);
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {
	kinc_g4_texture_generate_mipmaps(&texture->impl.texture, levels);
}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_image_t *mipmap, int level) {
	kinc_g4_texture_set_mipmap(&texture->impl.texture, mipmap, level);
}
