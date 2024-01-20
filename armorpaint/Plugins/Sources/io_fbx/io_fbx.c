#include "ufbx/ufbx.h"
#include <stdlib.h>
#include <math.h>

static uint8_t *buffer = NULL;
static uint32_t bufferLength = 0;
static int bufOff; /* Pointer to fbx file data */
static size_t size; /* Size of the file data */

static int index_count;
static int vertex_count;
static int indaOff;
static int posaOff;
static int noraOff;
static int texaOff;
static int colaOff;
static float scale_pos;
static char name[128];
static float transform[16];
static bool has_next = false;
static int current_node;

uint8_t *io_fbx_getBuffer() { return buffer; }
uint32_t io_fbx_getBufferLength() { return bufferLength; }

static int allocate(int size) {
	size += size % 4; // Byte align
	bufferLength += size;
	buffer = buffer == NULL ? (uint8_t *)malloc(bufferLength) : (uint8_t *)realloc(buffer, bufferLength);
	return bufferLength - size;
}

int io_fbx_init(int bufSize) {
	if (!has_next) {
		current_node = 0;
		scale_pos = 0;
	}

	size = bufSize;
	bufOff = allocate(sizeof(uint8_t) * bufSize);
	return bufOff;
}

void io_fbx_parse_mesh(ufbx_mesh *mesh) {
	uint32_t indices_size = mesh->max_face_triangles * 3;
	uint32_t *indices = (uint32_t *)malloc(sizeof(uint32_t) * mesh->max_face_triangles * 3);

	bool has_tex = mesh->vertex_uv.exists;
	bool has_col = mesh->vertex_color.exists;

	int numtri = mesh->num_triangles;
	float *posa32 = (float *)malloc(sizeof(float) * numtri * 3 * 3);
	float *nora32 = (float *)malloc(sizeof(float) * numtri * 3 * 3);
	float *texa32 = has_tex ? (float *)malloc(sizeof(float) * numtri * 3 * 2) : NULL;
	float *cola32 = has_col ? (float *)malloc(sizeof(float) * numtri * 3 * 3) : NULL;
	int pi = 0;
	int ni = 0;
	int ti = 0;
	int ci = 0;

	for (int j = 0; j < mesh->faces.count; ++j) {
		ufbx_face face = mesh->faces.data[j];
		uint32_t num_triangles = ufbx_triangulate_face(indices, indices_size, mesh, face);
		for (uint32_t v_ix = 0; v_ix < num_triangles * 3; v_ix++) {
			uint32_t a = indices[v_ix];

			posa32[pi++] = ufbx_get_vertex_vec3(&mesh->vertex_position, a).x;
			posa32[pi++] = ufbx_get_vertex_vec3(&mesh->vertex_position, a).y;
			posa32[pi++] = ufbx_get_vertex_vec3(&mesh->vertex_position, a).z;

			nora32[ni++] = ufbx_get_vertex_vec3(&mesh->vertex_normal, a).x;
			nora32[ni++] = ufbx_get_vertex_vec3(&mesh->vertex_normal, a).y;
			nora32[ni++] = ufbx_get_vertex_vec3(&mesh->vertex_normal, a).z;

			if (has_tex) {
				texa32[ti++] = ufbx_get_vertex_vec2(&mesh->vertex_uv, a).x;
				texa32[ti++] = ufbx_get_vertex_vec2(&mesh->vertex_uv, a).y;
			}

			if (has_col) {
				cola32[ci++] = ufbx_get_vertex_vec4(&mesh->vertex_color, a).x;
				cola32[ci++] = ufbx_get_vertex_vec4(&mesh->vertex_color, a).y;
				cola32[ci++] = ufbx_get_vertex_vec4(&mesh->vertex_color, a).z;
			}
		}
	}

	free(indices);

	vertex_count = pi / 3;
	index_count = vertex_count;

	indaOff = allocate(sizeof(uint32_t) * index_count);
	uint32_t *inda = (uint32_t *)&buffer[indaOff];
	for (int i = 0; i < index_count; ++i) {
		inda[i] = i;
	}

	// Pack positions to (-1, 1) range
	float hx = 0.0;
	float hy = 0.0;
	float hz = 0.0;
	for (int i = 0; i < vertex_count; ++i) {
		float f = fabsf(posa32[i * 3]);
		if (hx < f) hx = f;
		f = fabsf(posa32[i * 3 + 1]);
		if (hy < f) hy = f;
		f = fabsf(posa32[i * 3 + 2]);
		if (hz < f) hz = f;
	}
	float _scale_pos = fmax(hx, fmax(hy, hz));
	if (_scale_pos > scale_pos) scale_pos = _scale_pos;
	float inv = 1 / scale_pos;

    // Pack into 16bit
	posaOff = allocate(sizeof(short) * vertex_count * 4);
	short *posa = (short *)&buffer[posaOff];
	for (int i = 0; i < vertex_count; ++i) {
		posa[i * 4    ] = posa32[i * 3    ] * 32767 * inv;
		posa[i * 4 + 1] = posa32[i * 3 + 1] * 32767 * inv;
		posa[i * 4 + 2] = posa32[i * 3 + 2] * 32767 * inv;
	}

	noraOff = allocate(sizeof(short) * vertex_count * 2);
	short *nora = (short *)&buffer[noraOff];
	if (nora32 != NULL) {
		for (int i = 0; i < vertex_count; ++i) {
			nora[i * 2    ] = nora32[i * 3    ] * 32767;
			nora[i * 2 + 1] = nora32[i * 3 + 1] * 32767;
			posa[i * 4 + 3] = nora32[i * 3 + 2] * 32767;
		}
		free(nora32);
	}

	free(posa32);

	if (texa32 != NULL) {
		texaOff = allocate(sizeof(short) * vertex_count * 2);
		short *texa = (short *)&buffer[texaOff];
		for (int i = 0; i < vertex_count; ++i) {
			texa[i * 2    ] = 		 texa32[i * 2    ]  * 32767;
			texa[i * 2 + 1] = (1.0 - texa32[i * 2 + 1]) * 32767;
		}
		free(texa32);
	}
	else texaOff = 0;

	if (cola32 != NULL) {
		colaOff = allocate(sizeof(short) * vertex_count * 3);
		short *cola = (short *)&buffer[colaOff];
		for (int i = 0; i < vertex_count; ++i) {
			cola[i * 3    ] = cola32[i * 3    ] * 32767;
			cola[i * 3 + 1] = cola32[i * 3 + 1] * 32767;
			cola[i * 3 + 2] = cola32[i * 3 + 2] * 32767;
		}
		free(cola32);
	}
	else colaOff = 0;
}

void io_fbx_parse() {
	void *buf = &buffer[bufOff];
	ufbx_load_opts opts = { .generate_missing_normals = true };
	ufbx_scene *scene = ufbx_load_memory(buf, size, &opts, NULL);

	for (size_t i = current_node; i < scene->nodes.count; ++i) {
		current_node = i;
		ufbx_node *n = scene->nodes.data[i];
		if (n->mesh != NULL) {
			strcpy(name, n->name.data);
			// transform[0] = n->local_transform.translation.x;
			// transform[1] = n->local_transform.translation.y;
			// transform[2] = n->local_transform.translation.z;
			// transform[3] = n->local_transform.scale.x;
			// transform[4] = n->local_transform.scale.y;
			// transform[5] = n->local_transform.scale.z;
			// transform[6] = n->local_transform.rotation.x;
			// transform[7] = n->local_transform.rotation.y;
			// transform[8] = n->local_transform.rotation.z;
			// transform[9] = n->local_transform.rotation.w;
			transform[0] = n->unscaled_node_to_world.m00;
			transform[1] = n->unscaled_node_to_world.m01;
			transform[2] = n->unscaled_node_to_world.m02;
			transform[3] = n->unscaled_node_to_world.m03;
			transform[4] = n->unscaled_node_to_world.m10;
			transform[5] = n->unscaled_node_to_world.m11;
			transform[6] = n->unscaled_node_to_world.m12;
			transform[7] = n->unscaled_node_to_world.m13;
			transform[8] = n->unscaled_node_to_world.m20;
			transform[9] = n->unscaled_node_to_world.m21;
			transform[10] = n->unscaled_node_to_world.m22;
			transform[11] = n->unscaled_node_to_world.m23;
			transform[12] = 0.0;
			transform[13] = 0.0;
			transform[14] = 0.0;
			transform[15] = 1.0;
			io_fbx_parse_mesh(n->mesh);
			break;
		}
	}
	current_node++;

	has_next = false;
	for (size_t i = current_node; i < scene->nodes.count; ++i) {
		ufbx_node *n = scene->nodes.data[i];
		if (n->mesh != NULL) {
			has_next = true;
			break;
		}
	}
}

void io_fbx_destroy() {
	free(buffer);
	buffer = NULL;
}

int io_fbx_get_index_count() { return index_count; }
int io_fbx_get_vertex_count() { return vertex_count; }
float io_fbx_get_scale_pos() { return scale_pos; }
int io_fbx_get_indices() { return indaOff; }
int io_fbx_get_positions() { return posaOff; }
int io_fbx_get_normals() { return noraOff; }
int io_fbx_get_uvs() { return texaOff; }
int io_fbx_get_colors() { return colaOff; }
char *io_fbx_get_name() { return &name[0]; }
float *io_fbx_get_transform() { return &transform[0]; }
int io_fbx_has_next() { return has_next; }
