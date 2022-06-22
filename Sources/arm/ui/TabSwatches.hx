package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Time;
import iron.system.Input;
import arm.Enums;
import arm.ProjectFormat;

class TabSwatches {

	public static var empty(get, default): kha.Image = null;

	public static function get_empty() {
		if (empty == null) {
			var b = haxe.io.Bytes.alloc(4);
			b.set(0, 255);
			b.set(1, 255);
			b.set(2, 255);
			b.set(3, 255);
			empty = kha.Image.fromBytes(b, 1, 1);
		}
		return empty;
	}

	static var dragPosition: Int = -1;

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
			var dragPositionSet = false;

			var uix = 0.0;
			var uiy = 0.0;
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

					uix = ui._x;
					uiy = ui._y;

					// Draw the drag position indicator
					if (App.dragSwatch != null && dragPosition == i) {
						ui.fill(-1, -2 , 2, 32, ui.t.HIGHLIGHT_COL);
					}

					var state = ui.image(empty, Project.raw.swatches[i].base, slotw);

					if (state == State.Started) {
						Context.setSwatch(Project.raw.swatches[i]);

						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX - 2 * slotw);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						App.dragSwatch = Context.swatch;
					}
					else if (state == State.Hovered) {
						var mouse = Input.getMouse();
						dragPosition = (mouse.x > uix + ui._windowX + slotw / 2) ? i + 1 : i; // Switch to the next position if the mouse crosses the swatch rectangle center
						dragPositionSet = true;
					}
					else if (state == State.Released) {
						if (Time.time() - Context.selectTime < 0.25) {
							UIMenu.draw(function(ui) {
								ui.changed = false;
								var h = Id.handle();
								h.color = Context.swatch.base;

								Context.swatch.base = zui.Ext.colorWheel(ui, h, false, null, 11 * ui.t.ELEMENT_H * ui.SCALE(), true, function () {
									Context.colorPickerPreviousTool = Context.tool;
									Context.selectTool(ToolPicker);
									Context.colorPickerCallback = function (color: TSwatchColor) {
										Project.raw.swatches[i] = Project.cloneSwatch(color);
									};
								});
								var hopacity = Id.handle();
								hopacity.value = Context.swatch.opacity;
								Context.swatch.opacity = ui.slider(hopacity, "Opacity", 0, 1, true);
								var hocclusion = Id.handle();
								hocclusion.value = Context.swatch.occlusion;
								Context.swatch.occlusion = ui.slider(hocclusion, "Occlusion", 0, 1, true);
								var hroughness = Id.handle();
								hroughness.value = Context.swatch.roughness;
								Context.swatch.roughness = ui.slider(hroughness, "Roughness", 0, 1, true);
								var hmetallic = Id.handle();
								hmetallic.value = Context.swatch.metallic;
								Context.swatch.metallic = ui.slider(hmetallic, "Metallic", 0, 1, true);
								var hheight = Id.handle();
								hheight.value = Context.swatch.height;
								Context.swatch.height = ui.slider(hheight, "Height", 0, 1, true);

								if (ui.changed || ui.isTyping) UIMenu.keepOpen = true;
								if (ui.inputReleased) Context.setSwatch(Context.swatch); // Trigger material preview update
							}, 16, Std.int(Input.getMouse().x - 200 * ui.SCALE()), Std.int(Input.getMouse().y - 250 * ui.SCALE()));
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
								Context.setSwatch(Project.cloneSwatch(Context.swatch));
								Project.raw.swatches.push(Context.swatch);
							}
							#if (krom_windows || krom_linux || krom_darwin)
							else if (ui.button(tr("Copy Hex Code"), Left)) {
								var color = Context.swatch.base;
								color.A = Context.swatch.opacity;
								var val = untyped color;
								if (val < 0) val += untyped 4294967296;
								Krom.copyToClipboard(untyped val.toString(16));
							}
							#end
							else if (Project.raw.swatches.length > 1 && ui.button(tr("Delete"), Left, "delete")) {
								deleteSwatch(Project.raw.swatches[i]);
							}
							else if (ui.button(tr("Create Material"), Left)) {
								TabMaterials.acceptSwatchDrag(Project.raw.swatches[i]);
							}
							else if (ui.button(tr("Create Color Layer"), Left)) {
								var color = Project.raw.swatches[i].base;
								color.A = Project.raw.swatches[i].opacity;
			
								Layers.createColorLayer(color.value, Project.raw.swatches[i].occlusion, Project.raw.swatches[i].roughness, Project.raw.swatches[i].metallic);
							}
						}, 4 + add);
					}
					if (ui.isHovered) {
						var color = Project.raw.swatches[i].base;
						color.A = Project.raw.swatches[i].opacity;
						var val = untyped color;
						if (val < 0) val += untyped 4294967296;
						ui.tooltip("#" + untyped val.toString(16));
					}
				}
			}

			// Draw the rightmost line next to the last swatch
			if (App.dragSwatch != null && dragPosition == Project.raw.swatches.length) {
				ui._x = uix; // Reset the position because otherwise it would start in the row below
				ui._y = uiy;
				ui.fill(28, -2, 2, 32, ui.t.HIGHLIGHT_COL);
			}

			// Currently there is no valid dragPosition so reset it
			if (!dragPositionSet)
				dragPosition = -1;

			var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && Project.raw.swatches.length > 1) {
				ui.isDeleteDown = false;
				deleteSwatch(Context.swatch);
			}
		}
	}

	public static function acceptSwatchDrag(swatch: TSwatchColor) {
		// No valid position available
		if (TabSwatches.dragPosition == -1) return;

		var swatchPosition = Project.raw.swatches.indexOf(swatch);
		// A new swatch from color picker.
		if (swatchPosition == -1) { 
			Project.raw.swatches.insert(dragPosition, swatch);
		}
		else if (Math.abs(swatchPosition - dragPosition) > 0) { // Existing swatch is reordered
			Project.raw.swatches.remove(swatch);
			// If the new position is after the old one, decrease by one because the swatch has been deleted.
			var newPosition = dragPosition - swatchPosition > 0 ? dragPosition -1 : dragPosition;
			Project.raw.swatches.insert(newPosition, swatch);
		}
	}

	static function deleteSwatch(swatch: TSwatchColor) {
		var i = Project.raw.swatches.indexOf(swatch);
		Context.setSwatch(Project.raw.swatches[i == Project.raw.swatches.length - 1 ? i - 1 : i + 1]);
		Project.raw.swatches.splice(i, 1);
		UIStatus.inst.statusHandle.redraws = 2;
	}
}
