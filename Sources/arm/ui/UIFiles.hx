package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Input;
using StringTools;

class UIFiles {

	public static var show = false;
	public static var isSave = false;
	public static var filename = "untitled";
	public static var filesDone:String->Void;
	public static var filters = "";
	public static var path = '/';

	@:access(zui.Zui)
	public static function render(g:kha.graphics2.Graphics) {
		show = false;
		path = isSave ? Krom.saveDialog(filters, "") : Krom.openDialog(filters, "");
		if (path != null) {
			#if krom_windows
			if (!App.checkAscii(path)) path = App.shortPath(path);
			#end
			path = path.replace("\\\\", "\\");
			path = path.replace("\r", "");
			#if krom_windows
			var sep = "\\";
			#else
			var sep = "/";
			#end
			filename = path.substr(path.lastIndexOf(sep) + 1);
			if (isSave) path = path.substr(0, path.lastIndexOf(sep));
			filesDone(path);
		}
		releaseKeys();
	}

	static function releaseKeys() {
		// File dialog may prevent firing key up events
		var kb = Input.getKeyboard();
		@:privateAccess kb.upListener(kha.input.KeyCode.Shift);
		@:privateAccess kb.upListener(kha.input.KeyCode.Control);
	}
}
