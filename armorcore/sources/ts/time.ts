
let _time_last: f32 = 0.0;
let _time_real_delta: f32 = 0.0;
let _time_frequency: i32 = -1;

function time_delta(): f32 {
	if (_time_frequency < 0) {
		_time_frequency = sys_display_frequency();
	}
	return (1 / _time_frequency);
}

function time_real_delta(): f32 {
	return _time_real_delta;
}

function time_time(): f32 {
	return sys_time();
}

function time_update() {
	_time_real_delta = time_time() - _time_last;
	_time_last = time_time();
}
