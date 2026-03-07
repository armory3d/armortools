node_shader_context_t *make_mesh_preview_run(material_t *data, material_context_t *matcon) {
	char              *context_id = "mesh";
	shader_context_t      *props      = GC_ALLOC_INIT(shader_context_t, {.name            = context_id,
	                                                                     .depth_write     = true,
	                                                                     .compare_mode    = "less",
	                                                                     .cull_mode       = "clockwise",
	                                                                     .vertex_elements = any_array_create_from_raw(
                                                                   (any[]){
                                                                       GC_ALLOC_INIT(vertex_element_t, {.name = "pos", .data = "short4norm"}),
                                                                       GC_ALLOC_INIT(vertex_element_t, {.name = "nor", .data = "short2norm"}),
                                                                       GC_ALLOC_INIT(vertex_element_t, {.name = "tex", .data = "short2norm"}),
                                                                   },
                                                                   3),
	                                                                     .color_attachments = any_array_create_from_raw(
                                                                   (any[]){
                                                                       "RGBA64",
                                                                       "RGBA64",
                                                                   },
                                                                   2),
	                                                                     .depth_attachment = "D32"});
	node_shader_context_t *con_mesh   = node_shader_context_create(data, props);

	node_shader_t         *kong       = node_shader_context_make_kong(con_mesh);

	char              *pos        = "input.pos";

	node_shader_add_constant(kong, "WVP: float4x4", "_world_view_proj_matrix");
	node_shader_write_attrib_vert(kong, string_join(string_join("output.pos = constants.WVP * float4(", pos), ".xyz, 1.0);"));
	f32       sc          = context_raw->brush_scale * context_raw->brush_nodes_scale;
	char *brush_scale = string_join(f32_to_string(sc), "");
	node_shader_add_out(kong, "tex_coord: float2");
	node_shader_write_attrib_vert(kong, string_join(string_join("output.tex_coord = input.tex * float(", brush_scale), ");"));
	node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.tex_coord;");

	bool decal                         = context_raw->decal_preview;
	parser_material_sample_keep_aspect = decal;
	gc_unroot(parser_material_sample_uv_scale);
	parser_material_sample_uv_scale = string_copy(brush_scale);
	gc_root(parser_material_sample_uv_scale);
	parser_material_parse_height            = make_material_height_used;
	parser_material_parse_height_as_channel = true;
	shader_out_t *sout                      = parser_material_parse(context_raw->material->canvas, con_mesh, kong, matcon);
	parser_material_parse_height            = false;
	parser_material_parse_height_as_channel = false;
	parser_material_sample_keep_aspect      = false;
	char *base                          = sout->out_basecol;
	char *rough                         = sout->out_roughness;
	char *met                           = sout->out_metallic;
	char *occ                           = sout->out_occlusion;
	char *opac                          = sout->out_opacity;
	char *height                        = sout->out_height;
	char *nortan                        = parser_material_out_normaltan;
	node_shader_write_frag(kong, string_join(string_join("var basecol: float3 = pow3(", base), ", float3(2.2, 2.2, 2.2));"));
	node_shader_write_frag(kong, string_join(string_join("var roughness: float = ", rough), ";"));
	node_shader_write_frag(kong, string_join(string_join("var metallic: float = ", met), ";"));
	node_shader_write_frag(kong, string_join(string_join("var occlusion: float = ", occ), ";"));
	node_shader_write_frag(kong, string_join(string_join("var opacity: float = ", opac), ";"));
	node_shader_write_frag(kong, string_join(string_join("var nortan: float3 = ", nortan), ";"));
	node_shader_write_frag(kong, string_join(string_join("var height: float = ", height), ";"));

	if (decal) {
		if (context_raw->tool == TOOL_TYPE_TEXT) {
			node_shader_add_texture(kong, "textexttool", "_textexttool");
			node_shader_write_frag(kong, string_join(string_join("opacity *= sample_lod(textexttool, sampler_linear, tex_coord / float(", brush_scale),
			                                         "), 0.0).r;"));
		}
	}
	if (decal) {
		f32 opac = config_raw->brush_alpha_discard;
		node_shader_write_frag(kong, string_join(string_join("if (opacity <= float(", f32_to_string(opac)), ")) { discard; }"));
	}

	kong->frag_out = "float4[2]";
	kong->frag_n   = true;

	node_shader_add_function(kong, str_pack_float_int16);
	node_shader_add_function(kong, str_cotangent_frame);
	node_shader_add_function(kong, str_octahedron_wrap);

	// if (make_material_opac_used) {
	// 	kong.frag_wvpposition = true;
	// 	node_shader_add_function(kong, str_dither_bayer);
	// 	node_shader_write_frag(kong, "var fragcoord1: float2 = float2(input.wvpposition.x / input.wvpposition.w, input.wvpposition.y / input.wvpposition.w) * 0.5 + 0.5;");
	// 	node_shader_write_frag(kong, "var dither: float = dither_bayer(fragcoord1 * float2(256.0, 256.0));");
	// 	node_shader_write_frag(kong, "if (opacity < dither) { discard; }");
	// }

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
		kong->frag_vvec = true;
		node_shader_write_frag(kong, "var TBN: float3x3 = cotangent_frame(n, vvec, tex_coord);");
		node_shader_write_frag(kong, "n = nortan * 2.0 - 1.0;");
		node_shader_write_frag(kong, "n.y = -n.y;");
		node_shader_write_frag(kong, "n = normalize(TBN * n);");
	}

	node_shader_write_frag(kong, "n = n / (abs(n.x) + abs(n.y) + abs(n.z));");
	// node_shader_write_frag(kong, "n.xy = n.z >= 0.0 ? n.xy : octahedron_wrap(n.xy);");
	node_shader_write_frag(kong, "if (n.z < 0.0) { n.xy = octahedron_wrap(n.xy); }");
	// uint matid = uint(0);

	if (decal) {
		node_shader_write_frag(kong, "output[0] = float4(n.x, n.y, roughness, pack_f32_i16(metallic, uint(0)));"); // metallic/matid
		node_shader_write_frag(kong, "output[1] = float4(basecol, occlusion);");
	}
	else {
		node_shader_write_frag(kong, "output[0] = float4(n.x, n.y, lerp(1.0, roughness, opacity), pack_f32_i16(lerp(1.0, metallic, opacity), uint(0)));"); // metallic/matid
		node_shader_write_frag(kong, "output[1] = float4(lerp3(float3(0.0, 0.0, 0.0), basecol, opacity), occlusion);");
	}

	parser_material_finalize(con_mesh);

	con_mesh->data->shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_mesh->data->vertex_shader), ADDRESS(con_mesh->data->fragment_shader),
	                             ADDRESS(con_mesh->data->_->vertex_shader_size), ADDRESS(con_mesh->data->_->fragment_shader_size));
	return con_mesh;
}
