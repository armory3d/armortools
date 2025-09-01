
let resource_bundled: map_t<string, gpu_texture_t> = map_create();

function resource_load(names: string[]) {
	for (let i: i32 = 0; i < names.length; ++i) {
		let s: string = names[i];
		let image: gpu_texture_t = data_get_image(s);
		map_set(resource_bundled, s, image);
	}
}

function resource_get(name: string): gpu_texture_t {
	return map_get(resource_bundled, name);
}

function resource_tile50(img: gpu_texture_t, x: i32, y: i32): rect_t {
	let size: i32 = config_raw.window_scale > 1 ? 100 : 50;
	let r: rect_t = { x: x * size, y: y * size, w: size, h: size };
	return r;
}

function resource_tile25(img: gpu_texture_t, x: i32, y: i32): rect_t {
	let size: i32 = config_raw.window_scale > 1 ? 50 : 25;
	let r: rect_t = { x: x * size, y: y * size, w: size, h: size };
	return r;
}

function resource_tile18(img: gpu_texture_t, x: i32, y: i32): rect_t {
	let size: i32 = config_raw.window_scale > 1 ? 36 : 18;
	let r: rect_t = { x: x * size, y: img.height - (y + 1) * size, w: size, h: size };
	return r;
}

type rect_t = {
	x?: i32;
	y?: i32;
	w?: i32;
	h?: i32;
};
