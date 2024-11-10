#pragma once

void arm_g2_fill_circle(float cx, float cy, float radius, int segments);
void arm_g2_draw_circle(float cx, float cy, float radius, int segments, float strength);
void arm_g2_draw_cubic_bezier(float *x, float *y, int segments, float strength);
