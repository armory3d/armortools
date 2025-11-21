
let neural_node_results: map_t<i32, gpu_texture_t> = map_create();
let neural_node_current: ui_node_t;

function neural_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let result: gpu_texture_t = map_get(neural_node_results, node.id);
	if (result == null) {
		return "float3(0.0, 0.0, 0.0)";
	}
	let tex_name: string = parser_material_node_name(node);
	map_set(data_cached_images, tex_name, result);
	let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".rgb";
}

function neural_node_value(node: ui_node_t, socket: ui_node_socket_t): string {
	let result: gpu_texture_t = map_get(neural_node_results, node.id);
	if (result == null) {
		return "0.0";
	}
	let tex_name: string = parser_material_node_name(node);
	map_set(data_cached_images, tex_name, result);
	let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".r";
}

function neural_from_node(inp: ui_node_socket_t, socket: i32): ui_node_t {
	let result: ui_node_t = null;
	let canvas: ui_node_canvas_t = ui_nodes_get_canvas(true);
	for (let i: i32 = 0; i < canvas.links.length; ++i) {
		let l: ui_node_link_t = canvas.links[i];
		if (l.to_id == inp.node_id && l.to_socket == socket) {
			result = ui_get_node(canvas.nodes, l.from_id);
			break;
		}
	}
	return result;
}

function neural_node_button(node: ui_node_t, model: string): bool {
	let url: string = box_preferneces_model_url_from_name(model);
	let file_name: string = box_preferences_file_name_from_url(url);
	let found: bool = box_preferences_model_exists(file_name);

	if (iron_exec_async_done == 0) {
		if (node != neural_node_current) {
			ui.enabled = false;
		}
		ui_button(tr("Processing..."));
		if (node != neural_node_current) {
			ui.enabled = true;
		}
	}
	else if (!found && ui_button(tr("Setup"))) {
		sys_notify_on_next_frame(function() {
			box_preferences_htab.i = preference_tab_t.NEURAL;
			box_preferences_show();
		});
	}
	else if (found && ui_button(tr("Run"))) {
		return true;
	}
	return false;
}

function neural_node_check_result(node: ui_node_t) {
	neural_node_current = node;
	if (iron_exec_async_done == 1) {
		let file: string = neural_node_dir() + path_sep + "output.png";
		if (iron_file_exists(file)) {
			let result: gpu_texture_t = iron_load_texture(file);
			map_set(neural_node_results, node.id, result);
		}
		sys_remove_update(neural_node_check_result);
	}
}

function neural_node_dir(): string {
	let dir: string;
	if (path_is_protected()) {
		dir = iron_internal_save_path() + "models";
	}
	else {
		dir = iron_internal_files_location() + path_sep + "models";
	}
	return dir;
}

let neural_node_downloading: i32 = 0;

function neural_node_download(url: string) {
	let file_name: string = substring(url, string_last_index_of(url, "/") + 1, url.length);
	let file_path: string = neural_node_dir() + path_sep + file_name;
	let found: bool = iron_file_exists(file_path);
	if (found) {
		return;
	}

	neural_node_downloading++;
	file_download_to(url, file_path, function (url: string) {
		neural_node_downloading--;
		console_log(tr("Downloaded file from") + " " + url);

		/// if arm_linux
		// todo
		if (ends_with(url, "sd_vulkan")) {
			let bin: string = neural_node_dir() + "/sd_vulkan";
			iron_sys_command("chmod +x \"" + bin + "\"");
		}
		else if (ends_with(url, "sd_cpu")) {
			let bin: string = neural_node_dir() + "/sd_cpu";
			iron_sys_command("chmod +x \"" + bin + "\"");
		}
		/// end
	});
}

function neural_node_download_models(models: string[]) {
	if (file_read_directory(neural_node_dir())[0] == "") {
		file_create_directory(neural_node_dir());
	}

	/// if arm_windows
	neural_node_download("https://github.com/armory3d/armortools/raw/refs/heads/main/base/assets/bin/windows_x64/sd_cpu.exe");
	neural_node_download("https://github.com/armory3d/armortools/raw/refs/heads/main/base/assets/bin/windows_x64/sd_vulkan.exe");
	neural_node_download("https://github.com/armory3d/armortools/raw/refs/heads/main/base/assets/bin/windows_x64/sd_cuda.exe");
	/// elseif arm_linux
	neural_node_download("https://github.com/armory3d/armortools/raw/refs/heads/main/base/assets/bin/linux_x64/sd_cpu");
	neural_node_download("https://github.com/armory3d/armortools/raw/refs/heads/main/base/assets/bin/linux_x64/sd_vulkan");
	/// else
	neural_node_download("https://github.com/armory3d/armortools/raw/refs/heads/main/base/assets/bin/macos/sd_cpu");
	neural_node_download("https://github.com/armory3d/armortools/raw/refs/heads/main/base/assets/bin/macos/sd_vulkan");
	/// end

	iron_net_bytes_downloaded = 0;

	for (let i: i32 = 0; i < models.length; ++i) {
		neural_node_download(models[i]);
	}
}
