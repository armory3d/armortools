
type sys_callback_t = {
	f?: ()=>void;
};

type sys_string_callback_t = {
	f?: (s: string)=>void;
};

type callback_t = {
	f?: (data?: any)=>void;
	data?: any;
};

let _sys_render_listeners: sys_callback_t[] = [];
let _sys_foreground_listeners: sys_callback_t[] = [];
let _sys_resume_listeners: sys_callback_t[] = [];
let _sys_pause_listeners: sys_callback_t[] = [];
let _sys_background_listeners: sys_callback_t[] = [];
let _sys_shutdown_listeners: sys_callback_t[] = [];
let _sys_drop_files_listeners: sys_string_callback_t[] = [];

let _sys_start_time: f32;
let _sys_window_title: string;
let _sys_shaders: map_t<string, gpu_shader_t> = map_create();

declare type iron_window_options_t = {
	title?: string;
	x?: i32;
	y?: i32;
	width?: i32;
	height?: i32;
	features?: window_features_t;
	mode?: window_mode_t;
	frequency?: i32;
	vsync?: bool;
	display_index?: i32;
	visible?: bool;
	color_bits?: i32;
	depth_bits?: i32;
};

function sys_start(ops: iron_window_options_t) {
	_iron_init(ops);

	_sys_start_time = iron_time();
	draw_init(
		iron_load_blob(data_path() + "draw_image.vert" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_image.frag" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_rect.vert" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_rect.frag" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_tris.vert" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_tris.frag" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_text.vert" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_text.frag" + sys_shader_ext())
	);
	_iron_set_update_callback(sys_render_callback);
	_iron_set_drop_files_callback(sys_drop_files_callback);
	iron_set_application_state_callback(sys_foreground_callback, sys_resume_callback, sys_pause_callback, sys_background_callback, sys_shutdown_callback);
	iron_set_keyboard_down_callback(sys_keyboard_down_callback);
	iron_set_keyboard_up_callback(sys_keyboard_up_callback);
	iron_set_keyboard_press_callback(sys_keyboard_press_callback);
	iron_set_mouse_down_callback(sys_mouse_down_callback);
	iron_set_mouse_up_callback(sys_mouse_up_callback);
	iron_set_mouse_move_callback(sys_mouse_move_callback);
	iron_set_mouse_wheel_callback(sys_mouse_wheel_callback);
	iron_set_touch_down_callback(sys_touch_down_callback);
	iron_set_touch_up_callback(sys_touch_up_callback);
	iron_set_touch_move_callback(sys_touch_move_callback);
	iron_set_pen_down_callback(sys_pen_down_callback);
	iron_set_pen_up_callback(sys_pen_up_callback);
	iron_set_pen_move_callback(sys_pen_move_callback);
	iron_set_gamepad_axis_callback(sys_gamepad_axis_callback);
	iron_set_gamepad_button_callback(sys_gamepad_button_callback);
	input_register();
}

function _sys_callback_create(f: ()=>void): sys_callback_t {
	let cb: sys_callback_t = {};
	cb.f = f;
	return cb;
}

function sys_notify_on_frames(listener: ()=>void) {
	array_push(_sys_render_listeners, _sys_callback_create(listener));
}

function sys_notify_on_app_state(on_foreground: ()=>void, on_resume: ()=>void, on_pause: ()=>void, on_background: ()=>void, on_shutdown: ()=>void) {
	if (on_foreground != null) {
		array_push(_sys_foreground_listeners, _sys_callback_create(on_foreground));
	}
	if (on_resume != null) {
		array_push(_sys_resume_listeners, _sys_callback_create(on_resume));
	}
	if (on_pause != null) {
		array_push(_sys_pause_listeners, _sys_callback_create(on_pause));
	}
	if (on_background != null) {
		array_push(_sys_background_listeners, _sys_callback_create(on_background));
	}
	if (on_shutdown != null) {
		array_push(_sys_shutdown_listeners, _sys_callback_create(on_shutdown));
	}
}

function sys_notify_on_drop_files(drop_files_listener: (s: string)=>void) {
	let cb: sys_string_callback_t = {};
	cb.f = drop_files_listener;
	array_push(_sys_drop_files_listeners, cb);
}

function sys_foreground() {
	for (let i: i32 = 0; i < _sys_foreground_listeners.length; ++i) {
		_sys_foreground_listeners[i].f();
	}
}

function sys_resume() {
	for (let i: i32 = 0; i < _sys_resume_listeners.length; ++i) {
		_sys_resume_listeners[i].f();
	}
}

function sys_pause() {
	for (let i: i32 = 0; i < _sys_pause_listeners.length; ++i) {
		_sys_pause_listeners[i].f();
	}
}

function sys_background() {
	for (let i: i32 = 0; i < _sys_background_listeners.length; ++i) {
		_sys_background_listeners[i].f();
	}
}

function sys_shutdown() {
	for (let i: i32 = 0; i < _sys_shutdown_listeners.length; ++i) {
		_sys_shutdown_listeners[i].f();
	}
}

function sys_drop_files(file_path: string) {
	for (let i: i32 = 0; i < _sys_drop_files_listeners.length; ++i) {
		_sys_drop_files_listeners[i].f(file_path);
	}
}

function sys_time(): f32 {
	return iron_time() - _sys_start_time;
}

function sys_render_callback() {
	for (let i: i32 = 0; i < _sys_render_listeners.length; ++i) {
		_sys_render_listeners[i].f();
	}
}

function sys_drop_files_callback(file_path: string) {
	sys_drop_files(file_path);
}

function sys_foreground_callback() {
	sys_foreground();
}

function sys_resume_callback() {
	sys_resume();
}

function sys_pause_callback() {
	sys_pause();
}

function sys_background_callback() {
	sys_background();
}

function sys_shutdown_callback() {
	sys_shutdown();
}

function sys_keyboard_down_callback(code: i32) {
	keyboard_down_listener(code);
}

function sys_keyboard_up_callback(code: i32) {
	keyboard_up_listener(code);
}

function sys_keyboard_press_callback(char_code: i32) {
	keyboard_press_listener(string_from_char_code(char_code));
}

function sys_mouse_down_callback(button: i32, x: i32, y: i32) {
	mouse_down_listener(button, x, y);
}

function sys_mouse_up_callback(button: i32, x: i32, y: i32) {
	mouse_up_listener(button, x, y);
}

function sys_mouse_move_callback(x: i32, y: i32, mx: i32, my: i32) {
	mouse_move_listener(x, y, mx, my);
}

function sys_mouse_wheel_callback(delta: i32) {
	mouse_wheel_listener(delta);
}

function sys_touch_down_callback(index: i32, x: i32, y: i32) {
	///if (arm_android || arm_ios)
	mouse_on_touch_down(index, x, y);
	///end
}

function sys_touch_up_callback(index: i32, x: i32, y: i32) {
	///if (arm_android || arm_ios)
	mouse_on_touch_up(index, x, y);
	///end
}

function sys_touch_move_callback(index: i32, x: i32, y: i32) {
	///if (arm_android || arm_ios)
	mouse_on_touch_move(index, x, y);
	///end
}

function sys_pen_down_callback(x: i32, y: i32, pressure: f32) {
	pen_down_listener(x, y, pressure);
}

function sys_pen_up_callback(x: i32, y: i32, pressure: f32) {
	pen_up_listener(x, y, pressure);
}

function sys_pen_move_callback(x: i32, y: i32, pressure: f32) {
	pen_move_listener(x, y, pressure);
}

function sys_gamepad_axis_callback(gamepad: i32, axis: i32, value: f32) {
	gamepad_axis_listener(gamepad, axis, value);
}

function sys_gamepad_button_callback(gamepad: i32, button: i32, value: f32) {
	gamepad_button_listener(gamepad, button, value);
}

function sys_title(): string {
	return _sys_window_title;
}

function sys_title_set(value: string) {
	iron_set_window_title(value);
	_sys_window_title = value;
}

function sys_display_primary_id(): i32 {
	for (let i: i32 = 0; i < iron_count_displays(); ++i) {
		if (iron_display_is_primary(i)) {
			return i;
		}
	}
	return 0;
}

function sys_display_width(): i32 {
	return iron_display_width(sys_display_primary_id());
}

function sys_display_height(): i32 {
	return iron_display_height(sys_display_primary_id());
}

function sys_display_frequency(): i32 {
	return iron_display_frequency(sys_display_primary_id());
}

function sys_buffer_to_string(b: buffer_t): string {
	let str: string = string_alloc(b.length + 1);
	memcpy(str, b.buffer, b.length);
	return str;
}

function sys_string_to_buffer(str: string): buffer_t {
	let b: u8_array_t = u8_array_create(str.length);
	memcpy(b.buffer, str, str.length);
	return b;
}

function sys_shader_ext(): string {
	///if arm_vulkan
	return ".spirv";
	///elseif arm_metal
	return ".metal";
	///else
	return ".d3d11";
	///end
}

function sys_get_shader(name: string): gpu_shader_t {
	let shader: gpu_shader_t = map_get(_sys_shaders, name);
	if (shader == null) {
		shader = gpu_create_shader(
			iron_load_blob(data_path() + name + sys_shader_ext()),
			ends_with(name, ".frag") ? shader_type_t.FRAGMENT : shader_type_t.VERTEX
		);
		map_set(_sys_shaders, name, shader);
	}
	return shader;
}

function video_unload(self: video_t) {}

///if arm_audio
function sound_create(sound_: any): sound_t {
	let raw: sound_t = {};
	raw.sound_ = sound_;
	return raw;
}

function sound_unload(raw: sound_t) {
	iron_a1_sound_destroy(raw.sound_);
}
///end

type color_t = i32;

type video_t = {
	video_?: any;
};

type sound_t = {
	sound_?: any;
};

enum window_features_t {
    NONE = 0,
    RESIZABLE = 1,
    MINIMIZABLE = 2,
    MAXIMIZABLE = 4,
}

enum window_mode_t {
	WINDOWED,
	FULLSCREEN,
}

let _sys_on_resets: callback_t[] = [];
let _sys_on_next_frames: callback_t[] = [];
let _sys_on_end_frames: callback_t[] = [];
let _sys_on_inits: callback_t[] = [];
let _sys_on_updates: callback_t[] = [];
let _sys_on_renders: callback_t[] = [];
let _sys_on_renders_2d: callback_t[] = [];
let _sys_pause_updates: bool = false;
let _sys_lastw: i32 = -1;
let _sys_lasth: i32 = -1;
let sys_on_resize: ()=>void = null;
let sys_on_w: ()=>i32 = null;
let sys_on_h: ()=>i32 = null;
let sys_on_x: ()=>i32 = null;
let sys_on_y: ()=>i32 = null;

function sys_w(): i32 {
	if (sys_on_w != null) {
		return sys_on_w();
	}
	return iron_window_width();
}

function sys_h(): i32 {
	if (sys_on_h != null) {
		return sys_on_h();
	}
	return iron_window_height();
}

function sys_x(): i32 {
	if (sys_on_x != null) {
		return sys_on_x();
	}
	return 0;
}

function sys_y(): i32 {
	if (sys_on_y != null) {
		return sys_on_y();
	}
	return 0;
}

function sys_init() {
	sys_notify_on_frames(sys_render);
}

function sys_reset() {
	_sys_on_next_frames = [];
	_sys_on_end_frames = [];
	_sys_on_inits = [];
	_sys_on_updates = [];
	_sys_on_renders = [];
	_sys_on_renders_2d = [];
	for (let i: i32 = 0; i < _sys_on_resets.length; ++i) {
		let cb: callback_t = _sys_on_resets[i];
		cb.f(cb.data);
	}
}

function _sys_run_callbacks(cbs: callback_t[]) {
	for (let i: i32 = 0; i < cbs.length; ++i) {
		let cb: callback_t = cbs[i];
		cb.f(cb.data);
	}
}

function sys_update() {
	if (!_scene_ready) {
		return;
	}
	if (_sys_pause_updates) {
		return;
	}

	if (_sys_on_next_frames.length > 0) {
		_sys_run_callbacks(_sys_on_next_frames);
		array_splice(_sys_on_next_frames, 0, _sys_on_next_frames.length);
	}

	scene_update_frame();

	let i: i32 = 0;
	let l: i32 = _sys_on_updates.length;
	while (i < l) {
		if (_sys_on_inits.length > 0) {
			_sys_run_callbacks(_sys_on_inits);
			array_splice(_sys_on_inits, 0, _sys_on_inits.length);
		}

		let cb: callback_t = _sys_on_updates[i];
		cb.f(cb.data);

		// Account for removed traits
		if (l <= _sys_on_updates.length) {
			i++;
		}
		else {
			l = _sys_on_updates.length;
		}
	}

	_sys_run_callbacks(_sys_on_end_frames);

	// Rebuild projection on window resize
	if (_sys_lastw == -1) {
		_sys_lastw = sys_w();
		_sys_lasth = sys_h();
	}
	if (_sys_lastw != sys_w() || _sys_lasth != sys_h()) {
		if (sys_on_resize != null) {
			sys_on_resize();
		}
		else if (scene_camera != null) {
			camera_object_build_proj(scene_camera);
		}
	}
	_sys_lastw = sys_w();
	_sys_lasth = sys_h();
}

let _sys_time_last: f32 = 0.0;
let _sys_time_real_delta: f32 = 0.0;
let _sys_time_frequency: i32 = -1;

function sys_delta(): f32 {
	if (_sys_time_frequency < 0) {
		_sys_time_frequency = sys_display_frequency();
	}
	return (1 / _sys_time_frequency);
}

function sys_real_delta(): f32 {
	return _sys_time_real_delta;
}

function sys_render() {
	sys_update();

	_sys_time_real_delta = sys_time() - _sys_time_last;
	_sys_time_last = sys_time();

	if (!_scene_ready) {
		sys_render_2d();
		return;
	}

	if (_sys_on_inits.length > 0) {
		_sys_run_callbacks(_sys_on_inits);
		array_splice(_sys_on_inits, 0, _sys_on_inits.length);
	}

	scene_render_frame();
	_sys_run_callbacks(_sys_on_renders);
	sys_render_2d();
}

function sys_render_2d() {
	if (_sys_on_renders_2d.length > 0) {
		_sys_run_callbacks(_sys_on_renders_2d);
	}
}

function _callback_create(f: (data?: any)=>void, data: any): callback_t {
	let cb: callback_t = {};
	cb.f = f;
	cb.data = data;
	return cb;
}

// Hooks
function sys_notify_on_init(f: (data?: any)=>void, data: any = null) {
	array_push(_sys_on_inits, _callback_create(f, data));
}

function sys_notify_on_update(f: (data?: any)=>void, data: any = null) {
	array_push(_sys_on_updates, _callback_create(f, data));
}

function sys_notify_on_render(f: (data?: any)=>void, data: any = null) {
	array_push(_sys_on_renders, _callback_create(f, data));
}

function sys_notify_on_render_2d(f: (data?: any)=>void, data: any = null) {
	array_push(_sys_on_renders_2d, _callback_create(f, data));
}

function sys_notify_on_reset(f: (data?: any)=>void, data: any = null) {
	array_push(_sys_on_resets, _callback_create(f, data));
}

function sys_notify_on_next_frame(f: (data?: any)=>void, data: any = null) {
	array_push(_sys_on_next_frames, _callback_create(f, data));
}

function sys_notify_on_end_frame(f: (data?: any)=>void, data: any = null) {
	array_push(_sys_on_end_frames, _callback_create(f, data));
}

function _sys_remove_callback(ar: callback_t[], f: (data?: any)=>void) {
	for (let i: i32 = 0; i < ar.length; ++i) {
		if (ar[i].f == f) {
			array_splice(ar, i, 1);
			break;
		}
	}
}

function sys_remove_init(f: (data?: any)=>void) {
	_sys_remove_callback(_sys_on_inits, f);
}

function sys_remove_update(f: (data?: any)=>void) {
	_sys_remove_callback(_sys_on_updates, f);
}

function sys_remove_render(f: (data?: any)=>void) {
	_sys_remove_callback(_sys_on_renders, f);
}

function sys_remove_render_2d(f: (data?: any)=>void) {
	_sys_remove_callback(_sys_on_renders_2d, f);
}

function sys_remove_reset(f: (data?: any)=>void) {
	_sys_remove_callback(_sys_on_resets, f);
}

function sys_remove_end_frame(f: (data?: any)=>void) {
	_sys_remove_callback(_sys_on_end_frames, f);
}

