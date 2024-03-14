
type color_node_t = {
	base?: logic_node_t;
	value?: vec4_t;
	image?: image_t;
};

function color_node_create(r: f32 = 0.8, g: f32 = 0.8, b: f32 = 0.8, a: f32 = 1.0): color_node_t {
	let n: color_node_t = {};
	n.base = logic_node_create();
	n.base.get = color_node_get;
	n.base.get_as_image = color_node_get_as_image;
	n.base.set = color_node_set;
	n.value = vec4_create(r, g, b, a);
	return n;
}

function color_node_get(self: color_node_t, from: i32, done: (a: any)=>void) {
	if (self.base.inputs.length > 0) logic_node_input_get(self.base.inputs[0], done);
	else done(self.value);
}

function color_node_get_as_image(self: color_node_t, from: i32, done: (img: image_t)=>void) {
	if (self.base.inputs.length > 0) { logic_node_input_get_as_image(self.base.inputs[0], done); return; }
	if (self.image != null) image_unload(self.image);
	let b: ArrayBuffer = new ArrayBuffer(16);
	let v: DataView = new DataView(b);
	v.setFloat32(0, self.value.x, true);
	v.setFloat32(4, self.value.y, true);
	v.setFloat32(8, self.value.z, true);
	v.setFloat32(12, self.value.w, true);
	self.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
	done(self.image);
}

function color_node_set(self: color_node_t, value: any) {
	if (self.base.inputs.length > 0) logic_node_input_set(self.base.inputs[0], value);
	else self.value = value;
}
