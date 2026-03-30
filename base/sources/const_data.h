
#pragma once

#include "iron_gpu.h"

extern gpu_buffer_t *const_data_screen_aligned_vb;
extern gpu_buffer_t *const_data_screen_aligned_ib;
extern gpu_buffer_t *const_data_skydome_vb;
extern gpu_buffer_t *const_data_skydome_ib;

void const_data_create_screen_aligned_data(void);
void const_data_create_skydome_data(void);
