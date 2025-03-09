
type sys_callback_t = {
	f?: ()=>void;
};

type sys_string_callback_t = {
	f?: (s: string)=>void;
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
let _sys_shaders: map_t<string, iron_g5_shader_t> = map_create();

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

	use_depth?: bool;

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
		iron_load_blob(data_path() + "draw_colored.vert" + sys_shader_ext()),
		iron_load_blob(data_path() + "draw_colored.frag" + sys_shader_ext()),
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

function sys_get_shader(name: string): iron_g5_shader_t {
	let shader: iron_g5_shader_t = map_get(_sys_shaders, name);
	if (shader == null) {
		shader = iron_g4_create_shader(
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
