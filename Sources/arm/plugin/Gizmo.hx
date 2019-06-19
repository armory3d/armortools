package arm.plugin;

import iron.object.MeshObject;
import arm.ui.UITrait;

class Gizmo {

	public static function update() {
		var gizmo = UITrait.inst.gizmo;
		if (!gizmo.visible) return;

		if (UITrait.inst.selectedObject != null) {
			gizmo.transform.loc.setFrom(UITrait.inst.selectedObject.transform.loc);
			gizmo.transform.buildMatrix();
		}

		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		if (mouse.x < App.w()) {
			if (kb.started("delete") || kb.started("backspace")) {
				if (UITrait.inst.selectedObject != null) {
					UITrait.inst.selectedObject.remove();
					UITrait.inst.selectObject(iron.Scene.active.getChild("Scene"));
				}
			}
			if (kb.started("c") && UITrait.inst.selectedObject != null) {
				if (Std.is(UITrait.inst.selectedObject, MeshObject)) {
					var mo = cast(UITrait.inst.selectedObject, MeshObject);
					var object = iron.Scene.active.addMeshObject(mo.data, mo.materials, iron.Scene.active.getChild("Scene"));
					object.name = mo.name + '.1';

					object.transform.loc.setFrom(mo.transform.loc);
					object.transform.rot.setFrom(mo.transform.rot);
					object.transform.scale.setFrom(mo.transform.scale);

					var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.zAxis(), new iron.math.Vec4(), mouse.x, mouse.y, iron.Scene.active.camera);
					if (hit != null) {
						object.transform.loc.x = hit.x;
						object.transform.loc.y = hit.y;
						object.transform.setRotation(0, 0, Math.random() * 3.1516 * 2);
					}

					object.transform.buildMatrix();

					#if arm_physics
					object.addTrait(new armory.trait.physics.RigidBody(0, 0));
					#end

					UITrait.inst.selectObject(object);
				}
				else if (Std.is(UITrait.inst.selectedObject, iron.object.LightObject)) {
					var lo = cast(UITrait.inst.selectedObject, iron.object.LightObject);
					var object = iron.Scene.active.addLightObject(lo.data, iron.Scene.active.getChild("Scene"));
					object.name = lo.name + '.1';

					object.transform.loc.setFrom(lo.transform.loc);
					object.transform.rot.setFrom(lo.transform.rot);
					object.transform.scale.setFrom(lo.transform.scale);
					object.transform.buildMatrix();

					#if arm_physics
					object.addTrait(new armory.trait.physics.RigidBody(0, 0));
					#end

					UITrait.inst.selectObject(object);
				}
			}
			if (kb.started("m")) { // skip voxel
				UITrait.inst.selectedMaterialScene.data.raw.skip_context =
					UITrait.inst.selectedMaterialScene.data.raw.skip_context == '' ? 'voxel' : '';
			}
		}

		if (mouse.started("middle")) {
			#if arm_physics
			var physics = armory.trait.physics.PhysicsWorld.active;
			var rb = physics.pickClosest(mouse.x, mouse.y);
			if (rb != null) UITrait.inst.selectObject(rb.object);
			#end
		}

		if (mouse.started("left") && UITrait.inst.selectedObject.name != "Scene") {
			gizmo.transform.buildMatrix();
			var trs = [UITrait.inst.gizmoX.transform, UITrait.inst.gizmoY.transform, UITrait.inst.gizmoZ.transform];
			var hit = iron.math.RayCaster.closestBoxIntersect(trs, mouse.x, mouse.y, iron.Scene.active.camera);
			if (hit != null) {
				if (hit.object == UITrait.inst.gizmoX) UITrait.inst.axisX = true;
				else if (hit.object == UITrait.inst.gizmoY) UITrait.inst.axisY = true;
				else if (hit.object == UITrait.inst.gizmoZ) UITrait.inst.axisZ = true;
				if (UITrait.inst.axisX || UITrait.inst.axisY || UITrait.inst.axisZ) UITrait.inst.axisStart = 0.0;
			}
		}
		else if (mouse.released("left")) {
			UITrait.inst.axisX = UITrait.inst.axisY = UITrait.inst.axisZ = false;
		}

		if (UITrait.inst.axisX || UITrait.inst.axisY || UITrait.inst.axisZ) {
			var t = UITrait.inst.selectedObject.transform;
			var v = new iron.math.Vec4();
			v.set(t.worldx(), t.worldy(), t.worldz());
			
			if (UITrait.inst.axisX) {
				var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.yAxis(), v, mouse.x, mouse.y, iron.Scene.active.camera);
				if (hit != null) {
					if (UITrait.inst.axisStart == 0) UITrait.inst.axisStart = hit.x - UITrait.inst.selectedObject.transform.loc.x;
					UITrait.inst.selectedObject.transform.loc.x = hit.x - UITrait.inst.axisStart;
					UITrait.inst.selectedObject.transform.buildMatrix();
					#if arm_physics
					var rb = UITrait.inst.selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
					#end
				}
			}
			else if (UITrait.inst.axisY) {
				var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.xAxis(), v, mouse.x, mouse.y, iron.Scene.active.camera);
				if (hit != null) {
					if (UITrait.inst.axisStart == 0) UITrait.inst.axisStart = hit.y - UITrait.inst.selectedObject.transform.loc.y;
					UITrait.inst.selectedObject.transform.loc.y = hit.y - UITrait.inst.axisStart;
					UITrait.inst.selectedObject.transform.buildMatrix();
					#if arm_physics
					var rb = UITrait.inst.selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
					#end
				}
			}
			else if (UITrait.inst.axisZ) {
				var hit = iron.math.RayCaster.planeIntersect(iron.math.Vec4.xAxis(), v, mouse.x, mouse.y, iron.Scene.active.camera);
				if (hit != null) {
					if (UITrait.inst.axisStart == 0) UITrait.inst.axisStart = hit.z - UITrait.inst.selectedObject.transform.loc.z;
					UITrait.inst.selectedObject.transform.loc.z = hit.z - UITrait.inst.axisStart;
					UITrait.inst.selectedObject.transform.buildMatrix();
					#if arm_physics
					var rb = UITrait.inst.selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
					#end
				}
			}
		}

		iron.system.Input.occupied = (UITrait.inst.axisX || UITrait.inst.axisY || UITrait.inst.axisZ) && mouse.x < App.w();
	}
}
