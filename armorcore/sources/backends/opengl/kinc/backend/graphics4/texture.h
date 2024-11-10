#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned int texture;
#ifdef KINC_ANDROID
	bool external_oes;
#endif
	uint8_t pixfmt;
} kinc_g4_texture_impl_t;

#ifdef __cplusplus
}
#endif
