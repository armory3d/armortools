package arm.util;

import kha.arrays.Float32Array;
import iron.Scene;
import iron.math.Vec4;
import arm.ui.UISidebar;
import arm.plugin.Camera;
import arm.Enums;

class ViewportUtil {

	public static function scaleToBounds() {
		var po = Context.mergedObject == null ? Context.mainObject() : Context.mergedObject;
		var md = po.data;
		md.geom.calculateAABB();
		var r = Math.sqrt(md.geom.aabb.x * md.geom.aabb.x + md.geom.aabb.y * md.geom.aabb.y + md.geom.aabb.z * md.geom.aabb.z);
		po = Context.mainObject();
		po.transform.dim.x = md.geom.aabb.x;
		po.transform.dim.y = md.geom.aabb.y;
		po.transform.dim.z = md.geom.aabb.z;
		po.transform.scale.set(2 / r, 2 / r, 2 / r);
		po.transform.loc.set(0, 0, 0);
		po.transform.buildMatrix();
		for (c in po.children) {
			c.transform.loc.set(0, 0, 0);
			c.transform.buildMatrix();
		}
	}

	public static function resetViewport() {
		var cam = Scene.active.camera;
		for (o in Scene.active.raw.objects) {
			if (o.type == "camera_object") {
				cam.transform.local.setF32(o.transform.values);
				cam.transform.decompose();
				if (Context.fovHandle != null) Context.fovHandle.value = cam.data.raw.fov = 0.92;
				Context.camHandle.position = 0;
				cam.data.raw.ortho = null;
				cam.buildProjection();
				Context.ddirty = 2;
				Camera.inst.reset();
				break;
			}
		}
	}

	public static function setView(x: Float, y: Float, z: Float, rx: Float, ry: Float, rz: Float) {
		Context.paintObject.transform.rot.set(0, 0, 0, 1);
		Context.paintObject.transform.dirty = true;
		var cam = Scene.active.camera;
		var dist = cam.transform.loc.length();
		cam.transform.loc.set(x * dist, y * dist, z * dist);
		cam.transform.rot.fromEuler(rx, ry, rz);
		cam.transform.buildMatrix();
		cam.buildProjection();
		Context.ddirty = 2;
		Camera.inst.reset(Context.viewIndexLast);
	}

	public static function orbit(x: Float, y: Float) {
		var cam = Scene.active.camera;
		var dist = Camera.inst.distance();
		cam.transform.move(cam.lookWorld(), dist);
		cam.transform.rotate(new Vec4(0, 0, 1), x);
		cam.transform.rotate(cam.rightWorld(), y);
		cam.transform.move(cam.lookWorld(), -dist);
		Context.ddirty = 2;
	}

	public static function orbitOpposite() {
		var cam = Scene.active.camera;
		var z = Math.abs(cam.look().z) - 1.0;
		(z < 0.0001 && z > -0.0001) ? ViewportUtil.orbit(0, Math.PI) : ViewportUtil.orbit(Math.PI, 0);
	}

	public static function zoom(f: Float) {
		var cam = Scene.active.camera;
		cam.transform.move(cam.look(), f);
		Context.ddirty = 2;
	}

	public static function updateCameraType(cameraType: Int) {
		var cam = Scene.active.cameras[0];
		var light = Scene.active.lights[0];
		if (cameraType == CameraPerspective) {
			cam.data.raw.ortho = null;
			light.visible = true;
		}
		else {
			var f32 = new Float32Array(4);
			var f = cam.data.raw.fov * cam.transform.world.getLoc().length() / 2.5;
			f32[0] = -2 * f;
			f32[1] =  2 * f;
			f32[2] = -2 * f * (iron.App.h() / iron.App.w());
			f32[3] =  2 * f * (iron.App.h() / iron.App.w());
			cam.data.raw.ortho = f32;
			light.visible = false;
		}
		cam.buildProjection();
		Context.ddirty = 2;
	}
}
