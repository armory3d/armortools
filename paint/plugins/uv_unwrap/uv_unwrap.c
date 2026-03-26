
#include "uv_unwrap.h"
#include "iron_system.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cosine of 66 degrees - angle threshold for chart grouping
#define UV_ANGLE_THRESHOLD 0.4067f
#define UV_PACK_MARGIN     0.005f

// Position hash map entry for canonical vertex deduplication
typedef struct {
	int16_t x, y, z;
	int     id;
	bool    occupied;
} uv_pos_entry_t;

// Edge hash map entry for face adjacency
typedef struct {
	int  v0, v1;
	int  face0, face1;
	bool occupied;
} uv_edge_entry_t;

static uint32_t uv_hash_pos(int16_t x, int16_t y, int16_t z) {
	uint32_t h = (uint32_t)(x + 32768);
	h          = h * 2654435761u ^ (uint32_t)(y + 32768);
	h          = h * 2654435761u ^ (uint32_t)(z + 32768);
	return h;
}

static uint32_t uv_hash_edge(int v0, int v1) {
	return (uint32_t)v0 * 2654435761u ^ (uint32_t)v1 * 2246822519u;
}

void proc_uv_unwrap(raw_mesh_t *mesh) {
	double t = iron_time();

	// Decode input mesh data
	int    vertex_count = mesh->posa->length / 4;
	float *pa           = (float *)malloc(sizeof(float) * vertex_count * 3);
	float  inv          = 1.0f / 32767.0f;

	for (int i = 0; i < vertex_count; i++) {
		pa[i * 3]     = mesh->posa->buffer[i * 4] * inv;
		pa[i * 3 + 1] = mesh->posa->buffer[i * 4 + 1] * inv;
		pa[i * 3 + 2] = mesh->posa->buffer[i * 4 + 2] * inv;
	}

	int       index_count = mesh->inda->length;
	uint32_t *indices     = mesh->inda->buffer;
	int       face_count  = index_count / 3;

	// Compute face normals
	float *fnormals = (float *)malloc(sizeof(float) * face_count * 3);
	for (int f = 0; f < face_count; f++) {
		int   i0  = indices[f * 3];
		int   i1  = indices[f * 3 + 1];
		int   i2  = indices[f * 3 + 2];
		float e1x = pa[i1 * 3] - pa[i0 * 3];
		float e1y = pa[i1 * 3 + 1] - pa[i0 * 3 + 1];
		float e1z = pa[i1 * 3 + 2] - pa[i0 * 3 + 2];
		float e2x = pa[i2 * 3] - pa[i0 * 3];
		float e2y = pa[i2 * 3 + 1] - pa[i0 * 3 + 1];
		float e2z = pa[i2 * 3 + 2] - pa[i0 * 3 + 2];
		float nx  = e1y * e2z - e1z * e2y;
		float ny  = e1z * e2x - e1x * e2z;
		float nz  = e1x * e2y - e1y * e2x;
		float len = sqrtf(nx * nx + ny * ny + nz * nz);
		if (len > 1e-10f) {
			nx /= len;
			ny /= len;
			nz /= len;
		}
		fnormals[f * 3]     = nx;
		fnormals[f * 3 + 1] = ny;
		fnormals[f * 3 + 2] = nz;
	}

	// Build canonical vertex ID map (deduplicate positions)
	int             pos_cap     = vertex_count * 2 + 1;
	uv_pos_entry_t *pos_map     = (uv_pos_entry_t *)calloc(pos_cap, sizeof(uv_pos_entry_t));
	int            *vert_canon  = (int *)malloc(sizeof(int) * vertex_count);
	int             canon_count = 0;

	for (int i = 0; i < vertex_count; i++) {
		int16_t  x = mesh->posa->buffer[i * 4];
		int16_t  y = mesh->posa->buffer[i * 4 + 1];
		int16_t  z = mesh->posa->buffer[i * 4 + 2];
		uint32_t h = uv_hash_pos(x, y, z) % pos_cap;
		while (true) {
			if (!pos_map[h].occupied) {
				pos_map[h].x        = x;
				pos_map[h].y        = y;
				pos_map[h].z        = z;
				pos_map[h].id       = canon_count;
				pos_map[h].occupied = true;
				vert_canon[i]       = canon_count++;
				break;
			}
			if (pos_map[h].x == x && pos_map[h].y == y && pos_map[h].z == z) {
				vert_canon[i] = pos_map[h].id;
				break;
			}
			h = (h + 1) % pos_cap;
		}
	}
	free(pos_map);

	// Build face adjacency via shared edges
	int              edge_cap = face_count * 6 + 1;
	uv_edge_entry_t *edge_map = (uv_edge_entry_t *)calloc(edge_cap, sizeof(uv_edge_entry_t));
	int             *face_adj = (int *)malloc(sizeof(int) * face_count * 3);
	for (int i = 0; i < face_count * 3; i++) {
		face_adj[i] = -1;
	}

	for (int f = 0; f < face_count; f++) {
		for (int e = 0; e < 3; e++) {
			int c0 = vert_canon[indices[f * 3 + e]];
			int c1 = vert_canon[indices[f * 3 + (e + 1) % 3]];
			int ea = c0 < c1 ? c0 : c1;
			int eb = c0 < c1 ? c1 : c0;

			uint32_t h = uv_hash_edge(ea, eb) % edge_cap;
			while (true) {
				if (!edge_map[h].occupied) {
					edge_map[h].v0       = ea;
					edge_map[h].v1       = eb;
					edge_map[h].face0    = f;
					edge_map[h].face1    = -1;
					edge_map[h].occupied = true;
					break;
				}
				if (edge_map[h].v0 == ea && edge_map[h].v1 == eb) {
					if (edge_map[h].face1 == -1) {
						edge_map[h].face1 = f;
						int f0            = edge_map[h].face0;
						for (int s = 0; s < 3; s++) {
							if (face_adj[f0 * 3 + s] == -1) {
								face_adj[f0 * 3 + s] = f;
								break;
							}
						}
						for (int s = 0; s < 3; s++) {
							if (face_adj[f * 3 + s] == -1) {
								face_adj[f * 3 + s] = f0;
								break;
							}
						}
					}
					break;
				}
				h = (h + 1) % edge_cap;
			}
		}
	}
	free(edge_map);
	free(vert_canon);

	// Flood-fill faces into charts based on normal angle threshold
	int *chart_id    = (int *)malloc(sizeof(int) * face_count);
	int  chart_count = 0;
	int *stack       = (int *)malloc(sizeof(int) * face_count);
	memset(chart_id, -1, sizeof(int) * face_count);

	for (int f = 0; f < face_count; f++) {
		if (chart_id[f] != -1) {
			continue;
		}
		int   cid     = chart_count++;
		float seed_nx = fnormals[f * 3];
		float seed_ny = fnormals[f * 3 + 1];
		float seed_nz = fnormals[f * 3 + 2];
		chart_id[f]   = cid;
		int sp        = 0;
		stack[sp++]   = f;

		while (sp > 0) {
			int cf = stack[--sp];
			for (int s = 0; s < 3; s++) {
				int nf = face_adj[cf * 3 + s];
				if (nf == -1 || chart_id[nf] != -1) {
					continue;
				}
				// Compare against seed normal to prevent chart drift
				float dot = seed_nx * fnormals[nf * 3] + seed_ny * fnormals[nf * 3 + 1] + seed_nz * fnormals[nf * 3 + 2];
				if (dot >= UV_ANGLE_THRESHOLD) {
					chart_id[nf] = cid;
					stack[sp++]  = nf;
				}
			}
		}
	}
	free(stack);
	free(face_adj);

	// Compute average normal per chart
	float *chart_nx = (float *)calloc(chart_count, sizeof(float));
	float *chart_ny = (float *)calloc(chart_count, sizeof(float));
	float *chart_nz = (float *)calloc(chart_count, sizeof(float));

	for (int f = 0; f < face_count; f++) {
		int c = chart_id[f];
		chart_nx[c] += fnormals[f * 3];
		chart_ny[c] += fnormals[f * 3 + 1];
		chart_nz[c] += fnormals[f * 3 + 2];
	}
	free(fnormals);

	for (int c = 0; c < chart_count; c++) {
		float len = sqrtf(chart_nx[c] * chart_nx[c] + chart_ny[c] * chart_ny[c] + chart_nz[c] * chart_nz[c]);
		if (len > 1e-10f) {
			chart_nx[c] /= len;
			chart_ny[c] /= len;
			chart_nz[c] /= len;
		}
	}

	// Compute projection axes (U, V) per chart from average normal
	float *chart_ux = (float *)malloc(sizeof(float) * chart_count);
	float *chart_uy = (float *)malloc(sizeof(float) * chart_count);
	float *chart_uz = (float *)malloc(sizeof(float) * chart_count);
	float *chart_vx = (float *)malloc(sizeof(float) * chart_count);
	float *chart_vy = (float *)malloc(sizeof(float) * chart_count);
	float *chart_vz = (float *)malloc(sizeof(float) * chart_count);

	for (int c = 0; c < chart_count; c++) {
		float nx = chart_nx[c];
		float ny = chart_ny[c];
		float nz = chart_nz[c];

		// Reference vector not parallel to normal
		float rx, ry, rz;
		if (fabsf(ny) < 0.9f) {
			rx = 0.0f;
			ry = 1.0f;
			rz = 0.0f;
		}
		else {
			rx = 1.0f;
			ry = 0.0f;
			rz = 0.0f;
		}

		// U = normalize(cross(N, ref))
		float ux = ny * rz - nz * ry;
		float uy = nz * rx - nx * rz;
		float uz = nx * ry - ny * rx;
		float ul = sqrtf(ux * ux + uy * uy + uz * uz);
		if (ul > 1e-10f) {
			ux /= ul;
			uy /= ul;
			uz /= ul;
		}

		// V = cross(N, U)
		chart_ux[c] = ux;
		chart_uy[c] = uy;
		chart_uz[c] = uz;
		chart_vx[c] = ny * uz - nz * uy;
		chart_vy[c] = nz * ux - nx * uz;
		chart_vz[c] = nx * uy - ny * ux;
	}
	free(chart_nx);
	free(chart_ny);
	free(chart_nz);

	// Project vertices onto chart planes and compute per-chart bounding boxes
	float *uv_out  = (float *)malloc(sizeof(float) * index_count * 2);
	float *c_min_u = (float *)malloc(sizeof(float) * chart_count);
	float *c_min_v = (float *)malloc(sizeof(float) * chart_count);
	float *c_max_u = (float *)malloc(sizeof(float) * chart_count);
	float *c_max_v = (float *)malloc(sizeof(float) * chart_count);

	for (int c = 0; c < chart_count; c++) {
		c_min_u[c] = FLT_MAX;
		c_min_v[c] = FLT_MAX;
		c_max_u[c] = -FLT_MAX;
		c_max_v[c] = -FLT_MAX;
	}

	for (int f = 0; f < face_count; f++) {
		int c = chart_id[f];
		for (int k = 0; k < 3; k++) {
			int   vi        = indices[f * 3 + k];
			float px        = pa[vi * 3];
			float py        = pa[vi * 3 + 1];
			float pz        = pa[vi * 3 + 2];
			float u         = px * chart_ux[c] + py * chart_uy[c] + pz * chart_uz[c];
			float v         = px * chart_vx[c] + py * chart_vy[c] + pz * chart_vz[c];
			int   idx       = (f * 3 + k) * 2;
			uv_out[idx]     = u;
			uv_out[idx + 1] = v;
			if (u < c_min_u[c])
				c_min_u[c] = u;
			if (v < c_min_v[c])
				c_min_v[c] = v;
			if (u > c_max_u[c])
				c_max_u[c] = u;
			if (v > c_max_v[c])
				c_max_v[c] = v;
		}
	}
	free(chart_ux);
	free(chart_uy);
	free(chart_uz);
	free(chart_vx);
	free(chart_vy);
	free(chart_vz);

	// Normalize UVs per chart to origin
	for (int f = 0; f < face_count; f++) {
		int c = chart_id[f];
		for (int k = 0; k < 3; k++) {
			int idx = (f * 3 + k) * 2;
			uv_out[idx] -= c_min_u[c];
			uv_out[idx + 1] -= c_min_v[c];
		}
	}

	// Compute chart sizes and total area for scaling
	float *chart_w    = (float *)malloc(sizeof(float) * chart_count);
	float *chart_h    = (float *)malloc(sizeof(float) * chart_count);
	float  total_area = 0.0f;

	for (int c = 0; c < chart_count; c++) {
		chart_w[c] = c_max_u[c] - c_min_u[c];
		chart_h[c] = c_max_v[c] - c_min_v[c];
		if (chart_w[c] < 1e-10f)
			chart_w[c] = 1e-6f;
		if (chart_h[c] < 1e-10f)
			chart_h[c] = 1e-6f;
		total_area += chart_w[c] * chart_h[c];
	}
	free(c_min_u);
	free(c_min_v);
	free(c_max_u);
	free(c_max_v);

	// Sort charts by area (descending) for packing
	int *chart_order = (int *)malloc(sizeof(int) * chart_count);
	for (int i = 0; i < chart_count; i++) {
		chart_order[i] = i;
	}
	for (int i = 1; i < chart_count; i++) {
		int   key = chart_order[i];
		float ka  = chart_w[key] * chart_h[key];
		int   j   = i - 1;
		while (j >= 0 && chart_w[chart_order[j]] * chart_h[chart_order[j]] < ka) {
			chart_order[j + 1] = chart_order[j];
			j--;
		}
		chart_order[j + 1] = key;
	}

	// Determine per-chart rotation: if width > height, rotate 90 degrees for tighter packing
	bool  *chart_rotated = (bool *)calloc(chart_count, sizeof(bool));
	float *pack_w        = (float *)malloc(sizeof(float) * chart_count);
	float *pack_h        = (float *)malloc(sizeof(float) * chart_count);
	for (int c = 0; c < chart_count; c++) {
		if (chart_w[c] > chart_h[c]) {
			chart_rotated[c] = true;
			pack_w[c]        = chart_h[c];
			pack_h[c]        = chart_w[c];
		}
		else {
			pack_w[c] = chart_w[c];
			pack_h[c] = chart_h[c];
		}
	}

	// Skyline packing with binary search for optimal scale
	float *chart_off_u = (float *)malloc(sizeof(float) * chart_count);
	float *chart_off_v = (float *)malloc(sizeof(float) * chart_count);

	// Skyline: array of (x, y) pairs representing the top edge of placed islands
	int    sky_cap = chart_count + 1;
	float *sky_x   = (float *)malloc(sizeof(float) * sky_cap);
	float *sky_y   = (float *)malloc(sizeof(float) * sky_cap);
	int    sky_len;

	float margin   = UV_PACK_MARGIN;
	float scale_lo = 0.0f;
	float scale_hi = total_area > 1e-10f ? (2.0f / sqrtf(total_area)) : 2.0f;
	float scale    = 0.0f;

	// Binary search: find largest scale where all islands fit in [0,1]
	for (int iter = 0; iter < 40; iter++) {
		float try_scale = (scale_lo + scale_hi) * 0.5f;

		// Reset skyline
		sky_len  = 1;
		sky_x[0] = 0.0f;
		sky_y[0] = 0.0f;

		bool fits = true;
		for (int i = 0; i < chart_count; i++) {
			int   c  = chart_order[i];
			float cw = pack_w[c] * try_scale + margin;
			float ch = pack_h[c] * try_scale + margin;

			// Find best skyline position (lowest y where island fits)
			float best_x = 0.0f;
			float best_y = FLT_MAX;
			int   best_j = -1;

			for (int j = 0; j < sky_len; j++) {
				float x0    = sky_x[j];
				float x_end = sky_x[j] + cw;
				if (x_end > 1.0f + 1e-6f) {
					continue;
				}

				// Find max y across skyline segments this island spans
				float max_y = 0.0f;
				for (int k = j; k < sky_len; k++) {
					float seg_end = (k + 1 < sky_len) ? sky_x[k + 1] : 1.0f;
					if (sky_x[k] >= x_end - 1e-6f) {
						break;
					}
					if (sky_y[k] > max_y) {
						max_y = sky_y[k];
					}
				}

				if (max_y + ch <= 1.0f + 1e-6f && max_y < best_y) {
					best_y = max_y;
					best_x = x0;
					best_j = j;
				}
			}

			if (best_j == -1) {
				fits = false;
				break;
			}

			chart_off_u[c] = best_x + margin * 0.5f;
			chart_off_v[c] = best_y + margin * 0.5f;

			// Update skyline: insert new segment for this island
			float new_x0 = best_x;
			float new_x1 = best_x + cw;
			float new_y  = best_y + ch;

			// Collect segments that are NOT fully covered by the new island
			float tmp_x[1024];
			float tmp_y[1024];
			int   tmp_len = 0;

			for (int k = 0; k < sky_len; k++) {
				float seg_x0 = sky_x[k];
				float seg_x1 = (k + 1 < sky_len) ? sky_x[k + 1] : 1.0f;
				float seg_y  = sky_y[k];

				if (seg_x1 <= new_x0 + 1e-6f || seg_x0 >= new_x1 - 1e-6f) {
					// Segment fully outside new island
					if (tmp_len < 1024) {
						tmp_x[tmp_len] = seg_x0;
						tmp_y[tmp_len] = seg_y;
						tmp_len++;
					}
				}
				else {
					// Segment overlaps with new island
					if (seg_x0 < new_x0 - 1e-6f && tmp_len < 1024) {
						tmp_x[tmp_len] = seg_x0;
						tmp_y[tmp_len] = seg_y;
						tmp_len++;
					}
					// Insert the new island segment at its left edge
					if (tmp_len == 0 || tmp_x[tmp_len - 1] < new_x0 - 1e-6f || tmp_y[tmp_len - 1] != new_y) {
						if (tmp_len < 1024) {
							tmp_x[tmp_len] = new_x0;
							tmp_y[tmp_len] = new_y;
							tmp_len++;
						}
					}
					if (seg_x1 > new_x1 + 1e-6f && tmp_len < 1024) {
						tmp_x[tmp_len] = new_x1;
						tmp_y[tmp_len] = seg_y;
						tmp_len++;
					}
				}
			}

			// Deduplicate and copy back
			sky_len = 0;
			for (int k = 0; k < tmp_len && sky_len < sky_cap; k++) {
				if (sky_len > 0 && fabsf(tmp_y[k] - sky_y[sky_len - 1]) < 1e-6f) {
					continue; // Merge segments at same height
				}
				sky_x[sky_len] = tmp_x[k];
				sky_y[sky_len] = tmp_y[k];
				sky_len++;
			}
		}

		if (fits) {
			scale    = try_scale;
			scale_lo = try_scale;
		}
		else {
			scale_hi = try_scale;
		}
	}

	// Final pass with best scale to get definitive offsets
	sky_len  = 1;
	sky_x[0] = 0.0f;
	sky_y[0] = 0.0f;

	for (int i = 0; i < chart_count; i++) {
		int   c  = chart_order[i];
		float cw = pack_w[c] * scale + margin;
		float ch = pack_h[c] * scale + margin;

		float best_x = 0.0f;
		float best_y = FLT_MAX;
		int   best_j = -1;

		for (int j = 0; j < sky_len; j++) {
			float x_end = sky_x[j] + cw;
			if (x_end > 1.0f + 1e-6f) {
				continue;
			}
			float max_y = 0.0f;
			for (int k = j; k < sky_len; k++) {
				if (sky_x[k] >= x_end - 1e-6f) {
					break;
				}
				if (sky_y[k] > max_y) {
					max_y = sky_y[k];
				}
			}
			if (max_y + ch <= 1.0f + 1e-6f && max_y < best_y) {
				best_y = max_y;
				best_x = sky_x[j];
				best_j = j;
			}
		}

		chart_off_u[c] = best_x + margin * 0.5f;
		chart_off_v[c] = best_y + margin * 0.5f;

		float new_x0 = best_x;
		float new_x1 = best_x + cw;
		float new_y  = best_y + ch;

		float tmp_x[1024];
		float tmp_y[1024];
		int   tmp_len = 0;

		for (int k = 0; k < sky_len; k++) {
			float seg_x0 = sky_x[k];
			float seg_x1 = (k + 1 < sky_len) ? sky_x[k + 1] : 1.0f;
			float seg_y  = sky_y[k];

			if (seg_x1 <= new_x0 + 1e-6f || seg_x0 >= new_x1 - 1e-6f) {
				if (tmp_len < 1024) {
					tmp_x[tmp_len] = seg_x0;
					tmp_y[tmp_len] = seg_y;
					tmp_len++;
				}
			}
			else {
				if (seg_x0 < new_x0 - 1e-6f && tmp_len < 1024) {
					tmp_x[tmp_len] = seg_x0;
					tmp_y[tmp_len] = seg_y;
					tmp_len++;
				}
				if (tmp_len == 0 || tmp_x[tmp_len - 1] < new_x0 - 1e-6f || tmp_y[tmp_len - 1] != new_y) {
					if (tmp_len < 1024) {
						tmp_x[tmp_len] = new_x0;
						tmp_y[tmp_len] = new_y;
						tmp_len++;
					}
				}
				if (seg_x1 > new_x1 + 1e-6f && tmp_len < 1024) {
					tmp_x[tmp_len] = new_x1;
					tmp_y[tmp_len] = seg_y;
					tmp_len++;
				}
			}
		}

		sky_len = 0;
		for (int k = 0; k < tmp_len && sky_len < sky_cap; k++) {
			if (sky_len > 0 && fabsf(tmp_y[k] - sky_y[sky_len - 1]) < 1e-6f) {
				continue;
			}
			sky_x[sky_len] = tmp_x[k];
			sky_y[sky_len] = tmp_y[k];
			sky_len++;
		}
	}
	free(sky_x);
	free(sky_y);
	free(chart_order);

	// Apply packing offsets, scale, and rotation to all UVs
	for (int f = 0; f < face_count; f++) {
		int c = chart_id[f];
		for (int k = 0; k < 3; k++) {
			int   idx = (f * 3 + k) * 2;
			float u   = uv_out[idx];
			float v   = uv_out[idx + 1];
			if (chart_rotated[c]) {
				// Rotate 90 degrees: (u, v) -> (v, w - u) where w = chart_w[c]
				float ru = v;
				float rv = chart_w[c] - u;
				u        = ru;
				v        = rv;
			}
			uv_out[idx]     = u * scale + chart_off_u[c];
			uv_out[idx + 1] = v * scale + chart_off_v[c];
		}
	}
	free(chart_off_u);
	free(chart_off_v);
	free(chart_rotated);
	free(pack_w);
	free(pack_h);
	free(chart_w);
	free(chart_h);

	// Build output arrays (per-face-corner vertices)
	int       out_v_count = index_count;
	int16_t  *pa_out      = (int16_t *)malloc(sizeof(int16_t) * out_v_count * 4);
	int16_t  *na_out      = (int16_t *)malloc(sizeof(int16_t) * out_v_count * 2);
	int16_t  *ta_out      = (int16_t *)malloc(sizeof(int16_t) * out_v_count * 2);
	uint32_t *ia_out      = (uint32_t *)malloc(sizeof(uint32_t) * out_v_count);

	for (int i = 0; i < out_v_count; i++) {
		int vi            = indices[i];
		pa_out[i * 4]     = mesh->posa->buffer[vi * 4];
		pa_out[i * 4 + 1] = mesh->posa->buffer[vi * 4 + 1];
		pa_out[i * 4 + 2] = mesh->posa->buffer[vi * 4 + 2];
		pa_out[i * 4 + 3] = mesh->posa->buffer[vi * 4 + 3];
		na_out[i * 2]     = mesh->nora->buffer[vi * 2];
		na_out[i * 2 + 1] = mesh->nora->buffer[vi * 2 + 1];

		float u = uv_out[i * 2];
		float v = uv_out[i * 2 + 1];
		if (u < 0.0f)
			u = 0.0f;
		if (u > 1.0f)
			u = 1.0f;
		if (v < 0.0f)
			v = 0.0f;
		if (v > 1.0f)
			v = 1.0f;
		ta_out[i * 2]     = (int16_t)(u * 32767.0f);
		ta_out[i * 2 + 1] = (int16_t)((1.0f - v) * 32767.0f);

		ia_out[i] = i;
	}

	free(uv_out);
	free(chart_id);
	free(pa);

	// Replace mesh data
	free(mesh->posa->buffer);
	free(mesh->nora->buffer);
	free(mesh->inda->buffer);

	mesh->posa->buffer   = pa_out;
	mesh->posa->length   = out_v_count * 4;
	mesh->posa->capacity = out_v_count * 4;
	mesh->nora->buffer   = na_out;
	mesh->nora->length   = out_v_count * 2;
	mesh->nora->capacity = out_v_count * 2;

	if (mesh->texa == NULL) {
		mesh->texa = (i16_array_t *)calloc(1, sizeof(i16_array_t));
	}
	else {
		free(mesh->texa->buffer);
	}
	mesh->texa->buffer   = ta_out;
	mesh->texa->length   = out_v_count * 2;
	mesh->texa->capacity = out_v_count * 2;

	mesh->inda->buffer   = ia_out;
	mesh->inda->length   = out_v_count;
	mesh->inda->capacity = out_v_count;
	mesh->vertex_count   = out_v_count;
	mesh->index_count    = out_v_count;

	iron_log("Unwrapped in %fs\n", iron_time() - t);
}
