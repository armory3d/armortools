
///if (is_paint || is_sculpt)

function import_blend_material_run(path: string) {
	let b: buffer_t = data_get_blob(path);
	let bl: blend_t = parser_blend_init(b);
	if (bl.dna == null) {
		console_error(strings_error3());
		return;
	}

	let mats: bl_handle_t[] = parser_blend_get(bl, "Material");
	if (mats.length == 0) {
		console_error("Error: No materials found");
		return;
	}

	let imported: slot_material_t[] = [];

	for (let i: i32 = 0; i < mats.length; ++i) {
		let mat: bl_handle_t = mats[i];
		// Material slot
		context_raw.material = slot_material_create(project_materials[0].data);
		array_push(project_materials, context_raw.material);
		array_push(imported, context_raw.material);
		let nodes: ui_nodes_t = context_raw.material.nodes;
		let canvas: ui_node_canvas_t = context_raw.material.canvas;
		let id: bl_handle_t = bl_handle_get(mat, "id");
		canvas.name = bl_handle_get(id, "name"); // MAWood
		canvas.name = substring(canvas.name, 2, canvas.name.length);
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

		// Parse nodetree
		let nodetree: bl_handle_t = bl_handle_get(mat, "nodetree"); // bNodeTree
		let blnodes: bl_handle_t = bl_handle_get(nodetree, "nodes"); // ListBase
		let bllinks: bl_handle_t = bl_handle_get(nodetree, "links"); // bNodeLink

		// Look for Principled BSDF node
		let node: bl_handle_t = bl_handle_get(blnodes, "first", 0, "bNode");
		let last: bl_handle_t = bl_handle_get(blnodes, "last", 0, "bNode");
		while (true) {
			if (bl_handle_get(node, "idname") == "ShaderNodeBsdfPrincipled") {
				break;
			}
			if (bl_handle_get(node, "name") == bl_handle_get(last, "name")) {
				break;
			}
			node = bl_handle_get(node, "next");
		}
		if (bl_handle_get(node, "idname") != "ShaderNodeBsdfPrincipled") {
			console_error("Error: No Principled BSDF node found");
			continue;
		}

		// Use Principled BSDF as material output
		nout.name = bl_handle_get(node, "name");
		let nx: i32 = bl_handle_get_i(node, "locx");
		let ny: i32 = bl_handle_get_i(node, "locy");
		nout.x = nx + 400;
		nout.y = -ny + 400;

		// Place nodes
		node = bl_handle_get(blnodes, "first", 0, "bNode");
		while (true) {
			// Search for node in list
			let search: string = bl_handle_get(node, "idname");
			search = to_lower_case(substring(search, 10, search.length));
			let base: ui_node_t = null;
			for (let i: i32 = 0; i < nodes_material_list.length; ++i) {
				let list: ui_node_t[] = nodes_material_list[i];
				let found: bool = false;
				for (let i: i32 = 0; i < list.length; ++i) {
					let n: ui_node_t = list[i];
					let s: string = to_lower_case(string_replace_all(n.type, "_", ""));
					if (search == s) {
						base = n;
						found = true;
						break;
					}
				}
				if (found) {
					break;
				}
			}

			if (base != null) {
				let n: ui_node_t = ui_nodes_make_node(base, nodes, canvas);
				n.x = bl_handle_get_i(node, "locx") + 400;
				n.y = -bl_handle_get_i(node, "locy") + 400;
				n.name = bl_handle_get(node, "name");

				// Fill input socket values
				let inputs: bl_handle_t = bl_handle_get(node, "inputs");
				let sock: bl_handle_t = bl_handle_get(inputs, "first", 0, "bNodeSocket");
				let pos: i32 = 0;
				while (true) {
					if (pos >= n.inputs.length) {
						break;
					}
					n.inputs[pos].default_value = import_blend_material_read_blend_socket(sock);

					let last: bl_handle_t = sock;
					sock = bl_handle_get(sock, "next");
					if (last.block == sock.block) {
						break;
					}
					pos++;
				}

				// Fill button values
				if (search == "teximage") {
					let img: bl_handle_t = bl_handle_get(node, "id", 0, "Image");
					let file: string = bl_handle_get(img, "name"); // "//desktop\logo.png"
					file = substring(file, 2, file.length);
					file = path_base_dir(path) + file;
					import_texture_run(file);
					let ar: string[] = string_split(file, path_sep);
					let filename: string = ar[ar.length - 1];
					n.buttons[0].default_value = f32_array_create_x(base_get_asset_index(filename));
				}
				else if (search == "valtorgb") {
					// let ramp: bl_handle_t = bl_handle_get(node, "storage", 0, "ColorBand");
					// n.buttons[0].data = bl_handle_get(ramp, "ipotype") == 0 ? 0 : 1; // Linear / Constant
					// let elems: f32[][] = n.buttons[0].default_value;
					// for (let i: i32 = 0; i < bl_handle_get(ramp, "tot"); ++i) {
					// 	if (i >= elems.length) {
					// 		array_push(elems, [1.0, 1.0, 1.0, 1.0, 0.0]);
					// 	}
					// 	let cbdata: bl_handle_t = bl_handle_get(ramp, "data", i, "CBData");
					// 	elems[i][0] = math_floor(bl_handle_get(cbdata, "r") * 100) / 100;
					// 	elems[i][1] = math_floor(bl_handle_get(cbdata, "g") * 100) / 100;
					// 	elems[i][2] = math_floor(bl_handle_get(cbdata, "b") * 100) / 100;
					// 	elems[i][3] = math_floor(bl_handle_get(cbdata, "a") * 100) / 100;
					// 	elems[i][4] = math_floor(bl_handle_get(cbdata, "pos") * 100) / 100;
					// }
				}
				else if (search == "mixrgb" || search == "math") {
					n.buttons[0].default_value = f32_array_create_x(bl_handle_get_i(node, "custom1"));
					n.buttons[1].default_value = f32_array_create_x(bl_handle_get_i(node, "custom2") & 2);
				}
				else if (search == "mapping") {
					let storage: bl_handle_t = bl_handle_get(node, "storage", 0, "TexMapping");
					let v: f32_ptr = bl_handle_get(storage, "loc");
					n.buttons[0].default_value = f32_array_create(3);
					n.buttons[0].default_value[0] = ARRAY_ACCESS(v, 0);
					n.buttons[0].default_value[1] = ARRAY_ACCESS(v, 1);
					n.buttons[0].default_value[2] = ARRAY_ACCESS(v, 2);

					v = bl_handle_get(storage, "rot");
					n.buttons[1].default_value = f32_array_create(3);
					n.buttons[1].default_value[0] = ARRAY_ACCESS(v, 0);
					n.buttons[1].default_value[1] = ARRAY_ACCESS(v, 1);
					n.buttons[1].default_value[2] = ARRAY_ACCESS(v, 2);

					v = bl_handle_get(storage, "size");
					n.buttons[2].default_value = f32_array_create(3);
					n.buttons[2].default_value[0] = ARRAY_ACCESS(v, 0);
					n.buttons[2].default_value[1] = ARRAY_ACCESS(v, 1);
					n.buttons[2].default_value[2] = ARRAY_ACCESS(v, 2);

					// let mat: any = get(storage, "mat"); float[4][4]
					// storage.flag & 1 // use_min
					// storage.flag & 2 // use_max
					// storage.min[0]
					// storage.min[1]
					// storage.min[2]
					// storage.max[0]
					// storage.max[1]
					// storage.max[2]
				}

				// Fill output socket values
				let outputs: bl_handle_t = bl_handle_get(node, "outputs");
				sock = bl_handle_get(outputs, "first", 0, "bNodeSocket");
				pos = 0;
				while (true) {
					if (pos >= n.outputs.length) {
						break;
					}
					n.outputs[pos].default_value = import_blend_material_read_blend_socket(sock);

					let last: bl_handle_t = sock;
					sock = bl_handle_get(sock, "next");
					if (last.block == sock.block) {
						break;
					}
					pos++;
				}

				array_push(canvas.nodes, n);
			}

			if (bl_handle_get(node, "name") == bl_handle_get(last, "name")) {
				break;
			}
			node = bl_handle_get(node, "next");
		}

		// Place links
		let link: bl_handle_t = bl_handle_get(bllinks, "first", 0, "bNodeLink");
		while (true) {
			let fromnode: bl_handle_t = bl_handle_get(bl_handle_get(link, "fromnode"), "name");
			let tonode: string = bl_handle_get(bl_handle_get(link, "tonode"), "name");
			let fromsock: bl_handle_t = bl_handle_get(link, "fromsock");
			let tosock: bl_handle_t = bl_handle_get(link, "tosock");

			let from_id: i32 = -1;
			let to_id: i32 = -1;
			for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
				let n: ui_node_t = canvas.nodes[i];
				if (n.name == fromnode) {
					from_id = n.id;
					break;
				}
			}
			for (let i: i32 = 0; i < canvas.nodes.length; ++i) {
				let n: ui_node_t = canvas.nodes[i];
				if (n.name == tonode) {
					to_id = n.id;
					break;
				}
			}

			if (from_id >= 0 && to_id >= 0) {
				let from_socket: i32 = 0;
				let sock: bl_handle_t = fromsock;
				while (true) {
					let last: bl_handle_t = sock;
					sock = bl_handle_get(sock, "prev");
					if (last.block == sock.block) {
						break;
					}
					from_socket++;
				}

				let to_socket: i32 = 0;
				sock = tosock;
				while (true) {
					let last: bl_handle_t = sock;
					sock = bl_handle_get(sock, "prev");
					if (last.block == sock.block) {
						break;
					}
					to_socket++;
				}

				let valid: bool = true;

				// Remap principled
				if (tonode == nout.name) {
					if (to_socket == 0) {
						to_socket = 0; // Base
					}
					else if (to_socket == 18) {
						to_socket = 1; // Opac
					}
					else if (to_socket == 7) {
						to_socket = 3; // Rough
					}
					else if (to_socket == 4) {
						to_socket = 4; // Met
					}
					else if (to_socket == 19) {
						to_socket = 5; // TODO: auto-remove normal_map node
					}
					else if (to_socket == 17) {
						to_socket = 6; // Emis
					}
					else if (to_socket == 1) {
						to_socket = 8; // Subs
					}
					else {
						valid = false;
					}
				}

				if (valid) {
					let raw: ui_node_link_t = {
						id: ui_next_link_id(canvas.links),
						from_id: from_id,
						from_socket: from_socket,
						to_id: to_id,
						to_socket: to_socket
					};
					array_push(canvas.links, raw);
				}
			}

			let last: bl_handle_t = link;
			link = bl_handle_get(link, "next");
			if (last.block == link.block) {
				break;
			}
		}
		history_new_material();
	}

	app_notify_on_init(function (imported: slot_material_t[]) {
		for (let i: i32 = 0; i < imported.length; ++i) {
			let m: slot_material_t = imported[i];
			context_set_material(m);
			make_material_parse_paint_material();
			util_render_make_material_preview();
		}
	}, imported);

	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	data_delete_blob(path);
}

function import_blend_material_read_blend_socket(sock: bl_handle_t): f32_array_t {
	let idname: string = bl_handle_get(sock, "idname");
	if (starts_with(idname, "NodeSocketVector")) {
		let v: f32_ptr = bl_handle_get(bl_handle_get(sock, "default_value", 0, "bNodeSocketValueVector"), "value");
		let a: f32_array_t = f32_array_create(3);
		a[0] = math_floor(ARRAY_ACCESS(v, 0) * 100) / 100;
		a[1] = math_floor(ARRAY_ACCESS(v, 1) * 100) / 100;
		a[2] = math_floor(ARRAY_ACCESS(v, 2) * 100) / 100;
		return a;
	}
	else if (starts_with(idname, "NodeSocketColor")) {
		let v: f32_ptr = bl_handle_get(bl_handle_get(sock, "default_value", 0, "bNodeSocketValueRGBA"), "value");
		let a: f32_array_t = f32_array_create(4);
		a[0] = math_floor(ARRAY_ACCESS(v, 0) * 100) / 100;
		a[1] = math_floor(ARRAY_ACCESS(v, 1) * 100) / 100;
		a[2] = math_floor(ARRAY_ACCESS(v, 2) * 100) / 100;
		a[3] = math_floor(ARRAY_ACCESS(v, 3) * 100) / 100;
		return a;
	}
	else if (starts_with(idname, "NodeSocketFloat")) {
		let v: f32 = bl_handle_get_f(bl_handle_get(sock, "default_value", 0, "bNodeSocketValueFloat"), "value");
		let a: f32_array_t = f32_array_create(1);
		a[0] = math_floor(v * 100) / 100;
		return a;
	}
	else if (starts_with(idname, "NodeSocketInt")) {
		let v: i32 = bl_handle_get_i(bl_handle_get(sock, "default_value", 0, "bNodeSocketValueInt"), "value");
		let a: f32_array_t = f32_array_create(1);
		a[0] = v;
		return a;
	}
	else if (starts_with(idname, "NodeSocketBoolean")) {
		let v: bool = bl_handle_get_i(bl_handle_get(sock, "default_value", 0, "bNodeSocketValueBoolean"), "value") > 0;
		let a: f32_array_t = f32_array_create(1);
		a[0] = v;
		return a;
	}
	else if (starts_with(idname, "NodeSocketString")) {
		let v: string = bl_handle_get(bl_handle_get(sock, "default_value", 0, "bNodeSocketValueString"), "value");
		return null;
	}
	return null;
}

///end
