
class ColorNode extends LogicNode {

	value: vec4_t = vec4_create();
	image: image_t = null;

	constructor(r: f32 = 0.8, g: f32 = 0.8, b: f32 = 0.8, a: f32 = 1.0) {
		super();
		vec4_set(this.value, r, g, b, a);
	}

	override get = (from: i32, done: (a: any)=>void) => {
		if (this.inputs.length > 0) this.inputs[0].get(done);
		else done(this.value);
	}

	override get_as_image = (from: i32, done: (img: image_t)=>void) => {
		if (this.inputs.length > 0) { this.inputs[0].get_as_image(done); return; }
		if (this.image != null) image_unload(this.image);
		let b: ArrayBuffer = new ArrayBuffer(16);
		let v: DataView = new DataView(b);
		v.setFloat32(0, this.value.x, true);
		v.setFloat32(4, this.value.y, true);
		v.setFloat32(8, this.value.z, true);
		v.setFloat32(12, this.value.w, true);
		this.image = image_from_bytes(b, 1, 1, tex_format_t.RGBA128);
		done(this.image);
	}

	override set = (value: any) => {
		if (this.inputs.length > 0) this.inputs[0].set(value);
		else this.value = value;
	}
}
