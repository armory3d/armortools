
///if arm_anim

type anim_object_t = {
	base?: anim_raw_t;
	object?: object_t;
	oactions?: scene_t[];
	oaction?: obj_t;
};

function anim_object_create(object: object_t, oactions: scene_t[]): anim_object_t {
	let raw: anim_object_t = {};
	raw.base = anim_create();
	raw.base.ext = raw;
	raw.base.ext_type = "anim_bone_t";
	raw.object = object;
	raw.oactions = oactions;
	raw.base.is_skinned = false;
	anim_object_play(raw);
	return raw;
}

function anim_object_get_action(raw: anim_object_t, action: string): obj_t {
	for (let i: i32 = 0; i < raw.oactions.length; ++i) {
		let a: scene_t = raw.oactions[i];
		if (a != null && a.objects[0].name == action) {
			return a.objects[0];
		}
	}
	return null;
}

function anim_object_update_object_anim(raw: anim_object_t) {
	anim_object_update_transform_anim(raw, raw.oaction.anim, raw.object.transform);
	transform_build_matrix(raw.object.transform);
}

function anim_object_interpolate_linear(t: f32, t1: f32, t2: f32, v1: f32, v2: f32): f32 {
	let s: f32 = (t - t1) / (t2 - t1);
	return (1.0 - s) * v1 + s * v2;
}

function anim_object_check_frame_index_t(raw: anim_object_t, frame_values: u32_array_t, t: f32): bool {
	return raw.base.speed > 0 ?
		raw.base.frame_index < frame_values.length - 2 && t > frame_values[raw.base.frame_index + 1] * raw.base.frame_time :
		raw.base.frame_index > 1 && t > frame_values[raw.base.frame_index - 1] * raw.base.frame_time;
}

function anim_object_update_transform_anim(raw: anim_object_t, anim: anim_t, transform: transform_t) {
	if (anim == null) {
		return;
	}

	let total: i32 = anim.end * raw.base.frame_time - anim.begin * raw.base.frame_time;

	if (anim.has_delta) {
		let t: transform_t = transform;
		if (vec4_isnan(t.dloc)) {
			t.dloc = vec4_create();
			t.drot = quat_create();
			t.dscale = vec4_create();
		}
		t.dloc = vec4_create(0, 0, 0);
		t.dscale = vec4_create(0, 0, 0);
		t._deuler_x = t._deuler_y = t._deuler_z = 0.0;
	}

	for (let i: i32 = 0; i < anim.tracks.length; ++i) {
		let track: track_t = anim.tracks[i];

		if (raw.base.frame_index == -1) {
			anim_rewind(raw.base, track);
		}
		let sign: i32 = raw.base.speed > 0 ? 1 : -1;

		// End of current time range
		let t: f32 = raw.base.time + anim.begin * raw.base.frame_time;
		while (anim_object_check_frame_index_t(raw, track.frames, t)) {
			raw.base.frame_index += sign;
		}

		// No data for raw track at current time
		if (raw.base.frame_index >= track.frames.length) {
			continue;
		}

		// End of track
		if (raw.base.time > total) {
			if (raw.base.on_complete != null) {
				raw.base.on_complete();
			}
			if (raw.base.loop) {
				anim_rewind(raw.base, track);
			}
			else {
				raw.base.frame_index -= sign;
				raw.base.paused = true;
			}
			return;
		}

		let ti: i32 = raw.base.frame_index;
		let t1: f32 = track.frames[ti] * raw.base.frame_time;
		let t2: f32 = track.frames[ti + sign] * raw.base.frame_time;
		let v1: f32 = track.values[ti];
		let v2: f32 = track.values[ti + sign];

		let value: f32 = anim_object_interpolate_linear(t, t1, t2, v1, v2);

		let target: string = track.target;
		if (target == "xloc") {
			transform.loc.x = value;
		}
		else if (target == "yloc") {
			transform.loc.y = value;
		}
		else if (target == "zloc") {
			transform.loc.z = value;
		}
		else if (target == "xrot") {
			transform_set_rot(transform, value, transform._euler_y, transform._euler_z);
		}
		else if (target == "yrot") {
			transform_set_rot(transform, transform._euler_x, value, transform._euler_z);
		}
		else if (target == "zrot") {
			transform_set_rot(transform, transform._euler_x, transform._euler_y, value);
		}
		else if (target == "qwrot") {
			transform.rot.w = value;
		}
		else if (target == "qxrot") {
			transform.rot.x = value;
		}
		else if (target == "qyrot") {
			transform.rot.y = value;
		}
		else if (target == "qzrot") {
			transform.rot.z = value;
		}
		else if (target == "xscl") {
			transform.scale.x = value;
		}
		else if (target == "yscl") {
			transform.scale.y = value;
		}
		else if (target == "zscl") {
			transform.scale.z = value;
		}
		// Delta
		else if (target == "dxloc") {
			transform.dloc.x = value;
		}
		else if (target == "dyloc") {
			transform.dloc.y = value;
		}
		else if (target == "dzloc") {
			transform.dloc.z = value;
		}
		else if (target == "dxrot") {
			transform._deuler_x = value;
		}
		else if (target == "dyrot") {
			transform._deuler_y = value;
		}
		else if (target == "dzrot") {
			transform._deuler_z = value;
		}
		else if (target == "dqwrot") {
			transform.drot.w = value;
		}
		else if (target == "dqxrot") {
			transform.drot.x = value;
		}
		else if (target == "dqyrot") {
			transform.drot.y = value;
		}
		else if (target == "dqzrot") {
			transform.drot.z = value;
		}
		else if (target == "dxscl") {
			transform.dscale.x = value;
		}
		else if (target == "dyscl") {
			transform.dscale.y = value;
		}
		else if (target == "dzscl") {
			transform.dscale.z = value;
		}
	}
}

function anim_object_play(raw: anim_object_t, action: string = "", on_complete: ()=>void = null, blend_time: f32 = 0.0, speed: f32 = 1.0, loop: bool = true) {
	anim_play_super(raw.base, action, on_complete, blend_time, speed, loop);

	if (raw.base.action == "" && raw.oactions[0] != null) {
		raw.base.action = raw.oactions[0].objects[0].name;
	}
	raw.oaction = anim_object_get_action(raw, raw.base.action);
}

function anim_object_update(raw: anim_object_t, delta: f32) {
	if (!raw.object.visible || raw.object.culled || raw.oaction == null) {
		return;
	}

	anim_update_super(raw.base, delta);

	if (raw.base.paused) {
		return;
	}
	if (!raw.base.is_skinned) {
		anim_object_update_object_anim(raw);
	}
}

function anim_object_is_track_end(raw: anim_object_t, track: track_t): bool {
	return raw.base.speed > 0 ?
		raw.base.frame_index >= track.frames.length - 2 :
		raw.base.frame_index <= 0;
}

function anim_object_total_frames(raw: anim_object_t): i32 {
	if (raw.oaction == null || raw.oaction.anim == null) {
		return 0;
	}
	return raw.oaction.anim.end - raw.oaction.anim.begin;
}

///end
