
class UINodesExt {

	static lastVertices: DataView = null; // Before displacement

	static drawButtons = (ew: f32, startY: f32) => {
		let ui = UINodes.ui;
		if (ui.button(tr("Run"))) {
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
				let timer = Time.time();
				ParserLogic.parse(Project.canvas);

				PhotoToPBRNode.cachedSource = null;
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelBaseColor, (texbase: ImageRaw) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelOcclusion, (texocc: ImageRaw) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelRoughness, (texrough: ImageRaw) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelNormalMap, (texnor: ImageRaw) => {
				BrushOutputNode.inst.getAsImage(ChannelType.ChannelHeight, (texheight: ImageRaw) => {

					if (texbase != null) {
						let texpaint = RenderPath.renderTargets.get("texpaint").image;
						Graphics2.begin(texpaint.g2, false);
						Graphics2.drawScaledImage(texbase, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						Graphics2.end(texpaint.g2);
					}

					if (texnor != null) {
						let texpaint_nor = RenderPath.renderTargets.get("texpaint_nor").image;
						Graphics2.begin(texpaint_nor.g2, false);
						Graphics2.drawScaledImage(texnor, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						Graphics2.end(texpaint_nor.g2);
					}

					if (Base.pipeCopy == null) Base.makePipe();
					if (Base.pipeCopyA == null) Base.makePipeCopyA();
					if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();

					let texpaint_pack = RenderPath.renderTargets.get("texpaint_pack").image;

					if (texocc != null) {
						Graphics2.begin(texpaint_pack.g2, false);
						texpaint_pack.g2.pipeline = Base.pipeCopyR;
						Graphics2.drawScaledImage(texocc, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						Graphics2.end(texpaint_pack.g2);
					}

					if (texrough != null) {
						Graphics2.begin(texpaint_pack.g2, false);
						texpaint_pack.g2.pipeline = Base.pipeCopyG;
						Graphics2.drawScaledImage(texrough, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						Graphics2.end(texpaint_pack.g2);
					}

					if (texheight != null) {
						Graphics4.begin(texpaint_pack.g4);
						Graphics4.setPipeline(Base.pipeCopyA);
						Graphics4.setTexture(Base.pipeCopyATex, texheight);
						Graphics4.setVertexBuffer(ConstData.screenAlignedVB);
						Graphics4.setIndexBuffer(ConstData.screenAlignedIB);
						Graphics4.drawIndexedVertices();
						Graphics4.end();

						if (UIHeader.worktab.position == SpaceType.Space3D &&
							BrushOutputNode.inst.inputs[ChannelType.ChannelHeight].node.constructor != FloatNode) {

							// Make copy of vertices before displacement
							let o = Project.paintObjects[0];
							let g = o.data;
							let vertices = VertexBuffer.lock(g._vertexBuffer);
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
							VertexBuffer.unlock(g._vertexBuffer);

							// Apply displacement
							if (Config.raw.displace_strength > 0) {
								tasks++;
								Base.notifyOnNextFrame(() => {
									Console.progress(tr("Apply Displacement"));
									Base.notifyOnNextFrame(() => {
										let uv_scale = Scene.meshes[0].data.scale_tex * Context.raw.brushScale;
										UtilMesh.applyDisplacement(texpaint_pack, 0.05 * Config.raw.displace_strength, uv_scale);
										UtilMesh.calcNormals();
										taskDone();
									});
								});
							}
						}
					}

					Console.log("Processing finished in " + (Time.time() - timer));
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
		ui.combo(Base.resHandle, ["2K", "4K"], tr("Resolution"));
		///else
		ui.combo(Base.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
		///end
		if (Base.resHandle.changed) {
			Base.onLayersResized();
		}
		ui._x += ew + 3;
		ui._y = 2 + startY;
	}
}
