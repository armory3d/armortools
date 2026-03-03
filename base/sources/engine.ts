
declare type transform_t = {
	world?: mat4_t;
	local?: mat4_t;
	loc?: vec4_t;
	rot?: quat_t;
	scale?: vec4_t;
	scale_world?: f32;
	world_unpack?: mat4_t;
	dirty?: bool;
	object?: object_t;
	dim?: vec4_t;
	radius?: f32;
};

declare function transform_create(object: object_t): transform_t;
declare function transform_reset(raw: transform_t);
declare function transform_update(raw: transform_t);
declare function transform_build_matrix(raw: transform_t);
declare function transform_set_matrix(raw: transform_t, mat: mat4_t);
declare function transform_decompose(raw: transform_t);
declare function transform_rotate(raw: transform_t, axis: vec4_t, f: f32);
declare function transform_move(raw: transform_t, axis: vec4_t, f: f32 = 1.0);
declare function transform_compute_radius(raw: transform_t);
declare function transform_compute_dim(raw: transform_t);
declare function transform_look(raw: transform_t): vec4_t;
declare function transform_right(raw: transform_t): vec4_t;
declare function transform_up(raw: transform_t): vec4_t;
declare function transform_world_x(raw: transform_t): f32;
declare function transform_world_y(raw: transform_t): f32;
declare function transform_world_z(raw: transform_t): f32;

declare type object_t = {
	uid?: i32;
	urandom?: f32;
	raw?: obj_t;
	name?: string;
	transform?: transform_t;
	parent?: object_t;
	children?: object_t[];
	visible?: bool; // Skip render, keep updating
	culled?: bool;  // base_object_t was culled last frame
	is_empty?: bool;
	ext?: any; // mesh_object_t | camera_object_t
	ext_type?: string;
};

declare let _object_uid_counter: i32;

declare function object_create(is_empty: bool = true): object_t;
declare function object_set_parent(raw: object_t, parent_object: object_t);
declare function object_remove_super(raw: object_t);
declare function object_remove(raw: object_t);
declare function object_get_child(raw: object_t, name: string): object_t;

declare function mesh_data_parse(name: string, id: string): mesh_data_t;
declare function mesh_data_get_raw_by_name(datas: mesh_data_t[], name: string): mesh_data_t;
declare function mesh_data_create(raw: mesh_data_t): mesh_data_t;
declare function mesh_data_get_vertex_struct(vertex_arrays: vertex_array_t[]): gpu_vertex_structure_t;
declare function mesh_data_get_vertex_data(data: string): gpu_vertex_data_t;
declare function mesh_data_get_vertex_size(vertex_data: string): i32;
declare function mesh_data_build_vertices(vertex_buffer: gpu_buffer_t, vertex_arrays: vertex_array_t[]);
declare function mesh_data_build_indices(index_buffer: gpu_buffer_t, index_array: u32_array_t);
declare function mesh_data_get_vertex_array(raw: mesh_data_t, name: string): vertex_array_t;
declare function mesh_data_build(raw: mesh_data_t);
declare function mesh_data_calculate_aabb(raw: mesh_data_t): vec4_t;
declare function mesh_data_delete(raw: mesh_data_t);

declare function camera_data_parse(name: string, id: string): camera_data_t;
declare function camera_data_get_raw_by_name(datas: camera_data_t[], name: string): camera_data_t;

declare let _world_data_empty_irr: f32_array_t = null;
declare function world_data_parse(name: string, id: string): world_data_t;
declare function world_data_get_raw_by_name(datas: world_data_t[], name: string): world_data_t;
declare function world_data_get_empty_irradiance(): f32_array_t;
declare function world_data_set_irradiance(raw: world_data_t): f32_array_t;
declare function world_data_load_envmap(raw: world_data_t);

declare function shader_data_create(raw: shader_data_t): shader_data_t;
declare function shader_data_ext(): string;
declare function shader_data_parse(file: string, name: string): shader_data_t;
declare function shader_data_get_raw_by_name(datas: shader_data_t[], name: string): shader_data_t;
declare function shader_data_delete(raw: shader_data_t);
declare function shader_data_get_context(raw: shader_data_t, name: string): shader_context_t;
declare function shader_context_load(raw: shader_context_t);
declare function shader_context_compile(raw: shader_context_t);
declare function shader_context_type_size(t: string): i32;
declare function shader_context_type_pad(offset: i32, size: i32): i32;
declare function shader_context_finish_compile(raw: shader_context_t);
declare function shader_context_parse_data(data: string): gpu_vertex_data_t;
declare function shader_context_parse_vertex_struct(raw: shader_context_t);
declare function shader_context_delete(raw: shader_context_t);
declare function shader_context_get_compare_mode(s: string): gpu_compare_mode_t;
declare function shader_context_get_cull_mode(s: string): gpu_cull_mode_t;
declare function shader_context_get_blend_fac(s: string): gpu_blend_t;
declare function shader_context_get_tex_format(s: string): gpu_texture_format_t;
declare function shader_context_add_const(raw: shader_context_t, offset: i32);
declare function shader_context_add_tex(raw: shader_context_t, i: i32);

declare let _material_data_uid_counter: i32;
declare function material_data_create(raw: material_data_t, file: string = ""): material_data_t;
declare function material_data_parse(file: string, name: string): material_data_t;
declare function material_data_get_raw_by_name(datas: material_data_t[], name: string): material_data_t;
declare function material_data_get_context(raw: material_data_t, name: string): material_context_t;
declare function material_context_load(raw: material_context_t);
