
///if arm_anim

type anim_raw_t = {
	ext?: any; // anim_bone_t | anim_object_t
	ext_type?: string;
	is_skinned?: bool;
	action?: string;
	///if arm_skin
	armature?: armature_t; // Bone
	///end
	time?: f32;
	speed?: f32;
	loop?: bool;
	frame_index?: i32;
	on_complete?: ()=>void;
	paused?: bool;
	frame_time?: f32;
	blend_time?: f32;
	blend_current?: f32;
	blend_action?: string;
	blend_factor?: f32;
	last_frame_index?: i32;
	marker_events?: map_t<string, (()=>void)[]>;
};

// Lerp
let _anim_m1: mat4_t = mat4_identity();
let _anim_m2: mat4_t = mat4_identity();
let _anim_vpos: vec4_t = vec4_create();
let _anim_vpos2: vec4_t = vec4_create();
let _anim_vscale: vec4_t = vec4_create();
let _anim_vscale2: vec4_t = vec4_create();
let _anim_q1: quat_t = quat_create();
let _anim_q2: quat_t = quat_create();
let _anim_q3: quat_t = quat_create();
let _anim_vp: vec4_t = vec4_create();
let _anim_vs: vec4_t = vec4_create();

function anim_create(): anim_raw_t {
	let raw: anim_raw_t = {};
	raw.action = "";
	raw.time = 0.0;
	raw.speed = 1.0;
	raw.loop = true;
	raw.frame_index = 0;
	raw.paused = false;
	raw.frame_time = 1 / 60;
	raw.blend_time = 0.0;
	raw.blend_current = 0.0;
	raw.blend_action = "";
	raw.blend_factor = 0.0;
	raw.last_frame_index = -1;
	array_push(scene_animations, raw);
	return raw;
}

function anim_play_super(raw: anim_raw_t, action: string = "", on_complete: ()=>void = null, blend_time: f32 = 0.0, speed: f32 = 1.0, loop: bool = true) {
	if (blend_time > 0) {
		raw.blend_time = blend_time;
		raw.blend_current = 0.0;
		raw.blend_action = raw.action;
		raw.frame_index = 0;
		raw.time = 0.0;
	}
	else {
		raw.frame_index = -1;
	}
	raw.action = action;
	raw.on_complete = on_complete;
	raw.speed = speed;
	raw.loop = loop;
	raw.paused = false;
}

function anim_play(raw: anim_raw_t, action: string = "", on_complete: ()=>void = null, blend_time: f32 = 0.0, speed: f32 = 1.0, loop: bool = true) {
	if (raw.ext_type == "anim_object_t") {
		anim_object_play(raw.ext, action, on_complete, blend_time, speed, loop);
	}
	///if arm_skin
	else if (raw.ext_type == "anim_bone_t") {
		anim_bone_play(raw.ext, action, on_complete, blend_time, speed, loop);
	}
	///end
	else {
		anim_play_super(raw, action, on_complete, blend_time, speed, loop);
	}
}

function anim_blend_super(raw: anim_raw_t, action1: string, action2: string, factor: f32) {
	raw.blend_time = 1.0; // Enable blending
	raw.blend_factor = factor;
}

function anim_blend(raw: anim_raw_t, action1: string, action2: string, factor: f32) {
	if (raw.ext != null)  {
		///if arm_skin
		if (raw.ext_type == "anim_bone_t") {
			anim_bone_blend(raw.ext, action1, action2, factor);
		}
		///end
	}
	else {
		anim_blend_super(raw, action1, action2, factor);
	}
}

function anim_pause(raw: anim_raw_t) {
	raw.paused = true;
}

function anim_resume(raw: anim_raw_t) {
	raw.paused = false;
}

function anim_remove(raw: anim_raw_t) {
	array_remove(scene_animations, raw);
}

function anim_update_super(raw: anim_raw_t, delta: f32) {
	if (raw.paused || raw.speed == 0.0) {
		return;
	}
	raw.time += delta * raw.speed;

	if (raw.blend_time > 0 && raw.blend_factor == 0) {
		raw.blend_current += delta;
		if (raw.blend_current >= raw.blend_time) {
			raw.blend_time = 0.0;
		}
	}
}

function anim_update(raw: anim_raw_t, delta: f32) {
	if (raw.ext_type == "anim_object_t") {
		anim_object_update(raw.ext, delta);
	}
	///if arm_skin
	else if (raw.ext_type == "anim_bone_t") {
		anim_bone_update(raw.ext, delta);
	}
	///end
	else {
		anim_update_super(raw, delta);
	}
}

function anim_is_track_end(raw: anim_raw_t, track: track_t): bool {
	return raw.speed > 0 ?
		raw.frame_index >= track.frames.length - 1 :
		raw.frame_index <= 0;
}

function anim_check_frame_index(raw: anim_raw_t, frame_values: u32_array_t): bool {
	return raw.speed > 0 ?
		((raw.frame_index + 1) < frame_values.length && raw.time > frame_values[raw.frame_index + 1] * raw.frame_time) :
		((raw.frame_index - 1) > -1 && raw.time < frame_values[raw.frame_index - 1] * raw.frame_time);
}

function anim_rewind(raw: anim_raw_t, track: track_t) {
	raw.frame_index = raw.speed > 0 ? 0 : track.frames.length - 1;
	raw.time = track.frames[raw.frame_index] * raw.frame_time;
}

function anim_update_track(raw: anim_raw_t, anim: anim_t) {
	if (anim == null) {
		return;
	}

	let track: track_t = anim.tracks[0];

	if (raw.frame_index == -1) {
		anim_rewind(raw, track);
	}

	// Move keyframe
	let sign: i32 = raw.speed > 0 ? 1 : -1;
	while (anim_check_frame_index(raw, track.frames)) {
		raw.frame_index += sign;
	}

	// Marker events
	// if (raw.marker_events != null && anim.marker_names != null && raw.frame_index != raw.last_frame_index) {
	// 	for (let i: i32 = 0; i < anim.marker_frames.length; ++i) {
	// 		if (raw.frame_index == anim.marker_frames[i]) {
	// 			let ar: (()=>void)[] = map_get(raw.marker_events, anim.marker_names[i]);
	// 			if (ar != null) {
	// 				for (let i: i32 = 0; i < ar.length; ++i) {
	// 					ar[i]();
	// 				}
	// 			}
	// 		}
	// 	}
	// 	raw.last_frame_index = raw.frame_index;
	// }

	// End of track
	if (anim_is_track_end(raw, track)) {
		if (raw.loop || raw.blend_time > 0) {
			anim_rewind(raw, track);
		}
		else {
			raw.frame_index -= sign;
			raw.paused = true;
		}
		if (raw.on_complete != null && raw.blend_time == 0) {
			raw.on_complete();
		}
	}
}

function anim_update_anim_sampled(raw: anim_raw_t, anim: anim_t, m: mat4_t) {
	if (anim == null) {
		return;
	}
	let track: track_t = anim.tracks[0];
	let sign: i32 = raw.speed > 0 ? 1 : -1;

	let t: i32 = raw.time;
	let ti: i32 = raw.frame_index;
	let t1: f32 = track.frames[ti] * raw.frame_time;
	let t2: f32 = track.frames[ti + sign] * raw.frame_time;
	let s: f32 = (t - t1) / (t2 - t1); // Linear

	_anim_m1 = mat4_from_f32_array(track.values, ti * 16); // Offset to 4x4 matrix array
	_anim_m2 = mat4_from_f32_array(track.values, (ti + sign) * 16);

	// Decompose
	mat4_decompose(_anim_m1, _anim_vpos, _anim_q1, _anim_vscale);
	mat4_decompose(_anim_m2, _anim_vpos2, _anim_q2, _anim_vscale2);

	// Lerp
	_anim_vp = vec4_lerp(_anim_vpos, _anim_vpos2, s);
	_anim_vs = vec4_lerp(_anim_vscale, _anim_vscale2, s);
	_anim_q3 = quat_lerp(_anim_q1, _anim_q2, s);

	// Compose
	m = mat4_from_quat(_anim_q3);
	m = mat4_scale(m, _anim_vs);
	m.m30 = _anim_vp.x;
	m.m31 = _anim_vp.y;
	m.m32 = _anim_vp.z;
}

function anim_set_frame(raw: anim_raw_t, frame: i32) {
	raw.time = 0;
	raw.frame_index = frame;
	anim_update(raw, frame * raw.frame_time);
}

function anim_notify_on_marker(raw: anim_raw_t, name: string, on_marker: ()=>void) {
	// if (raw.marker_events == null) {
	// 	raw.marker_events = map_create();
	// }
	// let ar: (()=>void)[] = map_get(raw.marker_events, name);
	// if (ar == null) {
	// 	ar = [];
	// 	map_set(raw.marker_events, name, ar);
	// }
	// array_push(ar, on_marker);
}

function anim_remove_marker(raw: anim_raw_t, name: string, on_marker: ()=>void) {
	array_remove(map_get(raw.marker_events, name), on_marker);
}

function anim_current_frame(raw: anim_raw_t): i32 {
	return math_floor(raw.time / raw.frame_time);
}

function anim_total_frames(raw: anim_raw_t): i32 {
	return 0;
}

///end
