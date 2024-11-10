
type callback_t = {
	f?: (data?: any)=>void;
	data?: any;
};

let app_on_resets: callback_t[] = [];
let app_on_next_frames: callback_t[] = [];
let app_on_end_frames: callback_t[] = [];
let app_on_inits: callback_t[] = [];
let app_on_updates: callback_t[] = [];
let app_on_renders: callback_t[] = [];
let app_on_renders_2d: callback_t[] = [];
let app_pause_updates: bool = false;
let app_lastw: i32 = -1;
let app_lasth: i32 = -1;
let app_on_resize: ()=>void = null;
let app_on_w: ()=>i32 = null;
let app_on_h: ()=>i32 = null;
let app_on_x: ()=>i32 = null;
let app_on_y: ()=>i32 = null;

function app_w(): i32 {
	if (app_on_w != null) {
		return app_on_w();
	}
	return sys_width();
}

function app_h(): i32 {
	if (app_on_h != null) {
		return app_on_h();
	}
	return sys_height();
}

function app_x(): i32 {
	if (app_on_x != null) {
		return app_on_x();
	}
	return 0;
}

function app_y(): i32 {
	if (app_on_y != null) {
		return app_on_y();
	}
	return 0;
}

function app_init() {
	sys_notify_on_frames(app_render);
}

function app_reset() {
	app_on_next_frames = [];
	app_on_end_frames = [];
	app_on_inits = [];
	app_on_updates = [];
	app_on_renders = [];
	app_on_renders_2d = [];
	for (let i: i32 = 0; i < app_on_resets.length; ++i) {
		let cb: callback_t = app_on_resets[i];
		cb.f(cb.data);
	}
}

function _app_run_callbacks(cbs: callback_t[]) {
	for (let i: i32 = 0; i < cbs.length; ++i) {
		let cb: callback_t = cbs[i];
		cb.f(cb.data);
	}
}

function app_update() {
	if (!_scene_ready) {
		return;
	}
	if (app_pause_updates) {
		return;
	}

	if (app_on_next_frames.length > 0) {
		_app_run_callbacks(app_on_next_frames);
		array_splice(app_on_next_frames, 0, app_on_next_frames.length);
	}

	scene_update_frame();

	let i: i32 = 0;
	let l: i32 = app_on_updates.length;
	while (i < l) {
		if (app_on_inits.length > 0) {
			_app_run_callbacks(app_on_inits);
			array_splice(app_on_inits, 0, app_on_inits.length);
		}

		let cb: callback_t = app_on_updates[i];
		cb.f(cb.data);

		// Account for removed traits
		if (l <= app_on_updates.length) {
			i++;
		}
		else {
			l = app_on_updates.length;
		}
	}

	_app_run_callbacks(app_on_end_frames);

	// Rebuild projection on window resize
	if (app_lastw == -1) {
		app_lastw = app_w();
		app_lasth = app_h();
	}
	if (app_lastw != app_w() || app_lasth != app_h()) {
		if (app_on_resize != null) {
			app_on_resize();
		}
		else if (scene_camera != null) {
			camera_object_build_proj(scene_camera);
		}
	}
	app_lastw = app_w();
	app_lasth = app_h();
}

function app_render() {
	app_update();
	time_update();

	if (!_scene_ready) {
		app_render_2d();
		return;
	}

	if (app_on_inits.length > 0) {
		_app_run_callbacks(app_on_inits);
		array_splice(app_on_inits, 0, app_on_inits.length);
	}

	scene_render_frame();
	_app_run_callbacks(app_on_renders);
	app_render_2d();
}

function app_render_2d() {
	if (app_on_renders_2d.length > 0) {
		g2_begin();
		_app_run_callbacks(app_on_renders_2d);
		g2_end();
	}
}

function _callback_create(f: (data?: any)=>void, data: any): callback_t {
	let cb: callback_t = {};
	cb.f = f;
	cb.data = data;
	return cb;
}

// Hooks
function app_notify_on_init(f: (data?: any)=>void, data: any = null) {
	array_push(app_on_inits, _callback_create(f, data));
}

function app_notify_on_update(f: (data?: any)=>void, data: any = null) {
	array_push(app_on_updates, _callback_create(f, data));
}

function app_notify_on_render(f: (data?: any)=>void, data: any = null) {
	array_push(app_on_renders, _callback_create(f, data));
}

function app_notify_on_render_2d(f: (data?: any)=>void, data: any = null) {
	array_push(app_on_renders_2d, _callback_create(f, data));
}

function app_notify_on_reset(f: (data?: any)=>void, data: any = null) {
	array_push(app_on_resets, _callback_create(f, data));
}

function app_notify_on_next_frame(f: (data?: any)=>void, data: any = null) {
	array_push(app_on_next_frames, _callback_create(f, data));
}

function app_notify_on_end_frame(f: (data?: any)=>void, data: any = null) {
	array_push(app_on_end_frames, _callback_create(f, data));
}

function _app_remove_callback(ar: callback_t[], f: (data?: any)=>void) {
	for (let i: i32 = 0; i < ar.length; ++i) {
		if (ar[i].f == f) {
			array_splice(ar, i, 1);
			break;
		}
	}
}

function app_remove_init(f: (data?: any)=>void) {
	_app_remove_callback(app_on_inits, f);
}

function app_remove_update(f: (data?: any)=>void) {
	_app_remove_callback(app_on_updates, f);
}

function app_remove_render(f: (data?: any)=>void) {
	_app_remove_callback(app_on_renders, f);
}

function app_remove_render_2d(f: (data?: any)=>void) {
	_app_remove_callback(app_on_renders_2d, f);
}

function app_remove_reset(f: (data?: any)=>void) {
	_app_remove_callback(app_on_resets, f);
}

function app_remove_end_frame(f: (data?: any)=>void) {
	_app_remove_callback(app_on_end_frames, f);
}
