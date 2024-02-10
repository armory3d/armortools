
class Path {

	static get sep(): string {
		///if krom_windows
		return "\\";
		///else
		return "/";
		///end
	}

	static meshFormats = ["obj", "blend"];
	static textureFormats = ["jpg", "jpeg", "png", "tga", "bmp", "psd", "gif", "hdr", "k"];

	static meshImporters = new Map<string, (s: string, f: (a: any)=>void)=>void>();
	static textureImporters = new Map<string, (s: string, f: (img: image_t)=>void)=>void>();

	static baseColorExt = ["albedo", "alb", "basecol", "basecolor", "diffuse", "diff", "base", "bc", "d", "color", "col"];
	static opacityExt = ["opac", "opacity", "alpha"];
	static normalMapExt = ["normal", "nor", "n", "nrm", "normalgl"];
	static occlusionExt = ["ao", "occlusion", "ambientOcclusion", "o", "occ"];
	static roughnessExt = ["roughness", "rough", "r", "rgh"];
	static metallicExt = ["metallic", "metal", "metalness", "m", "met"];
	static displacementExt = ["displacement", "height", "h", "disp"];

	static workingDirCache: string = null;

	static data = (): string => {
		return krom_get_files_location() + Path.sep + data_path();
	}

	static toRelative = (from: string, to: string): string => {
		let a = from.split(Path.sep);
		let b = to.split(Path.sep);
		while (a[0] == b[0]) {
			a.shift();
			b.shift();
			if (a.length == 0 || b.length == 0) break;
		}
		let base = "";
		for (let i = 0; i < a.length - 1; ++i) base += ".." + Path.sep;
		base += b.join(Path.sep);
		return base;
	}

	static normalize = (path: string): string => {
		let ar = path.split(Path.sep);
		let i = 0;
		while (i < ar.length) {
			if (i > 0 && ar[i] == ".." && ar[i - 1] != "..") {
				ar.splice(i - 1, 2);
				i--;
			}
			else i++;
		}
		return ar.join(Path.sep);
	}

	static baseDir = (path: string): string => {
		return path.substr(0, path.lastIndexOf(Path.sep) + 1);
	}

	static get pwd(): string {
		///if krom_windows
		return "cd";
		///else
		return "echo $PWD";
		///end
	}

	static workingDir = (): string => {
		if (Path.workingDirCache == null) {
			let cmd = Path.pwd;
			let save = (Path.isProtected() ? krom_save_path() : Path.data() + Path.sep) + "working_dir.txt";
			krom_sys_command(cmd + ' > "' + save + '"');
			Path.workingDirCache = trim_end(sys_buffer_to_string(krom_load_blob(save)));
		}
		return Path.workingDirCache;
	}

	static isMesh = (path: string): bool => {
		let p = path.toLowerCase();
		for (let s of Path.meshFormats) if (p.endsWith("." + s)) return true;
		return false;
	}

	static isTexture = (path: string): bool => {
		let p = path.toLowerCase();
		for (let s of Path.textureFormats) if (p.endsWith("." + s)) return true;
		return false;
	}

	static isFont = (path: string): bool => {
		let p = path.toLowerCase();
		return p.endsWith(".ttf") ||
				p.endsWith(".ttc") ||
				p.endsWith(".otf");
	}

	static isProject = (path: string): bool => {
		let p = path.toLowerCase();
		return p.endsWith(".arm");
	}

	static isPlugin = (path: string): bool => {
		let p = path.toLowerCase();
		return p.endsWith(".js");
	}

	static isJson = (path: string): bool => {
		let p = path.toLowerCase();
		return p.endsWith(".json");
	}

	static isText = (path: string): bool => {
		let p = path.toLowerCase();
		return p.endsWith(".txt");
	}

	static isGimpColorPalette = (path: string): bool => {
		let p = path.toLowerCase();
		return p.endsWith(".gpl");
	}

	static isKnown = (path: string): bool => {
		return Path.isMesh(path) || Path.isTexture(path) || Path.isFont(path) || Path.isProject(path) || Path.isPlugin(path) || Path.isText(path) || Path.isGimpColorPalette(path);
	}

	static checkExt = (p: string, exts: string[]): bool => {
		p = p.replaceAll("-", "_");
		for (let ext of exts) {
			if (p.endsWith("_" + ext) ||
				(p.indexOf("_" + ext + "_") >= 0 && !p.endsWith("_preview") && !p.endsWith("_icon"))) {
				return true;
			}
		}
		return false;
	}

	static isBaseColorTex = (p: string): bool => {
		return Path.checkExt(p, Path.baseColorExt);
	}
	static isOpacityTex = (p: string): bool => {
		return Path.checkExt(p, Path.opacityExt);
	}
	static isNormalMapTex = (p: string): bool => {
		return Path.checkExt(p, Path.normalMapExt);
	}
	static isOcclusionTex = (p: string): bool => {
		return Path.checkExt(p, Path.occlusionExt);
	}
	static isRoughnessTex = (p: string): bool => {
		return Path.checkExt(p, Path.roughnessExt);
	}
	static isMetallicTex = (p: string): bool => {
		return Path.checkExt(p, Path.metallicExt);
	}
	static isDisplacementTex = (p: string): bool => {
		return Path.checkExt(p, Path.displacementExt);
	}

	static isFolder = (p: string): bool => {
		return p.replaceAll("\\", "/").split("/").pop().indexOf(".") == -1;
	}

	static isProtected = (): bool => {
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
