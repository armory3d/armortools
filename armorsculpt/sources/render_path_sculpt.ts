
function render_path_sculpt_commands() {
    let tid: i32 = context_raw.layer.id;
    let texpaint: string = "texpaint" + tid;
    render_path_set_target("texpaint_blend1");
    render_path_bind_target("texpaint_blend0", "tex");
    render_path_draw_shader("shader_datas/copy_pass/copyR8_pass");
    let additional: string[] = ["texpaint_blend0"];
    render_path_set_target(texpaint, additional);
    render_path_bind_target("gbufferD_undo", "gbufferD");
    if ((context_raw.xray || config_raw.brush_angle_reject) && config_raw.brush_3d) {
        render_path_bind_target("gbuffer0", "gbuffer0");
    }
    render_path_bind_target("texpaint_blend1", "paintmask");

    // Read texcoords from gbuffer
    let read_tc: bool = (context_raw.tool == workspace_tool_t.FILL && context_raw.fill_type_handle.position == fill_type_t.FACE) ||
                            context_raw.tool == workspace_tool_t.CLONE ||
                            context_raw.tool == workspace_tool_t.BLUR ||
                            context_raw.tool == workspace_tool_t.SMUDGE;
    if (read_tc) {
        render_path_bind_target("gbuffer2", "gbuffer2");
    }
    render_path_bind_target("gbuffer0_undo", "gbuffer0_undo");

    let material_contexts: material_context_t[] = [];
    let shader_contexts: shader_context_t[] = [];
    let mats: material_data_t[] = project_paint_objects[0].materials;
    mesh_object_get_contexts(project_paint_objects[0], "paint", mats, material_contexts, shader_contexts);

    let cc_context: shader_context_t = shader_contexts[0];
    g4_set_pipeline(cc_context._.pipe_state);
    uniforms_set_context_consts(cc_context,_render_path_bind_params);
    uniforms_set_obj_consts(cc_context, project_paint_objects[0].base);
    uniforms_set_material_consts(cc_context, material_contexts[0]);
    g4_set_vertex_buffer(const_data_screen_aligned_vb);
    g4_set_index_buffer(const_data_screen_aligned_ib);
    g4_draw();
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
        render_path_draw_shader("shader_datas/copy_pass/copy_pass");

        render_path_set_target("gbufferD_undo");
        render_path_bind_target("_main", "tex");
        render_path_draw_shader("shader_datas/copy_pass/copy_pass");
    }

    if (history_push_undo2 && history_undo_layers != null) {
        history_paint();
    }

    if (context_raw.paint2d) {
        render_path_paint_set_plane_mesh();
    }

    if (render_path_paint_live_layer_drawn > 0) {
        render_path_paint_live_layer_drawn--;
    }

    if (config_raw.brush_live && context_raw.pdirty <= 0 && context_raw.ddirty <= 0 && context_raw.brush_time == 0) {
        // Depth is unchanged, draw before gbuffer gets updated
        render_path_paint_commands_live_brush();
    }
}

function render_path_sculpt_bind_layers() {

	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		render_path_bind_target("texpaint" + l.id, "texpaint" + l.id);
	}
}
