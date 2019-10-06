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
package arm.nodes;

import zui.Nodes;
import iron.data.SceneFormat;
import arm.nodes.CyclesShader;
using StringTools;

class Cycles {

	static var con:CyclesShaderContext;
	static var vert:CyclesShader;
	static var frag:CyclesShader;
	static var geom:CyclesShader;
	static var tesc:CyclesShader;
	static var tese:CyclesShader;
	static var curshader:CyclesShader;
	static var matcon:TMaterialContext;
	static var parsed:Array<String>;

	static var nodes:Array<TNode>;
	static var links:Array<TNodeLink>;

	static var parsing_disp:Bool;
	static var normal_written:Bool; // Normal socket is linked on shader node - overwrite fs normal
	static var cotangentFrameWritten:Bool;
	static var sample_bump:Bool;
	static var sample_bump_res:String;

	public static var parse_surface = true;
	public static var parse_opacity = true;
	public static var parse_height = false;
	public static var parse_height_as_channel = false;
	public static var parse_emission = false;
	public static var parse_subsurface = false;
	public static var triplanar = false; // Sample using texCoord/1/2 & texCoordBlend

	public static var arm_export_tangents = true;
	public static var out_normaltan:String; // Raw tangent space normal parsed from normal map

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
		var ls:Array<TNodeLink> = null;
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

	public static function parse(canvas:TNodeCanvas, _con:CyclesShaderContext, _vert:CyclesShader, _frag:CyclesShader, _geom:CyclesShader, _tesc:CyclesShader, _tese:CyclesShader, _matcon:TMaterialContext, _parse_displacement = false):TShaderOut {
		nodes = canvas.nodes;
		links = canvas.links;

		parsed = [];
		con = _con;
		vert = _vert;
		frag = _frag;
		geom = _geom;
		tesc = _tesc;
		tese = _tese;
		curshader = frag;
		matcon = _matcon;

		parsing_disp = false;
		normal_written = false;
		cotangentFrameWritten = false;
		sample_bump = false;
		sample_bump_res = '';

		out_normaltan = 'vec3(0.5, 0.5, 1.0)';

		var output_node = node_by_type(nodes, 'OUTPUT_MATERIAL');
		if (output_node != null) {
			return parse_output(output_node);
		}
		output_node = node_by_type(nodes, 'OUTPUT_MATERIAL_PBR');
		if (output_node != null) {
			return parse_output_pbr(output_node);
		}
		return null;
	}

	public static function finalize(con:CyclesShaderContext) {
		var vert = con.vert;
		var frag = con.frag;

		if (frag.dotNV) { frag.vVec = true; frag.n = true; }
		if (frag.vVec) vert.wposition = true;

		if (frag.bposition) {
			if (triplanar) {
				frag.write_attrib('vec3 bposition = vec3(
					texCoord1.x * texCoordBlend.y + texCoord2.x * texCoordBlend.z,
					texCoord.x * texCoordBlend.x + texCoord2.y * texCoordBlend.z,
					texCoord.y * texCoordBlend.x + texCoord1.y * texCoordBlend.y);');
			}
			else if (frag.ndcpos) {
				vert.add_out('vec3 bposition');
				vert.write('bposition = (ndc.xyz / ndc.w);');
			}
			else {
				vert.add_out('vec3 bposition');
				vert.add_uniform('vec3 dim', '_dim');
				vert.add_uniform('vec3 hdim', '_halfDim');
				vert.write_attrib('bposition = (pos.xyz + hdim) / dim;');
			}
		}
		if (frag.wposition) {
			vert.add_uniform('mat4 W', '_worldMatrix');
			vert.add_out('vec3 wposition');
			vert.write_attrib('wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;');
		}
		else if (vert.wposition) {
			vert.add_uniform('mat4 W', '_worldMatrix');
			vert.write_attrib('vec3 wposition = vec4(mul(vec4(pos.xyz, 1.0), W)).xyz;');
		}
		if (frag.vposition) {
			vert.add_uniform('mat4 WV', '_worldViewMatrix');
			vert.add_out('vec3 vposition');
			vert.write_attrib('vposition = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz;');
		}
		if (frag.mposition) {
			vert.add_out('vec3 mposition');
			if (frag.ndcpos) {
				vert.write('mposition = (ndc.xyz / ndc.w);');
			}
			else {
				vert.write_attrib('mposition = pos.xyz;');
			}
		}
		if (frag.wtangent) {
			con.add_elem('tang', 'short4norm');
			vert.add_uniform('mat3 N', '_normalMatrix');
			vert.add_out('vec3 wtangent');
			vert.write_attrib('wtangent = normalize(mul(tang, N));');
		}
		if (frag.vVecCam) {
			vert.add_uniform('mat4 WV', '_worldViewMatrix');
			vert.add_out('vec3 eyeDirCam');
			vert.write_attrib('eyeDirCam = vec4(mul(vec4(pos.xyz, 1.0), WV)).xyz; eyeDirCam.z *= -1.0;');
			frag.write_attrib('vec3 vVecCam = normalize(eyeDirCam);');
		}
		if (frag.vVec) {
			vert.add_uniform('vec3 eye', '_cameraPosition');
			vert.add_out('vec3 eyeDir');
			vert.write_attrib('eyeDir = eye - wposition;');
			frag.write_attrib('vec3 vVec = normalize(eyeDir);');
		}
		if (frag.n) {
			vert.add_uniform('mat3 N', '_normalMatrix');
			vert.add_out('vec3 wnormal');
			vert.write_attrib('wnormal = mul(vec3(nor.xy, pos.w), N);');
			frag.write_attrib('vec3 n = normalize(wnormal);');
		}
		else if (vert.n) {
			vert.add_uniform('mat3 N', '_normalMatrix');
			vert.write_attrib('vec3 wnormal = normalize(mul(vec3(nor.xy, pos.w), N));');
		}
		if (frag.nAttr) {
			vert.add_out('vec3 nAttr');
			vert.write_attrib('nAttr = vec3(nor.xy, pos.w);');
		}
		if (frag.dotNV) {
			frag.write_attrib('float dotNV = max(dot(n, vVec), 0.0);');
		}
		if (frag.wvpposition) {
			vert.add_out('vec4 wvpposition');
			vert.write_end('wvpposition = gl_Position;');
		}
	}

	static function parse_output(node:TNode):TShaderOut {
		if (parse_surface || parse_opacity) {
			return parse_shader_input(node.inputs[0]);
		}
		return null;
		// Parse volume, displacement..
	}

	static function parse_output_pbr(node:TNode):TShaderOut {
		if (parse_surface || parse_opacity) {
			return parse_shader(node, null);
		}
		return null;
		// Parse volume, displacement..
	}

	static function parse_group(node:TNode, socket:TNodeSocket) { // Entering group
	//     index = socket_index(node, socket)
	//     output_node = node_by_type(node.node_tree.nodes, 'GROUP_OUTPUT')
	//     if output_node == None:
	//         return
	//     inp = output_node.inputs[index]
	//     parents.push(node)
	//     out_group = parse_input(inp)
	//     parents.pop()
	//     return out_group
	}

	static function parse_group_input(node:TNode, socket:TNodeSocket) {
	//     index = socket_index(node, socket)
	//     parent = parents.pop() # Leaving group
	//     inp = parent.inputs[index]
	//     res = parse_input(inp)
	//     parents.push(parent) # Return to group
	//     return res
	}

	static function parse_input(inp:TNodeSocket):Dynamic {
		if (inp.type == 'SHADER')
			return parse_shader_input(inp);
		else if (inp.type == 'RGB')
			return parse_vector_input(inp);
		else if (inp.type == 'RGBA')
			return parse_vector_input(inp);
		else if (inp.type == 'VECTOR')
			return parse_vector_input(inp);
		else if (inp.type == 'VALUE')
			return parse_value_input(inp);
		return null;
	}

	static function parse_shader_input(inp:TNodeSocket):TShaderOut {
		var l = getInputLink(inp);
		if (l != null) {
			var from_node = getNode(l.from_id);
			if (from_node.type == 'REROUTE')
				return parse_shader_input(from_node.inputs[0]);
			return parse_shader(from_node, from_node.outputs[l.from_socket]);
		}
		else {
			return {
				out_basecol: 'vec3(0.8, 0.8, 0.8)',
				out_roughness: '0.0',
				out_metallic: '0.0',
				out_occlusion: '1.0',
				out_opacity: '1.0',
				out_height: '0.0',
				out_emission: '0.0',
				out_subsurface: '0.0'
			};
		}
	}

	static function parse_shader(node:TNode, socket:TNodeSocket):TShaderOut {
		var sout:TShaderOut = {
			out_basecol: 'vec3(0.8, 0.8, 0.8)',
			out_roughness: '0.0',
			out_metallic: '0.0',
			out_occlusion: '1.0',
			out_opacity: '1.0',
			out_height: '0.0',
			out_emission: '0.0',
			out_subsurface: '0.0'
		}

		// if (node.type == 'GROUP') {
		if (node.type == 'Armory PBR' || node.type == 'OUTPUT_MATERIAL_PBR') {
			if (parse_surface) {
				// Normal - parsed first to retrieve uv coords
				parse_normal_map_color_input(node.inputs[5]);
				// Base color
				sout.out_basecol = parse_vector_input(node.inputs[0]);
				// Occlusion
				sout.out_occlusion = parse_value_input(node.inputs[2]);
				// Roughness
				sout.out_roughness = parse_value_input(node.inputs[3]);
				// Metallic
				sout.out_metallic = parse_value_input(node.inputs[4]);
				// Emission
				// if (isInputLinked(node.inputs[6]) || node.inputs[6].default_value != 0.0):
				if (parse_emission) {
					sout.out_emission = parse_value_input(node.inputs[6]);
					// sout.out_basecol = '(${sout.out_basecol} + ${to_vec3(sout.out_emission)})';
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
		// else:
			// return parse_group(node, socket)

		// else if (node.type == 'GROUP_INPUT') {
			// return parse_group_input(node, socket);
		// }

		else if (node.type == 'MIX_SHADER') {
			// var prefix = node.inputs[0].is_linked ? '' : 'const ';
			var prefix = '';
			var fac = parse_value_input(node.inputs[0]);
			var fac_var = node_name(node) + '_fac';
			var fac_inv_var = node_name(node) + '_fac_inv';
			curshader.write(prefix + 'float $fac_var = $fac;');
			curshader.write(prefix + 'float $fac_inv_var = 1.0 - $fac_var;');
			var sout1 = parse_shader_input(node.inputs[1]);
			var sout2 = parse_shader_input(node.inputs[2]);
			var bc1 = sout1.out_basecol;
			var bc2 = sout2.out_basecol;
			var rough1 = sout1.out_roughness;
			var rough2 = sout2.out_roughness;
			var met1 = sout1.out_metallic;
			var met2 = sout2.out_metallic;
			var occ1 = sout1.out_occlusion;
			var occ2 = sout2.out_occlusion;
			if (parse_surface) {
				sout.out_basecol = '($bc1 * $fac_inv_var + $bc2 * $fac_var)';
				sout.out_roughness = '($rough1 * $fac_inv_var + $rough2 * $fac_var)';
				sout.out_metallic = '($met1 * $fac_inv_var + $met2 * $fac_var)';
				sout.out_occlusion = '($occ1 * $fac_inv_var + $occ2 * $fac_var)';
			}
			if (parse_opacity) {
				// out_opacity = '({0} * {3} + {1} * {2})'.format(opac1, opac2, fac_var, fac_inv_var)
			}
		}

		else if (node.type == 'ADD_SHADER') {
		//     bc1, rough1, met1, occ1, opac1 = parse_shader_input(node.inputs[0])
		//     bc2, rough2, met2, occ2, opac2 = parse_shader_input(node.inputs[1])
		//     if parse_surface:
		//         out_basecol = '({0} + {1})'.format(bc1, bc2)
		//         out_roughness = '({0} * 0.5 + {1} * 0.5)'.format(rough1, rough2)
		//         out_metallic = '({0} * 0.5 + {1} * 0.5)'.format(met1, met2)
		//         out_occlusion = '({0} * 0.5 + {1} * 0.5)'.format(occ1, occ2)
		//     if parse_opacity:
		//         out_opacity = '({0} * 0.5 + {1} * 0.5)'.format(opac1, opac2)
		}

		else if (node.type == 'BSDF_PRINCIPLED') {
			//if parse_surface:
			write_normal(node.inputs[16]);
			sout.out_basecol = parse_vector_input(node.inputs[0]);
			// subsurface = parse_vector_input(node.inputs[1])
			// subsurface_radius = parse_vector_input(node.inputs[2])
			// subsurface_color = parse_vector_input(node.inputs[3])
			sout.out_metallic = parse_value_input(node.inputs[4]);
			// specular = parse_vector_input(node.inputs[5])
			// specular_tint = parse_vector_input(node.inputs[6])
			sout.out_roughness = parse_value_input(node.inputs[7]);
			// aniso = parse_vector_input(node.inputs[8])
			// aniso_rot = parse_vector_input(node.inputs[9])
			// sheen = parse_vector_input(node.inputs[10])
			// sheen_tint = parse_vector_input(node.inputs[11])
			// clearcoat = parse_vector_input(node.inputs[12])
			// clearcoat_rough = parse_vector_input(node.inputs[13])
			// ior = parse_vector_input(node.inputs[14])
			// transmission = parse_vector_input(node.inputs[15])
		}

		else if (node.type == 'BSDF_DIFFUSE') {
			//if parse_surface:
			write_normal(node.inputs[2]);
			sout.out_basecol = parse_vector_input(node.inputs[0]);
			sout.out_roughness = parse_value_input(node.inputs[1]);
		}

		else if (node.type == 'BSDF_GLOSSY') {
			// if parse_surface:
			write_normal(node.inputs[2]);
			sout.out_basecol = parse_vector_input(node.inputs[0]);
			sout.out_roughness = parse_value_input(node.inputs[1]);
			sout.out_metallic = '1.0';
		}

		return sout;
	}

	// static function parse_displacement_input() {}

	static function parse_vector_input(inp:TNodeSocket):String {
		var l = getInputLink(inp);
		if (l != null) {
			var from_node = getNode(l.from_id);
			if (from_node.type == 'REROUTE')
				return parse_vector_input(from_node.inputs[0]);

			var res_var = write_result(l);
			var st = from_node.outputs[l.from_socket].type;
			if (st == 'RGB' || st == 'RGBA' || st == 'VECTOR')
				return res_var;
			else //# VALUE
				return to_vec3(res_var);
		}
		else {
			if (inp.type == 'VALUE') //# Unlinked reroute
				return vec3([0.0, 0.0, 0.0]);
			else {
				// if mat_batch() && inp.is_uniform:
					// return to_uniform(inp);
				// else
					return vec3(inp.default_value);
			}
		}
	}

	static function parse_vector(node:TNode, socket:TNodeSocket):String {

		// RGB

		//     if (node.type == 'GROUP')
		//         return parse_group(node, socket)

		//     else if (node.type == 'GROUP_INPUT')
		//         return parse_group_input(node, socket)

		if (node.type == 'ATTRIBUTE') {
			if (socket == node.outputs[0]) { // Color
				curshader.context.add_elem('col', 'short4norm'); // Vcols only for now
				// return 'vcolor';
				return 'vec3(0.0, 0.0, 0.0)';
			}
			else { // Vector
				curshader.context.add_elem('tex', 'short2norm'); // UVMaps only for now
				// mat = mat_get_material()
				// mat_users = mat_get_material_users()
				// if mat_users != None and mat in mat_users:
					// mat_user = mat_users[mat][0]
					// if hasattr(mat_user.data, 'uv_layers'): # No uvlayers for Curve
						// lays = mat_user.data.uv_layers
						// # Second uvmap referenced
						// if len(lays) > 1 and node.attribute_name == lays[1].name:
							// con.add_elem('tex1', 'short2norm')
							// return 'vec3(texCoord1.x, 1.0 - texCoord1.y, 0.0)'
				return 'vec3(texCoord.x, 1.0 - texCoord.y, 0.0)';
			}
		}

		else if (node.type == 'RGB') {
			return vec3(socket.default_value);
		}

		else if (node.type == 'TEX_BRICK') {
			curshader.add_function(CyclesFunctions.str_tex_brick);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var col1 = parse_vector_input(node.inputs[1]);
			var col2 = parse_vector_input(node.inputs[2]);
			var col3 = parse_vector_input(node.inputs[3]);
			var scale = parse_value_input(node.inputs[4]);
			var res = 'tex_brick($co * $scale, $col1, $col2, $col3)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_CHECKER') {
			curshader.add_function(CyclesFunctions.str_tex_checker);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var col1 = parse_vector_input(node.inputs[1]);
			var col2 = parse_vector_input(node.inputs[2]);
			var scale = parse_value_input(node.inputs[3]);
			var res = 'tex_checker($co, $col1, $col2, $scale)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

	//     else if (node.type == 'TEX_ENVIRONMENT') {
	//         // Pass through
	//         return vec3([0.0, 0.0, 0.0])
	// }

		else if (node.type == 'TEX_GRADIENT') {
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var but = node.buttons[0]; //gradient_type;
			var grad:String = but.data[but.default_value].toUpperCase();
			grad = grad.replace(" ", "_");
			var f = '';
			if (grad == 'LINEAR') {
				f = '$co.x';
			}
			else if (grad == 'QUADRATIC') {
				f = '0.0';
			}
			else if (grad == 'EASING') {
				f = '0.0';
			}
			else if (grad == 'DIAGONAL') {
				f = '($co.x + $co.y) * 0.5';
			}
			else if (grad == 'RADIAL') {
				f = 'atan($co.y, $co.x) / (3.141592 * 2.0) + 0.5';
			}
			else if (grad == 'QUADRATIC_SPHERE') {
				f = '0.0';
			}
			else if (grad == 'SPHERICAL') {
				f = 'max(1.0 - sqrt($co.x * $co.x + $co.y * $co.y + $co.z * $co.z), 0.0)';
			}
			var res = to_vec3('clamp($f, 0.0, 1.0)');
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_IMAGE') {
			// Already fetched
			if (parsed.indexOf(res_var_name(node, node.outputs[1])) >= 0) { // TODO: node.outputs[0]
				var varname = store_var_name(node);
				return '$varname.rgb';
			}
			var tex_name = node_name(node);
			var tex = make_texture(node, tex_name);
			if (tex != null) {
				var to_linear = node.buttons[1].default_value == 1; // srgb to linear
				var texstore = texture_store(node, tex, tex_name, to_linear);
				return '$texstore.rgb';
			}
			// else if (node.image == null) { // Empty texture
				// tex = {};
				// tex['name'] = tex_name;
				// tex['file'] = '';
				// return '{0}.rgb'.format(texture_store(node, tex, tex_name, True));
			// }
			else {
				var tex_store = store_var_name(node); // Pink color for missing texture
				curshader.write('vec4 $tex_store = vec4(1.0, 0.0, 1.0, 1.0);');
				return '$tex_store.rgb';
			}
		}

		else if (node.type == 'TEX_MAGIC') {
			curshader.add_function(CyclesFunctions.str_tex_magic);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_magic($co * $scale * 4.0)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_MUSGRAVE') {
			curshader.add_function(CyclesFunctions.str_tex_musgrave);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			// detail = parse_value_input(node.inputs[2])
			// distortion = parse_value_input(node.inputs[3])
			var res = to_vec3('tex_musgrave_f($co * $scale * 0.5)');
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_NOISE') {
			curshader.add_function(CyclesFunctions.str_tex_noise);
			curshader.add_uniform("sampler2D snoise256", "$noise256.png");
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
	//         # detail = parse_value_input(node.inputs[2])
	//         # distortion = parse_value_input(node.inputs[3])
	//         # Slow..
			var res = 'vec3(tex_noise($co * $scale), tex_noise($co * $scale + 0.33), tex_noise($co * $scale + 0.66))';
			if (sample_bump) write_bump(node, res);
			return res;
		}
	//     elif node.type == 'TEX_POINTDENSITY':
	//         # Pass through
	//         return vec3([0.0, 0.0, 0.0])

	//     elif node.type == 'TEX_SKY':
	//         # Pass through
	//         return vec3([0.0, 0.0, 0.0])

		else if (node.type == 'TEX_VORONOI') {
			curshader.add_function(CyclesFunctions.str_tex_voronoi);
			curshader.add_uniform("sampler2D snoise256", "$noise256.png");
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			var but = node.buttons[0]; //coloring;
			var coloring:String = but.data[but.default_value].toUpperCase();
			coloring = coloring.replace(" ", "_");
			var res = '';
			if (coloring == 'INTENSITY') {
				res = to_vec3('tex_voronoi($co * $scale).a');
			}
			else { // Cells
				res = 'tex_voronoi($co * $scale).rgb';
			}
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_WAVE') {
			curshader.add_function(CyclesFunctions.str_tex_wave);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			var res = to_vec3('tex_wave_f($co * $scale)');
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'BRIGHTCONTRAST') {
			var out_col = parse_vector_input(node.inputs[0]);
			var bright = parse_value_input(node.inputs[1]);
			var contr = parse_value_input(node.inputs[2]);
			curshader.add_function(CyclesFunctions.str_brightcontrast);
			return 'brightcontrast($out_col, $bright, $contr)';
		}
		else if (node.type == 'GAMMA') {
			var out_col = parse_vector_input(node.inputs[0]);
			var gamma = parse_value_input(node.inputs[1]);
			return 'pow($out_col, ' + to_vec3('$gamma') + ')';
		}

		else if (node.type == 'HUE_SAT') {
			curshader.add_function(CyclesFunctions.str_hue_sat);
			var hue = parse_value_input(node.inputs[0]);
			var sat = parse_value_input(node.inputs[1]);
			var val = parse_value_input(node.inputs[2]);
			var fac = parse_value_input(node.inputs[3]);
			var col = parse_vector_input(node.inputs[4]);
			return 'hue_sat($col, vec4($hue-0.5, $sat, $val, 1.0-$fac))';
		}

		else if (node.type == 'INVERT') {
			var fac = parse_value_input(node.inputs[0]);
			var out_col = parse_vector_input(node.inputs[1]);
			return 'mix($out_col, vec3(1.0, 1.0, 1.0) - ($out_col), $fac)';
		}

		else if (node.type == 'MIX_RGB') {
			var fac = parse_value_input(node.inputs[0]);
			var fac_var = node_name(node) + '_fac';
			curshader.write('float $fac_var = $fac;');
			var col1 = parse_vector_input(node.inputs[1]);
			var col2 = parse_vector_input(node.inputs[2]);
			var but = node.buttons[0]; // blend_type
			var blend:String = but.data[but.default_value].toUpperCase();
			blend = blend.replace(" ", "_");
			but = node.buttons[1]; // use_clamp
			var use_clamp = but.default_value == "true";
			var out_col = '';
			if (blend == 'MIX') {
				out_col = 'mix($col1, $col2, $fac_var)';
			}
			//
			else if (blend == 'DARKEN') {
				out_col = 'min($col1, $col2 * $fac_var)';
			}
			else if (blend == 'MULTIPLY') {
				out_col = 'mix($col1, $col1 * $col2, $fac_var)';
			}
			else if (blend == 'BURN') {
				out_col = 'mix($col1, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $col1) / $col2, $fac_var)';
			}
			//
			else if (blend == 'LIGHTEN') {
				out_col = 'max($col1, $col2 * $fac_var)';
			}
			else if (blend == 'SCREEN') {
				out_col = '(vec3(1.0, 1.0, 1.0) - (' + to_vec3('1.0 - $fac_var') + ' + $fac_var * (vec3(1.0, 1.0, 1.0) - $col2)) * (vec3(1.0, 1.0, 1.0) - $col1))';
			}
			else if (blend == 'DODGE') {
				out_col = 'mix($col1, $col1 / (vec3(1.0, 1.0, 1.0) - $col2), $fac_var)';
			}
			else if (blend == 'ADD') {
				out_col = 'mix($col1, $col1 + $col2, $fac_var)';
			}
			//
			else if (blend == 'OVERLAY') {
				out_col = '($col1 < vec3(0.5, 0.5, 0.5) ? vec3(2.0, 2.0, 2.0) * $col1 * $col2 : vec3(1.0, 1.0, 1.0) - vec3(2.0, 2.0, 2.0) * (vec3(1.0, 1.0, 1.0) - $col2) * (vec3(1.0, 1.0, 1.0) - $col1))';
			}
			else if (blend == 'SOFT_LIGHT') {
				out_col = '((1.0 - $fac_var) * $col1 + $fac_var * ((vec3(1.0, 1.0, 1.0) - $col1) * $col2 * $col1 + $col1 * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $col2) * (vec3(1.0, 1.0, 1.0) - $col1))))';
			}
			else if (blend == 'LINEAR_LIGHT') {
				out_col = '($col1 + $fac_var * (vec3(2.0, 2.0, 2.0) * ($col2 - vec3(0.5, 0.5, 0.5))))';
			}
			//
			else if (blend == 'DIFFERENCE') {
				out_col = 'mix($col1, abs($col1 - $col2), $fac_var)';
			}
			else if (blend == 'SUBTRACT') {
				out_col = 'mix($col1, $col1 - $col2, $fac_var)';
			}
			else if (blend == 'DIVIDE') {
				out_col = '(' + to_vec3('(1.0 - $fac_var) * $col1 + $fac_var * $col1 / $col2') + ')';
			}
			//
			else if (blend == 'HUE') {
				curshader.add_function(CyclesFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col2).r, rgb_to_hsv($col1).g, rgb_to_hsv($col1).b)), $fac_var)';
			}
			else if (blend == 'SATURATION') {
				curshader.add_function(CyclesFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col1).r, rgb_to_hsv($col2).g, rgb_to_hsv($col1).b)), $fac_var)';
			}
			else if (blend == 'COLOR') {
				curshader.add_function(CyclesFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col2).r, rgb_to_hsv($col2).g, rgb_to_hsv($col1).b)), $fac_var)';
			}
			else if (blend == 'VALUE') {
				curshader.add_function(CyclesFunctions.str_hue_sat);
				out_col = 'mix($col1, hsv_to_rgb(vec3(rgb_to_hsv($col1).r, rgb_to_hsv($col1).g, rgb_to_hsv($col2).b)), $fac_var)';
			}
			//
			if (use_clamp) return 'clamp($out_col, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0))';
			else return out_col;
		}

	//     elif node.type == 'BLACKBODY':
	//         # Pass constant
	//         return vec3([0.84, 0.38, 0.0])

		else if (node.type == 'VALTORGB') { // ColorRamp
			var fac = parse_value_input(node.inputs[0]);
			var interp = node.buttons[0].data == 0 ? 'LINEAR' : 'CONSTANT';
			var elems = node.buttons[0].default_value;
			if (elems.length == 1) {
				return vec3(elems[0]);
			}
			// Write cols array
			var cols_var = node_name(node) + '_cols';
			curshader.write('vec3 $cols_var[${elems.length}];'); // TODO: Make const
			for (i in 0...elems.length) {
				curshader.write('$cols_var[$i] = ${vec3(elems[i])};');
			}
			// Get index
			var fac_var = node_name(node) + '_fac';
			curshader.write('float $fac_var = $fac;');
			var index = '0';
			for (i in 1...elems.length)
				index += ' + ($fac_var > ${elems[i][4]} ? 1 : 0)';
			// Write index
			var index_var = node_name(node) + '_i';
			curshader.write('int $index_var = $index;');
			if (interp == 'CONSTANT') {
				return '$cols_var[$index_var]';
			}
			else { // Linear
				// Write facs array
				var facs_var = node_name(node) + '_facs';
				curshader.write('float $facs_var[${elems.length}];'); // TODO: Make const
				for (i in 0...elems.length) {
					curshader.write('$facs_var[$i] = ${elems[i][4]};');
				}
				// Mix color
				// float f = (pos - start) * (1.0 / (finish - start))
				return 'mix($cols_var[$index_var], $cols_var[$index_var + 1], ($fac_var - $facs_var[$index_var]) * (1.0 / ($facs_var[$index_var + 1] - $facs_var[$index_var]) ))';
			}
		}

		else if (node.type == 'CURVE_VEC') {
			var fac = parse_value_input(node.inputs[0]);
			var vec = parse_vector_input(node.inputs[1]);
			var curves = node.buttons[0].default_value;
			var name = node_name(node);
			var vc0 = vector_curve(name + '0', vec + '.x', curves[0]);
			var vc1 = vector_curve(name + '1', vec + '.y', curves[1]);
			var vc2 = vector_curve(name + '2', vec + '.z', curves[2]);
			// mapping.curves[0].points[0].handle_type # bezier curve
			return '(vec3($vc0, $vc1, $vc2) * $fac)';
		}

		else if (node.type == 'CURVE_RGB') { // RGB Curves
			var fac = parse_value_input(node.inputs[0]);
			var vec = parse_vector_input(node.inputs[1]);
			var curves = node.buttons[0].default_value;
			var name = node_name(node);
			// mapping.curves[0].points[0].handle_type
			var vc0 = vector_curve(name + '0', vec + '.x', curves[0]);
			var vc1 = vector_curve(name + '1', vec + '.y', curves[1]);
			var vc2 = vector_curve(name + '2', vec + '.z', curves[2]);
			var vc3a = vector_curve(name + '3a', vec + '.x', curves[3]);
			var vc3b = vector_curve(name + '3b', vec + '.y', curves[3]);
			var vc3c = vector_curve(name + '3c', vec + '.z', curves[3]);
			return '(sqrt(vec3($vc0, $vc1, $vc2) * vec3($vc3a, $vc3b, $vc3c)) * $fac)';
		}

		else if (node.type == 'COMBHSV') {
			curshader.add_function(CyclesFunctions.str_hue_sat);
			var h = parse_value_input(node.inputs[0]);
			var s = parse_value_input(node.inputs[1]);
			var v = parse_value_input(node.inputs[2]);
			return 'hsv_to_rgb(vec3($h, $s, $v))';
		}

		else if (node.type == 'COMBRGB') {
			var r = parse_value_input(node.inputs[0]);
			var g = parse_value_input(node.inputs[1]);
			var b = parse_value_input(node.inputs[2]);
			return 'vec3($r, $g, $b)';
		}
		else if (node.type == 'WAVELENGTH') {
			curshader.add_function(CyclesFunctions.str_wavelength_to_rgb);
			var wl = parse_value_input(node.inputs[0]);
			// Roughly map to cycles - 450 to 600 nanometers
			curshader.add_function(CyclesFunctions.str_wavelength_to_rgb);
			return 'wavelength_to_rgb(($wl - 450.0) / 150.0)';
		}

		// VECTOR

		else if (node.type == 'CAMERA') {
			// View Vector in camera space
			curshader.vVecCam = true;
			return 'vVecCam';
		}

		else if (node.type == 'NEW_GEOMETRY') {
			if (socket == node.outputs[0]) { // Position
				curshader.wposition = true;
				return 'wposition';
			}
			else if (socket == node.outputs[1]) { // Normal
				curshader.n = true;
				return 'n';
			}
			else if (socket == node.outputs[2]) { // Tangent
				curshader.wtangent = true;
				return 'wtangent';
			}
			else if (socket == node.outputs[3]) { // True Normal
				curshader.n = true;
				return 'n';
			}
			else if (socket == node.outputs[4]) { // Incoming
				curshader.vVec = true;
				return 'vVec';
			}
			else if (socket == node.outputs[5]) { // Parametric
				curshader.mposition = true;
				return 'mposition';
			}
		}

		// elif node.type == 'HAIR_INFO':
		// 	return 'vec3(0.0, 0.0, 0.0)' # Tangent Normal

		else if (node.type == 'OBJECT_INFO') {
			curshader.wposition = true;
			return 'wposition';
		}

		// elif node.type == 'PARTICLE_INFO':
		// 	if socket == node.outputs[3]: # Location
		// 		return 'vec3(0.0, 0.0, 0.0)'
		// 	elif socket == node.outputs[5]: # Velocity
		// 		return 'vec3(0.0, 0.0, 0.0)'
		// 	elif socket == node.outputs[6]: # Angular Velocity
		// 		return 'vec3(0.0, 0.0, 0.0)'

		else if (node.type == 'TANGENT') {
			curshader.wtangent = true;
			return 'wtangent';
		}

		else if (node.type == 'TEX_COORD') {
			// obj = node.object
			// dupli = node.from_dupli
			if (socket == node.outputs[0]) { // Generated - bounds
				curshader.bposition = true;
				return 'bposition';
			}
			else if (socket == node.outputs[1]) { // Normal
				curshader.n = true;
				return 'n';
			}
			else if (socket == node.outputs[2]) {// UV
				curshader.context.add_elem('tex', 'short2norm');
				return 'vec3(texCoord.x, 1.0 - texCoord.y, 0.0)';
			}
			else if (socket == node.outputs[3]) { // Object
				curshader.mposition = true;
				return 'mposition';
			}
			else if (socket == node.outputs[4]) { // Camera
				curshader.vposition = true;
				return 'vposition';
			}
			else if (socket == node.outputs[5]) { // Window
				curshader.wvpposition = true;
				return 'wvpposition.xyz';
			}
			else if (socket == node.outputs[6]) { // Reflection
				return 'vec3(0.0, 0.0, 0.0)';
			}
		}

		else if (node.type == 'UVMAP') {
			//dupli = node.from_dupli
			curshader.context.add_elem('tex', 'short2norm');
			return 'vec3(texCoord.x, 1.0 - texCoord.y, 0.0)';
		}

		else if (node.type == 'BUMP') {
			var res = '';
			// Interpolation strength
			var strength = parse_value_input(node.inputs[0]);
			// Height multiplier
			// var distance = parse_value_input(node.inputs[1]);
			sample_bump = true;
			var height = parse_value_input(node.inputs[2]);
			sample_bump = false;
			var nor = parse_vector_input(node.inputs[3]);
			if (sample_bump_res != '') {
				curshader.nAttr = true;
				curshader.write('${sample_bump_res}_x *= ($strength) * 16.0;');
				curshader.write('${sample_bump_res}_y *= ($strength) * 16.0;');
				curshader.write('vec3 ${sample_bump_res}_a = normalize(vec3(1.0, 0.0, ${sample_bump_res}_x));');
				curshader.write('vec3 ${sample_bump_res}_b = normalize(vec3(0.0, 1.0, ${sample_bump_res}_y));');
				res = 'normalize(mul(nAttr, mat3(${sample_bump_res}_a, ${sample_bump_res}_b, normalize(vec3(${sample_bump_res}_x, ${sample_bump_res}_y, 1.0)))))';
				sample_bump_res = '';
			}
			else {
				curshader.n = true;
				res = 'n';
			}

			res = '($res + $nor)';

			if (!curshader.invTBN) {
				curshader.invTBN = true;
				curshader.nAttr = true;
				curshader.add_function(CyclesFunctions.str_cotangentFrame);
				curshader.write('mat3 invTBN = transpose(cotangentFrame(nAttr, -nAttr, texCoord));');
			}
			res = '(normalize(mul($res, invTBN)) * 0.5 + 0.5)';
			return res;
		}

		else if (node.type == 'MAPPING') {
			var out = parse_vector_input(node.inputs[0]);
			var node_translation = node.buttons[0].default_value;
			var node_rotation = node.buttons[1].default_value;
			var node_scale = node.buttons[2].default_value;
			if (node_scale[0] != 1.0 || node_scale[1] != 1.0 || node_scale[2] != 1.0) {
				out = '($out * ${vec3(node_scale)})';
			}
			if (node_rotation[2] != 0.0) {
				// ZYX rotation, Z axis for now..
				var a = node_rotation[2];
				// x * cos(theta) - y * sin(theta)
				// x * sin(theta) + y * cos(theta)
				out = 'vec3(${out}.x * ${Math.cos(a)} - (${out}.y) * ${Math.sin(a)}, ${out}.x * ${Math.sin(a)} + (${out}.y) * ${Math.cos(a)}, 0.0)';
			}
			// if node.rotation[1] != 0.0:
			//     a = node.rotation[1]
			//     out = 'vec3({0}.x * {1} - {0}.z * {2}, {0}.x * {2} + {0}.z * {1}, 0.0)'.format(out, math.cos(a), math.sin(a))
			// if node.rotation[0] != 0.0:
			//     a = node.rotation[0]
			//     out = 'vec3({0}.y * {1} - {0}.z * {2}, {0}.y * {2} + {0}.z * {1}, 0.0)'.format(out, math.cos(a), math.sin(a))

			if (node_translation[0] != 0.0 || node_translation[1] != 0.0 || node_translation[2] != 0.0) {
				out = '($out + ${vec3(node_translation)})';
			}
			// if node.use_min:
				// out = 'max({0}, vec3({1}, {2}, {3}))'.format(out, node.min[0], node.min[1])
			// if node.use_max:
				 // out = 'min({0}, vec3({1}, {2}, {3}))'.format(out, node.max[0], node.max[1])
			return out;
		}

		else if (node.type == 'NORMAL') {
			if (socket == node.outputs[0])
				return vec3(node.outputs[0].default_value);
			else if (socket == node.outputs[1]) {
				var nor = parse_vector_input(node.inputs[0]);
				var norout = vec3(node.outputs[0].default_value);
				return to_vec3('dot($norout, $nor)');
			}
		}

		else if (node.type == 'NORMAL_MAP') {
			// if curshader == tese:
				// return parse_vector_input(node.inputs[1])
			// else:
				// #space = node.space
				// #map = node.uv_map
				var strength = parse_value_input(node.inputs[0]);
				// Color
				parse_normal_map_color_input(node.inputs[1], strength);
				return null;
		}

		else if (node.type == 'VECT_TRANSFORM') {
		// 	#type = node.vector_type
		// 	#conv_from = node.convert_from
		// 	#conv_to = node.convert_to
		// 	# Pass throuh
		// 	return parse_vector_input(node.inputs[0])
		}

		else if (node.type == 'COMBXYZ') {
			var x = parse_value_input(node.inputs[0]);
			var y = parse_value_input(node.inputs[1]);
			var z = parse_value_input(node.inputs[2]);
			return 'vec3($x, $y, $z)';
		}

		else if (node.type == 'VECT_MATH') {
			var vec1 = parse_vector_input(node.inputs[0]);
			var vec2 = parse_vector_input(node.inputs[1]);
			var but = node.buttons[0]; //operation;
			var op:String = but.data[but.default_value].toUpperCase();
			op = op.replace(" ", "_");
			if (op == 'ADD') {
				return '($vec1 + $vec2)';
			}
			else if (op == 'SUBTRACT') {
				return '($vec1 - $vec2)';
			}
			else if (op == 'AVERAGE') {
				return '(($vec1 + $vec2) / 2.0)';
			}
			else if (op == 'DOT_PRODUCT') {
				return to_vec3('dot($vec1, $vec2)');
			}
			else if (op == 'CROSS_PRODUCT') {
				return 'cross($vec1, $vec2)';
			}
			else if (op == 'NORMALIZE') {
				return 'normalize($vec1)';
			}
		}

		else if (node.type == 'Displacement') {
			var height = parse_value_input(node.inputs[0]);
			var midlevel = parse_value_input(node.inputs[1]);
			var scale = parse_value_input(node.inputs[2]);
			var nor = parse_vector_input(node.inputs[3]);
			return to_vec3('$height');
		}

		return 'vec3(0.0, 0.0, 0.0)';
	}

	static function parse_normal_map_color_input(inp:TNodeSocket, strength = '1.0') {
		// if isInputLinked(inp) == False:
		//     return
		frag.write_normal++;
		out_normaltan = parse_vector_input(inp);
		if (!arm_export_tangents) {
			frag.write('vec3 texn = ($out_normaltan) * 2.0 - 1.0;');
			frag.write('texn.y = -texn.y;');
			if (!cotangentFrameWritten) {
				cotangentFrameWritten = true;
				frag.add_function(CyclesFunctions.str_cotangentFrame);
			}
			frag.n = true;
			#if (kha_direct3d11 || kha_direct3d12)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end

			frag.write('n = mul(normalize(texn), TBN);');
		}
		// else:
		//     frag.write('vec3 n = ({0}) * 2.0 - 1.0;'.format(out_normaltan))
		//     if (strength != '1.0') frag.write('n.xy *= $strength;');
		//     # frag.write('n = normalize(mul(normalize(n), TBN));')
		//     frag.write('n = mul(normalize(n), TBN);')
		//     con.add_elem('tang', 'short4norm')
		frag.write_normal--;
	}

	static function parse_value_input(inp:TNodeSocket):String {
		var l = getInputLink(inp);
		if (l != null) {
			var from_node = getNode(l.from_id);
			if (from_node.type == 'REROUTE')
				return parse_value_input(from_node.inputs[0]);

			var res_var = write_result(l);
			var st = from_node.outputs[l.from_socket].type;
			if (st == 'RGB' || st == 'RGBA' || st == 'VECTOR')
				return '$res_var.x';
			else //# VALUE
				return res_var;
		}
		else {
			// if c_state.mat_batch() and inp.is_uniform:
				// return to_uniform(inp)
			// else:
				return vec1(inp.default_value);
		}
	}

	static function parse_value(node:TNode, socket:TNodeSocket):String {
		// if node.type == 'GROUP':
		//     if node.node_tree.name.startswith('Armory PBR'):
		//         # Displacement
		//         if socket == node.outputs[1]:
		//             res = parse_value_input(node.inputs[10])
		//             if (isInputLinked(node.inputs[11]) or node.inputs[11].default_value != 1.0:
		//                 res = "({0} * {1})".format(res, parse_value_input(node.inputs[11]))
		//             return res
		//         else:
		//             return None
		//     else:
		//         return parse_group(node, socket)

		// elif node.type == 'GROUP_INPUT':
		//     return parse_group_input(node, socket)

		if (node.type == 'ATTRIBUTE') {
			// Pass time till drivers are implemented
			// if (node.attribute_name == 'time') {
				curshader.add_uniform('float time', '_time');
				return 'time';
			// }
			// else
				// return '0.0';
		}

		else if (node.type == 'CAMERA') {
			// View Z Depth
			if (socket == node.outputs[1]) {
				curshader.add_uniform('vec2 cameraProj', '_cameraPlaneProj');
				#if (kha_direct3d11 || kha_direct3d12)
				curshader.wvpposition = true;
				return '(cameraProj.y / ((wvpposition.z / wvpposition.w) - cameraProj.x))';
				#else
				return '(cameraProj.y / (gl_FragCoord.z - cameraProj.x))';
				#end
			}
			// View Distance
			else {
				curshader.add_uniform('vec3 eye', '_cameraPosition');
				curshader.wposition = true;
				return 'distance(eye, wposition)';
			}
		}

		else if (node.type == 'FRESNEL') {
			var ior = parse_value_input(node.inputs[0]);
			// var nor = parse_vector_input(node.inputs[1])
			curshader.dotNV = true;
			return 'pow(1.0 - dotNV, 7.25 / $ior)';
		}

		else if (node.type == 'NEW_GEOMETRY') {
			if (socket == node.outputs[6]) // Backfacing
				#if (kha_direct3d11 || kha_direct3d12)
				return '0.0'; // SV_IsFrontFace
				#else
				return '(1.0 - float(gl_FrontFacing))';
				#end
			else if (socket == node.outputs[7]) // Pointiness
				return '0.0';
		}

		else if (node.type == 'HAIR_INFO') {
			// Is Strand
			// Intercept
			// Thickness
			return '0.5';
		}

		else if (node.type == 'LAYER_WEIGHT') {
			var blend = parse_value_input(node.inputs[0]);
			// var nor = parse_vector_input(node.inputs[1])
			if (socket == node.outputs[0]) { // Fresnel
				curshader.dotNV = true;
				return 'clamp(pow(1.0 - dotNV, (1.0 - $blend) * 10.0), 0.0, 1.0)';
			}
			else if (socket == node.outputs[1]) { // Facing
				curshader.dotNV = true;
				return '((1.0 - dotNV) * $blend)';
			}
		}

		// elif node.type == 'LIGHT_PATH':
		//     if socket == node.outputs[0]: # Is Camera Ray
		//         return '1.0'
		//     elif socket == node.outputs[1]: # Is Shadow Ray
		//         return '0.0'
		//     elif socket == node.outputs[2]: # Is Diffuse Ray
		//         return '1.0'
		//     elif socket == node.outputs[3]: # Is Glossy Ray
		//         return '1.0'
		//     elif socket == node.outputs[4]: # Is Singular Ray
		//         return '0.0'
		//     elif socket == node.outputs[5]: # Is Reflection Ray
		//         return '0.0'
		//     elif socket == node.outputs[6]: # Is Transmission Ray
		//         return '0.0'
		//     elif socket == node.outputs[7]: # Ray Length
		//         return '0.0'
		//     elif socket == node.outputs[8]: # Ray Depth
		//         return '0.0'
		//     elif socket == node.outputs[9]: # Transparent Depth
		//         return '0.0'
		//     elif socket == node.outputs[10]: # Transmission Depth
		//         return '0.0'

		else if (node.type == 'OBJECT_INFO') {
			if (socket == node.outputs[1]) { // Object Index
				curshader.add_uniform('float objectInfoIndex', '_objectInfoIndex');
				return 'objectInfoIndex';
			}
			else if (socket == node.outputs[2]) { // Material Index
				curshader.add_uniform('float objectInfoMaterialIndex', '_objectInfoMaterialIndex');
				return 'objectInfoMaterialIndex';
			}
			else if (socket == node.outputs[3]) { // Random
				curshader.add_uniform('float objectInfoRandom', '_objectInfoRandom');
				return 'objectInfoRandom';
			}
		}

		// elif node.type == 'PARTICLE_INFO':
		//     if socket == node.outputs[0]: # Index
		//         return '0.0'
		//     elif socket == node.outputs[1]: # Age
		//         return '0.0'
		//     elif socket == node.outputs[2]: # Lifetime
		//         return '0.0'
		//     elif socket == node.outputs[4]: # Size
		//         return '0.0'

		else if (node.type == 'VALUE') {
			return vec1(node.outputs[0].default_value);
		}

		// elif node.type == 'WIREFRAME':
		//     #node.use_pixel_size
		//     # size = parse_value_input(node.inputs[0])
		//     return '0.0'

		else if (node.type == 'TEX_BRICK') {
			curshader.add_function(CyclesFunctions.str_tex_brick);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[4]);
			var res = 'tex_brick_f($co * $scale)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_CHECKER') {
			curshader.add_function(CyclesFunctions.str_tex_checker);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[3]);
			var res = 'tex_checker_f($co, $scale)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_GRADIENT') {
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var but = node.buttons[0]; //gradient_type;
			var grad:String = but.data[but.default_value].toUpperCase();
			grad = grad.replace(" ", "_");
			var f = '';
			if (grad == 'LINEAR') {
				f = '$co.x';
			}
			else if (grad == 'QUADRATIC') {
				f = '0.0';
			}
			else if (grad == 'EASING') {
				f = '0.0';
			}
			else if (grad == 'DIAGONAL') {
				f = '($co.x + $co.y) * 0.5';
			}
			else if (grad == 'RADIAL') {
				f = 'atan($co.y, $co.x) / (3.141592 * 2.0) + 0.5';
			}
			else if (grad == 'QUADRATIC_SPHERE') {
				f = '0.0';
			}
			else if (grad == 'SPHERICAL') {
				f = 'max(1.0 - sqrt($co.x * $co.x + $co.y * $co.y + $co.z * $co.z), 0.0)';
			}
			var res = '(clamp($f, 0.0, 1.0))';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_IMAGE') {
			// Already fetched
			if (parsed.indexOf(res_var_name(node, node.outputs[0])) >= 0) { // TODO: node.outputs[1]
				var varname = store_var_name(node);
				return '$varname.a';
			}
			var tex_name = node_name(node);
			var tex = make_texture(node, tex_name);
			if (tex != null) {
				var to_linear = node.buttons[1].default_value == 1; // srgb to linear
				var texstore = texture_store(node, tex, tex_name, to_linear);
				return '$texstore.a';
			}
			// # Already fetched
			// if res_var_name(node, node.outputs[0]) in parsed:
			//     return '{0}.a'.format(store_var_name(node))
			// tex_name = c_state.safesrc(node.name)
			// tex = c_state.make_texture(node, tex_name)
			// if tex != None:
			//     return '{0}.a'.format(texture_store(node, tex, tex_name))
			// else:
			//     tex_store = store_var_name(node) # Pink color for missing texture
			//     curshader.write('vec4 {0} = vec4(1.0, 0.0, 1.0, 1.0);'.format(tex_store))
			//     return '{0}.a'.format(tex_store)
		}

		else if (node.type == 'TEX_MAGIC') {
			curshader.add_function(CyclesFunctions.str_tex_magic);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_magic_f($co * $scale * 4.0)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_MUSGRAVE') {
			curshader.add_function(CyclesFunctions.str_tex_musgrave);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			// detail = parse_value_input(node.inputs[2])
			// distortion = parse_value_input(node.inputs[3])
			var res = 'tex_musgrave_f($co * $scale * 0.5)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_NOISE') {
			curshader.add_function(CyclesFunctions.str_tex_noise);
			curshader.add_uniform("sampler2D snoise256", "$noise256.png");
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			// detail = parse_value_input(node.inputs[2]);
			// distortion = parse_value_input(node.inputs[3]);
			var res = 'tex_noise($co * $scale)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		// elif node.type == 'TEX_POINTDENSITY':
		//     return '0.0'

		else if (node.type == 'TEX_VORONOI') {
			curshader.add_function(CyclesFunctions.str_tex_voronoi);
			curshader.add_uniform("sampler2D snoise256", "$noise256.png");
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			var but = node.buttons[0]; //coloring;
			var coloring:String = but.data[but.default_value].toUpperCase();
			coloring = coloring.replace(" ", "_");
			var res = '';
			if (coloring == 'INTENSITY') {
				res = 'tex_voronoi($co * $scale).a';
			}
			else { // Cells
				res = 'tex_voronoi($co * $scale).r';
			}
			if (sample_bump) write_bump(node, res);
			return res;
		}

		else if (node.type == 'TEX_WAVE') {
			curshader.add_function(CyclesFunctions.str_tex_wave);
			var co = '';
			if (getInputLink(node.inputs[0]) != null) {
				co = parse_vector_input(node.inputs[0]);
			}
			else {
				co = 'bposition';
				curshader.bposition = true;
			}
			var scale = parse_value_input(node.inputs[1]);
			var res = 'tex_wave_f($co * $scale)';
			if (sample_bump) write_bump(node, res);
			return res;
		}

		// elif node.type == 'LIGHT_FALLOFF':
		//     # Constant, linear, quadratic
		//     # Shaders default to quadratic for now
		//     return '1.0'

		else if (node.type == 'NORMAL') {
			var nor = parse_vector_input(node.inputs[0]);
			var norout = vec3(node.outputs[0].default_value);
			return 'dot($norout, $nor)';
		}

		// elif node.type == 'VALTORGB': # ColorRamp
		//     return '1.0'

		else if (node.type == 'MATH') {
			var val1 = parse_value_input(node.inputs[0]);
			var val2 = parse_value_input(node.inputs[1]);
			var but = node.buttons[0]; //operation;
			var op:String = but.data[but.default_value].toUpperCase();
			op = op.replace(" ", "_");
			but = node.buttons[1]; // use_clamp
			var use_clamp = but.default_value == "true";
			var out_val = '';
			if (op == 'ADD') {
				out_val = '($val1 + $val2)';
			}
			else if (op == 'SUBTRACT') {
				out_val = '($val1 - $val2)';
			}
			else if (op == 'MULTIPLY') {
				out_val = '($val1 * $val2)';
			}
			else if (op == 'DIVIDE') {
				out_val = '($val1 / $val2)';
			}
			else if (op == 'POWER') {
				out_val = 'pow($val1, $val2)';
			}
			else if (op == 'LOGARITHM') {
				out_val = 'log($val1)';
			}
			else if (op == 'SQUARE_ROOT') {
				out_val = 'sqrt($val1)';
			}
			else if (op == 'ABSOLUTE') {
				out_val = 'abs($val1)';
			}
			else if (op == 'MINIMUM') {
				out_val = 'min($val1, $val2)';
			}
			else if (op == 'MAXIMUM') {
				out_val = 'max($val1, $val2)';
			}
			else if (op == 'LESS_THAN') {
				out_val = 'float($val1 < $val2)';
			}
			else if (op == 'GREATER_THAN') {
				out_val = 'float($val1 > $val2)';
			}
			else if (op == 'ROUND') {
				out_val = 'floor($val1 + 0.5)';
			}
			else if (op == 'FLOOR') {
				out_val = 'floor($val1)';
			}
			else if (op == 'CEIL') {
				out_val = 'ceil($val1)';
			}
			else if (op == 'FRACT') {
				out_val = 'fract($val1)';
			}
			else if (op == 'MODULO') {
				out_val = 'mod($val1, $val2)';
			}
			else if (op == 'SINE') {
				out_val = 'sin($val1)';
			}
			else if (op == 'COSINE') {
				out_val = 'cos($val1)';
			}
			else if (op == 'TANGENT') {
				out_val = 'tan($val1)';
			}
			else if (op == 'ARCSINE') {
				out_val = 'asin($val1)';
			}
			else if (op == 'ARCCOSINE') {
				out_val = 'acos($val1)';
			}
			else if (op == 'ARCTANGENT') {
				out_val = 'atan($val1)';
			}
			else if (op == 'ARCTAN2') {
				out_val = 'atan2($val1, $val2)';
			}
			if (use_clamp) {
				return 'clamp($out_val, 0.0, 1.0)';
			}
			else {
				return out_val;
			}
		}

		else if (node.type == 'RGBTOBW') {
			var col = parse_vector_input(node.inputs[0]);
			return '((($col.r * 0.3 + $col.g * 0.59 + $col.b * 0.11) / 3.0) * 2.5)';
		}

		// elif node.type == 'SEPHSV':
		//     return '0.0'

		else if (node.type == 'SEPRGB') {
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

		else if (node.type == 'SEPXYZ') {
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

		else if (node.type == 'VECT_MATH') {
			var vec1 = parse_vector_input(node.inputs[0]);
			var vec2 = parse_vector_input(node.inputs[1]);
			var but = node.buttons[0]; //operation;
			var op:String = but.data[but.default_value].toUpperCase();
			op = op.replace(" ", "_");
			if (op == 'DOT_PRODUCT') {
				return 'dot($vec1, $vec2)';
			}
			else {
				return '0.0';
			}
		}

		return '0.0';
	}

	//

	static function vector_curve(name:String, fac:String, points:Array<Float>):String {
		// Write Ys array
		var ys_var = name + '_ys';
		var num = Std.int(points.length / 2);
		curshader.write('float $ys_var[$num];'); // TODO: Make const
		for (i in 0...num) {
			curshader.write('$ys_var[$i] = ${points[i * 2 + 1]};');
		}
		// Get index
		var fac_var = name + '_fac';
		curshader.write('float $fac_var = $fac;');
		var index = '0';
		for (i in 1...num) {
			index += ' + ($fac_var > ${points[i * 2]} ? 1 : 0)';
		}
		// Write index
		var index_var = name + '_i';
		curshader.write('int $index_var = $index;');
		// Linear
		// Write Xs array
		var facs_var = name + '_xs';
		curshader.write('float $facs_var[$num];'); // TODO: Make const
		for (i in 0...num) {
			curshader.write('$facs_var[$i] = ${points[i * 2]};');
		}
		// Map vector
		return 'mix($ys_var[$index_var], $ys_var[$index_var + 1], ($fac_var - $facs_var[$index_var]) * (1.0 / ($facs_var[$index_var + 1] - $facs_var[$index_var])))';
	}

	static function write_normal(inp) {
		var l = getInputLink(inp);
		if (l != null) {
			var normal_res = parse_vector_input(inp);
			if (normal_res != null) {
				curshader.write('n = $normal_res;');
				normal_written = true;
			}
		}
	}

	static function res_var_name(node:TNode, socket:TNodeSocket):String {
		return node_name(node) + '_' + safesrc(socket.name) + '_res';
	}

	static function write_result(l:TNodeLink):String {
		var from_node = getNode(l.from_id);
		var from_socket = from_node.outputs[l.from_socket];
		var res_var = res_var_name(from_node, from_socket);
		var st = from_socket.type;
		if (parsed.indexOf(res_var) < 0) {
			parsed.push(res_var);
			if (st == 'RGB' || st == 'RGBA' || st == 'VECTOR') {
				var res = parse_vector(from_node, from_socket);
				if (res == null)
					return null;
				curshader.write('vec3 $res_var = $res;');
			}
			else if (st == 'VALUE') {
				var res = parse_value(from_node, from_socket);
				if (res == null)
					return null;
				curshader.write('float $res_var = $res;');
			}
		}
		// # Normal map already parsed, return
		// else if (from_node.type == 'NORMAL_MAP')
			// return null
		return res_var;
	}

	// static function glsl_type(t:String):String {
	// 	if (t == 'RGB' || t == 'RGBA' || t == 'VECTOR')
	// 		return 'vec3';
	// 	else
	// 		return 'float';
	// }

	// def to_uniform(inp):
 //    uname = c_state.safesrc(inp.node.name) + c_state.safesrc(inp.name)
 //    curshader.add_uniform(glsl_type(inp.type) + ' ' + uname)
 //    return uname

	static function store_var_name(node:TNode):String {
		return node_name(node) + '_store';
	}

	static function texture_store(node:TNode, tex:TBindTexture, tex_name:String, to_linear = false):String {
		matcon.bind_textures.push(tex);
		curshader.context.add_elem('tex', 'short2norm');
		curshader.add_uniform('sampler2D $tex_name');
		var uv_name = '';
		if (getInputLink(node.inputs[0]) != null) {
			uv_name = parse_vector_input(node.inputs[0]);
		}
		else {
			uv_name = 'texCoord';
		}
		var tex_store = store_var_name(node);

		if (triplanar) {
			curshader.write('vec4 $tex_store = vec4(0.0, 0.0, 0.0, 0.0);');
			curshader.write('if (texCoordBlend.x > 0) $tex_store += texture($tex_name, ${uv_name}.xy) * texCoordBlend.x;');
			curshader.write('if (texCoordBlend.y > 0) $tex_store += texture($tex_name, ${uv_name}1.xy) * texCoordBlend.y;');
			curshader.write('if (texCoordBlend.z > 0) $tex_store += texture($tex_name, ${uv_name}2.xy) * texCoordBlend.z;');
		}
		else {
			// if c_state.mat_texture_grad():
				// curshader.write('vec4 {0} = textureGrad({1}, {2}.xy, g2.xy, g2.zw);'.format(tex_store, tex_name, uv_name))
			// else:
				curshader.write('vec4 $tex_store = texture($tex_name, $uv_name.xy);');
		}

		if (sample_bump) {
			sample_bump_res = tex_store;
			curshader.write('float ${tex_store}_x = dFdx($tex_store).x;');
			curshader.write('float ${tex_store}_y = dFdy($tex_store).x;');
			sample_bump = false;
		}
		if (to_linear) {
			curshader.write('$tex_store.rgb = pow($tex_store.rgb, vec3(2.2, 2.2, 2.2));');
		}
		return tex_store;
	}

	static function write_bump(node:TNode, res:String) {
		sample_bump_res = store_var_name(node) + '_bump';
		curshader.write('float ${sample_bump_res}_x = dFdx($res);');
		curshader.write('float ${sample_bump_res}_y = dFdy($res);');
		sample_bump = false;
	}

	static function vec1(v:Float):String {
		#if kha_webgl
		return 'float($v)';
		#else
		return '$v';
		#end
	}

	static function vec3(v:Array<Float>):String {
		#if kha_webgl
		return 'vec3(float(${v[0]}), float(${v[1]}), float(${v[2]}))';
		#else
		return 'vec3(${v[0]}, ${v[1]}, ${v[2]})';
		#end
	}

	static function to_vec3(s:String):String {
		#if (kha_direct3d11 || kha_direct3d12)
		return '($s).xxx';
		#else
		return 'vec3($s)';
		#end
	}

	static function node_by_type(nodes:Array<TNode>, ntype:String):TNode {
		for (n in nodes)
			if (n.type == ntype)
				return n;
		return null;
	}

	static function socket_index(node:TNode, socket:TNodeSocket):Int {
		for (i in 0...node.outputs.length)
			if (node.outputs[i] == socket)
				return i;
		return -1;
	}

	static function node_name(node:TNode):String {
		var s = safesrc(node.name) + node.id;
		// if len(parents) > 0:
			// s = c_state.safesrc(parents[-1].name) + '_' + s
		return s;
	}

	static function safesrc(s:String):String {
		for (i in 0...s.length) {
			var code = s.charCodeAt(i);
			var letter = (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
			if (!letter) s = s.replace(s.charAt(i), '_');
		}
		return s;
	}

	//

	static function make_texture(image_node:TNode, tex_name:String, matname:String = null):TBindTexture {

		var tex:TBindTexture = {
			name: tex_name,
			file: ''
		};

		// var image = image_node.image;
		// if (matname == null) {
			// matname = material.name;
		// }

		// if (image == null) {
			// return null;
		// }

		var filepath = image_node.buttons[0].data;

		if (filepath == '') {
			// log.warn(matname + '/' + image.name + ' - file path not found')
			return null;
		}

		// Reference image name
		// tex.file = extract_filename(image.filepath);
		// tex.file = safestr(tex.file);

		// tex.file = image_node.buttons[0].default_value;
		tex.file = filepath;

		var s = tex.file.split('.');

		if (s.length == 1) {
			// log.warn(matname + '/' + image.name + ' - file extension required for image name')
			return null;
		}

		// var ext = s[1].lower();
		// do_convert = ext != 'jpg' and ext != 'png' and ext != 'hdr' and ext != 'mp4' # Convert image
		// if do_convert:
			// tex['file'] = tex['file'].rsplit('.', 1)[0] + '.jpg'
			// # log.warn(matname + '/' + image.name + ' - image format is not (jpg/png/hdr), converting to jpg.')

		// if image.packed_file != None:
		// 	# Extract packed data
		// 	unpack_path = arm.utils.get_fp() + '/build/compiled/Assets/unpacked'
		// 	if not os.path.exists(unpack_path):
		// 		os.makedirs(unpack_path)
		// 	unpack_filepath = unpack_path + '/' + tex['file']

		// 	if do_convert:
		// 		if not os.path.isfile(unpack_filepath):
		// 			arm.utils.write_image(image, unpack_filepath)

		// 	# Write bytes if size is different or file does not exist yet
		// 	elif os.path.isfile(unpack_filepath) == False or os.path.getsize(unpack_filepath) != image.packed_file.size:
		// 		with open(unpack_filepath, 'wb') as f:
		// 			f.write(image.packed_file.data)

		// 	assets.add(unpack_filepath)

		// else:
			// if not os.path.isfile(arm.utils.asset_path(image.filepath)):
				// log.warn('Material ' + matname + '/' + image.name + ' - file not found(' + image.filepath + ')')
				// return None

			// if do_convert:
			// 	converted_path = arm.utils.get_fp() + '/build/compiled/Assets/unpacked/' + tex['file']
			// 	# TODO: delete cache when file changes
			// 	if not os.path.isfile(converted_path):
			// 		arm.utils.write_image(image, converted_path)
			// 	assets.add(converted_path)
			// else:
				// Link image path to assets
				// TODO: Khamake converts .PNG to .jpg? Convert ext to lowercase on windows
				// if arm.utils.get_os() == 'win':
					// s = image.filepath.rsplit('.', 1)
					// assets.add(arm.utils.asset_path(s[0] + '.' + s[1].lower()))
				// else:
					// assets.add(asset_path(image.filepath));


		 // if image_format != 'RGBA32':
			 // tex['format'] = image_format

		var interpolation = 'Smart';//image_node.interpolation;
		var aniso = 'On';//wrd.anisotropic_filtering_state;
		if (aniso == 'On') {
			interpolation = 'Smart';
		}
		// else if (aniso == 'Off' && interpolation == 'Smart') {
			// interpolation = 'Linear';
		// }

		// TODO: Blender seems to load full images on size request, cache size instead
		// var powimage = true;//is_pow(image.size[0]) && is_pow(image.size[1]);

		// Pow2 required to generate mipmaps
		// if (powimage) {
		// 	if (interpolation == 'Cubic') { // Mipmap linear
		// 		tex.mipmap_filter = 'linear';
		// 		tex.generate_mipmaps = true;
		// 	}
		// 	else if (interpolation == 'Smart') { // Mipmap anisotropic
				tex.min_filter = 'anisotropic';
				tex.mipmap_filter = 'linear';
				tex.generate_mipmaps = true;
		// 	}
		// }
		// else if (image_node.interpolation == 'Cubic' || image_node.interpolation == 'Smart') {
			// log.warn(matname + '/' + image.name + ' - power of 2 texture required for ' + image_node.interpolation + ' interpolation')
		// }

		tex.u_addressing = 'repeat';
		tex.v_addressing = 'repeat';
		// if (image_node.extension != 'REPEAT') { // Extend or clip
			// tex.u_addressing = 'clamp';
			// tex.v_addressing = 'clamp';
		// }
		// else {
			// if state.target == 'html5' and powimage == False:
				// log.warn(matname + '/' + image.name + ' - non power of 2 texture can not use repeat mode on HTML5 target')
				// tex.u_addressing = 'clamp';
				// tex.v_addressing = 'clamp';
		// }

		// if image.source == 'MOVIE': # Just append movie texture trait for now
		// 	movie_trait = {}
		// 	movie_trait['type'] = 'Script'
		// 	movie_trait['class_name'] = 'armory.trait.internal.MovieTexture'
		// 	movie_trait['parameters'] = [tex['file']]
		// 	for o in mat_state.mat_armusers[mat_state.material]:
		// 		o['traits'].append(movie_trait)
		// 	tex['source'] = 'movie'
		// 	tex['file'] = '' # MovieTexture will load the video

		return tex;
	}

	static function is_pow(num:Int):Bool {
		return ((num & (num - 1)) == 0) && num != 0;
	}

	static function asset_path(s:String):String {
		// return s[2:] if s[:2] == '//' else s # Remove leading '//';
		return s;
	}

	static function extract_filename(s:String):String {
		// return os.path.basename(asset_path(s));
		var ar = s.split(".");
		return ar[ar.length - 2] + "." + ar[ar.length - 1];
	}

	static function safestr(s:String):String {
		// for c in r'[]/\;,><&*:%=+@!#^()|?^':
			// s = s.replace(c, '-')
		return s;
	}
}

typedef TShaderOut = {
	var out_basecol:String;
	var out_roughness:String;
	var out_metallic:String;
	var out_occlusion:String;
	var out_opacity:String;
	var out_height:String;
	var out_emission:String;
	var out_subsurface:String;
}
