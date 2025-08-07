
///if is_paint

function make_paint_is_raytraced_bake(): bool {
	return context_raw.bake_type == bake_type_t.INIT;
}

function make_paint_color_attachments(): string[] {
	if (context_raw.tool == tool_type_t.COLORID) {
		let res: string[] = ["RGBA32"];
		return res;
	}
	if (context_raw.tool == tool_type_t.PICKER && context_raw.pick_pos_nor_tex) {
		let res: string[] = ["RGBA128", "RGBA128"];
		return res;
	}
	if (context_raw.tool == tool_type_t.PICKER || context_raw.tool == tool_type_t.MATERIAL) {
		let res: string[] = ["RGBA32", "RGBA32", "RGBA32", "RGBA32"];
		return res;
	}
	if (context_raw.tool == tool_type_t.BAKE && make_paint_is_raytraced_bake()) {
		let res: string[] = ["RGBA64", "RGBA64"];
		return res;
	}

	let format: tex_format_t =
		base_bits_handle.position == texture_bits_t.BITS8  ? tex_format_t.RGBA32 :
		base_bits_handle.position == texture_bits_t.BITS16 ? tex_format_t.RGBA64 :
															 tex_format_t.RGBA128;

	if (format == tex_format_t.RGBA64) {
		let res: string[] = ["RGBA64", "RGBA64", "RGBA64", "R8"];
		return res;
	}
	if (format == tex_format_t.RGBA128) {
		let res: string[] = ["RGBA128", "RGBA128", "RGBA128", "R8"];
		return res;
	}

	let res: string[] = ["RGBA32", "RGBA32", "RGBA32", "R8"];
	return res;
}

function make_paint_run(data: material_t, matcon: material_context_t): node_shader_context_t {
	let context_id: string = "paint";
	let props: shader_context_t = {
		name: context_id,
		depth_write: false,
		compare_mode: "always",
		cull_mode: "none",
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
		color_attachments: make_paint_color_attachments()
	};
	let con_paint: node_shader_context_t = node_shader_context_create(data, props);

	con_paint.data.color_writes_red = [true, true, true, true];
	con_paint.data.color_writes_green = [true, true, true, true];
	con_paint.data.color_writes_blue = [true, true, true, true];
	con_paint.data.color_writes_alpha = [true, true, true, true];
	con_paint.allow_vcols = mesh_data_get_vertex_array(context_raw.paint_object.data, "col") != null;

	let kong: node_shader_t = node_shader_context_make_kong(con_paint);

	if (context_raw.tool == tool_type_t.BAKE && context_raw.bake_type == bake_type_t.INIT) {
		// Init raytraced bake
		make_bake_position_normal(kong);
		con_paint.data.shader_from_source = true;
		gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader), ADDRESS(con_paint.data._.vertex_shader_size), ADDRESS(con_paint.data._.fragment_shader_size));
		return con_paint;
	}

	if (context_raw.tool == tool_type_t.BAKE) {
		make_bake_set_color_writes(con_paint);
	}

	if (context_raw.tool == tool_type_t.COLORID || context_raw.tool == tool_type_t.PICKER || context_raw.tool == tool_type_t.MATERIAL) {
		make_colorid_picker_run(kong);
		con_paint.data.shader_from_source = true;
		gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader), ADDRESS(con_paint.data._.vertex_shader_size), ADDRESS(con_paint.data._.fragment_shader_size));
		return con_paint;
	}

	let face_fill: bool = context_raw.tool == tool_type_t.FILL && context_raw.fill_type_handle.position == fill_type_t.FACE;
	let uv_island_fill: bool = context_raw.tool == tool_type_t.FILL && context_raw.fill_type_handle.position == fill_type_t.UV_ISLAND;
	let decal: bool = context_is_decal();

	node_shader_write_vert(kong, "var tpos: float2 = float2(input.tex.x * 2.0 - 1.0, (1.0 - input.tex.y) * 2.0 - 1.0);");

	node_shader_write_vert(kong, "output.pos = float4(tpos, 0.0, 1.0);");

	let decal_layer: bool = context_raw.layer.fill_layer != null && context_raw.layer.uv_type == uv_type_t.PROJECT;
	if (decal_layer) {
		node_shader_add_constant(kong, "WVP: float4x4", "_decal_layer_matrix");
	}
	else {
		node_shader_add_constant(kong, "WVP: float4x4", "_world_view_proj_matrix");
	}

	node_shader_add_out(kong, "ndc: float4");
	node_shader_write_attrib_vert(kong, "output.ndc = constants.WVP * float4(input.pos.xyz, 1.0);");

	node_shader_write_attrib_frag(kong, "var sp: float3 = (input.ndc.xyz / input.ndc.w) * 0.5 + 0.5;");
	node_shader_write_attrib_frag(kong, "sp.y = 1.0 - sp.y;");
	node_shader_write_attrib_frag(kong, "sp.z -= 0.0001;"); // small bias

	let uv_type: uv_type_t = context_raw.layer.fill_layer != null ? context_raw.layer.uv_type : context_raw.brush_paint;
	if (uv_type == uv_type_t.PROJECT) {
		kong.frag_ndcpos = true;
	}

	node_shader_add_constant(kong, "inp: float4", "_input_brush");
	node_shader_add_constant(kong, "inplast: float4", "_input_brush_last");
	node_shader_add_constant(kong, "aspect_ratio: float", "_aspect_ratio_window");
	node_shader_write_frag(kong, "var bsp: float2 = sp.xy * 2.0 - 1.0;");
	node_shader_write_frag(kong, "bsp.x *= constants.aspect_ratio;");
	node_shader_write_frag(kong, "bsp = bsp * 0.5 + 0.5;");

	node_shader_add_texture(kong, "gbufferD");

	kong.frag_out = "float4[4]";

	node_shader_add_constant(kong, "brush_radius: float", "_brush_radius");
	node_shader_add_constant(kong, "brush_opacity: float", "_brush_opacity");
	node_shader_add_constant(kong, "brush_hardness: float", "_brush_hardness");

	if (context_raw.tool == tool_type_t.BRUSH  ||
		context_raw.tool == tool_type_t.ERASER ||
		context_raw.tool == tool_type_t.CLONE  ||
		context_raw.tool == tool_type_t.BLUR   ||
		context_raw.tool == tool_type_t.SMUDGE   ||
		context_raw.tool == tool_type_t.PARTICLE ||
		decal) {

		let depth_reject: bool = !context_raw.xray;
		if (config_raw.brush_3d && !config_raw.brush_depth_reject) {
			depth_reject = false;
		}

		// TODO: sp.z needs to take height channel into account
		let particle: bool = context_raw.tool == tool_type_t.PARTICLE;
		if (config_raw.brush_3d && !decal && !particle) {
			if (make_material_height_used || context_raw.sym_x || context_raw.sym_y || context_raw.sym_z) {
				depth_reject = false;
			}
		}

		if (depth_reject) {
			node_shader_write_frag(kong, "if (sp.z > sample_lod(gbufferD, sampler_linear, sp.xy, 0.0).r - 0.00005) { discard; }");
		}

		make_brush_run(kong);
	}
	else { // Fill, Bake
		node_shader_write_frag(kong, "var dist: float = 0.0;");
		let angle_fill: bool = context_raw.tool == tool_type_t.FILL && context_raw.fill_type_handle.position == fill_type_t.ANGLE;
		if (angle_fill) {
			node_shader_add_function(kong, str_octahedron_wrap);
			node_shader_add_texture(kong, "gbuffer0");
			node_shader_write_frag(kong, "var g0: float2 = sample_lod(gbuffer0, sampler_linear, constants.inp.xy, 0.0).rg;");
			node_shader_write_frag(kong, "var wn: float3;");
			node_shader_write_frag(kong, "wn.z = 1.0 - abs(g0.x) - abs(g0.y);");
			// node_shader_write_frag(kong, "wn.xy = wn.z >= 0.0 ? g0.xy : octahedron_wrap(g0.xy);");
			node_shader_write_frag(kong, "if (wn.z >= 0.0) { wn.x = g0.x; wn.y = g0.y; } else { var f2: float2 = octahedron_wrap(g0.xy); wn.x = f2.x; wn.y = f2.y; }");
			node_shader_write_frag(kong, "wn = normalize(wn);");
			kong.frag_n = true;
			let angle: f32 = context_raw.brush_angle_reject_dot;
			node_shader_write_frag(kong, "if (dot(wn, n) < " + angle + ") { discard; }");
		}
		let stencil_fill: bool = context_raw.tool == tool_type_t.FILL && context_raw.brush_stencil_image != null;
		if (stencil_fill) {
			node_shader_write_frag(kong, "if (sp.z > sample_lod(gbufferD, sampler_linear, sp.xy, 0.0).r - 0.00005) { discard; }");
		}
	}

	if (context_raw.colorid_picked || face_fill || uv_island_fill) {
		node_shader_add_out(kong, "tex_coord_pick: float2");
		node_shader_write_vert(kong, "output.tex_coord_pick = input.tex;");
		if (context_raw.colorid_picked) {
			make_discard_color_id(kong);
		}
		if (face_fill) {
			make_discard_face(kong);
		}
		else if (uv_island_fill) {
			make_discard_uv_island(kong);
		}
	}

	if (context_raw.picker_mask_handle.position == picker_mask_t.MATERIAL) {
		make_discard_material_id(kong);
	}

	make_texcoord_run(kong);

	if (context_raw.tool == tool_type_t.CLONE || context_raw.tool == tool_type_t.BLUR || context_raw.tool == tool_type_t.SMUDGE) {
		node_shader_add_texture(kong, "gbuffer2");
		node_shader_add_constant(kong, "gbuffer_size: float2", "_gbuffer_size");
		node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo");
		node_shader_add_texture(kong, "texpaint_nor_undo", "_texpaint_nor_undo");
		node_shader_add_texture(kong, "texpaint_pack_undo", "_texpaint_pack_undo");

		if (context_raw.tool == tool_type_t.CLONE) {
			make_clone_run(kong);
		}
		else { // Blur, Smudge
			make_blur_run(kong);
		}
	}
	else {
		parser_material_parse_emission = context_raw.material.paint_emis;
		parser_material_parse_subsurface = context_raw.material.paint_subs;
		parser_material_parse_height = context_raw.material.paint_height;
		parser_material_parse_height_as_channel = true;
		let uv_type: uv_type_t = context_raw.layer.fill_layer != null ? context_raw.layer.uv_type : context_raw.brush_paint;
		parser_material_triplanar = uv_type == uv_type_t.TRIPLANAR && !decal;
		parser_material_sample_keep_aspect = decal;
		parser_material_sample_uv_scale = "constants.brush_scale";
		let sout: shader_out_t = parser_material_parse(ui_nodes_get_canvas_material(), con_paint, kong, matcon);
		parser_material_parse_emission = false;
		parser_material_parse_subsurface = false;
		parser_material_parse_height_as_channel = false;
		parser_material_parse_height = false;
		let base: string = sout.out_basecol;
		let rough: string = sout.out_roughness;
		let met: string = sout.out_metallic;
		let occ: string = sout.out_occlusion;
		let nortan: string = parser_material_out_normaltan;
		let height: string = sout.out_height;
		let opac: string = sout.out_opacity;
		let emis: string = sout.out_emission;
		let subs: string = sout.out_subsurface;
		node_shader_write_frag(kong, "var basecol: float3 = " + base + ";");
		node_shader_write_frag(kong, "var roughness: float = " + rough + ";");
		node_shader_write_frag(kong, "var metallic: float = " + met + ";");
		node_shader_write_frag(kong, "var occlusion: float = " + occ + ";");
		node_shader_write_frag(kong, "var nortan: float3 = " + nortan + ";");
		node_shader_write_frag(kong, "var height: float = " + height + ";");
		node_shader_write_frag(kong, "var mat_opacity: float = " + opac + ";");
		node_shader_write_frag(kong, "var opacity: float = mat_opacity;");
		///if is_forge
		node_shader_write_frag(kong, "opacity = 1.0;");
		///else
		if (context_raw.layer.fill_layer == null) {
			node_shader_write_frag(kong, "opacity *= constants.brush_opacity;");
		}
		///end
		if (context_raw.material.paint_emis) {
			node_shader_write_frag(kong, "var emis: float = " + emis + ";");
		}
		if (context_raw.material.paint_subs) {
			node_shader_write_frag(kong, "var subs: float = " + subs + ";");
		}
		if (!make_material_height_used && parse_float(height) != 0.0) {
			make_material_height_used = true;
			// Height used for the first time, also rebuild vertex shader
			return make_paint_run(data, matcon);
		}
		make_material_emis_used = parse_float(emis) != 0.0;
		make_material_subs_used = parse_float(subs) != 0.0;
	}

	if (context_raw.brush_mask_image != null && context_raw.tool == tool_type_t.DECAL) {
		node_shader_add_texture(kong, "texbrushmask", "_texbrushmask");
		node_shader_write_frag(kong, "var mask_sample: float4 = sample_lod(texbrushmask, sampler_linear, uvsp, 0.0);");
		if (context_raw.brush_mask_image_is_alpha) {
			node_shader_write_frag(kong, "opacity *= mask_sample.a;");
		}
		else {
			node_shader_write_frag(kong, "opacity *= mask_sample.r * mask_sample.a;");
		}
	}
	else if (context_raw.tool == tool_type_t.TEXT) {
		node_shader_add_texture(kong, "textexttool", "_textexttool");
		node_shader_write_frag(kong, "opacity *= sample_lod(textexttool, sampler_linear, uvsp, 0.0).r;");
	}

	if (context_raw.brush_stencil_image != null && (
		context_raw.tool == tool_type_t.BRUSH ||
		context_raw.tool == tool_type_t.ERASER ||
		context_raw.tool == tool_type_t.FILL ||
		context_raw.tool == tool_type_t.CLONE ||
		context_raw.tool == tool_type_t.BLUR ||
		context_raw.tool == tool_type_t.SMUDGE ||
		context_raw.tool == tool_type_t.PARTICLE ||
		decal)) {
		node_shader_add_texture(kong, "texbrushstencil", "_texbrushstencil");
		node_shader_add_constant(kong, "texbrushstencil_size: float2", "_size(_texbrushstencil)");
		node_shader_add_constant(kong, "stencil_transform: float4", "_stencil_transform");
		node_shader_write_frag(kong, "var stencil_uv: float2 = (sp.xy - constants.stencil_transform.xy) / constants.stencil_transform.z * float2(constants.aspect_ratio, 1.0);");
		node_shader_write_frag(kong, "var stencil_size: float2 = constants.texbrushstencil_size;");
		node_shader_write_frag(kong, "var stencil_ratio: float = stencil_size.y / stencil_size.x;");
		node_shader_write_frag(kong, "stencil_uv -= float2(0.5 / stencil_ratio, 0.5);");
		node_shader_write_frag(kong, "stencil_uv = float2(stencil_uv.x * cos(constants.stencil_transform.w) - stencil_uv.y * sin(constants.stencil_transform.w),\
												   stencil_uv.x * sin(constants.stencil_transform.w) + stencil_uv.y * cos(constants.stencil_transform.w));");
		node_shader_write_frag(kong, "stencil_uv += float2(0.5 / stencil_ratio, 0.5);");
		node_shader_write_frag(kong, "stencil_uv.x *= stencil_ratio;");
		node_shader_write_frag(kong, "if (stencil_uv.x < 0.0 || stencil_uv.x > 1.0 || stencil_uv.y < 0.0 || stencil_uv.y > 1.0) { discard; }");
		node_shader_write_frag(kong, "var texbrushstencil_sample: float4 = sample_lod(texbrushstencil, sampler_linear, stencil_uv, 0.0);");
		if (context_raw.brush_stencil_image_is_alpha) {
			node_shader_write_frag(kong, "opacity *= texbrushstencil_sample.a;");
		}
		else {
			node_shader_write_frag(kong, "opacity *= texbrushstencil_sample.r * texbrushstencil_sample.a;");
		}
	}

	if (context_raw.brush_mask_image != null && (context_raw.tool == tool_type_t.BRUSH || context_raw.tool == tool_type_t.ERASER)) {
		node_shader_add_texture(kong, "texbrushmask", "_texbrushmask");
		node_shader_write_frag(kong, "var binp_mask: float2 = constants.inp.xy * 2.0 - 1.0;");
		node_shader_write_frag(kong, "binp_mask.x *= constants.aspect_ratio;");
		node_shader_write_frag(kong, "binp_mask = binp_mask * 0.5 + 0.5;");
		node_shader_write_frag(kong, "var pa_mask: float2 = bsp.xy - binp_mask.xy;");
		if (context_raw.brush_directional) {
			node_shader_add_constant(kong, "brush_direction: float3", "_brush_direction");
			node_shader_write_frag(kong, "if (constants.brush_direction.z == 0.0) { discard; }");
			node_shader_write_frag(kong, "pa_mask = float2(pa_mask.x * constants.brush_direction.x - pa_mask.y * constants.brush_direction.y, pa_mask.x * constants.brush_direction.y + pa_mask.y * constants.brush_direction.x);");
		}
		let angle: f32 = context_raw.brush_angle + context_raw.brush_nodes_angle;
		if (angle != 0.0) {
			node_shader_add_constant(kong, "brush_angle: float2", "_brush_angle");
			node_shader_write_frag(kong, "pa_mask.xy = float2(pa_mask.x * constants.brush_angle.x - pa_mask.y * constants.brush_angle.y, pa_mask.x * constants.brush_angle.y + pa_mask.y * constants.brush_angle.x);");
		}
		node_shader_write_frag(kong, "pa_mask = pa_mask / constants.brush_radius;");
		if (config_raw.brush_3d) {
			node_shader_add_constant(kong, "eye: float3", "_camera_pos");
			node_shader_write_frag(kong, "pa_mask = pa_mask * (distance(constants.eye, winp.xyz) / 1.5);");
		}
		node_shader_write_frag(kong, "pa_mask = pa_mask.xy * 0.5 + 0.5;");
		node_shader_write_frag(kong, "var mask_sample: float4 = sample_lod(texbrushmask, sampler_linear, pa_mask, 0.0);");
		if (context_raw.brush_mask_image_is_alpha) {
			node_shader_write_frag(kong, "opacity *= mask_sample.a;");
		}
		else {
			node_shader_write_frag(kong, "opacity *= mask_sample.r * mask_sample.a;");
		}
	}

	node_shader_write_frag(kong, "if (opacity == 0.0) { discard; }");

	if (context_raw.tool == tool_type_t.PARTICLE) { // Particle mask
		make_particle_mask(kong);
	}
	else { // Brush cursor mask
		node_shader_write_frag(kong, "var str: float = clamp((constants.brush_radius - dist) * constants.brush_hardness * 400.0, 0.0, 1.0) * opacity;");
		// node_shader_write_frag(kong, "var str: float = pow(clamp(1.0 - (dist / constants.brush_radius), 0.0, 1.0), 1.0 - constants.brush_hardness) * opacity;");
		// node_shader_write_frag(kong, "var t: float = clamp(dist / constants.brush_radius, 0.0, 1.0); var falloff: float = 1.0 - smoothstep(0.0, 1.0, t); var str: float = pow(falloff, 1.0 / max(constants.brush_hardness, 0.01)) * opacity;");
	}

	// Manual blending to preserve memory
	kong.frag_wvpposition = true;
	node_shader_write_frag(kong, "var sample_tc: float2 = float2(input.wvpposition.x / input.wvpposition.w, input.wvpposition.y / input.wvpposition.w) * 0.5 + 0.5;");
	node_shader_write_frag(kong, "sample_tc.y = 1.0 - sample_tc.y;");
	node_shader_add_texture(kong, "paintmask");
	node_shader_write_frag(kong, "var sample_mask: float = sample_lod(paintmask, sampler_linear, sample_tc, 0.0).r;");
	node_shader_write_frag(kong, "str = max(str, sample_mask);");
	// write(frag, "str = clamp(str + sample_mask, 0.0, 1.0);");

	node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo");
	node_shader_write_frag(kong, "var sample_undo: float4 = sample_lod(texpaint_undo, sampler_linear, sample_tc, 0.0);");

	let matid: f32 = context_raw.material.id / 255;
	if (context_raw.picker_mask_handle.position == picker_mask_t.MATERIAL) {
		matid = context_raw.materialid_picked / 255; // Keep existing material id in place when mask is set
	}
	let matid_string: string = parser_material_vec1(matid * 3.0);
	node_shader_write_frag(kong, "var matid: float = " + matid_string + ";");

	// matid % 3 == 0 - normal, 1 - emission, 2 - subsurface
	if (context_raw.material.paint_emis && make_material_emis_used) {
		node_shader_write_frag(kong, "if (emis > 0.0) {");
		node_shader_write_frag(kong, "	matid += 1.0 / 255.0;");
		node_shader_write_frag(kong, "	if (str == 0.0) { discard; }");
		node_shader_write_frag(kong, "}");
	}
	else if (context_raw.material.paint_subs && make_material_subs_used) {
		node_shader_write_frag(kong, "if (subs > 0.0) {");
		node_shader_write_frag(kong, "	matid += 2.0 / 255.0;");
		node_shader_write_frag(kong, "	if (str == 0.0) { discard; }");
		node_shader_write_frag(kong, "}");
	}

	let is_mask: bool = slot_layer_is_mask(context_raw.layer);
	let layered: bool = context_raw.layer != project_layers[0];
	if (layered && !is_mask) {
		if (context_raw.tool == tool_type_t.ERASER) {
			node_shader_write_frag(kong, "output[0] = float4(lerp3(sample_undo.rgb, float3(0.0, 0.0, 0.0), str), sample_undo.a - str);");
			node_shader_write_frag(kong, "nortan = float3(0.5, 0.5, 1.0);");
			node_shader_write_frag(kong, "occlusion = 1.0;");
			node_shader_write_frag(kong, "roughness = 0.0;");
			node_shader_write_frag(kong, "metallic = 0.0;");
			node_shader_write_frag(kong, "matid = 0.0;");
		}
		else if (context_raw.tool == tool_type_t.PARTICLE || decal || context_raw.brush_mask_image != null) {
			node_shader_write_frag(kong, "output[0] = float4(" + make_material_blend_mode(kong, context_raw.brush_blending, "sample_undo.rgb", "basecol", "str") + ", max(str, sample_undo.a));");
		}
		else {
			if (context_raw.layer.fill_layer != null) {
				node_shader_write_frag(kong, "output[0] = float4(" + make_material_blend_mode(kong, context_raw.brush_blending, "sample_undo.rgb", "basecol", "opacity") + ", mat_opacity);");
			}
			else {
				node_shader_write_frag(kong, "output[0] = float4(" + make_material_blend_mode(kong, context_raw.brush_blending, "sample_undo.rgb", "basecol", "opacity") + ", max(str, sample_undo.a));");
			}
		}
		node_shader_write_frag(kong, "output[1] = float4(nortan, matid);");

		let height: string = "0.0";
		if (context_raw.material.paint_height && make_material_height_used) {
			height = "height";
		}

		if (decal) {
			node_shader_add_texture(kong, "texpaint_pack_undo", "_texpaint_pack_undo");
			node_shader_write_frag(kong, "var sample_pack_undo: float4 = sample_lod(texpaint_pack_undo, sampler_linear, sample_tc, 0.0);");
			node_shader_write_frag(kong, "output[2] = lerp4(sample_pack_undo, float4(occlusion, roughness, metallic, " + height + "), str);");
		}
		else {
			node_shader_write_frag(kong, "output[2] = float4(occlusion, roughness, metallic, " + height + ");");
		}
	}
	else {
		if (context_raw.tool == tool_type_t.ERASER) {
			node_shader_write_frag(kong, "output[0] = float4(lerp3(sample_undo.rgb, float3(0.0, 0.0, 0.0), str), sample_undo.a - str);");
			node_shader_write_frag(kong, "output[1] = float4(0.5, 0.5, 1.0, 0.0);");
			node_shader_write_frag(kong, "output[2] = float4(1.0, 0.0, 0.0, 0.0);");
		}
		else {
			node_shader_add_texture(kong, "texpaint_nor_undo", "_texpaint_nor_undo");
			node_shader_add_texture(kong, "texpaint_pack_undo", "_texpaint_pack_undo");
			node_shader_write_frag(kong, "var sample_nor_undo: float4 = sample_lod(texpaint_nor_undo, sampler_linear, sample_tc, 0.0);");
			node_shader_write_frag(kong, "var sample_pack_undo: float4 = sample_lod(texpaint_pack_undo, sampler_linear, sample_tc, 0.0);");
			///if is_forge
			node_shader_write_frag(kong, "output[0] = float4(" + make_material_blend_mode(kong, context_raw.brush_blending, "sample_undo.rgb", "basecol", "str") + ", mat_opacity);");
			///else
			node_shader_write_frag(kong, "output[0] = float4(" + make_material_blend_mode(kong, context_raw.brush_blending, "sample_undo.rgb", "basecol", "str") + ", max(str, sample_undo.a));");
			///end
			node_shader_write_frag(kong, "output[1] = float4(lerp3(sample_nor_undo.rgb, nortan, str), matid);");
			if (context_raw.material.paint_height && make_material_height_used) {
				node_shader_write_frag(kong, "output[2] = lerp4(sample_pack_undo, float4(occlusion, roughness, metallic, height), str);");
			}
			else {
				node_shader_write_frag(kong, "output[2] = float4(lerp3(sample_pack_undo.rgb, float3(occlusion, roughness, metallic), str), 0.0);");
			}
		}
	}
	node_shader_write_frag(kong, "output[3] = float4(str, 0.0, 0.0, 1.0);");

	if (!context_raw.material.paint_base) {
		con_paint.data.color_writes_red[0] = false;
		con_paint.data.color_writes_green[0] = false;
		con_paint.data.color_writes_blue[0] = false;
	}
	if (!context_raw.material.paint_opac) {
		con_paint.data.color_writes_alpha[0] = false;
	}
	if (!context_raw.material.paint_nor) {
		con_paint.data.color_writes_red[1] = false;
		con_paint.data.color_writes_green[1] = false;
		con_paint.data.color_writes_blue[1] = false;
	}
	if (!context_raw.material.paint_occ) {
		con_paint.data.color_writes_red[2] = false;
	}
	if (!context_raw.material.paint_rough) {
		con_paint.data.color_writes_green[2] = false;
	}
	if (!context_raw.material.paint_met) {
		con_paint.data.color_writes_blue[2] = false;
	}
	if (!context_raw.material.paint_height) {
		con_paint.data.color_writes_alpha[2] = false;
	}

	// Base color only as mask
	if (is_mask) {
		// TODO: Apply opacity into base
		// write(frag, "frag_color[0].rgb *= frag_color[0].a;");
		con_paint.data.color_writes_green[0] = false;
		con_paint.data.color_writes_blue[0] = false;
		con_paint.data.color_writes_alpha[0] = false;
		con_paint.data.color_writes_red[1] = false;
		con_paint.data.color_writes_green[1] = false;
		con_paint.data.color_writes_blue[1] = false;
		con_paint.data.color_writes_alpha[1] = false;
		con_paint.data.color_writes_red[2] = false;
		con_paint.data.color_writes_green[2] = false;
		con_paint.data.color_writes_blue[2] = false;
		con_paint.data.color_writes_alpha[2] = false;
	}

	if (context_raw.tool == tool_type_t.BAKE) {
		make_bake_run(con_paint, kong);
	}

	parser_material_finalize(con_paint);
	parser_material_triplanar = false;
	parser_material_sample_keep_aspect = false;
	con_paint.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader), ADDRESS(con_paint.data._.vertex_shader_size), ADDRESS(con_paint.data._.fragment_shader_size));

	return con_paint;
}

///end
