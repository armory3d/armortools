#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned _framebuffer;
	unsigned _texture;
	unsigned _depthTexture;
	bool _hasDepth;
	// unsigned _depthRenderbuffer;
	int format;
} kinc_g4_render_target_impl_t;

#ifdef __cplusplus
}
#endif
