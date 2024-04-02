
///if krom_windows
let path_sep: string = "\\";
///else
let path_sep: string = "/";
///end

///if krom_windows
let path_pwd: string = "cd";
///else
let path_pwd: string = "echo $PWD";
///end

let path_mesh_formats: string[] = ["obj", "blend"];
let path_texture_formats: string[] = ["jpg", "jpeg", "png", "tga", "bmp", "psd", "gif", "hdr", "k"];

let path_mesh_importers: map_t<string, (s: string)=>any> = map_create();
let path_texture_importers: map_t<string, (s: string)=>image_t> = map_create();

let path_base_color_ext: string[] = ["albedo", "alb", "basecol", "basecolor", "diffuse", "diff", "base", "bc", "d", "color", "col"];
let path_opacity_ext: string[] = ["opac", "opacity", "alpha"];
let path_normal_map_ext: string[] = ["normal", "nor", "n", "nrm", "normalgl"];
let path_occlusion_ext: string[] = ["ao", "occlusion", "ambientOcclusion", "o", "occ"];
let path_roughness_ext: string[] = ["roughness", "rough", "r", "rgh"];
let path_metallic_ext: string[] = ["metallic", "metal", "metalness", "m", "met"];
let path_displacement_ext: string[] = ["displacement", "height", "h", "disp"];

let path_working_dir_cache: string = null;

function path_data(): string {
	return krom_get_files_location() + path_sep + data_path();
}

function path_to_relative(from: string, to: string): string {
	let a: string[] = string_split(from, path_sep);
	let b: string[] = string_split(to, path_sep);
	while (a[0] == b[0]) {
		a.shift();
		b.shift();
		if (a.length == 0 || b.length == 0) {
			break;
		}
	}
	let base: string = "";
	for (let i: i32 = 0; i < a.length - 1; ++i) {
		base += ".." + path_sep;
	}
	base += b.join(path_sep);
	return base;
}

function path_normalize(path: string): string {
	let ar: string[] = string_split(path, path_sep);
	let i: i32 = 0;
	while (i < ar.length) {
		if (i > 0 && ar[i] == ".." && ar[i - 1] != "..") {
			array_splice(ar, i - 1, 2);
			i--;
		}
		else {
			i++;
		}
	}
	return ar.join(path_sep);
}

function path_base_dir(path: string): string {
	return substring(path, 0, string_last_index_of(path, path_sep) + 1);
}

function path_working_dir(): string {
	if (path_working_dir_cache == null) {
		let cmd: string = path_pwd;
		let save: string = (path_is_protected() ? krom_save_path() : path_data() + path_sep) + "working_dir.txt";
		krom_sys_command(cmd + " > \"" + save + "\"");
		path_working_dir_cache = trim_end(sys_buffer_to_string(krom_load_blob(save)));
	}
	return path_working_dir_cache;
}

function path_is_mesh(path: string): bool {
	let p: string = to_lower_case(path);
	for (let i: i32 = 0; i < path_mesh_formats.length; ++i) {
		let s: string = path_mesh_formats[i];
		if (ends_with(p, "." + s)) {
			return true;
		}
	}
	return false;
}

function path_is_texture(path: string): bool {
	let p: string = to_lower_case(path);
	for (let i: i32 = 0; i < path_texture_formats.length; ++i) {
		let s: string = path_texture_formats[i];
		if (ends_with(p, "." + s)) {
			return true;
		}
	}
	return false;
}

function path_is_font(path: string): bool {
	let p: string = to_lower_case(path);
	return ends_with(p, ".ttf") ||
		   ends_with(p, ".ttc") ||
		   ends_with(p, ".otf");
}

function path_is_project(path: string): bool {
	let p: string = to_lower_case(path);
	return ends_with(p, ".arm");
}

function path_is_plugin(path: string): bool {
	let p: string = to_lower_case(path);
	return ends_with(p, ".js");
}

function path_is_json(path: string): bool {
	let p: string = to_lower_case(path);
	return ends_with(p, ".json");
}

function path_is_text(path: string): bool {
	let p: string = to_lower_case(path);
	return ends_with(p, ".txt");
}

function path_is_gimp_color_palette(path: string): bool {
	let p: string = to_lower_case(path);
	return ends_with(p, ".gpl");
}

function path_is_known(path: string): bool {
	return path_is_mesh(path) || path_is_texture(path) || path_is_font(path) || path_is_project(path) || path_is_plugin(path) || path_is_text(path) || path_is_gimp_color_palette(path);
}

function path_check_ext(p: string, exts: string[]): bool {
	p = string_replace_all(p, "-", "_");
	for (let i: i32 = 0; i < exts.length; ++i) {
		let ext: string = exts[i];
		if (ends_with(p, "_" + ext) ||
			(string_index_of(p, "_" + ext + "_") >= 0 && !ends_with(p, "_preview") && !ends_with(p, "_icon"))) {
			return true;
		}
	}
	return false;
}

function path_is_base_color_tex(p: string): bool {
	return path_check_ext(p, path_base_color_ext);
}
function path_is_opacity_tex(p: string): bool {
	return path_check_ext(p, path_opacity_ext);
}
function path_is_normal_map_tex(p: string): bool {
	return path_check_ext(p, path_normal_map_ext);
}
function path_is_occlusion_tex(p: string): bool {
	return path_check_ext(p, path_occlusion_ext);
}
function path_is_roughness_tex(p: string): bool {
	return path_check_ext(p, path_roughness_ext);
}
function path_is_metallic_tex(p: string): bool {
	return path_check_ext(p, path_metallic_ext);
}
function path_is_displacement_tex(p: string): bool {
	return path_check_ext(p, path_displacement_ext);
}

function path_is_folder(p: string): bool {
	return string_index_of(string_split(string_replace_all(p, "\\", "/"), "/").pop(), ".") == -1;
}

function path_is_protected(): bool {
	///if krom_windows
	return string_index_of(krom_get_files_location(), "Program Files") >= 0;
	///elseif krom_android
	return true;
	///elseif krom_ios
	return true;
	///else
	return false;
	///end
}
