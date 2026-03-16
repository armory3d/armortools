#pragma once

#include "engine.h"
#include "iron_global.h"
#include "iron_math.h"

extern i32             line_draw_color;
extern f32             line_draw_strength;
extern mat4_t          line_draw_mat;
extern vec4_t          line_draw_dim;
extern gpu_buffer_t   *line_draw_vertex_buffer;
extern gpu_buffer_t   *line_draw_index_buffer;
extern gpu_pipeline_t *line_draw_pipeline;
extern gpu_pipeline_t *line_draw_overlay_pipeline;
extern mat4_t          line_draw_vp;
extern i32             line_draw_vp_loc;
extern i32             line_draw_color_loc;
extern buffer_t       *line_draw_vb_data;
extern u32_array_t    *line_draw_ib_data;
extern i32             line_draw_max_lines;
extern i32             line_draw_max_vertices;
extern i32             line_draw_max_indices;
extern i32             line_draw_lines;

void line_draw_init(void);
void line_draw_render(mat4_t matrix);
void line_draw_bounds(mat4_t mat, vec4_t dim);
void line_draw_lineb(i32 a, i32 b, i32 c, i32 d, i32 e, i32 f);
void line_draw_line(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
void line_draw_begin(void);
void line_draw_end(void);

extern gpu_buffer_t *_shape_draw_sphere_vb;
extern gpu_buffer_t *_shape_draw_sphere_ib;

void shape_draw_sphere(mat4_t mat);
