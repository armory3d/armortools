
#include "global.h"

void util_particle_init() {
	if (g_context->particle_material != NULL) {
		return;
	}

	{
		render_target_t *t = render_target_create();
		t->name            = "texparticle";
		t->width           = 0;
		t->height          = 0;
		t->format          = "R8";
		t->scale           = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}

	for (i32 i = 0; i < _scene_raw->material_datas->length; ++i) {
		material_data_t *mat = _scene_raw->material_datas->buffer[i];
		if (string_equals(mat->name, "Material2")) {
			material_data_t *m = util_clone_material_data(mat);
			m->name            = "MaterialParticle";
			any_array_push(_scene_raw->material_datas, m);
			break;
		}
	}

	material_data_t *md            = data_get_material("Scene", "MaterialParticle");
	g_context->particle_material = md;

	for (i32 i = 0; i < _scene_raw->objects->length; ++i) {
		obj_t *obj = _scene_raw->objects->buffer[i];
		if (string_equals(obj->name, ".Sphere")) {
			obj_t *particle        = util_clone_obj(obj);
			particle->name         = ".Particle";
			particle->material_ref = "MaterialParticle";
			any_array_push(_scene_raw->objects, particle);
			for (i32 i = 0; i < 16; ++i) {
				particle->transform->buffer[i] *= 0.01;
			}
			break;
		}
	}

	object_t      *o  = scene_spawn_object(".Sphere", NULL, true);
	mesh_object_t *mo = o->ext;
	mo->base->name    = ".ParticleEmitter";
	mo->base->raw     = util_clone_obj(mo->base->raw);
}

void util_particle_init_mesh() {
	if (g_context->paint_body != NULL) {
		return;
	}

	if (g_context->merged_object == NULL) {
		util_mesh_merge(NULL);
	}

	mesh_object_t *po              = g_context->merged_object;
	po->base->transform->scale.x   = po->base->parent->transform->scale.x;
	po->base->transform->scale.y   = po->base->parent->transform->scale.y;
	po->base->transform->scale.z   = po->base->parent->transform->scale.z;
	g_context->paint_body        = physics_body_create();
	g_context->paint_body->shape = PHYSICS_SHAPE_MESH;
	physics_body_init(g_context->paint_body, po->base);
}

void util_particle_init_physics() {
	if (physics_world_active != NULL) {
		util_particle_init_mesh();
		return;
	}

	physics_world_create();
	util_particle_init_mesh();
}
