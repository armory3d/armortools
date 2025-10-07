
let sculpt_push_undo: bool = false;

function sculpt_import_mesh_pack_to_texture(mesh: mesh_data_t, l: slot_layer_t) {
	let b: buffer_t = buffer_create(config_get_texture_res_x() * config_get_texture_res_y() * 4 * 4);
	for (let i: i32 = 0; i < math_floor(mesh.index_array.length); ++i) {
		let index: i32 = mesh.index_array[i];
		buffer_set_f32(b, 4 * i * 4,         mesh.vertex_arrays[0].values[index * 4]     / 32767);
		buffer_set_f32(b, 4 * i * 4 + 1 * 4, mesh.vertex_arrays[0].values[index * 4 + 1] / 32767);
		buffer_set_f32(b, 4 * i * 4 + 2 * 4, mesh.vertex_arrays[0].values[index * 4 + 2] / 32767);
		buffer_set_f32(b, 4 * i * 4 + 3 * 4, 1.0);
	}

	let imgmesh: gpu_texture_t = gpu_create_texture_from_bytes(b, config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.RGBA128);
	draw_begin(l.texpaint_sculpt);
	draw_set_pipeline(pipes_copy128);
	draw_scaled_image(imgmesh, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
	draw_set_pipeline(null);
	draw_end();
}

function sculpt_make_sculpt_run(data: material_t, matcon: material_context_t): node_shader_context_t {
	let context_id: string = "paint";
	let props: shader_context_t = {
		name: context_id,
		depth_write: false,
		compare_mode: "always",
		cull_mode: "none",
		vertex_elements: [
			{
				name: "pos",
				data: "float2"
			}
		],
		color_attachments: [
			"RGBA128",
			"R8"
		]
	};
	let con_paint: node_shader_context_t = node_shader_context_create(data, props);
	con_paint.data.color_writes_red = [true, true, true, true];
	con_paint.data.color_writes_green = [true, true, true, true];
	con_paint.data.color_writes_blue = [true, true, true, true];
	con_paint.data.color_writes_alpha = [true, true, true, true];

	let kong: node_shader_t = node_shader_context_make_kong(con_paint);
	let decal: bool = context_is_decal();
	node_shader_add_out(kong, "tex_coord: float2");
	node_shader_write_vert(kong, "var madd: float2 = float2(0.5, 0.5);");
	node_shader_write_vert(kong, "output.tex_coord = input.pos.xy * madd + madd;");
	node_shader_write_vert(kong, "output.tex_coord.y = 1.0 - output.tex_coord.y;");
	node_shader_write_vert(kong, "output.pos = float4(input.pos.xy, 0.0, 1.0);");
	node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.tex_coord;");
	node_shader_add_constant(kong, "inp: float4", "_input_brush");
	node_shader_add_constant(kong, "inplast: float4", "_input_brush_last");
	node_shader_add_texture(kong, "gbufferD");
	kong.frag_out = "float4[2]";
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
		node_shader_write_frag(kong, "var dist: float = 0.0;");
		node_shader_write_frag(kong, "var depth: float = sample_lod(gbufferD, sampler_linear, constants.inp.xy, 0.0).r;");
		node_shader_add_constant(kong, "invVP: float4x4", "_inv_view_proj_matrix");
		// node_shader_write_frag(kong, "var winp: float4 = float4(float2(constants.inp.x, 1.0 - constants.inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "var winp: float4 = float4(float2(constants.inp.x, 1.0 - constants.inp.y) * 2.0 - 1.0, depth, 1.0);");
		node_shader_write_frag(kong, "winp = constants.invVP * winp;");
		node_shader_write_frag(kong, "winp.xyz = winp.xyz / winp.w;");
		node_shader_add_constant(kong, "W: float4x4", "_world_matrix");
		// node_shader_add_constant(kong, "texpaint_undo_size: float2", "_size(texpaint_sculpt_undo)"); ////
		// node_shader_write_attrib_frag(kong, "var wposition: float3 = (constants.W * texpaint_sculpt_undo[uint2(uint(tex_coord.x * constants.texpaint_undo_size.x), uint(tex_coord.y * constants.texpaint_undo_size.y))]).xyz;");
		node_shader_write_attrib_frag(kong, "var wposition: float3 = (constants.W * texpaint_sculpt_undo[uint2(uint(tex_coord.x * 2048.0), uint(tex_coord.y * 2048.0))]).xyz;");
		node_shader_write_frag(kong, "var depthlast: float = sample_lod(gbufferD, sampler_linear, constants.inplast.xy, 0.0).r;");
		node_shader_write_frag(kong, "var winplast: float4 = float4(float2(constants.inplast.x, 1.0 - constants.inplast.y) * 2.0 - 1.0, depthlast, 1.0);");
		node_shader_write_frag(kong, "winplast = constants.invVP * winplast;");
		node_shader_write_frag(kong, "winplast.xyz = winplast.xyz / winplast.w;");
		node_shader_write_frag(kong, "dist = distance(wposition.xyz, winp.xyz);");
		node_shader_write_frag(kong, "if (dist > constants.brush_radius) { discard; }");
	}

	node_shader_write_frag(kong, "var basecol: float3 = float3(1.0, 1.0, 1.0);");
	node_shader_write_frag(kong, "var opacity: float = 1.0;");
	node_shader_write_frag(kong, "if (opacity == 0.0) { discard; }");
	node_shader_write_frag(kong, "var str: float = clamp((constants.brush_radius - dist) * constants.brush_hardness * 1.0, 0.0, 1.0) * opacity;");
	node_shader_add_texture(kong, "texpaint_sculpt_undo", "_texpaint_sculpt_undo");
	node_shader_write_frag(kong, "var sample_undo: float4 = sample_lod(texpaint_sculpt_undo, sampler_linear, tex_coord, 0.0);");
	node_shader_write_frag(kong, "if (sample_undo.r == 0.0 && sample_undo.g == 0.0 && sample_undo.b == 0.0) { discard; }");
	node_shader_add_function(kong, str_octahedron_wrap);
	node_shader_add_texture(kong, "gbuffer0_undo");
	node_shader_write_frag(kong, "var g0_undo: float2 = sample_lod(gbuffer0_undo, sampler_linear, constants.inp.xy, 0.0).rg;");
	node_shader_write_frag(kong, "var wn: float3;");
	node_shader_write_frag(kong, "wn.z = 1.0 - abs(g0_undo.x) - abs(g0_undo.y);");
	// node_shader_write_frag(kong, "wn.xy = wn.z >= 0.0 ? g0_undo.xy : octahedron_wrap(g0_undo.xy);");
	node_shader_write_frag(kong, "if (wn.z >= 0.0) { wn.xy = g0_undo.xy; } else { wn.xy = octahedron_wrap(g0_undo.xy); }");
	node_shader_write_frag(kong, "var n: float3 = normalize(wn);");
	node_shader_write_frag(kong, "output[0] = float4(sample_undo.rgb + n * 0.1 * str, 1.0);");
	node_shader_write_frag(kong, "output[1] = float4(str, 0.0, 0.0, 1.0);");
	parser_material_finalize(con_paint);
	con_paint.data.shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), ADDRESS(con_paint.data.vertex_shader), ADDRESS(con_paint.data.fragment_shader), ADDRESS(con_paint.data._.vertex_shader_size), ADDRESS(con_paint.data._.fragment_shader_size));
	return con_paint;
}

function sculpt_make_mesh_run(kong: node_shader_t, l: slot_layer_t) {
	node_shader_add_constant(kong, "WVP: float4x4", "_world_view_proj_matrix");
	let lid: i32 = l.id;
	node_shader_add_texture(kong, "texpaint_sculpt", "_texpaint_sculpt" + lid);
	node_shader_add_constant(kong, "texpaint_sculpt_size: float2", "_size(_texpaint_sculpt" + lid + ")");
	// node_shader_write_vert(kong, "var meshpos: float3 = sample_lod(texpaint_sculpt, sampler_linear, uint2(vertex_id() % constants.texpaint_sculpt_size.x, vertex_id() / constants.texpaint_sculpt_size.y), 0.0).xyz;");
	node_shader_write_vert(kong, "var meshpos: float4 = texpaint_sculpt[uint2(uint(float(vertex_id()) % constants.texpaint_sculpt_size.x), uint(float(vertex_id()) / constants.texpaint_sculpt_size.y))];");
	// + input.pos.xyz * 0.000001
	node_shader_write_vert(kong, "output.pos = constants.WVP * float4(meshpos.xyz, 1.0);");

	kong.frag_n = false;
	node_shader_add_constant(kong, "N: float3x3", "_normal_matrix");
	node_shader_add_out(kong, "wnormal: float3");
	node_shader_write_attrib_vert(kong, "var base_vertex0: float = float(vertex_id()) - (float(vertex_id()) % float(3));");
	node_shader_write_attrib_vert(kong, "var base_vertex1: float = base_vertex0 + 1.0;");
	node_shader_write_attrib_vert(kong, "var base_vertex2: float = base_vertex0 + 2.0;");
	// node_shader_write_attrib_vert(kong, "var meshpos0: float3 = sample_lod(texpaint_sculpt, sampler_linear, uint2(base_vertex0 % constants.texpaint_sculpt_size.x, base_vertex0 / constants.texpaint_sculpt_size.y), 0.0).xyz;");
	// node_shader_write_attrib_vert(kong, "var meshpos1: float3 = sample_lod(texpaint_sculpt, sampler_linear, uint2(base_vertex1 % constants.texpaint_sculpt_size.x, base_vertex1 / constants.texpaint_sculpt_size.y), 0.0).xyz;");
	// node_shader_write_attrib_vert(kong, "var meshpos2: float3 = sample_lod(texpaint_sculpt, sampler_linear, uint2(base_vertex2 % constants.texpaint_sculpt_size.x, base_vertex2 / constants.texpaint_sculpt_size.y), 0.0).xyz;");
	node_shader_write_attrib_vert(kong, "var meshpos0: float4 = texpaint_sculpt[uint2(uint(base_vertex0 % constants.texpaint_sculpt_size.x), uint(base_vertex0 / constants.texpaint_sculpt_size.y))];");
	node_shader_write_attrib_vert(kong, "var meshpos1: float4 = texpaint_sculpt[uint2(uint(base_vertex1 % constants.texpaint_sculpt_size.x), uint(base_vertex1 / constants.texpaint_sculpt_size.y))];");
	node_shader_write_attrib_vert(kong, "var meshpos2: float4 = texpaint_sculpt[uint2(uint(base_vertex2 % constants.texpaint_sculpt_size.x), uint(base_vertex2 / constants.texpaint_sculpt_size.y))];");
	node_shader_write_attrib_vert(kong, "var meshnor: float3 = normalize(cross(meshpos2.xyz - meshpos1.xyz, meshpos0.xyz - meshpos1.xyz));");
	node_shader_write_attrib_vert(kong, "output.wnormal = constants.N * meshnor;");
	node_shader_write_attrib_frag(kong, "var n: float3 = normalize(input.wnormal);");
}

function sculpt_layers_create_sculpt_layer() {
	let l: slot_layer_t = layers_new_layer();
	let id: i32 = l.id;

	{
		let t: render_target_t = render_target_create();
		t.name = "texpaint_sculpt" + id;
		t.width = config_get_texture_res_x();
		t.height = config_get_texture_res_y();
		t.format = "RGBA128";
		l.texpaint_sculpt = render_path_create_render_target(t)._image;
	}

	// util_mesh_merge();
	// let md: mesh_data_t = context_raw.merged_object.data;

	let md: mesh_data_t = context_raw.paint_object.data;
	sculpt_import_mesh_pack_to_texture(md, l);

	let posa: i16_array_t = i16_array_create(md.index_array.length * 4);
	let nora: i16_array_t = i16_array_create(md.index_array.length * 2);
	let texa: i16_array_t = i16_array_create(md.index_array.length * 2);
	for (let i: i32 = 0; i < posa.length; ++i) {
		posa[i] = 32767;
	}
	for (let i: i32 = 0; i < md.index_array.length; ++i) {
		let index: i32 = md.index_array[i];
		posa[i * 4    ] = md.vertex_arrays[0].values[index * 4    ];
		posa[i * 4 + 1] = md.vertex_arrays[0].values[index * 4 + 1];
		posa[i * 4 + 2] = md.vertex_arrays[0].values[index * 4 + 2];
		posa[i * 4 + 3] = md.vertex_arrays[0].values[index * 4 + 3];
		nora[i * 2    ] = md.vertex_arrays[1].values[index * 2    ];
		nora[i * 2 + 1] = md.vertex_arrays[1].values[index * 2 + 1];
		texa[i * 2    ] = md.vertex_arrays[2].values[index * 2    ];
		texa[i * 2 + 1] = md.vertex_arrays[2].values[index * 2 + 1];
	}
	let inda: u32_array_t = u32_array_create(md.index_array.length);
	for (let i: i32 = 0; i < inda.length; ++i) {
		inda[i] = i;
	}
	let raw: mesh_data_t = {
		name: md.name,
		vertex_arrays: [
			{
				values: posa,
				attrib: "pos",
				data: "short4norm"
			},
			{
				values: nora,
				attrib: "nor",
				data: "short2norm"
			},
			{
				values: texa,
				attrib: "tex",
				data: "short2norm"
			}
		],
		index_array: inda,
		scale_pos: 1.0,
		scale_tex: 1.0
	};
	md = mesh_data_create(raw);
	mesh_object_set_data(context_raw.paint_object, md);

	make_material_parse_paint_material();
	make_material_parse_mesh_material();

	{
		let t: render_target_t = render_target_create();
		t.name = "gbuffer0_undo";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA64";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		let t: render_target_t = render_target_create();
		t.name = "gbufferD_undo";
		t.width = 0;
		t.height = 0;
		t.format = "R32";
		t.scale = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}

	render_path_load_shader("Scene/copy_pass/copyR32_pass");

	for (let i: i32 = 0; i < history_undo_layers.length; ++i) {
		let len: i32 = history_undo_layers.length;
		let ext: string = "_undo" + i;
		let l: slot_layer_t = history_undo_layers[i];

		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_sculpt" + ext;
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA128";
			l.texpaint_sculpt = render_path_create_render_target(t)._image;
		}
	}
}

function render_path_sculpt_commands() {
	if (context_raw.pdirty <= 0) {
		return;
	}

	let tid: i32 = context_raw.layer.id;
	let texpaint_sculpt: string = "texpaint_sculpt" + tid;
	render_path_set_target("texpaint_blend1");
	render_path_bind_target("texpaint_blend0", "tex");
	render_path_draw_shader("Scene/copy_pass/copyR8_pass");
	let additional: string[] = ["texpaint_blend0"];
	render_path_set_target(texpaint_sculpt, additional);
	render_path_bind_target("gbufferD_undo", "gbufferD");
	if (context_raw.xray || config_raw.brush_angle_reject) {
	    render_path_bind_target("gbuffer0", "gbuffer0");
	}
	render_path_bind_target("texpaint_blend1", "paintmask");
	render_path_bind_target("gbuffer0_undo", "gbuffer0_undo");

	let material_context: material_context_t = null;
	let shader_context: shader_context_t = null;
	let mat: material_data_t = project_paint_objects[0].material;
	for (let j: i32 = 0; j < mat.contexts.length; ++j) {
		if (mat.contexts[j].name == "paint") {
			shader_context = shader_data_get_context(mat._.shader, "paint");
			material_context = mat.contexts[j];
			break;
		}
	}

	gpu_set_pipeline(shader_context._.pipe);
	uniforms_set_context_consts(shader_context, _render_path_bind_params);
	uniforms_set_obj_consts(shader_context, project_paint_objects[0].base);
	uniforms_set_material_consts(shader_context, material_context);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();
	render_path_end();
}

function render_path_sculpt_begin() {
	if (!render_path_paint_paint_enabled()) {
		return;
	}
	render_path_paint_push_undo_last = history_push_undo;
	if (history_push_undo && history_undo_layers != null) {
		history_paint();
		render_path_set_target("gbuffer0_undo");
		render_path_bind_target("gbuffer0", "tex");
		render_path_draw_shader("Scene/copy_pass/copyRGBA64_pass");
		render_path_set_target("gbufferD_undo");
		render_path_bind_target("main", "tex");
		render_path_draw_shader("Scene/copy_pass/copyR32_pass");
	}
	if (sculpt_push_undo && history_undo_layers != null) {
		history_paint();
	}
}
