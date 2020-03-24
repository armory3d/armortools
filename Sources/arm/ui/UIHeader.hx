package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import iron.RenderPath;
import arm.node.MaterialParser;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.io.ImportFont;
import arm.Enums;

class UIHeader {

	public static var inst: UIHeader;

	public static inline var defaultHeaderH = 28;

	public var headerHandle = new Handle({layout: Horizontal});
	public var headerh = defaultHeaderH;

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		var panelx = iron.App.x();
		if (ui.window(headerHandle, panelx, headerh, System.windowWidth() - UIToolbar.inst.toolbarw - UISidebar.inst.windowW, Std.int(defaultHeaderH * ui.SCALE()))) {
			ui._y += 2;

			if (UISidebar.inst.worktab.position == SpacePaint) {

				if (Context.tool == ToolColorId) {
					ui.text(tr("Picked Color"));
					if (UISidebar.inst.colorIdPicked) {
						ui.image(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0xffffffff, 64);
					}
					if (ui.button("Clear")) UISidebar.inst.colorIdPicked = false;
					ui.text(tr("Color ID Map"));
					var cid = ui.combo(UISidebar.inst.colorIdHandle, App.enumTexts("TEX_IMAGE"), tr("Color ID"));
					if (UISidebar.inst.colorIdHandle.changed) Context.ddirty = 2;
					if (Project.assets.length > 0) ui.image(UISidebar.inst.getImage(Project.assets[cid]));
				}
				else if (Context.tool == ToolPicker) {
					UISidebar.inst.baseRPicked = Math.round(UISidebar.inst.baseRPicked * 10) / 10;
					UISidebar.inst.baseGPicked = Math.round(UISidebar.inst.baseGPicked * 10) / 10;
					UISidebar.inst.baseBPicked = Math.round(UISidebar.inst.baseBPicked * 10) / 10;
					UISidebar.inst.normalRPicked = Math.round(UISidebar.inst.normalRPicked * 10) / 10;
					UISidebar.inst.normalGPicked = Math.round(UISidebar.inst.normalGPicked * 10) / 10;
					UISidebar.inst.normalBPicked = Math.round(UISidebar.inst.normalBPicked * 10) / 10;
					UISidebar.inst.occlusionPicked = Math.round(UISidebar.inst.occlusionPicked * 100) / 100;
					UISidebar.inst.roughnessPicked = Math.round(UISidebar.inst.roughnessPicked * 100) / 100;
					UISidebar.inst.metallicPicked = Math.round(UISidebar.inst.metallicPicked * 100) / 100;
					var baseRPicked = UISidebar.inst.baseRPicked;
					var baseGPicked = UISidebar.inst.baseGPicked;
					var baseBPicked = UISidebar.inst.baseBPicked;
					var normalRPicked = UISidebar.inst.normalRPicked;
					var normalGPicked = UISidebar.inst.normalGPicked;
					var normalBPicked = UISidebar.inst.normalBPicked;
					var occlusionPicked = UISidebar.inst.occlusionPicked;
					var roughnessPicked = UISidebar.inst.roughnessPicked;
					var metallicPicked = UISidebar.inst.metallicPicked;
					ui.text(tr("Base") + ' $baseRPicked,$baseGPicked,$baseBPicked');
					ui.text(tr("Nor") + ' $normalRPicked,$normalGPicked,$normalBPicked');
					ui.text(tr("Occlusion") + ' $occlusionPicked');
					ui.text(tr("Roughness") + ' $roughnessPicked');
					ui.text(tr("Metallic") + ' $metallicPicked');
					UISidebar.inst.pickerSelectMaterial = ui.check(Id.handle({selected: UISidebar.inst.pickerSelectMaterial}), tr("Select Material"));
					ui.combo(UISidebar.inst.pickerMaskHandle, [tr("None"), tr("Material")], tr("Mask"), true);
					if (UISidebar.inst.pickerMaskHandle.changed) {
						MaterialParser.parsePaintMaterial();
					}
				}
				else if (Context.tool == ToolBake) {
					ui.changed = false;
					var bakeHandle = Id.handle({position: UISidebar.inst.bakeType});
					var bakes = [
						tr("AO"),
						tr("Curvature"),
						tr("Normal"),
						tr("Normal (Object)"),
						tr("Height"),
						tr("Derivative"),
						tr("Position"),
						tr("TexCoord"),
						tr("Material ID"),
						tr("Object ID"),
						tr("Vertex Color"),
					];
					#if kha_direct3d12
					bakes.push(tr("Lightmap"));
					bakes.push(tr("Bent Normal"));
					bakes.push(tr("Thickness"));
					#end
					UISidebar.inst.bakeType = ui.combo(bakeHandle, bakes, tr("Bake"));
					if (UISidebar.inst.bakeType == BakeNormalObject || UISidebar.inst.bakeType == BakePosition || UISidebar.inst.bakeType == BakeBentNormal) {
						var bakeUpAxisHandle = Id.handle({position: UISidebar.inst.bakeUpAxis});
						UISidebar.inst.bakeUpAxis = ui.combo(bakeUpAxisHandle, ["Z", "Y"], tr("Up Axis"), true);
					}
					if (UISidebar.inst.bakeType == BakeAO || UISidebar.inst.bakeType == BakeCurvature) {
						var bakeAxisHandle = Id.handle({position: UISidebar.inst.bakeAxis});
						UISidebar.inst.bakeAxis = ui.combo(bakeAxisHandle, ["XYZ", "X", "Y", "Z", "-X", "-Y", "-Z"], tr("Axis"), true);
					}
					if (UISidebar.inst.bakeType == BakeAO) {
						var strengthHandle = Id.handle({value: UISidebar.inst.bakeAoStrength});
						UISidebar.inst.bakeAoStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
						var radiusHandle = Id.handle({value: UISidebar.inst.bakeAoRadius});
						UISidebar.inst.bakeAoRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
						var offsetHandle = Id.handle({value: UISidebar.inst.bakeAoOffset});
						UISidebar.inst.bakeAoOffset = ui.slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
					}
					#if kha_direct3d12
					if (UISidebar.inst.bakeType == BakeAO || UISidebar.inst.bakeType == BakeLightmap || UISidebar.inst.bakeType == BakeBentNormal || UISidebar.inst.bakeType == BakeThickness) {
						ui.text(tr("Rays/pix:") + ' ${arm.render.RenderPathRaytrace.raysPix}');
						ui.text(tr("Rays/sec:") + ' ${arm.render.RenderPathRaytrace.raysSec}');
					}
					#end
					if (UISidebar.inst.bakeType == BakeCurvature) {
						var strengthHandle = Id.handle({value: UISidebar.inst.bakeCurvStrength});
						UISidebar.inst.bakeCurvStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
						var radiusHandle = Id.handle({value: UISidebar.inst.bakeCurvRadius});
						UISidebar.inst.bakeCurvRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
						var offsetHandle = Id.handle({value: UISidebar.inst.bakeCurvOffset});
						UISidebar.inst.bakeCurvOffset = ui.slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
						var smoothHandle = Id.handle({value: UISidebar.inst.bakeCurvSmooth});
						UISidebar.inst.bakeCurvSmooth = Std.int(ui.slider(smoothHandle, tr("Smooth"), 0, 5, false, 1));
					}
					if (UISidebar.inst.bakeType == BakeNormal || UISidebar.inst.bakeType == BakeHeight || UISidebar.inst.bakeType == BakeDerivative) {
						var ar = [for (p in Project.paintObjects) p.name];
						var polyHandle = Id.handle({position: UISidebar.inst.bakeHighPoly});
						UISidebar.inst.bakeHighPoly = ui.combo(polyHandle, ar, tr("High Poly"));
					}
					if (ui.changed) {
						MaterialParser.parsePaintMaterial();
					}
				}
				else {
					if (Context.tool != ToolFill) {
						UISidebar.inst.brushRadius = ui.slider(UISidebar.inst.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					}

					if (Context.tool == ToolDecal) {
						UISidebar.inst.brushScaleX = ui.slider(UISidebar.inst.brushScaleXHandle, tr("Scale X"), 0.01, 2.0, true);
					}

					if (Context.tool == ToolBrush  ||
						Context.tool == ToolFill   ||
						Context.tool == ToolDecal  ||
						Context.tool == ToolText) {
						var brushScaleHandle = Id.handle({value: UISidebar.inst.brushScale});
						UISidebar.inst.brushScale = ui.slider(brushScaleHandle, tr("Scale"), 0.01, 5.0, true);
						if (brushScaleHandle.changed) {
							if (Context.tool == ToolDecal || Context.tool == ToolText) {
								ui.g.end();
								RenderUtil.makeDecalPreview();
								ui.g.begin(false);
							}
						}

						UISidebar.inst.brushAngle = ui.slider(UISidebar.inst.brushAngleHandle, tr("Angle"), 0.0, 360.0, true, 1);
						if (UISidebar.inst.brushAngleHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
					}

					UISidebar.inst.brushOpacity = ui.slider(UISidebar.inst.brushOpacityHandle, tr("Opacity"), 0.0, 1.0, true);

					if (Context.tool == ToolBrush || Context.tool == ToolEraser) {
						UISidebar.inst.brushHardness = ui.slider(Id.handle({value: UISidebar.inst.brushHardness}), tr("Hardness"), 0.0, 1.0, true);
					}

					if (Context.tool != ToolEraser) {
						var brushBlendingHandle = Id.handle({value: UISidebar.inst.brushBlending});
						UISidebar.inst.brushBlending = ui.combo(brushBlendingHandle, [
							tr("Mix"),
							tr("Darken"),
							tr("Multiply"),
							tr("Burn"),
							tr("Lighten"),
							tr("Screen"),
							tr("Dodge"),
							tr("Add"),
							tr("Overlay"),
							tr("Soft Light"),
							tr("Linear Light"),
							tr("Difference"),
							tr("Subtract"),
							tr("Divide"),
							tr("Hue"),
							tr("Saturation"),
							tr("Color"),
							tr("Value"),
						], tr("Blending"));
						if (brushBlendingHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
					}

					if (Context.tool == ToolBrush || Context.tool == ToolFill) {
						var paintHandle = Id.handle();
						UISidebar.inst.brushPaint = ui.combo(paintHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
						if (paintHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}
					}
					if (Context.tool == ToolText) {
						ui.combo(UISidebar.inst.textToolHandle, ImportFont.fontList, tr("Font"));
						var h = Id.handle();
						h.text = UISidebar.inst.textToolText;
						UISidebar.inst.textToolText = ui.textInput(h, "");
						if (h.changed || UISidebar.inst.textToolHandle.changed) {
							ui.g.end();
							RenderUtil.makeTextPreview();
							RenderUtil.makeDecalPreview();
							ui.g.begin(false);
						}
					}

					if (Context.tool == ToolFill) {
						ui.combo(UISidebar.inst.fillTypeHandle, [tr("Object"), tr("Face"), tr("Angle")], tr("Fill Mode"));
						if (UISidebar.inst.fillTypeHandle.changed) {
							if (UISidebar.inst.fillTypeHandle.position == FillFace) {
								ui.g.end();
								// UVUtil.cacheUVMap();
								UVUtil.cacheTriangleMap();
								ui.g.begin(false);
								// wireframeHandle.selected = drawWireframe = true;
							}
							MaterialParser.parsePaintMaterial();
							MaterialParser.parseMeshMaterial();
						}
					}
					else {
						var _w = ui._w;
						var sc = ui.SCALE();
						ui._w = Std.int(60 * sc);

						var xrayHandle = Id.handle({selected: UISidebar.inst.xray});
						UISidebar.inst.xray = ui.check(xrayHandle, tr("X-Ray"));
						if (xrayHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}

						var symXHandle = Id.handle({selected: false});
						var symYHandle = Id.handle({selected: false});
						var symZHandle = Id.handle({selected: false});
						ui._w = Std.int(55 * sc);
						ui.text("Symmetry");
						ui._w = Std.int(25 * sc);
						UISidebar.inst.symX = ui.check(symXHandle, "X");
						UISidebar.inst.symY = ui.check(symYHandle, "Y");
						UISidebar.inst.symZ = ui.check(symZHandle, "Z");
						if (symXHandle.changed || symYHandle.changed || symZHandle.changed) {
							MaterialParser.parsePaintMaterial();
						}

						ui._w = _w;
					}
				}
			}
		}
	}
}
