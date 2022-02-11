package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Time;
import iron.system.Input;
import arm.Enums;
import arm.ProjectFormat;

class TabSwatches {

	public static var empty: kha.Image = null;

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		var statush = Config.raw.layout[LayoutStatusH];
		if (ui.tab(UIStatus.inst.statustab, tr("Swatches")) && statush > UIStatus.defaultStatusH * ui.SCALE()) {

			ui.beginSticky();
			#if arm_touchui
			ui.row([1 / 5, 1 / 5, 1 / 5, 1 / 5, 1 / 5]);
			#else
			ui.row([1 / 14, 1 / 14, 1 / 14, 1 / 14, 1 / 14]);
			#end

			if (ui.button(tr("New"))) {
				Context.setSwatch(Project.makeSwatch());
				Project.raw.swatches.push(Context.swatch);
			}
			if (ui.isHovered) ui.tooltip(tr("Add new swatch"));

			if (ui.button(tr("Import"))) {
				UIMenu.draw(function(ui: Zui) {
					ui.text(tr("Import"), Right, ui.t.HIGHLIGHT_COL);
					if (ui.button(tr("Replace Existing"), Left)) {
						Project.importSwatches(true);
						Context.setSwatch(Project.raw.swatches[0]);
					}
					if (ui.button(tr("Append"), Left)) {
						Project.importSwatches(false);
					}
				}, 3);
			}	
			if (ui.isHovered) ui.tooltip(tr("Import swatches"));

			if (ui.button(tr("Export"))) Project.exportSwatches();
			if (ui.isHovered) ui.tooltip(tr("Export swatches"));

			if (ui.button(tr("Clear"))) {
				Context.setSwatch(Project.makeSwatch());
				Project.raw.swatches = [Context.swatch];
			}

			if (ui.button(tr("Restore"))) {
				Project.setDefaultSwatches();
				Context.setSwatch(Project.raw.swatches[0]);
			}
			if (ui.isHovered) ui.tooltip(tr("Restore default swatches"));

			ui.endSticky();
			ui.separator(3, false);

			var statusw = kha.System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW];
			var slotw = Std.int(26 * ui.SCALE());
			var num = Std.int(statusw / (slotw + 3));

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
								ui.changed = false;
								var h = Id.handle();
								h.color = Context.swatch.base;
								Context.swatch.base = zui.Ext.colorWheel(ui, h, false, null, true);
								if (ui.changed || ui.isTyping) UIMenu.keepOpen = true;
								if (ui.inputReleased) Context.setSwatch(Context.swatch); // Trigger material preview update
							}, 11, Std.int(Input.getMouse().x - 200 * ui.SCALE()), Std.int(Input.getMouse().y - 250 * ui.SCALE()));
						}

						Context.selectTime = Time.time();
					}
					if (ui.isHovered && ui.inputReleasedR) {
						Context.setSwatch(Project.raw.swatches[i]);
						var add = Project.raw.swatches.length > 1 ? 1 : 0;
						#if (krom_windows || krom_linux || krom_darwin)
						add += 1; // Copy
						#end

						UIMenu.draw(function(ui: Zui) {
							ui.text(tr("Swatch"), Right, ui.t.HIGHLIGHT_COL);
							if (ui.button(tr("Duplicate"), Left)) {
								Context.setSwatch(Project.makeSwatch(Context.swatch.base));
								Project.raw.swatches.push(Context.swatch);
							}
							#if (krom_windows || krom_linux || krom_darwin)
							else if (ui.button(tr("Copy"), Left)) {
								var val = untyped Context.swatch.base;
								if (val < 0) val += untyped 4294967296;
								Krom.copyToClipboard(untyped val.toString(16));
							}
							#end
							else if (Project.raw.swatches.length > 1 && ui.button(tr("Delete"), Left)) {
								deleteSwatch(Project.raw.swatches[i]);
							}
						}, 2 + add);
					}
					if (ui.isHovered) {
						var val = untyped Project.raw.swatches[i].base;
						if (val < 0) val += untyped 4294967296;
						ui.tooltip("#" + untyped val.toString(16));
					}
				}
			}

			var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && Project.raw.swatches.length > 1) {
				ui.isDeleteDown = false;
				deleteSwatch(Context.swatch);
			}
		}
	}

	static function deleteSwatch(swatch: TSwatchColor) {
		var i = Project.raw.swatches.indexOf(swatch);
		Context.setSwatch(Project.raw.swatches[i == Project.raw.swatches.length - 1 ? i - 1 : i + 1]);
		Project.raw.swatches.splice(i, 1);
		UIStatus.inst.statusHandle.redraws = 2;
	}
}
