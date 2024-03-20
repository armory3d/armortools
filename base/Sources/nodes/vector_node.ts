
type vector_node_t = {
	base?: logic_node_t;
	value?: vec4_t;
	image?: image_t;
};

function vector_node_create(x: Null<f32> = null, y: Null<f32> = null, z: Null<f32> = null): vector_node_t {
	let n: vector_node_t = {};
	n.base = logic_node_create();
	n.base.get = vector_node_get;
	n.base.get_as_image = vector_node_get_as_image;
	n.base.set = vector_node_set;
	n.value = vec4_create();

	if (x != null) {
		logic_node_add_input(n.base, float_node_create(x).base, 0);
		logic_node_add_input(n.base, float_node_create(y).base, 0);
		logic_node_add_input(n.base, float_node_create(z).base, 0);
	}

	return n;
}

function vector_node_get(self: vector_node_t, from: i32, done: (a: any)=>void) {
	logic_node_input_get(self.base.inputs[0], function (x: f32) {
		logic_node_input_get(self.base.inputs[1], function (y: f32) {
			logic_node_input_get(self.base.inputs[2], function (z: f32) {
				self.value.x = x;
				self.value.y = y;
				self.value.z = z;
				done(self.value);
			});
		});
	});
}

function vector_node_get_as_image(self: vector_node_t, from: i32, done: (img: image_t)=>void) {
	logic_node_input_get(self.base.inputs[0], function (x: f32) {
		logic_node_input_get(self.base.inputs[1], function (y: f32) {
			logic_node_input_get(self.base.inputs[2], function (z: f32) {
				if (self.image != null) {
					image_unload(self.image);
				}
				let b: buffer_t = buffer_create(16);
				let v: buffer_view_t = buffer_view_create(b);
				buffer_view_set_f32(v, 0, (self.base.inputs[0].node as any).value);
				buffer_view_set_f32(v, 4, (self.base.inputs[1].node as any).value);
				buffer_view_set_f32(v, 8, (self.base.inputs[2].node as any).value);
				buffer_view_set_f32(v, 12, 1.0);
				self.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
				done(self.image);
			});
		});
	});
}

function vector_node_set(self: vector_node_t, value: any) {
	logic_node_input_set(self.base.inputs[0], value.x);
	logic_node_input_set(self.base.inputs[1], value.y);
	logic_node_input_set(self.base.inputs[2], value.z);
}

let vector_node_def: zui_node_t = {
	id: 0,
	name: _tr("Vector"),
	type: "vector_node",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("X"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Y"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Z"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 0.0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: new f32_array_t([0.0, 0.0, 0.0])
		}
	],
	buttons: []
};
