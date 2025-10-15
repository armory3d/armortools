#include "iron_draw.h"
#include "const_data.h"
#include "iron_file.h"
#include "iron_gc.h"
#include "iron_gpu.h"
#include "iron_math.h"
#include "iron_simd.h"
#include "iron_string.h"
#include "iron_system.h"
#include "iron_vec2.h"
#include <math.h>

#define MATH_PI 3.14159265358979323846

draw_font_t   *draw_font = NULL;
int            draw_font_size;
gpu_texture_t *_draw_current = NULL;

static iron_matrix3x3_t draw_transform;
static uint32_t         draw_color           = 0;
static gpu_pipeline_t  *draw_custom_pipeline = NULL;

static gpu_buffer_t           rect_vertex_buffer;
static gpu_buffer_t           rect_index_buffer;
static gpu_buffer_t           tris_vertex_buffer;
static gpu_buffer_t           tris_index_buffer;
static gpu_vertex_structure_t draw_structure;

static gpu_pipeline_t image_pipeline;
static gpu_pipeline_t image_transform_pipeline;
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

static int vw() {
	return _draw_current != NULL ? _draw_current->width : iron_window_width();
}

static int vh() {
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
               buffer_t *rect_frag, buffer_t *tris_vert, buffer_t *tris_frag, buffer_t *text_vert, buffer_t *text_frag) {
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
		gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &image_pipeline);
	}
	else {
		gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &image_transform_pipeline);
		gpu_set_matrix3(image_transform_w_loc, draw_transform);
	}
	gpu_set_vertex_buffer(&rect_vertex_buffer);
	gpu_set_index_buffer(&rect_index_buffer);
	gpu_set_float4(image_pos_loc, dx / vw(), dy / vh(), dw / vw(), dh / vh());
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
	gpu_set_float4(rect_pos_loc, x / vw(), y / vh(), width / vw(), height / vh());
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
	iron_vector2_t vec;
	if (y1 == y0) {
		vec = (iron_vector2_t){0.0f, -1.0f};
	}
	else {
		vec = (iron_vector2_t){1.0f, -(x1 - x0) / (y1 - y0)};
	}

	float current_length = sqrtf(vec.x * vec.x + vec.y * vec.y);
	if (current_length != 0) {
		float mul = strength / current_length;
		vec.x *= mul;
		vec.y *= mul;
	}

	iron_vector2_t p0 = (iron_vector2_t){x0 + 0.5f * vec.x, y0 + 0.5f * vec.y};
	iron_vector2_t p1 = (iron_vector2_t){x1 + 0.5f * vec.x, y1 + 0.5f * vec.y};
	iron_vector2_t p2 = (iron_vector2_t){p0.x - vec.x, p0.y - vec.y};
	iron_vector2_t p3 = (iron_vector2_t){p1.x - vec.x, p1.y - vec.y};
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

typedef struct draw_font_aligned_quad {
	float x0, y0, s0, t0; // Top-left
	float x1, y1, s1, t1; // Bottom-right
	float xadvance;
} draw_font_aligned_quad_t;

typedef struct draw_font_image {
	float            m_size;
	stbtt_bakedchar *chars;
	gpu_texture_t   *tex;
	int              width, height, first_unused_y;
	float            baseline, descent, line_gap;
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

static int stbtt_BakeFontBitmapArr(unsigned char *data, int offset,       // Font location (use offset=0 for plain .ttf)
                                   float          pixel_height,           // Height of font in pixels
                                   unsigned char *pixels, int pw, int ph, // Bitmap to be filled in
                                   int *chars, int num_chars,             // Characters to bake
                                   stbtt_bakedchar *chardata) {
	float          scale;
	int            x, y, bottom_y, i;
	stbtt_fontinfo f;
	f.userdata = NULL;
	if (!stbtt_InitFont(&f, data, offset))
		return -1;
	STBTT_memset(pixels, 0, pw * ph); // Background of 0 around pixels
	x = y    = 1;
	bottom_y = 1;

	scale = stbtt_ScaleForPixelHeight(&f, pixel_height);

	for (i = 0; i < num_chars; ++i) {
		int advance, lsb, x0, y0, x1, y1, gw, gh;
		int g = stbtt_FindGlyphIndex(&f, chars[i]);
		stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
		stbtt_GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);
		gw = x1 - x0;
		gh = y1 - y0;
		if (x + gw + 1 >= pw)
			y = bottom_y, x = 1; // Advance to next row
		if (y + gh + 1 >= ph)    // Check if it fits vertically AFTER potentially moving to next row
			return -i;
		STBTT_assert(x + gw < pw);
		STBTT_assert(y + gh < ph);
		stbtt_MakeGlyphBitmap(&f, pixels + x + y * pw, gw, gh, pw, scale, scale, g);
		chardata[i].x0       = (stbtt_int16)x;
		chardata[i].y0       = (stbtt_int16)y;
		chardata[i].x1       = (stbtt_int16)(x + gw);
		chardata[i].y1       = (stbtt_int16)(y + gh);
		chardata[i].xadvance = scale * advance;
		chardata[i].xoff     = (float)x0;
		chardata[i].yoff     = (float)y0;
		x                    = x + gw + 1;
		if (y + gh + 1 > bottom_y)
			bottom_y = y + gh + 1;
	}
	return bottom_y;
}

bool draw_font_load(draw_font_t *font, int size) {
	if (!draw_prepare_font_load_internal(font, size)) {
		return true;
	}

	draw_font_image_t *img = &(font->images[font->m_images_len]);
	font->m_images_len += 1;
	int              width  = 64;
	int              height = 32;
	stbtt_bakedchar *baked  = (stbtt_bakedchar *)malloc(draw_font_glyphs->length * sizeof(stbtt_bakedchar));
	unsigned char   *pixels = NULL;

	int status = -1;
	while (status <= 0) {
		if (height < width)
			height *= 2;
		else
			width *= 2;
		if (pixels == NULL)
			pixels = (unsigned char *)malloc(width * height);
		else
			pixels = (unsigned char *)realloc(pixels, width * height);
		if (pixels == NULL)
			return false;
		status =
		    stbtt_BakeFontBitmapArr(font->blob, font->offset, (float)size, pixels, width, height, draw_font_glyphs->buffer, draw_font_glyphs->length, baked);
	}

	stbtt_fontinfo info;
	int            ascent, descent, line_gap;
	stbtt_InitFont(&info, font->blob, font->offset);
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	float scale         = stbtt_ScaleForPixelHeight(&info, (float)size);
	img->m_size         = (float)size;
	img->baseline       = (float)((int)((float)ascent * scale + 0.5f));
	img->descent        = (float)((int)((float)descent * scale + 0.5f));
	img->line_gap       = (float)((int)((float)line_gap * scale + 0.5f));
	img->width          = width;
	img->height         = height;
	img->chars          = baked;
	img->first_unused_y = status;
	img->tex            = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	gpu_texture_init_from_bytes(img->tex, pixels, width, height, GPU_TEXTURE_FORMAT_R8);
	free(pixels);
	return true;
}

gpu_texture_t *draw_font_get_texture(draw_font_t *font, int size) {
	draw_font_image_t *img = draw_font_get_image_internal(font, size);
	return img->tex;
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

bool draw_font_get_baked_quad(draw_font_t *font, int size, draw_font_aligned_quad_t *q, int char_code, float xpos, float ypos) {
	draw_font_image_t *img        = draw_font_get_image_internal(font, size);
	int                char_index = draw_font_get_char_index_internal(char_code);
	if (char_index >= draw_font_glyphs->length) {
		return false;
	}
	float           ipw     = 1.0f / (float)img->width;
	float           iph     = 1.0f / (float)img->height;
	stbtt_bakedchar b       = img->chars[char_index];
	int             round_x = (int)(xpos + b.xoff + 0.5);
	int             round_y = (int)(ypos + b.yoff + 0.5);

	q->x0 = (float)round_x;
	q->y0 = (float)round_y;
	q->x1 = (float)round_x + b.x1 - b.x0;
	q->y1 = (float)round_y + b.y1 - b.y0;

	q->s0 = b.x0 * ipw;
	q->t0 = b.y0 * iph;
	q->s1 = b.x1 * ipw;
	q->t1 = b.y1 * iph;

	q->xadvance = b.xadvance;

	return true;
}

void draw_string(const char *text, float x, float y) {
	draw_font_image_t       *img  = draw_font_get_image_internal(draw_font, draw_font_size);
	float                    xpos = x;
	float                    ypos = y + img->baseline;
	draw_font_aligned_quad_t q;

	gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : _draw_current != NULL ? &text_rt_pipeline : &text_pipeline);
	gpu_set_vertex_buffer(&rect_vertex_buffer);
	gpu_set_index_buffer(&rect_index_buffer);
	gpu_set_texture(text_tex_unit, img->tex);

	for (int i = 0; text[i] != 0;) {
		int l         = 0;
		int codepoint = string_utf8_decode(&text[i], &l);
		i += l;

		if (draw_font_get_baked_quad(draw_font, draw_font_size, &q, codepoint, xpos, ypos)) {
			xpos += q.xadvance;
			gpu_set_float4(text_pos_loc, q.x0 / vw(), q.y0 / vh(), (q.x1 - q.x0) / vw(), (q.y1 - q.y0) / vh());
			gpu_set_float4(text_tex_loc, q.s0, q.t0, q.s1 - q.s0, q.t1 - q.t0);
			gpu_set_float4(text_col_loc, _draw_color_r(draw_color) / 255.0, _draw_color_g(draw_color) / 255.0, _draw_color_b(draw_color) / 255.0,
			               _draw_color_a(draw_color) / 255.0);
			gpu_draw();
		}
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

void draw_font_13(draw_font_t *font) {
	if (draw_font_glyphs == NULL) {
		draw_font_init_glyphs(32, 127);
	}

	font->blob         = font->buf->buffer;
	font->images       = NULL;
	font->m_images_len = 0;
	font->m_capacity   = 0;
	font->offset       = 0;
	draw_prepare_font_load_internal(font, 13);

	draw_font_image_t *img = &(font->images[font->m_images_len]);
	font->m_images_len += 1;
	img->m_size         = 13;
	img->baseline       = 0; // 10
	img->descent        = 0;
	img->line_gap       = 0;
	img->width          = 128;
	img->height         = 128;
	img->first_unused_y = 0;
	img->tex            = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	gpu_texture_init_from_bytes(img->tex, (void *)iron_font_13_pixels, 128, 128, GPU_TEXTURE_FORMAT_R8);

	stbtt_bakedchar *baked = (stbtt_bakedchar *)malloc(95 * sizeof(stbtt_bakedchar));
	for (int i = 0; i < 95; ++i) {
		baked[i].x0       = iron_font_13_x0[i];
		baked[i].x1       = iron_font_13_x1[i];
		baked[i].y0       = iron_font_13_y0[i];
		baked[i].y1       = iron_font_13_y1[i];
		baked[i].xoff     = iron_font_13_xoff[i];
		baked[i].yoff     = iron_font_13_yoff[i];
		baked[i].xadvance = iron_font_13_xadvance[i];
	}
	img->chars = baked;
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
	return img->chars[i].xadvance;
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
	float theta = 2.0 * (float)MATH_PI / segments;
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
	iron_vector2_t vec;
	if (y2 == y1) {
		vec = vec2_create(0, -1);
	}
	else {
		vec = vec2_create(1, -(x2 - x1) / (y2 - y1));
	}
	vec               = vec2_set_len(vec, strength);
	iron_vector2_t p1 = {x1 + side * vec.x, y1 + side * vec.y};
	iron_vector2_t p2 = {x2 + side * vec.x, y2 + side * vec.y};
	iron_vector2_t p3 = vec2_sub(p1, vec);
	iron_vector2_t p4 = vec2_sub(p2, vec);
	draw_filled_triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
	draw_filled_triangle(p3.x, p3.y, p2.x, p2.y, p4.x, p4.y);
}

void draw_circle(float cx, float cy, float radius, int segments, float strength) {
	radius += strength / 2;
	if (segments <= 0) {
		segments = (int)floor(10 * sqrtf(radius));
	}
	float theta = 2 * (float)MATH_PI / segments;
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
