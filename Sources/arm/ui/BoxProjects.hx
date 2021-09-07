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
			if (ui.tab(htab, tr("Projects"), true)) {

				ui.beginSticky();
				if (ui.button(tr("New"))) {
					Project.projectNew();
					Viewport.scaleToBounds();
					UIBox.show = false;
					App.redrawUI();
				}
				ui.endSticky();
				ui.separator(3, false);

				var slotw = Std.int(300 * ui.SCALE());
				var num = Std.int(kha.System.windowWidth() / slotw);
				var recent_projects = Config.raw.recent_projects;

				for (row in 0...Std.int(Math.ceil(recent_projects.length / num))) {
					ui.row([for (i in 0...num) 1 / num]);

					for (j in 0...num) {
						var i = j + row * num;
						if (i >= recent_projects.length) {
							var imgw = Std.int(256 * ui.SCALE());
							@:privateAccess ui.endElement(imgw);
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

						ui.fill(0, 0, 256, 256, ui.t.SEPARATOR_COL);

						if (icon != null) {
							var state = ui.image(icon);
							if (state == Released) {
								iron.App.notifyOnInit(function() {
									ImportArm.runProject(path);
								});
								UIBox.show = false;
							}
							if (ui.isHovered && ui.inputReleasedR) {
								UIMenu.draw(function(ui: Zui) {
									var name = path.substr(path.lastIndexOf("/") + 1);
									ui.text(name, Right, ui.t.HIGHLIGHT_COL);
									// if (ui.button(tr("Duplicate"), Left)) {}
									if (ui.button(tr("Delete"), Left)) {
										iron.App.notifyOnInit(function() {
											arm.sys.File.delete(path);
											arm.sys.File.delete(iconPath);
											recent_projects.splice(i, 1);
										});
									}
								}, 2);
							}
						}
					}

					ui._y += 32;
				}
			}
		}, 600, 400, null, false);

		#if arm_touchui
		@:privateAccess alignToFullScreen();
		#end
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
