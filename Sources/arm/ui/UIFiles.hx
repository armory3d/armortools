package arm.ui;

import zui.Id;
import iron.system.Input;
import arm.App;

class UIFiles {

	public static var filters = "";

	@:access(zui.Zui)
	public static function render(g:kha.graphics2.Graphics) {

		// Krom with native file dialogs
		#if kha_krom
		if (untyped Krom.openDialog != null) {
			App.showFiles = false;
			App.path = untyped App.foldersOnly ? Krom.saveDialog(filters, "") : Krom.openDialog(filters, "");
			if (App.path != null) {
				if (!App.checkAscii(App.path)) return;
				App.path = StringTools.replace(App.path, "\\\\", "\\");
				App.path = StringTools.replace(App.path, "\r", "");
				#if krom_windows
				var sep = "\\";
				#else
				var sep = "/";
				#end
				App.filenameHandle.text = App.path.substr(App.path.lastIndexOf(sep) + 1);
				if (App.foldersOnly) App.path = App.path.substr(0, App.path.lastIndexOf(sep));
				App.filesDone(App.path);
			}
			releaseKeys();
		}
		#end
	}

	static function releaseKeys() {
		// File dialog may prevent firing key up events
		var kb = Input.getKeyboard();
		@:privateAccess kb.upListener(kha.input.KeyCode.Shift);
		@:privateAccess kb.upListener(kha.input.KeyCode.Control);
	}
}
