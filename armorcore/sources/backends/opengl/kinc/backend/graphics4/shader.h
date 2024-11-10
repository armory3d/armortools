#pragma once

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned _glid;
	const char *source;
	size_t length;
} kinc_g4_shader_impl_t;

typedef struct {
	int location;
	unsigned type;
} kinc_g4_constant_location_impl_t;

typedef struct {
	int unit;
} kinc_g4_texture_unit_impl_t;

#ifdef __cplusplus
}
#endif
