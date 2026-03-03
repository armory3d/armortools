#pragma once

#include "iron_armpack.h"
#include "iron_array.h"
#include "iron_gc.h"
#include "iron_gpu.h"
#include "iron_mat4.h"
#include "iron_quat.h"
#include "iron_string.h"
#include "iron_system.h"
#include "iron_vec4.h"
#include <math.h>
#include <string.h>

typedef struct object object_t;

typedef struct _obj_t_array {
	struct obj **buffer;
	int          length;
	int          capacity;
} _obj_t_array_t;
typedef struct _vertex_array_t_array {
	struct vertex_array **buffer;
	int                   length;
	int                   capacity;
} _vertex_array_t_array_t;
typedef struct _shader_data_t_array {
	struct shader_data **buffer;
	int                  length;
	int                  capacity;
} _shader_data_t_array_t;
typedef struct _tex_unit_t_array {
	struct tex_unit **buffer;
	int               length;
	int               capacity;
} _tex_unit_t_array_t;
typedef struct _shader_const_t_array {
	struct shader_const **buffer;
	int                   length;
	int                   capacity;
} _shader_const_t_array_t;
typedef struct _vertex_element_t_array {
	struct vertex_element **buffer;
	int                     length;
	int                     capacity;
} _vertex_element_t_array_t;
typedef struct _bind_const_t_array {
	struct bind_const **buffer;
	int                     length;
	int                     capacity;
} _bind_const_t_array_t;
typedef struct _bind_tex_t_array {
	struct bind_tex **buffer;
	int                     length;
	int                     capacity;
} _bind_tex_t_array_t;
typedef struct _material_context_t_array {
	struct material_context **buffer;
	int                     length;
	int                     capacity;
} _material_context_t_array_t;

typedef struct obj_runtime {
	void *_gc; // Link to armpack_decode result
} obj_runtime_t;

typedef struct obj {
	char           *name;
	char           *type; // object, mesh_object, camera_object
	char           *data_ref;
	f32_array_t    *transform;
	f32_array_t    *dimensions;
	bool            visible;
	bool            spawn; // Auto add object when creating scene
	void           *anim;  // TODO: deprecated
	char           *material_ref;
	_obj_t_array_t *children;
	obj_runtime_t  *_;
} obj_t;

typedef struct vertex_array {
	char        *attrib;
	char        *data; // short4norm, short2norm
	i16_array_t *values;
} vertex_array_t;

typedef struct mesh_data_runtime {
	char                  *handle;
	gpu_buffer_t          *vertex_buffer;
	gpu_buffer_t          *index_buffer;
	gpu_vertex_structure_t structure;
} mesh_data_runtime_t;

typedef struct mesh_data {
	char                    *name;
	float                    scale_pos;
	float                    scale_tex;
	_vertex_array_t_array_t *vertex_arrays;
	u32_array_t             *index_array;
	mesh_data_runtime_t     *_;
} mesh_data_t;

typedef struct camera_data {
	char        *name;
	float        near_plane;
	float        far_plane;
	float        fov;
	float        aspect;
	bool         frustum_culling;
	f32_array_t *ortho; // Indicates ortho camera, left, right, bottom, top
} camera_data_t;

typedef struct world_data_runtime {
	gpu_texture_t *envmap;
	gpu_texture_t *radiance;
	any_array_t   *radiance_mipmaps; // gpu_texture_t[]
	f32_array_t   *irradiance;
} world_data_runtime_t;

typedef struct world_data {
	char                 *name;
	i32                   color;
	float                 strength;
	char                 *irradiance; // Reference to irradiance_t blob
	char                 *radiance;
	i32                   radiance_mipmaps;
	char                 *envmap;
	world_data_runtime_t *_;
} world_data_t;

typedef struct irradiance {
	f32_array_t *irradiance; // Spherical harmonics, bands 0,1,2
} irradiance_t;

typedef struct _scene {
	char             *name;
	any_array_t      *objects;
	any_array_t      *mesh_datas;
	any_array_t      *camera_datas;
	char             *camera_ref;
	any_array_t      *material_datas;
	any_array_t      *shader_datas;
	any_array_t      *world_datas;
	char             *world_ref;
	any_array_t      *speaker_datas;  // TODO: deprecated
	char_ptr_array_t *embedded_datas; // Preload for this scene, images only for now
} _scene_t;

typedef struct shader_context_runtime {
	gpu_pipeline_t        *pipe;
	i32_array_t           *constants;
	i32_array_t           *tex_units;
	gpu_vertex_structure_t structure;
	i32                    vertex_shader_size;
	i32                    fragment_shader_size;
} shader_context_runtime_t;

typedef struct vertex_element {
	char *name;
	char *data; // "short4norm", "short2norm"
} vertex_element_t;

typedef struct shader_const {
	char *name;
	char *type;
	char *link;
} shader_const_t;

typedef struct tex_unit {
	char *name;
	char *link;
} tex_unit_t;

typedef struct shader_context {
	char                      *name;
	bool                       depth_write;
	char                      *compare_mode;
	char                      *cull_mode;
	char                      *vertex_shader;
	char                      *fragment_shader;
	bool                       shader_from_source;
	char                      *blend_source;
	char                      *blend_destination;
	char                      *alpha_blend_source;
	char                      *alpha_blend_destination;
	u8_array_t                *color_writes_red; // Per target masks
	u8_array_t                *color_writes_green;
	u8_array_t                *color_writes_blue;
	u8_array_t                *color_writes_alpha;
	char_ptr_array_t          *color_attachments; // RGBA32, RGBA64, R8
	char                      *depth_attachment;  // D32
	_vertex_element_t_array_t *vertex_elements;   // vertex_element_t[]
	_shader_const_t_array_t   *constants;         // shader_const_t[]
	_tex_unit_t_array_t       *texture_units;     // tex_unit_t[]
	shader_context_runtime_t  *_;
} shader_context_t;

typedef struct shader_data {
	char        *name;
	any_array_t *contexts; // shader_context_t[]
} shader_data_t;

typedef struct bind_const {
	char        *name;
	f32_array_t *vec; // bool (vec[0] > 0) | i32 | f32 | vec2 | vec3 | vec4
} bind_const_t;

typedef struct bind_tex {
	char *name;
	char *file;
} bind_tex_t;

typedef struct material_context_runtime {
	any_array_t *textures; // gpu_texture_t[]
} material_context_runtime_t;

typedef struct material_context {
	char                       *name;
	_bind_const_t_array_t                *bind_constants; // bind_const_t[]
	_bind_tex_t_array_t                *bind_textures;  // bind_tex_t[]
	material_context_runtime_t *_;
} material_context_t;

typedef struct material_data_runtime {
	float          uid;
	shader_data_t *shader;
} material_data_runtime_t;

typedef struct material_data {
	char                    *name;
	char                    *shader;
	_material_context_t_array_t             *contexts; // material_context_t[]
	material_data_runtime_t *_;
} material_data_t;

typedef struct transform {
	mat4_t    world;
	mat4_t    local;
	vec4_t    loc;
	quat_t    rot;
	vec4_t    scale;
	float     scale_world;
	mat4_t    world_unpack;
	bool      dirty;
	object_t *object;
	vec4_t    dim;
	float     radius;
} transform_t;

typedef struct object {
	i32          uid;
	float        urandom;
	obj_t       *raw;
	char        *name;
	transform_t *transform;
	object_t    *parent;
	any_array_t *children;
	bool         visible;
	bool         culled;
	bool         is_empty;
	void        *ext;
	char        *ext_type;
} object_t;

extern i32 _object_uid_counter;

object_t *object_create(bool is_empty);
void      object_set_parent(object_t *raw, object_t *parent_object);
void      object_remove_super(object_t *raw);
void      object_remove(object_t *raw);
object_t *object_get_child(object_t *raw, char *name);

transform_t *transform_create(object_t *object);
void         transform_reset(transform_t *raw);
void         transform_update(transform_t *raw);
void         transform_build_matrix(transform_t *raw);
void         transform_set_matrix(transform_t *raw, mat4_t mat);
void         transform_decompose(transform_t *raw);
void         transform_rotate(transform_t *raw, vec4_t axis, float f);
void         transform_move(transform_t *raw, vec4_t axis, float f);
void         transform_compute_radius(transform_t *raw);
void         transform_compute_dim(transform_t *raw);
vec4_t       transform_look(transform_t *raw);
vec4_t       transform_right(transform_t *raw);
vec4_t       transform_up(transform_t *raw);
float        transform_world_x(transform_t *raw);
float        transform_world_y(transform_t *raw);
float        transform_world_z(transform_t *raw);

camera_data_t *camera_data_parse(char *name, char *id);
camera_data_t *camera_data_get_raw_by_name(any_array_t *datas, char *name);

extern f32_array_t *_world_data_empty_irr;
world_data_t       *world_data_parse(char *name, char *id);
world_data_t       *world_data_get_raw_by_name(any_array_t *datas, char *name);
f32_array_t        *world_data_get_empty_irradiance();
f32_array_t        *world_data_set_irradiance(world_data_t *raw);
void                world_data_load_envmap(world_data_t *raw);

gpu_texture_t *data_get_image(char *file);
buffer_t      *data_get_blob(char *file);

extern i32          _material_data_uid_counter;
material_data_t    *material_data_create(material_data_t *raw, char *file);
material_data_t    *material_data_parse(char *file, char *name);
material_data_t    *material_data_get_raw_by_name(any_array_t *datas, char *name);
material_context_t *material_data_get_context(material_data_t *raw, char *name);
void                material_context_load(material_context_t *raw);
shader_data_t      *data_get_shader(char *file, char *name);

shader_data_t       *shader_data_create(shader_data_t *raw);
char                *shader_data_ext();
shader_data_t       *shader_data_parse(char *file, char *name);
shader_data_t       *shader_data_get_raw_by_name(_shader_data_t_array_t *datas, char *name);
void                 shader_data_delete(shader_data_t *raw);
shader_context_t    *shader_data_get_context(shader_data_t *raw, char *name);
void                 shader_context_load(shader_context_t *raw);
void                 shader_context_compile(shader_context_t *raw);
i32                  shader_context_type_size(char *t);
i32                  shader_context_type_pad(i32 offset, i32 size);
void                 shader_context_finish_compile(shader_context_t *raw);
gpu_vertex_data_t    shader_context_parse_data(char *data);
void                 shader_context_parse_vertex_struct(shader_context_t *raw);
void                 shader_context_delete(shader_context_t *raw);
gpu_compare_mode_t   shader_context_get_compare_mode(char *s);
gpu_cull_mode_t      shader_context_get_cull_mode(char *s);
gpu_blend_t          shader_context_get_blend_fac(char *s);
gpu_texture_format_t shader_context_get_tex_format(char *s);
void                 shader_context_add_const(shader_context_t *raw, i32 offset);
void                 shader_context_add_tex(shader_context_t *raw, i32 i);

mesh_data_t           *mesh_data_parse(char *name, char *id);
mesh_data_t           *mesh_data_get_raw_by_name(any_array_t *datas, char *name);
mesh_data_t           *mesh_data_create(mesh_data_t *raw);
gpu_vertex_structure_t mesh_data_get_vertex_struct(any_array_t *vertex_arrays);
gpu_vertex_data_t      mesh_data_get_vertex_data(char *data);
i32                    mesh_data_get_vertex_size(char *vertex_data);
void                   mesh_data_build_vertices(gpu_buffer_t *vertex_buffer, any_array_t *vertex_arrays);
void                   mesh_data_build_indices(gpu_buffer_t *index_buffer, u32_array_t *index_array);
vertex_array_t        *mesh_data_get_vertex_array(mesh_data_t *raw, char *name);
void                   mesh_data_build(mesh_data_t *raw);
vec4_t                 mesh_data_calculate_aabb(mesh_data_t *raw);
void                   mesh_data_delete(mesh_data_t *raw);
