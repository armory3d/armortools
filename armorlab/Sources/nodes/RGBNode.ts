
// @:keep
class RGBNode extends LogicNode {

	image: Image = null;

	constructor() {
		super();
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		if (image != null) {
			Base.notifyOnNextFrame(() => {
				image.unload();
			});
		}

		let f32a = new Float32Array(4);
		let raw = ParserLogic.getRawNode(this);
		let default_value = raw.outputs[0].default_value;
		f32a[0] = default_value[0];
		f32a[1] = default_value[1];
		f32a[2] = default_value[2];
		f32a[3] = default_value[3];
		image = Image.fromBytes(f32a.buffer, 1, 1, TextureFormat.RGBA128);
		done(image);
	}

	override getCachedImage = (): Image => {
		getAsImage(0, (img: Image) => {});
		return image;
	}

	static def: zui.Zui.TNode = {
		id: 0,
		name: _tr("RGB"),
		type: "RGBNode",
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
}
