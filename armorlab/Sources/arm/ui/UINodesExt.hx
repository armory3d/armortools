package arm.ui;

import kha.arrays.ByteArray;

@:access(zui.Zui)
class UINodesExt {

	static var lastVertices: ByteArray = null; // Before displacement

	public static function drawButtons(ew: Float, startY: Float) {
		var ui = UINodes.inst.ui;
		if (ui.button(tr("Run"))) {
			Console.progress(tr("Processing"));

			function delayIdleSleep(_) {
				Krom.delayIdleSleep();
			}
			iron.App.notifyOnRender2D(delayIdleSleep);

			var tasks = 1;

			function taskDone() {
				tasks--;
				if (tasks == 0) {
					Console.progress(null);
					Context.raw.ddirty = 2;
					iron.App.removeRender2D(delayIdleSleep);

					#if (kha_direct3d12 || kha_vulkan || kha_metal)
					arm.render.RenderPathRaytrace.ready = false;
					#end
				}
			}

			App.notifyOnNextFrame(function() {
				var timer = iron.system.Time.realTime();
				arm.logic.LogicParser.parse(Project.canvas, false);

				arm.logic.PhotoToPBRNode.cachedSource = null;
				@:privateAccess arm.logic.BrushOutputNode.inst.getAsImage(ChannelBaseColor, function(texbase: kha.Image) {
				@:privateAccess arm.logic.BrushOutputNode.inst.getAsImage(ChannelOcclusion, function(texocc: kha.Image) {
				@:privateAccess arm.logic.BrushOutputNode.inst.getAsImage(ChannelRoughness, function(texrough: kha.Image) {
				@:privateAccess arm.logic.BrushOutputNode.inst.getAsImage(ChannelNormalMap, function(texnor: kha.Image) {
				@:privateAccess arm.logic.BrushOutputNode.inst.getAsImage(ChannelHeight, function(texheight: kha.Image) {

					if (texbase != null) {
						var texpaint = iron.RenderPath.active.renderTargets.get("texpaint").image;
						texpaint.g2.begin(false);
						texpaint.g2.drawScaledImage(texbase, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint.g2.end();
					}

					if (texnor != null) {
						var texpaint_nor = iron.RenderPath.active.renderTargets.get("texpaint_nor").image;
						texpaint_nor.g2.begin(false);
						texpaint_nor.g2.drawScaledImage(texnor, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_nor.g2.end();
					}

					if (App.pipeCopy == null) App.makePipe();
					if (App.pipeCopyA == null) App.makePipeCopyA();
					if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

					var texpaint_pack = iron.RenderPath.active.renderTargets.get("texpaint_pack").image;

					if (texocc != null) {
						texpaint_pack.g2.begin(false);
						texpaint_pack.g2.pipeline = App.pipeCopyR;
						texpaint_pack.g2.drawScaledImage(texocc, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						texpaint_pack.g2.end();
					}

					if (texrough != null) {
						texpaint_pack.g2.begin(false);
						texpaint_pack.g2.pipeline = App.pipeCopyG;
						texpaint_pack.g2.drawScaledImage(texrough, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						texpaint_pack.g2.end();
					}

					if (texheight != null) {
						texpaint_pack.g4.begin();
						texpaint_pack.g4.setPipeline(App.pipeCopyA);
						texpaint_pack.g4.setTexture(App.pipeCopyATex, texheight);
						texpaint_pack.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
						texpaint_pack.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
						texpaint_pack.g4.drawIndexedVertices();
						texpaint_pack.g4.end();

						// Make copy of vertices before displacement
						var o = Project.paintObjects[0];
						var g = o.data.geom;
						var vertices = g.vertexBuffer.lock();
						if (lastVertices == null || lastVertices.byteLength != vertices.byteLength) {
							lastVertices = ByteArray.make(vertices.byteLength);
							for (i in 0...Std.int(vertices.byteLength / 2)) {
								lastVertices.setInt16(i * 2, vertices.getInt16(i * 2));
							}
						}
						else {
							for (i in 0...Std.int(vertices.byteLength / 2)) {
								vertices.setInt16(i * 2, lastVertices.getInt16(i * 2));
							}
						}
						g.vertexBuffer.unlock();

						// Apply displacement
						if (Config.raw.displace_strength > 0) {
							tasks++;
							arm.App.notifyOnNextFrame(function() {
								Console.progress(tr("Apply Displacement"));
								arm.App.notifyOnNextFrame(function() {
									arm.util.MeshUtil.applyDisplacement(texpaint_pack, 0.05 * Config.raw.displace_strength, Context.raw.brushScale);
									arm.util.MeshUtil.calcNormals();
									taskDone();
								});
							});
						}
					}

					Console.log("Processing finished in " + (iron.system.Time.realTime() - timer));
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

		#if (krom_android || krom_ios)
		ui.combo(App.resHandle, ["2K", "4K"], tr("Resolution"));
		#else
		ui.combo(App.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
		#end
		if (App.resHandle.changed) {
			App.onLayersResized();
		}
		ui._x += ew + 3;
		ui._y = 2 + startY;
	}
}
