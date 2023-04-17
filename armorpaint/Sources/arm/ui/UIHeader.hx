package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import iron.RenderPath;
import iron.system.Input;
import arm.shader.MakeMaterial;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.io.ImportAsset;
import arm.sys.Path;
import arm.ProjectBaseFormat;

class UIHeader {

	public static var inst: UIHeader;

	#if (krom_android || krom_ios)
	public static inline var defaultHeaderH = 28 + 4;
	#else
	public static inline var defaultHeaderH = 28;
	#end

	public var headerHandle = new Handle({ layout: Horizontal });
	public var headerh = defaultHeaderH;
	public var worktab = Id.handle();

	public function new() {
		inst = this;
	}

	@:access(zui.Zui)
	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UIBase.inst.ui;

		var panelx = iron.App.x();
		if (ui.window(headerHandle, panelx, headerh, System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW], Std.int(defaultHeaderH * ui.SCALE()))) {
			ui._y += 2;

			if (Context.raw.tool == ToolColorId) {
				ui.text(tr("Picked Color"));
				if (Context.raw.colorIdPicked) {
					ui.image(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0xffffffff, 64);
				}
				ui.enabled = Context.raw.colorIdPicked;
				if (ui.button(tr("Clear"))) {
					Context.raw.colorIdPicked = false;
					UIToolbar.inst.toolbarHandle.redraws = 1;
				}
				ui.enabled = true;
				ui.text(tr("Color ID Map"));
				if (Project.assetNames.length > 0) {
					var cid = ui.combo(Context.raw.colorIdHandle, App.enumTexts("TEX_IMAGE"), tr("Color ID"));
					if (Context.raw.colorIdHandle.changed) {
						Context.raw.ddirty = 2;
						Context.raw.colorIdPicked = false;
						UIToolbar.inst.toolbarHandle.redraws = 1;
					}
					ui.image(Project.getImage(Project.assets[cid]));
					if (ui.isHovered) ui.tooltipImage(Project.getImage(Project.assets[cid]), 256);
				}
				if (ui.button(tr("Import"))) {
					UIFiles.show(Path.textureFormats.join(","), false, true, function(path: String) {
						ImportAsset.run(path, -1.0, -1.0, true, false);

						Context.raw.colorIdHandle.position = Project.assetNames.length - 1;
						for (a in Project.assets) {
							// Already imported
							if (a.file == path) Context.raw.colorIdHandle.position = Project.assets.indexOf(a);
						}
						Context.raw.ddirty = 2;
						Context.raw.colorIdPicked = false;
						UIToolbar.inst.toolbarHandle.redraws = 1;
						UIBase.inst.hwnds[2].redraws = 2;
					});
				}
				ui.enabled = Context.raw.colorIdPicked;
				if (ui.button(tr("To Mask"))) {
					if (Context.raw.layer.isMask()) Context.setLayer(Context.raw.layer.parent);
					var m = App.newMask(false, Context.raw.layer);
					function _next() {
						if (App.pipeMerge == null) App.makePipe();
						if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
						m.texpaint.g4.begin();
						m.texpaint.g4.setPipeline(App.pipeColorIdToMask);
						m.texpaint.g4.setTexture(App.texpaintColorId, RenderPath.active.renderTargets.get("texpaint_colorid").image);
						m.texpaint.g4.setTexture(App.texColorId, Project.getImage(Project.assets[Context.raw.colorIdHandle.position]));
						m.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
						m.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
						m.texpaint.g4.drawIndexedVertices();
						m.texpaint.g4.end();
						Context.raw.colorIdPicked = false;
						UIToolbar.inst.toolbarHandle.redraws = 1;
						UIHeader.inst.headerHandle.redraws = 1;
						Context.raw.layerPreviewDirty = true;
						App.updateFillLayers();
					}
					App.notifyOnNextFrame(_next);
					History.newWhiteMask();
				}
				ui.enabled = true;
			}
			else if (Context.raw.tool == ToolPicker) {
				var baseRPicked = Math.round(Context.raw.pickedColor.base.R * 10) / 10;
				var baseGPicked = Math.round(Context.raw.pickedColor.base.G * 10) / 10;
				var baseBPicked = Math.round(Context.raw.pickedColor.base.B * 10) / 10;
				var normalRPicked = Math.round(Context.raw.pickedColor.normal.R * 10) / 10;
				var normalGPicked = Math.round(Context.raw.pickedColor.normal.G * 10) / 10;
				var normalBPicked = Math.round(Context.raw.pickedColor.normal.B * 10) / 10;
				var occlusionPicked = Math.round(Context.raw.pickedColor.occlusion * 100) / 100;
				var roughnessPicked = Math.round(Context.raw.pickedColor.roughness * 100) / 100;
				var metallicPicked = Math.round(Context.raw.pickedColor.metallic * 100) / 100;
				var heightPicked = Math.round(Context.raw.pickedColor.height * 100) / 100;
				var opacityPicked = Math.round(Context.raw.pickedColor.opacity * 100) / 100;

				var h = Id.handle();
				h.color.R = baseRPicked;
				h.color.G = baseGPicked;
				h.color.B = baseBPicked;
				var state = ui.text("", 0, h.color);
				if (state == State.Started) {
					var mouse = Input.getMouse();
					var uix = ui._x;
					var uiy = ui._y;
					App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
					App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
					App.dragSwatch = Project.cloneSwatch(Context.raw.pickedColor);
				}
				if (ui.isHovered) ui.tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
				if (ui.isHovered && ui.inputReleased) {
					UIMenu.draw(function(ui) {
						ui.fill(0, 0, ui._w / ui.ops.scaleFactor, ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
						ui.changed = false;
						zui.Ext.colorWheel(ui, h, false, null, 10 * ui.t.ELEMENT_H * ui.SCALE(), false);
						if (ui.changed) UIMenu.keepOpen = true;
					}, 10);
				}
				if (ui.button(tr("Add Swatch"))) {
					var newSwatch = Project.cloneSwatch(Context.raw.pickedColor);
					ContextBase.setSwatch(newSwatch);
					Project.raw.swatches.push(newSwatch);
					UIBase.inst.hwnds[2].redraws = 1;
				}
				if (ui.isHovered) ui.tooltip(tr("Add picked color to swatches"));

				ui.text(tr("Base") + ' ($baseRPicked,$baseGPicked,$baseBPicked)');
				ui.text(tr("Normal") + ' ($normalRPicked,$normalGPicked,$normalBPicked)');
				ui.text(tr("Occlusion") + ' ($occlusionPicked)');
				ui.text(tr("Roughness") + ' ($roughnessPicked)');
				ui.text(tr("Metallic") + ' ($metallicPicked)');
				ui.text(tr("Height") + ' ($heightPicked)');
				ui.text(tr("Opacity") + ' ($opacityPicked)');
				Context.raw.pickerSelectMaterial = ui.check(Id.handle({ selected: Context.raw.pickerSelectMaterial }), tr("Select Material"));
				ui.combo(Context.raw.pickerMaskHandle, [tr("None"), tr("Material")], tr("Mask"), true);
				if (Context.raw.pickerMaskHandle.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}
			else if (Context.raw.tool == ToolBake) {
				ui.changed = false;

				#if (kha_direct3d12 || kha_vulkan)
				var baking = Context.raw.pdirty > 0;
				var rtBake = Context.raw.bakeType == BakeAO || Context.raw.bakeType == BakeLightmap || Context.raw.bakeType == BakeBentNormal || Context.raw.bakeType == BakeThickness;
				if (baking && ui.button(tr("Stop"))) {
					Context.raw.pdirty = 0;
					Context.raw.rdirty = 2;
				}
				#else
				var baking = false;
				var rtBake = false;
				#end

				if (!baking && ui.button(tr("Bake"))) {
					Context.raw.pdirty = rtBake ? Context.raw.bakeSamples : 1;
					Context.raw.rdirty = 3;
					App.notifyOnNextFrame(function() {
						Context.raw.layerPreviewDirty = true;
					});
					UIBase.inst.hwnds[0].redraws = 2;
					History.pushUndo = true;
				}

				var bakeHandle = Id.handle({ position: Context.raw.bakeType });
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
				Context.raw.bakeType = ui.combo(bakeHandle, bakes, tr("Bake"));

				#if (kha_direct3d12 || kha_vulkan)
				if (rtBake) {
					var samplesHandle = Id.handle({ value: Context.raw.bakeSamples });
					Context.raw.bakeSamples = Std.int(ui.slider(samplesHandle, tr("Samples"), 1, 512, true, 1));
				}
				#end

				if (Context.raw.bakeType == BakeNormalObject || Context.raw.bakeType == BakePosition || Context.raw.bakeType == BakeBentNormal) {
					var bakeUpAxisHandle = Id.handle({ position: Context.raw.bakeUpAxis });
					Context.raw.bakeUpAxis = ui.combo(bakeUpAxisHandle, [tr("Z"), tr("Y")], tr("Up Axis"), true);
				}
				if (Context.raw.bakeType == BakeAO || Context.raw.bakeType == BakeCurvature) {
					var bakeAxisHandle = Id.handle({ position: Context.raw.bakeAxis });
					Context.raw.bakeAxis = ui.combo(bakeAxisHandle, [tr("XYZ"), tr("X"), tr("Y"), tr("Z"), tr("-X"), tr("-Y"), tr("-Z")], tr("Axis"), true);
				}
				if (Context.raw.bakeType == BakeAO) {
					var strengthHandle = Id.handle({ value: Context.raw.bakeAoStrength });
					Context.raw.bakeAoStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
					var radiusHandle = Id.handle({ value: Context.raw.bakeAoRadius });
					Context.raw.bakeAoRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
					var offsetHandle = Id.handle({ value: Context.raw.bakeAoOffset });
					Context.raw.bakeAoOffset = ui.slider(offsetHandle, tr("Offset"), 0.0, 2.0, true);
				}
				#if (kha_direct3d12 || kha_vulkan)
				if (rtBake) {
					ui.text(tr("Rays/pix:") + ' ${arm.render.RenderPathRaytraceBake.raysPix}');
					ui.text(tr("Rays/sec:") + ' ${arm.render.RenderPathRaytraceBake.raysSec}');
				}
				#end
				if (Context.raw.bakeType == BakeCurvature) {
					var strengthHandle = Id.handle({ value: Context.raw.bakeCurvStrength });
					Context.raw.bakeCurvStrength = ui.slider(strengthHandle, tr("Strength"), 0.0, 2.0, true);
					var radiusHandle = Id.handle({ value: Context.raw.bakeCurvRadius });
					Context.raw.bakeCurvRadius = ui.slider(radiusHandle, tr("Radius"), 0.0, 2.0, true);
					var offsetHandle = Id.handle({ value: Context.raw.bakeCurvOffset });
					Context.raw.bakeCurvOffset = ui.slider(offsetHandle, tr("Offset"), -2.0, 2.0, true);
					var smoothHandle = Id.handle({ value: Context.raw.bakeCurvSmooth });
					Context.raw.bakeCurvSmooth = Std.int(ui.slider(smoothHandle, tr("Smooth"), 0, 5, false, 1));
				}
				if (Context.raw.bakeType == BakeNormal || Context.raw.bakeType == BakeHeight || Context.raw.bakeType == BakeDerivative) {
					var ar = [for (p in Project.paintObjects) p.name];
					var polyHandle = Id.handle({ position: Context.raw.bakeHighPoly });
					Context.raw.bakeHighPoly = ui.combo(polyHandle, ar, tr("High Poly"));
				}
				if (ui.changed) {
					MakeMaterial.parsePaintMaterial();
				}
			}
			else if (Context.raw.tool == ToolBrush ||
					 Context.raw.tool == ToolEraser ||
					 Context.raw.tool == ToolFill ||
					 Context.raw.tool == ToolDecal ||
					 Context.raw.tool == ToolText ||
					 Context.raw.tool == ToolClone ||
					 Context.raw.tool == ToolBlur ||
					 Context.raw.tool == ToolSmudge ||
					 Context.raw.tool == ToolParticle) {

				var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
				var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutDown);
				if (Context.raw.tool != ToolFill) {
					if (decalMask) {
						Context.raw.brushDecalMaskRadius = ui.slider(Context.raw.brushDecalMaskRadiusHandle, tr("Radius"), 0.01, 2.0, true);
						if (ui.isHovered) ui.tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", ["brush_radius" => Config.keymap.brush_radius, "brush_radius_decrease" => Config.keymap.brush_radius_decrease, "brush_radius_increase" => Config.keymap.brush_radius_increase]));
					}
					else {
						Context.raw.brushRadius = ui.slider(Context.raw.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
						if (ui.isHovered) ui.tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", ["brush_radius" => Config.keymap.brush_radius, "brush_radius_decrease" => Config.keymap.brush_radius_decrease, "brush_radius_increase" => Config.keymap.brush_radius_increase]));	
					}
				}

				if (Context.raw.tool == ToolDecal || Context.raw.tool == ToolText) {
					Context.raw.brushScaleX = ui.slider(Context.raw.brushScaleXHandle, tr("Scale X"), 0.01, 2.0, true);
				}

				if (Context.raw.tool == ToolBrush  ||
					Context.raw.tool == ToolFill   ||
					Context.raw.tool == ToolDecal  ||
					Context.raw.tool == ToolText) {
					var brushScaleHandle = Id.handle({ value: Context.raw.brushScale });
					Context.raw.brushScale = ui.slider(brushScaleHandle, tr("UV Scale"), 0.01, 5.0, true);
					if (brushScaleHandle.changed) {
						if (Context.raw.tool == ToolDecal || Context.raw.tool == ToolText) {
							ui.g.end();
							RenderUtil.makeDecalPreview();
							ui.g.begin(false);
						}
					}

					Context.raw.brushAngle = ui.slider(Context.raw.brushAngleHandle, tr("Angle"), 0.0, 360.0, true, 1);
					if (ui.isHovered) ui.tooltip(tr("Hold {brush_angle} and move mouse to the left to decrease the angle\nHold {brush_angle} and move mouse to the right to increase the angle", ["brush_angle" => Config.keymap.brush_angle]));

					if (Context.raw.brushAngleHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
				}

				Context.raw.brushOpacity = ui.slider(Context.raw.brushOpacityHandle, tr("Opacity"), 0.0, 1.0, true);
				if (ui.isHovered) ui.tooltip(tr("Hold {brush_opacity} and move mouse to the left to decrease the opacity\nHold {brush_opacity} and move mouse to the right to increase the opacity", ["brush_opacity" => Config.keymap.brush_opacity]));

				if (Context.raw.tool == ToolBrush || Context.raw.tool == ToolEraser || Context.raw.tool == ToolClone || decalMask) {
					Context.raw.brushHardness = ui.slider(Id.handle({ value: Context.raw.brushHardness }), tr("Hardness"), 0.0, 1.0, true);
				}

				if (Context.raw.tool != ToolEraser) {
					var brushBlendingHandle = Id.handle({ value: Context.raw.brushBlending });
					Context.raw.brushBlending = ui.combo(brushBlendingHandle, [
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

				if (Context.raw.tool == ToolBrush || Context.raw.tool == ToolFill) {
					var paintHandle = Id.handle();
					Context.raw.brushPaint = ui.combo(paintHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], tr("TexCoord"));
					if (paintHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
				}
				if (Context.raw.tool == ToolText) {
					var h = Id.handle();
					h.text = Context.raw.textToolText;
					var w = ui._w;
					if (ui.textSelectedHandle == h || ui.submitTextHandle == h) {
						ui._w *= 3;
					}
					
					Context.raw.textToolText = ui.textInput(h, "", Left, true, true);
					ui._w = w;

					if (h.changed) {
						ui.g.end();
						RenderUtil.makeTextPreview();
						RenderUtil.makeDecalPreview();
						ui.g.begin(false);
					}
				}

				if (Context.raw.tool == ToolFill) {
					ui.combo(Context.raw.fillTypeHandle, [tr("Object"), tr("Face"), tr("Angle"), tr("UV Island")], tr("Fill Mode"));
					if (Context.raw.fillTypeHandle.changed) {
						if (Context.raw.fillTypeHandle.position == FillFace) {
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

					if (Config.raw.touch_ui) {
						ui._x -= 6 * sc;
					}

					var xrayHandle = Id.handle({ selected: Context.raw.xray });
					Context.raw.xray = ui.check(xrayHandle, tr("X-Ray"));
					if (xrayHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}

					var symXHandle = Id.handle({ selected: false });
					var symYHandle = Id.handle({ selected: false });
					var symZHandle = Id.handle({ selected: false });
					
					if (Config.raw.touch_ui) {
						ui._x -= 6 * sc;
						ui._w = Std.int(27 * sc);
						Context.raw.symX = ui.check(symXHandle, "");
						ui._x -= 12 * sc;
						Context.raw.symY = ui.check(symYHandle, "");
						ui._x -= 12 * sc;
						Context.raw.symZ = ui.check(symZHandle, "");
						ui._x -= 12 * sc;
						ui._w = Std.int(40 * sc);
						ui.text(tr("X") + tr("Y") + tr("Z"));
					}
					else {
						ui._w = Std.int(56 * sc);
						ui.text(tr("Symmetry"));
						ui._w = Std.int(25 * sc);
						Context.raw.symX = ui.check(symXHandle, tr("X"));
						Context.raw.symY = ui.check(symYHandle, tr("Y"));
						Context.raw.symZ = ui.check(symZHandle, tr("Z"));
					}
					
					if (symXHandle.changed || symYHandle.changed || symZHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
					ui._w = _w;
				}

				#if arm_physics
				if (Context.raw.tool == ToolParticle) {
					ui._x += 10 * ui.SCALE();
					var physHandle = Id.handle({ selected: false });
					Context.raw.particlePhysics = ui.check(physHandle, tr("Physics"));
					if (physHandle.changed) {
						arm.util.ParticleUtil.initParticlePhysics();
						MakeMaterial.parsePaintMaterial();
					}
				}
				#end
			}
		}
	}
}
