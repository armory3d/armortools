
///if (is_paint || is_sculpt)

class Gizmo {

	static v = Vec4.create();
	static v0 = Vec4.create();
	static q = Quat.create();
	static q0 = Quat.create();

	static update = () => {
		let isObject = Context.raw.tool == WorkspaceTool.ToolGizmo;
		let isDecal = Base.isDecalLayer();

		let gizmo = Context.raw.gizmo;
		let hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown);
		gizmo.visible = (isObject || isDecal) && !hide;
		if (!gizmo.visible) return;

		let paintObject: TBaseObject = Context.raw.paintObject.base;
		///if is_forge
		if (Context.raw.selectedObject != null) {
			paintObject = Context.raw.selectedObject;
		}
		///end

		if (isObject) {
			Vec4.setFrom(gizmo.transform.loc, paintObject.transform.loc);
		}
		else if (isDecal) {
			Vec4.set(gizmo.transform.loc, Context.raw.layer.decalMat._30, Context.raw.layer.decalMat._31, Context.raw.layer.decalMat._32);
		}
		let cam = Scene.camera;
		let fov = cam.data.fov;
		let dist = Vec4.distance(cam.base.transform.loc, gizmo.transform.loc) / 8 * fov;
		Vec4.set(gizmo.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoTranslateX.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoTranslateY.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoTranslateZ.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoScaleX.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoScaleY.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoScaleZ.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoRotateX.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoRotateY.transform.scale, dist, dist, dist);
		Vec4.set(Context.raw.gizmoRotateZ.transform.scale, dist, dist, dist);
		Transform.buildMatrix(gizmo.transform);

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
					Quat.fromAxisAngle(Gizmo.q0, Vec4.xAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					Quat.mult(paintObject.transform.rot, Gizmo.q0);
				}
				else if (Context.raw.rotateY) {
					Quat.fromAxisAngle(Gizmo.q0, Vec4.yAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					Quat.mult(paintObject.transform.rot, Gizmo.q0);
				}
				else if (Context.raw.rotateZ) {
					Quat.fromAxisAngle(Gizmo.q0, Vec4.zAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					Quat.mult(paintObject.transform.rot, Gizmo.q0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				Transform.buildMatrix(paintObject.transform);
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
					Context.raw.layer.decalMat._30 = Context.raw.gizmoDrag;
				}
				else if (Context.raw.translateY) {
					Context.raw.layer.decalMat._31 = Context.raw.gizmoDrag;
				}
				else if (Context.raw.translateZ) {
					Context.raw.layer.decalMat._32 = Context.raw.gizmoDrag;
				}
				else if (Context.raw.scaleX) {
					Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.x += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Mat4.compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scaleY) {
					Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.y += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Mat4.compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scaleZ) {
					Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.z += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Mat4.compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateX) {
					Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Quat.fromAxisAngle(Gizmo.q0, Vec4.xAxis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					Quat.multquats(Gizmo.q, Gizmo.q0, Gizmo.q);
					Mat4.compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateY) {
					Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Quat.fromAxisAngle(Gizmo.q0, Vec4.yAxis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					Quat.multquats(Gizmo.q, Gizmo.q0, Gizmo.q);
					Mat4.compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateZ) {
					Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
					Quat.fromAxisAngle(Gizmo.q0, Vec4.zAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					Quat.multquats(Gizmo.q, Gizmo.q0, Gizmo.q);
					Mat4.compose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				if (Context.raw.material != Context.raw.layer.fill_layer) {
					Context.setMaterial(Context.raw.layer.fill_layer);
				}
				Base.updateFillLayer(Context.raw.gizmoStarted);
			}
		}

		Context.raw.gizmoStarted = false;
		if (Mouse.started("left") && paintObject.name != "Scene") {
			// Translate, scale
			let trs = [Context.raw.gizmoTranslateX.transform, Context.raw.gizmoTranslateY.transform, Context.raw.gizmoTranslateZ.transform,
					   Context.raw.gizmoScaleX.transform, Context.raw.gizmoScaleY.transform, Context.raw.gizmoScaleZ.transform];
			let hit = RayCaster.closestBoxIntersect(trs, Mouse.viewX, Mouse.viewY, Scene.camera);
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
				let hit = RayCaster.closestBoxIntersect(trs, Mouse.viewX, Mouse.viewY, Scene.camera);
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
		else if (Mouse.released("left")) {
			Context.raw.translateX = Context.raw.translateY = Context.raw.translateZ = false;
			Context.raw.scaleX = Context.raw.scaleY = Context.raw.scaleZ = false;
			Context.raw.rotateX = Context.raw.rotateY = Context.raw.rotateZ = false;
		}

		if (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) {
			Context.raw.rdirty = 2;

			if (isObject) {
				let t = paintObject.transform;
				Vec4.set(Gizmo.v, Transform.worldx(t), Transform.worldy(t), Transform.worldz(t));
			}
			else if (isDecal) {
				Vec4.set(Gizmo.v, Context.raw.layer.decalMat._30, Context.raw.layer.decalMat._31, Context.raw.layer.decalMat._32);
			}

			if (Context.raw.translateX || Context.raw.scaleX) {
				let hit = RayCaster.planeIntersect(Vec4.yAxis(), Gizmo.v, Mouse.viewX, Mouse.viewY, Scene.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.x - Gizmo.v.x;
					Context.raw.gizmoDrag = hit.x - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateY || Context.raw.scaleY) {
				let hit = RayCaster.planeIntersect(Vec4.xAxis(), Gizmo.v, Mouse.viewX, Mouse.viewY, Scene.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.y - Gizmo.v.y;
					Context.raw.gizmoDrag = hit.y - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateZ || Context.raw.scaleZ) {
				let hit = RayCaster.planeIntersect(Vec4.xAxis(), Gizmo.v, Mouse.viewX, Mouse.viewY, Scene.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.z - Gizmo.v.z;
					Context.raw.gizmoDrag = hit.z - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateX) {
				let hit = RayCaster.planeIntersect(Vec4.xAxis(), Gizmo.v, Mouse.viewX, Mouse.viewY, Scene.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmoOffset = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateY) {
				let hit = RayCaster.planeIntersect(Vec4.yAxis(), Gizmo.v, Mouse.viewX, Mouse.viewY, Scene.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmoOffset = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateZ) {
				let hit = RayCaster.planeIntersect(Vec4.zAxis(), Gizmo.v, Mouse.viewX, Mouse.viewY, Scene.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Mat4.decompose(Context.raw.layer.decalMat, Gizmo.v, Gizmo.q, Gizmo.v0);
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

		Input.occupied = (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) && Mouse.viewX < Base.w();
	}
}

///end
