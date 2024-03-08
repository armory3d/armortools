
class Viewport {

	static scale_to_bounds = () => {
		let po: mesh_object_t = Context.raw.merged_object == null ? Context.main_object() : Context.raw.merged_object;
		let md: mesh_data_t = po.data;
		let aabb: vec4_t = mesh_data_calculate_aabb(md);
		let r: f32 = Math.sqrt(aabb.x * aabb.x + aabb.y * aabb.y + aabb.z * aabb.z);
		po = Context.main_object();
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
		let cam: camera_object_t = scene_camera;
		for (let o of _scene_raw.objects) {
			if (o.type == "camera_object") {
				mat4_set_from_f32_array(cam.base.transform.local, o.transform);
				transform_decompose(cam.base.transform);
				if (Context.raw.fov_handle != null) Context.raw.fov_handle.value = cam.data.fov = base_default_fov;
				Context.raw.cam_handle.position = 0;
				cam.data.ortho = null;
				camera_object_build_proj(cam);
				Context.raw.ddirty = 2;
				Camera.reset();
				transform_reset(Context.main_object().base.transform);
				break;
			}
		}
	}

	static set_view = (x: f32, y: f32, z: f32, rx: f32, ry: f32, rz: f32) => {
		quat_set(Context.raw.paint_object.base.transform.rot, 0, 0, 0, 1);
		Context.raw.paint_object.base.transform.dirty = true;
		let cam: camera_object_t = scene_camera;
		let dist: f32 = vec4_len(cam.base.transform.loc);
		vec4_set(cam.base.transform.loc, x * dist, y * dist, z * dist);
		quat_from_euler(cam.base.transform.rot, rx, ry, rz);
		transform_build_matrix(cam.base.transform);
		camera_object_build_proj(cam);
		Context.raw.ddirty = 2;
		Camera.reset(Context.raw.view_index_last);
	}

	static orbit = (x: f32, y: f32) => {
		let cam: camera_object_t = scene_camera;
		let dist: f32 = Camera.distance();
		transform_move(cam.base.transform, camera_object_look_world(cam), dist);
		transform_rotate(cam.base.transform, vec4_create(0, 0, 1), x);
		transform_rotate(cam.base.transform, camera_object_right_world(cam), y);
		transform_move(cam.base.transform, camera_object_look_world(cam), -dist);
		Context.raw.ddirty = 2;
	}

	static orbit_opposite = () => {
		let cam: camera_object_t = scene_camera;
		let z: f32 = Math.abs(camera_object_look(cam).z) - 1.0;
		(z < 0.0001 && z > -0.0001) ? Viewport.orbit(0, Math.PI) : Viewport.orbit(Math.PI, 0);
	}

	static zoom = (f: f32) => {
		let cam: camera_object_t = scene_camera;
		transform_move(cam.base.transform, camera_object_look(cam), f);
		Context.raw.ddirty = 2;
	}

	static update_camera_type = (cameraType: i32) => {
		let cam: camera_object_t = scene_cameras[0];
		let light: light_object_t = scene_lights[0];
		if (cameraType == camera_type_t.PERSPECTIVE) {
			cam.data.ortho = null;
			light.base.visible = true;
		}
		else {
			let f32a: Float32Array = new Float32Array(4);
			let f: f32 = cam.data.fov * vec4_len(mat4_get_loc(cam.base.transform.world)) / 2.5;
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
