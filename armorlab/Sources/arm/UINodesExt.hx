package arm;

import js.lib.DataView;
import js.lib.ArrayBuffer;
import iron.App;
import iron.System;
import iron.Time;
import iron.Scene;
import iron.RenderPath;
import iron.ConstData;

class UINodesExt {

	static var lastVertices: DataView = null; // Before displacement

	public static function drawButtons(ew: Float, startY: Float) {
		var ui = UINodes.inst.ui;
		if (ui.button(tr("Run"))) {
			Console.progress(tr("Processing"));

			function delayIdleSleep(_) {
				Krom.delayIdleSleep();
			}
			App.notifyOnRender2D(delayIdleSleep);

			var tasks = 1;

			function taskDone() {
				tasks--;
				if (tasks == 0) {
					Console.progress(null);
					Context.raw.ddirty = 2;
					App.removeRender2D(delayIdleSleep);

					#if (krom_direct3d12 || krom_vulkan || krom_metal)
					RenderPathRaytrace.ready = false;
					#end
				}
			}

			Base.notifyOnNextFrame(function() {
				var timer = Time.time();
				ParserLogic.parse(Project.canvas);

				arm.nodes.PhotoToPBRNode.cachedSource = null;
				arm.nodes.BrushOutputNode.inst.getAsImage(ChannelBaseColor, function(texbase: Image) {
				arm.nodes.BrushOutputNode.inst.getAsImage(ChannelOcclusion, function(texocc: Image) {
				arm.nodes.BrushOutputNode.inst.getAsImage(ChannelRoughness, function(texrough: Image) {
				arm.nodes.BrushOutputNode.inst.getAsImage(ChannelNormalMap, function(texnor: Image) {
				arm.nodes.BrushOutputNode.inst.getAsImage(ChannelHeight, function(texheight: Image) {

					if (texbase != null) {
						var texpaint = RenderPath.active.renderTargets.get("texpaint").image;
						texpaint.g2.begin(false);
						texpaint.g2.drawScaledImage(texbase, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint.g2.end();
					}

					if (texnor != null) {
						var texpaint_nor = RenderPath.active.renderTargets.get("texpaint_nor").image;
						texpaint_nor.g2.begin(false);
						texpaint_nor.g2.drawScaledImage(texnor, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_nor.g2.end();
					}

					if (Base.pipeCopy == null) Base.makePipe();
					if (Base.pipeCopyA == null) Base.makePipeCopyA();
					if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();

					var texpaint_pack = RenderPath.active.renderTargets.get("texpaint_pack").image;

					if (texocc != null) {
						texpaint_pack.g2.begin(false);
						texpaint_pack.g2.pipeline = Base.pipeCopyR;
						texpaint_pack.g2.drawScaledImage(texocc, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						texpaint_pack.g2.end();
					}

					if (texrough != null) {
						texpaint_pack.g2.begin(false);
						texpaint_pack.g2.pipeline = Base.pipeCopyG;
						texpaint_pack.g2.drawScaledImage(texrough, 0, 0, Config.getTextureResX(), Config.getTextureResY());
						texpaint_pack.g2.pipeline = null;
						texpaint_pack.g2.end();
					}

					if (texheight != null) {
						texpaint_pack.g4.begin();
						texpaint_pack.g4.setPipeline(Base.pipeCopyA);
						texpaint_pack.g4.setTexture(Base.pipeCopyATex, texheight);
						texpaint_pack.g4.setVertexBuffer(ConstData.screenAlignedVB);
						texpaint_pack.g4.setIndexBuffer(ConstData.screenAlignedIB);
						texpaint_pack.g4.drawIndexedVertices();
						texpaint_pack.g4.end();

						if (UIHeader.inst.worktab.position == Space3D &&
							!Std.isOfType(arm.nodes.BrushOutputNode.inst.inputs[ChannelHeight].node, arm.nodes.FloatNode)) {

							// Make copy of vertices before displacement
							var o = Project.paintObjects[0];
							var g = o.data;
							var vertices = g.vertexBuffer.lock();
							if (lastVertices == null || lastVertices.byteLength != vertices.byteLength) {
								lastVertices = new DataView(new ArrayBuffer(vertices.byteLength));
								for (i in 0...Std.int(vertices.byteLength / 2)) {
									lastVertices.setInt16(i * 2, vertices.getInt16(i * 2, true), true);
								}
							}
							else {
								for (i in 0...Std.int(vertices.byteLength / 2)) {
									vertices.setInt16(i * 2, lastVertices.getInt16(i * 2, true), true);
								}
							}
							g.vertexBuffer.unlock();

							// Apply displacement
							if (Config.raw.displace_strength > 0) {
								tasks++;
								Base.notifyOnNextFrame(function() {
									Console.progress(tr("Apply Displacement"));
									Base.notifyOnNextFrame(function() {
										var uv_scale = Scene.active.meshes[0].data.scaleTex * Context.raw.brushScale;
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

		#if (krom_android || krom_ios)
		ui.combo(Base.resHandle, ["2K", "4K"], tr("Resolution"));
		#else
		ui.combo(Base.resHandle, ["2K", "4K", "8K", "16K"], tr("Resolution"));
		#end
		if (Base.resHandle.changed) {
			Base.onLayersResized();
		}
		ui._x += ew + 3;
		ui._y = 2 + startY;
	}
}
