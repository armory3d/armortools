
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <math.h>
#include "iron_array.h"
#include "iron_obj.h"

uint32_t *io_gltf_read_u8_array(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	unsigned char *ar = (unsigned char *)v->buffer->data + v->offset;
	uint32_t *res = malloc(sizeof(unsigned int) * v->size);
	for (int i = 0; i < v->size; ++i) {
		res[i] = ar[i];
	}
	return res;
}

uint32_t *io_gltf_read_u16_array(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	unsigned short *ar = (unsigned short *)v->buffer->data + v->offset / 2;
	uint32_t *res = malloc(sizeof(unsigned int) * v->size);
	for (int i = 0; i < v->size / 2; ++i) {
		res[i] = ar[i];
	}
	return res;
}

uint32_t *io_gltf_read_u32_array(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	unsigned int *ar = (unsigned int *)v->buffer->data + v->offset / 4;
	return ar;
}

float *io_gltf_read_f32_array(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	float *ar = (float *)v->buffer->data + v->offset / 4;
	return ar;
}

void *io_gltf_parse(char *buf, size_t size) {
	cgltf_options options = {0};
	cgltf_data *data = NULL;
	cgltf_result result = cgltf_parse(&options, buf, size, &data);
	if (result != cgltf_result_success) {
		return NULL;
	}
	cgltf_load_buffers(&options, data, NULL);

	cgltf_mesh *mesh = &data->meshes[0];
	cgltf_primitive *prim = &mesh->primitives[0];

	cgltf_accessor *a = prim->indices;
	int elem_size = a->buffer_view->size / a->count;
	int index_count = a->count;
	uint32_t *inda = elem_size == 1 ? io_gltf_read_u8_array(a)  :
		      		 elem_size == 2 ? io_gltf_read_u16_array(a) :
									  io_gltf_read_u32_array(a);

	float *posa32 = NULL;
	float *nora32 = NULL;
	float *texa32 = NULL;
	for (int i = 0; i < prim->attributes_count; ++i) {
		cgltf_attribute* attrib = &prim->attributes[i];
		if (attrib->type == cgltf_attribute_type_position) {
			posa32 = io_gltf_read_f32_array(attrib->data);
		}
		else if (attrib->type == cgltf_attribute_type_normal) {
			nora32 = io_gltf_read_f32_array(attrib->data);
		}
		else if (attrib->type == cgltf_attribute_type_texcoord) {
			texa32 = io_gltf_read_f32_array(attrib->data);
		}
	}

	int vertex_count = prim->attributes[0].data->count; // Assume VEC3 position

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
	float scale_pos = fmax(hx, fmax(hy, hz));
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
	}
	else {
		// Calc normals
		for (int i = 0; i < index_count / 3; ++i) {
			int i1 = inda[i * 3    ];
			int i2 = inda[i * 3 + 1];
			int i3 = inda[i * 3 + 2];
			float vax = posa32[i1 * 3]; float vay = posa32[i1 * 3 + 1]; float vaz = posa32[i1 * 3 + 2];
			float vbx = posa32[i2 * 3]; float vby = posa32[i2 * 3 + 1]; float vbz = posa32[i2 * 3 + 2];
			float vcx = posa32[i3 * 3]; float vcy = posa32[i3 * 3 + 1]; float vcz = posa32[i3 * 3 + 2];
			float cbx = vcx - vbx; float cby = vcy - vby; float cbz = vcz - vbz;
			float abx = vax - vbx; float aby = vay - vby; float abz = vaz - vbz;
			float x = cbx, y = cby, z = cbz;
			cbx = y * abz - z * aby;
			cby = z * abx - x * abz;
			cbz = x * aby - y * abx;
			float n = sqrt(cbx * cbx + cby * cby + cbz * cbz);
			if (n > 0.0) {
				float inv_n = 1.0 / n;
				cbx *= inv_n;
				cby *= inv_n;
				cbz *= inv_n;
			}
			nora[i1 * 2    ] = (int)(cbx * 32767);
			nora[i1 * 2 + 1] = (int)(cby * 32767);
			posa[i1 * 4 + 3] = (int)(cbz * 32767);
			nora[i2 * 2    ] = (int)(cbx * 32767);
			nora[i2 * 2 + 1] = (int)(cby * 32767);
			posa[i2 * 4 + 3] = (int)(cbz * 32767);
			nora[i3 * 2    ] = (int)(cbx * 32767);
			nora[i3 * 2 + 1] = (int)(cby * 32767);
			posa[i3 * 4 + 3] = (int)(cbz * 32767);
		}
	}

	short *texa = NULL;
	if (texa32 != NULL) {
		texa = malloc(sizeof(short) * vertex_count * 2);
		for (int i = 0; i < vertex_count; ++i) {
			texa[i * 2    ] = texa32[i * 2    ] * 32767;
			texa[i * 2 + 1] = texa32[i * 2 + 1] * 32767;
		}
	}

	cgltf_free(data);

	raw_mesh_t *raw = (raw_mesh_t *)calloc(sizeof(raw_mesh_t), 1);

	// raw->name = (char *)malloc(strlen(mesh->name) + 1);
	// strcpy(raw->name, mesh->name);
	raw->name = "";

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

	return raw;
}
