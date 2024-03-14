
type rgb_node_t = {
	base?: logic_node_t;
	image?: image_t;
};

function rgb_node_create(): rgb_node_t {
	let n: rgb_node_t = {};
	n.base = logic_node_create();
	n.base.get_as_image = rgb_node_get_as_image;
	n.base.get_cached_image = rgb_node_get_cached_image;
	return n;
}

function rgb_node_get_as_image(self: rgb_node_t, from: i32, done: (img: image_t)=>void) {
	if (self.image != null) {
		base_notify_on_next_frame(() => {
			image_unload(self.image);
		});
	}

	let f32a = new Float32Array(4);
	let raw = parser_logic_get_raw_node(self);
	let default_value = raw.outputs[0].default_value;
	f32a[0] = default_value[0];
	f32a[1] = default_value[1];
	f32a[2] = default_value[2];
	f32a[3] = default_value[3];
	self.image = image_from_bytes(f32a.buffer, 1, 1, tex_format_t.RGBA128);
	done(self.image);
}

function rgb_node_get_cached_image(self: rgb_node_t): image_t {
	self.base.get_as_image(self, 0, (img: image_t) => {});
	return self.image;
}

let rgb_node_def: zui_node_t = {
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
			default_value: new Float32Array([0.8, 0.8, 0.8, 1.0])
		}
	],
	buttons: [
		{
			name: _tr("default_value"),
			type: "RGBA",
			output: 0,
			default_value: new Float32Array([0.8, 0.8, 0.8, 1.0])
		}
	]
};
