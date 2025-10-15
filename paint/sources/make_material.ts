
let make_material_default_scon: shader_context_t   = null;
let make_material_default_mcon: material_context_t = null;

let make_material_height_used: bool = false;
let make_material_emis_used: bool   = false;
let make_material_subs_used: bool   = false;

function make_material_get_mout(): bool {
	for (let i: i32 = 0; i < context_raw.material.canvas.nodes.length; ++i) {
		let n: ui_node_t = context_raw.material.canvas.nodes[i];
		if (n.type == "OUTPUT_MATERIAL_PBR") {
			return true;
		}
	}
	return false;
}

function make_material_parse_mesh_material() {
	let m: material_data_t = project_materials[0].data;

	for (let i: i32 = 0; i < m._.shader.contexts.length; ++i) {
		let c: shader_context_t = m._.shader.contexts[i];
		if (c.name == "mesh") {
			array_remove(m._.shader.contexts, c);
			make_material_delete_context(c);
			break;
		}
	}

	if (make_mesh_layer_pass_count > 1) {
		let i: i32 = 0;
		while (i < m._.shader.contexts.length) {
			let c: shader_context_t = m._.shader.contexts[i];
			for (let j: i32 = 1; j < make_mesh_layer_pass_count; ++j) {
				let name: string = "mesh" + j;
				if (c.name == name) {
					array_remove(m._.shader.contexts, c);
					make_material_delete_context(c);
					i--;
					break;
				}
			}
			i++;
		}

		i = 0;
		while (i < m.contexts.length) {
			let c: material_context_t = m.contexts[i];
			for (let j: i32 = 1; j < make_mesh_layer_pass_count; ++j) {
				let name: string = "mesh" + j;
				if (c.name == name) {
					array_remove(m.contexts, c);
					i--;
					break;
				}
			}
			i++;
		}
	}

	let mm: material_t = {name : "Material", canvas : null};

	let con: node_shader_context_t = make_mesh_run(mm);
	shader_context_load(con.data);
	array_push(m._.shader.contexts, con.data);

	for (let i: i32 = 1; i < make_mesh_layer_pass_count; ++i) {
		let mm: material_t             = {name : "Material", canvas : null};
		let con: node_shader_context_t = make_mesh_run(mm, i);
		shader_context_load(con.data);

		array_push(m._.shader.contexts, con.data);

		let mcon: material_context_t = {name : "mesh" + i, bind_textures : []};
		material_context_load(mcon);
		array_push(m.contexts, mcon);
	}

	context_raw.ddirty = 2;

	render_path_raytrace_dirty = 1;
}

function make_material_parse_mesh_preview_material(md: material_data_t = null) {
	if (!make_material_get_mout()) {
		return;
	}

	let m: material_data_t     = md == null ? project_materials[0].data : md;
	let scon: shader_context_t = null;
	for (let i: i32 = 0; i < m._.shader.contexts.length; ++i) {
		let c: shader_context_t = m._.shader.contexts[i];
		if (c.name == "mesh") {
			scon = c;
			break;
		}
	}

	array_remove(m._.shader.contexts, scon);

	let mcon: material_context_t = {name : "mesh", bind_textures : []};

	let sd: material_t             = {name : "Material", canvas : null};
	let con: node_shader_context_t = make_mesh_preview_run(sd, mcon);

	for (let i: i32 = 0; i < m.contexts.length; ++i) {
		if (m.contexts[i].name == "mesh") {
			material_context_load(mcon);
			m.contexts[i] = mcon;
			break;
		}
	}

	if (scon != null) {
		make_material_delete_context(scon);
	}

	let compile_error: bool = false;
	shader_context_load(con.data);
	if (con.data == null) {
		compile_error = true;
	}
	scon = con.data;
	if (compile_error) {
		return;
	}

	array_push(m._.shader.contexts, scon);
}

function make_material_parse_paint_material(bake_previews: bool = true) {
	if (!make_material_get_mout()) {
		return;
	}

	if (bake_previews) {
		let current: gpu_texture_t = _draw_current;
		let in_use: bool           = gpu_in_use;
		if (in_use)
			draw_end();
		make_material_bake_node_previews();
		if (in_use)
			draw_begin(current);
	}

	let m: material_data_t = project_materials[0].data;
	for (let i: i32 = 0; i < m._.shader.contexts.length; ++i) {
		let c: shader_context_t = m._.shader.contexts[i];
		if (c.name == "paint") {
			array_remove(m._.shader.contexts, c);
			if (c != make_material_default_scon) {
				make_material_delete_context(c);
			}
			break;
		}
	}
	for (let i: i32 = 0; i < m.contexts.length; ++i) {
		let c: material_context_t = m.contexts[i];
		if (c.name == "paint") {
			array_remove(m.contexts, c);
			break;
		}
	}

	let sdata: material_t          = {name : "Material", canvas : context_raw.material.canvas};
	let tmcon: material_context_t  = {name : "paint", bind_textures : []};
	let con: node_shader_context_t = make_paint_run(sdata, tmcon);

	let compile_error: bool = false;
	let scon: shader_context_t;
	shader_context_load(con.data);
	if (con.data == null) {
		compile_error = true;
	}
	scon = con.data;
	if (compile_error) {
		return;
	}
	material_context_load(tmcon);
	let mcon: material_context_t = tmcon;

	array_push(m._.shader.contexts, scon);
	array_push(m.contexts, mcon);

	if (make_material_default_scon == null) {
		make_material_default_scon = scon;
	}
	if (make_material_default_mcon == null) {
		make_material_default_mcon = mcon;
	}
}

function make_material_bake_node_previews() {
	context_raw.node_previews_used = [];
	if (context_raw.node_previews == null) {
		context_raw.node_previews = map_create();
	}
	let empty: ui_node_t[] = [];
	make_material_traverse_nodes(context_raw.material.canvas.nodes, null, empty);

	let keys: string[] = map_keys(context_raw.node_previews);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let key: string = keys[i];
		if (array_index_of(context_raw.node_previews_used, key) == -1) {
			let image: gpu_texture_t = map_get(context_raw.node_previews, key);
			gpu_delete_texture(image);
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
				let cname: string   = g.canvas.name;
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
		let id: string           = parser_material_node_name(node, parents);
		let image: gpu_texture_t = map_get(context_raw.node_previews, id);
		array_push(context_raw.node_previews_used, id);
		let res_x: i32 = math_floor(config_get_texture_res_x() / 4);
		let res_y: i32 = math_floor(config_get_texture_res_y() / 4);
		if (image == null || image.width != res_x || image.height != res_y) {
			if (image != null) {
				gpu_delete_texture(image);
			}
			image = gpu_create_render_target(res_x, res_y);
			map_set(context_raw.node_previews, id, image);
		}

		parser_material_blur_passthrough = true;
		util_render_make_node_preview(context_raw.material.canvas, node, image, group, parents);
		parser_material_blur_passthrough = false;
	}
	else if (node.type == "DIRECT_WARP") {
		let id: string           = parser_material_node_name(node, parents);
		let image: gpu_texture_t = map_get(context_raw.node_previews, id);
		array_push(context_raw.node_previews_used, id);
		let res_x: i32 = math_floor(config_get_texture_res_x());
		let res_y: i32 = math_floor(config_get_texture_res_y());
		if (image == null || image.width != res_x || image.height != res_y) {
			if (image != null) {
				gpu_delete_texture(image);
			}
			image = gpu_create_render_target(res_x, res_y);
			map_set(context_raw.node_previews, id, image);
		}

		parser_material_warp_passthrough = true;
		util_render_make_node_preview(context_raw.material.canvas, node, image, group, parents);
		parser_material_warp_passthrough = false;
	}
	else if (node.type == "BAKE_CURVATURE") {
		let id: string           = parser_material_node_name(node, parents);
		let image: gpu_texture_t = map_get(context_raw.node_previews, id);
		array_push(context_raw.node_previews_used, id);
		let res_x: i32 = math_floor(config_get_texture_res_x());
		let res_y: i32 = math_floor(config_get_texture_res_y());
		if (image == null || image.width != res_x || image.height != res_y) {
			if (image != null) {
				gpu_delete_texture(image);
			}
			image = gpu_create_render_target(res_x, res_y, tex_format_t.R8);
			map_set(context_raw.node_previews, id, image);
		}

		if (render_path_paint_live_layer == null) {
			render_path_paint_live_layer = slot_layer_create("_live");
		}

		let _tool: tool_type_t      = context_raw.tool;
		let _bake_type: bake_type_t = context_raw.bake_type;
		context_raw.tool            = tool_type_t.BAKE;
		context_raw.bake_type       = bake_type_t.CURVATURE;

		parser_material_bake_passthrough = true;
		parser_material_start_node       = node;
		parser_material_start_group      = group;
		parser_material_start_parents    = parents;
		make_material_parse_paint_material(false);
		parser_material_bake_passthrough = false;
		parser_material_start_node       = null;
		parser_material_start_group      = null;
		parser_material_start_parents    = null;
		context_raw.pdirty               = 1;
		render_path_paint_use_live_layer(true);
		render_path_paint_commands_paint(false);
		render_path_paint_dilate(true, false);
		render_path_paint_use_live_layer(false);
		context_raw.pdirty = 0;

		context_raw.tool      = _tool;
		context_raw.bake_type = _bake_type;
		make_material_parse_paint_material(false);

		let rts: map_t<string, render_target_t> = render_path_render_targets;
		let texpaint_live: render_target_t      = map_get(rts, "texpaint_live");

		draw_begin(image);
		draw_image(texpaint_live._image, 0, 0);
		draw_end();
	}
}

type parse_node_preview_result_t = {
	scon: shader_context_t; mcon : material_context_t;
};

function make_material_parse_node_preview_material(node: ui_node_t, group: ui_node_canvas_t = null, parents: ui_node_t[] = null): parse_node_preview_result_t {
	if (node.outputs.length == 0) {
		return null;
	}
	let sdata: material_t            = {name : "Material", canvas : context_raw.material.canvas};
	let mcon_raw: material_context_t = {name : "mesh", bind_textures : []};
	let con: node_shader_context_t   = make_node_preview_run(sdata, mcon_raw, node, group, parents);
	let compile_error: bool          = false;
	let scon: shader_context_t;
	shader_context_load(con.data);
	if (con.data == null) {
		compile_error = true;
	}
	scon = con.data;
	if (compile_error) {
		return null;
	}
	material_context_load(mcon_raw);
	let mcon: material_context_t            = mcon_raw;
	let result: parse_node_preview_result_t = {scon : scon, mcon : mcon};
	return result;
}

function make_material_parse_brush() {
	parser_logic_parse(context_raw.brush.canvas);
}

function make_material_blend_mode(kong: node_shader_t, blending: i32, cola: string, colb: string, opac: string): string {
	if (blending == blend_type_t.MIX) {
		return "lerp3(" + cola + ", " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.DARKEN) {
		return "lerp3(" + cola + ", min3(" + cola + ", " + colb + "), " + opac + ")";
	}
	else if (blending == blend_type_t.MULTIPLY) {
		return "lerp3(" + cola + ", " + cola + " * " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.BURN) {
		return "lerp3(" + cola + ", float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - " + cola + ") / " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.LIGHTEN) {
		return "max3(" + cola + ", " + colb + " * " + opac + ")";
	}
	else if (blending == blend_type_t.SCREEN) {
		return "(float3(1.0, 1.0, 1.0) - (float3(1.0 - " + opac + ", 1.0 - " + opac + ", 1.0 - " + opac + ") + " + opac + " * (float3(1.0, 1.0, 1.0) - " +
		       colb + ")) * (float3(1.0, 1.0, 1.0) - " + cola + "))";
	}
	else if (blending == blend_type_t.DODGE) {
		return "lerp3(" + cola + ", " + cola + " / (float3(1.0, 1.0, 1.0) - " + colb + "), " + opac + ")";
	}
	else if (blending == blend_type_t.ADD) {
		return "lerp3(" + cola + ", " + cola + " + " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.OVERLAY) {
		// return "lerp3(" + cola + ", float3( \
		// 	" + cola + ".r < 0.5 ? 2.0 * " + cola + ".r * " + colb + ".r : 1.0 - 2.0 * (1.0 - " + cola + ".r) * (1.0 - " + colb + ".r), \
		// 	" + cola + ".g < 0.5 ? 2.0 * " + cola + ".g * " + colb + ".g : 1.0 - 2.0 * (1.0 - " + cola + ".g) * (1.0 - " + colb + ".g), \
		// 	" + cola + ".b < 0.5 ? 2.0 * " + cola + ".b * " + colb + ".b : 1.0 - 2.0 * (1.0 - " + cola + ".b) * (1.0 - " + colb + ".b) \
		// ), " + opac + ")";
		let cola_rgb: string = string_replace_all(cola, ".", "_") + "_rgb";
		let colb_rgb: string = string_replace_all(colb, ".", "_") + "_rgb";
		let res_r: string    = string_replace_all(cola, ".", "_") + "_res_r";
		let res_g: string    = string_replace_all(cola, ".", "_") + "_res_g";
		let res_b: string    = string_replace_all(cola, ".", "_") + "_res_b";
		node_shader_write_frag(kong, "var " + res_r + ": float;");
		node_shader_write_frag(kong, "var " + res_g + ": float;");
		node_shader_write_frag(kong, "var " + res_b + ": float;");
		node_shader_write_frag(kong, "var " + cola_rgb + ": float3 = " + cola + ";"); // cola_rgb = cola.rgb
		node_shader_write_frag(kong, "var " + colb_rgb + ": float3 = " + colb + ";");
		node_shader_write_frag(kong, "if (" + cola_rgb + ".r < 0.5) { " + res_r + " = 2.0 * " + cola_rgb + ".r * " + colb_rgb + ".r; } else { " + res_r +
		                                 " = 1.0 - 2.0 * (1.0 - " + cola_rgb + ".r) * (1.0 - " + colb_rgb + ".r); }");
		node_shader_write_frag(kong, "if (" + cola_rgb + ".g < 0.5) { " + res_g + " = 2.0 * " + cola_rgb + ".g * " + colb_rgb + ".g; } else { " + res_g +
		                                 " = 1.0 - 2.0 * (1.0 - " + cola_rgb + ".g) * (1.0 - " + colb_rgb + ".g); }");
		node_shader_write_frag(kong, "if (" + cola_rgb + ".b < 0.5) { " + res_b + " = 2.0 * " + cola_rgb + ".b * " + colb_rgb + ".b; } else { " + res_b +
		                                 " = 1.0 - 2.0 * (1.0 - " + cola_rgb + ".b) * (1.0 - " + colb_rgb + ".b); }");
		return "lerp3(" + cola + ", float3(" + res_r + ", " + res_g + ", " + res_b + "), " + opac + ")";
	}
	else if (blending == blend_type_t.SOFT_LIGHT) {
		return "((1.0 - " + opac + ") * " + cola + " + " + opac + " * ((float3(1.0, 1.0, 1.0) - " + cola + ") * " + colb + " * " + cola + " + " + cola +
		       " * (float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - " + colb + ") * (float3(1.0, 1.0, 1.0) - " + cola + "))))";
	}
	else if (blending == blend_type_t.LINEAR_LIGHT) {
		return "(" + cola + " + " + opac + " * (float3(2.0, 2.0, 2.0) * (" + colb + " - float3(0.5, 0.5, 0.5))))";
	}
	else if (blending == blend_type_t.DIFFERENCE) {
		return "lerp3(" + cola + ", abs3(" + cola + " - " + colb + "), " + opac + ")";
	}
	else if (blending == blend_type_t.SUBTRACT) {
		return "lerp3(" + cola + ", " + cola + " - " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.DIVIDE) {
		return "float3(1.0 - " + opac + ", 1.0 - " + opac + ", 1.0 - " + opac + ") * " + cola + " + float3(" + opac + ", " + opac + ", " + opac + ") * " +
		       cola + " / " + colb + "";
	}
	else if (blending == blend_type_t.HUE) {
		node_shader_add_function(kong, str_hue_sat);
		return "lerp3(" + cola + ", hsv_to_rgb(float3(rgb_to_hsv(" + colb + ").r, rgb_to_hsv(" + cola + ").g, rgb_to_hsv(" + cola + ").b)), " + opac + ")";
	}
	else if (blending == blend_type_t.SATURATION) {
		node_shader_add_function(kong, str_hue_sat);
		return "lerp3(" + cola + ", hsv_to_rgb(float3(rgb_to_hsv(" + cola + ").r, rgb_to_hsv(" + colb + ").g, rgb_to_hsv(" + cola + ").b)), " + opac + ")";
	}
	else if (blending == blend_type_t.COLOR) {
		node_shader_add_function(kong, str_hue_sat);
		return "lerp3(" + cola + ", hsv_to_rgb(float3(rgb_to_hsv(" + colb + ").r, rgb_to_hsv(" + colb + ").g, rgb_to_hsv(" + cola + ").b)), " + opac + ")";
	}
	else { // BlendValue
		node_shader_add_function(kong, str_hue_sat);
		return "lerp3(" + cola + ", hsv_to_rgb(float3(rgb_to_hsv(" + cola + ").r, rgb_to_hsv(" + cola + ").g, rgb_to_hsv(" + colb + ").b)), " + opac + ")";
	}
}

function make_material_blend_mode_mask(kong: node_shader_t, blending: i32, cola: string, colb: string, opac: string): string {
	if (blending == blend_type_t.MIX) {
		return "lerp(" + cola + ", " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.DARKEN) {
		return "lerp(" + cola + ", min(" + cola + ", " + colb + "), " + opac + ")";
	}
	else if (blending == blend_type_t.MULTIPLY) {
		return "lerp(" + cola + ", " + cola + " * " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.BURN) {
		return "lerp(" + cola + ", 1.0 - (1.0 - " + cola + ") / " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.LIGHTEN) {
		return "max(" + cola + ", " + colb + " * " + opac + ")";
	}
	else if (blending == blend_type_t.SCREEN) {
		return "(1.0 - ((1.0 - " + opac + ") + " + opac + " * (1.0 - " + colb + ")) * (1.0 - " + cola + "))";
	}
	else if (blending == blend_type_t.DODGE) {
		return "lerp(" + cola + ", " + cola + " / (1.0 - " + colb + "), " + opac + ")";
	}
	else if (blending == blend_type_t.ADD) {
		return "lerp(" + cola + ", " + cola + " + " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.OVERLAY) {
		// return "lerp(" + cola + ", " + cola + " < 0.5 ? 2.0 * " + cola + " * " + colb + " : 1.0 - 2.0 * (1.0 - " + cola + ") * (1.0 - " + colb + "), " + opac
		// + ")";
		let res: string = string_replace_all(cola, ".", "_") + "_res";
		node_shader_write_frag(kong, "var " + res + ": float;");
		node_shader_write_frag(kong, "if (" + cola + " < 0.5) { " + res + " = 2.0 * " + cola + " * " + colb + "; } else { " + res + " = 1.0 - 2.0 * (1.0 - " +
		                                 cola + ") * (1.0 - " + colb + "); }");
		return "lerp(" + cola + ", " + res + ", " + opac + ")";
	}
	else if (blending == blend_type_t.SOFT_LIGHT) {
		return "((1.0 - " + opac + ") * " + cola + " + " + opac + " * ((1.0 - " + cola + ") * " + colb + " * " + cola + " + " + cola + " * (1.0 - (1.0 - " +
		       colb + ") * (1.0 - " + cola + "))))";
	}
	else if (blending == blend_type_t.LINEAR_LIGHT) {
		return "(" + cola + " + " + opac + " * (2.0 * (" + colb + " - 0.5)))";
	}
	else if (blending == blend_type_t.DIFFERENCE) {
		return "lerp(" + cola + ", abs(" + cola + " - " + colb + "), " + opac + ")";
	}
	else if (blending == blend_type_t.SUBTRACT) {
		return "lerp(" + cola + ", " + cola + " - " + colb + ", " + opac + ")";
	}
	else if (blending == blend_type_t.DIVIDE) {
		return "(1.0 - " + opac + ") * " + cola + " + " + opac + " * " + cola + " / " + colb + "";
	}
	else { // BlendHue, BlendSaturation, BlendColor, BlendValue
		return "lerp(" + cola + ", " + colb + ", " + opac + ")";
	}
}

function make_material_get_displace_strength(): f32 {
	let sc: vec4_t = context_main_object().base.transform.scale;
	return config_raw.displace_strength * 0.02 * sc.x;
}

function make_material_delete_context(c: shader_context_t) {
	sys_notify_on_next_frame(function(c: shader_context_t) { // Ensure pipeline is no longer in use
		shader_context_delete(c);
	}, c);
}
