
///if (is_paint || is_sculpt)

class Gizmo {

	static v = vec4_create();
	static v0 = vec4_create();
	static q = quat_create();
	static q0 = quat_create();

	static update = () => {
		let isObject = Context.raw.tool == WorkspaceTool.ToolGizmo;
		let isDecal = Base.isDecalLayer();

		let gizmo = Context.raw.gizmo;
		let hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown);
		gizmo.visible = (isObject || isDecal) && !hide;
		if (!gizmo.visible) return;

		let paintObject: object_t = Context.raw.paintObject.base;
		///if is_forge
		if (Context.raw.selectedObject != null) {
			paintObject = Context.raw.selectedObject;
		}
		///end

		if (isObject) {
			vec4_set_from(gizmo.transform.loc, paintObject.transform.loc);
		}
		else if (isDecal) {
			vec4_set(gizmo.transform.loc, Context.raw.layer.decalMat.m[12], Context.raw.layer.decalMat.m[13], Context.raw.layer.decalMat.m[14]);
		}
		let cam = scene_camera;
		let fov = cam.data.fov;
		let dist = vec4_dist(cam.base.transform.loc, gizmo.transform.loc) / 8 * fov;
		vec4_set(gizmo.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoTranslateX.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoTranslateY.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoTranslateZ.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoScaleX.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoScaleY.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoScaleZ.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoRotateX.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoRotateY.transform.scale, dist, dist, dist);
		vec4_set(Context.raw.gizmoRotateZ.transform.scale, dist, dist, dist);
		transform_build_matrix(gizmo.transform);

		// Scene control
		if (isObject) {
			if (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) {
				if (Context.raw.translateX) {
					paintObject.transform.loc.x = Context.raw.gizmoDrag;
				}
				else if (Context.raw.translateY) {
					paintObject.transform.loc.y = Context.raw.gizmoDrag;
				}
				else if (Context.raw.translateZ) {
					paintObject.transform.loc.z = Context.raw.gizmoDrag;
				}
				else if (Context.raw.scaleX) {
					paintObject.transform.scale.x += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
				}
				else if (Context.raw.scaleY) {
					paintObject.transform.scale.y += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
				}
				else if (Context.raw.scaleZ) {
					paintObject.transform.scale.z += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
				}
				else if (Context.raw.rotateX) {
					quat_from_axis_angle(Gizmo.q0, vec4_x_axis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					quat_mult(paintObject.transform.rot, Gizmo.q0);
				}
				else if (Context.raw.rotateY) {
					quat_from_axis_angle(Gizmo.q0, vec4_y_axis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					quat_mult(paintObject.transform.rot, Gizmo.q0);
				}
				else if (Context.raw.rotateZ) {
					quat_from_axis_angle(Gizmo.q0, vec4_z_axis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					quat_mult(paintObject.transform.rot, Gizmo.q0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				transform_build_matrix(paintObject.transform);
				///if arm_physics
				let pb = (paintObject as any).physicsBody;
				if (pb != null) pb.syncTransform();
				///end
			}
		}
		// Decal layer control
		else if (isDecal) {
			if (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) {
				if (Context.raw.translateX) {
					Context.raw.layer.decalMat.m[12] = Context.raw.gizmoDrag;
				}
				else if (Context.raw.translateY) {
					Context.raw.layer.decalMat.m[13] = Context.raw.gizmoDrag;
				}
				else if (Context.raw.translateZ) {
					Context.raw.layer.decalMat.m[14] = Context.raw.gizmoDrag;
				}
				else if (Context.raw.scaleX) {
					mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.x += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					mat4_compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scaleY) {
					mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.y += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					mat4_compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scaleZ) {
					mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.z += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					mat4_compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateX) {
					mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					quat_from_axis_angle(Gizmo.q0, vec4_x_axis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					quat_mult_quats(Gizmo.q, Gizmo.q0, Gizmo.q);
					mat4_compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateY) {
					mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					quat_from_axis_angle(Gizmo.q0, vec4_y_axis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					quat_mult_quats(Gizmo.q, Gizmo.q0, Gizmo.q);
					mat4_compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateZ) {
					mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					quat_from_axis_angle(Gizmo.q0, vec4_z_axis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					quat_mult_quats(Gizmo.q, Gizmo.q0, Gizmo.q);
					mat4_compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				if (Context.raw.material != Context.raw.layer.fill_layer) {
					Context.setMaterial(Context.raw.layer.fill_layer);
				}
				Base.updateFillLayer(Context.raw.gizmoStarted);
			}
		}

		Context.raw.gizmoStarted = false;
		if (mouse_started("left") && paintObject.name != "Scene") {
			// Translate, scale
			let trs = [Context.raw.gizmoTranslateX.transform, Context.raw.gizmoTranslateY.transform, Context.raw.gizmoTranslateZ.transform,
					   Context.raw.gizmoScaleX.transform, Context.raw.gizmoScaleY.transform, Context.raw.gizmoScaleZ.transform];
			let hit = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
			if (hit != null) {
				if (hit.object == Context.raw.gizmoTranslateX) Context.raw.translateX = true;
				else if (hit.object == Context.raw.gizmoTranslateY) Context.raw.translateY = true;
				else if (hit.object == Context.raw.gizmoTranslateZ) Context.raw.translateZ = true;
				else if (hit.object == Context.raw.gizmoScaleX) Context.raw.scaleX = true;
				else if (hit.object == Context.raw.gizmoScaleY) Context.raw.scaleY = true;
				else if (hit.object == Context.raw.gizmoScaleZ) Context.raw.scaleZ = true;
				if (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ) {
					Context.raw.gizmoOffset = 0.0;
					Context.raw.gizmoStarted = true;
				}
			}
			else {
				// Rotate
				let trs = [Context.raw.gizmoRotateX.transform, Context.raw.gizmoRotateY.transform, Context.raw.gizmoRotateZ.transform];
				let hit = raycast_closest_box_intersect(trs, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (hit.object == Context.raw.gizmoRotateX) Context.raw.rotateX = true;
					else if (hit.object == Context.raw.gizmoRotateY) Context.raw.rotateY = true;
					else if (hit.object == Context.raw.gizmoRotateZ) Context.raw.rotateZ = true;
					if (Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) {
						Context.raw.gizmoOffset = 0.0;
						Context.raw.gizmoStarted = true;
					}
				}
			}
		}
		else if (mouse_released("left")) {
			Context.raw.translateX = Context.raw.translateY = Context.raw.translateZ = false;
			Context.raw.scaleX = Context.raw.scaleY = Context.raw.scaleZ = false;
			Context.raw.rotateX = Context.raw.rotateY = Context.raw.rotateZ = false;
		}

		if (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) {
			Context.raw.rdirty = 2;

			if (isObject) {
				let t = paintObject.transform;
				vec4_set(Gizmo.v, transform_world_x(t), transform_world_y(t), transform_world_z(t));
			}
			else if (isDecal) {
				vec4_set(Gizmo.v, Context.raw.layer.decalMat.m[12], Context.raw.layer.decalMat.m[13], Context.raw.layer.decalMat.m[14]);
			}

			if (Context.raw.translateX || Context.raw.scaleX) {
				let hit = raycast_plane_intersect(vec4_y_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.x - Gizmo.v.x;
					Context.raw.gizmoDrag = hit.x - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateY || Context.raw.scaleY) {
				let hit = raycast_plane_intersect(vec4_x_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.y - Gizmo.v.y;
					Context.raw.gizmoDrag = hit.y - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateZ || Context.raw.scaleZ) {
				let hit = raycast_plane_intersect(vec4_x_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.z - Gizmo.v.z;
					Context.raw.gizmoDrag = hit.z - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateX) {
				let hit = raycast_plane_intersect(vec4_x_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmoOffset = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateY) {
				let hit = raycast_plane_intersect(vec4_y_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmoOffset = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateZ) {
				let hit = raycast_plane_intersect(vec4_z_axis(), Gizmo.v, mouse_view_x(), mouse_view_y(), scene_camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						mat4_decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmoOffset = Math.atan2(hit.y - Gizmo.v.y, hit.x - Gizmo.v.x);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.y - Gizmo.v.y, hit.x - Gizmo.v.x) - Context.raw.gizmoOffset;
				}
			}

			if (Context.raw.gizmoStarted) Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

			///if is_forge
			UtilMesh.removeMergedMesh();
			RenderPathRaytrace.ready = false;
			///end
		}

		_input_occupied = (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) && mouse_view_x() < Base.w();
	}
}

///end
