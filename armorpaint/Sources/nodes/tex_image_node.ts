
type tex_image_node_t = {
	base?: logic_node_t;
	file?: string;
	color_space?: string;
};

function tex_image_node_create(arg: any): tex_image_node_t {
	let n: tex_image_node_t = {};
	n.base = logic_node_create();
	n.base.get = tex_image_node_get;
	return n;
}

function tex_image_node_get(self: tex_image_node_t, from: i32, done: (a: any)=>void) {
	if (from == 0) {
		done(self.file + ".rgb");
	}
	else {
		done(self.file + ".a");
	}
}

let tex_image_node_def: zui_node_t = {
	id: 0,
	name: _tr("Image Texture"),
	type: "tex_image_node",
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
			default_value: new f32_array_t([0.0, 0.0, 0.0])
		}
	],
	outputs: [
		{
			id: 0,
			node_id: 0,
			name: _tr("Color"),
			type: "VALUE", // Match brush output socket type
			color: 0xffc7c729,
			default_value: new f32_array_t([0.0, 0.0, 0.0, 1.0])
		},
		{
			id: 0,
			node_id: 0,
			name: _tr("Alpha"),
			type: "VALUE",
			color: 0xffa1a1a1,
			default_value: 1.0
		}
	],
	buttons: [
		{
			name: _tr("file"),
			type: "ENUM",
			default_value: 0,
			data: ""
		},
		{
			name: _tr("color_space"),
			type: "ENUM",
			default_value: 0,
			data: ["linear", "srgb"]
		}
	]
};
