package arm.render;

import iron.system.Input;
import iron.math.RayCaster;
import iron.math.Vec4;
import iron.math.Quat;
import iron.Scene;

class Gizmo {

	static var v = new Vec4();
	static var v0 = new Vec4();
	static var q = new Quat();
	static var q0 = new Quat();

	public static function update() {
		var isObject = Context.raw.tool == ToolGizmo;
		var isDecal = App.isDecalLayer();

		var gizmo = Context.raw.gizmo;
		var hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutDown);
		gizmo.visible = (isObject || isDecal) && !hide;
		if (!gizmo.visible) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var paintObject: iron.object.Object = cast Context.raw.paintObject;
		#if is_forge
		if (Context.raw.selectedObject != null) {
			paintObject = Context.raw.selectedObject;
		}
		#end

		if (isObject) {
			gizmo.transform.loc.setFrom(paintObject.transform.loc);
		}
		else if (isDecal) {
			gizmo.transform.loc.set(Context.raw.layer.decalMat._30, Context.raw.layer.decalMat._31, Context.raw.layer.decalMat._32);
		}
		var cam = Scene.active.camera;
		var fov = cam.data.raw.fov;
		var dist = Vec4.distance(cam.transform.loc, gizmo.transform.loc) / 8 * fov;
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
					q0.fromAxisAngle(Vec4.xAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					paintObject.transform.rot.mult(q0);
				}
				else if (Context.raw.rotateY) {
					q0.fromAxisAngle(Vec4.yAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					paintObject.transform.rot.mult(q0);
				}
				else if (Context.raw.rotateZ) {
					q0.fromAxisAngle(Vec4.zAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					paintObject.transform.rot.mult(q0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				paintObject.transform.buildMatrix();
				#if arm_physics
				var pb = paintObject.getTrait(arm.plugin.PhysicsBody);
				if (pb != null) pb.syncTransform();
				#end
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
					Context.raw.layer.decalMat.decompose(v, q, v0);
					v0.x += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Context.raw.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.raw.scaleY) {
					Context.raw.layer.decalMat.decompose(v, q, v0);
					v0.y += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Context.raw.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.raw.scaleZ) {
					Context.raw.layer.decalMat.decompose(v, q, v0);
					v0.z += Context.raw.gizmoDrag - Context.raw.gizmoDragLast;
					Context.raw.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.raw.rotateX) {
					Context.raw.layer.decalMat.decompose(v, q, v0);
					q0.fromAxisAngle(Vec4.xAxis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					q.multquats(q0, q);
					Context.raw.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.raw.rotateY) {
					Context.raw.layer.decalMat.decompose(v, q, v0);
					q0.fromAxisAngle(Vec4.yAxis(), -Context.raw.gizmoDrag + Context.raw.gizmoDragLast);
					q.multquats(q0, q);
					Context.raw.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.raw.rotateZ) {
					Context.raw.layer.decalMat.decompose(v, q, v0);
					q0.fromAxisAngle(Vec4.zAxis(), Context.raw.gizmoDrag - Context.raw.gizmoDragLast);
					q.multquats(q0, q);
					Context.raw.layer.decalMat.compose(v, q, v0);
				}
				Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

				if (Context.raw.material != Context.raw.layer.fill_layer) {
					Context.setMaterial(Context.raw.layer.fill_layer);
				}
				App.updateFillLayer(Context.raw.gizmoStarted);
			}
		}

		Context.raw.gizmoStarted = false;
		if (mouse.started("left") && paintObject.name != "Scene") {
			// Translate, scale
			var trs = [Context.raw.gizmoTranslateX.transform, Context.raw.gizmoTranslateY.transform, Context.raw.gizmoTranslateZ.transform,
					   Context.raw.gizmoScaleX.transform, Context.raw.gizmoScaleY.transform, Context.raw.gizmoScaleZ.transform];
			var hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
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
				var trs = [Context.raw.gizmoRotateX.transform, Context.raw.gizmoRotateY.transform, Context.raw.gizmoRotateZ.transform];
				var hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
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
				var t = paintObject.transform;
				v.set(t.worldx(), t.worldy(), t.worldz());
			}
			else if (isDecal) {
				v.set(Context.raw.layer.decalMat._30, Context.raw.layer.decalMat._31, Context.raw.layer.decalMat._32);
			}

			if (Context.raw.translateX || Context.raw.scaleX) {
				var hit = RayCaster.planeIntersect(Vec4.yAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.x - v.x;
					Context.raw.gizmoDrag = hit.x - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateY || Context.raw.scaleY) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.y - v.y;
					Context.raw.gizmoDrag = hit.y - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.translateZ || Context.raw.scaleZ) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) Context.raw.gizmoOffset = hit.z - v.z;
					Context.raw.gizmoDrag = hit.z - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateX) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Context.raw.layer.decalMat.decompose(v, q, v0);
						Context.raw.gizmoOffset = Math.atan2(hit.y - v.y, hit.z - v.z);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.y - v.y, hit.z - v.z) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateY) {
				var hit = RayCaster.planeIntersect(Vec4.yAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Context.raw.layer.decalMat.decompose(v, q, v0);
						Context.raw.gizmoOffset = Math.atan2(hit.z - v.z, hit.x - v.x);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.z - v.z, hit.x - v.x) - Context.raw.gizmoOffset;
				}
			}
			else if (Context.raw.rotateZ) {
				var hit = RayCaster.planeIntersect(Vec4.zAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.raw.gizmoStarted) {
						Context.raw.layer.decalMat.decompose(v, q, v0);
						Context.raw.gizmoOffset = Math.atan2(hit.y - v.y, hit.x - v.x);
					}
					Context.raw.gizmoDrag = Math.atan2(hit.y - v.y, hit.x - v.x) - Context.raw.gizmoOffset;
				}
			}

			if (Context.raw.gizmoStarted) Context.raw.gizmoDragLast = Context.raw.gizmoDrag;

			#if is_forge
			arm.util.MeshUtil.removeMergedMesh();
			arm.render.RenderPathRaytrace.ready = false;
			#end
		}

		Input.occupied = (Context.raw.translateX || Context.raw.translateY || Context.raw.translateZ || Context.raw.scaleX || Context.raw.scaleY || Context.raw.scaleZ || Context.raw.rotateX || Context.raw.rotateY || Context.raw.rotateZ) && mouse.viewX < App.w();
	}
}
