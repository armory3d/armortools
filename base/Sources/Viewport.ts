
class Viewport {

	static scaleToBounds = () => {
		let po = Context.raw.mergedObject == null ? Context.mainObject() : Context.raw.mergedObject;
		let md = po.data;
		let aabb = MeshData.calculateAABB(md);
		let r = Math.sqrt(aabb.x * aabb.x + aabb.y * aabb.y + aabb.z * aabb.z);
		po = Context.mainObject();
		po.base.transform.dim.x = aabb.x;
		po.base.transform.dim.y = aabb.y;
		po.base.transform.dim.z = aabb.z;
		po.base.transform.scale.set(2 / r, 2 / r, 2 / r);
		po.base.transform.loc.set(0, 0, 0);
		po.base.transform.buildMatrix();
		for (let c of po.base.children) {
			c.transform.loc.set(0, 0, 0);
			c.transform.buildMatrix();
		}
	}

	static reset = () => {
		let cam = Scene.camera;
		for (let o of Scene.raw.objects) {
			if (o.type == "camera_object") {
				cam.base.transform.local.setF32(o.transform.values);
				cam.base.transform.decompose();
				if (Context.raw.fovHandle != null) Context.raw.fovHandle.value = cam.data.fov = Base.defaultFov;
				Context.raw.camHandle.position = 0;
				cam.data.ortho = null;
				cam.buildProjection();
				Context.raw.ddirty = 2;
				Camera.reset();
				Context.mainObject().base.transform.reset();
				break;
			}
		}
	}

	static setView = (x: f32, y: f32, z: f32, rx: f32, ry: f32, rz: f32) => {
		Context.raw.paintObject.base.transform.rot.set(0, 0, 0, 1);
		Context.raw.paintObject.base.transform.dirty = true;
		let cam = Scene.camera;
		let dist = cam.base.transform.loc.length();
		cam.base.transform.loc.set(x * dist, y * dist, z * dist);
		cam.base.transform.rot.fromEuler(rx, ry, rz);
		cam.base.transform.buildMatrix();
		cam.buildProjection();
		Context.raw.ddirty = 2;
		Camera.reset(Context.raw.viewIndexLast);
	}

	static orbit = (x: f32, y: f32) => {
		let cam = Scene.camera;
		let dist = Camera.distance();
		cam.base.transform.move(cam.lookWorld(), dist);
		cam.base.transform.rotate(new Vec4(0, 0, 1), x);
		cam.base.transform.rotate(cam.rightWorld(), y);
		cam.base.transform.move(cam.lookWorld(), -dist);
		Context.raw.ddirty = 2;
	}

	static orbitOpposite = () => {
		let cam = Scene.camera;
		let z = Math.abs(cam.look().z) - 1.0;
		(z < 0.0001 && z > -0.0001) ? Viewport.orbit(0, Math.PI) : Viewport.orbit(Math.PI, 0);
	}

	static zoom = (f: f32) => {
		let cam = Scene.camera;
		cam.base.transform.move(cam.look(), f);
		Context.raw.ddirty = 2;
	}

	static updateCameraType = (cameraType: i32) => {
		let cam = Scene.cameras[0];
		let light = Scene.lights[0];
		if (cameraType == CameraType.CameraPerspective) {
			cam.data.ortho = null;
			light.base.visible = true;
		}
		else {
			let f32a = new Float32Array(4);
			let f = cam.data.fov * cam.base.transform.world.getLoc().length() / 2.5;
			f32a[0] = -2 * f;
			f32a[1] =  2 * f;
			f32a[2] = -2 * f * (App.h() / App.w());
			f32a[3] =  2 * f * (App.h() / App.w());
			cam.data.ortho = f32a;
			light.base.visible = false;
		}
		cam.buildProjection();
		Context.raw.ddirty = 2;
	}
}
