#include "engine.h"

i32                 _object_uid_counter = 0;
extern any_array_t *scene_empties;
extern any_array_t *scene_meshes;
extern object_t    *_scene_scene_parent;

void      mesh_object_remove(void *raw);
void      camera_object_remove(void *raw);
_scene_t *data_get_scene_raw(char *name);

gpu_buffer_t   *gpu_create_vertex_buffer(i32 count, gpu_vertex_structure_t *structure);
gpu_buffer_t   *gpu_create_index_buffer(i32 count);
buffer_t       *gpu_lock_vertex_buffer(gpu_buffer_t *buffer);
u32_array_t    *gpu_lock_index_buffer(gpu_buffer_t *buffer);
void            gpu_delete_buffer(gpu_buffer_t *buffer);
f32             math_floor(f32 x);
gpu_shader_t   *gpu_create_shader(buffer_t *data, i32 shader_type);
gpu_shader_t   *gpu_create_shader_from_source(char *source, int source_size, gpu_shader_type_t shader_type);
gpu_pipeline_t *gpu_create_pipeline();
void            gpu_delete_pipeline(gpu_pipeline_t *pipeline);
#ifdef arm_embed
gpu_shader_t *sys_get_shader(char *name);
#endif

//  ██████╗ ██████╗      ██╗███████╗ ██████╗████████╗
// ██╔═══██╗██╔══██╗     ██║██╔════╝██╔════╝╚══██╔══╝
// ██║   ██║██████╔╝     ██║█████╗  ██║        ██║
// ██║   ██║██╔══██╗██   ██║██╔══╝  ██║        ██║
// ╚██████╔╝██████╔╝╚█████╔╝███████╗╚██████╗   ██║
//  ╚═════╝ ╚═════╝  ╚════╝ ╚══════╝ ╚═════╝   ╚═╝

object_t *object_create(bool is_empty) {
	object_t *raw  = gc_alloc(sizeof(object_t));
	raw->name      = "";
	raw->children  = any_array_create(0);
	raw->visible   = true;
	raw->culled    = false;
	raw->uid       = _object_uid_counter++;
	raw->transform = transform_create(raw);
	raw->is_empty  = is_empty;
	if (raw->is_empty) {
		any_array_push(scene_empties, raw);
	}
	return raw;
}

void object_set_parent(object_t *raw, object_t *parent_object) {
	if (parent_object == raw || parent_object == raw->parent) {
		return;
	}

	if (raw->parent != NULL) {
		array_remove(raw->parent->children, raw);
		raw->parent = NULL; // Rebuild matrix without a parent
		transform_build_matrix(raw->transform);
	}

	if (parent_object == NULL) {
		parent_object = _scene_scene_parent;
	}
	raw->parent = parent_object;
	any_array_push(raw->parent->children, raw);
}

void object_remove_super(object_t *raw) {
	if (raw->is_empty) {
		array_remove(scene_empties, raw);
	}
	while (raw->children->length > 0) {
		object_remove((object_t *)raw->children->buffer[0]);
	}
	if (raw->parent != NULL) {
		array_remove(raw->parent->children, raw);
		raw->parent = NULL;
	}
}

void object_remove(object_t *raw) {
	if (string_equals(raw->ext_type, "mesh_object_t")) {
		mesh_object_remove(raw->ext);
	}
	else if (string_equals(raw->ext_type, "camera_object_t")) {
		camera_object_remove(raw->ext);
	}
	else {
		object_remove_super(raw);
	}
}

object_t *object_get_child(object_t *raw, char *name) {
	if (string_equals(raw->name, name)) {
		return raw;
	}
	for (i32 i = 0; i < raw->children->length; ++i) {
		object_t *c = (object_t *)raw->children->buffer[i];
		object_t *r = object_get_child(c, name);
		if (r != NULL) {
			return r;
		}
	}
	return NULL;
}

// ████████╗██████╗  █████╗ ███╗   ██╗███████╗███████╗ ██████╗ ██████╗ ███╗   ███╗
// ╚══██╔══╝██╔══██╗██╔══██╗████╗  ██║██╔════╝██╔════╝██╔═══██╗██╔══██╗████╗ ████║
//    ██║   ██████╔╝███████║██╔██╗ ██║███████╗█████╗  ██║   ██║██████╔╝██╔████╔██║
//    ██║   ██╔══██╗██╔══██║██║╚██╗██║╚════██║██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║
//    ██║   ██║  ██║██║  ██║██║ ╚████║███████║██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║
//    ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝

transform_t *transform_create(object_t *object) {
	transform_t *raw = gc_alloc(sizeof(transform_t));
	raw->scale_world = 1.0;
	raw->object      = object;
	transform_reset(raw);
	return raw;
}

void transform_reset(transform_t *raw) {
	raw->world        = mat4_identity();
	raw->world_unpack = mat4_identity();
	raw->local        = mat4_identity();
	raw->loc          = vec4_create(0.0, 0.0, 0.0, 0.0);
	raw->rot          = quat_create(0.0, 0.0, 0.0, 1.0);
	raw->scale        = vec4_create(1.0, 1.0, 1.0, 0.0);
	raw->dim          = vec4_create(2.0, 2.0, 2.0, 0.0);
	raw->radius       = 1.0;
	raw->dirty        = true;
}

void transform_update(transform_t *raw) {
	if (raw->dirty) {
		transform_build_matrix(raw);
	}
}

void transform_build_matrix(transform_t *raw) {
	raw->local = mat4_compose(raw->loc, raw->rot, raw->scale);

	if (raw->object->parent != NULL) {
		raw->world = mat4_mult_mat3x4(raw->local, raw->object->parent->transform->world);
	}
	else {
		raw->world = mat4_clone(raw->local);
	}

	raw->world_unpack = mat4_clone(raw->world);
	if (raw->scale_world != 1.0) {
		raw->world_unpack.m00 *= raw->scale_world;
		raw->world_unpack.m01 *= raw->scale_world;
		raw->world_unpack.m02 *= raw->scale_world;
		raw->world_unpack.m03 *= raw->scale_world;
		raw->world_unpack.m10 *= raw->scale_world;
		raw->world_unpack.m11 *= raw->scale_world;
		raw->world_unpack.m12 *= raw->scale_world;
		raw->world_unpack.m13 *= raw->scale_world;
		raw->world_unpack.m20 *= raw->scale_world;
		raw->world_unpack.m21 *= raw->scale_world;
		raw->world_unpack.m22 *= raw->scale_world;
		raw->world_unpack.m23 *= raw->scale_world;
	}

	transform_compute_dim(raw);

	// Update children
	for (i32 i = 0; i < raw->object->children->length; ++i) {
		object_t *n = (object_t *)raw->object->children->buffer[i];
		transform_build_matrix(n->transform);
	}

	raw->dirty = false;
}

void transform_set_matrix(transform_t *raw, mat4_t mat) {
	raw->local = mat4_clone(mat);
	transform_decompose(raw);
	transform_build_matrix(raw);
}

void transform_decompose(transform_t *raw) {
	mat4_decomposed_t *dec = mat4_decompose(raw->local);
	raw->loc               = dec->loc;
	raw->rot               = dec->rot;
	raw->scale             = dec->scl;
}

void transform_rotate(transform_t *raw, vec4_t axis, float f) {
	quat_t q = quat_from_axis_angle(axis, f);
	raw->rot = quat_mult(q, raw->rot);
	transform_build_matrix(raw);
}

void transform_move(transform_t *raw, vec4_t axis, float f) {
	raw->loc = vec4_fadd(raw->loc, axis.x * f, axis.y * f, axis.z * f, 0.0);
	transform_build_matrix(raw);
}

void transform_compute_radius(transform_t *raw) {
	raw->radius = sqrtf(raw->dim.x * raw->dim.x + raw->dim.y * raw->dim.y + raw->dim.z * raw->dim.z);
}

void transform_compute_dim(transform_t *raw) {
	if (raw->object->raw == NULL && string_equals(raw->object->ext_type, "mesh_object_t")) {
		//// TODO
		// mesh_object_t *mo    = (mesh_object_t *)raw->object->ext;
		// vec4_t         aabb  = mesh_data_calculate_aabb(mo->data);
		// _obj_t        *o_raw = gc_alloc(sizeof(_obj_t));
		// o_raw->dimensions    = f32_array_create_xyz(aabb.x, aabb.y, aabb.z);
		// raw->object->raw     = o_raw;
	}

	if (raw->object->raw == NULL || raw->object->raw->dimensions == NULL) {
		raw->dim = vec4_create(2.0 * raw->scale.x, 2.0 * raw->scale.y, 2.0 * raw->scale.z, 0.0);
	}
	else {
		f32_array_t *d = raw->object->raw->dimensions;
		raw->dim       = vec4_create(d->buffer[0] * raw->scale.x, d->buffer[1] * raw->scale.y, d->buffer[2] * raw->scale.z, 0.0);
	}
	transform_compute_radius(raw);
}

vec4_t transform_look(transform_t *raw) {
	return mat4_look(raw->world);
}

vec4_t transform_right(transform_t *raw) {
	return mat4_right(raw->world);
}

vec4_t transform_up(transform_t *raw) {
	return mat4_up(raw->world);
}

float transform_world_x(transform_t *raw) {
	return raw->world.m30;
}

float transform_world_y(transform_t *raw) {
	return raw->world.m31;
}

float transform_world_z(transform_t *raw) {
	return raw->world.m32;
}

//  ██████╗ █████╗ ███╗   ███╗███████╗██████╗  █████╗     ██████╗  █████╗ ████████╗ █████╗
// ██╔════╝██╔══██╗████╗ ████║██╔════╝██╔══██╗██╔══██╗    ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
// ██║     ███████║██╔████╔██║█████╗  ██████╔╝███████║    ██║  ██║███████║   ██║   ███████║
// ██║     ██╔══██║██║╚██╔╝██║██╔══╝  ██╔══██╗██╔══██║    ██║  ██║██╔══██║   ██║   ██╔══██║
// ╚██████╗██║  ██║██║ ╚═╝ ██║███████╗██║  ██║██║  ██║    ██████╔╝██║  ██║   ██║   ██║  ██║
//  ╚═════╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝

camera_data_t *camera_data_parse(char *name, char *id) {
	_scene_t      *format = data_get_scene_raw(name);
	camera_data_t *raw    = camera_data_get_raw_by_name(format->camera_datas, id);
	if (raw == NULL) {
		iron_log("Camera data '%s' not found!", id);
	}
	return raw;
}

camera_data_t *camera_data_get_raw_by_name(any_array_t *datas, char *name) {
	if (strcmp(name, "") == 0) {
		return (camera_data_t *)datas->buffer[0];
	}
	for (i32 i = 0; i < datas->length; ++i) {
		camera_data_t *d = (camera_data_t *)datas->buffer[i];
		if (strcmp(d->name, name) == 0) {
			return d;
		}
	}
	return NULL;
}

// ██╗    ██╗ ██████╗ ██████╗ ██╗      ██████╗     ██████╗  █████╗ ████████╗ █████╗
// ██║    ██║██╔═══██╗██╔══██╗██║      ██╔══██╗    ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
// ██║ █╗ ██║██║   ██║██████╔╝██║      ██║  ██║    ██║  ██║███████║   ██║   ███████║
// ██║███╗██║██║   ██║██╔══██╗██║      ██║  ██║    ██║  ██║██╔══██║   ██║   ██╔══██║
// ╚███╔███╔╝╚██████╔╝██║  ██║███████╗ ██████╔╝    ██████╔╝██║  ██║   ██║   ██║  ██║
//  ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚══════╝ ╚═════╝     ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝

f32_array_t *_world_data_empty_irr = NULL;

world_data_t *world_data_parse(char *name, char *id) {
	_scene_t     *format = data_get_scene_raw(name);
	world_data_t *raw    = world_data_get_raw_by_name(format->world_datas, id);
	if (raw == NULL) {
		iron_log("World data '%s' not found!", id);
		return NULL;
	}

	raw->_                   = gc_alloc(sizeof(world_data_runtime_t));
	raw->_->radiance_mipmaps = any_array_create(0);

	raw->_->irradiance = world_data_set_irradiance(raw);
	if (raw->radiance != NULL) {
		raw->_->radiance = data_get_image(raw->radiance);
		while (raw->_->radiance_mipmaps->length < raw->radiance_mipmaps) {
			any_array_push(raw->_->radiance_mipmaps, NULL);
		}
		i32   dot  = string_last_index_of(raw->radiance, ".");
		char *ext  = substring(raw->radiance, dot, string_length(raw->radiance));
		char *base = substring(raw->radiance, 0, dot);

		for (i32 i = 0; i < raw->radiance_mipmaps; ++i) {
			char          *mip_name             = string_join(string_join(base, string_join("_", i32_to_string(i))), ext);
			gpu_texture_t *mipimg               = data_get_image(mip_name);
			raw->_->radiance_mipmaps->buffer[i] = mipimg;
		}
	}

	return raw;
}

world_data_t *world_data_get_raw_by_name(any_array_t *datas, char *name) {
	if (strcmp(name, "") == 0) {
		return (world_data_t *)datas->buffer[0];
	}
	for (i32 i = 0; i < datas->length; ++i) {
		world_data_t *d = (world_data_t *)datas->buffer[i];
		if (strcmp(d->name, name) == 0) {
			return d;
		}
	}
	return NULL;
}

f32_array_t *world_data_get_empty_irradiance() {
	if (_world_data_empty_irr == NULL) {
		_world_data_empty_irr = f32_array_create(28);
		gc_root(_world_data_empty_irr);
		for (i32 i = 0; i < _world_data_empty_irr->length; ++i) {
			_world_data_empty_irr->buffer[i] = 0.0;
		}
	}
	return _world_data_empty_irr;
}

f32_array_t *world_data_set_irradiance(world_data_t *raw) {
	if (raw->irradiance == NULL) {
		return world_data_get_empty_irradiance();
	}
	buffer_t     *b                 = data_get_blob(string_join(raw->irradiance, ".arm"));
	irradiance_t *irradiance_parsed = (irradiance_t *)armpack_decode(b);
	f32_array_t  *irr               = f32_array_create(28); // Align to mult of 4 - 27->28
	for (i32 i = 0; i < 27; ++i) {
		irr->buffer[i] = irradiance_parsed->irradiance->buffer[i];
	}
	return irr;
}

void world_data_load_envmap(world_data_t *raw) {
	if (raw->envmap != NULL) {
		raw->_->envmap = data_get_image(raw->envmap);
	}
}

// ███╗   ███╗ █████╗ ████████╗███████╗██████╗ ██╗ █████╗ ██╗         ██████╗  █████╗ ████████╗ █████╗
// ████╗ ████║██╔══██╗╚══██╔══╝██╔════╝██╔══██╗██║██╔══██╗██║        ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
// ██╔████╔██║███████║   ██║   █████╗  ██████╔╝██║███████║██║        ██║  ██║███████║   ██║   ███████║
// ██║╚██╔╝██║██╔══██║   ██║   ██╔══╝  ██╔══██╗██║██╔══██║██║        ██║  ██║██╔══██║   ██║   ██╔══██║
// ██║ ╚═╝ ██║██║  ██║   ██║   ███████╗██║  ██║██║██║  ██║███████╗   ██████╔╝██║  ██║   ██║   ██║  ██║
// ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚══════╝   ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝

i32 _material_data_uid_counter = 0;

material_data_t *material_data_create(material_data_t *raw, char *file) {
	raw->_      = gc_alloc(sizeof(material_data_runtime_t));
	raw->_->uid = (float)(++_material_data_uid_counter); // Start from 1

	any_array_t *ref         = string_split(raw->shader, "/");
	char        *object_file = "";
	char        *data_ref    = "";
	if (ref->length == 2) { // File reference
		object_file = (char *)ref->buffer[0];
		data_ref    = (char *)ref->buffer[1];
	}
	else { // Local data
		object_file = file;
		data_ref    = raw->shader;
	}

	raw->_->shader = data_get_shader(object_file, data_ref);
	for (i32 i = 0; i < (i32)raw->contexts->length; ++i) {
		material_context_t *c = (material_context_t *)raw->contexts->buffer[i];
		material_context_load(c);
	}
	return raw;
}

material_data_t *material_data_parse(char *file, char *name) {
	_scene_t        *format = data_get_scene_raw(file);
	material_data_t *raw    = material_data_get_raw_by_name(format->material_datas, name);
	if (raw == NULL) {
		iron_log("Material data '%s' not found!", name);
		return NULL;
	}
	return material_data_create(raw, file);
}

material_data_t *material_data_get_raw_by_name(any_array_t *datas, char *name) {
	if (strcmp(name, "") == 0) {
		return (material_data_t *)datas->buffer[0];
	}
	for (i32 i = 0; i < (i32)datas->length; ++i) {
		material_data_t *d = (material_data_t *)datas->buffer[i];
		if (strcmp(d->name, name) == 0) {
			return d;
		}
	}
	return NULL;
}

material_context_t *material_data_get_context(material_data_t *raw, char *name) {
	for (i32 i = 0; i < (i32)raw->contexts->length; ++i) {
		material_context_t *c = (material_context_t *)raw->contexts->buffer[i];
		if (strcmp(c->name, name) == 0) {
			return c;
		}
	}
	return NULL;
}

void material_context_load(material_context_t *raw) {
	raw->_ = gc_alloc(sizeof(material_context_runtime_t));
	if (raw->bind_textures != NULL && raw->bind_textures->length > 0) {
		raw->_->textures = any_array_create(0);
		for (i32 i = 0; i < (i32)raw->bind_textures->length; ++i) {
			bind_tex_t *tex = (bind_tex_t *)raw->bind_textures->buffer[i];
			if (strcmp(tex->file, "") == 0) { // Empty texture
				continue;
			}
			gpu_texture_t *image = data_get_image(tex->file);
			any_array_push(raw->_->textures, image);
		}
	}
}

// ███████╗██╗  ██╗ █████╗ ██████╗ ███████╗██████╗     ██████╗  █████╗ ████████╗ █████╗
// ██╔════╝██║  ██║██╔══██╗██╔══██╗██╔════╝██╔══██╗    ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
// ███████╗███████║███████║██║  ██║█████╗  ██████╔╝    ██║  ██║███████║   ██║   ███████║
// ╚════██║██╔══██║██╔══██║██║  ██║██╔══╝  ██╔══██╗    ██║  ██║██╔══██║   ██║   ██╔══██║
// ███████║██║  ██║██║  ██║██████╔╝███████╗██║  ██║    ██████╔╝██║  ██║   ██║   ██║  ██║
// ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝╚═╝  ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝

shader_data_t *shader_data_create(shader_data_t *raw) {
	for (i32 i = 0; i < raw->contexts->length; ++i) {
		shader_context_t *c = (shader_context_t *)raw->contexts->buffer[i];
		shader_context_load(c);
	}
	return raw;
}

char *shader_data_ext() {
#ifdef IRON_VULKAN
	return ".spirv";
#elif defined(IRON_METAL)
	return ".metal";
#elif defined(IRON_WASM)
	return ".wgsl";
#else
	return ".d3d11";
#endif
}

shader_data_t *shader_data_parse(char *file, char *name) {
	_scene_t      *format = data_get_scene_raw(file);
	shader_data_t *raw    = shader_data_get_raw_by_name(format->shader_datas, name);
	if (raw == NULL) {
		iron_log("Shader data '%s' not found!", name);
		return NULL;
	}
	return shader_data_create(raw);
}

shader_data_t *shader_data_get_raw_by_name(_shader_data_t_array_t *datas, char *name) {
	if (strcmp(name, "") == 0) {
		return (shader_data_t *)datas->buffer[0];
	}
	for (i32 i = 0; i < datas->length; ++i) {
		shader_data_t *d = (shader_data_t *)datas->buffer[i];
		if (strcmp(d->name, name) == 0) {
			return d;
		}
	}
	return NULL;
}

void shader_data_delete(shader_data_t *raw) {
	for (i32 i = 0; i < raw->contexts->length; ++i) {
		shader_context_t *c = (shader_context_t *)raw->contexts->buffer[i];
		shader_context_delete(c);
	}
}

shader_context_t *shader_data_get_context(shader_data_t *raw, char *name) {
	for (i32 i = 0; i < raw->contexts->length; ++i) {
		shader_context_t *c = (shader_context_t *)raw->contexts->buffer[i];
		if (strcmp(c->name, name) == 0) {
			return c;
		}
	}
	return NULL;
}

void shader_context_load(shader_context_t *raw) {
	if (raw->_ == NULL) {
		raw->_ = gc_alloc(sizeof(shader_context_runtime_t));
	}
	shader_context_parse_vertex_struct(raw);
	shader_context_compile(raw);
}

void shader_context_compile(shader_context_t *raw) {
	if (raw->_->pipe != NULL) {
		gpu_delete_pipeline(raw->_->pipe);
	}
	raw->_->pipe               = gpu_create_pipeline();
	raw->_->constants          = i32_array_create(0);
	raw->_->tex_units          = i32_array_create(0);
	raw->_->pipe->input_layout = &raw->_->structure;
	raw->_->pipe->depth_write  = raw->depth_write;
	raw->_->pipe->depth_mode   = shader_context_get_compare_mode(raw->compare_mode);
	raw->_->pipe->cull_mode    = shader_context_get_cull_mode(raw->cull_mode);
	if (raw->blend_source != NULL) {
		raw->_->pipe->blend_source = shader_context_get_blend_fac(raw->blend_source);
	}
	if (raw->blend_destination != NULL) {
		raw->_->pipe->blend_destination = shader_context_get_blend_fac(raw->blend_destination);
	}
	if (raw->alpha_blend_source != NULL) {
		raw->_->pipe->alpha_blend_source = shader_context_get_blend_fac(raw->alpha_blend_source);
	}
	if (raw->alpha_blend_destination != NULL) {
		raw->_->pipe->alpha_blend_destination = shader_context_get_blend_fac(raw->alpha_blend_destination);
	}
	if (raw->color_writes_red != NULL) {
		for (i32 i = 0; i < raw->color_writes_red->length; ++i) {
			raw->_->pipe->color_write_mask_red[i] = (bool)(intptr_t)raw->color_writes_red->buffer[i];
		}
	}
	if (raw->color_writes_green != NULL) {
		for (i32 i = 0; i < raw->color_writes_green->length; ++i) {
			raw->_->pipe->color_write_mask_green[i] = (bool)(intptr_t)raw->color_writes_green->buffer[i];
		}
	}
	if (raw->color_writes_blue != NULL) {
		for (i32 i = 0; i < raw->color_writes_blue->length; ++i) {
			raw->_->pipe->color_write_mask_blue[i] = (bool)(intptr_t)raw->color_writes_blue->buffer[i];
		}
	}
	if (raw->color_writes_alpha != NULL) {
		for (i32 i = 0; i < raw->color_writes_alpha->length; ++i) {
			raw->_->pipe->color_write_mask_alpha[i] = (bool)(intptr_t)raw->color_writes_alpha->buffer[i];
		}
	}
	if (raw->color_attachments != NULL) {
		raw->_->pipe->color_attachment_count = raw->color_attachments->length;
		for (i32 i = 0; i < raw->color_attachments->length; ++i) {
			raw->_->pipe->color_attachment[i] = shader_context_get_tex_format((char *)raw->color_attachments->buffer[i]);
		}
	}
	if (raw->depth_attachment != NULL) {
		raw->_->pipe->depth_attachment_bits = strcmp(raw->depth_attachment, "NONE") == 0 ? 0 : 32;
	}

	if (raw->shader_from_source) {
		raw->_->pipe->vertex_shader   = gpu_create_shader_from_source(raw->vertex_shader, raw->_->vertex_shader_size, GPU_SHADER_TYPE_VERTEX);
		raw->_->pipe->fragment_shader = gpu_create_shader_from_source(raw->fragment_shader, raw->_->fragment_shader_size, GPU_SHADER_TYPE_FRAGMENT);
		if (raw->_->pipe->vertex_shader == NULL || raw->_->pipe->fragment_shader == NULL) {
			return;
		}
	}
	else {
#ifdef arm_embed
		raw->_->pipe->fragment_shader = sys_get_shader(raw->fragment_shader);
		raw->_->pipe->vertex_shader   = sys_get_shader(raw->vertex_shader);
#else
		buffer_t *vs_buffer           = data_get_blob(string_join(raw->vertex_shader, shader_data_ext()));
		raw->_->pipe->vertex_shader   = gpu_create_shader(vs_buffer, GPU_SHADER_TYPE_VERTEX);
		buffer_t *fs_buffer           = data_get_blob(string_join(raw->fragment_shader, shader_data_ext()));
		raw->_->pipe->fragment_shader = gpu_create_shader(fs_buffer, GPU_SHADER_TYPE_FRAGMENT);
#endif
	}

	shader_context_finish_compile(raw);
}

i32 shader_context_type_size(char *t) {
#ifdef IRON_DIRECT3D12
	if (strcmp(t, "int") == 0)
		return 4;
	if (strcmp(t, "float") == 0)
		return 4;
	if (strcmp(t, "vec2") == 0)
		return 8;
	if (strcmp(t, "vec3") == 0)
		return 12;
	if (strcmp(t, "vec4") == 0)
		return 16;
	if (strcmp(t, "mat3") == 0)
		return 48;
	if (strcmp(t, "mat4") == 0)
		return 64;
	if (strcmp(t, "float2") == 0)
		return 8;
	if (strcmp(t, "float3") == 0)
		return 12;
	if (strcmp(t, "float4") == 0)
		return 16;
	if (strcmp(t, "float3x3") == 0)
		return 48;
	if (strcmp(t, "float4x4") == 0)
		return 64;
#else
	if (strcmp(t, "int") == 0)
		return 4;
	if (strcmp(t, "float") == 0)
		return 4;
	if (strcmp(t, "vec2") == 0)
		return 8;
	if (strcmp(t, "vec3") == 0)
		return 16;
	if (strcmp(t, "vec4") == 0)
		return 16;
	if (strcmp(t, "mat3") == 0)
		return 48;
	if (strcmp(t, "mat4") == 0)
		return 64;
	if (strcmp(t, "float2") == 0)
		return 8;
	if (strcmp(t, "float3") == 0)
		return 16;
	if (strcmp(t, "float4") == 0)
		return 16;
	if (strcmp(t, "float3x3") == 0)
		return 48;
	if (strcmp(t, "float4x4") == 0)
		return 64;
#endif
	return 0;
}

i32 shader_context_type_pad(i32 offset, i32 size) {
#ifdef IRON_DIRECT3D12
	i32 r = offset % 16;
	if (r == 0) {
		return 0;
	}
	if (size >= 16 || r + size > 16) {
		return 16 - r;
	}
	return 0;
#else
	if (size > 16) {
		size = 16;
	}
	return (size - (offset % size)) % size;
#endif
}

void shader_context_finish_compile(shader_context_t *raw) {
	gpu_pipeline_compile(raw->_->pipe);
	if (raw->constants != NULL) {
		i32 offset = 0;
		for (i32 i = 0; i < raw->constants->length; ++i) {
			shader_const_t *c    = (shader_const_t *)raw->constants->buffer[i];
			i32             size = shader_context_type_size(c->type);
			offset += shader_context_type_pad(offset, size);
			shader_context_add_const(raw, offset);
			offset += size;
		}
	}
	if (raw->texture_units != NULL) {
		for (i32 i = 0; i < raw->texture_units->length; ++i) {
			shader_context_add_tex(raw, i);
		}
	}
}

gpu_vertex_data_t shader_context_parse_data(char *data) {
	if (strcmp(data, "float1") == 0)
		return GPU_VERTEX_DATA_F32_1X;
	if (strcmp(data, "float2") == 0)
		return GPU_VERTEX_DATA_F32_2X;
	if (strcmp(data, "float3") == 0)
		return GPU_VERTEX_DATA_F32_3X;
	if (strcmp(data, "float4") == 0)
		return GPU_VERTEX_DATA_F32_4X;
	if (strcmp(data, "short2norm") == 0)
		return GPU_VERTEX_DATA_I16_2X_NORM;
	if (strcmp(data, "short4norm") == 0)
		return GPU_VERTEX_DATA_I16_4X_NORM;
	return GPU_VERTEX_DATA_F32_1X;
}

void shader_context_parse_vertex_struct(shader_context_t *raw) {
	raw->_->structure = (gpu_vertex_structure_t){0};
	for (i32 i = 0; i < raw->vertex_elements->length; ++i) {
		vertex_element_t *elem = (vertex_element_t *)raw->vertex_elements->buffer[i];
		gpu_vertex_structure_add(&raw->_->structure, elem->name, shader_context_parse_data(elem->data));
	}
}

void shader_context_delete(shader_context_t *raw) {
	if (raw->_->pipe->fragment_shader != NULL) {
		gpu_shader_destroy(raw->_->pipe->fragment_shader);
	}
	if (raw->_->pipe->vertex_shader != NULL) {
		gpu_shader_destroy(raw->_->pipe->vertex_shader);
	}
	gpu_delete_pipeline(raw->_->pipe);
}

gpu_compare_mode_t shader_context_get_compare_mode(char *s) {
	if (strcmp(s, "always") == 0)
		return GPU_COMPARE_MODE_ALWAYS;
	if (strcmp(s, "never") == 0)
		return GPU_COMPARE_MODE_NEVER;
	if (strcmp(s, "equal") == 0)
		return GPU_COMPARE_MODE_EQUAL;
	return GPU_COMPARE_MODE_LESS;
}

gpu_cull_mode_t shader_context_get_cull_mode(char *s) {
	if (strcmp(s, "none") == 0)
		return GPU_CULL_MODE_NONE;
	if (strcmp(s, "clockwise") == 0)
		return GPU_CULL_MODE_CLOCKWISE;
	return GPU_CULL_MODE_COUNTER_CLOCKWISE;
}

gpu_blend_t shader_context_get_blend_fac(char *s) {
	if (strcmp(s, "blend_one") == 0)
		return GPU_BLEND_ONE;
	if (strcmp(s, "blend_zero") == 0)
		return GPU_BLEND_ZERO;
	if (strcmp(s, "source_alpha") == 0)
		return GPU_BLEND_SOURCE_ALPHA;
	if (strcmp(s, "destination_alpha") == 0)
		return GPU_BLEND_DEST_ALPHA;
	if (strcmp(s, "inverse_source_alpha") == 0)
		return GPU_BLEND_INV_SOURCE_ALPHA;
	if (strcmp(s, "inverse_destination_alpha") == 0)
		return GPU_BLEND_INV_DEST_ALPHA;
	return GPU_BLEND_ONE;
}

gpu_texture_format_t shader_context_get_tex_format(char *s) {
	if (strcmp(s, "RGBA32") == 0)
		return GPU_TEXTURE_FORMAT_RGBA32;
	if (strcmp(s, "RGBA64") == 0)
		return GPU_TEXTURE_FORMAT_RGBA64;
	if (strcmp(s, "RGBA128") == 0)
		return GPU_TEXTURE_FORMAT_RGBA128;
	if (strcmp(s, "R32") == 0)
		return GPU_TEXTURE_FORMAT_R32;
	if (strcmp(s, "R16") == 0)
		return GPU_TEXTURE_FORMAT_R16;
	if (strcmp(s, "R8") == 0)
		return GPU_TEXTURE_FORMAT_R8;
	return GPU_TEXTURE_FORMAT_RGBA32;
}

void shader_context_add_const(shader_context_t *raw, i32 offset) {
	i32_array_push(raw->_->constants, offset);
}

void shader_context_add_tex(shader_context_t *raw, i32 i) {
	i32_array_push(raw->_->tex_units, i);
}

// ███╗   ███╗███████╗███████╗██╗  ██╗    ██████╗  █████╗ ████████╗ █████╗
// ████╗ ████║██╔════╝██╔════╝██║  ██║    ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
// ██╔████╔██║█████╗  ███████╗███████║    ██║  ██║███████║   ██║   ███████║
// ██║╚██╔╝██║██╔══╝  ╚════██║██╔══██║    ██║  ██║██╔══██║   ██║   ██╔══██║
// ██║ ╚═╝ ██║███████╗███████║██║  ██║    ██████╔╝██║  ██║   ██║   ██║  ██║
// ╚═╝     ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝

mesh_data_t *mesh_data_parse(char *name, char *id) {
	_scene_t    *format = data_get_scene_raw(name);
	mesh_data_t *raw    = mesh_data_get_raw_by_name(format->mesh_datas, id);
	if (raw == NULL) {
		iron_log("Mesh data '%s' not found!", id);
		return NULL;
	}
	return mesh_data_create(raw);
}

mesh_data_t *mesh_data_get_raw_by_name(any_array_t *datas, char *name) {
	if (string_equals(name, "")) {
		return (mesh_data_t *)datas->buffer[0];
	}
	for (i32 i = 0; i < datas->length; ++i) {
		mesh_data_t *d = (mesh_data_t *)datas->buffer[i];
		if (string_equals(d->name, name)) {
			return d;
		}
	}
	return NULL;
}

mesh_data_t *mesh_data_create(mesh_data_t *raw) {
	raw->_ = gc_alloc(sizeof(mesh_data_runtime_t));
	if (raw->scale_pos == 0.0) {
		raw->scale_pos = 1.0;
	}
	if (raw->scale_tex == 0.0) {
		raw->scale_tex = 1.0;
	}
	raw->_->structure = mesh_data_get_vertex_struct(raw->vertex_arrays);
	mesh_data_build(raw);
	return raw;
}

gpu_vertex_structure_t mesh_data_get_vertex_struct(any_array_t *vertex_arrays) {
	gpu_vertex_structure_t structure = {0};
	for (i32 i = 0; i < vertex_arrays->length; ++i) {
		vertex_array_t *va = (vertex_array_t *)vertex_arrays->buffer[i];
		gpu_vertex_structure_add(&structure, va->attrib, mesh_data_get_vertex_data(va->data));
	}
	return structure;
}

gpu_vertex_data_t mesh_data_get_vertex_data(char *data) {
	if (string_equals(data, "short4norm")) {
		return GPU_VERTEX_DATA_I16_4X_NORM;
	}
	return GPU_VERTEX_DATA_I16_2X_NORM; // short2norm
}

i32 mesh_data_get_vertex_size(char *vertex_data) {
	if (string_equals(vertex_data, "short4norm")) {
		return 4;
	}
	return 2; // short2norm
}

void mesh_data_build_vertices(gpu_buffer_t *vertex_buffer, any_array_t *vertex_arrays) {
	vertex_array_t *va0       = (vertex_array_t *)vertex_arrays->buffer[0];
	buffer_t       *vertices  = gpu_lock_vertex_buffer(vertex_buffer);
	i32             size      = mesh_data_get_vertex_size(va0->data);
	i32             num_verts = va0->values->length / size;
	i32             di        = -1;
	for (i32 i = 0; i < num_verts; ++i) {
		for (i32 va = 0; va < vertex_arrays->length; ++va) {
			vertex_array_t *v = (vertex_array_t *)vertex_arrays->buffer[va];
			i32             l = mesh_data_get_vertex_size(v->data);
			for (i32 o = 0; o < l; ++o) {
				buffer_set_i16(vertices, ++di * 2, v->values->buffer[i * l + o]);
			}
		}
	}
	gpu_vertex_buffer_unlock(vertex_buffer);
}

void mesh_data_build_indices(gpu_buffer_t *index_buffer, u32_array_t *index_array) {
	u32_array_t *ia = gpu_lock_index_buffer(index_buffer);
	memcpy(ia->buffer, index_array->buffer, ia->length * 4);
	gpu_index_buffer_unlock(index_buffer);
}

vertex_array_t *mesh_data_get_vertex_array(mesh_data_t *raw, char *name) {
	for (i32 i = 0; i < raw->vertex_arrays->length; ++i) {
		vertex_array_t *va = (vertex_array_t *)raw->vertex_arrays->buffer[i];
		if (string_equals(va->attrib, name)) {
			return va;
		}
	}
	return NULL;
}

void mesh_data_build(mesh_data_t *raw) {
	vertex_array_t *positions = mesh_data_get_vertex_array(raw, "pos");
	i32             size      = mesh_data_get_vertex_size(positions->data);
	raw->_->vertex_buffer     = gpu_create_vertex_buffer(positions->values->length / size, &raw->_->structure);
	mesh_data_build_vertices(raw->_->vertex_buffer, raw->vertex_arrays);
	raw->_->index_buffer = gpu_create_index_buffer(raw->index_array->length);
	mesh_data_build_indices(raw->_->index_buffer, raw->index_array);
}

vec4_t mesh_data_calculate_aabb(mesh_data_t *raw) {
	vec4_t          aabb_min  = vec4_create(-0.01, -0.01, -0.01, 0.0);
	vec4_t          aabb_max  = vec4_create(0.01, 0.01, 0.01, 0.0);
	vec4_t          aabb      = vec4_create(0.0, 0.0, 0.0, 0.0);
	i32             i         = 0;
	vertex_array_t *positions = mesh_data_get_vertex_array(raw, "pos");
	while (i < positions->values->length) {
		if (positions->values->buffer[i] > aabb_max.x) {
			aabb_max.x = positions->values->buffer[i];
		}
		if (positions->values->buffer[i + 1] > aabb_max.y) {
			aabb_max.y = positions->values->buffer[i + 1];
		}
		if (positions->values->buffer[i + 2] > aabb_max.z) {
			aabb_max.z = positions->values->buffer[i + 2];
		}
		if (positions->values->buffer[i] < aabb_min.x) {
			aabb_min.x = positions->values->buffer[i];
		}
		if (positions->values->buffer[i + 1] < aabb_min.y) {
			aabb_min.y = positions->values->buffer[i + 1];
		}
		if (positions->values->buffer[i + 2] < aabb_min.z) {
			aabb_min.z = positions->values->buffer[i + 2];
		}
		i += 4;
	}
	aabb.x = (fabsf(aabb_min.x) + fabsf(aabb_max.x)) / 32767 * raw->scale_pos;
	aabb.y = (fabsf(aabb_min.y) + fabsf(aabb_max.y)) / 32767 * raw->scale_pos;
	aabb.z = (fabsf(aabb_min.z) + fabsf(aabb_max.z)) / 32767 * raw->scale_pos;
	return aabb;
}

void mesh_data_delete(mesh_data_t *raw) {
	gpu_delete_buffer(raw->_->vertex_buffer);
	gpu_delete_buffer(raw->_->index_buffer);
}
