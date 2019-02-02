package arm;

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
		var cam = scene.cameras[0];
		for (o in scene.raw.objects) {
			if (o.type == 'camera_object') {
				cam.transform.local.setF32(o.transform.values);
				cam.transform.decompose();
				if (UITrait.inst.fovHandle != null) UITrait.inst.fovHandle.value = 0.92;
				UITrait.inst.camHandle.position = 0;
				cam.data.raw.ortho = null;
				if (UITrait.inst.originalShadowBias > 0) {
					iron.Scene.active.lights[0].data.raw.shadows_bias = UITrait.inst.originalShadowBias;
				}
				cam.buildProjection();
				UITrait.inst.selectedObject.transform.reset();
				UITrait.inst.ddirty = 2;
				break;
			}
		}
	}
}
