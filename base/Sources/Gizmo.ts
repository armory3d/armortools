
///if (is_paint || is_sculpt)

class Gizmo {

	static v: vec4_t = vec4_create();
	static v0: vec4_t = vec4_create();
	static q: quat_t = quat_create();
	static q0: quat_t = quat_create();

	static update = () => {
		let is_object: bool = Context.raw.tool == workspace_tool_t.GIZMO;
		let is_decal: bool = base_is_decal_layer();

		let gizmo: object_t = Context.raw.gizmo;
		let hide: bool = Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown);
		gizmo.visible = (is_object || is_decal) && !hide;
		if (!gizmo.visible) return;

		let paint_object: object_t = Context.raw.paint_object.base;
		///if is_forge
		if (Context.raw.selected_object != null) {
			paint_object = Context.raw.selected_object;
		}
		///end

		if (is_object) {
			vec4_set_from(gizmo.transform.loc, paint_object.transform.loc);
		}
		else if (is_decal) {
			vec4_set(gizmo.transform.loc, Context.raw.layer.decal_mat.m[12], Context.raw.layer.decal_mat.m[13], Context.raw.layer.decal_mat.m[14]);
		}
		let cam: camera_object_t = scene_camera;
		let fov: f32 = cam.data.fov;
		let dist: f32 = vec4_dist(cam.base.transform.loc, gizmo.transform.loc) / 8 * fov;
		vec4_set(gizmo.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_translate_x.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_translate_y.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_translate_z.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_scale_x.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_scale_y.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_scale_z.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_rotate_x.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_rotate_y.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmo_rotate_z.transform.scale, dist, dist, dist);
		transform_build_matrix(gizmo.transform);

		// Scene control
		if (is_object) {
			if (Context.raw.translate_x || Context.raw.translate_y || Context.raw.translate_z || Context.raw.scale_x || Context.raw.scale_y || Context.raw.scale_z || Context.raw.rotate_x || Context.raw.rotate_y || Context.raw.rotate_z) {
				if (Context.raw.translate_x) {
					paint_object.transform.loc.x = Context.raw.gizmo_drag;
				}
				else if (Context.raw.translate_y) {
					paint_object.transform.loc.y = Context.raw.gizmo_drag;
				}
				else if (Context.raw.translate_z) {
					paint_object.transform.loc.z = Context.raw.gizmo_drag;
				}
				else if (Context.raw.scale_x) {
					paint_object.transform.scale.x += Context.raw.gizmo_drag - Context.raw.gizmo_drag_last;
				}
				else if (Context.raw.scale_y) {
					paint_object.transform.scale.y += Context.raw.gizmo_drag - Context.raw.gizmo_drag_last;
				}
				else if (Context.raw.scale_z) {
					paint_object.transform.scale.z += Context.raw.gizmo_drag - Context.raw.gizmo_drag_last;
				}
				else if (Context.raw.rotate_x) {
					quat_from_axis_angle(Gizmo.q0, vec4_x_axis(), Context.raw.gizmo_drag - Context.raw.gizmo_drag_last);
					quat_mult(paint_object.transform.rot, Gizmo.q0);
				}
				else if (Context.raw.rotate_y) {
					quat_from_axis_angle(Gizmo.q0, vec4_y_axis(), Context.raw.gizmo_drag - Context.raw.gizmo_drag_last);
					quat_mult(paint_object.transform.rot, Gizmo.q0);
				}
				else if (Context.raw.rotate_z) {
					quat_from_axis_angle(Gizmo.q0, vec4_z_axis(), Context.raw.gizmo_drag - Context.raw.gizmo_drag_last);
					quat_mult(paint_object.transform.rot, Gizmo.q0);
				}
				Context.raw.gizmo_drag_last = Context.raw.gizmo_drag;

				transform_build_matrix(paint_object.transform);
				///if arm_physics
				let pb: any = (paint_object as any).physicsBody;
				if (pb != null) pb.syncTransform();
				///end
			}
		}
		// Decal layer control
		else if (is_decal) {
			if (Context.raw.translate_x || Context.raw.translate_y || Context.raw.translate_z || Context.raw.scale_x || Context.raw.scale_y || Context.raw.scale_z || Context.raw.rotate_x || Context.raw.rotate_y || Context.raw.rotate_z) {
				if (Context.raw.translate_x) {
					Context.raw.layer.decal_mat.m[12] = Context.raw.gizmo_drag;
				}
				else if (Context.raw.translate_y) {
					Context.raw.layer.decal_mat.m[13] = Context.raw.gizmo_drag;
				}
				else if (Context.raw.translate_z) {
					Context.raw.layer.decal_mat.m[14] = Context.raw.gizmo_drag;
				}
				else if (Context.raw.scale_x) {
					mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.x += Context.raw.gizmo_drag - Context.raw.gizmo_drag_last;
					mat4_compose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scale_y) {
					mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.y += Context.raw.gizmo_drag - Context.raw.gizmo_drag_last;
					mat4_compose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scale_z) {
					mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.z += Context.raw.gizmo_drag - Context.raw.gizmo_drag_last;
					mat4_compose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotate_x) {
					mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
					quat_from_axis_angle(Gizmo.q0, vec4_x_axis(), -Context.raw.gizmo_drag + Context.raw.gizmo_drag_last);
					quat_mult_quats(Gizmo.q, Gizmo.q0, Gizmo.q);
					mat4_compose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotate_y) {
					mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
					quat_from_axis_angle(Gizmo.q0, vec4_y_axis(), -Context.raw.gizmo_drag + Context.raw.gizmo_drag_last);
					quat_mult_quats(Gizmo.q, Gizmo.q0, Gizmo.q);
					mat4_compose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotate_z) {
					mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
					quat_from_axis_angle(Gizmo.q0, vec4_z_axis(), Context.raw.gizmo_drag - Context.raw.gizmo_drag_last);
					quat_mult_quats(Gizmo.q, Gizmo.q0, Gizmo.q);
					mat4_compose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				Context.raw.gizmo_drag_last = Context.raw.gizmo_drag;

				if (Context.raw.material != Context.raw.layer.fill_layer) {
					Context.set_material(Context.raw.layer.fill_layer);
				}
				base_update_fill_layer(Context.raw.gizmo_started);
			}
		}

		Context.raw.gizmo_started = false;
		if (mouse_started("left") && paint_object.name != "Scene") {
			// Translate, scale
			let trs: transform_t[] = [
				Context.raw.gizmo_translate_x.transform,
				Context.raw.gizmo_translate_y.transform,
				Context.raw.gizmo_translate_z.transform,
				Context.raw.gizmo_scale_x.transform,
				Context.raw.gizmo_scale_y.transform,
				Context.raw.gizmo_scale_z.transform
			];
			let hit: transform_t = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (hit.object == Context.raw.gizmo_translate_x) Context.raw.translate_x = true;
				else if (hit.object == Context.raw.gizmo_translate_y) Context.raw.translate_y = true;
				else if (hit.object == Context.raw.gizmo_translate_z) Context.raw.translate_z = true;
				else if (hit.object == Context.raw.gizmo_scale_x) Context.raw.scale_x = true;
				else if (hit.object == Context.raw.gizmo_scale_y) Context.raw.scale_y = true;
				else if (hit.object == Context.raw.gizmo_scale_z) Context.raw.scale_z = true;
				if (Context.raw.translate_x || Context.raw.translate_y || Context.raw.translate_z || Context.raw.scale_x || Context.raw.scale_y || Context.raw.scale_z) {
					Context.raw.gizmo_offset = 0.0;
					Context.raw.gizmo_started = true;
				}
			}
			else {
				// Rotate
				let trs: transform_t[] = [
					Context.raw.gizmo_rotate_x.transform,
					Context.raw.gizmo_rotate_y.transform,
					Context.raw.gizmo_rotate_z.transform
				];
				let hit: transform_t = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (hit.object == Context.raw.gizmo_rotate_x) Context.raw.rotate_x = true;
					else if (hit.object == Context.raw.gizmo_rotate_y) Context.raw.rotate_y = true;
					else if (hit.object == Context.raw.gizmo_rotate_z) Context.raw.rotate_z = true;
					if (Context.raw.rotate_x || Context.raw.rotate_y || Context.raw.rotate_z) {
						Context.raw.gizmo_offset = 0.0;
						Context.raw.gizmo_started = true;
					}
				}
			}
		}
		else if (mouse_released("left")) {
			Context.raw.translate_x = Context.raw.translate_y = Context.raw.translate_z = false;
			Context.raw.scale_x = Context.raw.scale_y = Context.raw.scale_z = false;
			Context.raw.rotate_x = Context.raw.rotate_y = Context.raw.rotate_z = false;
		}

		if (Context.raw.translate_x || Context.raw.translate_y || Context.raw.translate_z || Context.raw.scale_x || Context.raw.scale_y || Context.raw.scale_z || Context.raw.rotate_x || Context.raw.rotate_y || Context.raw.rotate_z) {
			Context.raw.rdirty = 2;

			if (is_object) {
				let t: transform_t = paint_object.transform;
				vec4_set(Gizmo.v, transform_world_x(t), transform_world_y(t), transform_world_z(t));
			}
			else if (is_decal) {
				vec4_set(Gizmo.v, Context.raw.layer.decal_mat.m[12], Context.raw.layer.decal_mat.m[13], Context.raw.layer.decal_mat.m[14]);
			}

			if (Context.raw.translate_x || Context.raw.scale_x) {
				let hit: vec4_t = raycast_plane_intersect(vec4_y_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmo_started) Context.raw.gizmo_offset = hit.x - Gizmo.v.x;
					Context.raw.gizmo_drag = hit.x - Context.raw.gizmo_offset;
				}
			}
			else if (Context.raw.translate_y || Context.raw.scale_y) {
				let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmo_started) Context.raw.gizmo_offset = hit.y - Gizmo.v.y;
					Context.raw.gizmo_drag = hit.y - Context.raw.gizmo_offset;
				}
			}
			else if (Context.raw.translate_z || Context.raw.scale_z) {
				let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmo_started) Context.raw.gizmo_offset = hit.z - Gizmo.v.z;
					Context.raw.gizmo_drag = hit.z - Context.raw.gizmo_offset;
				}
			}
			else if (Context.raw.rotate_x) {
				let hit: vec4_t = raycast_plane_intersect(vec4_x_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmo_started) {
						mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmo_offset = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z);
					}
					Context.raw.gizmo_drag = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z) - Context.raw.gizmo_offset;
				}
			}
			else if (Context.raw.rotate_y) {
				let hit: vec4_t = raycast_plane_intersect(vec4_y_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmo_started) {
						mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmo_offset = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x);
					}
					Context.raw.gizmo_drag = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x) - Context.raw.gizmo_offset;
				}
			}
			else if (Context.raw.rotate_z) {
				let hit: vec4_t = raycast_plane_intersect(vec4_z_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmo_started) {
						mat4_decompose(Context.raw.layer.decal_mat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmo_offset = Math.atan2(hit.y - Gizmo.v.y, hit.x - Gizmo.v.x);
					}
					Context.raw.gizmo_drag = Math.atan2(hit.y - Gizmo.v.y, hit.x - Gizmo.v.x) - Context.raw.gizmo_offset;
				}
			}

			if (Context.raw.gizmo_started) Context.raw.gizmo_drag_last = Context.raw.gizmo_drag;

			///if is_forge
			UtilMesh.remove_merged_mesh();
			RenderPathRaytrace.ready = false;
			///end
		}

		_input_occupied = (Context.raw.translate_x || Context.raw.translate_y || Context.raw.translate_z || Context.raw.scale_x || Context.raw.scale_y || Context.raw.scale_z || Context.raw.rotate_x || Context.raw.rotate_y || Context.raw.rotate_z) && mouse_view_x() < base_w();
	}
}

///end
