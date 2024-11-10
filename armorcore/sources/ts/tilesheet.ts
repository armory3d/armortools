
let tilesheet_datas: tilesheet_data_t[];

function tilesheet_create(scene_name: string, tilesheet_ref: string, tilesheet_action_ref: string): tilesheet_t {
	let t: tilesheet_t = {};
	t.ready = false;
	for (let i: i32 = 0; i < tilesheet_datas.length; ++i) {
		let ts: tilesheet_data_t = tilesheet_datas[i];
		if (ts.name == tilesheet_ref) {
			t.raw = ts;
			tilesheet_play(t, tilesheet_action_ref);
			t.ready = true;
			break;
		}
	}
	return t;
}

function tilesheet_play(self: tilesheet_t, action_ref: string, on_action_complete: ()=>void = null) {
	self.on_action_complete = on_action_complete;
	for (let i: i32 = 0; i < self.raw.actions.length; ++i) {
		let a: tilesheet_action_t = self.raw.actions[i];
		if (a.name == action_ref) {
			self.action = a;
			break;
		}
	}
	tilesheet_set_frame(self, self.action.start);
	self.paused = false;
	self.time = 0.0;
}

function tilesheet_pause(self: tilesheet_t) {
	self.paused = true;
}

function tilesheet_resume(self: tilesheet_t) {
	self.paused = false;
}

function tilesheet_set_frame_offset(self: tilesheet_t, frame: i32) {
	tilesheet_set_frame(self, self.action.start + frame);
	self.paused = false;
}

function tilesheet_get_frame_offset(self: tilesheet_t): i32 {
	return self.frame - self.action.start;
}

function tilesheet_update(self: tilesheet_t) {
	if (!self.ready || self.paused) {
		return;
	}

	self.time += time_real_delta();

	let frame_time: f32 = 1 / self.raw.framerate;
	let frames_to_advance: i32 = 0;

	while (self.time >= frame_time) {
		self.time -= frame_time;
		frames_to_advance++;
	}

	if (frames_to_advance != 0) {
		tilesheet_set_frame(self, self.frame + frames_to_advance);
	}
}

function tilesheet_set_frame(self: tilesheet_t, f: i32) {
	self.frame = f;

	// Action end
	if (self.frame > self.action.end) {
		if (self.on_action_complete != null) {
			self.on_action_complete();
		}
		if (self.action.loop) {
			tilesheet_set_frame(self, self.action.start);
		}
		else {
			self.paused = true;
		}
		return;
	}

	let tx: i32 = self.frame % self.raw.tiles_x;
	let ty: i32 = math_floor(self.frame / self.raw.tiles_x);
	self.tile_x = tx * (1 / self.raw.tiles_x);
	self.tile_y = ty * (1 / self.raw.tiles_y);
}

type tilesheet_data_t = {
	name?: string;
	tiles_x?: i32;
	tiles_y?: i32;
	framerate?: i32;
	actions?: tilesheet_action_t[];
};

type tilesheet_action_t = {
	name?: string;
	start?: i32;
	end?: i32;
	loop?: bool;
};

type tilesheet_t = {
	tile_x?: f32; // Tile offset on tilesheet texture 0-1
	tile_y?: f32;
	raw?: tilesheet_data_t;
	action?: tilesheet_action_t;
	ready?: bool;
	paused?: bool;
	frame?: i32;
	time?: f32;
	on_action_complete?: ()=>void;
};
