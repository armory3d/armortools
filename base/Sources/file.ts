
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
	if (starts_with(path, "cloud")) {
		let files: string[] = file_cloud != null ? map_get(file_cloud, string_replace_all(path, "\\", "/")) : null;
		return files != null ? files : [];
	}
	// ///if krom_android
	// path = string_replace_all(path, "//", "/");
	// if (file_internal == null) {
	// 	file_internal = [];
	// 	map_set(file_internal, "/data/plugins", BuildMacros.readDirectory("krom/data/plugins"));
	// 	map_set(file_internal, "/data/export_presets", BuildMacros.readDirectory("krom/data/export_presets"));
	// 	map_set(file_internal, "/data/keymap_presets", BuildMacros.readDirectory("krom/data/keymap_presets"));
	// 	map_set(file_internal, "/data/locale", BuildMacros.readDirectory("krom/data/locale"));
	// 	map_set(file_internal, "/data/meshes", BuildMacros.readDirectory("krom/data/meshes"));
	// 	map_set(file_internal, "/data/themes", BuildMacros.readDirectory("krom/data/themes"));
	// }
	// if (file_internal.exists(path)) return map_get(file_internal, path);
	// ///end
	return string_split(krom_read_directory(path, folders_only), "\n");
}

function file_create_directory(path: string) {
	krom_sys_command(file_cmd_mkdir + " \"" + path + "\"");
}

function file_copy(srcPath: string, dst_path: string) {
	krom_sys_command(file_cmd_copy + " \"" + srcPath + "\" \"" + dst_path + "\"");
}

function file_start(path: string) {
	///if krom_windows
	krom_sys_command("start \"\" \"" + path + "\"");
	///elseif krom_linux
	krom_sys_command("xdg-open \"" + path + "\"");
	///else
	krom_sys_command("open \"" + path + "\"");
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
	krom_http_request(url, size, function (ab: buffer_t) {
		if (ab != null) {
			krom_file_save_bytes(dstPath, ab);
		}
		done();
	});
	///elseif krom_linux
	krom_sys_command("wget -O \"" + dstPath + "\" \"" + url + "\"");
	done();
	///else
	krom_sys_command("curl -L " + url + " -o \"" + dstPath + "\"");
	done();
	///end
}

function file_download_bytes(url: string, done: (ab: buffer_t)=>void) {
	let save: string = (path_is_protected() ? krom_save_path() : path_data() + path_sep) + "download.bin";
	file_download(url, save, function () {
		let buffer: buffer_t = krom_load_blob(save);
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

	let file_dir: string = substring(dest, 0, string_last_index_of(dest, path_sep));
	if (file_read_directory(file_dir)[0] == "") {
		file_create_directory(file_dir);
	}
	///if krom_windows
	path = string_replace_all(path, "\\", "/");
	///end
	let url: string = config_raw.server + "/" + path;
	file_download(url, dest, function () {
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
	}, map_get(file_cloud_sizes, path));
}

function file_init_cloud_bytes(done: ()=>void, append: string = "") {
	file_download_bytes(config_raw.server + "/?list-type=2" + append, function (buffer: buffer_t) {
		if (buffer == null) {
			map_set(file_cloud, "cloud", []);
			console_error(strings_error5());
			return;
		}
		let files: string[] = [];
		let sizes: i32[] = [];

		let str: string = sys_buffer_to_string(buffer);
		let pos_start: i32 = 0;
		let pos_end: i32 = 0;

		while (true) {
			pos_start = string_index_of_pos(str, "<Key>", pos_start);
			if (pos_start == -1) {
				break;
			}
			pos_start += 5; // <Key>
			pos_end = string_index_of_pos(str, "</Key>", pos_start);

			array_push(files, substring(str, pos_start, pos_end));

			pos_start = string_index_of_pos(str, "<Size>", pos_end);
			pos_start += 6; //<Size>
			pos_end = string_index_of_pos(str, "</Size>", pos_start);

			array_push(sizes, Number(substring(str, pos_start, pos_end)));
		}

		for (let i: i32 = 0; i < files.length; ++i) {
			let file: string = files[i];
			if (path_is_folder(file)) {
				map_set(file_cloud, substring(file, 0, file.length - 1), []);
			}
		}
		for (let i: i32 = 0; i < files.length; ++i) {
			let file: string = files[i];
			let nested: bool = string_index_of(file, "/") != string_last_index_of(file, "/");
			if (nested) {
				let delim: i32 = path_is_folder(file) ? string_last_index_of(substring(file, 0, file.length - 1), "/") : string_last_index_of(file, "/");
				let parent: string = substring(file, 0, delim);
				let child: string = path_is_folder(file) ? substring(file, delim + 1, file.length - 1) : substring(file, delim + 1, file.length);
				array_push(map_get(file_cloud, parent), child);
				if (!path_is_folder(file)) {
					map_set(file_cloud_sizes, file, sizes[i]);
				}
			}
		}

		let is_truncated: bool = string_index_of(str, "<IsTruncated>true") > -1;
		if (is_truncated) {
			let pos_start: i32 = string_index_of(str, "<NextContinuationToken>");
			pos_start += 23;
			let pos_end: i32 = string_index_of_pos(str, "</NextContinuationToken>", pos_start);
			file_init_cloud_bytes(done, "&start-after=" + substring(str, pos_start, pos_end));
		}
		else done();
	});
}

function file_init_cloud(done: ()=>void) {
	file_cloud = map_create();
	file_cloud_sizes = map_create();
	file_init_cloud_bytes(done);
}
