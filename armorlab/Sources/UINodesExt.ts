
class UINodesExt {

	static lastVertices: DataView = null; // Before displacement

	static drawButtons = (ew: f32, startY: f32) => {
		let ui = UINodes.ui;
		if (zui_button(tr("Run"))) {
			Console.progress(tr("Processing"));

			let delayIdleSleep = () => {
				krom_delay_idle_sleep();
			}
			app_notify_on_render_2d(delayIdleSleep);

			let tasks = 1;

			let taskDone = () => {
				tasks--;
				if (tasks == 0) {
					Console.progress(null);
					Context.raw.ddirty = 2;
					app_remove_render_2d(delayIdleSleep);

					///if (krom_direct3d12 || krom_vulkan || krom_metal)
					RenderPathRaytrace.ready = false;
					///end
				}
			}

			Base.notifyOnNextFrame(() => {
				let timer = time_time();
				ParserLogic.parse(Project.canvas);

				PhotoToPBRNode.cachedSource = null;
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelBaseColor, (texbase: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelOcclusion, (texocc: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelRoughness, (texrough: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelNormalMap, (texnor: image_t) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelHeight, (texheight: image_t) => {

					if (texbase != null) {
						let texpaint = render_path_render_targets.get("texpaint").image;
						g2_begin(texpaint, false);
						g2_draw_scaled_image(texbase, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						g2_end();
					}

					if (texnor != null) {
						let texpaint_nor = render_path_render_targets.get("texpaint_nor").image;
						g2_begin(texpaint_nor, false);
						g2_draw_scaled_image(texnor, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						g2_end();
					}

					if (Base.pipeCopy == null) Base.makePipe();
					if (Base.pipeCopyA == null) Base.makePipeCopyA();
					if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();

					let texpaint_pack = render_path_render_targets.get("texpaint_pack").image;

					if (texocc != null) {
						g2_begin(texpaint_pack, false);
						g2_set_pipeline(Base.pipeCopyR);
						g2_draw_scaled_image(texocc, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						g2_set_pipeline(null);
						g2_end();
					}

					if (texrough != null) {
						g2_begin(texpaint_pack, false);
						g2_set_pipeline(Base.pipeCopyG);
						g2_draw_scaled_image(texrough, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						g2_set_pipeline(null);
						g2_end();
					}

					if (texheight != null) {
						g4_begin(texpaint_pack);
						g4_set_pipeline(Base.pipeCopyA);
						g4_set_tex(Base.pipeCopyATex, texheight);
						g4_set_vertex_buffer(const_data_screen_aligned_vb);
						g4_set_index_buffer(const_data_screen_aligned_ib);
						g4_draw();
						g4_end();

						if (UIHeader.worktab.position == SpaceType.Space3D &&
							BrushOutputNode.inst.inputs[ChannelType.ChannelHeight].node.constructor != FloatNode) {

							// Make copy of vertices before displacement
							let o = Project.paintObjects[0];
							let g = o.data;
							let vertices = g4_vertex_buffer_lock(g._vertex_buffer);
							if (UINodesExt.lastVertices == null || UINodesExt.lastVertices.byteLength != vertices.byteLength) {
								UINodesExt.lastVertices = new DataView(new ArrayBuffer(vertices.byteLength));
								for (let i = 0; i < Math.floor(vertices.byteLength / 2); ++i) {
									UINodesExt.lastVertices.setInt16(i * 2, vertices.getInt16(i * 2, true), true);
								}
							}
							else {
								for (let i = 0; i < Math.floor(vertices.byteLength / 2); ++i) {
									vertices.setInt16(i * 2, UINodesExt.lastVertices.getInt16(i * 2, true), true);
								}
							}
							g4_vertex_buffer_unlock(g._vertex_buffer);

							// Apply displacement
							if (Config.raw.displace_strength > 0) {
								tasks++;
								Base.notifyOnNextFrame(() => {
									Console.progress(tr("Apply Displacement"));
									Base.notifyOnNextFrame(() => {
										let uv_scale = scene_meshes[0].data.scale_tex * Context.raw.brushScale;
										UtilMesh.applyDisplacement(texpaint_pack, 0.05 * Config.raw.displace_strength, uv_scale);
										UtilMesh.calcNormals();
										taskDone();
									});
								});
							}
						}
					}

					Console.log("Processing finished in " + (time_time() - timer));
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
		zui_combo(Base.resHandle, ["2K", "4K"], tr("Resolution"));
		///else
		zui_combo(Base.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
		///end
		if (Base.resHandle.changed) {
			Base.onLayersResized();
		}
		ui._x += ew + 3;
		ui._y = 2 + startY;
	}
}
