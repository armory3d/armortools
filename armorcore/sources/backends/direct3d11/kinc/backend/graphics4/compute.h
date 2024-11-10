#pragma once

#include <kinc/backend/graphics4/ShaderHash.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D11Buffer;

typedef struct kinc_g4_compute_constant_location_impl {
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_g4_compute_constant_location_impl;

typedef struct kinc_g4_compute_texture_unit_impl {
	int unit;
} kinc_g4_compute_texture_unit_impl;

typedef struct kinc_g4_compute_internal_shader_constant {
	uint32_t hash;
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_g4_compute_internal_shader_constant;

typedef struct kinc_g4_compute_shader_impl {
	kinc_g4_compute_internal_shader_constant constants[64];
	int constantsSize;
	kinc_internal_hash_index_t attributes[64];
	kinc_internal_hash_index_t textures[64];
	struct ID3D11Buffer *constantBuffer;
	void *shader;
	uint8_t *data;
	int length;
} kinc_g4_compute_shader_impl;

#ifdef __cplusplus
}
#endif
