#include "tinyusdz/tinyusdz.hh"
#include <math.h>

static uint8_t *buffer = NULL;
static uint32_t bufferLength = 0;
static int bufOff; /* Pointer to usdc file data */
static size_t size; /* Size of the file data */

static int index_count;
static int vertex_count;
static int indaOff;
static int posaOff;
static int noraOff;
static int texaOff;
static float scale_pos;

extern "C" uint8_t *io_usd_getBuffer() { return buffer; }
extern "C" uint32_t io_usd_getBufferLength() { return bufferLength; }

static int allocate(int size) {
	size += size % 4; // Byte align
	bufferLength += size;
	buffer = buffer == NULL ? (uint8_t *)malloc(bufferLength) : (uint8_t *)realloc(buffer, bufferLength);
	return bufferLength - size;
}

extern "C" int io_usd_init(int bufSize) {
	size = bufSize;
	bufOff = allocate(sizeof(uint8_t) * bufSize);
	return bufOff;
}

extern "C" void io_usd_parse() {
	tinyusdz::Scene scene;
	std::string warn;
	std::string err;
	uint8_t *buf = &buffer[bufOff];
	tinyusdz::LoadUSDCFromMemory(buf, size, &scene, &warn, &err);
	tinyusdz::GeomMesh mesh = scene.geom_meshes[0];

	std::vector<float> vertices = mesh.points.buffer.GetAsVec3fArray();
	std::vector<uint32_t> dst_facevarying_indices;
	std::vector<float> dst_facevarying_normals;
	std::vector<float> dst_facevarying_texcoords;

	std::vector<float> facevarying_normals;
	mesh.GetFacevaryingNormals(&facevarying_normals);
	std::vector<float> facevarying_texcoords;
	mesh.GetFacevaryingTexcoords(&facevarying_texcoords);

	// Make facevarying indices
    size_t face_offset = 0;
    for (size_t fid = 0; fid < mesh.faceVertexCounts.size(); fid++) {
        int f_count = mesh.faceVertexCounts[fid];
        if (f_count == 3) {
            for (size_t f = 0; f < f_count; f++) {
	            dst_facevarying_indices.push_back(mesh.faceVertexIndices[face_offset + f]);
	            if (facevarying_normals.size()) {
	                dst_facevarying_normals.push_back(facevarying_normals[3 * (face_offset + f) + 0]);
	                dst_facevarying_normals.push_back(facevarying_normals[3 * (face_offset + f) + 1]);
	                dst_facevarying_normals.push_back(facevarying_normals[3 * (face_offset + f) + 2]);
	            }
	            if (facevarying_texcoords.size()) {
	                dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * (face_offset + f) + 0]);
	                dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * (face_offset + f) + 1]);
	            }
            }
        }
        else {
	        // Simple triangulation with triangle-fan decomposition
	        for (size_t f = 0; f < f_count - 2; f++) {
	            size_t f0 = 0;
	            size_t f1 = f + 1;
	            size_t f2 = f + 2;
	            size_t fid0 = face_offset + f0;
            	size_t fid1 = face_offset + f1;
            	size_t fid2 = face_offset + f2;
	            dst_facevarying_indices.push_back(mesh.faceVertexIndices[fid0]);
	            dst_facevarying_indices.push_back(mesh.faceVertexIndices[fid1]);
	            dst_facevarying_indices.push_back(mesh.faceVertexIndices[fid2]);
	            if (facevarying_normals.size()) {
		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid0 + 0]);
		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid0 + 1]);
		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid0 + 2]);

		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid1 + 0]);
		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid1 + 1]);
		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid1 + 2]);

		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid2 + 0]);
		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid2 + 1]);
		            dst_facevarying_normals.push_back(facevarying_normals[3 * fid2 + 2]);
	            }
	            if (facevarying_texcoords.size()) {
	            	size_t fid0 = face_offset + f0;
	            	size_t fid1 = face_offset + f1;
	            	size_t fid2 = face_offset + f2;

		            dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * fid0 + 0]);
		            dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * fid0 + 1]);

		            dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * fid1 + 0]);
		            dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * fid1 + 1]);

		            dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * fid2 + 0]);
		            dst_facevarying_texcoords.push_back(facevarying_texcoords[2 * fid2 + 1]);
	            }
	        }
        }
        face_offset += f_count;
    }

    vertex_count = dst_facevarying_indices.size();
    index_count = dst_facevarying_indices.size();

    // Pack positions to (-1, 1) range
	float hx = 0.0;
	float hy = 0.0;
	float hz = 0.0;
	for (int i = 0; i < vertices.size() / 3; ++i) {
		float f = fabsf(vertices[i * 3]);
		if (hx < f) hx = f;
		f = fabsf(vertices[i * 3 + 1]);
		if (hy < f) hy = f;
		f = fabsf(vertices[i * 3 + 2]);
		if (hz < f) hz = f;
	}
	scale_pos = fmax(hx, fmax(hy, hz));
	float inv = 1 / scale_pos;

	// Pack into 16bit
	indaOff = allocate(sizeof(unsigned int) * index_count);
	unsigned int *inda = (unsigned int *)&buffer[indaOff];
	for (int i = 0; i < index_count; ++i) {
		inda[i] = i;
	}
	posaOff = allocate(sizeof(short) * vertex_count * 4);
	noraOff = allocate(sizeof(short) * vertex_count * 2);
	texaOff = allocate(sizeof(short) * vertex_count * 2);
	short *posa = (short *)&buffer[posaOff];
	short *nora = (short *)&buffer[noraOff];
	short *texa = (short *)&buffer[texaOff];

	for (int i = 0; i < vertex_count; ++i) {
		posa[i * 4    ] = vertices[dst_facevarying_indices[i] * 3    ] * 32767 * inv;
		posa[i * 4 + 1] = vertices[dst_facevarying_indices[i] * 3 + 1] * 32767 * inv;
		posa[i * 4 + 2] = vertices[dst_facevarying_indices[i] * 3 + 2] * 32767 * inv;
		posa[i * 4 + 3] = dst_facevarying_normals[i * 3 + 2] * 32767;
		nora[i * 2    ] = dst_facevarying_normals[i * 3    ] * 32767;
		nora[i * 2 + 1] = dst_facevarying_normals[i * 3 + 1] * 32767;
		texa[i * 2    ] = dst_facevarying_texcoords[i * 2    ] * 32767;
		texa[i * 2 + 1] = (1.0 - dst_facevarying_texcoords[i * 2 + 1]) * 32767;
	}
}

extern "C" void io_usd_destroy() {
	free(buffer);
	buffer = NULL;
}

extern "C" int io_usd_get_index_count() { return index_count; }
extern "C" int io_usd_get_vertex_count() { return vertex_count; }
extern "C" float io_usd_get_scale_pos() { return scale_pos; }
extern "C" int io_usd_get_indices() { return indaOff; }
extern "C" int io_usd_get_positions() { return posaOff; }
extern "C" int io_usd_get_normals() { return noraOff; }
extern "C" int io_usd_get_uvs() { return texaOff; }
