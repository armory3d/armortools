
///if krom_windows
let file_cmd_mkdir: string = "mkdir";
let file_cmd_copy: string = "copy";
///else
let file_cmd_mkdir: string = "mkdir -p";
let file_cmd_copy: string = "cp";
///end

let file_cloud: map_t<string, string[]> = null;
let file_cloud_sizes: map_t<string, i32> = null;

// ///if krom_android
// let let file_internal: map_t<string, string[]> = null; // .apk contents
// ///end

function file_read_directory(path: string, folders_only: bool = false): string[] {
	if (path.startsWith("cloud")) {
		let files: string[] = file_cloud != null ? file_cloud.get(string_replace_all(path, "\\", "/")) : null;
		return files != null ? files : [];
	}
	// ///if krom_android
	// path = string_replace_all(path, "//", "/");
	// if (file_internal == null) {
	// 	file_internal = [];
	// 	file_internal.set("/data/plugins", BuildMacros.readDirectory("krom/data/plugins"));
	// 	file_internal.set("/data/export_presets", BuildMacros.readDirectory("krom/data/export_presets"));
	// 	file_internal.set("/data/keymap_presets", BuildMacros.readDirectory("krom/data/keymap_presets"));
	// 	file_internal.set("/data/locale", BuildMacros.readDirectory("krom/data/locale"));
	// 	file_internal.set("/data/meshes", BuildMacros.readDirectory("krom/data/meshes"));
	// 	file_internal.set("/data/themes", BuildMacros.readDirectory("krom/data/themes"));
	// }
	// if (file_internal.exists(path)) return file_internal.get(path);
	// ///end
	return krom_read_directory(path, folders_only).split("\n");
}

function file_create_directory(path: string) {
	krom_sys_command(file_cmd_mkdir + ' "' + path + '"');
}

function file_copy(srcPath: string, dst_path: string) {
	krom_sys_command(file_cmd_copy + ' "' + srcPath + '" "' + dst_path + '"');
}

function file_start(path: string) {
	///if krom_windows
	krom_sys_command('start "" "' + path + '"');
	///elseif krom_linux
	krom_sys_command('xdg-open "' + path + '"');
	///else
	krom_sys_command('open "' + path + '"');
	///end
}

function file_load_url(url: string) {
	krom_load_url(url);
}

function file_delete(path: string) {
	krom_delete_file(path);
}

function file_exists(path: string): bool {
	return krom_file_exists(path);
}

function file_download(url: string, dstPath: string, done: ()=>void, size: i32 = 0) {
	///if (krom_windows || krom_darwin || krom_ios || krom_android)
	krom_http_request(url, size, function (ab: ArrayBuffer) {
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

function file_download_bytes(url: string, done: (ab: ArrayBuffer)=>void) {
	let save: string = (path_is_protected() ? krom_save_path() : path_data() + path_sep) + "download.bin";
	file_download(url, save, function() {
		let buffer: ArrayBuffer = null;
		try {
			buffer = krom_load_blob(save);
		}
		catch (e: any) {}
		done(buffer);
	});
}

function file_cache_cloud(path: string, done: (s: string)=>void) {
	///if krom_ios
	let path2: string = string_replace_all(path, "/", "_"); // Cache everything into root folder
	///else
	let path2: string = path;
	///end
	let dest: string = (path_is_protected() ? krom_save_path() : krom_get_files_location() + path_sep) + path2;
	if (file_exists(dest)) {
		///if (krom_darwin || krom_ios)
		done(dest);
		///else
		done((path_is_protected() ? krom_save_path() : path_working_dir() + path_sep) + path);
		///end
		return;
	}

	let file_dir: string = dest.substr(0, dest.lastIndexOf(path_sep));
	if (file_read_directory(file_dir)[0] == "") {
		file_create_directory(file_dir);
	}
	///if krom_windows
	path = string_replace_all(path, "\\", "/");
	///end
	let url: string = config_raw.server + "/" + path;
	file_download(url, dest, function() {
		if (!file_exists(dest)) {
			console_error(strings_error5());
			done(null);
			return;
		}
		///if (krom_darwin || krom_ios)
		done(dest);
		///else
		done((path_is_protected() ? krom_save_path() : path_working_dir() + path_sep) + path);
		///end
	}, file_cloud_sizes.get(path));
}

function file_init_cloud_bytes(done: ()=>void, append: string = "") {
	file_download_bytes(config_raw.server + "/?list-type=2" + append, function (buffer: ArrayBuffer) {
		if (buffer == null) {
			file_cloud.set("cloud", []);
			console_error(strings_error5());
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
			if (path_is_folder(file)) {
				file_cloud.set(file.substr(0, file.length - 1), []);
			}
		}
		for (let i: i32 = 0; i < files.length; ++i) {
			let file: string = files[i];
			let nested: bool = file.indexOf("/") != file.lastIndexOf("/");
			if (nested) {
				let delim: i32 = path_is_folder(file) ? file.substr(0, file.length - 1).lastIndexOf("/") : file.lastIndexOf("/");
				let parent: string = file.substr(0, delim);
				let child: string = path_is_folder(file) ? file.substring(delim + 1, file.length - 1) : file.substr(delim + 1);
				file_cloud.get(parent).push(child);
				if (!path_is_folder(file)) {
					file_cloud_sizes.set(file, sizes[i]);
				}
			}
		}

		let is_truncated: bool = str.indexOf("<IsTruncated>true") > -1;
		if (is_truncated) {
			let pos_start: i32 = str.indexOf("<NextContinuationToken>");
			pos_start += 23;
			let pos_end: i32 = str.indexOf("</NextContinuationToken>", pos_start);
			file_init_cloud_bytes(done, "&start-after=" + str.substring(pos_start, pos_end));
		}
		else done();
	});
}

function file_init_cloud(done: ()=>void) {
	file_cloud = map_create();
	file_cloud_sizes = map_create();
	file_init_cloud_bytes(done);
}
