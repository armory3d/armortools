
///if arm_windows
let file_cmd_mkdir: string = "mkdir";
let file_cmd_copy: string = "copy";
///else
let file_cmd_mkdir: string = "mkdir -p";
let file_cmd_copy: string = "cp";
///end

let file_cloud: map_t<string, string[]> = null;
let file_cloud_sizes: map_t<string, i32> = null;

// ///if arm_android
// let let file_internal: map_t<string, string[]> = null; // .apk contents
// ///end

function file_read_directory(path: string, folders_only: bool = false): string[] {
	if (starts_with(path, "cloud")) {
		let files: string[] = file_cloud != null ? map_get(file_cloud, string_replace_all(path, "\\", "/")) : null;
		if (files != null) {
			return files;
		}
		else {
			let empty: string[] = [];
			return empty;
		}
	}
	// ///if arm_android
	// path = string_replace_all(path, "//", "/");
	// if (file_internal == null) {
	// 	file_internal = [];
	// 	map_set(file_internal, "/data/plugins", BuildMacros.readDirectory("out/data/plugins"));
	// 	map_set(file_internal, "/data/export_presets", BuildMacros.readDirectory("out/data/export_presets"));
	// 	map_set(file_internal, "/data/keymap_presets", BuildMacros.readDirectory("out/data/keymap_presets"));
	// 	map_set(file_internal, "/data/locale", BuildMacros.readDirectory("out/data/locale"));
	// 	map_set(file_internal, "/data/meshes", BuildMacros.readDirectory("out/data/meshes"));
	// 	map_set(file_internal, "/data/themes", BuildMacros.readDirectory("out/data/themes"));
	// }
	// if (file_internal.exists(path)) return map_get(file_internal, path);
	// ///end
	return string_split(iron_read_directory(path, folders_only), "\n");
}

function file_create_directory(path: string) {
	iron_sys_command(file_cmd_mkdir + " \"" + path + "\"");
}

function file_copy(srcPath: string, dst_path: string) {
	iron_sys_command(file_cmd_copy + " \"" + srcPath + "\" \"" + dst_path + "\"");
}

function file_start(path: string) {
	///if arm_windows
	iron_sys_command("start \"\" \"" + path + "\"");
	///elseif arm_linux
	iron_sys_command("xdg-open \"" + path + "\"");
	///else
	iron_sys_command("open \"" + path + "\"");
	///end
}

function file_load_url(url: string) {
	iron_load_url(url);
}

function file_delete(path: string) {
	iron_delete_file(path);
}

function file_exists(path: string): bool {
	return iron_file_exists(path);
}

type file_download_data_t = {
	dst_path: string;
	done: (url: string)=>void;
};

let _file_download_map: map_t<string, file_download_data_t> = map_create();

function file_download(url: string, dst_path: string, done: (url: string)=>void, size: i32 = 0) {
	///if (arm_windows || arm_macos || arm_ios || arm_android)
	let fdd: file_download_data_t = { dst_path: dst_path, done: done };
	map_set(_file_download_map, url, fdd);
	iron_http_request(url, size, function (url: string, ab: buffer_t) {
		let fdd: file_download_data_t = map_get(_file_download_map, url);
		if (ab != null) {
			iron_file_save_bytes(fdd.dst_path, ab, 0);
		}
		fdd.done(url);
	});
	///elseif arm_linux
	iron_sys_command("wget -O \"" + dst_path + "\" \"" + url + "\"");
	done(url);
	///else
	iron_sys_command("curl -L " + url + " -o \"" + dst_path + "\"");
	done(url);
	///end
}

type file_download_bytes_data_t = {
	url: string;
	save: string;
	done: (url: string, ab: buffer_t)=>void;
};

let _file_download_bytes_map: map_t<string, file_download_bytes_data_t> = map_create();

function file_download_bytes(url: string, done: (url: string, ab: buffer_t)=>void) {
	let save: string;
	if (path_is_protected()) {
		save = iron_save_path();
	}
	else {
		save = path_data() + path_sep;
	}
	save += "download.bin";

	let fdbd: file_download_bytes_data_t = { url: url, save: save, done: done };
	map_set(_file_download_bytes_map, url, fdbd);
	file_download(url, save, function (url: string) {
		let fdbd: file_download_bytes_data_t = map_get(_file_download_bytes_map, url);
		let buffer: buffer_t = iron_load_blob(fdbd.save);
		fdbd.done(fdbd.url, buffer);
	});
}

type file_cache_cloud_data_t = {
	dest: string;
	path: string;
	done: (dest: string)=>void;
};

let _file_cache_cloud_map: map_t<string, file_cache_cloud_data_t> = map_create();

function file_cache_cloud(path: string, done: (s: string)=>void) {
	///if arm_ios
	let path2: string = string_replace_all(path, "/", "_"); // Cache everything into root folder
	///else
	let path2: string = path;
	///end
	let dest: string;
	if (path_is_protected()) {
		dest = iron_save_path();
	}
	else {
		dest = iron_get_files_location() + path_sep;
	}
	dest += path2;

	if (file_exists(dest)) {
		///if (arm_macos || arm_ios)
		done(dest);
		///else
		let p: string;
		if (path_is_protected()) {
			p = iron_save_path();
		}
		else {
			p = path_working_dir() + path_sep;
		}
		p += path;
		done(p);
		///end
		return;
	}

	let file_dir: string = substring(dest, 0, string_last_index_of(dest, path_sep));
	if (file_read_directory(file_dir)[0] == "") {
		file_create_directory(file_dir);
	}
	///if arm_windows
	path = string_replace_all(path, "\\", "/");
	///end
	let url: string = config_raw.server + "/" + path;

	let fccd: file_cache_cloud_data_t = { dest: dest, path: path, done: done };
	map_set(_file_cache_cloud_map, url, fccd);

	file_download(url, dest, function (url: string) {
		let fccd: file_cache_cloud_data_t = map_get(_file_cache_cloud_map, url);
		if (!file_exists(fccd.dest)) {
			console_error(strings_error5());
			fccd.done(null);
			return;
		}
		///if (arm_macos || arm_ios)
		fccd.done(fccd.dest);
		///else
		let p: string;
		if (path_is_protected()) {
			p = iron_save_path();
		}
		else {
			p = path_working_dir() + path_sep;
		}
		p += fccd.path;
		fccd.done(p);
		///end
	}, map_get(file_cloud_sizes, path));
}

let _file_init_cloud_bytes_done: ()=>void;

function file_init_cloud_bytes(done: ()=>void, append: string = "") {
	_file_init_cloud_bytes_done = done;
	file_download_bytes(config_raw.server + "/?list-type=2" + append, function (url: string, buffer: buffer_t) {
		if (buffer == null) {
			let empty: string[] = [];
			map_set(file_cloud, "cloud", empty);
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

			array_push(sizes, parse_int(substring(str, pos_start, pos_end)));
		}

		for (let i: i32 = 0; i < files.length; ++i) {
			let file: string = files[i];
			if (path_is_folder(file)) {
				let empty: string[] = [];
				map_set(file_cloud, substring(file, 0, file.length - 1), empty);
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
			file_init_cloud_bytes(_file_init_cloud_bytes_done, "&start-after=" + substring(str, pos_start, pos_end));
		}
		else {
			_file_init_cloud_bytes_done();
		}
	});
}

function file_init_cloud(done: ()=>void) {
	file_cloud = map_create();
	file_cloud_sizes = map_create();
	file_init_cloud_bytes(done);
}
