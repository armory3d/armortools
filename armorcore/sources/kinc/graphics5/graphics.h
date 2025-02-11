#pragma once

#include <kinc/global.h>
#include <kinc/image.h>
#include "rendertarget.h"
#include "shader.h"
#include "vertexstructure.h"
#include <kinc/backend/graphics5/graphics.h>
#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

/*! \file graphics.h
    \brief Contains the base G5-functionality.
*/

bool kinc_g5_supports_raytracing(void);
int kinc_g5_max_bound_textures(void);
void kinc_g5_flush(void);
void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window);
void kinc_g5_end(int window);
bool kinc_g5_swap_buffers(void);

void kinc_g5_internal_init(void);
void kinc_g5_internal_init_window(int window, int depth_buffer_bits, bool vsync);
void kinc_g5_internal_destroy_window(int window);
void kinc_g5_internal_destroy(void);
