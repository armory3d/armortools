
#pragma once

#include "iron_gpu.h"

extern gpu_buffer_t *const_data_screen_aligned_vb;
extern gpu_buffer_t *const_data_screen_aligned_ib;
extern gpu_buffer_t *const_data_skydome_vb;
extern gpu_buffer_t *const_data_skydome_ib;

void const_data_create_screen_aligned_data(void);
void const_data_create_skydome_data(void);

extern const char          iron_font_13_x0[];
extern const char          iron_font_13_y0[];
extern const char          iron_font_13_x1[];
extern const char          iron_font_13_y1[];
extern const char          iron_font_13_xoff[];
extern const char          iron_font_13_yoff[];
extern const float         iron_font_13_xadvance[];
extern const unsigned char iron_font_13_pixels[];
