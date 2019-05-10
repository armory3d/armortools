package arm.util;

import iron.math.Vec4;
import arm.ui.UITrait;

class ViewportUtil {
	
	public static function scaleToBounds() {
		var po = UITrait.inst.mergedObject == null ? UITrait.inst.mainObject() : UITrait.inst.mergedObject;
		var md = po.data;
		md.geom.calculateAABB();
		var r = Math.sqrt(md.geom.aabb.x * md.geom.aabb.x + md.geom.aabb.y * md.geom.aabb.y + md.geom.aabb.z * md.geom.aabb.z);
		po = UITrait.inst.mainObject();
		po.transform.dim.x = md.geom.aabb.x;
		po.transform.dim.y = md.geom.aabb.y;
		po.transform.dim.z = md.geom.aabb.z;
		po.transform.scale.set(2 / r, 2 / r, 2 / r);
		po.transform.buildMatrix();
	}

	public static function resetViewport() {
		var scene = iron.Scene.active;
		var cam = scene.camera;
		for (o in scene.raw.objects) {
			if (o.type == 'camera_object') {
				cam.transform.local.setF32(o.transform.values);
				cam.transform.decompose();
				if (UITrait.inst.fovHandle != null) UITrait.inst.fovHandle.value = 0.92;
				UITrait.inst.camHandle.position = 0;
				cam.data.raw.ortho = null;
				cam.buildProjection();
				UITrait.inst.ddirty = 2;
				arm.trait.OrbitCamera.inst.reset();
				break;
			}
		}
	}

	public static function setView(x:Float, y:Float, z:Float, rx:Float, ry:Float, rz:Float) {
		UITrait.inst.selectedObject.transform.reset();
		var scene = iron.Scene.active;
		var cam = scene.camera;
		var dist = cam.transform.loc.length();
		cam.transform.loc.set(x * dist, y * dist, z * dist);
		cam.transform.rot.fromEuler(rx, ry, rz);
		cam.transform.buildMatrix();
		cam.buildProjection();
		UITrait.inst.ddirty = 2;
		arm.trait.OrbitCamera.inst.reset();
	}

	public static function orbit(x:Float, y:Float) {
		var camera = iron.Scene.active.camera;
		var dist = arm.trait.OrbitCamera.dist;
		camera.transform.move(camera.lookWorld(), dist);
		camera.transform.rotate(new iron.math.Vec4(0, 0, 1), x);
		camera.transform.rotate(camera.rightWorld(), y);
		camera.transform.move(camera.lookWorld(), -dist);
		UITrait.inst.ddirty = 2;
	}

	public static function updateCameraType(cameraType:Int) {
		var cam = iron.Scene.active.cameras[0];
		var light = iron.Scene.active.lights[0];
		if (cameraType == 0) {
			cam.data.raw.ortho = null;
			light.visible = true;
		}
		else {
			var f32 = new kha.arrays.Float32Array(4);
			var f = cam.data.raw.fov * cam.transform.world.getLoc().length() / 2.5;
			f32[0] = -2 * f;
			f32[1] =  2 * f;
			f32[2] = -2 * f * (iron.App.h() / iron.App.w());
			f32[3] =  2 * f * (iron.App.h() / iron.App.w());
			cam.data.raw.ortho = f32;
			light.visible = false;
		}
		cam.buildProjection();
		UITrait.inst.ddirty = 2;
	}
}
