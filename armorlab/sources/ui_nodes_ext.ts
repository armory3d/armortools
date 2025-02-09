
let ui_nodes_ext_last_vertices: buffer_t = null; // Before displacement

function ui_nodes_ext_delay_idle_sleep() {
	iron_delay_idle_sleep();
}

function ui_nodes_ext_draw_buttons(ew: f32, start_y: f32) {
	let ui: ui_t = ui_nodes_ui;
	if (ui_button(tr("Run"))) {
		// app_notify_on_init(function() {
			ui_nodes_ext_run();
		// });
	}
	ui._x += ew + 3;
	ui._y = 2 + start_y;

	///if (arm_android || arm_ios)
	let base_res_combo: string[] = ["128", "256", "512", "1K", "2K", "4K"];
	ui_combo(base_res_handle, base_res_combo, tr("Resolution"));
	///else
	let base_res_combo: string[] = ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"];
	ui_combo(base_res_handle, base_res_combo, tr("Resolution"));
	///end

	if (base_res_handle.changed) {
		layers_on_resized();
	}
	ui._x += ew + 3;
	ui._y = 2 + start_y;
}

function ui_nodes_ext_run() {
	app_notify_on_render_2d(ui_nodes_ext_delay_idle_sleep);

	console_progress(tr("Processing"));

	let timer: f32 = time_time();
	parser_logic_parse(project_canvas);

	photo_to_pbr_node_cached_source = null;
	let texbase: image_t = logic_node_get_as_image(context_raw.brush_output_node_inst.base, channel_type_t.BASE_COLOR);
	let texocc: image_t = logic_node_get_as_image(context_raw.brush_output_node_inst.base, channel_type_t.OCCLUSION);
	let texrough: image_t = logic_node_get_as_image(context_raw.brush_output_node_inst.base, channel_type_t.ROUGHNESS);
	let texnor: image_t = logic_node_get_as_image(context_raw.brush_output_node_inst.base, channel_type_t.NORMAL_MAP);
	let texheight: image_t = logic_node_get_as_image(context_raw.brush_output_node_inst.base, channel_type_t.HEIGHT);

	if (texbase != null) {
		let texpaint: render_target_t = map_get(render_path_render_targets, "texpaint");
		g2_begin(texpaint._image);
		g2_draw_scaled_image(texbase, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
		g2_end();
	}

	if (texnor != null) {
		let texpaint_nor: render_target_t = map_get(render_path_render_targets, "texpaint_nor");
		g2_begin(texpaint_nor._image);
		g2_draw_scaled_image(texnor, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
		g2_end();
	}

	let texpaint_pack: render_target_t = map_get(render_path_render_targets, "texpaint_pack");

	if (texocc != null) {
		g2_begin(texpaint_pack._image);
		g2_set_pipeline(pipes_copy_r);
		g2_draw_scaled_image(texocc, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
		g2_set_pipeline(null);
		g2_end();
	}

	if (texrough != null) {
		g2_begin(texpaint_pack._image);
		g2_set_pipeline(pipes_copy_g);
		g2_draw_scaled_image(texrough, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
		g2_set_pipeline(null);
		g2_end();
	}

	if (texheight != null) {
		g4_begin(texpaint_pack._image);
		g4_set_pipeline(pipes_copy_a);
		g4_set_tex(pipes_copy_a_tex, texheight);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();

		let is_float_node: bool = context_raw.brush_output_node_inst.base.inputs[channel_type_t.HEIGHT].node.base.get == float_node_get;

		if (ui_header_worktab.position == space_type_t.SPACE3D && !is_float_node) {

			// Make copy of vertices before displacement
			let o: mesh_object_t = project_paint_objects[0];
			let g: mesh_data_t = o.data;
			let vertices: buffer_t = g4_vertex_buffer_lock(g._.vertex_buffer);
			if (ui_nodes_ext_last_vertices == null || ui_nodes_ext_last_vertices.length != vertices.length) {
				ui_nodes_ext_last_vertices = buffer_create(vertices.length);
				for (let i: i32 = 0; i < math_floor((vertices.length) / 2); ++i) {
					buffer_set_i16(ui_nodes_ext_last_vertices, i * 2, buffer_get_i16(vertices, i * 2));
				}
			}
			else {
				for (let i: i32 = 0; i < math_floor((vertices.length) / 2); ++i) {
					buffer_set_i16(vertices, i * 2, buffer_get_i16(ui_nodes_ext_last_vertices, i * 2));
				}
			}
			g4_vertex_buffer_unlock(g._.vertex_buffer);

			// Apply displacement
			if (config_raw.displace_strength > 0) {
				console_progress(tr("Apply Displacement"));

				let uv_scale: f32 = scene_meshes[0].data.scale_tex * context_raw.brush_scale;
				util_mesh_apply_displacement(texpaint_pack._image, 0.05 * config_raw.displace_strength, uv_scale);
				util_mesh_calc_normals();
			}
		}
	}

	let t: f32 = time_time() - timer;
	console_log("Processing finished in " + t);
	iron_ml_unload();

	console_progress(null);
	context_raw.ddirty = 2;
	app_remove_render_2d(ui_nodes_ext_delay_idle_sleep);

	render_path_raytrace_ready = false;
}
