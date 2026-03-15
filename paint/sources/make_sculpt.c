
#include "global.h"

void sculpt_import_mesh_pack_to_texture(mesh_data_t *mesh, slot_layer_t *l) {
	buffer_t *b = buffer_create(config_get_texture_res_x() * config_get_texture_res_y() * 4 * 4);
	for (i32 i = 0; i < math_floor(mesh->index_array->length); ++i) {
		i32 index = mesh->index_array->buffer[i];
		buffer_set_f32(b, 4 * i * 4, mesh->vertex_arrays->buffer[0]->values->buffer[index * 4] / 32767.0);
		buffer_set_f32(b, 4 * i * 4 + 1 * 4, mesh->vertex_arrays->buffer[0]->values->buffer[index * 4 + 1] / 32767.0);
		buffer_set_f32(b, 4 * i * 4 + 2 * 4, mesh->vertex_arrays->buffer[0]->values->buffer[index * 4 + 2] / 32767.0);
		buffer_set_f32(b, 4 * i * 4 + 3 * 4, 1.0);
	}

	gpu_texture_t *imgmesh = gpu_create_texture_from_bytes(b, config_get_texture_res_x(), config_get_texture_res_y(), GPU_TEXTURE_FORMAT_RGBA128);
	draw_begin(l->texpaint_sculpt, false, 0);
	draw_set_pipeline(pipes_copy128);
	draw_scaled_image(imgmesh, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
	draw_set_pipeline(NULL);
	draw_end();
}

node_shader_context_t *sculpt_make_sculpt_run(material_t *data, material_context_t *matcon) {
	char                  *context_id = "paint";
	shader_context_t      *props      = GC_ALLOC_INIT(shader_context_t, {.name            = context_id,
	                                                                     .depth_write     = false,
	                                                                     .compare_mode    = "always",
	                                                                     .cull_mode       = "none",
	                                                                     .vertex_elements = any_array_create_from_raw(
                                                                   (void *[]){
                                                                       GC_ALLOC_INIT(vertex_element_t, {.name = "pos", .data = "float2"}),
                                                                   },
                                                                   1),
	                                                                     .color_attachments = any_array_create_from_raw(
                                                                   (void *[]){
                                                                       "RGBA128",
                                                                       "R8",
                                                                   },
                                                                   2)});
	node_shader_context_t *con_paint  = node_shader_context_create(data, props);
	con_paint->data->color_writes_red = u8_array_create_from_raw(
	    (u8[]){
	        true,
	        true,
	        true,
	        true,
	    },
	    4);
	con_paint->data->color_writes_green = u8_array_create_from_raw(
	    (u8[]){
	        true,
	        true,
	        true,
	        true,
	    },
	    4);
	con_paint->data->color_writes_blue = u8_array_create_from_raw(
	    (u8[]){
	        true,
	        true,
	        true,
	        true,
	    },
	    4);
	con_paint->data->color_writes_alpha = u8_array_create_from_raw(
	    (u8[]){
	        true,
	        true,
	        true,
	        true,
	    },
	    4);

	node_shader_t *kong  = node_shader_context_make_kong(con_paint);
	bool           decal = context_is_decal();
	node_shader_add_out(kong, "tex_coord: float2");
	node_shader_write_vert(kong, "var madd: float2 = float2(0.5, 0.5);");
	node_shader_write_vert(kong, "output.tex_coord = input.pos.xy * madd + madd;");
	node_shader_write_vert(kong, "output.tex_coord.y = 1.0 - output.tex_coord.y;");
	node_shader_write_vert(kong, "output.pos = float4(input.pos.xy, 0.0, 1.0);");
	node_shader_write_attrib_frag(kong, "var tex_coord: float2 = input.tex_coord;");
	node_shader_add_constant(kong, "inp: float4", "_input_brush");
	node_shader_add_constant(kong, "inplast: float4", "_input_brush_last");
	node_shader_add_texture(kong, "gbufferD", NULL);
	kong->frag_out = "float4[2]";
	node_shader_add_constant(kong, "brush_radius: float", "_brush_radius");
	node_shader_add_constant(kong, "brush_opacity: float", "_brush_opacity");
	node_shader_add_constant(kong, "brush_hardness: float", "_brush_hardness");

	if (context_raw->tool == TOOL_TYPE_BRUSH || context_raw->tool == TOOL_TYPE_ERASER || context_raw->tool == TOOL_TYPE_CLONE ||
	    context_raw->tool == TOOL_TYPE_BLUR || context_raw->tool == TOOL_TYPE_SMUDGE || context_raw->tool == TOOL_TYPE_PARTICLE || decal) {
		node_shader_write_frag(kong, "var dist: float = 0.0;");
		node_shader_write_frag(kong, "var depth: float = sample_lod(gbufferD, sampler_linear, constants.inp.xy, 0.0).r;");
		node_shader_add_constant(kong, "invVP: float4x4", "_inv_view_proj_matrix");
		// node_shader_write_frag(kong, "var winp: float4 = float4(float2(constants.inp.x, 1.0 - constants.inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);");
		node_shader_write_frag(kong, "var winp: float4 = float4(float2(constants.inp.x, 1.0 - constants.inp.y) * 2.0 - 1.0, depth, 1.0);");
		node_shader_write_frag(kong, "winp = constants.invVP * winp;");
		node_shader_write_frag(kong, "winp.xyz = winp.xyz / winp.w;");
		node_shader_add_constant(kong, "W: float4x4", "_world_matrix");
		// node_shader_add_constant(kong, "texpaint_undo_size: float2", "_size(texpaint_sculpt_undo)"); ////
		// node_shader_write_attrib_frag(kong, "var wposition: float3 = (constants.W * texpaint_sculpt_undo[uint2(uint(tex_coord.x *
		// constants.texpaint_undo_size.x), uint(tex_coord.y * constants.texpaint_undo_size.y))]).xyz;");
		node_shader_write_attrib_frag(
		    kong, "var wposition: float3 = (constants.W * texpaint_sculpt_undo[uint2(uint(tex_coord.x * 2048.0), uint(tex_coord.y * 2048.0))]).xyz;");
		node_shader_write_frag(kong, "var depthlast: float = sample_lod(gbufferD, sampler_linear, constants.inplast.xy, 0.0).r;");
		node_shader_write_frag(kong, "var winplast: float4 = float4(float2(constants.inplast.x, 1.0 - constants.inplast.y) * 2.0 - 1.0, depthlast, 1.0);");
		node_shader_write_frag(kong, "winplast = constants.invVP * winplast;");
		node_shader_write_frag(kong, "winplast.xyz = winplast.xyz / winplast.w;");
		node_shader_write_frag(kong, "dist = distance(wposition.xyz, winp.xyz);");
		node_shader_write_frag(kong, "if (dist > constants.brush_radius) { discard; }");
	}
	else if (context_raw->tool == TOOL_TYPE_FILL) {
		node_shader_write_frag(kong, "var dist: float = 0.0;");

		node_shader_add_constant(kong, "W: float4x4", "_world_matrix");
		node_shader_write_attrib_frag(
		    kong, "var wposition: float3 = (constants.W * texpaint_sculpt_undo[uint2(uint(tex_coord.x * 2048.0), uint(tex_coord.y * 2048.0))]).xyz;");
	}

	parser_material_parse_height            = true;
	parser_material_parse_height_as_channel = true;
	shader_out_t *sout                      = parser_material_parse(context_raw->material->canvas, con_paint, kong, matcon);
	char         *height                    = sout->out_height;
	node_shader_write_frag(kong, string("var height: float = %s;", height));

	if (kong->frag_bposition) {
		kong->frag_bposition = false;
		node_shader_write_attrib_frag(kong, "var bposition: float3 = wposition.xyz;");
	}

	node_shader_write_frag(kong, "var basecol: float3 = float3(1.0, 1.0, 1.0);");
	node_shader_write_frag(kong, "var opacity: float = 1.0;");
	node_shader_write_frag(kong, "if (opacity == 0.0) { discard; }");
	node_shader_write_frag(kong, "var str: float = clamp((constants.brush_radius - dist) * constants.brush_hardness * 1.0, 0.0, 1.0) * opacity;");
	node_shader_add_texture(kong, "texpaint_sculpt_undo", "_texpaint_sculpt_undo");
	node_shader_write_frag(kong, "var sample_undo: float4 = sample_lod(texpaint_sculpt_undo, sampler_linear, tex_coord, 0.0);");
	node_shader_write_frag(kong, "if (sample_undo.r == 0.0 && sample_undo.g == 0.0 && sample_undo.b == 0.0) { discard; }");
	node_shader_add_function(kong, str_octahedron_wrap);
	node_shader_add_texture(kong, "gbuffer0_undo", NULL);
	node_shader_write_frag(kong, "var g0_undo: float2 = sample_lod(gbuffer0_undo, sampler_linear, constants.inp.xy, 0.0).rg;");
	node_shader_write_frag(kong, "var wn: float3;");
	node_shader_write_frag(kong, "wn.z = 1.0 - abs(g0_undo.x) - abs(g0_undo.y);");
	// node_shader_write_frag(kong, "wn.xy = wn.z >= 0.0 ? g0_undo.xy : octahedron_wrap(g0_undo.xy);");
	node_shader_write_frag(kong, "if (wn.z >= 0.0) { wn.xy = g0_undo.xy; } else { wn.xy = octahedron_wrap(g0_undo.xy); }");
	node_shader_write_frag(kong, "var n: float3 = normalize(wn);");
	// node_shader_write_frag(kong, "output[0] = float4(sample_undo.rgb + n * 0.1 * str, 1.0);");
	node_shader_write_frag(kong, "output[0] = float4(sample_undo.rgb + n * (height / 10.0) * str, 1.0);");
	node_shader_write_frag(kong, "output[1] = float4(str, 0.0, 0.0, 1.0);");
	parser_material_finalize(con_paint);
	con_paint->data->shader_from_source = true;
	gpu_create_shaders_from_kong(node_shader_get(kong), &con_paint->data->vertex_shader, &con_paint->data->fragment_shader,
	                             &con_paint->data->_->vertex_shader_size, &con_paint->data->_->fragment_shader_size);
	return con_paint;
}

void sculpt_make_mesh_run(node_shader_t *kong, slot_layer_t *l) {
	node_shader_add_constant(kong, "WVP: float4x4", "_world_view_proj_matrix");
	i32 lid = l->id;
	node_shader_add_texture(kong, "texpaint_sculpt", string("_texpaint_sculpt%s", i32_to_string(lid)));
	node_shader_add_constant(kong, "texpaint_sculpt_size: float2", string("_size(_texpaint_sculpt%s)", i32_to_string(lid)));
	// node_shader_write_vert(kong, "var meshpos: float3 = sample_lod(texpaint_sculpt, sampler_linear, uint2(vertex_id() % constants.texpaint_sculpt_size.x,
	// vertex_id() / constants.texpaint_sculpt_size.y), 0.0).xyz;");
	node_shader_write_vert(kong, "var meshpos: float4 = texpaint_sculpt[uint2(uint(float(vertex_id()) % constants.texpaint_sculpt_size.x), "
	                             "uint(float(vertex_id()) / constants.texpaint_sculpt_size.y))];");
	// + input.pos.xyz * 0.000001
	node_shader_write_vert(kong, "output.pos = constants.WVP * float4(meshpos.xyz, 1.0);");
	kong->frag_n = false;
	node_shader_add_constant(kong, "N: float3x3", "_normal_matrix");
	node_shader_add_out(kong, "wnormal: float3");
	node_shader_write_attrib_vert(kong, "var base_vertex0: float = float(vertex_id()) - (float(vertex_id()) % float(3));");
	node_shader_write_attrib_vert(kong, "var base_vertex1: float = base_vertex0 + 1.0;");
	node_shader_write_attrib_vert(kong, "var base_vertex2: float = base_vertex0 + 2.0;");
	// node_shader_write_attrib_vert(kong, "var meshpos0: float3 = sample_lod(texpaint_sculpt, sampler_linear, uint2(base_vertex0 %
	// constants.texpaint_sculpt_size.x, base_vertex0 / constants.texpaint_sculpt_size.y), 0.0).xyz;"); node_shader_write_attrib_vert(kong, "var meshpos1:
	// float3 = sample_lod(texpaint_sculpt, sampler_linear, uint2(base_vertex1 % constants.texpaint_sculpt_size.x, base_vertex1 /
	// constants.texpaint_sculpt_size.y), 0.0).xyz;"); node_shader_write_attrib_vert(kong, "var meshpos2: float3 = sample_lod(texpaint_sculpt, sampler_linear,
	// uint2(base_vertex2 % constants.texpaint_sculpt_size.x, base_vertex2 / constants.texpaint_sculpt_size.y), 0.0).xyz;");
	node_shader_write_attrib_vert(kong, "var meshpos0: float4 = texpaint_sculpt[uint2(uint(base_vertex0 % constants.texpaint_sculpt_size.x), uint(base_vertex0 "
	                                    "/ constants.texpaint_sculpt_size.y))];");
	node_shader_write_attrib_vert(kong, "var meshpos1: float4 = texpaint_sculpt[uint2(uint(base_vertex1 % constants.texpaint_sculpt_size.x), uint(base_vertex1 "
	                                    "/ constants.texpaint_sculpt_size.y))];");
	node_shader_write_attrib_vert(kong, "var meshpos2: float4 = texpaint_sculpt[uint2(uint(base_vertex2 % constants.texpaint_sculpt_size.x), uint(base_vertex2 "
	                                    "/ constants.texpaint_sculpt_size.y))];");
	node_shader_write_attrib_vert(kong, "var meshnor: float3 = normalize(cross(meshpos2.xyz - meshpos1.xyz, meshpos0.xyz - meshpos1.xyz));");
	node_shader_write_attrib_vert(kong, "output.wnormal = constants.N * meshnor;");
	node_shader_write_attrib_frag(kong, "var n: float3 = normalize(input.wnormal);");
}

void sculpt_layers_create_sculpt_layer() {
	slot_layer_t *l  = layers_new_layer(true, -1);
	i32           id = l->id;

	{
		render_target_t *t = render_target_create();
		t->name            = string("texpaint_sculpt%s", i32_to_string(id));
		t->width           = config_get_texture_res_x();
		t->height          = config_get_texture_res_y();
		t->format          = "RGBA128";
		l->texpaint_sculpt = render_path_create_render_target(t)->_image;
	}

	// util_mesh_merge();
	// let md: mesh_data_t = context_raw.merged_object.data;

	mesh_data_t *md = context_raw->paint_object->data;
	sculpt_import_mesh_pack_to_texture(md, l);

	i16_array_t *posa = i16_array_create(md->index_array->length * 4);
	i16_array_t *nora = i16_array_create(md->index_array->length * 2);
	i16_array_t *texa = i16_array_create(md->index_array->length * 2);
	for (i32 i = 0; i < posa->length; ++i) {
		posa->buffer[i] = 32767;
	}
	for (i32 i = 0; i < md->index_array->length; ++i) {
		i32 index               = md->index_array->buffer[i];
		posa->buffer[i * 4]     = md->vertex_arrays->buffer[0]->values->buffer[index * 4];
		posa->buffer[i * 4 + 1] = md->vertex_arrays->buffer[0]->values->buffer[index * 4 + 1];
		posa->buffer[i * 4 + 2] = md->vertex_arrays->buffer[0]->values->buffer[index * 4 + 2];
		posa->buffer[i * 4 + 3] = md->vertex_arrays->buffer[0]->values->buffer[index * 4 + 3];
		nora->buffer[i * 2]     = md->vertex_arrays->buffer[1]->values->buffer[index * 2];
		nora->buffer[i * 2 + 1] = md->vertex_arrays->buffer[1]->values->buffer[index * 2 + 1];
		texa->buffer[i * 2]     = md->vertex_arrays->buffer[2]->values->buffer[index * 2];
		texa->buffer[i * 2 + 1] = md->vertex_arrays->buffer[2]->values->buffer[index * 2 + 1];
	}
	u32_array_t *inda = u32_array_create(md->index_array->length);
	for (i32 i = 0; i < inda->length; ++i) {
		inda->buffer[i] = i;
	}
	mesh_data_t *raw = GC_ALLOC_INIT(mesh_data_t, {.name          = md->name,
	                                               .vertex_arrays = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = posa, .attrib = "pos", .data = "short4norm"}),
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = nora, .attrib = "nor", .data = "short2norm"}),
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = texa, .attrib = "tex", .data = "short2norm"}),
	                                                   },
	                                                   3),
	                                               .index_array = inda,
	                                               .scale_pos   = 1.0,
	                                               .scale_tex   = 1.0});
	md               = mesh_data_create(raw);
	mesh_object_set_data(context_raw->paint_object, md);

	make_material_parse_paint_material(true);
	make_material_parse_mesh_material();
	{
		render_target_t *t = render_target_create();
		t->name            = "gbuffer0_undo";
		t->width           = 0;
		t->height          = 0;
		t->format          = "RGBA64";
		t->scale           = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}
	{
		render_target_t *t = render_target_create();
		t->name            = "gbufferD_undo";
		t->width           = 0;
		t->height          = 0;
		t->format          = "R32";
		t->scale           = render_path_base_get_super_sampling();
		render_path_create_render_target(t);
	}

	render_path_load_shader("Scene/copy_pass/copyR32_pass");

	for (i32 i = 0; i < history_undo_layers->length; ++i) {
		char         *ext = string("_undo%s", i32_to_string(i));
		slot_layer_t *l   = history_undo_layers->buffer[i];

		{
			render_target_t *t = render_target_create();
			t->name            = string("texpaint_sculpt%s", ext);
			t->width           = config_get_texture_res_x();
			t->height          = config_get_texture_res_y();
			t->format          = "RGBA128";
			l->texpaint_sculpt = render_path_create_render_target(t)->_image;
		}
	}
}

void render_path_sculpt_commands() {
	if (context_raw->pdirty <= 0) {
		return;
	}

	i32   tid             = context_raw->layer->id;
	char *texpaint_sculpt = string("texpaint_sculpt%s", i32_to_string(tid));
	render_path_set_target("texpaint_blend1", NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
	render_path_bind_target("texpaint_blend0", "tex");
	render_path_draw_shader("Scene/copy_pass/copyR8_pass");
	string_t_array_t *additional = any_array_create_from_raw(
	    (void *[]){
	        "texpaint_blend0",
	    },
	    1);
	render_path_set_target(texpaint_sculpt, additional, NULL, GPU_CLEAR_NONE, 0, 0.0);
	render_path_bind_target("gbufferD_undo", "gbufferD");
	if (context_raw->xray || config_raw->brush_angle_reject) {
		render_path_bind_target("gbuffer0", "gbuffer0");
	}
	render_path_bind_target("texpaint_blend1", "paintmask");
	render_path_bind_target("gbuffer0_undo", "gbuffer0_undo");

	material_context_t *material_context = NULL;
	shader_context_t   *shader_context   = NULL;
	material_data_t    *mat              = project_paint_objects->buffer[0]->material;
	for (i32 j = 0; j < mat->contexts->length; ++j) {
		if (string_equals(mat->contexts->buffer[j]->name, "paint")) {
			shader_context   = shader_data_get_context(mat->_->shader, "paint");
			material_context = mat->contexts->buffer[j];
			break;
		}
	}

	gpu_set_pipeline(shader_context->_->pipe);
	uniforms_set_context_consts(shader_context, _render_path_bind_params);
	uniforms_set_obj_consts(shader_context, project_paint_objects->buffer[0]->base);
	uniforms_set_material_consts(shader_context, material_context);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();
	render_path_end();
}

void render_path_sculpt_begin() {
	if (!render_path_paint_paint_enabled()) {
		return;
	}
	render_path_paint_push_undo_last = history_push_undo;
	if (history_push_undo && history_undo_layers != NULL) {
		history_paint();
		render_path_set_target("gbuffer0_undo", NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
		render_path_bind_target("gbuffer0", "tex");
		render_path_draw_shader("Scene/copy_pass/copyRGBA64_pass");
		render_path_set_target("gbufferD_undo", NULL, NULL, GPU_CLEAR_NONE, 0, 0.0);
		render_path_bind_target("main", "tex");
		render_path_draw_shader("Scene/copy_pass/copyR32_pass");
	}
	if (sculpt_push_undo && history_undo_layers != NULL) {
		history_paint();
	}
}
