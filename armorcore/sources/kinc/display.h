#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file display.h
    \brief Provides information for the active displays.
*/

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
