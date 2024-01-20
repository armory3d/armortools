// cgltf bridge for armorpaint

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <math.h>

static uint8_t *buffer = NULL;
static uint32_t bufferLength = 0;
static int bufOff; /* Pointer to glb or gltf file data */
static size_t size; /* Size of the file data */

static cgltf_data *data = NULL;

static int index_count;
static int vertex_count;
static int indaOff = 0;
static int posaOff = 0;
static int noraOff = 0;
static int texaOff = 0;
static float scale_pos;

uint8_t *io_gltf_getBuffer() { return buffer; }
uint32_t io_gltf_getBufferLength() { return bufferLength; }

static int allocate(int size) {
	size += size % 4; // Byte align
	bufferLength += size;
	buffer = buffer == NULL ? (uint8_t *)malloc(bufferLength) : (uint8_t *)realloc(buffer, bufferLength);
	return bufferLength - size;
}

int io_gltf_read_u8_array(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	unsigned char *ar = (unsigned char *)v->buffer->data + v->offset;
	int resOff = allocate(sizeof(unsigned int) * v->size);
	unsigned int *res = (unsigned int *)&buffer[resOff];
	for (int i = 0; i < v->size; ++i) {
		res[i] = ar[i];
	}
	return resOff;
}

int io_gltf_read_u16_array(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	unsigned short *ar = (unsigned short *)v->buffer->data + v->offset / 2;
	int resOff = allocate(sizeof(unsigned int) * v->size / 2);
	unsigned int *res = (unsigned int *)&buffer[resOff];
	for (int i = 0; i < v->size / 2; ++i) {
		res[i] = ar[i];
	}
	return resOff;
}

int io_gltf_read_u32_array(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	unsigned int *ar = (unsigned int *)v->buffer->data + v->offset / 4;
	int resOff = allocate(sizeof(unsigned int) * v->size / 4);
	unsigned int *res = (unsigned int *)&buffer[resOff];
	for (int i = 0; i < v->size / 4; ++i) {
		res[i] = ar[i];
	}
	return resOff;
}

float *io_gltf_read_f32_array_malloc(cgltf_accessor *a) {
	cgltf_buffer_view *v = a->buffer_view;
	float *ar = (float *)v->buffer->data + v->offset / 4;
	float *res = (float *)malloc(sizeof(float) * v->size / 4);
	for (int i = 0; i < v->size / 4; ++i) {
		res[i] = ar[i];
	}
	return res;
}

int io_gltf_init(int bufSize) {
	size = bufSize;
	bufOff = allocate(sizeof(char) * bufSize);
	return bufOff;
}

void io_gltf_parse() {
	cgltf_options options = {0};
	char *buf = (char *)&buffer[bufOff];
	cgltf_result result = cgltf_parse(&options, buf, size, &data);
	if (result != cgltf_result_success) { return; }
	cgltf_load_buffers(&options, data, NULL);

	cgltf_mesh *mesh = &data->meshes[0];
	cgltf_primitive *prim = &mesh->primitives[0];

	cgltf_accessor *a = prim->indices;
	int elem_size = a->buffer_view->size / a->count;
	index_count = a->count;
	indaOff = elem_size == 1 ? io_gltf_read_u8_array(a)  :
		      elem_size == 2 ? io_gltf_read_u16_array(a) :
							   io_gltf_read_u32_array(a);

	float *posa32 = NULL;
	float *nora32 = NULL;
	float *texa32 = NULL;
	for (int i = 0; i < prim->attributes_count; ++i) {
		cgltf_attribute* attrib = &prim->attributes[i];
		if (attrib->type == cgltf_attribute_type_position) {
			posa32 = io_gltf_read_f32_array_malloc(attrib->data);
		}
		else if (attrib->type == cgltf_attribute_type_normal) {
			nora32 = io_gltf_read_f32_array_malloc(attrib->data);
		}
		else if (attrib->type == cgltf_attribute_type_texcoord) {
			texa32 = io_gltf_read_f32_array_malloc(attrib->data);
		}
	}

	vertex_count = prim->attributes[0].data->count; // Assume VEC3 position

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
	scale_pos = fmax(hx, fmax(hy, hz));
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
	else {
		// Calc normals
		int *inda = (int *)&buffer[indaOff];
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

	free(posa32);

	if (texa32 != NULL) {
		texaOff = allocate(sizeof(short) * vertex_count * 2);
		short *texa = (short *)&buffer[texaOff];
		for (int i = 0; i < vertex_count; ++i) {
			texa[i * 2    ] = texa32[i * 2    ] * 32767;
			texa[i * 2 + 1] = texa32[i * 2 + 1] * 32767;
		}
		free(texa32);
	}
}

void io_gltf_destroy() {
	cgltf_free(data);
	free(buffer);
	buffer = NULL;
}

int io_gltf_get_index_count() { return index_count; }
int io_gltf_get_vertex_count() { return vertex_count; }
float io_gltf_get_scale_pos() { return scale_pos; }
int io_gltf_get_indices() { return indaOff; }
int io_gltf_get_positions() { return posaOff; }
int io_gltf_get_normals() { return noraOff; }
int io_gltf_get_uvs() { return texaOff; }
