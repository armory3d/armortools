#pragma once

#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

#include "iron_armpack.h"
#include "iron_array.h"
#include "iron_draw.h"
#include "iron_file.h"
#include "iron_gc.h"
#include "iron_gpu.h"
#include "iron_json.h"
#include "iron_map.h"
#include "iron_mat3.h"
#include "iron_mat4.h"
#include "iron_obj.h"
#include "iron_quat.h"
#include "iron_string.h"
#include "iron_system.h"
#include "iron_thread.h"
#include "iron_ui.h"
#include "iron_ui_nodes.h"
#include "iron_vec2.h"
#include "iron_vec3.h"
#include "iron_vec4.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef IRON_WINDOWS
#include <Windows.h>
#endif
#ifdef WITH_AUDIO
#include "iron_audio.h"
#endif
#ifdef WITH_EMBED
#include EMBED_H_PATH
#endif

int    _argc;
char **_argv;

#ifdef WITH_EVAL
#include "quickjs-libc.h"
#include "quickjs.h"
JSRuntime *js_runtime = NULL;
JSContext *js_ctx;
#ifdef WITH_PLUGINS
void plugin_api_init();
#endif

void js_init() {
	js_runtime = JS_NewRuntime();
	js_ctx     = JS_NewContext(js_runtime);
	js_std_add_helpers(js_ctx, _argc, _argv);
	js_init_module_std(js_ctx, "std");
	js_init_module_os(js_ctx, "os");
#ifdef WITH_PLUGINS
	plugin_api_init();
#endif
}

// float alang_eval(char *data);

float js_eval(const char *js) {
	// return alang_eval(js);
	if (js_runtime == NULL) {
		js_init();
	}
	JSValue ret = JS_Eval(js_ctx, js, strlen(js), "iron", JS_EVAL_TYPE_GLOBAL);
	if (JS_IsException(ret)) {
		js_std_dump_error(js_ctx);
		JS_ResetUncatchableError(js_ctx);
	}
	double d;
	JS_ToFloat64(js_ctx, &d, ret);
	JS_RunGC(js_runtime);
	if (d != d) { // nan
		d = 0.0;
	}
	return d;
}

JSValue js_call_arg(void *p, int argc, JSValue *argv) {
	if (js_runtime == NULL) {
		js_init();
	}
	JSValue fn             = *(JSValue *)p;
	JSValue global_obj     = JS_GetGlobalObject(js_ctx);
	JSValue js_call_result = JS_Call(js_ctx, fn, global_obj, argc, argv);
	if (JS_IsException(js_call_result)) {
		js_std_dump_error(js_ctx);
		JS_ResetUncatchableError(js_ctx);
	}
	JS_FreeValue(js_ctx, global_obj);
	return js_call_result;
}

char *js_call_ptr(void *p, void *arg) {
	JSValue argv[] = {JS_NewBigUint64(js_ctx, (uint64_t)arg)};
	return (char *)JS_ToCString(js_ctx, js_call_arg(p, 1, argv));
}

char *js_call_ptr_str(void *p, void *arg0, char *arg1) {
	JSValue argv[] = {JS_NewBigUint64(js_ctx, (uint64_t)arg0), JS_NewString(js_ctx, arg1)};
	return (char *)JS_ToCString(js_ctx, js_call_arg(p, 2, argv));
}

void *js_pcall_str(void *p, char *arg0) {
	JSValue  argv[] = {JS_NewString(js_ctx, arg0)};
	uint64_t result;
	JS_ToBigUint64(js_ctx, &result, js_call_arg(p, 1, argv));
	return (void *)result;
}

char *js_call(void *p) {
	return (char *)JS_ToCString(js_ctx, js_call_arg(p, 0, NULL));
}

#else

void  js_init() {}
float js_eval(const char *js) {
	return 0.0;
}
void  js_call_arg(void *p, int argc, void *argv) {}
char *js_call_ptr(void *p, void *arg) {
	return NULL;
}
char *js_call_ptr_str(void *p, void *arg0, char *arg1) {
	return NULL;
}
void *js_pcall_str(void *p, char *arg0) {
	return NULL;
}
char *js_call(void *p) {
	return NULL;
}
#endif

#ifdef WITH_EMBED
buffer_t *embed_get(char *key) {
#ifdef IRON_WINDOWS
	key = string_replace_all(key, "\\", "/");
#endif
	for (int i = 0; i < embed_count; ++i) {
		if (strcmp(embed_keys[i], key) == 0) {
			buffer_t *buffer = buffer_create(0);
			buffer->buffer   = embed_values[i];
			buffer->length   = embed_sizes[i];
			return buffer;
		}
	}
	return NULL;
}
#endif

#define f64                double
#define i64                int64_t
#define u64                uint64_t
#define f32                float
#define i32                int32_t
#define u32                uint32_t
#define i16                int16_t
#define u16                uint16_t
#define i8                 int8_t
#define u8                 uint8_t
#define string_t           char
#define any                void *
#define any_ptr            void **
#define f64_ptr            f64 *
#define i64_ptr            i64 *
#define u64_ptr            u64 *
#define f32_ptr            f32 *
#define i32_ptr            i32 *
#define u32_ptr            u32 *
#define i16_ptr            i16 *
#define u16_ptr            u16 *
#define i8_ptr             i8 *
#define u8_ptr             u8 *
#define null               NULL
#define DEREFERENCE        *
#define ADDRESS            &
#define ARRAY_ACCESS(a, i) a[i]

f32 f32_nan() {
	return NAN;
}

bool f32_isnan(f32 f) {
	return isnan(f);
}

void _kickstart();
bool enable_window = true;
bool in_background = false;
int  paused_frames = 0;
#ifdef IDLE_SLEEP
bool input_down         = false;
int  last_window_width  = 0;
int  last_window_height = 0;
#endif
char temp_string[1024 * 128];
char temp_string_vs[1024 * 128];
char temp_string_fs[1024 * 128];
#ifdef IRON_WINDOWS
wchar_t        temp_wstring[1024 * 32];
struct HWND__ *iron_windows_window_handle();
#endif

void (*iron_update)(void);
void (*iron_drop_files)(char *);
void (*iron_foreground)(void);
void (*iron_resume)(void);
void (*iron_pause)(void);
void (*iron_background)(void);
void (*iron_shutdown)(void);
void (*iron_pause)(void);
void (*iron_key_down)(int);
void (*iron_key_up)(int);
void (*iron_mouse_down)(int, int, int);
void (*iron_mouse_up)(int, int, int);
void (*iron_mouse_move)(int, int, int, int);
void (*iron_mouse_wheel)(int);
void (*iron_touch_down)(int, int, int);
void (*iron_touch_up)(int, int, int);
void (*iron_touch_move)(int, int, int);
void (*iron_pen_down)(int, int, float);
void (*iron_pen_up)(int, int, float);
void (*iron_pen_move)(int, int, float);
void (*iron_save_and_quit)(bool);

char *_substring(char *s, int32_t start, int32_t end) {
	char *buffer = calloc(1, end - start + 1);
	for (int i = 0; i < end - start; ++i) {
		buffer[i] = s[start + i];
	}
	return buffer;
}

int kickstart(int argc, char **argv) {
	_argc = argc;
	_argv = argv;
#ifdef IRON_ANDROID
	char *bindir = "/";
#elif defined(IRON_IOS)
	char *bindir = "";
#else
	char *bindir = argv[0];
#endif

#ifdef IRON_WINDOWS // Handle non-ascii path
	HMODULE hmodule = GetModuleHandleW(NULL);
	GetModuleFileNameW(hmodule, temp_wstring, 1024);
	WideCharToMultiByte(CP_UTF8, 0, temp_wstring, -1, temp_string, 4096, NULL, NULL);
	bindir = _substring(temp_string, 0, string_last_index_of(temp_string, "\\"));
#else
	bindir = _substring(bindir, 0, string_last_index_of(bindir, "/"));
#endif

	char *assetsdir = argc > 1 ? argv[1] : bindir;

	// Opening a file
	int l = strlen(assetsdir);
	if ((l > 6 && assetsdir[l - 6] == '.') || (l > 5 && assetsdir[l - 5] == '.') || (l > 4 && assetsdir[l - 4] == '.')) {
		assetsdir = bindir;
	}

	for (int i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "--nowindow") == 0) {
			enable_window = false;
		}
	}

#if !defined(IRON_MACOS) && !defined(IRON_IOS)
	iron_internal_set_files_location(assetsdir);
#endif

	iron_threads_init();
	iron_display_init();
	gc_start(&argc);
	_kickstart();

#ifdef WITH_AUDIO
	iron_a2_shutdown();
#endif
	gc_stop();
	return 0;
}

i32 iron_get_arg_count() {
	return _argc;
}

string_t *iron_get_arg(i32 index) {
	return _argv[index];
}

// ██╗██████╗  ██████╗ ███╗   ██╗     █████╗ ██████╗ ██╗
// ██║██╔══██╗██╔═══██╗████╗  ██║    ██╔══██╗██╔══██╗██║
// ██║██████╔╝██║   ██║██╔██╗ ██║    ███████║██████╔╝██║
// ██║██╔══██╗██║   ██║██║╚██╗██║    ██╔══██║██╔═══╝ ██║
// ██║██║  ██║╚██████╔╝██║ ╚████║    ██║  ██║██║     ██║
// ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝    ╚═╝  ╚═╝╚═╝     ╚═╝

#ifndef NO_IRON_API

#include "iron_math.h"
#include "iron_net.h"
#include "kong/dir.h"
#include <lz4x.h>
#include <stdio.h>
#ifdef WITH_AUDIO
#include "iron_audio.h"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#ifdef IRON_DIRECT3D12
#include <d3d12.h>
#endif
#ifdef WITH_D3DCOMPILER
#include <D3Dcompiler.h>
#include <d3d11.h>
#endif
#ifdef WITH_NFD
#include <nfd.h>
#elif defined(IRON_ANDROID)
#include "backends/android_file_dialog.h"
#elif defined(IRON_IOS)
#include "backends/ios_file_dialog.h"
#include <wchar.h>
#endif
#ifdef WITH_IMAGE_WRITE
#ifdef WITH_COMPRESS
unsigned char *iron_deflate_raw(unsigned char *data, int data_len, int *out_len, int quality);
#define STBIW_ZLIB_COMPRESS iron_deflate_raw
#endif
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif
#ifdef WITH_VIDEO_WRITE
#include <minih264e.h>
#include <minimp4.h>
#endif
#ifdef WITH_COMPRESS
#define SDEFL_IMPLEMENTATION
#include "sdefl.h"
#define SINFL_IMPLEMENTATION
#include "sinfl.h"
#endif
#if defined(IDLE_SLEEP) && !defined(IRON_WINDOWS)
#include <time.h>
#endif

void _update(void *data) {
#ifdef IRON_WINDOWS
	if (in_background && ++paused_frames > 3) {
		Sleep(1);
		return;
	}
#endif

#ifdef IDLE_SLEEP
	if (last_window_width != iron_window_width() || last_window_height != iron_window_height()) {
		last_window_width  = iron_window_width();
		last_window_height = iron_window_height();
		paused_frames      = 0;
	}
#if defined(IRON_IOS) || defined(IRON_ANDROID)
	const int start_sleep = 1200;
#else
	const int start_sleep = 120;
#endif
	if (++paused_frames > start_sleep && !input_down) {
#ifdef IRON_WINDOWS
		Sleep(1);
#else
		struct timespec t;
		t.tv_sec  = 0;
		t.tv_nsec = 1000000;
		nanosleep(&t, NULL);
#endif
		return;
	}
	if (paused_frames == 30) {
		gc_run();
	}
#endif

#ifdef WITH_AUDIO
	iron_a2_update();
#endif

	iron_update();
	if (ui_get_current())
		ui_end_frame();
	gpu_present();
}

char *_copy(void *data) {
	strcpy(temp_string, ui_copy());
	return temp_string;
}

char *_cut(void *data) {
	strcpy(temp_string, ui_cut());
	return temp_string;
}

void _paste(char *text, void *data) {
	ui_paste(text);
}

void _foreground(void *data) {
	iron_foreground();
	in_background = false;
}

void _resume(void *data) {
	iron_resume();
}

void _pause(void *data) {
	iron_pause();
}

void _background(void *data) {
	iron_background();
	in_background = true;
	paused_frames = 0;
}

void _shutdown(void *data) {
	iron_shutdown();
}

void _key_down(int code, void *data) {
	iron_key_down(code);
	if (ui_get_current())
		ui_key_down(ui_get_current(), code);

#ifdef IDLE_SLEEP
	input_down    = true;
	paused_frames = 0;
#endif
}

void _key_up(int code, void *data) {
	iron_key_up(code);
	if (ui_get_current())
		ui_key_up(ui_get_current(), code);

#ifdef IDLE_SLEEP
	input_down    = false;
	paused_frames = 0;
#endif
}

void _key_press(unsigned int character, void *data) {
	if (ui_get_current())
		ui_key_press(ui_get_current(), character);

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

void _mouse_down(int button, int x, int y, void *data) {
	iron_mouse_down(button, x, y);
	if (ui_get_current())
		ui_mouse_down(ui_get_current(), button, x, y);

#ifdef IDLE_SLEEP
	input_down    = true;
	paused_frames = 0;
#endif
}

void _mouse_up(int button, int x, int y, void *data) {
	iron_mouse_up(button, x, y);
	if (ui_get_current())
		ui_mouse_up(ui_get_current(), button, x, y);

#ifdef IDLE_SLEEP
	input_down    = false;
	paused_frames = 0;
#endif
}

void _mouse_move(int x, int y, int mx, int my, void *data) {
	iron_mouse_move(x, y, mx, my);
	if (ui_get_current())
		ui_mouse_move(ui_get_current(), x, y, mx, my);

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

void _mouse_wheel(int delta, void *data) {
	iron_mouse_wheel(delta);
	if (ui_get_current())
		ui_mouse_wheel(ui_get_current(), delta);

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

void _touch_move(int index, int x, int y) {
	iron_touch_move(index, x, y);

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	if (ui_get_current())
		ui_touch_move(ui_get_current(), index, x, y);
#endif

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

void _touch_down(int index, int x, int y) {
	iron_touch_down(index, x, y);

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	if (ui_get_current())
		ui_touch_down(ui_get_current(), index, x, y);
#endif

#ifdef IDLE_SLEEP
	input_down    = true;
	paused_frames = 0;
#endif
}

void _touch_up(int index, int x, int y) {
	iron_touch_up(index, x, y);

#if defined(IRON_ANDROID) || defined(IRON_IOS)
	if (ui_get_current())
		ui_touch_up(ui_get_current(), index, x, y);
#endif

#ifdef IDLE_SLEEP
	input_down    = false;
	paused_frames = 0;
#endif
}

void _pen_down(int x, int y, float pressure) {
	iron_pen_down(x, y, pressure);
	if (ui_get_current())
		ui_pen_down(ui_get_current(), x, y, pressure);

#ifdef IDLE_SLEEP
	input_down    = true;
	paused_frames = 0;
#endif
}

void _pen_up(int x, int y, float pressure) {
	iron_pen_up(x, y, pressure);
	if (ui_get_current())
		ui_pen_up(ui_get_current(), x, y, pressure);

#ifdef IDLE_SLEEP
	input_down    = false;
	paused_frames = 0;
#endif
}

void _pen_move(int x, int y, float pressure) {
	iron_pen_move(x, y, pressure);
	if (ui_get_current())
		ui_pen_move(ui_get_current(), x, y, pressure);

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

void _drop_files(char *file_path, void *data) {
// Update mouse position
#ifdef IRON_WINDOWS
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(iron_windows_window_handle(), &p);
	_mouse_move(p.x, p.y, 0, 0, NULL);
#endif

	iron_drop_files(file_path);

	in_background = false;
#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

f32 math_floor(f32 x) {
	return floorf(x);
}
f32 math_cos(f32 x) {
	return cosf(x);
}
f32 math_sin(f32 x) {
	return sinf(x);
}
f32 math_tan(f32 x) {
	return tanf(x);
}
f32 math_sqrt(f32 x) {
	return sqrtf(x);
}
f32 math_abs(f32 x) {
	return fabsf(x);
}
f32 math_random() {
	return rand() / (float)RAND_MAX;
}
f32 math_atan2(f32 y, f32 x) {
	return atan2f(y, x);
}
f32 math_asin(f32 x) {
	return asinf(x);
}
f32 math_pi() {
	return 3.14159265358979323846;
}
f32 math_pow(f32 x, f32 y) {
	return powf(x, y);
}
f32 math_round(f32 x) {
	return roundf(x);
}
f32 math_ceil(f32 x) {
	return ceilf(x);
}
f32 math_min(f32 x, f32 y) {
	return x < y ? x : y;
}
f32 math_max(f32 x, f32 y) {
	return x > y ? x : y;
}
f32 math_log(f32 x) {
	return logf(x);
}
f32 math_log2(f32 x) {
	return log2f(x);
}
f32 math_atan(f32 x) {
	return atanf(x);
}
f32 math_acos(f32 x) {
	return acosf(x);
}
f32 math_exp(f32 x) {
	return expf(x);
}
f32 math_fmod(f32 x, f32 y) {
	return fmod(x, y);
}

#ifdef _WIN32
i32 parse_int(const char *s) {
	return _strtoi64(s, NULL, 10);
}
i32 parse_int_hex(const char *s) {
	return _strtoi64(s, NULL, 16);
}
#else
i32 parse_int(const char *s) {
	return strtol(s, NULL, 10);
}
i32 parse_int_hex(const char *s) {
	return strtol(s, NULL, 16);
}
#endif
f32 parse_float(const char *s) {
	return strtof(s, NULL);
}

i32 color_from_floats(f32 r, f32 g, f32 b, f32 a) {
	return ((int)(a * 255) << 24) | ((int)(r * 255) << 16) | ((int)(g * 255) << 8) | (int)(b * 255);
}

u8 color_get_rb(i32 c) {
	return (c & 0x00ff0000) >> 16;
}

u8 color_get_gb(i32 c) {
	return (c & 0x0000ff00) >> 8;
}

u8 color_get_bb(i32 c) {
	return c & 0x000000ff;
}

u8 color_get_ab(i32 c) {
	return c & 0x000000ff;
}

i32 color_set_rb(i32 c, u8 i) {
	return (color_get_ab(c) << 24) | (i << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

i32 color_set_gb(i32 c, u8 i) {
	return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (i << 8) | color_get_bb(c);
}

i32 color_set_bb(i32 c, u8 i) {
	return (color_get_ab(c) << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | i;
}

i32 color_set_ab(i32 c, u8 i) {
	return (i << 24) | (color_get_rb(c) << 16) | (color_get_gb(c) << 8) | color_get_bb(c);
}

void _iron_init(iron_window_options_t *ops) {
	ops->display_index = -1;
	ops->visible       = enable_window;
	ops->color_bits    = 32;
	iron_init(ops);
	iron_random_init((int)(iron_time() * 1000));
	iron_set_cut_callback(_cut, NULL);
	iron_set_copy_callback(_copy, NULL);
	iron_set_paste_callback(_paste, NULL);
	iron_keyboard_set_key_press_callback(_key_press, NULL);
#ifdef WITH_AUDIO
	iron_a1_init();
	iron_a2_init();
#endif
}

void _iron_set_update_callback(void (*callback)(void)) {
	iron_update = callback;
	iron_set_update_callback(_update, NULL);
}

void _iron_set_drop_files_callback(void (*callback)(char *)) {
	iron_drop_files = callback;
	iron_set_drop_files_callback(_drop_files, NULL);
}

void iron_set_application_state_callback(void (*on_foreground)(void), void (*on_resume)(void), void (*on_pause)(void), void (*on_background)(void),
                                         void (*on_shutdown)(void)) {
	iron_set_foreground_callback(on_foreground != NULL ? _foreground : NULL, NULL);
	iron_set_resume_callback(on_resume != NULL ? _resume : NULL, NULL);
	iron_set_pause_callback(on_pause != NULL ? _pause : NULL, NULL);
	iron_set_background_callback(on_background != NULL ? _background : NULL, NULL);
	iron_set_shutdown_callback(on_shutdown != NULL ? _shutdown : NULL, NULL);
	iron_foreground = on_foreground;
	iron_resume     = on_resume;
	iron_pause      = on_pause;
	iron_background = on_background;
	iron_shutdown   = on_shutdown;
}

void iron_set_keyboard_down_callback(void (*callback)(int)) {
	iron_key_down = callback;
	iron_keyboard_set_key_down_callback(_key_down, NULL);
}

void iron_set_keyboard_up_callback(void (*callback)(int)) {
	iron_key_up = callback;
	iron_keyboard_set_key_up_callback(_key_up, NULL);
}

void iron_set_mouse_down_callback(void (*callback)(int, int, int)) {
	iron_mouse_down = callback;
	iron_mouse_set_press_callback(_mouse_down, NULL);
}

void iron_set_mouse_up_callback(void (*callback)(int, int, int)) {
	iron_mouse_up = callback;
	iron_mouse_set_release_callback(_mouse_up, NULL);
}

void iron_set_mouse_move_callback(void (*callback)(int, int, int, int)) {
	iron_mouse_move = callback;
	iron_mouse_set_move_callback(_mouse_move, NULL);
}

void iron_set_mouse_wheel_callback(void (*callback)(int)) {
	iron_mouse_wheel = callback;
	iron_mouse_set_scroll_callback(_mouse_wheel, NULL);
}

void iron_set_touch_down_callback(void (*callback)(int, int, int)) {
	iron_touch_down = callback;
	iron_surface_set_touch_start_callback(_touch_down);
}

void iron_set_touch_up_callback(void (*callback)(int, int, int)) {
	iron_touch_up = callback;
	iron_surface_set_touch_end_callback(_touch_up);
}

void iron_set_touch_move_callback(void (*callback)(int, int, int)) {
	iron_touch_move = callback;
	iron_surface_set_move_callback(_touch_move);
}

void iron_set_pen_down_callback(void (*callback)(int, int, float)) {
	iron_pen_down = callback;
	iron_pen_set_press_callback(_pen_down);
}

void iron_set_pen_up_callback(void (*callback)(int, int, float)) {
	iron_pen_up = callback;
	iron_pen_set_release_callback(_pen_up);
}

void iron_set_pen_move_callback(void (*callback)(int, int, float)) {
	iron_pen_move = callback;
	iron_pen_set_move_callback(_pen_move);
}

#ifdef WITH_GAMEPAD

void (*iron_gamepad_axis)(int, int, float);
void (*iron_gamepad_button)(int, int, float);

void _gamepad_axis(int gamepad, int axis, float value, void *data) {
	iron_gamepad_axis(gamepad, axis, value);

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

void _gamepad_button(int gamepad, int button, float value, void *data) {
	iron_gamepad_button(gamepad, button, value);

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

void iron_set_gamepad_axis_callback(void (*callback)(int, int, float)) {
	iron_gamepad_axis = callback;
	iron_gamepad_set_axis_callback(_gamepad_axis, NULL);
}

void iron_set_gamepad_button_callback(void (*callback)(int, int, float)) {
	iron_gamepad_button = callback;
	iron_gamepad_set_button_callback(_gamepad_button, NULL);
}

#endif

void gpu_delete_buffer(gpu_buffer_t *buffer) {
	gpu_buffer_destroy(buffer);
	free(buffer);
}

any gpu_create_index_buffer(i32 count) {
	gpu_buffer_t *buffer = (gpu_buffer_t *)malloc(sizeof(gpu_buffer_t));
	gpu_index_buffer_init(buffer, count);
	return buffer;
}

u32_array_t *gpu_lock_index_buffer(gpu_buffer_t *buffer) {
	u32_array_t *ar = (u32_array_t *)malloc(sizeof(u32_array_t));
	ar->buffer      = gpu_index_buffer_lock(buffer);
	ar->length      = buffer->count;
	return ar;
}

any gpu_create_vertex_buffer(i32 count, gpu_vertex_structure_t *structure) {
	gpu_buffer_t *buffer = (gpu_buffer_t *)malloc(sizeof(gpu_buffer_t));
	gpu_vertex_buffer_init(buffer, count, structure);
	return buffer;
}

buffer_t *gpu_lock_vertex_buffer(gpu_buffer_t *buffer) {
	buffer_t *b = (buffer_t *)malloc(sizeof(buffer_t));
	b->buffer   = gpu_vertex_buffer_lock(buffer);
	b->length   = buffer->count * buffer->stride;
	return b;
}

gpu_shader_t *gpu_create_shader(buffer_t *data, i32 shader_type) {
	gpu_shader_t *shader = (gpu_shader_t *)malloc(sizeof(gpu_shader_t));
	gpu_shader_init(shader, data->buffer, data->length, (gpu_shader_type_t)shader_type);
	return shader;
}

#ifdef WITH_KONG

#include "../../sources/libs/kong/analyzer.h"
#include "../../sources/libs/kong/backends/hlsl.h"
#include "../../sources/libs/kong/backends/metal.h"
#include "../../sources/libs/kong/backends/spirv.h"
#include "../../sources/libs/kong/backends/wgsl.h"
#include "../../sources/libs/kong/compiler.h"
#include "../../sources/libs/kong/disasm.h"
#include "../../sources/libs/kong/errors.h"
#include "../../sources/libs/kong/functions.h"
#include "../../sources/libs/kong/globals.h"
#include "../../sources/libs/kong/libs/stb_ds.h"
#include "../../sources/libs/kong/log.h"
#include "../../sources/libs/kong/names.h"
#include "../../sources/libs/kong/parser.h"
#include "../../sources/libs/kong/tokenizer.h"
#include "../../sources/libs/kong/transformer.h"
#include "../../sources/libs/kong/typer.h"
#include "../../sources/libs/kong/types.h"

extern uint64_t    next_variable_id;
extern size_t      allocated_globals_size;
extern function_id next_function_index;
extern global_id   globals_size;
extern name_id     names_index;
extern size_t      sets_count;
extern type_id     next_type_index;
extern size_t      vertex_inputs_size;
extern size_t      fragment_inputs_size;
extern size_t      vertex_functions_size;
extern size_t      fragment_functions_size;
extern struct {
	char   *key;
	name_id value;
}          *hash;
extern int  expression_index;
extern int  statement_index;
extern bool kong_error;

uint64_t    _next_variable_id;
size_t      _allocated_globals_size;
function_id _next_function_index;
global_id   _globals_size;
name_id     _names_index;
size_t      _sets_count;
type_id     _next_type_index;
size_t      _vertex_inputs_size;
size_t      _fragment_inputs_size;
size_t      _vertex_functions_size;
size_t      _fragment_functions_size;
struct {
	char   *key;
	name_id value;
}   *_hash;
int  _expression_index;
int  _statement_index;
void hlsl_export2(char **vs, char **fs, api_kind d3d, bool debug);
void spirv_export2(char **vs, char **fs, int *vs_size, int *fs_size, bool debug);
void console_info(char *s);

static struct {
	char   *key;
	name_id value;
} *_clone_hash(struct {
	char   *key;
	name_id value;
} * hash) {
	struct {
		char   *key;
		name_id value;
	} *clone = NULL;
	sh_new_arena(clone);
	ptrdiff_t len = shlen(hash);
	for (ptrdiff_t i = 0; i < len; i++) {
		shput(clone, hash[i].key, hash[i].value);
	}
	return clone;
}

void gpu_create_shaders_from_kong(char *kong, char **vs, char **fs, int *vs_size, int *fs_size) {
	static bool first = true;
	if (first) {
		first = false;
		names_init();
		types_init();
		functions_init();
		globals_init();

		_next_variable_id        = next_variable_id;
		_allocated_globals_size  = allocated_globals_size;
		_next_function_index     = next_function_index;
		_globals_size            = globals_size;
		_names_index             = names_index;
		_sets_count              = sets_count;
		_next_type_index         = next_type_index;
		_vertex_inputs_size      = vertex_inputs_size;
		_fragment_inputs_size    = fragment_inputs_size;
		_vertex_functions_size   = vertex_functions_size;
		_fragment_functions_size = fragment_functions_size;
		_hash                    = _clone_hash(hash);
		_expression_index        = expression_index;
		_statement_index         = statement_index;
	}
	else {
		next_variable_id        = _next_variable_id;
		allocated_globals_size  = _allocated_globals_size;
		next_function_index     = _next_function_index;
		globals_size            = _globals_size;
		names_index             = _names_index;
		sets_count              = _sets_count;
		next_type_index         = _next_type_index;
		vertex_inputs_size      = _vertex_inputs_size;
		fragment_inputs_size    = _fragment_inputs_size;
		vertex_functions_size   = _vertex_functions_size;
		fragment_functions_size = _fragment_functions_size;
		shfree(hash);
		hash             = _clone_hash(_hash);
		expression_index = _expression_index;
		statement_index  = _statement_index;
	}

	kong_error    = false;
	char  *from   = "";
	tokens tokens = tokenize(from, kong);
	parse(from, &tokens);
	resolve_types();
	if (kong_error) {
		console_info("Warning: Shader compilation failed");
		free(tokens.t);
#if defined(__APPLE__)
		*vs = "";
		*fs = "";
#endif
		return;
	}
	allocate_globals();
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		compile_function_block(&get_function(i)->code, get_function(i)->block);
	}
	analyze();

#ifdef _WIN32

	hlsl_export2(vs, fs, API_DIRECT3D11, false);

#elif defined(__APPLE__)

	static char vs_temp[1024 * 128];
	strcpy(vs_temp, "//>kong_vert\n");
	char *metal = metal_export("");
	strcat(vs_temp, metal);
	*vs = &vs_temp[0];
	*fs = "//>kong_frag\n";
	free(metal);

#else

	transform(TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE | TRANSFORM_FLAG_BINARY_UNIFY_LENGTH);
	spirv_export2(vs, fs, vs_size, fs_size, false);

#endif

	free(tokens.t);
}

#endif

gpu_shader_t *gpu_create_shader_from_source(string_t *source, int source_size, gpu_shader_type_t shader_type) {
	gpu_shader_t *shader        = (gpu_shader_t *)malloc(sizeof(gpu_shader_t));
	char         *temp_string_s = shader_type == GPU_SHADER_TYPE_VERTEX ? temp_string_vs : temp_string_fs;

#ifdef WITH_D3DCOMPILER

	strcpy(temp_string_s, source);

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	UINT      flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;
	HRESULT hr = D3DCompile(temp_string_s, strlen(source) + 1, NULL, NULL, NULL, "main", shader_type == GPU_SHADER_TYPE_VERTEX ? "vs_5_0" : "ps_5_0", flags, 0,
	                        &shader_buffer, &error_message);
	if (hr != S_OK) {
		iron_log("%s", (char *)error_message->lpVtbl->GetBufferPointer(error_message));
		return NULL;
	}

	int size = shader_buffer->lpVtbl->GetBufferSize(shader_buffer);
	gpu_shader_init(shader, (char *)shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), size, shader_type);
	shader_buffer->lpVtbl->Release(shader_buffer);

#elif defined(IRON_METAL)

	strcpy(temp_string_s, source);
	gpu_shader_init(shader, temp_string_s, strlen(temp_string_s), shader_type);

#elif defined(IRON_VULKAN)

	gpu_shader_init(shader, source, source_size, shader_type);

#endif

	return shader;
}

gpu_pipeline_t *gpu_create_pipeline() {
	gpu_pipeline_t *pipeline = (gpu_pipeline_t *)malloc(sizeof(gpu_pipeline_t));
	gpu_pipeline_init(pipeline);
	return pipeline;
}

void gpu_delete_pipeline(gpu_pipeline_t *pipeline) {
	gpu_pipeline_destroy(pipeline);
	free(pipeline);
}

gpu_texture_t *gpu_create_render_target(i32 width, i32 height, i32 format) {
	gpu_texture_t *render_target = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	gpu_render_target_init(render_target, width, height, (gpu_texture_format_t)format);
	render_target->buffer = NULL;
	return render_target;
}

gpu_texture_t *gpu_create_texture_from_bytes(buffer_t *data, i32 width, i32 height, i32 format) {
	gpu_texture_t *texture = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	texture->buffer        = NULL;
	gpu_texture_init_from_bytes(texture, data->buffer, width, height, (gpu_texture_format_t)format);
	return texture;
}

gpu_texture_t *gpu_create_texture_from_encoded_bytes(buffer_t *data, string_t *format) {
	gpu_texture_t *texture = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	texture->buffer        = NULL;
	unsigned char       *texture_data;
	gpu_texture_format_t texture_format;
	int                  width;
	int                  height;

	if (ends_with(format, "k")) {
		width  = iron_read_s32le(data->buffer);
		height = iron_read_s32le(data->buffer + 4);
		char fourcc[5];
		fourcc[0]           = data->buffer[8];
		fourcc[1]           = data->buffer[9];
		fourcc[2]           = data->buffer[10];
		fourcc[3]           = data->buffer[11];
		fourcc[4]           = 0;
		int compressed_size = data->length - 12;
		if (strcmp(fourcc, "LZ4 ") == 0) {
			int output_size = width * height * 4;
			texture_data    = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)data->buffer + 12, (char *)texture_data, compressed_size, output_size);
			texture_format = GPU_TEXTURE_FORMAT_RGBA32;
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			int output_size = width * height * 16;
			texture_data    = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)data->buffer + 12, (char *)texture_data, compressed_size, output_size);
			texture_format = GPU_TEXTURE_FORMAT_RGBA128;
		}
	}
	else if (ends_with(format, "hdr")) {
		int comp;
		texture_data   = (unsigned char *)stbi_loadf_from_memory(data->buffer, data->length, &width, &height, &comp, 4);
		texture_format = GPU_TEXTURE_FORMAT_RGBA128;
	}
	else { // jpg, png, ..
		int comp;
		texture_data   = stbi_load_from_memory(data->buffer, data->length, &width, &height, &comp, 4);
		texture_format = GPU_TEXTURE_FORMAT_RGBA32;
	}

	gpu_texture_init_from_bytes(texture, texture_data, width, height, texture_format);
	free(texture_data);

	return texture;
}

void gpu_delete_texture(gpu_texture_t *texture) {
	gpu_texture_destroy(texture);
	free(texture);
}

gpu_texture_t *iron_load_texture(string_t *file) {
#ifdef WITH_EMBED
	buffer_t *b = embed_get(file);
	if (b != NULL) {
		return gpu_create_texture_from_encoded_bytes(b, ".k");
	}
#endif

	iron_file_reader_t reader;
	if (!iron_file_reader_open(&reader, file, IRON_FILE_TYPE_ASSET)) {
		return NULL;
	}
	int            size = (int)iron_file_reader_size(&reader);
	unsigned char *data = (unsigned char *)malloc(size);
	iron_file_reader_read(&reader, data, size);
	iron_file_reader_close(&reader);
	buffer_t buf;
	buf.buffer = data;
	buf.length = size;
	return gpu_create_texture_from_encoded_bytes(&buf, file);
}

#ifdef WITH_AUDIO
any iron_load_sound(string_t *file) {
	iron_a1_sound_t *sound = iron_a1_sound_create(file);
	return sound;
}
#endif

buffer_t *iron_load_blob(string_t *file) {
#ifdef WITH_EMBED
	buffer_t *b = embed_get(file);
	if (b != NULL) {
		return b;
	}
#endif

	iron_file_reader_t reader;
	if (!iron_file_reader_open(&reader, file, IRON_FILE_TYPE_ASSET)) {
		return NULL;
	}
	uint32_t  reader_size = (uint32_t)iron_file_reader_size(&reader);
	buffer_t *buffer      = buffer_create(reader_size);
	iron_file_reader_read(&reader, buffer->buffer, reader_size);
	iron_file_reader_close(&reader);
	return buffer;
}

i32 iron_screen_dpi() {
	return iron_display_current_mode(iron_primary_display()).pixels_per_inch;
}

i32 iron_display_width(i32 index) {
	return iron_display_current_mode(index).width;
}

i32 iron_display_height(i32 index) {
	return iron_display_current_mode(index).height;
}

i32 iron_display_x(i32 index) {
	return iron_display_current_mode(index).x;
}

i32 iron_display_y(i32 index) {
	return iron_display_current_mode(index).y;
}

i32 iron_display_frequency(i32 index) {
	return iron_display_current_mode(index).frequency;
}

bool iron_display_is_primary(i32 index) {
	return index == iron_primary_display();
}

buffer_t *gpu_get_texture_pixels(gpu_texture_t *image) {
	if (image->buffer == NULL) {
		image->buffer         = malloc(sizeof(buffer_t));
		image->buffer->buffer = NULL;
	}
	image->buffer->length = gpu_texture_format_size(image->format) * image->width * image->height;

	if (image->buffer->buffer == NULL) {
		image->buffer->buffer = malloc(image->buffer->length);
	}

	gpu_get_render_target_pixels(image, image->buffer->buffer);
	return image->buffer;
}

void _gpu_begin(gpu_texture_t *render_target, any_array_t *additional, gpu_texture_t *depth_buffer, gpu_clear_t flags, unsigned color, float depth) {
	if (render_target == NULL) {
		gpu_begin(NULL, 0, NULL, flags, color, depth);
	}
	else {
		int32_t        length            = 1;
		gpu_texture_t *render_targets[8] = {render_target, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
		if (additional != NULL) {
			length = additional->length + 1;
			for (int32_t i = 1; i < length; ++i) {
				render_targets[i] = additional->buffer[i - 1];
			}
		}
		gpu_begin(render_targets, length, depth_buffer, flags, color, depth);
	}
}

void iron_file_save_bytes(string_t *path, buffer_t *bytes, u64 length) {
	u64 byte_length = length > 0 ? length : (u64)bytes->length;
	if (byte_length > (u64)bytes->length) {
		byte_length = (u64)bytes->length;
	}

#ifdef IRON_WINDOWS
	MultiByteToWideChar(CP_UTF8, 0, path, -1, temp_wstring, 1024);
	FILE *file = _wfopen(temp_wstring, L"wb");
#else
	FILE *file = fopen(path, "wb");
#endif
	if (file == NULL) {
		return;
	}
	fwrite(bytes->buffer, 1, byte_length, file);
	fclose(file);
}

i32 iron_sys_command(string_t *cmd) {
#ifdef IRON_WINDOWS

	int      wlen = MultiByteToWideChar(CP_UTF8, 0, cmd, -1, NULL, 0);
	wchar_t *wstr = malloc(sizeof(wchar_t) * wlen);
	MultiByteToWideChar(CP_UTF8, 0, cmd, -1, wstr, wlen);
	wchar_t comspec[MAX_PATH];
	GetEnvironmentVariableW(L"ComSpec", comspec, MAX_PATH);
	wchar_t cmdline[2048];
	swprintf(cmdline, 2048, L"\"%s\" /c %s", comspec, wstr);
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb          = sizeof(si);
	si.dwFlags     = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	free(wstr);
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exit_code = 0;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	int result = (int)exit_code;

#elif defined(IRON_IOS)
	int result = 0;
#else
	int result = system(cmd);
#endif
	return result;
}

typedef struct _callback_data {
	int32_t size;
	char    url[512];
	void (*func)(char *, buffer_t *);
} _callback_data_t;

void _https_callback(const char *body, void *callback_data) {
	_callback_data_t *cbd    = (_callback_data_t *)callback_data;
	buffer_t         *buffer = NULL;
	if (body != NULL) {
		buffer         = malloc(sizeof(buffer_t));
		buffer->length = cbd->size > 0 ? cbd->size : strlen(body);
		buffer->buffer = malloc(buffer->length);
		memcpy(buffer->buffer, body, buffer->length);
	}
	cbd->func(cbd->url, buffer);
	free(cbd);
}

void iron_file_download(string_t *url, void (*callback)(char *, buffer_t *), i32 size) {
	_callback_data_t *cbd = malloc(sizeof(_callback_data_t));
	cbd->size             = size;
	strcpy(cbd->url, url);
	cbd->func = callback;

	char url_base[512];
	char url_path[512];
	int  i = 0;
	for (; i < strlen(url) - 8; ++i) {
		if (url[i + 8] == '/') {
			break;
		}
		url_base[i] = url[i + 8]; // Strip https://
	}
	url_base[i] = 0;
	int j       = 0;
	if (strlen(url_base) < strlen(url) - 8) {
		++i; // Skip /
	}
	for (; j < strlen(url) - 8 - i; ++j) {
		if (url[i + 8 + j] == 0) {
			break;
		}
		url_path[j] = url[i + 8 + j];
	}
	url_path[j] = 0;

	iron_https_request(url_base, url_path, NULL, 443, 0, &_https_callback, cbd);
}

#if defined(IRON_WINDOWS) || (defined(IRON_LINUX) && defined(WITH_NFD))
bool _save_and_quit_callback_internal();
#endif

bool _save_and_quit_callback(void *data) {
#if defined(IRON_WINDOWS) || (defined(IRON_LINUX) && defined(WITH_NFD)) // Has gtk
	return _save_and_quit_callback_internal();
#endif
	return true;
}

void iron_set_save_and_quit_callback(void (*callback)(bool)) {
	iron_save_and_quit = callback;
	iron_window_set_close_callback(_save_and_quit_callback, NULL);
}

void iron_delay_idle_sleep() {
	paused_frames = 0;
}

#ifdef WITH_NFD
char_ptr_array_t *iron_open_dialog(char *filter_list, char *default_path, bool open_multiple) {
	nfdpathset_t out_paths;
	nfdchar_t   *out_path;
	nfdresult_t  result = open_multiple ? NFD_OpenDialogMultiple(filter_list, default_path, &out_paths) : NFD_OpenDialog(filter_list, default_path, &out_path);

	if (result == NFD_OKAY) {
		int               path_count = open_multiple ? (int)NFD_PathSet_GetCount(&out_paths) : 1;
		char_ptr_array_t *result     = any_array_create(path_count);

		if (open_multiple) {
			for (int i = 0; i < path_count; ++i) {
				nfdchar_t *out_path = NFD_PathSet_GetPath(&out_paths, i);
				result->buffer[i]   = out_path;
			}
			// NFD_PathSet_Free(&out_paths);
		}
		else {
			result->buffer[0] = out_path;
			// free(out_path);
		}
		return result;
	}
	return NULL;
}

static char iron_save_dialog_path[512];

char *iron_save_dialog(char *filter_list, char *default_path) {
	nfdchar_t  *out_path = NULL;
	nfdresult_t result   = NFD_SaveDialog(filter_list, default_path, &out_path);
	if (result == NFD_OKAY) {
		strcpy(iron_save_dialog_path, out_path);
		free(out_path);
		return iron_save_dialog_path;
	}
	return NULL;
}

#elif defined(IRON_ANDROID)

char_ptr_array_t *iron_open_dialog(char *filter_list, char *default_path, bool open_multiple) {
	AndroidFileDialogOpen();
	return NULL;
}

char *iron_save_dialog(char *filter_list, char *default_path) {
	wchar_t *out_path = AndroidFileDialogSave();
	wcstombs(temp_string, out_path, sizeof(temp_string));
	return temp_string;
}

#elif defined(IRON_IOS)

char_ptr_array_t *iron_open_dialog(char *filter_list, char *default_path, bool open_multiple) {
	// Once finished drop_files callback is called
	IOSFileDialogOpen();
	return NULL;
}

char *iron_save_dialog(char *filter_list, char *default_path) {
	// Path to app document directory
	wchar_t *out_path = IOSFileDialogSave();
	wcstombs(temp_string, out_path, sizeof(temp_string));
	return temp_string;
}
#endif

char *iron_read_directory(char *path) {
	char *files = temp_string;
	files[0]    = 0;

	directory dir = open_dir(path);
	if (dir.handle == NULL) {
		return files;
	}

	while (true) {
		file f = read_next_file(&dir);
		if (!f.valid) {
			break;
		}

#ifdef IRON_WINDOWS
		char file_path[512];
		strcpy(file_path, path);
		strcat(file_path, "\\");
		strcat(file_path, f.name);
		if (FILE_ATTRIBUTE_HIDDEN & GetFileAttributesA(file_path)) {
			continue; // Skip hidden files
		}
#endif

		if (files[0] != '\0') {
			strcat(files, "\n");
		}
		strcat(files, f.name);
	}
	close_dir(&dir);
	return files;
}

void iron_create_directory(char *path) {
#ifdef IRON_IOS
	IOSCreateDirectory(path);
#elif defined(IRON_WINDOWS)
	char cmd[1024];
	strcpy(cmd, "mkdir \"");
	strcat(cmd, path);
	strcat(cmd, "\"");
	iron_sys_command(cmd);
#else
	char cmd[1024];
	strcpy(cmd, "mkdir -p \"");
	strcat(cmd, path);
	strcat(cmd, "\"");
	iron_sys_command(cmd);
#endif
}

bool iron_file_exists(char *path) {
	iron_file_reader_t reader;
	if (iron_file_reader_open(&reader, path, IRON_FILE_TYPE_ASSET)) {
		iron_file_reader_close(&reader);
		return true;
	}
	return false;
}

void iron_delete_file(char *path) {
#ifdef IRON_IOS
	IOSDeleteFile(path);
#elif defined(IRON_WINDOWS)
	char cmd[1024];
	strcpy(cmd, "del /f \"");
	strcat(cmd, path);
	strcat(cmd, "\"");
	iron_sys_command(cmd);
#else
	char cmd[1024];
	strcpy(cmd, "rm \"");
	strcat(cmd, path);
	strcat(cmd, "\"");
	iron_sys_command(cmd);
#endif
}

#ifdef WITH_COMPRESS
buffer_t *iron_inflate(buffer_t *bytes, bool raw) {
	unsigned char *inflated     = NULL;
	int            inflated_len = bytes->length * 2;
	int            out_len;
	while (1) {
		inflated = (unsigned char *)realloc(inflated, inflated_len);
		out_len  = raw ? sinflate(inflated, inflated_len, bytes->buffer, bytes->length) : zsinflate(inflated, inflated_len, bytes->buffer, bytes->length);
		if (out_len >= 0 && out_len < inflated_len) {
			break;
		}
		inflated_len *= 2;
	}
	buffer_t *output = buffer_create(0);
	output->buffer   = inflated;
	output->length   = out_len;
	return output;
}

buffer_t *iron_deflate(buffer_t *bytes, bool raw) {
	struct sdefl sdefl;
	memset(&sdefl, 0, sizeof(sdefl));
	void     *deflated = malloc(sdefl_bound(bytes->length));
	int       out_len  = raw ? sdeflate(&sdefl, deflated, bytes->buffer, bytes->length, SDEFL_LVL_MIN)
	                         : zsdeflate(&sdefl, deflated, bytes->buffer, bytes->length, SDEFL_LVL_MIN);
	buffer_t *output   = buffer_create(0);
	output->buffer     = deflated;
	output->length     = out_len;
	return output;
}

unsigned char *iron_deflate_raw(unsigned char *data, int data_len, int *out_len, int quality) {
	struct sdefl sdefl;
	memset(&sdefl, 0, sizeof(sdefl));
	void *deflated = malloc(sdefl_bound(data_len));
	*out_len       = zsdeflate(&sdefl, deflated, data, data_len, SDEFL_LVL_MIN);
	return (unsigned char *)deflated;
}
#endif

#ifdef WITH_IMAGE_WRITE
void _write_image(char *path, buffer_t *bytes, i32 w, i32 h, i32 format, int image_format, int quality) {
	int            comp   = 0;
	unsigned char *pixels = NULL;
	unsigned char *rgba   = (unsigned char *)bytes->buffer;
	if (format == 0) { // RGBA
		comp   = 4;
		pixels = rgba;
	}
	else if (format == 1) { // R
		comp   = 1;
		pixels = rgba;
	}
	else if (format == 2) { // RGB1
		comp   = 3;
		pixels = (unsigned char *)malloc(w * h * comp);
		for (int i = 0; i < w * h; ++i) {
#ifdef IRON_BGRA
			pixels[i * 3]     = rgba[i * 4 + 2];
			pixels[i * 3 + 1] = rgba[i * 4 + 1];
			pixels[i * 3 + 2] = rgba[i * 4];
#else
			pixels[i * 3]     = rgba[i * 4];
			pixels[i * 3 + 1] = rgba[i * 4 + 1];
			pixels[i * 3 + 2] = rgba[i * 4 + 2];
#endif
		}
	}
	else if (format > 2) { // RRR1, GGG1, BBB1, AAA1
		comp    = 1;
		pixels  = (unsigned char *)malloc(w * h * comp);
		int off = format - 3;
#ifdef IRON_BGRA
		off = 2 - off;
#endif
		for (int i = 0; i < w * h; ++i) {
			pixels[i] = rgba[i * 4 + off];
		}
	}

	image_format == 0 ? stbi_write_jpg(path, w, h, comp, pixels, quality) : stbi_write_png(path, w, h, comp, pixels, w * comp);

	if (pixels != rgba) {
		free(pixels);
	}
}

void iron_write_jpg(char *path, buffer_t *bytes, i32 w, i32 h, i32 format, i32 quality) {
	// RGBA, R, RGB1, RRR1, GGG1, BBB1, AAA1
	_write_image(path, bytes, w, h, format, 0, quality);
}

void iron_write_png(char *path, buffer_t *bytes, i32 w, i32 h, i32 format) {
	_write_image(path, bytes, w, h, format, 1, 100);
}

unsigned char *_encode_data;
int            _encode_size;
void           _encode_image_func(void *context, void *data, int size) {
    memcpy(_encode_data + _encode_size, data, size);
    _encode_size += size;
}

buffer_t *_encode_image(buffer_t *bytes, i32 w, i32 h, i32 format, i32 quality) {
#ifdef IRON_BGRA
	unsigned char *pixels = bytes->buffer;
	for (int i = 0; i < w * h; ++i) {
		unsigned char c   = pixels[i * 4];
		pixels[i * 4]     = pixels[i * 4 + 2];
		pixels[i * 4 + 2] = c;
	}
#endif
	_encode_data = (unsigned char *)malloc(w * h * 4);
	_encode_size = 0;
	format == 0 ? stbi_write_jpg_to_func(&_encode_image_func, NULL, w, h, 4, bytes->buffer, quality)
	            : stbi_write_png_to_func(&_encode_image_func, NULL, w, h, 4, bytes->buffer, w * 4);
	buffer_t *buffer = malloc(sizeof(buffer_t));
	buffer->buffer   = _encode_data;
	buffer->length   = _encode_size;
	return buffer;
}

buffer_t *iron_encode_jpg(buffer_t *bytes, i32 w, i32 h, i32 format, i32 quality) {
	return _encode_image(bytes, w, h, 0, quality);
}

buffer_t *iron_encode_png(buffer_t *bytes, i32 w, i32 h, i32 format) {
	return _encode_image(bytes, w, h, 1, 100);
}
#endif

#ifdef WITH_VIDEO_WRITE

static FILE            *iron_mp4_fp;
static int              iron_mp4_w;
static int              iron_mp4_h;
static int              iron_mp4_stride;
static H264E_persist_t *iron_mp4_enc     = NULL;
static H264E_scratch_t *iron_mp4_scratch = NULL;
static char             iron_mp4_path[512];
static char             iron_mp4_path_264[512];
static char            *iron_mp4_yuv_buf;

static size_t iron_mp4_get_nal_size(uint8_t *buf, size_t size) {
	size_t pos = 3;
	while ((size - pos) > 3) {
		if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 1) {
			return pos;
		}
		if (buf[pos] == 0 && buf[pos + 1] == 0 && buf[pos + 2] == 0 && buf[pos + 3] == 1) {
			return pos;
		}
		pos++;
	}
	return size;
}

static int iron_mp4_write_callback(int64_t offset, const void *buffer, size_t size, void *token) {
	FILE *f = (FILE *)token;
	fseek(f, offset, SEEK_SET);
	return fwrite(buffer, 1, size, f) != size;
}

void iron_mp4_begin(char *path, i32 w, i32 h) {
	strcpy(iron_mp4_path, path);
	strcpy(iron_mp4_path_264, path);
	int len                    = strlen(iron_mp4_path_264);
	iron_mp4_path_264[len - 1] = '4';
	iron_mp4_path_264[len - 2] = '6';
	iron_mp4_path_264[len - 3] = '2';

	iron_mp4_stride = w;
	iron_mp4_w      = w - w % 16;
	iron_mp4_h      = h - h % 16;

	H264E_create_param_t create_param = {0};
	create_param.width                = iron_mp4_w;
	create_param.height               = iron_mp4_h;
	int sizeof_persist                = 0;
	int sizeof_scratch                = 0;
	H264E_sizeof(&create_param, &sizeof_persist, &sizeof_scratch);

	iron_mp4_enc     = (H264E_persist_t *)malloc(sizeof_persist);
	iron_mp4_scratch = (H264E_scratch_t *)malloc(sizeof_scratch);
	H264E_init(iron_mp4_enc, &create_param);

	iron_mp4_fp      = fopen(iron_mp4_path_264, "wb");
	int frame_size   = (int)(iron_mp4_w * iron_mp4_h * 1.5);
	iron_mp4_yuv_buf = malloc(frame_size);
}

void iron_mp4_end() {
	if (iron_mp4_fp == NULL) {
		return;
	}

	buffer_t         *blob     = iron_load_blob(iron_mp4_path_264);
	uint8_t          *buf      = blob->buffer;
	size_t            buf_size = blob->length;
	FILE             *fout     = fopen(iron_mp4_path, "wb");
	MP4E_mux_t       *mux      = MP4E_open(0, 0, fout, iron_mp4_write_callback);
	mp4_h26x_writer_t mp4wr;
	mp4_h26x_write_init(&mp4wr, mux, iron_mp4_w, iron_mp4_h, false);

	while (buf_size > 0) {
		size_t nal_size = iron_mp4_get_nal_size(buf, buf_size);
		if (nal_size < 4) {
			buf += 1;
			buf_size -= 1;
			continue;
		}

		int fps = 24;
		mp4_h26x_write_nal(&mp4wr, buf, nal_size, 90000 / fps);
		buf += nal_size;
		buf_size -= nal_size;
	}

	MP4E_close(mux);
	mp4_h26x_write_close(&mp4wr);
	free(iron_mp4_enc);
	free(iron_mp4_scratch);
	free(iron_mp4_yuv_buf);
	fclose(fout);
	fclose(iron_mp4_fp);
	iron_mp4_fp = NULL;
}

void iron_mp4_encode(buffer_t *pixels) {
	// rgba to yuv420p
	for (int i = 0; i < iron_mp4_w; ++i) {
		for (int j = 0; j < iron_mp4_h; ++j) {
			int     k                                                                     = i + j * iron_mp4_stride;
			uint8_t r                                                                     = pixels->buffer[k * 4];
			uint8_t g                                                                     = pixels->buffer[k * 4 + 1];
			uint8_t b                                                                     = pixels->buffer[k * 4 + 2];
			uint8_t y                                                                     = ((66 * r + 129 * g + 25 * b + 128) / 256) + 16;
			uint8_t u                                                                     = ((-38 * r - 74 * g + 112 * b + 128) / 256) + 128;
			uint8_t v                                                                     = ((112 * r - 94 * g - 18 * b + 128) / 256) + 128;
			int     l                                                                     = i + j * iron_mp4_w;
			int     m                                                                     = i / 2 + j / 2 * (iron_mp4_w / 2);
			iron_mp4_yuv_buf[l]                                                           = y;
			iron_mp4_yuv_buf[iron_mp4_w * iron_mp4_h + m]                                 = u;
			iron_mp4_yuv_buf[iron_mp4_w * iron_mp4_h + (iron_mp4_w * iron_mp4_h) / 4 + m] = v;
		}
	}

	H264E_run_param_t run_param   = {0};
	run_param.frame_type          = 0;
	run_param.encode_speed        = H264E_SPEED_SLOWEST;        // H264E_SPEED_FASTEST;
	run_param.desired_frame_bytes = (2048 * 4) * 1000 / 8 / 30; // 2048 * 4 kbps
	run_param.qp_min              = 10;
	run_param.qp_max              = 50;

	H264E_io_yuv_t yuv;
	yuv.yuv[0]    = iron_mp4_yuv_buf;
	yuv.stride[0] = iron_mp4_w;
	yuv.yuv[1]    = iron_mp4_yuv_buf + iron_mp4_w * iron_mp4_h;
	yuv.stride[1] = iron_mp4_w / 2;
	yuv.yuv[2]    = iron_mp4_yuv_buf + (int)(iron_mp4_w * iron_mp4_h * 1.25);
	yuv.stride[2] = iron_mp4_w / 2;

	uint8_t *coded_data;
	int      sizeof_coded_data;
	H264E_encode(iron_mp4_enc, iron_mp4_scratch, &run_param, &yuv, &coded_data, &sizeof_coded_data);
	fwrite(coded_data, sizeof_coded_data, 1, iron_mp4_fp);
}
#endif

#endif // NO_IRON_API
