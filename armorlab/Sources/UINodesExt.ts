
class UINodesExt {

	static lastVertices: DataView = null; // Before displacement

	static drawButtons = (ew: f32, startY: f32) => {
		let ui = UINodes.ui;
		if (Zui.button(tr("Run"))) {
			Console.progress(tr("Processing"));

			let delayIdleSleep = (_: any) => {
				Krom.delayIdleSleep();
			}
			App.notifyOnRender2D(delayIdleSleep);

			let tasks = 1;

			let taskDone = () => {
				tasks--;
				if (tasks == 0) {
					Console.progress(null);
					Context.raw.ddirty = 2;
					App.removeRender2D(delayIdleSleep);

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
						g2_begin(texpaint.g2, false);
						g2_draw_scaled_image(texbase, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						g2_end(texpaint.g2);
					}

					if (texnor != null) {
						let texpaint_nor = render_path_render_targets.get("texpaint_nor").image;
						g2_begin(texpaint_nor.g2, false);
						g2_draw_scaled_image(texnor, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						g2_end(texpaint_nor.g2);
					}

					if (Base.pipeCopy == null) Base.makePipe();
					if (Base.pipeCopyA == null) Base.makePipeCopyA();
					if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();

					let texpaint_pack = render_path_render_targets.get("texpaint_pack").image;

					if (texocc != null) {
						g2_begin(texpaint_pack.g2, false);
						texpaint_pack.g2.pipeline = Base.pipeCopyR;
						g2_draw_scaled_image(texocc, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						g2_end(texpaint_pack.g2);
					}

					if (texrough != null) {
						g2_begin(texpaint_pack.g2, false);
						texpaint_pack.g2.pipeline = Base.pipeCopyG;
						g2_draw_scaled_image(texrough, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						g2_end(texpaint_pack.g2);
					}

					if (texheight != null) {
						g4_begin(texpaint_pack.g4);
						g4_set_pipeline(Base.pipeCopyA);
						g4_set_tex(Base.pipeCopyATex, texheight);
						g4_set_vertex_buffer(ConstData.screenAlignedVB);
						g4_set_index_buffer(ConstData.screenAlignedIB);
						g4_draw();
						g4_end();

						if (UIHeader.worktab.position == SpaceType.Space3D &&
							BrushOutputNode.inst.inputs[ChannelType.ChannelHeight].node.constructor != FloatNode) {

							// Make copy of vertices before displacement
							let o = Project.paintObjects[0];
							let g = o.data;
							let vertices = vertex_buffer_lock(g._vertex_buffer);
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
							vertex_buffer_unlock(g._vertex_buffer);

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
					Krom.mlUnload();

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
		Zui.combo(Base.resHandle, ["2K", "4K"], tr("Resolution"));
		///else
		Zui.combo(Base.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
		///end
		if (Base.resHandle.changed) {
			Base.onLayersResized();
		}
		ui._x += ew + 3;
		ui._y = 2 + startY;
	}
}
