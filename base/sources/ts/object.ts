
type object_t = {
	uid?: i32;
	urandom?: f32;
	raw?: obj_t;
	name?: string;
	transform?: transform_t;
	parent?: object_t;
	children?: object_t[];
	visible?: bool; // Skip render, keep updating
	culled?: bool;  // base_object_t was culled last frame
	is_empty?: bool;
	ext?: any; // mesh_object_t | camera_object_t
	ext_type?: string;
};

let _object_uid_counter: i32 = 0;

function object_create(is_empty: bool = true): object_t {
	let raw: object_t = {};
	raw.name          = "";
	raw.children      = [];
	raw.visible       = true;
	raw.culled        = false;
	raw.uid           = _object_uid_counter++;
	raw.transform     = transform_create(raw);
	raw.is_empty      = is_empty;
	if (raw.is_empty) {
		array_push(scene_empties, raw);
	}
	return raw;
}

function object_set_parent(raw: object_t, parent_object: object_t) {
	if (parent_object == raw || parent_object == raw.parent) {
		return;
	}

	if (raw.parent != null) {
		array_remove(raw.parent.children, raw);
		raw.parent = null; // rebuild matrix without a parent
		transform_build_matrix(raw.transform);
	}

	if (parent_object == null) {
		parent_object = _scene_scene_parent;
	}
	raw.parent = parent_object;
	array_push(raw.parent.children, raw);
}

function object_remove_super(raw: object_t) {
	if (raw.is_empty) {
		array_remove(scene_empties, raw);
	}
	while (raw.children.length > 0) {
		object_remove(raw.children[0]);
	}
	if (raw.parent != null) {
		array_remove(raw.parent.children, raw);
		raw.parent = null;
	}
}

function object_remove(raw: object_t) {
	if (raw.ext_type == "mesh_object_t") {
		mesh_object_remove(raw.ext);
	}
	else if (raw.ext_type == "camera_object_t") {
		camera_object_remove(raw.ext);
	}
	else {
		object_remove_super(raw);
	}
}

function object_get_child(raw: object_t, name: string): object_t {
	if (raw.name == name) {
		return raw;
	}
	else {
		for (let i: i32 = 0; i < raw.children.length; ++i) {
			let c: object_t = raw.children[i];
			let r: object_t = object_get_child(c, name);
			if (r != null) {
				return r;
			}
		}
	}
	return null;
}
