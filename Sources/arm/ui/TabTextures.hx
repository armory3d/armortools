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
				App.showFiles = true;
				App.whandle.redraws = 2;
				App.foldersOnly = false;
				App.showFilename = false;
				UIFiles.filters = "jpg,png,tga,hdr";
				App.filesDone = function(path:String) {
					Importer.importFile(path);
				}
			}
			if (ui.isHovered) ui.tooltip("Import texture file (Ctrl + Shift + I)");

			if (ui.button("2D View")) UITrait.inst.show2DView(1);

			if (UITrait.inst.assets.length > 0) {
				for (i in 0...UITrait.inst.assets.length) {
					
					// Align into 5 items per row
					if (i % 5 == 0) {
						ui._y += ui.ELEMENT_OFFSET() * 1.5;
						ui.row([1/5, 1/5, 1/5, 1/5, 1/5]);
					}
					
					var asset = UITrait.inst.assets[i];
					if (asset == UITrait.inst.selectedTexture) {
						var off = i % 2 == 1 ? 1 : 0;
						var w = 51 - App.C.window_scale;
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
						UITrait.inst.selectedTexture = asset;

						if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView(1);
						UITrait.inst.selectTime = Time.time();
					}

					if (ui.isHovered) ui.tooltipImage(img, 256);

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.show(function(ui:Zui) {
							ui.fill(0, 0, ui._w, ui.t.ELEMENT_H * 3, ui.t.SEPARATOR_COL);
							ui.text(asset.name, Right);
							if (ui.button("To Mask", Left)) {
								UITrait.inst.createImageMask(asset);
							}
							if (ui.button("Delete", Left)) {
								UITrait.inst.hwnd2.redraws = 2;
								Data.deleteImage(asset.file);
								zui.Canvas.assetMap.remove(asset.id);
								UITrait.inst.assets.splice(i, 1);
								UITrait.inst.assetNames.splice(i, 1);
								// TODO: rebuild affected materials
							}
						});
					}
				}

				// Fill in unused row space
				if (UITrait.inst.assets.length % 5 > 0) {
					for (i in 0...5 - (UITrait.inst.assets.length % 5)) {
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
