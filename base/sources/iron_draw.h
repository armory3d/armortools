
// This implementation was adapted from tizilogic's krink and robdangerous' kha, licensed under zlib/libpng license
// https://github.com/tizilogic/krink/blob/main/Sources/krink/graphics2/graphics.h
// https://github.com/Kode/Kha/tree/main/Sources/kha/graphics2

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "iron_math.h"
#include "iron_gpu.h"
#include "iron_array.h"
#include "iron_mat3.h"

typedef struct draw_font_image draw_font_image_t;

typedef struct draw_font {
	unsigned char *blob;
	draw_font_image_t *images;
	size_t m_capacity;
	size_t m_images_len;
	int offset;

	buffer_t *buf;
	int glyphs_version;
	int index;
} draw_font_t;

void draw_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *rect_vert, buffer_t *rect_frag, buffer_t *tris_vert, buffer_t *tris_frag, buffer_t *text_vert, buffer_t *text_frag);
void draw_begin(gpu_texture_t *target, bool clear, unsigned color);
void draw_scaled_sub_image(gpu_texture_t *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void draw_scaled_image(gpu_texture_t *tex, float dx, float dy, float dw, float dh);
void draw_sub_image(gpu_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y);
void draw_image(gpu_texture_t *tex, float x, float y);
void draw_filled_triangle(float x0, float y0, float x1, float y1, float x2, float y2);
void draw_filled_rect(float x, float y, float width, float height);
void draw_rect(float x, float y, float width, float height, float strength);
void draw_line(float x0, float y0, float x1, float y1, float strength);
void draw_line_aa(float x0, float y0, float x1, float y1, float strength);
void draw_string(const char *text, float x, float y);
void draw_end(void);
void draw_set_color(uint32_t color);
uint32_t draw_get_color();
void draw_set_pipeline(gpu_pipeline_t *pipeline);
void draw_set_transform(mat3_t matrix);
bool draw_set_font(draw_font_t *font, int size);
void draw_font_init(draw_font_t *font);
void draw_font_destroy(draw_font_t *font);
void draw_font_13(draw_font_t *font);
bool draw_font_has_glyph(int glyph);
void draw_font_add_glyph(int glyph);
void draw_font_init_glyphs(int from, int to);
int draw_font_count(draw_font_t *font);
int draw_font_height(draw_font_t *font, int font_size);
float draw_sub_string_width(draw_font_t *font, int font_size, const char *text, int start, int end);
int draw_string_width(draw_font_t *font, int font_size, const char *text);
void draw_set_bilinear_filter(bool bilinear);

void draw_filled_circle(float cx, float cy, float radius, int segments);
void draw_circle(float cx, float cy, float radius, int segments, float strength);
void draw_cubic_bezier(f32_array_t *x, f32_array_t *y, int segments, float strength);

extern int draw_font_size;
extern draw_font_t *draw_font;
extern gpu_texture_t *_draw_current;
