
class Gizmo {

	static v = new Vec4();
	static v0 = new Vec4();
	static q = new Quat();
	static q0 = new Quat();

	static update = () => {
		let isObject = Context.raw.tool == WorkspaceTool.ToolGizmo;
		let isDecal = Base.isDecalLayer();

		let gizmo = Context.raw.gizmo;
		let hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutType.ShortcutDown);
		gizmo.visible = (isObject || isDecal) && !hide;
		if (!gizmo.visible) return;

		let mouse = Input.getMouse();
		let kb = Input.getKeyboard();

		let paintObject: BaseObject = Context.raw.paintObject;
		///if is_forge
		if (Context.raw.selectedObject != null) {
			paintObject = Context.raw.selectedObject;
		}
		///end

		if (isObject) {
			gizmo.transform.loc.setFrom(paintObject.transform.loc);
		}
		else if (isDecal) {
			gizmo.transform.loc.set(Context.raw.layer.decalMat._30, Context.raw.layer.decalMat._31, Context.raw.layer.decalMat._32);
		}
		let cam = Scene.active.camera;
		let fov = cam.data.raw.fov;
		let dist = Vec4.distance(cam.transform.loc, gizmo.transform.loc) / 8 * fov;
		gizmo.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoTranslateX.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoTranslateY.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoTranslateZ.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoScaleX.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoScaleY.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoScaleZ.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoRotateX.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoRotateY.transform.scale.set(dist, dist, dist);
		Context.raw.gizmoRotateZ.transform.scale.set(dist, dist, dist);
		gizmo.transform.buildMatrix();

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
					Gizmo.q0.fromAxisAngle(Vec4.xAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					paintObject.transform.rot.mult(Gizmo.q0);
				}
				else if (Context.raw.rotateY) {
					Gizmo.q0.fromAxisAngle(Vec4.yAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					paintObject.transform.rot.mult(Gizmo.q0);
				}
				else if (Context.raw.rotateZ) {
					Gizmo.q0.fromAxisAngle(Vec4.zAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					paintObject.transform.rot.mult(Gizmo.q0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				paintObject.transform.buildMatrix();
				///if arm_physics
				let pb = paintObject.getTrait(PhysicsBody);
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
					Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.x += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Context.raw.layer.decalMat.compose(Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scaleY) {
					Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.y += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Context.raw.layer.decalMat.compose(Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.scaleZ) {
					Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.v0.z += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Context.raw.layer.decalMat.compose(Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateX) {
					Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.q0.fromAxisAngle(Vec4.xAxis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					Gizmo.q.multquats(Gizmo.q0, Gizmo.q);
					Context.raw.layer.decalMat.compose(Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateY) {
					Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.q0.fromAxisAngle(Vec4.yAxis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					Gizmo.q.multquats(Gizmo.q0, Gizmo.q);
					Context.raw.layer.decalMat.compose(Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				else if (Context.raw.rotateZ) {
					Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
					Gizmo.q0.fromAxisAngle(Vec4.zAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					Gizmo.q.multquats(Gizmo.q0, Gizmo.q);
					Context.raw.layer.decalMat.compose(Gizmo.v, Gizmo.q, Gizmo.v0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				if (Context.raw.material != Context.raw.layer.fill_layer) {
					Context.setMaterial(Context.raw.layer.fill_layer);
				}
				Base.updateFillLayer(Context.raw.gizmoStarted);
			}
		}

		Context.raw.gizmoStarted = false;
		if (mouse.started("left") && paintObject.name != "Scene") {
			// Translate, scale
			let trs = [Context.raw.gizmoTranslateX.transform, Context.raw.gizmoTranslateY.transform, Context.raw.gizmoTranslateZ.transform,
					   Context.raw.gizmoScaleX.transform, Context.raw.gizmoScaleY.transform, Context.raw.gizmoScaleZ.transform];
			let hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
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
				let hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
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
		else if (mouse.released("left")) {
			Context.raw.translateX = Context.raw.translateY = Context.raw.translateZ = false;
			Context.raw.scaleX = Context.raw.scaleY = Context.raw.scaleZ = false;
			Context.raw.rotateX = Context.raw.rotateY = Context.raw.rotateZ = false;
		}

		if (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) {
			Context.raw.rdirty = 2;

			if (isObject) {
				let t = paintObject.transform;
				Gizmo.v.set(t.worldx(), t.worldy(), t.worldz());
			}
			else if (isDecal) {
				Gizmo.v.set(Context.raw.layer.decalMat._30, Context.raw.layer.decalMat._31, Context.raw.layer.decalMat._32);
			}

			if (Context.raw.translateX || Context.raw.scaleX) {
				let hit = RayCaster.planeIntersect(Vec4.yAxis(), Gizmo.v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.x - Gizmo.v.x;
					Context.raw.gizmoDrag = hit.x - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateY || Context.raw.scaleY) {
				let hit = RayCaster.planeIntersect(Vec4.xAxis(), Gizmo.v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.y - Gizmo.v.y;
					Context.raw.gizmoDrag = hit.y - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateZ || Context.raw.scaleZ) {
				let hit = RayCaster.planeIntersect(Vec4.xAxis(), Gizmo.v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.z - Gizmo.v.z;
					Context.raw.gizmoDrag = hit.z - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateX) {
				let hit = RayCaster.planeIntersect(Vec4.xAxis(), Gizmo.v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmoOffset = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.y - Gizmo.v.y, hit.z - Gizmo.v.z) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateY) {
				let hit = RayCaster.planeIntersect(Vec4.yAxis(), Gizmo.v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
						Context.raw.gizmoOffset = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.z - Gizmo.v.z, hit.x - Gizmo.v.x) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateZ) {
				let hit = RayCaster.planeIntersect(Vec4.zAxis(), Gizmo.v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Context.raw.layer.decalMat.decompose(Gizmo.v, Gizmo.q, Gizmo.v0);
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

		Input.occupied = (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) && mouse.viewX < Base.w();
	}
}
