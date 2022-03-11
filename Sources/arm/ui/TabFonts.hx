package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Time;
import arm.io.ImportFont;
import arm.data.FontSlot;
import arm.util.RenderUtil;
import arm.Enums;

class TabFonts {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(UIStatus.inst.statustab, tr("Fonts")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			#if arm_touchui
			ui.row([1 / 4, 1 / 4]);
			#else
			ui.row([1 / 14, 1 / 14]);
			#end

			if (ui.button(tr("Import"))) Project.importAsset("ttf,ttc,otf");
			if (ui.isHovered) ui.tooltip(tr("Import font file"));

			if (ui.button(tr("2D View"))) {
				UISidebar.inst.show2DView(View2DFont);
			}
			ui.endSticky();
			ui.separator(3, false);

			var statusw = kha.System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW];
			var slotw = Std.int(51 * ui.SCALE());
			var num = Std.int(statusw / slotw);

			for (row in 0...Std.int(Math.ceil(Project.fonts.length / num))) {
				var mult = Config.raw.show_asset_names ? 2 : 1;
				ui.row([for (i in 0...num * mult) 1 / num]);

				ui._x += 2;
				var off = Config.raw.show_asset_names ? ui.ELEMENT_OFFSET() * 10.0 : 6;
				if (row > 0) ui._y += off;

				for (j in 0...num) {
					var imgw = Std.int(50 * ui.SCALE());
					var i = j + row * num;
					if (i >= Project.fonts.length) {
						@:privateAccess ui.endElement(imgw);
						if (Config.raw.show_asset_names) @:privateAccess ui.endElement(0);
						continue;
					}
					var img = Project.fonts[i].image;

					if (Context.font == Project.fonts[i]) {
						// ui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						var off = row % 2 == 1 ? 1 : 0;
						var w = 50;
						if (Config.raw.window_scale > 1) w += Std.int(Config.raw.window_scale * 2);
						ui.fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					var uix = ui._x;
					var tile = ui.SCALE() > 1 ? 100 : 50;
					var state = State.Idle;
					if (Project.fonts[i].previewReady) {
						// ui.g.pipeline = UIView2D.inst.pipe; // L8
						// #if kha_opengl
						// ui.currentWindow.texture.g4.setPipeline(UIView2D.inst.pipe);
						// #end
						// ui.currentWindow.texture.g4.setInt(UIView2D.inst.channelLocation, 1);
						state = ui.image(img);
						// ui.g.pipeline = null;
					}
					else {
						state = ui.image(Res.get("icons.k"), -1, null, tile * 6, tile, tile, tile);
					}

					if (state == State.Started) {
						if (Context.font != Project.fonts[i]) {
							function _init() {
								Context.selectFont(i);
							}
							iron.App.notifyOnInit(_init);
						}
						if (Time.time() - Context.selectTime < 0.25) UISidebar.inst.show2DView(View2DFont);
						Context.selectTime = Time.time();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						Context.selectFont(i);
						var add = Project.fonts.length > 1 ? 1 : 0;
						var fontName = Project.fonts[i].name;
						UIMenu.draw(function(ui: Zui) {
							ui.text(fontName, Right, ui.t.HIGHLIGHT_COL);

							if (Project.fonts.length > 1 && ui.button(tr("Delete"), Left, "delete") && Project.fonts[i].file != "") {
								deleteFont(Project.fonts[i]);
							}
						}, 1 + add);
					}
					if (ui.isHovered) {
						if (img == null) {
							iron.App.notifyOnInit(function() {
								var _font = Context.font;
								Context.font = Project.fonts[i];
								RenderUtil.makeFontPreview();
								Context.font = _font;
							});
						}
						else {
							ui.tooltipImage(img);
							ui.tooltip(Project.fonts[i].name);
						}
					}

					if (Config.raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						ui.text(Project.fonts[i].name, Center);
						if (ui.isHovered) ui.tooltip(Project.fonts[i].name);
						ui._y -= slotw * 0.9;
						if (i == Project.fonts.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
						}
					}
				}

				ui._y += 6;
			}

			var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && Project.fonts.length > 1 && Context.font.file != "") {
				ui.isDeleteDown = false;
				deleteFont(Context.font);
			}
		}
	}

	static function deleteFont(font: FontSlot) {
		var i = Project.fonts.indexOf(font);
		function _init() {
			Context.selectFont(i == Project.fonts.length - 1 ? i - 1 : i + 1);
			iron.data.Data.deleteFont(Project.fonts[i].file);
			Project.fonts.splice(i, 1);
		}
		iron.App.notifyOnInit(_init);
		UIStatus.inst.statusHandle.redraws = 2;
	}
}
