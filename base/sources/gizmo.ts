
let gizmo_v: vec4_t = vec4_create();
let gizmo_v0: vec4_t = vec4_create();
let gizmo_q: quat_t = quat_create();
let gizmo_q0: quat_t = quat_create();

function gizmo_update() {
	let is_object: bool = context_raw.tool == workspace_tool_t.GIZMO;
	let is_decal: bool = base_is_decal_layer();

	let gizmo: object_t = context_raw.gizmo;
	let hide: bool = operator_shortcut(map_get(config_keymap, "stencil_hide"), shortcut_type_t.DOWN);
	gizmo.visible = (is_object || is_decal) && !hide;
	if (!gizmo.visible) {
		return;
	}

	let paint_object: object_t = context_raw.paint_object.base;
	///if is_forge
	if (context_raw.selected_object != null) {
		paint_object = context_raw.selected_object;
	}
	///end

	if (is_object) {
		gizmo.transform.loc = vec4_clone(paint_object.transform.loc);
	}
	else if (is_decal) {
		gizmo.transform.loc = vec4_create(context_raw.layer.decal_mat.m30, context_raw.layer.decal_mat.m31, context_raw.layer.decal_mat.m32);
	}
	let cam: camera_object_t = scene_camera;
	let fov: f32 = cam.data.fov;
	let dist: f32 = vec4_dist(cam.base.transform.loc, gizmo.transform.loc) / 8 * fov;
	gizmo.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_translate_x.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_translate_y.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_translate_z.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_scale_x.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_scale_y.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_scale_z.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_rotate_x.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_rotate_y.transform.scale = vec4_create(dist, dist, dist);
	context_raw.gizmo_rotate_z.transform.scale = vec4_create(dist, dist, dist);
	transform_build_matrix(gizmo.transform);

	// Scene control
	if (is_object) {
		if (context_raw.translate_x || context_raw.translate_y || context_raw.translate_z || context_raw.scale_x || context_raw.scale_y || context_raw.scale_z || context_raw.rotate_x || context_raw.rotate_y || context_raw.rotate_z) {
			if (context_raw.translate_x) {
				paint_object.transform.loc.x = context_raw.gizmo_drag;
			}
			else if (context_raw.translate_y) {
				paint_object.transform.loc.y = context_raw.gizmo_drag;
			}
			else if (context_raw.translate_z) {
				paint_object.transform.loc.z = context_raw.gizmo_drag;
			}
			else if (context_raw.scale_x) {
				paint_object.transform.scale.x += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
			}
			else if (context_raw.scale_y) {
				paint_object.transform.scale.y += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
			}
			else if (context_raw.scale_z) {
				paint_object.transform.scale.z += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
			}
			else if (context_raw.rotate_x) {
				gizmo_q0 = quat_from_axis_angle(vec4_x_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				paint_object.transform.rot = quat_mult(paint_object.transform.rot, gizmo_q0);
			}
			else if (context_raw.rotate_y) {
				gizmo_q0 = quat_from_axis_angle(vec4_y_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				paint_object.transform.rot = quat_mult(paint_object.transform.rot, gizmo_q0);
			}
			else if (context_raw.rotate_z) {
				gizmo_q0 = quat_from_axis_angle(vec4_z_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				paint_object.transform.rot = quat_mult(paint_object.transform.rot, gizmo_q0);
			}
			context_raw.gizmo_drag_last = context_raw.gizmo_drag;

			transform_build_matrix(paint_object.transform);
			///if arm_physics
			let pb: physics_body_t = map_get(physics_body_object_map, paint_object.uid);
			if (pb != null) {
				physics_body_sync_transform(pb);
			}
			///end
		}
	}
	// Decal layer control
	else if (is_decal) {
		if (context_raw.translate_x || context_raw.translate_y || context_raw.translate_z || context_raw.scale_x || context_raw.scale_y || context_raw.scale_z || context_raw.rotate_x || context_raw.rotate_y || context_raw.rotate_z) {
			if (context_raw.translate_x) {
				context_raw.layer.decal_mat.m30 = context_raw.gizmo_drag;
			}
			else if (context_raw.translate_y) {
				context_raw.layer.decal_mat.m31 = context_raw.gizmo_drag;
			}
			else if (context_raw.translate_z) {
				context_raw.layer.decal_mat.m32 = context_raw.gizmo_drag;
			}
			else if (context_raw.scale_x) {
				let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
				gizmo_v = dec.loc;
				gizmo_q = dec.rot;
				gizmo_v0 = dec.scl;
				gizmo_v0.x += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
				context_raw.layer.decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.scale_y) {
				let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
				gizmo_v = dec.loc;
				gizmo_q = dec.rot;
				gizmo_v0 = dec.scl;
				gizmo_v0.y += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
				context_raw.layer.decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.scale_z) {
				let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
				gizmo_v = dec.loc;
				gizmo_q = dec.rot;
				gizmo_v0 = dec.scl;
				gizmo_v0.z += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
				context_raw.layer.decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.rotate_x) {
				let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
				gizmo_v = dec.loc;
				gizmo_q = dec.rot;
				gizmo_v0 = dec.scl;
				gizmo_q0 = quat_from_axis_angle(vec4_x_axis(), -context_raw.gizmo_drag + context_raw.gizmo_drag_last);
				gizmo_q = quat_mult(gizmo_q0, gizmo_q);
				context_raw.layer.decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.rotate_y) {
				let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
				gizmo_v = dec.loc;
				gizmo_q = dec.rot;
				gizmo_v0 = dec.scl;
				gizmo_q0 = quat_from_axis_angle(vec4_y_axis(), -context_raw.gizmo_drag + context_raw.gizmo_drag_last);
				gizmo_q = quat_mult(gizmo_q0, gizmo_q);
				context_raw.layer.decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.rotate_z) {
				let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
				gizmo_v = dec.loc;
				gizmo_q = dec.rot;
				gizmo_v0 = dec.scl;
				gizmo_q0 = quat_from_axis_angle(vec4_z_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				gizmo_q = quat_mult(gizmo_q0, gizmo_q);
				context_raw.layer.decal_mat = mat4_compose(gizmo_v, gizmo_q, gizmo_v0);
			}
			context_raw.gizmo_drag_last = context_raw.gizmo_drag;

			if (context_raw.material != context_raw.layer.fill_layer) {
				context_set_material(context_raw.layer.fill_layer);
			}
			layers_update_fill_layer(context_raw.gizmo_started);
		}
	}

	context_raw.gizmo_started = false;
	if (mouse_started("left") && paint_object.name != "Scene") {
		// Translate, scale
		let trs: transform_t[] = [
			context_raw.gizmo_translate_x.transform,
			context_raw.gizmo_translate_y.transform,
			context_raw.gizmo_translate_z.transform,
			context_raw.gizmo_scale_x.transform,
			context_raw.gizmo_scale_y.transform,
			context_raw.gizmo_scale_z.transform
		];
		let hit: transform_t = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
		if (hit != null) {
			if (hit.object == context_raw.gizmo_translate_x) {
				context_raw.translate_x = true;
			}
			else if (hit.object == context_raw.gizmo_translate_y) {
				context_raw.translate_y = true;
			}
			else if (hit.object == context_raw.gizmo_translate_z) {
				context_raw.translate_z = true;
			}
			else if (hit.object == context_raw.gizmo_scale_x) {
				context_raw.scale_x = true;
			}
			else if (hit.object == context_raw.gizmo_scale_y) {
				context_raw.scale_y = true;
			}
			else if (hit.object == context_raw.gizmo_scale_z) {
				context_raw.scale_z = true;
			}
			if (context_raw.translate_x || context_raw.translate_y || context_raw.translate_z || context_raw.scale_x || context_raw.scale_y || context_raw.scale_z) {
				context_raw.gizmo_offset = 0.0;
				context_raw.gizmo_started = true;
			}
		}
		else {
			// Rotate
			let trs: transform_t[] = [
				context_raw.gizmo_rotate_x.transform,
				context_raw.gizmo_rotate_y.transform,
				context_raw.gizmo_rotate_z.transform
			];
			let hit: transform_t = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (hit.object == context_raw.gizmo_rotate_x) {
					context_raw.rotate_x = true;
				}
				else if (hit.object == context_raw.gizmo_rotate_y) {
					context_raw.rotate_y = true;
				}
				else if (hit.object == context_raw.gizmo_rotate_z) {
					context_raw.rotate_z = true;
				}
				if (context_raw.rotate_x || context_raw.rotate_y || context_raw.rotate_z) {
					context_raw.gizmo_offset = 0.0;
					context_raw.gizmo_started = true;
				}
			}
		}
	}
	else if (mouse_released("left")) {
		context_raw.translate_x = context_raw.translate_y = context_raw.translate_z = false;
		context_raw.scale_x = context_raw.scale_y = context_raw.scale_z = false;
		context_raw.rotate_x = context_raw.rotate_y = context_raw.rotate_z = false;
	}

	if (context_raw.translate_x || context_raw.translate_y || context_raw.translate_z || context_raw.scale_x || context_raw.scale_y || context_raw.scale_z || context_raw.rotate_x || context_raw.rotate_y || context_raw.rotate_z) {
		context_raw.rdirty = 2;

		if (is_object) {
			let t: transform_t = paint_object.transform;
			gizmo_v = vec4_create(transform_world_x(t), transform_world_y(t), transform_world_z(t));
		}
		else if (is_decal) {
			gizmo_v = vec4_create(context_raw.layer.decal_mat.m30, context_raw.layer.decal_mat.m31, context_raw.layer.decal_mat.m32);
		}

		if (context_raw.translate_x || context_raw.scale_x) {
			let hit: vec4_t = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw.gizmo_started) {
					context_raw.gizmo_offset = hit.x - gizmo_v.x;
				}
				context_raw.gizmo_drag = hit.x - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.translate_y || context_raw.scale_y) {
			let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw.gizmo_started) {
					context_raw.gizmo_offset = hit.y - gizmo_v.y;
				}
				context_raw.gizmo_drag = hit.y - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.translate_z || context_raw.scale_z) {
			let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw.gizmo_started) {
					context_raw.gizmo_offset = hit.z - gizmo_v.z;
				}
				context_raw.gizmo_drag = hit.z - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.rotate_x) {
			let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw.gizmo_started) {
					let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
					gizmo_v = dec.loc;
					gizmo_q = dec.rot;
					gizmo_v0 = dec.scl;
					context_raw.gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z);
				}
				context_raw.gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z) - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.rotate_y) {
			let hit: vec4_t = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw.gizmo_started) {
					let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
					gizmo_v = dec.loc;
					gizmo_q = dec.rot;
					gizmo_v0 = dec.scl;
					context_raw.gizmo_offset = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x);
				}
				context_raw.gizmo_drag = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x) - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.rotate_z) {
			let hit: vec4_t = raycast_plane_intersect(vec4_z_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (!vec4_isnan(hit)) {
				if (context_raw.gizmo_started) {
					let dec: mat4_decomposed_t = mat4_decompose(context_raw.layer.decal_mat);
					gizmo_v = dec.loc;
					gizmo_q = dec.rot;
					gizmo_v0 = dec.scl;
					context_raw.gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x);
				}
				context_raw.gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x) - context_raw.gizmo_offset;
			}
		}

		if (context_raw.gizmo_started) {
			context_raw.gizmo_drag_last = context_raw.gizmo_drag;
		}

		///if is_forge
		util_mesh_remove_merged();
		render_path_raytrace_ready = false;
		///end
	}

	_input_occupied = (context_raw.translate_x || context_raw.translate_y || context_raw.translate_z || context_raw.scale_x || context_raw.scale_y || context_raw.scale_z || context_raw.rotate_x || context_raw.rotate_y || context_raw.rotate_z) && mouse_view_x() < base_w();
}
