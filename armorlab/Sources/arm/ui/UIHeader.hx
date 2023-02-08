package arm.ui;

import kha.System;
import zui.Zui;
import zui.Id;
import iron.RenderPath;
import iron.system.Input;
import arm.node.MakeMaterial;
import arm.ProjectFormat.TSwatchColor;
import arm.Enums;

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
		var ui = UISidebar.inst.ui;

		var panelx = iron.App.x();
		if (ui.window(headerHandle, panelx, headerh, System.windowWidth(), Std.int(defaultHeaderH * ui.SCALE()))) {
			ui._y += 2;

			if (Context.tool == ToolPicker) {
				// var baseRPicked = Math.round(Context.pickedColor.base.R * 10) / 10;
				// var baseGPicked = Math.round(Context.pickedColor.base.G * 10) / 10;
				// var baseBPicked = Math.round(Context.pickedColor.base.B * 10) / 10;
				// var normalRPicked = Math.round(Context.pickedColor.normal.R * 10) / 10;
				// var normalGPicked = Math.round(Context.pickedColor.normal.G * 10) / 10;
				// var normalBPicked = Math.round(Context.pickedColor.normal.B * 10) / 10;
				// var occlusionPicked = Math.round(Context.pickedColor.occlusion * 100) / 100;
				// var roughnessPicked = Math.round(Context.pickedColor.roughness * 100) / 100;
				// var metallicPicked = Math.round(Context.pickedColor.metallic * 100) / 100;
				// var heightPicked = Math.round(Context.pickedColor.height * 100) / 100;
				// var opacityPicked = Math.round(Context.pickedColor.opacity * 100) / 100;

				// var h = Id.handle();
				// h.color.R = baseRPicked;
				// h.color.G = baseGPicked;
				// h.color.B = baseBPicked;
				// var state = ui.text("", 0, h.color);
				// if (state == State.Started) {
				// 	var mouse = Input.getMouse();
				// 	var uix = ui._x;
				// 	var uiy = ui._y;
				// 	App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
				// 	App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
				// 	App.dragSwatch = Project.cloneSwatch(Context.pickedColor);
				// }
				// if (ui.isHovered) ui.tooltip(tr("Drag and drop picked color to swatches, materials, layers or to the node editor"));
				// if (ui.isHovered && ui.inputReleased) {
				// 	UIMenu.draw(function(ui) {
				// 		ui.fill(0, 0, ui._w / ui.ops.scaleFactor, ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
				// 		ui.changed = false;
				// 		zui.Ext.colorWheel(ui, h, false, null, 10 * ui.t.ELEMENT_H * ui.SCALE(), false);
				// 		if (ui.changed) UIMenu.keepOpen = true;
				// 	}, 10);
				// }
				// if (ui.button(tr("Add Swatch"))) {
				// 	var newSwatch = Project.cloneSwatch(Context.pickedColor);
				// 	Context.setSwatch(newSwatch);
				// 	Project.raw.swatches.push(newSwatch);
				// 	UIStatus.inst.statusHandle.redraws = 1;
				// }
				// if (ui.isHovered) ui.tooltip(tr("Add picked color to swatches"));

				// ui.text(tr("Base") + ' ($baseRPicked,$baseGPicked,$baseBPicked)');
				// ui.text(tr("Normal") + ' ($normalRPicked,$normalGPicked,$normalBPicked)');
				// ui.text(tr("Occlusion") + ' ($occlusionPicked)');
				// ui.text(tr("Roughness") + ' ($roughnessPicked)');
				// ui.text(tr("Metallic") + ' ($metallicPicked)');
				// ui.text(tr("Height") + ' ($heightPicked)');
				// ui.text(tr("Opacity") + ' ($opacityPicked)');
				// Context.pickerSelectMaterial = ui.check(Id.handle({ selected: Context.pickerSelectMaterial }), tr("Select Material"));
				// ui.combo(Context.pickerMaskHandle, [tr("None"), tr("Material")], tr("Mask"), true);
				// if (Context.pickerMaskHandle.changed) {
				// 	MakeMaterial.parsePaintMaterial();
				// }
			}
			else if (Context.tool == ToolEraser ||
					 Context.tool == ToolClone ||
					 Context.tool == ToolBlur) {

				var inpaint = UINodes.inst.getNodes().nodesSelected.length > 0 && UINodes.inst.getNodes().nodesSelected[0].type == "InpaintNode";
				if (inpaint) {
					Context.brushRadius = ui.slider(Context.brushRadiusHandle, tr("Radius"), 0.01, 2.0, true);
					if (ui.isHovered) ui.tooltip(tr("Hold {brush_radius} and move mouse to the left or press {brush_radius_decrease} to decrease the radius\nHold {brush_radius} and move mouse to the right or press {brush_radius_increase} to increase the radius", ["brush_radius" => Config.keymap.brush_radius, "brush_radius_decrease" => Config.keymap.brush_radius_decrease, "brush_radius_increase" => Config.keymap.brush_radius_increase]));
				}

				// if (Context.tool != ToolEraser) {
				// 	var brushBlendingHandle = Id.handle({ value: Context.brushBlending });
				// 	Context.brushBlending = ui.combo(brushBlendingHandle, [
				// 		tr("Mix"),
				// 		tr("Darken"),
				// 		tr("Multiply"),
				// 		tr("Burn"),
				// 		tr("Lighten"),
				// 		tr("Screen"),
				// 		tr("Dodge"),
				// 		tr("Add"),
				// 		tr("Overlay"),
				// 		tr("Soft Light"),
				// 		tr("Linear Light"),
				// 		tr("Difference"),
				// 		tr("Subtract"),
				// 		tr("Divide"),
				// 		tr("Hue"),
				// 		tr("Saturation"),
				// 		tr("Color"),
				// 		tr("Value"),
				// 	], tr("Blending"));
				// 	if (brushBlendingHandle.changed) {
				// 		MakeMaterial.parsePaintMaterial();
				// 	}
				// }

				if (Context.tool == ToolBlur) {
					ui._x += 10 * ui.SCALE();
					var dirHandle = Id.handle({ selected: false });
					Context.blurDirectional = ui.check(dirHandle, tr("Directional"));
					if (dirHandle.changed) {
						MakeMaterial.parsePaintMaterial();
					}
				}
			}
		}
	}
}
