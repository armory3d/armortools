#include <kinc/display.h>

void kinc_display_init(void) {}

int kinc_primary_display(void) {
	return 0;
}

int kinc_count_displays(void) {
	return 1;
}

bool kinc_display_available(int display_index) {
	return false;
}

const char *kinc_display_name(int display_index) {
	return "Browser";
}

kinc_display_mode_t kinc_display_current_mode(int display_index) {
	kinc_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = 800;
	mode.height = 600;
	mode.pixels_per_inch = 96;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	return mode;
}

int kinc_display_count_available_modes(int display_index) {
	return 1;
}

kinc_display_mode_t kinc_display_available_mode(int display_index, int mode_index) {
	kinc_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = 800;
	mode.height = 600;
	mode.pixels_per_inch = 96;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	return mode;
}
