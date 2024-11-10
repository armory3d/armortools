
// This implementation was adapted from tizilogic's krink and robdangerous' kha, licensed under zlib/libpng license
// https://github.com/tizilogic/krink/blob/main/Sources/krink/graphics2/graphics.h
// https://github.com/Kode/Kha/tree/main/Sources/kha/graphics2

#pragma once

#include <kinc/math/matrix.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/pipeline.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct arm_g2_font_image arm_g2_font_image_t;

typedef struct arm_g2_font {
	unsigned char *blob;
	arm_g2_font_image_t *images;
	size_t m_capacity;
	size_t m_images_len;
	int offset;
} arm_g2_font_t;

typedef struct g2_font {
	arm_g2_font_t *font_;
	void *blob; // buffer_t
	void *glyphs; // i32_array_t
	int index;
} g2_font_t;

void arm_g2_init(void *image_vert, int image_vert_size, void *image_frag, int image_frag_size, void *colored_vert, int colored_vert_size, void *colored_frag, int colored_frag_size, void *text_vert, int text_vert_size, void *text_frag, int text_frag_size);
void arm_g2_begin(void);
void arm_g2_draw_scaled_sub_image(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void arm_g2_draw_scaled_image(kinc_g4_texture_t *tex, float dx, float dy, float dw, float dh);
void arm_g2_draw_sub_image(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y);
void arm_g2_draw_image(kinc_g4_texture_t *tex, float x, float y);
void arm_g2_draw_scaled_sub_render_target(kinc_g4_render_target_t *rt, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void arm_g2_draw_scaled_render_target(kinc_g4_render_target_t *rt, float dx, float dy, float dw, float dh);
void arm_g2_draw_sub_render_target(kinc_g4_render_target_t *rt, float sx, float sy, float sw, float sh, float x, float y);
void arm_g2_draw_render_target(kinc_g4_render_target_t *rt, float x, float y);
void arm_g2_fill_triangle(float x0, float y0, float x1, float y1, float x2, float y2);
void arm_g2_fill_rect(float x, float y, float width, float height);
void arm_g2_draw_rect(float x, float y, float width, float height, float strength);
void arm_g2_draw_line(float x0, float y0, float x1, float y1, float strength);
void arm_g2_draw_string(const char *text, float x, float y);
void arm_g2_end(void);
void arm_g2_set_color(uint32_t color);
uint32_t arm_g2_get_color();
void arm_g2_set_pipeline(kinc_g4_pipeline_t *pipeline);
void arm_g2_set_transform(kinc_matrix3x3_t *m);
void arm_g2_set_font(arm_g2_font_t *font, int size);
void arm_g2_font_init(arm_g2_font_t *font, void *blob, int font_index);
void arm_g2_font_13(arm_g2_font_t *font, void *blob);
bool arm_g2_font_has_glyph(int glyph);
void arm_g2_font_set_glyphs(int *glyphs, int count);
void arm_g2_font_add_glyph(int glyph);
int arm_g2_font_count(arm_g2_font_t *font);
float arm_g2_font_height(arm_g2_font_t *font, int font_size);
float arm_g2_sub_string_width(arm_g2_font_t *font, int font_size, const char *text, int start, int end);
float arm_g2_string_width(arm_g2_font_t *font, int font_size, const char *text);
void arm_g2_set_bilinear_filter(bool bilinear);
void arm_g2_restore_render_target(void);
void arm_g2_set_render_target(kinc_g4_render_target_t *target);
