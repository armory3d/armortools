package arm.ui;

import zui.Id;

class TabProperties {

	static var row4 = [1 / 4, 1 / 4, 1 / 4, 1 / 4];

	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab1, tr("Properties"))) {
			if (Context.object != null) {

				var h = Id.handle();
				h.selected = Context.object.visible;
				Context.object.visible = ui.check(h, tr("Visible"));
				if (h.changed) Context.ddirty = 2;

				var loc = Context.object.transform.loc;
				var scale = Context.object.transform.scale;
				var rot = Context.object.transform.rot.getEuler();
				rot.mult(180 / 3.141592);
				var f = 0.0;
				ui.changed = false;

				ui.row(row4);
				ui.text(tr("Location"));

				h = Id.handle();
				h.text = roundfp(loc.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				if (h.changed) { loc.x = f; Context.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(loc.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { loc.y = f; Context.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(loc.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { loc.z = f; Context.ddirty = 2; }

				ui.row(row4);
				ui.text(tr("Rotation"));

				h = Id.handle();
				h.text = roundfp(rot.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				if (h.changed) { rot.x = f; Context.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(rot.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { rot.y = f; Context.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(rot.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { rot.z = f; Context.ddirty = 2; }

				if (ui.changed && Context.object.name != "Scene") {
					rot.mult(3.141592 / 180);
					Context.object.transform.rot.fromEuler(rot.x, rot.y, rot.z);
					Context.object.transform.buildMatrix();
				}

				ui.row(row4);
				ui.text(tr("Scale"));

				h = Id.handle();
				h.text = roundfp(scale.x) + "";
				f = Std.parseFloat(ui.textInput(h, "X"));
				if (h.changed) { scale.x = f; Context.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(scale.y) + "";
				f = Std.parseFloat(ui.textInput(h, "Y"));
				if (h.changed) { scale.y = f; Context.ddirty = 2; }

				h = Id.handle();
				h.text = roundfp(scale.z) + "";
				f = Std.parseFloat(ui.textInput(h, "Z"));
				if (h.changed) { scale.z = f; Context.ddirty = 2; }

				Context.object.transform.dirty = true;

				if (ui.changed && Context.object.name != "Scene") {
					#if arm_physics
					var pb = Context.object.getTrait(arm.plugin.PhysicsBody);
					if (pb != null) {
						pb.setLinearVelocity(0, 0, 0);
						pb.setAngularVelocity(0, 0, 0);
						pb.syncTransform();
					}
					#end
				}

				// if (Std.is(Context.object, LightObject)) {
				// 	var light = cast(Context.object, LightObject);
				// 	var lhandle = Id.handle();
				// 	lhandle.value = light.data.raw.strength / 1333;
				// 	lhandle.value = Std.int(lhandle.value * 100) / 100;
				// 	light.data.raw.strength = ui.slider(lhandle, "Strength", 0.0, 4.0, true) * 1333;
				// 	if (lhandle.changed) {
				// 		Context.ddirty = 2;
				// 	}
				// }
				// else if (Std.is(Context.object, CameraObject)) {
				// 	var scene = Scene.active;
				// 	var cam = scene.cameras[0];
				// 	var fovHandle = Id.handle({value: Std.int(cam.data.raw.fov * 100) / 100});
				// 	cam.data.raw.fov = ui.slider(fovHandle, "FoV", 0.3, 2.0, true);
				// 	if (fovHandle.changed) {
				// 		cam.buildProjection();
				// 		Context.ddirty = 2;
				// 	}
				// }
			}
		}
	}

	static function roundfp(f: Float, precision = 2): Float {
		f *= Math.pow(10, precision);
		return Math.round(f) / Math.pow(10, precision);
	}
}
