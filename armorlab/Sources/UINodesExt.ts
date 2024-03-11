
class UINodesExt {

	static lastVertices: DataView = null; // Before displacement

	static drawButtons = (ew: f32, startY: f32) => {
		let ui = ui_nodes_ui;
		if (zui_button(tr("Run"))) {
			console_progress(tr("Processing"));

			let delayIdleSleep = () => {
				krom_delay_idle_sleep();
			}
			app_notify_on_render_2d(delayIdleSleep);

			let tasks = 1;

			let taskDone = () => {
				tasks--;
				if (tasks == 0) {
					console_progress(null);
					context_raw.ddirty = 2;
					app_remove_render_2d(delayIdleSleep);

					///if (krom_direct3d12 || krom_vulkan || krom_metal)
					RenderPathRaytrace.ready = false;
					///end
				}
			}

			base_notifyOnNextFrame(() => {
				let timer = time_time();
				ParserLogic.parse(project_canvas);

				PhotoToPBRNode.cachedSource = null;
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelBaseColor, (texbase: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelOcclusion, (texocc: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelRoughness, (texrough: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelNormalMap, (texnor: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelHeight, (texheight: image_t) => {

					if (texbase != null) {
						let texpaint = render_path_render_targets.get("texpaint")._image;
						g2_begin(texpaint);
						g2_draw_scaled_image(texbase, 0, 0, config_getTextureResX(), config_getTextureResY());
						g2_end();
					}

					if (texnor != null) {
						let texpaint_nor = render_path_render_targets.get("texpaint_nor")._image;
						g2_begin(texpaint_nor);
						g2_draw_scaled_image(texnor, 0, 0, config_getTextureResX(), config_getTextureResY());
						g2_end();
					}

					if (base_pipeCopy == null) base_makePipe();
					if (base_pipeCopyA == null) base_makePipeCopyA();
					if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();

					let texpaint_pack = render_path_render_targets.get("texpaint_pack")._image;

					if (texocc != null) {
						g2_begin(texpaint_pack);
						g2_set_pipeline(base_pipeCopyR);
						g2_draw_scaled_image(texocc, 0, 0, config_getTextureResX(), config_getTextureResY());
						g2_set_pipeline(null);
						g2_end();
					}

					if (texrough != null) {
						g2_begin(texpaint_pack);
						g2_set_pipeline(base_pipeCopyG);
						g2_draw_scaled_image(texrough, 0, 0, config_getTextureResX(), config_getTextureResY());
						g2_set_pipeline(null);
						g2_end();
					}

					if (texheight != null) {
						g4_begin(texpaint_pack);
						g4_set_pipeline(base_pipeCopyA);
						g4_set_tex(base_pipeCopyATex, texheight);
						g4_set_vertex_buffer(const_data_screen_aligned_vb);
						g4_set_index_buffer(const_data_screen_aligned_ib);
						g4_draw();
						g4_end();

						if (ui_header_worktab.position == SpaceType.Space3D &&
							BrushOutputNode.inst.inputs[ChannelType.ChannelHeight].node.constructor != FloatNode) {

							// Make copy of vertices before displacement
							let o = project_paintObjects[0];
							let g = o.data;
							let vertices = g4_vertex_buffer_lock(g._.vertex_buffer);
							if (UINodesExt.lastVertices == null || UINodesExt.lastVertices.byteLength != vertices.byteLength) {
								UINodesExt.lastVertices = new DataView(new ArrayBuffer(vertices.byteLength));
								for (let i = 0; i < math_floor(vertices.byteLength / 2); ++i) {
									UINodesExt.lastVertices.setInt16(i * 2, vertices.getInt16(i * 2, true), true);
								}
							}
							else {
								for (let i = 0; i < math_floor(vertices.byteLength / 2); ++i) {
									vertices.setInt16(i * 2, UINodesExt.lastVertices.getInt16(i * 2, true), true);
								}
							}
							g4_vertex_buffer_unlock(g._.vertex_buffer);

							// Apply displacement
							if (config_raw.displace_strength > 0) {
								tasks++;
								base_notifyOnNextFrame(() => {
									console_progress(tr("Apply Displacement"));
									base_notifyOnNextFrame(() => {
										let uv_scale = scene_meshes[0].data.scale_tex * context_raw.brushScale;
										util_mesh_applyDisplacement(texpaint_pack, 0.05 * config_raw.displace_strength, uv_scale);
										util_mesh_calcNormals();
										taskDone();
									});
								});
							}
						}
					}

					console_log("Processing finished in " + (time_time() - timer));
					krom_ml_unload();

					taskDone();
				});
				});
				});
				});
				});
			});
		}
		ui._x += ew + 3;
		ui._y = 2 + startY;

		///if (krom_android || krom_ios)
		zui_combo(base_resHandle, ["2K", "4K"], tr("Resolution"));
		///else
		zui_combo(base_resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
		///end
		if (base_resHandle.changed) {
			base_onLayersResized();
		}
		ui._x += ew + 3;
		ui._y = 2 + startY;
	}
}
