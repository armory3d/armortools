
class VectorNode extends LogicNode {

	value = vec4_create();
	image: image_t = null;

	constructor(x: Null<f32> = null, y: Null<f32> = null, z: Null<f32> = null) {
		super();

		if (x != null) {
			this.addInput(new FloatNode(x), 0);
			this.addInput(new FloatNode(y), 0);
			this.addInput(new FloatNode(z), 0);
		}
	}

	override get = (from: i32, done: (a: any)=>void) => {
		this.inputs[0].get((x: f32) => {
			this.inputs[1].get((y: f32) => {
				this.inputs[2].get((z: f32) => {
					this.value.x = x;
					this.value.y = y;
					this.value.z = z;
					done(this.value);
				});
			});
		});
	}

	override getAsImage = (from: i32, done: (img: image_t)=>void) => {
		this.inputs[0].get((x: f32) => {
			this.inputs[1].get((y: f32) => {
				this.inputs[2].get((z: f32) => {
					if (this.image != null) image_unload(this.image);
					let b = new ArrayBuffer(16);
					let v = new DataView(b);
					v.setFloat32(0, (this.inputs[0].node as any).value, true);
					v.setFloat32(4, (this.inputs[1].node as any).value, true);
					v.setFloat32(8, (this.inputs[2].node as any).value, true);
					v.setFloat32(12, 1.0, true);
					this.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
					done(this.image);
				});
			});
		});
	}

	override set = (value: any) => {
		this.inputs[0].set(value.x);
		this.inputs[1].set(value.y);
		this.inputs[2].set(value.z);
	}

	static def: zui_node_t = {
		id: 0,
		name: _tr("Vector"),
		type: "VectorNode",
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
				default_value: new Float32Array([0.0, 0.0, 0.0])
			}
		],
		buttons: []
	};
}
