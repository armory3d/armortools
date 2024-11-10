#pragma once

#include <kinc/math/vector.h>
#include "iron_vec4.h"

vec2_t vec2_create(float x, float y);
float vec2_len(vec2_t v);
vec2_t vec2_set_len(vec2_t v, float length);
vec2_t vec2_mult(vec2_t a, float f);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);
float vec2_cross(vec2_t a, vec2_t b);
vec2_t vec2_norm(vec2_t v);
float vec2_dot(vec2_t a, vec2_t b);
vec2_t vec2_nan();
bool vec2_isnan(vec2_t v);
