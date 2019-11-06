package arm.ui;

import zui.Zui;
import zui.Id;
import iron.system.Input;
import arm.util.Path;
using StringTools;

class UIFiles {

	public static var show = false;
	public static var isSave = false;
	public static var filename = "untitled";
	public static var filesDone:String->Void;
	public static var filters = "";
	public static var path = '/';

	public static function render(g:kha.graphics2.Graphics) {
		show = false;

		if (!UITrait.inst.nativeBrowser) {
			renderCustom(g);
			return;
		}

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

	@:access(zui.Zui)
	static function renderCustom(g:kha.graphics2.Graphics) {
		UIBox.showCustom(function(ui:Zui) {
			if (ui.tab(Id.handle(), "File Browser")) {
				var pathHandle = Id.handle();
				var fileHandle = Id.handle();
				ui.row([6/10, 2/10, 2/10]);
				filename = ui.textInput(fileHandle, "File");
				ui.text('*.' + filters, Center);
				var known = Path.checkTextureFormat(path) || Path.checkMeshFormat(path) || Path.checkProjectFormat(path);
				if (ui.button(isSave ? "Save" : "Open") || known) {
					UIBox.show = false;
					#if krom_windows
					var sep = "\\";
					#else
					var sep = "/";
					#end
					filesDone((known || isSave) ? path : path + sep + filename);
					if (known) pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(sep));
				}
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
