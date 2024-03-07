
class Path {

	static get sep(): string {
		///if krom_windows
		return "\\";
		///else
		return "/";
		///end
	}

	static mesh_formats: string[] = ["obj", "blend"];
	static texture_formats: string[] = ["jpg", "jpeg", "png", "tga", "bmp", "psd", "gif", "hdr", "k"];

	static mesh_importers: Map<string, (s: string, f: (a: any)=>void)=>void> = new Map();
	static texture_importers: Map<string, (s: string, f: (img: image_t)=>void)=>void> = new Map();

	static base_color_ext: string[] = ["albedo", "alb", "basecol", "basecolor", "diffuse", "diff", "base", "bc", "d", "color", "col"];
	static opacity_ext: string[] = ["opac", "opacity", "alpha"];
	static normal_map_ext: string[] = ["normal", "nor", "n", "nrm", "normalgl"];
	static occlusion_ext: string[] = ["ao", "occlusion", "ambientOcclusion", "o", "occ"];
	static roughness_ext: string[] = ["roughness", "rough", "r", "rgh"];
	static metallic_ext: string[] = ["metallic", "metal", "metalness", "m", "met"];
	static displacement_ext: string[] = ["displacement", "height", "h", "disp"];

	static working_dir_cache: string = null;

	static data = (): string => {
		return krom_get_files_location() + Path.sep + data_path();
	}

	static to_relative = (from: string, to: string): string => {
		let a: string[] = from.split(Path.sep);
		let b: string[] = to.split(Path.sep);
		while (a[0] == b[0]) {
			a.shift();
			b.shift();
			if (a.length == 0 || b.length == 0) break;
		}
		let base: string = "";
		for (let i: i32 = 0; i < a.length - 1; ++i) base += ".." + Path.sep;
		base += b.join(Path.sep);
		return base;
	}

	static normalize = (path: string): string => {
		let ar: string[] = path.split(Path.sep);
		let i: i32 = 0;
		while (i < ar.length) {
			if (i > 0 && ar[i] == ".." && ar[i - 1] != "..") {
				ar.splice(i - 1, 2);
				i--;
			}
			else i++;
		}
		return ar.join(Path.sep);
	}

	static base_dir = (path: string): string => {
		return path.substr(0, path.lastIndexOf(Path.sep) + 1);
	}

	static get pwd(): string {
		///if krom_windows
		return "cd";
		///else
		return "echo $PWD";
		///end
	}

	static working_dir = (): string => {
		if (Path.working_dir_cache == null) {
			let cmd: string = Path.pwd;
			let save: string = (Path.is_protected() ? krom_save_path() : Path.data() + Path.sep) + "working_dir.txt";
			krom_sys_command(cmd + ' > "' + save + '"');
			Path.working_dir_cache = trim_end(sys_buffer_to_string(krom_load_blob(save)));
		}
		return Path.working_dir_cache;
	}

	static is_mesh = (path: string): bool => {
		let p: string = path.toLowerCase();
		for (let s of Path.mesh_formats) if (p.endsWith("." + s)) return true;
		return false;
	}

	static is_texture = (path: string): bool => {
		let p: string = path.toLowerCase();
		for (let s of Path.texture_formats) if (p.endsWith("." + s)) return true;
		return false;
	}

	static is_font = (path: string): bool => {
		let p: string = path.toLowerCase();
		return p.endsWith(".ttf") ||
				p.endsWith(".ttc") ||
				p.endsWith(".otf");
	}

	static is_project = (path: string): bool => {
		let p: string = path.toLowerCase();
		return p.endsWith(".arm");
	}

	static is_plugin = (path: string): bool => {
		let p: string = path.toLowerCase();
		return p.endsWith(".js");
	}

	static is_json = (path: string): bool => {
		let p: string = path.toLowerCase();
		return p.endsWith(".json");
	}

	static is_text = (path: string): bool => {
		let p: string = path.toLowerCase();
		return p.endsWith(".txt");
	}

	static is_gimp_color_palette = (path: string): bool => {
		let p: string = path.toLowerCase();
		return p.endsWith(".gpl");
	}

	static is_known = (path: string): bool => {
		return Path.is_mesh(path) || Path.is_texture(path) || Path.is_font(path) || Path.is_project(path) || Path.is_plugin(path) || Path.is_text(path) || Path.is_gimp_color_palette(path);
	}

	static check_ext = (p: string, exts: string[]): bool => {
		p = string_replace_all(p, "-", "_");
		for (let ext of exts) {
			if (p.endsWith("_" + ext) ||
				(p.indexOf("_" + ext + "_") >= 0 && !p.endsWith("_preview") && !p.endsWith("_icon"))) {
				return true;
			}
		}
		return false;
	}

	static is_base_color_tex = (p: string): bool => {
		return Path.check_ext(p, Path.base_color_ext);
	}
	static is_opacity_tex = (p: string): bool => {
		return Path.check_ext(p, Path.opacity_ext);
	}
	static is_normal_map_tex = (p: string): bool => {
		return Path.check_ext(p, Path.normal_map_ext);
	}
	static is_occlusion_tex = (p: string): bool => {
		return Path.check_ext(p, Path.occlusion_ext);
	}
	static is_roughness_tex = (p: string): bool => {
		return Path.check_ext(p, Path.roughness_ext);
	}
	static is_metallic_tex = (p: string): bool => {
		return Path.check_ext(p, Path.metallic_ext);
	}
	static is_displacement_tex = (p: string): bool => {
		return Path.check_ext(p, Path.displacement_ext);
	}

	static is_folder = (p: string): bool => {
		return string_replace_all(p, "\\", "/").split("/").pop().indexOf(".") == -1;
	}

	static is_protected = (): bool => {
		///if krom_windows
		return krom_get_files_location().indexOf("Program Files") >= 0;
		///elseif krom_android
		return true;
		///elseif krom_ios
		return true;
		///else
		return false;
		///end
	}
}
