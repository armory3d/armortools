#pragma once

#include <kinc/global.h>
#include <stdint.h>

/*! \file color.h
    \brief Provides some utility functionality for handling 32 bit ARGB color values.
*/

void kinc_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha);

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
