
type rgb_node_t = {
	base?: logic_node_t;
	image?: image_t;
	raw?: ui_node_t;
};

function rgb_node_create(raw: ui_node_t, args: f32_array_t): rgb_node_t {
	let n: rgb_node_t = {};
	n.raw = raw;
	n.base = logic_node_create(n);
	n.base.get_as_image = rgb_node_get_as_image;
	n.base.get_cached_image = rgb_node_get_cached_image;
	return n;
}

function rgb_node_get_as_image(self: rgb_node_t, from: i32): image_t {
	if (self.image != null) {
		app_notify_on_next_frame(function (self: rgb_node_t) {
			image_unload(self.image);
		}, self);
	}

	let f32a: f32_array_t = f32_array_create(4);
	let raw: ui_node_t = self.raw;
	let default_value: f32_array_t = raw.outputs[0].default_value;
	f32a[0] = default_value[0];
	f32a[1] = default_value[1];
	f32a[2] = default_value[2];
	f32a[3] = default_value[3];
	let buf: buffer_t = buffer_create_from_raw(f32a.buffer, f32a.length * 4);
	self.image = image_from_bytes(buf, 1, 1, tex_format_t.RGBA128);
	return self.image;
}

function rgb_node_get_cached_image(self: rgb_node_t): image_t {
	self.base.get_as_image(self, 0);
	return self.image;
}

let rgb_node_def: ui_node_t = {
	id: 0,
	name: _tr("RGB"),
	type: "rgb_node",
	x: 0,
	y: 0,
	color: 0xffb34f5a,
	inputs: [],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "RGBA",
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [
		{
			name: _tr("default_value"),
			type: "RGBA",
			output: 0,
			default_value: f32_array_create_xyzw(0.8, 0.8, 0.8, 1.0),
			data: null,
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		}
	],
	width: 0
};
