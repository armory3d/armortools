
type file_download_data_t = {
	done : (url: string) => void;
};

type file_cache_cloud_data_t = {
	dest: string; path : string; done : (dest: string) => void;
};

let _file_download_map: map_t<string, file_download_data_t>       = map_create();
let _file_cache_cloud_map: map_t<string, file_cache_cloud_data_t> = map_create();

/// if arm_windows
let file_cmd_copy: string = "copy";
/// else
let file_cmd_copy: string = "cp";
/// end

let file_cloud: map_t<string, string[]>  = null;
let file_cloud_sizes: map_t<string, i32> = null;

let _file_init_cloud_bytes_done: () => void;

/// if (arm_android || arm_wasm)
let file_internal: map_t<string, string[]> = null; // .apk contents
/// end

function file_read_directory(path: string): string[] {
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

	/// if arm_wasm
	if (starts_with(path, "/./")) {
		path = substring(path, 2, path.length);
	}
	/// end

	/// if (arm_android || arm_wasm)
	path = string_replace_all(path, "//", "/");
	if (file_internal == null) {
		let s: string = sys_buffer_to_string(data_get_blob("data_list.json"));
		file_internal = json_parse_to_map(s);
	}
	if (map_get(file_internal, path) != null) {
		return string_split(map_get(file_internal, path), ",");
	}
	/// end

	let files: string[] = string_split(iron_read_directory(path), "\n");
	if (files.length == 1 && files[0] == ""){
		array_pop(files);
		return files;
	}
	array_sort(files, null);

	// Folders first
	let num: i32 = files.length;
	for (let i: i32 = 0; i < num; ++i) {
		let f: string = files[i];
		if (!iron_is_directory(path_join(path, f))) {
			array_splice(files, i, 1);
			array_push(files, f);
			i--;
			num--;
		}
	}

	return files;
}

function file_copy(src_path: string, dst_path: string) {
	iron_sys_command(file_cmd_copy + " \"" + src_path + "\" \"" + dst_path + "\"");
}

function file_start(path: string) {
	/// if arm_windows
	iron_sys_command("start \"\" \"" + path + "\"");
	/// elseif arm_linux
	iron_sys_command("xdg-open \"" + path + "\"");
	/// else
	iron_sys_command("open \"" + path + "\"");
	/// end
}

function file_download_to(url: string, dst_path: string, done: (url: string) => void, size: i32 = 0) {
	let fdd: file_download_data_t = {done : done};
	map_set(_file_download_map, url, fdd);
	iron_file_download(url, function(url: string, ab: buffer_t) {
		let fdd: file_download_data_t = map_get(_file_download_map, url);
		fdd.done(url);
	}, size, dst_path);
}

function file_cache_cloud(path: string, done: (s: string) => void) {
	let dest: string;
	if (path_is_protected()) {
		dest = iron_internal_save_path();
	}
	else {
		dest = iron_internal_files_location() + PATH_SEP;
	}
	dest += path;

	if (iron_file_exists(dest)) {
		done(dest);
		return;
	}

	let file_dir: string = substring(dest, 0, string_last_index_of(dest, PATH_SEP));
	if (file_read_directory(file_dir)[0] == "") {
		iron_create_directory(file_dir);
	}
	/// if arm_windows
	path = string_replace_all(path, "\\", "/");
	/// end
	let url: string = config_raw.server + "/" + path;

	let fccd: file_cache_cloud_data_t = {dest : dest, path : path, done : done};
	map_set(_file_cache_cloud_map, url, fccd);

	file_download_to(url, dest, function(url: string) {
		let fccd: file_cache_cloud_data_t = map_get(_file_cache_cloud_map, url);
		if (!iron_file_exists(fccd.dest)) {
			console_error(strings_check_internet_connection());
			fccd.done(null);
			return;
		}
		fccd.done(fccd.dest);
	}, map_get(file_cloud_sizes, path));
}

function file_init_cloud_bytes(done: () => void, append: string = "") {
	_file_init_cloud_bytes_done = done;
	iron_file_download(config_raw.server + "/index.txt" + append, function(url: string, buffer: buffer_t) {
		if (buffer == null) {
			let empty: string[] = [];
			map_set(file_cloud, "cloud", empty);
			console_error(strings_check_internet_connection());
			return;
		}
		let files: string[] = [];
		let sizes: i32[]    = [];
		let str: string     = sys_buffer_to_string(buffer);
		let str_len: i32    = str.length;
		let pos_start: i32  = 0;
		let pos_end: i32    = 0;

		while (true) {
			if (pos_start >= str_len) {
				break;
			}
			pos_end = string_index_of_pos(str, " ", pos_start);
			array_push(files, substring(str, pos_start, pos_end));
			pos_start = pos_end + 1;
			pos_end = string_index_of_pos(str, "\n", pos_start);
			if (pos_end == -1) {
				pos_end = str_len - 1;
			}
			array_push(sizes, parse_int(substring(str, pos_start, pos_end)));
			pos_start = pos_end + 1;
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
				let delim: i32     = path_is_folder(file) ? string_last_index_of(substring(file, 0, file.length - 1), "/") : string_last_index_of(file, "/");
				let parent: string = substring(file, 0, delim);
				let child: string  = path_is_folder(file) ? substring(file, delim + 1, file.length - 1) : substring(file, delim + 1, file.length);
				array_push(map_get(file_cloud, parent), child);
				if (!path_is_folder(file)) {
					map_set(file_cloud_sizes, file, sizes[i]);
				}
			}
		}

		_file_init_cloud_bytes_done();
	});
	iron_file_download("https://cloud-index.armory3d.workers.dev/", function(url: string, buffer: buffer_t) {});
}

function file_init_cloud(done: () => void) {
	file_cloud       = map_create();
	file_cloud_sizes = map_create();
	file_init_cloud_bytes(done);
}
