#include "wayland.h"

void kinc_wayland_display_init(void) {
	// This is a no-op because displays are already registered in kinc_wayland_init,
	// which should be called before this function is ever invoked
}

int kinc_wayland_display_primary(void) {
	return 0; // TODO
}

int kinc_wayland_count_displays(void) {
	return wl_ctx.num_displays;
}

bool kinc_wayland_display_available(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS) {
		return false;
	}
	struct kinc_wl_display *display = &wl_ctx.displays[display_index];
	return display->output != NULL;
}

const char *kinc_wayland_display_name(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS)
		display_index = 0;
	struct kinc_wl_display *display = &wl_ctx.displays[display_index];
	return display->name;
}

kinc_display_mode_t kinc_wayland_display_current_mode(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS)
		display_index = 0;
	struct kinc_wl_display *display = &wl_ctx.displays[display_index];
	return display->modes[display->current_mode];
}

int kinc_wayland_display_count_available_modes(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS)
		display_index = 0;
	struct kinc_wl_display *display = &wl_ctx.displays[display_index];
	return display->num_modes;
}

kinc_display_mode_t kinc_wayland_display_available_mode(int display_index, int mode_index) {
	if (display_index >= MAXIMUM_DISPLAYS)
		display_index = 0;
	struct kinc_wl_display *display = &wl_ctx.displays[display_index];
	if (mode_index >= display->num_modes)
		mode_index = 0;
	return display->modes[mode_index];
}