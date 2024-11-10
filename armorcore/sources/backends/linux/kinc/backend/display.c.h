#include "funcs.h"
#include <kinc/display.h>

void kinc_display_init() {
	static bool display_initialized = false;
	if (display_initialized) {
		return;
	}

	kinc_linux_init_procs();
	if (procs.display_init != NULL) {
		procs.display_init();
		display_initialized = true;
	}
}

kinc_display_mode_t kinc_display_available_mode(int display, int mode) {
	return procs.display_available_mode(display, mode);
}

int kinc_display_count_available_modes(int display) {
	return procs.display_count_available_modes(display);
}

bool kinc_display_available(int display) {
	return procs.display_available(display);
}

const char *kinc_display_name(int display) {
	return procs.display_name(display);
}

kinc_display_mode_t kinc_display_current_mode(int display) {
	return procs.display_current_mode(display);
}

int kinc_primary_display(void) {
	return procs.display_primary();
}

int kinc_count_displays(void) {
	return procs.count_displays();
}
