
class TEX_IMAGE extends LogicNode {

	file: string;
	color_space: string;

	constructor() {
		super();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		if (from == 0) done(this.file + ".rgb");
		else done(this.file + ".a");
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Image Texture"),
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
				default_value: new Float32Array([0.0, 0.0, 0.0])
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Color"),
				type: "VALUE", // Match brush output socket type
				color: 0xffc7c729,
				default_value: new Float32Array([0.0, 0.0, 0.0, 1.0])
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
}
