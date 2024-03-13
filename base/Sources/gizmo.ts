
///if (is_paint || is_sculpt)

let gizmo_v: vec4_t = vec4_create();
let gizmo_v0: vec4_t = vec4_create();
let gizmo_q: quat_t = quat_create();
let gizmo_q0: quat_t = quat_create();

function gizmo_update() {
	let is_object: bool = context_raw.tool == workspace_tool_t.GIZMO;
	let is_decal: bool = base_is_decal_layer();

	let gizmo: object_t = context_raw.gizmo;
	let hide: bool = operator_shortcut(config_keymap.stencil_hide, shortcut_type_t.DOWN);
	gizmo.visible = (is_object || is_decal) && !hide;
	if (!gizmo.visible) return;

	let paint_object: object_t = context_raw.paint_object.base;
	///if is_forge
	if (context_raw.selected_object != null) {
		paint_object = context_raw.selected_object;
	}
	///end

	if (is_object) {
		vec4_set_from(gizmo.transform.loc, paint_object.transform.loc);
	}
	else if (is_decal) {
		vec4_set(gizmo.transform.loc, context_raw.layer.decal_mat.m[12], context_raw.layer.decal_mat.m[13], context_raw.layer.decal_mat.m[14]);
	}
	let cam: camera_object_t = scene_camera;
	let fov: f32 = cam.data.fov;
	let dist: f32 = vec4_dist(cam.base.transform.loc, gizmo.transform.loc) / 8 * fov;
	vec4_set(gizmo.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_translate_x.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_translate_y.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_translate_z.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_scale_x.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_scale_y.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_scale_z.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_rotate_x.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_rotate_y.transform.scale, dist, dist, dist);
	vec4_set(context_raw.gizmo_rotate_z.transform.scale, dist, dist, dist);
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
				quat_from_axis_angle(gizmo_q0, vec4_x_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				quat_mult(paint_object.transform.rot, gizmo_q0);
			}
			else if (context_raw.rotate_y) {
				quat_from_axis_angle(gizmo_q0, vec4_y_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				quat_mult(paint_object.transform.rot, gizmo_q0);
			}
			else if (context_raw.rotate_z) {
				quat_from_axis_angle(gizmo_q0, vec4_z_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				quat_mult(paint_object.transform.rot, gizmo_q0);
			}
			context_raw.gizmo_drag_last = context_raw.gizmo_drag;

			transform_build_matrix(paint_object.transform);
			///if arm_physics
			let pb: any = (paint_object as any).physicsBody;
			if (pb != null) pb.syncTransform();
			///end
		}
	}
	// Decal layer control
	else if (is_decal) {
		if (context_raw.translate_x || context_raw.translate_y || context_raw.translate_z || context_raw.scale_x || context_raw.scale_y || context_raw.scale_z || context_raw.rotate_x || context_raw.rotate_y || context_raw.rotate_z) {
			if (context_raw.translate_x) {
				context_raw.layer.decal_mat.m[12] = context_raw.gizmo_drag;
			}
			else if (context_raw.translate_y) {
				context_raw.layer.decal_mat.m[13] = context_raw.gizmo_drag;
			}
			else if (context_raw.translate_z) {
				context_raw.layer.decal_mat.m[14] = context_raw.gizmo_drag;
			}
			else if (context_raw.scale_x) {
				mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
				gizmo_v0.x += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
				mat4_compose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.scale_y) {
				mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
				gizmo_v0.y += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
				mat4_compose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.scale_z) {
				mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
				gizmo_v0.z += context_raw.gizmo_drag - context_raw.gizmo_drag_last;
				mat4_compose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.rotate_x) {
				mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
				quat_from_axis_angle(gizmo_q0, vec4_x_axis(), -context_raw.gizmo_drag + context_raw.gizmo_drag_last);
				quat_mult_quats(gizmo_q, gizmo_q0, gizmo_q);
				mat4_compose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.rotate_y) {
				mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
				quat_from_axis_angle(gizmo_q0, vec4_y_axis(), -context_raw.gizmo_drag + context_raw.gizmo_drag_last);
				quat_mult_quats(gizmo_q, gizmo_q0, gizmo_q);
				mat4_compose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
			}
			else if (context_raw.rotate_z) {
				mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
				quat_from_axis_angle(gizmo_q0, vec4_z_axis(), context_raw.gizmo_drag - context_raw.gizmo_drag_last);
				quat_mult_quats(gizmo_q, gizmo_q0, gizmo_q);
				mat4_compose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
			}
			context_raw.gizmo_drag_last = context_raw.gizmo_drag;

			if (context_raw.material != context_raw.layer.fill_layer) {
				context_set_material(context_raw.layer.fill_layer);
			}
			base_update_fill_layer(context_raw.gizmo_started);
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
			if (hit.object == context_raw.gizmo_translate_x) context_raw.translate_x = true;
			else if (hit.object == context_raw.gizmo_translate_y) context_raw.translate_y = true;
			else if (hit.object == context_raw.gizmo_translate_z) context_raw.translate_z = true;
			else if (hit.object == context_raw.gizmo_scale_x) context_raw.scale_x = true;
			else if (hit.object == context_raw.gizmo_scale_y) context_raw.scale_y = true;
			else if (hit.object == context_raw.gizmo_scale_z) context_raw.scale_z = true;
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
				if (hit.object == context_raw.gizmo_rotate_x) context_raw.rotate_x = true;
				else if (hit.object == context_raw.gizmo_rotate_y) context_raw.rotate_y = true;
				else if (hit.object == context_raw.gizmo_rotate_z) context_raw.rotate_z = true;
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
			vec4_set(gizmo_v, transform_world_x(t), transform_world_y(t), transform_world_z(t));
		}
		else if (is_decal) {
			vec4_set(gizmo_v, context_raw.layer.decal_mat.m[12], context_raw.layer.decal_mat.m[13], context_raw.layer.decal_mat.m[14]);
		}

		if (context_raw.translate_x || context_raw.scale_x) {
			let hit: vec4_t = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (context_raw.gizmo_started) context_raw.gizmo_offset = hit.x - gizmo_v.x;
				context_raw.gizmo_drag = hit.x - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.translate_y || context_raw.scale_y) {
			let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (context_raw.gizmo_started) context_raw.gizmo_offset = hit.y - gizmo_v.y;
				context_raw.gizmo_drag = hit.y - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.translate_z || context_raw.scale_z) {
			let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (context_raw.gizmo_started) context_raw.gizmo_offset = hit.z - gizmo_v.z;
				context_raw.gizmo_drag = hit.z - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.rotate_x) {
			let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (context_raw.gizmo_started) {
					mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
					context_raw.gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z);
				}
				context_raw.gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.z - gizmo_v.z) - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.rotate_y) {
			let hit: vec4_t = raycast_plane_intersect(vec4_y_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (context_raw.gizmo_started) {
					mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
					context_raw.gizmo_offset = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x);
				}
				context_raw.gizmo_drag = math_atan2(hit.z - gizmo_v.z, hit.x - gizmo_v.x) - context_raw.gizmo_offset;
			}
		}
		else if (context_raw.rotate_z) {
			let hit: vec4_t = raycast_plane_intersect(vec4_z_axis(), gizmo_v, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (context_raw.gizmo_started) {
					mat4_decompose(context_raw.layer.decal_mat, gizmo_v, gizmo_q, gizmo_v0);
					context_raw.gizmo_offset = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x);
				}
				context_raw.gizmo_drag = math_atan2(hit.y - gizmo_v.y, hit.x - gizmo_v.x) - context_raw.gizmo_offset;
			}
		}

		if (context_raw.gizmo_started) context_raw.gizmo_drag_last = context_raw.gizmo_drag;

		///if is_forge
		util_mesh_remove_merged();
		render_path_raytrace_ready = false;
		///end
	}

	_input_occupied = (context_raw.translate_x || context_raw.translate_y || context_raw.translate_z || context_raw.scale_x || context_raw.scale_y || context_raw.scale_z || context_raw.rotate_x || context_raw.rotate_y || context_raw.rotate_z) && mouse_view_x() < base_w();
}

///end
