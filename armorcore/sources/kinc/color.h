#pragma once

#include <kinc/global.h>
#include <stdint.h>

/*! \file color.h
    \brief Provides some utility functionality for handling 32 bit ARGB color values.
*/

void kinc_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha);

#define KINC_COLOR_BLACK 0xff000000
#define KINC_COLOR_WHITE 0xffffffff
#define KINC_COLOR_RED 0xffff0000
#define KINC_COLOR_BLUE 0xff0000ff
#define KINC_COLOR_GREEN 0xff00ff00
#define KINC_COLOR_MAGENTA 0xffff00ff
#define KINC_COLOR_YELLOW 0xffffff00
#define KINC_COLOR_CYAN 0xff00ffff

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

void kinc_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha) {
	*alpha = ((color & 0xff000000) >> 24) / 255.0f;
	*red = ((color & 0x00ff0000) >> 16) / 255.0f;
	*green = ((color & 0x0000ff00) >> 8) / 255.0f;
	*blue = (color & 0x000000ff) / 255.0f;
}

#endif
