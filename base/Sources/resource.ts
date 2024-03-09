
let resource_bundled: Map<string, image_t> = new Map();

function resource_load(names: string[], done: ()=>void) {
	let loaded: i32 = 0;
	for (let s of names) {
		let image: image_t = data_get_image(s);
		resource_bundled.set(s, image);
		loaded++;
		if (loaded == names.length) done();
	}
}

function resource_get(name: string): image_t {
	return resource_bundled.get(name);
}

function resource_tile50(img: image_t, x: i32, y: i32): rect_t {
	let size: i32 = config_raw.window_scale > 1 ? 100 : 50;
	return { x: x * size, y: y * size, w: size, h: size };
}

function resource_tile25(img: image_t, x: i32, y: i32): rect_t {
	let size: i32 = config_raw.window_scale > 1 ? 50 : 25;
	return { x: x * size, y: y * size, w: size, h: size };
}

function resource_tile18(img: image_t, x: i32, y: i32): rect_t {
	let size: i32 = config_raw.window_scale > 1 ? 36 : 18;
	return { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
}

///if arm_snapshot
function resource_embed_raw(handle: string, name: string, file: ArrayBuffer) {
	data_cached_blobs.set(name, file);
	data_get_scene_raw(handle);
	data_cached_blobs.delete(name);
}

function resource_embed_blob(name: string, file: ArrayBuffer) {
	data_cached_blobs.set(name, file);
}

function resource_embed_font(name: string, file: ArrayBuffer) {
	data_cached_fonts.set(name, g2_font_create(file));
}
///end

type rect_t = {
	x?: i32;
	y?: i32;
	w?: i32;
	h?: i32;
};
