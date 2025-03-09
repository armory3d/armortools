#pragma once

#include <iron_global.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

void iron_log(const char *format, ...);
void iron_error(const char *format, ...);

typedef struct iron_display_mode {
	int x;
	int y;
	int width;
	int height;
	int pixels_per_inch;
	int frequency;
	int bits_per_pixel;
} iron_display_mode_t;

void iron_display_init(void);
int iron_primary_display(void);
int iron_count_displays(void);
bool iron_display_available(int display_index);
const char *iron_display_name(int display_index);
iron_display_mode_t iron_display_current_mode(int display_index);
int iron_display_count_available_modes(int display_index);
iron_display_mode_t iron_display_available_mode(int display_index, int mode_index);

typedef enum {
	IRON_WINDOW_MODE_WINDOW,
	IRON_WINDOW_MODE_FULLSCREEN
} iron_window_mode_t;

#define IRON_WINDOW_FEATURE_RESIZEABLE 1
#define IRON_WINDOW_FEATURE_MINIMIZABLE 2
#define IRON_WINDOW_FEATURE_MAXIMIZABLE 4
#define IRON_WINDOW_FEATURE_BORDERLESS 8
#define IRON_WINDOW_FEATURE_ON_TOP 16

typedef struct iron_window_options {
	const char *title;
	int x;
	int y;
	int width;
	int height;
	int features;
	iron_window_mode_t mode;
	int frequency;
	bool vsync;

	bool use_depth;

	int display_index;
	bool visible;
	int color_bits;
	int depth_bits;

} iron_window_options_t;

void iron_window_create(iron_window_options_t *win);
void iron_window_destroy();
void iron_window_options_set_defaults(iron_window_options_t *win);
void iron_window_resize(int width, int height);
void iron_window_move(int x, int y);
void iron_window_change_mode(iron_window_mode_t mode);
void iron_window_change_features(int features);
int iron_window_x();
int iron_window_y();
int iron_window_width();
int iron_window_height();
int iron_window_display();
iron_window_mode_t iron_window_get_mode();
void iron_window_show();
void iron_window_hide();
void iron_window_set_title(const char *title);
void iron_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data);
void iron_window_set_close_callback(bool (*callback)(void *data), void *data);

void iron_internal_call_resize_callback(int width, int height);
bool iron_internal_call_close_callback();

struct iron_window_options;

void iron_init(const char *name, int width, int height, struct iron_window_options *win);
const char *iron_application_name(void);
void iron_set_app_name(const char *name);
int iron_width(void);
int iron_height(void);
void iron_load_url(const char *url);
const char *iron_system_id(void);
const char *iron_language(void);

typedef uint64_t iron_ticks_t;

double iron_frequency(void);
iron_ticks_t iron_timestamp(void);
int iron_cpu_cores(void);
int iron_hardware_threads(void);
double iron_time(void);
// void iron_start(void);
void kinc_start(void);
void iron_stop(void);
void iron_set_keep_screen_on(bool on);

void iron_copy_to_clipboard(const char *text);
void iron_set_update_callback(void (*callback)(void *), void *data);
void iron_set_foreground_callback(void (*callback)(void *), void *data);
void iron_set_resume_callback(void (*callback)(void *), void *data);
void iron_set_pause_callback(void (*callback)(void *), void *data);
void iron_set_background_callback(void (*callback)(void *), void *data);
void iron_set_shutdown_callback(void (*callback)(void *), void *data);
void iron_set_drop_files_callback(void (*callback)(wchar_t *, void *), void *data);
void iron_set_cut_callback(char *(*callback)(void *), void *data);
void iron_set_copy_callback(char *(*callback)(void *), void *data);
void iron_set_paste_callback(void (*callback)(char *, void *), void *data);

bool iron_internal_frame(void);
const char *iron_internal_save_path(void);
bool iron_internal_handle_messages(void);
void iron_internal_shutdown(void);
void iron_internal_update_callback(void);
void iron_internal_foreground_callback(void);
void iron_internal_resume_callback(void);
void iron_internal_pause_callback(void);
void iron_internal_background_callback(void);
void iron_internal_shutdown_callback(void);
void iron_internal_drop_files_callback(wchar_t *);
char *iron_internal_cut_callback(void);
char *iron_internal_copy_callback(void);
void iron_internal_paste_callback(char *);

#define IRON_KEY_UNKNOWN 0
#define IRON_KEY_BACK 1 // Android
#define IRON_KEY_CANCEL 3
#define IRON_KEY_HELP 6
#define IRON_KEY_BACKSPACE 8
#define IRON_KEY_TAB 9
#define IRON_KEY_CLEAR 12
#define IRON_KEY_RETURN 13
#define IRON_KEY_SHIFT 16
#define IRON_KEY_CONTROL 17
#define IRON_KEY_ALT 18
#define IRON_KEY_PAUSE 19
#define IRON_KEY_CAPS_LOCK 20
#define IRON_KEY_KANA 21
#define IRON_KEY_HANGUL 21
#define IRON_KEY_EISU 22
#define IRON_KEY_JUNJA 23
#define IRON_KEY_FINAL 24
#define IRON_KEY_HANJA 25
#define IRON_KEY_KANJI 25
#define IRON_KEY_ESCAPE 27
#define IRON_KEY_CONVERT 28
#define IRON_KEY_NON_CONVERT 29
#define IRON_KEY_ACCEPT 30
#define IRON_KEY_MODE_CHANGE 31
#define IRON_KEY_SPACE 32
#define IRON_KEY_PAGE_UP 33
#define IRON_KEY_PAGE_DOWN 34
#define IRON_KEY_END 35
#define IRON_KEY_HOME 36
#define IRON_KEY_LEFT 37
#define IRON_KEY_UP 38
#define IRON_KEY_RIGHT 39
#define IRON_KEY_DOWN 40
#define IRON_KEY_SELECT 41
#define IRON_KEY_PRINT 42
#define IRON_KEY_EXECUTE 43
#define IRON_KEY_PRINT_SCREEN 44
#define IRON_KEY_INSERT 45
#define IRON_KEY_DELETE 46
#define IRON_KEY_0 48
#define IRON_KEY_1 49
#define IRON_KEY_2 50
#define IRON_KEY_3 51
#define IRON_KEY_4 52
#define IRON_KEY_5 53
#define IRON_KEY_6 54
#define IRON_KEY_7 55
#define IRON_KEY_8 56
#define IRON_KEY_9 57
#define IRON_KEY_COLON 58
#define IRON_KEY_SEMICOLON 59
#define IRON_KEY_LESS_THAN 60
#define IRON_KEY_EQUALS 61
#define IRON_KEY_GREATER_THAN 62
#define IRON_KEY_QUESTIONMARK 63
#define IRON_KEY_AT 64
#define IRON_KEY_A 65
#define IRON_KEY_B 66
#define IRON_KEY_C 67
#define IRON_KEY_D 68
#define IRON_KEY_E 69
#define IRON_KEY_F 70
#define IRON_KEY_G 71
#define IRON_KEY_H 72
#define IRON_KEY_I 73
#define IRON_KEY_J 74
#define IRON_KEY_K 75
#define IRON_KEY_L 76
#define IRON_KEY_M 77
#define IRON_KEY_N 78
#define IRON_KEY_O 79
#define IRON_KEY_P 80
#define IRON_KEY_Q 81
#define IRON_KEY_R 82
#define IRON_KEY_S 83
#define IRON_KEY_T 84
#define IRON_KEY_U 85
#define IRON_KEY_V 86
#define IRON_KEY_W 87
#define IRON_KEY_X 88
#define IRON_KEY_Y 89
#define IRON_KEY_Z 90
#define IRON_KEY_WIN 91
#define IRON_KEY_CONTEXT_MENU 93
#define IRON_KEY_SLEEP 95
#define IRON_KEY_NUMPAD_0 96
#define IRON_KEY_NUMPAD_1 97
#define IRON_KEY_NUMPAD_2 98
#define IRON_KEY_NUMPAD_3 99
#define IRON_KEY_NUMPAD_4 100
#define IRON_KEY_NUMPAD_5 101
#define IRON_KEY_NUMPAD_6 102
#define IRON_KEY_NUMPAD_7 103
#define IRON_KEY_NUMPAD_8 104
#define IRON_KEY_NUMPAD_9 105
#define IRON_KEY_MULTIPLY 106
#define IRON_KEY_ADD 107
#define IRON_KEY_SEPARATOR 108
#define IRON_KEY_SUBTRACT 109
#define IRON_KEY_DECIMAL 110
#define IRON_KEY_DIVIDE 111
#define IRON_KEY_F1 112
#define IRON_KEY_F2 113
#define IRON_KEY_F3 114
#define IRON_KEY_F4 115
#define IRON_KEY_F5 116
#define IRON_KEY_F6 117
#define IRON_KEY_F7 118
#define IRON_KEY_F8 119
#define IRON_KEY_F9 120
#define IRON_KEY_F10 121
#define IRON_KEY_F11 122
#define IRON_KEY_F12 123
#define IRON_KEY_F13 124
#define IRON_KEY_F14 125
#define IRON_KEY_F15 126
#define IRON_KEY_F16 127
#define IRON_KEY_F17 128
#define IRON_KEY_F18 129
#define IRON_KEY_F19 130
#define IRON_KEY_F20 131
#define IRON_KEY_F21 132
#define IRON_KEY_F22 133
#define IRON_KEY_F23 134
#define IRON_KEY_F24 135
#define IRON_KEY_NUM_LOCK 144
#define IRON_KEY_SCROLL_LOCK 145
#define IRON_KEY_WIN_OEM_FJ_JISHO 146
#define IRON_KEY_WIN_OEM_FJ_MASSHOU 147
#define IRON_KEY_WIN_OEM_FJ_TOUROKU 148
#define IRON_KEY_WIN_OEM_FJ_LOYA 149
#define IRON_KEY_WIN_OEM_FJ_ROYA 150
#define IRON_KEY_CIRCUMFLEX 160
#define IRON_KEY_EXCLAMATION 161
#define IRON_KEY_DOUBLE_QUOTE 162
#define IRON_KEY_HASH 163
#define IRON_KEY_DOLLAR 164
#define IRON_KEY_PERCENT 165
#define IRON_KEY_AMPERSAND 166
#define IRON_KEY_UNDERSCORE 167
#define IRON_KEY_OPEN_PAREN 168
#define IRON_KEY_CLOSE_PAREN 169
#define IRON_KEY_ASTERISK 170
#define IRON_KEY_PLUS 171
#define IRON_KEY_PIPE 172
#define IRON_KEY_HYPHEN_MINUS 173
#define IRON_KEY_OPEN_CURLY_BRACKET 174
#define IRON_KEY_CLOSE_CURLY_BRACKET 175
#define IRON_KEY_TILDE 176
#define IRON_KEY_VOLUME_MUTE 181
#define IRON_KEY_VOLUME_DOWN 182
#define IRON_KEY_VOLUME_UP 183
#define IRON_KEY_COMMA 188
#define IRON_KEY_PERIOD 190
#define IRON_KEY_SLASH 191
#define IRON_KEY_BACK_QUOTE 192
#define IRON_KEY_OPEN_BRACKET 219
#define IRON_KEY_BACK_SLASH 220
#define IRON_KEY_CLOSE_BRACKET 221
#define IRON_KEY_QUOTE 222
#define IRON_KEY_META 224
#define IRON_KEY_ALT_GR 225
#define IRON_KEY_WIN_ICO_HELP 227
#define IRON_KEY_WIN_ICO_00 228
#define IRON_KEY_WIN_ICO_CLEAR 230
#define IRON_KEY_WIN_OEM_RESET 233
#define IRON_KEY_WIN_OEM_JUMP 234
#define IRON_KEY_WIN_OEM_PA1 235
#define IRON_KEY_WIN_OEM_PA2 236
#define IRON_KEY_WIN_OEM_PA3 237
#define IRON_KEY_WIN_OEM_WSCTRL 238
#define IRON_KEY_WIN_OEM_CUSEL 239
#define IRON_KEY_WIN_OEM_ATTN 240
#define IRON_KEY_WIN_OEM_FINISH 241
#define IRON_KEY_WIN_OEM_COPY 242
#define IRON_KEY_WIN_OEM_AUTO 243
#define IRON_KEY_WIN_OEM_ENLW 244
#define IRON_KEY_WIN_OEM_BACK_TAB 245
#define IRON_KEY_ATTN 246
#define IRON_KEY_CRSEL 247
#define IRON_KEY_EXSEL 248
#define IRON_KEY_EREOF 249
#define IRON_KEY_PLAY 250
#define IRON_KEY_ZOOM 251
#define IRON_KEY_PA1 253
#define IRON_KEY_WIN_OEM_CLEAR 254

void iron_keyboard_show(void);
void iron_keyboard_hide(void);
bool iron_keyboard_active(void);
void iron_keyboard_set_key_down_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data);
void iron_keyboard_set_key_up_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data);
void iron_keyboard_set_key_press_callback(void (*value)(unsigned /*character*/, void * /*data*/), void *data);

void iron_internal_keyboard_trigger_key_down(int key_code);
void iron_internal_keyboard_trigger_key_up(int key_code);
void iron_internal_keyboard_trigger_key_press(unsigned character);

#define IRON_MOUSE_LEFT 0
#define IRON_MOUSE_RIGHT 1
#define IRON_MOUSE_MIDDLE 2
// eg backward sidebutton
#define IRON_MOUSE_EXTRA1 3
// eg forward sidebutton
#define IRON_MOUSE_EXTRA2 4

void iron_mouse_set_press_callback(void (*value)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data);
void iron_mouse_set_release_callback(void (*value)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data);
void iron_mouse_set_move_callback(void (*value)(int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/, void * /*data*/),
                                            void *data);
void iron_mouse_set_scroll_callback(void (*value)(int /*delta*/, void * /*data*/), void *data);
bool iron_mouse_can_lock(void);
bool iron_mouse_is_locked(void);
void iron_mouse_lock();
void iron_mouse_unlock(void);
void iron_mouse_set_cursor(int cursor);
void iron_mouse_show(void);
void iron_mouse_hide(void);
void iron_mouse_set_position(int x, int y);
void iron_mouse_get_position(int *x, int *y);

void iron_internal_mouse_trigger_press(int button, int x, int y);
void iron_internal_mouse_trigger_release(int button, int x, int y);
void iron_internal_mouse_trigger_move(int x, int y);
void iron_internal_mouse_trigger_scroll(int delta);
void iron_internal_mouse_lock();
void iron_internal_mouse_unlock(void);
void iron_internal_mouse_window_activated();
void iron_internal_mouse_window_deactivated();

void iron_pen_set_press_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void iron_pen_set_move_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void iron_pen_set_release_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void iron_eraser_set_press_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void iron_eraser_set_move_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));
void iron_eraser_set_release_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/));

void iron_internal_pen_trigger_move(int x, int y, float pressure);
void iron_internal_pen_trigger_press(int x, int y, float pressure);
void iron_internal_pen_trigger_release(int x, int y, float pressure);

void iron_internal_eraser_trigger_move(int x, int y, float pressure);
void iron_internal_eraser_trigger_press(int x, int y, float pressure);
void iron_internal_eraser_trigger_release(int x, int y, float pressure);

void iron_surface_set_touch_start_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));
void iron_surface_set_move_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));
void iron_surface_set_touch_end_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/));

void iron_internal_surface_trigger_touch_start(int index, int x, int y);
void iron_internal_surface_trigger_move(int index, int x, int y);
void iron_internal_surface_trigger_touch_end(int index, int x, int y);

#define IRON_GAMEPAD_MAX_COUNT 12

void iron_gamepad_set_connect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);
void iron_gamepad_set_disconnect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);
void iron_gamepad_set_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/, void * /*userdata*/), void *userdata);
void iron_gamepad_set_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/, void * /*userdata*/), void *userdata);
const char *iron_gamepad_vendor(int gamepad);
const char *iron_gamepad_product_name(int gamepad);
bool iron_gamepad_connected(int gamepad);
void iron_gamepad_rumble(int gamepad, float left, float right);

void iron_internal_gamepad_trigger_connect(int gamepad);
void iron_internal_gamepad_trigger_disconnect(int gamepad);
void iron_internal_gamepad_trigger_axis(int gamepad, int axis, float value);
void iron_internal_gamepad_trigger_button(int gamepad, int button, float value);
