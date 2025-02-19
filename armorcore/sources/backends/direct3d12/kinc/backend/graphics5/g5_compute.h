#pragma once

#include <kinc/backend/graphics5/shaderhash.h>

typedef struct {
	uint32_t hash;
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_compute_internal_shader_constant_t;

typedef struct kinc_g5_compute_shader_impl {
	kinc_compute_internal_shader_constant_t constants[64];
	int constantsSize;
	kinc_internal_hash_index_t attributes[64];
	kinc_internal_hash_index_t textures[64];
	uint8_t *data;
	int length;
	struct ID3D12Buffer *constantBuffer;
	struct ID3D12PipelineState *pso;
} kinc_g5_compute_shader_impl;

#ifdef __cplusplus
}
#endif
