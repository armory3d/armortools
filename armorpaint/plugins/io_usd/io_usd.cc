#include <stdint.h>
#include <math.h>
#include "tinyusdz/tinyusdz.hh"
#include "iron_obj.h"
#include "iron_array.h"

extern "C" void *io_usd_parse(uint8_t *buf, size_t size) {
	tinyusdz::Scene scene;
	std::string warn;
	std::string err;
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

    int vertex_count = dst_facevarying_indices.size();
    int index_count = dst_facevarying_indices.size();

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
	float scale_pos = fmax(hx, fmax(hy, hz));
	float inv = 1 / scale_pos;

	// Pack into 16bit
	uint32_t *inda = (uint32_t *)malloc(sizeof(uint32_t) * index_count);
	for (int i = 0; i < index_count; ++i) {
		inda[i] = i;
	}
	int16_t *posa = (int16_t *)malloc(sizeof(int16_t) * vertex_count * 4);
	int16_t *nora = (int16_t *)malloc(sizeof(int16_t) * vertex_count * 2);
	int16_t *texa = (int16_t *)malloc(sizeof(int16_t) * vertex_count * 2);

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

	raw_mesh_t *raw = (raw_mesh_t *)calloc(sizeof(raw_mesh_t), 1);

	raw->name = (char *)malloc(mesh.name.length() + 1);
	strcpy(raw->name, mesh.name.c_str());

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
	raw->inda->length = raw->inda->capacity = vertex_count * 2;

	raw->scale_pos = scale_pos;
	raw->scale_tex = 1.0;

	return raw;
}
