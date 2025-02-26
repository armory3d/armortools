
///if arm_skin

type armature_t = {
	uid?: i32;
	name?: string;
	actions?: armature_action_t[];
	mats_ready?: bool;
};

type armature_action_t = {
	name?: string;
	bones?: obj_t[];
	mats?: mat4_t[];
};

let _armature_traverse_bones_data: any;

function armature_traverse_bones_done(object: obj_t) {
	array_push(_armature_traverse_bones_data, object);
}

function armature_create(uid: i32, name: string, actions: scene_t[]): armature_t {
	let raw: armature_t = {};
	raw.actions = [];
	raw.mats_ready = false;
	raw.uid = uid;
	raw.name = name;

	for (let i: i32 = 0; i < actions.length; ++i) {
		let a: scene_t = actions[i];
		for (let i: i32 = 0; i < a.objects.length; ++i) {
			let o: obj_t = a.objects[i];
			armature_set_parents(o);
		}
		let bones: obj_t[] = [];
		_armature_traverse_bones_data = bones;
		armature_traverse_bones(a.objects, armature_traverse_bones_done);
		array_push(raw.actions, { name: a.name, bones: bones, mats: null });
	}
	return raw;
}

function armature_init_mats(raw: armature_t) {
	if (raw.mats_ready) {
		return;
	}
	raw.mats_ready = true;

	for (let i: i32 = 0; i < raw.actions.length; ++i) {
		let a: armature_action_t = raw.actions[i];
		if (a.mats != null) {
			continue;
		}
		a.mats = [];
		for (let i: i32 = 0; i < a.bones.length; ++i) {
			let b: obj_t = a.bones[i];
			array_push(a.mats, mat4_from_f32_array(b.transform));
		}
	}
}

function armature_get_action(raw: armature_t, name: string): armature_action_t {
	for (let i: i32 = 0; i < raw.actions.length; ++i) {
		let a: armature_action_t = raw.actions[i];
		if (a.name == name) {
			return a;
		}
	}
	return null;
}

function armature_set_parents(object: obj_t) {
	if (object.children == null) {
		return;
	}

	for (let i: i32 = 0; i < object.children.length; ++i) {
		let o: obj_t = object.children[i];
		o.parent = object;
		armature_set_parents(o);
	}
}

function armature_traverse_bones(objects: obj_t[], callback: (o: obj_t)=>void) {
	for (let i: i32 = 0; i < objects.length; ++i) {
		armature_traverse_bones_step(objects[i], callback);
	}
}

function armature_traverse_bones_step(object: obj_t, callback: (o: obj_t)=>void) {
	if (object.type == "bone_object") {
		callback(object);
	}
	if (object.children == null) {
		return;
	}
	for (let i: i32 = 0; i < object.children.length; ++i) {
		armature_traverse_bones_step(object.children[i], callback);
	}
}

///end
