package arm;

import iron.math.Mat4;
import iron.math.Vec4;
import iron.math.Quat;

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
				// if (UITrait.inst.originalShadowBias > 0) {
					// iron.Scene.active.lights[0].data.raw.shadows_bias = UITrait.inst.originalShadowBias;
				// }
				cam.buildProjection();
				UITrait.inst.selectedObject.transform.reset();
				UITrait.inst.ddirty = 2;
				break;
			}
		}
	}

	public static function setView(x:Float, y:Float, z:Float, rx:Float, ry:Float, rz:Float) {
		UITrait.inst.selectedObject.transform.reset();
		var scene = iron.Scene.active;
		var cam = scene.camera;
		cam.transform.loc.set(x, y, z);
		cam.transform.rot.fromEuler(rx, ry, rz);
		cam.transform.buildMatrix();
		cam.buildProjection();
		UITrait.inst.ddirty = 2;
	}

	public static inline function ortho(left:kha.FastFloat, right:kha.FastFloat, bottom:kha.FastFloat, top:kha.FastFloat, near:kha.FastFloat, far:kha.FastFloat):Mat4 {
		// For runtime shader, D3D NDC is not matched to OGL yet
		// Build proper ortho matrix
		var rl = right - left;
		var tb = top - bottom;
		var tx = -(right + left) / (rl);
		var ty = -(top + bottom) / (tb);

		#if (kha_opengl || kha_webgl)
		var fn = far - near;
		var tz = -(far + near) / (fn);
		var w = -2 / fn;
		#else
		var nf = near - far;
		var tz = -(near) / (nf);
		var w = 1 / nf;
		#end

		return new Mat4(
			2 / rl,	0,		0, tx,
			0,		2 / tb,	0, ty,
			0,		0,		w, tz,
			0,		0,		0, 1
		);
	}
}
