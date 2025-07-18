
let make_mesh_preview_opacity_discard_decal: f32 = 0.05;

function make_mesh_preview_run(data: material_t, matcon: material_context_t): node_shader_context_t {
	let context_id: string = "mesh";

	let props: shader_context_t = {
		name: context_id,
		depth_write: true,
		compare_mode: "less",
		cull_mode: "clockwise",
		vertex_elements: [
			{
				name: "pos",
				data: "short4norm"
			},
			{
				name: "nor",
				data: "short2norm"
			},
			{
				name: "tex",
				data: "short2norm"
			}
		],
		color_attachments: [
			"RGBA64",
			"RGBA64",
			"RGBA64"
		],
		depth_attachment: "D32"
	};

	let con_mesh: node_shader_context_t = node_shader_context_create(data, props);

	let kong: node_shader_t = node_shader_context_make_kong(con_mesh);

	let pos: string = "input.pos";

	///if arm_skin
	let skin: bool = mesh_data_get_vertex_array(context_raw.paint_object.data, "bone") != null;
	if (skin) {
		pos = "spos";
		node_shader_context_add_elem(con_mesh, "bone", "short4norm");
		node_shader_context_add_elem(con_mesh, "weight", "short4norm");
		node_shader_add_function(kong, str_get_skinning_dual_quat);
		node_shader_add_constant(kong, "skin_bones: float4[128 * 2]", "_skin_bones");
		node_shader_add_constant(kong, "pos_unpack: float", "_pos_unpack");
		node_shader_write_attrib_vert(kong, "var skin_a: float4;");
		node_shader_write_attrib_vert(kong, "var skin_b: float4;");
		node_shader_write_attrib_vert(kong, "get_skinning_dual_quat(int4(bone * 32767), weight, skin_a, skin_b);");
		node_shader_write_attrib_vert(kong, "var spos: float3 = input.pos.xyz;");
		node_shader_write_attrib_vert(kong, "spos.xyz *= constants.pos_unpack;");
		node_shader_write_attrib_vert(kong, "spos.xyz += 2.0 * cross(skin_a.xyz, cross(skin_a.xyz, spos.xyz) + skin_a.w * spos.xyz);");
		node_shader_write_attrib_vert(kong, "spos.xyz += 2.0 * (skin_a.w * skin_b.xyz - skin_b.w * skin_a.xyz + cross(skin_a.xyz, skin_b.xyz));");
		node_shader_write_attrib_vert(kong, "spos.xyz /= constants.pos_unpack;");
	}
	///end

	node_shader_add_constant(kong, "WVP: float4x4", "_world_view_proj_matrix");
	node_shader_write_attrib_vert(kong, "output.pos = constants.WVP * float4(" + pos + ".xyz, 1.0);");

	let sc: f32 = context_raw.brush_scale * context_raw.brush_nodes_scale;
	let brush_scale: string = sc + "";
	node_shader_add_out(kong, "tex_coord: float2");
	node_shader_write_attrib_vert(kong, "output.tex_coord = input.tex * float(" + brush_scale + ");");
	node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.tex_coord;");

	let decal: bool = context_raw.decal_preview;
	parser_material_sample_keep_aspect = decal;
	parser_material_sample_uv_scale = brush_scale;
	parser_material_parse_height = make_material_height_used;
	parser_material_parse_height_as_channel = true;
	parser_material_parse_height = false;
	parser_material_parse_height_as_channel = false;
	parser_material_sample_keep_aspect = false;
	let base: string = "float3(1.0, 1.0, 1.0)";
	let rough: string = "0.0";
	let met: string = "0.0";
	let occ: string = "0.0";
	let opac: string = "0.0";
	let height: string = "0.0";
	let nortan: string = "float3(1.0, 1.0, 1.0)";
	node_shader_write_frag(kong, "var basecol: float3 = pow3(" + base + ", float3(2.2, 2.2, 2.2));");
	node_shader_write_frag(kong, "var roughness: float = " + rough + ";");
	node_shader_write_frag(kong, "var metallic: float = " + met + ";");
	node_shader_write_frag(kong, "var occlusion: float = " + occ + ";");
	node_shader_write_frag(kong, "var opacity: float = " + opac + ";");
	node_shader_write_frag(kong, "var nortan: float3 = " + nortan + ";");
	node_shader_write_frag(kong, "var height: float = " + height + ";");

	if (decal) {
		if (context_raw.tool == workspace_tool_t.TEXT) {
			node_shader_add_texture(kong, "textexttool", "_textexttool");
			node_shader_write_frag(kong, "opacity *= sample_lod(textexttool, sampler_linear, tex_coord / float(" + brush_scale + "), 0.0).r;");
		}
	}
	if (decal) {
		let opac: f32 = make_mesh_preview_opacity_discard_decal;
		node_shader_write_frag(kong, "if (opacity < " + opac + ") { discard; }");
	}

	kong.frag_out = "float4[3]";
	kong.frag_n = true;

	node_shader_add_function(kong, str_pack_float_int16);
	node_shader_add_function(kong, str_cotangent_frame);
	node_shader_add_function(kong, str_octahedron_wrap);

	if (make_material_height_used) {
		node_shader_write_frag(kong, "if (height > 0.0) {");
		node_shader_write_frag(kong, "var height_dx: float = ddx(height * 2.0);");
		node_shader_write_frag(kong, "var height_dy: float = ddy(height * 2.0);");
		// Whiteout blend
		node_shader_write_frag(kong, "var n1: float3 = nortan * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
		node_shader_write_frag(kong, "var n2: float3 = normalize(float3(height_dx * 16.0, height_dy * 16.0, 1.0));");
		node_shader_write_frag(kong, "nortan = normalize(float3(n1.xy + n2.xy, n1.z * n2.z)) * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5);");
		node_shader_write_frag(kong, "}");
	}

	// Apply normal channel
	if (decal) {
		// TODO
	}
	else {
		kong.frag_vvec = true;
		node_shader_write_frag(kong, "var TBN: float3x3 = cotangent_frame(n, vvec, tex_coord);");
		node_shader_write_frag(kong, "n = nortan * 2.0 - 1.0;");
		node_shader_write_frag(kong, "n.y = -n.y;");
		node_shader_write_frag(kong, "n = normalize(TBN * n);");
	}

	node_shader_write_frag(kong, "n /= (abs(n.x) + abs(n.y) + abs(n.z));");
	// node_shader_write_frag(kong, "n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);");
	node_shader_write_frag(kong, "if (n.z < 0.0) { n.xy = octahedron_wrap(n.xy); }");
	// var matid: uint = uint(0);

	if (decal) {
		node_shader_write_frag(kong, "output[0] = float4(n.x, n.y, roughness, pack_f32_i16(metallic, uint(0)));"); // metallic/matid
		node_shader_write_frag(kong, "output[1] = float4(basecol, occlusion);");
	}
	else {
		node_shader_write_frag(kong, "output[0] = float4(n.x, n.y, lerp(1.0, roughness, opacity), pack_f32_i16(lerp(1.0, metallic, opacity), uint(0)));"); // metallic/matid
		node_shader_write_frag(kong, "output[1] = float4(lerp3(float3(0.0, 0.0, 0.0), basecol, opacity), occlusion);");
	}
	node_shader_write_frag(kong, "output[2] = float4(0.0, 0.0, 0.0, 0.0);"); // veloc

	parser_material_finalize(con_mesh);

	///if arm_skin
	if (skin) {
		node_shader_write_vert(kong, "wnormal = normalize(N * float3(input.nor.xy, input.pos.w) + 2.0 * cross(skin_a.xyz, cross(skin_a.xyz, float3(input.nor.xy, input.pos.w)) + skin_a.w * float3(input.nor.xy, input.pos.w)));");
	}
	///end

	con_mesh.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_mesh.data.vertex_shader), ADDRESS(con_mesh.data.fragment_shader), ADDRESS(con_mesh.data._.vertex_shader_size), ADDRESS(con_mesh.data._.fragment_shader_size));

	return con_mesh;
}
