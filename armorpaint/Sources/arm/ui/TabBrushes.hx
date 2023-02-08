package arm.ui;

import haxe.Json;
import iron.system.Time;
import zui.Zui;
import arm.data.BrushSlot;
import arm.node.MakeMaterial;
import arm.util.RenderUtil;
import arm.sys.Path;
import arm.Enums;

class TabBrushes {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab1, tr("Brushes"))) {
			ui.beginSticky();
			ui.row([1 / 4, 1 / 4, 1 / 4]);
			if (ui.button(tr("New"))) {
				Context.brush = new BrushSlot();
				Project.brushes.push(Context.brush);
				MakeMaterial.parseBrush();
				UINodes.inst.hwnd.redraws = 2;
			}
			if (ui.button(tr("Import"))) {
				Project.importBrush();
			}
			if (ui.button(tr("Nodes"))) {
				UISidebar.inst.showBrushNodes();
			}
			ui.endSticky();
			ui.separator(3, false);

			var slotw = Std.int(51 * ui.SCALE());
			var num = Std.int(Config.raw.layout[LayoutSidebarW] / slotw);

			for (row in 0...Std.int(Math.ceil(Project.brushes.length / num))) {
				var mult = Config.raw.show_asset_names ? 2 : 1;
				ui.row([for (i in 0...num * mult) 1 / num]);

				ui._x += 2;
				var off = Config.raw.show_asset_names ? ui.ELEMENT_OFFSET() * 10.0 : 6;
				if (row > 0) ui._y += off;

				for (j in 0...num) {
					var imgw = Std.int(50 * ui.SCALE());
					var i = j + row * num;
					if (i >= Project.brushes.length) {
						@:privateAccess ui.endElement(imgw);
						if (Config.raw.show_asset_names) @:privateAccess ui.endElement(0);
						continue;
					}
					var img = ui.SCALE() > 1 ? Project.brushes[i].image : Project.brushes[i].imageIcon;
					var imgFull = Project.brushes[i].image;

					if (Context.brush == Project.brushes[i]) {
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
					//var uiy = ui._y;
					var tile = ui.SCALE() > 1 ? 100 : 50;
					var state = Project.brushes[i].previewReady ? ui.image(img) : ui.image(Res.get("icons.k"), -1, null, tile * 5, tile, tile, tile);
					if (state == State.Started) {
						if (Context.brush != Project.brushes[i]) Context.selectBrush(i);
						if (Time.time() - Context.selectTime < 0.25) UISidebar.inst.showBrushNodes();
						Context.selectTime = Time.time();
						// var mouse = Input.getMouse();
						// App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						// App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						// App.dragBrush = Context.brush;
					}
					if (ui.isHovered && ui.inputReleasedR) {
						Context.selectBrush(i);
						var add = Project.brushes.length > 1 ? 1 : 0;
						UIMenu.draw(function(ui: Zui) {
							//var b = Project.brushes[i];
							ui.text(Project.brushes[i].canvas.name, Right, ui.t.HIGHLIGHT_COL);

							if (ui.button(tr("Export"), Left)) {
								Context.selectBrush(i);
								BoxExport.showBrush();
							}

							if (ui.button(tr("Duplicate"), Left)) {
								function _init() {
									Context.brush = new BrushSlot();
									Project.brushes.push(Context.brush);
									var cloned = Json.parse(Json.stringify(Project.brushes[i].canvas));
									Context.brush.canvas = cloned;
									Context.setBrush(Context.brush);
									RenderUtil.makeBrushPreview();
								}
								iron.App.notifyOnInit(_init);
							}

							if (Project.brushes.length > 1 && ui.button(tr("Delete"), Left, "delete")) {
								deleteBrush(Project.brushes[i]);
							}
						}, 3 + add);
					}

					if (ui.isHovered) {
						if (imgFull == null) {
							iron.App.notifyOnInit(function() {
								var _brush = Context.brush;
								Context.brush = Project.brushes[i];
								MakeMaterial.parseBrush();
								RenderUtil.makeBrushPreview();
								Context.brush = _brush;
							});
						}
						else {
							ui.tooltipImage(imgFull);
							ui.tooltip(Project.brushes[i].canvas.name);
						}
					}

					if (Config.raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						ui.text(Project.brushes[i].canvas.name, Center);
						if (ui.isHovered) ui.tooltip(Project.brushes[i].canvas.name);
						ui._y -= slotw * 0.9;
						if (i == Project.brushes.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
						}
					}
				}

				ui._y += 6;
			}

			var inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && Project.brushes.length > 1) {
				ui.isDeleteDown = false;
				deleteBrush(Context.brush);
			}
		}
	}

	static function deleteBrush(b: BrushSlot) {
		var i = Project.brushes.indexOf(b);
		Context.selectBrush(i == Project.brushes.length - 1 ? i - 1 : i + 1);
		Project.brushes.splice(i, 1);
		UISidebar.inst.hwnd1.redraws = 2;
	}
}
