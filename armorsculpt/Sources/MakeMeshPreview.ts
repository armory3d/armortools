
class MakeMeshPreview {

	static opacityDiscardDecal = 0.05;

	static run = (data: TMaterial, matcon: material_context_t): NodeShaderContextRaw => {
		let context_id = "mesh";
		let con_mesh = NodeShaderContext.create(data, {
			name: context_id,
			depth_write: true,
			compare_mode: "less",
			cull_mode: "clockwise",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}],
			color_attachments: ["RGBA64", "RGBA64", "RGBA64"],
			depth_attachment: "DEPTH32"
		});

		let vert = NodeShaderContext.make_vert(con_mesh);
		let frag = NodeShaderContext.make_frag(con_mesh);
		frag.ins = vert.outs;
		let pos = "pos";

		///if arm_skin
		let skin = mesh_data_get_vertex_array(Context.raw.paintObject.data, "bone") != null;
		if (skin) {
			pos = "spos";
			NodeShaderContext.add_elem(con_mesh, "bone", 'short4norm');
			NodeShaderContext.add_elem(con_mesh, "weight", 'short4norm');
			NodeShader.add_function(vert, ShaderFunctions.str_getSkinningDualQuat);
			NodeShader.add_uniform(vert, 'vec4 skinBones[128 * 2]', '_skin_bones');
			NodeShader.add_uniform(vert, 'float posUnpack', '_pos_unpack');
			NodeShader.write_attrib(vert, 'vec4 skinA;');
			NodeShader.write_attrib(vert, 'vec4 skinB;');
			NodeShader.write_attrib(vert, 'getSkinningDualQuat(ivec4(bone * 32767), weight, skinA, skinB);');
			NodeShader.write_attrib(vert, 'vec3 spos = pos.xyz;');
			NodeShader.write_attrib(vert, 'spos.xyz *= posUnpack;');
			NodeShader.write_attrib(vert, 'spos.xyz += 2.0 * cross(skinA.xyz, cross(skinA.xyz, spos.xyz) + skinA.w * spos.xyz);');
			NodeShader.write_attrib(vert, 'spos.xyz += 2.0 * (skinA.w * skinB.xyz - skinB.w * skinA.xyz + cross(skinA.xyz, skinB.xyz));');
			NodeShader.write_attrib(vert, 'spos.xyz /= posUnpack;');
		}
		///end

		NodeShader.add_uniform(vert, 'mat4 WVP', '_world_view_proj_matrix');
		NodeShader.write_attrib(vert, `gl_Position = mul(vec4(${pos}.xyz, 1.0), WVP);`);

		let brushScale = (Context.raw.brushScale * Context.raw.brushNodesScale) + "";
		NodeShader.add_out(vert, 'vec2 texCoord');
		NodeShader.write_attrib(vert, `texCoord = tex * float(${brushScale});`);

		let decal = Context.raw.decalPreview;
		ParserMaterial.sample_keep_aspect = decal;
		ParserMaterial.sample_uv_scale = brushScale;
		ParserMaterial.parse_height = MakeMaterial.heightUsed;
		ParserMaterial.parse_height_as_channel = true;
		// let sout = ParserMaterial.parse(UINodes.getCanvasMaterial(), con_mesh, vert, frag, matcon);
		ParserMaterial.parse_height = false;
		ParserMaterial.parse_height_as_channel = false;
		ParserMaterial.sample_keep_aspect = false;
		let base = "vec3(1.0, 1.0, 1.0)";//sout.out_basecol;
		let rough = "0.0";//sout.out_roughness;
		let met = "0.0";//sout.out_metallic;
		let occ = "0.0";//sout.out_occlusion;
		let opac = "0.0";//sout.out_opacity;
		let height = "0.0";//sout.out_height;
		let nortan = "vec3(1.0, 1.0, 1.0)";//ParserMaterial.out_normaltan;
		NodeShader.write(frag, `vec3 basecol = pow(${base}, vec3(2.2, 2.2, 2.2));`);
		NodeShader.write(frag, `float roughness = ${rough};`);
		NodeShader.write(frag, `float metallic = ${met};`);
		NodeShader.write(frag, `float occlusion = ${occ};`);
		NodeShader.write(frag, `float opacity = ${opac};`);
		NodeShader.write(frag, `vec3 nortan = ${nortan};`);
		NodeShader.write(frag, `float height = ${height};`);

		// ParserMaterial.parse_height_as_channel = false;
		// NodeShader.write(vert, `float vheight = ${height};`);
		// NodeShader.add_out(vert, 'float height');
		// NodeShader.write(vert, 'height = vheight;');
		// let displaceStrength = 0.1;
		// if (MakeMaterial.heightUsed && displaceStrength > 0.0) {
		// 	NodeShader.write(vert, `vec3 pos2 = ${pos}.xyz + vec3(nor.xy, pos.w) * vec3(${height}, ${height}, ${height}) * vec3(${displaceStrength}, ${displaceStrength}, ${displaceStrength});`);
		// 	NodeShader.write(vert, 'gl_Position = mul(vec4(pos2.xyz, 1.0), WVP);');
		// }

		if (decal) {
			if (Context.raw.tool == WorkspaceTool.ToolText) {
				NodeShader.add_uniform(frag, 'sampler2D textexttool', '_textexttool');
				NodeShader.write(frag, `opacity *= textureLod(textexttool, texCoord / float(${brushScale}), 0.0).r;`);
			}
		}
		if (decal) {
			let opac = MakeMeshPreview.opacityDiscardDecal;
			NodeShader.write(frag, `if (opacity < ${opac}) discard;`);
		}

		NodeShader.add_out(frag, 'vec4 fragColor[3]');
		frag.n = true;

		NodeShader.add_function(frag, ShaderFunctions.str_packFloatInt16);
		NodeShader.add_function(frag, ShaderFunctions.str_cotangentFrame);
		NodeShader.add_function(frag, ShaderFunctions.str_octahedronWrap);

		if (MakeMaterial.heightUsed) {
			NodeShader.write(frag, 'if (height > 0.0) {');
			NodeShader.write(frag, 'float height_dx = dFdx(height * 2.0);');
			NodeShader.write(frag, 'float height_dy = dFdy(height * 2.0);');
			// NodeShader.write(frag, 'float height_dx = height0 - height1;');
			// NodeShader.write(frag, 'float height_dy = height2 - height3;');
			// Whiteout blend
			NodeShader.write(frag, 'vec3 n1 = nortan * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
			NodeShader.write(frag, 'vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));');
			NodeShader.write(frag, 'nortan = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
			NodeShader.write(frag, '}');
		}

		// Apply normal channel
		if (decal) {
			// TODO
		}
		else {
			frag.vVec = true;
			///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
			NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			///else
			NodeShader.write(frag, 'mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			///end
			NodeShader.write(frag, 'n = nortan * 2.0 - 1.0;');
			NodeShader.write(frag, 'n.y = -n.y;');
			NodeShader.write(frag, 'n = normalize(mul(n, TBN));');
		}

		NodeShader.write(frag, 'n /= (abs(n.x) + abs(n.y) + abs(n.z));');
		NodeShader.write(frag, 'n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
		// uint matid = 0;

		if (decal) {
			NodeShader.write(frag, 'fragColor[0] = vec4(n.x, n.y, roughness, packFloatInt16(metallic, uint(0)));'); // metallic/matid
			NodeShader.write(frag, 'fragColor[1] = vec4(basecol, occlusion);');
		}
		else {
			NodeShader.write(frag, 'fragColor[0] = vec4(n.x, n.y, mix(1.0, roughness, opacity), packFloatInt16(mix(1.0, metallic, opacity), uint(0)));'); // metallic/matid
			NodeShader.write(frag, 'fragColor[1] = vec4(mix(vec3(0.0, 0.0, 0.0), basecol, opacity), occlusion);');
		}
		NodeShader.write(frag, 'fragColor[2] = vec4(0.0, 0.0, 0.0, 0.0);'); // veloc

		ParserMaterial.finalize(con_mesh);

		///if arm_skin
		if (skin) {
			NodeShader.write(vert, 'wnormal = normalize(mul(vec3(nor.xy, pos.w) + 2.0 * cross(skinA.xyz, cross(skinA.xyz, vec3(nor.xy, pos.w)) + skinA.w * vec3(nor.xy, pos.w)), N));');
		}
		///end

		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = NodeShader.get(vert);
		con_mesh.data.fragment_shader = NodeShader.get(frag);

		return con_mesh;
	}
}
