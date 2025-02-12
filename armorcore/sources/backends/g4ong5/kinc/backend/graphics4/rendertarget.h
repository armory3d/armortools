#pragma once

#include <kinc/graphics5/rendertarget.h>

enum kinc_internal_render_target_state {
	KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET,
	KINC_INTERNAL_RENDER_TARGET_STATE_TEXTURE
};

typedef struct {
	kinc_g5_render_target_t _renderTarget;
	enum kinc_internal_render_target_state state;
} kinc_g4_render_target_impl_t;
