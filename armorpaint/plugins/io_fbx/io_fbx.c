#include "ufbx/ufbx.h"
#include <stdlib.h>
#include <math.h>
#include "iron_array.h"
#include "iron_obj.h"

static bool has_next = false;
static int current_node = 0;
static float scale_pos = 1.0;

void io_fbx_parse_mesh(raw_mesh_t *raw, ufbx_mesh *mesh, ufbx_matrix *to_world, ufbx_matrix *to_world_unscaled) {
	uint32_t indices_size = mesh->max_face_triangles * 3;
	uint32_t *indices = (uint32_t *)malloc(sizeof(uint32_t) * indices_size);

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

			ufbx_vec3 v = ufbx_transform_position(to_world, ufbx_get_vertex_vec3(&mesh->vertex_position, a));
			posa32[pi++] = v.x;
			posa32[pi++] = v.y;
			posa32[pi++] = v.z;

			v = ufbx_transform_direction(to_world_unscaled, ufbx_get_vertex_vec3(&mesh->vertex_normal, a));
			nora32[ni++] = v.x;
			nora32[ni++] = v.y;
			nora32[ni++] = v.z;

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

	int vertex_count = pi / 3;
	int index_count = vertex_count;

	uint32_t *inda = malloc(sizeof(uint32_t) * index_count);
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
	short *posa = malloc(sizeof(short) * vertex_count * 4);
	for (int i = 0; i < vertex_count; ++i) {
		posa[i * 4    ] = posa32[i * 3    ] * 32767 * inv;
		posa[i * 4 + 1] = posa32[i * 3 + 1] * 32767 * inv;
		posa[i * 4 + 2] = posa32[i * 3 + 2] * 32767 * inv;
	}

	short *nora = malloc(sizeof(short) * vertex_count * 2);
	if (nora32 != NULL) {
		for (int i = 0; i < vertex_count; ++i) {
			nora[i * 2    ] = nora32[i * 3    ] * 32767;
			nora[i * 2 + 1] = nora32[i * 3 + 1] * 32767;
			posa[i * 4 + 3] = nora32[i * 3 + 2] * 32767;
		}
		free(nora32);
	}

	free(posa32);

	short *texa = NULL;
	if (texa32 != NULL) {
		texa = malloc(sizeof(short) * vertex_count * 2);
		for (int i = 0; i < vertex_count; ++i) {
			texa[i * 2    ] = 		 texa32[i * 2    ]  * 32767;
			texa[i * 2 + 1] = (1.0 - texa32[i * 2 + 1]) * 32767;
		}
		free(texa32);
	}

	short *cola = NULL;
	if (cola32 != NULL) {
		short *cola = malloc(sizeof(short) * vertex_count * 3);
		for (int i = 0; i < vertex_count; ++i) {
			cola[i * 3    ] = cola32[i * 3    ] * 32767;
			cola[i * 3 + 1] = cola32[i * 3 + 1] * 32767;
			cola[i * 3 + 2] = cola32[i * 3 + 2] * 32767;
		}
		free(cola32);
	}

	raw->posa = (i16_array_t *)malloc(sizeof(i16_array_t));
	raw->posa->buffer = posa;
	raw->posa->length = raw->posa->capacity = vertex_count * 4;

	raw->nora = (i16_array_t *)malloc(sizeof(i16_array_t));
	raw->nora->buffer = nora;
	raw->nora->length = raw->nora->capacity = vertex_count * 2;

	raw->texa = (i16_array_t *)malloc(sizeof(i16_array_t));
	raw->texa->buffer = texa;
	raw->texa->length = raw->texa->capacity = vertex_count * 2;

	raw->inda = (u32_array_t *)malloc(sizeof(u32_array_t));
	raw->inda->buffer = inda;
	raw->inda->length = raw->inda->capacity = index_count;

	raw->scale_pos = scale_pos;
	raw->scale_tex = 1.0;
}

void *io_fbx_parse(char *buf, size_t size) {
	ufbx_load_opts opts = { .generate_missing_normals = true };
	ufbx_scene *scene = ufbx_load_memory(buf, size, &opts, NULL);
	raw_mesh_t *raw = (raw_mesh_t *)calloc(sizeof(raw_mesh_t), 1);

	for (; current_node < scene->nodes.count; ++current_node) {
		ufbx_node *n = scene->nodes.data[current_node];
		if (n->mesh != NULL) {
			raw->name = malloc(strlen(n->name.data) + 1);
			strcpy(raw->name, n->name.data);
			io_fbx_parse_mesh(raw, n->mesh, &n->node_to_world, &n->unscaled_node_to_world);
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

	if (!has_next) {
		current_node = 0;
	}

	raw->has_next = has_next;

	return raw;
}
