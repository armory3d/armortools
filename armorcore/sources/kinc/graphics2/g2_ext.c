#include "g2_ext.h"

#include <math.h>
#include "g2.h"
#include "../../iron_vec2.h"

#define MATH_PI 3.14159265358979323846

void arm_g2_fill_circle(float cx, float cy, float radius, int segments) {
	if (segments <= 0) {
		segments = (int)floor(10 * sqrtf(radius));
	}

	float theta = 2.0 * (float)MATH_PI / segments;
	float c = cosf(theta);
	float s = sinf(theta);

	float x = radius;
	float y = 0.0;

	for (int n = 0; n < segments; ++n) {
		float px = x + cx;
		float py = y + cy;

		float t = x;
		x = c * x - s * y;
		y = c * y + s * t;

		arm_g2_fill_triangle(px, py, x + cx, y + cy, cx, cy);
	}
}

void arm_g2_draw_inner_line(float x1, float y1, float x2, float y2, float strength) {
	int side = y2 > y1 ? 1 : 0;
	if (y2 == y1) {
		side = x2 - x1 > 0 ? 1 : 0;
	}

	kinc_vector2_t vec;
	if (y2 == y1) {
		vec = vec2_create(0, -1);
	}
	else {
		vec = vec2_create(1, -(x2 - x1) / (y2 - y1));
	}
	vec = vec2_set_len(vec, strength);
	kinc_vector2_t p1 = {x1 + side * vec.x, y1 + side * vec.y};
	kinc_vector2_t p2 = {x2 + side * vec.x, y2 + side * vec.y};
	kinc_vector2_t p3 = vec2_sub(p1, vec);
	kinc_vector2_t p4 = vec2_sub(p2, vec);
	arm_g2_fill_triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
	arm_g2_fill_triangle(p3.x, p3.y, p2.x, p2.y, p4.x, p4.y);
}

void arm_g2_draw_circle(float cx, float cy, float radius, int segments, float strength) {
	radius += strength / 2;

	if (segments <= 0) {
		segments = (int)floor(10 * sqrtf(radius));
	}

	float theta = 2 * (float)MATH_PI / segments;
	float c = cosf(theta);
	float s = sinf(theta);

	float x = radius;
	float y = 0.0;

	for (int n = 0; n < segments; ++n) {
		float px = x + cx;
		float py = y + cy;

		float t = x;
		x = c * x - s * y;
		y = c * y + s * t;

		arm_g2_draw_inner_line(x + cx, y + cy, px, py, strength);
	}
}

void arm_g2_calculate_cubic_bezier_point(float t, float *x, float *y, float *out) {
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	// first term
	out[0] = uuu * x[0];
	out[1] = uuu * y[0];

	// second term
	out[0] += 3 * uu * t * x[1];
	out[1] += 3 * uu * t * y[1];

	// third term
	out[0] += 3 * u * tt * x[2];
	out[1] += 3 * u * tt * y[2];

	// fourth term
	out[0] += ttt * x[3];
	out[1] += ttt * y[3];
}

/**
 * Draws a cubic bezier using 4 pairs of points. If the x and y arrays have a length bigger than 4, the additional
 * points will be ignored. With a length smaller of 4 a error will occur, there is no check for this.
 * You can construct the curves visually in Inkscape with a path using default nodes.
 * Provide x and y in the following order: startPoint, controlPoint1, controlPoint2, endPoint
 * Reference: http://devmag.org.za/2011/04/05/bzier-curves-a-tutorial/
 */
void arm_g2_draw_cubic_bezier(float *x, float *y, int segments, float strength) {
	float q0[2];
	float q1[2];
	arm_g2_calculate_cubic_bezier_point(0, x, y, q0);

	for (int i = 1; i < (segments + 1); ++i) {
		float t = (float)i / segments;
		arm_g2_calculate_cubic_bezier_point(t, x, y, q1);
		arm_g2_draw_line(q0[0], q0[1], q1[0], q1[1], strength);
		q0[0] = q1[0];
		q0[1] = q1[1];
	}
}
