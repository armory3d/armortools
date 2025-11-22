
function viewport_scale_to_bounds(bounds: f32 = 2.0) {
	let po: mesh_object_t   = context_raw.merged_object == null ? context_main_object() : context_raw.merged_object;
	let md: mesh_data_t     = po.data;
	let aabb: vec4_t        = mesh_data_calculate_aabb(md);
	let r: f32              = math_sqrt(aabb.x * aabb.x + aabb.y * aabb.y + aabb.z * aabb.z);
	po                      = context_main_object();
	po.base.transform.dim.x = aabb.x;
	po.base.transform.dim.y = aabb.y;
	po.base.transform.dim.z = aabb.z;
	po.base.transform.scale = vec4_create(bounds / r, bounds / r, bounds / r);
	po.base.transform.loc   = vec4_create(0, 0, 0);
	transform_build_matrix(po.base.transform);
	for (let i: i32 = 0; i < po.base.children.length; ++i) {
		let c: object_t = po.base.children[i];
		c.transform.loc = vec4_create(0, 0, 0);
		transform_build_matrix(c.transform);
	}
}

function viewport_reset() {
	let cam: camera_object_t = scene_camera;
	for (let i: i32 = 0; i < _scene_raw.objects.length; ++i) {
		let o: obj_t = _scene_raw.objects[i];
		if (o.type == "camera_object") {
			cam.base.transform.local = mat4_from_f32_array(o.transform);
			transform_decompose(cam.base.transform);
			if (context_raw.fov_handle != null) {
				context_raw.fov_handle.f = cam.data.fov = base_default_fov;
			}
			context_raw.cam_handle.i = 0;
			cam.data.ortho           = null;
			camera_object_build_proj(cam);
			context_raw.ddirty = 2;
			camera_reset();
			transform_reset(context_main_object().base.transform);
			break;
		}
	}
}

function viewport_set_view(x: f32, y: f32, z: f32, rx: f32, ry: f32, rz: f32) {
	context_raw.paint_object.base.transform.rot   = quat_create(0, 0, 0, 1);
	context_raw.paint_object.base.transform.dirty = true;
	let cam: camera_object_t                      = scene_camera;
	let dist: f32                                 = vec4_len(cam.base.transform.loc);
	cam.base.transform.loc                        = vec4_create(x * dist, y * dist, z * dist);
	cam.base.transform.rot                        = quat_from_euler(rx, ry, rz);
	transform_build_matrix(cam.base.transform);
	camera_object_build_proj(cam);
	context_raw.ddirty = 2;
	camera_reset(context_raw.view_index_last);
}

function viewport_orbit(x: f32, y: f32) {
	let cam: camera_object_t = scene_camera;
	let dist: f32            = camera_distance();
	transform_move(cam.base.transform, camera_object_look_world(cam), dist);
	transform_rotate(cam.base.transform, vec4_create(0, 0, 1), x);
	transform_rotate(cam.base.transform, camera_object_right_world(cam), y);
	transform_move(cam.base.transform, camera_object_look_world(cam), -dist);
	context_raw.ddirty = 2;
}

function viewport_orbit_opposite() {
	let cam: camera_object_t = scene_camera;
	let look: vec4_t         = camera_object_look(cam);
	let z: f32               = math_abs(look.z) - 1.0;
    (z < 0.0001 && z > -0.0001) ? viewport_orbit(0, math_pi()) : viewport_orbit(math_pi(), 0);
}

function viewport_zoom(f: f32) {
	let cam: camera_object_t = scene_camera;
	transform_move(cam.base.transform, camera_object_look(cam), f);
	context_raw.ddirty = 2;
}

function viewport_update_camera_type(camera_type: i32) {
	let cam: camera_object_t = scene_cameras[0];
	if (camera_type == camera_type_t.PERSPECTIVE) {
		cam.data.ortho = null;
	}
	else {
		let f32a: f32_array_t = f32_array_create(4);
		let f: f32            = cam.data.fov * vec4_len(mat4_get_loc(cam.base.transform.world)) / 2.5;
		f32a[0]               = -2 * f;
		f32a[1]               = 2 * f;
		f32a[2]               = -2 * f * (sys_h() / sys_w());
		f32a[3]               = 2 * f * (sys_h() / sys_w());
		cam.data.ortho        = f32a;
	}
	camera_object_build_proj(cam);
	context_raw.ddirty = 2;
}

function viewport_capture_screenshot() {
	let rt: render_target_t       = map_get(render_path_render_targets, "last");
	let tex: gpu_texture_t        = rt._image;
	let screenshot: gpu_texture_t = gpu_create_render_target(512, 512);
	let r: f32                    = sys_w() / sys_h();
	draw_begin(screenshot);
	draw_scaled_image(tex, -(512 * r - 512) / 2, 0, 512 * r, 512);
	draw_end();

	if (project_raw.packed_assets == null) {
		project_raw.packed_assets = [];
	}

	let num: i32    = 0;
	let abs: string = "/packed/screenshot0.png";
	for (let i: i32 = 0; i < project_raw.packed_assets.length; ++i) {
		let pa: packed_asset_t = project_raw.packed_assets[i];
		if (pa.name == abs) {
			i = 0;
			num++;
			abs = "/packed/screenshot" + num + ".png";
		}
	}

	let pa: packed_asset_t = {name : abs, bytes : iron_encode_png(gpu_get_texture_pixels(screenshot), screenshot.width, screenshot.height, 0)};
	array_push(project_raw.packed_assets, pa);
	map_set(data_cached_images, abs, screenshot);
	import_texture_run(abs);
}
