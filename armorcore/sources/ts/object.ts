
type object_t = {
	uid?: i32;
	urandom?: f32;
	raw?: obj_t;
	name?: string;
	transform?: transform_t;
	parent?: object_t;
	children?: object_t[];
	///if arm_anim
	animation?: anim_raw_t;
	///end
	visible?: bool; // Skip render, keep updating
	culled?: bool; // base_object_t was culled last frame
	is_empty?: bool;
	ext?: any; // mesh_object_t | camera_object_t | speaker_object_t
	ext_type?: string;
};

let _object_uid_counter: i32 = 0;

function object_create(is_empty: bool = true): object_t {
	let raw: object_t = {};
	raw.name = "";
	raw.children = [];
	raw.visible = true;
	raw.culled = false;
	raw.uid = _object_uid_counter++;
	raw.transform = transform_create(raw);
	raw.is_empty = is_empty;
	if (raw.is_empty && _scene_ready) {
		array_push(scene_empties, raw);
	}
	return raw;
}

function object_set_parent(raw: object_t, parent_object: object_t, parent_inv: bool = false, keep_transform: bool = false) {
	if (parent_object == raw || parent_object == raw.parent) {
		return;
	}

	if (raw.parent != null) {
		array_remove(raw.parent.children, raw);
		if (keep_transform) {
			transform_apply_parent(raw.transform);
		}
		raw.parent = null; // rebuild matrix without a parent
		transform_build_matrix(raw.transform);
	}

	if (parent_object == null) {
		parent_object = _scene_scene_parent;
	}
	raw.parent = parent_object;
	array_push(raw.parent.children, raw);
	if (parent_inv) {
		transform_apply_parent_inv(raw.transform);
	}
}

function object_remove_super(raw: object_t) {
	if (raw.is_empty && _scene_ready) {
		array_remove(scene_empties, raw);
	}
	///if arm_anim
	if (raw.animation != null) {
		anim_remove(raw.animation);
	}
	///end
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
	///if arm_audio
	else if (raw.ext_type == "speaker_object_t") {
		speaker_object_remove(raw.ext);
	}
	///end
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

function object_get_children(raw: object_t, recursive: bool = false): object_t[] {
	if (!recursive) {
		return raw.children;
	}

	let ret_children: object_t[] = array_slice(raw.children, 0, raw.children.length);
	for (let i: i32 = 0; i < raw.children.length; ++i) {
		let c: object_t = raw.children[i];
		ret_children = array_concat(ret_children, object_get_children(c, recursive));
	}
	return ret_children;
}

///if arm_anim

function object_setup_animation_super(raw: object_t, oactions: scene_t[] = null) {
	// Parented to bone
	///if arm_skin
	if (raw.raw.anim != null && raw.raw.anim.parent_bone != null) {
		app_notify_on_init(_object_setup_animation_on_init, raw);
	}
	///end
	// object_t actions
	if (oactions == null) {
		return;
	}
	raw.animation = anim_object_create(raw, oactions).base;
}

///if arm_skin
function _object_setup_animation_on_init(raw: object_t) {
	let banim: anim_bone_t = object_get_parent_armature(raw, raw.parent.name);
	if (banim != null) {
		anim_bone_add_bone_child(banim, raw.raw.anim.parent_bone, raw);
	}
}
function object_get_parent_armature(raw: object_t, name: string): anim_bone_t {
	for (let i: i32 = 0; i < scene_animations.length; ++i) {
		let a: anim_raw_t = scene_animations[i];
		if (a.armature != null && a.armature.name == name) {
			return a.ext;
		}
	}
	return null;
}
///end

function object_setup_animation(raw: object_t, oactions: scene_t[] = null) {
	if (raw.ext_type == "mesh_object_t") {
		mesh_object_setup_animation(raw.ext, oactions);
	}
	else {
		object_setup_animation_super(raw, oactions);
	}
}

///end
