
type light_object_t = {
	base?: object_t;
	data?: light_data_t;
	v?: mat4_t;
};

function light_object_create(data: light_data_t): light_object_t {
	let raw: light_object_t = {};
	raw.v = mat4_identity();
	raw.base = object_create(false);
	raw.base.ext = raw;
	raw.base.ext_type = "light_object_t";
	raw.data = data;

	array_push(scene_lights, raw);
	return raw;
}

function light_object_remove(raw: light_object_t) {
	array_remove(scene_lights, raw);
	if (_render_path_light == raw) {
		_render_path_light = null;
	}

	object_remove_super(raw.base);
}

function light_object_build_mat(raw: light_object_t, camera: camera_object_t) {
	transform_build_matrix(raw.base.transform);
}

function light_object_right(raw: light_object_t): vec4_t {
	return vec4_create(raw.v.m00, raw.v.m10, raw.v.m20);
}

function light_object_up(raw: light_object_t): vec4_t {
	return vec4_create(raw.v.m01, raw.v.m11, raw.v.m21);
}

function light_object_look(raw: light_object_t): vec4_t {
	return vec4_create(raw.v.m02, raw.v.m12, raw.v.m22);
}
