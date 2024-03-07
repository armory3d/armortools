
class File {

	///if krom_windows
	static cmd_mkdir: string = "mkdir";
	static cmd_copy: string = "copy";
	///else
	static cmd_mkdir: string = "mkdir -p";
	static cmd_copy: string = "cp";
	///end

	static cloud: Map<string, string[]> = null;
	static cloud_sizes: Map<string, i32> = null;

	// ///if krom_android
	// static let internal: Map<string, string[]> = null; // .apk contents
	// ///end

	static read_directory = (path: string, foldersOnly: bool = false): string[] => {
		if (path.startsWith("cloud")) {
			let files: string[] = File.cloud != null ? File.cloud.get(string_replace_all(path, "\\", "/")) : null;
			return files != null ? files : [];
		}
		// ///if krom_android
		// path = string_replace_all(path, "//", "/");
		// if (internal == null) {
		// 	internal = [];
		// 	internal.set("/data/plugins", BuildMacros.readDirectory("krom/data/plugins"));
		// 	internal.set("/data/export_presets", BuildMacros.readDirectory("krom/data/export_presets"));
		// 	internal.set("/data/keymap_presets", BuildMacros.readDirectory("krom/data/keymap_presets"));
		// 	internal.set("/data/locale", BuildMacros.readDirectory("krom/data/locale"));
		// 	internal.set("/data/meshes", BuildMacros.readDirectory("krom/data/meshes"));
		// 	internal.set("/data/themes", BuildMacros.readDirectory("krom/data/themes"));
		// }
		// if (internal.exists(path)) return internal.get(path);
		// ///end
		return krom_read_directory(path, foldersOnly).split("\n");
	}

	static create_directory = (path: string) => {
		krom_sys_command(File.cmd_mkdir + ' "' + path + '"');
	}

	static copy = (srcPath: string, dstPath: string) => {
		krom_sys_command(File.cmd_copy + ' "' + srcPath + '" "' + dstPath + '"');
	}

	static start = (path: string) => {
		///if krom_windows
		krom_sys_command('start "" "' + path + '"');
		///elseif krom_linux
		krom_sys_command('xdg-open "' + path + '"');
		///else
		krom_sys_command('open "' + path + '"');
		///end
	}

	static load_url = (url: string) => {
		krom_load_url(url);
	}

	static delete = (path: string) => {
		krom_delete_file(path);
	}

	static exists = (path: string): bool => {
		return krom_file_exists(path);
	}

	static download = (url: string, dstPath: string, done: ()=>void, size: i32 = 0) => {
		///if (krom_windows || krom_darwin || krom_ios || krom_android)
		krom_http_request(url, size, (ab: ArrayBuffer) => {
			if (ab != null) krom_file_save_bytes(dstPath, ab);
			done();
		});
		///elseif krom_linux
		krom_sys_command('wget -O "' + dstPath + '" "' + url + '"');
		done();
		///else
		krom_sys_command('curl -L ' + url + ' -o "' + dstPath + '"');
		done();
		///end
	}

	static download_bytes = (url: string, done: (ab: ArrayBuffer)=>void) => {
		let save: string = (Path.is_protected() ? krom_save_path() : Path.data() + Path.sep) + "download.bin";
		File.download(url, save, () => {
			let buffer: ArrayBuffer = null;
			try {
				buffer = krom_load_blob(save);
			}
			catch (e: any) {}
			done(buffer);
		});
	}

	static cache_cloud = (path: string, done: (s: string)=>void) => {
		///if krom_ios
		let path2: string = string_replace_all(path, "/", "_"); // Cache everything into root folder
		///else
		let path2: string = path;
		///end
		let dest: string = (Path.is_protected() ? krom_save_path() : krom_get_files_location() + Path.sep) + path2;
		if (File.exists(dest)) {
			///if (krom_darwin || krom_ios)
			done(dest);
			///else
			done((Path.is_protected() ? krom_save_path() : Path.working_dir() + Path.sep) + path);
			///end
			return;
		}

		let fileDir: string = dest.substr(0, dest.lastIndexOf(Path.sep));
		if (File.read_directory(fileDir)[0] == "") {
			File.create_directory(fileDir);
		}
		///if krom_windows
		path = string_replace_all(path, "\\", "/");
		///end
		let url: string = Config.raw.server + "/" + path;
		File.download(url, dest, () => {
			if (!File.exists(dest)) {
				Console.error(Strings.error5());
				done(null);
				return;
			}
			///if (krom_darwin || krom_ios)
			done(dest);
			///else
			done((Path.is_protected() ? krom_save_path() : Path.working_dir() + Path.sep) + path);
			///end
		}, File.cloud_sizes.get(path));
	}

	static init_cloud_bytes = (done: ()=>void, append: string = "") => {
		File.download_bytes(Config.raw.server + "/?list-type=2" + append, (buffer: ArrayBuffer) => {
			if (buffer == null) {
				File.cloud.set("cloud", []);
				Console.error(Strings.error5());
				return;
			}
			let files: string[] = [];
			let sizes: i32[] = [];

			let str: string = sys_buffer_to_string(buffer);
			let pos_start: i32 = 0;
			let pos_end: i32 = 0;

			while (true) {
				pos_start = str.indexOf("<Key>", pos_start);
				if (pos_start == -1) break;
				pos_start += 5; // <Key>
				pos_end = str.indexOf("</Key>", pos_start);

				files.push(str.substring(pos_start, pos_end));

				pos_start = str.indexOf("<Size>", pos_end);
				pos_start += 6; //<Size>
				pos_end = str.indexOf("</Size>", pos_start);

				sizes.push(Number(str.substring(pos_start, pos_end)));
			}

			for (let file of files) {
				if (Path.is_folder(file)) {
					File.cloud.set(file.substr(0, file.length - 1), []);
				}
			}
			for (let i: i32 = 0; i < files.length; ++i) {
				let file: string = files[i];
				let nested: bool = file.indexOf("/") != file.lastIndexOf("/");
				if (nested) {
					let delim: i32 = Path.is_folder(file) ? file.substr(0, file.length - 1).lastIndexOf("/") : file.lastIndexOf("/");
					let parent: string = file.substr(0, delim);
					let child: string = Path.is_folder(file) ? file.substring(delim + 1, file.length - 1) : file.substr(delim + 1);
					File.cloud.get(parent).push(child);
					if (!Path.is_folder(file)) {
						File.cloud_sizes.set(file, sizes[i]);
					}
				}
			}

			let isTruncated: bool = str.indexOf("<IsTruncated>true") > -1;
			if (isTruncated) {
				let pos_start: i32 = str.indexOf("<NextContinuationToken>");
				pos_start += 23;
				let pos_end: i32 = str.indexOf("</NextContinuationToken>", pos_start);
				File.init_cloud_bytes(done, "&start-after=" + str.substring(pos_start, pos_end));
			}
			else done();
		});
	}

	static init_cloud = (done: ()=>void) => {
		File.cloud = new Map();
		File.cloud_sizes = new Map();
		File.init_cloud_bytes(done);
	}
}
