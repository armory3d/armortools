
// This implementation was adapted from tizilogic's krink and robdangerous' kha, licensed under zlib/libpng license
// https://github.com/tizilogic/krink/blob/main/Sources/krink/graphics2/graphics.h
// https://github.com/Kode/Kha/tree/main/Sources/kha/graphics2

#pragma once

#include <iron_math.h>
#include <iron_gpu.h>
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

typedef struct draw_font_image draw_font_image_t;

typedef struct draw_font {
	unsigned char *blob;
	draw_font_image_t *images;
	size_t m_capacity;
	size_t m_images_len;
	int offset;
} draw_font_t;

typedef struct g2_font {
	draw_font_t *font_;
	void *blob; // buffer_t
	void *glyphs; // i32_array_t
	int index;
} g2_font_t;

void draw_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *colored_vert, buffer_t *colored_frag, buffer_t *text_vert, buffer_t *text_frag);
void draw_begin(void);
void draw_scaled_sub_image(image_t *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void draw_scaled_sub_texture(kinc_g5_texture_t *tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void draw_scaled_image(kinc_g5_texture_t *tex, float dx, float dy, float dw, float dh);
void draw_sub_image(kinc_g5_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y);
void draw_image(kinc_g5_texture_t *tex, float x, float y);
void draw_scaled_sub_render_target(kinc_g5_render_target_t *rt, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void draw_scaled_render_target(kinc_g5_render_target_t *rt, float dx, float dy, float dw, float dh);
void draw_sub_render_target(kinc_g5_render_target_t *rt, float sx, float sy, float sw, float sh, float x, float y);
void draw_render_target(kinc_g5_render_target_t *rt, float x, float y);
void draw_filled_triangle(float x0, float y0, float x1, float y1, float x2, float y2);
void draw_filled_rect(float x, float y, float width, float height);
void draw_rect(float x, float y, float width, float height, float strength);
void draw_line(float x0, float y0, float x1, float y1, float strength);
void draw_line_aa(float x0, float y0, float x1, float y1, float strength);
void draw_string(const char *text, float x, float y);
void draw_end(void);
void draw_set_color(uint32_t color);
uint32_t draw_get_color();
void draw_set_pipeline(kinc_g5_pipeline_t *pipeline);
void draw_set_transform(buffer_t *matrix);
bool draw_set_font(draw_font_t *font, int size);
draw_font_t *draw_font_init(buffer_t *blob, int font_index);
draw_font_t *draw_font_13(buffer_t *blob);
bool draw_font_has_glyph(int glyph);
void draw_font_set_glyphs(i32_array_t *glyphs);
void draw_font_add_glyph(int glyph);
int draw_font_count(draw_font_t *font);
int draw_font_height(draw_font_t *font, int font_size);
float draw_sub_string_width(draw_font_t *font, int font_size, const char *text, int start, int end);
int draw_string_width(draw_font_t *font, int font_size, const char *text);
void draw_set_bilinear_filter(bool bilinear);
void draw_restore_render_target(void);
void draw_set_render_target(kinc_g5_render_target_t *target);

void draw_filled_circle(float cx, float cy, float radius, int segments);
void draw_circle(float cx, float cy, float radius, int segments, float strength);
void draw_cubic_bezier(f32_array_t *x, f32_array_t *y, int segments, float strength);
