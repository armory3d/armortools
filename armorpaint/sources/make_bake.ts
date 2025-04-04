
function make_bake_run(con: node_shader_context_t, kong: node_shader_t) {
	if (context_raw.bake_type == bake_type_t.CURVATURE) {
		let pass: bool = parser_material_bake_passthrough;
		let strength: string = pass ? parser_material_bake_passthrough_strength : context_raw.bake_curv_strength + "";
		let radius: string = pass ? parser_material_bake_passthrough_radius : context_raw.bake_curv_radius + "";
		let offset: string = pass ? parser_material_bake_passthrough_offset : context_raw.bake_curv_offset + "";
		strength = "float(" + strength + ")";
		radius = "float(" + radius + ")";
		offset = "float(" + offset + ")";
		kong.frag_n = true;
		node_shader_write_frag(kong, "var dx: float3 = ddx3(n);");
		node_shader_write_frag(kong, "var dy: float3 = ddy3(n);");
		node_shader_write_frag(kong, "var curvature: float = max(dot(dx, dx), dot(dy, dy));");
		node_shader_write_frag(kong, "curvature = clamp(pow(curvature, (1.0 / " + radius + ") * 0.25) * " + strength + " * 2.0 + " + offset + " / 10.0, 0.0, 1.0);");
		if (context_raw.bake_axis != bake_axis_t.XYZ) {
			let axis: string = make_bake_axis_string(context_raw.bake_axis);
			node_shader_write_frag(kong, "curvature *= dot(n, " + axis + ");");
		}
		node_shader_write_frag(kong, "output[0].rgba = float4(curvature, curvature, curvature, 1.0);");
	}
	else if (context_raw.bake_type == bake_type_t.NORMAL) { // Tangent
		kong.frag_n = true;
		node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo"); // Baked high-poly normals
		node_shader_write_frag(kong, "var n0: float3 = sample_lod(texpaint_undo, input.tex_coord, 0.0).rgb * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
		node_shader_add_function(kong, str_cotangent_frame);
		node_shader_add_function(kong, str_transpose);
		node_shader_write_frag(kong, "var invTBN: float3x3 = _transpose(cotangent_frame(n, n, input.tex_coord));");
		node_shader_write_frag(kong, "var res: float3 = normalize(invTBN * n0) * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5);");
		node_shader_write_frag(kong, "output[0].rgba = float4(res, 1.0);");
	}
	else if (context_raw.bake_type == bake_type_t.NORMAL_OBJECT) {
		kong.frag_n = true;
		node_shader_write_frag(kong, "output[0].rgba = float4(n * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5), 1.0);");
		if (context_raw.bake_up_axis == bake_up_axis_t.Y) {
			node_shader_write_frag(kong, "output[0].rgb = float3(output[0].r, output[0].b, 1.0 - output[0].g);");
		}
	}
	else if (context_raw.bake_type == bake_type_t.HEIGHT) {
		kong.frag_wposition = true;
		node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo"); // Baked high-poly positions
		node_shader_write_frag(kong, "var wpos0: float3 = sample_lod(texpaint_undo, input.tex_coord, 0.0).rgb * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);");
		node_shader_write_frag(kong, "var res: float = distance(wpos0, wposition) * 10.0;");
		node_shader_write_frag(kong, "output[0].rgba = float4(res, res, res, 1.0);");
	}
	else if (context_raw.bake_type == bake_type_t.DERIVATIVE) {
		node_shader_add_texture(kong, "texpaint_undo", "_texpaint_undo"); // Baked height
		node_shader_write_frag(kong, "var tex_dx: float2 = ddx2(input.tex_coord);");
		node_shader_write_frag(kong, "var tex_dy: float2 = ddy2(input.tex_coord);");
		node_shader_write_frag(kong, "var h0: float = sample_lod(texpaint_undo, input.tex_coord, 0.0).r * 100;");
		node_shader_write_frag(kong, "var h1: float = sample_lod(texpaint_undo, input.tex_coord + tex_dx, 0.0).r * 100;");
		node_shader_write_frag(kong, "var h2: float = sample_lod(texpaint_undo, input.tex_coord + tex_dy, 0.0).r * 100;");
		node_shader_write_frag(kong, "output[0].rgba = float4((h1 - h0) * 0.5 + 0.5, (h2 - h0) * 0.5 + 0.5, 0.0, 1.0);");
	}
	else if (context_raw.bake_type == bake_type_t.POSITION) {
		kong.frag_wposition = true;
		node_shader_write_frag(kong, "output[0].rgba = float4(wposition * float3(0.5, 0.5, 0.5) + float3(0.5, 0.5, 0.5), 1.0);");
		if (context_raw.bake_up_axis == bake_up_axis_t.Y) {
			node_shader_write_frag(kong, "output[0].rgb = float3(output[0].r, output[0].b, 1.0 - output[0].g);");
		}
	}
	else if (context_raw.bake_type == bake_type_t.TEXCOORD) {
		node_shader_write_frag(kong, "output[0].rgba = float4(input.tex_coord.xy, 0.0, 1.0);");
	}
	else if (context_raw.bake_type == bake_type_t.MATERIALID) {
		node_shader_add_texture(kong, "texpaint_nor_undo", "_texpaint_nor_undo");
		node_shader_write_frag(kong, "var sample_matid: float = sample_lod(texpaint_nor_undo, input.tex_coord, 0.0).a + 1.0 / 255.0;");
		node_shader_write_frag(kong, "var matid_r: float = frac(sin(dot(float2(sample_matid, sample_matid * 20.0), float2(12.9898, 78.233))) * 43758.5453);");
		node_shader_write_frag(kong, "var matid_g: float = frac(sin(dot(float2(sample_matid * 20.0, sample_matid), float2(12.9898, 78.233))) * 43758.5453);");
		node_shader_write_frag(kong, "var matid_b: float = frac(sin(dot(float2(sample_matid, sample_matid * 40.0), float2(12.9898, 78.233))) * 43758.5453);");
		node_shader_write_frag(kong, "output[0].rgba = float4(matid_r, matid_g, matid_b, 1.0);");
	}
	else if (context_raw.bake_type == bake_type_t.OBJECTID) {
		node_shader_add_constant(kong, "object_id: float", "_object_id");
		node_shader_write_frag(kong, "var obid: float = constants.object_id + 1.0 / 255.0;");
		node_shader_write_frag(kong, "var id_r: float = frac(sin(dot(float2(obid, obid * 20.0), float2(12.9898, 78.233))) * 43758.5453);");
		node_shader_write_frag(kong, "var id_g: float = frac(sin(dot(float2(obid * 20.0, obid), float2(12.9898, 78.233))) * 43758.5453);");
		node_shader_write_frag(kong, "var id_b: float = frac(sin(dot(float2(obid, obid * 40.0), float2(12.9898, 78.233))) * 43758.5453);");
		node_shader_write_frag(kong, "output[0].rgba = float4(id_r, id_g, id_b, 1.0);");
	}
	else if (context_raw.bake_type == bake_type_t.VERTEX_COLOR) {
		if (con.allow_vcols) {
			node_shader_context_add_elem(con, "col", "short4norm");
			node_shader_write_frag(kong, "output[0].rgba = float4(vcolor.r, vcolor.g, vcolor.b, 1.0);");
		}
		else {
			node_shader_write_frag(kong, "output[0].rgba = float4(1.0, 1.0, 1.0, 1.0);");
		}
	}
}

function make_bake_position_normal(kong: node_shader_t) {
	node_shader_add_out(kong, "position: float3");
	node_shader_add_out(kong, "normal: float3");
	node_shader_add_constant(kong, "W: float4x4", "_world_matrix");
	node_shader_write_vert(kong, "output.position = float4(constants.W * float4(input.pos.xyz, 1.0)).xyz;");
	node_shader_write_vert(kong, "output.normal = float3(input.nor.xy, input.pos.w);");
	node_shader_write_vert(kong, "var tpos: float2 = float2(input.tex.x * 2.0 - 1.0, (1.0 - input.tex.y) * 2.0 - 1.0);");
	node_shader_write_vert(kong, "output.pos = float4(tpos, 0.0, 1.0);");
	kong.frag_out = "float4[2]";
	node_shader_write_frag(kong, "output[0].rgba = float4(input.position, 1.0);");
	node_shader_write_frag(kong, "output[1].rgba = float4(input.normal, 1.0);");
}

function make_bake_set_color_writes(con_paint: node_shader_context_t) {
	// Bake into base color, disable other slots
	con_paint.data.color_writes_red[1] = false;
	con_paint.data.color_writes_green[1] = false;
	con_paint.data.color_writes_blue[1] = false;
	con_paint.data.color_writes_alpha[1] = false;
	con_paint.data.color_writes_red[2] = false;
	con_paint.data.color_writes_green[2] = false;
	con_paint.data.color_writes_blue[2] = false;
	con_paint.data.color_writes_alpha[2] = false;
}

function make_bake_axis_string(i: i32): string {
	return i == bake_axis_t.X  ? "float3(1,0,0)"  :
		   i == bake_axis_t.Y  ? "float3(0,1,0)"  :
		   i == bake_axis_t.Z  ? "float3(0,0,1)"  :
		   i == bake_axis_t.MX ? "float3(-1,0,0)" :
		   i == bake_axis_t.MY ? "float3(0,-1,0)" :
								 "float3(0,0,-1)";
}
