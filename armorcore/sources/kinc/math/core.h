#pragma once

#include <kinc/global.h>

/*! \file core.h
    \brief Just a few very simple additions to math.h
   the C-lib.
*/

#define KINC_PI 3.141592654
#define KINC_TAU 6.283185307

float kinc_cot(float x);
float kinc_round(float value);
float kinc_abs(float value);
float kinc_min(float a, float b);
float kinc_max(float a, float b);
int kinc_mini(int a, int b);
int kinc_maxi(int a, int b);
float kinc_clamp(float value, float minValue, float maxValue);

#ifdef KINC_IMPLEMENTATION_MATH
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <math.h>

float kinc_cot(float x) {
	return cosf(x) / sinf(x);
}

float kinc_round(float value) {
	return floorf(value + 0.5f);
}

float kinc_abs(float value) {
	return value < 0 ? -value : value;
}

float kinc_min(float a, float b) {
	return a > b ? b : a;
}

float kinc_max(float a, float b) {
	return a > b ? a : b;
}

int kinc_mini(int a, int b) {
	return a > b ? b : a;
}

int kinc_maxi(int a, int b) {
	return a > b ? a : b;
}

float kinc_clamp(float value, float minValue, float maxValue) {
	return kinc_max(minValue, kinc_min(maxValue, value));
}

#endif
