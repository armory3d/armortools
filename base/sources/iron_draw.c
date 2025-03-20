#include "iron_draw.h"
#include <math.h>
#include "const_data.h"
#include "iron_gpu.h"
#include "iron_file.h"
#include "iron_simd.h"
#include "iron_math.h"
#include "iron_system.h"
#include "iron_string.h"
#include "iron_vec2.h"
#include "iron_gc.h"
#ifdef IRON_DIRECT3D12
extern bool waitAfterNextDraw;
#endif

#define MATH_PI 3.14159265358979323846
#define DRAW_BUFFER_SIZE 1000

draw_font_t *draw_font = NULL;
int draw_font_size;
iron_gpu_texture_t *_draw_current = NULL;
bool _draw_in_use = false;

static iron_matrix4x4_t draw_projection_matrix;
static bool draw_bilinear_filter = true;
static uint32_t draw_color = 0;
static iron_gpu_pipeline_t *draw_last_pipeline = NULL;
static iron_gpu_pipeline_t *draw_custom_pipeline = NULL;
static iron_matrix3x3_t draw_transform;
static bool _draw_thrown = false;

static iron_gpu_buffer_t image_vertex_buffer;
static iron_gpu_buffer_t image_index_buffer;
static iron_gpu_shader_t image_vert_shader;
static iron_gpu_shader_t image_frag_shader;
static iron_gpu_pipeline_t image_pipeline;
static iron_gpu_texture_unit_t image_tex_unit;
static iron_gpu_constant_location_t image_proj_loc;
static float *image_rect_verts = NULL;
static int image_buffer_index = 0;
static int image_buffer_start = 0;
static iron_gpu_texture_t *image_last_texture = NULL;

static iron_gpu_buffer_t colored_rect_vertex_buffer;
static iron_gpu_buffer_t colored_rect_index_buffer;
static iron_gpu_buffer_t colored_tris_vertex_buffer;
static iron_gpu_buffer_t colored_tris_index_buffer;
static iron_gpu_shader_t colored_vert_shader;
static iron_gpu_shader_t colored_frag_shader;
static iron_gpu_pipeline_t colored_pipeline;
static iron_gpu_constant_location_t colored_proj_loc;
static float *colored_rect_verts = NULL;
static float *colored_tris_verts = NULL;
static int colored_rect_buffer_index = 0;
static int colored_rect_buffer_start = 0;
static int colored_tris_buffer_index = 0;
static int colored_tris_buffer_start = 0;

static iron_gpu_buffer_t text_vertex_buffer;
static iron_gpu_buffer_t text_index_buffer;
static iron_gpu_shader_t text_vert_shader;
static iron_gpu_shader_t text_frag_shader;
static iron_gpu_pipeline_t text_pipeline;
static iron_gpu_pipeline_t text_pipeline_rt;
static iron_gpu_texture_unit_t text_tex_unit;
static iron_gpu_constant_location_t text_proj_loc;
static float *text_rect_verts = NULL;
static int text_buffer_index = 0;
static int text_buffer_start = 0;
static iron_gpu_texture_t *text_last_texture = NULL;

static i32_array_t *draw_font_glyph_blocks = NULL;
static i32_array_t *draw_font_glyphs = NULL;
static int draw_glyphs_version = 0;

void draw_image_end(void);
void draw_colored_end(void);
void draw_colored_rect_end(bool tris_done);
void draw_colored_tris_end(bool rects_done);
void draw_text_end(void);

iron_matrix4x4_t draw_matrix4x4_orthogonal_projection(float left, float right, float bottom, float top, float zn, float zf) {
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	float tz = -zn / (zf - zn);
	return (iron_matrix4x4_t) {
		2.0f / (right - left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f / (zf - zn), 0.0f,
		tx, ty, tz, 1.0f
    };
}

void draw_matrix3x3_multquad(iron_matrix3x3_t *m, float qx, float qy, float qw, float qh, iron_vector2_t *out) {
	iron_float32x4_t xx = iron_float32x4_load(qx, qx, qx + qw, qx + qw);
	iron_float32x4_t yy = iron_float32x4_load(qy + qh, qy, qy, qy + qh);

	iron_float32x4_t m00 = iron_float32x4_load_all(m->m[0]);
	iron_float32x4_t m01 = iron_float32x4_load_all(m->m[1]);
	iron_float32x4_t m02 = iron_float32x4_load_all(m->m[2]);
	iron_float32x4_t m10 = iron_float32x4_load_all(m->m[3]);
	iron_float32x4_t m11 = iron_float32x4_load_all(m->m[4]);
	iron_float32x4_t m12 = iron_float32x4_load_all(m->m[5]);
	iron_float32x4_t m20 = iron_float32x4_load_all(m->m[6]);
	iron_float32x4_t m21 = iron_float32x4_load_all(m->m[7]);
	iron_float32x4_t m22 = iron_float32x4_load_all(m->m[8]);

	iron_float32x4_t w = iron_float32x4_add(iron_float32x4_add(iron_float32x4_mul(m02, xx), iron_float32x4_mul(m12, yy)), m22);
	iron_float32x4_t px = iron_float32x4_div(iron_float32x4_add(iron_float32x4_add(iron_float32x4_mul(m00, xx), iron_float32x4_mul(m10, yy)), m20), w);
	iron_float32x4_t py = iron_float32x4_div(iron_float32x4_add(iron_float32x4_add(iron_float32x4_mul(m01, xx), iron_float32x4_mul(m11, yy)), m21), w);

	out[0] = (iron_vector2_t){iron_float32x4_get(px, 0), iron_float32x4_get(py, 0)};
	out[1] = (iron_vector2_t){iron_float32x4_get(px, 1), iron_float32x4_get(py, 1)};
	out[2] = (iron_vector2_t){iron_float32x4_get(px, 2), iron_float32x4_get(py, 2)};
	out[3] = (iron_vector2_t){iron_float32x4_get(px, 3), iron_float32x4_get(py, 3)};
}

iron_vector2_t draw_matrix3x3_multvec(iron_matrix3x3_t *m, iron_vector2_t v) {
	float w = m->m[2] * v.x + m->m[5] * v.y + m->m[8] * 1.0f;
	float x = (m->m[0] * v.x + m->m[3] * v.y + m->m[6] * 1.0f) / w;
	float y = (m->m[1] * v.x + m->m[4] * v.y + m->m[7] * 1.0f) / w;
	return (iron_vector2_t){x, y};
}

void draw_internal_set_projection_matrix(iron_gpu_texture_t *target) {
	if (target != NULL) {
		draw_projection_matrix = draw_matrix4x4_orthogonal_projection(0.0f, (float)target->width, (float)target->height, 0.0f, 0.1f, 1000.0f);
	}
	else {
		draw_projection_matrix = draw_matrix4x4_orthogonal_projection(0.0f, (float)iron_window_width(), (float)iron_window_height(), 0.0f, 0.1f, 1000.0f);
	}
}

void draw_init(buffer_t *image_vert, buffer_t *image_frag, buffer_t *colored_vert, buffer_t *colored_frag, buffer_t *text_vert, buffer_t *text_frag) {
	draw_internal_set_projection_matrix(NULL);
	draw_transform = iron_matrix3x3_identity();

	// Image painter
	iron_gpu_shader_init(&image_vert_shader, image_vert->buffer, image_vert->length, IRON_GPU_SHADER_TYPE_VERTEX);
	iron_gpu_shader_init(&image_frag_shader, image_frag->buffer, image_frag->length, IRON_GPU_SHADER_TYPE_FRAGMENT);

	{
		iron_gpu_vertex_structure_t structure;
		iron_gpu_vertex_structure_init(&structure);
		iron_gpu_vertex_structure_add(&structure, "pos", IRON_GPU_VERTEX_DATA_F32_3X);
		iron_gpu_vertex_structure_add(&structure, "tex", IRON_GPU_VERTEX_DATA_F32_2X);
		iron_gpu_vertex_structure_add(&structure, "col", IRON_GPU_VERTEX_DATA_U8_4X_NORM);
		iron_gpu_pipeline_init(&image_pipeline);
		image_pipeline.input_layout = &structure;
		image_pipeline.vertex_shader = &image_vert_shader;
		image_pipeline.fragment_shader = &image_frag_shader;
		image_pipeline.blend_source = IRON_GPU_BLEND_ONE;
		image_pipeline.blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;
		image_pipeline.alpha_blend_source = IRON_GPU_BLEND_ONE;
		image_pipeline.alpha_blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;

		image_pipeline.kong = true;

		iron_gpu_pipeline_compile(&image_pipeline);

		image_tex_unit = iron_gpu_pipeline_get_texture_unit(&image_pipeline, "tex");
		image_tex_unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] = 0;

		image_proj_loc = iron_gpu_pipeline_get_constant_location(&image_pipeline, "P");
		image_proj_loc.impl.vertexOffset = 0;
		image_proj_loc.impl.fragmentOffset = -1;

		gpu_vertex_buffer_init(&image_vertex_buffer, DRAW_BUFFER_SIZE * 4, &structure, GPU_USAGE_DYNAMIC);
		image_rect_verts = gpu_vertex_buffer_lock_all(&image_vertex_buffer);

		iron_gpu_index_buffer_init(&image_index_buffer, DRAW_BUFFER_SIZE * 3 * 2, true);
		int *indices = iron_gpu_index_buffer_lock_all(&image_index_buffer);
		for (int i = 0; i < DRAW_BUFFER_SIZE; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		gpu_index_buffer_unlock_all(&image_index_buffer);
	}

	// Colored painter
	iron_gpu_shader_init(&colored_vert_shader, colored_vert->buffer, colored_vert->length, IRON_GPU_SHADER_TYPE_VERTEX);
	iron_gpu_shader_init(&colored_frag_shader, colored_frag->buffer, colored_frag->length, IRON_GPU_SHADER_TYPE_FRAGMENT);

	{
		iron_gpu_vertex_structure_t structure;
		iron_gpu_vertex_structure_init(&structure);
		iron_gpu_vertex_structure_add(&structure, "pos", IRON_GPU_VERTEX_DATA_F32_3X);
		iron_gpu_vertex_structure_add(&structure, "col", IRON_GPU_VERTEX_DATA_U8_4X_NORM);
		iron_gpu_pipeline_init(&colored_pipeline);
		colored_pipeline.input_layout = &structure;
		colored_pipeline.vertex_shader = &colored_vert_shader;
		colored_pipeline.fragment_shader = &colored_frag_shader;
		// colored_pipeline.blend_source = IRON_GPU_BLEND_ONE;
		colored_pipeline.blend_source = IRON_GPU_BLEND_SOURCE_ALPHA;
		colored_pipeline.blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;
		colored_pipeline.alpha_blend_source = IRON_GPU_BLEND_ONE;
		colored_pipeline.alpha_blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;

		colored_pipeline.kong = true;

		iron_gpu_pipeline_compile(&colored_pipeline);

		colored_proj_loc = iron_gpu_pipeline_get_constant_location(&colored_pipeline, "P");
		colored_proj_loc.impl.vertexOffset = 0;
		colored_proj_loc.impl.fragmentOffset = -1;

		gpu_vertex_buffer_init(&colored_rect_vertex_buffer, DRAW_BUFFER_SIZE * 4, &structure, GPU_USAGE_DYNAMIC);
		colored_rect_verts = gpu_vertex_buffer_lock_all(&colored_rect_vertex_buffer);

		iron_gpu_index_buffer_init(&colored_rect_index_buffer, DRAW_BUFFER_SIZE * 3 * 2, true);
		int *indices = iron_gpu_index_buffer_lock_all(&colored_rect_index_buffer);
		for (int i = 0; i < DRAW_BUFFER_SIZE; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		gpu_index_buffer_unlock_all(&colored_rect_index_buffer);

		gpu_vertex_buffer_init(&colored_tris_vertex_buffer, DRAW_BUFFER_SIZE * 3, &structure, GPU_USAGE_DYNAMIC);
		colored_tris_verts = gpu_vertex_buffer_lock_all(&colored_tris_vertex_buffer);

		iron_gpu_index_buffer_init(&colored_tris_index_buffer, DRAW_BUFFER_SIZE * 3, true);
		indices = iron_gpu_index_buffer_lock_all(&colored_tris_index_buffer);
		for (int i = 0; i < DRAW_BUFFER_SIZE; ++i) {
			indices[i * 3 + 0] = i * 3 + 0;
			indices[i * 3 + 1] = i * 3 + 1;
			indices[i * 3 + 2] = i * 3 + 2;
		}
		gpu_index_buffer_unlock_all(&colored_tris_index_buffer);
	}

	// Text painter
	iron_gpu_shader_init(&text_vert_shader, text_vert->buffer, text_vert->length, IRON_GPU_SHADER_TYPE_VERTEX);
	iron_gpu_shader_init(&text_frag_shader, text_frag->buffer, text_frag->length, IRON_GPU_SHADER_TYPE_FRAGMENT);

	{
		iron_gpu_vertex_structure_t structure;
		iron_gpu_vertex_structure_init(&structure);
		iron_gpu_vertex_structure_add(&structure, "pos", IRON_GPU_VERTEX_DATA_F32_3X);
		iron_gpu_vertex_structure_add(&structure, "tex", IRON_GPU_VERTEX_DATA_F32_2X);
		iron_gpu_vertex_structure_add(&structure, "col", IRON_GPU_VERTEX_DATA_U8_4X_NORM);

		iron_gpu_pipeline_init(&text_pipeline);
		text_pipeline.input_layout = &structure;
		text_pipeline.vertex_shader = &text_vert_shader;
		text_pipeline.fragment_shader = &text_frag_shader;
		text_pipeline.blend_source = IRON_GPU_BLEND_SOURCE_ALPHA;
		text_pipeline.blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;
		text_pipeline.alpha_blend_source = IRON_GPU_BLEND_SOURCE_ALPHA;
		text_pipeline.alpha_blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;

		text_pipeline.kong = true;

		iron_gpu_pipeline_compile(&text_pipeline);

		text_tex_unit = iron_gpu_pipeline_get_texture_unit(&text_pipeline, "tex");
		text_tex_unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] = 0;

		text_proj_loc = iron_gpu_pipeline_get_constant_location(&text_pipeline, "P");
		text_proj_loc.impl.vertexOffset = 0;
		text_proj_loc.impl.fragmentOffset = -1;

		iron_gpu_pipeline_init(&text_pipeline_rt);
		text_pipeline_rt.input_layout = &structure;
		text_pipeline_rt.vertex_shader = &text_vert_shader;
		text_pipeline_rt.fragment_shader = &text_frag_shader;
		text_pipeline_rt.blend_source = IRON_GPU_BLEND_SOURCE_ALPHA;
		text_pipeline_rt.blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;
		text_pipeline_rt.alpha_blend_source = IRON_GPU_BLEND_ONE;
		text_pipeline_rt.alpha_blend_destination = IRON_GPU_BLEND_INV_SOURCE_ALPHA;

		text_pipeline_rt.kong = true;

		iron_gpu_pipeline_compile(&text_pipeline_rt);

		gpu_vertex_buffer_init(&text_vertex_buffer, DRAW_BUFFER_SIZE * 4, &structure, GPU_USAGE_DYNAMIC);
		text_rect_verts = gpu_vertex_buffer_lock_all(&text_vertex_buffer);

		iron_gpu_index_buffer_init(&text_index_buffer, DRAW_BUFFER_SIZE * 3 * 2, true);
		int *indices = iron_gpu_index_buffer_lock_all(&text_index_buffer);
		for (int i = 0; i < DRAW_BUFFER_SIZE; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		gpu_index_buffer_unlock_all(&text_index_buffer);
	}
}

static void _draw_begin() {
	draw_set_color(0xffffffff);
}

void draw_begin(iron_gpu_texture_t *render_target) {
	if (_draw_in_use && !_draw_thrown) {
		_draw_thrown = true;
		iron_log("End before you begin");
	}
	_draw_in_use = true;

	_draw_begin();

	if (render_target != NULL) {
		draw_set_render_target(render_target);
	}
	else {
		draw_restore_render_target();
	}
}

void draw_set_image_rect_verts(float btlx, float btly, float tplx, float tply, float tprx, float tpry, float btrx, float btry) {
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

void draw_set_image_rect_tex_coords(float left, float top, float right, float bottom) {
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

void draw_set_image_rect_colors(uint32_t color) {
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	int base_idx = (image_buffer_index - image_buffer_start) * 6 * 4;
	image_rect_verts[base_idx + 5] = *(float *)&color;
	image_rect_verts[base_idx + 11] = *(float *)&color;
	image_rect_verts[base_idx + 17] = *(float *)&color;
	image_rect_verts[base_idx + 23] = *(float *)&color;
}

void draw_image_buffer(bool end) {
	if (image_buffer_index - image_buffer_start == 0) return;
	iron_gpu_vertex_buffer_unlock(&image_vertex_buffer, (image_buffer_index - image_buffer_start) * 4);
	iron_gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &image_pipeline);
	gpu_set_matrix4(&image_proj_loc, draw_projection_matrix);
	gpu_set_vertex_buffer(&image_vertex_buffer);
	gpu_set_index_buffer(&image_index_buffer);
	gpu_set_texture(&image_tex_unit, image_last_texture);
	gpu_set_texture_addressing(image_tex_unit, GPU_TEXTURE_DIRECTION_U, GPU_TEXTURE_ADDRESSING_CLAMP);
	gpu_set_texture_addressing(image_tex_unit, GPU_TEXTURE_DIRECTION_V, GPU_TEXTURE_ADDRESSING_CLAMP);
	// gpu_set_texture_mipmap_filter(image_tex_unit, draw_bilinear_filter ? GPU_MIPMAP_FILTER_LINEAR : GPU_MIPMAP_FILTER_NONE);
	gpu_set_texture_mipmap_filter(image_tex_unit, GPU_MIPMAP_FILTER_NONE);
	gpu_set_texture_minification_filter(image_tex_unit, draw_bilinear_filter ? GPU_TEXTURE_FILTER_LINEAR : GPU_TEXTURE_FILTER_POINT);
	gpu_set_texture_magnification_filter(image_tex_unit, draw_bilinear_filter ? GPU_TEXTURE_FILTER_LINEAR : GPU_TEXTURE_FILTER_POINT);
	iron_gpu_draw_indexed_vertices_from_to(image_buffer_start * 2 * 3, (image_buffer_index - image_buffer_start) * 2 * 3);

	if (end || image_buffer_index + 1 >= DRAW_BUFFER_SIZE) {
		image_buffer_start = 0;
		image_buffer_index = 0;
		image_rect_verts = gpu_vertex_buffer_lock(&image_vertex_buffer, 0, DRAW_BUFFER_SIZE * 4);
	}
	else {
		image_buffer_start = image_buffer_index;
		image_rect_verts = gpu_vertex_buffer_lock(&image_vertex_buffer, image_buffer_start * 4, (DRAW_BUFFER_SIZE - image_buffer_start) * 4);
	}
}

void draw_scaled_sub_texture(iron_gpu_texture_t *tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	draw_colored_end();
	draw_text_end();
	if (image_buffer_start + image_buffer_index + 1 >= DRAW_BUFFER_SIZE ||
		(image_last_texture != NULL && tex != image_last_texture)) {
		draw_image_buffer(false);
	}

	draw_set_image_rect_tex_coords(sx / tex->width, sy / tex->height, (sx + sw) / tex->width, (sy + sh) / tex->height);
	draw_set_image_rect_colors(draw_color);
	iron_vector2_t p[4];
	draw_matrix3x3_multquad(&draw_transform, dx, dy, dw, dh, p);
	draw_set_image_rect_verts(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);

	++image_buffer_index;
	image_last_texture = tex;
}

void draw_scaled_texture(iron_gpu_texture_t *tex, float dx, float dy, float dw, float dh) {
	draw_scaled_sub_texture(tex, 0, 0, (float)tex->width, (float)tex->height, dx, dy, dw, dh);
}

void draw_sub_texture(iron_gpu_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y) {
	draw_scaled_sub_texture(tex, sx, sy, sw, sh, x, y, sw, sh);
}

void draw_texture(iron_gpu_texture_t *tex, float x, float y) {
	draw_scaled_sub_texture(tex, 0, 0, (float)tex->width, (float)tex->height, x, y, (float)tex->width, (float)tex->height);
}

void draw_scaled_sub_image(iron_gpu_texture_t *image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	#ifdef IRON_DIRECT3D12
	waitAfterNextDraw = true;
	#endif
	draw_scaled_sub_texture(image, sx, sy, sw, sh, dx, dy, dw, dh);
}

void draw_scaled_image(iron_gpu_texture_t *tex, float dx, float dy, float dw, float dh) {
	draw_scaled_sub_image(tex, 0, 0, (float)tex->width, (float)tex->height, dx, dy, dw, dh);
}

void draw_sub_image(iron_gpu_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y) {
	draw_scaled_sub_image(tex, sx, sy, sw, sh, x, y, sw, sh);
}

void draw_image(iron_gpu_texture_t *tex, float x, float y) {
	draw_scaled_sub_image(tex, 0, 0, (float)tex->width, (float)tex->height, x, y, (float)tex->width, (float)tex->height);
}

void draw_colored_rect_set_verts(float btlx, float btly, float tplx, float tply, float tprx, float tpry, float btrx, float btry) {
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

void draw_colored_rect_set_colors(uint32_t color) {
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	int base_idx = (colored_rect_buffer_index - colored_rect_buffer_start) * 4 * 4;
	colored_rect_verts[base_idx + 3] = *(float *)&color;
	colored_rect_verts[base_idx + 7] = *(float *)&color;
	colored_rect_verts[base_idx + 11] = *(float *)&color;
	colored_rect_verts[base_idx + 15] = *(float *)&color;
}

void draw_colored_rect_draw_buffer(bool tris_done) {
	if (colored_rect_buffer_index - colored_rect_buffer_start == 0) {
		return;
	}

	if (!tris_done) draw_colored_tris_end(true);

	iron_gpu_vertex_buffer_unlock(&colored_rect_vertex_buffer, (colored_rect_buffer_index - colored_rect_buffer_start) * 4);
	iron_gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &colored_pipeline);
	gpu_set_matrix4(&colored_proj_loc, draw_projection_matrix);
	gpu_set_vertex_buffer(&colored_rect_vertex_buffer);
	gpu_set_index_buffer(&colored_rect_index_buffer);
	iron_gpu_draw_indexed_vertices_from_to(colored_rect_buffer_start * 2 * 3, (colored_rect_buffer_index - colored_rect_buffer_start) * 2 * 3);

	if (colored_rect_buffer_index + 1 >= DRAW_BUFFER_SIZE) {
		colored_rect_buffer_start = 0;
		colored_rect_buffer_index = 0;
		colored_rect_verts = gpu_vertex_buffer_lock(&colored_rect_vertex_buffer, 0, DRAW_BUFFER_SIZE * 4);
	}
	else {
		colored_rect_buffer_start = colored_rect_buffer_index;
		colored_rect_verts = gpu_vertex_buffer_lock(&colored_rect_vertex_buffer, colored_rect_buffer_start * 4, (DRAW_BUFFER_SIZE - colored_rect_buffer_start) * 4);
	}
}

void draw_colored_tris_set_verts(float x0, float y0, float x1, float y1, float x2, float y2) {
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

void draw_colored_tris_set_colors(uint32_t color) {
	int base_idx = (colored_tris_buffer_index - colored_tris_buffer_start) * 4 * 3;
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	colored_tris_verts[base_idx + 3] = *(float *)&color;
	colored_tris_verts[base_idx + 7] = *(float *)&color;
	colored_tris_verts[base_idx + 11] = *(float *)&color;
}

void draw_colored_tris_draw_buffer(bool rect_done) {
	if (colored_tris_buffer_index - colored_tris_buffer_start == 0) {
		return;
	}

	if (!rect_done) draw_colored_rect_end(true);

	iron_gpu_vertex_buffer_unlock(&colored_tris_vertex_buffer, (colored_tris_buffer_index - colored_tris_buffer_start) * 3);
	iron_gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : &colored_pipeline);
	gpu_set_matrix4(&colored_proj_loc, draw_projection_matrix);
	gpu_set_vertex_buffer(&colored_tris_vertex_buffer);
	gpu_set_index_buffer(&colored_tris_index_buffer);
	iron_gpu_draw_indexed_vertices_from_to(colored_tris_buffer_start * 3, (colored_tris_buffer_index - colored_tris_buffer_start) * 3);

	if (colored_tris_buffer_index + 1 >= DRAW_BUFFER_SIZE) {
		colored_tris_buffer_start = 0;
		colored_tris_buffer_index = 0;
		colored_tris_verts = gpu_vertex_buffer_lock(&colored_tris_vertex_buffer, 0, DRAW_BUFFER_SIZE * 3);
	}
	else {
		colored_tris_buffer_start = colored_tris_buffer_index;
		colored_tris_verts = gpu_vertex_buffer_lock(&colored_tris_vertex_buffer, colored_tris_buffer_start * 3, (DRAW_BUFFER_SIZE - colored_tris_buffer_start) * 3);
	}
}

void draw_colored_rect_end(bool tris_done) {
	if (colored_rect_buffer_index - colored_rect_buffer_start > 0) draw_colored_rect_draw_buffer(tris_done);
}

void draw_colored_tris_end(bool rects_done) {
	if (colored_tris_buffer_index - colored_tris_buffer_start > 0) draw_colored_tris_draw_buffer(rects_done);
}

void draw_filled_triangle(float x0, float y0, float x1, float y1, float x2, float y2) {
	draw_image_end();
	draw_text_end();
	if (colored_rect_buffer_index - colored_rect_buffer_start > 0) draw_colored_rect_draw_buffer(true);
	if (colored_tris_buffer_index + 1 >= DRAW_BUFFER_SIZE) draw_colored_tris_draw_buffer(false);

	draw_colored_tris_set_colors(draw_color);

	iron_vector2_t p0 = draw_matrix3x3_multvec(&draw_transform, (iron_vector2_t){x0, y0}); // Bottom-left
	iron_vector2_t p1 = draw_matrix3x3_multvec(&draw_transform, (iron_vector2_t){x1, y1}); // Top-left
	iron_vector2_t p2 = draw_matrix3x3_multvec(&draw_transform, (iron_vector2_t){x2, y2}); // Top-right
	draw_colored_tris_set_verts(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);

	++colored_tris_buffer_index;
}

void draw_filled_rect(float x, float y, float width, float height) {
	draw_image_end();
	draw_text_end();
	if (colored_tris_buffer_index - colored_tris_buffer_start > 0) draw_colored_tris_draw_buffer(true);
	if (colored_rect_buffer_index + 1 >= DRAW_BUFFER_SIZE) draw_colored_rect_draw_buffer(false);

	draw_colored_rect_set_colors(draw_color);

	iron_vector2_t p[4];
	draw_matrix3x3_multquad(&draw_transform, x, y, width, height, p);
	draw_colored_rect_set_verts(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);

	++colored_rect_buffer_index;
}

void draw_rect(float x, float y, float width, float height, float strength) {
	float hs = strength / 2.0f;
	draw_filled_rect(x - hs, y - hs, width + strength, strength); // Top
	draw_filled_rect(x - hs, y + hs, strength, height - strength); // Left
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

static uint8_t _draw_color_r(uint32_t color) {
	return (color & 0x00ff0000) >> 16;
}

static uint8_t _draw_color_g(uint32_t color) {
	return (color & 0x0000ff00) >>  8;
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

void draw_text_set_rect_verts(float btlx, float btly, float tplx, float tply, float tprx, float tpry, float btrx, float btry) {
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

void draw_text_set_rect_tex_coords(float left, float top, float right, float bottom) {
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

void draw_text_set_rect_colors(uint32_t color) {
	color = (color & 0xff000000) | ((color & 0x00ff0000) >> 16) | (color & 0x0000ff00) | ((color & 0x000000ff) << 16);
	int base_idx = (text_buffer_index - text_buffer_start) * 6 * 4;
	text_rect_verts[base_idx + 5] = *(float *)&color;
	text_rect_verts[base_idx + 11] = *(float *)&color;
	text_rect_verts[base_idx + 17] = *(float *)&color;
	text_rect_verts[base_idx + 23] = *(float *)&color;
}

void draw_text_draw_buffer(bool end) {
	if (text_buffer_index - text_buffer_start == 0) return;
	iron_gpu_vertex_buffer_unlock(&text_vertex_buffer, text_buffer_index * 4);
	iron_gpu_set_pipeline(draw_custom_pipeline != NULL ? draw_custom_pipeline : _draw_current != NULL ? &text_pipeline_rt : &text_pipeline);
	gpu_set_matrix4(&text_proj_loc, draw_projection_matrix);
	gpu_set_vertex_buffer(&text_vertex_buffer);
	gpu_set_index_buffer(&text_index_buffer);
	gpu_set_texture(&text_tex_unit, text_last_texture);
	gpu_set_texture_addressing(text_tex_unit, GPU_TEXTURE_DIRECTION_U, GPU_TEXTURE_ADDRESSING_CLAMP);
	gpu_set_texture_addressing(text_tex_unit, GPU_TEXTURE_DIRECTION_V, GPU_TEXTURE_ADDRESSING_CLAMP);
	gpu_set_texture_mipmap_filter(text_tex_unit, GPU_MIPMAP_FILTER_NONE);
	gpu_set_texture_minification_filter(text_tex_unit, draw_bilinear_filter ? GPU_TEXTURE_FILTER_LINEAR : GPU_TEXTURE_FILTER_POINT);
	gpu_set_texture_magnification_filter(text_tex_unit, draw_bilinear_filter ? GPU_TEXTURE_FILTER_LINEAR : GPU_TEXTURE_FILTER_POINT);
	iron_gpu_draw_indexed_vertices_from_to(text_buffer_start * 2 * 3, (text_buffer_index - text_buffer_start) * 2 * 3);

	if (end || text_buffer_index + 1 >= DRAW_BUFFER_SIZE) {
		text_buffer_start = 0;
		text_buffer_index = 0;
		text_rect_verts = gpu_vertex_buffer_lock(&text_vertex_buffer, 0, DRAW_BUFFER_SIZE * 4);
	}
	else {
		text_buffer_start = text_buffer_index;
		text_rect_verts = gpu_vertex_buffer_lock(&text_vertex_buffer, text_buffer_start * 4, (DRAW_BUFFER_SIZE - text_buffer_start) * 4);
	}
}

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

typedef struct draw_font_aligned_quad {
	float x0, y0, s0, t0; // Top-left
	float x1, y1, s1, t1; // Bottom-right
	float xadvance;
} draw_font_aligned_quad_t;

typedef struct draw_font_image {
	float m_size;
	stbtt_bakedchar *chars;
	iron_gpu_texture_t *tex;
	int width, height, first_unused_y;
	float baseline, descent, line_gap;
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

bool draw_font_load(draw_font_t *font, int size) {
	if (!draw_prepare_font_load_internal(font, size)) {
		return true;
	}

	draw_font_image_t *img = &(font->images[font->m_images_len]);
	font->m_images_len += 1;
	int width = 64;
	int height = 32;
	stbtt_bakedchar *baked = (stbtt_bakedchar *)malloc(draw_font_glyphs->length * sizeof(stbtt_bakedchar));
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
		status = stbtt_BakeFontBitmapArr(
			font->blob, font->offset, (float)size, pixels, width, height,
			draw_font_glyphs->buffer, draw_font_glyphs->length, baked);
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
	img->tex = (iron_gpu_texture_t *)malloc(sizeof(iron_gpu_texture_t));
	iron_gpu_texture_init_from_bytes(img->tex, pixels, width, height, IRON_IMAGE_FORMAT_R8);
	free(pixels);
	return true;
}

iron_gpu_texture_t *draw_font_get_texture(draw_font_t *font, int size) {
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
		int start = draw_font_glyph_blocks->buffer[i * 2];
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
	draw_font_image_t *img = draw_font_get_image_internal(font, size);
	int char_index = draw_font_get_char_index_internal(char_code);
	if (char_index >= draw_font_glyphs->length) {
		return false;
	}
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

void draw_string(const char *text, float x, float y) {
	draw_image_end();
	draw_colored_end();

	draw_font_image_t *img = draw_font_get_image_internal(draw_font, draw_font_size);
	iron_gpu_texture_t *tex = img->tex;

	if (text_last_texture != NULL && tex != text_last_texture) draw_text_draw_buffer(false);
	text_last_texture = tex;

	float xpos = x;
	float ypos = y + img->baseline;
	draw_font_aligned_quad_t q;
	for (int i = 0; text[i] != 0; ) {
		int l = 0;
		int codepoint = string_utf8_decode(&text[i], &l);
		i += l;

		if (draw_font_get_baked_quad(draw_font, draw_font_size, &q, codepoint, xpos, ypos)) {
			if (text_buffer_index + 1 >= DRAW_BUFFER_SIZE) draw_text_draw_buffer(false);
			draw_text_set_rect_colors(draw_color);
			draw_text_set_rect_tex_coords(q.s0, q.t0, q.s1, q.t1);

			iron_vector2_t p[4];
			draw_matrix3x3_multquad(&draw_transform, q.x0, q.y0, q.x1 - q.x0, q.y1 - q.y0, p);
			draw_text_set_rect_verts(p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);

			xpos += q.xadvance;
			++text_buffer_index;
		}
	}
}

void draw_font_init(draw_font_t *font) {
	if (draw_font_glyphs == NULL) {
		draw_font_init_glyphs(32, 127);
	}

	if (font->glyphs_version != draw_glyphs_version) {
		font->glyphs_version = draw_glyphs_version;
		font->blob = font->buf->buffer;
		font->images = NULL;
		font->m_images_len = 0;
		font->m_capacity = 0;
		font->offset = stbtt_GetFontOffsetForIndex(font->blob, font->index);
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

	font->blob = font->buf->buffer;
	font->images = NULL;
	font->m_images_len = 0;
	font->m_capacity = 0;
	font->offset = 0;
	draw_prepare_font_load_internal(font, 13);

	draw_font_image_t *img = &(font->images[font->m_images_len]);
	font->m_images_len += 1;
	img->m_size = 13;
	img->baseline = 0; // 10
	img->descent = 0;
	img->line_gap = 0;
	img->width = 128;
	img->height = 128;
	img->first_unused_y = 0;
	img->tex = (iron_gpu_texture_t *)malloc(sizeof(iron_gpu_texture_t));
	iron_gpu_texture_init_from_bytes(img->tex, (void *)iron_font_13_pixels, 128, 128, IRON_IMAGE_FORMAT_R8);

	stbtt_bakedchar *baked = (stbtt_bakedchar *)malloc(95 * sizeof(stbtt_bakedchar));
	for (int i = 0; i < 95; ++i) {
		baked[i].x0 = iron_font_13_x0[i];
		baked[i].x1 = iron_font_13_x1[i];
		baked[i].y0 = iron_font_13_y0[i];
		baked[i].y1 = iron_font_13_y1[i];
		baked[i].xoff = iron_font_13_xoff[i];
		baked[i].yoff = iron_font_13_yoff[i];
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
	draw_font_glyph_blocks = i32_array_create(2);
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

	int blocks = 1;
	int next_char = draw_font_glyphs->buffer[0] + 1;
	int pos = 1;
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
	next_char = draw_font_glyphs->buffer[0] + 1;
	pos = 1;
	for (int i = 1; i < draw_font_glyphs->length; ++i) {
		if (draw_font_glyphs->buffer[i] != next_char) {
			draw_font_glyph_blocks->buffer[pos * 2 - 1] = draw_font_glyphs->buffer[i - 1];
			draw_font_glyph_blocks->buffer[pos * 2] = draw_font_glyphs->buffer[i];
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
	draw_font_image_t *img = draw_font_get_image_internal(font, font_size);
	float width = 0.0;
	for (int i = start; i < end; ++i) {
		if (text[i] == '\0') break;
		width += draw_font_get_char_width_internal(img, text[i]);
	}
	return width;
}

int draw_string_width(draw_font_t *font, int font_size, const char *text) {
	return (int)draw_sub_string_width(font, font_size, text, 0, (int)strlen(text));
}

void draw_image_end(void) {
	if (image_buffer_index > 0) draw_image_buffer(true);
	image_last_texture = NULL;
}

void draw_colored_end(void) {
	draw_colored_rect_end(false);
	draw_colored_tris_end(false);
}

void draw_text_end(void) {
	if (text_buffer_index > 0) draw_text_draw_buffer(true);
	text_last_texture = NULL;
}

static void _draw_end(void) {
	draw_image_end();
	draw_colored_end();
	draw_text_end();
}

void draw_end(void) {
	if (!_draw_in_use && !_draw_thrown) {
		_draw_thrown = true;
		iron_log("Begin before you end");
	}
	_draw_in_use = false;
	_draw_end();
}

void draw_set_color(uint32_t color) {
	draw_color = color;
}

uint32_t draw_get_color() {
	return draw_color;
}

void draw_set_pipeline(iron_gpu_pipeline_t *pipeline) {
	if (pipeline == draw_last_pipeline) {
		return;
	}
	draw_last_pipeline = pipeline;
	_draw_end(); // flush
	draw_custom_pipeline = pipeline;
}

void draw_set_transform(mat3_t matrix) {
	if (mat3_isnan(matrix)) {
		draw_transform = iron_matrix3x3_identity();
	}
	else {
		draw_transform = matrix;
	}
}

bool draw_set_font(draw_font_t *font, int size) {
	_draw_end(); // flush
	draw_font = font;
	draw_font_size = size;
	return draw_font_load(font, size);
}

void draw_set_bilinear_filter(bool bilinear) {
	if (draw_bilinear_filter == bilinear) {
		return;
	}
	_draw_end(); // flush
	draw_bilinear_filter = bilinear;
}

void draw_restore_render_target(void) {
	_draw_current = NULL;
	_draw_end();
	_draw_begin();
	gpu_restore_render_target();
	draw_internal_set_projection_matrix(NULL);
}

void draw_set_render_target(iron_gpu_texture_t *target) {
	_draw_current = target;
	_draw_end();
	_draw_begin();
	iron_gpu_texture_t *render_targets[1] = { target };
	gpu_set_render_targets(render_targets, 1);
	draw_internal_set_projection_matrix(target);
}

void draw_filled_circle(float cx, float cy, float radius, int segments) {
	if (segments <= 0) {
		segments = (int)floor(10 * sqrtf(radius));
	}

	float theta = 2.0 * (float)MATH_PI / segments;
	float c = cosf(theta);
	float s = sinf(theta);

	float x = radius;
	float y = 0.0;

	for (int n = 0; n < segments; ++n) {
		float px = x + cx;
		float py = y + cy;

		float t = x;
		x = c * x - s * y;
		y = c * y + s * t;

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
	vec = vec2_set_len(vec, strength);
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
	float c = cosf(theta);
	float s = sinf(theta);

	float x = radius;
	float y = 0.0;

	for (int n = 0; n < segments; ++n) {
		float px = x + cx;
		float py = y + cy;

		float t = x;
		x = c * x - s * y;
		y = c * y + s * t;

		draw_inner_line(x + cx, y + cy, px, py, strength);
	}
}

void draw_calculate_cubic_bezier_point(float t, float *x, float *y, float *out) {
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
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

/**
 * Draws a cubic bezier using 4 pairs of points. If the x and y arrays have a length bigger than 4, the additional
 * points will be ignored. With a length smaller of 4 a error will occur, there is no check for this.
 * You can construct the curves visually in Inkscape with a path using default nodes.
 * Provide x and y in the following order: startPoint, controlPoint1, controlPoint2, endPoint
 * Reference: http://devmag.org.za/2011/04/05/bzier-curves-a-tutorial/
 */
void draw_cubic_bezier(f32_array_t *xa, f32_array_t *ya, int segments, float strength) {
	float *x = xa->buffer;
	float *y = ya->buffer;
	float q0[2];
	float q1[2];
	draw_calculate_cubic_bezier_point(0, x, y, q0);

	for (int i = 1; i < (segments + 1); ++i) {
		float t = (float)i / segments;
		draw_calculate_cubic_bezier_point(t, x, y, q1);
		draw_line(q0[0], q0[1], q1[0], q1[1], strength);
		q0[0] = q1[0];
		q0[1] = q1[1];
	}
}
