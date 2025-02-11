
let make_material_default_scon: shader_context_t = null;
let make_material_default_mcon: material_context_t = null;

let make_material_height_used: bool = false;
let make_material_emis_used: bool = false;
let make_material_subs_used: bool = false;

function make_material_get_mout(): bool {
	for (let i: i32 = 0; i < ui_nodes_get_canvas_material().nodes.length; ++i) {
		let n: ui_node_t = ui_nodes_get_canvas_material().nodes[i];
		if (n.type == "OUTPUT_MATERIAL_PBR") {
			return true;
		}
	}
	return false;
}

function make_material_parse_mesh_material() {
	let m: material_data_t = project_materials[0].data;

	for (let i: i32 = 0; i < m._.shader._.contexts.length; ++i) {
		let c: shader_context_t = m._.shader._.contexts[i];
		if (c.name == "mesh") {
			array_remove(m._.shader.contexts, c);
			array_remove(m._.shader._.contexts, c);
			make_material_delete_context(c);
			break;
		}
	}

	if (make_mesh_layer_pass_count > 1) {
		let i: i32 = 0;
		while (i < m._.shader._.contexts.length) {
			let c: shader_context_t = m._.shader._.contexts[i];
			for (let j: i32 = 1; j < make_mesh_layer_pass_count; ++j) {
				let name: string = "mesh" + j;
				if (c.name == name) {
					array_remove(m._.shader.contexts, c);
					array_remove(m._.shader._.contexts, c);
					make_material_delete_context(c);
					i--;
					break;
				}
			}
			i++;
		}

		i = 0;
		while (i < m._.contexts.length) {
			let c: material_context_t = m._.contexts[i];
			for (let j: i32 = 1; j < make_mesh_layer_pass_count; ++j) {
				let name: string = "mesh" + j;
				if (c.name == name) {
					array_remove(m.contexts, c);
					array_remove(m._.contexts, c);
					i--;
					break;
				}
			}
			i++;
		}
	}

	let mm: material_t = { name: "Material", canvas: null };
	let con: node_shader_context_t = make_mesh_run(mm);
	let scon: shader_context_t = shader_context_create(con.data);
	let override_context: _shader_override_t = {};
	if (con.frag.shared_samplers.length > 0) {
		let sampler: string = con.frag.shared_samplers[0];
		override_context.shared_sampler = substring(sampler, string_last_index_of(sampler, " ") + 1, sampler.length);
	}
	if (!context_raw.texture_filter) {
		override_context.filter = "point";
	}
	scon._.override_context = override_context;
	array_push(m._.shader.contexts, scon);
	array_push(m._.shader._.contexts, scon);

	for (let i: i32 = 1; i < make_mesh_layer_pass_count; ++i) {
		let mm: material_t = { name: "Material", canvas: null };
		let con: node_shader_context_t = make_mesh_run(mm, i);
		let scon: shader_context_t = shader_context_create(con.data);
		let override_context: _shader_override_t = {};
		if (con.frag.shared_samplers.length > 0) {
			let sampler: string = con.frag.shared_samplers[0];
			override_context.shared_sampler = substring(sampler, string_last_index_of(sampler, " ") + 1, sampler.length);
		}
		if (!context_raw.texture_filter) {
			override_context.filter = "point";
		}
		scon._.override_context = override_context;
		array_push(m._.shader.contexts, scon);
		array_push(m._.shader._.contexts, scon);

		let tmcon: material_context_t = { name: "mesh" + i, bind_textures: [] };
		let mcon: material_context_t = material_context_create(tmcon);
		array_push(m.contexts, mcon);
		array_push(m._.contexts, mcon);
	}

	context_raw.ddirty = 2;
}

function make_material_parse_mesh_preview_material() {
	if (!make_material_get_mout()) {
		return;
	}

	let m: material_data_t = project_materials[0].data;
	let scon: shader_context_t = null;
	for (let i: i32 = 0; i < m._.shader._.contexts.length; ++i) {
		let c: shader_context_t = m._.shader._.contexts[i];
		if (c.name == "mesh") {
			scon = c;
			break;
		}
	}

	array_remove(m._.shader.contexts, scon);
	array_remove(m._.shader._.contexts, scon);

	let mcon: material_context_t = { name: "mesh", bind_textures: [] };

	let sd: material_t = { name: "Material", canvas: null };
	let con: node_shader_context_t = make_mesh_preview_run(sd, mcon);

	for (let i: i32 = 0; i < m._.contexts.length; ++i) {
		if (m._.contexts[i].name == "mesh") {
			m._.contexts[i] = material_context_create(mcon);
			break;
		}
	}

	if (scon != null) {
		make_material_delete_context(scon);
	}

	let compile_error: bool = false;
	let _scon: shader_context_t = shader_context_create(con.data);
	if (_scon == null) {
		compile_error = true;
	}
	scon = _scon;
	if (compile_error) {
		return;
	}

	array_push(m._.shader.contexts, scon);
	array_push(m._.shader._.contexts, scon);
}

function make_material_parse_paint_material(bake_previews: bool = true) {
	if (!make_material_get_mout()) {
		return;
	}

	if (bake_previews) {
		let current: image_t = _g2_current;
		let g2_in_use: bool = _g2_in_use;
		if (g2_in_use) g2_end();
		make_material_bake_node_previews();
		if (g2_in_use) g2_begin(current);
	}

	let m: material_data_t = project_materials[0].data;
	// let scon: shader_context_t = null;
	// let mcon: material_context_t = null;
	for (let i: i32 = 0; i < m._.shader._.contexts.length; ++i) {
		let c: shader_context_t = m._.shader._.contexts[i];
		if (c.name == "paint") {
			array_remove(m._.shader.contexts, c);
			array_remove(m._.shader._.contexts, c);
			if (c != make_material_default_scon) {
				make_material_delete_context(c);
			}
			break;
		}
	}
	for (let i: i32 = 0; i < m._.contexts.length; ++i) {
		let c: material_context_t = m._.contexts[i];
		if (c.name == "paint") {
			array_remove(m.contexts, c);
			array_remove(m._.contexts, c);
			break;
		}
	}

	let sdata: material_t = { name: "Material", canvas: ui_nodes_get_canvas_material() };
	let mcon2: material_context_t = { name: "paint", bind_textures: [] };
	let con: node_shader_context_t = make_sculpt_run(sdata, mcon2);

	let compile_error: bool = false;
	let scon2: shader_context_t;
	let _scon: shader_context_t = shader_context_create(con.data);
	if (_scon == null) {
		compile_error = true;
	}
	scon2 = _scon;

	if (compile_error) {
		return;
	}

	let override_context: _shader_override_t = {};
	override_context.addressing = "repeat";
	scon2._.override_context = override_context;
	let mcon3: material_context_t = material_context_create(mcon2);

	array_push(m._.shader.contexts, scon2);
	array_push(m._.shader._.contexts, scon2);
	array_push(m.contexts, mcon3);
	array_push(m._.contexts, mcon3);

	if (make_material_default_scon == null) {
		make_material_default_scon = scon2;
	}
	if (make_material_default_mcon == null) {
		make_material_default_mcon = mcon3;
	}
}

function make_material_bake_node_previews() {
	context_raw.node_previews_used = [];
	if (context_raw.node_previews == null) {
		context_raw.node_previews = map_create();
	}
	let empty: ui_node_t[] = [];
	make_material_traverse_nodes(ui_nodes_get_canvas_material().nodes, null, empty);

	let keys: string[] = map_keys(context_raw.node_previews);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let key: string = keys[i];
		if (array_index_of(context_raw.node_previews_used, key) == -1) {
			let image: image_t = map_get(context_raw.node_previews, key);
			app_notify_on_next_frame(function (image: image_t) {
				image_unload(image);
			}, image);
			map_delete(context_raw.node_previews, key);
		}
	}
}

function make_material_traverse_nodes(nodes: ui_node_t[], group: ui_node_canvas_t, parents: ui_node_t[]) {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let node: ui_node_t = nodes[i];
		make_material_bake_node_preview(node, group, parents);
		if (node.type == "GROUP") {
			for (let j: i32 = 0; j < project_material_groups.length; ++j) {
				let g: node_group_t = project_material_groups[j];
				let cname: string = g.canvas.name;
				if (cname == node.name) {
					array_push(parents, node);
					make_material_traverse_nodes(g.canvas.nodes, g.canvas, parents);
					array_pop(parents);
					break;
				}
			}
		}
	}
}

function make_material_bake_node_preview(node: ui_node_t, group: ui_node_canvas_t, parents: ui_node_t[]) {
	if (node.type == "BLUR") {
		let id: string = parser_material_node_name(node, parents);
		let image: image_t = map_get(context_raw.node_previews, id);
		array_push(context_raw.node_previews_used, id);
		let res_x: i32 = math_floor(config_get_texture_res_x() / 4);
		let res_y: i32 = math_floor(config_get_texture_res_y() / 4);
		if (image == null || image.width != res_x || image.height != res_y) {
			if (image != null) {
				image_unload(image);
			}
			image = image_create_render_target(res_x, res_y);
			map_set(context_raw.node_previews, id, image);
		}

		parser_material_blur_passthrough = true;
		util_render_make_node_preview(ui_nodes_get_canvas_material(), node, image, group, parents);
		parser_material_blur_passthrough = false;
	}
	else if (node.type == "DIRECT_WARP") {
		let id: string = parser_material_node_name(node, parents);
		let image: image_t = map_get(context_raw.node_previews, id);
		array_push(context_raw.node_previews_used, id);
		let res_x: i32 = math_floor(config_get_texture_res_x());
		let res_y: i32 = math_floor(config_get_texture_res_y());
		if (image == null || image.width != res_x || image.height != res_y) {
			if (image != null) {
				image_unload(image);
			}
			image = image_create_render_target(res_x, res_y);
			map_set(context_raw.node_previews, id, image);
		}

		parser_material_warp_passthrough = true;
		util_render_make_node_preview(ui_nodes_get_canvas_material(), node, image, group, parents);
		parser_material_warp_passthrough = false;
	}
}

type parse_node_preview_result_t = {
	scon: shader_context_t;
	mcon: material_context_t;
};

function make_material_parse_node_preview_material(node: ui_node_t, group: ui_node_canvas_t = null, parents: ui_node_t[] = null): parse_node_preview_result_t {
	if (node.outputs.length == 0) {
		return null;
	}
	let sdata: material_t = { name: "Material", canvas: ui_nodes_get_canvas_material() };
	let mcon_raw: material_context_t = { name: "mesh", bind_textures: [] };
	let con: node_shader_context_t = make_node_preview_run(sdata, mcon_raw, node, group, parents);
	let compile_error: bool = false;
	let scon: shader_context_t;
	let _scon: shader_context_t = shader_context_create(con.data);
	if (_scon == null) {
		compile_error = true;
	}
	scon = _scon;

	if (compile_error) {
		return null;
	}
	let mcon: material_context_t = material_context_create(mcon_raw);
	let result: parse_node_preview_result_t = {
		scon: scon,
		mcon: mcon
	};
	return result;
}

function make_material_parse_brush() {
	parser_logic_parse(context_raw.brush.canvas);
}

function make_material_get_displace_strength(): f32 {
	let sc: vec4_t = context_main_object().base.transform.scale;
	return config_raw.displace_strength * 0.02 * sc.x;
}

function make_material_delete_context(c: shader_context_t) {
	app_notify_on_next_frame(function (c: shader_context_t) { // Ensure pipeline is no longer in use
		shader_context_delete(c);
	}, c);
}
