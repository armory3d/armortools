#pragma once

#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "iron_string.h"
#include "iron_array.h"
#include "iron_map.h"
#include "iron_armpack.h"
#include "iron_json.h"
#include "iron_gc.h"

#define f64 double
#define i64 int64_t
#define u64 uint64_t
#define f32 float
#define i32 int32_t
#define u32 uint32_t
#define i16 int16_t
#define u16 uint16_t
#define i8 int8_t
#define u8 uint8_t
#define string_t char
#define any void *
#define any_ptr void **
#define u8_ptr u8 *
#define u32_ptr u32 *
#define f32_ptr f32 *
#define null NULL
#define DEREFERENCE *
#define ADDRESS &

void _kickstart();
void kickstart() {
    int bos;
    gc_start(&bos);
    _kickstart();
}

f32 math_floor(f32 x) { return floorf(x); }
f32 math_cos(f32 x) { return cosf(x); }
f32 math_sin(f32 x) { return sinf(x); }
f32 math_tan(f32 x) { return tanf(x); }
f32 math_sqrt(f32 x) { return sqrtf(x); }
f32 math_abs(f32 x) { return fabsf(x); }
f32 math_random() { return rand() / (float)RAND_MAX; }
f32 math_atan2(f32 y, f32 x) { return atan2f(y, x); }
f32 math_asin(f32 x) { return asinf(x); }
f32 math_pi() { return 3.14159265358979323846; }
f32 math_pow(f32 x, f32 y) { return powf(x, y); }
f32 math_round(f32 x) { return roundf(x); }
f32 math_ceil(f32 x) { return ceilf(x); }
f32 math_min(f32 x, f32 y) { return x < y ? x : y; }
f32 math_max(f32 x, f32 y) { return x > y ? x : y; }
f32 math_log(f32 x) { return logf(x); }
f32 math_log2(f32 x) { return log2f(x); }
f32 math_atan(f32 x) { return atanf(x); }
f32 math_acos(f32 x) { return acosf(x); }
f32 math_exp(f32 x) { return expf(x); }
f32 math_fmod(f32 x, f32 y) { return fmod(x, y); }

#ifdef _WIN32
i32 parse_int(const char *s) { return _strtoi64(s, NULL, 10); }
i32 parse_int_hex(const char *s) { return _strtoi64(s, NULL, 16); }
#else
i32 parse_int(const char *s) { return strtol(s, NULL, 10); }
i32 parse_int_hex(const char *s) { return strtol(s, NULL, 16); }
#endif
f32 parse_float(const char *s) { return strtof(s, NULL); }
