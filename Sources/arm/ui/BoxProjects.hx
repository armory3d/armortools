package arm.ui;

import zui.Zui;
import zui.Id;
import arm.io.ImportArm;

#if arm_touchui

class BoxProjects {

	public static var htab = Id.handle();

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
					if (ui.button("")) {
						ImportArm.runProject(path);
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
