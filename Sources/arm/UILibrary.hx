package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;

@:access(zui.Zui)
@:access(iron.data.Data)
class UILibrary extends iron.Trait {

	public static var inst:UILibrary;

	public var isScrolling = false;
	public var show = true;
	public var windowH = 200;
	public static var defaultWindowH = 200;

	var ui:Zui;

	public var assets:Array<TAsset> = [];
	public var assetNames:Array<String> = [];
	public var assetId = 0;

	public function new() {
		super();
		inst = this;

		windowH = Std.int(windowH * armory.data.Config.raw.window_scale);
		defaultWindowH = windowH;
		
		var scale = armory.data.Config.raw.window_scale;
		ui = new Zui( { font: App.font, scaleFactor: scale } );
		
		haxeTrace = haxe.Log.trace;
		haxe.Log.trace = consoleTrace;

		notifyOnUpdate(update);
		notifyOnRender2D(render);
	}

	var haxeTrace:Dynamic->haxe.PosInfos->Void;
	var lastTrace = '';
	function consoleTrace(v:Dynamic, ?inf:haxe.PosInfos) {
		lastTrace = Std.string(v);
		haxeTrace(v, inf);
    }

	function update() {
		isScrolling = ui.isScrolling;
		if (!show) return;
		if (!arm.App.uienabled) return;
	}

	var hwnd = Id.handle();
	function render(g:kha.graphics2.Graphics) {
		if (!show) return;
		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;
		var mouse = iron.system.Input.getMouse();
		g.color = 0xffffffff;

		g.end();
		ui.begin(g);
		
		if (ui.window(hwnd, 0, arm.App.realh() - windowH, arm.App.realw() - UITrait.inst.windowW, windowH)) {

			var htab = Id.handle({position: 3});

			var lastWindowH = windowH;
			if (ui.tab(htab, "")) windowH = Std.int(ui.ELEMENT_H());
			else windowH = defaultWindowH;
			if (lastWindowH != windowH) arm.App.resize();

			if (ui.tab(htab, "Preferences")) {
				ui.text("v0.3");
			}

			if (ui.tab(htab, "Console")) {
				ui.text("Ready");
				ui.text(lastTrace);
			}

			if (ui.tab(htab, "Textures")) {

				if (ui.button("Import")) {
					arm.App.showFiles = true;
					arm.App.foldersOnly = false;
					arm.App.filesDone = function(path:String) {
						UITrait.inst.importAsset(path);
					}
				}

				if (assets.length > 0) {
					var i = assets.length - 1;
					while (i >= 0) {
						var asset = assets[i];
						if (ui.image(UITrait.inst.getImage(asset)) == State.Started) {
							arm.App.dragAsset = asset;
						}
						ui.row([1/8, 7/8]);
						var b = ui.button("X");
						asset.name = ui.textInput(Id.handle().nest(asset.id, {text: asset.name}), "", Right);
						assetNames[i] = asset.name;
						if (b) {
							UITrait.inst.getImage(asset).unload();
							assets.splice(i, 1);
							assetNames.splice(i, 1);
						}
						i--;
					}
				}
				else {
					ui.text("(Drag & drop assets)");
				}
			}

			if (ui.tab(htab, "Materials")) {
			}

			if (ui.tab(htab, "Brushes")) {
			}
		}
		ui.end();
		g.begin(false);
	}
}
