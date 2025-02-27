
///if arm_skin

type anim_bone_t = {
	base?: anim_raw_t;
	object?: mesh_object_t;
	data?: mesh_data_t;
	skin_buffer?: f32_array_t;
	skeleton_bones?: obj_t[];
	skeleton_mats?: mat4_t[];
	skeleton_bones_blend?: obj_t[];
	skeleton_mats_blend?: mat4_t[];
	abs_mats?: mat4_t[];
	apply_parent?: bool[];
	mats_fast?: mat4_t[];
	mats_fast_sort?: i32[];
	mats_fast_blend?: mat4_t[];
	mats_fast_blend_sort?: i32[];
	bone_children?: map_t<string, object_t[]>; // Parented to bone
	// Do inverse kinematics here
	on_updates?: (()=>void)[];
};

let _anim_bone_skin_max_bones: i32 = 128;
let _anim_bone_m: mat4_t = mat4_identity(); // Skinning matrix
let _anim_bone_m1: mat4_t = mat4_identity();
let _anim_bone_m2: mat4_t = mat4_identity();
let _anim_bone_bm: mat4_t = mat4_identity(); // Absolute bone matrix
let _anim_bone_wm: mat4_t = mat4_identity();
let _anim_bone_vpos: vec4_t = vec4_create();
let _anim_bone_vscale: vec4_t = vec4_create();
let _anim_bone_q1: quat_t = quat_create();
let _anim_bone_q2: quat_t = quat_create();
let _anim_bone_q3: quat_t = quat_create();
let _anim_bone_vpos2: vec4_t = vec4_create();
let _anim_bone_vscale2: vec4_t = vec4_create();
let _anim_bone_v1: vec4_t = vec4_create();
let _anim_bone_v2: vec4_t = vec4_create();

function anim_bone_create(armature_name: string = ""): anim_bone_t {
	let raw: anim_bone_t = {};
	raw.mats_fast = [];
	raw.mats_fast_sort = [];
	raw.mats_fast_blend = [];
	raw.mats_fast_blend_sort = [];
	raw.base = anim_create();
	raw.base.ext = raw;
	raw.base.ext_type = "anim_bone_t";

	for (let i: i32 = 0; i < scene_armatures.length; ++i) {
		let a: armature_t = scene_armatures[i];
		if (a.name == armature_name) {
			raw.base.armature = a;
			break;
		}
	}
	return raw;
}

function anim_bone_set_skin(raw: anim_bone_t, mo: mesh_object_t) {
	raw.object = mo;
	raw.data = mo != null ? mo.data : null;
	raw.base.is_skinned = raw.data != null ? raw.data.skin != null : false;
	if (raw.base.is_skinned) {
		let bone_size: i32 = 8; // Dual-quat skinning
		raw.skin_buffer = f32_array_create(_anim_bone_skin_max_bones * bone_size);
		for (let i: i32 = 0; i < raw.skin_buffer.length; ++i) {
			raw.skin_buffer[i] = 0;
		}
		// Rotation is already applied to skin at export
		raw.object.base.transform.rot = quat_create(0, 0, 0, 1);
		transform_build_matrix(raw.object.base.transform);

		let refs: string[] = mo.base.parent.raw.anim.bone_actions;
		if (refs != null && refs.length > 0) {
			let action: scene_t = data_get_scene_raw(refs[0]);
			anim_bone_play(raw, action.name);
		}
	}
}

function anim_bone_add_bone_child(raw: anim_bone_t, bone: string, o: object_t) {
	if (raw.bone_children == null) {
		raw.bone_children = map_create();
	}
	let ar: object_t[] = map_get(raw.bone_children, bone);
	if (ar == null) {
		ar = [];
		map_set(raw.bone_children, bone, ar);
	}
	array_push(ar, o);
}

function anim_bone_remove_bone_child(raw: anim_bone_t, bone: string, o: object_t) {
	if (raw.bone_children != null) {
		let ar: object_t[] = map_get(raw.bone_children, bone);
		if (ar != null) {
			array_remove(ar, o);
		}
	}
}

function anim_bone_update_bone_children(raw: anim_bone_t, bone: obj_t, bm: mat4_t) {
	let ar: object_t[] = map_get(raw.bone_children, bone.name);
	if (ar == null) {
		return;
	}
	for (let i: i32 = 0; i < ar.length; ++i) {
		let o: object_t = ar[i];
		let t: transform_t = o.transform;
		if (t.bone_parent == null) {
			t.bone_parent = mat4_identity();
		}
		if (o.raw.anim.parent_bone_tail != null) {
			if (o.raw.anim.parent_bone_connected || raw.base.is_skinned) {
				let v: f32_array_t = o.raw.anim.parent_bone_tail;
				t.bone_parent = mat4_init_translate(v[0], v[1], v[2]);
				t.bone_parent = mat4_mult_mat(t.bone_parent, bm);
			}
			else {
				let v: f32_array_t = o.raw.anim.parent_bone_tail_pose;
				t.bone_parent = mat4_clone(bm);
				t.bone_parent = mat4_translate(t.bone_parent, v[0], v[1], v[2]);
			}
		}
		else {
			t.bone_parent = mat4_clone(bm);
		}
		transform_build_matrix(t);
	}
}

function anim_bone_num_parents(b: obj_t): i32 {
	let i: i32 = 0;
	let p: obj_t = b.parent;
	while (p != null) {
		i++;
		p = p.parent;
	}
	return i;
}

let _anim_bone_sort_data: any;

function anim_bone_sort(pa: any_ptr, pb: any_ptr) {
	let a: i32 = DEREFERENCE(pa);
	let b: i32 = DEREFERENCE(pb);
	let i: i32 = anim_bone_num_parents(_anim_bone_sort_data[a]);
	let j: i32 = anim_bone_num_parents(_anim_bone_sort_data[b]);
	return i < j ? -1 : i > j ? 1 : 0;
}

function anim_bone_set_mats(raw: anim_bone_t) {
	while (raw.mats_fast.length < raw.skeleton_bones.length) {
		array_push(raw.mats_fast, mat4_identity());
		array_push(raw.mats_fast_sort, raw.mats_fast_sort.length);
	}
	// Calc bones with 0 parents first
	_anim_bone_sort_data = raw.skeleton_bones;
	array_sort(raw.mats_fast_sort, anim_bone_sort);

	if (raw.skeleton_bones_blend != null) {
		while (raw.mats_fast_blend.length < raw.skeleton_bones_blend.length) {
			array_push(raw.mats_fast_blend, mat4_identity());
			array_push(raw.mats_fast_blend_sort, raw.mats_fast_blend_sort.length);
		}
		_anim_bone_sort_data = raw.skeleton_bones_blend;
		array_sort(raw.mats_fast_blend_sort, anim_bone_sort);
	}
}

function anim_bone_set_action(raw: anim_bone_t, action: string) {
	if (raw.base.is_skinned) {
		raw.skeleton_bones = map_get(raw.data._actions, action);
		raw.skeleton_mats = map_get(raw.data._mats, action);
		raw.skeleton_bones_blend = null;
		raw.skeleton_mats_blend = null;
	}
	else {
		armature_init_mats(raw.base.armature);
		let a = armature_get_action(raw.base.armature, action);
		raw.skeleton_bones = a.bones;
		raw.skeleton_mats = a.mats;
	}
	anim_bone_set_mats(raw);
}

function anim_bone_set_action_blend(raw: anim_bone_t, action: string) {
	if (raw.base.is_skinned) {
		raw.skeleton_bones_blend = raw.skeleton_bones;
		raw.skeleton_mats_blend = raw.skeleton_mats;
		raw.skeleton_bones = map_get(raw.data._actions, action);
		raw.skeleton_mats = map_get(raw.data._mats, action);
	}
	else {
		armature_init_mats(raw.base.armature);
		let a: armature_action_t = armature_get_action(raw.base.armature, action);
		raw.skeleton_bones = a.bones;
		raw.skeleton_mats = a.mats;
	}
	anim_bone_set_mats(raw);
}

function anim_bone_mult_parent(raw: anim_bone_t, i: i32, fasts: mat4_t[], bones: obj_t[], mats: mat4_t[]) {
	let f: mat4_t = fasts[i];
	if (raw.apply_parent != null && !raw.apply_parent[i]) {
		f = mat4_clone(mats[i]);
		return;
	}
	let p = bones[i].parent;
	let bi = anim_bone_get_bone_index(raw, p, bones);
	if (p == null || bi == -1) {
		f = mat4_clone(mats[i]);
	}
	else {
		f = mat4_mult_mat(mats[i], fasts[bi]);
	}
}

function anim_bone_mult_parents(raw: anim_bone_t, m: mat4_t, i: i32, bones: obj_t[], mats: mat4_t[]) {
	let bone: obj_t = bones[i];
	let p: obj_t = bone.parent;
	while (p != null) {
		let i: i32 = anim_bone_get_bone_index(raw, p, bones);
		if (i == -1) {
			continue;
		}
		m = mat4_mult_mat(m, mats[i]);
		p = p.parent;
	}
}

function anim_bone_notify_on_update(raw: anim_bone_t, f: ()=>void) {
	if (raw.on_updates == null) {
		raw.on_updates = [];
	}
	array_push(raw.on_updates, f);
}

function anim_bone_remove_update(raw: anim_bone_t, f: ()=>void) {
	array_remove(raw.on_updates, f);
}

function anim_bone_update_bones_only(raw: anim_bone_t) {
	if (raw.bone_children != null) {
		for (let i: i32 = 0; i < raw.skeleton_bones.length; ++i) {
			let b = raw.skeleton_bones[i]; // TODO: blend_time > 0
			_anim_bone_m = mat4_clone(raw.mats_fast[i]);
			anim_bone_update_bone_children(raw, b, _anim_bone_m);
		}
	}
}

function anim_bone_update_skin_gpu(raw: anim_bone_t) {
	let bones: obj_t[] = raw.skeleton_bones;

	let s: f32 = raw.base.blend_current / raw.base.blend_time;
	s = s * s * (3.0 - 2.0 * s); // Smoothstep
	if (raw.base.blend_factor != 0.0) {
		s = 1.0 - raw.base.blend_factor;
	}

	// Update skin buffer
	for (let i: i32 = 0; i < bones.length; ++i) {
		_anim_bone_m = mat4_clone(raw.mats_fast[i]);

		if (raw.base.blend_time > 0 && raw.skeleton_bones_blend != null) {
			// Decompose
			_anim_bone_m1 = mat4_clone(raw.mats_fast_blend[i]);
			mat4_decompose(_anim_bone_m1, _anim_bone_vpos, _anim_bone_q1, _anim_bone_vscale);
			mat4_decompose(_anim_bone_m, _anim_bone_vpos2, _anim_bone_q2, _anim_bone_vscale2);

			// Lerp
			_anim_bone_v1 = vec4_lerp(_anim_bone_vpos, _anim_bone_vpos2, s);
			_anim_bone_v2 = vec4_lerp(_anim_bone_vscale, _anim_bone_vscale2, s);
			_anim_bone_q3 = quat_lerp(_anim_bone_q1, _anim_bone_q2, s);

			// Compose
			_anim_bone_m = mat4_from_quat(_anim_bone_q3);
			_anim_bone_m = mat4_scale(_anim_bone_m, _anim_bone_v2);
			_anim_bone_m.m30 = _anim_bone_v1.x;
			_anim_bone_m.m31 = _anim_bone_v1.y;
			_anim_bone_m.m32 = _anim_bone_v1.z;
		}

		if (raw.abs_mats != null && i < raw.abs_mats.length) {
			raw.abs_mats[i] = mat4_clone(_anim_bone_m);
		}
		if (raw.bone_children != null) {
			anim_bone_update_bone_children(raw, bones[i], _anim_bone_m);
		}

		_anim_bone_m = mat4_mult_mat(raw.data._skeleton_transforms_inv[i], _anim_bone_m);
		anim_bone_update_skin_buffer(raw, _anim_bone_m, i);
	}
}

function anim_bone_update_skin_buffer(raw: anim_bone_t, m: mat4_t, i: i32) {
	// Dual quat skinning
	mat4_decompose(m, _anim_bone_vpos, _anim_bone_q1, _anim_bone_vscale);
	_anim_bone_q1 = quat_norm(_anim_bone_q1);
	_anim_bone_q2 = quat_create(_anim_bone_vpos.x, _anim_bone_vpos.y, _anim_bone_vpos.z, 0.0);
	_anim_bone_q2 = quat_mult(_anim_bone_q2, _anim_bone_q1);
	raw.skin_buffer[i * 8] = _anim_bone_q1.x; // Real
	raw.skin_buffer[i * 8 + 1] = _anim_bone_q1.y;
	raw.skin_buffer[i * 8 + 2] = _anim_bone_q1.z;
	raw.skin_buffer[i * 8 + 3] = _anim_bone_q1.w;
	raw.skin_buffer[i * 8 + 4] = _anim_bone_q2.x * 0.5; // Dual
	raw.skin_buffer[i * 8 + 5] = _anim_bone_q2.y * 0.5;
	raw.skin_buffer[i * 8 + 6] = _anim_bone_q2.z * 0.5;
	raw.skin_buffer[i * 8 + 7] = _anim_bone_q2.w * 0.5;
}

function anim_bone_get_bone(raw: anim_bone_t, name: string): obj_t {
	if (raw.skeleton_bones == null) {
		return null;
	}
	for (let i: i32 = 0; i < raw.skeleton_bones.length; ++i) {
		let b: obj_t = raw.skeleton_bones[i];
		if (b.name == name) {
			return b;
		}
	}
	return null;
}

function anim_bone_get_bone_index(raw: anim_bone_t, bone: obj_t, bones: obj_t[] = null): i32 {
	if (bones == null) {
		bones = raw.skeleton_bones;
	}
	if (bones != null) {
		for (let i: i32 = 0; i < bones.length; ++i) {
			if (bones[i] == bone) {
				return i;
			}
		}
	}
	return -1;
}

function anim_bone_get_bone_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	return raw.skeleton_mats != null ? raw.skeleton_mats[anim_bone_get_bone_index(raw, bone)] : null;
}

function anim_bone_get_bone_mat_blend(raw: anim_bone_t, bone: obj_t): mat4_t {
	return raw.skeleton_mats_blend != null ? raw.skeleton_mats_blend[anim_bone_get_bone_index(raw, bone)] : null;
}

function anim_bone_get_abs_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	// With applied blending
	if (raw.skeleton_mats == null) {
		return null;
	}
	if (raw.abs_mats == null) {
		raw.abs_mats = [];
		while (raw.abs_mats.length < raw.skeleton_mats.length) {
			array_push(raw.abs_mats, mat4_identity());
		}
	}
	return raw.abs_mats[anim_bone_get_bone_index(raw, bone)];
}

function anim_bone_get_world_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	if (raw.skeleton_mats == null) {
		return null;
	}
	if (raw.apply_parent == null) {
		raw.apply_parent = [];
		for (let i: i32 = 0; i < raw.skeleton_mats.length; ++i) {
			array_push(raw.apply_parent, true);
		}
	}
	let i: i32 = anim_bone_get_bone_index(raw, bone);
	_anim_bone_wm = mat4_clone(raw.skeleton_mats[i]);
	anim_bone_mult_parents(raw, _anim_bone_wm, i, raw.skeleton_bones, raw.skeleton_mats);
	// anim_bone_wm = mat4_clone(raw.mats_fast[i]); // TODO
	return _anim_bone_wm;
}

function anim_bone_get_bone_len(raw: anim_bone_t, bone: obj_t): f32 {
	let refs: string[] = raw.data.skin.bone_ref_array;
	let lens: f32_array_t = raw.data.skin.bone_len_array;
	for (let i: i32 = 0; i < refs.length; ++i) {
		if (refs[i] == bone.name) {
			return lens[i];
		}
	}
	return 0.0;
}

// Returns bone length with scale applied
function anim_bone_get_bone_abs_len(raw: anim_bone_t, bone: obj_t): f32 {
	let refs: string[] = raw.data.skin.bone_ref_array;
	let lens: f32_array_t = raw.data.skin.bone_len_array;
	let scale: f32 = mat4_get_scale(raw.object.base.parent.transform.world).z;
	for (let i: i32 = 0; i < refs.length; ++i) {
		if (refs[i] == bone.name) {
			return lens[i] * scale;
		}
	}
	return 0.0;
}

// Returns bone matrix in world space
function anim_bone_get_abs_world_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	let wm = anim_bone_get_world_mat(raw, bone);
	wm = mat4_mult_mat(wm, raw.object.base.parent.transform.world);
	return wm;
}

function anim_bone_solve_ik(raw: anim_bone_t, effector: obj_t, goal: vec4_t, precision: f32 = 0.01, max_iters: i32 = 100, chain_lenght: i32 = 100, roll_angle: f32 = 0.0) {
	// Array of bones to solve IK for, effector at 0
	let bones: obj_t[] = [];

	// Array of bones lengths, effector length at 0
	let lengths: f32[] = [];

	// Array of bones matrices in world coordinates, effector at 0
	let bone_world_mats: mat4_t[];

	let temp_loc: vec4_t = vec4_create();
	let temp_rot: quat_t = quat_create();
	let temp_rot2: quat_t = quat_create();
	let temp_scale: vec4_t = vec4_create();
	let roll: quat_t = quat_from_euler(0, roll_angle, 0);

	// Store all bones and lengths in array
	let tip: obj_t = effector;
	array_push(bones, tip);
	array_push(lengths, anim_bone_get_bone_abs_len(raw, tip));
	let root: obj_t = tip;

	while (root.parent != null) {
		if (bones.length > chain_lenght - 1) {
			break;
		}
		array_push(bones, root.parent);
		array_push(lengths, anim_bone_get_bone_abs_len(raw, root.parent));
		root = root.parent;
	}

	// Root bone
	root = bones[bones.length - 1];

	// World matrix of root bone
	let root_world_mat: mat4_t = mat4_clone(anim_bone_get_world_mat(raw, root));
	// World matrix of armature
	let armature_mat: mat4_t = mat4_clone(raw.object.base.parent.transform.world);
	// Apply armature transform to world matrix
	root_world_mat = mat4_mult_mat(root_world_mat, armature_mat);
	// Distance from root to goal
	let dist: f32 = vec4_dist(goal, mat4_get_loc(root_world_mat));

	// Total bones length
	let total_length: f32 = 0.0;
	for (let i: i32 = 0; i < lengths.length; ++i) {
		let l: f32 = lengths[i];
		total_length += l;
	}

	// Unreachable distance
	if (dist > total_length) {
		// Calculate unit vector from root to goal
		let new_look: vec4_t = vec4_clone(goal);
		new_look = vec4_sub(new_look, mat4_get_loc(root_world_mat));
		new_look = vec4_norm(new_look);

		// Rotate root bone to point at goal
		mat4_decompose(root_world_mat, temp_loc, temp_rot, temp_scale);
		temp_rot2 = quat_from_to(vec4_norm(mat4_look(root_world_mat)), new_look);
		temp_rot2 = quat_mult(temp_rot2, temp_rot);
		temp_rot2 = quat_mult(temp_rot2, roll);
		root_world_mat = mat4_compose(temp_loc, temp_rot2, temp_scale);

		// Set bone matrix in local space from world space
		anim_bone_set_bone_mat_from_world_mat(raw, root_world_mat, root);

		// Set child bone rotations to zero
		for (let i: i32 = 0; i < bones.length - 1; ++i) {
			mat4_decompose(anim_bone_get_bone_mat(raw, bones[i]), temp_loc, temp_rot, temp_scale);
			mat4_compose(anim_bone_get_bone_mat(raw, bones[i]), temp_loc, roll, temp_scale);
		}
		return;
	}

	// Get all bone mats in world space
	bone_world_mats = anim_bone_get_world_mats_fast(raw, effector, bones.length);

	// Array of bone locations in world space, root location at [0]
	let bone_world_locs: vec4_t[] = [];
	for (let i: i32 = 0; i < bone_world_mats.length; ++i) {
		let b: mat4_t = bone_world_mats[i];
		array_push(bone_world_locs, mat4_get_loc(b));
	}

	// Solve FABRIK
	let vec: vec4_t = vec4_create();
	let start_loc: vec4_t = vec4_clone(bone_world_locs[0]);
	let l: i32 = bone_world_locs.length;

	for (let iter: i32 = 0; iter < max_iters; ++iter) {
		// Backward
		vec = vec4_clone(goal);
		vec = vec4_sub(vec, bone_world_locs[l - 1]);
		vec = vec4_norm(vec);
		vec = vec4_mult(vec, lengths[0]);
		bone_world_locs[l - 1] = vec4_clone(goal);
		bone_world_locs[l - 1] = vec4_sub(bone_world_locs[l - 1], vec);

		for (let j: i32 = 1; j < l; ++j) {
			vec = vec4_clone(bone_world_locs[l - 1 - j]);
			vec = vec4_sub(vec, bone_world_locs[l - j]);
			vec = vec4_norm(vec);
			vec = vec4_mult(vec, lengths[j]);
			bone_world_locs[l - 1 - j] = vec4_clone(bone_world_locs[l - j]);
			bone_world_locs[l - 1 - j] = vec4_add(bone_world_locs[l - 1 - j], vec);
		}

		// Forward
		bone_world_locs[0] = vec4_clone(start_loc);
		for (let j: i32 = 1; j < l; ++j) {
			vec = vec4_clone(bone_world_locs[j]);
			vec = vec4_sub(vec, bone_world_locs[j - 1]);
			vec = vec4_norm(vec);
			vec = vec4_mult(vec, lengths[l - j]);
			bone_world_locs[j] = vec4_clone(bone_world_locs[j - 1]);
			bone_world_locs[j] = vec4_add(bone_world_locs[j], vec);
		}

		if (vec4_dist(bone_world_locs[l - 1], goal) - lengths[0] <= precision) {
			break;
		}
	}

	// Correct rotations
	// Applying locations and rotations
	let temp_look: vec4_t = vec4_create();
	let temp_loc2: vec4_t = vec4_create();

	for (let i: i32 = 0; i < l - 1; ++i) {
		// Decompose matrix
		mat4_decompose(bone_world_mats[i], temp_loc, temp_rot, temp_scale);

		// Rotate to point to parent bone
		temp_loc2 = vec4_clone(bone_world_locs[i + 1]);
		temp_loc2 = vec4_sub(temp_loc2, bone_world_locs[i]);
		temp_loc2 = vec4_norm(temp_loc2);
		temp_look = vec4_clone(mat4_look(bone_world_mats[i]));
		temp_look = vec4_norm(temp_look);
		temp_rot2 = quat_from_to(temp_look, temp_loc2);
		temp_rot2 = quat_mult(temp_rot2, temp_rot);
		temp_rot2 = quat_mult(temp_rot2, roll);

		// Compose matrix with new rotation and location
		bone_world_mats[i] = mat4_compose(bone_world_locs[i], temp_rot2, temp_scale);

		// Set bone matrix in local space from world space
		anim_bone_set_bone_mat_from_world_mat(raw, bone_world_mats[i], bones[bones.length - 1 - i]);
	}

	// Decompose matrix
	mat4_decompose(bone_world_mats[l - 1], temp_loc, temp_rot, temp_scale);

	// Rotate to point to goal
	temp_loc2 = vec4_clone(goal);
	temp_loc2 = vec4_sub(temp_loc2, temp_loc);
	temp_loc2 = vec4_norm(temp_loc2);
	temp_look = vec4_clone(mat4_look(bone_world_mats[l - 1]));
	temp_look = vec4_norm(temp_look);
	temp_rot2 = quat_from_to(temp_look, temp_loc2);
	temp_rot2 = quat_mult(temp_rot2, temp_rot);
	temp_rot2 = quat_mult(temp_rot2, roll);

	// Compose matrix with new rotation and location
	bone_world_mats[l - 1] = mat4_compose(bone_world_locs[l - 1], temp_rot2, temp_scale);

	// Set bone matrix in local space from world space
	anim_bone_set_bone_mat_from_world_mat(raw, bone_world_mats[l - 1], bones[0]);
}

// Returns an array of bone matrices in world space
function anim_bone_get_world_mats_fast(raw: anim_bone_t, tip: obj_t, chain_len: i32): mat4_t[] {
	let wm_array: mat4_t[] = [];
	let root: obj_t = tip;
	let num_p: i32 = chain_len;
	for (let i: i32 = 0; i < chain_len; ++i) {
		let wm: mat4_t = anim_bone_get_abs_world_mat(raw, root);
		wm_array[chain_len - 1 - i] = mat4_clone(wm);
		root = root.parent;
		num_p--;
	}

	// Root bone at [0]
	return wm_array;
}

// Set bone transforms in world space
function anim_bone_set_bone_mat_from_world_mat(raw: anim_bone_t, wm: mat4_t, bone: obj_t) {
	let inv_mat: mat4_t = mat4_identity();
	let temp_mat: mat4_t = mat4_clone(wm);
	inv_mat = mat4_inv(raw.object.base.parent.transform.world);
	temp_mat = mat4_mult_mat(temp_mat, inv_mat);
	let bones: obj_t[] = [];
	let p_bone: obj_t = bone;
	while (p_bone.parent != null) {
		array_push(bones, p_bone.parent);
		p_bone = p_bone.parent;
	}

	for (let i: i32 = 0; i < bones.length; ++i) {
		let x: i32 = bones.length - 1;
		inv_mat = mat4_inv(anim_bone_get_bone_mat(raw, bones[x - i]));
		temp_mat = mat4_mult_mat(temp_mat, inv_mat);
	}

	mat4_set_from(anim_bone_get_bone_mat(raw, bone), temp_mat);
}

function anim_bone_play(raw: anim_bone_t, action: string = "", on_complete: ()=>void = null, blend_time: f32 = 0.2, speed: f32 = 1.0, loop: bool = true) {
	if (action != "") {
		blend_time > 0 ? anim_bone_set_action_blend(raw, action) : anim_bone_set_action(raw, action);
	}
	raw.base.blend_factor = 0.0;
}

function anim_bone_blend(raw: anim_bone_t, action1: string, action2: string, factor: f32) {
	if (factor == 0.0) {
		anim_bone_set_action(raw, action1);
		return;
	}
	anim_bone_set_action(raw, action2);
	anim_bone_set_action_blend(raw, action1);

	anim_blend_super(raw.base, action1, action2, factor);
}

function anim_bone_update(raw: anim_bone_t, delta: f32) {
	if (!raw.base.is_skinned && raw.skeleton_bones == null) {
		anim_bone_set_action(raw, raw.base.armature.actions[0].name);
	}
	if (raw.object != null && (!raw.object.base.visible || raw.object.base.culled)) {
		return;
	}
	if (raw.skeleton_bones == null || raw.skeleton_bones.length == 0) {
		return;
	}

	anim_update_super(raw.base, delta);

	if (raw.base.paused || raw.base.speed == 0.0) {
		return;
	}

	let last_bones: obj_t[] = raw.skeleton_bones;
	for (let i: i32 = 0; i < raw.skeleton_bones.length; ++i) {
		let b: obj_t = raw.skeleton_bones[i];
		if (b.anim != null) {
			anim_update_track(raw.base, b.anim);
			break;
		}
	}
	// Action has been changed by on_complete
	if (last_bones != raw.skeleton_bones) {
		return;
	}

	for (let i: i32 = 0; i < raw.skeleton_bones.length; ++i) {
		anim_update_anim_sampled(raw.base, raw.skeleton_bones[i].anim, raw.skeleton_mats[i]);
	}
	if (raw.base.blend_time > 0 && raw.skeleton_bones_blend != null) {
		for (let i: i32 = 0; i < raw.skeleton_bones_blend.length; ++i) {
			let b: obj_t = raw.skeleton_bones_blend[i];
			if (b.anim != null) {
				anim_update_track(raw.base, b.anim);
				break;
			}
		}
		for (let i: i32 = 0; i < raw.skeleton_bones_blend.length; ++i) {
			anim_update_anim_sampled(raw.base, raw.skeleton_bones_blend[i].anim, raw.skeleton_mats_blend[i]);
		}
	}

	// Do forward kinematics and inverse kinematics here
	if (raw.on_updates != null) {
		let i: i32 = 0;
		let l: i32 = raw.on_updates.length;
		while (i < l) {
			raw.on_updates[i]();
			l <= raw.on_updates.length ? i++ : l = raw.on_updates.length;
		}
	}

	// Calc absolute bones
	for (let i: i32 = 0; i < raw.skeleton_bones.length; ++i) {
		// Take bones with 0 parents first
		anim_bone_mult_parent(raw, raw.mats_fast_sort[i], raw.mats_fast, raw.skeleton_bones, raw.skeleton_mats);
	}
	if (raw.skeleton_bones_blend != null) {
		for (let i: i32 = 0; i < raw.skeleton_bones_blend.length; ++i) {
			anim_bone_mult_parent(raw, raw.mats_fast_blend_sort[i], raw.mats_fast_blend, raw.skeleton_bones_blend, raw.skeleton_mats_blend);
		}
	}

	if (raw.base.is_skinned) {
		anim_bone_update_skin_gpu(raw);
	}
	else {
		anim_bone_update_bones_only(raw);
	}
}

function anim_bone_total_frames(raw: anim_bone_t): i32 {
	if (raw.skeleton_bones == null) {
		return 0;
	}
	let track: track_t = raw.skeleton_bones[0].anim.tracks[0];
	return math_floor(track.frames[track.frames.length - 1] - track.frames[0]);
}

///end
