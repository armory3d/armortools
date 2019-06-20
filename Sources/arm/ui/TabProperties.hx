package arm.ui;

import zui.Id;
import iron.object.LightObject;
import iron.object.CameraObject;
import iron.Scene;

class TabProperties {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab1, 'Properties')) {
			if (UITrait.inst.selectedObject != null) {

				var h = Id.handle();
				h.selected = UITrait.inst.selectedObject.visible;
				UITrait.inst.selectedObject.visible = ui.check(h, "Visible");
				if (h.changed) UITrait.inst.ddirty = 2;

				var loc = UITrait.inst.selectedObject.transform.loc;
				var scale = UITrait.inst.selectedObject.transform.scale;
				var rot = UITrait.inst.selectedObject.transform.rot.getEuler();
				rot.mult(180 / 3.141592);
				var f = 0.0;

				ui.row(UITrait.inst.row4);
				ui.text("Location");

				h = Id.handle();
				h.text = roundfp(loc.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				if (h.changed) { loc.x = f; UITrait.inst.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(loc.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { loc.y = f; UITrait.inst.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(loc.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { loc.z = f; UITrait.inst.ddirty = 2; }

				ui.row(UITrait.inst.row4);
				ui.text("Rotation");
				
				h = Id.handle();
				h.text = roundfp(rot.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				var changed = false;
				if (h.changed) { changed = true; rot.x = f; UITrait.inst.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(rot.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { changed = true; rot.y = f; UITrait.inst.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(rot.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { changed = true; rot.z = f; UITrait.inst.ddirty = 2; }

				if (changed && UITrait.inst.selectedObject.name != "Scene") {
					rot.mult(3.141592 / 180);
					UITrait.inst.selectedObject.transform.rot.fromEuler(rot.x, rot.y, rot.z);
					UITrait.inst.selectedObject.transform.buildMatrix();
					#if arm_physics
					var rb = UITrait.inst.selectedObject.getTrait(armory.trait.physics.RigidBody);
					if (rb != null) rb.syncTransform();
					#end
				}

				ui.row(UITrait.inst.row4);
				ui.text("Scale");
				
				h = Id.handle();
				h.text = roundfp(scale.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				if (h.changed) { scale.x = f; UITrait.inst.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(scale.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { scale.y = f; UITrait.inst.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(scale.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { scale.z = f; UITrait.inst.ddirty = 2; }

				UITrait.inst.selectedObject.transform.dirty = true;

				if (UITrait.inst.selectedObject.name == "Scene") {
					var p = Scene.active.world.probe;
					var envHandle = Id.handle({value: p.raw.strength});
					p.raw.strength = ui.slider(envHandle, "Strength", 0.0, 5.0, true);
					if (envHandle.changed) {
						UITrait.inst.ddirty = 2;
					}
				}
				else if (Std.is(UITrait.inst.selectedObject, LightObject)) {
					var light = cast(UITrait.inst.selectedObject, LightObject);
					var lhandle = Id.handle();
					lhandle.value = light.data.raw.strength / 1333;
					lhandle.value = Std.int(lhandle.value * 100) / 100;
					light.data.raw.strength = ui.slider(lhandle, "Strength", 0.0, 4.0, true) * 1333;
					if (lhandle.changed) {
						UITrait.inst.ddirty = 2;
					}
				}
				else if (Std.is(UITrait.inst.selectedObject, CameraObject)) {
					var scene = Scene.active;
					var cam = scene.cameras[0];
					var fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
					cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
					if (fovHandle.changed) {
						cam.buildProjection();
						UITrait.inst.ddirty = 2;
					}
				}
			}
		}
	}

	static function roundfp(f:Float, precision = 2):Float {
		f *= Math.pow(10, precision);
		return Math.round(f) / Math.pow(10, precision);
	}
}
