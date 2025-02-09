#pragma once

#include <kinc/global.h>

/*! \file keyboard.h
    \brief Provides keyboard-support.
*/

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

#ifdef KINC_IMPLEMENTATION_INPUT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <memory.h>
#include <stddef.h>

static void (*keyboard_key_down_callback)(int /*key_code*/, void * /*data*/) = NULL;
static void *keyboard_key_down_callback_data = NULL;
static void (*keyboard_key_up_callback)(int /*key_code*/, void * /*data*/) = NULL;
static void *keyboard_key_up_callback_data = NULL;
static void (*keyboard_key_press_callback)(unsigned /*character*/, void * /*data*/) = NULL;
static void *keyboard_key_press_callback_data = NULL;

void kinc_keyboard_set_key_down_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data) {
	keyboard_key_down_callback = value;
	keyboard_key_down_callback_data = data;
}

void kinc_keyboard_set_key_up_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data) {
	keyboard_key_up_callback = value;
	keyboard_key_up_callback_data = data;
}

void kinc_keyboard_set_key_press_callback(void (*value)(unsigned /*character*/, void * /*data*/), void *data) {
	keyboard_key_press_callback = value;
	keyboard_key_press_callback_data = data;
}

void kinc_internal_keyboard_trigger_key_down(int key_code) {
	if (keyboard_key_down_callback != NULL) {
		keyboard_key_down_callback(key_code, keyboard_key_down_callback_data);
	}
}

void kinc_internal_keyboard_trigger_key_up(int key_code) {
	if (keyboard_key_up_callback != NULL) {
		keyboard_key_up_callback(key_code, keyboard_key_up_callback_data);
	}
}

void kinc_internal_keyboard_trigger_key_press(unsigned character) {
	if (keyboard_key_press_callback != NULL) {
		keyboard_key_press_callback(character, keyboard_key_press_callback_data);
	}
}

#endif
