
// Global data list
let data_cached_scene_raws: map_t<string, scene_t>        = map_create();
let data_cached_meshes: map_t<string, mesh_data_t>        = map_create();
let data_cached_cameras: map_t<string, camera_data_t>     = map_create();
let data_cached_materials: map_t<string, material_data_t> = map_create();
let data_cached_worlds: map_t<string, world_data_t>       = map_create();
let data_cached_shaders: map_t<string, shader_data_t>     = map_create();

let data_cached_blobs: map_t<string, buffer_t>       = map_create();
let data_cached_images: map_t<string, gpu_texture_t> = map_create();
let data_cached_videos: map_t<string, video_t>       = map_create();
let data_cached_fonts: map_t<string, draw_font_t>    = map_create();
/// if arm_audio
let data_cached_sounds: map_t<string, sound_t> = map_create();
/// end
let data_assets_loaded: i32 = 0;

function data_get_mesh(file: string, name: string): mesh_data_t {
	let handle: string      = file + name;
	let cached: mesh_data_t = map_get(data_cached_meshes, handle);
	if (cached != null) {
		return cached;
	}

	let b: mesh_data_t = mesh_data_parse(file, name);
	map_set(data_cached_meshes, handle, b);
	b._.handle = handle;
	return b;
}

function data_get_camera(file: string, name: string): camera_data_t {
	let handle: string        = file + name;
	let cached: camera_data_t = map_get(data_cached_cameras, handle);
	if (cached != null) {
		return cached;
	}

	let b: camera_data_t = camera_data_parse(file, name);
	map_set(data_cached_cameras, handle, b);
	return b;
}

function data_get_material(file: string, name: string): material_data_t {
	let handle: string          = file + name;
	let cached: material_data_t = map_get(data_cached_materials, handle);
	if (cached != null) {
		return cached;
	}

	let b: material_data_t = material_data_parse(file, name);
	map_set(data_cached_materials, handle, b);
	return b;
}

function data_get_world(file: string, name: string): world_data_t {
	if (name == null) { // No world defined in scene
		return null;
	}

	let handle: string       = file + name;
	let cached: world_data_t = map_get(data_cached_worlds, handle);
	if (cached != null) {
		return cached;
	}

	let b: world_data_t = world_data_parse(file, name);
	map_set(data_cached_worlds, handle, b);
	return b;
}

function data_get_shader(file: string, name: string): shader_data_t {
	// Only one context override per shader data for now
	let handle: string        = name;
	let cached: shader_data_t = map_get(data_cached_shaders, handle); // Shader must have unique name
	if (cached != null) {
		return cached;
	}

	let b: shader_data_t = shader_data_parse(file, name);
	map_set(data_cached_shaders, handle, b);
	return b;
}

function data_get_scene_raw(file: string): scene_t {
	let cached: scene_t = map_get(data_cached_scene_raws, file);
	if (cached != null) {
		return cached;
	}

	// If no extension specified, set to .arm
	let ext: string     = ends_with(file, ".arm") ? "" : ".arm";
	let b: buffer_t     = data_get_blob(file + ext);
	let parsed: scene_t = armpack_decode(b);
	map_set(data_cached_scene_raws, file, parsed);
	return parsed;
}

// Raw assets
function data_get_blob(file: string): buffer_t {
	let cached: buffer_t = map_get(data_cached_blobs, file);
	if (cached != null) {
		return cached;
	}

	let b: buffer_t = iron_load_blob(data_resolve_path(file));
	map_set(data_cached_blobs, file, b);
	data_assets_loaded++;
	return b;
}

function data_get_image(file: string): gpu_texture_t {
	let cached: gpu_texture_t = map_get(data_cached_images, file);
	if (cached != null) {
		return cached;
	}

	let image_: any = iron_load_texture(data_resolve_path(file));
	if (image_ == null) {
		return null;
	}

	let b: gpu_texture_t = image_;
	map_set(data_cached_images, file, b);
	data_assets_loaded++;
	return b;
}

function data_get_video(file: string): video_t {
	file                = substring(file, 0, file.length - 4) + ".webm";
	let cached: video_t = map_get(data_cached_videos, file);
	if (cached != null) {
		return cached;
	}

	// let b: video_t = iron_load_video(data_resolve_path(file));
	// map_set(data_cached_videos, file, b);
	// data_assets_loaded++;
	// return b;
	return null;
}

function data_get_font(file: string): draw_font_t {
	let cached: draw_font_t = map_get(data_cached_fonts, file);
	if (cached != null) {
		return cached;
	}

	let blob: buffer_t = iron_load_blob(data_resolve_path(file));
	let b: draw_font_t = {buf : blob, index : 0};
	map_set(data_cached_fonts, file, b);
	data_assets_loaded++;
	return b;
}

/// if arm_audio
function data_get_sound(file: string): sound_t {
	let cached: sound_t = map_get(data_cached_sounds, file);
	if (cached != null) {
		return cached;
	}

	let b: sound_t = sound_create(iron_load_sound(data_resolve_path(file)));
	map_set(data_cached_sounds, file, b);
	data_assets_loaded++;
	return b;
}
/// end

function data_delete_mesh(handle: string) {
	let mesh: mesh_data_t = map_get(data_cached_meshes, handle);
	if (mesh == null) {
		return;
	}
	mesh_data_delete(mesh);
	map_delete(data_cached_meshes, handle);
}

function data_delete_blob(handle: string) {
	let blob: buffer_t = map_get(data_cached_blobs, handle);
	if (blob == null) {
		return;
	}
	map_delete(data_cached_blobs, handle);
}

function data_delete_image(handle: string) {
	let image: gpu_texture_t = map_get(data_cached_images, handle);
	if (image == null) {
		return;
	}
	gpu_delete_texture(image);
	map_delete(data_cached_images, handle);
}

function data_delete_video(handle: string) {
	let video: video_t = map_get(data_cached_videos, handle);
	if (video == null) {
		return;
	}
	video_unload(video);
	map_delete(data_cached_videos, handle);
}

function data_delete_font(handle: string) {
	let font: draw_font_t = map_get(data_cached_fonts, handle);
	if (font == null) {
		return;
	}
	draw_font_destroy(font);
	map_delete(data_cached_fonts, handle);
}

/// if arm_audio
function data_delete_sound(handle: string) {
	let sound: sound_t = map_get(data_cached_sounds, handle);
	if (sound == null) {
		return;
	}
	sound_unload(sound);
	map_delete(data_cached_sounds, handle);
}
/// end

function data_path(): string {
	/// if arm_android
	return "data" + PATH_SEP;
	/// else
	return "." + PATH_SEP + "data" + PATH_SEP;
	/// end
}

function data_is_abs(file: string): bool {
	return char_at(file, 0) == "/" || char_at(file, 1) == ":" || (char_at(file, 0) == "\\" && char_at(file, 1) == "\\");
}

function data_is_up(file: string): bool {
	return char_at(file, 0) == "." && char_at(file, 1) == ".";
}

function data_resolve_path(file: string): string {
	if (data_is_abs(file) || data_is_up(file)) {
		return file;
	}
	return data_path() + file;
}
