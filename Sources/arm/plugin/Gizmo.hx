package arm.plugin;

import iron.object.MeshObject;
import iron.object.LightObject;
import iron.system.Input;
import iron.math.RayCaster;
import iron.math.Vec4;
import iron.math.Quat;
import iron.Scene;
import arm.ui.UISidebar;
import arm.ui.UIHeader;
import arm.Enums;

class Gizmo {

	static var v = new Vec4();
	static var v0 = new Vec4();
	static var q = new Quat();
	static var q0 = new Quat();

	public static function update() {

		var isRender = UIHeader.inst.worktab.position == SpaceRender;
		var isPaint = UIHeader.inst.worktab.position == SpacePaint;
		var isObject = isRender && Context.object != null;
		var isDecal = isPaint && Context.layer.fill_layer != null && Context.layer.uvType == UVProject && !Context.layerIsMask;

		var gizmo = Context.gizmo;
		var hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutDown);
		gizmo.visible = (isObject || isDecal) && !hide;
		if (!gizmo.visible) return;

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		if (isObject) {
			gizmo.transform.loc.setFrom(Context.object.transform.loc);
		}
		else if (isDecal) {
			gizmo.transform.loc.set(Context.layer.decalMat._30, Context.layer.decalMat._31, Context.layer.decalMat._32);
		}
		var cam = Scene.active.camera;
		var fov = cam.data.raw.fov;
		var dist = Vec4.distance(cam.transform.loc, gizmo.transform.loc) / 8 * fov;
		gizmo.transform.scale.set(dist, dist, dist);
		Context.gizmoTranslateX.transform.scale.set(dist, dist, dist);
		Context.gizmoTranslateY.transform.scale.set(dist, dist, dist);
		Context.gizmoTranslateZ.transform.scale.set(dist, dist, dist);
		Context.gizmoScaleX.transform.scale.set(dist, dist, dist);
		Context.gizmoScaleY.transform.scale.set(dist, dist, dist);
		Context.gizmoScaleZ.transform.scale.set(dist, dist, dist);
		Context.gizmoRotateX.transform.scale.set(dist, dist, dist);
		Context.gizmoRotateY.transform.scale.set(dist, dist, dist);
		Context.gizmoRotateZ.transform.scale.set(dist, dist, dist);
		gizmo.transform.buildMatrix();

		// Scene control
		if (isObject) {
			if (mouse.viewX < App.w()) {
				if (kb.started("delete") || kb.started("backspace")) {
					if (Context.object != null) {
						Context.object.remove();
						Context.selectObject(Scene.active.getChild("Scene"));
					}
				}
				if (kb.started("c") && Context.object != null) {
					if (Std.is(Context.object, MeshObject)) {
						var mo = cast(Context.object, MeshObject);
						var object = Scene.active.addMeshObject(mo.data, mo.materials, Scene.active.getChild("Scene"));
						object.name = mo.name + ".1";

						object.transform.loc.setFrom(mo.transform.loc);
						object.transform.rot.setFrom(mo.transform.rot);
						object.transform.scale.setFrom(mo.transform.scale);

						var hit = RayCaster.planeIntersect(Vec4.zAxis(), new Vec4(), mouse.viewX, mouse.viewY, Scene.active.camera);
						if (hit != null) {
							object.transform.loc.x = hit.x;
							object.transform.loc.y = hit.y;
							object.transform.setRotation(0, 0, Math.random() * 3.1516 * 2);
						}

						object.transform.buildMatrix();

						for (t in mo.traits) { // Clone traits
							var trait = Type.createInstance(Type.getClass(t), []);
							object.addTrait(trait);
						}

						Context.selectObject(object);
					}
					else if (Std.is(Context.object, LightObject)) {
						var lo = cast(Context.object, LightObject);
						var object = Scene.active.addLightObject(lo.data, Scene.active.getChild("Scene"));
						object.name = lo.name + ".1";

						object.transform.loc.setFrom(lo.transform.loc);
						object.transform.rot.setFrom(lo.transform.rot);
						object.transform.scale.setFrom(lo.transform.scale);
						object.transform.buildMatrix();

						Context.selectObject(object);
					}
					Context.rdirty = 3;
					Context.ddirty = 3;
				}
			}

			if (mouse.started("middle")) {
				#if arm_physics
				var physics = arm.plugin.PhysicsWorld.active;
				var pb = physics.pickClosest(mouse.viewX, mouse.viewY);
				if (pb != null) Context.selectObject(pb.object);
				#end
			}

			if (Context.translateX || Context.translateY || Context.translateZ || Context.scaleX || Context.scaleY || Context.scaleZ || Context.rotateX || Context.rotateY || Context.rotateZ) {
				if (Context.translateX) {
					Context.object.transform.loc.x = Context.gizmoDrag;
				}
				else if (Context.translateY) {
					Context.object.transform.loc.y = Context.gizmoDrag;
				}
				else if (Context.translateZ) {
					Context.object.transform.loc.z = Context.gizmoDrag;
				}
				else if (Context.scaleX) {
					Context.object.transform.scale.x += Context.gizmoDrag - Context.gizmoDragLast;
				}
				else if (Context.scaleY) {
					Context.object.transform.scale.y += Context.gizmoDrag - Context.gizmoDragLast;
				}
				else if (Context.scaleZ) {
					Context.object.transform.scale.z += Context.gizmoDrag - Context.gizmoDragLast;
				}
				else if (Context.rotateX) {
					q0.fromAxisAngle(Vec4.xAxis(), Context.gizmoDrag - Context.gizmoDragLast);
					Context.object.transform.rot.mult(q0);
				}
				else if (Context.rotateY) {
					q0.fromAxisAngle(Vec4.yAxis(), Context.gizmoDrag - Context.gizmoDragLast);
					Context.object.transform.rot.mult(q0);
				}
				else if (Context.rotateZ) {
					q0.fromAxisAngle(Vec4.zAxis(), Context.gizmoDrag - Context.gizmoDragLast);
					Context.object.transform.rot.mult(q0);
				}
				Context.gizmoDragLast = Context.gizmoDrag;

				Context.object.transform.buildMatrix();
				#if arm_physics
				var pb = Context.object.getTrait(arm.plugin.PhysicsBody);
				if (pb != null) pb.syncTransform();
				#end
			}
		}
		// Decal layer control
		else if (isDecal) {
			if (Context.translateX || Context.translateY || Context.translateZ || Context.scaleX || Context.scaleY || Context.scaleZ || Context.rotateX || Context.rotateY || Context.rotateZ) {
				if (Context.translateX) {
					Context.layer.decalMat._30 = Context.gizmoDrag;
				}
				else if (Context.translateY) {
					Context.layer.decalMat._31 = Context.gizmoDrag;
				}
				else if (Context.translateZ) {
					Context.layer.decalMat._32 = Context.gizmoDrag;
				}
				else if (Context.scaleX) {
					Context.layer.decalMat.decompose(v, q, v0);
					v0.x += Context.gizmoDrag - Context.gizmoDragLast;
					Context.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.scaleY) {
					Context.layer.decalMat.decompose(v, q, v0);
					v0.y += Context.gizmoDrag - Context.gizmoDragLast;
					Context.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.scaleZ) {
					Context.layer.decalMat.decompose(v, q, v0);
					v0.z += Context.gizmoDrag - Context.gizmoDragLast;
					Context.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.rotateX) {
					Context.layer.decalMat.decompose(v, q, v0);
					q0.fromAxisAngle(Vec4.xAxis(), -Context.gizmoDrag + Context.gizmoDragLast);
					q.multquats(q0, q);
					Context.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.rotateY) {
					Context.layer.decalMat.decompose(v, q, v0);
					q0.fromAxisAngle(Vec4.yAxis(), -Context.gizmoDrag + Context.gizmoDragLast);
					q.multquats(q0, q);
					Context.layer.decalMat.compose(v, q, v0);
				}
				else if (Context.rotateZ) {
					Context.layer.decalMat.decompose(v, q, v0);
					q0.fromAxisAngle(Vec4.zAxis(), Context.gizmoDrag - Context.gizmoDragLast);
					q.multquats(q0, q);
					Context.layer.decalMat.compose(v, q, v0);
				}
				Context.gizmoDragLast = Context.gizmoDrag;

				if (Context.material != Context.layer.fill_layer) {
					Context.setMaterial(Context.layer.fill_layer);
				}
				Layers.updateFillLayer(Context.gizmoStarted);
			}
		}

		Context.gizmoStarted = false;
		if (mouse.started("left") && Context.object.name != "Scene") {
			// Translate, scale
			var trs = [Context.gizmoTranslateX.transform, Context.gizmoTranslateY.transform, Context.gizmoTranslateZ.transform,
					   Context.gizmoScaleX.transform, Context.gizmoScaleY.transform, Context.gizmoScaleZ.transform];
			var hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
			if (hit != null) {
				if (hit.object == Context.gizmoTranslateX) Context.translateX = true;
				else if (hit.object == Context.gizmoTranslateY) Context.translateY = true;
				else if (hit.object == Context.gizmoTranslateZ) Context.translateZ = true;
				else if (hit.object == Context.gizmoScaleX) Context.scaleX = true;
				else if (hit.object == Context.gizmoScaleY) Context.scaleY = true;
				else if (hit.object == Context.gizmoScaleZ) Context.scaleZ = true;
				if (Context.translateX || Context.translateY || Context.translateZ || Context.scaleX || Context.scaleY || Context.scaleZ) {
					Context.gizmoOffset = 0.0;
					Context.gizmoStarted = true;
				}
			}
			else {
				// Rotate
				var trs = [Context.gizmoRotateX.transform, Context.gizmoRotateY.transform, Context.gizmoRotateZ.transform];
				var hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (hit.object == Context.gizmoRotateX) Context.rotateX = true;
					else if (hit.object == Context.gizmoRotateY) Context.rotateY = true;
					else if (hit.object == Context.gizmoRotateZ) Context.rotateZ = true;
					if (Context.rotateX || Context.rotateY || Context.rotateZ) {
						Context.gizmoOffset = 0.0;
						Context.gizmoStarted = true;
					}
				}
			}
		}
		else if (mouse.released("left")) {
			Context.translateX = Context.translateY = Context.translateZ = false;
			Context.scaleX = Context.scaleY = Context.scaleZ = false;
			Context.rotateX = Context.rotateY = Context.rotateZ = false;
		}

		if (Context.translateX || Context.translateY || Context.translateZ || Context.scaleX || Context.scaleY || Context.scaleZ || Context.rotateX || Context.rotateY || Context.rotateZ) {
			Context.rdirty = 2;

			if (isObject) {
				var t = Context.object.transform;
				v.set(t.worldx(), t.worldy(), t.worldz());
			}
			else if (isDecal) {
				v.set(Context.layer.decalMat._30, Context.layer.decalMat._31, Context.layer.decalMat._32);
			}

			if (Context.translateX || Context.scaleX) {
				var hit = RayCaster.planeIntersect(Vec4.yAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.gizmoStarted) Context.gizmoOffset = hit.x - v.x;
					Context.gizmoDrag = hit.x - Context.gizmoOffset;
				}
			}
			else if (Context.translateY || Context.scaleY) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.gizmoStarted) Context.gizmoOffset = hit.y - v.y;
					Context.gizmoDrag = hit.y - Context.gizmoOffset;
				}
			}
			else if (Context.translateZ || Context.scaleZ) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.gizmoStarted) Context.gizmoOffset = hit.z - v.z;
					Context.gizmoDrag = hit.z - Context.gizmoOffset;
				}
			}
			else if (Context.rotateX) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.gizmoStarted) {
						Context.layer.decalMat.decompose(v, q, v0);
						Context.gizmoOffset = Math.atan2(hit.y - v.y, hit.z - v.z);
					}
					Context.gizmoDrag = Math.atan2(hit.y - v.y, hit.z - v.z) - Context.gizmoOffset;
				}
			}
			else if (Context.rotateY) {
				var hit = RayCaster.planeIntersect(Vec4.yAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.gizmoStarted) {
						Context.layer.decalMat.decompose(v, q, v0);
						Context.gizmoOffset = Math.atan2(hit.z - v.z, hit.x - v.x);
					}
					Context.gizmoDrag = Math.atan2(hit.z - v.z, hit.x - v.x) - Context.gizmoOffset;
				}
			}
			else if (Context.rotateZ) {
				var hit = RayCaster.planeIntersect(Vec4.zAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.gizmoStarted) {
						Context.layer.decalMat.decompose(v, q, v0);
						Context.gizmoOffset = Math.atan2(hit.y - v.y, hit.x - v.x);
					}
					Context.gizmoDrag = Math.atan2(hit.y - v.y, hit.x - v.x) - Context.gizmoOffset;
				}
			}

			if (Context.gizmoStarted) Context.gizmoDragLast = Context.gizmoDrag;
		}

		Input.occupied = (Context.translateX || Context.translateY || Context.translateZ || Context.scaleX || Context.scaleY || Context.scaleZ || Context.rotateX || Context.rotateY || Context.rotateZ) && mouse.viewX < App.w();
	}
}
