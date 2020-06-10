package arm.plugin;

import iron.object.MeshObject;
import iron.object.LightObject;
import iron.system.Input;
import iron.math.RayCaster;
import iron.math.Vec4;
import iron.Scene;
import arm.ui.UISidebar;
import arm.ui.UIHeader;
import arm.Enums;

class Gizmo {

	static var v = new Vec4();

	public static function update() {

		var isRender = UIHeader.inst.worktab.position == SpaceRender;
		var isPaint = UIHeader.inst.worktab.position == SpacePaint;
		var isObject = isRender && Context.object != null;
		var isDecal = isPaint && Context.layer.material_mask != null && Context.layer.uvType == UVProject;

		var gizmo = Context.gizmo;
		gizmo.visible = isObject || isDecal;
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
		var dist = Vec4.distance(cam.transform.loc, gizmo.transform.loc) / 10;
		gizmo.transform.scale.set(dist, dist, dist);
		Context.gizmoX.transform.scale.set(dist, dist, dist);
		Context.gizmoY.transform.scale.set(dist, dist, dist);
		Context.gizmoZ.transform.scale.set(dist, dist, dist);
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
				if (kb.started("m")) { // skip voxel
					var raw = Context.materialScene.data.raw;
					raw.skip_context = raw.skip_context == "" ? "voxel" : "";
				}
			}

			if (mouse.started("middle")) {
				#if arm_physics
				var physics = arm.plugin.PhysicsWorld.active;
				var pb = physics.pickClosest(mouse.viewX, mouse.viewY);
				if (pb != null) Context.selectObject(pb.object);
				#end
			}

			if (Context.axisX || Context.axisY || Context.axisZ) {
				if (Context.axisX) {
					Context.object.transform.loc.x = Context.axisLoc;
				}
				else if (Context.axisY) {
					Context.object.transform.loc.y = Context.axisLoc;
				}
				else if (Context.axisZ) {
					Context.object.transform.loc.z = Context.axisLoc;
				}

				Context.object.transform.buildMatrix();
				#if arm_physics
				var pb = Context.object.getTrait(arm.plugin.PhysicsBody);
				if (pb != null) pb.syncTransform();
				#end
			}
		}
		// Decal layer control
		else if (isDecal) {
			if (Context.axisX || Context.axisY || Context.axisZ) {
				if (Context.axisX) {
					Context.layer.decalMat._30 = Context.axisLoc;
				}
				else if (Context.axisY) {
					Context.layer.decalMat._31 = Context.axisLoc;
				}
				else if (Context.axisZ) {
					Context.layer.decalMat._32 = Context.axisLoc;
				}
				Layers.updateFillLayer();
			}
		}

		// Axis movement
		if (mouse.started("left") && Context.object.name != "Scene") {
			var trs = [Context.gizmoX.transform, Context.gizmoY.transform, Context.gizmoZ.transform];
			var hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
			if (hit != null) {
				if (hit.object == Context.gizmoX) Context.axisX = true;
				else if (hit.object == Context.gizmoY) Context.axisY = true;
				else if (hit.object == Context.gizmoZ) Context.axisZ = true;
				if (Context.axisX || Context.axisY || Context.axisZ) Context.axisStart = 0.0;
			}
		}
		else if (mouse.released("left")) {
			Context.axisX = Context.axisY = Context.axisZ = false;
		}

		if (Context.axisX || Context.axisY || Context.axisZ) {
			Context.rdirty = 2;

			if (isObject) {
				var t = Context.object.transform;
				v.set(t.worldx(), t.worldy(), t.worldz());
			}
			else if (isDecal) {
				v.set(Context.layer.decalMat._30, Context.layer.decalMat._31, Context.layer.decalMat._32);
			}

			if (Context.axisX) {
				var hit = RayCaster.planeIntersect(Vec4.yAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.axisStart == 0) Context.axisStart = hit.x - v.x;
					Context.axisLoc = hit.x - Context.axisStart;
				}
			}
			else if (Context.axisY) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.axisStart == 0) Context.axisStart = hit.y - v.y;
					Context.axisLoc = hit.y - Context.axisStart;
				}
			}
			else if (Context.axisZ) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (Context.axisStart == 0) Context.axisStart = hit.z - v.z;
					Context.axisLoc = hit.z - Context.axisStart;
				}
			}
		}

		Input.occupied = (Context.axisX || Context.axisY || Context.axisZ) && mouse.viewX < App.w();
	}
}
