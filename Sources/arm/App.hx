package arm;

import zui.*;
import zui.Zui.State;
import zui.Canvas;

class App extends iron.Trait {

	public static var uienabled = true;
	public static var isDragging = false;
	public static var dragAsset:TAsset = null;
	public static var showFiles = false;
	public static var foldersOnly = false;
	public static var filesDone:String->Void;
	public static var dropPath = "";
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font:kha.Font = null;
	public static var theme:zui.Themes.TTheme;
	public static var color_wheel:kha.Image;
	static var uimodal:Zui;

	public static function getEnumTexts():Array<String> {
		return UITrait.inst.assetNames.length > 0 ? UITrait.inst.assetNames : [""];
	}

	public static function mapEnum(s:String):String {
		for (a in UITrait.inst.assets) if (a.name == s) return a.file;
		return "";
	}

	public function new() {
		super();

		kha.System.notifyOnDropFiles(function(filePath:String) {
			dropPath = StringTools.rtrim(filePath);
			var mouse = iron.system.Input.getMouse();
			dropX = mouse.x;
			dropY = mouse.y;
		});

		iron.data.Data.getFont("droid_sans.ttf", function(f:kha.Font) {
			iron.data.Data.getBlob("theme.arm", function(b:kha.Blob) {
				iron.data.Data.getImage('color_wheel.png', function(image:kha.Image) {
					theme = haxe.Json.parse(b.toString());
					font = f;
					color_wheel = image;
					var scale = armory.data.Config.raw.window_scale;
					zui.Nodes.getEnumTexts = getEnumTexts;
					zui.Nodes.mapEnum = mapEnum;
					uimodal = new Zui( { font: f, scaleFactor: scale } );
					
					notifyOnInit(function() {
						notifyOnUpdate(update);
						notifyOnRender2D(render);
						object.addTrait(new UITrait());
						object.addTrait(new UINodes());
						object.addTrait(new UIView2D());
						object.addTrait(new FlyCamera());
						object.addTrait(new OrbitCamera());
					});
				});
			});
		});
	}

	public static function w():Int {
		// TODO: account for Config.raw.window_scale
		var res = 0;
		if (UINodes.inst == null || UITrait.inst == null) {
			res = kha.System.windowWidth() - UITrait.defaultWindowW;
		}
		else if (UINodes.inst.show || UIView2D.inst.show) {
			res = Std.int((kha.System.windowWidth() - UITrait.inst.windowW) / 2);
		}
		else {
			res = kha.System.windowWidth() - UITrait.inst.windowW;
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h():Int {
		// TODO: account for Config.raw.window_scale
		var res = 0;
		res = kha.System.windowHeight();
		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function realw():Int {
		return kha.System.windowWidth();
	}

	public static function realh():Int {
		return kha.System.windowHeight();
	}

	public static function resize() {
		iron.Scene.active.camera.buildProjection();
		UITrait.inst.dirty = true;

		if (UINodes.inst.grid != null) {
			UINodes.inst.grid.unload();
			UINodes.inst.grid = null;
		}
	}

	static function update() {
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();
		// if (mouse.started() && mouse.x < 50 && mouse.y < 50) show = !show;

		isDragging = dragAsset != null;
		if (mouse.released() && isDragging) {
			if (UINodes.inst.show && mouse.x > UINodes.inst.wx && mouse.y > UINodes.inst.wy) {
				var index = 0;
				for (i in 0...UITrait.inst.assets.length) if (UITrait.inst.assets[i] == dragAsset) { index = i; break; }
				UINodes.inst.acceptDrag(index);
			}
			dragAsset = null;
		}

		if (dropPath != "") {
			var p = dropPath.toLowerCase();
			if (StringTools.endsWith(p, ".obj") ||
				StringTools.endsWith(p, ".fbx") ||
				// StringTools.endsWith(p, ".blend") ||
				StringTools.endsWith(p, ".gltf")) {
				UITrait.inst.importMesh(dropPath);
			}
			else {
				UITrait.inst.importAsset(dropPath);
				// Place image node
				if (UINodes.inst.show && dropX > UINodes.inst.wx && dropX < UINodes.inst.wx + UINodes.inst.ww) {
					UINodes.inst.acceptDrag(UITrait.inst.assets.length - 1);
					UINodes.inst.nodes.nodeDrag = null;
					UINodes.inst.hwnd.redraws = 2;
				}
			}
			dropPath = "";
		}

		updateFiles();
	}

	static function updateFiles() {
		if (!showFiles) return;

		var mouse = iron.system.Input.getMouse();

		if (mouse.released()) {
			var appw = kha.System.windowWidth();
			var apph = kha.System.windowHeight();
			var left = appw / 2 - modalRectW / 2;
			var right = appw / 2 + modalRectW / 2;
			var top = apph / 2 - modalRectH / 2;
			var bottom = apph / 2 + modalRectH / 2;
			if (mouse.x < left || mouse.x > right || mouse.y < top + modalHeaderH || mouse.y > bottom) {
				showFiles = false;
			}
		}
	}

	static var modalW = 625;
	static var modalH = 545;
	static var modalHeaderH = 66;
	static var modalRectW = 625; // No shadow
	static var modalRectH = 545;

	static var path = '/';
	static function renderFiles(g:kha.graphics2.Graphics) {
		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var left = appw / 2 - modalW / 2;
		var top = apph / 2 - modalH / 2;
		g.color = 0xff202020;
		g.fillRect(left, top, modalW, modalH);

		var leftRect = Std.int(appw / 2 - modalRectW / 2);
		var rightRect = Std.int(appw / 2 + modalRectW / 2);
		var topRect = Std.int(apph / 2 - modalRectH / 2);
		var bottomRect = Std.int(apph / 2 + modalRectH / 2);
		topRect += modalHeaderH;
		
		g.end();
		uimodal.begin(g);
		if (uimodal.window(Id.handle(), leftRect, topRect, modalRectW, modalRectH - 100)) {
			var pathHandle = Id.handle();
			pathHandle.text = uimodal.textInput(pathHandle);
			path = zui.Ext.fileBrowser(uimodal, pathHandle, foldersOnly);
		}
		uimodal.end(false);
		g.begin(false);

		uimodal.beginLayout(g, rightRect - 100, bottomRect - 30, 100);
		if (uimodal.button("OK")) {
			showFiles = false;
			filesDone(path);
			UITrait.inst.dirty = true;
		}
		uimodal.endLayout(false);

		uimodal.beginLayout(g, rightRect - 200, bottomRect - 30, 100);
		if (uimodal.button("Cancel")) {
			showFiles = false;
			UITrait.inst.dirty = true;
		}
		uimodal.endLayout();

		g.begin(false);
	}

	static var lastW = -1;
	static var lastH = -1;
	static var wasResized = false;
	static function render(g:kha.graphics2.Graphics) {
		if (lastW >= 0 && arm.App.realw() > 0 && (lastW != arm.App.realw() || lastH != arm.App.realh())) {
			arm.App.resize();
		}
		lastW = arm.App.realw();
		lastH = arm.App.realh();

		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		uienabled = !showFiles;
		if (showFiles) renderFiles(g);

		// var ready = showFiles || dirty;
		// TODO: Texture params get overwritten
		// if (ready) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;
		// if (UINodes.inst._matcon != null) for (t in UINodes.inst._matcon.bind_textures) t.params_set = null;

		// iron.Scene.active.camera.renderPath.ready = ready;
		// dirty = false;
	}
}
