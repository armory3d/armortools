
let ui_nodes_ext_last_vertices: buffer_view_t = null; // Before displacement

function ui_nodes_ext_draw_buttons(ew: f32, start_y: f32) {
	let ui = ui_nodes_ui;
	if (zui_button(tr("Run"))) {
		console_progress(tr("Processing"));

		let delay_idle_sleep = function () {
			krom_delay_idle_sleep();
		}
		app_notify_on_render_2d(delay_idle_sleep);

		let tasks: i32 = 1;

		let task_done = function () {
			tasks--;
			if (tasks == 0) {
				console_progress(null);
				context_raw.ddirty = 2;
				app_remove_render_2d(delay_idle_sleep);

				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				render_path_raytrace_ready = false;
				///end
			}
		}

		base_notify_on_next_frame(function () {
			let timer = time_time();
			parser_logic_parse(project_canvas);

			photo_to_pbr_node_cached_source = null;
			brush_output_node_inst.get_as_image(channel_type_t.BASE_COLOR, function (texbase: image_t) {
			brush_output_node_inst.get_as_image(channel_type_t.OCCLUSION, function (texocc: image_t) {
			brush_output_node_inst.get_as_image(channel_type_t.ROUGHNESS, function (texrough: image_t) {
			brush_output_node_inst.get_as_image(channel_type_t.NORMAL_MAP, function (texnor: image_t) {
			brush_output_node_inst.get_as_image(channel_type_t.HEIGHT, function (texheight: image_t) {

				if (texbase != null) {
					let texpaint = map_get(render_path_render_targets, "texpaint")._image;
					g2_begin(texpaint);
					g2_draw_scaled_image(texbase, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
					g2_end();
				}

				if (texnor != null) {
					let texpaint_nor = map_get(render_path_render_targets, "texpaint_nor")._image;
					g2_begin(texpaint_nor);
					g2_draw_scaled_image(texnor, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
					g2_end();
				}

				if (base_pipe_copy == null) {
					base_make_pipe();
				}
				if (base_pipe_copy_a == null) {
					base_make_pipe_copy_a();
				}
				if (const_data_screen_aligned_vb == null) {
					const_data_create_screen_aligned_data();
				}

				let texpaint_pack = map_get(render_path_render_targets, "texpaint_pack")._image;

				if (texocc != null) {
					g2_begin(texpaint_pack);
					g2_set_pipeline(base_pipe_copy_r);
					g2_draw_scaled_image(texocc, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
					g2_set_pipeline(null);
					g2_end();
				}

				if (texrough != null) {
					g2_begin(texpaint_pack);
					g2_set_pipeline(base_pipe_copy_g);
					g2_draw_scaled_image(texrough, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
					g2_set_pipeline(null);
					g2_end();
				}

				if (texheight != null) {
					g4_begin(texpaint_pack);
					g4_set_pipeline(base_pipe_copy_a);
					g4_set_tex(base_pipe_copy_a_tex, texheight);
					g4_set_vertex_buffer(const_data_screen_aligned_vb);
					g4_set_index_buffer(const_data_screen_aligned_ib);
					g4_draw();
					g4_end();

					if (ui_header_worktab.position == space_type_t.SPACE3D &&
						BrushOutputNode.inst.inputs[channel_type_t.HEIGHT].node.constructor != FloatNode) {

						// Make copy of vertices before displacement
						let o = project_paint_objects[0];
						let g = o.data;
						let vertices = g4_vertex_buffer_lock(g._.vertex_buffer);
						if (ui_nodes_ext_last_vertices == null || buffer_view_size(ui_nodes_ext_last_vertices) != buffer_view_size(vertices)) {
							ui_nodes_ext_last_vertices = buffer_view_create(buffer_create(buffer_view_size(vertices)));
							for (let i: i32 = 0; i < math_floor(buffer_view_size(vertices) / 2); ++i) {
								buffer_view_set_i16(ui_nodes_ext_last_vertices, i * 2, buffer_view_get_i16(vertices, i * 2));
							}
						}
						else {
							for (let i: i32 = 0; i < math_floor(buffer_view_size(vertices) / 2); ++i) {
								buffer_view_set_i16(vertices, i * 2, buffer_view_get_i16(ui_nodes_ext_last_vertices, i * 2));
							}
						}
						g4_vertex_buffer_unlock(g._.vertex_buffer);

						// Apply displacement
						if (config_raw.displace_strength > 0) {
							tasks++;
							base_notify_on_next_frame(function () {
								console_progress(tr("Apply Displacement"));
								base_notify_on_next_frame(function () {
									let uv_scale = scene_meshes[0].data.scale_tex * context_raw.brush_scale;
									util_mesh_apply_displacement(texpaint_pack, 0.05 * config_raw.displace_strength, uv_scale);
									util_mesh_calc_normals();
									task_done();
								});
							});
						}
					}
				}

				console_log("Processing finished in " + (time_time() - timer));
				krom_ml_unload();

				task_done();
			});
			});
			});
			});
			});
		});
	}
	ui._x += ew + 3;
	ui._y = 2 + start_y;

	///if (krom_android || krom_ios)
	zui_combo(base_res_handle, ["2K", "4K"], tr("Resolution"));
	///else
	zui_combo(base_res_handle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
	///end
	if (base_res_handle.changed) {
		base_on_layers_resized();
	}
	ui._x += ew + 3;
	ui._y = 2 + start_y;
}
