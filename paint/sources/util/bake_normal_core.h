#pragma once

// Geometry-only baker. No renderer, project state or allocations owned by the UI.
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct bn_v3 {
	float x, y, z;
} bn_v3;
typedef struct bn_face {
	bn_v3 p[3], n[3];
} bn_face;
typedef struct bn_low {
	bn_face face;
	float   uv[3][2];
} bn_low;
typedef struct bn_node {
	bn_v3 min, max;
	int   first, count, left, right;
} bn_node;
typedef struct bn_tree {
	const bn_face *faces;
	int           *order;
	bn_node       *nodes;
	int            used;
	int            total, completed;
} bn_tree;
typedef enum {
	BN_NORMAL,
	BN_AO,
	BN_CURVATURE,
	BN_HEIGHT
} bn_type;
typedef struct bn_options {
	int   size, padding, samples;
	float outer, inner;
	bool  directx, backfaces;
	int   type, rays;
	float radius, strength;
} bn_options;
typedef struct bn_result {
	uint8_t *rgba, *mask;
	int      covered, missed;
	uint8_t *face_risk;
} bn_result;
typedef bool (*bn_progress)(void *, int);

static bn_v3 bn_add(bn_v3 a, bn_v3 b) {
	return (bn_v3){a.x + b.x, a.y + b.y, a.z + b.z};
}
static bool bn_finite(bn_v3 p) {
	return isfinite(p.x) && isfinite(p.y) && isfinite(p.z);
}
static bn_v3 bn_sub(bn_v3 a, bn_v3 b) {
	return (bn_v3){a.x - b.x, a.y - b.y, a.z - b.z};
}
static bn_v3 bn_mul(bn_v3 a, float s) {
	return (bn_v3){a.x * s, a.y * s, a.z * s};
}
static float bn_dot(bn_v3 a, bn_v3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
static bn_v3 bn_cross(bn_v3 a, bn_v3 b) {
	return (bn_v3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
static bn_v3 bn_norm(bn_v3 a) {
	float d = bn_dot(a, a);
	return d > 1e-30f ? bn_mul(a, 1.0f / sqrtf(d)) : (bn_v3){0, 0, 0};
}
static float bn_axis(bn_v3 a, int k) {
	return k == 0 ? a.x : k == 1 ? a.y : a.z;
}
static bn_v3 bn_min(bn_v3 a, bn_v3 b) {
	return (bn_v3){fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)};
}
static bn_v3 bn_max(bn_v3 a, bn_v3 b) {
	return (bn_v3){fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)};
}
static bn_v3 bn_lerp3(bn_v3 *v, float a, float b, float c) {
	return bn_add(bn_add(bn_mul(v[0], a), bn_mul(v[1], b)), bn_mul(v[2], c));
}
static float bn_centroid(const bn_face *f, int axis) {
	return (bn_axis(f->p[0], axis) + bn_axis(f->p[1], axis) + bn_axis(f->p[2], axis)) / 3.0f;
}

static int bn_build_node(bn_tree *t, int first, int count, bn_progress progress, void *user) {
	if (progress && !progress(user, t->total ? t->completed * 15 / t->total : 0))
		return -1;
	int      index = t->used++;
	bn_node *n     = &t->nodes[index];
	n->first       = first;
	n->count       = count;
	n->left = n->right = -1;
	n->min             = (bn_v3){INFINITY, INFINITY, INFINITY};
	n->max             = bn_mul(n->min, -1);
	for (int i = first; i < first + count; i++)
		for (int k = 0; k < 3; k++) {
			bn_v3 p = t->faces[t->order[i]].p[k];
			n->min  = bn_min(n->min, p);
			n->max  = bn_max(n->max, p);
		}
	if (count <= 8) {
		t->completed += count;
		return index;
	}
	bn_v3 ext  = bn_sub(n->max, n->min);
	int   axis = ext.x > ext.y ? (ext.x > ext.z ? 0 : 2) : (ext.y > ext.z ? 1 : 2);
	int   lo = first, hi = first + count - 1, mid = first + count / 2;
	// Median partition keeps recursion bounded, including coincident triangles.
	while (lo < hi) {
		float pivot = bn_centroid(&t->faces[t->order[(lo + hi) / 2]], axis);
		int   a = lo, b = hi;
		while (a <= b) {
			while (bn_centroid(&t->faces[t->order[a]], axis) < pivot)
				a++;
			while (bn_centroid(&t->faces[t->order[b]], axis) > pivot)
				b--;
			if (a <= b) {
				int tmp       = t->order[a];
				t->order[a++] = t->order[b];
				t->order[b--] = tmp;
			}
		}
		if (mid <= b)
			hi = b;
		else if (mid >= a)
			lo = a;
		else
			break;
	}
	n->left = bn_build_node(t, first, mid - first, progress, user);
	if (n->left < 0)
		return -1;
	n->right = bn_build_node(t, mid, first + count - mid, progress, user);
	if (n->right < 0)
		return -1;
	n->count = 0;
	return index;
}

static bool bn_box(const bn_node *n, bn_v3 o, bn_v3 d, float limit) {
	float t_near = 0, t_far = limit;
	for (int k = 0; k < 3; k++) {
		float v = bn_axis(d, k), p = bn_axis(o, k), a = bn_axis(n->min, k), b = bn_axis(n->max, k);
		if (fabsf(v) < 1e-20f) {
			if (p < a || p > b)
				return false;
			continue;
		}
		float x = (a - p) / v, y = (b - p) / v;
		if (x > y) {
			float z = x;
			x       = y;
			y       = z;
		}
		t_near = fmaxf(t_near, x);
		t_far  = fminf(t_far, y);
		if (t_near > t_far)
			return false;
	}
	return true;
}

static bool bn_hit(const bn_tree *tree, bn_v3 origin, bn_v3 direction, float limit, bool backfaces, bn_v3 *normal, float *hit_distance) {
	int stack[64], top = 0;
	stack[top++] = 0;
	bool found   = false;
	while (top) {
		const bn_node *node = &tree->nodes[stack[--top]];
		if (!bn_box(node, origin, direction, limit))
			continue;
		if (!node->count) {
			stack[top++] = node->left;
			stack[top++] = node->right;
			continue;
		}
		for (int i = node->first; i < node->first + node->count; i++) {
			const bn_face *f  = &tree->faces[tree->order[i]];
			bn_v3          e1 = bn_sub(f->p[1], f->p[0]), e2 = bn_sub(f->p[2], f->p[0]), h = bn_cross(direction, e2);
			float          det = bn_dot(e1, h), epsilon = 1e-8f * sqrtf(bn_dot(e1, e1) * bn_dot(e2, e2));
			if (fabsf(det) <= epsilon || (!backfaces && det < 0))
				continue;
			float inv = 1.0f / det;
			bn_v3 s   = bn_sub(origin, f->p[0]);
			float u   = bn_dot(s, h) * inv;
			if (u < 0 || u > 1)
				continue;
			bn_v3 q = bn_cross(s, e1);
			float v = bn_dot(direction, q) * inv;
			if (v < 0 || u + v > 1)
				continue;
			float distance = bn_dot(e2, q) * inv;
			if (distance < 0 || distance > limit)
				continue;
			limit = distance;
			if (hit_distance)
				*hit_distance = distance;
			found   = true;
			*normal = bn_norm(bn_add(bn_add(bn_mul(f->n[0], 1 - u - v), bn_mul(f->n[1], u)), bn_mul(f->n[2], v)));
			if (bn_dot(*normal, *normal) < 0.5f)
				*normal = bn_norm(bn_cross(e1, e2));
		}
	}
	return found;
}

static void bn_result_free(bn_result *r) {
	free(r->rgba);
	free(r->mask);
	free(r->face_risk);
	memset(r, 0, sizeof(*r));
}
static uint8_t bn_encode(float v) {
	return (uint8_t)lroundf(fmaxf(0, fminf(1, v * 0.5f + 0.5f)) * 255);
}

static uint8_t bn_risk(bool hit, float height, float alignment, bn_options opt) {
	if (!hit)
		return 3;
	if (alignment < 0.5f)
		return 2;
	float reach = height >= 0 ? opt.outer : opt.inner;
	return reach > 0 && fabsf(height) > reach * 0.85f ? 1 : 0;
}

static void bn_basis(bn_v3 n, bn_v3 *t, bn_v3 *b) {
	*t = bn_norm(bn_cross(fabsf(n.z) < 0.9f ? (bn_v3){0, 0, 1} : (bn_v3){0, 1, 0}, n));
	*b = bn_cross(n, *t);
}

static float bn_scalar(const bn_tree *tree, bn_v3 p, bn_v3 n, float height, bn_options opt) {
	if (opt.type == BN_HEIGHT)
		return fmaxf(0, fminf(1, 0.5f + 0.5f * height / fmaxf(opt.outer, opt.inner)));
	bn_v3 t, b;
	bn_basis(n, &t, &b);
	float sum = 0, bias = fmaxf(1e-7f, opt.radius * 1e-4f);
	int   count = 0;
	for (int i = 0; i < opt.rays; i++) {
		float angle  = i * 2.39996323f;
		bn_v3 radial = bn_add(bn_mul(t, cosf(angle)), bn_mul(b, sinf(angle))), hn;
		if (opt.type == BN_AO) {
			float u         = (i + 0.5f) / opt.rays;
			bn_v3 direction = bn_add(bn_mul(radial, sqrtf(u)), bn_mul(n, sqrtf(1 - u)));
			if (!bn_hit(tree, bn_add(p, bn_mul(n, bias)), direction, opt.radius, true, &hn, NULL))
				sum += 1;
			count++;
		}
		else {
			bn_v3 origin = bn_add(bn_add(p, bn_mul(radial, opt.radius)), bn_mul(n, opt.radius));
			if (bn_hit(tree, origin, bn_mul(n, -1), 2 * opt.radius, opt.backfaces, &hn, NULL)) {
				sum += bn_dot(bn_sub(hn, n), radial);
				count++;
			}
		}
	}
	if (opt.type == BN_AO)
		return count ? sum / count : 1;
	return fmaxf(0, fminf(1, 0.5f + (count ? sum / count : 0) * opt.strength * 0.5f));
}

static bool bn_tree_init(bn_tree *tree, const bn_face *high, int count, bn_progress progress, void *user) {
	*tree = (bn_tree){.faces = high, .total = count};
	if (!high || count <= 0)
		return false;
	tree->order = malloc((size_t)count * sizeof(int));
	tree->nodes = malloc(((size_t)count / 2 + 2) * sizeof(bn_node));
	if (!tree->order || !tree->nodes)
		return false;
	for (int i = 0; i < count; i++) {
		tree->order[i] = i;
		for (int k = 0; k < 3; k++)
			if (!bn_finite(high[i].p[k]) || !bn_finite(high[i].n[k]))
				return false;
	}
	return bn_build_node(tree, 0, count, progress, user) >= 0;
}

// Sample the same low-poly faces displayed by the reduced wireframe preview.
static bool bn_analyze(const bn_low *low, int count, const bn_tree *tree, bn_options opt, uint8_t *risk, bn_progress progress, void *user) {
	int step = count / 16000 + 1;
	for (int i = 0; i < count; i += step) {
		if (progress && !progress(user, 20 + i * 70 / count))
			return false;
		uint8_t worst = 0;
		for (int j = 0; j < 4; j++) {
			float   a = j == 0 ? 1.0f / 3 : j == 1 ? 0.8f : 0.1f, b = j == 0 ? 1.0f / 3 : j == 2 ? 0.8f : 0.1f, c = 1 - a - b;
			bn_v3   p = bn_lerp3((bn_v3 *)low[i].face.p, a, b, c), n = bn_norm(bn_lerp3((bn_v3 *)low[i].face.n, a, b, c)), hn;
			float   distance = 0;
			bool    hit      = bn_hit(tree, bn_add(p, bn_mul(n, opt.outer)), bn_mul(n, -1), opt.outer + opt.inner, opt.backfaces, &hn, &distance);
			uint8_t r        = bn_risk(hit, opt.outer - distance, hit ? bn_dot(n, hn) : 0, opt);
			if (r > worst)
				worst = r;
		}
		risk[i] = worst;
	}
	return !progress || progress(user, 100);
}

// UV coordinates use image orientation (V points down). Encode OpenGL +Y by
// negating the image-space bitangent, matching ArmorPaint's normal-map reader.
static bool bn_bake_tree(const bn_low *low, int low_count, const bn_tree *tree, bn_options opt, bn_result *out, bn_progress progress, void *user) {
	memset(out, 0, sizeof(*out));
	if (!low || !tree || low_count <= 0 || opt.size < 16 || opt.size > 8192 || !isfinite(opt.outer) || !isfinite(opt.inner) ||
	    !isfinite(opt.outer + opt.inner) || opt.outer < 0 || opt.inner < 0 || opt.outer + opt.inner <= 0 || opt.padding < 0 || opt.padding > 128 ||
	    (opt.samples != 1 && opt.samples != 4))
		return false;
	if (opt.type < BN_NORMAL || opt.type > BN_HEIGHT)
		return false;
	if ((opt.type == BN_AO || opt.type == BN_CURVATURE) &&
	    (!isfinite(opt.radius) || opt.radius <= 0 || opt.rays < 1 || opt.rays > 128 || !isfinite(opt.strength) || opt.strength <= 0))
		return false;
	bool   ok     = true;
	size_t pixels = (size_t)opt.size * opt.size;
	if (ok) {
		out->rgba      = malloc(pixels * 4);
		out->mask      = calloc(pixels, 1);
		out->face_risk = calloc(low_count, 1);
		ok             = out->rgba && out->mask && out->face_risk;
	}
	if (!ok) {
		bn_result_free(out);
		return false;
	}
	for (size_t i = 0; i < pixels; i++) {
		out->rgba[i * 4]     = opt.type == BN_AO ? 255 : 128;
		out->rgba[i * 4 + 1] = opt.type == BN_AO ? 255 : 128;
		out->rgba[i * 4 + 2] = opt.type == BN_NORMAL || opt.type == BN_AO ? 255 : 128;
		out->rgba[i * 4 + 3] = 255;
	}
	for (int ti = 0; ti < low_count; ti++) {
		if (progress && !progress(user, 20 + ti * 70 / low_count)) {
			ok = false;
			break;
		}
		const bn_low *l = &low[ti];
		float         x[3], y[3];
		for (int k = 0; k < 3; k++) {
			x[k] = l->uv[k][0] * opt.size;
			y[k] = l->uv[k][1] * opt.size;
		}
		float area = (x[1] - x[0]) * (y[2] - y[0]) - (x[2] - x[0]) * (y[1] - y[0]);
		if (!isfinite(area) || fabsf(area) < 1e-8f)
			continue;
		bn_v3 e1 = bn_sub(l->face.p[1], l->face.p[0]), e2 = bn_sub(l->face.p[2], l->face.p[0]);
		float du1 = l->uv[1][0] - l->uv[0][0], dv1 = l->uv[1][1] - l->uv[0][1];
		float du2 = l->uv[2][0] - l->uv[0][0], dv2 = l->uv[2][1] - l->uv[0][1];
		int   x0 = (int)fmaxf(0, fminf(opt.size - 1, floorf(fminf(x[0], fminf(x[1], x[2])))));
		int   y0 = (int)fmaxf(0, fminf(opt.size - 1, floorf(fminf(y[0], fminf(y[1], y[2])))));
		int   x1 = (int)fmaxf(0, fminf(opt.size - 1, ceilf(fmaxf(x[0], fmaxf(x[1], x[2])))));
		int   y1 = (int)fmaxf(0, fminf(opt.size - 1, ceilf(fmaxf(y[0], fmaxf(y[1], y[2])))));
		for (int py = y0; py <= y1; py++) {
			if (progress && !progress(user, 20 + (int)(70.0f * (ti + (py - y0) / (float)(y1 - y0 + 1)) / low_count))) {
				ok = false;
				break;
			}
			for (int px = x0; px <= x1; px++) {
				size_t at = (size_t)py * opt.size + px;
				if (out->mask[at] == 2)
					continue;
				bn_v3 sum     = {0};
				int   hits    = 0;
				bool  covered = false;
				for (int sample = 0; sample < opt.samples; sample++) {
					float cx = px + (opt.samples == 1 ? 0.5f : 0.25f + 0.5f * (sample & 1));
					float cy = py + (opt.samples == 1 ? 0.5f : 0.25f + 0.5f * (sample >> 1));
					float a  = ((x[1] - cx) * (y[2] - cy) - (x[2] - cx) * (y[1] - cy)) / area;
					float b = ((x[2] - cx) * (y[0] - cy) - (x[0] - cx) * (y[2] - cy)) / area, c = 1 - a - b;
					if (a < 0 || b < 0 || c < 0)
						continue;
					covered   = true;
					bn_v3   p = bn_lerp3((bn_v3 *)l->face.p, a, b, c), n = bn_norm(bn_lerp3((bn_v3 *)l->face.n, a, b, c)), hn;
					bn_v3   origin   = bn_add(p, bn_mul(n, opt.outer));
					float   distance = 0;
					bool    hit      = bn_hit(tree, origin, bn_mul(n, -1), opt.outer + opt.inner, opt.backfaces, &hn, &distance);
					uint8_t risk     = bn_risk(hit, opt.outer - distance, hit ? bn_dot(n, hn) : 0, opt);
					if (risk > out->face_risk[ti])
						out->face_risk[ti] = risk;
					if (!hit)
						continue;
					if (opt.type != BN_NORMAL) {
						float value = bn_scalar(tree, bn_add(origin, bn_mul(n, -distance)), hn, opt.outer - distance, opt);
						sum.x += value;
						hits++;
						continue;
					}
					// Match the renderer's cotangent frame, including relative axis lengths
					// on stretched UVs. Independent normalization changes the decoded normal.
					bn_v3 e2perp = bn_cross(e2, n), e1perp = bn_cross(n, e1);
					bn_v3 t          = bn_add(bn_mul(e2perp, du1), bn_mul(e1perp, du2));
					bn_v3 bt         = bn_add(bn_mul(e2perp, dv1), bn_mul(e1perp, dv2));
					float max_length = fmaxf(bn_dot(t, t), bn_dot(bt, bt));
					if (max_length < 1e-30f)
						continue;
					t  = bn_mul(t, 1.0f / sqrtf(max_length));
					bt = bn_mul(bt, 1.0f / sqrtf(max_length));
					// Solve the basis instead of dotting against a possibly skewed UV frame.
					float det = bn_dot(t, bn_cross(bt, n));
					if (fabsf(det) < 1e-8f)
						continue;
					bn_v3 tn = {bn_dot(hn, bn_cross(bt, n)) / det, bn_dot(hn, bn_cross(n, t)) / det, bn_dot(hn, bn_cross(t, bt)) / det};
					tn.y *= opt.directx ? 1 : -1;
					sum = bn_add(sum, bn_norm(tn));
					hits++;
				}
				if (!covered)
					continue;
				if (!out->mask[at]) {
					out->covered++;
					out->missed++;
					out->mask[at] = 1;
				}
				if (hits) {
					out->missed--;
					out->mask[at] = 2;
					if (opt.type == BN_NORMAL) {
						sum                   = bn_norm(sum);
						out->rgba[at * 4]     = bn_encode(sum.x);
						out->rgba[at * 4 + 1] = bn_encode(sum.y);
						out->rgba[at * 4 + 2] = bn_encode(sum.z);
					}
					else
						out->rgba[at * 4] = out->rgba[at * 4 + 1] = out->rgba[at * 4 + 2] = (uint8_t)lroundf(sum.x / hits * 255);
				}
			}
		}
		if (!ok)
			break;
	}
	// Breadth-first dilation never fills missed surface texels, only empty UV space.
	if (ok && opt.padding > 0) {
		int     *queue    = malloc(pixels * sizeof(int));
		uint8_t *distance = calloc(pixels, 1);
		if (!queue || !distance)
			ok = false;
		if (ok) {
			size_t head = 0, tail = 0;
			for (size_t i = 0; i < pixels; i++)
				if (out->mask[i] == 2) {
					queue[tail++] = (int)i;
					distance[i]   = 1;
				}
			while (head < tail) {
				int at = queue[head++];
				if ((head & 4095) == 0 && progress && !progress(user, 90 + (int)(head * 9 / pixels))) {
					ok = false;
					break;
				}
				if (distance[at] > opt.padding)
					continue;
				int x = at % opt.size, y = at / opt.size;
				for (int dy = -1; dy <= 1; dy++)
					for (int dx = -1; dx <= 1; dx++) {
						int nx = x + dx, ny = y + dy;
						if (nx < 0 || ny < 0 || nx >= opt.size || ny >= opt.size)
							continue;
						int next = ny * opt.size + nx;
						if (out->mask[next] || distance[next])
							continue;
						memcpy(out->rgba + next * 4, out->rgba + at * 4, 4);
						distance[next] = distance[at] + 1;
						queue[tail++]  = next;
					}
			}
		}
		free(queue);
		free(distance);
	}
	if (!ok || !out->covered) {
		bn_result_free(out);
		return false;
	}
	if (progress && !progress(user, 100)) {
		bn_result_free(out);
		return false;
	}
	return true;
}

// Single-map callers and batch jobs share the same rasterizer. Batch jobs retain
// one acceleration structure across maps instead of rebuilding the high-poly BVH.
static inline bool bn_bake(const bn_low *low, int low_count, const bn_face *high, int high_count, bn_options opt, bn_result *out, bn_progress progress,
                           void *user) {
	memset(out, 0, sizeof(*out));
	bn_tree tree;
	bool    ok = bn_tree_init(&tree, high, high_count, progress, user);
	if (ok)
		ok = bn_bake_tree(low, low_count, &tree, opt, out, progress, user);
	free(tree.order);
	free(tree.nodes);
	return ok;
}
