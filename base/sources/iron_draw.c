#include "iron_draw.h"
#include "const_data.h"
#include "iron_file.h"
#include "iron_gc.h"
#include "iron_gpu.h"
#include "iron_math.h"
#include "iron_simd.h"
#include "iron_string.h"
#include "iron_system.h"
#include <math.h>
#include <string.h>

draw_font_t   *draw_font = NULL;
int            draw_font_size;
gpu_texture_t *_draw_current = NULL;

static mat3_t          draw_transform;
static uint32_t        draw_color           = 0;
static gpu_pipeline_t *draw_custom_pipeline = NULL;

static gpu_buffer_t           rect_vertex_buffer;
static gpu_buffer_t           rect_index_buffer;
static gpu_buffer_t           tris_vertex_buffer;
static gpu_buffer_t           tris_index_buffer;
static gpu_vertex_structure_t draw_structure;

static gpu_pipeline_t image_pipeline;
static gpu_pipeline_t image_transform_pipeline;
static gpu_pipeline_t image_r8_pipeline;
static int            image_tex_unit;
static int            image_pos_loc;
static int            image_tex_loc;
static int            image_col_loc;
static int            image_transform_w_loc;

static gpu_pipeline_t rect_pipeline;
static int            rect_pos_loc;
static int            rect_col_loc;

static gpu_pipeline_t tris_pipeline;
static int            tris_pos0_loc;
static int            tris_pos1_loc;
static int            tris_pos2_loc;
static int            tris_col_loc;

static gpu_pipeline_t text_pipeline;
static gpu_pipeline_t text_rt_pipeline;
static int            text_tex_unit;
static int            text_pos_loc;
static int            text_tex_loc;
static int            text_col_loc;

static gpu_pipeline_t slug_pipeline;
static gpu_pipeline_t slug_rt_pipeline;
static int            slug_curve_tex_unit;
static int            slug_band_tex_unit;
static int            slug_pos_loc;
static int            slug_col_loc;
static int            slug_em_rect_loc;
static int            slug_glyph_loc_loc;
static int            slug_band_max_loc;
static int            slug_band_tf_loc;

static i32_array_t *draw_font_glyph_blocks = NULL;
static i32_array_t *draw_font_glyphs       = NULL;
static int          draw_glyphs_version    = 0;

static uint8_t _draw_color_r(uint32_t color) {
	return (color & 0x00ff0000) >> 16;
}

static uint8_t _draw_color_g(uint32_t color) {
	return (color & 0x0000ff00) >> 8;
}

static uint8_t _draw_color_b(uint32_t color) {
	return (color & 0x000000ff);
}

static uint8_t _draw_color_a(uint32_t color) {
	return (color) >> 24;
}

static uint32_t _draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (a << 24) | (r << 16) | (g << 8) | b;
}

static float vw() {
	return _draw_current != NULL ? _draw_current->width : iron_window_width();
}

static float vh() {
	return _draw_current != NULL ? _draw_current->height : iron_window_height();
}

static void draw_pipeline_init(gpu_pipeline_t *pipe, gpu_shader_t *vert, gpu_shader_t *frag) {
	gpu_pipeline_init(pipe);
	pipe->input_layout            = &draw_structure;
	pipe->blend_source            = GPU_BLEND_ONE;
	pipe->blend_destination       = GPU_BLEND_INV_SOURCE_ALPHA;
	pipe->alpha_blend_source      = GPU_BLEND_ONE;
	pipe->alpha_blend_destination = GPU_BLEND_INV_SOURCE_ALPHA;
	pipe->vertex_shader           = vert;
	pipe->fragment_shader         = frag;
}

void draw_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *image_transform_vert, buffer_t *image_transform_frag, buffer_t *rect_vert,
               buffer_t *rect_frag, buffer_t *tris_vert, buffer_t *tris_frag, buffer_t *text_vert, buffer_t *text_frag, buffer_t *slug_vert,
               buffer_t *slug_frag) {
	draw_transform      = mat3_nan();
	draw_structure.size = 0;
	gpu_vertex_structure_add(&draw_structure, "pos", GPU_VERTEX_DATA_F32_2X);

	{
		gpu_vertex_buffer_init(&rect_vertex_buffer, 4, &draw_structure);
		float *verts = gpu_vertex_buffer_lock(&rect_vertex_buffer);
		verts[0]     = 0.0; // Bottom-left
		verts[1]     = 1.0;
		verts[2]     = 0.0; // Top-left
		verts[3]     = 0.0;
		verts[4]     = 1.0; // Top-right
		verts[5]     = 0.0;
		verts[6]     = 1.0; // Bottom-right
		verts[7]     = 1.0;
		gpu_vertex_buffer_unlock(&rect_vertex_buffer);

		gpu_index_buffer_init(&rect_index_buffer, 3 * 2);
		int *indices = gpu_index_buffer_lock(&rect_index_buffer);
		indices[0]   = 0;
		indices[1]   = 1;
		indices[2]   = 2;
		indices[3]   = 0;
		indices[4]   = 2;
		indices[5]   = 3;
		gpu_index_buffer_unlock(&rect_index_buffer);
	}

	{
		gpu_vertex_buffer_init(&tris_vertex_buffer, 3, &draw_structure);
		float *verts = gpu_vertex_buffer_lock(&tris_vertex_buffer);
		verts[0]     = 0.0; // Bottom-left
		verts[1]     = 1.0;
		verts[2]     = 0.0; // Top-left
		verts[3]     = 0.0;
		verts[4]     = 1.0; // Top-right
		verts[5]     = 0.0;
		gpu_vertex_buffer_unlock(&tris_vertex_buffer);

		gpu_index_buffer_init(&tris_index_buffer, 3);
		int *indices = gpu_index_buffer_lock(&tris_index_buffer);
		indices[0]   = 0;
		indices[1]   = 1;
		indices[2]   = 2;
		gpu_index_buffer_unlock(&tris_index_buffer);
	}

	gpu_shader_t vert_shader;
	gpu_shader_t frag_shader;

	// Image painter
	{
		gpu_shader_init(&vert_shader, image_vert->buffer, image_vert->length, GPU_SHADER_TYPE_VERTEX);
		gpu_shader_init(&frag_shader, image_frag->buffer, image_frag->length, GPU_SHADER_TYPE_FRAGMENT);
		draw_pipeline_init(&image_pipeline, &vert_shader, &frag_shader);
		gpu_pipeline_compile(&image_pipeline);
		image_tex_unit = 0;
		image_pos_loc  = 0;
		image_tex_loc  = 16;
		image_col_loc  = 32;

		draw_pipeline_init(&image_r8_pipeline, &vert_shader, &frag_shader);
		image_r8_pipeline.color_attachment[0] = GPU_TEXTURE_FORMAT_R8;
		gpu_pipeline_compile(&image_r8_pipeline);

		gpu_shader_init(&vert_shader, image_transform_vert->buffer, image_transform_vert->length, GPU_SHADER_TYPE_VERTEX);
		gpu_shader_init(&frag_shader, image_transform_frag->buffer, image_transform_frag->length, GPU_SHADER_TYPE_FRAGMENT);
		draw_pipeline_init(&image_transform_pipeline, &vert_shader, &frag_shader);
		gpu_pipeline_compile(&image_transform_pipeline);
		image_transform_w_loc = 48;
	}

	// Rect painter
	{
		gpu_shader_init(&vert_shader, rect_vert->buffer, rect_vert->length, GPU_SHADER_TYPE_VERTEX);
		gpu_shader_init(&frag_shader, rect_frag->buffer, rect_frag->length, GPU_SHADER_TYPE_FRAGMENT);
		draw_pipeline_init(&rect_pipeline, &vert_shader, &frag_shader);
		rect_pipeline.blend_source = GPU_BLEND_SOURCE_ALPHA;
		gpu_pipeline_compile(&rect_pipeline);
		rect_pos_loc = 0;
		rect_col_loc = 16;
	}

	// Tris painter
	{
		gpu_shader_init(&vert_shader, tris_vert->buffer, tris_vert->length, GPU_SHADER_TYPE_VERTEX);
		gpu_shader_init(&frag_shader, tris_frag->buffer, tris_frag->length, GPU_SHADER_TYPE_FRAGMENT);
		draw_pipeline_init(&tris_pipeline, &vert_shader, &frag_shader);
		tris_pipeline.blend_source = GPU_BLEND_SOURCE_ALPHA;
		gpu_pipeline_compile(&tris_pipeline);
		tris_pos0_loc = 0;
		tris_pos1_loc = 8;
		tris_pos2_loc = 16;
		tris_col_loc  = 32; // 16 align
	}

	// Text painter
	{
		gpu_shader_init(&vert_shader, text_vert->buffer, text_vert->length, GPU_SHADER_TYPE_VERTEX);
		gpu_shader_init(&frag_shader, text_frag->buffer, text_frag->length, GPU_SHADER_TYPE_FRAGMENT);
		draw_pipeline_init(&text_pipeline, &vert_shader, &frag_shader);
		text_pipeline.blend_source = GPU_BLEND_SOURCE_ALPHA;
		gpu_pipeline_compile(&text_pipeline);
		text_tex_unit = 0;
		text_pos_loc  = 0;
		text_tex_loc  = 16;
		text_col_loc  = 32;

		draw_pipeline_init(&text_rt_pipeline, &vert_shader, &frag_shader);
		text_rt_pipeline.blend_source = GPU_BLEND_SOURCE_ALPHA;
		gpu_pipeline_compile(&text_rt_pipeline);
	}

	// Slug painter
	{
		gpu_shader_init(&vert_shader, slug_vert->buffer, slug_vert->length, GPU_SHADER_TYPE_VERTEX);
		gpu_shader_init(&frag_shader, slug_frag->buffer, slug_frag->length, GPU_SHADER_TYPE_FRAGMENT);
		draw_pipeline_init(&slug_pipeline, &vert_shader, &frag_shader);
		slug_pipeline.blend_source = GPU_BLEND_SOURCE_ALPHA;
		gpu_pipeline_compile(&slug_pipeline);
		slug_curve_tex_unit = 0;
		slug_band_tex_unit  = 1;
		slug_pos_loc        = 0;
		slug_col_loc        = 16;
		slug_em_rect_loc    = 32;
		slug_glyph_loc_loc  = 48;
		slug_band_max_loc   = 64;
		slug_band_tf_loc    = 80;

		draw_pipeline_init(&slug_rt_pipeline, &vert_shader, &frag_shader);
		slug_rt_pipeline.blend_source = GPU_BLEND_SOURCE_ALPHA;
		gpu_pipeline_compile(&slug_rt_pipeline);
	}
}

void draw_begin(gpu_texture_t *target, bool clear, unsigned color) {
	draw_set_color(0xffffffff);
	_draw_current = target;
	if (target == NULL) {
		gpu_begin(NULL, 0, NULL, clear ? GPU_CLEAR_COLOR : GPU_CLEAR_NONE, color, 0.0);
	}
	else {
		gpu_texture_t *targets[1] = {target};
		gpu_begin(targets, 1, NULL, clear ? GPU_CLEAR_COLOR : GPU_CLEAR_NONE, color, 0.0);
	}
}

void draw_end(void) {
	gpu_end();
}

void draw_scaled_sub_image(gpu_texture_t *tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	if (mat3_isnan(draw_transform)) {
		gpu_set_pipeline(draw_custom_pipeline != NULL                                                ? draw_custom_pipeline
		                 : (_draw_current != NULL && _draw_current->format == GPU_TEXTURE_FORMAT_R8) ? &image_r8_pipeline
		                                                                                             : &image_pipeline);
	}
	else {
		gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &image_transform_pipeline);
		gpu_set_mat3(image_transform_w_loc, draw_transform);
	}
	gpu_set_vertex_buffer(&rect_vertex_buffer);
	gpu_set_index_buffer(&rect_index_buffer);
	gpu_set_float4(image_pos_loc, (int)dx / vw(), (int)dy / vh(), (int)dw / vw(), (int)dh / vh());
	gpu_set_float4(image_tex_loc, sx / tex->width, sy / tex->height, sw / tex->width, sh / tex->height);
	gpu_set_float4(image_col_loc, _draw_color_r(draw_color) / 255.0, _draw_color_g(draw_color) / 255.0, _draw_color_b(draw_color) / 255.0,
	               _draw_color_a(draw_color) / 255.0);
	gpu_set_texture(image_tex_unit, tex);
	gpu_draw();
}

void draw_scaled_image(gpu_texture_t *tex, float dx, float dy, float dw, float dh) {
	draw_scaled_sub_image(tex, 0, 0, (float)tex->width, (float)tex->height, dx, dy, dw, dh);
}

void draw_sub_image(gpu_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y) {
	draw_scaled_sub_image(tex, sx, sy, sw, sh, x, y, sw, sh);
}

void draw_image(gpu_texture_t *tex, float x, float y) {
	draw_scaled_sub_image(tex, 0, 0, (float)tex->width, (float)tex->height, x, y, (float)tex->width, (float)tex->height);
}

void draw_filled_triangle(float x0, float y0, float x1, float y1, float x2, float y2) {
	gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &tris_pipeline);
	gpu_set_vertex_buffer(&tris_vertex_buffer);
	gpu_set_index_buffer(&tris_index_buffer);
	gpu_set_float2(tris_pos0_loc, x0 / vw(), y0 / vh());
	gpu_set_float2(tris_pos1_loc, x1 / vw(), y1 / vh());
	gpu_set_float2(tris_pos2_loc, x2 / vw(), y2 / vh());
	gpu_set_float4(tris_col_loc, _draw_color_r(draw_color) / 255.0, _draw_color_g(draw_color) / 255.0, _draw_color_b(draw_color) / 255.0,
	               _draw_color_a(draw_color) / 255.0);
	gpu_draw();
}

void draw_filled_rect(float x, float y, float width, float height) {
	gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &rect_pipeline);
	gpu_set_vertex_buffer(&rect_vertex_buffer);
	gpu_set_index_buffer(&rect_index_buffer);
	gpu_set_float4(rect_pos_loc, (int)x / vw(), (int)y / vh(), (int)width / vw(), (int)height / vh());
	gpu_set_float4(rect_col_loc, _draw_color_r(draw_color) / 255.0, _draw_color_g(draw_color) / 255.0, _draw_color_b(draw_color) / 255.0,
	               _draw_color_a(draw_color) / 255.0);
	gpu_draw();
}

void draw_rect(float x, float y, float width, float height, float strength) {
	float hs = strength / 2.0f;
	draw_filled_rect(x - hs, y - hs, width + strength, strength);          // Top
	draw_filled_rect(x - hs, y + hs, strength, height - strength);         // Left
	draw_filled_rect(x - hs, y + height - hs, width + strength, strength); // Bottom
	draw_filled_rect(x + width - hs, y + hs, strength, height - strength); // Right
}

void draw_line(float x0, float y0, float x1, float y1, float strength) {
	vec2_t vec;
	if (y1 == y0) {
		vec = (vec2_t){0.0f, -1.0f};
	}
	else {
		vec = (vec2_t){1.0f, -(x1 - x0) / (y1 - y0)};
	}

	float current_length = sqrtf(vec.x * vec.x + vec.y * vec.y);
	if (current_length != 0) {
		float mul = strength / current_length;
		vec.x *= mul;
		vec.y *= mul;
	}

	vec2_t p0 = (vec2_t){x0 + 0.5f * vec.x, y0 + 0.5f * vec.y};
	vec2_t p1 = (vec2_t){x1 + 0.5f * vec.x, y1 + 0.5f * vec.y};
	vec2_t p2 = (vec2_t){p0.x - vec.x, p0.y - vec.y};
	vec2_t p3 = (vec2_t){p1.x - vec.x, p1.y - vec.y};
	draw_filled_triangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
	draw_filled_triangle(p2.x, p2.y, p1.x, p1.y, p3.x, p3.y);
}

void draw_line_aa(float x0, float y0, float x1, float y1, float strength) {
	draw_line(x0, y0, x1, y1, strength);
	uint32_t _color = draw_color;
	draw_set_color(_draw_color(_draw_color_r(draw_color), _draw_color_g(draw_color), _draw_color_b(draw_color), 150));
	draw_line(x0 + 0.5, y0, x1 + 0.5, y1, strength);
	draw_line(x0 - 0.5, y0, x1 - 0.5, y1, strength);
	draw_line(x0, y0 + 0.5, x1, y1 + 0.5, strength);
	draw_line(x0, y0 - 0.5, x1, y1 - 0.5, strength);
	draw_set_color(_color);
}

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

typedef struct draw_glyph {
	int16_t x0, y0, x1, y1;
	float   xoff, yoff, xadvance;
} draw_glyph_t;

#define SLUG_HBAND_COUNT     16
#define SLUG_VBAND_COUNT     16
#define SLUG_CURVE_TEX_WIDTH 4096
#define SLUG_BAND_TEX_WIDTH  4096

typedef struct slug_glyph {
	int   band_tex_x, band_tex_y;
	float xoff, yoff, glyph_w, glyph_h;
	float xadvance;
	float band_scale_x, band_scale_y;
} slug_glyph_t;

typedef struct draw_font_image {
	float          m_size;
	slug_glyph_t  *slug_glyphs;
	gpu_texture_t *curve_tex;
	gpu_texture_t *band_tex;
	int            curve_tex_width;
	int            curve_tex_height;
	int            band_tex_width;
	int            band_tex_height;
	int            width;
	int            height;
	int            first_unused_y;
	float          baseline;
	float          descent;
	float          line_gap;
} draw_font_image_t;

draw_font_image_t *draw_font_get_image_internal(draw_font_t *font, int size) {
	for (int i = 0; i < font->m_images_len; ++i) {
		if ((int)font->images[i].m_size == size) {
			return &(font->images[i]);
		}
	}
	return NULL;
}

static inline bool draw_prepare_font_load_internal(draw_font_t *font, int size) {
	if (draw_font_get_image_internal(font, size) != NULL) {
		return false; // Nothing to do
	}

	// Resize images array if necessary
	if (font->m_capacity <= font->m_images_len) {
		font->m_capacity = font->m_images_len + 1;
		if (font->images == NULL) {
			font->images = (draw_font_image_t *)malloc(font->m_capacity * sizeof(draw_font_image_t));
		}
		else {
			font->images = (draw_font_image_t *)realloc(font->images, font->m_capacity * sizeof(draw_font_image_t));
		}
	}

	return true;
}

typedef struct {
	float p1x, p1y, p2x, p2y, p3x, p3y;
} _slug_curve_t;

static int _slug_extract_curves(stbtt_fontinfo *info, int glyph_index, float scale, int bx0, int by0, _slug_curve_t **out) {
	stbtt_vertex *verts;
	int           nv = stbtt_GetGlyphShape(info, glyph_index, &verts);
	if (nv == 0) {
		*out = NULL;
		return 0;
	}

	_slug_curve_t *c  = (_slug_curve_t *)malloc(nv * 2 * sizeof(_slug_curve_t));
	int            nc = 0;
	float          cx = 0.0f, cy = 0.0f;

	for (int i = 0; i < nv; i++) {
		float vx  = verts[i].x * scale - bx0;
		float vy  = -verts[i].y * scale - by0;
		float vcx = verts[i].cx * scale - bx0;
		float vcy = -verts[i].cy * scale - by0;

		switch (verts[i].type) {
		case STBTT_vmove:
			cx = vx;
			cy = vy;
			break;
		case STBTT_vline:
			c[nc++] = (_slug_curve_t){cx, cy, (cx + vx) * 0.5f, (cy + vy) * 0.5f, vx, vy};
			cx      = vx;
			cy      = vy;
			break;
		case STBTT_vcurve:
			c[nc++] = (_slug_curve_t){cx, cy, vcx, vcy, vx, vy};
			cx      = vx;
			cy      = vy;
			break;
		case STBTT_vcubic: {
			float cx2 = verts[i].cx1 * scale - bx0;
			float cy2 = -verts[i].cy1 * scale - by0;
			float m1x = (cx + vcx) * 0.5f, m1y = (cy + vcy) * 0.5f;
			float m2x = (vcx + cx2) * 0.5f, m2y = (vcy + cy2) * 0.5f;
			float m3x = (cx2 + vx) * 0.5f, m3y = (cy2 + vy) * 0.5f;
			float m12x = (m1x + m2x) * 0.5f, m12y = (m1y + m2y) * 0.5f;
			float m23x = (m2x + m3x) * 0.5f, m23y = (m2y + m3y) * 0.5f;
			float midx = (m12x + m23x) * 0.5f, midy = (m12y + m23y) * 0.5f;
			c[nc++] = (_slug_curve_t){cx, cy, m12x, m12y, midx, midy};
			c[nc++] = (_slug_curve_t){midx, midy, m23x, m23y, vx, vy};
			cx      = vx;
			cy      = vy;
			break;
		}
		}
	}
	stbtt_FreeShape(info, verts);
	*out = c;
	return nc;
}

static _slug_curve_t *_slug_sort_curves;

static int _slug_cmp_hband(const void *a, const void *b) {
	int   ia = *(const int *)a, ib = *(const int *)b;
	float ma = fmaxf(fmaxf(_slug_sort_curves[ia].p1x, _slug_sort_curves[ia].p2x), _slug_sort_curves[ia].p3x);
	float mb = fmaxf(fmaxf(_slug_sort_curves[ib].p1x, _slug_sort_curves[ib].p2x), _slug_sort_curves[ib].p3x);
	return (ma < mb) - (ma > mb);
}

static int _slug_cmp_vband(const void *a, const void *b) {
	int   ia = *(const int *)a, ib = *(const int *)b;
	float ma = fmaxf(fmaxf(_slug_sort_curves[ia].p1y, _slug_sort_curves[ia].p2y), _slug_sort_curves[ia].p3y);
	float mb = fmaxf(fmaxf(_slug_sort_curves[ib].p1y, _slug_sort_curves[ib].p2y), _slug_sort_curves[ib].p3y);
	return (ma < mb) - (ma > mb);
}

typedef struct {
	float *data;
	int    texels;
	int    cap;
} _slug_fbuf_t;

static void _slug_push(_slug_fbuf_t *b, float r, float g, float bv, float a) {
	if (b->texels >= b->cap) {
		b->cap  = b->cap < 64 ? 256 : b->cap * 2;
		b->data = (float *)realloc(b->data, (size_t)b->cap * 4 * sizeof(float));
	}
	int i          = b->texels * 4;
	b->data[i]     = r;
	b->data[i + 1] = g;
	b->data[i + 2] = bv;
	b->data[i + 3] = a;
	b->texels++;
}

bool draw_font_load(draw_font_t *font, int size) {
	if (!draw_prepare_font_load_internal(font, size)) {
		return true;
	}

	draw_font_image_t *img = &(font->images[font->m_images_len]);
	font->m_images_len += 1;
	memset(img, 0, sizeof(draw_font_image_t));
	img->m_size = (float)size;

	stbtt_fontinfo info;
	stbtt_InitFont(&info, font->blob, font->offset);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	float scale   = stbtt_ScaleForPixelHeight(&info, (float)size);
	img->baseline = (float)((int)(ascent * scale + 0.5f));
	img->descent  = (float)((int)(descent * scale + 0.5f));
	img->line_gap = (float)((int)(line_gap * scale + 0.5f));

	int num_glyphs   = draw_font_glyphs->length;
	img->slug_glyphs = (slug_glyph_t *)malloc((size_t)num_glyphs * sizeof(slug_glyph_t));
	memset(img->slug_glyphs, 0, (size_t)num_glyphs * sizeof(slug_glyph_t));

	_slug_fbuf_t curve_buf = {NULL, 0, 0};
	_slug_fbuf_t band_buf  = {NULL, 0, 0};

	int total_bands = SLUG_HBAND_COUNT + SLUG_VBAND_COUNT;

	for (int gi = 0; gi < num_glyphs; gi++) {
		int codepoint   = draw_font_glyphs->buffer[gi];
		int glyph_index = stbtt_FindGlyphIndex(&info, codepoint);
		int advance, lsb;
		stbtt_GetGlyphHMetrics(&info, glyph_index, &advance, &lsb);
		img->slug_glyphs[gi].xadvance = advance * scale;

		int bx0, by0, bx1, by1;
		stbtt_GetGlyphBitmapBox(&info, glyph_index, scale, scale, &bx0, &by0, &bx1, &by1);
		int gw = bx1 - bx0, gh = by1 - by0;

		img->slug_glyphs[gi].xoff    = (float)bx0;
		img->slug_glyphs[gi].yoff    = (float)by0;
		img->slug_glyphs[gi].glyph_w = (float)gw;
		img->slug_glyphs[gi].glyph_h = (float)gh;

		if (gw <= 0 || gh <= 0) {
			continue; // whitespace or zero-size glyph
		}

		_slug_curve_t *curves;
		int            nc = _slug_extract_curves(&info, glyph_index, scale, bx0, by0, &curves);

		// Glyph's band texture origin (top-left of its band data block)
		int glyph_band_start              = band_buf.texels;
		img->slug_glyphs[gi].band_tex_x   = glyph_band_start % SLUG_BAND_TEX_WIDTH;
		img->slug_glyphs[gi].band_tex_y   = glyph_band_start / SLUG_BAND_TEX_WIDTH;
		img->slug_glyphs[gi].band_scale_x = (gw > 0) ? (float)(SLUG_VBAND_COUNT - 1) / (float)gw : 0.0f;
		img->slug_glyphs[gi].band_scale_y = (gh > 0) ? (float)(SLUG_HBAND_COUNT - 1) / (float)gh : 0.0f;

		// Reserve descriptor texels: HBAND_COUNT + VBAND_COUNT
		int descriptor_start = band_buf.texels;
		for (int b = 0; b < total_bands; b++) {
			_slug_push(&band_buf, 0.0f, 0.0f, 0.0f, 0.0f); // placeholder
		}

		// Build per-band curve index lists (use COUNT-1 to match shader's band_scale)
		float hband_h = (float)gh / (SLUG_HBAND_COUNT - 1);
		float vband_w = (float)gw / (SLUG_VBAND_COUNT - 1);

		// Per-band curve index arrays (stored as flat array: band * nc + index)
		int *band_indices = (int *)malloc((size_t)(total_bands * (nc + 1)) * sizeof(int));
		int *band_sizes   = (int *)calloc((size_t)total_bands, sizeof(int));

		_slug_sort_curves = curves;
		for (int c = 0; c < nc; c++) {
			float ymin = fminf(fminf(curves[c].p1y, curves[c].p2y), curves[c].p3y);
			float ymax = fmaxf(fmaxf(curves[c].p1y, curves[c].p2y), curves[c].p3y);
			float xmin = fminf(fminf(curves[c].p1x, curves[c].p2x), curves[c].p3x);
			float xmax = fmaxf(fmaxf(curves[c].p1x, curves[c].p2x), curves[c].p3x);

			int hlo = (int)(ymin / hband_h);
			if (hlo < 0)
				hlo = 0;
			int hhi = (int)(ymax / hband_h);
			if (hhi >= SLUG_HBAND_COUNT)
				hhi = SLUG_HBAND_COUNT - 1;
			for (int b = hlo; b <= hhi; b++) {
				band_indices[b * (nc + 1) + band_sizes[b]] = c;
				band_sizes[b]++;
			}

			int vlo = (int)(xmin / vband_w);
			if (vlo < 0)
				vlo = 0;
			int vhi = (int)(xmax / vband_w);
			if (vhi >= SLUG_VBAND_COUNT)
				vhi = SLUG_VBAND_COUNT - 1;
			for (int b = vlo; b <= vhi; b++) {
				int bi                                       = SLUG_HBAND_COUNT + b;
				band_indices[bi * (nc + 1) + band_sizes[bi]] = c;
				band_sizes[bi]++;
			}
		}

		// Sort horizontal bands by descending max-x, vertical by descending max-y
		for (int b = 0; b < SLUG_HBAND_COUNT; b++) {
			qsort(&band_indices[b * (nc + 1)], (size_t)band_sizes[b], sizeof(int), _slug_cmp_hband);
		}
		for (int b = 0; b < SLUG_VBAND_COUNT; b++) {
			int bi = SLUG_HBAND_COUNT + b;
			qsort(&band_indices[bi * (nc + 1)], (size_t)band_sizes[bi], sizeof(int), _slug_cmp_vband);
		}

		// Write each curve exactly once to the curve texture; record its (tx, ty) position.
		int *curve_tex_pos_x = (int *)malloc((size_t)nc * sizeof(int));
		int *curve_tex_pos_y = (int *)malloc((size_t)nc * sizeof(int));
		for (int c = 0; c < nc; c++) {
			curve_tex_pos_x[c] = curve_buf.texels % SLUG_CURVE_TEX_WIDTH;
			curve_tex_pos_y[c] = curve_buf.texels / SLUG_CURVE_TEX_WIDTH;
			_slug_push(&curve_buf, curves[c].p1x, curves[c].p1y, curves[c].p2x, curves[c].p2y);
			_slug_push(&curve_buf, curves[c].p3x, curves[c].p3y, 0.0f, 0.0f);
		}

		// Write band curve-location lists into band texture; back-fill descriptors.
		for (int b = 0; b < total_bands; b++) {
			int count  = band_sizes[b];
			int offset = band_buf.texels - glyph_band_start;

			for (int k = 0; k < count; k++) {
				int ci = band_indices[b * (nc + 1) + k];
				_slug_push(&band_buf, (float)curve_tex_pos_x[ci], (float)curve_tex_pos_y[ci], 0.0f, 0.0f);
			}

			int desc_abs                    = descriptor_start + b;
			band_buf.data[desc_abs * 4 + 0] = (float)count;
			band_buf.data[desc_abs * 4 + 1] = (float)offset;
			band_buf.data[desc_abs * 4 + 2] = 0.0f;
			band_buf.data[desc_abs * 4 + 3] = 0.0f;
		}

		free(curve_tex_pos_x);
		free(curve_tex_pos_y);

		free(band_indices);
		free(band_sizes);
		free(curves);
	}

	// Build GPU textures
	int ctw = SLUG_CURVE_TEX_WIDTH;
	int cth = (curve_buf.texels + ctw - 1) / ctw;
	if (cth < 1)
		cth = 1;
	int btw = SLUG_BAND_TEX_WIDTH;
	int bth = (band_buf.texels + btw - 1) / btw;
	if (bth < 1)
		bth = 1;

	// Pad to full rows
	int curve_total = ctw * cth;
	int band_total  = btw * bth;
	curve_buf.data  = (float *)realloc(curve_buf.data, (size_t)curve_total * 4 * sizeof(float));
	if (curve_buf.texels < curve_total)
		memset(&curve_buf.data[curve_buf.texels * 4], 0, (size_t)(curve_total - curve_buf.texels) * 4 * sizeof(float));
	band_buf.data = (float *)realloc(band_buf.data, (size_t)band_total * 4 * sizeof(float));
	if (band_buf.texels < band_total)
		memset(&band_buf.data[band_buf.texels * 4], 0, (size_t)(band_total - band_buf.texels) * 4 * sizeof(float));

	img->curve_tex        = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	img->band_tex         = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	img->curve_tex_width  = ctw;
	img->curve_tex_height = cth;
	img->band_tex_width   = btw;
	img->band_tex_height  = bth;

	gpu_texture_init_from_bytes(img->curve_tex, curve_buf.data, ctw, cth, GPU_TEXTURE_FORMAT_RGBA128);
	gpu_texture_init_from_bytes(img->band_tex, band_buf.data, btw, bth, GPU_TEXTURE_FORMAT_RGBA128);

	free(curve_buf.data);
	free(band_buf.data);
	return true;
}

int draw_font_get_char_index_internal(int char_index) {
	if (draw_font_glyphs->length <= 0) {
		return 0;
	}
	int offset = draw_font_glyph_blocks->buffer[0];
	if (char_index < offset) {
		return 0;
	}

	for (int i = 1; i < draw_font_glyph_blocks->length / 2; ++i) {
		int prev_end = draw_font_glyph_blocks->buffer[i * 2 - 1];
		int start    = draw_font_glyph_blocks->buffer[i * 2];
		if (char_index > start - 1) {
			offset += start - 1 - prev_end;
		}
	}

	if (char_index - offset >= draw_font_glyphs->length) {
		return 0;
	}
	return char_index - offset;
}

void draw_string(const char *text, float x, float y) {
	draw_font_image_t *img  = draw_font_get_image_internal(draw_font, draw_font_size);
	float              xpos = x;
	float              ypos = y + img->baseline;
	float              cr   = _draw_color_r(draw_color) / 255.0f;
	float              cg   = _draw_color_g(draw_color) / 255.0f;
	float              cb   = _draw_color_b(draw_color) / 255.0f;
	float              ca   = _draw_color_a(draw_color) / 255.0f;

	// Slug vector rendering path

	for (int i = 0; text[i] != 0;) {

		gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : _draw_current != NULL ? &slug_rt_pipeline : &slug_pipeline);
		gpu_set_vertex_buffer(&rect_vertex_buffer);
		gpu_set_index_buffer(&rect_index_buffer);
		gpu_set_texture(slug_curve_tex_unit, img->curve_tex);
		gpu_set_texture(slug_band_tex_unit, img->band_tex);
		gpu_set_float4(slug_col_loc, cr, cg, cb, ca);

		int l         = 0;
		int codepoint = string_utf8_decode(&text[i], &l);
		i += l;

		int char_index = draw_font_get_char_index_internal(codepoint);
		if (char_index >= draw_font_glyphs->length) {
			continue;
		}

		slug_glyph_t *g = &img->slug_glyphs[char_index];
		xpos += g->xadvance;
		if (g->glyph_w <= 0.0f || g->glyph_h <= 0.0f) {
			continue;
		}

		float rx = (float)(int)(xpos - g->xadvance + g->xoff + 0.5f);
		float ry = (float)(int)(ypos + g->yoff + 0.5f);

		gpu_set_float4(slug_pos_loc, rx / vw(), ry / vh(), g->glyph_w / vw(), g->glyph_h / vh());
		gpu_set_float4(slug_em_rect_loc, 0.0f, 0.0f, g->glyph_w, g->glyph_h);
		gpu_set_int4(slug_glyph_loc_loc, g->band_tex_x, g->band_tex_y, 0, 0);
		gpu_set_int4(slug_band_max_loc, SLUG_VBAND_COUNT - 1, SLUG_HBAND_COUNT - 1, 0, 0);
		gpu_set_float4(slug_band_tf_loc, g->band_scale_x, g->band_scale_y, 0.0f, 0.0f);
		gpu_draw();
	}
}

void draw_font_init(draw_font_t *font) {
	if (draw_font_glyphs == NULL) {
		draw_font_init_glyphs(32, 127);
	}

	if (font->glyphs_version != draw_glyphs_version) {
		font->glyphs_version = draw_glyphs_version;
		font->blob           = font->buf->buffer;
		font->images         = NULL;
		font->m_images_len   = 0;
		font->m_capacity     = 0;
		font->offset         = stbtt_GetFontOffsetForIndex(font->blob, font->index);
		if (font->offset == -1) {
			font->offset = stbtt_GetFontOffsetForIndex(font->blob, 0);
		}
	}
}

void draw_font_destroy(draw_font_t *font) {
	font->buf = NULL;
}

void draw_font_init_glyphs(int from, int to) {
	gc_unroot(draw_font_glyphs);
	gc_unroot(draw_font_glyph_blocks);

	draw_font_glyphs = i32_array_create(to - from);
	for (int i = from; i < to; ++i) {
		draw_font_glyphs->buffer[i - from] = i;
	}
	draw_font_glyph_blocks            = i32_array_create(2);
	draw_font_glyph_blocks->buffer[0] = from;
	draw_font_glyph_blocks->buffer[1] = to - 1;
	draw_glyphs_version++;

	gc_root(draw_font_glyphs);
	gc_root(draw_font_glyph_blocks);
}

bool draw_font_has_glyph(int glyph) {
	return i32_array_index_of(draw_font_glyphs, glyph) > -1;
}

void draw_font_build_glyphs() {
	i32_array_sort(draw_font_glyphs, NULL);

	int blocks    = 1;
	int next_char = draw_font_glyphs->buffer[0] + 1;
	int pos       = 1;
	while (pos < draw_font_glyphs->length) {
		if (draw_font_glyphs->buffer[pos] != next_char) {
			++blocks;
			next_char = draw_font_glyphs->buffer[pos] + 1;
		}
		else {
			++next_char;
		}
		++pos;
	}

	gc_unroot(draw_font_glyph_blocks);
	draw_font_glyph_blocks = i32_array_create(2 * blocks);
	gc_root(draw_font_glyph_blocks);

	draw_font_glyph_blocks->buffer[0] = draw_font_glyphs->buffer[0];
	next_char                         = draw_font_glyphs->buffer[0] + 1;
	pos                               = 1;
	for (int i = 1; i < draw_font_glyphs->length; ++i) {
		if (draw_font_glyphs->buffer[i] != next_char) {
			draw_font_glyph_blocks->buffer[pos * 2 - 1] = draw_font_glyphs->buffer[i - 1];
			draw_font_glyph_blocks->buffer[pos * 2]     = draw_font_glyphs->buffer[i];
			++pos;
			next_char = draw_font_glyphs->buffer[i] + 1;
		}
		else {
			++next_char;
		}
	}
	draw_font_glyph_blocks->buffer[blocks * 2 - 1] = draw_font_glyphs->buffer[draw_font_glyphs->length - 1];
	draw_glyphs_version++;
}

void draw_font_add_glyph(int glyph) {
	i32_array_push(draw_font_glyphs, glyph);
	draw_font_build_glyphs();
}

int draw_font_count(draw_font_t *font) {
	return stbtt_GetNumberOfFonts(font->blob);
}

int draw_font_height(draw_font_t *font, int font_size) {
	draw_font_load(font, font_size);
	return (int)draw_font_get_image_internal(font, font_size)->m_size;
}

float draw_font_get_char_width_internal(draw_font_image_t *img, int char_index) {
	int i = draw_font_get_char_index_internal(char_index);
	return img->slug_glyphs[i].xadvance;
}

float draw_sub_string_width(draw_font_t *font, int font_size, const char *text, int start, int end) {
	draw_font_load(font, font_size);
	draw_font_image_t *img   = draw_font_get_image_internal(font, font_size);
	float              width = 0.0;
	for (int i = start; i < end; ++i) {
		if (text[i] == '\0')
			break;
		width += draw_font_get_char_width_internal(img, text[i]);
	}
	return width;
}

int draw_string_width(draw_font_t *font, int font_size, const char *text) {
	return (int)draw_sub_string_width(font, font_size, text, 0, (int)strlen(text));
}

void draw_set_color(uint32_t color) {
	draw_color = color;
}

uint32_t draw_get_color() {
	return draw_color;
}

void draw_set_pipeline(gpu_pipeline_t *pipeline) {
	draw_custom_pipeline = pipeline;
}

void draw_set_transform(mat3_t matrix) {
	draw_transform = matrix;
}

bool draw_set_font(draw_font_t *font, int size) {
	draw_font      = font;
	draw_font_size = size;
	return draw_font_load(font, size);
}

void draw_filled_circle(float cx, float cy, float radius, int segments) {
	if (segments <= 0) {
		segments = (int)floor(10 * sqrtf(radius));
	}
	float theta = 2.0 * (float)IRON_PI / segments;
	float c     = cosf(theta);
	float s     = sinf(theta);
	float x     = radius;
	float y     = 0.0;
	for (int n = 0; n < segments; ++n) {
		float px = x + cx;
		float py = y + cy;
		float t  = x;
		x        = c * x - s * y;
		y        = c * y + s * t;
		draw_filled_triangle(px, py, x + cx, y + cy, cx, cy);
	}
}

void draw_inner_line(float x1, float y1, float x2, float y2, float strength) {
	int side = y2 > y1 ? 1 : 0;
	if (y2 == y1) {
		side = x2 - x1 > 0 ? 1 : 0;
	}
	vec2_t vec;
	if (y2 == y1) {
		vec = vec2_create(0, -1);
	}
	else {
		vec = vec2_create(1, -(x2 - x1) / (y2 - y1));
	}
	vec       = vec2_set_len(vec, strength);
	vec2_t p1 = {x1 + side * vec.x, y1 + side * vec.y};
	vec2_t p2 = {x2 + side * vec.x, y2 + side * vec.y};
	vec2_t p3 = vec2_sub(p1, vec);
	vec2_t p4 = vec2_sub(p2, vec);
	draw_filled_triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
	draw_filled_triangle(p3.x, p3.y, p2.x, p2.y, p4.x, p4.y);
}

void draw_circle(float cx, float cy, float radius, int segments, float strength) {
	radius += strength / 2;
	if (segments <= 0) {
		segments = (int)floor(10 * sqrtf(radius));
	}
	float theta = 2 * (float)IRON_PI / segments;
	float c     = cosf(theta);
	float s     = sinf(theta);
	float x     = radius;
	float y     = 0.0;
	for (int n = 0; n < segments; ++n) {
		float px = x + cx;
		float py = y + cy;
		float t  = x;
		x        = c * x - s * y;
		y        = c * y + s * t;
		draw_inner_line(x + cx, y + cy, px, py, strength);
	}
}

void draw_calculate_cubic_bezier_point(float t, float *x, float *y, float *out) {
	float u   = 1 - t;
	float tt  = t * t;
	float uu  = u * u;
	float uuu = uu * u;
	float ttt = tt * t;
	// first term
	out[0] = uuu * x[0];
	out[1] = uuu * y[0];
	// second term
	out[0] += 3 * uu * t * x[1];
	out[1] += 3 * uu * t * y[1];
	// third term
	out[0] += 3 * u * tt * x[2];
	out[1] += 3 * u * tt * y[2];
	// fourth term
	out[0] += ttt * x[3];
	out[1] += ttt * y[3];
}

void draw_cubic_bezier(f32_array_t *xa, f32_array_t *ya, int segments, float strength) {
	float *x = xa->buffer;
	float *y = ya->buffer;
	float  q0[2];
	float  q1[2];
	draw_calculate_cubic_bezier_point(0, x, y, q0);

	for (int i = 1; i < (segments + 1); ++i) {
		float t = (float)i / segments;
		draw_calculate_cubic_bezier_point(t, x, y, q1);
		draw_line(q0[0], q0[1], q1[0], q1[1], strength);
		q0[0] = q1[0];
		q0[1] = q1[1];
	}
}
