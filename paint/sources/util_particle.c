
#include "global.h"

void util_particle_init_mesh() {
	if (g_context->paint_body != NULL) {
		return;
	}
	if (g_context->merged_object == NULL) {
		util_mesh_merge(NULL);
	}

	g_context->paint_body        = physics_body_create();
	g_context->paint_body->shape = PHYSICS_SHAPE_MESH;
	physics_body_init(g_context->paint_body, g_context->merged_object->base);
}

void util_particle_init_physics() {
	if (physics_world_active == NULL) {
		physics_world_create();
	}
	util_particle_init_mesh();
}
