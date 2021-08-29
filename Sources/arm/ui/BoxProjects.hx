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

				for (path in Config.raw.recent_projects) {

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

					var state = ui.image(icon);
					if (state == Released) {
						ImportArm.runProject(path);
						UIBox.show = false;
					}
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
