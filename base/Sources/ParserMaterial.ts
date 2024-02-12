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

class ParserMaterial {

	static con: NodeShaderContextRaw;
	static vert: NodeShaderRaw;
	static frag: NodeShaderRaw;
	static curshader: NodeShaderRaw;
	static matcon: material_context_t;
	static parsed: string[];
	static parents: zui_node_t[];

	static canvases: zui_node_canvas_t[];
	static nodes: zui_node_t[];
	static links: zui_node_link_t[];

	static cotangentFrameWritten: bool;
	static tex_coord = "texCoord";
	static eps = 0.000001;

	static customNodes = new Map();
	static parse_surface = true;
	static parse_opacity = true;
	static parse_height = false;
	static parse_height_as_channel = false;
	static parse_emission = false;
	static parse_subsurface = false;
	static parsing_basecolor = false;
	static triplanar = false; // Sample using texCoord/1/2 & texCoordBlend
	static sample_keep_aspect = false; // Adjust uvs to preserve texture aspect ratio
	static sample_uv_scale = `1.0`;
	static transform_color_space = true;

	static blur_passthrough = false;
	static warp_passthrough = false;
	static bake_passthrough = false;
	static bake_passthrough_strength = "0.0";
	static bake_passthrough_radius = "0.0";
	static bake_passthrough_offset = "0.0";
	static start_group: zui_node_canvas_t = null;
	static start_parents: zui_node_t[] = null;
	static start_node: zui_node_t = null;

	static arm_export_tangents = true;
	static out_normaltan: string; // Raw tangent space normal parsed from normal map

	static script_links: Map<string, string> = null;

	static getNode = (id: i32): zui_node_t => {
		for (let n of ParserMaterial.nodes) if (n.id == id) return n;
		return null;
	}

	static getLink = (id: i32): zui_node_link_t => {
		for (let l of ParserMaterial.links) if (l.id == id) return l;
		return null;
	}

	static getInputLink = (inp: zui_node_socket_t): zui_node_link_t => {
		for (let l of ParserMaterial.links) {
			if (l.to_id == inp.node_id) {
				let node = ParserMaterial.getNode(inp.node_id);
				if (node.inputs.length <= l.to_socket) return null;
				if (node.inputs[l.to_socket] == inp) return l;
			}
		}
		return null;
	}

	static getOutputLinks = (out: zui_node_socket_t): zui_node_link_t[] => {
		let ls: zui_node_link_t[] = null;
		for (let l of ParserMaterial.links) {
			if (l.from_id == out.node_id) {
				let node = ParserMaterial.getNode(out.node_id);
				if (node.outputs.length <= l.from_socket) continue;
				if (node.outputs[l.from_socket] == out) {
					if (ls == null) ls = [];
					ls.push(l);
				}
			}
		}
		return ls;
	}

	static init = () => {
		ParserMaterial.parsed = [];
		ParserMaterial.parents = [];
		ParserMaterial.cotangentFrameWritten = false;
		ParserMaterial.out_normaltan = "vec3(0.5, 0.5, 1.0)";
		ParserMaterial.script_links = null;
		ParserMaterial.parsing_basecolor = false;
	}

	static parse = (canvas: zui_node_canvas_t, _con: NodeShaderContextRaw, _vert: NodeShaderRaw, _frag: NodeShaderRaw, _matcon: material_context_t): TShaderOut => {
		zui_nodes_update_canvas_format(canvas);
		ParserMaterial.init();
		ParserMaterial.canvases = [canvas];
		ParserMaterial.nodes = canvas.nodes;
		ParserMaterial.links = canvas.links;
		ParserMaterial.con = _con;
		ParserMaterial.vert = _vert;
		ParserMaterial.frag = _frag;
		ParserMaterial.curshader = ParserMaterial.frag;
		ParserMaterial.matcon = _matcon;

		if (ParserMaterial.start_group != null) {
			ParserMaterial.push_group(ParserMaterial.start_group);
			ParserMaterial.parents = ParserMaterial.start_parents;
		}

		if (ParserMaterial.start_node != null) {
			let link: zui_node_link_t = { id: 99999, from_id: ParserMaterial.start_node.id, from_socket: 0, to_id: -1, to_socket: -1 };
			ParserMaterial.write_result(link);
			return {
				out_basecol: `vec3(0.0, 0.0, 0.0)`,
				out_roughness: `0.0`,
				out_metallic: `0.0`,
				out_occlusion: `1.0`,
				out_opacity: `1.0`,
				out_height: `0.0`,
				out_emission: `0.0`,
				out_subsurface: `0.0`
			}
		}

		let output_node = ParserMaterial.node_by_type(ParserMaterial.nodes, "OUTPUT_MATERIAL");
		if (output_node != null) {
			return ParserMaterial.parse_output(output_node);
		}
		output_node = ParserMaterial.node_by_type(ParserMaterial.nodes, "OUTPUT_MATERIAL_PBR");
		if (output_node != null) {
			return ParserMaterial.parse_output_pbr(output_node);
		}
		return null;
	}

	static finalize = (con: NodeShaderContextRaw) => {
		let vert = con.vert;
		let frag = con.frag;

		if (frag.dotNV) {
			frag.vVec = true;
			frag.n = true;
		}
		if (frag.vVec) {
			vert.wposition = true;
		}

		if (frag.bposition) {
			if (ParserMaterial.triplanar) {
				NodeShader.write_attrib(frag, `vec3 bposition = vec3(
					texCoord1.x * texCoordBlend.y + texCoord2.x * texCoordBlend.z,
					texCoord.x * texCoordBlend.x + texCoord2.y * texCoordBlend.z,
					texCoord.y * texCoordBlend.x + texCoord1.y * texCoordBlend.y);`);
			}
			else if (frag.ndcpos) {
				NodeShader.add_out(vert, "vec3 bposition");
				NodeShader.write(vert, `bposition = (ndc.xyz / ndc.w);`);
			}
			else {
				NodeShader.add_out(vert, "vec3 bposition");
				NodeShader.add_uniform(vert, "vec3 dim", "_dim");
				NodeShader.add_uniform(vert, "vec3 hdim", "_half_dim");
				NodeShader.write_attrib(vert, `bposition = (pos.xyz + hdim) / dim;`);
			}
		}
		if (frag.wposition) {
			NodeShader.add_uniform(vert, "mat4 W", "_world_matrix");
			NodeShader.add_out(vert, "vec3 wposition");
			NodeShader.write_attrib(vert, `wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;`);
		}
		else if (vert.wposition) {
			NodeShader.add_uniform(vert, "mat4 W", "_world_matrix");
			NodeShader.write_attrib(vert, `vec3 wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;`);
		}
		if (frag.vposition) {
			NodeShader.add_uniform(vert, "mat4 WV", "_world_view_matrix");
			NodeShader.add_out(vert, "vec3 vposition");
			NodeShader.write_attrib(vert, `vposition = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz;`);
		}
		if (frag.mposition) {
			NodeShader.add_out(vert, "vec3 mposition");
			if (frag.ndcpos) {
				NodeShader.write(vert, `mposition = (ndc.xyz / ndc.w);`);
			}
			else {
				NodeShader.write_attrib(vert, `mposition = pos.xyz;`);
			}
		}
		if (frag.wtangent) {
			// NodeShaderContext.add_elem(con, "tang", "short4norm");
			// NodeShader.add_uniform(vert, "mat3 N", "_normal_matrix");
			NodeShader.add_out(vert, "vec3 wtangent");
			// NodeShader.write_attrib(vert, `wtangent = normalize(mul(tang.xyz, N));`);
			NodeShader.write_attrib(vert, `wtangent = vec3(0.0, 0.0, 0.0);`);
		}
		if (frag.vVecCam) {
			NodeShader.add_uniform(vert, "mat4 WV", "_world_view_matrix");
			NodeShader.add_out(vert, "vec3 eyeDirCam");
			NodeShader.write_attrib(vert, `eyeDirCam = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz; eyeDirCam.z *= -1.0;`);
			NodeShader.write_attrib(frag, `vec3 vVecCam = normalize(eyeDirCam);`);
		}
		if (frag.vVec) {
			NodeShader.add_uniform(vert, "vec3 eye", "_camera_pos");
			NodeShader.add_out(vert, "vec3 eyeDir");
			NodeShader.write_attrib(vert, `eyeDir = eye - wposition;`);
			NodeShader.write_attrib(frag, `vec3 vVec = normalize(eyeDir);`);
		}
		if (frag.n) {
			NodeShader.add_uniform(vert, "mat3 N", "_normal_matrix");
			NodeShader.add_out(vert, "vec3 wnormal");
			NodeShader.write_attrib(vert, `wnormal = mul(vec3(nor.xy, pos.w), N);`);
			NodeShader.write_attrib(frag, `vec3 n = normalize(wnormal);`);
		}
		else if (vert.n) {
			NodeShader.add_uniform(vert, "mat3 N", "_normal_matrix");
			NodeShader.write_attrib(vert, `vec3 wnormal = normalize(mul(vec3(nor.xy, pos.w), N));`);
		}
		if (frag.nAttr) {
			NodeShader.add_out(vert, "vec3 nAttr");
			NodeShader.write_attrib(vert, `nAttr = vec3(nor.xy, pos.w);`);
		}
		if (frag.dotNV) {
			NodeShader.write_attrib(frag, `float dotNV = max(dot(n, vVec), 0.0);`);
		}
		if (frag.wvpposition) {
			NodeShader.add_out(vert, "vec4 wvpposition");
			NodeShader.write_end(vert, `wvpposition = gl_Position;`);
		}
		if (NodeShaderContext.is_elem(con, `col`)) {
			NodeShader.add_out(vert, `vec3 vcolor`);
			NodeShader.write_attrib(vert, `vcolor = col.rgb;`);
		}
	}

	static parse_output = (node: zui_node_t): TShaderOut => {
		if (ParserMaterial.parse_surface || ParserMaterial.parse_opacity) {
			return ParserMaterial.parse_shader_input(node.inputs[0]);
		}
		return null;
		// Parse volume, displacement..
	}

	static parse_output_pbr = (node: zui_node_t): TShaderOut => {
		if (ParserMaterial.parse_surface || ParserMaterial.parse_opacity) {
			return ParserMaterial.parse_shader(node, null);
		}
		return null;
		// Parse volume, displacement..
	}

	static get_group = (name: string): zui_node_canvas_t => {
		for (let g of Project.materialGroups) if (g.canvas.name == name) return g.canvas;
		return null;
	}

	static push_group = (g: zui_node_canvas_t) => {
		ParserMaterial.canvases.push(g);
		ParserMaterial.nodes = g.nodes;
		ParserMaterial.links = g.links;
	}

	static pop_group = () => {
		ParserMaterial.canvases.pop();
		let g = ParserMaterial.canvases[ParserMaterial.canvases.length - 1];
		ParserMaterial.nodes = g.nodes;
		ParserMaterial.links = g.links;
	}

	static parse_group = (node: zui_node_t, socket: zui_node_socket_t): string => { // Entering group
		ParserMaterial.parents.push(node);
		ParserMaterial.push_group(ParserMaterial.get_group(node.name));
		let output_node = ParserMaterial.node_by_type(ParserMaterial.nodes, `GROUP_OUTPUT`);
		if (output_node == null) return null;
		let index = ParserMaterial.socket_index(node, socket);
		let inp = output_node.inputs[index];
		let out_group = ParserMaterial.parse_input(inp);
		ParserMaterial.parents.pop();
		ParserMaterial.pop_group();
		return out_group;
	}

	static parse_group_input = (node: zui_node_t, socket: zui_node_socket_t): string => {
		let parent = ParserMaterial.parents.pop(); // Leaving group
		ParserMaterial.pop_group();
		let index = ParserMaterial.socket_index(node, socket);
		let inp = parent.inputs[index];
		let res = ParserMaterial.parse_input(inp);
		ParserMaterial.parents.push(parent); // Return to group
		ParserMaterial.push_group(ParserMaterial.get_group(parent.name));
		return res;
	}

	static parse_input = (inp: zui_node_socket_t): string => {
		if (inp.type == "RGB") {
			return ParserMaterial.parse_vector_input(inp);
		}
		else if (inp.type == "RGBA") {
			return ParserMaterial.parse_vector_input(inp);
		}
		else if (inp.type == "VECTOR") {
			return ParserMaterial.parse_vector_input(inp);
		}
		else if (inp.type == "VALUE") {
			return ParserMaterial.parse_value_input(inp);
		}
		return null;
	}

	static parse_shader_input = (inp: zui_node_socket_t): TShaderOut => {
		let l = ParserMaterial.getInputLink(inp);
		let from_node = l != null ? ParserMaterial.getNode(l.from_id) : null;
		if (from_node != null) {
			if (from_node.type == "REROUTE") {
				return ParserMaterial.parse_shader_input(from_node.inputs[0]);
			}
			return ParserMaterial.parse_shader(from_node, from_node.outputs[l.from_socket]);
		}
		else {
			return {
				out_basecol: "vec3(0.8, 0.8, 0.8)",
				out_roughness: "0.0",
				out_metallic: "0.0",
				out_occlusion: "1.0",
				out_opacity: "1.0",
				out_height: "0.0",
				out_emission: "0.0",
				out_subsurface: "0.0"
			};
		}
	}

	static parse_shader = (node: zui_node_t, socket: zui_node_socket_t): TShaderOut => {
		let sout: TShaderOut = {
			out_basecol: "vec3(0.8, 0.8, 0.8)",
			out_roughness: "0.0",
			out_metallic: "0.0",
			out_occlusion: "1.0",
			out_opacity: "1.0",
			out_height: "0.0",
			out_emission: "0.0",
			out_subsurface: "0.0"
		}

		if (node.type == "OUTPUT_MATERIAL_PBR") {
			if (ParserMaterial.parse_surface) {
				// Normal - parsed first to retrieve uv coords
				ParserMaterial.parse_normal_map_color_input(node.inputs[5]);
				// Base color
				ParserMaterial.parsing_basecolor = true;
				sout.out_basecol = ParserMaterial.parse_vector_input(node.inputs[0]);
				ParserMaterial.parsing_basecolor = false;
				// Occlusion
				sout.out_occlusion = ParserMaterial.parse_value_input(node.inputs[2]);
				// Roughness
				sout.out_roughness = ParserMaterial.parse_value_input(node.inputs[3]);
				// Metallic
				sout.out_metallic = ParserMaterial.parse_value_input(node.inputs[4]);
				// Emission
				if (ParserMaterial.parse_emission) {
					sout.out_emission = ParserMaterial.parse_value_input(node.inputs[6]);
				}
				// Subsurface
				if (ParserMaterial.parse_subsurface) {
					sout.out_subsurface = ParserMaterial.parse_value_input(node.inputs[8]);
				}
			}

			if (ParserMaterial.parse_opacity) {
				sout.out_opacity = ParserMaterial.parse_value_input(node.inputs[1]);
			}

			// Displacement / Height
			if (node.inputs.length > 7 && ParserMaterial.parse_height) {
				if (!ParserMaterial.parse_height_as_channel) ParserMaterial.curshader = ParserMaterial.vert;
				sout.out_height = ParserMaterial.parse_value_input(node.inputs[7]);
				if (!ParserMaterial.parse_height_as_channel) ParserMaterial.curshader = ParserMaterial.frag;
			}
		}

		return sout;
	}

	static parse_vector_input = (inp: zui_node_socket_t): string => {
		let l = ParserMaterial.getInputLink(inp);
		let from_node = l != null ? ParserMaterial.getNode(l.from_id) : null;
		if (from_node != null) {
			if (from_node.type == "REROUTE") {
				return ParserMaterial.parse_vector_input(from_node.inputs[0]);
			}

			let res_var = ParserMaterial.write_result(l);
			let st = from_node.outputs[l.from_socket].type;
			if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
				return res_var;
			}
			else {// VALUE
				return ParserMaterial.to_vec3(res_var);
			}
		}
		else {
			if (inp.type == "VALUE") { // Unlinked reroute
				return ParserMaterial.vec3([0.0, 0.0, 0.0]);
			}
			else {
				return ParserMaterial.vec3(inp.default_value);
			}
		}
	}

	static parse_vector = (node: zui_node_t, socket: zui_node_socket_t): string => {
		if (node.type == `GROUP`) {
			return ParserMaterial.parse_group(node, socket);
		}
		else if (node.type == `GROUP_INPUT`) {
			return ParserMaterial.parse_group_input(node, socket);
		}
		else if (node.type == "ATTRIBUTE") {
			if (socket == node.outputs[0]) { // Color
				if (ParserMaterial.curshader.context.allow_vcols) {
					NodeShaderContext.add_elem(ParserMaterial.curshader.context, "col", "short4norm"); // Vcols only for now
					return "vcolor";
				}
				else {
					return("vec3(0.0, 0.0, 0.0)");
				}
			}
			else { // Vector
				NodeShaderContext.add_elem(ParserMaterial.curshader.context, "tex", "short2norm"); // UVMaps only for now
				return "vec3(texCoord.x, texCoord.y, 0.0)";
			}
		}
		else if (node.type == "VERTEX_COLOR") {
			if (ParserMaterial.curshader.context.allow_vcols) {
				NodeShaderContext.add_elem(ParserMaterial.curshader.context, "col", "short4norm");
				return "vcolor";
			}
			else {
				return("vec3(0.0, 0.0, 0.0)");
			}

		}
		else if (node.type == "RGB") {
			return ParserMaterial.vec3(socket.default_value);
		}
		else if (node.type == "TEX_BRICK") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_brick);
			let co = ParserMaterial.getCoord(node);
			let col1 = ParserMaterial.parse_vector_input(node.inputs[1]);
			let col2 = ParserMaterial.parse_vector_input(node.inputs[2]);
			let col3 = ParserMaterial.parse_vector_input(node.inputs[3]);
			let scale = ParserMaterial.parse_value_input(node.inputs[4]);
			let res = `tex_brick(${co} * ${scale}, ${col1}, ${col2}, ${col3})`;
			return res;
		}
		else if (node.type == "TEX_CHECKER") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_checker);
			let co = ParserMaterial.getCoord(node);
			let col1 = ParserMaterial.parse_vector_input(node.inputs[1]);
			let col2 = ParserMaterial.parse_vector_input(node.inputs[2]);
			let scale = ParserMaterial.parse_value_input(node.inputs[3]);
			let res = `tex_checker(${co}, ${col1}, ${col2}, ${scale})`;
			return res;
		}
		else if (node.type == "TEX_GRADIENT") {
			let co = ParserMaterial.getCoord(node);
			let but = node.buttons[0]; //gradient_type;
			let grad: string = but.data[but.default_value].toUpperCase();
			grad = grad.replaceAll(" ", "_");
			let f = ParserMaterial.getGradient(grad, co);
			let res = ParserMaterial.to_vec3(`clamp(${f}, 0.0, 1.0)`);
			return res;
		}
		else if (node.type == "TEX_IMAGE") {
			// Already fetched
			if (ParserMaterial.parsed.indexOf(ParserMaterial.res_var_name(node, node.outputs[1])) >= 0) { // TODO: node.outputs[0]
				let varname = ParserMaterial.store_var_name(node);
				return `${varname}.rgb`;
			}
			let tex_name = ParserMaterial.node_name(node);
			let tex = ParserMaterial.make_texture(node, tex_name);
			if (tex != null) {
				let color_space = node.buttons[1].default_value;
				let texstore = ParserMaterial.texture_store(node, tex, tex_name, color_space);
				return `${texstore}.rgb`;
			}
			else {
				let tex_store = ParserMaterial.store_var_name(node); // Pink color for missing texture
				NodeShader.write(ParserMaterial.curshader, `vec4 ${tex_store} = vec4(1.0, 0.0, 1.0, 1.0);`);
				return `${tex_store}.rgb`;
			}
		}
		else if (node.type == "TEX_MAGIC") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_magic);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let res = `tex_magic(${co} * ${scale} * 4.0)`;
			return res;
		}
		else if (node.type == "TEX_NOISE") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_noise);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let res = `vec3(tex_noise(${co} * ${scale}), tex_noise(${co} * ${scale} + 0.33), tex_noise(${co} * ${scale} + 0.66))`;
			return res;
		}
		else if (node.type == "TEX_VORONOI") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_voronoi);
			NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D snoise256", "$noise256.k");
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let but = node.buttons[0]; //coloring;
			let coloring: string = but.data[but.default_value].toUpperCase();
			coloring = coloring.replaceAll(" ", "_");
			let res = "";
			if (coloring == "INTENSITY") {
				res = ParserMaterial.to_vec3(`tex_voronoi(${co} * ${scale}, texturePass(snoise256)).a`);
			}
			else { // Cells
				res = `tex_voronoi(${co} * ${scale}, texturePass(snoise256)).rgb`;
			}
			return res;
		}
		else if (node.type == "TEX_WAVE") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_wave);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let res = ParserMaterial.to_vec3(`tex_wave_f(${co} * ${scale})`);
			return res;
		}
		else if (node.type == "BRIGHTCONTRAST") {
			let out_col = ParserMaterial.parse_vector_input(node.inputs[0]);
			let bright = ParserMaterial.parse_value_input(node.inputs[1]);
			let contr = ParserMaterial.parse_value_input(node.inputs[2]);
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_brightcontrast);
			return `brightcontrast(${out_col}, ${bright}, ${contr})`;
		}
		else if (node.type == "GAMMA") {
			let out_col = ParserMaterial.parse_vector_input(node.inputs[0]);
			let gamma = ParserMaterial.parse_value_input(node.inputs[1]);
			return `pow(${out_col}, ` + ParserMaterial.to_vec3(`${gamma}`) + ")";
		}
		else if (node.type == "DIRECT_WARP") {
			if (ParserMaterial.warp_passthrough) return ParserMaterial.parse_vector_input(node.inputs[0]);
			let angle = ParserMaterial.parse_value_input(node.inputs[1], true);
			let mask = ParserMaterial.parse_value_input(node.inputs[2], true);
			let tex_name = "texwarp_" + ParserMaterial.node_name(node);
			NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D " + tex_name, "_" + tex_name);
			let store = ParserMaterial.store_var_name(node);
			NodeShader.write(ParserMaterial.curshader, `float ${store}_rad = ${angle} * (${Math.PI} / 180);`);
			NodeShader.write(ParserMaterial.curshader, `float ${store}_x = cos(${store}_rad);`);
			NodeShader.write(ParserMaterial.curshader, `float ${store}_y = sin(${store}_rad);`);
			return `texture(${tex_name}, texCoord + vec2(${store}_x, ${store}_y) * ${mask}).rgb;`;
		}
		else if (node.type == "BLUR") {
			if (ParserMaterial.blur_passthrough) return ParserMaterial.parse_vector_input(node.inputs[0]);
			let strength = ParserMaterial.parse_value_input(node.inputs[1]);
			if (strength == "0.0") return "vec3(0.0, 0.0, 0.0)";
			let steps = `int(${strength} * 10 + 1)`;
			let tex_name = "texblur_" + ParserMaterial.node_name(node);
			NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D " + tex_name, "_" + tex_name);
			let store = ParserMaterial.store_var_name(node);
			NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_res = vec3(0.0, 0.0, 0.0);`);
			NodeShader.write(ParserMaterial.curshader, `for (int i = -$steps; i <= $steps; ++i) {`);
			NodeShader.write(ParserMaterial.curshader, `for (int j = -$steps; j <= $steps; ++j) {`);
			NodeShader.write(ParserMaterial.curshader, `${store}_res += texture(${tex_name}, texCoord + vec2(i, j) / vec2(textureSize(${tex_name}, 0))).rgb;`);
			NodeShader.write(ParserMaterial.curshader, `}`);
			NodeShader.write(ParserMaterial.curshader, `}`);
			NodeShader.write(ParserMaterial.curshader, `${store}_res /= ($steps * 2 + 1) * ($steps * 2 + 1);`);
			return `${store}_res`;
		}
		else if (node.type == "HUE_SAT") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_hue_sat);
			let hue = ParserMaterial.parse_value_input(node.inputs[0]);
			let sat = ParserMaterial.parse_value_input(node.inputs[1]);
			let val = ParserMaterial.parse_value_input(node.inputs[2]);
			let fac = ParserMaterial.parse_value_input(node.inputs[3]);
			let col = ParserMaterial.parse_vector_input(node.inputs[4]);
			return `hue_sat(${col}, vec4(${hue}-0.5, ${sat}, ${val}, 1.0-${fac}))`;
		}
		else if (node.type == "INVERT") {
			let fac = ParserMaterial.parse_value_input(node.inputs[0]);
			let out_col = ParserMaterial.parse_vector_input(node.inputs[1]);
			return `mix(${out_col}, vec3(1.0, 1.0, 1.0) - (${out_col}), ${fac})`;
		}
		else if (node.type == "MIX_RGB") {
			let fac = ParserMaterial.parse_value_input(node.inputs[0]);
			let fac_var = ParserMaterial.node_name(node) + "_fac";
			NodeShader.write(ParserMaterial.curshader, `float ${fac_var} = ${fac};`);
			let col1 = ParserMaterial.parse_vector_input(node.inputs[1]);
			let col2 = ParserMaterial.parse_vector_input(node.inputs[2]);
			let but = node.buttons[0]; // blend_type
			let blend: string = but.data[but.default_value].toUpperCase();
			blend = blend.replaceAll(" ", "_");
			let use_clamp = node.buttons[1].default_value == true;
			let out_col = "";
			if (blend == "MIX") {
				out_col = `mix(${col1}, ${col2}, ${fac_var})`;
			}
			else if (blend == "DARKEN") {
				out_col = `min(${col1}, ${col2} * ${fac_var})`;
			}
			else if (blend == "MULTIPLY") {
				out_col = `mix(${col1}, ${col1} * ${col2}, ${fac_var})`;
			}
			else if (blend == "BURN") {
				out_col = `mix(${col1}, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - ${col1}) / ${col2}, ${fac_var})`;
			}
			else if (blend == "LIGHTEN") {
				out_col = `max(${col1}, ${col2} * ${fac_var})`;
			}
			else if (blend == "SCREEN") {
				out_col = `(vec3(1.0, 1.0, 1.0) - (` + ParserMaterial.to_vec3(`1.0 - ${fac_var}`) + ` + ${fac_var} * (vec3(1.0, 1.0, 1.0) - ${col2})) * (vec3(1.0, 1.0, 1.0) - ${col1}))`;
			}
			else if (blend == "DODGE") {
				out_col = `mix(${col1}, ${col1} / (vec3(1.0, 1.0, 1.0) - ${col2}), ${fac_var})`;
			}
			else if (blend == "ADD") {
				out_col = `mix(${col1}, ${col1} + ${col2}, ${fac_var})`;
			}
			else if (blend == "OVERLAY") {
				out_col = `mix(${col1}, vec3(
					${col1}.r < 0.5 ? 2.0 * ${col1}.r * ${col2}.r : 1.0 - 2.0 * (1.0 - ${col1}.r) * (1.0 - ${col2}.r),
					${col1}.g < 0.5 ? 2.0 * ${col1}.g * ${col2}.g : 1.0 - 2.0 * (1.0 - ${col1}.g) * (1.0 - ${col2}.g),
					${col1}.b < 0.5 ? 2.0 * ${col1}.b * ${col2}.b : 1.0 - 2.0 * (1.0 - ${col1}.b) * (1.0 - ${col2}.b)
				), ${fac_var})`;
			}
			else if (blend == "SOFT_LIGHT") {
				out_col = `((1.0 - ${fac_var}) * ${col1} + ${fac_var} * ((vec3(1.0, 1.0, 1.0) - ${col1}) * ${col2} * ${col1} + ${col1} * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - ${col2}) * (vec3(1.0, 1.0, 1.0) - ${col1}))))`;
			}
			else if (blend == "LINEAR_LIGHT") {
				out_col = `(${col1} + ${fac_var} * (vec3(2.0, 2.0, 2.0) * (${col2} - vec3(0.5, 0.5, 0.5))))`;
			}
			else if (blend == "DIFFERENCE") {
				out_col = `mix(${col1}, abs(${col1} - ${col2}), ${fac_var})`;
			}
			else if (blend == "SUBTRACT") {
				out_col = `mix(${col1}, ${col1} - ${col2}, ${fac_var})`;
			}
			else if (blend == "DIVIDE") {
				let eps = 0.000001;
				col2 = `max(${col2}, vec3(${eps}, ${eps}, ${eps}))`;
				out_col = "(" + ParserMaterial.to_vec3(`(1.0 - ${fac_var}) * ${col1} + ${fac_var} * ${col1} / ${col2}`) + ")";
			}
			else if (blend == "HUE") {
				NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_hue_sat);
				out_col = `mix(${col1}, hsv_to_rgb(vec3(rgb_to_hsv(${col2}).r, rgb_to_hsv(${col1}).g, rgb_to_hsv(${col1}).b)), ${fac_var})`;
			}
			else if (blend == "SATURATION") {
				NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_hue_sat);
				out_col = `mix(${col1}, hsv_to_rgb(vec3(rgb_to_hsv(${col1}).r, rgb_to_hsv(${col2}).g, rgb_to_hsv(${col1}).b)), ${fac_var})`;
			}
			else if (blend == "COLOR") {
				NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_hue_sat);
				out_col = `mix(${col1}, hsv_to_rgb(vec3(rgb_to_hsv(${col2}).r, rgb_to_hsv(${col2}).g, rgb_to_hsv(${col1}).b)), ${fac_var})`;
			}
			else if (blend == "VALUE") {
				NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_hue_sat);
				out_col = `mix(${col1}, hsv_to_rgb(vec3(rgb_to_hsv(${col1}).r, rgb_to_hsv(${col1}).g, rgb_to_hsv(${col2}).b)), ${fac_var})`;
			}
			if (use_clamp) return `clamp(${out_col}, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0))`;
			else return out_col;
		}
		else if (node.type == "QUANTIZE") {
			let strength = ParserMaterial.parse_value_input(node.inputs[0]);
			let col = ParserMaterial.parse_vector_input(node.inputs[1]);
			return `(floor(100.0 * ${strength} * ${col}) / (100.0 * ${strength}))`;
		}
		else if (node.type == "VALTORGB") { // ColorRamp
			let fac = ParserMaterial.parse_value_input(node.inputs[0]);
			let interp = node.buttons[0].data == 0 ? "LINEAR" : "CONSTANT";
			let elems: f32[][] = node.buttons[0].default_value;
			if (elems.length == 1) {
				return ParserMaterial.vec3(elems[0]);
			}
			// Write cols array
			let cols_var = ParserMaterial.node_name(node) + "_cols";
			NodeShader.write(ParserMaterial.curshader, `vec3 ${cols_var}[${elems.length}];`); // TODO: Make const
			for (let i = 0; i < elems.length; ++i) {
				NodeShader.write(ParserMaterial.curshader, `${cols_var}[${i}] = ${ParserMaterial.vec3(elems[i])};`);
			}
			// Get index
			let fac_var = ParserMaterial.node_name(node) + "_fac";
			NodeShader.write(ParserMaterial.curshader, `float ${fac_var} = ${fac};`);
			let index = "0";
			for (let i = 1; i < elems.length; ++i) {
				index += ` + (${fac_var} > ${elems[i][4]} ? 1 : 0)`;
			}
			// Write index
			let index_var = ParserMaterial.node_name(node) + "_i";
			NodeShader.write(ParserMaterial.curshader, `int ${index_var} = ${index};`);
			if (interp == "CONSTANT") {
				return `${cols_var}[${index_var}]`;
			}
			else { // Linear
				// Write facs array
				let facs_var = ParserMaterial.node_name(node) + "_facs";
				NodeShader.write(ParserMaterial.curshader, `float ${facs_var}[${elems.length}];`); // TODO: Make const
				for (let i = 0; i < elems.length; ++i) {
					NodeShader.write(ParserMaterial.curshader, `${facs_var}[${i}] = ${elems[i][4]};`);
				}
				// Mix color
				// float f = (pos - start) * (1.0 / (finish - start))
				return `mix(${cols_var}[${index_var}], ${cols_var}[${index_var} + 1], (${fac_var} - ${facs_var}[${index_var}]) * (1.0 / (${facs_var}[${index_var} + 1] - ${facs_var}[${index_var}]) ))`;
			}
		}
		else if (node.type == "CURVE_VEC") {
			let fac = ParserMaterial.parse_value_input(node.inputs[0]);
			let vec = ParserMaterial.parse_vector_input(node.inputs[1]);
			let curves = node.buttons[0].default_value;
			let name = ParserMaterial.node_name(node);
			let vc0 = ParserMaterial.vector_curve(name + "0", vec + ".x", curves[0]);
			let vc1 = ParserMaterial.vector_curve(name + "1", vec + ".y", curves[1]);
			let vc2 = ParserMaterial.vector_curve(name + "2", vec + ".z", curves[2]);
			// mapping.curves[0].points[0].handle_type // bezier curve
			return `(vec3(${vc0}, ${vc1}, ${vc2}) * ${fac})`;
		}
		else if (node.type == "CURVE_RGB") { // RGB Curves
			let fac = ParserMaterial.parse_value_input(node.inputs[0]);
			let vec = ParserMaterial.parse_vector_input(node.inputs[1]);
			let curves = node.buttons[0].default_value;
			let name = ParserMaterial.node_name(node);
			// mapping.curves[0].points[0].handle_type
			let vc0 = ParserMaterial.vector_curve(name + "0", vec + ".x", curves[0]);
			let vc1 = ParserMaterial.vector_curve(name + "1", vec + ".y", curves[1]);
			let vc2 = ParserMaterial.vector_curve(name + "2", vec + ".z", curves[2]);
			let vc3a = ParserMaterial.vector_curve(name + "3a", vec + ".x", curves[3]);
			let vc3b = ParserMaterial.vector_curve(name + "3b", vec + ".y", curves[3]);
			let vc3c = ParserMaterial.vector_curve(name + "3c", vec + ".z", curves[3]);
			return `(sqrt(vec3(${vc0}, ${vc1}, ${vc2}) * vec3(${vc3a}, ${vc3b}, ${vc3c})) * ${fac})`;
		}
		else if (node.type == "COMBHSV") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_hue_sat);
			let h = ParserMaterial.parse_value_input(node.inputs[0]);
			let s = ParserMaterial.parse_value_input(node.inputs[1]);
			let v = ParserMaterial.parse_value_input(node.inputs[2]);
			return `hsv_to_rgb(vec3(${h}, ${s}, ${v}))`;
		}
		else if (node.type == "COMBRGB") {
			let r = ParserMaterial.parse_value_input(node.inputs[0]);
			let g = ParserMaterial.parse_value_input(node.inputs[1]);
			let b = ParserMaterial.parse_value_input(node.inputs[2]);
			return `vec3(${r}, ${g}, ${b})`;
		}
		else if (node.type == "WAVELENGTH") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_wavelength_to_rgb);
			let wl = ParserMaterial.parse_value_input(node.inputs[0]);
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_wavelength_to_rgb);
			return `wavelength_to_rgb((${wl} - 450.0) / 150.0)`;
		}
		else if (node.type == "CAMERA") {
			ParserMaterial.curshader.vVecCam = true;
			return "vVecCam";
		}
		else if (node.type == "LAYER") {
			let l = node.buttons[0].default_value;
			if (socket == node.outputs[0]) { // Base
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint" + l, "_texpaint" + l);
				return "texture(texpaint" + l + ", texCoord).rgb";
			}
			else if (socket == node.outputs[5]) { // Normal
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint_nor" + l, "_texpaint_nor" + l);
				return "texture(texpaint_nor" + l + ", texCoord).rgb";
			}
		}
		else if (node.type == "MATERIAL") {
			let result = "vec3(0.0, 0.0, 0.0)";
			let mi = node.buttons[0].default_value;
			if (mi >= Project.materials.length) return result;
			let m = Project.materials[mi];
			let _nodes = ParserMaterial.nodes;
			let _links = ParserMaterial.links;
			ParserMaterial.nodes = m.canvas.nodes;
			ParserMaterial.links = m.canvas.links;
			ParserMaterial.parents.push(node);
			let output_node = ParserMaterial.node_by_type(ParserMaterial.nodes, "OUTPUT_MATERIAL_PBR");
			if (socket == node.outputs[0]) { // Base
				result = ParserMaterial.parse_vector_input(output_node.inputs[0]);
			}
			else if (socket == node.outputs[5]) { // Normal
				result = ParserMaterial.parse_vector_input(output_node.inputs[5]);
			}
			ParserMaterial.nodes = _nodes;
			ParserMaterial.links = _links;
			ParserMaterial.parents.pop();
			return result;
		}
		else if (node.type == "PICKER") {
			if (socket == node.outputs[0]) { // Base
				NodeShader.add_uniform(ParserMaterial.curshader, "vec3 pickerBase", "_pickerBase");
				return "pickerBase";
			}
			else if (socket == node.outputs[5]) { // Normal
				NodeShader.add_uniform(ParserMaterial.curshader, "vec3 pickerNormal", "_pickerNormal");
				return "pickerNormal";
			}
		}
		else if (node.type == "NEW_GEOMETRY") {
			if (socket == node.outputs[0]) { // Position
				ParserMaterial.curshader.wposition = true;
				return "wposition";
			}
			else if (socket == node.outputs[1]) { // Normal
				ParserMaterial.curshader.n = true;
				return "n";
			}
			else if (socket == node.outputs[2]) { // Tangent
				ParserMaterial.curshader.wtangent = true;
				return "wtangent";
			}
			else if (socket == node.outputs[3]) { // True Normal
				ParserMaterial.curshader.n = true;
				return "n";
			}
			else if (socket == node.outputs[4]) { // Incoming
				ParserMaterial.curshader.vVec = true;
				return "vVec";
			}
			else if (socket == node.outputs[5]) { // Parametric
				ParserMaterial.curshader.mposition = true;
				return "mposition";
			}
		}
		else if (node.type == "OBJECT_INFO") {
			if (socket == node.outputs[0]) { // Location
				ParserMaterial.curshader.wposition = true;
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
			ParserMaterial.curshader.wtangent = true;
			return "wtangent";
		}
		else if (node.type == "TEX_COORD") {
			if (socket == node.outputs[0]) { // Generated - bounds
				ParserMaterial.curshader.bposition = true;
				return "bposition";
			}
			else if (socket == node.outputs[1]) { // Normal
				ParserMaterial.curshader.n = true;
				return "n";
			}
			else if (socket == node.outputs[2]) {// UV
				NodeShaderContext.add_elem(ParserMaterial.curshader.context, "tex", "short2norm");
				return "vec3(texCoord.x, texCoord.y, 0.0)";
			}
			else if (socket == node.outputs[3]) { // Object
				ParserMaterial.curshader.mposition = true;
				return "mposition";
			}
			else if (socket == node.outputs[4]) { // Camera
				ParserMaterial.curshader.vposition = true;
				return "vposition";
			}
			else if (socket == node.outputs[5]) { // Window
				ParserMaterial.curshader.wvpposition = true;
				return "wvpposition.xyz";
			}
			else if (socket == node.outputs[6]) { // Reflection
				return "vec3(0.0, 0.0, 0.0)";
			}
		}
		else if (node.type == "UVMAP") {
			NodeShaderContext.add_elem(ParserMaterial.curshader.context, "tex", "short2norm");
			return "vec3(texCoord.x, texCoord.y, 0.0)";
		}
		else if (node.type == "BUMP") {
			let strength = ParserMaterial.parse_value_input(node.inputs[0]);
			// let distance = ParserMaterial.parse_value_input(node.inputs[1]);
			let height = ParserMaterial.parse_value_input(node.inputs[2]);
			let nor = ParserMaterial.parse_vector_input(node.inputs[3]);
			let sample_bump_res = ParserMaterial.store_var_name(node) + "_bump";
			NodeShader.write(ParserMaterial.curshader, `float ${sample_bump_res}_x = dFdx(float(${height})) * (${strength}) * 16.0;`);
			NodeShader.write(ParserMaterial.curshader, `float ${sample_bump_res}_y = dFdy(float(${height})) * (${strength}) * 16.0;`);
			return `(normalize(vec3(${sample_bump_res}_x, ${sample_bump_res}_y, 1.0) + ${nor}) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5))`;
		}
		else if (node.type == "MAPPING") {
			let out = ParserMaterial.parse_vector_input(node.inputs[0]);
			let node_translation = ParserMaterial.parse_vector_input(node.inputs[1]);
			let node_rotation = ParserMaterial.parse_vector_input(node.inputs[2]);
			let node_scale = ParserMaterial.parse_vector_input(node.inputs[3]);
			if (node_scale != `vec3(1, 1, 1)`) {
				out = `(${out} * ${node_scale})`;
			}
			if (node_rotation != `vec3(0, 0, 0)`) {
				// ZYX rotation, Z axis for now..
				let a = `${node_rotation}.z * (3.1415926535 / 180)`;
				// x * cos(theta) - y * sin(theta)
				// x * sin(theta) + y * cos(theta)
				out = `vec3(${out}.x * cos(${a}) - ${out}.y * sin(${a}), ${out}.x * sin(${a}) + ${out}.y * cos(${a}), 0.0)`;
			}
			// if node.rotation[1] != 0.0:
			//     a = node.rotation[1]
			//     out = `vec3({0}.x * {1} - {0}.z * {2}, {0}.x * {2} + {0}.z * {1}, 0.0)`.format(out, math.cos(a), math.sin(a))
			// if node.rotation[0] != 0.0:
			//     a = node.rotation[0]
			//     out = `vec3({0}.y * {1} - {0}.z * {2}, {0}.y * {2} + {0}.z * {1}, 0.0)`.format(out, math.cos(a), math.sin(a))
			if (node_translation != `vec3(0, 0, 0)`) {
				out = `(${out} + ${node_translation})`;
			}
			// if node.use_min:
				// out = `max({0}, vec3({1}, {2}, {3}))`.format(out, node.min[0], node.min[1])
			// if node.use_max:
				 // out = `min({0}, vec3({1}, {2}, {3}))`.format(out, node.max[0], node.max[1])
			return out;
		}
		else if (node.type == "NORMAL") {
			if (socket == node.outputs[0]) {
				return ParserMaterial.vec3(node.outputs[0].default_value);
			}
			else if (socket == node.outputs[1]) {
				let nor = ParserMaterial.parse_vector_input(node.inputs[0]);
				let norout = ParserMaterial.vec3(node.outputs[0].default_value);
				return ParserMaterial.to_vec3(`dot(${norout}, ${nor})`);
			}
		}
		else if (node.type == "NORMAL_MAP") {
			let strength = ParserMaterial.parse_value_input(node.inputs[0]);
			let norm = ParserMaterial.parse_vector_input(node.inputs[1]);

			let store = ParserMaterial.store_var_name(node);
			NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_texn = ${norm} * 2.0 - 1.0;`);
			NodeShader.write(ParserMaterial.curshader, `${store}_texn.xy = ${strength} * ${store}_texn.xy;`);
			NodeShader.write(ParserMaterial.curshader, `${store}_texn = normalize(${store}_texn);`);

			return `(0.5 * ${store}_texn + 0.5)`;
		}
		else if (node.type == "MIX_NORMAL_MAP") {
			let nm1 = ParserMaterial.parse_vector_input(node.inputs[0]);
			let nm2 = ParserMaterial.parse_vector_input(node.inputs[1]);
			let but = node.buttons[0];
			let blend: string = but.data[but.default_value].toUpperCase(); // blend_type
			blend = blend.replaceAll(" ", "_");
			let store = ParserMaterial.store_var_name(node);

			// The blending algorithms are based on the paper `Blending in Detail` by Colin Barré-Brisebois and Stephen Hill 2012
			// https://blog.selfshadow.com/publications/blending-in-detail/
			if (blend == "PARTIAL_DERIVATIVE") { //partial derivate blending
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_n1 = ${nm1} * 2.0 - 1.0;`);
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_n2 = ${nm2} * 2.0 - 1.0;`);
				return `0.5 * normalize(vec3(${store}_n1.xy * ${store}_n2.z + ${store}_n2.xy * ${store}_n1.z, ${store}_n1.z * ${store}_n2.z)) + 0.5;`;
			}
			else if (blend == "WHITEOUT") { //whiteout blending
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_n1 = ${nm1} * 2.0 - 1.0;`);
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_n2 = ${nm2} * 2.0 - 1.0;`);
				return `0.5 * normalize(vec3(${store}_n1.xy + ${store}_n2.xy, ${store}_n1.z * ${store}_n2.z)) + 0.5;`;
			}
			else if (blend == "REORIENTED") { //reoriented normal mapping
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_n1 = ${nm1} * 2.0 - vec3(1.0, 1.0, 0.0);`);
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_n2 = ${nm2} * vec3(-2.0, -2.0, 2.0) - vec3(-1.0, -1.0, 1.0);`);
				return `0.5 * normalize(${store}_n1 * dot(${store}_n1, ${store}_n2) - ${store}_n2 * ${store}_n1.z) + 0.5`;
			}
		}
		else if (node.type == "VECT_TRANSFORM") {
		// 	type = node.vector_type
		// 	conv_from = node.convert_from
		// 	conv_to = node.convert_to
		// 	// Pass throuh
		// 	return ParserMaterial.parse_vector_input(node.inputs[0])
		}
		else if (node.type == "COMBXYZ") {
			let x = ParserMaterial.parse_value_input(node.inputs[0]);
			let y = ParserMaterial.parse_value_input(node.inputs[1]);
			let z = ParserMaterial.parse_value_input(node.inputs[2]);
			return `vec3(${x}, ${y}, ${z})`;
		}
		else if (node.type == "VECT_MATH") {
			let vec1 = ParserMaterial.parse_vector_input(node.inputs[0]);
			let vec2 = ParserMaterial.parse_vector_input(node.inputs[1]);
			let but = node.buttons[0]; //operation;
			let op: string = but.data[but.default_value].toUpperCase();
			op = op.replaceAll(" ", "_");
			if (op == "ADD") {
				return `(${vec1} + ${vec2})`;
			}
			else if (op == "SUBTRACT") {
				return `(${vec1} - ${vec2})`;
			}
			else if (op == "AVERAGE") {
				return `((${vec1} + ${vec2}) / 2.0)`;
			}
			else if (op == "DOT_PRODUCT") {
				return ParserMaterial.to_vec3(`dot(${vec1}, ${vec2})`);
			}
			else if (op == "LENGTH") {
				return ParserMaterial.to_vec3(`length(${vec1})`);
			}
			else if (op == "DISTANCE") {
				return ParserMaterial.to_vec3(`distance(${vec1}, ${vec2})`);
			}
			else if (op == "CROSS_PRODUCT") {
				return `cross(${vec1}, ${vec2})`;
			}
			else if (op == "NORMALIZE") {
				return `normalize(${vec1})`;
			}
			else if (op == "MULTIPLY") {
				return `(${vec1} * ${vec2})`;
			}
			else if (op == "DIVIDE") {
				return `vec3(${vec1}.x / (${vec2}.x == 0 ? 0.000001 : ${vec2}.x), ${vec1}.y / (${vec2}.y == 0 ? 0.000001 : ${vec2}.y), ${vec1}.z / (${vec2}.z == 0 ? 0.000001 : ${vec2}.z))`;
			}
			else if (op == "PROJECT") {
				return `(dot(${vec1}, ${vec2}) / dot(${vec2}, ${vec2}) * ${vec2})`;
			}
			else if (op == "REFLECT") {
				return `reflect(${vec1}, normalize(${vec2}))`;
			}
			else if (op == "SCALE") {
				return `(${vec2}.x * ${vec1})`;
			}
			else if (op == "ABSOLUTE") {
				return `abs(${vec1})`;
			}
			else if (op == "MINIMUM") {
				return `min(${vec1}, ${vec2})`;
			}
			else if (op == "MAXIMUM") {
				return `max(${vec1}, ${vec2})`;
			}
			else if (op == "FLOOR") {
				return `floor(${vec1})`;
			}
			else if (op == "CEIL") {
				return `ceil(${vec1})`;
			}
			else if (op == "FRACTION") {
				return `fract(${vec1})`;
			}
			else if (op == "MODULO") {
				return `mod(${vec1}, ${vec2})`;
			}
			else if(op == "SNAP") {
				return `(floor(${vec1} / ${vec2}) * ${vec2})`;
			}
			else if (op == "SINE") {
				return `sin(${vec1})`;
			}
			else if (op == "COSINE") {
				return `cos(${vec1})`;
			}
			else if (op == "TANGENT") {
				return `tan(${vec1})`;
			}
		}
		else if (node.type == "Displacement") {
			let height = ParserMaterial.parse_value_input(node.inputs[0]);
			return ParserMaterial.to_vec3(`${height}`);
		}
		else if (ParserMaterial.customNodes.get(node.type) != null) {
			return ParserMaterial.customNodes.get(node.type)(node, socket);
		}
		return "vec3(0.0, 0.0, 0.0)";
	}

	static parse_normal_map_color_input = (inp: zui_node_socket_t) => {
		ParserMaterial.frag.write_normal++;
		ParserMaterial.out_normaltan = ParserMaterial.parse_vector_input(inp);
		if (!ParserMaterial.arm_export_tangents) {
			NodeShader.write(ParserMaterial.frag, `vec3 texn = (${ParserMaterial.out_normaltan}) * 2.0 - 1.0;`);
			NodeShader.write(ParserMaterial.frag, `texn.y = -texn.y;`);
			if (!ParserMaterial.cotangentFrameWritten) {
				ParserMaterial.cotangentFrameWritten = true;
				NodeShader.add_function(ParserMaterial.frag, ShaderFunctions.str_cotangentFrame);
			}
			ParserMaterial.frag.n = true;
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(ParserMaterial.frag, `mat3 TBN = cotangentFrame(n, vVec, texCoord);`);
			///else
			NodeShader.write(ParserMaterial.frag, `mat3 TBN = cotangentFrame(n, -vVec, texCoord);`);
			///end

			NodeShader.write(ParserMaterial.frag, `n = mul(normalize(texn), TBN);`);
		}
		ParserMaterial.frag.write_normal--;
	}

	static parse_value_input = (inp: zui_node_socket_t, vector_as_grayscale = false) : string => {
		let l = ParserMaterial.getInputLink(inp);
		let from_node = l != null ? ParserMaterial.getNode(l.from_id) : null;
		if (from_node != null) {
			if (from_node.type == "REROUTE") {
				return ParserMaterial.parse_value_input(from_node.inputs[0]);
			}

			let res_var = ParserMaterial.write_result(l);
			let st = from_node.outputs[l.from_socket].type;
			if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
				if (vector_as_grayscale) {
					return `dot(${res_var}.rbg, vec3(0.299, 0.587, 0.114))`;
				}
				else {
					return `${res_var}.x`;
				}
			}
			else { // VALUE
				return res_var;
			}
		}
		else {
			return ParserMaterial.vec1(inp.default_value);
		}
	}

	static parse_value = (node: zui_node_t, socket: zui_node_socket_t): string => {
		if (node.type == `GROUP`) {
			return ParserMaterial.parse_group(node, socket);
		}
		else if (node.type == `GROUP_INPUT`) {
			return ParserMaterial.parse_group_input(node, socket);
		}
		else if (node.type == "ATTRIBUTE") {
			NodeShader.add_uniform(ParserMaterial.curshader, "float time", "_time");
			return "time";
		}
		else if (node.type == "VERTEX_COLOR") {
			return "1.0";
		}
		else if (node.type == "WIREFRAME") {
			NodeShader.add_uniform(ParserMaterial.curshader, `sampler2D texuvmap`, `_texuvmap`);
			let use_pixel_size = node.buttons[0].default_value == "true";
			let pixel_size = ParserMaterial.parse_value_input(node.inputs[0]);
			return "textureLod(texuvmap, texCoord, 0.0).r";
		}
		else if (node.type == "CAMERA") {
			if (socket == node.outputs[1]) { // View Z Depth
				NodeShader.add_uniform(ParserMaterial.curshader, "vec2 cameraProj", "_camera_plane_proj");
				///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
				ParserMaterial.curshader.wvpposition = true;
				return "(cameraProj.y / ((wvpposition.z / wvpposition.w) - cameraProj.x))";
				///else
				return "(cameraProj.y / (gl_FragCoord.z - cameraProj.x))";
				///end
			}
			else { // View Distance
				NodeShader.add_uniform(ParserMaterial.curshader, "vec3 eye", "_camera_pos");
				ParserMaterial.curshader.wposition = true;
				return "distance(eye, wposition)";
			}
		}
		else if (node.type == "LAYER") {
			let l = node.buttons[0].default_value;
			if (socket == node.outputs[1]) { // Opac
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint" + l, "_texpaint" + l);
				return "texture(texpaint" + l + ", texCoord).a";
			}
			else if (socket == node.outputs[2]) { // Occ
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).r";
			}
			else if (socket == node.outputs[3]) { // Rough
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).g";
			}
			else if (socket == node.outputs[4]) { // Metal
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).b";
			}
			else if (socket == node.outputs[7]) { // Height
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).a";
			}
		}
		else if (node.type == "LAYER_MASK") {
			if (socket == node.outputs[0]) {
				let l = node.buttons[0].default_value;
				NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D texpaint" + l, "_texpaint" + l);
				return "texture(texpaint" + l + ", texCoord).r";
			}
		}
		else if (node.type == "MATERIAL") {
			let result = "0.0";
			let mi = node.buttons[0].default_value;
			if (mi >= Project.materials.length) return result;
			let m = Project.materials[mi];
			let _nodes = ParserMaterial.nodes;
			let _links = ParserMaterial.links;
			ParserMaterial.nodes = m.canvas.nodes;
			ParserMaterial.links = m.canvas.links;
			ParserMaterial.parents.push(node);
			let output_node = ParserMaterial.node_by_type(ParserMaterial.nodes, "OUTPUT_MATERIAL_PBR");
			if (socket == node.outputs[1]) { // Opac
				result = ParserMaterial.parse_value_input(output_node.inputs[1]);
			}
			else if (socket == node.outputs[2]) { // Occ
				result = ParserMaterial.parse_value_input(output_node.inputs[2]);
			}
			else if (socket == node.outputs[3]) { // Rough
				result = ParserMaterial.parse_value_input(output_node.inputs[3]);
			}
			else if (socket == node.outputs[4]) { // Metal
				result = ParserMaterial.parse_value_input(output_node.inputs[4]);
			}
			else if (socket == node.outputs[7]) { // Height
				result = ParserMaterial.parse_value_input(output_node.inputs[7]);
			}
			ParserMaterial.nodes = _nodes;
			ParserMaterial.links = _links;
			ParserMaterial.parents.pop();
			return result;
		}
		else if (node.type == "PICKER") {
			if (socket == node.outputs[1]) {
				NodeShader.add_uniform(ParserMaterial.curshader, "float pickerOpacity", "_pickerOpacity");
				return "pickerOpacity";
			}
			else if (socket == node.outputs[2]) {
				NodeShader.add_uniform(ParserMaterial.curshader, "float pickerOcclusion", "_pickerOcclusion");
				return "pickerOcclusion";
			}
			else if (socket == node.outputs[3]) {
				NodeShader.add_uniform(ParserMaterial.curshader, "float pickerRoughness", "_pickerRoughness");
				return "pickerRoughness";
			}
			else if (socket == node.outputs[4]) {
				NodeShader.add_uniform(ParserMaterial.curshader, "float pickerMetallic", "_pickerMetallic");
				return "pickerMetallic";
			}
			else if (socket == node.outputs[7]) {
				NodeShader.add_uniform(ParserMaterial.curshader, "float pickerHeight", "_pickerHeight");
				return "pickerHeight";
			}
		}
		else if (node.type == "FRESNEL") {
			let ior = ParserMaterial.parse_value_input(node.inputs[0]);
			ParserMaterial.curshader.dotNV = true;
			return `pow(1.0 - dotNV, 7.25 / ${ior})`;
		}
		else if (node.type == "NEW_GEOMETRY") {
			if (socket == node.outputs[6]) { // Backfacing
				///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
				return "0.0"; // SV_IsFrontFace
				///else
				return "(1.0 - float(gl_FrontFacing))";
				///end
			}
			else if (socket == node.outputs[7]) { // Pointiness
				let strength = 1.0;
				let radius = 1.0;
				let offset = 0.0;
				let store = ParserMaterial.store_var_name(node);
				ParserMaterial.curshader.n = true;
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_dx = dFdx(n);`);
				NodeShader.write(ParserMaterial.curshader, `vec3 ${store}_dy = dFdy(n);`);
				NodeShader.write(ParserMaterial.curshader, `float ${store}_curvature = max(dot(${store}_dx, ${store}_dx), dot(${store}_dy, ${store}_dy));`);
				NodeShader.write(ParserMaterial.curshader, `${store}_curvature = clamp(pow(${store}_curvature, (1.0 / ` + radius + `) * 0.25) * ` + strength + ` * 2.0 + ` + offset + ` / 10.0, 0.0, 1.0);`);
				return `${store}_curvature`;
			}
			else if (socket == node.outputs[8]) { // Random Per Island
				return "0.0";
			}
		}
		else if (node.type == "HAIR_INFO") {
			return "0.5";
		}
		else if (node.type == "LAYER_WEIGHT") {
			let blend = ParserMaterial.parse_value_input(node.inputs[0]);
			if (socket == node.outputs[0]) { // Fresnel
				ParserMaterial.curshader.dotNV = true;
				return `clamp(pow(1.0 - dotNV, (1.0 - ${blend}) * 10.0), 0.0, 1.0)`;
			}
			else if (socket == node.outputs[1]) { // Facing
				ParserMaterial.curshader.dotNV = true;
				return `((1.0 - dotNV) * ${blend})`;
			}
		}
		else if (node.type == "OBJECT_INFO") {
			if (socket == node.outputs[1]) { // Object Index
				NodeShader.add_uniform(ParserMaterial.curshader, "float objectInfoIndex", "_object_info_index");
				return "objectInfoIndex";
			}
			else if (socket == node.outputs[2]) { // Material Index
				NodeShader.add_uniform(ParserMaterial.curshader, "float objectInfoMaterialIndex", "_object_info_material_index");
				return "objectInfoMaterialIndex";
			}
			else if (socket == node.outputs[3]) { // Random
				NodeShader.add_uniform(ParserMaterial.curshader, "float objectInfoRandom", "_object_info_random");
				return "objectInfoRandom";
			}
		}
		else if (node.type == "VALUE") {
			return ParserMaterial.vec1(node.outputs[0].default_value);
		}
		else if (node.type == "TEX_BRICK") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_brick);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[4]);
			let res = `tex_brick_f(${co} * ${scale})`;
			return res;
		}
		else if (node.type == "TEX_CHECKER") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_checker);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[3]);
			let res = `tex_checker_f(${co}, ${scale})`;
			return res;
		}
		else if (node.type == "TEX_GRADIENT") {
			let co = ParserMaterial.getCoord(node);
			let but = node.buttons[0]; //gradient_type;
			let grad: string = but.data[but.default_value].toUpperCase();
			grad = grad.replaceAll(" ", "_");
			let f = ParserMaterial.getGradient(grad, co);
			let res = `(clamp(${f}, 0.0, 1.0))`;
			return res;
		}
		else if (node.type == "TEX_IMAGE") {
			// Already fetched
			if (ParserMaterial.parsed.indexOf(ParserMaterial.res_var_name(node, node.outputs[0])) >= 0) { // TODO: node.outputs[1]
				let varname = ParserMaterial.store_var_name(node);
				return `${varname}.a`;
			}
			let tex_name = ParserMaterial.node_name(node);
			let tex = ParserMaterial.make_texture(node, tex_name);
			if (tex != null) {
				let color_space = node.buttons[1].default_value;
				let texstore = ParserMaterial.texture_store(node, tex, tex_name, color_space);
				return `${texstore}.a`;
			}
		}
		else if (node.type == "TEX_MAGIC") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_magic);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let res = `tex_magic_f(${co} * ${scale} * 4.0)`;
			return res;
		}
		else if (node.type == "TEX_MUSGRAVE") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_musgrave);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let res = `tex_musgrave_f(${co} * ${scale} * 0.5)`;
			return res;
		}
		else if (node.type == "TEX_NOISE") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_noise);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let res = `tex_noise(${co} * ${scale})`;
			return res;
		}
		else if (node.type == "TEX_VORONOI") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_voronoi);
			NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D snoise256", "$noise256.k");
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let but = node.buttons[0]; // coloring
			let coloring: string = but.data[but.default_value].toUpperCase();
			coloring = coloring.replaceAll(" ", "_");
			let res = "";
			if (coloring == "INTENSITY") {
				res = `tex_voronoi(${co} * ${scale}, texturePass(snoise256)).a`;
			}
			else { // Cells
				res = `tex_voronoi(${co} * ${scale}, texturePass(snoise256)).r`;
			}
			return res;
		}
		else if (node.type == "TEX_WAVE") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_tex_wave);
			let co = ParserMaterial.getCoord(node);
			let scale = ParserMaterial.parse_value_input(node.inputs[1]);
			let res = `tex_wave_f(${co} * ${scale})`;
			return res;
		}
		else if (node.type == "BAKE_CURVATURE") {
			if (ParserMaterial.bake_passthrough) {
				ParserMaterial.bake_passthrough_strength = ParserMaterial.parse_value_input(node.inputs[0]);
				ParserMaterial.bake_passthrough_radius = ParserMaterial.parse_value_input(node.inputs[1]);
				ParserMaterial.bake_passthrough_offset = ParserMaterial.parse_value_input(node.inputs[2]);
				return "0.0";
			}
			let tex_name = "texbake_" + ParserMaterial.node_name(node);
			NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D " + tex_name, "_" + tex_name);
			let store = ParserMaterial.store_var_name(node);
			NodeShader.write(ParserMaterial.curshader, `float ${store}_res = texture(${tex_name}, texCoord).r;`);
			return `${store}_res`;
		}
		else if (node.type == "NORMAL") {
			let nor = ParserMaterial.parse_vector_input(node.inputs[0]);
			let norout = ParserMaterial.vec3(node.outputs[0].default_value);
			return `dot(${norout}, ${nor})`;
		}
		else if (node.type == "COLMASK") {
			let inputColor = ParserMaterial.parse_vector_input(node.inputs[0]);
			let maskColor = ParserMaterial.parse_vector_input(node.inputs[1]);
			let radius = ParserMaterial.parse_value_input(node.inputs[2]);
			let fuzziness = ParserMaterial.parse_value_input(node.inputs[3]);
			return `clamp(1.0 - (distance(${inputColor}, ${maskColor}) - ${radius}) / max(${fuzziness}, ${ParserMaterial.eps}), 0.0, 1.0)`;
		}
		else if (node.type == "MATH") {
			let val1 = ParserMaterial.parse_value_input(node.inputs[0]);
			let val2 = ParserMaterial.parse_value_input(node.inputs[1]);
			let but = node.buttons[0]; // operation
			let op: string = but.data[but.default_value].toUpperCase();
			op = op.replaceAll(" ", "_");
			let use_clamp = node.buttons[1].default_value == true;
			let out_val = "";
			if (op == "ADD") {
				out_val = `(${val1} + ${val2})`;
			}
			else if (op == "SUBTRACT") {
				out_val = `(${val1} - ${val2})`;
			}
			else if (op == "MULTIPLY") {
				out_val = `(${val1} * ${val2})`;
			}
			else if (op == "DIVIDE") {
				val2 = `(${val2} == 0.0 ? ${ParserMaterial.eps} : ${val2})`;
				out_val = `(${val1} / ${val2})`;
			}
			else if (op == "POWER") {
				out_val = `pow(${val1}, ${val2})`;
			}
			else if (op == "LOGARITHM") {
				out_val = `log(${val1})`;
			}
			else if (op == "SQUARE_ROOT") {
				out_val = `sqrt(${val1})`;
			}
			else if(op == "INVERSE_SQUARE_ROOT") {
				out_val = `inversesqrt(${val1})`;
			}
			else if (op == "EXPONENT") {
				out_val = `exp(${val1})`;
			}
			else if (op == "ABSOLUTE") {
				out_val = `abs(${val1})`;
			}
			else if (op == "MINIMUM") {
				out_val = `min(${val1}, ${val2})`;
			}
			else if (op == "MAXIMUM") {
				out_val = `max(${val1}, ${val2})`;
			}
			else if (op == "LESS_THAN") {
				out_val = `float(${val1} < ${val2})`;
			}
			else if (op == "GREATER_THAN") {
				out_val = `float(${val1} > ${val2})`;
			}
			else if (op == "SIGN") {
				out_val = `sign(${val1})`;
			}
			else if (op == "ROUND") {
				out_val = `floor(${val1} + 0.5)`;
			}
			else if (op == "FLOOR") {
				out_val = `floor(${val1})`;
			}
			else if (op == "CEIL") {
				out_val = `ceil(${val1})`;
			}
			else if(op == "SNAP") {
				out_val = `(floor(${val1} / ${val2}) * ${val2})`;
			}
			else if (op == "TRUNCATE") {
				out_val = `trunc(${val1})`;
			}
			else if (op == "FRACTION") {
				out_val = `fract(${val1})`;
			}
			else if (op == "MODULO") {
				out_val = `mod(${val1}, ${val2})`;
			}
			else if (op == "PING-PONG") {
				out_val = `((${val2} != 0.0) ? abs(fract((${val1} - ${val2}) / (${val2} * 2.0)) * ${val2} * 2.0 - ${val2}) : 0.0)`;
			}
			else if (op == "SINE") {
				out_val = `sin(${val1})`;
			}
			else if (op == "COSINE") {
				out_val = `cos(${val1})`;
			}
			else if (op == "TANGENT") {
				out_val = `tan(${val1})`;
			}
			else if (op == "ARCSINE") {
				out_val = `asin(${val1})`;
			}
			else if (op == "ARCCOSINE") {
				out_val = `acos(${val1})`;
			}
			else if (op == "ARCTANGENT") {
				out_val = `atan(${val1})`;
			}
			else if (op == "ARCTAN2") {
				out_val = `atan2(${val1}, ${val2})`;
			}
			else if (op == "HYPERBOLIC_SINE") {
				out_val = `sinh(${val1})`;
			}
			else if (op == "HYPERBOLIC_COSINE") {
				out_val = `cosh(${val1})`;
			}
			else if (op == "HYPERBOLIC_TANGENT") {
				out_val = `tanh(${val1})`;
			}
			else if (op == "TO_RADIANS") {
				out_val = `radians(${val1})`;
			}
			else if (op == "TO_DEGREES") {
				out_val = `degrees(${val1})`;
			}
			if (use_clamp) {
				return `clamp(${out_val}, 0.0, 1.0)`;
			}
			else {
				return out_val;
			}
		}
		else if (node.type == "SCRIPT_CPU") {
			if (ParserMaterial.script_links == null) ParserMaterial.script_links = new Map();
			let script = node.buttons[0].default_value;
			let link = ParserMaterial.node_name(node);
			ParserMaterial.script_links.set(link, script);
			NodeShader.add_uniform(ParserMaterial.curshader, "float " + link, "_" + link);
			return link;
		}
		else if (node.type == "SHADER_GPU") {
			let shader = node.buttons[0].default_value;
			return shader == "" ? "0.0" : shader;
		}
		else if (node.type == "RGBTOBW") {
			let col = ParserMaterial.parse_vector_input(node.inputs[0]);
			return `(((${col}.r * 0.3 + ${col}.g * 0.59 + ${col}.b * 0.11) / 3.0) * 2.5)`;
		}
		else if (node.type == "SEPHSV") {
			NodeShader.add_function(ParserMaterial.curshader, ShaderFunctions.str_hue_sat);
			let col = ParserMaterial.parse_vector_input(node.inputs[0]);
			if (socket == node.outputs[0]) {
				return `rgb_to_hsv(${col}).r`;
			}
			else if (socket == node.outputs[1]) {
				return `rgb_to_hsv(${col}).g`;
			}
			else if (socket == node.outputs[2]) {
				return `rgb_to_hsv(${col}).b`;
			}
		}
		else if (node.type == "SEPRGB") {
			let col = ParserMaterial.parse_vector_input(node.inputs[0]);
			if (socket == node.outputs[0]) {
				return `${col}.r`;
			}
			else if (socket == node.outputs[1]) {
				return `${col}.g`;
			}
			else if (socket == node.outputs[2]) {
				return `${col}.b`;
			}
		}
		else if (node.type == "SEPXYZ") {
			let vec = ParserMaterial.parse_vector_input(node.inputs[0]);
			if (socket == node.outputs[0]) {
				return `${vec}.x`;
			}
			else if (socket == node.outputs[1]) {
				return `${vec}.y`;
			}
			else if (socket == node.outputs[2]) {
				return `${vec}.z`;
			}
		}
		else if (node.type == "VECT_MATH") {
			let vec1 = ParserMaterial.parse_vector_input(node.inputs[0]);
			let vec2 = ParserMaterial.parse_vector_input(node.inputs[1]);
			let but = node.buttons[0]; //operation;
			let op: string = but.data[but.default_value].toUpperCase();
			op = op.replaceAll(" ", "_");
			if (op == "DOT_PRODUCT") {
				return `dot(${vec1}, ${vec2})`;
			}
			else if (op == "LENGTH") {
				return `length(${vec1})`;
			}
			else if (op == "DISTANCE") {
				return `distance(${vec1}, ${vec2})`;
			}
			else {
				return "0.0";
			}
		}
		else if (node.type == "CLAMP") {
			let val = ParserMaterial.parse_value_input(node.inputs[0]);
			let min = ParserMaterial.parse_value_input(node.inputs[1]);
			let max = ParserMaterial.parse_value_input(node.inputs[2]);
			let but = node.buttons[0]; //operation;
			let op: string = but.data[but.default_value].toUpperCase();
			op = op.replaceAll(" ", "_");

			if (op == "MIN_MAX") {
				return `(clamp(${val}, ${min}, ${max}))`;
			}
			else if (op == "RANGE") {
				return `(clamp(${val}, min(${min}, ${max}), max(${min}, ${max})))`;
			}
		}
		else if (node.type == "MAPRANGE") {
			let val = ParserMaterial.parse_value_input(node.inputs[0]);
			let fmin = ParserMaterial.parse_value_input(node.inputs[1]);
			let fmax = ParserMaterial.parse_value_input(node.inputs[2]);
			let tmin = ParserMaterial.parse_value_input(node.inputs[3]);
			let tmax = ParserMaterial.parse_value_input(node.inputs[4]);

			let use_clamp = node.buttons[0].default_value == true;

			let a = `((${tmin} - ${tmax}) / (${fmin} - ${fmax}))`;
			let out_val = `(${a} * ${val} + ${tmin} - ${a} * ${fmin})`;
			if (use_clamp) {
				return `(clamp(${out_val}, ${tmin}, ${tmax}))`;
			}
			else {
				return out_val;
			}
		}
		else if (ParserMaterial.customNodes.get(node.type) != null) {
			return ParserMaterial.customNodes.get(node.type)(node, socket);
		}
		return "0.0";
	}

	static getCoord = (node: zui_node_t): string => {
		if (ParserMaterial.getInputLink(node.inputs[0]) != null) {
			return ParserMaterial.parse_vector_input(node.inputs[0]);
		}
		else {
			ParserMaterial.curshader.bposition = true;
			return "bposition";
		}
	}

	static getGradient = (grad: string, co: string): string => {
		if (grad == "LINEAR") {
			return `${co}.x`;
		}
		else if (grad == "QUADRATIC") {
			return "0.0";
		}
		else if (grad == "EASING") {
			return "0.0";
		}
		else if (grad == "DIAGONAL") {
			return `(${co}.x + ${co}.y) * 0.5`;
		}
		else if (grad == "RADIAL") {
			return `atan2(${co}.x, ${co}.y) / (3.141592 * 2.0) + 0.5`;
		}
		else if (grad == "QUADRATIC_SPHERE") {
			return "0.0";
		}
		else { // "SPHERICAL"
			return `max(1.0 - sqrt(${co}.x * ${co}.x + ${co}.y * ${co}.y + ${co}.z * ${co}.z), 0.0)`;
		}
	}

	static vector_curve = (name: string, fac: string, points: Float32Array[]): string => {
		// Write Ys array
		let ys_var = name + "_ys";
		let num = points.length;
		NodeShader.write(ParserMaterial.curshader, `float ${ys_var}[${num}];`); // TODO: Make const
		for (let i = 0; i < num; ++i) {
			NodeShader.write(ParserMaterial.curshader, `${ys_var}[${i}] = ${points[i][1]};`);
		}
		// Get index
		let fac_var = name + "_fac";
		NodeShader.write(ParserMaterial.curshader, `float ${fac_var} = ${fac};`);
		let index = "0";
		for (let i = 1; i < num; ++i) {
			index += ` + (${fac_var} > ${points[i][0]} ? 1 : 0)`;
		}
		// Write index
		let index_var = name + "_i";
		NodeShader.write(ParserMaterial.curshader, `int ${index_var} = ${index};`);
		// Linear
		// Write Xs array
		let facs_var = name + "_xs";
		NodeShader.write(ParserMaterial.curshader, `float ${facs_var}[${num}];`); // TODO: Make const
		for (let i = 0; i < num; ++i) {
			NodeShader.write(ParserMaterial.curshader, `${facs_var}[${i}] = ${points[i][0]};`);
		}
		// Map vector
		return `mix(${ys_var}[${index_var}], ${ys_var}[${index_var} + 1], (${fac_var} - ${facs_var}[${index_var}]) * (1.0 / (${facs_var}[${index_var} + 1] - ${facs_var}[${index_var}])))`;
	}

	static res_var_name = (node: zui_node_t, socket: zui_node_socket_t): string => {
		return ParserMaterial.node_name(node) + "_" + ParserMaterial.safesrc(socket.name) + "_res";
	}

	static parsedMap = new Map<string, string>();
	static textureMap = new Map<string, string>();

	static write_result = (l: zui_node_link_t): string => {
		let from_node = ParserMaterial.getNode(l.from_id);
		let from_socket = from_node.outputs[l.from_socket];
		let res_var = ParserMaterial.res_var_name(from_node, from_socket);
		let st = from_socket.type;
		if (ParserMaterial.parsed.indexOf(res_var) < 0) {
			ParserMaterial.parsed.push(res_var);
			if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
				let res = ParserMaterial.parse_vector(from_node, from_socket);
				if (res == null) {
					return null;
				}
				ParserMaterial.parsedMap.set(res_var, res);
				NodeShader.write(ParserMaterial.curshader, `vec3 ${res_var} = ${res};`);
			}
			else if (st == "VALUE") {
				let res = ParserMaterial.parse_value(from_node, from_socket);
				if (res == null) {
					return null;
				}
				ParserMaterial.parsedMap.set(res_var, res);
				NodeShader.write(ParserMaterial.curshader, `float ${res_var} = ${res};`);
			}
		}
		return res_var;
	}

	static store_var_name = (node: zui_node_t): string => {
		return ParserMaterial.node_name(node) + "_store";
	}

	static texture_store = (node: zui_node_t, tex: bind_tex_t, tex_name: string, color_space: i32): string => {
		ParserMaterial.matcon.bind_textures.push(tex);
		NodeShaderContext.add_elem(ParserMaterial.curshader.context, "tex", "short2norm");
		NodeShader.add_uniform(ParserMaterial.curshader, "sampler2D " + tex_name);
		let uv_name = "";
		if (ParserMaterial.getInputLink(node.inputs[0]) != null) {
			uv_name = ParserMaterial.parse_vector_input(node.inputs[0]);
		}
		else {
			uv_name = ParserMaterial.tex_coord;
		}
		let tex_store = ParserMaterial.store_var_name(node);

		if (ParserMaterial.sample_keep_aspect) {
			NodeShader.write(ParserMaterial.curshader, `vec2 ${tex_store}_size = vec2(textureSize(${tex_name}, 0));`);
			NodeShader.write(ParserMaterial.curshader, `float ${tex_store}_ax = ${tex_store}_size.x / ${tex_store}_size.y;`);
			NodeShader.write(ParserMaterial.curshader, `float ${tex_store}_ay = ${tex_store}_size.y / ${tex_store}_size.x;`);
			NodeShader.write(ParserMaterial.curshader, `vec2 ${tex_store}_uv = ((${uv_name}.xy / ${ParserMaterial.sample_uv_scale} - vec2(0.5, 0.5)) * vec2(max(${tex_store}_ay, 1.0), max(${tex_store}_ax, 1.0))) + vec2(0.5, 0.5);`);
			NodeShader.write(ParserMaterial.curshader, `if (${tex_store}_uv.x < 0.0 || ${tex_store}_uv.y < 0.0 || ${tex_store}_uv.x > 1.0 || ${tex_store}_uv.y > 1.0) discard;`);
			NodeShader.write(ParserMaterial.curshader, `${tex_store}_uv *= ${ParserMaterial.sample_uv_scale};`);
			uv_name = `${tex_store}_uv`;
		}

		if (ParserMaterial.triplanar) {
			NodeShader.write(ParserMaterial.curshader, `vec4 ${tex_store} = vec4(0.0, 0.0, 0.0, 0.0);`);
			NodeShader.write(ParserMaterial.curshader, `if (texCoordBlend.x > 0) ${tex_store} += texture(${tex_name}, ${uv_name}.xy) * texCoordBlend.x;`);
			NodeShader.write(ParserMaterial.curshader, `if (texCoordBlend.y > 0) ${tex_store} += texture(${tex_name}, ${uv_name}1.xy) * texCoordBlend.y;`);
			NodeShader.write(ParserMaterial.curshader, `if (texCoordBlend.z > 0) ${tex_store} += texture(${tex_name}, ${uv_name}2.xy) * texCoordBlend.z;`);
		}
		else {
			if (ParserMaterial.curshader == ParserMaterial.frag) {
				ParserMaterial.textureMap.set(tex_store, `texture(${tex_name}, ${uv_name}.xy)`);
				NodeShader.write(ParserMaterial.curshader, `vec4 ${tex_store} = texture(${tex_name}, ${uv_name}.xy);`);
			}
			else {
				ParserMaterial.textureMap.set(tex_store, `textureLod(${tex_name}, ${uv_name}.xy, 0.0)`);
				NodeShader.write(ParserMaterial.curshader, `vec4 ${tex_store} = textureLod(${tex_name}, ${uv_name}.xy, 0.0);`);
			}
			if (!tex.file.endsWith(".jpg")) { // Pre-mult alpha
				NodeShader.write(ParserMaterial.curshader, `${tex_store}.rgb *= ${tex_store}.a;`);
			}
		}

		if (ParserMaterial.transform_color_space) {
			// Base color socket auto-converts from sRGB to linear
			if (color_space == ColorSpace.SpaceLinear && ParserMaterial.parsing_basecolor) { // Linear to sRGB
				NodeShader.write(ParserMaterial.curshader, `${tex_store}.rgb = pow(${tex_store}.rgb, vec3(2.2, 2.2, 2.2));`);
			}
			else if (color_space == ColorSpace.SpaceSRGB && !ParserMaterial.parsing_basecolor) { // sRGB to linear
				NodeShader.write(ParserMaterial.curshader, `${tex_store}.rgb = pow(${tex_store}.rgb, vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));`);
			}
			else if (color_space == ColorSpace.SpaceDirectXNormalMap) { // DirectX normal map to OpenGL normal map
				NodeShader.write(ParserMaterial.curshader, `${tex_store}.y = 1.0 - ${tex_store}.y;`);
			}
		}
		return tex_store;
	}

	static vec1 = (v: f32): string => {
		///if krom_android
		return `float(${v})`;
		///else
		return `${v}`;
		///end
	}

	static vec3 = (v: f32[]): string => {
		///if krom_android
		return `vec3(float(${v[0]}), float(${v[1]}), float(${v[2]}))`;
		///else
		return `vec3(${v[0]}, ${v[1]}, ${v[2]})`;
		///end
	}

	static to_vec3 = (s: string): string => {
		///if (krom_direct3d11 || krom_direct3d12)
		return `(${s}).xxx`;
		///else
		return `vec3(${s})`;
		///end
	}

	static node_by_type = (nodes: zui_node_t[], ntype: string): zui_node_t => {
		for (let n of nodes) if (n.type == ntype) return n;
		return null;
	}

	static socket_index = (node: zui_node_t, socket: zui_node_socket_t): i32 => {
		for (let i = 0; i < node.outputs.length; ++i) if (node.outputs[i] == socket) return i;
		return -1;
	}

	static node_name = (node: zui_node_t, _parents: zui_node_t[] = null): string => {
		if (_parents == null) _parents = ParserMaterial.parents;
		let s = node.name;
		for (let p of _parents) s = p.name + p.id + `_` + s;
		s = ParserMaterial.safesrc(s) + node.id;
		return s;
	}

	static safesrc = (s: string): string => {
		for (let i = 0; i < s.length; ++i) {
			let code = s.charCodeAt(i);
			let letter = (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
			let digit = code >= 48 && code <= 57;
			if (!letter && !digit) s = s.replaceAll(s.charAt(i), "_");
			if (i == 0 && digit) s = "_" + s;
		}
		///if krom_opengl
		while (s.indexOf("__") >= 0) s = s.replaceAll("__", "_");
		///end
		return s;
	}

	static enumData = (s: string): string => {
		for (let a of Project.assets) if (a.name == s) return a.file;
		return "";
	}

	static make_texture = (image_node: zui_node_t, tex_name: string, matname: string = null): bind_tex_t => {

		let filepath = ParserMaterial.enumData(Base.enumTexts(image_node.type)[image_node.buttons[0].default_value]);
		if (filepath == "" || filepath.indexOf(".") == -1) {
			return null;
		}

		let tex: bind_tex_t = {
			name: tex_name,
			file: filepath
		};

		if (Context.raw.textureFilter) {
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

	static is_pow = (num: i32): bool => {
		return ((num & (num - 1)) == 0) && num != 0;
	}

	static asset_path = (s: string): string => {
		return s;
	}

	static extract_filename = (s: string): string => {
		let ar = s.split(".");
		return ar[ar.length - 2] + "." + ar[ar.length - 1];
	}

	static safestr = (s: string): string => {
		return s;
	}
}

type TShaderOut = {
	out_basecol: string;
	out_roughness: string;
	out_metallic: string;
	out_occlusion: string;
	out_opacity: string;
	out_height: string;
	out_emission: string;
	out_subsurface: string;
}

enum ColorSpace {
	SpaceAuto, // sRGB for base color, otherwise linear
	SpaceLinear,
	SpaceSRGB,
	SpaceDirectXNormalMap,
}
