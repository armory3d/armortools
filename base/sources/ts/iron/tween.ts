
let _tween_anims: tween_anim_t[] = [];
let _tween_registered: bool = false;

function tween_on_reset() {
	app_notify_on_update(tween_update);
	tween_reset();
}

function _tween_register() {
	_tween_registered = true;
	app_notify_on_update(tween_update);
	app_notify_on_reset(tween_on_reset);
}

function tween_to(anim: tween_anim_t): tween_anim_t {
	if (!_tween_registered) {
		_tween_register();
	}
	let f32_target: f32_ptr = anim.target;
	if (f32_target != null) {
		anim._from = DEREFERENCE(f32_target);
	}
	array_push(_tween_anims, anim);
	return anim;
}

function tween_timer(delay: f32, done: (data: any)=>void, data: any = null): tween_anim_t {
	let a: tween_anim_t = { delay: delay, done: done, done_data: data };
	return tween_to(a);
}

function tween_stop(anim: tween_anim_t) {
	array_remove(_tween_anims, anim);
}

function tween_reset() {
	_tween_anims = [];
}

function tween_update() {
	let d: f32 = time_delta();
	let i: i32 = _tween_anims.length;
	while (i-- > 0 && _tween_anims.length > 0) {
		let a: tween_anim_t = _tween_anims[i];

		if (a.delay > 0) {
			a.delay -= d;
			continue;
		}

		a._time += d;
		if (a._time >= a.duration) {
			array_splice(_tween_anims, i, 1);
			i--;
			if (a.done != null) {
				a.done(a.done_data);
			}
			continue;
		}

		let k: f32 = a._time / a.duration;
		if (k > 1) {
			k = 1;
		}

		let f32_target: f32_ptr = a.target;
		DEREFERENCE(f32_target) = a._from + (a.to - a._from) * _tween_ease(a.ease, k);

		if (a.tick != null) {
			a.tick();
		}
	}
}

function _tween_ease_linear(k: f32): f32 { return k; }
// function _tween_ease_quad_in(k: f32): f32 { return k * k; }
// function _tween_ease_quad_out(k: f32): f32 { return -k * (k - 2); }
// function _tween_ease_quad_in_out(k: f32): f32 { return (k < 0.5) ? 2 * k * k : -2 * ((k -= 1) * k) + 1; }
function _tween_ease_expo_in(k: f32): f32 { return k == 0 ? 0 : math_pow(2, 10 * (k - 1)); }
function _tween_ease_expo_out(k: f32): f32 { return k == 1 ? 1 : (1 - math_pow(2, -10 * k)); }
// function _tween_ease_expo_in_out(k: f32): f32 { if (k == 0) { return 0; } if (k == 1) { return 1; } if ((k /= 1 / 2.0) < 1.0) { return 0.5 * math_pow(2, 10 * (k - 1)); } return 0.5 * (2 - math_pow(2, -10 * --k)); }
// function _tween_ease_bounce_in(k: f32): f32 { return 1 - _tween_ease_bounce_out(1 - k); }
// function _tween_ease_bounce_out(k: f32): f32 { if (k < (1 / 2.75)) { return 7.5625 * k * k; } else if (k < (2 / 2.75)) { return 7.5625 * (k -= (1.5 / 2.75)) * k + 0.75; } else if (k < (2.5 / 2.75)) { return 7.5625 * (k -= (2.25 / 2.75)) * k + 0.9375; } else { return 7.5625 * (k -= (2.625 / 2.75)) * k + 0.984375; } }
// function _tween_ease_bounce_in_out(k: f32): f32 { return (k < 0.5) ? _tween_ease_bounce_in(k * 2) * 0.5 : _tween_ease_bounce_out(k * 2 - 1) * 0.5 + 0.5; }

// function _tween_ease_elastic_in(k: f32): f32 {
// 	let s: f32;
// 	let a: f32 = 0.1;
// 	let p: f32 = 0.4;
// 	if (k == 0) {
// 		return 0;
// 	}
// 	if (k == 1) {
// 		return 1;
// 	}
// 	if (a < 1) {
// 		a = 1;
// 		s = p / 4;
// 	}
// 	else {
// 		s = p * math_asin(1 / a) / (2 * math_pi());
// 	}
// 	return -(a * math_pow(2, 10 * (k -= 1)) * math_sin((k - s) * (2 * math_pi()) / p));
// }

// function _tween_ease_elastic_out(k: f32): f32 {
// 	let s: f32;
// 	let a: f32 = 0.1;
// 	let p: f32 = 0.4;
// 	if (k == 0) {
// 		return 0;
// 	}
// 	if (k == 1) {
// 		return 1;
// 	}
// 	if (a < 1) {
// 		a = 1;
// 		s = p / 4;
// 	}
// 	else {
// 		s = p * math_asin(1 / a) / (2 * math_pi());
// 	}
// 	return (a * math_pow(2, -10 * k) * math_sin((k - s) * (2 * math_pi()) / p) + 1);
// }

// function _tween_ease_elastic_in_out(k: f32): f32 {
// 	let s: i32;
// 	let a: f32 = 0.1;
// 	let p: f32 = 0.4;
// 	if (k == 0) {
// 		return 0;
// 	}
// 	if (k == 1) {
// 		return 1;
// 	}
// 	if (a != 0 || a < 1) {
// 		a = 1;
// 		s = p / 4;
// 	}
// 	else {
// 		s = p * math_asin(1 / a) / (2 * math_pi());
// 	}
// 	if ((k *= 2) < 1) {
// 		return - 0.5 * (a * math_pow(2, 10 * (k -= 1)) * math_sin((k - s) * (2 * math_pi()) / p));
// 	}
// 	return a * math_pow(2, -10 * (k -= 1)) * math_sin((k - s) * (2 * math_pi()) / p) * 0.5 + 1;
// }

function _tween_ease(ease: ease_t, k: f32): f32 {
	if (ease == ease_t.LINEAR) {
		return _tween_ease_linear(k);
	}
	if (ease == ease_t.QUAD_IN) {
		// return _tween_ease_quad_in(k);
	}
	if (ease == ease_t.QUAD_OUT) {
		// return _tween_ease_quad_out(k);
	}
	if (ease == ease_t.QUAD_IN_OUT) {
		// return _tween_ease_quad_in_out(k);
	}
	if (ease == ease_t.EXPO_IN) {
		return _tween_ease_expo_in(k);
	}
	if (ease == ease_t.EXPO_OUT) {
		return _tween_ease_expo_out(k);
	}
	if (ease == ease_t.EXPO_IN_OUT) {
		// return _tween_ease_expo_in_out(k);
	}
	if (ease == ease_t.BOUNCE_IN) {
		// return _tween_ease_bounce_in(k);
	}
	if (ease == ease_t.BOUNCE_OUT) {
		// return _tween_ease_bounce_out(k);
	}
	if (ease == ease_t.BOUNCE_IN_OUT) {
		// return _tween_ease_bounce_in_out(k);
	}
	if (ease == ease_t.ELASTIC_IN) {
		// return _tween_ease_elastic_in(k);
	}
	if (ease == ease_t.ELASTIC_OUT) {
		// return _tween_ease_elastic_out(k);
	}
	if (ease == ease_t.ELASTIC_IN_OUT) {
		// return _tween_ease_elastic_in_out(k);
	}
	return 0.0;
}

type tween_anim_t = {
	target?: any;
	to?: f32;
	duration?: f32;
	delay?: f32;
	ease?: ease_t;
	done?: (data?: any)=>void;
	tick?: ()=>void;
	done_data?: any;
	_from?: f32;
	_time?: f32;
};

enum ease_t {
	LINEAR,
	QUAD_IN,
	QUAD_OUT,
	QUAD_IN_OUT,
	EXPO_IN,
	EXPO_OUT,
	EXPO_IN_OUT,
	BOUNCE_IN,
	BOUNCE_OUT,
	BOUNCE_IN_OUT,
	ELASTIC_IN,
	ELASTIC_OUT,
	ELASTIC_IN_OUT,
}
