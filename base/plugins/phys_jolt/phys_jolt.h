
#pragma once

#include "iron_vec4.h"
#include "iron_quat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct physics_pair {
    float pos_a_x;
    float pos_a_y;
    float pos_a_z;
} physics_pair_t;

void jolt_world_create();
void jolt_world_update();
physics_pair_t *jolt_world_get_contact_pairs();
void jolt_world_destroy();

void* jolt_body_create(int shape, float mass, float dimx, float dimy, float dimz, float x, float y, float z, void *f32a_triangles);
void jolt_body_apply_impulse(void *b, float x, float y, float z);
void jolt_body_get_pos(void *b, void *p);
void jolt_body_get_rot(void *b, void *r);
void jolt_body_sync_transform(void *b, vec4_t p, quat_t r);
void jolt_body_remove(void *b);

#ifdef __cplusplus
}
#endif
