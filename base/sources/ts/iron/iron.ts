
declare type i8 = number;
declare type i16 = number;
declare type i32 = number;
declare type i64 = number;
declare type u8 = number;
declare type u16 = number;
declare type u32 = number;
declare type u64 = number;
declare type f32 = number;
declare type f64 = number;
declare type bool = boolean;
declare type any_ptr = any;
declare type i8_ptr = any;
declare type i16_ptr = any;
declare type i32_ptr = any;
declare type i64_ptr = any;
declare type u8_ptr = any;
declare type u16_ptr = any;
declare type u32_ptr = any;
declare type u64_ptr = any;
declare type f32_ptr = any;
declare type f64_ptr = any;
declare let __ID__: string; // file:line - ts preprocessor

declare type map_t<K, V> = any;
declare type buffer_t = u8_array_t;
declare type f32_array_t = any;
declare type u32_array_t = any;
declare type i32_array_t = any;
declare type u16_array_t = any;
declare type i16_array_t = any;
declare type u8_array_t = any;
declare type i8_array_t = any;

declare function DEREFERENCE(a: any): any;
declare function ADDRESS(a: any): any;
declare function ARRAY_ACCESS(a: any, i: i32): any;
declare function map_create<K, V>(): map_t<K, V>;
declare function buffer_create(length: i32): buffer_t;
declare function buffer_create_from_raw(from: u8_ptr, length: i32): buffer_t;
declare function f32_array_create(length: i32): f32_array_t;
declare function f32_array_create_from_buffer(b: buffer_t): f32_array_t;
declare function f32_array_create_from_array(a: f32[]): f32_array_t;
declare function f32_array_create_x(x: f32): void;
declare function f32_array_create_xy(x: f32, y: f32): void;
declare function f32_array_create_xyz(x: f32, y: f32, z: f32): void;
declare function f32_array_create_xyzw(x: f32, y: f32, z: f32, w: f32): void;
declare function f32_array_create_xyzwv(x: f32, y: f32, z: f32, w: f32, v: f32): void;
declare function u32_array_create(length: i32): u32_array_t;
declare function u32_array_create_from_array(a: u32[]): u32_array_t;
declare function i32_array_create(length: i32): i32_array_t;
declare function i32_array_create_from_array(a: i32[]): i32_array_t;
declare function u16_array_create(length: i32): u16_array_t;
declare function i16_array_create(length: i32): i16_array_t;
declare function i16_array_create_from_array(a: i16[]): void;
declare function u8_array_create(length: i32): u8_array_t;
declare function u8_array_create_from_array(a: u8[]): u8_array_t;
declare function u8_array_create_from_string(s: string): u8_array_t;
declare function u8_array_to_string(a: u8_array_t): string;
declare function i8_array_create(length: i32): i8_array_t;

declare function math_floor(x: f32): f32;
declare function math_cos(x: f32): f32;
declare function math_sin(x: f32): f32;
declare function math_tan(x: f32): f32;
declare function math_sqrt(x: f32): f32;
declare function math_abs(x: f32): f32;
declare function math_random(): f32;
declare function math_atan2(y: f32, x: f32): f32;
declare function math_asin(x: f32): f32;
declare function math_pi(): f32;
declare function math_pow(x: f32, y: f32): f32;
declare function math_round(x: f32): f32;
declare function math_ceil(x: f32): f32;
declare function math_min(x: f32, y: f32): f32;
declare function math_max(x: f32, y: f32): f32;
declare function math_log(x: f32): f32;
declare function math_log2(x: f32): f32;
declare function math_atan(x: f32): f32;
declare function math_acos(x: f32): f32;
declare function math_exp(x: f32): f32;
declare function math_fmod(x: f32, y: f32): f32;

declare function map_get<K, V>(m: map_t<K, V>, k: any): any;
declare function map_set<K, V>(m: map_t<K, V>, k: any, v: any): void;
declare function map_delete<K, V>(m: map_t<K, V>, k: any): void;
declare function map_keys(m: map_t<any, any>): any[];
declare function array_sort(ar: any[], fn: (a: any_ptr, b: any_ptr)=>i32): void;
declare function array_push(ar: any[], e: any): void;
declare function array_pop(ar: any[]): any;
declare function array_shift(ar: any[]): any;
declare function array_splice(ar: any[], start: u32, delete_count: u32): void;
declare function array_slice(a: any[], begin: u32, end: u32): any[];
declare function array_insert(ar: any[], at: u32, e: any): void;
declare function array_concat(a: any[], b: any[]): any[];
declare function array_index_of(a: any[], search: any): i32;
declare function array_reverse(a: any[]): void;
declare function string_index_of(s: string, search: string): i32;
declare function string_index_of_pos(s: string, search: string, pos: u32): i32;
declare function string_last_index_of(s: string, search: string): i32;
declare function string_split(s: string, sep: string): string[];
declare function string_array_join(a: string[], sep: string): string;
declare function string_replace_all(s: string, search: string, replace: string): string;
declare function substring(s: string, start: u32, end: u32): string;
declare function string_from_char_code(c: i32): string;
declare function char_code_at(s: string, i: i32): i32;
declare function char_at(s: string, i: i32): string;
declare function starts_with(s: string, start: string): bool;
declare function ends_with(s: string, end: string): bool;
declare function to_lower_case(s: string): string;
declare function to_upper_case(s: string): string;
declare function buffer_slice(a: buffer_t, begin: u32, end: u32): buffer_t;
declare function buffer_get_u8(b: buffer_t, p: u32): u8;
declare function buffer_get_i8(b: buffer_t, p: u32): i8;
declare function buffer_get_u16(b: buffer_t, p: u32): u16;
declare function buffer_get_i16(b: buffer_t, p: u32): i16;
declare function buffer_get_u32(b: buffer_t, p: u32): u32;
declare function buffer_get_i32(b: buffer_t, p: u32): i32;
declare function buffer_get_f32(b: buffer_t, p: u32): f32;
declare function buffer_get_f64(b: buffer_t, p: u32): f64;
declare function buffer_get_i64(b: buffer_t, p: u32): i32;
declare function buffer_set_u8(b: buffer_t, p: u32, n: u8): void;
declare function buffer_set_i8(b: buffer_t, p: u32, n: i8): void;
declare function buffer_set_u16(b: buffer_t, p: u32, n: u16): void;
declare function buffer_set_i16(b: buffer_t, p: u32, n: i16): void;
declare function buffer_set_u32(b: buffer_t, p: u32, n: u32): void;
declare function buffer_set_i32(b: buffer_t, p: u32, n: i32): void;
declare function buffer_set_f32(b: buffer_t, p: u32, n: f32): void;
declare function parse_int(s: string): i32;
declare function parse_int_hex(s: string): i32;
declare function parse_float(s: string): i32;
declare function i32_to_string(i: i32): string;
declare function i32_to_string_hex(i: i32): string;
declare function i64_to_string(i: i64): string;
declare function u64_to_string(i: u64): string;
declare function f32_to_string(f: f32): string;
declare function json_parse(s: string): any;

declare function json_parse_to_map(s: string): map_t<string, string>;
declare function json_encode_begin(): void;
declare function json_encode_string(k: string, v: string): void;
declare function json_encode_string_array(k: string, v: string[]): void;
declare function json_encode_i32(k: string, v: i32): void;
declare function json_encode_i32_array(k: string, v: i32[]): void;
declare function json_encode_f32(k: string, v: f32): void;
declare function json_encode_bool(k: string, v: bool): void;
declare function json_encode_end(): string;
declare function json_encode_begin_array(k: string): void;
declare function json_encode_end_array(): void;
declare function json_encode_begin_object(): void;
declare function json_encode_end_object(): void;
declare function json_encode_map(m: map_t<string, string>): void;
declare function uri_decode(s: string): string;

declare function js_eval(js: string): f32;
declare function js_call(f: any): string;
declare function js_call_ptr(f: any, arg: any): string;
declare function js_call_ptr_str(f: any, arg0: any, arg1: string): string;
declare function js_pcall_str(f: any, arg0: string): any;
declare function array_remove(ar: any[], e: any): void;
declare function trim_end(str: string): string;
declare function gc_run(): void;
declare function gc_pause(): void;
declare function gc_resume(): void;
declare function gc_free(ptr: any): void;
declare function gc_root(ptr: any): void;
declare function gc_unroot(ptr: any): void;
declare function gc_leaf(ptr: any): void;
declare function sizeof(ptr: any): i32;
declare function memcpy(dst: any, src: any, n: i32): void;

declare function color_from_floats(r: f32, g: f32, b: f32, a: f32): i32;
declare function color_get_rb(c: i32): u8;
declare function color_get_gb(c: i32): u8;
declare function color_get_bb(c: i32): u8;
declare function color_get_ab(c: i32): u8;
declare function color_set_rb(c: i32, i: u8): i32;
declare function color_set_gb(c: i32, i: u8): i32;
declare function color_set_bb(c: i32, i: u8): i32;
declare function color_set_ab(c: i32, i: u8): i32;

declare function _iron_init(ops: iron_window_options_t): void;
declare function iron_set_app_name(name: string): void;
declare function iron_log(v: any): void;
declare function _iron_set_update_callback(callback: ()=>void): void;
declare function _iron_set_drop_files_callback(callback: (file: string)=>void): void;
declare function iron_set_cut_copy_paste_callback(on_cut: ()=>string, on_copy: ()=>string, on_paste: (text: string)=>void): void;
declare function iron_set_application_state_callback(on_foreground: ()=>void, on_resume: ()=>void, on_pause: ()=>void, on_background: ()=>void, on_shutdown: ()=>void): void;
declare function iron_set_keyboard_down_callback(callback: (code: i32)=>void): void;
declare function iron_set_keyboard_up_callback(callback: (code: i32)=>void): void;
declare function iron_set_keyboard_press_callback(callback: (char_code: i32)=>void): void;
declare function iron_set_mouse_down_callback(callback: (button: i32, x: i32, y: i32)=>void): void;
declare function iron_set_mouse_up_callback(callback: (button: i32, x: i32, y: i32)=>void): void;
declare function iron_set_mouse_move_callback(callback: (x: i32, y: i32, mx: i32, my: i32)=>void): void;
declare function iron_set_mouse_wheel_callback(callback: (button: i32)=>void): void;
declare function iron_set_touch_down_callback(callback: (index: i32, x: i32, y: i32)=>void): void;
declare function iron_set_touch_up_callback(callback: (index: i32, x: i32, y: i32)=>void): void;
declare function iron_set_touch_move_callback(callback: (index: i32, x: i32, y: i32)=>void): void;
declare function iron_set_pen_down_callback(callback: (x: i32, y: i32, pressure: f32)=>void): void;
declare function iron_set_pen_up_callback(callback: (x: i32, y: i32, pressure: f32)=>void): void;
declare function iron_set_pen_move_callback(callback: (x: i32, y: i32, pressure: f32)=>void): void;
declare function iron_set_gamepad_axis_callback(callback: (gamepad: i32, axis: i32, value: f32)=>void): void;
declare function iron_set_gamepad_button_callback(callback: (gamepad: i32, button: i32, value: f32)=>void): void;
declare function iron_mouse_lock(): void;
declare function iron_mouse_unlock(): void;
declare function iron_mouse_can_lock(): bool;
declare function iron_mouse_is_locked(): bool;
declare function iron_mouse_set_position(x: i32, y: i32): void;
declare function iron_show_mouse(show: bool): void;
declare function iron_show_keyboard(show: bool): void;

declare function gpu_create_index_buffer(count: i32): any;
declare function gpu_delete_index_buffer(buffer: any): void;
declare function gpu_lock_index_buffer(buffer: any): u32_array_t;
declare function gpu_index_buffer_unlock(buffer: any): void;
declare function gpu_set_index_buffer(buffer: any): void;
declare function gpu_create_vertex_buffer(count: i32, structure: iron_gpu_vertex_structure_t, usage: i32): any;
declare function gpu_delete_vertex_buffer(buffer: any): void;
declare function gpu_lock_vertex_buffer(buffer: any): buffer_t;
declare function iron_gpu_vertex_buffer_unlock(buffer: any): void;
declare function gpu_set_vertex_buffer(buffer: any): void;
declare function gpu_draw(): void;
declare function gpu_create_shader(data: buffer_t, type: i32): iron_gpu_shader_t;
declare function gpu_create_shader_from_source(source: string, source_size: i32, shader_type: shader_type_t): iron_gpu_shader_t;
declare function iron_gpu_shader_destroy(shader: iron_gpu_shader_t): void;
declare function gpu_create_pipeline(): any;
declare function gpu_delete_pipeline(pipeline: any): void;
declare function gpu_compile_pipeline(pipeline: any): void;
declare function gpu_set_pipeline(pipeline: any): void;
declare function iron_load_image(file: string, readable: bool): any;
declare function iron_unload_image(image: iron_gpu_texture_t): void;
declare function iron_load_sound(file: string): any;
declare function iron_a1_sound_destroy(sound: any): void;
declare function iron_a1_play_sound(sound: any, loop: bool, pitch: f32, unique: bool): audio_channel_t;
declare function iron_a1_stop_sound(sound: any): void;
declare function iron_a1_channel_set_pitch(channel: audio_channel_t, pitch: f32): void;
declare function iron_load_blob(file: string): buffer_t;
declare function iron_load_url(url: string): void;
declare function iron_copy_to_clipboard(text: string): void;

declare function gpu_get_constant_location(pipeline: any, name: string): any;
declare function gpu_get_texture_unit(pipeline: any, name: string): any;
declare function gpu_set_texture(stage: any, texture: any): void;
declare function gpu_set_texture_depth(unit: any, texture: any): void;
declare function gpu_set_bool(location: any, value: bool): void;
declare function gpu_set_int(location: any, value: i32): void;
declare function gpu_set_float(location: any, value: f32): void;
declare function gpu_set_float2(location: any, value1: f32, value2: f32): void;
declare function gpu_set_float3(location: any, value1: f32, value2: f32, value3: f32): void;
declare function gpu_set_float4(location: any, value1: f32, value2: f32, value3: f32, value4: f32): void;
declare function gpu_set_floats(location: any, values: f32_array_t): void;
declare function gpu_set_matrix4(location: any, matrix: mat4_t): void;
declare function gpu_set_matrix3(location: any, matrix: mat3_t): void;

declare function iron_time(): f32;
declare function iron_window_width(): i32;
declare function iron_window_height(): i32;
declare function iron_set_window_title(title: string): void;
declare function iron_window_get_mode(): i32;
declare function iron_set_window_mode(mode: i32): void;
declare function iron_window_resize(width: i32, height: i32): void;
declare function iron_window_move(x: i32, y: i32): void;
declare function iron_screen_dpi(): i32;
declare function iron_system_id(): string;
declare function iron_stop(): void;
declare function iron_count_displays(): i32;
declare function iron_display_width(index: i32): i32;
declare function iron_display_height(index: i32): i32;
declare function iron_display_x(index: i32): i32;
declare function iron_display_y(index: i32): i32;
declare function iron_display_frequency(index: i32): i32;
declare function iron_display_is_primary(index: i32): bool;

declare function gpu_create_render_target(width: i32, height: i32, format: i32 = tex_format_t.RGBA32, depth_buffer_bits: i32 = 0): any;
declare function gpu_create_texture_from_bytes(data: buffer_t, width: i32, height: i32, format: i32 = tex_format_t.RGBA32, readable: bool = true): any;
declare function gpu_create_texture_from_encoded_bytes(data: buffer_t, format: string, readable: bool = false): any;
declare function gpu_get_texture_pixels(texture: any): buffer_t;
declare function iron_gpu_texture_generate_mipmaps(texture: any, levels: i32): void;
declare function gpu_set_mipmaps(texture: any, mipmaps: iron_gpu_texture_t[]): void;
declare function iron_gpu_render_target_set_depth_from(target: any, source: any): void;
declare function gpu_viewport(x: i32, y: i32, width: i32, height: i32): void;
declare function gpu_scissor(x: i32, y: i32, width: i32, height: i32): void;
declare function gpu_disable_scissor(): void;
declare function _gpu_begin(render_target: iron_gpu_texture_t, additional: iron_gpu_texture_t[] = null, flags: i32 = clear_flag_t.NONE, color: i32 = 0, depth: f32 = 0.0): void;
declare function _gpu_end(): void;
declare function gpu_swap_buffers(): void;
declare function iron_file_save_bytes(path: string, bytes: buffer_t, length?: i32): void;
declare function iron_sys_command(cmd: string): i32;
declare function iron_internal_save_path(): string;
declare function iron_get_arg_count(): i32;
declare function iron_get_arg(index: i32): string;
declare function iron_get_files_location(): string;
declare function _iron_http_request(url: string, size: i32, callback: (url: string, _: buffer_t)=>void): void;

declare function draw_init(image_vert: buffer_t, image_frag: buffer_t, rect_vert: buffer_t, rect_frag: buffer_t, tris_vert: buffer_t, tris_frag: buffer_t, text_vert: buffer_t, text_frag: buffer_t): void;
declare function draw_begin(render_target: iron_gpu_texture_t = null, clear: bool = false, color: u32 = 0): void;
declare function draw_end(): void;
declare function draw_scaled_sub_image(image: iron_gpu_texture_t, sx: f32, sy: f32, sw: f32, sh: f32, dx: f32, dy: f32, dw: f32, dh: f32): void;
declare function draw_sub_image(image: iron_gpu_texture_t, x: f32, y: f32, sx: f32, sy: f32, sw: f32, sh: f32): void;
declare function draw_scaled_image(image: iron_gpu_texture_t, dx: f32, dy: f32, dw: f32, dh: f32): void;
declare function draw_image(image: iron_gpu_texture_t, x: f32, y: f32): void;
declare function draw_filled_triangle(x0: f32, y0: f32, x1: f32, y1: f32, x2: f32, y2: f32): void;
declare function draw_filled_rect(x: f32, y: f32, width: f32, height: f32): void;
declare function draw_rect(x: f32, y: f32, width: f32, height: f32, strength: f32 = 1.0): void;
declare function draw_line(x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0): void;
declare function draw_line_aa(x0: f32, y0: f32, x1: f32, y1: f32, strength: f32 = 1.0): void;
declare function draw_string(text: string, x: f32, y: f32): void;
declare function draw_set_font(font: draw_font_t, size: i32): bool;
declare function draw_font_init(font: draw_font_t): void;
declare function draw_font_destroy(font: draw_font_t): void;
declare function draw_font_13(font: draw_font_t): any;
declare function draw_font_has_glyph(glyph: i32): bool;
declare function draw_font_add_glyph(glyph: i32): void;
declare function draw_font_init_glyphs(from: i32, to: i32): void;
declare function draw_font_count(font: draw_font_t): i32;
declare function draw_font_height(font: draw_font_t, size: i32): i32;
declare function draw_string_width(font: draw_font_t, size: i32, text: string): i32;
declare function draw_set_bilinear_filter(bilinear: bool): void;
declare function draw_set_color(color: i32): void;
declare function draw_set_pipeline(pipeline: any): void;
declare function draw_set_transform(matrix: mat3_t): void;
declare function draw_filled_circle(cx: f32, cy: f32, radius: f32, segments: i32 = 0): void;
declare function draw_circle(cx: f32, cy: f32, radius: f32, segments: i32 = 0, strength: f32 = 1.0): void;
declare function draw_cubic_bezier(x: f32[], y: f32[], segments: i32 = 20, strength: f32 = 1.0): void;
declare let draw_font: draw_font_t;
declare let draw_font_size: i32;

declare type draw_font_t = {
	blob?: any; // unsigned char *
	images?: any; // draw_font_image_t *
	m_capacity?: i32;
	m_images_len?: i32;
	offset?: 32;
	buf?: buffer_t;
	index?: i32;
};

declare function iron_set_save_and_quit_callback(callback: (save: bool)=>void): void;
declare function iron_set_mouse_cursor(id: i32): void;
declare function iron_delay_idle_sleep(): void;
declare function iron_open_dialog(filter_list: string, default_path: string, open_multiple: bool): string[];
declare function iron_save_dialog(filter_list: string, default_path: string): string;
declare function iron_read_directory(path: string): string;
declare function iron_file_exists(path: string): bool;
declare function iron_delete_file(path: string): void;
declare function iron_inflate(bytes: buffer_t, raw: bool): buffer_t;
declare function iron_deflate(bytes: buffer_t, raw: bool): buffer_t;
declare function iron_write_jpg(path: string, bytes: buffer_t, w: i32, h: i32, format: i32, quality: i32): void; // RGBA, R, RGB1, RRR1, GGG1, BBB1, AAA1
declare function iron_write_png(path: string, bytes: buffer_t, w: i32, h: i32, format: i32): void;
declare function iron_encode_jpg(bytes: buffer_t, w: i32, h: i32, format: i32, quality: i32): buffer_t;
declare function iron_encode_png(bytes: buffer_t, w: i32, h: i32, format: i32): buffer_t;
declare function iron_mp4_begin(path: string, w: i32, h: i32): void;
declare function iron_mp4_end(): void;
declare function iron_mp4_encode(pixels: buffer_t): void;
declare function iron_ml_inference(model: buffer_t, tensors: buffer_t[], input_shape?: i32[][], output_shape?: i32[], use_gpu?: bool): buffer_t;
declare function iron_ml_unload(): void;

declare function iron_gpu_raytrace_supported(): bool;
declare function iron_raytrace_init(shader: buffer_t): void;
declare function iron_raytrace_as_init(): void;
declare function iron_raytrace_as_add(vb: any, ib: any, transform: mat4_t): void;
declare function iron_raytrace_as_build(vb_full: any, ib_full: any): void;

declare function iron_raytrace_set_textures(tex0: iron_gpu_texture_t, tex1: iron_gpu_texture_t, tex2: iron_gpu_texture_t, texenv: any, tex_sobol: any, tex_scramble: any, tex_rank: any): void;
declare function iron_raytrace_dispatch_rays(target: any, cb: buffer_t): void;

declare function iron_window_x(): i32;
declare function iron_window_y(): i32;
declare function iron_language(): string;
declare function obj_parse(file_bytes: buffer_t, split_code: i32, start_pos: i32, udim: bool): any;

declare function armpack_decode(b: buffer_t): any;
declare function armpack_encode_start(encoded: any): void;
declare function armpack_encode_end(): i32;
declare function armpack_encode_map(count: u32): void;
declare function armpack_encode_array(count: u32): void;
declare function armpack_encode_array_f32(f32a: f32_array_t): void;
declare function armpack_encode_array_i32(i32a: i32_array_t): void;
declare function armpack_encode_array_i16(i16a: i16_array_t): void;
declare function armpack_encode_array_u8(u8a: u8_array_t): void;
declare function armpack_encode_array_string(strings: string[]): void;
declare function armpack_encode_string(str: string): void;
declare function armpack_encode_i32(i: i32): void;
declare function armpack_encode_f32(f: f32): void;
declare function armpack_encode_bool(b: bool): void;
declare function armpack_encode_null(): void;
declare function armpack_size_map(): i32;
declare function armpack_size_array(): i32;
declare function armpack_size_array_f32(f32a: f32_array_t): i32;
declare function armpack_size_array_u8(u8a: u8_array_t): i32;
declare function armpack_size_string(str: string): i32;
declare function armpack_size_i32(): i32;
declare function armpack_size_f32(): i32;
declare function armpack_size_bool(): i32;
declare function armpack_decode_to_map(b: buffer_t): map_t<string, any>;
declare function armpack_map_get_f32(map: map_t<string, any>, key: string): f32;
declare function armpack_map_get_i32(map: map_t<string, any>, key: string): i32;

declare type audio_channel_t = {
	sound: any; // iron_a1_sound_t
	position: f32;
	loop: bool;
	volume: f32;
	pitch: f32;
};

declare type quat_t = {
	x: f32;
	y: f32;
	z: f32;
	w: f32;
};

declare function quat_create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): quat_t;
declare function quat_from_axis_angle(axis: vec4_t, angle: f32): quat_t;
declare function quat_from_mat(m: mat4_t): quat_t;
declare function quat_from_rot_mat(m: mat4_t): quat_t;
declare function quat_mult(a: quat_t, b: quat_t): quat_t;
declare function quat_norm(q: quat_t): quat_t;
declare function quat_clone(q: quat_t): quat_t;
declare function quat_get_euler(q: quat_t): vec4_t;
declare function quat_from_euler(x: f32, y: f32, z: f32): quat_t;
declare function quat_lerp(from: quat_t, to: quat_t, s: f32): quat_t;
declare function quat_dot(a: quat_t, b: quat_t): f32;
declare function quat_from_to(v0: vec4_t, v1: vec4_t): quat_t;
declare function quat_inv(q: quat_t): quat_t;

declare type mat3_t = {
	m?: f32_ptr;
	m00: f32;
	m01: f32;
	m02: f32;
	m10: f32;
	m11: f32;
	m12: f32;
	m20: f32;
	m21: f32;
	m22: f32;
};

declare function mat3_create(_00: f32, _10: f32, _20: f32,
							 _01: f32, _11: f32, _21: f32,
							 _02: f32, _12: f32, _22: f32): mat3_t;
declare function mat3_identity(): mat3_t;
declare function mat3_translation(x: f32, y: f32): mat3_t;
declare function mat3_rotation(alpha: f32): mat3_t;
declare function mat3_set_from4(m4: mat4_t): mat3_t;
declare function mat3_multmat(a: mat3_t, b: mat3_t): mat3_t;
declare function mat3_nan(): mat3_t;
declare function mat3_isnan(m: mat3_t): bool;

declare type mat4_t = {
	m?: f32_ptr;
	m00: f32;
	m01: f32;
	m02: f32;
	m03: f32;
	m10: f32;
	m11: f32;
	m12: f32;
	m13: f32;
	m20: f32;
	m21: f32;
	m22: f32;
	m23: f32;
	m30: f32;
	m31: f32;
	m32: f32;
	m33: f32;
};

declare type mat4_decomposed_t = {
	loc: vec4_t;
	rot: quat_t;
	scl: vec4_t;
}

declare function mat4_create(_00: f32, _10: f32, _20: f32, _30: f32,
							 _01: f32, _11: f32, _21: f32, _31: f32,
							 _02: f32, _12: f32, _22: f32, _32: f32,
							 _03: f32, _13: f32, _23: f32, _33: f32): mat4_t;
declare function mat4_identity(): mat4_t;
declare function mat4_from_f32_array(a: f32_array_t, offset: i32 = 0): mat4_t;
declare function mat4_persp(fov_y: f32, aspect: f32, zn: f32, zf: f32): mat4_t;
declare function mat4_ortho(left: f32, right: f32, bottom: f32, top: f32, znear: f32, zfar: f32): mat4_t;
declare function mat4_rot_z(alpha: f32): mat4_t;
declare function mat4_compose(loc: vec4_t, rot: quat_t, scl: vec4_t): mat4_t;
declare function mat4_decompose(m: mat4_t): mat4_decomposed_t;
declare function mat4_set_loc(m: mat4_t, v: vec4_t): mat4_t;
declare function mat4_from_quat(q: quat_t): mat4_t;
declare function mat4_init_translate(x: f32, y: f32, z: f32): mat4_t;
declare function mat4_translate(m: mat4_t, x: f32, y: f32, z: f32): mat4_t;
declare function mat4_scale(m: mat4_t, v: vec4_t): mat4_t;
declare function mat4_mult_mat3x4(a: mat4_t, b: mat4_t): mat4_t;
declare function mat4_mult_mat(a: mat4_t, b: mat4_t): mat4_t;
declare function mat4_inv(a: mat4_t): mat4_t;
declare function mat4_transpose(m: mat4_t): mat4_t;
declare function mat4_transpose3x3(m: mat4_t): mat4_t;
declare function mat4_clone(m: mat4_t): mat4_t;
declare function mat4_get_loc(m: mat4_t): vec4_t;
declare function mat4_get_scale(m: mat4_t): vec4_t;
declare function mat4_mult(m: mat4_t, s: f32): mat4_t;
declare function mat4_to_rot(m: mat4_t): mat4_t;
declare function mat4_right(m: mat4_t): vec4_t;
declare function mat4_look(m: mat4_t): vec4_t;
declare function mat4_up(m: mat4_t): vec4_t;
declare function mat4_to_f32_array(m: mat4_t): f32_array_t;
declare function mat4_cofactor(m0: f32, m1: f32, m2: f32, m3: f32, m4: f32, m5: f32, m6: f32, m7: f32, m8: f32): f32;
declare function mat4_determinant(m: mat4_t): f32;
declare function mat4_nan(): mat4_t;
declare function mat4_isnan(m: mat4_t): bool;
declare let mat4nan: mat4_t;

type mat4_box_t = {
	v: mat4_t;
};

declare type vec2_t = {
	x: f32;
	y: f32;
};

declare function vec2_create(x: f32 = 0.0, y: f32 = 0.0): vec2_t;
declare function vec2_len(v: vec2_t): f32;
declare function vec2_set_len(v: vec2_t, length: f32): vec2_t;
declare function vec2_mult(v: vec2_t, f: f32): vec2_t;
declare function vec2_add(a: vec2_t, b: vec2_t): vec2_t;
declare function vec2_sub(a: vec2_t, b: vec2_t): vec2_t;
declare function vec2_cross(a: vec2_t, b: vec2_t): f32;
declare function vec2_norm(v: vec2_t): vec2_t;
declare function vec2_dot(a: vec2_t, b: vec2_t): f32;
declare function vec2_nan(): vec2_t;
declare function vec2_isnan(a: vec2_t): bool;

declare type vec4_t = {
	x: f32;
	y: f32;
	z: f32;
	w: f32;
};

declare function vec4_create(x: f32 = 0.0, y: f32 = 0.0, z: f32 = 0.0, w: f32 = 1.0): vec4_t;
declare function vec4_cross(a: vec4_t, b: vec4_t): vec4_t;
declare function vec4_add(a: vec4_t, b: vec4_t): vec4_t;
declare function vec4_fadd(a: vec4_t, x: f32, y: f32, z: f32, w: f32 = 0.0): vec4_t;
declare function vec4_norm(a: vec4_t): vec4_t;
declare function vec4_mult(v: vec4_t, f: f32): vec4_t;
declare function vec4_dot(a: vec4_t, b: vec4_t): f32;
declare function vec4_clone(v: vec4_t): vec4_t;
declare function vec4_lerp(from: vec4_t, to: vec4_t, s: f32): vec4_t;
declare function vec4_apply_proj(a: vec4_t, m: mat4_t): vec4_t;
declare function vec4_apply_mat(a: vec4_t, m: mat4_t): vec4_t;
declare function vec4_apply_mat4(a: vec4_t, m: mat4_t): vec4_t;
declare function vec4_apply_axis_angle(a: vec4_t, axis: vec4_t, angle: f32): vec4_t;
declare function vec4_apply_quat(a: vec4_t, q: quat_t): vec4_t;
declare function vec4_equals(a: vec4_t, b: vec4_t): bool;
declare function vec4_almost_equals(a: vec4_t, b: vec4_t, prec: f32): bool;
declare function vec4_len(a: vec4_t): f32;
declare function vec4_sub(a: vec4_t, b: vec4_t): vec4_t;
declare function vec4_exp(a: vec4_t): vec4_t;
declare function vec4_dist(v1: vec4_t, v2: vec4_t): f32;
declare function vec4_fdist(v1x: f32, v1y: f32, v1z: f32, v2x: f32, v2y: f32, v2z: f32): f32;
declare function vec4_reflect(a: vec4_t, n: vec4_t): vec4_t;
declare function vec4_clamp(a: vec4_t, min: f32, max: f32): vec4_t;
declare function vec4_x_axis(): vec4_t;
declare function vec4_y_axis(): vec4_t;
declare function vec4_z_axis(): vec4_t;
declare function vec4_nan(): vec4_t;
declare function vec4_isnan(a: vec4_t): bool;

type vec4_box_t = {
	v: vec4_t;
};

declare function f32_nan(): f32;
declare function f32_isnan(f: f32): bool;


function gpu_vertex_struct_create(): iron_gpu_vertex_structure_t {
	let raw: iron_gpu_vertex_structure_t = {};
	return raw;
}

function gpu_vertex_struct_add(raw: iron_gpu_vertex_structure_t, name: string, data: vertex_data_t) {
	let e: iron_gpu_vertex_element_t = ADDRESS(ARRAY_ACCESS(raw.elements, raw.size));
	e.name = name;
	e.data = data;
	raw.size++;
}

declare function iron_gpu_vertex_struct_size(s: iron_gpu_vertex_structure_t): i32;
declare function iron_gpu_vertex_data_size(data: vertex_data_t);

declare type iron_gpu_pipeline_t = {
	input_layout?: any;
	vertex_shader?: any;
	fragment_shader?: any;

	cull_mode?: cull_mode_t;
	depth_write?: bool;
	depth_mode?: compare_mode_t;

	blend_source?: blend_factor_t;
	blend_destination?: blend_factor_t;
	alpha_blend_source?: blend_factor_t;
	alpha_blend_destination?: blend_factor_t;

	color_write_mask_red?: any;
	color_write_mask_green?: any;
	color_write_mask_blue?: any;
	color_write_mask_alpha?: any;

	color_attachment?: any;
	color_attachment_count?: i32;
	depth_attachment_bits?: i32;

	impl?: any;
};

declare type iron_gpu_shader_t = {
	impl?: any;
};

declare type iron_gpu_vertex_element_t = {
	name?: string;
	data?: vertex_data_t;
};

declare type iron_gpu_vertex_structure_t = {
	elements?: any; // iron_gpu_vertex_element_t[IRON_GPU_MAX_VERTEX_ELEMENTS];
	size?: i32;
};

declare type iron_gpu_buffer_t = {
	impl?: any;
};

declare type iron_gpu_constant_location_t = any;
declare type iron_gpu_texture_unit_t = any;

enum clear_flag_t {
	NONE = 0,
	COLOR = 1,
	DEPTH = 2,
}

enum usage_t {
	STATIC,
	DYNAMIC,
	READABLE,
}

enum tex_format_t {
	RGBA32,
	RGBA64,
	RGBA128,
	R8,
	R16,
	R32,
}

enum vertex_data_t {
	F32_1X,
	F32_2X,
	F32_3X,
	F32_4X,
	I16_2X_NORM,
	I16_4X_NORM,
}

enum blend_factor_t {
	BLEND_ONE,
	BLEND_ZERO,
	SOURCE_ALPHA,
	DEST_ALPHA,
	INV_SOURCE_ALPHA,
	INV_DEST_ALPHA,
}

enum compare_mode_t {
	ALWAYS,
	NEVER,
	LESS,
}

enum cull_mode_t {
	CLOCKWISE,
	COUNTER_CLOCKWISE,
	NONE,
}

enum shader_type_t {
	VERTEX,
	FRAGMENT,
}

declare let ui_nodes_enum_texts: (s: string)=>string[];
declare let ui_touch_scroll: bool;
declare let ui_touch_hold : bool;
declare let ui_touch_tooltip: bool;
declare let ui_always_redraw_window: bool;
declare let ui_on_border_hover: any;
declare let ui_on_text_hover: any;
declare let ui_on_deselect_text: any;
declare let ui_on_tab_drop: any;
declare let ui_nodes_on_link_drag: any;
declare let ui_nodes_on_socket_released: any;
declare let ui_nodes_on_canvas_released: any;
declare let ui_nodes_on_canvas_control: any;
declare let ui_text_area_line_numbers: bool;
declare let ui_text_area_scroll_past_end: bool;
declare let ui_text_area_coloring: any;
declare let ui_nodes_socket_released: bool;
declare let ui_is_cut: bool;
declare let ui_is_copy: bool;
declare let ui_is_paste: bool;
declare let ui_nodes_exclude_remove: string[];
declare let ui_clipboard: string;
declare let ui_nodes_grid_snap: bool;

declare function ui_nest(handle: ui_handle_t, pos: i32): ui_handle_t;
declare function ui_theme_default(theme: ui_theme_t): void;
declare function ui_tab(handle: ui_handle_t, text: string, vertical: bool = false, color: i32 = -1): bool;
declare function ui_combo(handle: ui_handle_t, texts: string[], label: string = "", show_label: bool = false, align: ui_align_t = ui_align_t.LEFT, search_bar: bool = true): i32;
declare function ui_slider(handle: ui_handle_t, text: string, from: f32 = 0.0, to: f32 = 1.0, filled: bool = false, precision: f32 = 100.0, display_value: bool = true, align: ui_align_t = ui_align_t.RIGHT, text_edit: bool = true): f32;
declare function ui_button(text: string, align: ui_align_t = ui_align_t.CENTER, label: string = ""): bool;
declare function ui_text(text: string, align: ui_align_t = ui_align_t.LEFT, bg: i32 = 0x00000000): ui_state_t;
declare function ui_text_input(handle: ui_handle_t, label: string = "", align: ui_align_t = ui_align_t.LEFT, editable: bool = true, live_update: bool = false): string;
declare function ui_check(handle: ui_handle_t, text: string, label: string = ""): bool;
declare function ui_color_wheel(handle: ui_handle_t, alpha: bool = false, w: f32 = -1.0, h: f32 = -1.0, color_preview: bool = true, picker: ()=>void = null, data: any = null): color_t;
declare function ui_hovered_tab_name(): string;
declare function ui_radio(handle: ui_handle_t, position: i32, text: string, label: string = ""): bool;
declare function ui_start_text_edit(handle: ui_handle_t, align: ui_align_t = ui_align_t.LEFT): void;
declare function ui_tooltip(s: string): void;
declare function ui_tooltip_image(tex: any, max_width: i32 = 0): void;
declare function ui_separator(h: i32 = 4, fill: bool = true): void;
declare function ui_text_area(handle: ui_handle_t, align: ui_align_t = ui_align_t.LEFT, editable: bool = true, label: string = "", word_wrap: bool = false): string;
declare function _ui_window(handle: ui_handle_t, x: i32, y: i32, w: i32, h: i32, drag: bool = false): bool;
declare function ui_begin(ui: ui_t): void;
declare function _ui_end(last: bool = true): void;
declare function ui_end_window(bind_global_g: bool = true): void;
declare function ui_end_region(last: bool = true): void;
declare function ui_float_input(handle: ui_handle_t, label: string = "", align: ui_align_t = ui_align_t.LEFT, precision: f32 = 1000.0): f32;
declare function ui_get_current(): ui_t;
declare function ui_remove_node(n: ui_node_t, canvas: ui_node_canvas_t): void;
declare function ui_next_link_id(links: ui_node_link_t[]): i32;
declare function ui_set_hovered_tab_name(s: string): void;
declare function ui_begin_sticky(): void;
declare function ui_end_sticky(): void;
declare function ui_row(r: f32[]): void;
declare function ui_row2(): void;
declare function ui_row3(): void;
declare function ui_row4(): void;
declare function ui_row5(): void;
declare function ui_row6(): void;
declare function ui_row7(): void;
declare function ui_handle_create(): ui_handle_t;
declare function ui_fill(x: i32, y: i32, w: i32, h: i32, color: i32): void;
declare function ui_rect(x: i32, y: i32, w: i32, h: i32, color: i32, strength: f32): void;
declare function ui_draw_shadow(x: i32, y: i32, w: i32, h: i32): void;
declare function ui_draw_rect(filled: bool, x: i32, y: i32, w: i32, h: i32): void;
declare function ui_draw_round_bottom(x: i32, y: i32, w: i32): void;
declare function ui_begin_menu(): void;
declare function ui_end_menu(): void;
declare function _ui_menu_button(s: string): bool;
declare function ui_begin_region(ui: ui_t, x: i32, y: i32, w: i32): void;
declare function ui_end_region(last: bool): void;
declare function ui_inline_radio(handle: ui_handle_t, texts: string[], align: i32): int;
declare function ui_end_input(): void;
declare function ui_panel(handle: ui_handle_t, text: string, is_tree: bool, filled: bool): bool;
declare function ui_nodes_rgba_popup(nhandle: ui_handle_t, val: f32_ptr, x: i32, y: i32): void;
declare function ui_get_link(links: ui_node_link_t[], id: i32): ui_node_link_t;
declare function ui_get_node(nodes: ui_node_t[], id: i32): ui_node_t;
declare function ui_input_in_rect(x: f32, y: f32, w: f32, h: f32): bool;
declare function ui_get_socket_id(nodes: ui_node_t[]): i32;
declare function ui_draw_string(text: string, x_offset: f32, y_offset: f32, align: i32, truncation: bool): void;
declare function ui_next_node_id(nodes: ui_node_t[]): i32;
declare function ui_node_canvas(nodes: ui_nodes_t, canvas: ui_node_canvas_t): void;

declare type iron_gpu_texture_t = {
	width: i32;
	height: i32;
};
declare type ui_theme_t = any;

declare type ui_t = {
	ops: ui_options_t;
	is_hovered: bool;
	is_typing: bool;
	is_escape_down: bool;
	is_delete_down: bool;
	is_return_down: bool;
	is_ctrl_down: bool;
	is_released: bool;
	is_key_pressed: bool;
	is_scrolling: bool;
	key_code: i32;
	input_started: bool;
	input_started_r: bool;
	input_released: bool;
	input_released_r: bool;
	input_x: i32;
	input_y: i32;
	input_started_x: i32;
	input_started_y: i32;
	input_enabled: bool;
	input_down_r: bool;
	input_dx: i32;
	input_dy: i32;
	input_wheel_delta: i32;
	_x: i32;
	_y: i32;
	_w: i32;
	_window_w: i32;
	_window_h: i32;
	_window_x: i32;
	_window_y: i32;
	scroll_enabled: bool;
	input_down: bool;
	font_size: i32;
	image_scroll_align: bool;
	changed: bool;
	font_offset_y: f32;
	enabled: bool;
	scissor: bool;
	text_selected_handle: ui_handle_t;
	submit_text_handle: ui_handle_t;
	combo_selected_handle: ui_handle_t;
	current_ratio: i32;
	image_invert_y: bool;
	elements_baked: bool;
	window_border_right: i32;
	window_border_top: i32;
	window_border_bottom: i32;
};

declare type ui_handle_t = {
	selected: bool;
	position: i32;
	color: u32;
	value: f32;
	text: string;
	// iron_gpu_texture_t texture;
	redraws: i32;
	scroll_offset: f32;
	scroll_enabled: bool;
	layout: i32;
	last_max_x: f32;
	last_max_y: f32;
	drag_enabled: bool;
	drag_x: i32;
	drag_y: i32;
	changed: bool;
	init: bool;
	children: ui_handle_t[];
};

declare type ui_options_t = {
	font?: draw_font_t;
	theme?: ui_theme_t;
	scale_factor?: f32;
	color_wheel?: iron_gpu_texture_t;
	black_white_gradient?: iron_gpu_texture_t;
};

declare type ui_coloring_t = {
	color?: u32;
	start?: string[];
	end?: string;
	separated?: bool;
};

declare type ui_text_coloring_t = {
	colorings?: ui_coloring_t[];
	default_color?: u32;
};

declare type ui_canvas_control_t = {
	pan_x?: f32;
	pan_y?: f32;
	zoom?: f32;
	controls_down?: bool;
};

declare type ui_node_canvas_t = {
	name?: string;
	nodes?: ui_node_t[];
	links?: ui_node_link_t[];
};

declare type ui_node_t = {
	id?: i32;
	name?: string;
	type?: string;
	x?: f32;
	y?: f32;
	color?: u32;
	inputs?: ui_node_socket_t[];
	outputs?: ui_node_socket_t[];
	buttons?: ui_node_button_t[];
	width?: f32;
};

declare type ui_node_socket_t = {
	id?: i32;
	node_id?: i32;
	name?: string;
	type?: string;
	color?: u32;
	default_value?: f32_array_t;
	min?: f32;
	max?: f32;
	precision?: f32;
	display?: i32;
};

declare type ui_node_link_t = {
	id?: i32;
	from_id?: i32;
	from_socket?: i32;
	to_id?: i32;
	to_socket?: i32;
};

declare type ui_node_button_t = {
	name?: string;
	type?: string;
	output?: i32;
	default_value?: f32_array_t;
	data?: u8_array_t;
	min?: f32;
	max?: f32;
	precision?: f32;
	height?: f32;
};

declare type ui_nodes_t = {
	color_picker_callback?: (col: color_t)=>void;
	nodes_selected_id?: i32[];
	_input_started?: bool;
	nodes_drag?: bool;
	pan_x?: f32;
	pan_y?: f32;
	zoom?: f32;
	link_drag_id?: i32;
};

enum ui_layout_t {
	VERTICAL,
	HORIZONTAL,
}

enum ui_align_t {
	LEFT,
	CENTER,
	RIGHT,
}

enum ui_state_t {
	IDLE,
	STARTED,
	DOWN,
	RELEASED,
	HOVERED,
}

let ui_children: map_t<string, ui_handle_t> = map_create();
let ui_nodes_custom_buttons: map_t<string, (i: i32)=>void> = map_create();

declare function UI_OUTPUTS_H(sockets_count: i32, length: i32 = -1): f32;
declare function ui_tooltip_image(image: iron_gpu_texture_t, max_width: i32 = 0);

function ui_SCALE(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_SCALE();
	ui_set_current(current);
	return f;
}

function ui_ELEMENT_OFFSET(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_ELEMENT_OFFSET();
	ui_set_current(current);
	return f;
}

function ui_ELEMENT_W(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_ELEMENT_W();
	ui_set_current(current);
	return f;
}

function ui_ELEMENT_H(ui: ui_t): f32 {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	let f: f32 = UI_ELEMENT_H();
	ui_set_current(current);
	return f;
}

function ui_MENUBAR_H(ui: ui_t): f32 {
	let button_offset_y: f32 = (ui.ops.theme.ELEMENT_H * ui_SCALE(ui) - ui.ops.theme.BUTTON_H * ui_SCALE(ui)) / 2;
	return ui.ops.theme.BUTTON_H * ui_SCALE(ui) * 1.1 + 2 + button_offset_y;
}

function ui_nodes_INPUT_Y(canvas: ui_node_canvas_t, sockets: ui_node_socket_t[], pos: i32): f32 {
	return UI_INPUT_Y(canvas, sockets.buffer, sockets.length, pos);
}

function _ui_image(image: iron_gpu_texture_t, tint: i32 = 0xffffffff, h: f32 = -1.0, sx: i32 = 0, sy: i32 = 0, sw: i32 = 0, sh: i32 = 0): ui_state_t {
	return ui_sub_image(image, tint, h, sx, sy, sw, sh);
}

function ui_window(handle: ui_handle_t, x: i32, y: i32, w: i32, h: i32, drag: bool = false): bool {
	_draw_in_use = true;
	return _ui_window(handle, x, y, w, h, drag);
}

function ui_end(last: bool = true) {
	_draw_in_use = false;
	_ui_end(last);
}

function _ui_set_scale(ui: ui_t, factor: f32) {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	ui_set_scale(factor);
	ui_set_current(current);
}

function _ui_end_element(element_size: f32 = -1.0) {
	if (element_size < 0) {
		ui_end_element();
	}
	else {
		ui_end_element_of_size(element_size);
	}
}

function _ui_end_input(ui: ui_t) {
	let current: ui_t = ui_get_current();
	ui_set_current(ui);
	ui_end_input();
	ui_set_current(current);
}

function ui_handle(s: string): ui_handle_t {
	let h: ui_handle_t = map_get(ui_children, s);
	if (h == null) {
		h = ui_handle_create();
		map_set(ui_children, s, h);
		return h;
	}
	h.init = false;
	return h;
}

function ui_create(ops: ui_options_t): ui_t {
    let raw: ui_t = {};
    ui_init(raw, ops);
    return raw;
}

function ui_theme_create(): ui_theme_t {
	let raw: ui_theme_t = {};
	ui_theme_default(raw);
	return raw;
}

function nodes_on_custom_button(node_id: i32, button_name: string) {
	let f: (i: i32) => void = map_get(ui_nodes_custom_buttons, button_name);
	f(node_id);
}

function ui_nodes_create(): ui_nodes_t {
	let raw: ui_nodes_t = {};
	ui_nodes_init(raw);
	ui_nodes_exclude_remove = [
		"OUTPUT_MATERIAL_PBR",
		"GROUP_OUTPUT",
		"GROUP_INPUT",
		"BrushOutputNode"
	];
	ui_nodes_on_custom_button = nodes_on_custom_button;
	return raw;
}

function ui_get_socket(nodes: ui_node_t[], id: i32): ui_node_socket_t {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: ui_node_t = nodes[i];
		for (let j: i32 = 0; j < n.inputs.length; ++j) {
			let s: ui_node_socket_t = n.inputs[j];
			if (s.id == id) {
				return s;
			}
		}
		for (let j: i32 = 0; j < n.outputs.length; ++j) {
			let s: ui_node_socket_t = n.outputs[j];
			if (s.id == id) {
				return s;
			}
		}
	}
	return null;
}

function ui_set_font(raw: ui_t, font: draw_font_t) {
	draw_font_init(font); // Make sure font is ready
	raw.ops.font = font;
}
