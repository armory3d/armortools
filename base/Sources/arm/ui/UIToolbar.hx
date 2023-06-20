package arm.ui;

#if (is_paint || is_sculpt)

import kha.System;
import zui.Zui;
import iron.RenderPath;
import arm.Translator._tr;

@:access(zui.Zui)
class UIToolbar {

	public static var inst: UIToolbar;
	public static inline var defaultToolbarW = 36;

	public var toolbarHandle = new Handle();
	public var toolbarw = defaultToolbarW;
	var lastTool = 0;

	public var toolNames = [
		_tr("Brush"),
		_tr("Eraser"),
		_tr("Fill"),
		_tr("Decal"),
		_tr("Text"),
		_tr("Clone"),
		_tr("Blur"),
		_tr("Smudge"),
		_tr("Particle"),
		_tr("ColorID"),
		_tr("Picker"),
		_tr("Bake"),
		_tr("Gizmo"),
		_tr("Material"),
	];

	public function new() {
		inst = this;
	}

	public function renderUI(g: kha.graphics2.Graphics) {
		var ui = UIBase.inst.ui;

		if (Config.raw.touch_ui) {
			toolbarw = defaultToolbarW + 6;
		}
		else {
			toolbarw = defaultToolbarW;
		}
		toolbarw = Std.int(toolbarw * ui.SCALE());

		if (ui.window(toolbarHandle, 0, UIHeader.headerh, toolbarw, System.windowHeight() - UIHeader.headerh)) {
			ui._y -= 4 * ui.SCALE();

			ui.imageScrollAlign = false;
			var img = Res.get("icons.k");
			var imgw = ui.SCALE() > 1 ? 100 : 50;

			var col = ui.t.WINDOW_BG_COL;
			if (col < 0) col += untyped 4294967296;
			var light = col > 0xff666666 + 4294967296;
			var iconAccent = light ? 0xff666666 : -1;

			// Properties icon
			if (Config.raw.layout[LayoutHeader] == 1) {
				var rect = Res.tile50(img, 7, 1);
				if (ui.image(img, light ? 0xff666666 : ui.t.BUTTON_COL, null, rect.x, rect.y, rect.w, rect.h) == State.Released) {
					Config.raw.layout[LayoutHeader] = 0;
				}
			}
			// Draw ">>" button if header is hidden
			else {
				var _ELEMENT_H = ui.t.ELEMENT_H;
				var _BUTTON_H = ui.t.BUTTON_H;
				var _BUTTON_COL = ui.t.BUTTON_COL;
				var _fontOffsetY = ui.fontOffsetY;
				ui.t.ELEMENT_H = Std.int(ui.t.ELEMENT_H * 1.5);
				ui.t.BUTTON_H = ui.t.ELEMENT_H;
				ui.t.BUTTON_COL = ui.t.WINDOW_BG_COL;
				var fontHeight = ui.ops.font.height(ui.fontSize);
				ui.fontOffsetY = (ui.ELEMENT_H() - fontHeight) / 2;
				var _w = ui._w;
				ui._w = toolbarw;

				if (ui.button(">>")) {
					toolPropertiesMenu();
				}

				ui._w = _w;
				ui.t.ELEMENT_H = _ELEMENT_H;
				ui.t.BUTTON_H = _BUTTON_H;
				ui.t.BUTTON_COL = _BUTTON_COL;
				ui.fontOffsetY = _fontOffsetY;
			}
			if (ui.isHovered) ui.tooltip(tr("Toggle header"));
			ui._y -= 4 * ui.SCALE();

			#if is_paint

			var keys = [
				"(" + Config.keymap.tool_brush + ") - " + tr("Hold {action_paint} to paint\nHold {key} and press {action_paint} to paint a straight line (ruler mode)", ["key" => Config.keymap.brush_ruler, "action_paint" => Config.keymap.action_paint]),
				"(" + Config.keymap.tool_eraser + ") - " + tr("Hold {action_paint} to erase\nHold {key} and press {action_paint} to erase a straight line (ruler mode)", ["key" => Config.keymap.brush_ruler, "action_paint" => Config.keymap.action_paint]),
				"(" + Config.keymap.tool_fill + ")",
				"(" + Config.keymap.tool_decal + ") - " + tr("Hold {key} to paint on a decal mask", ["key" => Config.keymap.decal_mask]),
				"(" + Config.keymap.tool_text + ") - " + tr("Hold {key} to use the text as a mask", ["key" => Config.keymap.decal_mask]),
				"(" + Config.keymap.tool_clone + ") - " + tr("Hold {key} to set source", ["key" => Config.keymap.set_clone_source]),
				"(" + Config.keymap.tool_blur + ")",
				"(" + Config.keymap.tool_smudge + ")",
				"(" + Config.keymap.tool_particle + ")",
				"(" + Config.keymap.tool_colorid + ")",
				"(" + Config.keymap.tool_picker + ")",
				"(" + Config.keymap.tool_bake + ")",
				"(" + Config.keymap.tool_gizmo + ")",
				"(" + Config.keymap.tool_material + ")",
			];

			function drawTool(i: Int) {
				ui._x += 2;
				if (Context.raw.tool == i) drawHighlight();
				var tileY = Std.int(i / 12);
				var tileX = tileY % 2 == 0 ? i % 12 : (11 - (i % 12));
				var rect = Res.tile50(img, tileX, tileY);
				var _y = ui._y;

				var imageState = ui.image(img, iconAccent, null, rect.x, rect.y, rect.w, rect.h);
				if (imageState == State.Started) {
					Context.selectTool(i);
				}
				else if (imageState == State.Released && Config.raw.layout[LayoutHeader] == 0) {
					if (lastTool == i) {
						toolPropertiesMenu();
					}
					lastTool = i;
				}

				if (i == ToolColorId && Context.raw.colorIdPicked) {
					ui.g.drawScaledSubImage(RenderPath.active.renderTargets.get("texpaint_colorid").image, 0, 0, 1, 1, 0, _y + 1.5 * ui.SCALE(), 5 * ui.SCALE(), 34 * ui.SCALE());
				}

				if (ui.isHovered) ui.tooltip(tr(toolNames[i]) + " " + keys[i]);
				ui._x -= 2;
				ui._y += 2;
			}

			drawTool(ToolBrush);
			drawTool(ToolEraser);
			drawTool(ToolFill);
			drawTool(ToolDecal);
			drawTool(ToolText);
			drawTool(ToolClone);
			drawTool(ToolBlur);
			drawTool(ToolSmudge);
			drawTool(ToolParticle);
			drawTool(ToolColorId);
			drawTool(ToolPicker);
			drawTool(ToolBake);
			drawTool(ToolMaterial);

			#if is_forge
			drawTool(ToolGizmo);
			#end

			#end

			ui.imageScrollAlign = true;
		}

		if (Config.raw.touch_ui) {
			// Hide scrollbar
			var _SCROLL_W = ui.t.SCROLL_W;
			ui.t.SCROLL_W = 0;
			ui.endWindow();
			ui.t.SCROLL_W = _SCROLL_W;
		}
	}

	static function toolPropertiesMenu() {
		var ui = UIBase.inst.ui;
		var _x = ui._x;
		var _y = ui._y;
		var _w = ui._w;
		UIMenu.draw(function(ui: Zui) {
			var startY = ui._y;
			ui.changed = false;

			UIHeader.inst.drawToolProperties(ui);

			if (ui.changed) {
				UIMenu.keepOpen = true;
			}

			if (ui.button(tr("Pin to Header"), Left)) {
				Config.raw.layout[LayoutHeader] = 1;
			}

			var h = ui._y - startY;
			UIMenu.menuElements = Std.int(h / ui.ELEMENT_H());
			UIMenu.menuX = Std.int(_x + _w + 2);
			UIMenu.menuY = Std.int(_y - 6 * ui.SCALE());
			UIMenu.fitToScreen();

		}, 0);

		// First draw out of screen, then align the menu based on menu height
		UIMenu.menuX = -kha.System.windowWidth();
		UIMenu.menuY = -kha.System.windowHeight();
	}

	@:access(zui.Zui)
	static function drawHighlight() {
		var ui = UIBase.inst.ui;
		var size = UIToolbar.inst.toolbarw - 4;
		ui.g.color = ui.t.HIGHLIGHT_COL;
		ui.drawRect(ui.g, true, ui._x + -1,  ui._y + 2, size + 2, size + 2);
	}
}

#end
