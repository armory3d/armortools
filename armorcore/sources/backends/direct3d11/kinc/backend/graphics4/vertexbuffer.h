#pragma once

struct ID3D11Buffer;

typedef struct {
	struct ID3D11Buffer *vb;
	int stride;
	int count;
	int lockStart;
	int lockCount;
	float *vertices;
	int usage;
} kinc_g4_vertex_buffer_impl_t;
