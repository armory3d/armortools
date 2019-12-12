package arm.ui;

import zui.Zui;
import iron.data.Data;
import iron.system.Time;
import iron.system.Input;
import arm.io.ImportAsset;
import arm.sys.Path;
using StringTools;

class TabTextures {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Textures")) {
			ui.row([1 / 4, 1 / 4]);

			if (ui.button("Import")) {
				UIFiles.show(Path.textureFormats.join(","), false, function(path: String) {
					ImportAsset.run(path);
				});
			}
			if (ui.isHovered) ui.tooltip("Import texture file (" + Config.keymap.file_import_assets + ")");

			if (ui.button("2D View")) UITrait.inst.show2DView(1);

			if (Project.assets.length > 0) {

				var slotw = Std.int(51 * ui.SCALE());
				var num = Std.int(UITrait.inst.windowW / slotw);
				for (i in 0...Project.assets.length) {

					// Align into rows
					if (i % num == 0) {
						ui._y += ui.ELEMENT_OFFSET() * 1.5;
						ui.row([for (i in 0...num) 1 / num]);
					}
					else ui._x += 1;

					var asset = Project.assets[i];
					var img = UITrait.inst.getImage(asset);
					var uix = ui._x;
					var uiy = ui._y;
					var ih = img.height > img.width ? slotw : null;
					if (ui.image(img, 0xffffffff, ih) == State.Started && ui.inputY > ui._windowY) {
						var mouse = Input.getMouse();
						App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						App.dragAsset = asset;
						Context.texture = asset;

						if (Time.time() - UITrait.inst.selectTime < 0.25) UITrait.inst.show2DView(1);
						UITrait.inst.selectTime = Time.time();
					}

					var _uix = ui._x;
					var _uiy = ui._y;
					ui._x = uix;
					ui._y = uiy;
					if (asset == Context.texture) {
						var off = i % 2 == 1 ? 1 : 0;
						var w = 50;
						ui.fill(0,               0, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(0,     w - off + 2, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(0,               0,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 2,           0,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}
					ui._x = _uix;
					ui._y = _uiy;

					// End of row align
					if (i % num == num - 1) ui._y = uiy + slotw;

					if (ui.isHovered) ui.tooltipImage(img, 256);

					if (ui.isHovered && ui.inputReleasedR) {
						UIMenu.draw(function(ui: Zui) {
							ui.fill(0, 0, ui._w / ui.SCALE(), ui.t.ELEMENT_H * 4, ui.t.SEPARATOR_COL);
							ui.text(asset.name, Right, ui.t.HIGHLIGHT_COL);
							if (ui.button("Export", Left)) {
								UIFiles.show("png", true, function(path: String) {
									var target = kha.Image.createRenderTarget(img.width, img.height);
									function exportTexture(g: kha.graphics4.Graphics) {
										target.g2.begin(false);
										target.g2.drawImage(img, 0, 0);
										target.g2.end();
										var f = UIFiles.filename;
										if (f == "") f = "untitled";
										if (!f.endsWith(".png")) f += ".png";
										var out = new haxe.io.BytesOutput();
										var writer = new arm.format.PngWriter(out);
										var data = arm.format.PngTools.build32RGB1(target.width, target.height, target.getPixels());
										writer.write(data);
										Krom.fileSaveBytes(path + "/" + f, out.getBytes().getData());
										iron.App.removeRender(exportTexture);
									};
									iron.App.notifyOnRender(exportTexture);
								});
							}
							if (ui.button("To Mask", Left)) {
								Layers.createImageMask(asset);
							}
							if (ui.button("Delete", Left)) {
								UITrait.inst.hwnd2.redraws = 2;
								Data.deleteImage(asset.file);
								Project.assetMap.remove(asset.id);
								Project.assets.splice(i, 1);
								Project.assetNames.splice(i, 1);
								iron.system.Tween.timer(0.1, function() {
									arm.node.MaterialParser.parsePaintMaterial();
									arm.util.RenderUtil.makeMaterialPreview();
									UITrait.inst.hwnd1.redraws = 2;
								});
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
				var img = Res.get("icons.k");
				var imgw = ui.SCALE() > 1 ? 100 : 50;
				ui.image(img, ui.t.BUTTON_COL, imgw, 0, imgw, imgw, imgw);
				if (ui.isHovered) ui.tooltip("Drag and drop files here");
			}
		}
	}
}
