package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Input;
import arm.sys.Path;
using StringTools;

class UIFiles {

	public static var filename = "untitled";
	public static var path = "/";

	public static function show(filters: String, isSave: Bool, filesDone: String->Void) {
		if (!UITrait.inst.nativeBrowser) {
			if (path == null) path = "/";
			showCustom(filters, isSave, filesDone);
			return;
		}

		path = isSave ? Krom.saveDialog(filters, "") : Krom.openDialog(filters, "");
		if (path != null) {
			#if krom_windows
			if (!Path.isAscii(path)) path = Path.shortPath(path);
			#end
			path = path.replace("\\\\", "\\");
			path = path.replace("\r", "");
			filename = path.substr(path.lastIndexOf(Path.sep) + 1);
			if (isSave) path = path.substr(0, path.lastIndexOf(Path.sep));
			filesDone(path);
		}
		releaseKeys();
	}

	@:access(zui.Zui) //
	static function showCustom(filters: String, isSave: Bool, filesDone: String->Void) {
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), "File Browser")) {
				var pathHandle = Id.handle();
				var fileHandle = Id.handle();
				ui.row([6 / 10, 2 / 10, 2 / 10]);
				filename = ui.textInput(fileHandle, "File");
				ui.text("*." + filters, Center);
				var known = Path.isTexture(path) || Path.isMesh(path) || Path.isProject(path);
				if (ui.button(isSave ? "Save" : "Open") || known || ui.isReturnDown) {
					UIBox.show = false;
					filesDone((known || isSave) ? path : path + Path.sep + filename);
					if (known) pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(Path.sep));
				}
				zui.Ext.dataPath = iron.data.Data.dataPath;
				path = zui.Ext.fileBrowser(ui, pathHandle, false);
				if (pathHandle.changed) ui.currentWindow.redraws = 3;
			}
		}, 600, 500);
	}

	static function releaseKeys() {
		// File dialog may prevent firing key up events
		var kb = Input.getKeyboard();
		@:privateAccess kb.upListener(kha.input.KeyCode.Shift);
		@:privateAccess kb.upListener(kha.input.KeyCode.Control);
	}
}
