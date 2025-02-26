//
// This module builds upon Cycles nodes work licensed as
// Copyright 2011-2013 Blender Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

let parser_material_con: node_shader_context_t;
let parser_material_vert: node_shader_t;
let parser_material_frag: node_shader_t;
let parser_material_curshader: node_shader_t;
let parser_material_matcon: material_context_t;
let parser_material_parsed: string[];
let parser_material_parents: ui_node_t[];

let parser_material_canvases: ui_node_canvas_t[];
let parser_material_nodes: ui_node_t[];
let parser_material_links: ui_node_link_t[];

let parser_material_cotangent_frame_written: bool;
let parser_material_tex_coord: string = "tex_coord";
let parser_material_eps: f32 = 0.000001;

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
let parser_material_bake_passthrough_strength: string = "0.0";
let parser_material_bake_passthrough_radius: string = "0.0";
let parser_material_bake_passthrough_offset: string = "0.0";
let parser_material_start_group: ui_node_canvas_t = null;
let parser_material_start_parents: ui_node_t[] = null;
let parser_material_start_node: ui_node_t = null;

let parser_material_arm_export_tangents: bool = true;
let parser_material_out_normaltan: string; // Raw tangent space normal parsed from normal map

let parser_material_script_links: map_t<string, string> = null;

let parser_material_parsed_map: map_t<string, string> = map_create();
let parser_material_texture_map: map_t<string, string> = map_create();

function parser_material_get_node(id: i32): ui_node_t {
	for (let i: i32 = 0; i < parser_material_nodes.length; ++i) {
		let n: ui_node_t = parser_material_nodes[i];
		if (n.id == id) {
			return n;
		}
	}
	return null;
}

function parser_material_get_link(id: i32): ui_node_link_t {
	for (let i: i32 = 0; i < parser_material_links.length; ++i) {
		let l: ui_node_link_t = parser_material_links[i];
		if (l.id == id) {
			return l;
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

function parser_material_get_output_links(out: ui_node_socket_t): ui_node_link_t[] {
	let ls: ui_node_link_t[] = null;
	for (let i: i32 = 0; i < parser_material_links.length; ++i) {
		let l: ui_node_link_t = parser_material_links[i];
		if (l.from_id == out.node_id) {
			let node: ui_node_t = parser_material_get_node(out.node_id);
			if (node.outputs.length <= l.from_socket) {
				continue;
			}
			if (node.outputs[l.from_socket] == out) {
				if (ls == null) {
					ls = [];
				}
				array_push(ls, l);
			}
		}
	}
	return ls;
}

function parser_material_init() {
	parser_material_parsed = [];
	parser_material_parents = [];
	parser_material_cotangent_frame_written = false;
	parser_material_out_normaltan = "vec3(0.5, 0.5, 1.0)";
	parser_material_script_links = null;
	parser_material_parsing_basecolor = false;
}

function parser_material_parse(canvas: ui_node_canvas_t, _con: node_shader_context_t, _vert: node_shader_t, _frag: node_shader_t, _matcon: material_context_t): shader_out_t {
	parser_material_init();
	parser_material_canvases = [canvas];
	parser_material_nodes = canvas.nodes;
	parser_material_links = canvas.links;
	parser_material_con = _con;
	parser_material_vert = _vert;
	parser_material_frag = _frag;
	parser_material_curshader = parser_material_frag;
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
			out_basecol: "vec3(0.0, 0.0, 0.0)",
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
	let vert: node_shader_t = con.vert;
	let frag: node_shader_t = con.frag;

	if (frag.dotnv) {
		frag.vvec = true;
		frag.n = true;
	}
	if (frag.vvec) {
		vert.wposition = true;
	}

	if (frag.bposition) {
		if (parser_material_triplanar) {
			node_shader_write_attrib(frag, "vec3 bposition = vec3(\
				tex_coord1.x * tex_coord_blend.y + tex_coord2.x * tex_coord_blend.z,\
				tex_coord.x * tex_coord_blend.x + tex_coord2.y * tex_coord_blend.z,\
				tex_coord.y * tex_coord_blend.x + tex_coord1.y * tex_coord_blend.y);");
		}
		else if (frag.ndcpos) {
			node_shader_add_out(vert, "vec3 bposition");
			node_shader_write(vert, "bposition = (ndc.xyz / ndc.w);");
		}
		else {
			node_shader_add_out(vert, "vec3 bposition");
			node_shader_add_uniform(vert, "vec3 dim", "_dim");
			node_shader_add_uniform(vert, "vec3 hdim", "_half_dim");
			node_shader_write_attrib(vert, "bposition = (pos.xyz + hdim) / dim;");
		}
	}
	if (frag.wposition) {
		node_shader_add_uniform(vert, "mat4 W", "_world_matrix");
		node_shader_add_out(vert, "vec3 wposition");
		node_shader_write_attrib(vert, "wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;");
	}
	else if (vert.wposition) {
		node_shader_add_uniform(vert, "mat4 W", "_world_matrix");
		node_shader_write_attrib(vert, "vec3 wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;");
	}
	if (frag.vposition) {
		node_shader_add_uniform(vert, "mat4 WV", "_world_view_matrix");
		node_shader_add_out(vert, "vec3 vposition");
		node_shader_write_attrib(vert, "vposition = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz;");
	}
	if (frag.mposition) {
		node_shader_add_out(vert, "vec3 mposition");
		if (frag.ndcpos) {
			node_shader_write(vert, "mposition = (ndc.xyz / ndc.w);");
		}
		else {
			node_shader_write_attrib(vert, "mposition = pos.xyz;");
		}
	}
	if (frag.wtangent) {
		// NodeShaderadd_elem(con, "tang", "short4norm");
		// add_uniform(vert, "mat3 N", "_normal_matrix");
		node_shader_add_out(vert, "vec3 wtangent");
		// write_attrib(vert, "wtangent = normalize(mul(tang.xyz, N));");
		node_shader_write_attrib(vert, "wtangent = vec3(0.0, 0.0, 0.0);");
	}
	if (frag.vvec_cam) {
		node_shader_add_uniform(vert, "mat4 WV", "_world_view_matrix");
		node_shader_add_out(vert, "vec3 eye_dir_cam");
		node_shader_write_attrib(vert, "eye_dir_cam = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz; eye_dir_cam.z *= -1.0;");
		node_shader_write_attrib(frag, "vec3 vvec_cam = normalize(eye_dir_cam);");
	}
	if (frag.vvec) {
		node_shader_add_uniform(vert, "vec3 eye", "_camera_pos");
		node_shader_add_out(vert, "vec3 eye_dir");
		node_shader_write_attrib(vert, "eye_dir = eye - wposition;");
		node_shader_write_attrib(frag, "vec3 vvec = normalize(eye_dir);");
	}
	if (frag.n) {
		node_shader_add_uniform(vert, "mat3 N", "_normal_matrix");
		node_shader_add_out(vert, "vec3 wnormal");
		node_shader_write_attrib(vert, "wnormal = mul(vec3(nor.xy, pos.w), N);");
		node_shader_write_attrib(frag, "vec3 n = normalize(wnormal);");
	}
	else if (vert.n) {
		node_shader_add_uniform(vert, "mat3 N", "_normal_matrix");
		node_shader_write_attrib(vert, "vec3 wnormal = normalize(mul(vec3(nor.xy, pos.w), N));");
	}
	if (frag.nattr) {
		node_shader_add_out(vert, "vec3 nattr");
		node_shader_write_attrib(vert, "nattr = vec3(nor.xy, pos.w);");
	}
	if (frag.dotnv) {
		node_shader_write_attrib(frag, "float dotnv = max(dot(n, vvec), 0.0);");
	}
	if (frag.wvpposition) {
		node_shader_add_out(vert, "vec4 wvpposition");
		node_shader_write_end(vert, "wvpposition = gl_Position;");
	}
	if (node_shader_context_is_elem(con, "col")) {
		node_shader_add_out(vert, "vec3 vcolor");
		node_shader_write_attrib(vert, "vcolor = col.rgb;");
	}
}

function parser_material_parse_output(node: ui_node_t): shader_out_t {
	if (parser_material_parse_surface || parser_material_parse_opacity) {
		return parser_material_parse_shader_input(node.inputs[0]);
	}
	return null;
	// Parse volume, displacement..
}

function parser_material_parse_output_pbr(node: ui_node_t): shader_out_t {
	if (parser_material_parse_surface || parser_material_parse_opacity) {
		return parser_material_parse_shader(node, null);
	}
	return null;
	// Parse volume, displacement..
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
		if (from_node.type == "REROUTE") {
			return parser_material_parse_shader_input(from_node.inputs[0]);
		}
		return parser_material_parse_shader(from_node, from_node.outputs[l.from_socket]);
	}
	else {
		let sout: shader_out_t = {
			out_basecol: "vec3(0.8, 0.8, 0.8)",
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
		out_basecol: "vec3(0.8, 0.8, 0.8)",
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
		if (node.inputs.length > 7 && parser_material_parse_height) {
			if (!parser_material_parse_height_as_channel) {
				parser_material_curshader = parser_material_vert;
			}
			sout.out_height = parser_material_parse_value_input(node.inputs[7]);
			if (!parser_material_parse_height_as_channel) {
				parser_material_curshader = parser_material_frag;
			}
		}
	}

	return sout;
}

function parser_material_parse_vector_input(inp: ui_node_socket_t): string {
	let l: ui_node_link_t = parser_material_get_input_link(inp);
	let from_node: ui_node_t = l != null ? parser_material_get_node(l.from_id) : null;
	if (from_node != null) {
		if (from_node.type == "REROUTE") {
			return parser_material_parse_vector_input(from_node.inputs[0]);
		}

		let res_var: string = parser_material_write_result(l);
		let st: string = from_node.outputs[l.from_socket].type;
		if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
			return res_var;
		}
		else {// VALUE
			return parser_material_to_vec3(res_var);
		}
	}
	else {
		if (inp.type == "VALUE") { // Unlinked reroute
			return parser_material_vec3(f32_array_create_xyz(0.0, 0.0, 0.0));
		}
		else {
			return parser_material_vec3(inp.default_value);
		}
	}
}

function _parser_material_cache_tex_text_node(file: string, text: string) {
	if (map_get(data_cached_images, file) == null) {
		app_notify_on_init(function(text: string) {
			let _text_tool_text: string = context_raw.text_tool_text;
			let _text_tool_image: image_t = context_raw.text_tool_image;
			context_raw.text_tool_text = text;
			context_raw.text_tool_image = null;

			util_render_make_text_preview();
			let file: string = "tex_text_" + text;

			// TODO: remove old cache
			map_set(data_cached_images, file, context_raw.text_tool_image);

			context_raw.text_tool_text = _text_tool_text;
			context_raw.text_tool_image = _text_tool_image;
		}, text);
	}
}

function parser_material_parse_vector(node: ui_node_t, socket: ui_node_socket_t): string {
	if (node.type == "GROUP") {
		return parser_material_parse_group(node, socket);
	}
	else if (node.type == "GROUP_INPUT") {
		return parser_material_parse_group_input(node, socket);
	}
	else if (node.type == "ATTRIBUTE") {
		if (socket == node.outputs[0]) { // Color
			if (parser_material_curshader.context.allow_vcols) {
				node_shader_context_add_elem(parser_material_curshader.context, "col", "short4norm"); // Vcols only for now
				return "vcolor";
			}
			else {
				return("vec3(0.0, 0.0, 0.0)");
			}
		}
		else { // Vector
			node_shader_context_add_elem(parser_material_curshader.context, "tex", "short2norm"); // UVMaps only for now
			return "vec3(tex_coord.x, tex_coord.y, 0.0)";
		}
	}
	else if (node.type == "VERTEX_COLOR") {
		if (parser_material_curshader.context.allow_vcols) {
			node_shader_context_add_elem(parser_material_curshader.context, "col", "short4norm");
			return "vcolor";
		}
		else {
			return("vec3(0.0, 0.0, 0.0)");
		}

	}
	else if (node.type == "RGB") {
		return parser_material_vec3(socket.default_value);
	}
	else if (node.type == "TEX_BRICK") {
		node_shader_add_function(parser_material_curshader, str_tex_brick);
		let co: string = parser_material_get_coord(node);
		let col1: string = parser_material_parse_vector_input(node.inputs[1]);
		let col2: string = parser_material_parse_vector_input(node.inputs[2]);
		let col3: string = parser_material_parse_vector_input(node.inputs[3]);
		let scale: string = parser_material_parse_value_input(node.inputs[4]);
		let res: string = "tex_brick(" + co + " * " + scale + ", " + col1 + ", " + col2 + ", " + col3 + ")";
		return res;
	}
	else if (node.type == "TEX_CHECKER") {
		node_shader_add_function(parser_material_curshader, str_tex_checker);
		let co: string = parser_material_get_coord(node);
		let col1: string = parser_material_parse_vector_input(node.inputs[1]);
		let col2: string = parser_material_parse_vector_input(node.inputs[2]);
		let scale: string = parser_material_parse_value_input(node.inputs[3]);
		let res: string = "tex_checker(" + co + ", " + col1 + ", " + col2 + ", " + scale + ")";
		return res;
	}
	else if (node.type == "TEX_GRADIENT") {
		let co: string = parser_material_get_coord(node);
		let but: ui_node_button_t = node.buttons[0]; //gradient_type;
		let grad: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		grad = string_replace_all(grad, " ", "_");
		let f: string = parser_material_get_gradient(grad, co);
		let res: string = parser_material_to_vec3("clamp(" + f + ", 0.0, 1.0)");
		return res;
	}
	else if (node.type == "TEX_IMAGE") {
		// Already fetched
		if (array_index_of(parser_material_parsed, parser_material_res_var_name(node, node.outputs[1])) >= 0) { // TODO: node.outputs[0]
			let varname: string = parser_material_store_var_name(node);
			return varname + ".rgb";
		}
		let tex_name: string = parser_material_node_name(node);
		let tex: bind_tex_t = parser_material_make_texture(node, tex_name);
		if (tex != null) {
			let color_space: i32 = node.buttons[1].default_value[0];
			let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space);
			return texstore + ".rgb";
		}
		else {
			let tex_store: string = parser_material_store_var_name(node); // Pink color for missing texture
			node_shader_write(parser_material_curshader, "vec4 " + tex_store + " = vec4(1.0, 0.0, 1.0, 1.0);");
			return tex_store + ".rgb";
		}
	}
	else if (node.type == "TEX_TEXT") {
		let tex_name: string = parser_material_node_name(node);
		let text_buffer: buffer_t = node.buttons[0].default_value;
		let text: string = sys_buffer_to_string(text_buffer);
		let file: string = "tex_text_" + text;
		_parser_material_cache_tex_text_node(file, text);
		let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, file);
		let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
		return texstore + ".rrr";
	}
	else if (node.type == "TEX_MAGIC") {
		node_shader_add_function(parser_material_curshader, str_tex_magic);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let res: string = "tex_magic(" + co + " * " + scale + " * 4.0)";
		return res;
	}
	else if (node.type == "TEX_NOISE") {
		node_shader_add_function(parser_material_curshader, str_tex_noise);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let res: string = "vec3(tex_noise(" + co + " * " + scale + "), tex_noise(" + co + " * " + scale + " + 0.33), tex_noise(" + co + " * " + scale + " + 0.66))";
		return res;
	}
	else if (node.type == "TEX_VORONOI") {
		node_shader_add_function(parser_material_curshader, str_tex_voronoi);
		node_shader_add_uniform(parser_material_curshader, "sampler2D snoise256", "$noise256.k");
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let but: ui_node_button_t = node.buttons[0]; //coloring;
		let coloring: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		coloring = string_replace_all(coloring, " ", "_");
		let res: string = "";
		if (coloring == "INTENSITY") {
			let voronoi: string = "tex_voronoi(" + co + " * " + scale + ", texturePass(snoise256)).a";
			res = parser_material_to_vec3(voronoi);
		}
		else { // Cells
			res = "tex_voronoi(" + co + " * " + scale + ", texturePass(snoise256)).rgb";
		}
		return res;
	}
	else if (node.type == "TEX_WAVE") {
		node_shader_add_function(parser_material_curshader, str_tex_wave);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let res: string = parser_material_to_vec3("tex_wave_f(" + co + " * " + scale + ")");
		return res;
	}
	else if (node.type == "BRIGHTCONTRAST") {
		let out_col: string = parser_material_parse_vector_input(node.inputs[0]);
		let bright: string = parser_material_parse_value_input(node.inputs[1]);
		let contr: string = parser_material_parse_value_input(node.inputs[2]);
		node_shader_add_function(parser_material_curshader, str_brightcontrast);
		return "brightcontrast(" + out_col + ", " + bright + ", " + contr + ")";
	}
	else if (node.type == "GAMMA") {
		let out_col: string = parser_material_parse_vector_input(node.inputs[0]);
		let gamma: string = parser_material_parse_value_input(node.inputs[1]);
		return "pow(" + out_col + ", " + parser_material_to_vec3(gamma) + ")";
	}
	else if (node.type == "DIRECT_WARP") {
		if (parser_material_warp_passthrough) {
			return parser_material_parse_vector_input(node.inputs[0]);
		}
		let angle: string = parser_material_parse_value_input(node.inputs[1], true);
		let mask: string = parser_material_parse_value_input(node.inputs[2], true);
		let tex_name: string = "texwarp_" + parser_material_node_name(node);
		node_shader_add_uniform(parser_material_curshader, "sampler2D " + tex_name, "_" + tex_name);
		let store: string = parser_material_store_var_name(node);
		let pi: f32 = math_pi();
		node_shader_write(parser_material_curshader, "float " + store + "_rad = " + angle + " * (" + pi + " / 180);");
		node_shader_write(parser_material_curshader, "float " + store + "_x = cos(" + store + "_rad);");
		node_shader_write(parser_material_curshader, "float " + store + "_y = sin(" + store + "_rad);");
		return "texture(" + tex_name + ", tex_coord + vec2(" + store + "_x, " + store + "_y) * " + mask + ").rgb;";
	}
	else if (node.type == "BLUR") {
		if (parser_material_blur_passthrough) {
			return parser_material_parse_vector_input(node.inputs[0]);
		}
		let strength: string = parser_material_parse_value_input(node.inputs[1]);
		if (strength == "0.0") {
			return "vec3(0.0, 0.0, 0.0)";
		}
		let steps: string = "int(" + strength + " * 10 + 1)";
		let tex_name: string = "texblur_" + parser_material_node_name(node);
		node_shader_add_uniform(parser_material_curshader, "sampler2D " + tex_name, "_" + tex_name);
		let store: string = parser_material_store_var_name(node);
		node_shader_write(parser_material_curshader, "vec3 " + store + "_res = vec3(0.0, 0.0, 0.0);");
		node_shader_write(parser_material_curshader, "for (int i = -" + steps + "; i <= " + steps + "; ++i) {");
		node_shader_write(parser_material_curshader, "for (int j = -" + steps + "; j <= " + steps + "; ++j) {");
		node_shader_write(parser_material_curshader, store + "_res += texture(" + tex_name + ", tex_coord + vec2(i, j) / vec2(textureSize(" + tex_name + ", 0))).rgb;");
		node_shader_write(parser_material_curshader, "}");
		node_shader_write(parser_material_curshader, "}");
		node_shader_write(parser_material_curshader, store + "_res /= (" + steps + " * 2 + 1) * (" + steps + " * 2 + 1);");
		return store + "_res";
	}
	else if (node.type == "HUE_SAT") {
		node_shader_add_function(parser_material_curshader, str_hue_sat);
		let hue: string = parser_material_parse_value_input(node.inputs[0]);
		let sat: string = parser_material_parse_value_input(node.inputs[1]);
		let val: string = parser_material_parse_value_input(node.inputs[2]);
		let fac: string = parser_material_parse_value_input(node.inputs[3]);
		let col: string = parser_material_parse_vector_input(node.inputs[4]);
		return "hue_sat(" + col + ", vec4(" + hue + "-0.5, " + sat + ", " + val + ", 1.0-" + fac + "))";
	}
	else if (node.type == "INVERT") {
		let fac: string = parser_material_parse_value_input(node.inputs[0]);
		let out_col: string = parser_material_parse_vector_input(node.inputs[1]);
		return "mix(" + out_col + ", vec3(1.0, 1.0, 1.0) - (" + out_col + "), " + fac + ")";
	}
	else if (node.type == "MIX_RGB") {
		let fac: string = parser_material_parse_value_input(node.inputs[0]);
		let fac_var: string = parser_material_node_name(node) + "_fac";
		node_shader_write(parser_material_curshader, "float " + fac_var + " = " + fac + ";");
		let col1: string = parser_material_parse_vector_input(node.inputs[1]);
		let col2: string = parser_material_parse_vector_input(node.inputs[2]);
		let but: ui_node_button_t = node.buttons[0]; // blend_type
		let blend: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		blend = string_replace_all(blend, " ", "_");
		let use_clamp: bool = node.buttons[1].default_value[0] > 0;
		let out_col: string = "";
		if (blend == "MIX") {
			out_col = "mix(" + col1 + ", " + col2 + ", " + fac_var + ")";
		}
		else if (blend == "DARKEN") {
			out_col = "min(" + col1 + ", " + col2 + " * " + fac_var + ")";
		}
		else if (blend == "MULTIPLY") {
			out_col = "mix(" + col1 + ", " + col1 + " * " + col2 + ", " + fac_var + ")";
		}
		else if (blend == "BURN") {
			out_col = "mix(" + col1 + ", vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - " + col1 + ") / " + col2 + ", " + fac_var + ")";
		}
		else if (blend == "LIGHTEN") {
			out_col = "max(" + col1 + ", " + col2 + " * " + fac_var + ")";
		}
		else if (blend == "SCREEN") {
			let v3: string = parser_material_to_vec3("1.0 - " + fac_var + "");
			out_col = "(vec3(1.0, 1.0, 1.0) - (" + v3 + " + " + fac_var + " * (vec3(1.0, 1.0, 1.0) - " + col2 + ")) * (vec3(1.0, 1.0, 1.0) - " + col1 + "))";
		}
		else if (blend == "DODGE") {
			out_col = "mix(" + col1 + ", " + col1 + " / (vec3(1.0, 1.0, 1.0) - " + col2 + "), " + fac_var + ")";
		}
		else if (blend == "ADD") {
			out_col = "mix(" + col1 + ", " + col1 + " + " + col2 + ", " + fac_var + ")";
		}
		else if (blend == "OVERLAY") {
			out_col = "mix(" + col1 + ", vec3( \
				" + col1 + ".r < 0.5 ? 2.0 * " + col1 + ".r * " + col2 + ".r : 1.0 - 2.0 * (1.0 - " + col1 + ".r) * (1.0 - " + col2 + ".r), \
				" + col1 + ".g < 0.5 ? 2.0 * " + col1 + ".g * " + col2 + ".g : 1.0 - 2.0 * (1.0 - " + col1 + ".g) * (1.0 - " + col2 + ".g), \
				" + col1 + ".b < 0.5 ? 2.0 * " + col1 + ".b * " + col2 + ".b : 1.0 - 2.0 * (1.0 - " + col1 + ".b) * (1.0 - " + col2 + ".b) \
			), " + fac_var + ")";
		}
		else if (blend == "SOFT_LIGHT") {
			out_col = "((1.0 - " + fac_var + ") * " + col1 + " + " + fac_var + " * ((vec3(1.0, 1.0, 1.0) - " + col1 + ") * " + col2 + " * " + col1 + " + " + col1 + " * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - " + col2 + ") * (vec3(1.0, 1.0, 1.0) - " + col1 + "))))";
		}
		else if (blend == "LINEAR_LIGHT") {
			out_col = "(" + col1 + " + " + fac_var + " * (vec3(2.0, 2.0, 2.0) * (" + col2 + " - vec3(0.5, 0.5, 0.5))))";
		}
		else if (blend == "DIFFERENCE") {
			out_col = "mix(" + col1 + ", abs(" + col1 + " - " + col2 + "), " + fac_var + ")";
		}
		else if (blend == "SUBTRACT") {
			out_col = "mix(" + col1 + ", " + col1 + " - " + col2 + ", " + fac_var + ")";
		}
		else if (blend == "DIVIDE") {
			let eps: f32 = 0.000001;
			col2 = "max(" + col2 + ", vec3(" + eps + ", " + eps + ", " + eps + "))";
			let v3: string = parser_material_to_vec3("(1.0 - " + fac_var + ") * " + col1 + " + " + fac_var + " * " + col1 + " / " + col2);
			out_col = "(" + v3 + ")";
		}
		else if (blend == "HUE") {
			node_shader_add_function(parser_material_curshader, str_hue_sat);
			out_col = "mix(" + col1 + ", hsv_to_rgb(vec3(rgb_to_hsv(" + col2 + ").r, rgb_to_hsv(" + col1 + ").g, rgb_to_hsv(" + col1 + ").b)), " + fac_var + ")";
		}
		else if (blend == "SATURATION") {
			node_shader_add_function(parser_material_curshader, str_hue_sat);
			out_col = "mix(" + col1 + ", hsv_to_rgb(vec3(rgb_to_hsv(" + col1 + ").r, rgb_to_hsv(" + col2 + ").g, rgb_to_hsv(" + col1 + ").b)), " + fac_var + ")";
		}
		else if (blend == "COLOR") {
			node_shader_add_function(parser_material_curshader, str_hue_sat);
			out_col = "mix(" + col1 + ", hsv_to_rgb(vec3(rgb_to_hsv(" + col2 + ").r, rgb_to_hsv(" + col2 + ").g, rgb_to_hsv(" + col1 + ").b)), " + fac_var + ")";
		}
		else if (blend == "VALUE") {
			node_shader_add_function(parser_material_curshader, str_hue_sat);
			out_col = "mix(" + col1 + ", hsv_to_rgb(vec3(rgb_to_hsv(" + col1 + ").r, rgb_to_hsv(" + col1 + ").g, rgb_to_hsv(" + col2 + ").b)), " + fac_var + ")";
		}
		if (use_clamp) {
			return "clamp(" + out_col + ", vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0))";
		}
		else {
			return out_col;
		}
	}
	else if (node.type == "QUANTIZE") {
		let strength: string = parser_material_parse_value_input(node.inputs[0]);
		let col: string = parser_material_parse_vector_input(node.inputs[1]);
		return "(floor(100.0 * " + strength + " * " + col + ") / (100.0 * " + strength + "))";
	}
	else if (node.type == "REPLACECOL") {
		let input_color: string = parser_material_parse_vector_input(node.inputs[0]);
		let old_color: string = parser_material_parse_vector_input(node.inputs[1]);
		let new_color: string = parser_material_parse_vector_input(node.inputs[2]);
		let radius: string = parser_material_parse_value_input(node.inputs[3]);
		let fuzziness: string = parser_material_parse_value_input(node.inputs[4]);
		return "mix(" + new_color + ", " + input_color + ", clamp((distance(" + old_color + ", " + input_color + ") - " + radius + ") / max(" + fuzziness + ", " + parser_material_eps + "), 0.0, 1.0))";
	}
	else if (node.type == "VALTORGB") { // ColorRamp
		let fac: string = parser_material_parse_value_input(node.inputs[0]);
		let data0: i32 = node.buttons[0].data[0];
		let interp: string = data0 == 0 ? "LINEAR" : "CONSTANT";
		let elems: f32[] = node.buttons[0].default_value;
		let len: i32 = elems.length / 5;
		if (len == 1) {
			return parser_material_vec3(elems);
		}
		// Write cols array
		let cols_var: string = parser_material_node_name(node) + "_cols";
		node_shader_write(parser_material_curshader, "vec3 " + cols_var + "[" + len + "];"); // TODO: Make const
		for (let i: i32 = 0; i < len; ++i) {
			let tmp: f32[] = [];
			array_push(tmp, elems[i * 5]);
			array_push(tmp, elems[i * 5 + 1]);
			array_push(tmp, elems[i * 5 + 2]);
			node_shader_write(parser_material_curshader, cols_var + "[" + i + "] = " + parser_material_vec3(tmp) + ";");
		}
		// Get index
		let fac_var: string = parser_material_node_name(node) + "_fac";
		node_shader_write(parser_material_curshader, "float " + fac_var + " = " + fac + ";");
		let index: string = "0";
		for (let i: i32 = 1; i < len; ++i) {
			let e: f32 = elems[i * 5 + 4];
			index += " + (" + fac_var + " > " + e + " ? 1 : 0)";
		}
		// Write index
		let index_var: string = parser_material_node_name(node) + "_i";
		node_shader_write(parser_material_curshader, "int " + index_var + " = " + index + ";");
		if (interp == "CONSTANT") {
			return cols_var + "[" + index_var + "]";
		}
		else { // Linear
			// Write facs array
			let facs_var: string = parser_material_node_name(node) + "_facs";
			node_shader_write(parser_material_curshader, "float " + facs_var + "[" + len + "];"); // TODO: Make const
			for (let i: i32 = 0; i < len; ++i) {
				let e: f32 = elems[i * 5 + 4];
				node_shader_write(parser_material_curshader, facs_var + "[" + i + "] = " + e + ";");
			}
			// Mix color
			// float f = (pos - start) * (1.0 / (finish - start))
			// TODO: index_var + 1 out of bounds
			return "mix(" + cols_var + "[" + index_var + "], " + cols_var + "[" + index_var + " + 1], (" +
				fac_var + " - " + facs_var + "[" + index_var + "]) * (1.0 / (" + facs_var + "[" + index_var + " + 1] - " +
				facs_var + "[" + index_var + "]) ))";
		}
	}
	else if (node.type == "CURVE_VEC") {
		let fac: string = parser_material_parse_value_input(node.inputs[0]);
		let vec: string = parser_material_parse_vector_input(node.inputs[1]);
		let curves: f32_array_t = node.buttons[0].default_value;
		let name: string = parser_material_node_name(node);
		let vc0: string = parser_material_vector_curve(name + "0", vec + ".x", curves.buffer + 32 * 0, curves[96]);
		let vc1: string = parser_material_vector_curve(name + "1", vec + ".y", curves.buffer + 32 * 1, curves[97]);
		let vc2: string = parser_material_vector_curve(name + "2", vec + ".z", curves.buffer + 32 * 2, curves[98]);
		// mapping.curves[0].points[0].handle_type // bezier curve
		return "(vec3(" + vc0 + ", " + vc1 + ", " + vc2 + ") * " + fac + ")";
	}
	// else if (node.type == "CURVE_RGB") { // RGB Curves
	// 	let fac: string = parser_material_parse_value_input(node.inputs[0]);
	// 	let vec: string = parser_material_parse_vector_input(node.inputs[1]);
	// 	let curves: f32_array_t = node.buttons[0].default_value;
	// 	let name: string = parser_material_node_name(node);
	// 	// mapping.curves[0].points[0].handle_type
	// 	let vc0: string = parser_material_vector_curve(name + "0", vec + ".x", curves[0]);
	// 	let vc1: string = parser_material_vector_curve(name + "1", vec + ".y", curves[1]);
	// 	let vc2: string = parser_material_vector_curve(name + "2", vec + ".z", curves[2]);
	// 	let vc3a: string = parser_material_vector_curve(name + "3a", vec + ".x", curves[3]);
	// 	let vc3b: string = parser_material_vector_curve(name + "3b", vec + ".y", curves[3]);
	// 	let vc3c: string = parser_material_vector_curve(name + "3c", vec + ".z", curves[3]);
	// 	return "(sqrt(vec3(" + vc0 + ", " + vc1 + ", " + vc2 + ") * vec3(" + vc3a + ", " + vc3b + ", " + vc3c + ")) * " + fac + ")";
	// }
	else if (node.type == "COMBHSV") {
		node_shader_add_function(parser_material_curshader, str_hue_sat);
		let h: string = parser_material_parse_value_input(node.inputs[0]);
		let s: string = parser_material_parse_value_input(node.inputs[1]);
		let v: string = parser_material_parse_value_input(node.inputs[2]);
		return "hsv_to_rgb(vec3(" + h + ", " + s + ", " + v + "))";
	}
	else if (node.type == "COMBRGB") {
		let r: string = parser_material_parse_value_input(node.inputs[0]);
		let g: string = parser_material_parse_value_input(node.inputs[1]);
		let b: string = parser_material_parse_value_input(node.inputs[2]);
		return "vec3(" + r + ", " + g + ", " + b + ")";
	}
	else if (node.type == "WAVELENGTH") {
		node_shader_add_function(parser_material_curshader, str_wavelength_to_rgb);
		let wl: string = parser_material_parse_value_input(node.inputs[0]);
		node_shader_add_function(parser_material_curshader, str_wavelength_to_rgb);
		return "wavelength_to_rgb((" + wl + " - 450.0) / 150.0)";
	}
	else if (node.type == "CAMERA") {
		parser_material_curshader.vvec_cam = true;
		return "vvec_cam";
	}
	else if (node.type == "LAYER") {
		let l: any = node.buttons[0].default_value;
		if (socket == node.outputs[0]) { // Base
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint" + l, "_texpaint" + l);
			return "texture(texpaint" + l + ", tex_coord).rgb";
		}
		else if (socket == node.outputs[5]) { // Normal
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint_nor" + l, "_texpaint_nor" + l);
			return "texture(texpaint_nor" + l + ", tex_coord).rgb";
		}
	}
	else if (node.type == "MATERIAL") {
		let result: string = "vec3(0.0, 0.0, 0.0)";
		let mi: i32 = node.buttons[0].default_value[0];
		if (mi >= project_materials.length) {
			return result;
		}
		let m: slot_material_t = project_materials[mi];
		let _nodes: ui_node_t[] = parser_material_nodes;
		let _links: ui_node_link_t[] = parser_material_links;
		parser_material_nodes = m.canvas.nodes;
		parser_material_links = m.canvas.links;
		array_push(parser_material_parents, node);
		let output_node: ui_node_t = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL_PBR");
		if (socket == node.outputs[0]) { // Base
			result = parser_material_parse_vector_input(output_node.inputs[0]);
		}
		else if (socket == node.outputs[5]) { // Normal
			result = parser_material_parse_vector_input(output_node.inputs[5]);
		}
		parser_material_nodes = _nodes;
		parser_material_links = _links;
		array_pop(parser_material_parents);
		return result;
	}
	else if (node.type == "PICKER") {
		if (socket == node.outputs[0]) { // Base
			node_shader_add_uniform(parser_material_curshader, "vec3 picker_base", "_picker_base");
			return "picker_base";
		}
		else if (socket == node.outputs[5]) { // Normal
			node_shader_add_uniform(parser_material_curshader, "vec3 picker_normal", "_picker_normal");
			return "picker_normal";
		}
	}
	else if (node.type == "NEW_GEOMETRY") {
		if (socket == node.outputs[0]) { // Position
			parser_material_curshader.wposition = true;
			return "wposition";
		}
		else if (socket == node.outputs[1]) { // Normal
			parser_material_curshader.n = true;
			return "n";
		}
		else if (socket == node.outputs[2]) { // Tangent
			parser_material_curshader.wtangent = true;
			return "wtangent";
		}
		else if (socket == node.outputs[3]) { // True Normal
			parser_material_curshader.n = true;
			return "n";
		}
		else if (socket == node.outputs[4]) { // Incoming
			parser_material_curshader.vvec = true;
			return "vvec";
		}
		else if (socket == node.outputs[5]) { // Parametric
			parser_material_curshader.mposition = true;
			return "mposition";
		}
	}
	else if (node.type == "OBJECT_INFO") {
		if (socket == node.outputs[0]) { // Location
			parser_material_curshader.wposition = true;
			return "wposition";
		}
		else if (socket == node.outputs[1]) { // Color
			return "vec3(0.0, 0.0, 0.0)";
		}
	}
	// else if (node.type == "PARTICLE_INFO") {
	// 	if (socket == node.outputs[3]) { // Location
	// 		return "vec3(0.0, 0.0, 0.0)";
	// 	}
	// 	else if (socket == node.outputs[5]) { // Velocity
	// 		return "vec3(0.0, 0.0, 0.0)";
	// 	}
	// 	else if (socket == node.outputs[6]) { // Angular Velocity
	// 		return "vec3(0.0, 0.0, 0.0)";
	// 	}
	// }
	else if (node.type == "TANGENT") {
		parser_material_curshader.wtangent = true;
		return "wtangent";
	}
	else if (node.type == "TEX_COORD") {
		if (socket == node.outputs[0]) { // Generated - bounds
			parser_material_curshader.bposition = true;
			return "bposition";
		}
		else if (socket == node.outputs[1]) { // Normal
			parser_material_curshader.n = true;
			return "n";
		}
		else if (socket == node.outputs[2]) {// UV
			node_shader_context_add_elem(parser_material_curshader.context, "tex", "short2norm");
			return "vec3(tex_coord.x, tex_coord.y, 0.0)";
		}
		else if (socket == node.outputs[3]) { // Object
			parser_material_curshader.mposition = true;
			return "mposition";
		}
		else if (socket == node.outputs[4]) { // Camera
			parser_material_curshader.vposition = true;
			return "vposition";
		}
		else if (socket == node.outputs[5]) { // Window
			parser_material_curshader.wvpposition = true;
			return "wvpposition.xyz";
		}
		else if (socket == node.outputs[6]) { // Reflection
			return "vec3(0.0, 0.0, 0.0)";
		}
	}
	else if (node.type == "UVMAP") {
		node_shader_context_add_elem(parser_material_curshader.context, "tex", "short2norm");
		return "vec3(tex_coord.x, tex_coord.y, 0.0)";
	}
	else if (node.type == "BUMP") {
		let strength: string = parser_material_parse_value_input(node.inputs[0]);
		// let distance: string = parse_value_input(node.inputs[1]);
		let height: string = parser_material_parse_value_input(node.inputs[2]);
		let nor: string = parser_material_parse_vector_input(node.inputs[3]);
		let sample_bump_res: string = parser_material_store_var_name(node) + "_bump";
		node_shader_write(parser_material_curshader, "float " + sample_bump_res + "_x = dFdx(float(" + height + ")) * (" + strength + ") * 16.0;");
		node_shader_write(parser_material_curshader, "float " + sample_bump_res + "_y = dFdy(float(" + height + ")) * (" + strength + ") * 16.0;");
		return "(normalize(vec3(" + sample_bump_res + "_x, " + sample_bump_res + "_y, 1.0) + " + nor + ") * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5))";
	}
	else if (node.type == "MAPPING") {
		let out: string = parser_material_parse_vector_input(node.inputs[0]);
		let node_translation: string = parser_material_parse_vector_input(node.inputs[1]);
		let node_rotation: string = parser_material_parse_vector_input(node.inputs[2]);
		let node_scale: string = parser_material_parse_vector_input(node.inputs[3]);
		if (node_scale != "vec3(1, 1, 1)") {
			out = "(" + out + " * " + node_scale + ")";
		}
		if (node_rotation != "vec3(0, 0, 0)") {
			// ZYX rotation, Z axis for now..
			let a: string = node_rotation + ".z * (3.1415926535 / 180)";
			// x * cos(theta) - y * sin(theta)
			// x * sin(theta) + y * cos(theta)
			out = "vec3(" + out + ".x * cos(" + a + ") - " + out + ".y * sin(" + a + "), " + out + ".x * sin(" + a + ") + " + out + ".y * cos(" + a + "), 0.0)";
		}
		// if node.rotation[1] != 0.0:
		//     a = node.rotation[1]
		//     out = "vec3({0}.x * {1} - {0}.z * {2}, {0}.x * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
		// if node.rotation[0] != 0.0:
		//     a = node.rotation[0]
		//     out = "vec3({0}.y * {1} - {0}.z * {2}, {0}.y * {2} + {0}.z * {1}, 0.0)".format(out, math_cos(a), math_sin(a))
		if (node_translation != "vec3(0, 0, 0)") {
			out = "(" + out + " + " + node_translation + ")";
		}
		// if node.use_min:
			// out = "max({0}, vec3({1}, {2}, {3}))".format(out, node.min[0], node.min[1])
		// if node.use_max:
				// out = "min({0}, vec3({1}, {2}, {3}))".format(out, node.max[0], node.max[1])
		return out;
	}
	else if (node.type == "NORMAL") {
		if (socket == node.outputs[0]) {
			return parser_material_vec3(node.outputs[0].default_value);
		}
		else if (socket == node.outputs[1]) {
			let nor: string = parser_material_parse_vector_input(node.inputs[0]);
			let norout: string = parser_material_vec3(node.outputs[0].default_value);
			return parser_material_to_vec3("dot(" + norout + ", " + nor + ")");
		}
	}
	else if (node.type == "NORMAL_MAP") {
		let strength: string = parser_material_parse_value_input(node.inputs[0]);
		let norm: string = parser_material_parse_vector_input(node.inputs[1]);

		let store: string = parser_material_store_var_name(node);
		node_shader_write(parser_material_curshader, "vec3 " + store + "_texn = " + norm + " * 2.0 - 1.0;");
		node_shader_write(parser_material_curshader, "" + store + "_texn.xy = " + strength + " * " + store + "_texn.xy;");
		node_shader_write(parser_material_curshader, "" + store + "_texn = normalize(" + store + "_texn);");

		return "(0.5 * " + store + "_texn + 0.5)";
	}
	else if (node.type == "MIX_NORMAL_MAP") {
		let nm1: string = parser_material_parse_vector_input(node.inputs[0]);
		let nm2: string = parser_material_parse_vector_input(node.inputs[1]);
		let but: ui_node_button_t = node.buttons[0];
		let blend: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0])); // blend_type
		blend = string_replace_all(blend, " ", "_");
		let store: string = parser_material_store_var_name(node);

		// The blending algorithms are based on the paper "Blending in Detail" by Colin Barré-Brisebois and Stephen Hill 2012
		// https://blog.selfshadow.com/publications/blending-in-detail/
		if (blend == "PARTIAL_DERIVATIVE") { //partial derivate blending
			node_shader_write(parser_material_curshader, "vec3 " + store + "_n1 = " + nm1 + " * 2.0 - 1.0;");
			node_shader_write(parser_material_curshader, "vec3 " + store + "_n2 = " + nm2 + " * 2.0 - 1.0;");
			return "0.5 * normalize(vec3(" + store + "_n1.xy * " + store + "_n2.z + " + store + "_n2.xy * " + store + "_n1.z, " + store + "_n1.z * " + store + "_n2.z)) + 0.5;";
		}
		else if (blend == "WHITEOUT") { //whiteout blending
			node_shader_write(parser_material_curshader, "vec3 " + store + "_n1 = " + nm1 + " * 2.0 - 1.0;");
			node_shader_write(parser_material_curshader, "vec3 " + store + "_n2 = " + nm2 + " * 2.0 - 1.0;");
			return "0.5 * normalize(vec3(" + store + "_n1.xy + " + store + "_n2.xy, " + store + "_n1.z * " + store + "_n2.z)) + 0.5;";
		}
		else if (blend == "REORIENTED") { //reoriented normal mapping
			node_shader_write(parser_material_curshader, "vec3 " + store + "_n1 = " + nm1 + " * 2.0 - vec3(1.0, 1.0, 0.0);");
			node_shader_write(parser_material_curshader, "vec3 " + store + "_n2 = " + nm2 + " * vec3(-2.0, -2.0, 2.0) - vec3(-1.0, -1.0, 1.0);");
			return "0.5 * normalize(" + store + "_n1 * dot(" + store + "_n1, " + store + "_n2) - " + store + "_n2 * " + store + "_n1.z) + 0.5";
		}
	}
	else if (node.type == "VECT_TRANSFORM") {
	// 	type = node.vector_type
	// 	conv_from = node.convert_from
	// 	conv_to = node.convert_to
	// 	// Pass throuh
	// 	return parse_vector_input(node.inputs[0])
	}
	else if (node.type == "COMBXYZ") {
		let x: string = parser_material_parse_value_input(node.inputs[0]);
		let y: string = parser_material_parse_value_input(node.inputs[1]);
		let z: string = parser_material_parse_value_input(node.inputs[2]);
		return "vec3(" + x + ", " + y + ", " + z + ")";
	}
	else if (node.type == "VECT_MATH") {
		let vec1: string = parser_material_parse_vector_input(node.inputs[0]);
		let vec2: string = parser_material_parse_vector_input(node.inputs[1]);
		let but: ui_node_button_t = node.buttons[0]; //operation;
		let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		op = string_replace_all(op, " ", "_");
		if (op == "ADD") {
			return "(" + vec1 + " + " + vec2 + ")";
		}
		else if (op == "SUBTRACT") {
			return "(" + vec1 + " - " + vec2 + ")";
		}
		else if (op == "AVERAGE") {
			return "((" + vec1 + " + " + vec2 + ") / 2.0)";
		}
		else if (op == "DOT_PRODUCT") {
			return parser_material_to_vec3("dot(" + vec1 + ", " + vec2 + ")");
		}
		else if (op == "LENGTH") {
			return parser_material_to_vec3("length(" + vec1 + ")");
		}
		else if (op == "DISTANCE") {
			return parser_material_to_vec3("distance(" + vec1 + ", " + vec2 + ")");
		}
		else if (op == "CROSS_PRODUCT") {
			return "cross(" + vec1 + ", " + vec2 + ")";
		}
		else if (op == "NORMALIZE") {
			return "normalize(" + vec1 + ")";
		}
		else if (op == "MULTIPLY") {
			return "(" + vec1 + " * " + vec2 + ")";
		}
		else if (op == "DIVIDE") {
			return "vec3(" + vec1 + ".x / (" + vec2 + ".x == 0 ? 0.000001 : " + vec2 + ".x), " + vec1 + ".y / (" + vec2 + ".y == 0 ? 0.000001 : " + vec2 + ".y), " + vec1 + ".z / (" + vec2 + ".z == 0 ? 0.000001 : " + vec2 + ".z))";
		}
		else if (op == "PROJECT") {
			return "(dot(" + vec1 + ", " + vec2 + ") / dot(" + vec2 + ", " + vec2 + ") * " + vec2 + ")";
		}
		else if (op == "REFLECT") {
			return "reflect(" + vec1 + ", normalize(" + vec2 + "))";
		}
		else if (op == "SCALE") {
			return "(" + vec2 + ".x * " + vec1 + ")";
		}
		else if (op == "ABSOLUTE") {
			return "abs(" + vec1 + ")";
		}
		else if (op == "MINIMUM") {
			return "min(" + vec1 + ", " + vec2 + ")";
		}
		else if (op == "MAXIMUM") {
			return "max(" + vec1 + ", " + vec2 + ")";
		}
		else if (op == "FLOOR") {
			return "floor(" + vec1 + ")";
		}
		else if (op == "CEIL") {
			return "ceil(" + vec1 + ")";
		}
		else if (op == "FRACTION") {
			return "fract(" + vec1 + ")";
		}
		else if (op == "MODULO") {
			return "mod(" + vec1 + ", " + vec2 + ")";
		}
		else if(op == "SNAP") {
			return "(floor(" + vec1 + " / " + vec2 + ") * " + vec2 + ")";
		}
		else if (op == "SINE") {
			return "sin(" + vec1 + ")";
		}
		else if (op == "COSINE") {
			return "cos(" + vec1 + ")";
		}
		else if (op == "TANGENT") {
			return "tan(" + vec1 + ")";
		}
	}
	else if (node.type == "Displacement") {
		let height: string = parser_material_parse_value_input(node.inputs[0]);
		return parser_material_to_vec3(height);
	}
	else if (map_get(parser_material_custom_nodes, node.type) != null) {
		let cb: any = map_get(parser_material_custom_nodes, node.type); // JSValue -> (n: ui_node_t, s: string)=>string
		return js_call_ptr_str(cb, node, socket.name);
	}
	return "vec3(0.0, 0.0, 0.0)";
}

function parse_normal_map_color_input(inp: ui_node_socket_t) {
	parser_material_frag.write_normal++;
	parser_material_out_normaltan = parser_material_parse_vector_input(inp);
	if (!parser_material_arm_export_tangents) {
		node_shader_write(parser_material_frag, "vec3 texn = (" + parser_material_out_normaltan + ") * 2.0 - 1.0;");
		node_shader_write(parser_material_frag, "texn.y = -texn.y;");
		if (!parser_material_cotangent_frame_written) {
			parser_material_cotangent_frame_written = true;
			node_shader_add_function(parser_material_frag, str_cotangent_frame);
		}
		parser_material_frag.n = true;
		node_shader_write(parser_material_frag, "mat3 TBN = cotangent_frame(n, vvec, tex_coord);");

		node_shader_write(parser_material_frag, "n = mul(normalize(texn), TBN);");
	}
	parser_material_frag.write_normal--;
}

function parser_material_parse_value_input(inp: ui_node_socket_t, vector_as_grayscale: bool = false): string {
	let l: ui_node_link_t = parser_material_get_input_link(inp);
	let from_node: ui_node_t = l != null ? parser_material_get_node(l.from_id) : null;
	if (from_node != null) {
		if (from_node.type == "REROUTE") {
			return parser_material_parse_value_input(from_node.inputs[0]);
		}

		let res_var: string = parser_material_write_result(l);
		let st: string = from_node.outputs[l.from_socket].type;
		if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
			if (vector_as_grayscale) {
				return "dot(" + res_var + ".rbg, vec3(0.299, 0.587, 0.114))";
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
	if (node.type == "GROUP") {
		return parser_material_parse_group(node, socket);
	}
	else if (node.type == "GROUP_INPUT") {
		return parser_material_parse_group_input(node, socket);
	}
	else if (node.type == "ATTRIBUTE") {
		node_shader_add_uniform(parser_material_curshader, "float time", "_time");
		return "time";
	}
	else if (node.type == "VERTEX_COLOR") {
		return "1.0";
	}
	else if (node.type == "WIREFRAME") {
		node_shader_add_uniform(parser_material_curshader, "sampler2D texuvmap", "_texuvmap");
		// let use_pixel_size: bool = node.buttons[0].default_value == "true";
		// let pixel_size: f32 = parse_value_input(node.inputs[0]);
		return "textureLod(texuvmap, tex_coord, 0.0).r";
	}
	else if (node.type == "CAMERA") {
		if (socket == node.outputs[1]) { // View Z Depth
			node_shader_add_uniform(parser_material_curshader, "vec2 camera_proj", "_camera_plane_proj");
			parser_material_curshader.wvpposition = true;
			return "(camera_proj.y / ((wvpposition.z / wvpposition.w) - camera_proj.x))";
		}
		else { // View Distance
			node_shader_add_uniform(parser_material_curshader, "vec3 eye", "_camera_pos");
			parser_material_curshader.wposition = true;
			return "distance(eye, wposition)";
		}
	}
	else if (node.type == "LAYER") {
		let l: any = node.buttons[0].default_value;
		if (socket == node.outputs[1]) { // Opac
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint" + l, "_texpaint" + l);
			return "texture(texpaint" + l + ", tex_coord).a";
		}
		else if (socket == node.outputs[2]) { // Occ
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
			return "texture(texpaint_pack" + l + ", tex_coord).r";
		}
		else if (socket == node.outputs[3]) { // Rough
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
			return "texture(texpaint_pack" + l + ", tex_coord).g";
		}
		else if (socket == node.outputs[4]) { // Metal
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
			return "texture(texpaint_pack" + l + ", tex_coord).b";
		}
		else if (socket == node.outputs[7]) { // Height
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
			return "texture(texpaint_pack" + l + ", tex_coord).a";
		}
	}
	else if (node.type == "LAYER_MASK") {
		if (socket == node.outputs[0]) {
			let l: any = node.buttons[0].default_value;
			node_shader_add_uniform(parser_material_curshader, "sampler2D texpaint" + l, "_texpaint" + l);
			return "texture(texpaint" + l + ", tex_coord).r";
		}
	}
	else if (node.type == "MATERIAL") {
		let result: string = "0.0";
		let mi: i32 = node.buttons[0].default_value[0];
		if (mi >= project_materials.length) return result;
		let m: slot_material_t = project_materials[mi];
		let _nodes: ui_node_t[] = parser_material_nodes;
		let _links: ui_node_link_t[] = parser_material_links;
		parser_material_nodes = m.canvas.nodes;
		parser_material_links = m.canvas.links;
		array_push(parser_material_parents, node);
		let output_node: ui_node_t = parser_material_node_by_type(parser_material_nodes, "OUTPUT_MATERIAL_PBR");
		if (socket == node.outputs[1]) { // Opac
			result = parser_material_parse_value_input(output_node.inputs[1]);
		}
		else if (socket == node.outputs[2]) { // Occ
			result = parser_material_parse_value_input(output_node.inputs[2]);
		}
		else if (socket == node.outputs[3]) { // Rough
			result = parser_material_parse_value_input(output_node.inputs[3]);
		}
		else if (socket == node.outputs[4]) { // Metal
			result = parser_material_parse_value_input(output_node.inputs[4]);
		}
		else if (socket == node.outputs[7]) { // Height
			result = parser_material_parse_value_input(output_node.inputs[7]);
		}
		parser_material_nodes = _nodes;
		parser_material_links = _links;
		array_pop(parser_material_parents);
		return result;
	}
	else if (node.type == "PICKER") {
		if (socket == node.outputs[1]) {
			node_shader_add_uniform(parser_material_curshader, "float picker_opacity", "_picker_opacity");
			return "picker_opacity";
		}
		else if (socket == node.outputs[2]) {
			node_shader_add_uniform(parser_material_curshader, "float picker_occlusion", "_picker_occlusion");
			return "picker_occlusion";
		}
		else if (socket == node.outputs[3]) {
			node_shader_add_uniform(parser_material_curshader, "float picker_roughness", "_picker_roughness");
			return "picker_roughness";
		}
		else if (socket == node.outputs[4]) {
			node_shader_add_uniform(parser_material_curshader, "float picker_metallic", "_picker_metallic");
			return "picker_metallic";
		}
		else if (socket == node.outputs[7]) {
			node_shader_add_uniform(parser_material_curshader, "float picker_height", "_picker_height");
			return "picker_height";
		}
	}
	else if (node.type == "FRESNEL") {
		let ior: string = parser_material_parse_value_input(node.inputs[0]);
		parser_material_curshader.dotnv = true;
		return "pow(1.0 - dotnv, 7.25 / " + ior + ")";
	}
	else if (node.type == "NEW_GEOMETRY") {
		if (socket == node.outputs[6]) { // Backfacing
			return "0.0"; // SV_IsFrontFace
			// return "(1.0 - float(gl_FrontFacing))";
		}
		else if (socket == node.outputs[7]) { // Pointiness
			let strength: f32 = 1.0;
			let radius: f32 = 1.0;
			let offset: f32 = 0.0;
			let store: string = parser_material_store_var_name(node);
			parser_material_curshader.n = true;
			node_shader_write(parser_material_curshader, "vec3 " + store + "_dx = dFdx(n);");
			node_shader_write(parser_material_curshader, "vec3 " + store + "_dy = dFdy(n);");
			node_shader_write(parser_material_curshader, "float " + store + "_curvature = max(dot(" + store + "_dx, " + store + "_dx), dot(" + store + "_dy, " + store + "_dy));");
			node_shader_write(parser_material_curshader, store + "_curvature = clamp(pow(" + store + "_curvature, (1.0 / " + radius + ") * 0.25) * " + strength + " * 2.0 + " + offset + " / 10.0, 0.0, 1.0);");
			return store + "_curvature";
		}
		else if (socket == node.outputs[8]) { // Random Per Island
			return "0.0";
		}
	}
	else if (node.type == "HAIR_INFO") {
		return "0.5";
	}
	else if (node.type == "LAYER_WEIGHT") {
		let blend: string = parser_material_parse_value_input(node.inputs[0]);
		if (socket == node.outputs[0]) { // Fresnel
			parser_material_curshader.dotnv = true;
			return "clamp(pow(1.0 - dotnv, (1.0 - " + blend + ") * 10.0), 0.0, 1.0)";
		}
		else if (socket == node.outputs[1]) { // Facing
			parser_material_curshader.dotnv = true;
			return "((1.0 - dotnv) * " + blend + ")";
		}
	}
	else if (node.type == "OBJECT_INFO") {
		if (socket == node.outputs[1]) { // Object Index
			node_shader_add_uniform(parser_material_curshader, "float object_info_index", "_object_info_index");
			return "object_info_index";
		}
		else if (socket == node.outputs[2]) { // Material Index
			node_shader_add_uniform(parser_material_curshader, "float object_info_material_index", "_object_info_material_index");
			return "object_info_material_index";
		}
		else if (socket == node.outputs[3]) { // Random
			node_shader_add_uniform(parser_material_curshader, "float object_info_random", "_object_info_random");
			return "object_info_random";
		}
	}
	else if (node.type == "VALUE") {
		return parser_material_vec1(node.outputs[0].default_value[0]);
	}
	else if (node.type == "TEX_BRICK") {
		node_shader_add_function(parser_material_curshader, str_tex_brick);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[4]);
		let res: string = "tex_brick_f(" + co + " * " + scale + ")";
		return res;
	}
	else if (node.type == "TEX_CHECKER") {
		node_shader_add_function(parser_material_curshader, str_tex_checker);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[3]);
		let res: string = "tex_checker_f(" + co + ", " + scale + ")";
		return res;
	}
	else if (node.type == "TEX_GRADIENT") {
		let co: string = parser_material_get_coord(node);
		let but: ui_node_button_t = node.buttons[0]; //gradient_type;
		let grad: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		grad = string_replace_all(grad, " ", "_");
		let f: string = parser_material_get_gradient(grad, co);
		let res: string = "(clamp(" + f + ", 0.0, 1.0))";
		return res;
	}
	else if (node.type == "TEX_IMAGE") {
		// Already fetched
		if (array_index_of(parser_material_parsed, parser_material_res_var_name(node, node.outputs[0])) >= 0) { // TODO: node.outputs[1]
			let varname: string = parser_material_store_var_name(node);
			return varname + ".a";
		}
		let tex_name: string = parser_material_node_name(node);
		let tex: bind_tex_t = parser_material_make_texture(node, tex_name);
		if (tex != null) {
			let color_space: i32 = node.buttons[1].default_value[0];
			let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space);
			return texstore + ".a";
		}
	}
	else if (node.type == "TEX_TEXT") {
		let tex_name: string = parser_material_node_name(node);
		let text_buffer: buffer_t = node.buttons[0].default_value;
		let text: string = sys_buffer_to_string(text_buffer);
		let file: string = "tex_text_" + text;
		_parser_material_cache_tex_text_node(file, text);
		let tex: bind_tex_t = parser_material_make_bind_tex(tex_name, file);
		let texstore: string = parser_material_texture_store(node, tex, tex_name, color_space_t.AUTO);
		return texstore + ".r";
	}
	else if (node.type == "TEX_MAGIC") {
		node_shader_add_function(parser_material_curshader, str_tex_magic);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let res: string = "tex_magic_f(" + co + " * " + scale + " * 4.0)";
		return res;
	}
	else if (node.type == "TEX_MUSGRAVE") {
		node_shader_add_function(parser_material_curshader, str_tex_musgrave);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let res: string = "tex_musgrave_f(" + co + " * " + scale + " * 0.5)";
		return res;
	}
	else if (node.type == "TEX_NOISE") {
		node_shader_add_function(parser_material_curshader, str_tex_noise);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let res: string = "tex_noise(" + co + " * " + scale + ")";
		return res;
	}
	else if (node.type == "TEX_VORONOI") {
		node_shader_add_function(parser_material_curshader, str_tex_voronoi);
		node_shader_add_uniform(parser_material_curshader, "sampler2D snoise256", "$noise256.k");
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let but: ui_node_button_t = node.buttons[0]; // coloring
		let coloring: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		coloring = string_replace_all(coloring, " ", "_");
		let res: string = "";
		if (coloring == "INTENSITY") {
			res = "tex_voronoi(" + co + " * " + scale + ", texturePass(snoise256)).a";
		}
		else { // Cells
			res = "tex_voronoi(" + co + " * " + scale + ", texturePass(snoise256)).r";
		}
		return res;
	}
	else if (node.type == "TEX_WAVE") {
		node_shader_add_function(parser_material_curshader, str_tex_wave);
		let co: string = parser_material_get_coord(node);
		let scale: string = parser_material_parse_value_input(node.inputs[1]);
		let res: string = "tex_wave_f(" + co + " * " + scale + ")";
		return res;
	}
	else if (node.type == "BAKE_CURVATURE") {
		if (parser_material_bake_passthrough) {
			parser_material_bake_passthrough_strength = parser_material_parse_value_input(node.inputs[0]);
			parser_material_bake_passthrough_radius = parser_material_parse_value_input(node.inputs[1]);
			parser_material_bake_passthrough_offset = parser_material_parse_value_input(node.inputs[2]);
			return "0.0";
		}
		let tex_name: string = "texbake_" + parser_material_node_name(node);
		node_shader_add_uniform(parser_material_curshader, "sampler2D " + tex_name, "_" + tex_name);
		let store: string = parser_material_store_var_name(node);
		node_shader_write(parser_material_curshader, "float " + store + "_res = texture(" + tex_name + ", tex_coord).r;");
		return store + "_res";
	}
	else if (node.type == "NORMAL") {
		let nor: string = parser_material_parse_vector_input(node.inputs[0]);
		let norout: string = parser_material_vec3(node.outputs[0].default_value);
		return "dot(" + norout + ", " + nor + ")";
	}
	else if (node.type == "COLMASK") {
		let input_color: string = parser_material_parse_vector_input(node.inputs[0]);
		let mask_color: string = parser_material_parse_vector_input(node.inputs[1]);
		let radius: string = parser_material_parse_value_input(node.inputs[2]);
		let fuzziness: string = parser_material_parse_value_input(node.inputs[3]);
		return "clamp(1.0 - (distance(" + input_color + ", " + mask_color + ") - " + radius + ") / max(" + fuzziness + ", " + parser_material_eps + "), 0.0, 1.0)";
	}
	else if (node.type == "MATH") {
		let val1: string = parser_material_parse_value_input(node.inputs[0]);
		let val2: string = parser_material_parse_value_input(node.inputs[1]);
		let but: ui_node_button_t = node.buttons[0]; // operation
		let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		op = string_replace_all(op, " ", "_");
		let use_clamp: bool = node.buttons[1].default_value[0] > 0;
		let out_val: string = "";
		if (op == "ADD") {
			out_val = "(" + val1 + " + " + val2 + ")";
		}
		else if (op == "SUBTRACT") {
			out_val = "(" + val1 + " - " + val2 + ")";
		}
		else if (op == "MULTIPLY") {
			out_val = "(" + val1 + " * " + val2 + ")";
		}
		else if (op == "DIVIDE") {
			val2 = "(" + val2 + " == 0.0 ? " + parser_material_eps + " : " + val2 + ")";
			out_val = "(" + val1 + " / " + val2 + ")";
		}
		else if (op == "POWER") {
			out_val = "pow(" + val1 + ", " + val2 + ")";
		}
		else if (op == "LOGARITHM") {
			out_val = "log(" + val1 + ")";
		}
		else if (op == "SQUARE_ROOT") {
			out_val = "sqrt(" + val1 + ")";
		}
		else if(op == "INVERSE_SQUARE_ROOT") {
			out_val = "inversesqrt(" + val1 + ")";
		}
		else if (op == "EXPONENT") {
			out_val = "exp(" + val1 + ")";
		}
		else if (op == "ABSOLUTE") {
			out_val = "abs(" + val1 + ")";
		}
		else if (op == "MINIMUM") {
			out_val = "min(" + val1 + ", " + val2 + ")";
		}
		else if (op == "MAXIMUM") {
			out_val = "max(" + val1 + ", " + val2 + ")";
		}
		else if (op == "LESS_THAN") {
			out_val = "float(" + val1 + " < " + val2 + ")";
		}
		else if (op == "GREATER_THAN") {
			out_val = "float(" + val1 + " > " + val2 + ")";
		}
		else if (op == "SIGN") {
			out_val = "sign(" + val1 + ")";
		}
		else if (op == "ROUND") {
			out_val = "floor(" + val1 + " + 0.5)";
		}
		else if (op == "FLOOR") {
			out_val = "floor(" + val1 + ")";
		}
		else if (op == "CEIL") {
			out_val = "ceil(" + val1 + ")";
		}
		else if(op == "SNAP") {
			out_val = "(floor(" + val1 + " / " + val2 + ") * " + val2 + ")";
		}
		else if (op == "TRUNCATE") {
			out_val = "trunc(" + val1 + ")";
		}
		else if (op == "FRACTION") {
			out_val = "fract(" + val1 + ")";
		}
		else if (op == "MODULO") {
			out_val = "mod(" + val1 + ", " + val2 + ")";
		}
		else if (op == "PING-PONG") {
			out_val = "((" + val2 + " != 0.0) ? abs(fract((" + val1 + " - " + val2 + ") / (" + val2 + " * 2.0)) * " + val2 + " * 2.0 - " + val2 + ") : 0.0)";
		}
		else if (op == "SINE") {
			out_val = "sin(" + val1 + ")";
		}
		else if (op == "COSINE") {
			out_val = "cos(" + val1 + ")";
		}
		else if (op == "TANGENT") {
			out_val = "tan(" + val1 + ")";
		}
		else if (op == "ARCSINE") {
			out_val = "asin(" + val1 + ")";
		}
		else if (op == "ARCCOSINE") {
			out_val = "acos(" + val1 + ")";
		}
		else if (op == "ARCTANGENT") {
			out_val = "atan(" + val1 + ")";
		}
		else if (op == "ARCTAN2") {
			out_val = "atan2(" + val1 + ", " + val2 + ")";
		}
		else if (op == "HYPERBOLIC_SINE") {
			out_val = "sinh(" + val1 + ")";
		}
		else if (op == "HYPERBOLIC_COSINE") {
			out_val = "cosh(" + val1 + ")";
		}
		else if (op == "HYPERBOLIC_TANGENT") {
			out_val = "tanh(" + val1 + ")";
		}
		else if (op == "TO_RADIANS") {
			out_val = "radians(" + val1 + ")";
		}
		else if (op == "TO_DEGREES") {
			out_val = "degrees(" + val1 + ")";
		}
		if (use_clamp) {
			return "clamp(" + out_val + ", 0.0, 1.0)";
		}
		else {
			return out_val;
		}
	}
	else if (node.type == "SCRIPT_CPU") {
		if (parser_material_script_links == null) {
			parser_material_script_links = map_create();
		}
		let script: buffer_t = node.buttons[0].default_value;
		let str: string = sys_buffer_to_string(script);
		let link: string = parser_material_node_name(node);
		map_set(parser_material_script_links, link, str);
		node_shader_add_uniform(parser_material_curshader, "float " + link, "_" + link);
		return link;
	}
	else if (node.type == "SHADER_GPU") {
		let shader: buffer_t = node.buttons[0].default_value;
		let str: string = sys_buffer_to_string(shader);
		return str == "" ? "0.0" : str;
	}
	else if (node.type == "RGBTOBW") {
		let col: string = parser_material_parse_vector_input(node.inputs[0]);
		return "(((" + col + ".r * 0.3 + " + col + ".g * 0.59 + " + col + ".b * 0.11) / 3.0) * 2.5)";
	}
	else if (node.type == "SEPHSV") {
		node_shader_add_function(parser_material_curshader, str_hue_sat);
		let col: string = parser_material_parse_vector_input(node.inputs[0]);
		if (socket == node.outputs[0]) {
			return "rgb_to_hsv(" + col + ").r";
		}
		else if (socket == node.outputs[1]) {
			return "rgb_to_hsv(" + col + ").g";
		}
		else if (socket == node.outputs[2]) {
			return "rgb_to_hsv(" + col + ").b";
		}
	}
	else if (node.type == "SEPRGB") {
		let col: string = parser_material_parse_vector_input(node.inputs[0]);
		if (socket == node.outputs[0]) {
			return col + ".r";
		}
		else if (socket == node.outputs[1]) {
			return col + ".g";
		}
		else if (socket == node.outputs[2]) {
			return col + ".b";
		}
	}
	else if (node.type == "SEPXYZ") {
		let vec: string = parser_material_parse_vector_input(node.inputs[0]);
		if (socket == node.outputs[0]) {
			return vec + ".x";
		}
		else if (socket == node.outputs[1]) {
			return vec + ".y";
		}
		else if (socket == node.outputs[2]) {
			return vec + ".z";
		}
	}
	else if (node.type == "VECT_MATH") {
		let vec1: string = parser_material_parse_vector_input(node.inputs[0]);
		let vec2: string = parser_material_parse_vector_input(node.inputs[1]);
		let but: ui_node_button_t = node.buttons[0]; //operation;
		let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		op = string_replace_all(op, " ", "_");
		if (op == "DOT_PRODUCT") {
			return "dot(" + vec1 + ", " + vec2 + ")";
		}
		else if (op == "LENGTH") {
			return "length(" + vec1 + ")";
		}
		else if (op == "DISTANCE") {
			return "distance(" + vec1 + ", " + vec2 + ")";
		}
		else {
			return "0.0";
		}
	}
	else if (node.type == "CLAMP") {
		let val: string = parser_material_parse_value_input(node.inputs[0]);
		let min: string = parser_material_parse_value_input(node.inputs[1]);
		let max: string = parser_material_parse_value_input(node.inputs[2]);
		let but: ui_node_button_t = node.buttons[0]; //operation;
		let op: string = to_upper_case(u8_array_string_at(but.data, but.default_value[0]));
		op = string_replace_all(op, " ", "_");

		if (op == "MIN_MAX") {
			return "(clamp(" + val + ", " + min + ", " + max + "))";
		}
		else if (op == "RANGE") {
			return "(clamp(" + val + ", min(" + min + ", " + max + "), max(" + min + ", " + max + ")))";
		}
	}
	else if (node.type == "MAPRANGE") {
		let val: string = parser_material_parse_value_input(node.inputs[0]);
		let fmin: string = parser_material_parse_value_input(node.inputs[1]);
		let fmax: string = parser_material_parse_value_input(node.inputs[2]);
		let tmin: string = parser_material_parse_value_input(node.inputs[3]);
		let tmax: string = parser_material_parse_value_input(node.inputs[4]);

		let use_clamp: bool = node.buttons[0].default_value[0] > 0;

		let a: string = "((" + tmin + " - " + tmax + ") / (" + fmin + " - " + fmax + "))";
		let out_val: string = "(" + a + " * " + val + " + " + tmin + " - " + a + " * " + fmin + ")";
		if (use_clamp) {
			return "(clamp(" + out_val + ", " + tmin + ", " + tmax + "))";
		}
		else {
			return out_val;
		}
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
		parser_material_curshader.bposition = true;
		return "bposition";
	}
}

function parser_material_get_gradient(grad: string, co: string): string {
	if (grad == "LINEAR") {
		return co + ".x";
	}
	else if (grad == "QUADRATIC") {
		return "0.0";
	}
	else if (grad == "EASING") {
		return "0.0";
	}
	else if (grad == "DIAGONAL") {
		return "(" + co + ".x + " + co + ".y) * 0.5";
	}
	else if (grad == "RADIAL") {
		return "atan2(" + co + ".x, " + co + ".y) / (3.141592 * 2.0) + 0.5";
	}
	else if (grad == "QUADRATIC_SPHERE") {
		return "0.0";
	}
	else { // "SPHERICAL"
		return "max(1.0 - sqrt(" + co + ".x * " + co + ".x + " + co + ".y * " + co + ".y + " + co + ".z * " + co + ".z), 0.0)";
	}
}

function parser_material_vector_curve(name: string, fac: string, points: f32_ptr, num: i32): string {
	// Write Ys array
	let ys_var: string = name + "_ys";
	node_shader_write(parser_material_curshader, "float " + ys_var + "[" + num + "];"); // TODO: Make const
	for (let i: i32 = 0; i < num; ++i) {
		let p: f32 = ARRAY_ACCESS(points, i * 2 + 1);
		node_shader_write(parser_material_curshader, ys_var + "[" + i + "] = " + p + ";");
	}
	// Get index
	let fac_var: string = name + "_fac";
	node_shader_write(parser_material_curshader, "float " + fac_var + " = " + fac + ";");
	let index: string = "0";
	for (let i: i32 = 1; i < num; ++i) {
		let p: f32 = ARRAY_ACCESS(points, i * 2 + 0);
		index += " + (" + fac_var + " > " + p + " ? 1 : 0)";
	}
	// Write index
	let index_var: string = name + "_i";
	node_shader_write(parser_material_curshader, "int " + index_var + " = " + index + ";");
	// Linear
	// Write Xs array
	let facs_var: string = name + "_xs";
	node_shader_write(parser_material_curshader, "float " + facs_var + "[" + num + "];"); // TODO: Make const
	for (let i: i32 = 0; i < num; ++i) {
		let p: f32 = ARRAY_ACCESS(points, i * 2 + 0);
		node_shader_write(parser_material_curshader, "" + facs_var + "[" + i + "] = " + p + ";");
	}
	// Map vector
	return "mix(" +
		ys_var + "[" + index_var + "], " + ys_var + "[" + index_var + " + 1], (" + fac_var + " - " +
		facs_var + "[" + index_var + "]) * (1.0 / (" + facs_var + "[" + index_var + " + 1] - " + facs_var + "[" + index_var + "])))";
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
			node_shader_write(parser_material_curshader, "vec3 " + res_var + " = " + res + ";");
		}
		else if (st == "VALUE") {
			let res: string = parser_material_parse_value(from_node, from_socket);
			if (res == null) {
				return null;
			}
			map_set(parser_material_parsed_map, res_var, res);
			node_shader_write(parser_material_curshader, "float " + res_var + " = " + res + ";");
		}
	}
	return res_var;
}

function parser_material_store_var_name(node: ui_node_t): string {
	return parser_material_node_name(node) + "_store";
}

function parser_material_texture_store(node: ui_node_t, tex: bind_tex_t, tex_name: string, color_space: i32): string {
	array_push(parser_material_matcon.bind_textures, tex);
	node_shader_context_add_elem(parser_material_curshader.context, "tex", "short2norm");
	node_shader_add_uniform(parser_material_curshader, "sampler2D " + tex_name);
	let uv_name: string = "";
	if (parser_material_get_input_link(node.inputs[0]) != null) {
		uv_name = parser_material_parse_vector_input(node.inputs[0]);
	}
	else {
		uv_name = parser_material_tex_coord;
	}
	let tex_store: string = parser_material_store_var_name(node);

	if (parser_material_sample_keep_aspect) {
		node_shader_write(parser_material_curshader, "vec2 " + tex_store + "_size = vec2(textureSize(" + tex_name + ", 0));");
		node_shader_write(parser_material_curshader, "float " + tex_store + "_ax = " + tex_store + "_size.x / " + tex_store + "_size.y;");
		node_shader_write(parser_material_curshader, "float " + tex_store + "_ay = " + tex_store + "_size.y / " + tex_store + "_size.x;");
		node_shader_write(parser_material_curshader, "vec2 " + tex_store + "_uv = ((" + uv_name + ".xy / " + parser_material_sample_uv_scale + " - vec2(0.5, 0.5)) * vec2(max(" + tex_store + "_ay, 1.0), max(" + tex_store + "_ax, 1.0))) + vec2(0.5, 0.5);");
		node_shader_write(parser_material_curshader, "if (" + tex_store + "_uv.x < 0.0 || " + tex_store + "_uv.y < 0.0 || " + tex_store + "_uv.x > 1.0 || " + tex_store + "_uv.y > 1.0) discard;");
		node_shader_write(parser_material_curshader, "" + tex_store + "_uv *= " + parser_material_sample_uv_scale + ";");
		uv_name = tex_store + "_uv";
	}

	if (parser_material_triplanar) {
		node_shader_write(parser_material_curshader, "vec4 " + tex_store + " = vec4(0.0, 0.0, 0.0, 0.0);");
		node_shader_write(parser_material_curshader, "if (tex_coord_blend.x > 0) " + tex_store + " += texture(" + tex_name + ", " + uv_name + ".xy) * tex_coord_blend.x;");
		node_shader_write(parser_material_curshader, "if (tex_coord_blend.y > 0) " + tex_store + " += texture(" + tex_name + ", " + uv_name + "1.xy) * tex_coord_blend.y;");
		node_shader_write(parser_material_curshader, "if (tex_coord_blend.z > 0) " + tex_store + " += texture(" + tex_name + ", " + uv_name + "2.xy) * tex_coord_blend.z;");
	}
	else {
		if (parser_material_curshader == parser_material_frag) {
			map_set(parser_material_texture_map, tex_store, "texture(" + tex_name + ", " + uv_name + ".xy)");
			node_shader_write(parser_material_curshader, "vec4 " + tex_store + " = texture(" + tex_name + ", " + uv_name + ".xy);");
		}
		else {
			map_set(parser_material_texture_map, tex_store, "textureLod(" + tex_name + ", " + uv_name + ".xy, 0.0)");
			node_shader_write(parser_material_curshader, "vec4 " + tex_store + " = textureLod(" + tex_name + ", " + uv_name + ".xy, 0.0);");
		}
		if (!ends_with(tex.file, ".jpg")) { // Pre-mult alpha
			node_shader_write(parser_material_curshader, tex_store + ".rgb *= " + tex_store + ".a;");
		}
	}

	if (parser_material_transform_color_space) {
		// Base color socket auto-converts from sRGB to linear
		if (color_space == color_space_t.LINEAR && parser_material_parsing_basecolor) { // Linear to sRGB
			node_shader_write(parser_material_curshader, tex_store + ".rgb = pow(" + tex_store + ".rgb, vec3(2.2, 2.2, 2.2));");
		}
		else if (color_space == color_space_t.SRGB && !parser_material_parsing_basecolor) { // sRGB to linear
			node_shader_write(parser_material_curshader, tex_store + ".rgb = pow(" + tex_store + ".rgb, vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));");
		}
		else if (color_space == color_space_t.DIRECTX_NORMAL_MAP) { // DirectX normal map to OpenGL normal map
			node_shader_write(parser_material_curshader, tex_store + ".y = 1.0 - " + tex_store + ".y;");
		}
	}
	return tex_store;
}

function parser_material_vec1(v: f32): string {
	///if arm_android
	return "float(" + v + ")";
	///else
	return v + "";
	///end
}

function parser_material_vec3(v: f32_array_t): string {
	let v0: f32 = v[0];
	let v1: f32 = v[1];
	let v2: f32 = v[2];
	///if arm_android
	return "vec3(float(" + v0 + "), float(" + v1 + "), float(" + v2 + "))";
	///else
	return "vec3(" + v0 + ", " + v1 + ", " + v2 + ")";
	///end
}

function parser_material_to_vec3(s: string): string {
	///if arm_direct3d12
	return "(" + s + ").xxx";
	///else
	return "vec3(" + s + ")";
	///end
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

	if (context_raw.texture_filter) {
		tex.min_filter = "anisotropic";
		tex.mag_filter = "linear";
		tex.mipmap_filter = "linear";
		tex.generate_mipmaps = true;
	}
	else {
		tex.min_filter = "point";
		tex.mag_filter = "point";
		tex.mipmap_filter = "no";
	}

	tex.u_addressing = "repeat";
	tex.v_addressing = "repeat";
	return tex;
}

function parser_material_make_texture(image_node: ui_node_t, tex_name: string): bind_tex_t {
	let i: i32 = image_node.buttons[0].default_value[0];
	let filepath: string = parser_material_enum_data(base_enum_texts(image_node.type)[i]);
	if (filepath == "" || string_index_of(filepath, ".") == -1) {
		return null;
	}

	return parser_material_make_bind_tex(tex_name, filepath);
}

function parser_material_is_pow(num: i32): bool {
	return ((num & (num - 1)) == 0) && num != 0;
}

function parser_material_asset_path(s: string): string {
	return s;
}

function parser_material_extract_filename(s: string): string {
	let ar: string[] = string_split(s, ".");
	return ar[ar.length - 2] + "." + ar[ar.length - 1];
}

function parser_material_safestr(s: string): string {
	return s;
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

enum color_space_t {
	AUTO, // sRGB for base color, otherwise linear
	LINEAR,
	SRGB,
	DIRECTX_NORMAL_MAP,
}
