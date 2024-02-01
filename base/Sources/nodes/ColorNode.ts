
class ColorNode extends LogicNode {

	value = Vec4.create();
	image: Image = null;

	constructor(r = 0.8, g = 0.8, b = 0.8, a = 1.0) {
		super();
		Vec4.set(this.value, r, g, b, a);
	}

	override get = (from: i32, done: (a: any)=>void) => {
		if (this.inputs.length > 0) this.inputs[0].get(done);
		else done(this.value);
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		if (this.inputs.length > 0) { this.inputs[0].getAsImage(done); return; }
		if (this.image != null) this.image.unload();
		let b = new ArrayBuffer(16);
		let v = new DataView(b);
		v.setFloat32(0, this.value.x, true);
		v.setFloat32(4, this.value.y, true);
		v.setFloat32(8, this.value.z, true);
		v.setFloat32(12, this.value.w, true);
		this.image = Image.fromBytes(b, 1, 1, TextureFormat.RGBA128);
		done(this.image);
	}

	override set = (value: any) => {
		if (this.inputs.length > 0) this.inputs[0].set(value);
		else this.value = value;
	}
}
