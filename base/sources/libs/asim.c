#include "asim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GRAVITY -9.81f
#define MAX_BVH_DEPTH 20

typedef struct {
	vec4_t min;
	vec4_t max;
} aabb_t;

typedef struct {
	vec4_t position;
	vec4_t velocity;
	float radius;
	float mass;
} sphere_t;

typedef struct {
	vec4_t v0;
	vec4_t v1;
	vec4_t v2;
	vec4_t normal;
	aabb_t bounds;
} triangle_t;

typedef struct bvh_node {
	aabb_t bounds;
	struct bvh_node *left, *right;
	triangle_t *triangles;
	int num_tris;
	int is_leaf;
} bvh_node_t;

typedef struct {
	bvh_node_t *root;
} mesh_t;

sphere_t sphere = {
	.position = {0, 0, 0},
	.velocity = {0, 0, 0},
	// .radius = 1.0f,
	.radius = 0.2f,
	.mass = 1.0f
};

mesh_t mesh;

aabb_t root_bounds = {
	{-10, -10, -10},
	{10, 10, 10}
};

physics_pair_t ppair;

static inline aabb_t merge_aabbs(aabb_t a, aabb_t b) {
	return (aabb_t){
		.min = {fminf(a.min.x, b.min.x), fminf(a.min.y, b.min.y), fminf(a.min.z, b.min.z)},
		.max = {fmaxf(a.max.x, b.max.x), fmaxf(a.max.y, b.max.y), fmaxf(a.max.z, b.max.z)}
	};
}

static inline int sphere_aabb_intersect(sphere_t *s, aabb_t *a) {
	float x = fmaxf(a->min.x, fminf(s->position.x, a->max.x));
	float y = fmaxf(a->min.y, fminf(s->position.y, a->max.y));
	float z = fmaxf(a->min.z, fminf(s->position.z, a->max.z));
	float dx = x - s->position.x, dy = y - s->position.y, dz = z - s->position.z;
	return dx * dx + dy * dy + dz * dz <= s->radius * s->radius;
}

// Comparison function for sorting triangles by x-coordinate of centroid
static int compare_triangles(const void *a, const void *b) {
	triangle_t *ta = (triangle_t *)a;
	triangle_t *tb = (triangle_t *)b;
	vec4_t ca = vec4_mult(vec4_add(vec4_add(ta->v0, ta->v1), ta->v2), 1.0f / 3.0f);
	vec4_t cb = vec4_mult(vec4_add(vec4_add(tb->v0, tb->v1), tb->v2), 1.0f / 3.0f);
	return (ca.x > cb.x) - (ca.x < cb.x);
}

static bvh_node_t *create_bvh_node(triangle_t *tris, int num_tris, int depth) {
	bvh_node_t *node = (bvh_node_t *)malloc(sizeof(bvh_node_t));

	*node = (bvh_node_t){
		.left = NULL,
		.right = NULL,
		.triangles = NULL,
		.num_tris = 0,
		.is_leaf = 1
	};

	if (num_tris <= 1 || depth >= MAX_BVH_DEPTH) {
		node->num_tris = num_tris;
		if (num_tris) {
			node->triangles = (triangle_t *)malloc(num_tris * sizeof(triangle_t));
			memcpy(node->triangles, tris, num_tris * sizeof(triangle_t));
			node->bounds = tris[0].bounds;
		}
		else {
			node->bounds = root_bounds;
		}
		return node;
	}

	qsort(tris, num_tris, sizeof(triangle_t), compare_triangles);

	int mid = num_tris / 2;
	node->is_leaf = 0;
	node->left = create_bvh_node(tris, mid, depth + 1);
	node->right = create_bvh_node(tris + mid, num_tris - mid, depth + 1);
	node->bounds = merge_aabbs(node->left->bounds, node->right->bounds);
	return node;
}

static void collide_sphere_triangle(sphere_t *s, triangle_t *t) {
	if (!sphere_aabb_intersect(s, &t->bounds)) {
		return;
	}

	vec4_t to_sphere = vec4_sub(s->position, t->v0);
	float dist = vec4_dot(to_sphere, t->normal);
	if (fabsf(dist) > s->radius) {
		return;
	}

	vec4_t p = vec4_sub(s->position, vec4_mult(t->normal, dist));
	vec4_t e0 = vec4_sub(t->v1, t->v0), e1 = vec4_sub(t->v2, t->v1), e2 = vec4_sub(t->v0, t->v2);
	vec4_t c0 = vec4_sub(p, t->v0), c1 = vec4_sub(p, t->v1), c2 = vec4_sub(p, t->v2);

	if (vec4_dot(t->normal, vec4_cross(e0, c0)) >= 0 &&
		vec4_dot(t->normal, vec4_cross(e1, c1)) >= 0 &&
		vec4_dot(t->normal, vec4_cross(e2, c2)) >= 0) {

		float penetration = s->radius - dist;
		if (dist < 0) {
			penetration = -penetration;
		}
		s->position = vec4_add(s->position, vec4_mult(t->normal, penetration));
		float v_dot_n = vec4_dot(s->velocity, t->normal);
		vec4_t n_vel = vec4_mult(t->normal, v_dot_n);
		s->velocity = vec4_add(vec4_sub(s->velocity, n_vel), vec4_mult(n_vel, -0.01f));

		vec4_t contact_point = vec4_sub(s->position, vec4_mult(t->normal, s->radius));
        ppair.pos_a_x = contact_point.x;
        ppair.pos_a_y = contact_point.y;
        ppair.pos_a_z = contact_point.z;
	}
}

static void query_bvh(sphere_t *s, bvh_node_t *n) {
	if (!n || !sphere_aabb_intersect(s, &n->bounds)) {
		return;
	}

	if (n->is_leaf) {
		for (int i = 0; i < n->num_tris; i++) {
			collide_sphere_triangle(s, &n->triangles[i]);
		}
	}
	else {
		query_bvh(s, n->left);
		query_bvh(s, n->right);
	}
}

static void free_bvh(bvh_node_t *n) {
	if (!n) {
		return;
	}
	if (n->is_leaf) {
		free(n->triangles);
	}
	else {
		free_bvh(n->left);
		free_bvh(n->right);
	}
	free(n);
}

void asim_world_create() {

}

void asim_world_destroy() {
	free_bvh(mesh.root);
	mesh.root = NULL;
}

void asim_world_update(float time_step) {
	ppair.pos_a_x = 0;
	ppair.pos_a_y = 0;
	ppair.pos_a_z = 0;

	const int sub_steps = 2;
	float dt = time_step / sub_steps;
	for (int i = 0; i < sub_steps; i++) {
		sphere.velocity.z += GRAVITY * dt;
		sphere.position = vec4_add(sphere.position, vec4_mult(sphere.velocity, dt));
		query_bvh(&sphere, mesh.root);
	}
}

physics_pair_t *asim_world_get_contact() {
	return &ppair;
}

void *asim_body_create(int shape, float mass, float dimx, float dimy, float dimz, float x, float y, float z, void *posa, void *inda, float scale_pos) {

	if (shape == 1) { // SPHERE

		sphere.position.x = x;
		sphere.position.y = y;
		sphere.position.z = z;
		sphere.velocity.x = 0;
		sphere.velocity.y = 0;
		sphere.velocity.z = 0;

		return NULL;
	}

	i16_array_t *pa = posa;
	u32_array_t *ia = inda;
	int num_tris = ia->length / 3;
	triangle_t *tris = (triangle_t *)malloc(num_tris * sizeof(triangle_t));
	float scale = (1.0 / 32767.0) * scale_pos;

	for (int i = 0; i < num_tris; i++) {
		tris[i].v0 = (vec4_t){pa->buffer[ia->buffer[i * 3    ] * 4    ] * scale,
							  pa->buffer[ia->buffer[i * 3    ] * 4 + 1] * scale,
							  pa->buffer[ia->buffer[i * 3    ] * 4 + 2] * scale};
		tris[i].v1 = (vec4_t){pa->buffer[ia->buffer[i * 3 + 1] * 4    ] * scale,
							  pa->buffer[ia->buffer[i * 3 + 1] * 4 + 1] * scale,
							  pa->buffer[ia->buffer[i * 3 + 1] * 4 + 2] * scale};
		tris[i].v2 = (vec4_t){pa->buffer[ia->buffer[i * 3 + 2] * 4    ] * scale,
							  pa->buffer[ia->buffer[i * 3 + 2] * 4 + 1] * scale,
							  pa->buffer[ia->buffer[i * 3 + 2] * 4 + 2] * scale};

		vec4_t edge1 = vec4_sub(tris[i].v1, tris[i].v0);
		vec4_t edge2 = vec4_sub(tris[i].v2, tris[i].v0);
		tris[i].normal = vec4_mult(vec4_cross(edge1, edge2), 1.0f / vec4_len(vec4_cross(edge1, edge2)));

		tris[i].bounds = (aabb_t){
			.min = {fminf(fminf(tris[i].v0.x, tris[i].v1.x), tris[i].v2.x),
					fminf(fminf(tris[i].v0.y, tris[i].v1.y), tris[i].v2.y),
					fminf(fminf(tris[i].v0.z, tris[i].v1.z), tris[i].v2.z)},
			.max = {fmaxf(fmaxf(tris[i].v0.x, tris[i].v1.x), tris[i].v2.x),
					fmaxf(fmaxf(tris[i].v0.y, tris[i].v1.y), tris[i].v2.y),
					fmaxf(fmaxf(tris[i].v0.z, tris[i].v1.z), tris[i].v2.z)}
		};
	}

	mesh.root = create_bvh_node(tris, num_tris, 0);
	free(tris);

	return NULL;
}

void asim_body_apply_impulse(void *body, float x, float y, float z) {
	sphere.velocity.x += x;
	sphere.velocity.y += y;
	sphere.velocity.z += z;
}

void asim_body_get_pos(void *body, vec4_t *pos) {
	pos->x = sphere.position.x;
	pos->y = sphere.position.y;
	pos->z = sphere.position.z;
}

void asim_body_get_rot(void *body, quat_t *rot) {
}

void asim_body_sync_transform(void *body, vec4_t pos, quat_t rot) {
	sphere.position.x = pos.x;
	sphere.position.y = pos.y;
	sphere.position.z = pos.z;
}

void asim_body_remove(void *body) {
}
