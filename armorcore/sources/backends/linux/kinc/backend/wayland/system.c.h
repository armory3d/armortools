#include "wayland.h"

#include <kinc/input/pen.h>
#include <kinc/log.h>
#include <wayland-generated/wayland-pointer-constraint.h>
#include <wayland-generated/wayland-relative-pointer.h>
#include <wayland-generated/wayland-tablet.h>
#include <wayland-generated/xdg-shell.h>

#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#include <dlfcn.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>

static void load_lib(void **lib, const char *name);

struct kinc_wl_procs wl = {0};
struct kinc_xkb_procs wl_xkb = {0};

bool kinc_wayland_load_procs() {
	void *wayland_client = NULL;
	load_lib(&wayland_client, "wayland-client");
	if (wayland_client == NULL) {
		return false;
	}
	bool has_missing_symbol = false;
#undef LOAD_FUN
#define LOAD_FUN(lib, symbol, name)                                                                                                                            \
	wl.symbol = dlsym(lib, name);                                                                                                                              \
	if (wl.symbol == NULL) {                                                                                                                                   \
		has_missing_symbol = true;                                                                                                                             \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Did not find symbol %s.", name);                                                                                       \
	}
#define KINC_WL_FUN(ret, name, args) LOAD_FUN(wayland_client, _##name, #name)
#include "wayland-funs.h"
#undef KINC_WL_FN

	void *wayland_cursor = NULL;
	load_lib(&wayland_cursor, "wayland-cursor");
	if (wayland_cursor == NULL) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to find libwayland-cursor.so");
		return false;
	}
	LOAD_FUN(wayland_cursor, _wl_cursor_theme_load, "wl_cursor_theme_load")
	LOAD_FUN(wayland_cursor, _wl_cursor_theme_destroy, "wl_cursor_theme_destroy")
	LOAD_FUN(wayland_cursor, _wl_cursor_theme_get_cursor, "wl_cursor_theme_get_cursor")
	LOAD_FUN(wayland_cursor, _wl_cursor_image_get_buffer, "wl_cursor_image_get_buffer")
	LOAD_FUN(wayland_cursor, _wl_cursor_frame, "wl_cursor_frame")
	LOAD_FUN(wayland_cursor, _wl_cursor_frame_and_duration, "wl_cursor_frame_and_duration")

#undef LOAD_FUN
#define LOAD_FUN(symbol)                                                                                                                                       \
	wl_xkb.symbol = dlsym(xkb, #symbol);                                                                                                                       \
	if (wl_xkb.symbol == NULL) {                                                                                                                               \
		has_missing_symbol = true;                                                                                                                             \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Did not find symbol %s.", #symbol);                                                                                    \
	}
	void *xkb = NULL;
	load_lib(&xkb, "xkbcommon");

	if (xkb == NULL) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to find libxkb_common.so");
		return false;
	}
	LOAD_FUN(xkb_context_new)
	LOAD_FUN(xkb_context_unref)
	LOAD_FUN(xkb_state_new)
	LOAD_FUN(xkb_keymap_new_from_string)
	LOAD_FUN(xkb_state_key_get_one_sym)
	LOAD_FUN(xkb_state_key_get_utf32)
	LOAD_FUN(xkb_state_serialize_mods)
	LOAD_FUN(xkb_state_update_mask)
	LOAD_FUN(xkb_state_mod_name_is_active)
	LOAD_FUN(xkb_keymap_key_repeats)
#undef LOAD_FUN

	if (has_missing_symbol) {
		return false;
	}

	return true;
}

struct wayland_context wl_ctx = {0};

static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *shell, uint32_t serial) {
	xdg_wm_base_pong(shell, serial);
};

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    xdg_wm_base_handle_ping,
};

static void wl_output_handle_geometry(void *data, struct wl_output *wl_output, int x, int y, int physical_width, int physical_height, int subpixel,
                                      const char *make, const char *model, int transform) {
	struct kinc_wl_display *display = data;
	snprintf(display->name, sizeof(display->name), "%s %s", make, model);
	display->x = x;
	display->y = y;
	display->physical_width = physical_width;
	display->physical_height = physical_height;
	display->subpixel = subpixel;
	display->transform = transform;
}

static void wl_output_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	struct kinc_wl_display *display = data;
	if (display->num_modes < MAXIMUM_DISPLAY_MODES) {
		int mode_index = display->num_modes++;
		kinc_display_mode_t *mode = &display->modes[mode_index];
		mode->x = 0;
		mode->y = 0;
		mode->width = width;
		mode->height = height;
		mode->bits_per_pixel = 32;
		mode->pixels_per_inch = 96;
		mode->frequency = (refresh / 1000);
		if (flags & WL_OUTPUT_MODE_CURRENT)
			display->current_mode = mode_index;
	}
}
static void wl_output_handle_done(void *data, struct wl_output *wl_output) {
	// struct kinc_wl_display *display = data;
}
static void wl_output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	struct kinc_wl_display *display = data;
	display->scale = factor;
}

static const struct wl_output_listener wl_output_listener = {
    wl_output_handle_geometry,
    wl_output_handle_mode,
    wl_output_handle_done,
    wl_output_handle_scale,
};

struct kinc_wl_window *kinc_wayland_window_from_surface(struct wl_surface *surface, enum kinc_wl_decoration_focus *focus) {
	struct kinc_wl_window *window = wl_surface_get_user_data(surface);
	if (window == NULL) {
		for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
			struct kinc_wl_window *_window = &wl_ctx.windows[i];
			if (_window->surface == surface) {
				*focus = KINC_WL_DECORATION_FOCUS_MAIN;
				window = _window;
			}
			else if (surface == _window->decorations.top.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_TOP;
				window = _window;
			}
			else if (surface == _window->decorations.left.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_LEFT;
				window = _window;
			}
			else if (surface == _window->decorations.right.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_RIGHT;
				window = _window;
			}
			else if (surface == _window->decorations.bottom.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_BOTTOM;
				window = _window;
			}
			else if (surface == _window->decorations.close.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_CLOSE_BUTTON;
				window = _window;
			}
		}
	}
	return window;
}

void wl_pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x,
                             wl_fixed_t surface_y) {
	enum kinc_wl_decoration_focus focus = KINC_WL_DECORATION_FOCUS_MAIN;
	struct kinc_wl_window *window = kinc_wayland_window_from_surface(surface, &focus);
	struct kinc_wl_mouse *mouse = data;
	mouse->enter_serial = serial;
	window->decorations.focus = focus;
	if (window != NULL) {
		mouse->current_window = window->window_id;
	}
}

void wl_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
	enum kinc_wl_decoration_focus focus = KINC_WL_DECORATION_FOCUS_MAIN;
	struct kinc_wl_window *window = kinc_wayland_window_from_surface(surface, &focus);
	if (window != NULL) {
	}
}

#include <wayland-cursor.h>

void kinc_wayland_set_cursor(struct kinc_wl_mouse *mouse, const char *name) {
	if (!name)
		return;
	struct wl_cursor *cursor = wl_cursor_theme_get_cursor(wl_ctx.cursor_theme, name);
	if (!cursor) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland: No cursor found '%'.", name);
		return;
	}
	struct wl_cursor_image *image = cursor->images[0];
	if (!image)
		return;
	struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
	if (!buffer)
		return;

	wl_pointer_set_cursor(mouse->pointer, mouse->enter_serial, mouse->surface, image->hotspot_x, image->hotspot_y);
	wl_surface_attach(mouse->surface, buffer, 0, 0);
	wl_surface_damage(mouse->surface, 0, 0, image->width, image->height);
	wl_surface_commit(mouse->surface);
	mouse->previous_cursor_name = name;
}

void wl_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	struct kinc_wl_mouse *mouse = data;
	struct kinc_wl_window *window = &wl_ctx.windows[mouse->current_window];

	int x = wl_fixed_to_int(surface_x);
	int y = wl_fixed_to_int(surface_y);

	mouse->x = x;
	mouse->y = y;

	if (!window->decorations.server_side) {
		const char *cursor_name = "default";

		switch (window->decorations.focus) {
		case KINC_WL_DECORATION_FOCUS_MAIN:
			kinc_internal_mouse_trigger_move(mouse->current_window, x, y);
			break;
		case KINC_WL_DECORATION_FOCUS_TOP:
			if (y < KINC_WL_DECORATION_TOP_HEIGHT / 2)
				cursor_name = "n-resize";
			else
				cursor_name = "left_ptr";
			break;
		case KINC_WL_DECORATION_FOCUS_LEFT:
			if (y < KINC_WL_DECORATION_WIDTH)
				cursor_name = "nw-resize";
			else if (mouse->y > KINC_WL_DECORATION_TOP_HEIGHT - KINC_WL_DECORATION_WIDTH)
				cursor_name = "sw-resize";
			else
				cursor_name = "w-resize";
			break;
		case KINC_WL_DECORATION_FOCUS_RIGHT:
			if (y < KINC_WL_DECORATION_WIDTH)
				cursor_name = "ne-resize";
			else if (mouse->y > KINC_WL_DECORATION_RIGHT_HEIGHT - KINC_WL_DECORATION_WIDTH)
				cursor_name = "se-resize";
			else
				cursor_name = "e-resize";
			break;
		case KINC_WL_DECORATION_FOCUS_BOTTOM:
			if (x < 10)
				cursor_name = "sw-resize";
			else if (x > window->width + 10)
				cursor_name = "se-resize";
			else
				cursor_name = "s-resize";
			break;
		case KINC_WL_DECORATION_FOCUS_CLOSE_BUTTON:
			break;
		default:
			break;
		}

		if (mouse->previous_cursor_name != cursor_name) {
			kinc_wayland_set_cursor(mouse, cursor_name);
		}
	}
	else {
		kinc_internal_mouse_trigger_move(mouse->current_window, x, y);
	}
}

#include <linux/input-event-codes.h>

void wl_pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	struct kinc_wl_mouse *mouse = data;
	struct kinc_wl_window *window = &wl_ctx.windows[mouse->current_window];
	int kinc_button = button - BTN_MOUSE; // evdev codes should have the same order as Kinc buttons
	if (!window->decorations.server_side) {
		if (kinc_button == 0) {
			enum xdg_toplevel_resize_edge edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
			switch (window->decorations.focus) {
			case KINC_WL_DECORATION_FOCUS_MAIN:
				break;
			case KINC_WL_DECORATION_FOCUS_TOP:
				if (mouse->y > KINC_WL_DECORATION_TOP_HEIGHT / 2)
					edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
				else {
					xdg_toplevel_move(window->toplevel, wl_ctx.seat.seat, serial);
				}
				break;
			case KINC_WL_DECORATION_FOCUS_LEFT:
				if (mouse->y < KINC_WL_DECORATION_TOP_HEIGHT / 2)
					edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
				else if (mouse->y > KINC_WL_DECORATION_TOP_HEIGHT - (KINC_WL_DECORATION_TOP_HEIGHT / 2))
					edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
				else
					edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
				break;
			case KINC_WL_DECORATION_FOCUS_RIGHT:
				if (mouse->y < KINC_WL_DECORATION_TOP_HEIGHT / 2)
					edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
				else if (mouse->y > KINC_WL_DECORATION_RIGHT_HEIGHT - (KINC_WL_DECORATION_TOP_HEIGHT / 2))
					edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
				else
					edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
				break;
			case KINC_WL_DECORATION_FOCUS_BOTTOM:
				edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
				break;
			case KINC_WL_DECORATION_FOCUS_CLOSE_BUTTON:
				if (kinc_button == 0) {
					if (kinc_internal_call_close_callback(window->window_id)) {
						kinc_window_destroy(window->window_id);
						if (wl_ctx.num_windows <= 0) {
							// no windows left, stop
							kinc_stop();
						}
					}
				}
				break;
			default:
				break;
			}
			if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE) {
				xdg_toplevel_resize(window->toplevel, wl_ctx.seat.seat, serial, edges);
			}
		}
		else if (kinc_button == 1) {
			if (window->decorations.focus == KINC_WL_DECORATION_FOCUS_TOP) {
				xdg_toplevel_show_window_menu(window->toplevel, mouse->seat->seat, serial, mouse->x, mouse->y);
			}
		}
	}

	if (window->decorations.focus == KINC_WL_DECORATION_FOCUS_MAIN) {
		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			kinc_internal_mouse_trigger_press(mouse->current_window, kinc_button, mouse->x, mouse->y);
		}
		if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
			kinc_internal_mouse_trigger_release(mouse->current_window, kinc_button, mouse->x, mouse->y);
		}
	}
}

void wl_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
	struct kinc_wl_mouse *mouse = data;
	// FIXME: figure out what the other backends give as deltas
	int delta = wl_fixed_to_int(value);
	kinc_internal_mouse_trigger_scroll(mouse->current_window, delta);
}

void wl_pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {}

void wl_pointer_handle_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source) {}

void wl_pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {}

void wl_pointer_handle_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {}

void wl_pointer_handle_axis_value120(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t value120) {}

static const struct wl_pointer_listener wl_pointer_listener = {wl_pointer_handle_enter,         wl_pointer_handle_leave, wl_pointer_handle_motion,
                                                               wl_pointer_handle_button,        wl_pointer_handle_axis,
#ifdef WL_POINTER_FRAME_SINCE_VERSION
                                                               wl_pointer_handle_frame,
#endif
#ifdef WL_POINTER_AXIS_SOURCE_SINCE_VERSION
                                                               wl_pointer_handle_axis_source,
#endif
#ifdef WL_POINTER_AXIS_STOP_SINCE_VERSION
                                                               wl_pointer_handle_axis_stop,
#endif
#ifdef WL_POINTER_AXIS_DISCRETE_SINCE_VERSION
                                                               wl_pointer_handle_axis_discrete,
#endif
#ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
                                                               wl_pointer_handle_axis_value120
#endif
};

void zwp_relative_pointer_v1_handle_relative_motion(void *data, struct zwp_relative_pointer_v1 *zwp_relative_pointer_v1, uint32_t utime_hi, uint32_t utime_lo,
                                                    wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel) {
	struct kinc_wl_mouse *mouse = data;
	if (mouse->locked) {
		mouse->x += wl_fixed_to_int(dx);
		mouse->y += wl_fixed_to_int(dy);
		kinc_internal_mouse_trigger_move(mouse->current_window, mouse->x, mouse->y);
	}
}

static const struct zwp_relative_pointer_v1_listener zwp_relative_pointer_v1_listener = {
    zwp_relative_pointer_v1_handle_relative_motion,
};

#include <sys/mman.h>
#include <unistd.h>

void wl_keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
	switch (format) {
	case WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1: {
		char *mapStr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		if (mapStr == MAP_FAILED) {
			mapStr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapStr == MAP_FAILED) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to map wayland keymap.");
				close(fd);
				return;
			}
		}
		keyboard->keymap = wl_xkb.xkb_keymap_new_from_string(wl_ctx.xkb_context, mapStr, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
		munmap(mapStr, size);
		close(fd);
		keyboard->state = wl_xkb.xkb_state_new(keyboard->keymap);
		keyboard->ctrlDown = false;
		break;
	}
	default:
		close(fd);
		kinc_log(KINC_LOG_LEVEL_WARNING, "Unsupported wayland keymap format %i", format);
	}
}

void wl_keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	// struct kinc_wl_seat *seat = data;
	// struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
}

void wl_keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
	// struct kinc_wl_seat *seat = data;
	// struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
}

int xkb_to_kinc(xkb_keysym_t symbol);

void handle_paste(void *data, size_t data_size, void *user_data) {
	kinc_internal_paste_callback(data);
}

#include <sys/timerfd.h>

void wl_keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
	if (keyboard->keymap && keyboard->state) {
		xkb_keysym_t symbol = wl_xkb.xkb_state_key_get_one_sym(keyboard->state, key + 8);
		uint32_t character = wl_xkb.xkb_state_key_get_utf32(keyboard->state, key + 8);
		int kinc_key = xkb_to_kinc(symbol);
		if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			if (keyboard->ctrlDown && (symbol == XKB_KEY_c || symbol == XKB_KEY_C)) {
				char *text = kinc_internal_copy_callback();
				if (text != NULL) {
					kinc_wayland_set_selection(keyboard->seat, text, serial);
				}
			}
			else if (keyboard->ctrlDown && (symbol == XKB_KEY_x || symbol == XKB_KEY_X)) {
				char *text = kinc_internal_copy_callback();
				if (text != NULL) {
					kinc_wayland_set_selection(keyboard->seat, text, serial);
				}
			}
			else if (keyboard->ctrlDown && (symbol == XKB_KEY_v || symbol == XKB_KEY_V)) {
				if (keyboard->seat->current_selection_offer != NULL) {
					kinc_wl_data_offer_accept(keyboard->seat->current_selection_offer, handle_paste, NULL);
				}
			}
			kinc_internal_keyboard_trigger_key_down(kinc_key);
			if (character != 0) {
				kinc_internal_keyboard_trigger_key_press(character);
				if (wl_xkb.xkb_keymap_key_repeats(keyboard->keymap, key + 8) && keyboard->repeat_rate > 0) {
					struct itimerspec timer = {};
					keyboard->last_character = character;
					keyboard->last_key_code = key + 8;
					if (keyboard->repeat_rate > 1) {
						timer.it_interval.tv_nsec = 1000000000 / keyboard->repeat_rate;
					}
					else {
						timer.it_interval.tv_sec = 1;
					}

					timer.it_value.tv_sec = keyboard->repeat_delay / 1000;
					timer.it_value.tv_nsec = (keyboard->repeat_delay % 1000) * 1000000;

					timerfd_settime(keyboard->timerfd, 0, &timer, NULL);
				}
			}
		}
		if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
			if (key + 8 == keyboard->last_key_code) {
				keyboard->last_key_code = keyboard->last_character = -1;
			}

			kinc_internal_keyboard_trigger_key_up(kinc_key);
		}
	}
}

void wl_keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
                                  uint32_t mods_locked, uint32_t group) {
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
	if (keyboard->keymap && keyboard->state) {
		wl_xkb.xkb_state_update_mask(keyboard->state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
		wl_xkb.xkb_state_serialize_mods(keyboard->state,
		                                XKB_STATE_MODS_DEPRESSED | XKB_STATE_LAYOUT_DEPRESSED | XKB_STATE_MODS_LATCHED | XKB_STATE_LAYOUT_LATCHED);
		keyboard->ctrlDown = wl_xkb.xkb_state_mod_name_is_active(keyboard->state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE) > 0;
	}
}

void wl_keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
	keyboard->repeat_rate = rate;
	keyboard->repeat_delay = delay;
	kinc_log(KINC_LOG_LEVEL_INFO, "Keyboard repeat rate: %i, delay: %i", rate, delay);
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    wl_keyboard_handle_keymap,      wl_keyboard_handle_enter, wl_keyboard_handle_leave, wl_keyboard_handle_key, wl_keyboard_handle_modifiers,
#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    wl_keyboard_handle_repeat_info,
#endif
};

void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
	struct kinc_wl_seat *seat = data;
	seat->capabilities = capabilities;
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		seat->keyboard.keyboard = wl_seat_get_keyboard(wl_seat);
		seat->keyboard.seat = seat;
		seat->keyboard.timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
		seat->keyboard.repeat_delay = 0;
		seat->keyboard.repeat_rate = 0;
		seat->keyboard.last_key_code = -1;
		wl_keyboard_add_listener(seat->keyboard.keyboard, &wl_keyboard_listener, &seat->keyboard);
	}
	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		seat->mouse.pointer = wl_seat_get_pointer(wl_seat);
		seat->mouse.surface = wl_compositor_create_surface(wl_ctx.compositor);
		seat->mouse.seat = seat;
		wl_pointer_add_listener(seat->mouse.pointer, &wl_pointer_listener, &seat->mouse);
		if (wl_ctx.relative_pointer_manager) {
			seat->mouse.relative = zwp_relative_pointer_manager_v1_get_relative_pointer(wl_ctx.relative_pointer_manager, seat->mouse.pointer);
			zwp_relative_pointer_v1_add_listener(seat->mouse.relative, &zwp_relative_pointer_v1_listener, &seat->mouse);
		}
	}
	if (capabilities & WL_SEAT_CAPABILITY_TOUCH) {
		seat->touch = wl_seat_get_touch(wl_seat);
	}
}

void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
	struct kinc_wl_seat *seat = data;
	snprintf(seat->name, sizeof(seat->name), "%s", name);
}

static const struct wl_seat_listener wl_seat_listener = {
    wl_seat_capabilities,
    wl_seat_name,
};

void wl_data_source_handle_target(void *data, struct wl_data_source *wl_data_source, const char *mime_type) {}

void wl_data_source_handle_send(void *data, struct wl_data_source *wl_data_source, const char *mime_type, int32_t fd) {
	struct kinc_wl_data_source *data_source = wl_data_source_get_user_data(wl_data_source);
	if (write(fd, data_source->data, data_source->data_size) < data_source->data_size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to write all data for data source");
	}
	close(fd);
}

void wl_data_source_handle_cancelled(void *data, struct wl_data_source *wl_data_source) {}

void wl_data_source_handle_dnd_drop_performed(void *data, struct wl_data_source *wl_data_source) {}

void wl_data_source_handle_dnd_finished(void *data, struct wl_data_source *wl_data_source) {}

void wl_data_source_handle_action(void *data, struct wl_data_source *wl_data_source, uint32_t dnd_action) {}

static const struct wl_data_source_listener wl_data_source_listener = {
    wl_data_source_handle_target,       wl_data_source_handle_send,   wl_data_source_handle_cancelled, wl_data_source_handle_dnd_drop_performed,
    wl_data_source_handle_dnd_finished, wl_data_source_handle_action,
};

struct kinc_wl_data_source *kinc_wl_create_data_source(struct kinc_wl_seat *seat, const char *mime_types[], int num_mime_types, void *data, size_t data_size) {
	struct kinc_wl_data_source *data_source = malloc(sizeof *data_source);
	data_source->source = wl_data_device_manager_create_data_source(wl_ctx.data_device_manager);
	data_source->data = data;
	data_source->data_size = data_size;
	data_source->mime_types = mime_types;
	data_source->num_mime_types = num_mime_types;

	for (int i = 0; i < num_mime_types; i++) {
		wl_data_source_offer(data_source->source, mime_types[i]);
	}
	// wl_data_source_set_actions(data_source->source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
	wl_data_source_set_user_data(data_source->source, data_source);
	wl_data_source_add_listener(data_source->source, &wl_data_source_listener, data_source);
	return data_source;
}

void kinc_wl_data_source_destroy(struct kinc_wl_data_source *data_source) {}

void wl_data_offer_handle_offer(void *data, struct wl_data_offer *wl_data_offer, const char *mime_type) {
	struct kinc_wl_data_offer *offer = wl_data_offer_get_user_data(wl_data_offer);
	if (offer != NULL) {
		offer->mime_type_count++;
		offer->mime_types = realloc(offer->mime_types, offer->mime_type_count * sizeof(const char *));
		char *copy = malloc(strlen(mime_type) + 1);
		strcpy(copy, mime_type);
		offer->mime_types[offer->mime_type_count - 1] = copy;
	}
}

void wl_data_offer_handle_source_actions(void *data, struct wl_data_offer *wl_data_offer, uint32_t source_actions) {
	struct kinc_wl_data_offer *offer = wl_data_offer_get_user_data(wl_data_offer);
	offer->source_actions = source_actions;
}

void wl_data_offer_handle_action(void *data, struct wl_data_offer *wl_data_offer, uint32_t dnd_action) {
	struct kinc_wl_data_offer *offer = wl_data_offer_get_user_data(wl_data_offer);
	offer->dnd_action = dnd_action;
}

static const struct wl_data_offer_listener wl_data_offer_listener = {
    wl_data_offer_handle_offer,
    wl_data_offer_handle_source_actions,
    wl_data_offer_handle_action,
};

void kinc_wl_init_data_offer(struct wl_data_offer *id) {
	struct kinc_wl_data_offer *offer = malloc(sizeof *offer);
	memset(offer, 0, sizeof *offer);
	offer->id = id;
	offer->mime_type_count = 0;
	offer->mime_types = NULL;

	wl_data_offer_set_user_data(id, offer);
	wl_data_offer_add_listener(id, &wl_data_offer_listener, offer);
}

void kinc_wl_data_offer_accept(struct kinc_wl_data_offer *offer, void (*callback)(void *data, size_t data_size, void *user_data), void *user_data) {
	offer->callback = callback;
	offer->user_data = user_data;

	int fds[2];
	if (pipe(fds) != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to create pipe to accept wayland data offer (errno=%i)", errno);
		return;
	}
	wl_data_offer_receive(offer->id, "text/plain", fds[1]);
	close(fds[1]);

	wl_display_roundtrip(wl_ctx.display);

	offer->read_fd = fds[0];

	struct kinc_wl_data_offer **queue = &wl_ctx.data_offer_queue;

	while (*queue != NULL)
		queue = &(*queue)->next;
	*queue = offer;
}

void kinc_wl_destroy_data_offer(struct kinc_wl_data_offer *offer) {
	wl_data_offer_destroy(offer->id);
	if (offer->buffer != NULL) {
		free(offer->buffer);
	}
	for (int i = 0; i < offer->mime_type_count; i++) {
		free(offer->mime_types[i]);
	}
	free(offer->mime_types);
	free(offer);
}

void wl_data_device_handle_data_offer(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id) {
	// struct kinc_wl_seat *seat = data;
	kinc_wl_init_data_offer(id);
}

void wl_data_device_handle_enter(void *data, struct wl_data_device *wl_data_device, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y,
                                 struct wl_data_offer *id) {
	struct kinc_wl_seat *seat = data;
	seat->current_dnd_offer = wl_data_offer_get_user_data(id);
	wl_data_offer_set_actions(id, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
}

void wl_data_device_handle_leave(void *data, struct wl_data_device *wl_data_device) {
	struct kinc_wl_seat *seat = data;
	kinc_wl_destroy_data_offer(seat->current_dnd_offer);
	seat->current_dnd_offer = NULL;
}

void wl_data_device_handle_motion(void *data, struct wl_data_device *wl_data_device, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	// struct kinc_wl_seat *seat = data;
}

static void dnd_callback(void *data, size_t data_size, void *user_data) {
	char *str = data;
	if (strncmp(data, "file://", strlen("file://")) == 0) {
		str += strlen("file://");
	}
	size_t wide_size = mbstowcs(NULL, str, 0) + 1;
	wchar_t *dest = malloc(wide_size * sizeof(wchar_t));
	mbstowcs(dest, str, wide_size);
	kinc_internal_drop_files_callback(dest);
	free(dest);
}

void wl_data_device_handle_drop(void *data, struct wl_data_device *wl_data_device) {
	struct kinc_wl_seat *seat = data;
	if (seat->current_dnd_offer != NULL) {
		kinc_wl_data_offer_accept(seat->current_dnd_offer, dnd_callback, NULL);
	}
}

void wl_data_device_handle_selection(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id) {
	struct kinc_wl_seat *seat = data;
	if (seat->current_selection_offer != NULL && seat->current_selection_offer->id != id) {
		kinc_wl_destroy_data_offer(seat->current_selection_offer);
		seat->current_selection_offer = NULL;
	}

	if (id != NULL) {
		seat->current_selection_offer = wl_data_offer_get_user_data(id);
	}
}

static const struct wl_data_device_listener wl_data_device_listener = {
    wl_data_device_handle_data_offer, wl_data_device_handle_enter, wl_data_device_handle_leave,
    wl_data_device_handle_motion,     wl_data_device_handle_drop,  wl_data_device_handle_selection,
};

void kinc_wl_tablet_tool_destroy(struct kinc_wl_tablet_tool *tool) {
	zwp_tablet_tool_v2_destroy(tool->id);
	free(tool);
}

void zwp_tablet_tool_v2_handle_type(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t tool_type) {
	struct kinc_wl_tablet_tool *tool = data;
	tool->type = tool_type;
}

#ifdef KINC_LITTLE_ENDIAN
#define HI_LO_TO_64(hi, lo) (uint64_t) lo | ((uint64_t)hi << 32)
#else
#define HI_LO_TO_64(hi, lo) (uint64_t) hi | ((uint64_t)lo << 32)
#endif

void zwp_tablet_tool_v2_handle_hardware_serial(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t hardware_serial_hi,
                                               uint32_t hardware_serial_lo) {
	struct kinc_wl_tablet_tool *tool = data;
	tool->hardware_serial = HI_LO_TO_64(hardware_serial_hi, hardware_serial_lo);
}

void zwp_tablet_tool_v2_handle_hardware_id_wacom(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t hardware_id_hi, uint32_t hardware_id_lo) {
	struct kinc_wl_tablet_tool *tool = data;
	tool->hardware_id_wacom = HI_LO_TO_64(hardware_id_hi, hardware_id_lo);
}

void zwp_tablet_tool_v2_handle_capability(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t capability) {
	struct kinc_wl_tablet_tool *tool = data;
	tool->capabilities |= capability;
}

void zwp_tablet_tool_v2_handle_done(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2) {
	struct kinc_wl_tablet_tool *tool = data;
	switch (tool->type) {
	case ZWP_TABLET_TOOL_V2_TYPE_PEN:
		tool->press = kinc_internal_pen_trigger_press;
		tool->move = kinc_internal_pen_trigger_move;
		tool->release = kinc_internal_pen_trigger_release;
		break;
	case ZWP_TABLET_TOOL_V2_TYPE_ERASER:
		tool->press = kinc_internal_eraser_trigger_press;
		tool->move = kinc_internal_eraser_trigger_move;
		tool->release = kinc_internal_eraser_trigger_release;
		break;
	default:
		break;
	}
}

void zwp_tablet_tool_v2_handle_removed(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2) {
	struct kinc_wl_tablet_tool *tool = data;
	struct kinc_wl_tablet_seat *seat = tool->seat;
	struct kinc_wl_tablet_tool **tools = &seat->tablet_tools;
	while (*tools != NULL) {
		struct kinc_wl_tablet_tool *current = *tools;
		struct kinc_wl_tablet_tool **next = &current->next;

		if (current == tool) {
			*tools = *next;

			break;
		}
		else {
			tools = next;
		}
	}

	kinc_wl_tablet_tool_destroy(tool);
}

void zwp_tablet_tool_v2_handle_proximity_in(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t serial, struct zwp_tablet_v2 *tablet,
                                            struct wl_surface *surface) {
	struct kinc_wl_tablet_tool *tool = data;
	enum kinc_wl_decoration_focus focus;
	struct kinc_wl_window *window = kinc_wayland_window_from_surface(surface, &focus);
	tool->current_window = window->window_id;
}

void zwp_tablet_tool_v2_handle_proximity_out(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2) {
	struct kinc_wl_tablet_tool *tool = data;
	tool->current_window = -1;
}

void zwp_tablet_tool_v2_handle_down(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t serial) {
	struct kinc_wl_tablet_tool *tool = data;
	if (tool->current_window >= 0 && tool->press) {
		tool->press(tool->current_window, tool->x, tool->y, tool->current_pressure);
	}
}

void zwp_tablet_tool_v2_handle_up(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2) {
	struct kinc_wl_tablet_tool *tool = data;
	if (tool->current_window >= 0 && tool->release) {
		tool->release(tool->current_window, tool->x, tool->y, tool->current_pressure);
	}
}

void zwp_tablet_tool_v2_handle_motion(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, wl_fixed_t x, wl_fixed_t y) {
	struct kinc_wl_tablet_tool *tool = data;
	tool->x = wl_fixed_to_int(x);
	tool->y = wl_fixed_to_int(y);
	if (tool->current_window >= 0 && tool->move) {
		tool->move(tool->current_window, tool->x, tool->y, tool->current_pressure);
	}
}

void zwp_tablet_tool_v2_handle_pressure(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t pressure) {
	struct kinc_wl_tablet_tool *tool = data;
	// TODO: verify what the other backends give
	tool->current_pressure = (float)pressure / 65535.f;
}

void zwp_tablet_tool_v2_handle_distance(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t distance) {
	struct kinc_wl_tablet_tool *tool = data;
	tool->current_distance = (float)distance / 65535.f;
}

void zwp_tablet_tool_v2_handle_tilt(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, wl_fixed_t tilt_x, wl_fixed_t tilt_y) {}

void zwp_tablet_tool_v2_handle_rotation(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, wl_fixed_t degrees) {}

void zwp_tablet_tool_v2_handle_slider(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, int32_t position) {}

void zwp_tablet_tool_v2_handle_wheel(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, wl_fixed_t degrees, int32_t clicks) {}

void zwp_tablet_tool_v2_handle_button(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t serial, uint32_t button, uint32_t state) {}

void zwp_tablet_tool_v2_handle_frame(void *data, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t time) {}

static const struct zwp_tablet_tool_v2_listener zwp_tablet_tool_v2_listener = {
    zwp_tablet_tool_v2_handle_type,
    zwp_tablet_tool_v2_handle_hardware_serial,
    zwp_tablet_tool_v2_handle_hardware_id_wacom,
    zwp_tablet_tool_v2_handle_capability,
    zwp_tablet_tool_v2_handle_done,
    zwp_tablet_tool_v2_handle_removed,
    zwp_tablet_tool_v2_handle_proximity_in,
    zwp_tablet_tool_v2_handle_proximity_out,
    zwp_tablet_tool_v2_handle_down,
    zwp_tablet_tool_v2_handle_up,
    zwp_tablet_tool_v2_handle_motion,
    zwp_tablet_tool_v2_handle_pressure,
    zwp_tablet_tool_v2_handle_distance,
    zwp_tablet_tool_v2_handle_tilt,
    zwp_tablet_tool_v2_handle_rotation,
    zwp_tablet_tool_v2_handle_slider,
    zwp_tablet_tool_v2_handle_wheel,
    zwp_tablet_tool_v2_handle_button,
    zwp_tablet_tool_v2_handle_frame,
};

void zwp_tablet_seat_v2_handle_tablet_added(void *data, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, struct zwp_tablet_v2 *id) {
	struct kinc_wl_tablet *tablet = malloc(sizeof *tablet);
	tablet->id = id;
	tablet->seat = zwp_tablet_seat_v2_get_user_data(zwp_tablet_seat_v2);
	tablet->next = tablet->seat->tablets;
	tablet->seat->tablets = tablet;

	// zwp_tablet_v2_add_listener(tablet->id, NULL, tablet);
}

void zwp_tablet_seat_v2_handle_tool_added(void *data, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, struct zwp_tablet_tool_v2 *id) {
	struct kinc_wl_tablet_tool *tool = malloc(sizeof *tool);
	tool->id = id;
	tool->seat = zwp_tablet_seat_v2_get_user_data(zwp_tablet_seat_v2);
	tool->next = tool->seat->tablet_tools;
	tool->seat->tablet_tools = tool;

	zwp_tablet_tool_v2_add_listener(tool->id, &zwp_tablet_tool_v2_listener, tool);
}

void zwp_tablet_seat_v2_handle_pad_added(void *data, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, struct zwp_tablet_pad_v2 *id) {}

static const struct zwp_tablet_seat_v2_listener zwp_tablet_seat_v2_listener = {
    zwp_tablet_seat_v2_handle_tablet_added,
    zwp_tablet_seat_v2_handle_tool_added,
    zwp_tablet_seat_v2_handle_pad_added,
};

static void wl_registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		wl_ctx.compositor = wl_registry_bind(wl_ctx.registry, name, &wl_compositor_interface, 4);
	}
	else if (strcmp(interface, wl_shm_interface.name) == 0) {
		wl_ctx.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}
	else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
		wl_ctx.subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
	}
	else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
		wl_ctx.viewporter = wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
	}
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wl_ctx.xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wl_ctx.xdg_wm_base, &xdg_wm_base_listener, NULL);
	}
	else if (strcmp(interface, wl_seat_interface.name) == 0) {
		if (wl_ctx.seat.seat) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Multi-seat configurations not supported");
			return;
		}
		wl_ctx.seat.seat = wl_registry_bind(registry, name, &wl_seat_interface, version);

		wl_seat_add_listener(wl_ctx.seat.seat, &wl_seat_listener, &wl_ctx.seat);
		if (wl_ctx.data_device_manager != NULL) {
			wl_ctx.seat.data_device = wl_data_device_manager_get_data_device(wl_ctx.data_device_manager, wl_ctx.seat.seat);
			wl_data_device_add_listener(wl_ctx.seat.data_device, &wl_data_device_listener, &wl_ctx.seat);
		}
	}
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		int display_index = -1;
		for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
			if (wl_ctx.displays[i].output == NULL) {
				display_index = i;
				break;
			}
		}
		if (display_index == -1) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Too much displays (maximum is %i)", MAXIMUM_DISPLAYS);
		}
		else {
			struct kinc_wl_display *display = &wl_ctx.displays[display_index];
			display->output = wl_registry_bind(registry, name, &wl_output_interface, 2);
			display->scale = 1;
			wl_output_set_user_data(display->output, display);
			wl_output_add_listener(display->output, &wl_output_listener, display);
			wl_ctx.num_displays++;
		}
	}
	else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		wl_ctx.decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
	}
	else if (strcmp(interface, wl_data_device_manager_interface.name) == 0) {
		wl_ctx.data_device_manager = wl_registry_bind(registry, name, &wl_data_device_manager_interface, 3);
		if (wl_ctx.seat.seat != NULL) {
			wl_ctx.seat.data_device = wl_data_device_manager_get_data_device(wl_ctx.data_device_manager, wl_ctx.seat.seat);
			wl_data_device_add_listener(wl_ctx.seat.data_device, &wl_data_device_listener, &wl_ctx.seat);
		}
	}
	else if (strcmp(interface, zwp_tablet_manager_v2_interface.name) == 0) {
		wl_ctx.tablet_manager = wl_registry_bind(registry, name, &zwp_tablet_manager_v2_interface, 1);
		if (wl_ctx.seat.seat != NULL) {
			wl_ctx.seat.tablet_seat.seat = zwp_tablet_manager_v2_get_tablet_seat(wl_ctx.tablet_manager, wl_ctx.seat.seat);
			zwp_tablet_seat_v2_add_listener(wl_ctx.seat.tablet_seat.seat, &zwp_tablet_seat_v2_listener, &wl_ctx.seat.tablet_seat);
		}
	}
	else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0) {
		wl_ctx.pointer_constraints = wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);
	}
	else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
		wl_ctx.relative_pointer_manager = wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
	}
}

static void wl_registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
	// TODO: handle output removal
}

static const struct wl_registry_listener registry_listener = {
    wl_registry_handle_global,
    wl_registry_handle_global_remove,
};

bool kinc_wayland_init() {
	if (!kinc_wayland_load_procs()) {
		return false;
	}

	wl_ctx.xkb_context = wl_xkb.xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	wl_ctx.display = wl_display_connect(NULL);
	if (!wl_ctx.display) {
		return false;
	}
	wl_ctx.registry = wl_display_get_registry(wl_ctx.display);
	wl_registry_add_listener(wl_ctx.registry, &registry_listener, NULL);
	wl_display_dispatch(wl_ctx.display);
	wl_display_roundtrip(wl_ctx.display);
	wl_display_roundtrip(wl_ctx.display);

	if (wl_ctx.seat.mouse.pointer && wl_ctx.shm) {
		const char *cursor_theme = getenv("XCURSOR_THEME");
		const char *cursor_size_str = getenv("XCURSOR_SIZE");
		int cursor_size = 32;

		if (cursor_size_str) {
			char *end_ptr;
			long size = strtol(cursor_size_str, &end_ptr, 10);
			if (!(*end_ptr) && size > 0 && size < INT32_MAX) {
				cursor_size = (int)size;
			}
		}

		wl_ctx.cursor_theme = wl_cursor_theme_load(cursor_theme, cursor_size, wl_ctx.shm);
	}

	return true;
}

void kinc_wayland_shutdown() {
	wl_display_disconnect(wl_ctx.display);
	wl_xkb.xkb_context_unref(wl_ctx.xkb_context);
}

void kinc_wayland_set_selection(struct kinc_wl_seat *seat, const char *text, int serial) {
	static const char *mime_types[] = {"text/plain"};
	char *copy = malloc(strlen(text) + 1);
	strcpy(copy, text);
	struct kinc_wl_data_source *data_source = kinc_wl_create_data_source(seat, mime_types, sizeof mime_types / sizeof mime_types[0], copy, strlen(text));
	wl_data_device_set_selection(seat->data_device, data_source->source, serial);
}

void kinc_wayland_copy_to_clipboard(const char *text) {}

#define READ_SIZE 64

static bool flush_display(void) {
	while (wl_display_flush(wl_ctx.display) == -1) {
		if (errno != EAGAIN)
			return false;

		struct pollfd fd = {wl_display_get_fd(wl_ctx.display), POLLOUT};

		while (poll(&fd, 1, 10) == -1) {
			if (errno != EINTR && errno != EAGAIN)
				return false;
		}
	}

	return true;
}

bool kinc_wayland_handle_messages() {
	while (wl_display_prepare_read(wl_ctx.display) != 0)
		wl_display_dispatch_pending(wl_ctx.display);
	if (!flush_display()) {
		// Something went wrong, abort.
		wl_display_cancel_read(wl_ctx.display);

		for (int i = 0; i < wl_ctx.num_windows; i++) {
			kinc_window_destroy(i);
		}

		return true;
	}

	struct pollfd fds[] = {
	    {wl_display_get_fd(wl_ctx.display), POLLIN},
	    {wl_ctx.seat.keyboard.timerfd, POLLIN},
	};

	if (poll(fds, sizeof(fds) / sizeof(struct pollfd), 10) <= 0) {
		wl_display_cancel_read(wl_ctx.display);
		return false;
	}

	if (fds[0].revents & POLLIN) {
		wl_display_read_events(wl_ctx.display);
		wl_display_dispatch_pending(wl_ctx.display);
	}
	else {
		wl_display_cancel_read(wl_ctx.display);
	}

	if (fds[1].revents & POLLIN) {
		uint64_t repeats;

		if (read(wl_ctx.seat.keyboard.timerfd, &repeats, sizeof(repeats)) == 8) {
			if (wl_ctx.seat.keyboard.last_key_code != -1) {
				for (uint64_t i = 0; i < repeats; i++) {
					kinc_internal_keyboard_trigger_key_press(wl_ctx.seat.keyboard.last_character);
				}
			}
		}
	}

	struct kinc_wl_data_offer **offer = &wl_ctx.data_offer_queue;
	while (*offer != NULL) {
		struct kinc_wl_data_offer *current = *offer;
		struct kinc_wl_data_offer **next = &current->next;
		if (current->buf_pos + READ_SIZE > current->buf_size) {
			current->buffer = realloc(current->buffer, current->buf_size + READ_SIZE);
			current->buf_size += READ_SIZE;
		}

		ssize_t n = read(current->read_fd, current->buffer + current->buf_pos, READ_SIZE);
		if (n <= 0) {
			*offer = *next;
			close(current->read_fd);

			current->callback(current->buffer, current->buf_pos, current->user_data);

			free(current->buffer);
			current->buffer = NULL;
			current->buf_pos = 0;
			current->buf_size = 0;
			current->read_fd = 0;
			current->next = NULL;
		}
		else {
			current->buf_pos += n;
			offer = next;
		}
	}
	return false;
}

#undef READ_SIZE

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
VkResult kinc_wayland_vulkan_create_surface(VkInstance instance, int window_index, VkSurfaceKHR *surface) {
	VkWaylandSurfaceCreateInfoKHR info = {0};
	info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.display = wl_ctx.display;
	info.surface = wl_ctx.windows[window_index].surface;
	return vkCreateWaylandSurfaceKHR(instance, &info, NULL, surface);
}

#include <assert.h>

void kinc_wayland_vulkan_get_instance_extensions(const char **names, int *index, int max) {
	assert(*index + 1 < max);
	names[(*index)++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
}

VkBool32 kinc_wayland_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
	return vkGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, wl_ctx.display);
}

void zwp_locked_pointer_v1_handle_locked(void *data, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1) {
	struct kinc_wl_mouse *mouse = data;
	mouse->locked = true;
}

void zwp_locked_pointer_v1_handle_unlocked(void *data, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1) {
	struct kinc_wl_mouse *mouse = data;
	mouse->locked = false;
}

static const struct zwp_locked_pointer_v1_listener zwp_locked_pointer_v1_listener = {
    zwp_locked_pointer_v1_handle_locked,
    zwp_locked_pointer_v1_handle_unlocked,
};

void kinc_wl_mouse_show() {
	kinc_wayland_set_cursor(&wl_ctx.seat.mouse, "default"); // TODO: should use the last set cursor instead
}

void kinc_wl_mouse_hide() {
	wl_pointer_set_cursor(wl_ctx.seat.mouse.pointer, wl_ctx.seat.mouse.serial, NULL, 0, 0);
}

void kinc_wl_mouse_lock(int window) {
	struct kinc_wl_mouse *mouse = &wl_ctx.seat.mouse;
	struct wl_region *region = wl_compositor_create_region(wl_ctx.compositor);
	wl_region_add(region, mouse->x, mouse->y, 0, 0);
	mouse->lock = zwp_pointer_constraints_v1_lock_pointer(wl_ctx.pointer_constraints, wl_ctx.windows[window].surface, wl_ctx.seat.mouse.pointer, region,
	                                                      ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
	zwp_locked_pointer_v1_add_listener(mouse->lock, &zwp_locked_pointer_v1_listener, mouse);
}

void kinc_wl_mouse_unlock(void) {
	zwp_locked_pointer_v1_destroy(wl_ctx.seat.mouse.lock);
	wl_ctx.seat.mouse.lock = NULL;
	wl_ctx.seat.mouse.locked = false;
	kinc_wl_mouse_show();
}

bool kinc_wl_mouse_can_lock(void) {
	return true;
}

void kinc_wl_mouse_set_cursor(int cursorIndex) {
	const char *name;
	switch (cursorIndex) {
	case 0: {
		name = "arrow";
		break;
	}
	case 1: {
		name = "hand1";
		break;
	}
	case 2: {
		name = "xterm";
		break;
	}
	case 3: {
		name = "sb_h_double_arrow";
		break;
	}
	case 4: {
		name = "sb_v_double_arrow";
		break;
	}
	case 5: {
		name = "top_right_corner";
		break;
	}
	case 6: {
		name = "bottom_right_corner";
		break;
	}
	case 7: {
		name = "top_left_corner";
		break;
	}
	case 8: {
		name = "bottom_left_corner";
		break;
	}
	case 9: {
		name = "grab";
		break;
	}
	case 10: {
		name = "grabbing";
		break;
	}
	case 11: {
		name = "not-allowed";
		break;
	}
	case 12: {
		name = "watch";
		break;
	}
	case 13: {
		name = "crosshair";
		break;
	}
	default: {
		name = "arrow";
		break;
	}
	}
	if (!wl_ctx.seat.mouse.hidden) {
		kinc_wayland_set_cursor(&wl_ctx.seat.mouse, name);
	}
}

void kinc_wl_mouse_set_position(int window_index, int x, int y) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland: cannot set the mouse position.");
}

void kinc_wl_mouse_get_position(int window_index, int *x, int *y) {
	*x = wl_ctx.seat.mouse.x;
	*y = wl_ctx.seat.mouse.y;
}
