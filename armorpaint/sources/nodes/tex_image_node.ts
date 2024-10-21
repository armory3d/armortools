
type tex_image_node_t = {
	base?: logic_node_t;
	raw?: ui_node_t;
};

function tex_image_node_create(raw: ui_node_t, args: f32_array_t): tex_image_node_t {
	let n: tex_image_node_t = {};
	n.base = logic_node_create(n);
	n.base.get = tex_image_node_get;
	n.raw = raw;
	return n;
}

function tex_image_node_get(self: tex_image_node_t, from: i32): logic_node_value_t {
	let ar: string[] = ui_nodes_enum_texts(self.raw.type);
	let i: i32 = self.raw.buttons[0].default_value[0];
	let file: string = ar[i];

	if (from == 0) {
		let v: logic_node_value_t = { _str: file + ".rgb" };
		return v;
	}
	else {
		let v: logic_node_value_t = { _str: file + ".a" };
		return v;
	}
}

let tex_image_node_def: ui_node_t = {
	id: 0,
	name: _tr("Image Texture"),
	// type: "tex_image_node",
	type: "TEX_IMAGE",
	x: 0,
	y: 0,
	color: 0xff4982a0,
	inputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Vector"),
			type: "VECTOR",
			color: 0xff6363c7,
			default_value: f32_array_create_xyz(0.0, 0.0, 0.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "VALUE", // Match brush output socket type
			color: 0xffc7c729,
			default_value: f32_array_create_xyzw(0.0, 0.0, 0.0, 1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Alpha"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: f32_array_create_x(1.0),
			min: 0.0,
			max: 1.0,
			precision: 100,
			display: 0
		}
	],
	buttons: [
		{
			name: _tr("file"),
			type: "ENUM",
			output: -1,
			default_value: f32_array_create_x(0),
			data: u8_array_create_from_string(""),
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		},
		{
			name: _tr("color_space"),
			type: "ENUM",
			output: -1,
			default_value: f32_array_create_x(0),
			data: u8_array_create_from_string("linear\nsrgb"),
			min: 0.0,
			max: 1.0,
			precision: 100,
			height: 0
		}
	],
	width: 0
};
