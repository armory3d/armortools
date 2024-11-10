#pragma once

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/textureunit.h>

#ifdef __cplusplus
extern "C" {
#endif

int Kinc_G4_Internal_TextureAddressingU(kinc_g4_texture_unit_t unit);
int Kinc_G4_Internal_TextureAddressingV(kinc_g4_texture_unit_t unit);
int Kinc_G4_Internal_StencilFunc(kinc_g4_compare_mode_t mode);

#ifdef __cplusplus
}
#endif
