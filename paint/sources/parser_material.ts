let parser_material_con: node_shader_context_t;
let parser_material_kong: node_shader_t;
let parser_material_matcon: material_context_t;
let parser_material_parsed: string[];
let parser_material_parents: ui_node_t[];
let parser_material_canvases: ui_node_canvas_t[];
let parser_material_nodes: ui_node_t[];
let parser_material_links: ui_node_link_t[];
let parser_material_cotangent_frame_written: bool;
let parser_material_tex_coord: string = "tex_coord";
let parser_material_eps: f32 = 0.000001;
let parser_material_node_values: map_t<string, any> = map_create();
let parser_material_node_vectors: map_t<string, any> = map_create();
let parser_material_custom_nodes: map_t<string, any> = map_create(); // JSValue -> (n: ui_node_t, s: string)=>string
let parser_material_parse_surface: bool = true;
let parser_material_parse_opacity: bool = true;
let parser_material_parse_height: bool = false;
let parser_material_parse_height_as_channel: bool = false;
let parser_material_parse_emission: bool = false;
let parser_material_parse_subsurface: bool = false;
let parser_material_parsing_basecolor: bool = false;
let parser_material_triplanar: bool = false; // Sample using tex_coord/1/2 & tex_coord_blend
let parser_material_sample_keep_aspect: bool = false; // Adjust uvs to preserve texture aspect ratio
let parser_material_sample_uv_scale: string = "1.0";
let parser_material_transform_color_space: bool = true;
let parser_material_blur_passthrough: bool = false;
let parser_material_warp_passthrough: bool = false;
let parser_material_bake_passthrough: bool = false;
let parser_material_start_group: ui_node_canvas_t = null;
let parser_material_start_parents: ui_node_t[] = null;
let parser_material_start_node: ui_node_t = null;
let parser_material_arm_export_tangents: bool = true;
let parser_material_out_normaltan: string; // Raw tangent space normal parsed from normal map
let parser_material_script_links: map_t<string, string> = null;
let parser_material_parsed_map: map_t<string, string> = map_create();
let parser_material_texture_map: map_t<string, string> = map_create();
let parser_material_is_frag: bool = true;

function parser_material_get_node(id: i32): ui_node_t {
	for (let i: i32 = 0; i < parser_material_nodes.length; ++i) {
		let n: ui_node_t = parser_material_nodes[i];
		if (n.id == id) {
			return n;
		}
	}
	return null;
}

function parser_material_get_input_link(inp: ui_node_socket_t): ui_node_link_t {
	for (let i: i32 = 0; i < parser_material_links.length; ++i) {
		let l: ui_node_link_t = parser_material_links[i];
		if (l.to_id == inp.node_id) {
			let node: ui_node_t = parser_material_get_node(inp.node_id);
			if (node.inputs.length <= l.to_socket) {
				return null;
			}
			if (node.inputs[l.to_socket] == inp) {
				return l;
			}
		}
	}
	return null;
}

function parser_material_init() {
	parser_material_parsed = [];
	parser_material_parents = [];
	parser_material_cotangent_frame_written = false;
	parser_material_out_normaltan = "float3(0.5, 0.5, 1.0)";
	parser_material_script_links = null;
	parser_material_parsing_basecolor = false;
}

function parser_material_parse(canvas: ui_node_canvas_t, _con: node_shader_context_t, _kong: node_shader_t, _matcon: material_context_t): shader_out_t {
	parser_material_init();
	parser_material_canvases = [canvas];
	parser_material_nodes = canvas.nodes;
	parser_material_links = canvas.links;
	parser_material_con = _con;
	parser_material_kong = _kong;
	parser_material_matcon = _matcon;

	if (parser_material_start_group != null) {
		parser_material_push_group(parser_material_start_group);
		parser_material_parents = parser_material_start_parents;
	}

	if (parser_material_start_node != null) {
		let link: ui_node_link_t = {
			id: 99999,
			from_id: parser_material_start_node.id,
			from_socket: 0,
			to_id: -1,
			to_socket: -1
		};
		parser_material_write_result(link);
		let sout: shader_out_t = {
			out_basecol: "float3(0.0, 0.0, 0.0)",
			out_roughness: "0.0",
			out_metallic: "0.0",
			out_occlusion: "1.0",
			out_opacity: "1.0",
			out_height: "0.0",
			out_emission: "0.0",
			out_subsurface: "0.0"
		};
		return sout;
	}

	let output_node: ui_node_t = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL");
	if (output_node != null) {
		return parser_material_parse_output(output_node);
	}
	output_node = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL_PBR");
	if (output_node != null) {
		return parser_material_parse_output_pbr(output_node);
	}
	return null;
}

function parser_material_finalize(con: node_shader_context_t) {
	let kong: node_shader_t = con.kong;

	if (kong.frag_dotnv) {
		kong.frag_vvec = true;
		kong.frag_n = true;
	}
	if (kong.frag_vvec) {
		kong.frag_wposition = true;
	}

	if (kong.frag_bposition) {
		if (parser_material_triplanar) {
			node_shader_write_attrib_frag(kong, "var bposition: float3 = float3(\
				tex_coord1.x * tex_coord_blend.y + tex_coord2.x * tex_coord_blend.z,\
				tex_coord.x * tex_coord_blend.x + tex_coord2.y * tex_coord_blend.z,\
				tex_coord.y * tex_coord_blend.x + tex_coord1.y * tex_coord_blend.y);");
		}
		else if (kong.frag_ndcpos) {
			node_shader_add_out(kong, "_bposition: float3");
			node_shader_write_vert(kong, "output._bposition = (output.ndc.xyz / output.ndc.w);");
			node_shader_write_attrib_frag(kong, "var bposition: float3 = input._bposition;");
		}
		else {
			node_shader_add_out(kong, "_bposition: float3");
			node_shader_add_constant(kong, "dim: float3", "_dim");
			node_shader_add_constant(kong, "hdim: float3", "_half_dim");
			node_shader_write_attrib_vert(kong, "output._bposition = (input.pos.xyz + constants.hdim) / constants.dim;");
			node_shader_write_attrib_frag(kong, "var bposition: float3 = input._bposition;");
		}
	}
	if (kong.frag_wposition) {
		node_shader_add_constant(kong, "W: float4x4", "_world_matrix");
		node_shader_add_out(kong, "wposition: float3");
		node_shader_write_attrib_vert(kong, "output.wposition = (constants.W * float4(input.pos.xyz, 1.0)).xyz;");
	}
	if (kong.frag_vposition) {
		node_shader_add_constant(kong, "WV: float4x4", "_world_view_matrix");
		node_shader_add_out(kong, "vposition: float3");
		node_shader_write_attrib_vert(kong, "output.vposition = (constants.WV * float4(input.pos.xyz, 1.0)).xyz;");
	}
	if (kong.frag_mposition) {
		node_shader_add_out(kong, "mposition: float3");
		if (kong.frag_ndcpos) {
			node_shader_write_vert(kong, "output.mposition = (output.ndc.xyz / output.ndc.w);");
		}
		else {
			node_shader_write_attrib_vert(kong, "output.mposition = input.pos.xyz;");
		}
	}
	if (kong.frag_wtangent) {
		node_shader_add_out(kong, "wtangent: float3");
		node_shader_write_attrib_vert(kong, "output.wtangent = float3(0.0, 0.0, 0.0);");
	}
	if (kong.frag_vvec_cam) {
		node_shader_add_constant(kong, "WV: float4x4", "_world_view_matrix");
		node_shader_add_out(kong, "eye_dir_cam: float3");
		node_shader_write_attrib_vert(kong, "output.eye_dir_cam = (constants.WV * float4(input.pos.xyz, 1.0)).xyz;");
		node_shader_write_attrib_vert(kong, "output.eye_dir_cam.z *= -1.0;");
		node_shader_write_attrib_frag(kong, "var vvec_cam: float3 = normalize(input.eye_dir_cam);");
	}
	if (kong.frag_vvec) {
		node_shader_add_constant(kong, "eye: float3", "_camera_pos");
		node_shader_add_out(kong, "eye_dir: float3");
		node_shader_write_attrib_vert(kong, "output.eye_dir = constants.eye - output.wposition;");
		node_shader_write_attrib_frag(kong, "var vvec: float3 = normalize(input.eye_dir);");
	}
	if (kong.frag_n) {
		node_shader_add_constant(kong, "N: float3x3", "_normal_matrix");
		node_shader_add_out(kong, "wnormal: float3");
		node_shader_write_attrib_vert(kong, "output.wnormal = constants.N * float3(input.nor.xy, input.pos.w);");
		node_shader_write_attrib_frag(kong, "var n: float3 = normalize(input.wnormal);");
	}
	else if (kong.vert_n) {
		node_shader_add_constant(kong, "N: float3x3", "_normal_matrix");
		node_shader_write_attrib_vert(kong, "var wnormal: float3 = normalize(constants.N * float3(input.nor.xy, input.pos.w));");
	}
	if (kong.frag_nattr) {
		node_shader_add_out(kong, "nattr: float3");
		node_shader_write_attrib_vert(kong, "output.nattr = float3(input.nor.xy, input.pos.w);");
	}
	if (kong.frag_dotnv) {
		node_shader_write_attrib_frag(kong, "var dotnv: float = max(dot(n, vvec), 0.0);");
	}
	if (kong.frag_wvpposition) {
		node_shader_add_out(kong, "wvpposition: float4");
		node_shader_write_end_vert(kong, "output.wvpposition = output.pos;");
	}
	if (node_shader_context_is_elem(con, "col")) {
		node_shader_add_out(kong, "vcolor: float3");
		node_shader_write_attrib_vert(kong, "output.vcolor = input.col.rgb;");
	}
}

function parser_material_parse_output(node: ui_node_t): shader_out_t {
	if (parser_material_parse_surface || parser_material_parse_opacity) {
		return parser_material_parse_shader_input(node.inputs[0]);
	}
	return null;
}

function parser_material_parse_output_pbr(node: ui_node_t): shader_out_t {
	if (parser_material_parse_surface || parser_material_parse_opacity) {
		return parser_material_parse_shader(node, null);
	}
	return null;
}

function parser_material_get_group(name: string): ui_node_canvas_t {
	for (let i: i32 = 0; i < project_material_groups.length; ++i) {
		let g: node_group_t = project_material_groups[i];
		if (g.canvas.name == name) {
			return g.canvas;
		}
	}
	return null;
}

function parser_material_push_group(g: ui_node_canvas_t) {
	array_push(parser_material_canvases, g);
	parser_material_nodes = g.nodes;
	parser_material_links = g.links;
}

function parser_material_pop_group() {
	array_pop(parser_material_canvases);
	let g: ui_node_canvas_t = parser_material_canvases[parser_material_canvases.length - 1];
	parser_material_nodes = g.nodes;
	parser_material_links = g.links;
}

function parser_material_parse_group(node: ui_node_t, socket: ui_node_socket_t): string {
	array_push(parser_material_parents, node); // Entering group
	parser_material_push_group(parser_material_get_group(node.name));
	let output_node: ui_node_t = parser_material_node_by_type(parser_material_nodes, "GROUP_OUTPUT");
	if (output_node == null) {
		return null;
	}
	let index: i32 = parser_material_socket_index(node, socket);
	let inp: ui_node_socket_t = output_node.inputs[index];
	let out_group: string = parser_material_parse_input(inp);
	array_pop(parser_material_parents);
	parser_material_pop_group();
	return out_group;
}

function parser_material_parse_group_input(node: ui_node_t, socket: ui_node_socket_t): string {
	let parent: ui_node_t = array_pop(parser_material_parents); // Leaving group
	parser_material_pop_group();
	let index: i32 = parser_material_socket_index(node, socket);
	let inp: ui_node_socket_t = parent.inputs[index];
	let res: string = parser_material_parse_input(inp);
	array_push(parser_material_parents, parent); // Return to group
	parser_material_push_group(parser_material_get_group(parent.name));
	return res;
}

function parser_material_parse_input(inp: ui_node_socket_t): string {
	if (inp.type == "RGB") {
		return parser_material_parse_vector_input(inp);
	}
	else if (inp.type == "RGBA") {
		return parser_material_parse_vector_input(inp);
	}
	else if (inp.type == "VECTOR") {
		return parser_material_parse_vector_input(inp);
	}
	else if (inp.type == "VALUE") {
		return parser_material_parse_value_input(inp);
	}
	return null;
}

function parser_material_parse_shader_input(inp: ui_node_socket_t): shader_out_t {
	let l: ui_node_link_t = parser_material_get_input_link(inp);
	let from_node: ui_node_t = l != null ? parser_material_get_node(l.from_id) : null;
	if (from_node != null) {
		return parser_material_parse_shader(from_node, from_node.outputs[l.from_socket]);
	}
	else {
		let sout: shader_out_t = {
			out_basecol: "float3(0.8, 0.8, 0.8)",
			out_roughness: "0.0",
			out_metallic: "0.0",
			out_occlusion: "1.0",
			out_opacity: "1.0",
			out_height: "0.0",
			out_emission: "0.0",
			out_subsurface: "0.0"
		};
		return sout;
	}
}

function parser_material_parse_shader(node: ui_node_t, socket: ui_node_socket_t): shader_out_t {
	let sout: shader_out_t = {
		out_basecol: "float3(0.8, 0.8, 0.8)",
		out_roughness: "0.0",
		out_metallic: "0.0",
		out_occlusion: "1.0",
		out_opacity: "1.0",
		out_height: "0.0",
		out_emission: "0.0",
		out_subsurface: "0.0"
	};

	if (node.type == "OUTPUT_MATERIAL_PBR") {
		if (parser_material_parse_surface) {
			// Normal - parsed first to retrieve uv coords
			parse_normal_map_color_input(node.inputs[5]);
			// Base color
			parser_material_parsing_basecolor = true;
			sout.out_basecol = parser_material_parse_vector_input(node.inputs[0]);
			parser_material_parsing_basecolor = false;
			// Occlusion
			sout.out_occlusion = parser_material_parse_value_input(node.inputs[2]);
			// Roughness
			sout.out_roughness = parser_material_parse_value_input(node.inputs[3]);
			// Metallic
			sout.out_metallic = parser_material_parse_value_input(node.inputs[4]);
			// Emission
			if (parser_material_parse_emission) {
				sout.out_emission = parser_material_parse_value_input(node.inputs[6]);
			}
			// Subsurface
			if (parser_material_parse_subsurface) {
				sout.out_subsurface = parser_material_parse_value_input(node.inputs[8]);
			}
		}

		if (parser_material_parse_opacity) {
			sout.out_opacity = parser_material_parse_value_input(node.inputs[1]);
		}

		// Displacement / Height
		if (parser_material_parse_height) {
			if (!parser_material_parse_height_as_channel) {
				parser_material_is_frag = false;
			}
			sout.out_height = parser_material_parse_value_input(node.inputs[7]);
			if (!parser_material_parse_height_as_channel) {
				parser_material_is_frag = true;
			}
		}
	}

	return sout;
}

function parser_material_write(raw: node_shader_t, s: string) {
	if (parser_material_is_frag) {
		node_shader_write_frag(raw, s);
	}
	else {
		node_shader_write_vert(raw, s);
	}
}

function parser_material_parse_vector_input(inp: ui_node_socket_t): string {
	let l: ui_node_link_t = parser_material_get_input_link(inp);
	let from_node: ui_node_t = l != null ? parser_material_get_node(l.from_id) : null;
	if (from_node != null) {
		let res_var: string = parser_material_write_result(l);
		let st: string = from_node.outputs[l.from_socket].type;
		if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
			return res_var;
		}
		else { // VALUE
			return parser_material_to_vec3(res_var);
		}
	}
	else {
		return parser_material_vec3(inp.default_value);
	}
}

function parser_material_parse_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	let node_vector: (node: ui_node_t, socket: ui_node_socket_t)=>string = map_get(parser_material_node_vectors, node.type);
	if (node_vector != null) {
		return node_vector(node, socket);
	}
	else if (node.type == "GROUP_INPUT") {
		return parser_material_parse_group_input(node, socket);
	}
	else if (map_get(parser_material_custom_nodes, node.type) != null) {
		let cb: any = map_get(parser_material_custom_nodes, node.type); // JSValue -> (n: ui_node_t, s: string)=>string
		return js_call_ptr_str(cb, node, socket.name);
	}
	return "float3(0.0, 0.0, 0.0)";
}

function parse_normal_map_color_input(inp: ui_node_socket_t) {
	parser_material_kong.frag_write_normal++;
	parser_material_out_normaltan = parser_material_parse_vector_input(inp);
	let _parser_material_is_frag: bool = parser_material_is_frag;
	parser_material_is_frag = true;
	if (!parser_material_arm_export_tangents) {
		parser_material_write(parser_material_kong, "var texn: float3 = (" + parser_material_out_normaltan + ") * 2.0 - 1.0;");
		parser_material_write(parser_material_kong, "texn.y = -texn.y;");
		if (!parser_material_cotangent_frame_written) {
			parser_material_cotangent_frame_written = true;
			node_shader_add_function(parser_material_kong, str_cotangent_frame);
		}
		parser_material_kong.frag_n = true;
		parser_material_write(parser_material_kong, "var TBN: float3x3 = cotangent_frame(n, vvec, tex_coord);");

		parser_material_write(parser_material_kong, "n = TBN * normalize(texn);");
	}
	parser_material_is_frag = _parser_material_is_frag;
	parser_material_kong.frag_write_normal--;
}

function parser_material_parse_value_input(inp: ui_node_socket_t, vector_as_grayscale: bool = false): string {
	let l: ui_node_link_t = parser_material_get_input_link(inp);
	let from_node: ui_node_t = l != null ? parser_material_get_node(l.from_id) : null;
	if (from_node != null) {
		let res_var: string = parser_material_write_result(l);
		let st: string = from_node.outputs[l.from_socket].type;
		if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
			if (vector_as_grayscale) {
				return "dot(" + res_var + ".rbg, float3(0.299, 0.587, 0.114))";
			}
			else {
				return res_var + ".x";
			}
		}
		else { // VALUE
			return res_var;
		}
	}
	else {
		return parser_material_vec1(inp.default_value[0]);
	}
}

function parser_material_parse_value(node: ui_node_t, socket: ui_node_socket_t): string {
	let node_value: (node: ui_node_t, socket: ui_node_socket_t)=>string = map_get(parser_material_node_values, node.type);
	if (node_value != null) {
		return node_value(node, socket);
	}
	else if (node.type == "GROUP_INPUT") {
		return parser_material_parse_group_input(node, socket);
	}
	else if (map_get(parser_material_custom_nodes, node.type) != null) {
		let cb: any = map_get(parser_material_custom_nodes, node.type);
		return js_call_ptr_str(cb, node, socket.name);
	}
	return "0.0";
}

function parser_material_get_coord(node: ui_node_t): string {
	if (parser_material_get_input_link(node.inputs[0]) != null) {
		return parser_material_parse_vector_input(node.inputs[0]);
	}
	else {
		parser_material_kong.frag_bposition = true;
		return "bposition";
	}
}

function parser_material_res_var_name(node: ui_node_t, socket: ui_node_socket_t): string {
	return parser_material_node_name(node) + "_" + parser_material_safesrc(socket.name) + "_res";
}

function parser_material_write_result(l: ui_node_link_t): string {
	let from_node: ui_node_t = parser_material_get_node(l.from_id);
	let from_socket: ui_node_socket_t = from_node.outputs[l.from_socket];
	let res_var: string = parser_material_res_var_name(from_node, from_socket);
	let st: string = from_socket.type;
	if (array_index_of(parser_material_parsed, res_var) < 0) {
		array_push(parser_material_parsed, res_var);
		if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
			let res: string = parser_material_parse_vector(from_node, from_socket);
			if (res == null) {
				return null;
			}
			map_set(parser_material_parsed_map, res_var, res);
			parser_material_write(parser_material_kong, "var " + res_var + ": float3 = " + res + ";");
		}
		else if (st == "VALUE") {
			let res: string = parser_material_parse_value(from_node, from_socket);
			if (res == null) {
				return null;
			}
			map_set(parser_material_parsed_map, res_var, res);
			parser_material_write(parser_material_kong, "var " + res_var + ": float = " + res + ";");
		}
	}
	return res_var;
}

function parser_material_store_var_name(node: ui_node_t): string {
	return parser_material_node_name(node) + "_store";
}

function parser_material_vec1(v: f32): string {
	return f32_to_string_with_zeros(v);
	// return "float(" + v + ")";
	// return v + "";
}

function parser_material_vec3(v: f32_array_t): string {
	// let v0: f32 = v[0];
	// let v1: f32 = v[1];
	// let v2: f32 = v[2];
	let v0: string = f32_to_string_with_zeros(v[0]);
	let v1: string = f32_to_string_with_zeros(v[1]);
	let v2: string = f32_to_string_with_zeros(v[2]);
	return "float3(" + v0 + ", " + v1 + ", " + v2 + ")";
}

function parser_material_to_vec3(s: string): string {
	// return "float3(" + s + ")";
	return "float3(" + s + ", " + s + ", " + s + ")";
}

function parser_material_node_by_type(nodes: ui_node_t[], ntype: string): ui_node_t {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: ui_node_t = nodes[i];
		if (n.type == ntype) {
			return n;
		}
	}
	return null;
}

function parser_material_socket_index(node: ui_node_t, socket: ui_node_socket_t): i32 {
	for (let i: i32 = 0; i < node.outputs.length; ++i) {
		if (node.outputs[i] == socket) {
			return i;
		}
	}
	return -1;
}

function parser_material_node_name(node: ui_node_t, _parents: ui_node_t[] = null): string {
	if (_parents == null) {
		_parents = parser_material_parents;
	}
	let s: string = node.name;
	for (let i: i32 = 0; i < _parents.length; ++i) {
		let p: ui_node_t = _parents[i];
		s = p.name + p.id + "_" + s;
	}
	s = parser_material_safesrc(s);
	let nid: i32 = node.id;
	s = s + nid;
	return s;
}

function parser_material_safesrc(s: string): string {
	for (let i: i32 = 0; i < s.length; ++i) {
		let code: i32 = char_code_at(s, i);
		let letter: bool = (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
		let digit: bool = code >= 48 && code <= 57;
		if (!letter && !digit) {
			s = string_replace_all(s, char_at(s, i), "_");
		}
		if (i == 0 && digit) {
			s = "_" + s;
		}
	}
	return s;
}

function parser_material_enum_data(s: string): string {
	for (let i: i32 = 0; i < project_assets.length; ++i) {
		let a: asset_t = project_assets[i];
		if (a.name == s) {
			return a.file;
		}
	}
	return "";
}

function parser_material_make_bind_tex(tex_name: string, file: string): bind_tex_t {
	let tex: bind_tex_t = {
		name: tex_name,
		file: file
	};
	return tex;
}

function u8_array_string_at(a: u8_array_t, i: i32): string {
	let s: string = u8_array_to_string(a);
	let ss: string[] = string_split(s, "\n");
	return ss[i];
}

type shader_out_t = {
	out_basecol?: string;
	out_roughness?: string;
	out_metallic?: string;
	out_occlusion?: string;
	out_opacity?: string;
	out_height?: string;
	out_emission?: string;
	out_subsurface?: string;
};
