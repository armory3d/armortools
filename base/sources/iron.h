#pragma once

#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "iron_system.h"
#include "iron_thread.h"
#include "iron_gpu.h"
#include "iron_file.h"
#include "iron_string.h"
#include "iron_array.h"
#include "iron_map.h"
#include "iron_armpack.h"
#include "iron_json.h"
#include "iron_gc.h"
#include "iron_obj.h"
#include "iron_vec2.h"
#include "iron_vec3.h"
#include "iron_vec4.h"
#include "iron_quat.h"
#include "iron_mat3.h"
#include "iron_mat4.h"
#include "iron_ui.h"
#include "iron_ui_nodes.h"
#include "iron_draw.h"
#ifdef IRON_WINDOWS
#include <Windows.h>
#endif
#ifdef WITH_AUDIO
#include "iron_audio.h"
#endif
#ifdef WITH_EMBED
#include EMBED_H_PATH
#endif
#ifdef WITH_ONNX
#include <onnxruntime_c_api.h>
#ifdef IRON_WINDOWS
#include <dml_provider_factory.h>
#elif defined(IRON_MACOS)
#include <coreml_provider_factory.h>
#endif
const OrtApi *ort = NULL;
OrtEnv *ort_env;
OrtSessionOptions *ort_session_options;
OrtSession *session = NULL;
#endif

int _argc;
char **_argv;

#ifdef WITH_EVAL
#include "quickjs.h"
#include "quickjs-libc.h"
JSRuntime *js_runtime = NULL;
JSContext *js_ctx;
#ifdef WITH_PLUGINS
void plugin_api_init();
#endif

void js_init() {
	js_runtime = JS_NewRuntime();
	js_ctx = JS_NewContext(js_runtime);
	js_std_add_helpers(js_ctx, _argc, _argv);
	js_init_module_std(js_ctx, "std");
	js_init_module_os(js_ctx, "os");
	#ifdef WITH_PLUGINS
	plugin_api_init();
	#endif
}

float js_eval(const char *js) {
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

JSValue js_call_result;

JSValue *js_call_arg(void *p, int argc, JSValue *argv) {
	if (js_runtime == NULL) {
		js_init();
	}
	JSValue fn = *(JSValue *)p;
	JSValue global_obj = JS_GetGlobalObject(js_ctx);
	js_call_result = JS_Call(js_ctx, fn, global_obj, argc, argv);
	if (JS_IsException(js_call_result)) {
		js_std_dump_error(js_ctx);
		JS_ResetUncatchableError(js_ctx);
	}
	JS_FreeValue(js_ctx, global_obj);
	return &js_call_result;
}

char *js_call_ptr(void *p, void *arg) {
	JSValue argv[] = { JS_NewInt64(js_ctx, (int64_t)arg) };
	return (char *)JS_ToCString(js_ctx, *js_call_arg(p, 1, argv));
}

char *js_call_ptr_str(void *p, void *arg0, char *arg1) {
	JSValue argv[] = { JS_NewInt64(js_ctx, (int64_t)arg0), JS_NewString(js_ctx, arg1) };
	return (char *)JS_ToCString(js_ctx, *js_call_arg(p, 2, argv));
}

void *js_pcall_str(void *p, char *arg0) {
	JSValue argv[] = { JS_NewString(js_ctx, arg0) };
	int64_t result;
	JS_ToInt64(js_ctx, &result, *js_call_arg(p, 1, argv));
	return (void *)result;
}

char *js_call(void *p) {
	return (char *)JS_ToCString(js_ctx, *js_call_arg(p, 0, NULL));
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
			buffer->buffer = embed_values[i];
			buffer->length = embed_sizes[i];
			return buffer;
		}
	}
	return NULL;
}
#endif

#define f64 double
#define i64 int64_t
#define u64 uint64_t
#define f32 float
#define i32 int32_t
#define u32 uint32_t
#define i16 int16_t
#define u16 uint16_t
#define i8 int8_t
#define u8 uint8_t
#define string_t char
#define any void *
#define any_ptr void **
#define f64_ptr f64 *
#define i64_ptr i64 *
#define u64_ptr u64 *
#define f32_ptr f32 *
#define i32_ptr i32 *
#define u32_ptr u32 *
#define i16_ptr i16 *
#define u16_ptr u16 *
#define i8_ptr i8 *
#define u8_ptr u8 *
#define null NULL
#define DEREFERENCE *
#define ADDRESS &
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
int paused_frames = 0;
bool save_and_quit_callback_set = false;
#ifdef IDLE_SLEEP
bool input_down = false;
int last_window_width = 0;
int last_window_height = 0;
#endif

char temp_string[1024 * 32];
char temp_string_vs[1024 * 128];
char temp_string_fs[1024 * 128];
#ifdef IRON_WINDOWS
wchar_t temp_wstring[1024 * 32];
struct HWND__ *iron_windows_window_handle();
#endif

void (*iron_update)(void);
void (*iron_drop_files)(char *);
char *(*iron_cut)(void *);
char *(*iron_copy)(void *);
void (*iron_paste)(char *, void *);
void (*iron_foreground)(void);
void (*iron_resume)(void);
void (*iron_pause)(void);
void (*iron_background)(void);
void (*iron_shutdown)(void);
void (*iron_pause)(void);
void (*iron_key_down)(int);
void (*iron_key_up)(int);
void (*iron_key_press)(int);
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
void (*iron_gamepad_axis)(int, int, float);
void (*iron_gamepad_button)(int, int, float);
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
	if ((l > 6 && assetsdir[l - 6] == '.') ||
		(l > 5 && assetsdir[l - 5] == '.') ||
		(l > 4 && assetsdir[l - 4] == '.')) {
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
	#ifdef WITH_ONNX
	if (ort != NULL) {
		ort->ReleaseEnv(ort_env);
		ort->ReleaseSessionOptions(ort_session_options);
	}
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

#include <stdio.h>
#include "iron_math.h"
#include "iron_net.h"
#include <lz4x.h>
#include "kong/dir.h"
#ifdef WITH_AUDIO
#include "iron_audio.h"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#ifdef IRON_DIRECT3D12
#include <d3d12.h>
#endif
#ifdef WITH_D3DCOMPILER
#include <d3d11.h>
#include <D3Dcompiler.h>
#endif
#ifdef WITH_NFD
#include <nfd.h>
#elif defined(IRON_ANDROID)
#include "backends/android_file_dialog.h"
#include "backends/android_http_request.h"
#elif defined(IRON_IOS)
#include <wchar.h>
#include "backends/ios_file_dialog.h"
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
// #include <unistd.h>
int usleep(unsigned int usec);
#endif

#ifdef IRON_MACOS
const char *macgetresourcepath();
#endif
#ifdef IRON_IOS
const char *iphonegetresourcepath();
#endif

#if defined(IRON_IOS) || defined(IRON_ANDROID)
char mobile_title[1024];
#endif

static gpu_buffer_t constant_buffer;
static gpu_texture_t *render_target;
static gpu_raytrace_pipeline_t rt_pipeline;
static gpu_raytrace_acceleration_structure_t accel;
static bool raytrace_created = false;
static bool raytrace_accel_created = false;
const int constant_buffer_size = 24;

void _update(void *data) {
	#ifdef IRON_WINDOWS
	if (in_background && ++paused_frames > 3) {
		Sleep(1);
		return;
	}
	#endif

	#ifdef IDLE_SLEEP
	if (last_window_width != iron_window_width() || last_window_height != iron_window_height()) {
		last_window_width = iron_window_width();
		last_window_height = iron_window_height();
		paused_frames = 0;
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
		usleep(1000);
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

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_key_down(ui_instances[i], code);
	}

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _key_up(int code, void *data) {
	iron_key_up(code);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_key_up(ui_instances[i], code);
	}

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _key_press(unsigned int character, void *data) {
	iron_key_press(character);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_key_press(ui_instances[i], character);
	}

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _mouse_down(int button, int x, int y, void *data) {
	iron_mouse_down(button, x, y);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_mouse_down(ui_instances[i], button, x, y);
	}

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _mouse_up(int button, int x, int y, void *data) {
	iron_mouse_up(button, x, y);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_mouse_up(ui_instances[i], button, x, y);
	}

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _mouse_move(int x, int y, int mx, int my, void *data) {
	iron_mouse_move(x, y, mx, my);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_mouse_move(ui_instances[i], x, y, mx, my);
	}

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _mouse_wheel(int delta, void *data) {
	iron_mouse_wheel(delta);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_mouse_wheel(ui_instances[i], delta);
	}

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _touch_move(int index, int x, int y) {
	iron_touch_move(index, x, y);

	#if defined(IRON_ANDROID) || defined(IRON_IOS)
	for (int i = 0; i < ui_instances_count; ++i) {
		ui_touch_move(ui_instances[i], index, x, y);
	}
	#endif

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

void _touch_down(int index, int x, int y) {
	iron_touch_down(index, x, y);

	#if defined(IRON_ANDROID) || defined(IRON_IOS)
	for (int i = 0; i < ui_instances_count; ++i) {
		ui_touch_down(ui_instances[i], index, x, y);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _touch_up(int index, int x, int y) {
	iron_touch_up(index, x, y);

	#if defined(IRON_ANDROID) || defined(IRON_IOS)
	for (int i = 0; i < ui_instances_count; ++i) {
		ui_touch_up(ui_instances[i], index, x, y);
	}
	#endif

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _pen_down(int x, int y, float pressure) {
	iron_pen_down(x, y, pressure);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_pen_down(ui_instances[i], x, y, pressure);
	}

	#ifdef IDLE_SLEEP
	input_down = true;
	paused_frames = 0;
	#endif
}

void _pen_up(int x, int y, float pressure) {
	iron_pen_up(x, y, pressure);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_pen_up(ui_instances[i], x, y, pressure);
	}

	#ifdef IDLE_SLEEP
	input_down = false;
	paused_frames = 0;
	#endif
}

void _pen_move(int x, int y, float pressure) {
	iron_pen_move(x, y, pressure);

	for (int i = 0; i < ui_instances_count; ++i) {
		ui_pen_move(ui_instances[i], x, y, pressure);
	}

	#ifdef IDLE_SLEEP
	paused_frames = 0;
	#endif
}

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

void _drop_files(wchar_t *file_path, void *data) {
// Update mouse position
#ifdef IRON_WINDOWS
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(iron_windows_window_handle(), &p);
	_mouse_move(p.x, p.y, 0, 0, NULL);
#endif

	char buffer[1024];

#ifdef IRON_WINDOWS
	WideCharToMultiByte(CP_UTF8, 0, file_path, wcslen(file_path) + 1, buffer, sizeof(buffer), NULL, NULL);
#else
	wcstombs(buffer, file_path, sizeof(buffer));
#endif

	iron_drop_files(buffer);
	in_background = false;

#ifdef IDLE_SLEEP
	paused_frames = 0;
#endif
}

char *uri_decode(const char *src) {
	char *res = gc_alloc(1024);
	char *dst = res;
	char a, b;
	while (*src) {
		if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
			if (a >= 'a') {
				a -= 'a' - 'A';
			}
			if (a >= 'A') {
				a -= ('A' - 10);
			}
			else {
				a -= '0';
			}
			if (b >= 'a') {
				b -= 'a' - 'A';
			}
			if (b >= 'A') {
				b -= ('A' - 10);
			}
			else {
				b -= '0';
			}
			*dst++ = 16 * a + b;
			src += 3;
		}
		else if (*src == '+') {
			*dst++ = ' ';
			src++;
		}
		else {
			*dst++ = *src++;
		}
	}
	*dst++ = '\0';
	return res;
}

f32 math_floor(f32 x) { return floorf(x); }
f32 math_cos(f32 x) { return cosf(x); }
f32 math_sin(f32 x) { return sinf(x); }
f32 math_tan(f32 x) { return tanf(x); }
f32 math_sqrt(f32 x) { return sqrtf(x); }
f32 math_abs(f32 x) { return fabsf(x); }
f32 math_random() { return rand() / (float)RAND_MAX; }
f32 math_atan2(f32 y, f32 x) { return atan2f(y, x); }
f32 math_asin(f32 x) { return asinf(x); }
f32 math_pi() { return 3.14159265358979323846; }
f32 math_pow(f32 x, f32 y) { return powf(x, y); }
f32 math_round(f32 x) { return roundf(x); }
f32 math_ceil(f32 x) { return ceilf(x); }
f32 math_min(f32 x, f32 y) { return x < y ? x : y; }
f32 math_max(f32 x, f32 y) { return x > y ? x : y; }
f32 math_log(f32 x) { return logf(x); }
f32 math_log2(f32 x) { return log2f(x); }
f32 math_atan(f32 x) { return atanf(x); }
f32 math_acos(f32 x) { return acosf(x); }
f32 math_exp(f32 x) { return expf(x); }
f32 math_fmod(f32 x, f32 y) { return fmod(x, y); }

#ifdef _WIN32
i32 parse_int(const char *s) { return _strtoi64(s, NULL, 10); }
i32 parse_int_hex(const char *s) { return _strtoi64(s, NULL, 16); }
#else
i32 parse_int(const char *s) { return strtol(s, NULL, 10); }
i32 parse_int_hex(const char *s) { return strtol(s, NULL, 16); }
#endif
f32 parse_float(const char *s) { return strtof(s, NULL); }

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
	ops->visible = enable_window;
	ops->color_bits = 32;
	ops->depth_bits = ops->use_depth ? 24 : 0;
	iron_init(ops->title, ops->width, ops->height, ops);
	iron_random_init((int)(iron_time() * 1000));

	#ifdef IRON_WINDOWS
	// Maximized window has x < -1, prevent window centering
	if (ops->x < -1 && ops->y < -1) {
		iron_window_move(ops->x, ops->y);
	}
	#endif

	#ifdef WITH_AUDIO
	iron_a1_init();
	iron_a2_init();
	#endif

	#ifdef IRON_ANDROID
	android_check_permissions();
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

void iron_set_cut_copy_paste_callback(char *(*on_cut)(void *), char *(*on_copy)(void *), void (*on_paste)(char *, void *)) {
	iron_set_cut_callback(_cut, NULL);
	iron_set_copy_callback(_copy, NULL);
	iron_set_paste_callback(_paste, NULL);
	iron_cut = on_cut;
	iron_copy = on_copy;
	iron_paste = on_paste;
}

void iron_set_application_state_callback(void (*on_foreground)(void), void (*on_resume)(void), void (*on_pause)(void), void (*on_background)(void), void (*on_shutdown)(void)) {
	iron_set_foreground_callback(on_foreground != NULL ? _foreground : NULL, NULL);
	iron_set_resume_callback(on_resume != NULL ? _resume : NULL, NULL);
	iron_set_pause_callback(on_pause != NULL ? _pause : NULL, NULL);
	iron_set_background_callback(on_background != NULL ? _background : NULL, NULL);
	iron_set_shutdown_callback(on_shutdown != NULL ? _shutdown : NULL, NULL);
	iron_foreground = on_foreground;
	iron_resume = on_resume;
	iron_pause = on_pause;
	iron_background = on_background;
	iron_shutdown = on_shutdown;
}

void iron_set_keyboard_down_callback(void (*callback)(int)) {
	iron_key_down = callback;
	iron_keyboard_set_key_down_callback(_key_down, NULL);
}

void iron_set_keyboard_up_callback(void (*callback)(int)) {
	iron_key_up = callback;
	iron_keyboard_set_key_up_callback(_key_up, NULL);
}

void iron_set_keyboard_press_callback(void (*callback)(int)) {
	iron_key_press = callback;
	iron_keyboard_set_key_press_callback(_key_press, NULL);
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

void iron_set_gamepad_axis_callback(void (*callback)(int, int, float)) {
	iron_gamepad_axis = callback;
	iron_gamepad_set_axis_callback(_gamepad_axis, NULL);
}

void iron_set_gamepad_button_callback(void (*callback)(int, int, float)) {
	iron_gamepad_button = callback;
	iron_gamepad_set_button_callback(_gamepad_button, NULL);
}

void iron_show_mouse(bool show) {
	show ? iron_mouse_show() : iron_mouse_hide();
}

void iron_show_keyboard(bool show) {
	show ? iron_keyboard_show() : iron_keyboard_hide();
}

any gpu_create_index_buffer(i32 count) {
	gpu_buffer_t *buffer = (gpu_buffer_t *)malloc(sizeof(gpu_buffer_t));
	gpu_index_buffer_init(buffer, count);
	return buffer;
}

void gpu_delete_index_buffer(gpu_buffer_t *buffer) {
	gpu_index_buffer_destroy(buffer);
	free(buffer);
}

u32_array_t *gpu_lock_index_buffer(gpu_buffer_t *buffer) {
	void *vertices = gpu_index_buffer_lock(buffer);
	u32_array_t *ar = (u32_array_t *)malloc(sizeof(u32_array_t));
	ar->buffer = vertices;
	ar->length = gpu_index_buffer_count(buffer);
	return ar;
}

any gpu_create_vertex_buffer(i32 count, gpu_vertex_structure_t *structure, i32 usage) {
	gpu_buffer_t *buffer = (gpu_buffer_t *)malloc(sizeof(gpu_buffer_t));
	gpu_vertex_buffer_init(buffer, count, structure);
	return buffer;
}

void gpu_delete_vertex_buffer(gpu_buffer_t *buffer) {
	gpu_vertex_buffer_destroy(buffer);
	free(buffer);
}

buffer_t *gpu_lock_vertex_buffer(gpu_buffer_t *buffer) {
	float *vertices = gpu_vertex_buffer_lock(buffer);
	buffer_t *b = (buffer_t *)malloc(sizeof(buffer_t));
	b->buffer = vertices;
	b->length = buffer->count * gpu_vertex_buffer_stride(buffer);
	return b;
}

gpu_shader_t *gpu_create_shader(buffer_t *data, i32 shader_type) {
	gpu_shader_t *shader = (gpu_shader_t *)malloc(sizeof(gpu_shader_t));
	gpu_shader_init(shader, data->buffer, data->length, (gpu_shader_type_t)shader_type);
	return shader;
}

#ifdef WITH_KONG

#include "../../sources/libs/kong/analyzer.h"
#include "../../sources/libs/kong/compiler.h"
#include "../../sources/libs/kong/disasm.h"
#include "../../sources/libs/kong/errors.h"
#include "../../sources/libs/kong/functions.h"
#include "../../sources/libs/kong/globals.h"
#include "../../sources/libs/kong/log.h"
#include "../../sources/libs/kong/names.h"
#include "../../sources/libs/kong/parser.h"
#include "../../sources/libs/kong/tokenizer.h"
#include "../../sources/libs/kong/typer.h"
#include "../../sources/libs/kong/types.h"
#include "../../sources/libs/kong/transformer.h"
#include "../../sources/libs/kong/backends/hlsl.h"
#include "../../sources/libs/kong/backends/metal.h"
#include "../../sources/libs/kong/backends/spirv.h"
#include "../../sources/libs/kong/backends/wgsl.h"

extern uint64_t next_variable_id;
extern size_t allocated_globals_size;
extern function_id next_function_index;
extern global_id globals_size;
extern name_id names_index;
extern size_t sets_count;
extern type_id next_type_index;
void hlsl_export2(char **vs, char **fs, api_kind d3d, bool debug);
void spirv_export2(char **vs, char **fs, int *vs_size, int *fs_size, bool debug);
extern size_t vertex_inputs_size;
extern size_t fragment_inputs_size;
extern size_t vertex_functions_size;
extern size_t fragment_functions_size;

void gpu_create_shaders_from_kong(char *kong, char **vs, char **fs, int *vs_size, int *fs_size) {
	next_variable_id = 1;
	allocated_globals_size = 0;
	next_function_index = 0;
	globals_size = 0;
	names_index = 1;
	sets_count = 0;
	next_type_index = 0;
	vertex_inputs_size = 0;
	fragment_inputs_size = 0;
	vertex_functions_size = 0;
	fragment_functions_size = 0;
	names_init();
	types_init();
	functions_init();
	globals_init();
	char *from = "";
	tokens tokens = tokenize(from, kong);
	parse(from, &tokens);
	resolve_types();
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

	#else

	transform(TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE | TRANSFORM_FLAG_BINARY_UNIFY_LENGTH);
	spirv_export2(vs, fs, vs_size, fs_size, false);

	#endif
}

#endif

gpu_shader_t *gpu_create_shader_from_source(string_t *source, int source_size, gpu_shader_type_t shader_type) {
	gpu_shader_t *shader = (gpu_shader_t *)malloc(sizeof(gpu_shader_t));
	char *temp_string_s = shader_type == GPU_SHADER_TYPE_VERTEX ? temp_string_vs : temp_string_fs;

	#ifdef WITH_D3DCOMPILER

	strcpy(temp_string_s, source);

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;
	HRESULT hr = D3DCompile(temp_string_s, strlen(source) + 1, NULL, NULL, NULL, "main",
		shader_type == GPU_SHADER_TYPE_VERTEX ? "vs_5_0" : "ps_5_0", flags, 0, &shader_buffer, &error_message);
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

void gpu_compile_pipeline(gpu_pipeline_t *pipeline) {
	gpu_pipeline_compile(pipeline);
}

bool _load_image(iron_file_reader_t *reader, const char *filename, unsigned char **output, int *width, int *height, iron_image_format_t *format) {
	*format = IRON_IMAGE_FORMAT_RGBA32;
	int size = (int)iron_file_reader_size(reader);
	bool success = true;
	unsigned char *data = (unsigned char *)malloc(size);
	iron_file_reader_read(reader, data, size);
	iron_file_reader_close(reader);

	if (ends_with(filename, "k")) {
		*width = iron_read_s32le(data);
		*height = iron_read_s32le(data + 4);
		char fourcc[5];
		fourcc[0] = data[8];
		fourcc[1] = data[9];
		fourcc[2] = data[10];
		fourcc[3] = data[11];
		fourcc[4] = 0;
		int compressed_size = size - 12;
		if (strcmp(fourcc, "LZ4 ") == 0) {
			int output_size = *width * *height * 4;
			*output = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)(data + 12), (char *)*output, compressed_size, output_size);
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			int output_size = *width * *height * 16;
			*output = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)(data + 12), (char *)*output, compressed_size, output_size);
			*format = IRON_IMAGE_FORMAT_RGBA128;

			#ifdef IRON_IOS // No RGBA128 filtering, convert to RGBA64
			uint32_t *_output32 = (uint32_t *)*output;
			unsigned char *_output = (unsigned char *)malloc(output_size / 2);
			uint16_t *_output16 = (uint16_t *)_output;
			for (int i = 0; i < output_size / 4; ++i) {
				uint32_t x = *((uint32_t *)&_output32[i]);
				_output16[i] = ((x >> 16) & 0x8000) | ((((x & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((x >> 13) & 0x03ff);
			}
			*format = IRON_IMAGE_FORMAT_RGBA64;
			free(*output);
			*output = _output;
			#endif
		}
		else {
			success = false;
		}
	}
	else if (ends_with(filename, "hdr")) {
		int comp;
		*output = (unsigned char *)stbi_loadf_from_memory(data, size, width, height, &comp, 4);
		if (*output == NULL) {
			iron_error(stbi_failure_reason());
			success = false;
		}
		*format = IRON_IMAGE_FORMAT_RGBA128;
	}
	else { // jpg, png, ..
		int comp;
		*output = stbi_load_from_memory(data, size, width, height, &comp, 4);
		if (*output == NULL) {
			iron_error(stbi_failure_reason());
			success = false;
		}
	}
	free(data);
	return success;
}

gpu_texture_t *gpu_create_texture_from_encoded_bytes(buffer_t *data, string_t *format, bool readable);

gpu_texture_t *iron_load_image(string_t *file, bool readable) {
	#ifdef WITH_EMBED
	buffer_t *b = embed_get(file);
	if (b != NULL) {
		gpu_texture_t *texture = gpu_create_texture_from_encoded_bytes(b, ".k", readable);
		return texture;
	}
	#endif

	iron_file_reader_t reader;
	if (!iron_file_reader_open(&reader, file, IRON_FILE_TYPE_ASSET)) {
		return NULL;
	}

	unsigned char *image_data;
	int image_width;
	int image_height;
	iron_image_format_t image_format;
	if (!_load_image(&reader, file, &image_data, &image_width, &image_height, &image_format)) {
		return NULL;
	}

	gpu_texture_t *texture = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	gpu_texture_init_from_bytes(texture, image_data, image_width, image_height, image_format);
	if (!readable) {
		free(image_data);
	}

	return texture;
}

void iron_unload_image(gpu_texture_t *image) {
	if (image != NULL) {
		gpu_texture_destroy(image);
		// free(image);
	}
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
	uint32_t reader_size = (uint32_t)iron_file_reader_size(&reader);
	buffer_t *buffer = buffer_create(reader_size);
	iron_file_reader_read(&reader, buffer->buffer, reader_size);
	iron_file_reader_close(&reader);
	return buffer;
}

void iron_set_window_title(string_t *title) {
	iron_window_set_title(title);
	#if defined(IRON_IOS) || defined(IRON_ANDROID)
	strcpy(mobile_title, title);
	#endif
}

void iron_set_window_mode(i32 mode) {
	iron_window_change_mode((iron_window_mode_t)mode);
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

gpu_texture_t *gpu_create_render_target(i32 width, i32 height, i32 format, i32 depth_buffer_bits) {
	gpu_texture_t *render_target = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	gpu_render_target_init(render_target, width, height, (iron_image_format_t)format, depth_buffer_bits);
	render_target->buffer = NULL;
	return render_target;
}

gpu_texture_t *gpu_create_texture_from_bytes(buffer_t *data, i32 width, i32 height, i32 format, bool readable) {
	gpu_texture_t *texture = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	texture->buffer = NULL;
	void *image_data;
	if (readable) {
		image_data = malloc(data->length);
		memcpy(image_data, data->buffer, data->length);
	}
	else {
		image_data = data->buffer;
	}
	gpu_texture_init_from_bytes(texture, image_data, width, height, (iron_image_format_t)format);
	return texture;
}

gpu_texture_t *gpu_create_texture_from_encoded_bytes(buffer_t *data, string_t *format, bool readable) {
	gpu_texture_t *texture = (gpu_texture_t *)malloc(sizeof(gpu_texture_t));
	texture->buffer = NULL;

	unsigned char *content_data = (unsigned char *)data->buffer;
	int content_length = (int)data->length;
	unsigned char *image_data;
	iron_image_format_t image_format;
	int image_width;
	int image_height;

	if (ends_with(format, "k")) {
		image_width = iron_read_s32le(content_data);
		image_height = iron_read_s32le(content_data + 4);
		char fourcc[5];
		fourcc[0] = content_data[8];
		fourcc[1] = content_data[9];
		fourcc[2] = content_data[10];
		fourcc[3] = content_data[11];
		fourcc[4] = 0;
		int compressed_size = content_length - 12;
		if (strcmp(fourcc, "LZ4 ") == 0) {
			int output_size = image_width * image_height * 4;
			image_data = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)content_data + 12, (char *)image_data, compressed_size, output_size);
			image_format = IRON_IMAGE_FORMAT_RGBA32;
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			int output_size = image_width * image_height * 16;
			image_data = (unsigned char *)malloc(output_size);
			LZ4_decompress_safe((char *)content_data + 12, (char *)image_data, compressed_size, output_size);
			image_format = IRON_IMAGE_FORMAT_RGBA128;
		}
	}
	else if (ends_with(format, "hdr")) {
		int comp;
		image_data = (unsigned char *)stbi_loadf_from_memory(content_data, content_length, &image_width, &image_height, &comp, 4);
		image_format = IRON_IMAGE_FORMAT_RGBA128;
	}
	else { // jpg, png, ..
		int comp;
		image_data = stbi_load_from_memory(content_data, content_length, &image_width, &image_height, &comp, 4);
		image_format = IRON_IMAGE_FORMAT_RGBA32;
	}

	gpu_texture_init_from_bytes(texture, image_data, image_width, image_height, image_format);
	if (!readable) {
		free(image_data);;
	}

	return texture;
}

int _format_byte_size(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return 16;
	case IRON_IMAGE_FORMAT_RGBA64:
		return 8;
	case IRON_IMAGE_FORMAT_R8:
		return 1;
	case IRON_IMAGE_FORMAT_R16:
		return 2;
	case IRON_IMAGE_FORMAT_RGBA32:
	case IRON_IMAGE_FORMAT_R32:
	default:
		return 4;
	}
}

buffer_t *gpu_get_texture_pixels(gpu_texture_t *image) {
	if (image->buffer == NULL) {
		image->buffer = malloc(sizeof(buffer_t));
		image->buffer->buffer = NULL;
	}
	image->buffer->length = _format_byte_size(image->format) * image->width * image->height;

	if (image->data == NULL) {
		if (image->buffer->buffer == NULL) {
			image->buffer->buffer = malloc(image->buffer->length);
		}

		uint8_t *b = (uint8_t *)image->buffer->buffer;
		gpu_get_render_target_pixels(image, b);

		// Release staging texture immediately to save memory
		#ifdef IRON_DIRECT3D12
		image->impl.readback->lpVtbl->Release(image->impl.readback);
		image->impl.readback = NULL;
		#elif defined(IRON_METAL)
		// id<MTLTexture> texReadback = (__bridge_transfer id<MTLTexture>)image->impl._texReadback;
		// texReadback = nil;
		// image->impl._texReadback = NULL;
		#endif
	}
	else {
		image->buffer->buffer = image->data;
	}
	return image->buffer;
}

void gpu_set_mipmaps(gpu_texture_t *texture, any_array_t *mipmaps) {
	for (int32_t i = 0; i < mipmaps->length; ++i) {
		gpu_texture_t *img = mipmaps->buffer[i];
		gpu_texture_t *img_tex = img;
		gpu_texture_set_mipmap(texture, img_tex, i + 1);
	}
}

void _gpu_begin(gpu_texture_t *render_target, any_array_t *additional, unsigned flags, unsigned color, float depth) {
	if (render_target == NULL) {
		gpu_begin(NULL, 0, flags, color, depth);
	}
	else {
		int32_t length = 1;
		gpu_texture_t *render_targets[8] = { render_target, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		if (additional != NULL) {
			length = additional->length + 1;
			for (int32_t i = 1; i < length; ++i) {
				render_targets[i] = additional->buffer[i - 1];
			}
		}
		gpu_begin(render_targets, length, flags, color, depth);
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
	int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd, -1, NULL, 0);
	wchar_t *wstr = malloc(sizeof(wchar_t) * wlen);
	MultiByteToWideChar(CP_UTF8, 0, cmd, -1, wstr, wlen);
	int result = _wsystem(wstr);
	free(wstr);
	#elif defined(IRON_IOS)
	int result = 0;
	#else
	int result = system(cmd);
	#endif
	return result;
}

string_t *iron_get_files_location() {
	#ifdef IRON_MACOS
	char path[1024];
	strcpy(path, macgetresourcepath());
	strcat(path, "/");
	strcat(path, IRON_OUTDIR);
	strcat(path, "/");
	return path;
	#elif defined(IRON_IOS)
	char path[1024];
	strcpy(path, iphonegetresourcepath());
	strcat(path, "/");
	strcat(path, IRON_OUTDIR);
	strcat(path, "/");
	return path;
	#else
	return iron_internal_get_files_location();
	#endif
}

typedef struct _callback_data {
	int32_t size;
	char url[512];
	void (*func)(char *, buffer_t *);
} _callback_data_t;

void _http_callback(int error, int response, const char *body, void *callback_data) {
	_callback_data_t *cbd = (_callback_data_t *)callback_data;
	buffer_t *buffer = NULL;
	if (body != NULL) {
		buffer = malloc(sizeof(buffer_t));
		buffer->length = cbd->size > 0 ? cbd->size : strlen(body);
		buffer->buffer = body;
	}
	cbd->func(cbd->url, buffer);
	free(cbd);
}

void _iron_http_request(string_t *url, i32 size, void (*callback)(char *, buffer_t *)) {
	_callback_data_t *cbd = malloc(sizeof(_callback_data_t));
	cbd->size = size;
	strcpy(cbd->url, url);
	cbd->func = callback;

	char url_base[512];
	char url_path[512];
	const char *curl = url;
	int i = 0;
	for (; i < strlen(curl) - 8; ++i) {
		if (curl[i + 8] == '/') {
			break;
		}
		url_base[i] = curl[i + 8]; // Strip https://
	}
	url_base[i] = 0;
	int j = 0;
	if (strlen(url_base) < strlen(curl) - 8) {
		++i; // Skip /
	}
	for (; j < strlen(curl) - 8 - i; ++j) {
		if (curl[i + 8 + j] == 0) {
			break;
		}
		url_path[j] = curl[i + 8 + j];
	}
	url_path[j] = 0;
	#ifdef IRON_ANDROID // TODO: move to iron
	android_http_request(curl, url_path, NULL, 443, true, 0, NULL, &_http_callback, cbd);
	#elif defined(IRON_LINUX)
	// TODO
	#else
	iron_http_request(url_base, url_path, NULL, 443, true, 0, NULL, &_http_callback, cbd);
	#endif
}

bool _window_close_callback(void *data) {
	#ifdef IRON_WINDOWS
	bool save = false;
	wchar_t title[1024];
	GetWindowTextW(iron_windows_window_handle(), title, sizeof(title));
	bool dirty = wcsstr(title, L"* - ArmorPaint") != NULL;
	if (dirty) {
		int res = MessageBox(iron_windows_window_handle(), L"Project has been modified, save changes?", L"Save Changes?", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
		if (res == IDYES) {
			save = true;
		}
		else if (res == IDNO) {
			save = false;
		}
		else { // Cancel
			return false;
		}
	}
	if (save_and_quit_callback_set) {
		iron_save_and_quit(save);
		return false;
	}
	#endif
	return true;
}

void iron_set_save_and_quit_callback(void (*callback)(bool)) {
	iron_save_and_quit = callback;
	save_and_quit_callback_set = true;
	iron_window_set_close_callback(_window_close_callback, NULL);
}

void iron_set_mouse_cursor(i32 id) {
	iron_mouse_set_cursor(id);
	#ifdef IRON_WINDOWS
	// Set hand icon for drag even when mouse button is pressed
	if (id == 1) {
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}
	#endif
}

void iron_delay_idle_sleep() {
	paused_frames = 0;
}

#ifdef WITH_NFD
char_ptr_array_t *iron_open_dialog(char *filter_list, char *default_path, bool open_multiple) {
	nfdpathset_t out_paths;
	nfdchar_t* out_path;
	nfdresult_t result = open_multiple ? NFD_OpenDialogMultiple(filter_list, default_path, &out_paths) : NFD_OpenDialog(filter_list, default_path, &out_path);

	if (result == NFD_OKAY) {
		int path_count = open_multiple ? (int)NFD_PathSet_GetCount(&out_paths) : 1;
		char_ptr_array_t *result = any_array_create(path_count);

		if (open_multiple) {
			for (int i = 0; i < path_count; ++i) {
				nfdchar_t* out_path = NFD_PathSet_GetPath(&out_paths, i);
				result->buffer[i] = out_path;
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
	nfdchar_t *out_path = NULL;
	nfdresult_t result = NFD_SaveDialog(filter_list, default_path, &out_path);
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
	files[0] = 0;

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
	unsigned char *inflated;
	int inflated_len = bytes->length * 2;
	int out_len = -1;
	while (out_len == -1) {
		inflated_len *= 2;
		inflated = (unsigned char *)realloc(inflated, inflated_len);
		out_len = sinflate(inflated, inflated_len, bytes->buffer, bytes->length);
	}
	buffer_t *output = buffer_create(0);
	output->buffer = inflated;
	output->length = out_len;
	return output;
}

buffer_t *iron_deflate(buffer_t *bytes, bool raw) {
	struct sdefl sdefl;
	memset(&sdefl, 0, sizeof(sdefl));
	void *deflated = malloc(sdefl_bound(bytes->length));
	// raw == sdeflate
	int out_len = zsdeflate(&sdefl, deflated, bytes->buffer, bytes->length, SDEFL_LVL_MIN);
	buffer_t *output = buffer_create(0);
	output->buffer = deflated;
	output->length = out_len;
	return output;
}

unsigned char *iron_deflate_raw(unsigned char *data, int data_len, int *out_len, int quality) {
	struct sdefl sdefl;
	memset(&sdefl, 0, sizeof(sdefl));
	void *deflated = malloc(sdefl_bound(data_len));
	*out_len = zsdeflate(&sdefl, deflated, data, data_len, SDEFL_LVL_MIN);
	return (unsigned char *)deflated;
}
#endif

#ifdef WITH_IMAGE_WRITE
void _write_image(char *path, buffer_t *bytes, i32 w, i32 h, i32 format, int image_format, int quality) {
	int comp = 0;
	unsigned char *pixels = NULL;
	unsigned char *rgba = (unsigned char *)bytes->buffer;
	if (format == 0) { // RGBA
		comp = 4;
		pixels = rgba;
	}
	else if (format == 1) { // R
		comp = 1;
		pixels = rgba;
	}
	else if (format == 2) { // RGB1
		comp = 3;
		pixels = (unsigned char *)malloc(w * h * comp);
		for (int i = 0; i < w * h; ++i) {
			#if defined(IRON_METAL) || defined(IRON_VULKAN)
			pixels[i * 3    ] = rgba[i * 4 + 2];
			pixels[i * 3 + 1] = rgba[i * 4 + 1];
			pixels[i * 3 + 2] = rgba[i * 4    ];
			#else
			pixels[i * 3    ] = rgba[i * 4    ];
			pixels[i * 3 + 1] = rgba[i * 4 + 1];
			pixels[i * 3 + 2] = rgba[i * 4 + 2];
			#endif
		}
	}
	else if (format > 2) { // RRR1, GGG1, BBB1, AAA1
		comp = 1;
		pixels = (unsigned char *)malloc(w * h * comp);
		int off = format - 3;
		#if defined(IRON_METAL) || defined(IRON_VULKAN)
		off = 2 - off;
		#endif
		for (int i = 0; i < w * h; ++i) {
			pixels[i] = rgba[i * 4 + off];
		}
	}

	image_format == 0 ?
		stbi_write_jpg(path, w, h, comp, pixels, quality) :
		stbi_write_png(path, w, h, comp, pixels, w * comp);

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
int _encode_size;
void _encode_image_func(void *context, void *data, int size) {
	memcpy(_encode_data + _encode_size, data, size);
	_encode_size += size;
}

buffer_t *_encode_image(buffer_t *bytes, i32 w, i32 h, i32 format, i32 quality) {
	_encode_data = (unsigned char *)malloc(w * h * 4);
	_encode_size = 0;
	format == 0 ?
		stbi_write_jpg_to_func(&_encode_image_func, NULL, w, h, 4, bytes->buffer, quality) :
		stbi_write_png_to_func(&_encode_image_func, NULL, w, h, 4, bytes->buffer, w * 4);
	buffer_t *buffer = malloc(sizeof(buffer_t));
	buffer->buffer = _encode_data;
	buffer->length = _encode_size;
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

static FILE *iron_mp4_fp;
static int iron_mp4_w;
static int iron_mp4_h;
static int iron_mp4_stride;
static H264E_persist_t *iron_mp4_enc = NULL;
static H264E_scratch_t *iron_mp4_scratch = NULL;
static char iron_mp4_path[512];
static char iron_mp4_path_264[512];
static char *iron_mp4_yuv_buf;

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
	int len = strlen(iron_mp4_path_264);
	iron_mp4_path_264[len - 1] = '4';
	iron_mp4_path_264[len - 2] = '6';
	iron_mp4_path_264[len - 3] = '2';

	iron_mp4_stride = w;
	iron_mp4_w = w - w % 16;
	iron_mp4_h = h - h % 16;

	H264E_create_param_t create_param = {0};
	create_param.width = iron_mp4_w;
	create_param.height = iron_mp4_h;
	int sizeof_persist = 0;
	int sizeof_scratch = 0;
	H264E_sizeof(&create_param, &sizeof_persist, &sizeof_scratch);

	iron_mp4_enc = (H264E_persist_t *)malloc(sizeof_persist);
    iron_mp4_scratch = (H264E_scratch_t *)malloc(sizeof_scratch);
    H264E_init(iron_mp4_enc, &create_param);

	iron_mp4_fp = fopen(iron_mp4_path_264, "wb");
	int frame_size = (int)(iron_mp4_w * iron_mp4_h * 1.5);
	iron_mp4_yuv_buf = malloc(frame_size);
}

void iron_mp4_end() {
	if (iron_mp4_fp == NULL) {
		return;
	}

	buffer_t *blob = iron_load_blob(iron_mp4_path_264);
	uint8_t *buf = blob->buffer;
	size_t buf_size = blob->length;
	FILE *fout = fopen(iron_mp4_path, "wb");
	MP4E_mux_t *mux = MP4E_open(0, 0, fout, iron_mp4_write_callback);
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
			int k = i + j * iron_mp4_stride;
			uint8_t r = pixels->buffer[k * 4];
			uint8_t g = pixels->buffer[k * 4 + 1];
			uint8_t b = pixels->buffer[k * 4 + 2];
			uint8_t y = (( 66 * r + 129 * g +  25 * b + 128) / 256) +  16;
			uint8_t u = ((-38 * r -  74 * g + 112 * b + 128) / 256) + 128;
			uint8_t v = ((112 * r -  94 * g -  18 * b + 128) / 256) + 128;
			int l = i + j * iron_mp4_w;
			int m = i / 2 + j / 2 * (iron_mp4_w / 2);
			iron_mp4_yuv_buf[l] = y;
			iron_mp4_yuv_buf[iron_mp4_w * iron_mp4_h + m] = u;
			iron_mp4_yuv_buf[iron_mp4_w * iron_mp4_h + (iron_mp4_w * iron_mp4_h) / 4 + m] = v;
		}
	}

	H264E_run_param_t run_param = {0};
	run_param.frame_type = 0;
    run_param.encode_speed = H264E_SPEED_SLOWEST; // H264E_SPEED_FASTEST;
	run_param.desired_frame_bytes = (2048 * 4) * 1000 / 8 / 30; // 2048 * 4 kbps
	run_param.qp_min = 10;
	run_param.qp_max = 50;

	H264E_io_yuv_t yuv;
	yuv.yuv[0] = iron_mp4_yuv_buf;
	yuv.stride[0] = iron_mp4_w;
    yuv.yuv[1] = iron_mp4_yuv_buf + iron_mp4_w * iron_mp4_h;
	yuv.stride[1] = iron_mp4_w / 2;
    yuv.yuv[2] = iron_mp4_yuv_buf + (int)(iron_mp4_w * iron_mp4_h * 1.25);
	yuv.stride[2] = iron_mp4_w / 2;

	uint8_t *coded_data;
	int sizeof_coded_data;
	H264E_encode(iron_mp4_enc, iron_mp4_scratch, &run_param, &yuv, &coded_data, &sizeof_coded_data);
	fwrite(coded_data, sizeof_coded_data, 1, iron_mp4_fp);
}
#endif

#ifdef WITH_ONNX
buffer_t *iron_ml_inference(buffer_t *model, any_array_t *tensors, any_array_t *input_shape, i32_array_t *output_shape, bool use_gpu) {
	OrtStatus *onnx_status = NULL;
	static bool use_gpu_last = false;
	if (ort == NULL || use_gpu_last != use_gpu) {
		use_gpu_last = use_gpu;
		ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
		ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "iron", &ort_env);

		ort->CreateSessionOptions(&ort_session_options);
		ort->SetIntraOpNumThreads(ort_session_options, 8);
		ort->SetInterOpNumThreads(ort_session_options, 8);

		if (use_gpu) {
			#ifdef IRON_WINDOWS
			ort->SetSessionExecutionMode(ort_session_options, ORT_SEQUENTIAL);
			ort->DisableMemPattern(ort_session_options);
			onnx_status = OrtSessionOptionsAppendExecutionProvider_DML(ort_session_options, 0);
			#elif defined(IRON_LINUX)
			// onnx_status = OrtSessionOptionsAppendExecutionProvider_CUDA(ort_session_options, 0);
			#elif defined(IRON_MACOS)
			onnx_status = OrtSessionOptionsAppendExecutionProvider_CoreML(ort_session_options, 0);
			#endif
			if (onnx_status != NULL) {
				const char *msg = ort->GetErrorMessage(onnx_status);
				iron_error("%s", msg);
				ort->ReleaseStatus(onnx_status);
			}
		}
	}

	static void *content_last = 0;
	if (content_last != model->buffer || session == NULL) {
		if (session != NULL) {
			ort->ReleaseSession(session);
			session = NULL;
		}
		onnx_status = ort->CreateSessionFromArray(ort_env, model->buffer, (int)model->length, ort_session_options, &session);
		if (onnx_status != NULL) {
			const char* msg = ort->GetErrorMessage(onnx_status);
			iron_error("%s", msg);
			ort->ReleaseStatus(onnx_status);
		}
	}
	content_last = model->buffer;

	OrtAllocator *allocator;
	ort->GetAllocatorWithDefaultOptions(&allocator);
	OrtMemoryInfo *memory_info;
	ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);

	int32_t length = tensors->length;
	if (length > 4) {
		length = 4;
	}
	char *input_node_names[4];
	OrtValue *input_tensors[4];
	for (int32_t i = 0; i < length; ++i) {
		ort->SessionGetInputName(session, i, allocator, &input_node_names[i]);

		OrtTypeInfo *input_type_info;
		ort->SessionGetInputTypeInfo(session, i, &input_type_info);
		const OrtTensorTypeAndShapeInfo *input_tensor_info;
		ort->CastTypeInfoToTensorInfo(input_type_info, &input_tensor_info);
		size_t num_input_dims;
		ort->GetDimensionsCount(input_tensor_info, &num_input_dims);
		int64_t input_node_dims[32];

		if (input_shape != NULL) {
			for (int32_t j = 0; j < num_input_dims; ++j) {
				i32_array_t *a = input_shape->buffer[i];
				input_node_dims[j] = a->buffer[j];
			}
		}
		else {
			ort->GetDimensions(input_tensor_info, (int64_t *)input_node_dims, num_input_dims);
		}
		ONNXTensorElementDataType tensor_element_type;
		ort->GetTensorElementType(input_tensor_info, &tensor_element_type);

		buffer_t *b = tensors->buffer[i];
		ort->CreateTensorWithDataAsOrtValue(memory_info, b->buffer, (int)b->length, input_node_dims, num_input_dims,  tensor_element_type, &input_tensors[i]);
		ort->ReleaseTypeInfo(input_type_info);
	}

	char *output_node_name;
	ort->SessionGetOutputName(session, 0, allocator, &output_node_name);
	OrtValue *output_tensor = NULL;
	onnx_status = ort->Run(session, NULL, input_node_names, input_tensors, length, &output_node_name, 1, &output_tensor);
	if (onnx_status != NULL) {
		const char* msg = ort->GetErrorMessage(onnx_status);
		iron_error("%s", msg);
		ort->ReleaseStatus(onnx_status);
	}
	float *float_array;
	ort->GetTensorMutableData(output_tensor, (void **)&float_array);

	size_t output_byte_length = 4;
	if (output_shape != NULL) {
		int32_t length = output_shape->length;
		for (int i = 0; i < length; ++i) {
			output_byte_length *= output_shape->buffer[i];
		}
	}
	else {
		OrtTypeInfo *output_type_info;
		ort->SessionGetOutputTypeInfo(session, 0, &output_type_info);
		const OrtTensorTypeAndShapeInfo *output_tensor_info;
		ort->CastTypeInfoToTensorInfo(output_type_info, &output_tensor_info);
		size_t num_output_dims;
		ort->GetDimensionsCount(output_tensor_info, &num_output_dims);
		int64_t output_node_dims[32];
		ort->GetDimensions(output_tensor_info, (int64_t *)output_node_dims, num_output_dims);
		ort->ReleaseTypeInfo(output_type_info);
		for (int i = 0; i < num_output_dims; ++i) {
			if (output_node_dims[i] > 1) {
				output_byte_length *= output_node_dims[i];
			}
		}
	}

	buffer_t *output = buffer_create(output_byte_length);
	memcpy(output->buffer, float_array, output_byte_length);

	ort->ReleaseMemoryInfo(memory_info);
	ort->ReleaseValue(output_tensor);
	for (int i = 0; i < length; ++i) {
		ort->ReleaseValue(input_tensors[i]);
	}
	return output;
}

void iron_ml_unload() {
	if (session != NULL) {
		ort->ReleaseSession(session);
		session = NULL;
	}
}
#endif

void iron_raytrace_init(buffer_t *shader) {
	if (raytrace_created) {
		gpu_constant_buffer_destroy(&constant_buffer);
		gpu_raytrace_pipeline_destroy(&rt_pipeline);
	}
	raytrace_created = true;
	gpu_constant_buffer_init(&constant_buffer, constant_buffer_size * 4);
	gpu_raytrace_pipeline_init(&rt_pipeline, shader->buffer, (int)shader->length, &constant_buffer);
}

void iron_raytrace_as_init() {
	if (raytrace_accel_created) {
		gpu_raytrace_acceleration_structure_destroy(&accel);
	}
	raytrace_accel_created = true;
	gpu_raytrace_acceleration_structure_init(&accel);
}

void iron_raytrace_as_add(struct gpu_buffer *vb, gpu_buffer_t *ib, iron_matrix4x4_t transform) {
	gpu_raytrace_acceleration_structure_add(&accel, vb, ib, transform);
}

void iron_raytrace_as_build(struct gpu_buffer *vb_full, gpu_buffer_t *ib_full) {
	gpu_raytrace_acceleration_structure_build(&accel, vb_full, ib_full);
}

void iron_raytrace_set_textures(gpu_texture_t *tex0, gpu_texture_t *tex1, gpu_texture_t *tex2, gpu_texture_t *texenv, gpu_texture_t *texsobol, gpu_texture_t *texscramble, gpu_texture_t *texrank) {
	if (!texenv->uploaded) {
		gpu_upload_texture(texenv);
		texenv->uploaded = true;
	}
	if (!texsobol->uploaded) {
		gpu_upload_texture(texsobol);
		texsobol->uploaded = true;
	}
	if (!texscramble->uploaded) {
		gpu_upload_texture(texscramble);
		texscramble->uploaded = true;
	}
	if (!texrank->uploaded) {
		gpu_upload_texture(texrank);
		texrank->uploaded = true;
	}

	gpu_raytrace_set_textures(tex0, tex1, tex2, texenv, texsobol, texscramble, texrank);
}

void iron_raytrace_dispatch_rays(gpu_texture_t *render_target, buffer_t *buffer) {
	float *cb = (float *)buffer->buffer;
	gpu_constant_buffer_lock(&constant_buffer, 0, gpu_constant_buffer_size(&constant_buffer));
	for (int i = 0; i < constant_buffer_size; ++i) {
		float *floats = (float *)(&constant_buffer.data[i * 4]);
		floats[0] = cb[i];
	}
	gpu_constant_buffer_unlock(&constant_buffer);

	gpu_raytrace_set_acceleration_structure(&accel);
	gpu_raytrace_set_pipeline(&rt_pipeline);
	gpu_raytrace_set_target(render_target);
	gpu_raytrace_dispatch_rays();
}

#endif
