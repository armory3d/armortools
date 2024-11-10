#pragma once

#include <kinc/graphics5/shader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	// ShaderImpl(void *data, int length, Graphics5::ShaderType type);
	kinc_g5_shader_t _shader;
} kinc_g4_shader_impl_t;

#ifdef __cplusplus
}
#endif
