package arm.ui;

import zui.Zui;
import iron.data.Data;
import iron.system.Time;
import iron.system.Input;
import arm.io.Importer;

class TabTextures {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Textures")) {
			ui.row([1/4, 1/4]);

			if (ui.button("Import")) {
				UIFiles.show = true;
				UIFiles.isSave = false;
				UIFiles.filters = "jpg,png,tga,bmp,psd,gif,hdr";
				UIFiles.filesDone = function(path:String) {
					Importer.importFile(path);
				}
			}
			if (ui.isHovered) ui.tooltip("Import texture file (Ctrl + Shift + I)");

			if (ui.button("2D View")) UITrait.inst.show2DView(1);

			if (Project.assets.length > 0) {

				var slotw = Std.int(51 * ui.SCALE);
				var num = Std.int(UITrait.inst.windowW / slotw);

				for (i in 0...Project.assets.length) {
					
					// Align into rows
					if (i % num == 0) {
						ui._y += ui.ELEMENT_OFFSET() * 1.5;
						ui.row([for (i in 0...num) 1/num]);
					}
					
					var asset = Project.assets[i];
					if (asset == Context.texture) {
						var off = i % 2 == 1 ? 1 : 0;
						var w = 51 - Config.raw.window_scale;
						ui.fill(1,          -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(1,     w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(1,          -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 3,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					var img = UITrait.inst.getImage(asset);
					var uix = ui._x;
					var uiy = ui._y;
					if (ui.image(img) == State.Started) {
						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX + iron.App.x() - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + iron.App.y() + 1);
						App.dragAsset = asset;
						Context.texture = asset;

						if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView(1);
						UITrait.inst.selectTime = Time.time();
					}

					if (ui.isHovered) ui.tooltipImage(img, 256);

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui:Zui) {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 3, ui.t.SEPARATOR_COL);
							ui.text(asset.name, Right);
							if (ui.button("To Mask", Left)) {
								Layers.createImageMask(asset);
							}
							if (ui.button("Delete", Left)) {
								UITrait.inst.hwnd2.redraws = 2;
								Data.deleteImage(asset.file);
								Project.assetMap.remove(asset.id);
								Project.assets.splice(i, 1);
								Project.assetNames.splice(i, 1);
								// TODO: rebuild affected materials
							}
						});
					}
				}

				// Fill in unused row space
				if (Project.assets.length % num > 0) {
					for (i in 0...num - (Project.assets.length % num)) {
						@:privateAccess ui.endElement(ui._w);
					}
				}
			}
			else {
				var img = Res.get('icons.png');
				var imgw = ui.SCALE > 1 ? 100 : 50;
				ui.image(img, ui.t.BUTTON_COL, imgw, 0, imgw, imgw, imgw);
				if (ui.isHovered) ui.tooltip("Drag and drop files here");
			}
		}
	}
}
