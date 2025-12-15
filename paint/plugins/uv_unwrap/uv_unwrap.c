
#include "uv_unwrap.h"
#include <float.h>
#include <iron_system.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	float x, y, z;
} vec3_t;

typedef struct {
	float x, y;
} vec2_t;

static inline vec3_t v3_add(vec3_t a, vec3_t b) {
	return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}
static inline vec3_t v3_sub(vec3_t a, vec3_t b) {
	return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}
static inline vec3_t v3_mul(vec3_t a, float s) {
	return (vec3_t){a.x * s, a.y * s, a.z * s};
}
static inline float v3_dot(vec3_t a, vec3_t b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline float v3_len(vec3_t a) {
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}
static inline vec3_t v3_cross(vec3_t a, vec3_t b) {
	return (vec3_t){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
static inline vec3_t v3_norm(vec3_t a) {
	float l = v3_len(a);
	if (l < 1e-6f)
		return (vec3_t){0, 0, 1};
	return v3_mul(a, 1.0f / l);
}

#define sb__raw(a)     ((int *)(a) - 2)
#define sb__len(a)     (sb__raw(a)[0])
#define sb__cap_val(a) (sb__raw(a)[1])
#define sb_count(a)    ((a) ? sb__len(a) : 0)
#define sb_cap(a)      ((a) ? sb__cap_val(a) : 0)
#define sb_free(a)     ((a) ? free(sb__raw(a)), 0 : 0)
#define sb_push(a, v)  (sb__grow((void **)&(a), sizeof(*(a))) ? ((a)[sb__len(a)++] = (v), 0) : 0)

static int sb__grow(void **ptr, size_t item_size) {
	int *arr = *ptr ? sb__raw(*ptr) : NULL;
	if (arr) {
		int len = arr[0];
		int cap = arr[1];
		if (len < cap)
			return 1;
		int  new_cap = cap * 2;
		int *new_arr = (int *)realloc(arr, sizeof(int) * 2 + item_size * new_cap);
		if (!new_arr)
			return 0;
		new_arr[1] = new_cap;
		*ptr       = new_arr + 2;
	}
	else {
		int  new_cap = 16;
		int *new_arr = (int *)malloc(sizeof(int) * 2 + item_size * new_cap);
		if (!new_arr)
			return 0;
		new_arr[0] = 0;
		new_arr[1] = new_cap;
		*ptr       = new_arr + 2;
	}
	return 1;
}

typedef struct {
	int    *faces; // SB
	vec2_t *uvs;   // SB
	vec2_t  min, max;
	int     p_x, p_y;
} Island;

typedef struct {
	float *pa;
	int    index;
} sort_vert_t;

typedef struct {
	int u1, u2, face;
} edge_ref_t;

typedef struct {
	int spatial_idx;
	int local_idx;
} relax_vert_t;

typedef struct {
	int u, v;
} relax_edge_t;

typedef struct pack_node {
	struct pack_node *child[2];
	int               x, y, w, h;
	int               occupied;
} pack_node_t;

static int compare_verts(const void *a, const void *b) {
	const sort_vert_t *va  = (const sort_vert_t *)a;
	const sort_vert_t *vb  = (const sort_vert_t *)b;
	float              eps = 0.0001f;
	if (fabsf(va->pa[0] - vb->pa[0]) > eps)
		return (va->pa[0] > vb->pa[0]) ? 1 : -1;
	if (fabsf(va->pa[1] - vb->pa[1]) > eps)
		return (va->pa[1] > vb->pa[1]) ? 1 : -1;
	if (fabsf(va->pa[2] - vb->pa[2]) > eps)
		return (va->pa[2] > vb->pa[2]) ? 1 : -1;
	return 0;
}

static int compare_edges(const void *a, const void *b) {
	const edge_ref_t *ea = (const edge_ref_t *)a;
	const edge_ref_t *eb = (const edge_ref_t *)b;
	if (ea->u1 != eb->u1)
		return ea->u1 - eb->u1;
	return ea->u2 - eb->u2;
}

static int compare_relax_verts(const void *a, const void *b) {
	const relax_vert_t *va = (const relax_vert_t *)a;
	const relax_vert_t *vb = (const relax_vert_t *)b;
	return va->spatial_idx - vb->spatial_idx;
}

static int compare_relax_edges(const void *a, const void *b) {
	const relax_edge_t *ea = (const relax_edge_t *)a;
	const relax_edge_t *eb = (const relax_edge_t *)b;
	if (ea->u != eb->u)
		return ea->u - eb->u;
	return ea->v - eb->v;
}

static int compare_islands_area(const void *a, const void *b) {
	const Island *ia     = (const Island *)a;
	const Island *ib     = (const Island *)b;
	float         area_a = (ia->max.x - ia->min.x) * (ia->max.y - ia->min.y);
	float         area_b = (ib->max.x - ib->min.x) * (ib->max.y - ib->min.y);
	return (area_a < area_b) ? 1 : -1;
}

static pack_node_t *pack_insert(pack_node_t *node, int w, int h) {
	if (!node)
		return NULL;

	if (w > node->w || h > node->h)
		return NULL;

	if (!node->child[0]) {
		if (node->occupied)
			return NULL;

		// Perfect fit or fits within node
		// If perfectly same size, use it
		if (w == node->w && h == node->h) {
			node->occupied = 1;
			return node;
		}

		// Split the node to create a perfect fit for the requested size
		node->child[0] = (pack_node_t *)calloc(1, sizeof(pack_node_t));
		node->child[1] = (pack_node_t *)calloc(1, sizeof(pack_node_t));

		// Decide split axis based on maximizing the remaining rectangle area
		int dw = node->w - w;
		int dh = node->h - h;

		if (dw > dh) {
			// Split vertically: Child 0 is Left (fit w), Child 1 is Right (remainder)
			node->child[0]->x = node->x;
			node->child[0]->y = node->y;
			node->child[0]->w = w;
			node->child[0]->h = node->h; // Keep full height

			node->child[1]->x = node->x + w;
			node->child[1]->y = node->y;
			node->child[1]->w = dw;
			node->child[1]->h = node->h;
		}
		else {
			// Split horizontally: Child 0 is Top (fit h), Child 1 is Bottom (remainder)
			node->child[0]->x = node->x;
			node->child[0]->y = node->y;
			node->child[0]->w = node->w; // Keep full width
			node->child[0]->h = h;

			node->child[1]->x = node->x;
			node->child[1]->y = node->y + h;
			node->child[1]->w = node->w;
			node->child[1]->h = dh;
		}

		// Insert into the newly created child that matches the dimension we just set up
		return pack_insert(node->child[0], w, h);
	}

	// Recursive insert
	pack_node_t *res = pack_insert(node->child[0], w, h);
	if (!res)
		res = pack_insert(node->child[1], w, h);
	return res;
}

static void free_pack_node(pack_node_t *node) {
	if (!node)
		return;
	free_pack_node(node->child[0]);
	free_pack_node(node->child[1]);
	free(node);
}

static void relax_island(Island *isl, uint32_t *indices, float *pa, int iterations) {
	if (sb_count(isl->faces) < 2)
		return;

	int num_uvs = sb_count(isl->uvs);

	relax_vert_t *rverts = (relax_vert_t *)malloc(sizeof(relax_vert_t) * num_uvs);
	for (int i = 0; i < num_uvs; ++i) {
		int f                 = i / 3;
		int v                 = i % 3;
		rverts[i].spatial_idx = indices[isl->faces[f] * 3 + v];
		rverts[i].local_idx   = i;
	}
	qsort(rverts, num_uvs, sizeof(relax_vert_t), compare_relax_verts);

	int *uv_weld_map      = (int *)malloc(sizeof(int) * num_uvs);
	int  current_group_id = -1;
	int  last_spatial     = -1;
	for (int i = 0; i < num_uvs; ++i) {
		if (rverts[i].spatial_idx != last_spatial) {
			current_group_id = rverts[i].local_idx;
			last_spatial     = rverts[i].spatial_idx;
		}
		uv_weld_map[rverts[i].local_idx] = current_group_id;
	}
	free(rverts);

	relax_edge_t *redges = NULL;
	typedef struct {
		int   count;
		int   neighbors[8];
		float dists[8];
	} VInfo;
	VInfo *v_info = (VInfo *)calloc(num_uvs, sizeof(VInfo));

	for (int f = 0; f < sb_count(isl->faces); ++f) {
		for (int k = 0; k < 3; ++k) {
			int u_local = f * 3 + k;
			int v_local = f * 3 + (k + 1) % 3;
			int u_weld  = uv_weld_map[u_local];
			int v_weld  = uv_weld_map[v_local];
			if (u_weld > v_weld) {
				int t  = u_weld;
				u_weld = v_weld;
				v_weld = t;
			}

			relax_edge_t re = {u_weld, v_weld};
			sb_push(redges, re);

			int    idx_u = indices[isl->faces[f] * 3 + k];
			int    idx_v = indices[isl->faces[f] * 3 + (k + 1) % 3];
			vec3_t p1    = {pa[idx_u * 3], pa[idx_u * 3 + 1], pa[idx_u * 3 + 2]};
			vec3_t p2    = {pa[idx_v * 3], pa[idx_v * 3 + 1], pa[idx_v * 3 + 2]};
			float  d     = v3_len(v3_sub(p1, p2));

			if (v_info[u_local].count < 8) {
				v_info[u_local].neighbors[v_info[u_local].count] = v_local;
				v_info[u_local].dists[v_info[u_local].count]     = d;
				v_info[u_local].count++;
			}
			if (v_info[v_local].count < 8) {
				v_info[v_local].neighbors[v_info[v_local].count] = u_local;
				v_info[v_local].dists[v_info[v_local].count]     = d;
				v_info[v_local].count++;
			}
		}
	}

	qsort(redges, sb_count(redges), sizeof(relax_edge_t), compare_relax_edges);

	char *weld_is_border = (char *)calloc(num_uvs, 1);
	int   num_edges      = sb_count(redges);
	int   i              = 0;
	while (i < num_edges) {
		int count = 1;
		while (i + count < num_edges && redges[i + count].u == redges[i].u && redges[i + count].v == redges[i].v)
			count++;
		if (count == 1) {
			weld_is_border[redges[i].u] = 1;
			weld_is_border[redges[i].v] = 1;
		}
		i += count;
	}
	sb_free(redges);

	char *is_border = (char *)calloc(num_uvs, 1);
	for (int k = 0; k < num_uvs; ++k) {
		if (weld_is_border[uv_weld_map[k]])
			is_border[k] = 1;
	}
	free(weld_is_border);

	vec2_t *temp_uvs = (vec2_t *)malloc(sizeof(vec2_t) * num_uvs);
	float  *sum_x    = (float *)calloc(num_uvs, sizeof(float));
	float  *sum_y    = (float *)calloc(num_uvs, sizeof(float));
	int    *counts   = (int *)calloc(num_uvs, sizeof(int));

	for (int iter = 0; iter < iterations; ++iter) {
		memcpy(temp_uvs, isl->uvs, sizeof(vec2_t) * num_uvs);

		for (int k = 0; k < num_uvs; ++k) {
			if (is_border[k])
				continue;

			vec2_t sum_pos    = {0, 0};
			float  weight_sum = 0;

			for (int n = 0; n < v_info[k].count; ++n) {
				int   neighbor_idx = v_info[k].neighbors[n];
				float target_dist  = v_info[k].dists[n];
				if (target_dist < 0.0001f)
					target_dist = 0.0001f;

				vec2_t n_pos = temp_uvs[neighbor_idx];
				float  w     = 1.0f / target_dist;

				sum_pos.x += n_pos.x * w;
				sum_pos.y += n_pos.y * w;
				weight_sum += w;
			}

			if (weight_sum > 0) {
				isl->uvs[k].x = sum_pos.x / weight_sum;
				isl->uvs[k].y = sum_pos.y / weight_sum;
			}
		}

		memset(sum_x, 0, sizeof(float) * num_uvs);
		memset(sum_y, 0, sizeof(float) * num_uvs);
		memset(counts, 0, sizeof(int) * num_uvs);

		for (int k = 0; k < num_uvs; ++k) {
			int wid = uv_weld_map[k];
			sum_x[wid] += isl->uvs[k].x;
			sum_y[wid] += isl->uvs[k].y;
			counts[wid]++;
		}

		for (int k = 0; k < num_uvs; ++k) {
			int wid = uv_weld_map[k];
			if (counts[wid] > 1) {
				isl->uvs[k].x = sum_x[wid] / counts[wid];
				isl->uvs[k].y = sum_y[wid] / counts[wid];
			}
		}
	}

	free(sum_x);
	free(sum_y);
	free(counts);
	free(temp_uvs);
	free(uv_weld_map);
	free(v_info);
	free(is_border);
}

void proc_uv_unwrap(raw_mesh_t *mesh) {
	double t = iron_time();

	// Prepare Data
	int    vertex_count = mesh->posa->length / 4;
	float *pa           = (float *)malloc(sizeof(float) * vertex_count * 3);
	float *na           = (float *)malloc(sizeof(float) * vertex_count * 3);
	float  inv          = 1.0f / 32767.0f;

	for (int i = 0; i < vertex_count; i++) {
		pa[i * 3]     = mesh->posa->buffer[i * 4] * inv;
		pa[i * 3 + 1] = mesh->posa->buffer[i * 4 + 1] * inv;
		pa[i * 3 + 2] = mesh->posa->buffer[i * 4 + 2] * inv;
		na[i * 3]     = mesh->nora->buffer[i * 2] * inv;
		na[i * 3 + 1] = mesh->nora->buffer[i * 2 + 1] * inv;
		na[i * 3 + 2] = mesh->posa->buffer[i * 4 + 3] * inv;
	}

	int       index_count = mesh->inda->length;
	uint32_t *indices     = mesh->inda->buffer;
	int       face_count  = index_count / 3;

	// Weld
	int         *weld_map = (int *)malloc(sizeof(int) * vertex_count);
	sort_vert_t *sverts   = (sort_vert_t *)malloc(sizeof(sort_vert_t) * vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		sverts[i].pa    = &pa[i * 3];
		sverts[i].index = i;
	}
	qsort(sverts, vertex_count, sizeof(sort_vert_t), compare_verts);

	int   unique_counter = 0;
	float eps            = 0.0001f;
	for (int i = 0; i < vertex_count; i++) {
		if (i > 0) {
			if (fabsf(sverts[i].pa[0] - sverts[i - 1].pa[0]) > eps || fabsf(sverts[i].pa[1] - sverts[i - 1].pa[1]) > eps ||
			    fabsf(sverts[i].pa[2] - sverts[i - 1].pa[2]) > eps) {
				unique_counter++;
			}
		}
		weld_map[sverts[i].index] = unique_counter;
	}
	free(sverts);

	// Adjacency
	vec3_t *face_normals = (vec3_t *)malloc(sizeof(vec3_t) * face_count);
	for (int i = 0; i < face_count; i++) {
		uint32_t i0     = indices[i * 3];
		uint32_t i1     = indices[i * 3 + 1];
		uint32_t i2     = indices[i * 3 + 2];
		vec3_t   v0     = {pa[i0 * 3], pa[i0 * 3 + 1], pa[i0 * 3 + 2]};
		vec3_t   v1     = {pa[i1 * 3], pa[i1 * 3 + 1], pa[i1 * 3 + 2]};
		vec3_t   v2     = {pa[i2 * 3], pa[i2 * 3 + 1], pa[i2 * 3 + 2]};
		face_normals[i] = v3_norm(v3_cross(v3_sub(v1, v0), v3_sub(v2, v0)));
	}

	edge_ref_t *edges = (edge_ref_t *)malloc(sizeof(edge_ref_t) * face_count * 3);
	for (int i = 0; i < face_count; i++) {
		for (int j = 0; j < 3; j++) {
			int idx1 = weld_map[indices[i * 3 + j]];
			int idx2 = weld_map[indices[i * 3 + (j + 1) % 3]];
			if (idx1 > idx2) {
				int t = idx1;
				idx1  = idx2;
				idx2  = t;
			}
			edges[i * 3 + j] = (edge_ref_t){idx1, idx2, i};
		}
	}
	qsort(edges, face_count * 3, sizeof(edge_ref_t), compare_edges);

	int **adj = (int **)calloc(face_count, sizeof(int *));
	for (int i = 0; i < face_count * 3 - 1; i++) {
		if (edges[i].u1 == edges[i + 1].u1 && edges[i].u2 == edges[i + 1].u2) {
			int f1 = edges[i].face;
			int f2 = edges[i + 1].face;
			sb_push(adj[f1], f2);
			sb_push(adj[f2], f1);
		}
	}
	free(edges);

	// Segmentation
	float   angle_limit_deg = 66.0f;
	float   angle_threshold = cosf(angle_limit_deg * (3.14159f / 180.0f));
	int    *face_visited    = (int *)calloc(face_count, sizeof(int));
	Island *islands         = NULL;

	for (int i = 0; i < face_count; i++) {
		if (face_visited[i])
			continue;

		Island island = {0};
		int   *queue  = NULL;
		sb_push(queue, i);
		face_visited[i] = 1;
		sb_push(island.faces, i);

		int    head       = 0;
		vec3_t island_avg = face_normals[i];

		while (head < sb_count(queue)) {
			int  curr_f    = queue[head++];
			int *neighbors = adj[curr_f];
			for (int k = 0; k < sb_count(neighbors); k++) {
				int next_f = neighbors[k];
				if (face_visited[next_f])
					continue;

				float dot_local  = v3_dot(face_normals[curr_f], face_normals[next_f]);
				float dot_global = v3_dot(v3_norm(island_avg), face_normals[next_f]);

				if (dot_local > angle_threshold && dot_global > 0.5f) {
					face_visited[next_f] = 1;
					sb_push(island.faces, next_f);
					sb_push(queue, next_f);
					island_avg = v3_add(island_avg, face_normals[next_f]);
				}
			}
		}

		// Project
		island_avg = v3_norm(island_avg);
		vec3_t up  = {0, 1, 0};
		if (fabsf(v3_dot(up, island_avg)) > 0.9f)
			up = (vec3_t){0, 0, 1};
		vec3_t right = v3_norm(v3_cross(up, island_avg));
		up           = v3_norm(v3_cross(island_avg, right));

		for (int k = 0; k < sb_count(island.faces); k++) {
			int f = island.faces[k];
			for (int v = 0; v < 3; v++) {
				int    idx = indices[f * 3 + v];
				vec3_t pos = {pa[idx * 3], pa[idx * 3 + 1], pa[idx * 3 + 2]};
				vec2_t uv;
				uv.x = v3_dot(pos, right);
				uv.y = v3_dot(pos, up);
				sb_push(island.uvs, uv);
			}
		}

		// Relax
		relax_island(&island, indices, pa, 5);

		// Calc Bounds
		island.min = (vec2_t){FLT_MAX, FLT_MAX};
		island.max = (vec2_t){-FLT_MAX, -FLT_MAX};
		for (int k = 0; k < sb_count(island.uvs); k++) {
			if (island.uvs[k].x < island.min.x)
				island.min.x = island.uvs[k].x;
			if (island.uvs[k].y < island.min.y)
				island.min.y = island.uvs[k].y;
			if (island.uvs[k].x > island.max.x)
				island.max.x = island.uvs[k].x;
			if (island.uvs[k].y > island.max.y)
				island.max.y = island.uvs[k].y;
		}

		// Normalize to local 0,0
		for (int k = 0; k < sb_count(island.uvs); k++) {
			island.uvs[k].x -= island.min.x;
			island.uvs[k].y -= island.min.y;
		}

		sb_push(islands, island);
		sb_free(queue);
	}

	free(face_visited);
	for (int i = 0; i < face_count; i++)
		sb_free(adj[i]);
	free(adj);
	free(weld_map);

	// Sort by Area
	qsort(islands, sb_count(islands), sizeof(Island), compare_islands_area);

	int map_size = 2048;
	int padding  = 4;

	// Calc total area
	float total_area = 0;
	for (int i = 0; i < sb_count(islands); i++) {
		total_area += (islands[i].max.x - islands[i].min.x) * (islands[i].max.y - islands[i].min.y);
	}

	// Iterative Fit
	float ideal_scale = 1.0f;
	if (total_area > 0.0001f) {
		ideal_scale = sqrtf((map_size * map_size * 1.0f) / total_area); // Start high
	}
	if (ideal_scale > 1000.0f)
		ideal_scale = 1000.0f;

	float final_scale = 0;

	// Retry loop
	for (float try_scale = ideal_scale; try_scale > 0.001f; try_scale *= 0.95f) {
		pack_node_t *root = (pack_node_t *)calloc(1, sizeof(pack_node_t));
		root->w           = map_size;
		root->h           = map_size;

		int success = 1;
		for (int i = 0; i < sb_count(islands); i++) {
			int w = (int)((islands[i].max.x - islands[i].min.x) * try_scale) + padding * 2;
			int h = (int)((islands[i].max.y - islands[i].min.y) * try_scale) + padding * 2;
			if (w < 1)
				w = 1;
			if (h < 1)
				h = 1;

			pack_node_t *node = pack_insert(root, w, h);
			if (node) {
				// Store temporary packed coords in island
				islands[i].p_x = node->x;
				islands[i].p_y = node->y;
			}
			else {
				success = 0;
				break;
			}
		}

		free_pack_node(root);

		if (success) {
			final_scale = try_scale;
			break;
		}
	}

	// Final Apply
	for (int i = 0; i < sb_count(islands); i++) {
		float off_x = (islands[i].p_x + padding) / (float)map_size;
		float off_y = (islands[i].p_y + padding) / (float)map_size;

		// Recalculate dimensions based on the winning scale
		int w_px = (int)((islands[i].max.x - islands[i].min.x) * final_scale) + padding * 2;
		int h_px = (int)((islands[i].max.y - islands[i].min.y) * final_scale) + padding * 2;

		float uv_w = (w_px - padding * 2) / (float)map_size;
		float uv_h = (h_px - padding * 2) / (float)map_size;

		float local_w = (islands[i].max.x - islands[i].min.x);
		float local_h = (islands[i].max.y - islands[i].min.y);

		for (int k = 0; k < sb_count(islands[i].uvs); k++) {
			float u_norm        = (local_w > 1e-6f) ? islands[i].uvs[k].x / local_w : 0.0f;
			float v_norm        = (local_h > 1e-6f) ? islands[i].uvs[k].y / local_h : 0.0f;
			islands[i].uvs[k].x = off_x + u_norm * uv_w;
			islands[i].uvs[k].y = off_y + v_norm * uv_h;
		}
	}

	// Output
	int16_t  *pa_out      = NULL;
	int16_t  *na_out      = NULL;
	int16_t  *ta_out      = NULL;
	uint32_t *ia_out      = NULL;
	int       out_v_count = 0;

	for (int i = 0; i < sb_count(islands); i++) {
		Island *isl = &islands[i];
		for (int f = 0; f < sb_count(isl->faces); f++) {
			int face_idx = isl->faces[f];
			for (int v = 0; v < 3; v++) {
				uint32_t old_idx = indices[face_idx * 3 + v];
				sb_push(pa_out, (int16_t)(pa[old_idx * 3] / inv));
				sb_push(pa_out, (int16_t)(pa[old_idx * 3 + 1] / inv));
				sb_push(pa_out, (int16_t)(pa[old_idx * 3 + 2] / inv));
				sb_push(pa_out, (int16_t)(na[old_idx * 3 + 2] / inv));
				sb_push(na_out, (int16_t)(na[old_idx * 3] / inv));
				sb_push(na_out, (int16_t)(na[old_idx * 3 + 1] / inv));
				vec2_t uv = isl->uvs[f * 3 + v];
				sb_push(ta_out, (int16_t)(uv.x / inv));
				sb_push(ta_out, (int16_t)(uv.y / inv));
				sb_push(ia_out, out_v_count);
				out_v_count++;
			}
		}
		sb_free(isl->faces);
		sb_free(isl->uvs);
	}
	sb_free(islands);

	if (mesh->posa->buffer)
		free(mesh->posa->buffer);
	if (mesh->nora->buffer)
		free(mesh->nora->buffer);
	if (mesh->inda->buffer)
		free(mesh->inda->buffer);
	if (mesh->texa) {
		if (mesh->texa->buffer)
			free(mesh->texa->buffer);
		free(mesh->texa);
	}

	mesh->posa->buffer   = pa_out;
	mesh->posa->length   = out_v_count * 4;
	mesh->posa->capacity = sb_cap(pa_out);
	mesh->nora->buffer   = na_out;
	mesh->nora->length   = out_v_count * 2;
	mesh->texa           = (i16_array_t *)malloc(sizeof(i16_array_t));
	mesh->texa->buffer   = ta_out;
	mesh->texa->length   = out_v_count * 2;
	mesh->texa->capacity = sb_cap(ta_out);
	mesh->inda->buffer   = ia_out;
	mesh->inda->length   = out_v_count;
	mesh->inda->capacity = sb_cap(ia_out);
	mesh->vertex_count   = out_v_count;
	mesh->index_count    = out_v_count;
	free(pa);
	free(na);
	free(face_normals);
	iron_log("Unwrapped in %fs\n", iron_time() - t);
}
