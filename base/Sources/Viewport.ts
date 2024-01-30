
class Viewport {

	static scaleToBounds = () => {
		let po = Context.raw.mergedObject == null ? Context.mainObject() : Context.raw.mergedObject;
		let md = po.data;
		let aabb = MeshData.calculateAABB(md);
		let r = Math.sqrt(aabb.x * aabb.x + aabb.y * aabb.y + aabb.z * aabb.z);
		po = Context.mainObject();
		po.transform.dim.x = aabb.x;
		po.transform.dim.y = aabb.y;
		po.transform.dim.z = aabb.z;
		po.transform.scale.set(2 / r, 2 / r, 2 / r);
		po.transform.loc.set(0, 0, 0);
		po.transform.buildMatrix();
		for (let c of po.children) {
			c.transform.loc.set(0, 0, 0);
			c.transform.buildMatrix();
		}
	}

	static reset = () => {
		let cam = Scene.camera;
		for (let o of Scene.raw.objects) {
			if (o.type == "camera_object") {
				cam.transform.local.setF32(o.transform.values);
				cam.transform.decompose();
				if (Context.raw.fovHandle != null) Context.raw.fovHandle.value = cam.data.fov = Base.defaultFov;
				Context.raw.camHandle.position = 0;
				cam.data.ortho = null;
				cam.buildProjection();
				Context.raw.ddirty = 2;
				Camera.reset();
				Context.mainObject().transform.reset();
				break;
			}
		}
	}

	static setView = (x: f32, y: f32, z: f32, rx: f32, ry: f32, rz: f32) => {
		Context.raw.paintObject.transform.rot.set(0, 0, 0, 1);
		Context.raw.paintObject.transform.dirty = true;
		let cam = Scene.camera;
		let dist = cam.transform.loc.length();
		cam.transform.loc.set(x * dist, y * dist, z * dist);
		cam.transform.rot.fromEuler(rx, ry, rz);
		cam.transform.buildMatrix();
		cam.buildProjection();
		Context.raw.ddirty = 2;
		Camera.reset(Context.raw.viewIndexLast);
	}

	static orbit = (x: f32, y: f32) => {
		let cam = Scene.camera;
		let dist = Camera.distance();
		cam.transform.move(cam.lookWorld(), dist);
		cam.transform.rotate(new Vec4(0, 0, 1), x);
		cam.transform.rotate(cam.rightWorld(), y);
		cam.transform.move(cam.lookWorld(), -dist);
		Context.raw.ddirty = 2;
	}

	static orbitOpposite = () => {
		let cam = Scene.camera;
		let z = Math.abs(cam.look().z) - 1.0;
		(z < 0.0001 && z > -0.0001) ? Viewport.orbit(0, Math.PI) : Viewport.orbit(Math.PI, 0);
	}

	static zoom = (f: f32) => {
		let cam = Scene.camera;
		cam.transform.move(cam.look(), f);
		Context.raw.ddirty = 2;
	}

	static updateCameraType = (cameraType: i32) => {
		let cam = Scene.cameras[0];
		let light = Scene.lights[0];
		if (cameraType == CameraType.CameraPerspective) {
			cam.data.ortho = null;
			light.visible = true;
		}
		else {
			let f32a = new Float32Array(4);
			let f = cam.data.fov * cam.transform.world.getLoc().length() / 2.5;
			f32a[0] = -2 * f;
			f32a[1] =  2 * f;
			f32a[2] = -2 * f * (App.h() / App.w());
			f32a[3] =  2 * f * (App.h() / App.w());
			cam.data.ortho = f32a;
			light.visible = false;
		}
		cam.buildProjection();
		Context.raw.ddirty = 2;
	}
}
