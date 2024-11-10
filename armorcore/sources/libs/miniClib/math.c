#include "math.h"

#ifdef KINC_WASM
__attribute__((import_module("imports"), import_name("js_pow"))) float js_pow(float base, float exponent);
__attribute__((import_module("imports"), import_name("js_floor"))) float js_floor(float x);
__attribute__((import_module("imports"), import_name("js_sin"))) float js_sin(float x);
__attribute__((import_module("imports"), import_name("js_cos"))) float js_cos(float x);
__attribute__((import_module("imports"), import_name("js_tan"))) float js_tan(float x);
__attribute__((import_module("imports"), import_name("js_log"))) float js_log(float x);
__attribute__((import_module("imports"), import_name("js_exp"))) float js_exp(float x);
__attribute__((import_module("imports"), import_name("js_sqrt"))) float js_sqrt(float x);
#endif

double ldexp(double x, int exp) {
	return 0.0;
}

double pow(double base, double exponent) {
#ifdef KINC_WASM
	return js_pow(base, exponent);
#endif
	return 0.0;
}

double floor(double x) {
#ifdef KINC_WASM
	return js_floor(x);
#endif
	return 0.0;
}

float floorf(float x) {
#ifdef KINC_WASM
	return js_floor(x);
#endif
	return 0.0f;
}

double sin(double x) {
#ifdef KINC_WASM
	return js_sin(x);
#endif
	return 0.0;
}

float sinf(float x) {
#ifdef KINC_WASM
	return js_sin(x);
#endif
	return 0.0f;
}

double cos(double x) {
#ifdef KINC_WASM
	return js_cos(x);
#endif
	return 0.0;
}

float cosf(float x) {
#ifdef KINC_WASM
	return js_cos(x);
#endif
	return 0.0f;
}

double tan(double x) {
#ifdef KINC_WASM
	return js_tan(x);
#endif
	return 0.0;
}

float tanf(float x) {
#ifdef KINC_WASM
	return js_tan(x);
#endif
	return 0.0f;
}

double log(double x) {
#ifdef KINC_WASM
	return js_log(x);
#endif
	return 0.0;
}

double exp(double x) {
#ifdef KINC_WASM
	return js_exp(x);
#endif
	return 0.0;
}

double sqrt(double x) {
#ifdef KINC_WASM
	return js_sqrt(x);
#endif
	return 0.0;
}
