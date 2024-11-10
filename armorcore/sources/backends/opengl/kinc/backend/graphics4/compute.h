#pragma once

typedef struct kinc_g4_compute_constant_location_impl {
	int location;
	unsigned int type;
} kinc_g4_compute_constant_location_impl;

typedef struct kinc_g4_compute_texture_unit_impl {
	int unit;
} kinc_g4_compute_texture_unit_impl;

typedef struct kinc_g4_compute_shader_impl {
	char **textures;
	int *textureValues;
	int textureCount;
	unsigned _id;
	unsigned _programid;
	char *_source;
	int _length;
} kinc_g4_compute_shader_impl;
