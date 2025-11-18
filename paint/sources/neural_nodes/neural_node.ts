
let neural_node_results: map_t<i32, gpu_texture_t> = map_create();
let neural_node_current: ui_node_t;

function neural_node_vector(node: ui_node_t, socket: ui_node_socket_t): string {
    let result: gpu_texture_t = map_get(neural_node_results, node.id);
    if (result == null) {
		return "float3(0.0, 0.0, 0.0)";
	}
	let tex_name: string = parser_material_node_name(node);
	map_set(data_cached_images, tex_name, result);
	let tex: bind_tex_t  = parser_material_make_bind_tex(tex_name, tex_name);
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
	let tex: bind_tex_t  = parser_material_make_bind_tex(tex_name, tex_name);
	let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
	return texstore + ".r";
}

function neural_from_node(inp: ui_node_socket_t, socket: i32): ui_node_t {
	let result: ui_node_t  = null;
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

function neural_node_button(node: ui_node_t): bool {
    let found: bool = true;

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
		box_preferences_htab.i = preference_tab_t.NEURAL;
		box_preferences_show();
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
