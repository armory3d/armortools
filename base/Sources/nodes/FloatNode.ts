
class FloatNode extends LogicNode {

	value: f32;
	image: image_t = null;

	constructor(value = 0.0) {
		super();
		this.value = value;
	}

	override get = (from: i32, done: (a: any)=>void) => {
		if (this.inputs.length > 0) this.inputs[0].get(done);
		else done(this.value);
	}

	override getAsImage = (from: i32, done: (img: image_t)=>void) => {
		if (this.inputs.length > 0) { this.inputs[0].getAsImage(done); return; }
		if (this.image != null) image_unload(this.image);
		let b = new ArrayBuffer(16);
		let v = new DataView(b);
		v.setFloat32(0, this.value, true);
		v.setFloat32(4, this.value, true);
		v.setFloat32(8, this.value, true);
		v.setFloat32(12, 1.0, true);
		this.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
		done(this.image);
	}

	override set = (value: any) => {
		if (this.inputs.length > 0) this.inputs[0].set(value);
		else this.value = value;
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Value"),
		type: "FloatNode",
		x: 0,
		y: 0,
		color: 0xffb34f5a,
		inputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5,
				min: 0.0,
				max: 10.0
			}
		],
		outputs: [
			{
				id: 0,
				node_id: 0,
				name: _tr("Value"),
				type: "VALUE",
				color: 0xffa1a1a1,
				default_value: 0.5
			}
		],
		buttons: []
	};
}
