package arm.node;

import iron.object.MeshObject;
import iron.data.SceneFormat;
import arm.ui.UIHeader;
import arm.ui.UINodes;
import arm.shader.MaterialParser;
import arm.shader.NodeShader;
import arm.shader.NodeShaderData;
import arm.shader.NodeShaderContext;
import arm.shader.ShaderFunctions;
import arm.Enums;

class MakeMeshPreview {

	public static var opacityDiscardDecal = 0.05;

	public static function run(data: NodeShaderData, matcon: TMaterialContext): NodeShaderContext {
		var context_id = "mesh";
		var con_mesh: NodeShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: "less",
			cull_mode: "clockwise",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
			color_attachments: ["RGBA64", "RGBA64", "RGBA64"],
			depth_attachment: "DEPTH32"
		});

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();
		frag.ins = vert.outs;
		var pos = "pos";

		#if arm_skin
		var isMesh = Std.isOfType(Context.object, MeshObject);
		var skin = isMesh && cast(Context.object, MeshObject).data.geom.getVArray("bone") != null;
		if (skin) {
			pos = "spos";
			con_mesh.add_elem("bone", 'short4norm');
			con_mesh.add_elem("weight", 'short4norm');
			vert.add_function(ShaderFunctions.str_getSkinningDualQuat);
			vert.add_uniform('vec4 skinBones[128 * 2]', '_skinBones');
			vert.add_uniform('float posUnpack', '_posUnpack');
			vert.write_attrib('vec4 skinA;');
			vert.write_attrib('vec4 skinB;');
			vert.write_attrib('getSkinningDualQuat(ivec4(bone * 32767), weight, skinA, skinB);');
			vert.write_attrib('vec3 spos = pos.xyz;');
			vert.write_attrib('spos.xyz *= posUnpack;');
			vert.write_attrib('spos.xyz += 2.0 * cross(skinA.xyz, cross(skinA.xyz, spos.xyz) + skinA.w * spos.xyz);');
			vert.write_attrib('spos.xyz += 2.0 * (skinA.w * skinB.xyz - skinB.w * skinA.xyz + cross(skinA.xyz, skinB.xyz));');
			vert.write_attrib('spos.xyz /= posUnpack;');
		}
		#end

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write_attrib('gl_Position = mul(vec4($pos.xyz, 1.0), WVP);');

		var brushScale = (Context.brushScale * Context.brushNodesScale) + "";
		vert.add_out('vec2 texCoord');
		vert.write_attrib('texCoord = tex * float(${brushScale});');

		var decal = Context.decalPreview;
		MaterialParser.sample_keep_aspect = decal;
		MaterialParser.sample_uv_scale = brushScale;
		MaterialParser.parse_height = MakeMaterial.heightUsed;
		MaterialParser.parse_height_as_channel = true;
		var sout = MaterialParser.parse(UINodes.inst.getCanvasMaterial(), con_mesh, vert, frag, matcon);
		MaterialParser.parse_height = false;
		MaterialParser.parse_height_as_channel = false;
		MaterialParser.sample_keep_aspect = false;
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var opac = sout.out_opacity;
		var height = sout.out_height;
		var nortan = MaterialParser.out_normaltan;
		frag.write('vec3 basecol = pow($base, vec3(2.2, 2.2, 2.2));');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('float opacity = $opac;');
		frag.write('vec3 nortan = $nortan;');
		frag.write('float height = $height;');

		// MaterialParser.parse_height_as_channel = false;
		// vert.write('float vheight = $height;');
		// vert.add_out('float height');
		// vert.write('height = vheight;');
		// var displaceStrength = 0.1;
		// if (MakeMaterial.heightUsed && displaceStrength > 0.0) {
		// 	vert.write('vec3 pos2 = $pos.xyz + vec3(nor.xy, pos.w) * vec3($height, $height, $height) * vec3($displaceStrength, $displaceStrength, $displaceStrength);');
		// 	vert.write('gl_Position = mul(vec4(pos2.xyz, 1.0), WVP);');
		// }

		if (decal) {
			if (Context.tool == ToolText) {
				frag.add_uniform('sampler2D textexttool', '_textexttool');
				frag.write('opacity *= textureLod(textexttool, texCoord / float(${brushScale}), 0.0).r;');
			}
		}
		if (decal) {
			var opac = opacityDiscardDecal;
			frag.write('if (opacity < $opac) discard;');
		}

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;

		frag.add_function(ShaderFunctions.str_packFloatInt16);
		frag.add_function(ShaderFunctions.str_cotangentFrame);
		frag.add_function(ShaderFunctions.str_octahedronWrap);

		if (MakeMaterial.heightUsed) {
			frag.write('if (height > 0.0) {');
			frag.write('float height_dx = dFdx(height * 2.0);');
			frag.write('float height_dy = dFdy(height * 2.0);');
			// frag.write('float height_dx = height0 - height1;');
			// frag.write('float height_dy = height2 - height3;');
			// Whiteout blend
			frag.write('vec3 n1 = nortan * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			frag.write('vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));');
			frag.write('nortan = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			frag.write('}');
		}

		// Apply normal channel
		if (decal) {
			// TODO
		}
		else {
			frag.vVec = true;
			#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end
			frag.write('n = nortan * 2.0 - 1.0;');
			frag.write('n.y = -n.y;');
			frag.write('n = normalize(mul(n, TBN));');
		}

		frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
		frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
		// uint matid = 0;

		if (decal) {
			frag.write('fragColor[0] = vec4(n.x, n.y, roughness, packFloatInt16(metallic, uint(0)));'); // metallic/matid
			frag.write('fragColor[1] = vec4(basecol, occlusion);');
		}
		else {
			frag.write('fragColor[0] = vec4(n.x, n.y, mix(1.0, roughness, opacity), packFloatInt16(mix(1.0, metallic, opacity), uint(0)));'); // metallic/matid
			frag.write('fragColor[1] = vec4(mix(vec3(0.0, 0.0, 0.0), basecol, opacity), occlusion);');
		}
		frag.write('fragColor[2] = vec4(0.0, 0.0, 0.0, 0.0);'); // veloc

		MaterialParser.finalize(con_mesh);

		#if arm_skin
		if (skin) {
			vert.write('wnormal = normalize(mul(vec3(nor.xy, pos.w) + 2.0 * cross(skinA.xyz, cross(skinA.xyz, vec3(nor.xy, pos.w)) + skinA.w * vec3(nor.xy, pos.w)), N));');
		}
		#end

		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}
}
