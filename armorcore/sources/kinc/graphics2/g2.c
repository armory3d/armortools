#include "g2.h"
#include "g2_font.h"

#include <math.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/textureunit.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/vertexstructure.h>
#include <kinc/io/filereader.h>
#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>
#include <kinc/simd/float32x4.h>
#include <kinc/color.h>
#include <kinc/window.h>
#include <iron_string.h>
#ifdef KINC_DIRECT3D12
extern bool waitAfterNextDraw;
#endif

#define G2_BUFFER_SIZE 1000

static kinc_matrix4x4_t g2_projection_matrix;
static bool g2_bilinear_filter = true;
static uint32_t g2_color = 0;
static kinc_g2_font_t *g2_font = NULL;
static int g2_font_size;
static bool g2_is_render_target = false;
static kinc_g4_pipeline_t *g2_last_pipeline = NULL;
static kinc_g4_pipeline_t *g2_custom_pipeline = NULL;
static kinc_matrix3x3_t g2_transform;

static kinc_g4_vertex_buffer_t image_vertex_buffer;
static kinc_g4_index_buffer_t image_index_buffer;
static kinc_g4_shader_t image_vert_shader;
static kinc_g4_shader_t image_frag_shader;
static kinc_g4_pipeline_t image_pipeline;
static kinc_g4_texture_unit_t image_tex_unit;
static kinc_g4_constant_location_t image_proj_loc;
static float *image_rect_verts = NULL;
static int image_buffer_index = 0;
static int image_buffer_start = 0;
static kinc_g4_texture_t *image_last_texture = NULL;
static kinc_g4_render_target_t *image_last_render_target = NULL;

static kinc_g4_vertex_buffer_t colored_rect_vertex_buffer;
static kinc_g4_index_buffer_t colored_rect_index_buffer;
static kinc_g4_vertex_buffer_t colored_tris_vertex_buffer;
static kinc_g4_index_buffer_t colored_tris_index_buffer;
static kinc_g4_shader_t colored_vert_shader;
static kinc_g4_shader_t colored_frag_shader;
static kinc_g4_pipeline_t colored_pipeline;
static kinc_g4_constant_location_t colored_proj_loc;
static float *colored_rect_verts = NULL;
static float *colored_tris_verts = NULL;
static int colored_rect_buffer_index = 0;
static int colored_rect_buffer_start = 0;
static int colored_tris_buffer_index = 0;
static int colored_tris_buffer_start = 0;

static kinc_g4_vertex_buffer_t text_vertex_buffer;
static kinc_g4_index_buffer_t text_index_buffer;
static kinc_g4_shader_t text_vert_shader;
static kinc_g4_shader_t text_frag_shader;
static kinc_g4_pipeline_t text_pipeline;
static kinc_g4_pipeline_t text_pipeline_rt;
static kinc_g4_texture_unit_t text_tex_unit;
static kinc_g4_constant_location_t text_proj_loc;
static float *text_rect_verts = NULL;
static int text_buffer_index = 0;
static int text_buffer_start = 0;
static kinc_g4_texture_t *text_last_texture = NULL;

static int *g2_font_glyph_blocks = NULL;
static int *g2_font_glyphs = NULL;
static int g2_font_num_glyph_blocks = -1;
static int g2_font_num_glyphs = -1;

void kinc_g2_image_end(void);
void kinc_g2_colored_end(void);
void kinc_g2_colored_rect_end(bool tris_done);
void kinc_g2_colored_tris_end(bool rects_done);
void kinc_g2_text_end(void);

kinc_matrix4x4_t kinc_g2_matrix4x4_orthogonal_projection(float left, float right, float bottom, float top, float zn, float zf) {
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	float tz = -(zf + zn) / (zf - zn);
	return (kinc_matrix4x4_t) {
		2.0f / (right - left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
		0.0f, 0.0f, -2.0f / (zf - zn), 0.0f,
		tx, ty, tz, 1.0f
	};
}

void kinc_g2_matrix3x3_multquad(kinc_matrix3x3_t *m, float qx, float qy, float qw, float qh, kinc_vector2_t *out) {
	kinc_float32x4_t xx = kinc_float32x4_load(qx, qx, qx + qw, qx + qw);
	kinc_float32x4_t yy = kinc_float32x4_load(qy + qh, qy, qy, qy + qh);

	kinc_float32x4_t m00 = kinc_float32x4_load_all(m->m[0]);
	kinc_float32x4_t m01 = kinc_float32x4_load_all(m->m[1]);
	kinc_float32x4_t m02 = kinc_float32x4_load_all(m->m[2]);
	kinc_float32x4_t m10 = kinc_float32x4_load_all(m->m[3]);
	kinc_float32x4_t m11 = kinc_float32x4_load_all(m->m[4]);
	kinc_float32x4_t m12 = kinc_float32x4_load_all(m->m[5]);
	kinc_float32x4_t m20 = kinc_float32x4_load_all(m->m[6]);
	kinc_float32x4_t m21 = kinc_float32x4_load_all(m->m[7]);
	kinc_float32x4_t m22 = kinc_float32x4_load_all(m->m[8]);

	kinc_float32x4_t w = kinc_float32x4_add(kinc_float32x4_add(kinc_float32x4_mul(m02, xx), kinc_float32x4_mul(m12, yy)), m22);
	kinc_float32x4_t px = kinc_float32x4_div(kinc_float32x4_add(kinc_float32x4_add(kinc_float32x4_mul(m00, xx), kinc_float32x4_mul(m10, yy)), m20), w);
	kinc_float32x4_t py = kinc_float32x4_div(kinc_float32x4_add(kinc_float32x4_add(kinc_float32x4_mul(m01, xx), kinc_float32x4_mul(m11, yy)), m21), w);

	out[0] = (kinc_vector2_t){kinc_float32x4_get(px, 0), kinc_float32x4_get(py, 0)};
	out[1] = (kinc_vector2_t){kinc_float32x4_get(px, 1), kinc_float32x4_get(py, 1)};
	out[2] = (kinc_vector2_t){kinc_float32x4_get(px, 2), kinc_float32x4_get(py, 2)};
	out[3] = (kinc_vector2_t){kinc_float32x4_get(px, 3), kinc_float32x4_get(py, 3)};
}

kinc_vector2_t kinc_g2_matrix3x3_multvec(kinc_matrix3x3_t *m, kinc_vector2_t v) {
	float w = m->m[2] * v.x + m->m[5] * v.y + m->m[8] * 1.0f;
	float x = (m->m[0] * v.x + m->m[3] * v.y + m->m[6] * 1.0f) / w;
	float y = (m->m[1] * v.x + m->m[4] * v.y + m->m[7] * 1.0f) / w;
	return (kinc_vector2_t){x, y};
}

void kinc_g2_internal_set_projection_matrix(kinc_g4_render_target_t *target) {
	if (target != NULL) {
		g2_projection_matrix = kinc_g2_matrix4x4_orthogonal_projection(0.0f, (float)target->width, (float)target->height, 0.0f, 0.1f, 1000.0f);
	}
	else {
		g2_projection_matrix = kinc_g2_matrix4x4_orthogonal_projection(0.0f, (float)kinc_window_width(0), (float)kinc_window_height(0), 0.0f, 0.1f, 1000.0f);
	}
}

void kinc_g2_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *colored_vert, buffer_t *colored_frag, buffer_t *text_vert, buffer_t *text_frag) {
	kinc_g2_internal_set_projection_matrix(NULL);
	g2_transform = kinc_matrix3x3_identity();

	// Image painter
	kinc_g4_shader_init(&image_vert_shader, image_vert->buffer, image_vert->length, KINC_G4_SHADER_TYPE_VERTEX);
	kinc_g4_shader_init(&image_frag_shader, image_frag->buffer, image_frag->length, KINC_G4_SHADER_TYPE_FRAGMENT);

	{
		kinc_g4_vertex_structure_t structure;
		kinc_g4_vertex_structure_init(&structure);
		kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_F32_3X);
		kinc_g4_vertex_structure_add(&structure, "tex", KINC_G4_VERTEX_DATA_F32_2X);
		kinc_g4_vertex_structure_add(&structure, "col", KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED);
		kinc_g4_pipeline_init(&image_pipeline);
		image_pipeline.input_layout = &structure;
		image_pipeline.vertex_shader = &image_vert_shader;
		image_pipeline.fragment_shader = &image_frag_shader;
		image_pipeline.blend_source = KINC_G4_BLEND_ONE;
		image_pipeline.blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		image_pipeline.alpha_blend_source = KINC_G4_BLEND_ONE;
		image_pipeline.alpha_blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		kinc_g4_pipeline_compile(&image_pipeline);

		image_tex_unit = kinc_g4_pipeline_get_texture_unit(&image_pipeline, "tex");
		image_proj_loc = kinc_g4_pipeline_get_constant_location(&image_pipeline, "P");

		kinc_g4_vertex_buffer_init(&image_vertex_buffer, G2_BUFFER_SIZE * 4, &structure, KINC_G4_USAGE_DYNAMIC);
		image_rect_verts = kinc_g4_vertex_buffer_lock_all(&image_vertex_buffer);

		kinc_g4_index_buffer_init(&image_index_buffer, G2_BUFFER_SIZE * 3 * 2, KINC_G4_USAGE_STATIC);
		int *indices = kinc_g4_index_buffer_lock_all(&image_index_buffer);
		for (int i = 0; i < G2_BUFFER_SIZE; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		kinc_g4_index_buffer_unlock_all(&image_index_buffer);
	}

	// Colored painter
	kinc_g4_shader_init(&colored_vert_shader, colored_vert->buffer, colored_vert->length, KINC_G4_SHADER_TYPE_VERTEX);
	kinc_g4_shader_init(&colored_frag_shader, colored_frag->buffer, colored_frag->length, KINC_G4_SHADER_TYPE_FRAGMENT);

	{
		kinc_g4_vertex_structure_t structure;
		kinc_g4_vertex_structure_init(&structure);
		kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_F32_3X);
		kinc_g4_vertex_structure_add(&structure, "col", KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED);
		kinc_g4_pipeline_init(&colored_pipeline);
		colored_pipeline.input_layout = &structure;
		colored_pipeline.vertex_shader = &colored_vert_shader;
		colored_pipeline.fragment_shader = &colored_frag_shader;
		// colored_pipeline.blend_source = KINC_G4_BLEND_ONE;
		colored_pipeline.blend_source = KINC_G4_BLEND_SOURCE_ALPHA;
		colored_pipeline.blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		colored_pipeline.alpha_blend_source = KINC_G4_BLEND_ONE;
		colored_pipeline.alpha_blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		kinc_g4_pipeline_compile(&colored_pipeline);

		colored_proj_loc = kinc_g4_pipeline_get_constant_location(&colored_pipeline, "P");

		kinc_g4_vertex_buffer_init(&colored_rect_vertex_buffer, G2_BUFFER_SIZE * 4, &structure, KINC_G4_USAGE_DYNAMIC);
		colored_rect_verts = kinc_g4_vertex_buffer_lock_all(&colored_rect_vertex_buffer);

		kinc_g4_index_buffer_init(&colored_rect_index_buffer, G2_BUFFER_SIZE * 3 * 2, KINC_G4_USAGE_STATIC);
		int *indices = kinc_g4_index_buffer_lock_all(&colored_rect_index_buffer);
		for (int i = 0; i < G2_BUFFER_SIZE; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		kinc_g4_index_buffer_unlock_all(&colored_rect_index_buffer);

		kinc_g4_vertex_buffer_init(&colored_tris_vertex_buffer, G2_BUFFER_SIZE * 3, &structure, KINC_G4_USAGE_DYNAMIC);
		colored_tris_verts = kinc_g4_vertex_buffer_lock_all(&colored_tris_vertex_buffer);

		kinc_g4_index_buffer_init(&colored_tris_index_buffer, G2_BUFFER_SIZE * 3, KINC_G4_USAGE_STATIC);
		indices = kinc_g4_index_buffer_lock_all(&colored_tris_index_buffer);
		for (int i = 0; i < G2_BUFFER_SIZE; ++i) {
			indices[i * 3 + 0] = i * 3 + 0;
			indices[i * 3 + 1] = i * 3 + 1;
			indices[i * 3 + 2] = i * 3 + 2;
		}
		kinc_g4_index_buffer_unlock_all(&colored_tris_index_buffer);
	}

	// Text painter
	kinc_g4_shader_init(&text_vert_shader, text_vert->buffer, text_vert->length, KINC_G4_SHADER_TYPE_VERTEX);
	kinc_g4_shader_init(&text_frag_shader, text_frag->buffer, text_frag->length, KINC_G4_SHADER_TYPE_FRAGMENT);

	{
		kinc_g4_vertex_structure_t structure;
		kinc_g4_vertex_structure_init(&structure);
		kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_F32_3X);
		kinc_g4_vertex_structure_add(&structure, "tex", KINC_G4_VERTEX_DATA_F32_2X);
		kinc_g4_vertex_structure_add(&structure, "col", KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED);

		kinc_g4_pipeline_init(&text_pipeline);
		text_pipeline.input_layout = &structure;
		text_pipeline.vertex_shader = &text_vert_shader;
		text_pipeline.fragment_shader = &text_frag_shader;
		text_pipeline.blend_source = KINC_G4_BLEND_SOURCE_ALPHA;
		text_pipeline.blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		text_pipeline.alpha_blend_source = KINC_G4_BLEND_SOURCE_ALPHA;
		text_pipeline.alpha_blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		kinc_g4_pipeline_compile(&text_pipeline);
		text_tex_unit = kinc_g4_pipeline_get_texture_unit(&text_pipeline, "tex");
		text_proj_loc = kinc_g4_pipeline_get_constant_location(&text_pipeline, "P");

		kinc_g4_pipeline_init(&text_pipeline_rt);
		text_pipeline_rt.input_layout = &structure;
		text_pipeline_rt.vertex_shader = &text_vert_shader;
		text_pipeline_rt.fragment_shader = &text_frag_shader;
		text_pipeline_rt.blend_source = KINC_G4_BLEND_SOURCE_ALPHA;
		text_pipeline_rt.blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		text_pipeline_rt.alpha_blend_source = KINC_G4_BLEND_ONE;
		text_pipeline_rt.alpha_blend_destination = KINC_G4_BLEND_INV_SOURCE_ALPHA;
		kinc_g4_pipeline_compile(&text_pipeline_rt);

		kinc_g4_vertex_buffer_init(&text_vertex_buffer, G2_BUFFER_SIZE * 4, &structure, KINC_G4_USAGE_DYNAMIC);
		text_rect_verts = kinc_g4_vertex_buffer_lock_all(&text_vertex_buffer);

		kinc_g4_index_buffer_init(&text_index_buffer, G2_BUFFER_SIZE * 3 * 2, KINC_G4_USAGE_STATIC);
		int *indices = kinc_g4_index_buffer_lock_all(&text_index_buffer);
		for (int i = 0; i < G2_BUFFER_SIZE; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		kinc_g4_index_buffer_unlock_all(&text_index_buffer);
	}
}

void kinc_g2_begin(void) {
	kinc_g2_set_color(0xffffffff);
}

void kinc_g2_set_image_rect_verts(float btlx, float btly, float tplx, float tply, float tprx, float tpry, float btrx, float btry) {
	int base_idx = (image_buffer_index - image_buffer_start) * 6 * 4;
	image_rect_verts[base_idx + 0] = btlx;
	image_rect_verts[base_idx + 1] = btly;
	image_rect_verts[base_idx + 2] = -5.0f;

	image_rect_verts[base_idx + 6] = tplx;
	image_rect_verts[base_idx + 7] = tply;
	image_rect_verts[base_idx + 8] = -5.0f;

	image_rect_verts[base_idx + 12] = tprx;
	image_rect_verts[base_idx + 13] = tpry;
	image_rect_verts[base_idx + 14] = -5.0f;

	image_rect_verts[base_idx + 18] = btrx;
	image_rect_verts[base_idx + 19] = btry;
	image_rect_verts[base_idx + 20] = -5.0f;
}

void kinc_g2_set_image_rect_tex_coords(float left, float top, float right, float bottom) {
	int base_idx = (image_buffer_index - image_buffer_start) * 6 * 4;
	image_rect_verts[base_idx + 3] = left;
	image_rect_verts[base_idx + 4] = bottom;

	image_rect_verts[base_idx + 9] = left;
	image_rect_verts[base_idx + 10] = top;

	image_rect_verts[base_idx + 15] = right;
	image_rect_verts[base_idx + 16] = top;

	image_rect_verts[base_idx + 21] = right;
	image_rect_verts[base_idx + 22] = bottom;
}

void kinc_g2_set_image_rect_colors(uint32_t color) {
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	int base_idx = (image_buffer_index - image_buffer_start) * 6 * 4;
	image_rect_verts[base_idx + 5] = *(float *)&color;
	image_rect_verts[base_idx + 11] = *(float *)&color;
	image_rect_verts[base_idx + 17] = *(float *)&color;
	image_rect_verts[base_idx + 23] = *(float *)&color;
}

void kinc_g2_draw_image_buffer(bool end) {
	if (image_buffer_index - image_buffer_start == 0) return;
	kinc_g4_vertex_buffer_unlock(&image_vertex_buffer, (image_buffer_index - image_buffer_start) * 4);
	kinc_g4_set_pipeline(g2_custom_pipeline != NULL ? g2_custom_pipeline : &image_pipeline);
	kinc_g4_set_matrix4(image_proj_loc, &g2_projection_matrix);
	kinc_g4_set_vertex_buffer(&image_vertex_buffer);
	kinc_g4_set_index_buffer(&image_index_buffer);
	if (image_last_texture != NULL) {
		kinc_g4_set_texture(image_tex_unit, image_last_texture);
	}
	else {
		kinc_g4_render_target_use_color_as_texture(image_last_render_target, image_tex_unit);
	}
	kinc_g4_set_texture_addressing(image_tex_unit, KINC_G4_TEXTURE_DIRECTION_U, KINC_G4_TEXTURE_ADDRESSING_CLAMP);
	kinc_g4_set_texture_addressing(image_tex_unit, KINC_G4_TEXTURE_DIRECTION_V, KINC_G4_TEXTURE_ADDRESSING_CLAMP);
	// kinc_g4_set_texture_mipmap_filter(image_tex_unit, g2_bilinear_filter ? KINC_G4_MIPMAP_FILTER_LINEAR : KINC_G4_MIPMAP_FILTER_NONE);
	kinc_g4_set_texture_mipmap_filter(image_tex_unit, KINC_G4_MIPMAP_FILTER_NONE);
	kinc_g4_set_texture_minification_filter(image_tex_unit, g2_bilinear_filter ? KINC_G4_TEXTURE_FILTER_LINEAR : KINC_G4_TEXTURE_FILTER_POINT);
	kinc_g4_set_texture_magnification_filter(image_tex_unit, g2_bilinear_filter ? KINC_G4_TEXTURE_FILTER_LINEAR : KINC_G4_TEXTURE_FILTER_POINT);
	kinc_g4_draw_indexed_vertices_from_to(image_buffer_start * 2 * 3, (image_buffer_index - image_buffer_start) * 2 * 3);

	if (end || image_buffer_index + 1 >= G2_BUFFER_SIZE) {
		image_buffer_start = 0;
		image_buffer_index = 0;
		image_rect_verts = kinc_g4_vertex_buffer_lock(&image_vertex_buffer, 0, G2_BUFFER_SIZE * 4);
	}
	else {
		image_buffer_start = image_buffer_index;
		image_rect_verts = kinc_g4_vertex_buffer_lock(&image_vertex_buffer, image_buffer_start * 4, (G2_BUFFER_SIZE - image_buffer_start) * 4);
	}
}

void kinc_g2_draw_scaled_sub_texture(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	kinc_g2_colored_end();
	kinc_g2_text_end();
	if (image_buffer_start + image_buffer_index + 1 >= G2_BUFFER_SIZE ||
		(image_last_texture != NULL && tex != image_last_texture) ||
		image_last_render_target != NULL) {

		kinc_g2_draw_image_buffer(false);
	}

	kinc_g2_set_image_rect_tex_coords(sx / tex->tex_width, sy / tex->tex_height, (sx + sw) / tex->tex_width, (sy + sh) / tex->tex_height);
	kinc_g2_set_image_rect_colors(g2_color);
	kinc_vector2_t p[4];
	kinc_g2_matrix3x3_multquad(&g2_transform, dx, dy, dw, dh, p);
	kinc_g2_set_image_rect_verts(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);

	++image_buffer_index;
	image_last_texture = tex;
	image_last_render_target = NULL;
}

void kinc_g2_draw_scaled_image(kinc_g4_texture_t *tex, float dx, float dy, float dw, float dh) {
	kinc_g2_draw_scaled_sub_texture(tex, 0, 0, (float)tex->tex_width, (float)tex->tex_height, dx, dy, dw, dh);
}

void kinc_g2_draw_sub_image(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y) {
	kinc_g2_draw_scaled_sub_texture(tex, sx, sy, sw, sh, x, y, sw, sh);
}

void kinc_g2_draw_image(kinc_g4_texture_t *tex, float x, float y) {
	kinc_g2_draw_scaled_sub_texture(tex, 0, 0, (float)tex->tex_width, (float)tex->tex_height, x, y, (float)tex->tex_width, (float)tex->tex_height);
}

void kinc_g2_draw_scaled_sub_render_target(kinc_g4_render_target_t *rt, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	kinc_g2_colored_end();
	kinc_g2_text_end();
	if (image_buffer_start + image_buffer_index + 1 >= G2_BUFFER_SIZE ||
		(image_last_render_target != NULL && rt != image_last_render_target) ||
		image_last_texture != NULL) {

		kinc_g2_draw_image_buffer(false);
	}

	kinc_g2_set_image_rect_tex_coords(sx / rt->width, sy / rt->height, (sx + sw) / rt->width, (sy + sh) / rt->height);
	kinc_g2_set_image_rect_colors(g2_color);
	kinc_vector2_t p[4];
	kinc_g2_matrix3x3_multquad(&g2_transform, dx, dy, dw, dh, p);
	kinc_g2_set_image_rect_verts(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);

	++image_buffer_index;
	image_last_render_target = rt;
	image_last_texture = NULL;
}

void kinc_g2_draw_scaled_render_target(kinc_g4_render_target_t *rt, float dx, float dy, float dw, float dh) {
	kinc_g2_draw_scaled_sub_render_target(rt, 0, 0, (float)rt->width, (float)rt->height, dx, dy, dw, dh);
}

void kinc_g2_draw_sub_render_target(kinc_g4_render_target_t *rt, float sx, float sy, float sw, float sh, float x, float y) {
	kinc_g2_draw_scaled_sub_render_target(rt, sx, sy, sw, sh, x, y, sw, sh);
}

void kinc_g2_draw_render_target(kinc_g4_render_target_t *rt, float x, float y) {
	kinc_g2_draw_scaled_sub_render_target(rt, 0, 0, (float)rt->width, (float)rt->height, x, y, (float)rt->width, (float)rt->height);
}

void kinc_g2_draw_scaled_sub_image(image_t *image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	#ifdef KINC_DIRECT3D12
	waitAfterNextDraw = true;
	#endif
	if (image->texture_ != NULL) {
		kinc_g4_texture_t *texture = (kinc_g4_texture_t *)image->texture_;
		kinc_g2_draw_scaled_sub_texture(texture, sx, sy, sw, sh, dx, dy, dw, dh);
	}
	else {
		kinc_g4_render_target_t *render_target = (kinc_g4_render_target_t *)image->render_target_;
		kinc_g2_draw_scaled_sub_render_target(render_target, sx, sy, sw, sh, dx, dy, dw, dh);
	}
}

void kinc_g2_colored_rect_set_verts(float btlx, float btly, float tplx, float tply, float tprx, float tpry, float btrx, float btry) {
	int base_idx = (colored_rect_buffer_index - colored_rect_buffer_start) * 4 * 4;
	colored_rect_verts[base_idx + 0] = btlx;
	colored_rect_verts[base_idx + 1] = btly;
	colored_rect_verts[base_idx + 2] = -5.0f;

	colored_rect_verts[base_idx + 4] = tplx;
	colored_rect_verts[base_idx + 5] = tply;
	colored_rect_verts[base_idx + 6] = -5.0f;

	colored_rect_verts[base_idx + 8] = tprx;
	colored_rect_verts[base_idx + 9] = tpry;
	colored_rect_verts[base_idx + 10] = -5.0f;

	colored_rect_verts[base_idx + 12] = btrx;
	colored_rect_verts[base_idx + 13] = btry;
	colored_rect_verts[base_idx + 14] = -5.0f;
}

void kinc_g2_colored_rect_set_colors(uint32_t color) {
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	int base_idx = (colored_rect_buffer_index - colored_rect_buffer_start) * 4 * 4;
	colored_rect_verts[base_idx + 3] = *(float *)&color;
	colored_rect_verts[base_idx + 7] = *(float *)&color;
	colored_rect_verts[base_idx + 11] = *(float *)&color;
	colored_rect_verts[base_idx + 15] = *(float *)&color;
}

void kinc_g2_colored_rect_draw_buffer(bool tris_done) {
	if (colored_rect_buffer_index - colored_rect_buffer_start == 0) {
		return;
	}

	if (!tris_done) kinc_g2_colored_tris_end(true);

	kinc_g4_vertex_buffer_unlock(&colored_rect_vertex_buffer, (colored_rect_buffer_index - colored_rect_buffer_start) * 4);
	kinc_g4_set_pipeline(g2_custom_pipeline != NULL ? g2_custom_pipeline : &colored_pipeline);
	kinc_g4_set_matrix4(colored_proj_loc, &g2_projection_matrix);
	kinc_g4_set_vertex_buffer(&colored_rect_vertex_buffer);
	kinc_g4_set_index_buffer(&colored_rect_index_buffer);
	kinc_g4_draw_indexed_vertices_from_to(colored_rect_buffer_start * 2 * 3, (colored_rect_buffer_index - colored_rect_buffer_start) * 2 * 3);

	if (colored_rect_buffer_index + 1 >= G2_BUFFER_SIZE) {
		colored_rect_buffer_start = 0;
		colored_rect_buffer_index = 0;
		colored_rect_verts = kinc_g4_vertex_buffer_lock(&colored_rect_vertex_buffer, 0, G2_BUFFER_SIZE * 4);
	}
	else {
		colored_rect_buffer_start = colored_rect_buffer_index;
		colored_rect_verts = kinc_g4_vertex_buffer_lock(&colored_rect_vertex_buffer, colored_rect_buffer_start * 4, (G2_BUFFER_SIZE - colored_rect_buffer_start) * 4);
	}
}

void kinc_g2_colored_tris_set_verts(float x0, float y0, float x1, float y1, float x2, float y2) {
	int base_idx = (colored_tris_buffer_index - colored_tris_buffer_start) * 4 * 3;
	colored_tris_verts[base_idx + 0] = x0;
	colored_tris_verts[base_idx + 1] = y0;
	colored_tris_verts[base_idx + 2] = -5.0f;

	colored_tris_verts[base_idx + 4] = x1;
	colored_tris_verts[base_idx + 5] = y1;
	colored_tris_verts[base_idx + 6] = -5.0f;

	colored_tris_verts[base_idx + 8] = x2;
	colored_tris_verts[base_idx + 9] = y2;
	colored_tris_verts[base_idx + 10] = -5.0f;
}

void kinc_g2_colored_tris_set_colors(uint32_t color) {
	int base_idx = (colored_tris_buffer_index - colored_tris_buffer_start) * 4 * 3;
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	colored_tris_verts[base_idx + 3] = *(float *)&color;
	colored_tris_verts[base_idx + 7] = *(float *)&color;
	colored_tris_verts[base_idx + 11] = *(float *)&color;
}

void kinc_g2_colored_tris_draw_buffer(bool rect_done) {
	if (colored_tris_buffer_index - colored_tris_buffer_start == 0) {
		return;
	}

	if (!rect_done) kinc_g2_colored_rect_end(true);

	kinc_g4_vertex_buffer_unlock(&colored_tris_vertex_buffer, (colored_tris_buffer_index - colored_tris_buffer_start) * 3);
	kinc_g4_set_pipeline(g2_custom_pipeline != NULL ? g2_custom_pipeline : &colored_pipeline);
	kinc_g4_set_matrix4(colored_proj_loc, &g2_projection_matrix);
	kinc_g4_set_vertex_buffer(&colored_tris_vertex_buffer);
	kinc_g4_set_index_buffer(&colored_tris_index_buffer);
	kinc_g4_draw_indexed_vertices_from_to(colored_tris_buffer_start * 3, (colored_tris_buffer_index - colored_tris_buffer_start) * 3);

	if (colored_tris_buffer_index + 1 >= G2_BUFFER_SIZE) {
		colored_tris_buffer_start = 0;
		colored_tris_buffer_index = 0;
		colored_tris_verts = kinc_g4_vertex_buffer_lock(&colored_tris_vertex_buffer, 0, G2_BUFFER_SIZE * 3);
	}
	else {
		colored_tris_buffer_start = colored_tris_buffer_index;
		colored_tris_verts = kinc_g4_vertex_buffer_lock(&colored_tris_vertex_buffer, colored_tris_buffer_start * 3, (G2_BUFFER_SIZE - colored_tris_buffer_start) * 3);
	}
}

void kinc_g2_colored_rect_end(bool tris_done) {
	if (colored_rect_buffer_index - colored_rect_buffer_start > 0) kinc_g2_colored_rect_draw_buffer(tris_done);
}

void kinc_g2_colored_tris_end(bool rects_done) {
	if (colored_tris_buffer_index - colored_tris_buffer_start > 0) kinc_g2_colored_tris_draw_buffer(rects_done);
}

void kinc_g2_fill_triangle(float x0, float y0, float x1, float y1, float x2, float y2) {
	kinc_g2_image_end();
	kinc_g2_text_end();
	if (colored_rect_buffer_index - colored_rect_buffer_start > 0) kinc_g2_colored_rect_draw_buffer(true);
	if (colored_tris_buffer_index + 1 >= G2_BUFFER_SIZE) kinc_g2_colored_tris_draw_buffer(false);

	kinc_g2_colored_tris_set_colors(g2_color);

	kinc_vector2_t p0 = kinc_g2_matrix3x3_multvec(&g2_transform, (kinc_vector2_t){x0, y0}); // Bottom-left
	kinc_vector2_t p1 = kinc_g2_matrix3x3_multvec(&g2_transform, (kinc_vector2_t){x1, y1}); // Top-left
	kinc_vector2_t p2 = kinc_g2_matrix3x3_multvec(&g2_transform, (kinc_vector2_t){x2, y2}); // Top-right
	kinc_g2_colored_tris_set_verts(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);

	++colored_tris_buffer_index;
}

void kinc_g2_fill_rect(float x, float y, float width, float height) {
	kinc_g2_image_end();
	kinc_g2_text_end();
	if (colored_tris_buffer_index - colored_tris_buffer_start > 0) kinc_g2_colored_tris_draw_buffer(true);
	if (colored_rect_buffer_index + 1 >= G2_BUFFER_SIZE) kinc_g2_colored_rect_draw_buffer(false);

	kinc_g2_colored_rect_set_colors(g2_color);

	kinc_vector2_t p[4];
	kinc_g2_matrix3x3_multquad(&g2_transform, x, y, width, height, p);
	kinc_g2_colored_rect_set_verts(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);

	++colored_rect_buffer_index;
}

void kinc_g2_draw_rect(float x, float y, float width, float height, float strength) {
	float hs = strength / 2.0f;
	kinc_g2_fill_rect(x - hs, y - hs, width + strength, strength); // Top
	kinc_g2_fill_rect(x - hs, y + hs, strength, height - strength); // Left
	kinc_g2_fill_rect(x - hs, y + height - hs, width + strength, strength); // Bottom
	kinc_g2_fill_rect(x + width - hs, y + hs, strength, height - strength); // Right
}

void kinc_g2_draw_line(float x0, float y0, float x1, float y1, float strength) {
	kinc_vector2_t vec;
	if (y1 == y0) {
		vec = (kinc_vector2_t){0.0f, -1.0f};
	}
	else {
		vec = (kinc_vector2_t){1.0f, -(x1 - x0) / (y1 - y0)};
	}

	float current_length = sqrtf(vec.x * vec.x + vec.y * vec.y);
	if (current_length != 0) {
		float mul = strength / current_length;
		vec.x *= mul;
		vec.y *= mul;
	}

	kinc_vector2_t p0 = (kinc_vector2_t){x0 + 0.5f * vec.x, y0 + 0.5f * vec.y};
	kinc_vector2_t p1 = (kinc_vector2_t){x1 + 0.5f * vec.x, y1 + 0.5f * vec.y};
	kinc_vector2_t p2 = (kinc_vector2_t){p0.x - vec.x, p0.y - vec.y};
	kinc_vector2_t p3 = (kinc_vector2_t){p1.x - vec.x, p1.y - vec.y};
	kinc_g2_fill_triangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
	kinc_g2_fill_triangle(p2.x, p2.y, p1.x, p1.y, p3.x, p3.y);
}

static uint8_t _g2_color_r(uint32_t color) {
	return (color & 0x00ff0000) >> 16;
}

static uint8_t _g2_color_g(uint32_t color) {
	return (color & 0x0000ff00) >>  8;
}

static uint8_t _g2_color_b(uint32_t color) {
	return (color & 0x000000ff);
}

static uint8_t _g2_color_a(uint32_t color) {
	return (color) >> 24;
}

static uint32_t _g2_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (a << 24) | (r << 16) | (g << 8) | b;
}

void kinc_g2_draw_line_aa(float x0, float y0, float x1, float y1, float strength) {
	kinc_g2_draw_line(x0, y0, x1, y1, strength);
	uint32_t _color = g2_color;
	kinc_g2_set_color(_g2_color(_g2_color_r(g2_color), _g2_color_g(g2_color), _g2_color_b(g2_color), 150));
	kinc_g2_draw_line(x0 + 0.5, y0, x1 + 0.5, y1, strength);
	kinc_g2_draw_line(x0 - 0.5, y0, x1 - 0.5, y1, strength);
	kinc_g2_draw_line(x0, y0 + 0.5, x1, y1 + 0.5, strength);
	kinc_g2_draw_line(x0, y0 - 0.5, x1, y1 - 0.5, strength);
	kinc_g2_set_color(_color);
}

void kinc_g2_text_set_rect_verts(float btlx, float btly, float tplx, float tply, float tprx, float tpry, float btrx, float btry) {
	int base_idx = (text_buffer_index - text_buffer_start) * 6 * 4;
	text_rect_verts[base_idx + 0] = btlx;
	text_rect_verts[base_idx + 1] = btly;
	text_rect_verts[base_idx + 2] = -5.0f;

	text_rect_verts[base_idx + 6] = tplx;
	text_rect_verts[base_idx + 7] = tply;
	text_rect_verts[base_idx + 8] = -5.0f;

	text_rect_verts[base_idx + 12] = tprx;
	text_rect_verts[base_idx + 13] = tpry;
	text_rect_verts[base_idx + 14] = -5.0f;

	text_rect_verts[base_idx + 18] = btrx;
	text_rect_verts[base_idx + 19] = btry;
	text_rect_verts[base_idx + 20] = -5.0f;
}

void kinc_g2_text_set_rect_tex_coords(float left, float top, float right, float bottom) {
	int base_idx = (text_buffer_index - text_buffer_start) * 6 * 4;
	text_rect_verts[base_idx + 3] = left;
	text_rect_verts[base_idx + 4] = bottom;

	text_rect_verts[base_idx + 9] = left;
	text_rect_verts[base_idx + 10] = top;

	text_rect_verts[base_idx + 15] = right;
	text_rect_verts[base_idx + 16] = top;

	text_rect_verts[base_idx + 21] = right;
	text_rect_verts[base_idx + 22] = bottom;
}

void kinc_g2_text_set_rect_colors(uint32_t color) {
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	int base_idx = (text_buffer_index - text_buffer_start) * 6 * 4;
	text_rect_verts[base_idx + 5] = *(float *)&color;
	text_rect_verts[base_idx + 11] = *(float *)&color;
	text_rect_verts[base_idx + 17] = *(float *)&color;
	text_rect_verts[base_idx + 23] = *(float *)&color;
}

void kinc_g2_text_draw_buffer(bool end) {
	if (text_buffer_index - text_buffer_start == 0) return;
	kinc_g4_vertex_buffer_unlock(&text_vertex_buffer, text_buffer_index * 4);
	kinc_g4_set_pipeline(g2_custom_pipeline != NULL ? g2_custom_pipeline : g2_is_render_target ? &text_pipeline_rt : &text_pipeline);
	kinc_g4_set_matrix4(text_proj_loc, &g2_projection_matrix);
	kinc_g4_set_vertex_buffer(&text_vertex_buffer);
	kinc_g4_set_index_buffer(&text_index_buffer);
	kinc_g4_set_texture(text_tex_unit, text_last_texture);
	kinc_g4_set_texture_addressing(text_tex_unit, KINC_G4_TEXTURE_DIRECTION_U, KINC_G4_TEXTURE_ADDRESSING_CLAMP);
	kinc_g4_set_texture_addressing(text_tex_unit, KINC_G4_TEXTURE_DIRECTION_V, KINC_G4_TEXTURE_ADDRESSING_CLAMP);
	kinc_g4_set_texture_mipmap_filter(text_tex_unit, KINC_G4_MIPMAP_FILTER_NONE);
	kinc_g4_set_texture_minification_filter(text_tex_unit, g2_bilinear_filter ? KINC_G4_TEXTURE_FILTER_LINEAR : KINC_G4_TEXTURE_FILTER_POINT);
	kinc_g4_set_texture_magnification_filter(text_tex_unit, g2_bilinear_filter ? KINC_G4_TEXTURE_FILTER_LINEAR : KINC_G4_TEXTURE_FILTER_POINT);
	kinc_g4_draw_indexed_vertices_from_to(text_buffer_start * 2 * 3, (text_buffer_index - text_buffer_start) * 2 * 3);

	if (end || text_buffer_index + 1 >= G2_BUFFER_SIZE) {
		text_buffer_start = 0;
		text_buffer_index = 0;
		text_rect_verts = kinc_g4_vertex_buffer_lock(&text_vertex_buffer, 0, G2_BUFFER_SIZE * 4);
	}
	else {
		text_buffer_start = text_buffer_index;
		text_rect_verts = kinc_g4_vertex_buffer_lock(&text_vertex_buffer, text_buffer_start * 4, (G2_BUFFER_SIZE - text_buffer_start) * 4);
	}
}

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

typedef struct kinc_g2_font_aligned_quad {
	float x0, y0, s0, t0; // Top-left
	float x1, y1, s1, t1; // Bottom-right
	float xadvance;
} kinc_g2_font_aligned_quad_t;

typedef struct kinc_g2_font_image {
	float m_size;
	stbtt_bakedchar *chars;
	kinc_g4_texture_t *tex;
	int width, height, first_unused_y;
	float baseline, descent, line_gap;
} kinc_g2_font_image_t;

kinc_g2_font_image_t *kinc_g2_font_get_image_internal(kinc_g2_font_t *font, int size) {
	for (int i = 0; i < font->m_images_len; ++i) {
		if ((int)font->images[i].m_size == size) {
			return &(font->images[i]);
		}
	}
	return NULL;
}

static inline bool kinc_g2_prepare_font_load_internal(kinc_g2_font_t *font, int size) {
	if (kinc_g2_font_get_image_internal(font, size) != NULL) return false; // Nothing to do

	// Resize images array if necessary
	if (font->m_capacity <= font->m_images_len) {
		font->m_capacity = font->m_images_len + 1;
		if (font->images == NULL) {
			font->images = (kinc_g2_font_image_t *)malloc(font->m_capacity * sizeof(kinc_g2_font_image_t));
		}
		else {
			font->images = (kinc_g2_font_image_t *)realloc(font->images, font->m_capacity * sizeof(kinc_g2_font_image_t));
		}
	}

	return true;
}

static int stbtt_BakeFontBitmapArr(unsigned char *data, int offset,        // Font location (use offset=0 for plain .ttf)
								   float pixel_height,                     // Height of font in pixels
								   unsigned char *pixels, int pw, int ph,  // Bitmap to be filled in
								   int* chars, int num_chars,              // Characters to bake
								   stbtt_bakedchar *chardata) {
	float scale;
	int x, y, bottom_y, i;
	stbtt_fontinfo f;
	f.userdata = NULL;
	if (!stbtt_InitFont(&f, data, offset))
		return -1;
	STBTT_memset(pixels, 0, pw * ph); // Background of 0 around pixels
	x = y = 1;
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
		if (y + gh + 1 >= ph) // Check if it fits vertically AFTER potentially moving to next row
			return -i;
		STBTT_assert(x + gw < pw);
		STBTT_assert(y + gh < ph);
		stbtt_MakeGlyphBitmap(&f, pixels + x + y * pw, gw,gh,pw, scale,scale, g);
		chardata[i].x0 = (stbtt_int16)x;
		chardata[i].y0 = (stbtt_int16)y;
		chardata[i].x1 = (stbtt_int16)(x + gw);
		chardata[i].y1 = (stbtt_int16)(y + gh);
		chardata[i].xadvance = scale * advance;
		chardata[i].xoff     = (float)x0;
		chardata[i].yoff     = (float)y0;
		x = x + gw + 1;
		if (y + gh + 1 > bottom_y)
			bottom_y = y + gh + 1;
   }
   return bottom_y;
}

bool kinc_g2_font_load(kinc_g2_font_t *font, int size) {
	if (!kinc_g2_prepare_font_load_internal(font, size)) {
		return true;
	}

	kinc_g2_font_image_t *img = &(font->images[font->m_images_len]);
	font->m_images_len += 1;
	int width = 64;
	int height = 32;
	stbtt_bakedchar *baked = (stbtt_bakedchar *)malloc(g2_font_num_glyphs * sizeof(stbtt_bakedchar));
	unsigned char *pixels = NULL;

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
		status = stbtt_BakeFontBitmapArr(font->blob, font->offset, (float)size, pixels, width, height, g2_font_glyphs, g2_font_num_glyphs, baked);
	}

	stbtt_fontinfo info;
	int ascent, descent, line_gap;
	stbtt_InitFont(&info, font->blob, font->offset);
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	float scale = stbtt_ScaleForPixelHeight(&info, (float)size);
	img->m_size = (float)size;
	img->baseline = (float)((int)((float)ascent * scale + 0.5f));
	img->descent = (float)((int)((float)descent * scale + 0.5f));
	img->line_gap = (float)((int)((float)line_gap * scale + 0.5f));
	img->width = width;
	img->height = height;
	img->chars = baked;
	img->first_unused_y = status;
	kinc_image_t fontimg;
	kinc_image_init_from_bytes(&fontimg, pixels, width, height, KINC_IMAGE_FORMAT_GREY8);
	img->tex = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init_from_image(img->tex, &fontimg);
	kinc_image_destroy(&fontimg);
	free(pixels);
	return true;
}

kinc_g4_texture_t *kinc_g2_font_get_texture(kinc_g2_font_t *font, int size) {
	kinc_g2_font_image_t *img = kinc_g2_font_get_image_internal(font, size);
	return img->tex;
}

int kinc_g2_font_get_char_index_internal(kinc_g2_font_image_t *img, int char_index) {
	if (g2_font_num_glyphs <= 0) {
		return 0;
	}
	int offset = g2_font_glyph_blocks[0];
	if (char_index < offset) {
		return 0;
	}

	for (int i = 1; i < g2_font_num_glyph_blocks / 2; ++i) {
		int prev_end = g2_font_glyph_blocks[i * 2 - 1];
		int start = g2_font_glyph_blocks[i * 2];
		if (char_index > start - 1) {
			offset += start - 1 - prev_end;
		}
	}

	if (char_index - offset >= g2_font_num_glyphs) {
		return 0;
	}
	return char_index - offset;
}

bool kinc_g2_font_get_baked_quad(kinc_g2_font_t *font, int size, kinc_g2_font_aligned_quad_t *q, int char_code, float xpos, float ypos) {
	kinc_g2_font_image_t *img = kinc_g2_font_get_image_internal(font, size);
	int char_index = kinc_g2_font_get_char_index_internal(img, char_code);
	if (char_index >= g2_font_num_glyphs) return false;
	float ipw = 1.0f / (float)img->width;
	float iph = 1.0f / (float)img->height;
	stbtt_bakedchar b = img->chars[char_index];
	int round_x = (int)(xpos + b.xoff + 0.5);
	int round_y = (int)(ypos + b.yoff + 0.5);

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

void kinc_g2_draw_string(const char *text, float x, float y) {
	kinc_g2_image_end();
	kinc_g2_colored_end();

	kinc_g2_font_image_t *img = kinc_g2_font_get_image_internal(g2_font, g2_font_size);
	kinc_g4_texture_t *tex = img->tex;

	if (text_last_texture != NULL && tex != text_last_texture) kinc_g2_text_draw_buffer(false);
	text_last_texture = tex;

	float xpos = x;
	float ypos = y + img->baseline;
	kinc_g2_font_aligned_quad_t q;
	for (int i = 0; text[i] != 0; ) {
		int l = 0;
		int codepoint = string_utf8_decode(&text[i], &l);
		i += l;

		if (kinc_g2_font_get_baked_quad(g2_font, g2_font_size, &q, codepoint, xpos, ypos)) {
			if (text_buffer_index + 1 >= G2_BUFFER_SIZE) kinc_g2_text_draw_buffer(false);
			kinc_g2_text_set_rect_colors(g2_color);
			kinc_g2_text_set_rect_tex_coords(q.s0, q.t0, q.s1, q.t1);

			kinc_vector2_t p[4];
			kinc_g2_matrix3x3_multquad(&g2_transform, q.x0, q.y0, q.x1 - q.x0, q.y1 - q.y0, p);
			kinc_g2_text_set_rect_verts(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);

			xpos += q.xadvance;
			++text_buffer_index;
		}
	}
}

void kinc_g2_font_default_glyphs() {
	g2_font_num_glyphs = 127 - 32;
	g2_font_glyphs = (int *)malloc(g2_font_num_glyphs * sizeof(int));
	for (int i = 32; i < 127; ++i) g2_font_glyphs[i - 32] = i;
	g2_font_num_glyph_blocks = 2;
	g2_font_glyph_blocks = (int *)malloc(g2_font_num_glyph_blocks * sizeof(int));
	g2_font_glyph_blocks[0] = 32;
	g2_font_glyph_blocks[1] = 126;
}

void _kinc_g2_font_init(kinc_g2_font_t *font, void *blob, int font_index) {
	if (g2_font_glyphs == NULL) {
		kinc_g2_font_default_glyphs();
	}

	font->blob = blob;
	font->images = NULL;
	font->m_images_len = 0;
	font->m_capacity = 0;
	font->offset = stbtt_GetFontOffsetForIndex(font->blob, font_index);
	if (font->offset == -1) {
		font->offset = stbtt_GetFontOffsetForIndex(font->blob, 0);
	}
}

kinc_g2_font_t *kinc_g2_font_init(buffer_t *blob, int font_index) {
	kinc_g2_font_t *font = (kinc_g2_font_t *)malloc(sizeof(kinc_g2_font_t));
	_kinc_g2_font_init(font, blob->buffer, font_index);
	return font;
}

void _kinc_g2_font_13(kinc_g2_font_t *font, void *blob) {
	if (g2_font_glyphs == NULL) {
		kinc_g2_font_default_glyphs();
	}

	font->blob = blob;
	font->images = NULL;
	font->m_images_len = 0;
	font->m_capacity = 0;
	font->offset = 0;
	kinc_g2_prepare_font_load_internal(font, 13);

	kinc_g2_font_image_t *img = &(font->images[font->m_images_len]);
	font->m_images_len += 1;
	img->m_size = 13;
	img->baseline = 0; // 10
	img->descent = 0;
	img->line_gap = 0;
	img->width = 128;
	img->height = 128;
	img->first_unused_y = 0;
	kinc_image_t font_img;
	kinc_image_init_from_bytes(&font_img, (void *)g2_font_13_pixels, 128, 128, KINC_IMAGE_FORMAT_GREY8);
	img->tex = (kinc_g4_texture_t *)malloc(sizeof(kinc_g4_texture_t));
	kinc_g4_texture_init_from_image(img->tex, &font_img);
	kinc_image_destroy(&font_img);

	stbtt_bakedchar *baked = (stbtt_bakedchar *)malloc(95 * sizeof(stbtt_bakedchar));
	for (int i = 0; i < 95; ++i) {
		baked[i].x0 = g2_font_13_x0[i];
		baked[i].x1 = g2_font_13_x1[i];
		baked[i].y0 = g2_font_13_y0[i];
		baked[i].y1 = g2_font_13_y1[i];
		baked[i].xoff = g2_font_13_xoff[i];
		baked[i].yoff = g2_font_13_yoff[i];
		baked[i].xadvance = g2_font_13_xadvance[i];
	}
	img->chars = baked;
}

kinc_g2_font_t *kinc_g2_font_13(buffer_t *blob) {
	kinc_g2_font_t *font = (kinc_g2_font_t *)malloc(sizeof(kinc_g2_font_t));
	_kinc_g2_font_13(font, blob->buffer);
	return font;
}

bool kinc_g2_font_has_glyph(int glyph) {
	for (int i = 0; i < g2_font_num_glyphs; ++i) {
		if (g2_font_glyphs[i] == glyph) {
			return true;
		}
	}
	return false;
}

void _kinc_g2_font_set_glyphs(int *glyphs, int count) {
	free(g2_font_glyphs);
	g2_font_num_glyphs = count;
	g2_font_glyphs = (int *)malloc(g2_font_num_glyphs * sizeof(int));

	int blocks = 1;
	g2_font_glyphs[0] = glyphs[0];
	int next_char = glyphs[0] + 1;
	int pos = 1;
	while (pos < count) {
		g2_font_glyphs[pos] = glyphs[pos];
		if (glyphs[pos] != next_char) {
			++blocks;
			next_char = glyphs[pos] + 1;
		}
		else {
			++next_char;
		}
		++pos;
	}

	g2_font_num_glyph_blocks = 2 * blocks;
	g2_font_glyph_blocks = (int *)malloc(g2_font_num_glyph_blocks * sizeof(int));
	g2_font_glyph_blocks[0] = glyphs[0];
	next_char = glyphs[0] + 1;
	pos = 1;
	for (int i = 1; i < count; ++i) {
		if (glyphs[i] != next_char) {
			g2_font_glyph_blocks[pos * 2 - 1] = glyphs[i - 1];
			g2_font_glyph_blocks[pos * 2] = glyphs[i];
			++pos;
			next_char = glyphs[i] + 1;
		}
		else {
			++next_char;
		}
	}
	g2_font_glyph_blocks[blocks * 2 - 1] = glyphs[count - 1];
}

void kinc_g2_font_set_glyphs(i32_array_t *glyphs) {
	_kinc_g2_font_set_glyphs(glyphs->buffer, glyphs->length);
}

void kinc_g2_font_add_glyph(int glyph) {
	// TODO: slow
	g2_font_num_glyphs++;
	int *font_glyphs = (int *)malloc(g2_font_num_glyphs * sizeof(int));
	for (int i = 0; i < g2_font_num_glyphs - 1; ++i) font_glyphs[i] = g2_font_glyphs[i];
	font_glyphs[g2_font_num_glyphs - 1] = glyph;
	_kinc_g2_font_set_glyphs(font_glyphs, g2_font_num_glyphs);
}

int kinc_g2_font_count(kinc_g2_font_t *font) {
	return stbtt_GetNumberOfFonts(font->blob);
}

int kinc_g2_font_height(kinc_g2_font_t *font, int font_size) {
	kinc_g2_font_load(font, font_size);
	return (int)kinc_g2_font_get_image_internal(font, font_size)->m_size;
}

float kinc_g2_font_get_char_width_internal(kinc_g2_font_image_t *img, int char_index) {
	int i = kinc_g2_font_get_char_index_internal(img, char_index);
	return img->chars[i].xadvance;
}

float kinc_g2_sub_string_width(kinc_g2_font_t *font, int font_size, const char *text, int start, int end) {
	kinc_g2_font_load(font, font_size);
	kinc_g2_font_image_t *img = kinc_g2_font_get_image_internal(font, font_size);
	float width = 0.0;
	for (int i = start; i < end; ++i) {
		if (text[i] == '\0') break;
		width += kinc_g2_font_get_char_width_internal(img, text[i]);
	}
	return width;
}

int kinc_g2_string_width(kinc_g2_font_t *font, int font_size, const char *text) {
	return (int)kinc_g2_sub_string_width(font, font_size, text, 0, (int)strlen(text));
}

void kinc_g2_image_end(void) {
	if (image_buffer_index > 0) kinc_g2_draw_image_buffer(true);
	image_last_texture = NULL;
	image_last_render_target = NULL;
}

void kinc_g2_colored_end(void) {
	kinc_g2_colored_rect_end(false);
	kinc_g2_colored_tris_end(false);
}

void kinc_g2_text_end(void) {
	if (text_buffer_index > 0) kinc_g2_text_draw_buffer(true);
	text_last_texture = NULL;
}

void kinc_g2_end(void) {
	kinc_g2_image_end();
	kinc_g2_colored_end();
	kinc_g2_text_end();
}

void kinc_g2_set_color(uint32_t color) {
	g2_color = color;
}

uint32_t kinc_g2_get_color() {
	return g2_color;
}

void kinc_g2_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	if (pipeline == g2_last_pipeline) return;
	g2_last_pipeline = pipeline;
	kinc_g2_end(); // flush
	g2_custom_pipeline = pipeline;
}

void kinc_g2_set_transform(buffer_t *matrix) {
	kinc_matrix3x3_t *m = matrix != NULL ? (kinc_matrix3x3_t *)matrix->buffer : NULL;
	if (m == NULL) {
		g2_transform = kinc_matrix3x3_identity();
	}
	else {
		for (int i = 0; i < 3 * 3; ++i) {
			g2_transform.m[i] = m->m[i];
		}
	}
}

bool kinc_g2_set_font(kinc_g2_font_t *font, int size) {
	kinc_g2_end(); // flush
	g2_font = font;
	g2_font_size = size;
	return kinc_g2_font_load(font, size);
}

void kinc_g2_set_bilinear_filter(bool bilinear) {
	if (g2_bilinear_filter == bilinear) return;
	kinc_g2_end(); // flush
	g2_bilinear_filter = bilinear;
}

void kinc_g2_restore_render_target(void) {
	g2_is_render_target = false;
	kinc_g2_end();
	kinc_g2_begin();
	kinc_g4_restore_render_target();
	kinc_g2_internal_set_projection_matrix(NULL);
}

void kinc_g2_set_render_target(kinc_g4_render_target_t *target) {
	g2_is_render_target = true;
	kinc_g2_end();
	kinc_g2_begin();
	kinc_g4_render_target_t *render_targets[1] = { target };
	kinc_g4_set_render_targets(render_targets, 1);
	kinc_g2_internal_set_projection_matrix(target);
}
