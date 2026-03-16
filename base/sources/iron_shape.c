#include "iron_shape.h"

#include "iron_array.h"
#include "iron_math.h"

gpu_shader_t *sys_get_shader(char *name);
object_t     *scene_get_child(char *name);
i32           shader_context_type_size(char *t);
i32           shader_context_type_pad(i32 offset, i32 size);

extern i32              pipes_offset;
extern camera_object_t *scene_camera;

i32 pipes_get_constant_location(char *type);
// i32 pipes_get_constant_location(char *type) {
// 	i32 size = shader_context_type_size(type);
// 	pipes_offset += shader_context_type_pad(pipes_offset, size);
// 	i32 loc = pipes_offset;
// 	pipes_offset += size;
// 	return loc;
// }

u8 color_get_rb(i32 c);
u8 color_get_gb(i32 c);
u8 color_get_bb(i32 c);

gpu_buffer_t   *gpu_create_vertex_buffer(i32 count, gpu_vertex_structure_t *structure);
gpu_buffer_t   *gpu_create_index_buffer(i32 count);
buffer_t       *gpu_lock_vertex_buffer(gpu_buffer_t *buffer);
u32_array_t    *gpu_lock_index_buffer(gpu_buffer_t *buffer);
void            gpu_vertex_buffer_unlock(gpu_buffer_t *buffer);
void            gpu_index_buffer_unlock(gpu_buffer_t *buffer);
void            gpu_pipeline_compile(gpu_pipeline_t *pipeline);
gpu_pipeline_t *gpu_create_pipeline(void);
void            gpu_set_pipeline(gpu_pipeline_t *pipeline);
void            gpu_set_vertex_buffer(gpu_buffer_t *buffer);
void            gpu_set_index_buffer(gpu_buffer_t *buffer);
void            gpu_set_mat4(i32 location, mat4_t value);
void            gpu_set_float3(i32 location, f32 value1, f32 value2, f32 value3);
void            gpu_draw(void);
void            gpu_vertex_structure_add(gpu_vertex_structure_t *structure, const char *name, gpu_vertex_data_t data);
void            buffer_set_f32(buffer_t *b, u32 p, f32 n);

i32             line_draw_color    = 0xffff0000;
f32             line_draw_strength = 0.002;
mat4_t          line_draw_mat;
vec4_t          line_draw_dim;
gpu_buffer_t   *line_draw_vertex_buffer    = NULL;
gpu_buffer_t   *line_draw_index_buffer     = NULL;
gpu_pipeline_t *line_draw_pipeline         = NULL;
gpu_pipeline_t *line_draw_overlay_pipeline = NULL;
mat4_t          line_draw_vp;
i32             line_draw_vp_loc       = 0;
i32             line_draw_color_loc    = 0;
buffer_t       *line_draw_vb_data      = NULL;
u32_array_t    *line_draw_ib_data      = NULL;
i32             line_draw_max_lines    = 300;
i32             line_draw_max_vertices = 300 * 4;
i32             line_draw_max_indices  = 300 * 6;
i32             line_draw_lines        = 0;

static vec4_t line_draw_wpos;
static vec4_t line_draw_vx = {0};
static vec4_t line_draw_vy = {0};
static vec4_t line_draw_vz = {0};

static vec4_t line_draw_v1 = {0};
static vec4_t line_draw_v2 = {0};
static vec4_t line_draw_t  = {0};

static vec4_t line_draw_mid_point   = {0};
static vec4_t line_draw_mid_line    = {0};
static vec4_t line_draw_corner1     = {0};
static vec4_t line_draw_corner2     = {0};
static vec4_t line_draw_corner3     = {0};
static vec4_t line_draw_corner4     = {0};
static vec4_t line_draw_camera_look = {0};

gpu_buffer_t *_shape_draw_sphere_vb = NULL;
gpu_buffer_t *_shape_draw_sphere_ib = NULL;

void line_draw_init(void) {
	if (line_draw_pipeline == NULL) {
		gpu_vertex_structure_t structure = {0};
		gpu_vertex_structure_add(&structure, "pos", GPU_VERTEX_DATA_F32_3X);
		line_draw_pipeline                         = gpu_create_pipeline();
		line_draw_pipeline->input_layout           = &structure;
		line_draw_pipeline->fragment_shader        = sys_get_shader("line.frag");
		line_draw_pipeline->vertex_shader          = sys_get_shader("line.vert");
		line_draw_pipeline->depth_write            = true;
		line_draw_pipeline->depth_mode             = GPU_COMPARE_MODE_LESS;
		line_draw_pipeline->cull_mode              = GPU_CULL_MODE_NONE;
		line_draw_pipeline->color_attachment_count = 2;
		line_draw_pipeline->color_attachment[0]    = GPU_TEXTURE_FORMAT_RGBA64;
		line_draw_pipeline->color_attachment[1]    = GPU_TEXTURE_FORMAT_RGBA64;
		line_draw_pipeline->depth_attachment_bits  = 32;
		gpu_pipeline_compile(line_draw_pipeline);
		pipes_offset            = 0;
		line_draw_vp_loc        = pipes_get_constant_location("mat4");
		line_draw_color_loc     = pipes_get_constant_location("vec3");
		line_draw_vp            = mat4_identity();
		line_draw_vertex_buffer = gpu_create_vertex_buffer(line_draw_max_vertices, &structure);
		line_draw_index_buffer  = gpu_create_index_buffer(line_draw_max_indices);
	}
	if (line_draw_overlay_pipeline == NULL) {
		gpu_vertex_structure_t structure = {0};
		gpu_vertex_structure_add(&structure, "pos", GPU_VERTEX_DATA_F32_3X);
		line_draw_overlay_pipeline                         = gpu_create_pipeline();
		line_draw_overlay_pipeline->input_layout           = &structure;
		line_draw_overlay_pipeline->fragment_shader        = sys_get_shader("line_overlay.frag");
		line_draw_overlay_pipeline->vertex_shader          = sys_get_shader("line_overlay.vert");
		line_draw_overlay_pipeline->depth_write            = false;
		line_draw_overlay_pipeline->depth_mode             = GPU_COMPARE_MODE_ALWAYS;
		line_draw_overlay_pipeline->cull_mode              = GPU_CULL_MODE_NONE;
		line_draw_overlay_pipeline->color_attachment_count = 1;
		line_draw_overlay_pipeline->color_attachment[0]    = GPU_TEXTURE_FORMAT_RGBA64;
		gpu_pipeline_compile(line_draw_overlay_pipeline);
	}
}

void line_draw_render(mat4_t matrix) {
	line_draw_mat = matrix;
	line_draw_dim = mat4_get_scale(matrix);

	line_draw_begin();
	line_draw_bounds(line_draw_mat, line_draw_dim);
	line_draw_end();
}

void line_draw_bounds(mat4_t mat, vec4_t dim) {
	line_draw_wpos = mat4_get_loc(mat);
	f32 dx         = dim.x / 2;
	f32 dy         = dim.y / 2;
	f32 dz         = dim.z / 2;

	vec4_t up    = mat4_up(mat);
	vec4_t look  = mat4_look(mat);
	vec4_t right = mat4_right(mat);
	up           = vec4_norm(up);
	look         = vec4_norm(look);
	right        = vec4_norm(right);

	line_draw_vx = vec4_mult(right, dx);
	line_draw_vy = vec4_mult(look, dy);
	line_draw_vz = vec4_mult(up, dz);

	line_draw_lineb(-1, -1, -1, 1, -1, -1);
	line_draw_lineb(-1, 1, -1, 1, 1, -1);
	line_draw_lineb(-1, -1, 1, 1, -1, 1);
	line_draw_lineb(-1, 1, 1, 1, 1, 1);

	line_draw_lineb(-1, -1, -1, -1, 1, -1);
	line_draw_lineb(-1, -1, 1, -1, 1, 1);
	line_draw_lineb(1, -1, -1, 1, 1, -1);
	line_draw_lineb(1, -1, 1, 1, 1, 1);

	line_draw_lineb(-1, -1, -1, -1, -1, 1);
	line_draw_lineb(-1, 1, -1, -1, 1, 1);
	line_draw_lineb(1, -1, -1, 1, -1, 1);
	line_draw_lineb(1, 1, -1, 1, 1, 1);
}

void line_draw_lineb(i32 a, i32 b, i32 c, i32 d, i32 e, i32 f) {
	line_draw_v1 = vec4_clone(line_draw_wpos);

	line_draw_t  = vec4_mult(line_draw_vx, (f32)a);
	line_draw_v1 = vec4_add(line_draw_v1, line_draw_t);

	line_draw_t  = vec4_mult(line_draw_vy, (f32)b);
	line_draw_v1 = vec4_add(line_draw_v1, line_draw_t);

	line_draw_t  = vec4_mult(line_draw_vz, (f32)c);
	line_draw_v1 = vec4_add(line_draw_v1, line_draw_t);

	line_draw_v2 = vec4_clone(line_draw_wpos);

	line_draw_t  = vec4_mult(line_draw_vx, (f32)d);
	line_draw_v2 = vec4_add(line_draw_v2, line_draw_t);

	line_draw_t  = vec4_mult(line_draw_vy, (f32)e);
	line_draw_v2 = vec4_add(line_draw_v2, line_draw_t);

	line_draw_t  = vec4_mult(line_draw_vz, (f32)f);
	line_draw_v2 = vec4_add(line_draw_v2, line_draw_t);

	line_draw_line(line_draw_v1.x, line_draw_v1.y, line_draw_v1.z, line_draw_v2.x, line_draw_v2.y, line_draw_v2.z);
}

void line_draw_line(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
	if (line_draw_lines >= line_draw_max_lines) {
		line_draw_end();
		line_draw_begin();
	}

	line_draw_mid_point = vec4_mult(vec4_create(x1 + x2, y1 + y2, z1 + z2, 0.0), 0.5);

	line_draw_mid_line = vec4_sub(vec4_create(x1, y1, z1, 0.0), line_draw_mid_point);

	line_draw_camera_look = vec4_sub(mat4_get_loc(scene_camera->base->transform->world), line_draw_mid_point);

	vec4_t line_width = vec4_cross(line_draw_camera_look, line_draw_mid_line);
	line_width        = vec4_norm(line_width);
	line_width        = vec4_mult(line_width, line_draw_strength);

	line_draw_corner1 = vec4_add(vec4_create(x1, y1, z1, 0.0), line_width);
	line_draw_corner2 = vec4_sub(vec4_create(x1, y1, z1, 0.0), line_width);
	line_draw_corner3 = vec4_sub(vec4_create(x2, y2, z2, 0.0), line_width);
	line_draw_corner4 = vec4_add(vec4_create(x2, y2, z2, 0.0), line_width);

	i32 i = line_draw_lines * 12; // 4 * 3 (structure len)

	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner1.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner1.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner1.z);

	i += 3;
	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner2.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner2.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner2.z);

	i += 3;
	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner3.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner3.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner3.z);

	i += 3;
	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner4.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner4.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner4.z);

	i                                = line_draw_lines * 6;
	line_draw_ib_data->buffer[i]     = line_draw_lines * 4;
	line_draw_ib_data->buffer[i + 1] = line_draw_lines * 4 + 1;
	line_draw_ib_data->buffer[i + 2] = line_draw_lines * 4 + 2;
	line_draw_ib_data->buffer[i + 3] = line_draw_lines * 4 + 2;
	line_draw_ib_data->buffer[i + 4] = line_draw_lines * 4 + 3;
	line_draw_ib_data->buffer[i + 5] = line_draw_lines * 4;

	line_draw_lines++;
}

void line_draw_begin(void) {
	line_draw_init();
	line_draw_lines   = 0;
	line_draw_vb_data = gpu_lock_vertex_buffer(line_draw_vertex_buffer);
	line_draw_ib_data = gpu_lock_index_buffer(line_draw_index_buffer);
	for (i32 i = 0; i < line_draw_max_indices; ++i) {
		line_draw_ib_data->buffer[i] = 0;
	}
}

void line_draw_end(void) {
	gpu_vertex_buffer_unlock(line_draw_vertex_buffer);
	gpu_index_buffer_unlock(line_draw_index_buffer);

	gpu_set_vertex_buffer(line_draw_vertex_buffer);
	gpu_set_index_buffer(line_draw_index_buffer);
	gpu_set_pipeline(line_draw_pipeline);
	line_draw_vp = mat4_clone(scene_camera->vp);
	gpu_set_mat4(line_draw_vp_loc, line_draw_vp);
	gpu_set_float3(line_draw_color_loc, color_get_rb(line_draw_color) / 255.0, color_get_gb(line_draw_color) / 255.0, color_get_bb(line_draw_color) / 255.0);
	gpu_draw();
}

void shape_draw_sphere(mat4_t mat) {
	line_draw_init();

	if (_shape_draw_sphere_vb == NULL) {
		object_t      *child  = scene_get_child(".Sphere");
		mesh_object_t *sphere = (mesh_object_t *)child->ext;
		mesh_data_t   *md     = sphere->data;

		i16_array_t           *posa      = ((vertex_array_t *)md->vertex_arrays->buffer[0])->values;
		gpu_vertex_structure_t structure = {0};
		gpu_vertex_structure_add(&structure, "pos", GPU_VERTEX_DATA_F32_3X);
		_shape_draw_sphere_vb = gpu_create_vertex_buffer(posa->length, &structure);
		buffer_t *data        = gpu_lock_vertex_buffer(_shape_draw_sphere_vb);
		for (i32 i = 0; i < (i32)posa->length / 4; ++i) {
			buffer_set_f32(data, (i * 3 + 0) * 4, posa->buffer[i * 4 + 0] / 32767.0);
			buffer_set_f32(data, (i * 3 + 1) * 4, posa->buffer[i * 4 + 1] / 32767.0);
			buffer_set_f32(data, (i * 3 + 2) * 4, posa->buffer[i * 4 + 2] / 32767.0);
		}
		gpu_vertex_buffer_unlock(_shape_draw_sphere_vb);
		_shape_draw_sphere_ib = md->_->index_buffer;
	}

	gpu_set_vertex_buffer(_shape_draw_sphere_vb);
	gpu_set_index_buffer(_shape_draw_sphere_ib);
	gpu_set_pipeline(line_draw_overlay_pipeline);
	f32 f        = line_draw_strength * 50;
	line_draw_vp = mat4_clone(mat);
	line_draw_vp = mat4_scale(line_draw_vp, vec4_create(f, f, f, 0.0));
	line_draw_vp = mat4_mult_mat(line_draw_vp, scene_camera->v);
	line_draw_vp = mat4_mult_mat(line_draw_vp, scene_camera->p);
	gpu_set_mat4(line_draw_vp_loc, line_draw_vp);
	gpu_set_float3(line_draw_color_loc, color_get_rb(line_draw_color) / 255.0, color_get_gb(line_draw_color) / 255.0, color_get_bb(line_draw_color) / 255.0);
	gpu_draw();
}
