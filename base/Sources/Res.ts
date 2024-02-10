
class Res {

	static bundled: Map<string, image_t> = new Map();

	static load = (names: string[], done: ()=>void) => {
		let loaded = 0;
		for (let s of names) {
			data_get_image(s, (image: image_t) => {
				Res.bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	static get = (name: string): image_t => {
		return Res.bundled.get(name);
	}

	static tile50 = (img: image_t, x: i32, y: i32): TRect => {
		let size = Config.raw.window_scale > 1 ? 100 : 50;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	static tile25 = (img: image_t, x: i32, y: i32): TRect => {
		let size = Config.raw.window_scale > 1 ? 50 : 25;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	static tile18 = (img: image_t, x: i32, y: i32): TRect => {
		let size = Config.raw.window_scale > 1 ? 36 : 18;
		return { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
	}

	///if arm_snapshot
	static embedRaw = (handle: string, name: string, file: ArrayBuffer) => {
		data_cached_blobs.set(name, file);
		data_get_scene_raw(handle, (_) => {});
		data_cached_blobs.delete(name);
	}

	static embedBlob = (name: string, file: ArrayBuffer) => {
		data_cached_blobs.set(name, file);
	}

	static embedFont = (name: string, file: ArrayBuffer) => {
		data_cached_fonts.set(name, g2_font_create(file));
	}
	///end
}

type TRect = {
	x: i32;
	y: i32;
	w: i32;
	h: i32;
}
