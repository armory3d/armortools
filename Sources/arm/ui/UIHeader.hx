package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import iron.RenderPath;
import arm.node.MakeMaterial;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.io.ImportFont;
import arm.Enums;

class UIHeader {

	public static var inst: UIHeader;

	public static inline var defaultHeaderH = 28;

	public var headerHandle = new Handle({layout: Horizontal});
	public var headerh = defaultHeaderH;
	public var worktab = Id.handle();

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UISidebar.inst.ui;

		var panelx = iron.App.x();
		if (ui.window(headerHandle, panelx, headerh, System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW], Std.int(defaultHeaderH * ui.SCALE()))) {
			ui._y += 2;

			if (Context.tool == ToolColorId) {
				ui.text(tr("Picked Color"));
				if (Context.colorIdPicked) {
					ui.image(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0xffffffff, 64);
				}
				if (ui.button(tr("Clear"))) Context.colorIdPicked = false;
				ui.text(tr("Color ID Map"));
				var cid = ui.combo(Context.colorIdHandle, App.enumTexts("TEX_IMAGE"), tr("Color ID"));
				if (Context.colorIdHandle.changed) Context.ddirty = 2;
				if (Project.assets.length > 0) ui.image(Project.getImage(Project.assets[cid]));
			}
			else if (Context.tool == ToolPicker) {
				var baseRPicked = Math.round(Context.swatch.base.R * 10) / 10;
				var baseGPicked = Math.round(Context.swatch.base.G * 10) / 10;
				var baseBPicked = Math.round(Context.swatch.base.B * 10) / 10;
				var normalRPicked = Math.round(Context.swatch.normal.R * 10) / 10;
				var normalGPicked = Math.round(Context.swatch.normal.G * 10) / 10;
				var normalBPicked = Math.round(Context.swatch.normal.B * 10) / 10;
				var occlusionPicked = Math.round(Context.swatch.occlusion * 100) / 100;
				var roughnessPicked = Math.round(Context.swatch.roughness * 100) / 100;
				var metallicPicked = Math.round(Context.swatch.metallic * 100) / 100;
				var heightPicked = Math.round(Context.swatch.height * 100) / 100;
				var opacityPicked = Math.round(Context.swatch.opacity * 100) / 100;

				var h = Id.handle();
				h.color.R = baseRPicked;
				h.color.G = baseGPicked;
				h.color.B = baseBPicked;
				ui.text("", 0, h.color);
				if (ui.isHovered && ui.inputReleased) {
					UIMenu.draw(function(ui) {
						ui.fill(0, 0, ui._w / ui.ops.scaleFactor, ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
						ui.changed = false;
						zui.Ext.colorWheel(ui, h, false, null, 10 * ui.t.ELEMENT_H * ui.SCALE(), false);
						if (ui.changed) UIMenu.keepOpen = true;
					}, 10);
				}

				ui.text(tr("Base") + ' ($baseRPicked,$baseGPicked,$baseBPicked)');
				ui.text(tr("Normal") + ' ($normalRPicked,$normalGPicked,$normalBPicked)');
				ui.text(tr("Occlusion") + ' ($occlusionPicked)');
				ui.text(tr("Roughness") + ' ($roughnessPicked)');
				ui.text(tr("Metallic") + ' ($metallicPicked)');
				ui.text(tr("Height") + ' ($heightPicked)');
				ui.text(tr("Opacity") + ' ($opacityPicked)');
				Context.pickerSelectMaterial = ui.check(Id.handle({selected: Context.pickerSelectMaterial}), tr("Select Material"));
				ui.combo(Context.pickerMaskHandle, [tr("None"), tr("Material")], tr("Mask"), true);
				if (Context.pickerMaskHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}
			else if (Context.tool == ToolBake) {
				ui.changed = false;

				#if (kha_direct3d12 || kha_vulkan)
				var baking = Context.pdirty > 0;
				var rtBake = Context.bakeType == BakeAO || Context.bakeType == BakeLightmap || Context.bakeType == BakeBentNormal || Context.bakeType == BakeThickness;
				if (baking && ui.button(tr("Stop"))) {
					Context.pdirty = 0;
					Context.rdirty = 2;
				}
				#else
				var baking = false;
				var rtBake = false;
				#end

				if (!baking && ui.button(tr("Bake"))) {
					Context.pdirty = rtBake ? Context.bakeSamples : 1;
					Context.rdirty = 3;
					App.notifyOnNextFrame(function() {
						Context.layerPreviewDirty = true;
					});
					UISidebar.inst.hwnd0.redraws = 2;
					History.pushUndo = true;
				}

				var bakeHandle = Id.handle({position: Context.bakeType});
				var bakes = [
					tr("AO"),
					tr("Curvature"),
					tr("Normal"),
					tr("Object Normal"),
					tr("Height"),
					tr("Derivative"),
					tr("Position"),
					tr("TexCoord"),
					tr("Material ID"),
					tr("Object ID"),
					tr("Vertex Color"),
				];
				#if (kha_direct3d12 || kha_vulkan)
				bakes.push(tr("Lightmap"));
				bakes.push(tr("Bent Normal"));
				bakes.push(tr("Thickness"));
				#end
				Context.bakeType = ui.combo(bakeHandle, bakes, tr("Bake"));

				#if (kha_direct3d12 || kha_vulkan)
				if (rtBake) {
					var samplesHandle = Id.handle({value: Context.bakeSamples});
					Context.bakeSamples = Std.int(ui.slider(samplesHandle, tr("Samples"), 1, 512, true, 1));
				}
				#end

				if (Context.bakeType == BakeNormalObject || Context.bakeType == BakePosition || Context.bakeType == BakeBentNormal) {
					var bakeUpAxisHandle = Id.handle({position: Context.bakeUpAxis});
					Context.bakeUpAxis = ui.combo(bakeUpAxisHandle, [tr("Z"), tr("Y")], tr("Up Axis"), true);
				}
				if (Context.bakeType == BakeAO || Context.bakeType == BakeCurvature) {
					var bakeAxisHandle = Id.handle({position: Context.bakeAxis});
					Context.bakeAxis = ui.combo(bakeAxisHandle, [tr("XYZ"), tr("X"), tr("Y"), tr("Z"), tr("-X"), tr("-Y"), tr("-Z")], tr("Axis"), true);
				}
				if (Context.bakeType == BakeAO) {
					var strengthHandle = Id.handle({value: Context.bakeAoStrength});
					Context.bakeAoStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
					var radiusHandle = Id.handle({value: Context.bakeAoRadius});
					Context.bakeAoRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
					var offsetHandle = Id.handle({value: Context.bakeAoOffset});
					Context.bakeAoOffset = ui.slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
				}
				#if (kha_direct3d12 || kha_vulkan)
				if (rtBake) {
					ui.text(tr("Rays/pix:") + ' ${arm.render.RenderPathRaytraceBake.raysPix}');
					ui.text(tr("Rays/sec:") + ' ${arm.render.RenderPathRaytraceBake.raysSec}');
				}
				#end
				if (Context.bakeType == BakeCurvature) {
					var strengthHandle = Id.handle({value: Context.bakeCurvStrength});
					Context.bakeCurvStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
					var radiusHandle = Id.handle({value: Context.bakeCurvRadius});
					Context.bakeCurvRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
					var offsetHandle = Id.handle({value: Context.bakeCurvOffset});
					Context.bakeCurvOffset = ui.slider(offsetHandle, tr("Offset"), -2.0, 2.0, true);
					var smoothHandle = Id.handle({value: Context.bakeCurvSmooth});
					Context.bakeCurvSmooth = Std.int(ui.slider(smoothHandle, tr("Smooth"), 0, 5, false, 1));
				}
				if (Context.bakeType == BakeNormal || Context.bakeType == BakeHeight || Context.bakeType == BakeDerivative) {
					var ar = [for (p in Project.paintObjects) p.name];
					var polyHandle = Id.handle({position: Context.bakeHighPoly});
					Context.bakeHighPoly = ui.combo(polyHandle, ar, tr("High Poly"));
				}
				if (ui.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}
			else if (Context.tool == ToolBrush ||
					 Context.tool == ToolEraser ||
					 Context.tool == ToolFill ||
					 Context.tool == ToolDecal ||
					 Context.tool == ToolText ||
					 Context.tool == ToolClone ||
					 Context.tool == ToolBlur ||
					 Context.tool == ToolParticle) {

				var decal = Context.tool == ToolDecal || Context.tool == ToolText;
				var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);
				if (Context.tool != ToolFill) {
					if (decalMask) {
						Context.brushDecalMaskRadius = ui.slider(Context.brushDecalMaskRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					}
					else {
						Context.brushRadius = ui.slider(Context.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					}
				}

				if (Context.tool == ToolDecal || Context.tool == ToolText) {
					Context.brushScaleX = ui.slider(Context.brushScaleXHandle, tr("Scale X"), 0.01, 2.0, true);
				}

				if (Context.tool == ToolBrush  ||
					Context.tool == ToolFill   ||
					Context.tool == ToolDecal  ||
					Context.tool == ToolText) {
					var brushScaleHandle = Id.handle({value: Context.brushScale});
					Context.brushScale = ui.slider(brushScaleHandle, tr("UV Scale"), 0.01, 5.0, true);
					if (brushScaleHandle.changed) {
						if (Context.tool == ToolDecal || Context.tool == ToolText) {
							ui.g.end();
							RenderUtil.makeDecalPreview();
							ui.g.begin(false);
						}
					}

					Context.brushAngle = ui.slider(Context.brushAngleHandle, tr("Angle"), 0.0, 360.0, true, 1);
					if (Context.brushAngleHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
				}

				Context.brushOpacity = ui.slider(Context.brushOpacityHandle, tr("Opacity"), 0.0, 1.0, true);

				if (Context.tool == ToolBrush || Context.tool == ToolEraser || decalMask) {
					Context.brushHardness = ui.slider(Id.handle({value: Context.brushHardness}), tr("Hardness"), 0.0, 1.0, true);
				}

				if (Context.tool != ToolEraser) {
					var brushBlendingHandle = Id.handle({value: Context.brushBlending});
					Context.brushBlending = ui.combo(brushBlendingHandle, [
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
						MakeMaterial.parsePaintMaterial();
					}
				}

				if (Context.tool == ToolBrush || Context.tool == ToolFill) {
					var paintHandle = Id.handle();
					Context.brushPaint = ui.combo(paintHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
					if (paintHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
				}
				if (Context.tool == ToolText) {
					var h = Id.handle();
					h.text = Context.textToolText;
					Context.textToolText = ui.textInput(h, "");
					if (h.changed) {
						ui.g.end();
						RenderUtil.makeTextPreview();
						RenderUtil.makeDecalPreview();
						ui.g.begin(false);
					}
				}

				if (Context.tool == ToolFill) {
					ui.combo(Context.fillTypeHandle, [tr("Object"), tr("Face"), tr("Angle"), tr("UV Island")], tr("Fill Mode"));
					if (Context.fillTypeHandle.changed) {
						if (Context.fillTypeHandle.position == FillFace) {
							ui.g.end();
							// UVUtil.cacheUVMap();
							UVUtil.cacheTriangleMap();
							ui.g.begin(false);
							// wireframeHandle.selected = drawWireframe = true;
						}
						MakeMaterial.parsePaintMaterial();
						MakeMaterial.parseMeshMaterial();
					}
				}
				else {
					var _w = ui._w;
					var sc = ui.SCALE();
					ui._w = Std.int(60 * sc);

					var xrayHandle = Id.handle({selected: Context.xray});
					Context.xray = ui.check(xrayHandle, tr("X-Ray"));
					if (xrayHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}

					var symXHandle = Id.handle({selected: false});
					var symYHandle = Id.handle({selected: false});
					var symZHandle = Id.handle({selected: false});
					#if krom_ios
					ui._x -= 10 * sc;
					#else
					ui._w = Std.int(56 * sc);
					ui.text(tr("Symmetry"));
					#end
					ui._w = Std.int(25 * sc);
					Context.symX = ui.check(symXHandle, tr("X"));
					Context.symY = ui.check(symYHandle, tr("Y"));
					Context.symZ = ui.check(symZHandle, tr("Z"));
					if (symXHandle.changed || symYHandle.changed || symZHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
					ui._w = _w;
				}

				if (Context.tool == ToolBlur) {
					ui._x += 10 * ui.SCALE();
					var dirHandle = Id.handle({selected: false});
					Context.blurDirectional = ui.check(dirHandle, tr("Directional"));
					if (dirHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
				}

				if (Context.tool == ToolParticle) {
					ui._x += 10 * ui.SCALE();
					var physHandle = Id.handle({selected: false});
				}
			}
		}
	}
}
