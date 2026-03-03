
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

declare type camera_object_t = {
	base?: object_t;
	data?: camera_data_t;
	p?: mat4_t;
	no_jitter_p?: mat4_t;
	frame?: i32;
	v?: mat4_t;
	vp?: mat4_t;
	frustum_planes?: frustum_plane_t[];
};

declare let _camera_object_sphere_center: vec4_t;
declare let camera_object_taa_frames: i32;

declare function camera_object_create(data: camera_data_t): camera_object_t;
declare function camera_object_build_proj(raw: camera_object_t, screen_aspect: f32 = -1.0);
declare function camera_object_remove(raw: camera_object_t);
declare function camera_object_render_frame(raw: camera_object_t);
declare function camera_object_proj_jitter(raw: camera_object_t);
declare function camera_object_build_mat(raw: camera_object_t);
declare function camera_object_right(raw: camera_object_t): vec4_t;
declare function camera_object_up(raw: camera_object_t): vec4_t;
declare function camera_object_look(raw: camera_object_t): vec4_t;
declare function camera_object_right_world(raw: camera_object_t): vec4_t;
declare function camera_object_up_world(raw: camera_object_t): vec4_t;
declare function camera_object_look_world(raw: camera_object_t): vec4_t;
declare function camera_object_build_view_frustum(vp: mat4_t, frustum_planes: frustum_plane_t[]);
declare function camera_object_sphere_in_frustum(frustum_planes: frustum_plane_t[], t: transform_t, radius_scale: f32 = 1.0, offset_x: f32 = 0.0, offset_y: f32 = 0.0, offset_z: f32 = 0.0): bool;

declare type frustum_plane_t = {
	normal?: vec4_t;
	constant?: f32;
};

declare function frustum_plane_create(): frustum_plane_t;
declare function frustum_plane_normalize(raw: frustum_plane_t);
declare function frustum_plane_dist_to_sphere(raw: frustum_plane_t, sphere_center: vec4_t, sphere_radius: f32): f32;
declare function frustum_plane_set_components(raw: frustum_plane_t, x: f32, y: f32, z: f32, w: f32);

declare type mesh_object_t = {
	base?: object_t;
	data?: mesh_data_t;
	material?: material_data_t;
	camera_dist?: f32;
	frustum_culling?: bool;
	skip_context?: string;  // Do not draw this context
	force_context?: string; // Draw only this context
};

declare let _mesh_object_last_pipeline: gpu_pipeline_t;

declare function mesh_object_create(data: mesh_data_t, material: material_data_t): mesh_object_t;
declare function mesh_object_set_data(raw: mesh_object_t, data: mesh_data_t);
declare function mesh_object_remove(raw: mesh_object_t);
declare function mesh_object_cull_material(raw: mesh_object_t, context: string): bool;
declare function mesh_object_cull_mesh(raw: mesh_object_t, context: string, camera: camera_object_t): bool;
declare function mesh_object_render(raw: mesh_object_t, context: string, bind_params: string[]);
declare function mesh_object_valid_context(raw: mesh_object_t, mat: material_data_t, context: string): bool;

declare let uniforms_tex_links: (o: object_t, md: material_data_t, s: string) => gpu_texture_t;
declare let uniforms_mat4_links: (o: object_t, md: material_data_t, s: string) => mat4_t;
declare let uniforms_vec4_links: (o: object_t, md: material_data_t, s: string) => vec4_t;
declare let uniforms_vec3_links: (o: object_t, md: material_data_t, s: string) => vec4_t;
declare let uniforms_vec2_links: (o: object_t, md: material_data_t, s: string) => vec2_t;
declare let uniforms_f32_links: (o: object_t, md: material_data_t, s: string) => f32;
declare let uniforms_f32_array_links: (o: object_t, md: material_data_t, s: string) => f32_array_t;
declare let uniforms_i32_links: (o: object_t, md: material_data_t, s: string) => i32;
declare let uniforms_pos_unpack: f32;
declare let uniforms_tex_unpack: f32;

declare function uniforms_set_context_consts(context: shader_context_t, bind_params: string[]);
declare function uniforms_set_obj_consts(context: shader_context_t, object: object_t);
declare function uniforms_bind_render_target(rt: render_target_t, context: shader_context_t, sampler_id: string);
declare function uniforms_set_context_const(location: i32, c: shader_const_t): bool;
declare function uniforms_set_obj_const(obj: object_t, loc: i32, c: shader_const_t);
declare function uniforms_set_material_consts(context: shader_context_t, material_context: material_context_t);
declare function current_material(object: object_t): material_data_t;
declare function uniforms_set_material_const(location: i32, shader_const: shader_const_t, material_const: bind_const_t);

declare type mesh_data_t = {
	name?: string;
	scale_pos?: f32; // Unpack pos from (-1,1) coords
	scale_tex?: f32; // Unpack tex from (-1,1) coords
	vertex_arrays?: vertex_array_t[];
	index_array?: u32_array_t; // size = 3
	_?: mesh_data_runtime_t;
};

declare type mesh_data_runtime_t = {
	handle?: string; // Handle used to retrieve this object in Data
	vertex_buffer?: gpu_buffer_t;
	index_buffer?: gpu_buffer_t;
	structure?: gpu_vertex_structure_t;
};

declare type vertex_array_t = {
	attrib?: string;
	data?: string; // short4norm, short2norm
	values?: i16_array_t;
};

declare type camera_data_t = {
	name?: string;
	near_plane?: f32;
	far_plane?: f32;
	fov?: f32;
	aspect?: f32;
	frustum_culling?: bool;
	ortho?: f32_array_t; // Indicates ortho camera, left, right, bottom, top
};

declare type material_data_t = {
	name?: string;
	shader?: string;
	contexts?: material_context_t[];
	_?: material_data_runtime_t;
};

declare type material_data_runtime_t = {
	uid?: f32;
	shader?: shader_data_t;
};

declare type material_context_t = {
	name?: string;
	bind_constants?: bind_const_t[];
	bind_textures?: bind_tex_t[];
	_?: material_context_runtime_t;
};

declare type material_context_runtime_t = {
	textures?: gpu_texture_t[];
};

declare type bind_const_t = {
	name?: string;
	vec?: f32_array_t; // bool (vec[0] > 0) | i32 | f32 | vec2 | vec3 | vec4
};

declare type bind_tex_t = {
	name?: string;
	file?: string;
};

declare type shader_data_t = {
	name?: string;
	contexts?: shader_context_t[];
};

declare type shader_context_t = {
	name?: string;
	depth_write?: bool;
	compare_mode?: string;
	cull_mode?: string;
	vertex_shader?: string;
	fragment_shader?: string;
	shader_from_source?: bool; // Build shader at runtime using from_source()
	blend_source?: string;
	blend_destination?: string;
	alpha_blend_source?: string;
	alpha_blend_destination?: string;
	color_writes_red?: bool[]; // Per target masks
	color_writes_green?: bool[];
	color_writes_blue?: bool[];
	color_writes_alpha?: bool[];
	color_attachments?: string[]; // RGBA32, RGBA64, R8
	depth_attachment?: string;    // D32
	vertex_elements?: vertex_element_t[];
	constants?: shader_const_t[];
	texture_units?: tex_unit_t[];
	_?: shader_context_runtime_t;
};

declare type shader_context_runtime_t = {
	pipe?: gpu_pipeline_t;
	constants?: i32[];
	tex_units?: i32[];
	structure?: gpu_vertex_structure_t;
	vertex_shader_size?: i32;
	fragment_shader_size?: i32;
};

declare type vertex_element_t = {
	name?: string;
	data?: string; // "short4norm", "short2norm"
};

declare type shader_const_t = {
	name?: string;
	type?: string;
	link?: string;
};

declare type tex_unit_t = {
	name?: string;
	link?: string;
};

declare type world_data_t = {
	name?: string;
	color?: i32;
	strength?: f32;
	irradiance?: string; // Reference to irradiance_t blob
	radiance?: string;
	radiance_mipmaps?: i32;
	envmap?: string;
	_?: world_data_runtime_t;
};

declare type world_data_runtime_t = {
	envmap?: gpu_texture_t;
	radiance?: gpu_texture_t;
	radiance_mipmaps?: gpu_texture_t[];
	irradiance?: f32_array_t;
};

declare type irradiance_t = {
	irradiance?: f32_array_t; // Blob with spherical harmonics, bands 0,1,2
};

declare type obj_t = {
	name?: string;
	type?: string; // object, mesh_object, camera_object
	data_ref?: string;
	transform?: f32_array_t;
	dimensions?: f32_array_t; // Geometry objects
	visible?: bool;
	spawn?: bool;  // Auto add object when creating scene
	anim?: any; // TODO: deprecated
	material_ref?: string;
	children?: obj_t[];
	_?: obj_runtime_t;
};

declare type obj_runtime_t = {
	_gc?: scene_t; // Link to armpack_decode result
};

declare type scene_t = {
	name?: string;
	objects?: obj_t[];
	mesh_datas?: mesh_data_t[];
	camera_datas?: camera_data_t[];
	camera_ref?: string;
	material_datas?: material_data_t[];
	shader_datas?: shader_data_t[];
	world_datas?: world_data_t[];
	world_ref?: string;
	speaker_datas?: any; // TODO: deprecated
	embedded_datas?: string[]; // Preload for this scene, images only for now
};

declare let data_cached_scene_raws: map_t<string, scene_t>;
declare let data_cached_meshes: map_t<string, mesh_data_t>;
declare let data_cached_cameras: map_t<string, camera_data_t>;
declare let data_cached_materials: map_t<string, material_data_t>;
declare let data_cached_worlds: map_t<string, world_data_t>;
declare let data_cached_shaders: map_t<string, shader_data_t>;
declare let data_cached_blobs: map_t<string, buffer_t>;
declare let data_cached_images: map_t<string, gpu_texture_t>;
declare let data_cached_videos: map_t<string, video_t>;
declare let data_cached_fonts: map_t<string, draw_font_t>;
/// if arm_audio
declare let data_cached_sounds: map_t<string, sound_t>;
/// end
declare let data_assets_loaded: i32;

declare function data_get_mesh(file: string, name: string): mesh_data_t;
declare function data_get_camera(file: string, name: string): camera_data_t;
declare function data_get_material(file: string, name: string): material_data_t;
declare function data_get_world(file: string, name: string): world_data_t;
declare function data_get_shader(file: string, name: string): shader_data_t;
declare function data_get_scene_raw(file: string): scene_t;
declare function data_get_blob(file: string): buffer_t;
declare function data_get_image(file: string): gpu_texture_t;
declare function data_get_video(file: string): video_t;
declare function data_get_font(file: string): draw_font_t;
declare function data_get_sound(file: string): sound_t;
declare function data_delete_mesh(handle: string);
declare function data_delete_blob(handle: string);
declare function data_delete_image(handle: string);
declare function data_delete_video(handle: string);
declare function data_delete_font(handle: string);
declare function data_delete_sound(handle: string);
declare function data_path(): string;
declare function data_is_abs(file: string): bool;
declare function data_is_up(file: string): bool;
declare function data_resolve_path(file: string): string;


declare let scene_camera: camera_object_t;
declare let scene_world: world_data_t;
declare let scene_meshes: mesh_object_t[];
declare let scene_cameras: camera_object_t[];
declare let scene_empties: object_t[];
declare let scene_embedded: map_t<string, gpu_texture_t>;
declare let _scene_uid_counter: i32;
declare let _scene_uid: i32;
declare let _scene_raw: scene_t;
declare let _scene_root: object_t;
declare let _scene_scene_parent: object_t;
declare let _scene_objects_traversed: i32;
declare let _scene_objects_count: i32;

declare function scene_create(format: scene_t): object_t;
declare function scene_remove();
declare function scene_set_active(scene_name: string): object_t;
declare function scene_render_frame();
declare function scene_add_object(parent: object_t = null): object_t;
declare function scene_get_child(name: string): object_t;
declare function scene_add_mesh_object(data: mesh_data_t, material: material_data_t, parent: object_t = null): mesh_object_t;
declare function scene_add_camera_object(data: camera_data_t, parent: object_t = null): camera_object_t;
declare function scene_traverse_objects(format: scene_t, parent: object_t, objects: obj_t[]);
declare function scene_add_scene(scene_name: string, parent: object_t): object_t;
declare function scene_get_objects_count(objects: obj_t[]): i32;
declare function _scene_spawn_object_tree(obj: obj_t, parent: object_t, spawn_children: bool): object_t;
declare function scene_spawn_object(name: string, parent: object_t = null, spawn_children: bool = true): object_t;
declare function scene_get_raw_object_by_name(format: scene_t, name: string): obj_t;
declare function scene_traverse_objs(children: obj_t[], name: string): obj_t;
declare function scene_create_object(o: obj_t, format: scene_t, parent: object_t): object_t;
declare function scene_create_mesh_object(o: obj_t, format: scene_t, parent: object_t, material: material_data_t): object_t;
declare function scene_return_mesh_object(object_file: string, data_ref: string, material: material_data_t, parent: object_t, o: obj_t): object_t;
declare function scene_return_object(object: object_t, o: obj_t): object_t;
declare function scene_gen_transform(object: obj_t, transform: transform_t);
declare function scene_load_embedded_data(datas: string[]);
declare function scene_embed_data(file: string);


declare type render_target_t = {
	name?: string;
	width?: i32;
	height?: i32;
	// Opt
	format?: string;
	scale?: f32;
	// Runtime
	_image?: gpu_texture_t;
};

declare type cached_shader_context_t = {
	context?: shader_context_t;
};

declare let render_path_commands: () => void;
declare let render_path_render_targets: map_t<string, render_target_t>;
declare let render_path_current_w: i32;
declare let render_path_current_h: i32;
declare let _render_path_frame_time: f32;
declare let _render_path_frame: i32;
declare let _render_path_current_target: render_target_t;
declare let _render_path_current_image: gpu_texture_t;
declare let _render_path_paused: bool;
declare let _render_path_last_w: i32;
declare let _render_path_last_h: i32;
declare let _render_path_bind_params: string[];
declare let _render_path_last_frame_time: f32                                           = 0.0;
declare let _render_path_loading: i32                                                   = 0;
declare let _render_path_cached_shader_contexts: map_t<string, cached_shader_context_t> = map_create();

declare function render_path_ready(): bool;
declare function render_path_render_frame();
declare function render_path_set_target(target: string, additional: string[] = null, depth_buffer: string = null, flags: gpu_clear_t = gpu_clear_t.NONE, color: i32 = 0, depth: f32 = 0.0);
declare function render_path_end();
declare function render_path_draw_meshes(context: string);
declare function render_path_submit_draw(context: string);
declare function render_path_draw_skydome(handle: string);
declare function render_path_bind_target(target: string, uniform: string);
declare function render_path_draw_shader(handle: string);
declare function render_path_load_shader(handle: string);
declare function render_path_resize();
declare function render_path_create_render_target(t: render_target_t): render_target_t;
declare function render_path_create_image(t: render_target_t): gpu_texture_t;
declare function render_path_get_tex_format(s: string): gpu_texture_format_t;
declare function render_target_create(): render_target_t;
