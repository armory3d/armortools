
let make_material_default_scon: shader_context_t = null;
let make_material_default_mcon: material_context_t = null;

let make_material_height_used: bool = false;
let make_material_emis_used: bool = false;
let make_material_subs_used: bool = false;

function make_material_get_mout(): bool {
	for (let n of ui_nodes_get_canvas_material().nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
	return false;
}

function make_material_parse_mesh_material() {
	let m = project_materials[0].data;

	for (let c of m._.shader._.contexts) {
		if (c.name == "mesh") {
			array_remove(m._.shader.contexts, c);
			array_remove(m._.shader._.contexts, c);
			make_material_delete_context(c);
			break;
		}
	}

	if (make_mesh_layer_pass_count > 1) {
		let i = 0;
		while (i < m._.shader._.contexts.length) {
			let c = m._.shader._.contexts[i];
			for (let j = 1; j < make_mesh_layer_pass_count; ++j) {
				if (c.name == "mesh" + j) {
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
		while (i < m.contexts.length) {
			let c = m.contexts[i];
			for (let j = 1; j < make_mesh_layer_pass_count; ++j) {
				if (c.name == "mesh" + j) {
					array_remove(m.contexts, c);
					array_remove(m._.contexts, c);
					i--;
					break;
				}
			}
			i++;
		}
	}

	let con = make_mesh_run({ name: "Material", canvas: null });
	let scon: shader_context_t = shader_context_create(con.data);
	scon._.override_context = {};
	if (con.frag.shared_samplers.length > 0) {
		let sampler = con.frag.shared_samplers[0];
		scon._.override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
	}
	if (!context_raw.texture_filter) {
		scon._.override_context.filter = "point";
	}
	m._.shader.contexts.push(scon);
	m._.shader._.contexts.push(scon);

	for (let i = 1; i < make_mesh_layer_pass_count; ++i) {
		let con = make_mesh_run({ name: "Material", canvas: null }, i);
		let scon: shader_context_t = shader_context_create(con.data);
		scon._.override_context = {};
		if (con.frag.shared_samplers.length > 0) {
			let sampler = con.frag.shared_samplers[0];
			scon._.override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!context_raw.texture_filter) {
			scon._.override_context.filter = "point";
		}
		m._.shader.contexts.push(scon);
		m._.shader._.contexts.push(scon);

		let mcon: material_context_t = material_context_create({ name: "mesh" + i, bind_textures: [] });
		m.contexts.push(mcon);
		m._.contexts.push(mcon);
	}

	context_raw.ddirty = 2;

	///if arm_voxels
	make_material_make_voxel(m);
	///end
}

function make_material_parse_particle_material() {
	let m = context_raw.particle_material;
	let sc: shader_context_t = null;
	for (let c of m._.shader._.contexts) {
		if (c.name == "mesh") {
			sc = c;
			break;
		}
	}
	if (sc != null) {
		array_remove(m._.shader.contexts, sc);
		array_remove(m._.shader._.contexts, sc);
	}
	let con = make_particle_run({ name: "MaterialParticle", canvas: null });
	if (sc != null) make_material_delete_context(sc);
	sc = shader_context_create(con.data);
	m._.shader.contexts.push(sc);
	m._.shader._.contexts.push(sc);
}

function make_material_parse_mesh_preview_material() {
	if (!make_material_get_mout()) return;

	let m = project_materials[0].data;
	let scon: shader_context_t = null;
	for (let c of m._.shader._.contexts) {
		if (c.name == "mesh") {
			scon = c;
			break;
		}
	}
	array_remove(m._.shader.contexts, scon);
	array_remove(m._.shader._.contexts, scon);

	let mcon: material_context_t = { name: "mesh", bind_textures: [] };

	let sd: material_t = { name: "Material", canvas: null };
	let con = make_mesh_preview_run(sd, mcon);

	for (let i = 0; i < m.contexts.length; ++i) {
		if (m.contexts[i].name == "mesh") {
			m.contexts[i] = material_context_create(mcon);
			break;
		}
	}

	if (scon != null) make_material_delete_context(scon);

	let compileError = false;
	let _scon: shader_context_t = shader_context_create(con.data);
	if (_scon == null) compileError = true;
	scon = _scon;
	if (compileError) return;

	m._.shader.contexts.push(scon);
	m._.shader._.contexts.push(scon);
}

///if arm_voxels
function make_material_make_voxel(m: material_data_t) {
	let rebuild = make_material_height_used;
	if (config_raw.rp_gi != false && rebuild) {
		let scon: shader_context_t = null;
		for (let c of m._.shader._.contexts) {
			if (c.name == "voxel") {
				scon = c;
				break;
			}
		}
		if (scon != null) make_voxel_run(scon);
	}
}
///end

function make_material_parse_paint_material(bake_previews: bool = true) {
	if (!make_material_get_mout()) return;

	if (bake_previews) {
		let current = _g2_current;
		let g2_in_use: bool = _g2_in_use;
		if (g2_in_use) g2_end();
		make_material_bake_node_previews();
		if (g2_in_use) g2_begin(current);
	}

	let m = project_materials[0].data;
	let scon: shader_context_t = null;
	let mcon: material_context_t = null;
	for (let c of m._.shader._.contexts) {
		if (c.name == "paint") {
			array_remove(m._.shader.contexts, c);
			array_remove(m._.shader._.contexts, c);
			if (c != make_material_default_scon) make_material_delete_context(c);
			break;
		}
	}
	for (let c of m.contexts) {
		if (c.name == "paint") {
			array_remove(m.contexts, c);
			array_remove(m._.contexts, c);
			break;
		}
	}

	let sdata: material_t = { name: "Material", canvas: ui_nodes_get_canvas_material() };
	let mcon2: material_context_t = { name: "paint", bind_textures: [] };
	let con = make_sculpt_run(sdata, mcon2);

	let compileError = false;
	let scon2: shader_context_t;
	let _scon: shader_context_t = shader_context_create(con.data);
	if (_scon == null) compileError = true;
	scon2 = _scon;

	if (compileError) return;
	scon2._.override_context = {};
	scon2._.override_context.addressing = "repeat";
	let mcon3: material_context_t = material_context_create(mcon2);

	m._.shader.contexts.push(scon2);
	m._.shader._.contexts.push(scon2);
	m.contexts.push(mcon3);
	m._.contexts.push(mcon3);

	if (make_material_default_scon == null) make_material_default_scon = scon2;
	if (make_material_default_mcon == null) make_material_default_mcon = mcon3;
}

function make_material_bake_node_previews() {
	context_raw.node_previews_used = [];
	if (context_raw.node_previews == null) context_raw.node_previews = map_create();
	make_material_traverse_nodes(ui_nodes_get_canvas_material().nodes, null, []);
	for (let key of context_raw.node_previews.keys()) {
		if (context_raw.node_previews_used.indexOf(key) == -1) {
			let image = context_raw.node_previews.get(key);
			base_notify_on_next_frame(function() { image_unload(image); });
			context_raw.node_previews.delete(key);
		}
	}
}

function make_material_traverse_nodes(nodes: zui_node_t[], group: zui_node_canvas_t, parents: zui_node_t[]) {
	for (let node of nodes) {
		make_material_bake_node_preview(node, group, parents);
		if (node.type == "GROUP") {
			for (let g of project_material_groups) {
				if (g.canvas.name == node.name) {
					parents.push(node);
					make_material_traverse_nodes(g.canvas.nodes, g.canvas, parents);
					parents.pop();
					break;
				}
			}
		}
	}
}

function make_material_bake_node_preview(node: zui_node_t, group: zui_node_canvas_t, parents: zui_node_t[]) {
	if (node.type == "BLUR") {
		let id = parser_material_node_name(node, parents);
		let image = context_raw.node_previews.get(id);
		context_raw.node_previews_used.push(id);
		let resX = math_floor(config_get_texture_res_x() / 4);
		let resY = math_floor(config_get_texture_res_y() / 4);
		if (image == null || image.width != resX || image.height != resY) {
			if (image != null) image_unload(image);
			image = image_create_render_target(resX, resY);
			context_raw.node_previews.set(id, image);
		}

		parser_material_blur_passthrough = true;
		util_render_make_node_preview(ui_nodes_get_canvas_material(), node, image, group, parents);
		parser_material_blur_passthrough = false;
	}
	else if (node.type == "DIRECT_WARP") {
		let id = parser_material_node_name(node, parents);
		let image = context_raw.node_previews.get(id);
		context_raw.node_previews_used.push(id);
		let resX = math_floor(config_get_texture_res_x());
		let resY = math_floor(config_get_texture_res_y());
		if (image == null || image.width != resX || image.height != resY) {
			if (image != null) image_unload(image);
			image = image_create_render_target(resX, resY);
			context_raw.node_previews.set(id, image);
		}

		parser_material_warp_passthrough = true;
		util_render_make_node_preview(ui_nodes_get_canvas_material(), node, image, group, parents);
		parser_material_warp_passthrough = false;
	}
}

function make_material_parse_node_preview_material(node: zui_node_t, group: zui_node_canvas_t = null, parents: zui_node_t[] = null): { scon: shader_context_t, mcon: material_context_t } {
	if (node.outputs.length == 0) return null;
	let sdata: material_t = { name: "Material", canvas: ui_nodes_get_canvas_material() };
	let mcon_raw: material_context_t = { name: "mesh", bind_textures: [] };
	let con = make_node_preview_run(sdata, mcon_raw, node, group, parents);
	let compileError = false;
	let scon: shader_context_t;
	let _scon: shader_context_t = shader_context_create(con.data);
	if (_scon == null) compileError = true;
	scon = _scon;

	if (compileError) return null;
	let mcon: material_context_t = material_context_create(mcon_raw);
	return { scon: scon, mcon: mcon };
}

function make_material_parse_brush() {
	parser_logic_parse(context_raw.brush.canvas);
}

function make_material_get_displace_strength(): f32 {
	let sc = context_main_object().base.transform.scale.x;
	return config_raw.displace_strength * 0.02 * sc;
}

function make_material_voxelgi_half_extents(): string {
	let ext = context_raw.vxao_ext;
	return `const vec3 voxelgiHalfExtents = vec3(${ext}, ${ext}, ${ext});`;
}

function make_material_delete_context(c: shader_context_t) {
	base_notify_on_next_frame(() => { // Ensure pipeline is no longer in use
		shader_context_delete(c);
	});
}
