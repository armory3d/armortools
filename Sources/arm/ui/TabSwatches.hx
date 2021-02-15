package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Time;
import iron.system.Input;
import arm.io.ImportFont;
import arm.util.RenderUtil;
import arm.Enums;

class TabSwatches {

	public static var empty: kha.Image = null;

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab2, tr("Swatches"))) {

			ui.beginSticky();
			ui.row([1 / 4, 1 / 4, 1 / 4]);

			if (ui.button(tr("New"))) {
				Context.setSwatch(Project.makeSwatch());
				Project.raw.swatches.push(Context.swatch);
			}
			if (ui.button(tr("Import"))) Project.importSwatches();
			if (ui.isHovered) ui.tooltip(tr("Import swatches"));
			if (ui.button(tr("Tools..."))) {
				UIMenu.draw(function(ui: Zui) {
					ui.text(tr("Tools"), Right, ui.t.HIGHLIGHT_COL);
					if (ui.button(tr("Clear"), Left)) {
						Context.setSwatch(Project.makeSwatch());
						Project.raw.swatches = [Context.swatch];
					}
					if (ui.button(tr("Restore"), Left)) {
						Project.setDefaultSwatches();
						Context.setSwatch(Project.raw.swatches[0]);
					}
				}, 3);
			}

			ui.endSticky();
			ui.separator(3, false);

			var slotw = Std.int(26 * ui.SCALE());
			var num = Std.int(Config.raw.layout[LayoutSidebarW] / (slotw + 3));

			for (row in 0...Std.int(Math.ceil(Project.raw.swatches.length / num))) {
				ui.row([for (i in 0...num) 1 / num]);

				ui._x += 2;
				if (row > 0) ui._y += 6;

				for (j in 0...num) {
					var i = j + row * num;
					if (i >= Project.raw.swatches.length) {
						@:privateAccess ui.endElement(slotw);
						continue;
					}

					if (Context.swatch == Project.raw.swatches[i]) {
						var off = row % 2 == 1 ? 1 : 0;
						var w = 32;
						ui.fill(-2, -2, w, w, ui.t.HIGHLIGHT_COL);
					}

					if (empty == null) {
						var b = haxe.io.Bytes.alloc(4);
						b.set(0, 255);
						b.set(1, 255);
						b.set(2, 255);
						b.set(3, 255);
						empty = kha.Image.fromBytes(b, 1, 1);
					}

					var uix = ui._x;
					var uiy = ui._y;
					var state = ui.image(empty, Project.raw.swatches[i].base, slotw);

					if (state == State.Started) {
						Context.setSwatch(Project.raw.swatches[i]);

						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						App.dragSwatch = Context.swatch;
					}
					else if (state == State.Released) {
						if (Time.time() - Context.selectTime < 0.25) {

							UIMenu.draw(function(ui) {
								ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
								ui.changed = false;
								var h = Id.handle();
								h.color = Context.swatch.base;
								Context.swatch.base = zui.Ext.colorWheel(ui, h, false, null, false);
								if (ui.changed) UIMenu.keepOpen = true;
								if (ui.inputReleased) Context.setSwatch(Context.swatch); // Trigger material preview update
							}, 3, Std.int(Input.getMouse().x - 200 * ui.SCALE()), Std.int(Input.getMouse().y - 250 * ui.SCALE()));
						}

						Context.selectTime = Time.time();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						var add = Project.raw.swatches.length > 1 ? 1 : 0;
						UIMenu.draw(function(ui: Zui) {
							ui.text(tr("Swatch"), Right, ui.t.HIGHLIGHT_COL);
							if (Project.raw.swatches.length > 1 && ui.button(tr("Delete"), Left)) {
								Context.setSwatch(Project.raw.swatches[i == 0 ? 1 : 0]);
								Project.raw.swatches.splice(i, 1);
								UISidebar.inst.hwnd2.redraws = 2;
							}
						}, 1 + add);
					}
					if (ui.isHovered) {
						var val = untyped Project.raw.swatches[i].base;
						if (val < 0) val += untyped 4294967296;
						ui.tooltip("#" + untyped val.toString(16));
					}
				}
			}
		}
	}
}
