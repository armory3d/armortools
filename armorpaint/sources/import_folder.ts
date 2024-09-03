
function import_folder_run(path: string) {
	let files: string[] = file_read_directory(path);
	let mapbase: string = "";
	let mapopac: string = "";
	let mapnor: string = "";
	let mapocc: string = "";
	let maprough: string = "";
	let mapmet: string = "";
	let mapheight: string = "";

	let found_texture: bool = false;
	// Import maps
	for (let i: i32 = 0; i < files.length; ++i) {
		let f: string = files[i];
		if (!path_is_texture(f)) {
			continue;
		}

		// TODO: handle -albedo

		let base: string = to_lower_case(substring(f, 0, string_last_index_of(f, ".")));
		let valid: bool = false;
		if (mapbase == "" && path_is_base_color_tex(base)) {
			mapbase = f;
			valid = true;
		}
		if (mapopac == "" && path_is_opacity_tex(base)) {
			mapopac = f;
			valid = true;
		}
		if (mapnor == "" && path_is_normal_map_tex(base)) {
			mapnor = f;
			valid = true;
		}
		if (mapocc == "" && path_is_occlusion_tex(base)) {
			mapocc = f;
			valid = true;
		}
		if (maprough == "" && path_is_roughness_tex(base)) {
			maprough = f;
			valid = true;
		}
		if (mapmet == "" && path_is_metallic_tex(base)) {
			mapmet = f;
			valid = true;
		}
		if (mapheight == "" && path_is_displacement_tex(base)) {
			mapheight = f;
			valid = true;
		}

		if (valid) {
			import_texture_run(path + path_sep + f, false);
			found_texture = true;
		}
	}

	if (!found_texture) {
		console_info(tr("Folder does not contain textures"));
		return;
	}

	// Create material
	context_raw.material = slot_material_create(project_materials[0].data);
	array_push(project_materials, context_raw.material);
	let nodes: ui_nodes_t = context_raw.material.nodes;
	let canvas: ui_node_canvas_t = context_raw.material.canvas;
	let dirs: string[] = string_split(path, path_sep);
	canvas.name = dirs[dirs.length - 1];
	let nout: ui_node_t = null;
	for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
		let n: ui_node_t = canvas.nodes[i];
		if (n.type == "OUTPUT_MATERIAL_PBR") {
			nout = n;
			break;
		}
	}
	for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
		let n: ui_node_t = canvas.nodes[i];
		if (n.name == "RGB") {
			ui_remove_node(n, canvas);
			break;
		}
	}

	// Place nodes
	let pos: i32 = 0;
	let start_y: i32 = 100;
	let node_h: i32 = 164;
	if (mapbase != "") {
		import_folder_place_image_node(nodes, canvas, mapbase, start_y + node_h * pos, nout.id, 0);
		pos++;
	}
	if (mapopac != "") {
		import_folder_place_image_node(nodes, canvas, mapopac, start_y + node_h * pos, nout.id, 1);
		pos++;
	}
	if (mapocc != "") {
		import_folder_place_image_node(nodes, canvas, mapocc, start_y + node_h * pos, nout.id, 2);
		pos++;
	}
	if (maprough != "") {
		import_folder_place_image_node(nodes, canvas, maprough, start_y + node_h * pos, nout.id, 3);
		pos++;
	}
	if (mapmet != "") {
		import_folder_place_image_node(nodes, canvas, mapmet, start_y + node_h * pos, nout.id, 4);
		pos++;
	}
	if (mapnor != "") {
		import_folder_place_image_node(nodes, canvas, mapnor, start_y + node_h * pos, nout.id, 5);
		pos++;
	}
	if (mapheight != "") {
		import_folder_place_image_node(nodes, canvas, mapheight, start_y + node_h * pos, nout.id, 7);
		pos++;
	}

	make_material_parse_paint_material();
	util_render_make_material_preview();
	ui_base_hwnds[1].redraws = 2;
	history_new_material();
}

function import_folder_place_image_node(nodes: ui_nodes_t, canvas: ui_node_canvas_t, asset: string, ny: i32, to_id: i32, to_socket: i32) {
	let n: ui_node_t = nodes_material_create_node("TEX_IMAGE");
	n.buttons[0].default_value = f32_array_create_x(base_get_asset_index(asset));
	n.x = 72;
	n.y = ny;
	let l: ui_node_link_t = {
		id: ui_next_link_id(canvas.links),
		from_id: n.id,
		from_socket: 0,
		to_id: to_id,
		to_socket: to_socket
	};
	array_push(canvas.links, l);
}
