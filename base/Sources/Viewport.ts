
class Viewport {

	static scaleToBounds = () => {
		let po = Context.raw.mergedObject == null ? Context.mainObject() : Context.raw.mergedObject;
		let md = po.data;
		let aabb = mesh_data_calculate_aabb(md);
		let r = Math.sqrt(aabb.x * aabb.x + aabb.y * aabb.y + aabb.z * aabb.z);
		po = Context.mainObject();
		po.base.transform.dim.x = aabb.x;
		po.base.transform.dim.y = aabb.y;
		po.base.transform.dim.z = aabb.z;
		vec4_set(po.base.transform.scale, 2 / r, 2 / r, 2 / r);
		vec4_set(po.base.transform.loc, 0, 0, 0);
		transform_build_matrix(po.base.transform);
		for (let c of po.base.children) {
			vec4_set(c.transform.loc, 0, 0, 0);
			transform_build_matrix(c.transform);
		}
	}

	static reset = () => {
		let cam = scene_camera;
		for (let o of _scene_raw.objects) {
			if (o.type == "camera_object") {
				mat4_set_from_f32_array(cam.base.transform.local, o.transform.values);
				transform_decompose(cam.base.transform);
				if (Context.raw.fovHandle != null) Context.raw.fovHandle.value = cam.data.fov = Base.defaultFov;
				Context.raw.camHandle.position = 0;
				cam.data.ortho = null;
				camera_object_build_proj(cam);
				Context.raw.ddirty = 2;
				Camera.reset();
				transform_reset(Context.mainObject().base.transform);
				break;
			}
		}
	}

	static setView = (x: f32, y: f32, z: f32, rx: f32, ry: f32, rz: f32) => {
		quat_set(Context.raw.paintObject.base.transform.rot, 0, 0, 0, 1);
		Context.raw.paintObject.base.transform.dirty = true;
		let cam = scene_camera;
		let dist = vec4_len(cam.base.transform.loc);
		vec4_set(cam.base.transform.loc, x * dist, y * dist, z * dist);
		quat_from_euler(cam.base.transform.rot, rx, ry, rz);
		transform_build_matrix(cam.base.transform);
		camera_object_build_proj(cam);
		Context.raw.ddirty = 2;
		Camera.reset(Context.raw.viewIndexLast);
	}

	static orbit = (x: f32, y: f32) => {
		let cam = scene_camera;
		let dist = Camera.distance();
		transform_move(cam.base.transform, camera_object_look_world(cam), dist);
		transform_rotate(cam.base.transform, vec4_create(0, 0, 1), x);
		transform_rotate(cam.base.transform, camera_object_right_world(cam), y);
		transform_move(cam.base.transform, camera_object_look_world(cam), -dist);
		Context.raw.ddirty = 2;
	}

	static orbitOpposite = () => {
		let cam = scene_camera;
		let z = Math.abs(camera_object_look(cam).z) - 1.0;
		(z < 0.0001 && z > -0.0001) ? Viewport.orbit(0, Math.PI) : Viewport.orbit(Math.PI, 0);
	}

	static zoom = (f: f32) => {
		let cam = scene_camera;
		transform_move(cam.base.transform, camera_object_look(cam), f);
		Context.raw.ddirty = 2;
	}

	static updateCameraType = (cameraType: i32) => {
		let cam = scene_cameras[0];
		let light = scene_lights[0];
		if (cameraType == CameraType.CameraPerspective) {
			cam.data.ortho = null;
			light.base.visible = true;
		}
		else {
			let f32a = new Float32Array(4);
			let f = cam.data.fov * vec4_len(mat4_get_loc(cam.base.transform.world)) / 2.5;
			f32a[0] = -2 * f;
			f32a[1] =  2 * f;
			f32a[2] = -2 * f * (app_h() / app_w());
			f32a[3] =  2 * f * (app_h() / app_w());
			cam.data.ortho = f32a;
			light.base.visible = false;
		}
		camera_object_build_proj(cam);
		Context.raw.ddirty = 2;
	}
}
