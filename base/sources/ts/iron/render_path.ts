
type render_target_t = {
	name?: string;
	width?: i32;
	height?: i32;
	// Opt
	format?: string;
	scale?: f32;
	// Runtime
	_image?: gpu_texture_t;
};

type cached_shader_context_t = {
	context?: shader_context_t;
};

enum draw_order_t {
	DIST, // Early-z
	SHADER, // Less state changes
}

let render_path_commands: ()=>void = null;
let render_path_render_targets: map_t<string, render_target_t> = map_create();
let render_path_current_w: i32;
let render_path_current_h: i32;
let _render_path_frame_time: f32 = 0.0;
let _render_path_frame: i32 = 0;
let _render_path_current_target: render_target_t = null;
let _render_path_current_image: gpu_texture_t = null;
let _render_path_draw_order: draw_order_t = draw_order_t.DIST;
let _render_path_paused: bool = false;
let _render_path_last_w: i32 = 0;
let _render_path_last_h: i32 = 0;
let _render_path_bind_params: string[];
let _render_path_meshes_sorted: bool;
let _render_path_last_frame_time: f32 = 0.0;
let _render_path_loading: i32 = 0;
let _render_path_cached_shader_contexts: map_t<string, cached_shader_context_t> = map_create();

function render_path_ready(): bool {
	return _render_path_loading == 0;
}

function render_path_render_frame() {
	if (!render_path_ready() || _render_path_paused || sys_w() == 0 || sys_h() == 0) {
		return;
	}

	if (_render_path_last_w > 0 && (_render_path_last_w != sys_w() || _render_path_last_h != sys_h())) {
		render_path_resize();
	}
	_render_path_last_w = sys_w();
	_render_path_last_h = sys_h();

	_render_path_frame_time = sys_time() -_render_path_last_frame_time;
	_render_path_last_frame_time = sys_time();

	render_path_current_w = sys_w();
	render_path_current_h = sys_h();
	_render_path_meshes_sorted = false;

	render_path_commands();

	_render_path_frame++;
}

function render_path_set_target(target: string, additional: string[] = null, depth_buffer: string = null, flags: i32 = clear_flag_t.NONE, color: i32 = 0, depth: f32 = 0.0) {
	if (_render_path_current_image != null) {
		render_path_end();
	}

	if (target == "") { // Framebuffer
		_render_path_current_target = null;
		render_path_current_w = sys_w();
		render_path_current_h = sys_h();
		_gpu_begin(null, null, null, flags, color, depth);
		gpu_viewport(sys_x(), render_path_current_h - (sys_h() - sys_y()), sys_w(), sys_h());
	}
	else { // Render target
		let rt: render_target_t = map_get(render_path_render_targets, target);
		_render_path_current_target = rt;
		let additional_images: gpu_texture_t[] = null;
		if (additional != null) {
			additional_images = [];
			for (let i: i32 = 0; i < additional.length; ++i) {
				let s: string = additional[i];
				let t: render_target_t = map_get(render_path_render_targets, s);
				array_push(additional_images, t._image);
			}
		}
		render_path_current_w = rt._image.width;
		render_path_current_h = rt._image.height;
		let db: render_target_t = map_get(render_path_render_targets, depth_buffer);
		_render_path_current_image = rt._image;
		_gpu_begin(rt._image, additional_images, db != null ? db._image : null, flags, color, depth);
	}
	_render_path_bind_params = null;
}

function render_path_end() {
	gpu_end();
	_render_path_current_image = null;
	_render_path_bind_params = null;
}

function _render_path_sort_dist(a: any_ptr, b: any_ptr): i32 {
	let ma: mesh_object_t = DEREFERENCE(a);
	let mb: mesh_object_t = DEREFERENCE(b);
	return ma.camera_dist >= mb.camera_dist ? 1 : -1;
}

function render_path_sort_meshes_dist(meshes: mesh_object_t[]) {
	array_sort(meshes, _render_path_sort_dist);
}

function _render_path_sort_shader(a: any_ptr, b: any_ptr): i32 {
	let ma: mesh_object_t = DEREFERENCE(a);
	let mb: mesh_object_t = DEREFERENCE(b);
	return strcmp(ma.materials[0].name, mb.materials[0].name);
}

function render_path_sort_meshes_shader(meshes: mesh_object_t[]) {
	array_sort(meshes, _render_path_sort_shader);
}

function render_path_draw_meshes(context: string) {
	render_path_submit_draw(context);
	render_path_end();
}

function render_path_submit_draw(context: string) {
	let camera: camera_object_t = scene_camera;
	let meshes: mesh_object_t[] = scene_meshes;
	_mesh_object_last_pipeline = null;

	if (!_render_path_meshes_sorted && camera != null) { // Order max once per frame for now
		let cam_x: f32 = transform_world_x(camera.base.transform);
		let cam_y: f32 = transform_world_y(camera.base.transform);
		let cam_z: f32 = transform_world_z(camera.base.transform);
		for (let i: i32 = 0; i < meshes.length; ++i) {
			let mesh: mesh_object_t = meshes[i];
			mesh_object_compute_camera_dist(mesh, cam_x, cam_y, cam_z);
		}
		if (_render_path_draw_order == draw_order_t.SHADER) {
			render_path_sort_meshes_shader(meshes);
		}
		else {
			render_path_sort_meshes_dist(meshes);
		}
		_render_path_meshes_sorted = true;
	}

	for (let i: i32 = 0; i < meshes.length; ++i) {
		let mesh: mesh_object_t = meshes[i];
		mesh_object_render(mesh, context, _render_path_bind_params);
	}
}

function render_path_draw_skydome(handle: string) {
	if (const_data_skydome_vb == null) {
		const_data_create_skydome_data();
	}
	let cc: cached_shader_context_t = map_get(_render_path_cached_shader_contexts, handle);
	if (cc.context == null) {
		return; // World data not specified
	}
	gpu_set_pipeline(cc.context._.pipe);
	uniforms_set_context_consts(cc.context, _render_path_bind_params);
	uniforms_set_obj_consts(cc.context, null); // External hosek
	gpu_set_vertex_buffer(const_data_skydome_vb);
	gpu_set_index_buffer(const_data_skydome_ib);
	gpu_draw();
	render_path_end();
}

function render_path_bind_target(target: string, uniform: string) {
	if (_render_path_bind_params != null) {
		array_push(_render_path_bind_params, target);
		array_push(_render_path_bind_params, uniform);
	}
	else {
		_render_path_bind_params = [target, uniform];
	}
}

// Full-screen triangle
function render_path_draw_shader(handle: string) {
	// file/data_name/context
	let cc: cached_shader_context_t = map_get(_render_path_cached_shader_contexts, handle);
	if (const_data_screen_aligned_vb == null) {
		const_data_create_screen_aligned_data();
	}
	gpu_set_pipeline(cc.context._.pipe);
	uniforms_set_context_consts(cc.context, _render_path_bind_params);
	uniforms_set_obj_consts(cc.context, null);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();

	render_path_end();
}

function render_path_load_shader(handle: string) {
	_render_path_loading++;
	let cc: cached_shader_context_t = map_get(_render_path_cached_shader_contexts, handle);
	if (cc != null) {
		_render_path_loading--;
		return;
	}

	cc = {};
	map_set(_render_path_cached_shader_contexts, handle, cc);

	// file/data_name/context
	let shader_path: string[] = string_split(handle, "/");

	let res: shader_data_t = data_get_shader(shader_path[0], shader_path[1]);
	cc.context = shader_data_get_context(res, shader_path[2]);
	_render_path_loading--;
}

function render_path_unload_shader(handle: string) {
	map_delete(_render_path_cached_shader_contexts, handle);

	// file/data_name/context
	let shader_path: string[] = string_split(handle, "/");
	// Todo: Handle context overrides (see data_get_shader())
	map_delete(data_cached_shaders, shader_path[1]);
}

function render_path_unload() {
	let render_targets_keys: string[] = map_keys(render_path_render_targets);
	for (let i: i32 = 0; i < render_targets_keys.length; ++i) {
		let rt: render_target_t = map_get(render_path_render_targets, render_targets_keys[i]);
		render_target_unload(rt);
	}
}

function render_path_resize() {
	if (iron_window_width() == 0 || iron_window_height() == 0) {
		return;
	}

	let render_targets_keys: string[] = map_keys(render_path_render_targets);
	for (let i: i32 = 0; i < render_targets_keys.length; ++i) {
		let rt: render_target_t = map_get(render_path_render_targets, render_targets_keys[i]);
		if (rt != null && rt.width == 0) {
			iron_delete_texture(rt._image);
			rt._image = render_path_create_image(rt);
		}
	}
}

function render_path_create_render_target(t: render_target_t): render_target_t {
	t._image = render_path_create_image(t);
	map_set(render_path_render_targets, t.name, t);
	return t;
}

function render_path_create_image(t: render_target_t): gpu_texture_t {
	let width: i32 = t.width == 0 ? sys_w() : t.width;
	let height: i32 = t.height == 0 ? sys_h() : t.height;
	width = math_floor(width * t.scale);
	height = math_floor(height * t.scale);
	if (width < 1) {
		width = 1;
	}
	if (height < 1) {
		height = 1;
	}
	return gpu_create_render_target(width, height,
		t.format != null ? render_path_get_tex_format(t.format) : tex_format_t.RGBA32);
}

function render_path_get_tex_format(s: string): tex_format_t {
	if (s == "RGBA32") {
		return tex_format_t.RGBA32;
	}
	if (s == "RGBA64") {
		return tex_format_t.RGBA64;
	}
	if (s == "RGBA128") {
		return tex_format_t.RGBA128;
	}
	if (s == "R32") {
		return tex_format_t.R32;
	}
	if (s == "R16") {
		return tex_format_t.R16;
	}
	if (s == "R8") {
		return tex_format_t.R8;
	}
	if (s == "D32") {
		return tex_format_t.D32;
	}
	return tex_format_t.RGBA32;
}

function render_target_create(): render_target_t {
	let raw: render_target_t = {};
	raw.scale = 1.0;
	return raw;
}

function render_target_unload(raw: render_target_t) {
	if (raw._image != null) {
		iron_delete_texture(raw._image);
	}
}
