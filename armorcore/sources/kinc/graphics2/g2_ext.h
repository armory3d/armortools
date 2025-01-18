#pragma once

#include "iron_array.h"

void kinc_g2_fill_circle(float cx, float cy, float radius, int segments);
void kinc_g2_draw_circle(float cx, float cy, float radius, int segments, float strength);
void kinc_g2_draw_cubic_bezier(f32_array_t *x, f32_array_t *y, int segments, float strength);
