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
package arm.shader;

import zui.Nodes;
import iron.data.SceneFormat;
import arm.shader.NodeShader;

class MaterialParser {

	static var con: NodeShaderContext;
	static var vert: NodeShader;
	static var frag: NodeShader;
	static var curshader: NodeShader;
	static var matcon: TMaterialContext;
	static var parsed: Array<String>;
	static var parents: Array<TNode>;

	static var canvases: Array<TNodeCanvas>;
	static var nodes: Array<TNode>;
	static var links: Array<TNodeLink>;

	static var cotangentFrameWritten: Bool;
	static var tex_coord = "texCoord";
	static inline var eps = 0.000001;

	public static var customNodes = js.Syntax.code("new Map()");
	public static var parse_surface = true;
	public static var parse_opacity = true;
	public static var parse_height = false;
	public static var parse_height_as_channel = false;
	public static var parse_emission = false;
	public static var parse_subsurface = false;
	public static var parsing_basecolor = false;
	public static var triplanar = false; // Sample using texCoord/1/2 & texCoordBlend
	public static var sample_keep_aspect = false; // Adjust uvs to preserve texture aspect ratio
	public static var sample_uv_scale = '1.0';
	public static var transform_color_space = true;

	public static var blur_passthrough = false;
	public static var warp_passthrough = false;
	public static var bake_passthrough = false;
	public static var bake_passthrough_strength = "0.0";
	public static var bake_passthrough_radius = "0.0";
	public static var bake_passthrough_offset = "0.0";
	public static var start_group: TNodeCanvas = null;
	public static var start_parents: Array<TNode> = null;
	public static var start_node: TNode = null;

	public static var arm_export_tangents = true;
	public static var out_normaltan: String; // Raw tangent space normal parsed from normal map

	public static var script_links: Map<String, String> = null;

	public static function getNode(id: Int): TNode {
		for (n in nodes) if (n.id == id) return n;
		return null;
	}

	public static function getLink(id: Int): TNodeLink {
		for (l in links) if (l.id == id) return l;
		return null;
	}

	public static function getInputLink(inp: TNodeSocket): TNodeLink {
		for (l in links) {
			if (l.to_id == inp.node_id) {
				var node = getNode(inp.node_id);
				if (node.inputs.length <= l.to_socket) return null;
				if (node.inputs[l.to_socket] == inp) return l;
			}
		}
		return null;
	}

	public static function getOutputLinks(out: TNodeSocket): Array<TNodeLink> {
		var ls: Array<TNodeLink> = null;
		for (l in links) {
			if (l.from_id == out.node_id) {
				var node = getNode(out.node_id);
				if (node.outputs.length <= l.from_socket) continue;
				if (node.outputs[l.from_socket] == out) {
					if (ls == null) ls = [];
					ls.push(l);
				}
			}
		}
		return ls;
	}

	static function init() {
		parsed = [];
		parents = [];
		cotangentFrameWritten = false;
		out_normaltan = "vec3(0.5, 0.5, 1.0)";
		script_links = null;
		parsing_basecolor = false;
	}

	public static function parse(canvas: TNodeCanvas, _con: NodeShaderContext, _vert: NodeShader, _frag: NodeShader, _matcon: TMaterialContext): TShaderOut {
		init();
		canvases = [canvas];
		nodes = canvas.nodes;
		links = canvas.links;
		con = _con;
		vert = _vert;
		frag = _frag;
		curshader = frag;
		matcon = _matcon;

		if (start_group != null) {
			push_group(start_group);
			parents = start_parents;
		}

		if (start_node != null) {
			var link: TNodeLink = { id: 99999, from_id: start_node.id, from_socket: 0, to_id: -1, to_socket: -1 };
			write_result(link);
			return {
				out_basecol: 'vec3(0.0, 0.0, 0.0)',
				out_roughness: '0.0',
				out_metallic: '0.0',
				out_occlusion: '1.0',
				out_opacity: '1.0',
				out_height: '0.0',
				out_emission: '0.0',
				out_subsurface: '0.0'
			}
		}

		var output_node = node_by_type(nodes, "OUTPUT_MATERIAL");
		if (output_node != null) {
			return parse_output(output_node);
		}
		output_node = node_by_type(nodes, "OUTPUT_MATERIAL_PBR");
		if (output_node != null) {
			return parse_output_pbr(output_node);
		}
		return null;
	}

	public static function finalize(con: NodeShaderContext) {
		var vert = con.vert;
		var frag = con.frag;

		if (frag.dotNV) {
			frag.vVec = true;
			frag.n = true;
		}
		if (frag.vVec) {
			vert.wposition = true;
		}

		if (frag.bposition) {
			if (triplanar) {
				frag.write_attrib('vec3 bposition = vec3(
					texCoord1.x * texCoordBlend.y + texCoord2.x * texCoordBlend.z,
					texCoord.x * texCoordBlend.x + texCoord2.y * texCoordBlend.z,
					texCoord.y * texCoordBlend.x + texCoord1.y * texCoordBlend.y);');
			}
			else if (frag.ndcpos) {
				vert.add_out("vec3 bposition");
				vert.write('bposition = (ndc.xyz / ndc.w);');
			}
			else {
				vert.add_out("vec3 bposition");
				vert.add_uniform("vec3 dim", "_dim");
				vert.add_uniform("vec3 hdim", "_halfDim");
				vert.write_attrib('bposition = (pos.xyz + hdim) / dim;');
			}
		}
		if (frag.wposition) {
			vert.add_uniform("mat4 W", "_worldMatrix");
			vert.add_out("vec3 wposition");
			vert.write_attrib('wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;');
		}
		else if (vert.wposition) {
			vert.add_uniform("mat4 W", "_worldMatrix");
			vert.write_attrib('vec3 wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;');
		}
		if (frag.vposition) {
			vert.add_uniform("mat4 WV", "_worldViewMatrix");
			vert.add_out("vec3 vposition");
			vert.write_attrib('vposition = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz;');
		}
		if (frag.mposition) {
			vert.add_out("vec3 mposition");
			if (frag.ndcpos) {
				vert.write('mposition = (ndc.xyz / ndc.w);');
			}
			else {
				vert.write_attrib('mposition = pos.xyz;');
			}
		}
		if (frag.wtangent) {
			// con.add_elem("tang", "short4norm");
			// vert.add_uniform("mat3 N", "_normalMatrix");
			vert.add_out("vec3 wtangent");
			// vert.write_attrib('wtangent = normalize(mul(tang.xyz, N));');
			vert.write_attrib('wtangent = vec3(0.0, 0.0, 0.0);');
		}
		if (frag.vVecCam) {
			vert.add_uniform("mat4 WV", "_worldViewMatrix");
			vert.add_out("vec3 eyeDirCam");
			vert.write_attrib('eyeDirCam = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz; eyeDirCam.z *= -1.0;');
			frag.write_attrib('vec3 vVecCam = normalize(eyeDirCam);');
		}
		if (frag.vVec) {
			vert.add_uniform("vec3 eye", "_cameraPosition");
			vert.add_out("vec3 eyeDir");
			vert.write_attrib('eyeDir = eye - wposition;');
			frag.write_attrib('vec3 vVec = normalize(eyeDir);');
		}
		if (frag.n) {
			vert.add_uniform("mat3 N", "_normalMatrix");
			vert.add_out("vec3 wnormal");
			vert.write_attrib('wnormal = mul(vec3(nor.xy, pos.w), N);');
			frag.write_attrib('vec3 n = normalize(wnormal);');
		}
		else if (vert.n) {
			vert.add_uniform("mat3 N", "_normalMatrix");
			vert.write_attrib('vec3 wnormal = normalize(mul(vec3(nor.xy, pos.w), N));');
		}
		if (frag.nAttr) {
			vert.add_out("vec3 nAttr");
			vert.write_attrib('nAttr = vec3(nor.xy, pos.w);');
		}
		if (frag.dotNV) {
			frag.write_attrib('float dotNV = max(dot(n, vVec), 0.0);');
		}
		if (frag.wvpposition) {
			vert.add_out("vec4 wvpposition");
			vert.write_end('wvpposition = gl_Position;');
		}
		if (con.is_elem('col')) {
			vert.add_out('vec3 vcolor');
			vert.write_attrib('vcolor = col.rgb;');
		}
	}

	static function parse_output(node: TNode): TShaderOut {
		if (parse_surface || parse_opacity) {
			return parse_shader_input(node.inputs[0]);
		}
		return null;
		// Parse volume, displacement..
	}

	static function parse_output_pbr(node: TNode): TShaderOut {
		if (parse_surface || parse_opacity) {
			return parse_shader(node, null);
		}
		return null;
		// Parse volume, displacement..
	}

	static function get_group(name: String): TNodeCanvas {
		for (g in Project.materialGroups) if (g.canvas.name == name) return g.canvas;
		return null;
	}

	static function push_group(g: TNodeCanvas) {
		canvases.push(g);
		nodes = g.nodes;
		links = g.links;
	}

	static function pop_group() {
		canvases.pop();
		var g = canvases[canvases.length - 1];
		nodes = g.nodes;
		links = g.links;
	}

	static function parse_group(node: TNode, socket: TNodeSocket): String { // Entering group
		parents.push(node);
		push_group(get_group(node.name));
		var output_node = node_by_type(nodes, 'GROUP_OUTPUT');
		if (output_node == null) return null;
		var index = socket_index(node, socket);
		var inp = output_node.inputs[index];
		var out_group = parse_input(inp);
		parents.pop();
		pop_group();
		return out_group;
	}

	static function parse_group_input(node: TNode, socket: TNodeSocket): String {
		var parent = parents.pop(); // Leaving group
		pop_group();
		var index = socket_index(node, socket);
		var inp = parent.inputs[index];
		var res = parse_input(inp);
		parents.push(parent); // Return to group
		push_group(get_group(parent.name));
		return res;
	}

	static function parse_input(inp: TNodeSocket): String {
		if (inp.type == "RGB") {
			return parse_vector_input(inp);
		}
		else if (inp.type == "RGBA") {
			return parse_vector_input(inp);
		}
		else if (inp.type == "VECTOR") {
			return parse_vector_input(inp);
		}
		else if (inp.type == "VALUE") {
			return parse_value_input(inp);
		}
		return null;
	}

	static function parse_shader_input(inp: TNodeSocket): TShaderOut {
		var l = getInputLink(inp);
		var from_node = l != null ? getNode(l.from_id) : null;
		if (from_node != null) {
			if (from_node.type == "REROUTE") {
				return parse_shader_input(from_node.inputs[0]);
			}
			return parse_shader(from_node, from_node.outputs[l.from_socket]);
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

	static function parse_shader(node: TNode, socket: TNodeSocket): TShaderOut {
		var sout: TShaderOut = {
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
			if (parse_surface) {
				// Normal - parsed first to retrieve uv coords
				parse_normal_map_color_input(node.inputs[5]);
				// Base color
				parsing_basecolor = true;
				sout.out_basecol = parse_vector_input(node.inputs[0]);
				parsing_basecolor = false;
				// Occlusion
				sout.out_occlusion = parse_value_input(node.inputs[2]);
				// Roughness
				sout.out_roughness = parse_value_input(node.inputs[3]);
				// Metallic
				sout.out_metallic = parse_value_input(node.inputs[4]);
				// Emission
				if (parse_emission) {
					sout.out_emission = parse_value_input(node.inputs[6]);
				}
				// Subsurface
				if (parse_subsurface) {
					sout.out_subsurface = parse_value_input(node.inputs[8]);
				}
			}

			if (parse_opacity) {
				sout.out_opacity = parse_value_input(node.inputs[1]);
			}

			// Displacement / Height
			if (node.inputs.length > 7 && parse_height) {
				if (!parse_height_as_channel) curshader = vert;
				sout.out_height = parse_value_input(node.inputs[7]);
				if (!parse_height_as_channel) curshader = frag;
			}
		}

		return sout;
	}

	static function parse_vector_input(inp: TNodeSocket): String {
		var l = getInputLink(inp);
		var from_node = l != null ? getNode(l.from_id) : null;
		if (from_node != null) {
			if (from_node.type == "REROUTE") {
				return parse_vector_input(from_node.inputs[0]);
			}

			var res_var = write_result(l);
			var st = from_node.outputs[l.from_socket].type;
			if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
				return res_var;
			}
			else {// VALUE
				return to_vec3(res_var);
			}
		}
		else {
			if (inp.type == "VALUE") { //# Unlinked reroute
				return vec3([0.0, 0.0, 0.0]);
			}
			else {
				return vec3(inp.default_value);
			}
		}
	}

	static function parse_vector(node: TNode, socket: TNodeSocket): String {
		if (node.type == 'GROUP') {
			return parse_group(node, socket);
		}
		else if (node.type == 'GROUP_INPUT') {
			return parse_group_input(node, socket);
		}
		else if (node.type == "ATTRIBUTE") {
			if (socket == node.outputs[0]) { // Color
				if (curshader.context.allow_vcols) {
					curshader.context.add_elem("col", "short4norm"); // Vcols only for now
					return "vcolor";
				}
				else {
					return("vec3(0.0, 0.0, 0.0)");
				}
			}
			else { // Vector
				curshader.context.add_elem("tex", "short2norm"); // UVMaps only for now
				return "vec3(texCoord.x, texCoord.y, 0.0)";
			}
		}
		else if (node.type == "VERTEX_COLOR") {
			if (curshader.context.allow_vcols) {
				curshader.context.add_elem("col", "short4norm");
				return "vcolor";
			}
			else {
				return("vec3(0.0, 0.0, 0.0)");
			}

		}
		else if (node.type == "RGB") {
			return vec3(socket.default_value);
		}
		else if (node.type == "TEX_BRICK") {
			curshader.add_function(ShaderFunctions.str_tex_brick);
			var co = getCoord(node);
			var col1 = parse_vector_input(node.inputs[1]);
			var col2 = parse_vector_input(node.inputs[2]);
			var col3 = parse_vector_input(node.inputs[3]);
			var scale = parse_value_input(node.inputs[4]);
			var res = 'tex_brick($co * $scale, $col1, $col2, $col3)';
			return res;
		}
		else if (node.type == "TEX_CHECKER") {
			curshader.add_function(ShaderFunctions.str_tex_checker);
			var co = getCoord(node);
			var col1 = parse_vector_input(node.inputs[1]);
			var col2 = parse_vector_input(node.inputs[2]);
			var scale = parse_value_input(node.inputs[3]);
			var res = 'tex_checker($co, $col1, $col2, $scale)';
			return res;
		}
		else if (node.type == "TEX_GRADIENT") {
			var co = getCoord(node);
			var but = node.buttons[0]; //gradient_type;
			var grad: String = but.data[but.default_value].toUpperCase();
			grad = grad.replace(" ", "_");
			var f = getGradient(grad, co);
			var res = to_vec3('clamp($f, 0.0, 1.0)');
			return res;
		}
		else if (node.type == "TEX_IMAGE") {
			// Already fetched
			if (parsed.indexOf(res_var_name(node, node.outputs[1])) >= 0) { // TODO: node.outputs[0]
				var varname = store_var_name(node);
				return '$varname.rgb';
			}
			var tex_name = node_name(node);
			var tex = make_texture(node, tex_name);
			if (tex != null) {
				var color_space = node.buttons[1].default_value;
				var texstore = texture_store(node, tex, tex_name, color_space);
				return '$texstore.rgb';
			}
			else {
				var tex_store = store_var_name(node); // Pink color for missing texture
				curshader.write('vec4 $tex_store = vec4(1.0, 0.0, 1.0, 1.0);');
				return '$tex_store.rgb';
			}
		}
		else if (node.type == "TEX_MAGIC") {
			curshader.add_function(ShaderFunctions.str_tex_magic);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_magic($co * $scale * 4.0)';
			return res;
		}
		else if (node.type == "TEX_NOISE") {
			curshader.add_function(ShaderFunctions.str_tex_noise);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var res = 'vec3(tex_noise($co * $scale), tex_noise($co * $scale + 0.33), tex_noise($co * $scale + 0.66))';
			return res;
		}
		else if (node.type == "TEX_VORONOI") {
			curshader.add_function(ShaderFunctions.str_tex_voronoi);
			curshader.add_uniform("sampler2D snoise256", "$noise256.k");
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var but = node.buttons[0]; //coloring;
			var coloring: String = but.data[but.default_value].toUpperCase();
			coloring = coloring.replace(" ", "_");
			var res = "";
			if (coloring == "INTENSITY") {
				res = to_vec3('tex_voronoi($co * $scale, texturePass(snoise256)).a');
			}
			else { // Cells
				res = 'tex_voronoi($co * $scale, texturePass(snoise256)).rgb';
			}
			return res;
		}
		else if (node.type == "TEX_WAVE") {
			curshader.add_function(ShaderFunctions.str_tex_wave);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var res = to_vec3('tex_wave_f($co * $scale)');
			return res;
		}
		else if (node.type == "BRIGHTCONTRAST") {
			var out_col = parse_vector_input(node.inputs[0]);
			var bright = parse_value_input(node.inputs[1]);
			var contr = parse_value_input(node.inputs[2]);
			curshader.add_function(ShaderFunctions.str_brightcontrast);
			return 'brightcontrast($out_col, $bright, $contr)';
		}
		else if (node.type == "GAMMA") {
			var out_col = parse_vector_input(node.inputs[0]);
			var gamma = parse_value_input(node.inputs[1]);
			return 'pow($out_col, ' + to_vec3('$gamma') + ")";
		}
		else if (node.type == "DIRECT_WARP") {
			if (warp_passthrough) return parse_vector_input(node.inputs[0]);
			var angle = parse_value_input(node.inputs[1], true);
			var mask = parse_value_input(node.inputs[2], true);
			var tex_name = "texwarp_" + node_name(node);
			curshader.add_uniform("sampler2D " + tex_name, "_" + tex_name);
			var store = store_var_name(node);
			curshader.write('float ${store}_rad = $angle * (${Math.PI} / 180);');
			curshader.write('float ${store}_x = cos(${store}_rad);');
			curshader.write('float ${store}_y = sin(${store}_rad);');
			return 'texture($tex_name, texCoord + vec2(${store}_x, ${store}_y) * $mask).rgb;';
		}
		else if (node.type == "BLUR") {
			if (blur_passthrough) return parse_vector_input(node.inputs[0]);
			var strength = parse_value_input(node.inputs[1]);
			if (strength == "0.0") return "vec3(0.0, 0.0, 0.0)";
			var steps = 'int($strength * 10 + 1)';
			var tex_name = "texblur_" + node_name(node);
			curshader.add_uniform("sampler2D " + tex_name, "_" + tex_name);
			var store = store_var_name(node);
			curshader.write('vec3 ${store}_res = vec3(0.0, 0.0, 0.0);');
			curshader.write('for (int i = -$steps; i <= $steps; ++i) {');
			curshader.write('for (int j = -$steps; j <= $steps; ++j) {');
			curshader.write('${store}_res += texture($tex_name, texCoord + vec2(i, j) / vec2(textureSize($tex_name, 0))).rgb;');
			curshader.write('}');
			curshader.write('}');
			curshader.write('${store}_res /= ($steps * 2 + 1) * ($steps * 2 + 1);');
			return '${store}_res';
		}
		else if (node.type == "HUE_SAT") {
			curshader.add_function(ShaderFunctions.str_hue_sat);
			var hue = parse_value_input(node.inputs[0]);
			var sat = parse_value_input(node.inputs[1]);
			var val = parse_value_input(node.inputs[2]);
			var fac = parse_value_input(node.inputs[3]);
			var col = parse_vector_input(node.inputs[4]);
			return 'hue_sat($col, vec4($hue-0.5, $sat, $val, 1.0-$fac))';
		}
		else if (node.type == "INVERT") {
			var fac = parse_value_input(node.inputs[0]);
			var out_col = parse_vector_input(node.inputs[1]);
			return 'mix($out_col, vec3(1.0, 1.0, 1.0) - ($out_col), $fac)';
		}
		else if (node.type == "MIX_RGB") {
			var fac = parse_value_input(node.inputs[0]);
			var fac_var = node_name(node) + "_fac";
			curshader.write('float $fac_var = $fac;');
			var col1 = parse_vector_input(node.inputs[1]);
			var col2 = parse_vector_input(node.inputs[2]);
			var but = node.buttons[0]; // blend_type
			var blend: String = but.data[but.default_value].toUpperCase();
			blend = blend.replace(" ", "_");
			var use_clamp = node.buttons[1].default_value == true;
			var out_col = "";
			if (blend == "MIX") {
				out_col = 'mix($col1, $col2, $fac_var)';
			}
			else if (blend == "DARKEN") {
				out_col = 'min($col1, $col2 * $fac_var)';
			}
			else if (blend == "MULTIPLY") {
				out_col = 'mix($col1, $col1 * $col2, $fac_var)';
			}
			else if (blend == "BURN") {
				out_col = 'mix($col1, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $col1) / $col2, $fac_var)';
			}
			else if (blend == "LIGHTEN") {
				out_col = 'max($col1, $col2 * $fac_var)';
			}
			else if (blend == "SCREEN") {
				out_col = '(vec3(1.0, 1.0, 1.0) - (' + to_vec3('1.0 - $fac_var') + ' + $fac_var * (vec3(1.0, 1.0, 1.0) - $col2)) * (vec3(1.0, 1.0, 1.0) - $col1))';
			}
			else if (blend == "DODGE") {
				out_col = 'mix($col1, $col1 / (vec3(1.0, 1.0, 1.0) - $col2), $fac_var)';
			}
			else if (blend == "ADD") {
				out_col = 'mix($col1, $col1 + $col2, $fac_var)';
			}
			else if (blend == "OVERLAY") {
				out_col = 'mix($col1, vec3(
					$col1.r < 0.5 ? 2.0 * $col1.r * $col2.r : 1.0 - 2.0 * (1.0 - $col1.r) * (1.0 - $col2.r),
					$col1.g < 0.5 ? 2.0 * $col1.g * $col2.g : 1.0 - 2.0 * (1.0 - $col1.g) * (1.0 - $col2.g),
					$col1.b < 0.5 ? 2.0 * $col1.b * $col2.b : 1.0 - 2.0 * (1.0 - $col1.b) * (1.0 - $col2.b)
				), $fac_var)';
			}
			else if (blend == "SOFT_LIGHT") {
				out_col = '((1.0 - $fac_var) * $col1 + $fac_var * ((vec3(1.0, 1.0, 1.0) - $col1) * $col2 * $col1 + $col1 * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $col2) * (vec3(1.0, 1.0, 1.0) - $col1))))';
			}
			else if (blend == "LINEAR_LIGHT") {
				out_col = '($col1 + $fac_var * (vec3(2.0, 2.0, 2.0) * ($col2 - vec3(0.5, 0.5, 0.5))))';
			}
			else if (blend == "DIFFERENCE") {
				out_col = 'mix($col1, abs($col1 - $col2), $fac_var)';
			}
			else if (blend == "SUBTRACT") {
				out_col = 'mix($col1, $col1 - $col2, $fac_var)';
			}
			else if (blend == "DIVIDE") {
				var eps = 0.000001;
				col2 = 'max($col2, vec3($eps, $eps, $eps))';
				out_col = "(" + to_vec3('(1.0 - $fac_var) * $col1 + $fac_var * $col1 / $col2') + ")";
			}
			else if (blend == "HUE") {
				curshader.add_function(ShaderFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col2).r, rgb_to_hsv($col1).g, rgb_to_hsv($col1).b)), $fac_var)';
			}
			else if (blend == "SATURATION") {
				curshader.add_function(ShaderFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col1).r, rgb_to_hsv($col2).g, rgb_to_hsv($col1).b)), $fac_var)';
			}
			else if (blend == "COLOR") {
				curshader.add_function(ShaderFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col2).r, rgb_to_hsv($col2).g, rgb_to_hsv($col1).b)), $fac_var)';
			}
			else if (blend == "VALUE") {
				curshader.add_function(ShaderFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col1).r, rgb_to_hsv($col1).g, rgb_to_hsv($col2).b)), $fac_var)';
			}
			if (use_clamp) return 'clamp($out_col, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0))';
			else return out_col;
		}
		else if (node.type == "QUANTIZE") {
			var strength = parse_value_input(node.inputs[0]);
			var col = parse_vector_input(node.inputs[1]);
			return '(floor(100.0 * $strength * $col) / (100.0 * $strength))';
		}
		else if (node.type == "VALTORGB") { // ColorRamp
			var fac = parse_value_input(node.inputs[0]);
			var interp = node.buttons[0].data == 0 ? "LINEAR" : "CONSTANT";
			var elems: Array<Array<Float>> = node.buttons[0].default_value;
			if (elems.length == 1) {
				return vec3(elems[0]);
			}
			// Write cols array
			var cols_var = node_name(node) + "_cols";
			curshader.write('vec3 $cols_var[${elems.length}];'); // TODO: Make const
			for (i in 0...elems.length) {
				curshader.write('$cols_var[$i] = ${vec3(elems[i])};');
			}
			// Get index
			var fac_var = node_name(node) + "_fac";
			curshader.write('float $fac_var = $fac;');
			var index = "0";
			for (i in 1...elems.length) {
				index += ' + ($fac_var > ${elems[i][4]} ? 1 : 0)';
			}
			// Write index
			var index_var = node_name(node) + "_i";
			curshader.write('int $index_var = $index;');
			if (interp == "CONSTANT") {
				return '$cols_var[$index_var]';
			}
			else { // Linear
				// Write facs array
				var facs_var = node_name(node) + "_facs";
				curshader.write('float $facs_var[${elems.length}];'); // TODO: Make const
				for (i in 0...elems.length) {
					curshader.write('$facs_var[$i] = ${elems[i][4]};');
				}
				// Mix color
				// float f = (pos - start) * (1.0 / (finish - start))
				return 'mix($cols_var[$index_var], $cols_var[$index_var + 1], ($fac_var - $facs_var[$index_var]) * (1.0 / ($facs_var[$index_var + 1] - $facs_var[$index_var]) ))';
			}
		}
		else if (node.type == "CURVE_VEC") {
			var fac = parse_value_input(node.inputs[0]);
			var vec = parse_vector_input(node.inputs[1]);
			var curves = node.buttons[0].default_value;
			var name = node_name(node);
			var vc0 = vector_curve(name + "0", vec + ".x", curves[0]);
			var vc1 = vector_curve(name + "1", vec + ".y", curves[1]);
			var vc2 = vector_curve(name + "2", vec + ".z", curves[2]);
			// mapping.curves[0].points[0].handle_type # bezier curve
			return '(vec3($vc0, $vc1, $vc2) * $fac)';
		}
		else if (node.type == "CURVE_RGB") { // RGB Curves
			var fac = parse_value_input(node.inputs[0]);
			var vec = parse_vector_input(node.inputs[1]);
			var curves = node.buttons[0].default_value;
			var name = node_name(node);
			// mapping.curves[0].points[0].handle_type
			var vc0 = vector_curve(name + "0", vec + ".x", curves[0]);
			var vc1 = vector_curve(name + "1", vec + ".y", curves[1]);
			var vc2 = vector_curve(name + "2", vec + ".z", curves[2]);
			var vc3a = vector_curve(name + "3a", vec + ".x", curves[3]);
			var vc3b = vector_curve(name + "3b", vec + ".y", curves[3]);
			var vc3c = vector_curve(name + "3c", vec + ".z", curves[3]);
			return '(sqrt(vec3($vc0, $vc1, $vc2) * vec3($vc3a, $vc3b, $vc3c)) * $fac)';
		}
		else if (node.type == "COMBHSV") {
			curshader.add_function(ShaderFunctions.str_hue_sat);
			var h = parse_value_input(node.inputs[0]);
			var s = parse_value_input(node.inputs[1]);
			var v = parse_value_input(node.inputs[2]);
			return 'hsv_to_rgb(vec3($h, $s, $v))';
		}
		else if (node.type == "COMBRGB") {
			var r = parse_value_input(node.inputs[0]);
			var g = parse_value_input(node.inputs[1]);
			var b = parse_value_input(node.inputs[2]);
			return 'vec3($r, $g, $b)';
		}
		else if (node.type == "WAVELENGTH") {
			curshader.add_function(ShaderFunctions.str_wavelength_to_rgb);
			var wl = parse_value_input(node.inputs[0]);
			curshader.add_function(ShaderFunctions.str_wavelength_to_rgb);
			return 'wavelength_to_rgb(($wl - 450.0) / 150.0)';
		}
		else if (node.type == "CAMERA") {
			curshader.vVecCam = true;
			return "vVecCam";
		}
		else if (node.type == "LAYER") {
			var l = node.buttons[0].default_value;
			if (socket == node.outputs[0]) { // Base
				curshader.add_uniform("sampler2D texpaint" + l, "_texpaint" + l);
				return "texture(texpaint" + l + ", texCoord).rgb";
			}
			else if (socket == node.outputs[5]) { // Normal
				curshader.add_uniform("sampler2D texpaint_nor" + l, "_texpaint_nor" + l);
				return "texture(texpaint_nor" + l + ", texCoord).rgb";
			}
		}
		else if (node.type == "MATERIAL") {
			var result = "vec3(0.0, 0.0, 0.0)";
			var mi = node.buttons[0].default_value;
			if (mi >= Project.materials.length) return result;
			var m = Project.materials[mi];
			var _nodes = nodes;
			var _links = links;
			nodes = m.canvas.nodes;
			links = m.canvas.links;
			parents.push(node);
			var output_node = node_by_type(nodes, "OUTPUT_MATERIAL_PBR");
			if (socket == node.outputs[0]) { // Base
				result = parse_vector_input(output_node.inputs[0]);
			}
			else if (socket == node.outputs[5]) { // Normal
				result = parse_vector_input(output_node.inputs[5]);
			}
			nodes = _nodes;
			links = _links;
			parents.pop();
			return result;
		}
		else if (node.type == "PICKER") {
			if (socket == node.outputs[0]) { // Base
				curshader.add_uniform("vec3 pickerBase", "_pickerBase");
				return "pickerBase";
			}
			else if (socket == node.outputs[5]) { // Normal
				curshader.add_uniform("vec3 pickerNormal", "_pickerNormal");
				return "pickerNormal";
			}
		}
		else if (node.type == "NEW_GEOMETRY") {
			if (socket == node.outputs[0]) { // Position
				curshader.wposition = true;
				return "wposition";
			}
			else if (socket == node.outputs[1]) { // Normal
				curshader.n = true;
				return "n";
			}
			else if (socket == node.outputs[2]) { // Tangent
				curshader.wtangent = true;
				return "wtangent";
			}
			else if (socket == node.outputs[3]) { // True Normal
				curshader.n = true;
				return "n";
			}
			else if (socket == node.outputs[4]) { // Incoming
				curshader.vVec = true;
				return "vVec";
			}
			else if (socket == node.outputs[5]) { // Parametric
				curshader.mposition = true;
				return "mposition";
			}
		}
		else if (node.type == "OBJECT_INFO") {
			if (socket == node.outputs[0]) { // Location
				curshader.wposition = true;
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
			curshader.wtangent = true;
			return "wtangent";
		}
		else if (node.type == "TEX_COORD") {
			if (socket == node.outputs[0]) { // Generated - bounds
				curshader.bposition = true;
				return "bposition";
			}
			else if (socket == node.outputs[1]) { // Normal
				curshader.n = true;
				return "n";
			}
			else if (socket == node.outputs[2]) {// UV
				curshader.context.add_elem("tex", "short2norm");
				return "vec3(texCoord.x, texCoord.y, 0.0)";
			}
			else if (socket == node.outputs[3]) { // Object
				curshader.mposition = true;
				return "mposition";
			}
			else if (socket == node.outputs[4]) { // Camera
				curshader.vposition = true;
				return "vposition";
			}
			else if (socket == node.outputs[5]) { // Window
				curshader.wvpposition = true;
				return "wvpposition.xyz";
			}
			else if (socket == node.outputs[6]) { // Reflection
				return "vec3(0.0, 0.0, 0.0)";
			}
		}
		else if (node.type == "UVMAP") {
			curshader.context.add_elem("tex", "short2norm");
			return "vec3(texCoord.x, texCoord.y, 0.0)";
		}
		else if (node.type == "BUMP") {
			var strength = parse_value_input(node.inputs[0]);
			// var distance = parse_value_input(node.inputs[1]);
			var height = parse_value_input(node.inputs[2]);
			var nor = parse_vector_input(node.inputs[3]);
			var sample_bump_res = store_var_name(node) + "_bump";
			curshader.write('float ${sample_bump_res}_x = dFdx(float($height)) * ($strength) * 16.0;');
			curshader.write('float ${sample_bump_res}_y = dFdy(float($height)) * ($strength) * 16.0;');
			return '(normalize(vec3(${sample_bump_res}_x, ${sample_bump_res}_y, 1.0) + $nor) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5))';
		}
		else if (node.type == "MAPPING") {
			var out = parse_vector_input(node.inputs[0]);
			var node_translation = parse_vector_input(node.inputs[1]);
			var node_rotation = parse_vector_input(node.inputs[2]);
			var node_scale = parse_vector_input(node.inputs[3]);
			if (node_scale != 'vec3(1, 1, 1)') {
				out = '($out * $node_scale)';
			}
			if (node_rotation != 'vec3(0, 0, 0)') {
				// ZYX rotation, Z axis for now..
				var a = '${node_rotation}.z * (3.1415926535 / 180)';
				// x * cos(theta) - y * sin(theta)
				// x * sin(theta) + y * cos(theta)
				out = 'vec3(${out}.x * cos($a) - ${out}.y * sin($a), ${out}.x * sin($a) + ${out}.y * cos($a), 0.0)';
			}
			// if node.rotation[1] != 0.0:
			//     a = node.rotation[1]
			//     out = 'vec3({0}.x * {1} - {0}.z * {2}, {0}.x * {2} + {0}.z * {1}, 0.0)'.format(out, math.cos(a), math.sin(a))
			// if node.rotation[0] != 0.0:
			//     a = node.rotation[0]
			//     out = 'vec3({0}.y * {1} - {0}.z * {2}, {0}.y * {2} + {0}.z * {1}, 0.0)'.format(out, math.cos(a), math.sin(a))
			if (node_translation != 'vec3(0, 0, 0)') {
				out = '($out + $node_translation)';
			}
			// if node.use_min:
				// out = 'max({0}, vec3({1}, {2}, {3}))'.format(out, node.min[0], node.min[1])
			// if node.use_max:
				 // out = 'min({0}, vec3({1}, {2}, {3}))'.format(out, node.max[0], node.max[1])
			return out;
		}
		else if (node.type == "NORMAL") {
			if (socket == node.outputs[0]) {
				return vec3(node.outputs[0].default_value);
			}
			else if (socket == node.outputs[1]) {
				var nor = parse_vector_input(node.inputs[0]);
				var norout = vec3(node.outputs[0].default_value);
				return to_vec3('dot($norout, $nor)');
			}
		}
		else if (node.type == "NORMAL_MAP") {
			var strength = parse_value_input(node.inputs[0]);
			var norm = parse_vector_input(node.inputs[1]);

			var store = store_var_name(node);
			curshader.write('vec3 ${store}_texn = $norm * 2.0 - 1.0;');
			curshader.write('${store}_texn.xy = $strength * ${store}_texn.xy;');
			curshader.write('${store}_texn = normalize(${store}_texn);');

			return '(0.5 * ${store}_texn + 0.5)';
		}
		else if (node.type == "MIX_NORMAL_MAP") {
			var nm1 = parse_vector_input(node.inputs[0]);
			var nm2 = parse_vector_input(node.inputs[1]);
			var but = node.buttons[0];
			var blend: String = but.data[but.default_value].toUpperCase(); // blend_type
			blend = blend.replace(" ", "_");
			var store = store_var_name(node);

			// The blending algorithms are based on the paper 'Blending in Detail' by Colin Barré-Brisebois and Stephen Hill 2012
			// https://blog.selfshadow.com/publications/blending-in-detail/
			if (blend == "PARTIAL_DERIVATIVE") { //partial derivate blending
				curshader.write('vec3 ${store}_n1 = $nm1 * 2.0 - 1.0;');
				curshader.write('vec3 ${store}_n2 = $nm2 * 2.0 - 1.0;');
				return '0.5 * normalize(vec3(${store}_n1.xy * ${store}_n2.z + ${store}_n2.xy * ${store}_n1.z, ${store}_n1.z * ${store}_n2.z)) + 0.5;';
			}
			else if (blend == "WHITEOUT") { //whiteout blending
				curshader.write('vec3 ${store}_n1 = $nm1 * 2.0 - 1.0;');
				curshader.write('vec3 ${store}_n2 = $nm2 * 2.0 - 1.0;');
				return '0.5 * normalize(vec3(${store}_n1.xy + ${store}_n2.xy, ${store}_n1.z * ${store}_n2.z)) + 0.5;';
			}
			else if (blend == "REORIENTED") { //reoriented normal mapping
				curshader.write('vec3 ${store}_n1 = $nm1 * 2.0 - vec3(1.0, 1.0, 0.0);');
				curshader.write('vec3 ${store}_n2 = $nm2 * vec3(-2.0, -2.0, 2.0) - vec3(-1.0, -1.0, 1.0);');
				return '0.5 * normalize(${store}_n1 * dot(${store}_n1, ${store}_n2) - ${store}_n2 * ${store}_n1.z) + 0.5';
			}
		}
		else if (node.type == "VECT_TRANSFORM") {
		// 	#type = node.vector_type
		// 	#conv_from = node.convert_from
		// 	#conv_to = node.convert_to
		// 	# Pass throuh
		// 	return parse_vector_input(node.inputs[0])
		}
		else if (node.type == "COMBXYZ") {
			var x = parse_value_input(node.inputs[0]);
			var y = parse_value_input(node.inputs[1]);
			var z = parse_value_input(node.inputs[2]);
			return 'vec3($x, $y, $z)';
		}
		else if (node.type == "VECT_MATH") {
			var vec1 = parse_vector_input(node.inputs[0]);
			var vec2 = parse_vector_input(node.inputs[1]);
			var but = node.buttons[0]; //operation;
			var op: String = but.data[but.default_value].toUpperCase();
			op = op.replace(" ", "_");
			if (op == "ADD") {
				return '($vec1 + $vec2)';
			}
			else if (op == "SUBTRACT") {
				return '($vec1 - $vec2)';
			}
			else if (op == "AVERAGE") {
				return '(($vec1 + $vec2) / 2.0)';
			}
			else if (op == "DOT_PRODUCT") {
				return to_vec3('dot($vec1, $vec2)');
			}
			else if (op == "LENGTH") {
				return to_vec3('length($vec1)');
			}
			else if (op == "DISTANCE") {
				return to_vec3('distance($vec1, $vec2)');
			}
			else if (op == "CROSS_PRODUCT") {
				return 'cross($vec1, $vec2)';
			}
			else if (op == "NORMALIZE") {
				return 'normalize($vec1)';
			}
			else if (op == "MULTIPLY") {
				return '($vec1 * $vec2)';
			}
			else if (op == "DIVIDE") {
				return 'vec3(${vec1}.x / (${vec2}.x == 0 ? 0.000001 : ${vec2}.x), ${vec1}.y / (${vec2}.y == 0 ? 0.000001 : ${vec2}.y), ${vec1}.z / (${vec2}.z == 0 ? 0.000001 : ${vec2}.z))';
			}
			else if (op == "PROJECT") {
				return '(dot($vec1, $vec2) / dot($vec2, $vec2) * $vec2)';
			}
			else if (op == "REFLECT") {
				return 'reflect($vec1, normalize($vec2))';
			}
			else if (op == "SCALE") {
				return '(${vec2}.x * $vec1)';
			}
			else if (op == "ABSOLUTE") {
				return 'abs($vec1)';
			}
			else if (op == "MINIMUM") {
				return 'min($vec1, $vec2)';
			}
			else if (op == "MAXIMUM") {
				return 'max($vec1, $vec2)';
			}
			else if (op == "FLOOR") {
				return 'floor($vec1)';
			}
			else if (op == "CEIL") {
				return 'ceil($vec1)';
			}
			else if (op == "FRACTION") {
				return 'fract($vec1)';
			}
			else if (op == "MODULO") {
				return 'mod($vec1, $vec2)';
			}
			else if(op == "SNAP") {
				return '(floor($vec1 / $vec2) * $vec2)';
			}
			else if (op == "SINE") {
				return 'sin($vec1)';
			}
			else if (op == "COSINE") {
				return 'cos($vec1)';
			}
			else if (op == "TANGENT") {
				return 'tan($vec1)';
			}
		}
		else if (node.type == "Displacement") {
			var height = parse_value_input(node.inputs[0]);
			return to_vec3('$height');
		}
		else if (customNodes.get(node.type) != null) {
			return customNodes.get(node.type)(node, socket);
		}
		return "vec3(0.0, 0.0, 0.0)";
	}

	static function parse_normal_map_color_input(inp: TNodeSocket) {
		frag.write_normal++;
		out_normaltan = parse_vector_input(inp);
		if (!arm_export_tangents) {
			frag.write('vec3 texn = ($out_normaltan) * 2.0 - 1.0;');
			frag.write('texn.y = -texn.y;');
			if (!cotangentFrameWritten) {
				cotangentFrameWritten = true;
				frag.add_function(ShaderFunctions.str_cotangentFrame);
			}
			frag.n = true;
			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end

			frag.write('n = mul(normalize(texn), TBN);');
		}
		frag.write_normal--;
	}

	static function parse_value_input(inp: TNodeSocket, vector_as_grayscale = false) : String {
		var l = getInputLink(inp);
		var from_node = l != null ? getNode(l.from_id) : null;
		if (from_node != null) {
			if (from_node.type == "REROUTE") {
				return parse_value_input(from_node.inputs[0]);
			}

			var res_var = write_result(l);
			var st = from_node.outputs[l.from_socket].type;
			if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
				if (vector_as_grayscale) {
					return 'dot($res_var.rbg, vec3(0.299, 0.587, 0.114))';
				}
				else {
					return '$res_var.x';
				}
			}
			else { // VALUE
				return res_var;
			}
		}
		else {
			return vec1(inp.default_value);
		}
	}

	static function parse_value(node: TNode, socket: TNodeSocket): String {
		if (node.type == 'GROUP') {
			return parse_group(node, socket);
		}
		else if (node.type == 'GROUP_INPUT') {
			return parse_group_input(node, socket);
		}
		else if (node.type == "ATTRIBUTE") {
			curshader.add_uniform("float time", "_time");
			return "time";
		}
		else if (node.type == "VERTEX_COLOR") {
			return "1.0";
		}
		else if (node.type == "WIREFRAME") {
			curshader.add_uniform('sampler2D texuvmap', '_texuvmap');
			var use_pixel_size = node.buttons[0].default_value == "true";
			var pixel_size = parse_value_input(node.inputs[0]);
			return "textureLod(texuvmap, texCoord, 0.0).r";
		}
		else if (node.type == "CAMERA") {
			if (socket == node.outputs[1]) { // View Z Depth
				curshader.add_uniform("vec2 cameraProj", "_cameraPlaneProj");
				#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
				curshader.wvpposition = true;
				return "(cameraProj.y / ((wvpposition.z / wvpposition.w) - cameraProj.x))";
				#else
				return "(cameraProj.y / (gl_FragCoord.z - cameraProj.x))";
				#end
			}
			else { // View Distance
				curshader.add_uniform("vec3 eye", "_cameraPosition");
				curshader.wposition = true;
				return "distance(eye, wposition)";
			}
		}
		else if (node.type == "LAYER") {
			var l = node.buttons[0].default_value;
			if (socket == node.outputs[1]) { // Opac
				curshader.add_uniform("sampler2D texpaint" + l, "_texpaint" + l);
				return "texture(texpaint" + l + ", texCoord).a";
			}
			else if (socket == node.outputs[2]) { // Occ
				curshader.add_uniform("sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).r";
			}
			else if (socket == node.outputs[3]) { // Rough
				curshader.add_uniform("sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).g";
			}
			else if (socket == node.outputs[4]) { // Metal
				curshader.add_uniform("sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).b";
			}
			else if (socket == node.outputs[7]) { // Height
				curshader.add_uniform("sampler2D texpaint_pack" + l, "_texpaint_pack" + l);
				return "texture(texpaint_pack" + l + ", texCoord).a";
			}
		}
		else if (node.type == "LAYER_MASK") {
			if (socket == node.outputs[0]) {
				var l = node.buttons[0].default_value;
				curshader.add_uniform("sampler2D texpaint" + l, "_texpaint" + l);
				return "texture(texpaint" + l + ", texCoord).r";
			}
		}
		else if (node.type == "MATERIAL") {
			var result = "0.0";
			var mi = node.buttons[0].default_value;
			if (mi >= Project.materials.length) return result;
			var m = Project.materials[mi];
			var _nodes = nodes;
			var _links = links;
			nodes = m.canvas.nodes;
			links = m.canvas.links;
			parents.push(node);
			var output_node = node_by_type(nodes, "OUTPUT_MATERIAL_PBR");
			if (socket == node.outputs[1]) { // Opac
				result = parse_value_input(output_node.inputs[1]);
			}
			else if (socket == node.outputs[2]) { // Occ
				result = parse_value_input(output_node.inputs[2]);
			}
			else if (socket == node.outputs[3]) { // Rough
				result = parse_value_input(output_node.inputs[3]);
			}
			else if (socket == node.outputs[4]) { // Metal
				result = parse_value_input(output_node.inputs[4]);
			}
			else if (socket == node.outputs[7]) { // Height
				result = parse_value_input(output_node.inputs[7]);
			}
			nodes = _nodes;
			links = _links;
			parents.pop();
			return result;
		}
		else if (node.type == "PICKER") {
			if (socket == node.outputs[1]) {
				curshader.add_uniform("float pickerOpacity", "_pickerOpacity");
				return "pickerOpacity";
			}
			else if (socket == node.outputs[2]) {
				curshader.add_uniform("float pickerOcclusion", "_pickerOcclusion");
				return "pickerOcclusion";
			}
			else if (socket == node.outputs[3]) {
				curshader.add_uniform("float pickerRoughness", "_pickerRoughness");
				return "pickerRoughness";
			}
			else if (socket == node.outputs[4]) {
				curshader.add_uniform("float pickerMetallic", "_pickerMetallic");
				return "pickerMetallic";
			}
			else if (socket == node.outputs[7]) {
				curshader.add_uniform("float pickerHeight", "_pickerHeight");
				return "pickerHeight";
			}
		}
		else if (node.type == "FRESNEL") {
			var ior = parse_value_input(node.inputs[0]);
			curshader.dotNV = true;
			return 'pow(1.0 - dotNV, 7.25 / $ior)';
		}
		else if (node.type == "NEW_GEOMETRY") {
			if (socket == node.outputs[6]) { // Backfacing
				#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
				return "0.0"; // SV_IsFrontFace
				#else
				return "(1.0 - float(gl_FrontFacing))";
				#end
			}
			else if (socket == node.outputs[7]) { // Pointiness
				var strength = 1.0;
				var radius = 1.0;
				var offset = 0.0;
				var store = store_var_name(node);
				curshader.n = true;
				curshader.write('vec3 ${store}_dx = dFdx(n);');
				curshader.write('vec3 ${store}_dy = dFdy(n);');
				curshader.write('float ${store}_curvature = max(dot(${store}_dx, ${store}_dx), dot(${store}_dy, ${store}_dy));');
				curshader.write('${store}_curvature = clamp(pow(${store}_curvature, (1.0 / ' + radius + ') * 0.25) * ' + strength + ' * 2.0 + ' + offset + ' / 10.0, 0.0, 1.0);');
				return '${store}_curvature';
			}
			else if (socket == node.outputs[8]) { // Random Per Island
				return "0.0";
			}
		}
		else if (node.type == "HAIR_INFO") {
			return "0.5";
		}
		else if (node.type == "LAYER_WEIGHT") {
			var blend = parse_value_input(node.inputs[0]);
			if (socket == node.outputs[0]) { // Fresnel
				curshader.dotNV = true;
				return 'clamp(pow(1.0 - dotNV, (1.0 - $blend) * 10.0), 0.0, 1.0)';
			}
			else if (socket == node.outputs[1]) { // Facing
				curshader.dotNV = true;
				return '((1.0 - dotNV) * $blend)';
			}
		}
		else if (node.type == "OBJECT_INFO") {
			if (socket == node.outputs[1]) { // Object Index
				curshader.add_uniform("float objectInfoIndex", "_objectInfoIndex");
				return "objectInfoIndex";
			}
			else if (socket == node.outputs[2]) { // Material Index
				curshader.add_uniform("float objectInfoMaterialIndex", "_objectInfoMaterialIndex");
				return "objectInfoMaterialIndex";
			}
			else if (socket == node.outputs[3]) { // Random
				curshader.add_uniform("float objectInfoRandom", "_objectInfoRandom");
				return "objectInfoRandom";
			}
		}
		else if (node.type == "VALUE") {
			return vec1(node.outputs[0].default_value);
		}
		else if (node.type == "TEX_BRICK") {
			curshader.add_function(ShaderFunctions.str_tex_brick);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[4]);
			var res = 'tex_brick_f($co * $scale)';
			return res;
		}
		else if (node.type == "TEX_CHECKER") {
			curshader.add_function(ShaderFunctions.str_tex_checker);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[3]);
			var res = 'tex_checker_f($co, $scale)';
			return res;
		}
		else if (node.type == "TEX_GRADIENT") {
			var co = getCoord(node);
			var but = node.buttons[0]; //gradient_type;
			var grad: String = but.data[but.default_value].toUpperCase();
			grad = grad.replace(" ", "_");
			var f = getGradient(grad, co);
			var res = '(clamp($f, 0.0, 1.0))';
			return res;
		}
		else if (node.type == "TEX_IMAGE") {
			// Already fetched
			if (parsed.indexOf(res_var_name(node, node.outputs[0])) >= 0) { // TODO: node.outputs[1]
				var varname = store_var_name(node);
				return '$varname.a';
			}
			var tex_name = node_name(node);
			var tex = make_texture(node, tex_name);
			if (tex != null) {
				var color_space = node.buttons[1].default_value;
				var texstore = texture_store(node, tex, tex_name, color_space);
				return '$texstore.a';
			}
		}
		else if (node.type == "TEX_MAGIC") {
			curshader.add_function(ShaderFunctions.str_tex_magic);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_magic_f($co * $scale * 4.0)';
			return res;
		}
		else if (node.type == "TEX_MUSGRAVE") {
			curshader.add_function(ShaderFunctions.str_tex_musgrave);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_musgrave_f($co * $scale * 0.5)';
			return res;
		}
		else if (node.type == "TEX_NOISE") {
			curshader.add_function(ShaderFunctions.str_tex_noise);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_noise($co * $scale)';
			return res;
		}
		else if (node.type == "TEX_VORONOI") {
			curshader.add_function(ShaderFunctions.str_tex_voronoi);
			curshader.add_uniform("sampler2D snoise256", "$noise256.k");
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var but = node.buttons[0]; // coloring
			var coloring: String = but.data[but.default_value].toUpperCase();
			coloring = coloring.replace(" ", "_");
			var res = "";
			if (coloring == "INTENSITY") {
				res = 'tex_voronoi($co * $scale, texturePass(snoise256)).a';
			}
			else { // Cells
				res = 'tex_voronoi($co * $scale, texturePass(snoise256)).r';
			}
			return res;
		}
		else if (node.type == "TEX_WAVE") {
			curshader.add_function(ShaderFunctions.str_tex_wave);
			var co = getCoord(node);
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_wave_f($co * $scale)';
			return res;
		}
		else if (node.type == "BAKE_CURVATURE") {
			if (bake_passthrough) {
				bake_passthrough_strength = parse_value_input(node.inputs[0]);
				bake_passthrough_radius = parse_value_input(node.inputs[1]);
				bake_passthrough_offset = parse_value_input(node.inputs[2]);
				return "0.0";
			}
			var tex_name = "texbake_" + node_name(node);
			curshader.add_uniform("sampler2D " + tex_name, "_" + tex_name);
			var store = store_var_name(node);
			curshader.write('float ${store}_res = texture($tex_name, texCoord).r;');
			return '${store}_res';
		}
		else if (node.type == "NORMAL") {
			var nor = parse_vector_input(node.inputs[0]);
			var norout = vec3(node.outputs[0].default_value);
			return 'dot($norout, $nor)';
		}
		else if (node.type == "COLMASK") {
			var inputColor = parse_vector_input(node.inputs[0]);
			var maskColor = parse_vector_input(node.inputs[1]);
			var radius = parse_value_input(node.inputs[2]);
			var fuzziness = parse_value_input(node.inputs[3]);
			return 'clamp(1.0 - (distance($inputColor, $maskColor) - $radius) / max($fuzziness, $eps), 0.0, 1.0)';
		}
		else if (node.type == "MATH") {
			var val1 = parse_value_input(node.inputs[0]);
			var val2 = parse_value_input(node.inputs[1]);
			var but = node.buttons[0]; // operation
			var op: String = but.data[but.default_value].toUpperCase();
			op = op.replace(" ", "_");
			var use_clamp = node.buttons[1].default_value == true;
			var out_val = "";
			if (op == "ADD") {
				out_val = '($val1 + $val2)';
			}
			else if (op == "SUBTRACT") {
				out_val = '($val1 - $val2)';
			}
			else if (op == "MULTIPLY") {
				out_val = '($val1 * $val2)';
			}
			else if (op == "DIVIDE") {
				val2 = '($val2 == 0.0 ? $eps : $val2)';
				out_val = '($val1 / $val2)';
			}
			else if (op == "POWER") {
				out_val = 'pow($val1, $val2)';
			}
			else if (op == "LOGARITHM") {
				out_val = 'log($val1)';
			}
			else if (op == "SQUARE_ROOT") {
				out_val = 'sqrt($val1)';
			}
			else if(op == "INVERSE_SQUARE_ROOT") {
				out_val = 'inversesqrt($val1)';
			}
			else if (op == "EXPONENT") {
				out_val = 'exp($val1)';
			}
			else if (op == "ABSOLUTE") {
				out_val = 'abs($val1)';
			}
			else if (op == "MINIMUM") {
				out_val = 'min($val1, $val2)';
			}
			else if (op == "MAXIMUM") {
				out_val = 'max($val1, $val2)';
			}
			else if (op == "LESS_THAN") {
				out_val = 'float($val1 < $val2)';
			}
			else if (op == "GREATER_THAN") {
				out_val = 'float($val1 > $val2)';
			}
			else if (op == "SIGN") {
				out_val = 'sign($val1)';
			}
			else if (op == "ROUND") {
				out_val = 'floor($val1 + 0.5)';
			}
			else if (op == "FLOOR") {
				out_val = 'floor($val1)';
			}
			else if (op == "CEIL") {
				out_val = 'ceil($val1)';
			}
			else if(op == "SNAP") {
				out_val = '(floor($val1 / $val2) * $val2)';
			}
			else if (op == "TRUNCATE") {
				out_val = 'trunc($val1)';
			}
			else if (op == "FRACTION") {
				out_val = 'fract($val1)';
			}
			else if (op == "MODULO") {
				out_val = 'mod($val1, $val2)';
			}
			else if (op == "PING-PONG") {
				out_val = '(($val2 != 0.0) ? abs(fract(($val1 - $val2) / ($val2 * 2.0)) * $val2 * 2.0 - $val2) : 0.0)';
			}
			else if (op == "SINE") {
				out_val = 'sin($val1)';
			}
			else if (op == "COSINE") {
				out_val = 'cos($val1)';
			}
			else if (op == "TANGENT") {
				out_val = 'tan($val1)';
			}
			else if (op == "ARCSINE") {
				out_val = 'asin($val1)';
			}
			else if (op == "ARCCOSINE") {
				out_val = 'acos($val1)';
			}
			else if (op == "ARCTANGENT") {
				out_val = 'atan($val1)';
			}
			else if (op == "ARCTAN2") {
				out_val = 'atan2($val1, $val2)';
			}
			else if (op == "HYPERBOLIC_SINE") {
				out_val = 'sinh($val1)';
			}
			else if (op == "HYPERBOLIC_COSINE") {
				out_val = 'cosh($val1)';
			}
			else if (op == "HYPERBOLIC_TANGENT") {
				out_val = 'tanh($val1)';
			}
			else if (op == "TO_RADIANS") {
				out_val = 'radians($val1)';
			}
			else if (op == "TO_DEGREES") {
				out_val = 'degrees($val1)';
			}
			if (use_clamp) {
				return 'clamp($out_val, 0.0, 1.0)';
			}
			else {
				return out_val;
			}
		}
		else if (node.type == "SCRIPT_CPU") {
			if (script_links == null) script_links = [];
			var script = node.buttons[0].default_value;
			var link = node_name(node);
			script_links.set(link, script);
			curshader.add_uniform("float " + link, "_" + link);
			return link;
		}
		else if (node.type == "SHADER_GPU") {
			var shader = node.buttons[0].default_value;
			return shader == "" ? "0.0" : shader;
		}
		else if (node.type == "RGBTOBW") {
			var col = parse_vector_input(node.inputs[0]);
			return '((($col.r * 0.3 + $col.g * 0.59 + $col.b * 0.11) / 3.0) * 2.5)';
		}
		else if (node.type == "SEPHSV") {
			curshader.add_function(ShaderFunctions.str_hue_sat);
			var col = parse_vector_input(node.inputs[0]);
			if (socket == node.outputs[0]) {
				return 'rgb_to_hsv($col).r';
			}
			else if (socket == node.outputs[1]) {
				return 'rgb_to_hsv($col).g';
			}
			else if (socket == node.outputs[2]) {
				return 'rgb_to_hsv($col).b';
			}
		}
		else if (node.type == "SEPRGB") {
			var col = parse_vector_input(node.inputs[0]);
			if (socket == node.outputs[0]) {
				return '$col.r';
			}
			else if (socket == node.outputs[1]) {
				return '$col.g';
			}
			else if (socket == node.outputs[2]) {
				return '$col.b';
			}
		}
		else if (node.type == "SEPXYZ") {
			var vec = parse_vector_input(node.inputs[0]);
			if (socket == node.outputs[0]) {
				return '$vec.x';
			}
			else if (socket == node.outputs[1]) {
				return '$vec.y';
			}
			else if (socket == node.outputs[2]) {
				return '$vec.z';
			}
		}
		else if (node.type == "VECT_MATH") {
			var vec1 = parse_vector_input(node.inputs[0]);
			var vec2 = parse_vector_input(node.inputs[1]);
			var but = node.buttons[0]; //operation;
			var op: String = but.data[but.default_value].toUpperCase();
			op = op.replace(" ", "_");
			if (op == "DOT_PRODUCT") {
				return 'dot($vec1, $vec2)';
			}
			else if (op == "LENGTH") {
				return 'length($vec1)';
			}
			else if (op == "DISTANCE") {
				return 'distance($vec1, $vec2)';
			}
			else {
				return "0.0";
			}
		}
		else if (node.type == "CLAMP") {
			var val = parse_value_input(node.inputs[0]);
			var min = parse_value_input(node.inputs[1]);
			var max = parse_value_input(node.inputs[2]);
			var but = node.buttons[0]; //operation;
			var op: String = but.data[but.default_value].toUpperCase();
			op = op.replace(" ", "_");

			if (op == "MIN_MAX") {
				return '(clamp($val, $min, $max))';
			}
			else if (op == "RANGE") {
				return '(clamp($val, min($min, $max), max($min, $max)))';
			}
		}
		else if (node.type == "MAPRANGE") {
			var val = parse_value_input(node.inputs[0]);
			var fmin = parse_value_input(node.inputs[1]);
			var fmax = parse_value_input(node.inputs[2]);
			var tmin = parse_value_input(node.inputs[3]);
			var tmax = parse_value_input(node.inputs[4]);

			var use_clamp = node.buttons[0].default_value == true;

			var a = '(($tmin - $tmax) / ($fmin - $fmax))';
			var out_val = '($a * $val + $tmin - $a * $fmin)';
			if (use_clamp) {
				return '(clamp($out_val, $tmin, $tmax))';
			}
			else {
				return out_val;
			}
		}
		else if (customNodes.get(node.type) != null) {
			return customNodes.get(node.type)(node, socket);
		}
		return "0.0";
	}

	static function getCoord(node: TNode): String {
		if (getInputLink(node.inputs[0]) != null) {
			return parse_vector_input(node.inputs[0]);
		}
		else {
			curshader.bposition = true;
			return "bposition";
		}
	}

	static function getGradient(grad: String, co: String): String {
		if (grad == "LINEAR") {
			return '$co.x';
		}
		else if (grad == "QUADRATIC") {
			return "0.0";
		}
		else if (grad == "EASING") {
			return "0.0";
		}
		else if (grad == "DIAGONAL") {
			return '($co.x + $co.y) * 0.5';
		}
		else if (grad == "RADIAL") {
			return 'atan2($co.x, $co.y) / (3.141592 * 2.0) + 0.5';
		}
		else if (grad == "QUADRATIC_SPHERE") {
			return "0.0";
		}
		else { // "SPHERICAL"
			return 'max(1.0 - sqrt($co.x * $co.x + $co.y * $co.y + $co.z * $co.z), 0.0)';
		}
	}

	static function vector_curve(name: String, fac: String, points: Array<kha.arrays.Float32Array>): String {
		// Write Ys array
		var ys_var = name + "_ys";
		var num = points.length;
		curshader.write('float $ys_var[$num];'); // TODO: Make const
		for (i in 0...num) {
			curshader.write('$ys_var[$i] = ${points[i][1]};');
		}
		// Get index
		var fac_var = name + "_fac";
		curshader.write('float $fac_var = $fac;');
		var index = "0";
		for (i in 1...num) {
			index += ' + ($fac_var > ${points[i][0]} ? 1 : 0)';
		}
		// Write index
		var index_var = name + "_i";
		curshader.write('int $index_var = $index;');
		// Linear
		// Write Xs array
		var facs_var = name + "_xs";
		curshader.write('float $facs_var[$num];'); // TODO: Make const
		for (i in 0...num) {
			curshader.write('$facs_var[$i] = ${points[i][0]};');
		}
		// Map vector
		return 'mix($ys_var[$index_var], $ys_var[$index_var + 1], ($fac_var - $facs_var[$index_var]) * (1.0 / ($facs_var[$index_var + 1] - $facs_var[$index_var])))';
	}

	static function res_var_name(node: TNode, socket: TNodeSocket): String {
		return node_name(node) + "_" + safesrc(socket.name) + "_res";
	}

	static var parsedMap = new Map<String, String>();
	static var textureMap = new Map<String, String>();

	static function write_result(l: TNodeLink): String {
		var from_node = getNode(l.from_id);
		var from_socket = from_node.outputs[l.from_socket];
		var res_var = res_var_name(from_node, from_socket);
		var st = from_socket.type;
		if (parsed.indexOf(res_var) < 0) {
			parsed.push(res_var);
			if (st == "RGB" || st == "RGBA" || st == "VECTOR") {
				var res = parse_vector(from_node, from_socket);
				if (res == null) {
					return null;
				}
				parsedMap.set(res_var, res);
				curshader.write('vec3 $res_var = $res;');
			}
			else if (st == "VALUE") {
				var res = parse_value(from_node, from_socket);
				if (res == null) {
					return null;
				}
				parsedMap.set(res_var, res);
				curshader.write('float $res_var = $res;');
			}
		}
		return res_var;
	}

	static function store_var_name(node: TNode): String {
		return node_name(node) + "_store";
	}

	static function texture_store(node: TNode, tex: TBindTexture, tex_name: String, color_space: Int): String {
		matcon.bind_textures.push(tex);
		curshader.context.add_elem("tex", "short2norm");
		curshader.add_uniform("sampler2D " + tex_name);
		var uv_name = "";
		if (getInputLink(node.inputs[0]) != null) {
			uv_name = parse_vector_input(node.inputs[0]);
		}
		else {
			uv_name = tex_coord;
		}
		var tex_store = store_var_name(node);

		if (sample_keep_aspect) {
			curshader.write('vec2 ${tex_store}_size = vec2(textureSize($tex_name, 0));');
			curshader.write('float ${tex_store}_ax = ${tex_store}_size.x / ${tex_store}_size.y;');
			curshader.write('float ${tex_store}_ay = ${tex_store}_size.y / ${tex_store}_size.x;');
			curshader.write('vec2 ${tex_store}_uv = ((${uv_name}.xy / ${sample_uv_scale} - vec2(0.5, 0.5)) * vec2(max(${tex_store}_ay, 1.0), max(${tex_store}_ax, 1.0))) + vec2(0.5, 0.5);');
			curshader.write('if (${tex_store}_uv.x < 0.0 || ${tex_store}_uv.y < 0.0 || ${tex_store}_uv.x > 1.0 || ${tex_store}_uv.y > 1.0) discard;');
			curshader.write('${tex_store}_uv *= ${sample_uv_scale};');
			uv_name = '${tex_store}_uv';
		}

		if (triplanar) {
			curshader.write('vec4 $tex_store = vec4(0.0, 0.0, 0.0, 0.0);');
			curshader.write('if (texCoordBlend.x > 0) $tex_store += texture($tex_name, ${uv_name}.xy) * texCoordBlend.x;');
			curshader.write('if (texCoordBlend.y > 0) $tex_store += texture($tex_name, ${uv_name}1.xy) * texCoordBlend.y;');
			curshader.write('if (texCoordBlend.z > 0) $tex_store += texture($tex_name, ${uv_name}2.xy) * texCoordBlend.z;');
		}
		else {
			if (curshader == frag) {
				textureMap.set(tex_store, 'texture($tex_name, $uv_name.xy)');
				curshader.write('vec4 $tex_store = texture($tex_name, $uv_name.xy);');
			}
			else {
				textureMap.set(tex_store, 'textureLod($tex_name, $uv_name.xy, 0.0)');
				curshader.write('vec4 $tex_store = textureLod($tex_name, $uv_name.xy, 0.0);');
			}
			if (!tex.file.endsWith(".jpg")) { // Pre-mult alpha
				curshader.write('$tex_store.rgb *= $tex_store.a;');
			}
		}

		if (transform_color_space) {
			// Base color socket auto-converts from sRGB to linear
			if (color_space == SpaceLinear && parsing_basecolor) { // Linear to sRGB
				curshader.write('$tex_store.rgb = pow($tex_store.rgb, vec3(2.2, 2.2, 2.2));');
			}
			else if (color_space == SpaceSRGB && !parsing_basecolor) { // sRGB to linear
				curshader.write('$tex_store.rgb = pow($tex_store.rgb, vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));');
			}
			else if (color_space == SpaceDirectXNormalMap) { // DirectX normal map to OpenGL normal map
				curshader.write('$tex_store.y = 1.0 - $tex_store.y;');
			}
		}
		return tex_store;
	}

	public static inline function vec1(v: Float): String {
		#if krom_android
		return 'float($v)';
		#else
		return '$v';
		#end
	}

	public static inline function vec3(v: Array<Float>): String {
		#if krom_android
		return 'vec3(float(${v[0]}), float(${v[1]}), float(${v[2]}))';
		#else
		return 'vec3(${v[0]}, ${v[1]}, ${v[2]})';
		#end
	}

	public static inline function to_vec3(s: String): String {
		#if (kha_direct3d11 || kha_direct3d12)
		return '($s).xxx';
		#else
		return 'vec3($s)';
		#end
	}

	public static function node_by_type(nodes: Array<TNode>, ntype: String): TNode {
		for (n in nodes) if (n.type == ntype) return n;
		return null;
	}

	static function socket_index(node: TNode, socket: TNodeSocket): Int {
		for (i in 0...node.outputs.length) if (node.outputs[i] == socket) return i;
		return -1;
	}

	public static function node_name(node: TNode, _parents: Array<TNode> = null): String {
		if (_parents == null) _parents = parents;
		var s = node.name;
		for (p in _parents) s = p.name + p.id + '_' + s;
		s = safesrc(s) + node.id;
		return s;
	}

	static function safesrc(s: String): String {
		for (i in 0...s.length) {
			var code = s.charCodeAt(i);
			var letter = (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
			var digit = code >= 48 && code <= 57;
			if (!letter && !digit) s = s.replace(s.charAt(i), "_");
			if (i == 0 && digit) s = "_" + s;
		}
		#if kha_opengl
		while (s.indexOf("__") >= 0) s = s.replace("__", "_");
		#end
		return s;
	}

	public static function enumData(s: String): String {
		for (a in Project.assets) if (a.name == s) return a.file;
		return "";
	}

	static function make_texture(image_node: TNode, tex_name: String, matname: String = null): TBindTexture {

		var filepath = enumData(App.enumTexts(image_node.type)[image_node.buttons[0].default_value]);
		if (filepath == "" || filepath.indexOf(".") == -1) {
			return null;
		}

		var tex: TBindTexture = {
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

	static function is_pow(num: Int): Bool {
		return ((num & (num - 1)) == 0) && num != 0;
	}

	static function asset_path(s: String): String {
		return s;
	}

	static function extract_filename(s: String): String {
		var ar = s.split(".");
		return ar[ar.length - 2] + "." + ar[ar.length - 1];
	}

	static function safestr(s: String): String {
		return s;
	}
}

typedef TShaderOut = {
	var out_basecol: String;
	var out_roughness: String;
	var out_metallic: String;
	var out_occlusion: String;
	var out_opacity: String;
	var out_height: String;
	var out_emission: String;
	var out_subsurface: String;
}

@:enum abstract ColorSpace(Int) from Int to Int {
	var SpaceAuto = 0; // sRGB for base color, otherwise linear
	var SpaceLinear = 1;
	var SpaceSRGB = 2;
	var SpaceDirectXNormalMap = 3;
}
