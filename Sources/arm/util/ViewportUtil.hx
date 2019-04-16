package arm.util;

import iron.math.Mat4;
import iron.math.Vec4;
import iron.math.Quat;
import arm.ui.*;

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
				UITrait.inst.selectedObject.transform.reset();
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
}
