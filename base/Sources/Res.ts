
class Res {

	static bundled: Map<string, image_t> = new Map();

	static load = (names: string[], done: ()=>void) => {
		let loaded: i32 = 0;
		for (let s of names) {
			let image: image_t = data_get_image(s);
			Res.bundled.set(s, image);
			loaded++;
			if (loaded == names.length) done();
		}
	}

	static get = (name: string): image_t => {
		return Res.bundled.get(name);
	}

	static tile50 = (img: image_t, x: i32, y: i32): rect_t => {
		let size: i32 = Config.raw.window_scale > 1 ? 100 : 50;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	static tile25 = (img: image_t, x: i32, y: i32): rect_t => {
		let size: i32 = Config.raw.window_scale > 1 ? 50 : 25;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	static tile18 = (img: image_t, x: i32, y: i32): rect_t => {
		let size: i32 = Config.raw.window_scale > 1 ? 36 : 18;
		return { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
	}

	///if arm_snapshot
	static embed_raw = (handle: string, name: string, file: ArrayBuffer) => {
		data_cached_blobs.set(name, file);
		data_get_scene_raw(handle, (_) => {});
		data_cached_blobs.delete(name);
	}

	static embed_blob = (name: string, file: ArrayBuffer) => {
		data_cached_blobs.set(name, file);
	}

	static embed_font = (name: string, file: ArrayBuffer) => {
		data_cached_fonts.set(name, g2_font_create(file));
	}
	///end
}

type rect_t = {
	x?: i32;
	y?: i32;
	w?: i32;
	h?: i32;
};
