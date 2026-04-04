
#ifdef arm_physics

#include "global.h"
#include "../libs/asim.h"

physics_world_t *physics_world_create() {
	asim_world_create();
	physics_world_t *world = GC_ALLOC_INIT(physics_world_t, {0});
	gc_unroot(physics_world_active);
	physics_world_active = world;
	gc_root(physics_world_active);
	return world;
}

void physics_world_update(physics_world_t *world) {
	asim_world_update(sys_delta());

	i32_array_t *keys = imap_keys(physics_body_object_map);
	for (i32 i = 0; i < keys->length; ++i) {
		physics_body_t *body = any_imap_get(physics_body_object_map, keys->buffer[i]);
		physics_body_update(body);
	}
}

physics_pair_t_array_t *physics_world_get_contact_pairs(physics_world_t *world, physics_body_t *body) {
	physics_pair_t_array_t *pairs = any_array_create_from_raw((void *[]){}, 0);
	physics_pair_t         *p     = asim_world_get_contact();
	any_array_push(pairs, p);
	return pairs;
}

#endif
