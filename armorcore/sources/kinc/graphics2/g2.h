
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
#include "iron_array.h"

typedef struct image {
	void *texture_;
	void *render_target_;
	int format; // tex_format_t;
	bool readable;
	buffer_t *pixels;
	int width;
	int height;
	int depth;
} image_t;

typedef struct kinc_g2_font_image kinc_g2_font_image_t;

typedef struct kinc_g2_font {
	unsigned char *blob;
	kinc_g2_font_image_t *images;
	size_t m_capacity;
	size_t m_images_len;
	int offset;
} kinc_g2_font_t;

typedef struct g2_font {
	kinc_g2_font_t *font_;
	void *blob; // buffer_t
	void *glyphs; // i32_array_t
	int index;
} g2_font_t;

void kinc_g2_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *colored_vert, buffer_t *colored_frag, buffer_t *text_vert, buffer_t *text_frag);
void kinc_g2_begin(void);
void kinc_g2_draw_scaled_sub_image(image_t *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void kinc_g2_draw_scaled_sub_texture(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void kinc_g2_draw_scaled_image(kinc_g4_texture_t *tex, float dx, float dy, float dw, float dh);
void kinc_g2_draw_sub_image(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y);
void kinc_g2_draw_image(kinc_g4_texture_t *tex, float x, float y);
void kinc_g2_draw_scaled_sub_render_target(kinc_g4_render_target_t *rt, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void kinc_g2_draw_scaled_render_target(kinc_g4_render_target_t *rt, float dx, float dy, float dw, float dh);
void kinc_g2_draw_sub_render_target(kinc_g4_render_target_t *rt, float sx, float sy, float sw, float sh, float x, float y);
void kinc_g2_draw_render_target(kinc_g4_render_target_t *rt, float x, float y);
void kinc_g2_fill_triangle(float x0, float y0, float x1, float y1, float x2, float y2);
void kinc_g2_fill_rect(float x, float y, float width, float height);
void kinc_g2_draw_rect(float x, float y, float width, float height, float strength);
void kinc_g2_draw_line(float x0, float y0, float x1, float y1, float strength);
void kinc_g2_draw_line_aa(float x0, float y0, float x1, float y1, float strength);
void kinc_g2_draw_string(const char *text, float x, float y);
void kinc_g2_end(void);
void kinc_g2_set_color(uint32_t color);
uint32_t kinc_g2_get_color();
void kinc_g2_set_pipeline(kinc_g4_pipeline_t *pipeline);
void kinc_g2_set_transform(buffer_t *matrix);
bool kinc_g2_set_font(kinc_g2_font_t *font, int size);
kinc_g2_font_t *kinc_g2_font_init(buffer_t *blob, int font_index);
kinc_g2_font_t *kinc_g2_font_13(buffer_t *blob);
bool kinc_g2_font_has_glyph(int glyph);
void kinc_g2_font_set_glyphs(i32_array_t *glyphs);
void kinc_g2_font_add_glyph(int glyph);
int kinc_g2_font_count(kinc_g2_font_t *font);
int kinc_g2_font_height(kinc_g2_font_t *font, int font_size);
float kinc_g2_sub_string_width(kinc_g2_font_t *font, int font_size, const char *text, int start, int end);
int kinc_g2_string_width(kinc_g2_font_t *font, int font_size, const char *text);
void kinc_g2_set_bilinear_filter(bool bilinear);
void kinc_g2_restore_render_target(void);
void kinc_g2_set_render_target(kinc_g4_render_target_t *target);
