package arm.ui;

import zui.Zui;
import zui.Id;
import arm.io.ImportArm;

#if arm_touchui

class BoxProjects {

	public static var htab = Id.handle();

	static var iconMap: Map<String, kha.Image> = null;

	@:access(zui.Zui)
	public static function show() {

		if (iconMap != null) {
			for (handle in iconMap.keys()) iron.data.Data.deleteImage(handle);
			iconMap = null;
		}

		UIBox.showCustom(function(ui: Zui) {
			alignToFullScreen();

			if (ui.tab(htab, tr("Projects"), true)) {
				ui.beginSticky();
				if (ui.button(tr("New"))) {
					Project.projectNew();
					Viewport.scaleToBounds();
					UIBox.show = false;
					App.redrawUI();
					// Pick unique name
					var i = 0;
					var j = 0;
					var title = tr("untitled") + i;
					while (j < Config.raw.recent_projects.length) {
						var base = Config.raw.recent_projects[j];
						base = base.substring(base.lastIndexOf(arm.sys.Path.sep) + 1, base.lastIndexOf("."));
						j++;
						if (title == base) {
							i++;
							title = tr("untitled") + i;
							j = 0;
						}
					}
					kha.Window.get(0).title = title;
				}
				ui.endSticky();
				ui.separator(3, false);

				var slotw = Std.int(150 * ui.SCALE());
				var num = Std.int(kha.System.windowWidth() / slotw);
				var recent_projects = Config.raw.recent_projects;
				var show_asset_names = true;

				for (row in 0...Std.int(Math.ceil(recent_projects.length / num))) {
					var mult = show_asset_names ? 2 : 1;
					ui.row([for (i in 0...num * mult) 1 / num]);

					ui._x += 2;
					var off = show_asset_names ? ui.ELEMENT_OFFSET() * 16.0 : 6;
					if (row > 0) ui._y += off;

					for (j in 0...num) {
						var imgw = Std.int(128 * ui.SCALE());
						var i = j + row * num;
						if (i >= recent_projects.length) {
							@:privateAccess ui.endElement(imgw);
							if (show_asset_names) @:privateAccess ui.endElement(0);
							continue;
						}

						var path = recent_projects[i];

						#if krom_ios
						var documentDirectory = Krom.saveDialog("", "");
						documentDirectory = documentDirectory.substr(0, documentDirectory.length - 8); // Strip /'untitled'
						path = documentDirectory + path;
						#end

						var iconPath = path.substr(0, path.length - 4) + "_icon.png";
						if (iconMap == null) iconMap = [];
						var icon = iconMap.get(iconPath);
						if (icon == null) {
							iron.data.Data.getImage(iconPath, function(image: kha.Image) {
								icon = image;
								iconMap.set(iconPath, icon);
							});
						}

						var uix = ui._x;
						if (icon != null) {
							ui.fill(0, 0, 128, 128, ui.t.SEPARATOR_COL);

							var state = ui.image(icon, 0xffffffff, 128  * ui.SCALE());
							if (state == Released) {
								var _uix = ui._x;
								ui._x = uix;
								ui.fill(0, 0, 128, 128, 0x66000000);
								ui._x = _uix;
								function doImport() {
									iron.App.notifyOnInit(function() {
										UIBox.show = false;
										ImportArm.runProject(path);
									});
								}

								#if (krom_android || krom_ios)
								arm.App.notifyOnNextFrame(function() {
									Console.toast(tr("Opening project"));
									arm.App.notifyOnNextFrame(doImport);
								});
								#else
								doImport();
								#end
							}

							var name = path.substring(path.lastIndexOf(arm.sys.Path.sep) + 1, path.lastIndexOf("."));
							if (ui.isHovered && ui.inputReleasedR) {
								UIMenu.draw(function(ui: Zui) {
									ui.text(name, Right, ui.t.HIGHLIGHT_COL);
									// if (ui.button(tr("Duplicate"), Left)) {}
									if (ui.button(tr("Delete"), Left)) {
										iron.App.notifyOnInit(function() {
											arm.sys.File.delete(path);
											arm.sys.File.delete(iconPath);
											var dataPath = path.substr(0, path.length - 4);
											arm.sys.File.delete(dataPath);
											recent_projects.splice(i, 1);
										});
									}
								}, 2);
							}

							if (show_asset_names) {
								ui._x = uix - (150 - 128) / 2;
								ui._y += slotw * 0.9;
								ui.text(name, Center);
								if (ui.isHovered) ui.tooltip(name);
								ui._y -= slotw * 0.9;
								if (i == recent_projects.length - 1) {
									ui._y += j == num - 1 ? imgw : imgw + ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
								}
							}
						}
						else {
							@:privateAccess ui.endElement(0);
							if (show_asset_names) @:privateAccess ui.endElement(0);
							ui._x = uix;
						}
					}

					ui._y += 150;
				}
			}
		}, 600, 400, null, false);
	}

	static function alignToFullScreen() {
		@:privateAccess UIBox.modalW = Std.int(kha.System.windowWidth() / App.uiBox.SCALE());
		@:privateAccess UIBox.modalH = Std.int(kha.System.windowHeight() / App.uiBox.SCALE());
		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var mw = appw;
		var mh = apph;
		UIBox.hwnd.dragX = Std.int(-appw / 2 + mw / 2);
		UIBox.hwnd.dragY = Std.int(-apph / 2 + mh / 2);
	}
}

#end
