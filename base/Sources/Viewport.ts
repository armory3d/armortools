
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
		Vec4.set(po.base.transform.scale, 2 / r, 2 / r, 2 / r);
		Vec4.set(po.base.transform.loc, 0, 0, 0);
		Transform.buildMatrix(po.base.transform);
		for (let c of po.base.children) {
			Vec4.set(c.transform.loc, 0, 0, 0);
			Transform.buildMatrix(c.transform);
		}
	}

	static reset = () => {
		let cam = Scene.camera;
		for (let o of Scene.raw.objects) {
			if (o.type == "camera_object") {
				Mat4.setF32(cam.base.transform.local, o.transform.values);
				Transform.decompose(cam.base.transform);
				if (Context.raw.fovHandle != null) Context.raw.fovHandle.value = cam.data.fov = Base.defaultFov;
				Context.raw.camHandle.position = 0;
				cam.data.ortho = null;
				CameraObject.buildProjection(cam);
				Context.raw.ddirty = 2;
				Camera.reset();
				Transform.reset(Context.mainObject().base.transform);
				break;
			}
		}
	}

	static setView = (x: f32, y: f32, z: f32, rx: f32, ry: f32, rz: f32) => {
		Quat.set(Context.raw.paintObject.base.transform.rot, 0, 0, 0, 1);
		Context.raw.paintObject.base.transform.dirty = true;
		let cam = Scene.camera;
		let dist = Vec4.vec4_length(cam.base.transform.loc);
		Vec4.set(cam.base.transform.loc, x * dist, y * dist, z * dist);
		Quat.fromEuler(cam.base.transform.rot, rx, ry, rz);
		Transform.buildMatrix(cam.base.transform);
		CameraObject.buildProjection(cam);
		Context.raw.ddirty = 2;
		Camera.reset(Context.raw.viewIndexLast);
	}

	static orbit = (x: f32, y: f32) => {
		let cam = Scene.camera;
		let dist = Camera.distance();
		Transform.move(cam.base.transform, CameraObject.lookWorld(cam), dist);
		Transform.rotate(cam.base.transform, Vec4.create(0, 0, 1), x);
		Transform.rotate(cam.base.transform, CameraObject.rightWorld(cam), y);
		Transform.move(cam.base.transform, CameraObject.lookWorld(cam), -dist);
		Context.raw.ddirty = 2;
	}

	static orbitOpposite = () => {
		let cam = Scene.camera;
		let z = Math.abs(CameraObject.look(cam).z) - 1.0;
		(z < 0.0001 && z > -0.0001) ? Viewport.orbit(0, Math.PI) : Viewport.orbit(Math.PI, 0);
	}

	static zoom = (f: f32) => {
		let cam = Scene.camera;
		Transform.move(cam.base.transform, CameraObject.look(cam), f);
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
			let f = cam.data.fov * Vec4.vec4_length(Mat4.getLoc(cam.base.transform.world)) / 2.5;
			f32a[0] = -2 * f;
			f32a[1] =  2 * f;
			f32a[2] = -2 * f * (App.h() / App.w());
			f32a[3] =  2 * f * (App.h() / App.w());
			cam.data.ortho = f32a;
			light.base.visible = false;
		}
		CameraObject.buildProjection(cam);
		Context.raw.ddirty = 2;
	}
}
