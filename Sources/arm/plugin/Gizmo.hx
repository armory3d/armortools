package arm.plugin;

import iron.object.MeshObject;
import iron.object.LightObject;
import iron.system.Input;
import iron.math.RayCaster;
import iron.math.Vec4;
import iron.Scene;
import arm.ui.UISidebar;

class Gizmo {

	public static function update() {
		var gizmo = UISidebar.inst.gizmo;
		if (!gizmo.visible) return;

		if (Context.object != null) {
			var cam = Scene.active.camera;
			gizmo.transform.loc.setFrom(Context.object.transform.loc);
			var dist = Vec4.distance(cam.transform.loc, gizmo.transform.loc) / 10;
			gizmo.transform.scale.set(dist, dist, dist);
			UISidebar.inst.gizmoX.transform.scale.set(dist, dist, dist);
			UISidebar.inst.gizmoY.transform.scale.set(dist, dist, dist);
			UISidebar.inst.gizmoZ.transform.scale.set(dist, dist, dist);
			gizmo.transform.buildMatrix();
		}

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

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

		if (mouse.started("left") && Context.object.name != "Scene") {
			gizmo.transform.buildMatrix();
			var trs = [UISidebar.inst.gizmoX.transform, UISidebar.inst.gizmoY.transform, UISidebar.inst.gizmoZ.transform];
			var hit = RayCaster.closestBoxIntersect(trs, mouse.viewX, mouse.viewY, Scene.active.camera);
			if (hit != null) {
				if (hit.object == UISidebar.inst.gizmoX) UISidebar.inst.axisX = true;
				else if (hit.object == UISidebar.inst.gizmoY) UISidebar.inst.axisY = true;
				else if (hit.object == UISidebar.inst.gizmoZ) UISidebar.inst.axisZ = true;
				if (UISidebar.inst.axisX || UISidebar.inst.axisY || UISidebar.inst.axisZ) UISidebar.inst.axisStart = 0.0;
			}
		}
		else if (mouse.released("left")) {
			UISidebar.inst.axisX = UISidebar.inst.axisY = UISidebar.inst.axisZ = false;
		}

		if (UISidebar.inst.axisX || UISidebar.inst.axisY || UISidebar.inst.axisZ) {
			var t = Context.object.transform;
			var v = new Vec4();
			v.set(t.worldx(), t.worldy(), t.worldz());

			if (UISidebar.inst.axisX) {
				var hit = RayCaster.planeIntersect(Vec4.yAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (UISidebar.inst.axisStart == 0) UISidebar.inst.axisStart = hit.x - Context.object.transform.loc.x;
					Context.object.transform.loc.x = hit.x - UISidebar.inst.axisStart;
					Context.object.transform.buildMatrix();
					#if arm_physics
					var pb = Context.object.getTrait(arm.plugin.PhysicsBody);
					if (pb != null) pb.syncTransform();
					#end
				}
			}
			else if (UISidebar.inst.axisY) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (UISidebar.inst.axisStart == 0) UISidebar.inst.axisStart = hit.y - Context.object.transform.loc.y;
					Context.object.transform.loc.y = hit.y - UISidebar.inst.axisStart;
					Context.object.transform.buildMatrix();
					#if arm_physics
					var pb = Context.object.getTrait(arm.plugin.PhysicsBody);
					if (pb != null) pb.syncTransform();
					#end
				}
			}
			else if (UISidebar.inst.axisZ) {
				var hit = RayCaster.planeIntersect(Vec4.xAxis(), v, mouse.viewX, mouse.viewY, Scene.active.camera);
				if (hit != null) {
					if (UISidebar.inst.axisStart == 0) UISidebar.inst.axisStart = hit.z - Context.object.transform.loc.z;
					Context.object.transform.loc.z = hit.z - UISidebar.inst.axisStart;
					Context.object.transform.buildMatrix();
					#if arm_physics
					var pb = Context.object.getTrait(arm.plugin.PhysicsBody);
					if (pb != null) pb.syncTransform();
					#end
				}
			}
		}

		Input.occupied = (UISidebar.inst.axisX || UISidebar.inst.axisY || UISidebar.inst.axisZ) && mouse.viewX < App.w();
	}
}
