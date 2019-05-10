package arm.ui;

import zui.Id;
import arm.App;

class UIFiles {

	@:access(zui.Zui)
	public static function render(g:kha.graphics2.Graphics) {

		var uibox = App.uibox;

		// Krom with native file dialogs
		#if kha_krom
		if (untyped Krom.openDialog != null) {
			App.showFiles = false;
			App.path = untyped App.foldersOnly ? Krom.saveDialog() : Krom.openDialog();
			if (App.path != null) {
				App.path = StringTools.replace(App.path, "\\\\", "\\");
				App.path = StringTools.replace(App.path, "\r", "");
				var sep = kha.System.systemId == "Windows" ? "\\" : "/";
				App.filenameHandle.text = App.path.substr(App.path.lastIndexOf(sep) + 1);
				if (App.foldersOnly) App.path = App.path.substr(0, App.path.lastIndexOf(sep));
				App.filesDone(App.path);
			}
			return;
		}
		#end

		UIBox.modalW = Std.int(625 * uibox.SCALE);
		UIBox.modalH = Std.int(545 * uibox.SCALE);

		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var left = Std.int(appw / 2 - UIBox.modalW / 2);
		var right = Std.int(appw / 2 + UIBox.modalW / 2);
		var top = Std.int(apph / 2 - UIBox.modalH / 2);
		var bottom = Std.int(apph / 2 + UIBox.modalH / 2);
		
		// g.color = 0x44000000;
		// g.fillRect(0, 0, appw, apph);

		g.color = uibox.t.SEPARATOR_COL;
		g.fillRect(left, top, UIBox.modalW, UIBox.modalH);
		
		g.end();
		uibox.begin(g);
		var pathHandle = Id.handle();
		if (uibox.window(App.whandle, left, top, UIBox.modalW, UIBox.modalH - 50)) {
			pathHandle.text = uibox.textInput(pathHandle, "Path");
			if (App.showFilename) uibox.textInput(App.filenameHandle, "File");
			App.path = zui.Ext.fileBrowser(uibox, pathHandle, App.foldersOnly);
		}
		uibox.end(false);
		g.begin(false);

		if (Format.checkTextureFormat(App.path) || Format.checkMeshFormat(App.path) || Format.checkProjectFormat(App.path)) {
			App.showFiles = false;
			App.filesDone(App.path);
			var sep = kha.System.systemId == "Windows" ? "\\" : "/";
			pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(sep));
			App.whandle.redraws = 2;
			UITrait.inst.ddirty = 2;
		}

		g.end();
		uibox.beginLayout(g, right - Std.int(uibox.ELEMENT_W()), bottom - Std.int(uibox.ELEMENT_H() * 1.2), Std.int(uibox.ELEMENT_W()));
		if (uibox.button("OK")) {
			App.showFiles = false;
			App.filesDone(App.path);
			UITrait.inst.ddirty = 2;
		}
		uibox.endLayout(false);

		uibox.beginLayout(g, right - Std.int(uibox.ELEMENT_W() * 2), bottom - Std.int(uibox.ELEMENT_H() * 1.2), Std.int(uibox.ELEMENT_W()));
		if (uibox.button("Cancel")) {
			App.showFiles = false;
			UITrait.inst.ddirty = 2;
		}
		uibox.endLayout();

		g.begin(false);
	}
}
