package arm.ui;

import iron.system.Time;
import zui.Zui;
import arm.data.BrushSlot;

class TabBrushes {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab1, "Brushes")) {
			ui.row([1 / 4, 1 / 4]);
			if (ui.button("New")) {
				// UITrait.inst.headerHandle.redraws = 2;
				Context.brush = new BrushSlot();
				Project.brushes.push(Context.brush);
				// MaterialParser.parsePaintMaterial();
				// RenderUtil.makeMaterialPreview();
			}
			if (ui.button("Nodes")) UITrait.inst.showBrushNodes();

			var slotw = Std.int(51 * ui.SCALE());
			var num = Std.int(UITrait.inst.windowW / slotw);

			for (row in 0...Std.int(Math.ceil(Project.brushes.length / num))) {
				ui.row([for (i in 0...num) 1 / num]);

				ui._x += 2;
				if (row > 0) ui._y += 6;

				for (j in 0...num) {
					var imgw = Std.int(50 * ui.SCALE());
					var i = j + row * num;
					if (i >= Project.brushes.length) {
						@:privateAccess ui.endElement(imgw);
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

					#if (kha_opengl || kha_webgl)
					ui.imageInvertY = Project.brushes[i].previewReady;
					#end

					//var uix = ui._x;
					//var uiy = ui._y;
					var tile = ui.SCALE() > 1 ? 100 : 50;
					var state = Project.brushes[i].previewReady ? ui.image(img) : ui.image(Res.get("icons.k"), -1, null, tile, tile, tile, tile);
					if (state == State.Started) {
						if (Context.brush != Project.brushes[i]) Context.selectBrush(i);
						if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.showBrushNodes();
						UITrait.inst.selectTime = Time.time();
						// var mouse = Input.getMouse();
						// App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						// App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						// App.dragBrush = Context.brush;
					}
					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui: Zui) {
							//var b = Project.brushes[i];
							ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 2, ui.t.SEPARATOR_COL);
							ui.text(Project.brushes[i].canvas.name, Right, ui.t.HIGHLIGHT_COL);

							if (ui.button("Delete", Left) && Project.brushes.length > 1) {
								Context.selectBrush(i == 0 ? 1 : 0);
								Project.brushes.splice(i, 1);
								UITrait.inst.hwnd1.redraws = 2;
							}
						});
					}
					if (ui.isHovered) ui.tooltipImage(imgFull);
				}

				ui._y += 6;

				#if (kha_opengl || kha_webgl)
				ui.imageInvertY = false; // Material preview
				#end
			}
		}
	}
}
