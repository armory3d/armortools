#include "math.h"

#ifdef IRON_WASM
__attribute__((import_module("imports"), import_name("js_pow"))) float   js_pow(float base, float exponent);
__attribute__((import_module("imports"), import_name("js_floor"))) float js_floor(float x);
__attribute__((import_module("imports"), import_name("js_sin"))) float   js_sin(float x);
__attribute__((import_module("imports"), import_name("js_cos"))) float   js_cos(float x);
__attribute__((import_module("imports"), import_name("js_tan"))) float   js_tan(float x);
__attribute__((import_module("imports"), import_name("js_log"))) float   js_log(float x);
__attribute__((import_module("imports"), import_name("js_exp"))) float   js_exp(float x);
__attribute__((import_module("imports"), import_name("js_sqrt"))) float  js_sqrt(float x);
#endif

double fabs(double n) {
	return n < 0 ? -n : n;
}

float fabsf(float n) {
	return n < 0 ? -n : n;
}

double fmax(double x, double y) {
	return 0.0;
}

float fmaxf(float x, float y) {
	return 0.0f;
}

double fmin(double x, double y) {
	return 0.0;
}

float fminf(float x, float y) {
	return 0.0f;
}

double ldexp(double x, int exp) {
	return 0.0;
}

double pow(double base, double exponent) {
#ifdef IRON_WASM
	return js_pow(base, exponent);
#endif
	return 0.0;
}

float powf(float base, float exponent) {
#ifdef IRON_WASM
	return js_pow(base, exponent);
#endif
	return 0.0f;
}

double floor(double x) {
#ifdef IRON_WASM
	return js_floor(x);
#endif
	return 0.0;
}

float floorf(float x) {
#ifdef IRON_WASM
	return js_floor(x);
#endif
	return 0.0f;
}

double ceil(double x) {
	// #ifdef IRON_WASM
	// 	return js_ceil(x);
	// #endif
	return 0.0;
}

float ceilf(float x) {
	// #ifdef IRON_WASM
	// 	return js_ceil(x);
	// #endif
	return 0.0f;
}

double round(double x) {
	// #ifdef IRON_WASM
	// 	return js_round(x);
	// #endif
	return 0.0;
}

float roundf(float x) {
	// #ifdef IRON_WASM
	// 	return js_round(x);
	// #endif
	return 0.0f;
}

double sin(double x) {
#ifdef IRON_WASM
	return js_sin(x);
#endif
	return 0.0;
}

float sinf(float x) {
#ifdef IRON_WASM
	return js_sin(x);
#endif
	return 0.0f;
}

float asinf(float x) {
	return 0.0f;
}

double cos(double x) {
#ifdef IRON_WASM
	return js_cos(x);
#endif
	return 0.0;
}

float cosf(float x) {
#ifdef IRON_WASM
	return js_cos(x);
#endif
	return 0.0f;
}

double acos(double x) {
	return 0.0;
}

float acosf(float x) {
	return 0.0f;
}

double tan(double x) {
#ifdef IRON_WASM
	return js_tan(x);
#endif
	return 0.0;
}

float tanf(float x) {
#ifdef IRON_WASM
	return js_tan(x);
#endif
	return 0.0f;
}

float atanf(float x) {
	return 0.0f;
}

double atan2(double x, double y) {
	return 0.0;
}

float atan2f(float x, float y) {
	return 0.0f;
}

double log(double x) {
#ifdef IRON_WASM
	return js_log(x);
#endif
	return 0.0;
}

float logf(float x) {
#ifdef IRON_WASM
	return js_log(x);
#endif
	return 0.0f;
}

float log2f(float x) {
	return 0.0f;
}

double exp(double x) {
#ifdef IRON_WASM
	return js_exp(x);
#endif
	return 0.0;
}

float expf(float x) {
#ifdef IRON_WASM
	return js_exp(x);
#endif
	return 0.0;
}

double exp2(double x) {
	return 0.0;
}

float exp2f(float x) {
	return 0.0f;
}

double frexp(double x, int *exp) {
	return 0.0;
}

float frexpf(float x, int *exp) {
	return 0.0;
}

double sqrt(double x) {
#ifdef IRON_WASM
	return js_sqrt(x);
#endif
	return 0.0;
}

float sqrtf(float x) {
#ifdef IRON_WASM
	return js_sqrt(x);
#endif
	return 0.0f;
}

double fmod(double x, double y) {
	return 0.0;
}

float fmodf(float x, float y) {
	return 0.0f;
}

int isnan(float x) {
	return 0;
}
