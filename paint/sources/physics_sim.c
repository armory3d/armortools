
#include "global.h"

void sim_init() {
	if (sim_initialized) {
		return;
	}
	physics_world_create();
	sim_initialized = true;
}

void sim_update() {

	render_path_raytrace_ready = false;

	if (sim_running) {
		// if (render_path_raytrace_frame != 1) {
		// return;
		// }

		object_t_array_t *objects = map_keys(sim_object_script_map);
		for (i32 i = 0; i < objects->length; ++i) {
			object_t *o    = objects->buffer[i];
			char     *s    = any_map_get(sim_object_script_map, o);
			char     *addr = i64_to_string((i64)(o->transform));
			s              = string("{let transform=%s;%s}", addr, s);
			// minic_eval(s);
		}

		physics_world_t *world = physics_world_active;
		physics_world_update(world);

		iron_delay_idle_sleep();

		if (sim_record) {
			render_target_t *rt     = any_map_get(render_path_render_targets, "last");
			buffer_t        *pixels = gpu_get_texture_pixels(rt->_image);
#ifdef IRON_BGRA
			export_arm_bgra_swap(pixels);
#endif
			// iron_mp4_encode(pixels);
		}
	}
}

void sim_play() {
	sim_running = true;

	if (sim_record) {
		if (string_equals(project_filepath, "")) {
			console_error(tr("Save project first"));
			sim_record = false;
			return;
		}
		char            *path = string("%s/output.mp4", path_base_dir(project_filepath));
		render_target_t *rt   = any_map_get(render_path_render_targets, "last");
		// iron_mp4_begin(path, rt._image.width, rt._image.height);
	}

	// Save transforms
	gc_unroot(sim_transforms);
	sim_transforms = any_array_create_from_raw((void *[]){}, 0);
	gc_root(sim_transforms);
	mesh_object_t_array_t *pos = project_paint_objects;
	for (i32 i = 0; i < pos->length; ++i) {
		mat4_box_t *m = GC_ALLOC_INIT(mat4_box_t, {.v = pos->buffer[i]->base->transform->local});
		any_array_push(sim_transforms, m);
	}
}

void sim_stop() {
	sim_running = false;

	if (sim_record) {
		// iron_mp4_end();
	}

	// Restore transforms
	mesh_object_t_array_t *pos = project_paint_objects;
	for (i32 i = 0; i < pos->length; ++i) {
		transform_set_matrix(pos->buffer[i]->base->transform, sim_transforms->buffer[i]->v);
		physics_body_t *pb = any_imap_get(physics_body_object_map, pos->buffer[i]->base->uid);
		if (pb != NULL) {
			physics_body_sync_transform(pb);
		}
	}
}

void sim_add_body(object_t *o, physics_shape_t shape, f32 mass) {
	physics_body_t *body = physics_body_create();
	body->shape          = shape;
	body->mass           = mass;
	physics_body_init(body, o);
}

void sim_remove_body(i32 uid) {
	physics_body_remove(uid);
}

void sim_duplicate() {
	// Mesh
	mesh_object_t *so  = context_raw->selected_object->ext;
	mesh_object_t *dup = scene_add_mesh_object(so->data, so->material, so->base->parent);
	transform_set_matrix(dup->base->transform, so->base->transform->local);
	any_array_push(project_paint_objects, dup);
	dup->base->name = so->base->name;

	// Physics
	physics_body_t *pb = any_imap_get(physics_body_object_map, so->base->uid);
	if (pb != NULL) {
		physics_body_t *pbdup = physics_body_create();
		pbdup->shape          = pb->shape;
		pbdup->mass           = pb->mass;
		physics_body_init(pbdup, dup->base);
	}

	_tab_scene_paint_object_length++;
	tab_scene_sort();
}

void sim_delete() {
	mesh_object_t *so = context_raw->selected_object->ext;
	array_remove(project_paint_objects, so);
	mesh_object_remove(so);
	sim_remove_body(so->base->uid);
	_tab_scene_paint_object_length--;
	tab_scene_sort();
}
