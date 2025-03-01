#pragma once

#include <iron_global.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

void kinc_log(const char *format, ...);
void kinc_error(const char *format, ...);

typedef struct kinc_display_mode {
	int x;
	int y;
	int width;
	int height;
	int pixels_per_inch;
	int frequency;
	int bits_per_pixel;
} kinc_display_mode_t;

void kinc_display_init(void);
int kinc_primary_display(void);
int kinc_count_displays(void);
bool kinc_display_available(int display_index);
const char *kinc_display_name(int display_index);
kinc_display_mode_t kinc_display_current_mode(int display_index);
int kinc_display_count_available_modes(int display_index);
kinc_display_mode_t kinc_display_available_mode(int display_index, int mode_index);

typedef struct kinc_framebuffer_options {
	int frequency;
	bool vertical_sync;
	int color_bits;
	int depth_bits;
} kinc_framebuffer_options_t;

typedef enum {
	KINC_WINDOW_MODE_WINDOW,
	KINC_WINDOW_MODE_FULLSCREEN
} kinc_window_mode_t;

#define KINC_WINDOW_FEATURE_RESIZEABLE 1
#define KINC_WINDOW_FEATURE_MINIMIZABLE 2
#define KINC_WINDOW_FEATURE_MAXIMIZABLE 4
#define KINC_WINDOW_FEATURE_BORDERLESS 8
#define KINC_WINDOW_FEATURE_ON_TOP 16

typedef struct kinc_window_options {
	const char *title;
	int x;
	int y;
	int width;
	int height;
	int display_index;
	bool visible;
	int window_features;
	kinc_window_mode_t mode;
} kinc_window_options_t;

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame);
void kinc_window_destroy();
void kinc_window_options_set_defaults(kinc_window_options_t *win);
void kinc_framebuffer_options_set_defaults(kinc_framebuffer_options_t *frame);
void kinc_window_resize(int width, int height);
void kinc_window_move(int x, int y);
void kinc_window_change_mode(kinc_window_mode_t mode);
void kinc_window_change_features(int features);
int kinc_window_x();
int kinc_window_y();
int kinc_window_width();
int kinc_window_height();
int kinc_window_display();
kinc_window_mode_t kinc_window_get_mode();
void kinc_window_show();
void kinc_window_hide();
void kinc_window_set_title(const char *title);
void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data);
void kinc_window_set_close_callback(bool (*callback)(void *data), void *data);

void kinc_internal_call_resize_callback(int width, int height);
bool kinc_internal_call_close_callback();

struct kinc_window_options;
struct kinc_framebuffer_options;

void kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame);
const char *kinc_application_name(void);
void kinc_set_app_name(const char *name);
int kinc_width(void);
int kinc_height(void);
void kinc_load_url(const char *url);
const char *kinc_system_id(void);
const char *kinc_language(void);

typedef uint64_t kinc_ticks_t;

double kinc_frequency(void);
kinc_ticks_t kinc_timestamp(void);
int kinc_cpu_cores(void);
int kinc_hardware_threads(void);
double kinc_time(void);
void kinc_start(void);
void kinc_stop(void);
void kinc_set_keep_screen_on(bool on);

void kinc_copy_to_clipboard(const char *text);
void kinc_set_update_callback(void (*callback)(void *), void *data);
void kinc_set_foreground_callback(void (*callback)(void *), void *data);
void kinc_set_resume_callback(void (*callback)(void *), void *data);
void kinc_set_pause_callback(void (*callback)(void *), void *data);
void kinc_set_background_callback(void (*callback)(void *), void *data);
void kinc_set_shutdown_callback(void (*callback)(void *), void *data);
void kinc_set_drop_files_callback(void (*callback)(wchar_t *, void *), void *data);
void kinc_set_cut_callback(char *(*callback)(void *), void *data);
void kinc_set_copy_callback(char *(*callback)(void *), void *data);
void kinc_set_paste_callback(void (*callback)(char *, void *), void *data);

bool kinc_internal_frame(void);
const char *kinc_internal_save_path(void);
bool kinc_internal_handle_messages(void);
void kinc_internal_shutdown(void);
void kinc_internal_update_callback(void);
void kinc_internal_foreground_callback(void);
void kinc_internal_resume_callback(void);
void kinc_internal_pause_callback(void);
void kinc_internal_background_callback(void);
void kinc_internal_shutdown_callback(void);
void kinc_internal_drop_files_callback(wchar_t *);
char *kinc_internal_cut_callback(void);
char *kinc_internal_copy_callback(void);
void kinc_internal_paste_callback(char *);

#define KINC_KEY_UNKNOWN 0
#define KINC_KEY_BACK 1 // Android
#define KINC_KEY_CANCEL 3
#define KINC_KEY_HELP 6
#define KINC_KEY_BACKSPACE 8
#define KINC_KEY_TAB 9
#define KINC_KEY_CLEAR 12
#define KINC_KEY_RETURN 13
#define KINC_KEY_SHIFT 16
#define KINC_KEY_CONTROL 17
#define KINC_KEY_ALT 18
#define KINC_KEY_PAUSE 19
#define KINC_KEY_CAPS_LOCK 20
#define KINC_KEY_KANA 21
#define KINC_KEY_HANGUL 21
#define KINC_KEY_EISU 22
#define KINC_KEY_JUNJA 23
#define KINC_KEY_FINAL 24
#define KINC_KEY_HANJA 25
#define KINC_KEY_KANJI 25
#define KINC_KEY_ESCAPE 27
#define KINC_KEY_CONVERT 28
#define KINC_KEY_NON_CONVERT 29
#define KINC_KEY_ACCEPT 30
#define KINC_KEY_MODE_CHANGE 31
#define KINC_KEY_SPACE 32
#define KINC_KEY_PAGE_UP 33
#define KINC_KEY_PAGE_DOWN 34
#define KINC_KEY_END 35
#define KINC_KEY_HOME 36
#define KINC_KEY_LEFT 37
#define KINC_KEY_UP 38
#define KINC_KEY_RIGHT 39
#define KINC_KEY_DOWN 40
#define KINC_KEY_SELECT 41
#define KINC_KEY_PRINT 42
#define KINC_KEY_EXECUTE 43
#define KINC_KEY_PRINT_SCREEN 44
#define KINC_KEY_INSERT 45
#define KINC_KEY_DELETE 46
#define KINC_KEY_0 48
#define KINC_KEY_1 49
#define KINC_KEY_2 50
#define KINC_KEY_3 51
#define KINC_KEY_4 52
#define KINC_KEY_5 53
#define KINC_KEY_6 54
#define KINC_KEY_7 55
#define KINC_KEY_8 56
#define KINC_KEY_9 57
#define KINC_KEY_COLON 58
#define KINC_KEY_SEMICOLON 59
#define KINC_KEY_LESS_THAN 60
#define KINC_KEY_EQUALS 61
#define KINC_KEY_GREATER_THAN 62
#define KINC_KEY_QUESTIONMARK 63
#define KINC_KEY_AT 64
#define KINC_KEY_A 65
#define KINC_KEY_B 66
#define KINC_KEY_C 67
#define KINC_KEY_D 68
#define KINC_KEY_E 69
#define KINC_KEY_F 70
#define KINC_KEY_G 71
#define KINC_KEY_H 72
#define KINC_KEY_I 73
#define KINC_KEY_J 74
#define KINC_KEY_K 75
#define KINC_KEY_L 76
#define KINC_KEY_M 77
#define KINC_KEY_N 78
#define KINC_KEY_O 79
#define KINC_KEY_P 80
#define KINC_KEY_Q 81
#define KINC_KEY_R 82
#define KINC_KEY_S 83
#define KINC_KEY_T 84
#define KINC_KEY_U 85
#define KINC_KEY_V 86
#define KINC_KEY_W 87
#define KINC_KEY_X 88
#define KINC_KEY_Y 89
#define KINC_KEY_Z 90
#define KINC_KEY_WIN 91
#define KINC_KEY_CONTEXT_MENU 93
#define KINC_KEY_SLEEP 95
#define KINC_KEY_NUMPAD_0 96
#define KINC_KEY_NUMPAD_1 97
#define KINC_KEY_NUMPAD_2 98
#define KINC_KEY_NUMPAD_3 99
#define KINC_KEY_NUMPAD_4 100
#define KINC_KEY_NUMPAD_5 101
#define KINC_KEY_NUMPAD_6 102
#define KINC_KEY_NUMPAD_7 103
#define KINC_KEY_NUMPAD_8 104
#define KINC_KEY_NUMPAD_9 105
#define KINC_KEY_MULTIPLY 106
#define KINC_KEY_ADD 107
#define KINC_KEY_SEPARATOR 108
#define KINC_KEY_SUBTRACT 109
#define KINC_KEY_DECIMAL 110
#define KINC_KEY_DIVIDE 111
#define KINC_KEY_F1 112
#define KINC_KEY_F2 113
#define KINC_KEY_F3 114
#define KINC_KEY_F4 115
#define KINC_KEY_F5 116
#define KINC_KEY_F6 117
#define KINC_KEY_F7 118
#define KINC_KEY_F8 119
#define KINC_KEY_F9 120
#define KINC_KEY_F10 121
#define KINC_KEY_F11 122
#define KINC_KEY_F12 123
#define KINC_KEY_F13 124
#define KINC_KEY_F14 125
#define KINC_KEY_F15 126
#define KINC_KEY_F16 127
#define KINC_KEY_F17 128
#define KINC_KEY_F18 129
#define KINC_KEY_F19 130
#define KINC_KEY_F20 131
#define KINC_KEY_F21 132
#define KINC_KEY_F22 133
#define KINC_KEY_F23 134
#define KINC_KEY_F24 135
#define KINC_KEY_NUM_LOCK 144
#define KINC_KEY_SCROLL_LOCK 145
#define KINC_KEY_WIN_OEM_FJ_JISHO 146
#define KINC_KEY_WIN_OEM_FJ_MASSHOU 147
#define KINC_KEY_WIN_OEM_FJ_TOUROKU 148
#define KINC_KEY_WIN_OEM_FJ_LOYA 149
#define KINC_KEY_WIN_OEM_FJ_ROYA 150
#define KINC_KEY_CIRCUMFLEX 160
#define KINC_KEY_EXCLAMATION 161
#define KINC_KEY_DOUBLE_QUOTE 162
#define KINC_KEY_HASH 163
#define KINC_KEY_DOLLAR 164
#define KINC_KEY_PERCENT 165
#define KINC_KEY_AMPERSAND 166
#define KINC_KEY_UNDERSCORE 167
#define KINC_KEY_OPEN_PAREN 168
#define KINC_KEY_CLOSE_PAREN 169
#define KINC_KEY_ASTERISK 170
#define KINC_KEY_PLUS 171
#define KINC_KEY_PIPE 172
#define KINC_KEY_HYPHEN_MINUS 173
#define KINC_KEY_OPEN_CURLY_BRACKET 174
#define KINC_KEY_CLOSE_CURLY_BRACKET 175
#define KINC_KEY_TILDE 176
#define KINC_KEY_VOLUME_MUTE 181
#define KINC_KEY_VOLUME_DOWN 182
#define KINC_KEY_VOLUME_UP 183
#define KINC_KEY_COMMA 188
#define KINC_KEY_PERIOD 190
#define KINC_KEY_SLASH 191
#define KINC_KEY_BACK_QUOTE 192
#define KINC_KEY_OPEN_BRACKET 219
#define KINC_KEY_BACK_SLASH 220
#define KINC_KEY_CLOSE_BRACKET 221
#define KINC_KEY_QUOTE 222
#define KINC_KEY_META 224
#define KINC_KEY_ALT_GR 225
#define KINC_KEY_WIN_ICO_HELP 227
#define KINC_KEY_WIN_ICO_00 228
#define KINC_KEY_WIN_ICO_CLEAR 230
#define KINC_KEY_WIN_OEM_RESET 233
#define KINC_KEY_WIN_OEM_JUMP 234
#define KINC_KEY_WIN_OEM_PA1 235
#define KINC_KEY_WIN_OEM_PA2 236
#define KINC_KEY_WIN_OEM_PA3 237
#define KINC_KEY_WIN_OEM_WSCTRL 238
#define KINC_KEY_WIN_OEM_CUSEL 239
#define KINC_KEY_WIN_OEM_ATTN 240
#define KINC_KEY_WIN_OEM_FINISH 241
#define KINC_KEY_WIN_OEM_COPY 242
#define KINC_KEY_WIN_OEM_AUTO 243
#define KINC_KEY_WIN_OEM_ENLW 244
#define KINC_KEY_WIN_OEM_BACK_TAB 245
#define KINC_KEY_ATTN 246
#define KINC_KEY_CRSEL 247
#define KINC_KEY_EXSEL 248
#define KINC_KEY_EREOF 249
#define KINC_KEY_PLAY 250
#define KINC_KEY_ZOOM 251
#define KINC_KEY_PA1 253
#define KINC_KEY_WIN_OEM_CLEAR 254

void kinc_keyboard_show(void);
void kinc_keyboard_hide(void);
bool kinc_keyboard_active(void);
void kinc_keyboard_set_key_down_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data);
void kinc_keyboard_set_key_up_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data);
void kinc_keyboard_set_key_press_callback(void (*value)(unsigned /*character*/, void * /*data*/), void *data);

void kinc_internal_keyboard_trigger_key_down(int key_code);
void kinc_internal_keyboard_trigger_key_up(int key_code);
void kinc_internal_keyboard_trigger_key_press(unsigned character);

#define KINC_MOUSE_LEFT 0
#define KINC_MOUSE_RIGHT 1
#define KINC_MOUSE_MIDDLE 2
// eg backward sidebutton
#define KINC_MOUSE_EXTRA1 3
// eg forward sidebutton
#define KINC_MOUSE_EXTRA2 4

void kinc_mouse_set_press_callback(void (*value)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data);
void kinc_mouse_set_release_callback(void (*value)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data);
void kinc_mouse_set_move_callback(void (*value)(int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/, void * /*data*/),
                                            void *data);
void kinc_mouse_set_scroll_callback(void (*value)(int /*delta*/, void * /*data*/), void *data);
bool kinc_mouse_can_lock(void);
bool kinc_mouse_is_locked(void);
void kinc_mouse_lock();
void kinc_mouse_unlock(void);
void kinc_mouse_set_cursor(int cursor);
void kinc_mouse_show(void);
void kinc_mouse_hide(void);
void kinc_mouse_set_position(int x, int y);
void kinc_mouse_get_position(int *x, int *y);

void kinc_internal_mouse_trigger_press(int button, int x, int y);
void kinc_internal_mouse_trigger_release(int button, int x, int y);
void kinc_internal_mouse_trigger_move(int x, int y);
void kinc_internal_mouse_trigger_scroll(int delta);
void kinc_internal_mouse_lock();
void kinc_internal_mouse_unlock(void);
void kinc_internal_mouse_window_activated();
void kinc_internal_mouse_window_deactivated();

void kinc_pen_set_press_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void kinc_pen_set_move_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void kinc_pen_set_release_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void kinc_eraser_set_press_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void kinc_eraser_set_move_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void kinc_eraser_set_release_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));

void kinc_internal_pen_trigger_move(int x, int y, float pressure);
void kinc_internal_pen_trigger_press(int x, int y, float pressure);
void kinc_internal_pen_trigger_release(int x, int y, float pressure);

void kinc_internal_eraser_trigger_move(int x, int y, float pressure);
void kinc_internal_eraser_trigger_press(int x, int y, float pressure);
void kinc_internal_eraser_trigger_release(int x, int y, float pressure);

void kinc_surface_set_touch_start_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));
void kinc_surface_set_move_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));
void kinc_surface_set_touch_end_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));

void kinc_internal_surface_trigger_touch_start(int index, int x, int y);
void kinc_internal_surface_trigger_move(int index, int x, int y);
void kinc_internal_surface_trigger_touch_end(int index, int x, int y);

#define KINC_GAMEPAD_MAX_COUNT 12

void kinc_gamepad_set_connect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);
void kinc_gamepad_set_disconnect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);
void kinc_gamepad_set_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/, void * /*userdata*/), void *userdata);
void kinc_gamepad_set_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/, void * /*userdata*/), void *userdata);
const char *kinc_gamepad_vendor(int gamepad);
const char *kinc_gamepad_product_name(int gamepad);
bool kinc_gamepad_connected(int gamepad);
void kinc_gamepad_rumble(int gamepad, float left, float right);

void kinc_internal_gamepad_trigger_connect(int gamepad);
void kinc_internal_gamepad_trigger_disconnect(int gamepad);
void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value);
void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value);
