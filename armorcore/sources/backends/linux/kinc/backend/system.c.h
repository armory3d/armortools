#include "kinc/graphics4/graphics.h"
#include <kinc/display.h>
#include <kinc/input/gamepad.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/input/pen.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/video.h>
#include <kinc/window.h>
#include "gamepad.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "funcs.h"

bool kinc_internal_handle_messages() {
	if (!procs.handle_messages()) {
		return false;
	}

	kinc_linux_updateHIDGamepads();

	return true;
}

const char *kinc_system_id() {
	return "Linux";
}

void kinc_set_keep_screen_on(bool on) {}

void kinc_keyboard_show() {}

void kinc_keyboard_hide() {}

bool kinc_keyboard_active() {
	return true;
}

void kinc_load_url(const char *url) {
#define MAX_COMMAND_BUFFER_SIZE 256
#define HTTP "http://"
#define HTTPS "https://"
	if (strncmp(url, HTTP, sizeof(HTTP) - 1) == 0 || strncmp(url, HTTPS, sizeof(HTTPS) - 1) == 0) {
		char openUrlCommand[MAX_COMMAND_BUFFER_SIZE];
		snprintf(openUrlCommand, MAX_COMMAND_BUFFER_SIZE, "xdg-open %s", url);
		int err = system(openUrlCommand);
		if (err != 0) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Error opening url %s", url);
		}
	}
#undef HTTPS
#undef HTTP
#undef MAX_COMMAND_BUFFER_SIZE
}

const char *kinc_language() {
	return "en";
}

static char save[2000];
static bool saveInitialized = false;

const char *kinc_internal_save_path() {
	// first check for an existing directory in $HOME
	// if one exists, use it, else create one in $XDG_DATA_HOME
	// See: https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
	if (!saveInitialized) {
		const char *homedir;

		if ((homedir = getenv("HOME")) == NULL) {
			homedir = getpwuid(getuid())->pw_dir;
		}

		strcpy(save, homedir);
		strcat(save, "/.");
		strcat(save, kinc_application_name());
		strcat(save, "/");
		struct stat st;
		if (stat(save, &st) == 0) {
			// use existing folder in $HOME
		}
		else {
			// use XDG folder
			const char *data_home;
			if ((data_home = getenv("XDG_DATA_HOME")) == NULL) {
				// $XDG_DATA_HOME is not defined, fall back to the default, $HOME/.local/share
				strcpy(save, homedir);
				strcat(save, "/.local/share/");
			}
			else {
				// use $XDG_DATA_HOME
				strcpy(save, data_home);
				if (data_home[strlen(data_home) - 1] != '/') {
					strcat(save, "/");
				}
			}
			strcat(save, kinc_application_name());
			strcat(save, "/");
			int res = mkdir(save, 0700);
			if (res != 0 && errno != EEXIST) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not create save directory '%s'. Error %d", save, errno);
			}
		}

		saveInitialized = true;
	}
	return save;
}

static const char *videoFormats[] = {"ogv", NULL};

const char **kinc_video_formats() {
	return videoFormats;
}

#include <sys/time.h>
#include <time.h>

double kinc_frequency(void) {
	return 1000000.0;
}

static struct timeval start;

kinc_ticks_t kinc_timestamp(void) {
	struct timeval now;
	gettimeofday(&now, NULL);
	now.tv_sec -= start.tv_sec;
	now.tv_usec -= start.tv_usec;
	return (kinc_ticks_t)now.tv_sec * 1000000 + (kinc_ticks_t)now.tv_usec;
}

void kinc_linux_init_procs();

int kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	kinc_linux_initHIDGamepads();

	gettimeofday(&start, NULL);
	kinc_linux_init_procs();
	kinc_display_init();

	kinc_set_application_name(name);

	kinc_g4_internal_init();

	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}
	kinc_framebuffer_options_t defaultFrame;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFrame);
		frame = &defaultFrame;
	}
	win->width = width;
	win->height = height;
	if (win->title == NULL) {
		win->title = name;
	}

	int window = kinc_window_create(win, frame);

	return window;
}

void kinc_internal_shutdown() {
	kinc_g4_internal_destroy();
	kinc_linux_closeHIDGamepads();
	procs.shutdown();
	kinc_internal_shutdown_callback();
}

#ifndef KINC_NO_MAIN
int main(int argc, char **argv) {
	return kickstart(argc, argv);
}
#endif

void kinc_copy_to_clipboard(const char *text) {
	procs.copy_to_clipboard(text);
}

static int parse_number_at_end_of_line(char *line) {
	char *end = &line[strlen(line) - 2];
	int num = 0;
	int multi = 1;
	while (*end >= '0' && *end <= '9') {
		num += (*end - '0') * multi;
		multi *= 10;
		--end;
	}
	return num;
}

int kinc_cpu_cores(void) {
	char line[1024];
	FILE *file = fopen("/proc/cpuinfo", "r");

	if (file != NULL) {
		int cores[1024];
		memset(cores, 0, sizeof(cores));

		int cpu_count = 0;
		int physical_id = -1;
		int per_cpu_cores = -1;
		int processor_count = 0;

		while (fgets(line, sizeof(line), file)) {
			if (strncmp(line, "processor", 9) == 0) {
				++processor_count;
				if (physical_id >= 0 && per_cpu_cores > 0) {
					if (physical_id + 1 > cpu_count) {
						cpu_count = physical_id + 1;
					}
					cores[physical_id] = per_cpu_cores;
					physical_id = -1;
					per_cpu_cores = -1;
				}
			}
			else if (strncmp(line, "physical id", 11) == 0) {
				physical_id = parse_number_at_end_of_line(line);
			}
			else if (strncmp(line, "cpu cores", 9) == 0) {
				per_cpu_cores = parse_number_at_end_of_line(line);
			}
		}
		fclose(file);

		if (physical_id >= 0 && per_cpu_cores > 0) {
			if (physical_id + 1 > cpu_count) {
				cpu_count = physical_id + 1;
			}
			cores[physical_id] = per_cpu_cores;
		}

		int proper_cpu_count = 0;
		for (int i = 0; i < cpu_count; ++i) {
			proper_cpu_count += cores[i];
		}

		if (proper_cpu_count > 0) {
			return proper_cpu_count;
		}
		else {
			return processor_count == 0 ? 1 : processor_count;
		}
	}
	else {
		return 1;
	};
}

int kinc_hardware_threads(void) {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

#include <xkbcommon/xkbcommon.h>

int xkb_to_kinc(xkb_keysym_t symbol) {
#define KEY(xkb, kinc)                                                                                                                                         \
	case xkb:                                                                                                                                                  \
		return kinc;
	switch (symbol) {
		KEY(XKB_KEY_Right, KINC_KEY_RIGHT)
		KEY(XKB_KEY_Left, KINC_KEY_LEFT)
		KEY(XKB_KEY_Up, KINC_KEY_UP)
		KEY(XKB_KEY_Down, KINC_KEY_DOWN)
		KEY(XKB_KEY_space, KINC_KEY_SPACE)
		KEY(XKB_KEY_BackSpace, KINC_KEY_BACKSPACE)
		KEY(XKB_KEY_Tab, KINC_KEY_TAB)
		KEY(XKB_KEY_Return, KINC_KEY_RETURN)
		KEY(XKB_KEY_Shift_L, KINC_KEY_SHIFT)
		KEY(XKB_KEY_Shift_R, KINC_KEY_SHIFT)
		KEY(XKB_KEY_Control_L, KINC_KEY_CONTROL)
		KEY(XKB_KEY_Control_R, KINC_KEY_CONTROL)
		KEY(XKB_KEY_Alt_L, KINC_KEY_ALT)
		KEY(XKB_KEY_Alt_R, KINC_KEY_ALT)
		KEY(XKB_KEY_Delete, KINC_KEY_DELETE)
		KEY(XKB_KEY_comma, KINC_KEY_COMMA)
		KEY(XKB_KEY_period, KINC_KEY_PERIOD)
		KEY(XKB_KEY_bracketleft, KINC_KEY_OPEN_BRACKET)
		KEY(XKB_KEY_bracketright, KINC_KEY_CLOSE_BRACKET)
		KEY(XKB_KEY_braceleft, KINC_KEY_OPEN_CURLY_BRACKET)
		KEY(XKB_KEY_braceright, KINC_KEY_CLOSE_CURLY_BRACKET)
		KEY(XKB_KEY_parenleft, KINC_KEY_OPEN_PAREN)
		KEY(XKB_KEY_parenright, KINC_KEY_CLOSE_PAREN)
		KEY(XKB_KEY_backslash, KINC_KEY_BACK_SLASH)
		KEY(XKB_KEY_apostrophe, KINC_KEY_QUOTE)
		KEY(XKB_KEY_colon, KINC_KEY_COLON)
		KEY(XKB_KEY_semicolon, KINC_KEY_SEMICOLON)
		KEY(XKB_KEY_minus, KINC_KEY_HYPHEN_MINUS)
		KEY(XKB_KEY_underscore, KINC_KEY_UNDERSCORE)
		KEY(XKB_KEY_slash, KINC_KEY_SLASH)
		KEY(XKB_KEY_bar, KINC_KEY_PIPE)
		KEY(XKB_KEY_question, KINC_KEY_QUESTIONMARK)
		KEY(XKB_KEY_less, KINC_KEY_LESS_THAN)
		KEY(XKB_KEY_greater, KINC_KEY_GREATER_THAN)
		KEY(XKB_KEY_asterisk, KINC_KEY_ASTERISK)
		KEY(XKB_KEY_ampersand, KINC_KEY_AMPERSAND)
		KEY(XKB_KEY_asciicircum, KINC_KEY_CIRCUMFLEX)
		KEY(XKB_KEY_percent, KINC_KEY_PERCENT)
		KEY(XKB_KEY_dollar, KINC_KEY_DOLLAR)
		KEY(XKB_KEY_numbersign, KINC_KEY_HASH)
		KEY(XKB_KEY_at, KINC_KEY_AT)
		KEY(XKB_KEY_exclam, KINC_KEY_EXCLAMATION)
		KEY(XKB_KEY_equal, KINC_KEY_EQUALS)
		KEY(XKB_KEY_plus, KINC_KEY_ADD)
		KEY(XKB_KEY_quoteleft, KINC_KEY_BACK_QUOTE)
		KEY(XKB_KEY_quotedbl, KINC_KEY_DOUBLE_QUOTE)
		KEY(XKB_KEY_asciitilde, KINC_KEY_TILDE)
		KEY(XKB_KEY_Pause, KINC_KEY_PAUSE)
		KEY(XKB_KEY_Scroll_Lock, KINC_KEY_SCROLL_LOCK)
		KEY(XKB_KEY_Home, KINC_KEY_HOME)
		KEY(XKB_KEY_Page_Up, KINC_KEY_PAGE_UP)
		KEY(XKB_KEY_Page_Down, KINC_KEY_PAGE_DOWN)
		KEY(XKB_KEY_End, KINC_KEY_END)
		KEY(XKB_KEY_Insert, KINC_KEY_INSERT)
		KEY(XKB_KEY_KP_Enter, KINC_KEY_RETURN)
		KEY(XKB_KEY_KP_Multiply, KINC_KEY_MULTIPLY)
		KEY(XKB_KEY_KP_Add, KINC_KEY_ADD)
		KEY(XKB_KEY_KP_Subtract, KINC_KEY_SUBTRACT)
		KEY(XKB_KEY_KP_Decimal, KINC_KEY_DECIMAL)
		KEY(XKB_KEY_KP_Divide, KINC_KEY_DIVIDE)
		KEY(XKB_KEY_KP_0, KINC_KEY_NUMPAD_0)
		KEY(XKB_KEY_KP_1, KINC_KEY_NUMPAD_1)
		KEY(XKB_KEY_KP_2, KINC_KEY_NUMPAD_2)
		KEY(XKB_KEY_KP_3, KINC_KEY_NUMPAD_3)
		KEY(XKB_KEY_KP_4, KINC_KEY_NUMPAD_4)
		KEY(XKB_KEY_KP_5, KINC_KEY_NUMPAD_5)
		KEY(XKB_KEY_KP_6, KINC_KEY_NUMPAD_6)
		KEY(XKB_KEY_KP_7, KINC_KEY_NUMPAD_7)
		KEY(XKB_KEY_KP_8, KINC_KEY_NUMPAD_8)
		KEY(XKB_KEY_KP_9, KINC_KEY_NUMPAD_9)
		KEY(XKB_KEY_KP_Insert, KINC_KEY_INSERT)
		KEY(XKB_KEY_KP_Delete, KINC_KEY_DELETE)
		KEY(XKB_KEY_KP_End, KINC_KEY_END)
		KEY(XKB_KEY_KP_Home, KINC_KEY_HOME)
		KEY(XKB_KEY_KP_Left, KINC_KEY_LEFT)
		KEY(XKB_KEY_KP_Up, KINC_KEY_UP)
		KEY(XKB_KEY_KP_Right, KINC_KEY_RIGHT)
		KEY(XKB_KEY_KP_Down, KINC_KEY_DOWN)
		KEY(XKB_KEY_KP_Page_Up, KINC_KEY_PAGE_UP)
		KEY(XKB_KEY_KP_Page_Down, KINC_KEY_PAGE_DOWN)
		KEY(XKB_KEY_Menu, KINC_KEY_CONTEXT_MENU)
		KEY(XKB_KEY_a, KINC_KEY_A)
		KEY(XKB_KEY_b, KINC_KEY_B)
		KEY(XKB_KEY_c, KINC_KEY_C)
		KEY(XKB_KEY_d, KINC_KEY_D)
		KEY(XKB_KEY_e, KINC_KEY_E)
		KEY(XKB_KEY_f, KINC_KEY_F)
		KEY(XKB_KEY_g, KINC_KEY_G)
		KEY(XKB_KEY_h, KINC_KEY_H)
		KEY(XKB_KEY_i, KINC_KEY_I)
		KEY(XKB_KEY_j, KINC_KEY_J)
		KEY(XKB_KEY_k, KINC_KEY_K)
		KEY(XKB_KEY_l, KINC_KEY_L)
		KEY(XKB_KEY_m, KINC_KEY_M)
		KEY(XKB_KEY_n, KINC_KEY_N)
		KEY(XKB_KEY_o, KINC_KEY_O)
		KEY(XKB_KEY_p, KINC_KEY_P)
		KEY(XKB_KEY_q, KINC_KEY_Q)
		KEY(XKB_KEY_r, KINC_KEY_R)
		KEY(XKB_KEY_s, KINC_KEY_S)
		KEY(XKB_KEY_t, KINC_KEY_T)
		KEY(XKB_KEY_u, KINC_KEY_U)
		KEY(XKB_KEY_v, KINC_KEY_V)
		KEY(XKB_KEY_w, KINC_KEY_W)
		KEY(XKB_KEY_x, KINC_KEY_X)
		KEY(XKB_KEY_y, KINC_KEY_Y)
		KEY(XKB_KEY_z, KINC_KEY_Z)
		KEY(XKB_KEY_1, KINC_KEY_1)
		KEY(XKB_KEY_2, KINC_KEY_2)
		KEY(XKB_KEY_3, KINC_KEY_3)
		KEY(XKB_KEY_4, KINC_KEY_4)
		KEY(XKB_KEY_5, KINC_KEY_5)
		KEY(XKB_KEY_6, KINC_KEY_6)
		KEY(XKB_KEY_7, KINC_KEY_7)
		KEY(XKB_KEY_8, KINC_KEY_8)
		KEY(XKB_KEY_9, KINC_KEY_9)
		KEY(XKB_KEY_0, KINC_KEY_0)
		KEY(XKB_KEY_Escape, KINC_KEY_ESCAPE)
		KEY(XKB_KEY_F1, KINC_KEY_F1)
		KEY(XKB_KEY_F2, KINC_KEY_F2)
		KEY(XKB_KEY_F3, KINC_KEY_F3)
		KEY(XKB_KEY_F4, KINC_KEY_F4)
		KEY(XKB_KEY_F5, KINC_KEY_F5)
		KEY(XKB_KEY_F6, KINC_KEY_F6)
		KEY(XKB_KEY_F7, KINC_KEY_F7)
		KEY(XKB_KEY_F8, KINC_KEY_F8)
		KEY(XKB_KEY_F9, KINC_KEY_F9)
		KEY(XKB_KEY_F10, KINC_KEY_F10)
		KEY(XKB_KEY_F11, KINC_KEY_F11)
		KEY(XKB_KEY_F12, KINC_KEY_F12)
	default:
		return KINC_KEY_UNKNOWN;
	}
#undef KEY
}