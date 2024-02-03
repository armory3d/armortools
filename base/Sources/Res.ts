
class Res {

	static bundled: Map<string, ImageRaw> = new Map();

	static load = (names: string[], done: ()=>void) => {
		let loaded = 0;
		for (let s of names) {
			Data.getImage(s, (image: ImageRaw) => {
				Res.bundled.set(s, image);
				loaded++;
				if (loaded == names.length) done();
			});
		}
	}

	static get = (name: string): ImageRaw => {
		return Res.bundled.get(name);
	}

	static tile50 = (img: ImageRaw, x: i32, y: i32): TRect => {
		let size = Config.raw.window_scale > 1 ? 100 : 50;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	static tile25 = (img: ImageRaw, x: i32, y: i32): TRect => {
		let size = Config.raw.window_scale > 1 ? 50 : 25;
		return { x: x * size, y: y * size, w: size, h: size };
	}

	static tile18 = (img: ImageRaw, x: i32, y: i32): TRect => {
		let size = Config.raw.window_scale > 1 ? 36 : 18;
		return { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
	}

	///if arm_snapshot
	static embedRaw = (handle: string, name: string, file: ArrayBuffer) => {
		Data.cachedBlobs.set(name, file);
		Data.getSceneRaw(handle, (_) => {});
		Data.cachedBlobs.delete(name);
	}

	static embedBlob = (name: string, file: ArrayBuffer) => {
		Data.cachedBlobs.set(name, file);
	}

	static embedFont = (name: string, file: ArrayBuffer) => {
		Data.cachedFonts.set(name, Font.create(file));
	}
	///end
}

type TRect = {
	x: i32;
	y: i32;
	w: i32;
	h: i32;
}
