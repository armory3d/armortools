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
	public static var showFilename = false;
	public static var whandle = new Zui.Handle();
	public static var filenameHandle = new Zui.Handle({text: "untitled"});
	public static var filesDone:String->Void;
	public static var dropPath = "";
	public static var dropX = 0.0;
	public static var dropY = 0.0;
	public static var font:kha.Font = null;
	public static var theme:zui.Themes.TTheme;
	public static var color_wheel:kha.Image;
	public static var uimodal:Zui;
	static var modalW = 625;
	static var modalH = 545;
	static var lastW = -1;
	static var lastH = -1;

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

		iron.data.Data.getFont("font_default.ttf", function(f:kha.Font) {
			iron.data.Data.getBlob("theme.arm", function(b:kha.Blob) {
				iron.data.Data.getImage('color_wheel.png', function(image:kha.Image) {
					theme = haxe.Json.parse(b.toString());
					theme.WINDOW_BG_COL = Std.parseInt(cast theme.WINDOW_BG_COL);
					theme.WINDOW_TINT_COL = Std.parseInt(cast theme.WINDOW_TINT_COL);
					theme.ACCENT_COL = Std.parseInt(cast theme.ACCENT_COL);
					theme.ACCENT_HOVER_COL = Std.parseInt(cast theme.ACCENT_HOVER_COL);
					theme.ACCENT_SELECT_COL = Std.parseInt(cast theme.ACCENT_SELECT_COL);
					theme.PANEL_BG_COL = Std.parseInt(cast theme.PANEL_BG_COL);
					theme.PANEL_TEXT_COL = Std.parseInt(cast theme.PANEL_TEXT_COL);
					theme.BUTTON_COL = Std.parseInt(cast theme.BUTTON_COL);
					theme.BUTTON_TEXT_COL = Std.parseInt(cast theme.BUTTON_TEXT_COL);
					theme.BUTTON_HOVER_COL = Std.parseInt(cast theme.BUTTON_HOVER_COL);
					theme.BUTTON_PRESSED_COL = Std.parseInt(cast theme.BUTTON_PRESSED_COL);
					theme.TEXT_COL = Std.parseInt(cast theme.TEXT_COL);
					theme.LABEL_COL = Std.parseInt(cast theme.LABEL_COL);
					theme.ARROW_COL = Std.parseInt(cast theme.ARROW_COL);
					theme.SEPARATOR_COL = Std.parseInt(cast theme.SEPARATOR_COL);
					font = f;
					color_wheel = image;
					zui.Nodes.getEnumTexts = getEnumTexts;
					zui.Nodes.mapEnum = mapEnum;
					uimodal = new Zui({ font: f, scaleFactor: armory.data.Config.raw.window_scale });
					
					iron.App.notifyOnInit(function() {
						// #if arm_debug
						// iron.Scene.active.sceneParent.getTrait(armory.trait.internal.DebugConsole).visible = false;
						// #end
						iron.App.notifyOnUpdate(update);
						iron.Scene.active.root.addTrait(new UITrait());
						iron.Scene.active.root.addTrait(new UINodes());
						iron.Scene.active.root.addTrait(new UIView2D());
						iron.Scene.active.root.addTrait(new arm.trait.FlyCamera());
						iron.Scene.active.root.addTrait(new arm.trait.OrbitCamera());
						iron.Scene.active.root.addTrait(new arm.trait.ArcBallCamera());
						iron.App.notifyOnInit(function() {
							iron.App.notifyOnRender2D(render); // Draw on top
						});
						
					});
				});
			});
		});
	}

	public static function w():Int {
		if (UITrait.inst != null && UITrait.inst.materialPreview) return 100;
		if (UITrait.inst != null && UITrait.inst.stickerPreview) return 512;
		
		var res = 0;
		if (UINodes.inst == null || UITrait.inst == null) {
			res = kha.System.windowWidth() - UITrait.defaultWindowW;
		}
		else if (UINodes.inst.show || UIView2D.inst.show) {
			res = Std.int((kha.System.windowWidth() - UITrait.inst.windowW) / 2);
		}
		else if (UITrait.inst.show) {
			res = kha.System.windowWidth() - UITrait.inst.windowW;
		}
		else {
			res = kha.System.windowWidth();
		}

		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	public static function h():Int {
		if (UITrait.inst != null && UITrait.inst.materialPreview) return 100;
		if (UITrait.inst != null && UITrait.inst.stickerPreview) return 512;

		var res = 0;
		res = kha.System.windowHeight();
		return res > 0 ? res : 1; // App was minimized, force render path resize
	}

	static var appx = 0;
	public static function x():Int {
		return appx;
	}

	public static function y():Int {
		return 0;
	}

	public static function realw():Int {
		return kha.System.windowWidth();
	}

	public static function realh():Int {
		return kha.System.windowHeight();
	}

	public static function resize() {
		iron.Scene.active.camera.buildProjection();
		UITrait.inst.ddirty = 2;

		var lay = UITrait.inst.C.ui_layout;
		appx = lay == 0 ? 0 : UITrait.inst.windowW;
		if (lay == 1 && (UINodes.inst.show || UIView2D.inst.show)) appx += iron.App.w();

		if (UINodes.inst.grid != null) {
			UINodes.inst.grid.unload();
			UINodes.inst.grid = null;
		}
	}

	static function getAssetIndex(f:String):Int {
		for (i in 0...UITrait.inst.assets.length) {
			if (UITrait.inst.assets[i].file == f) {
				return i;
			}
		}
		return 0;
	}

	static function update() {
		var mouse = iron.system.Input.getMouse();
		var kb = iron.system.Input.getKeyboard();

		isDragging = dragAsset != null;
		if (mouse.released() && isDragging) {
			if (UINodes.inst.show && mouse.x > UINodes.inst.wx && mouse.y > UINodes.inst.wy) {
				var index = 0;
				for (i in 0...UITrait.inst.assets.length) {
					if (UITrait.inst.assets[i] == dragAsset) {
						index = i;
						break;
					}
				}
				UINodes.inst.acceptDrag(index);
			}
			dragAsset = null;
		}

		if (dropPath != "") {
			var p = dropPath.toLowerCase();
			// Mesh
			if (StringTools.endsWith(p, ".obj") ||
				StringTools.endsWith(p, ".fbx") ||
				StringTools.endsWith(p, ".blend") ||
				StringTools.endsWith(p, ".gltf")) {
				UITrait.inst.importMesh(dropPath);
			}
			// Image
			else if (StringTools.endsWith(p, ".jpg") ||
					 StringTools.endsWith(p, ".png") ||
					 StringTools.endsWith(p, ".tga") ||
					 StringTools.endsWith(p, ".hdr")) {
				UITrait.inst.importAsset(dropPath);
				// Place image node
				if (UINodes.inst.show && dropX > UINodes.inst.wx && dropX < UINodes.inst.wx + UINodes.inst.ww) {
					UINodes.inst.acceptDrag(UITrait.inst.assets.length - 1);
					UINodes.inst.nodes.nodeDrag = null;
					UINodes.inst.hwnd.redraws = 2;
				}
			}
			// Project
			else if (StringTools.endsWith(p, ".arm")) {
				UITrait.inst.importProject(dropPath);
			}
			// Folder
			else if (p.indexOf(".") == -1) {
				#if kha_krom
				var systemId = kha.System.systemId;
				var cmd = systemId == "Windows" ? "dir /b " : "ls ";
				var sep = systemId == "Windows" ? "\\" : "/";
				var save = systemId == "Linux" ? "/tmp" : Krom.savePath();
				save += sep + "dir.txt";
				Krom.sysCommand(cmd + '"' + dropPath + '"' + ' > ' + '"' + save + '"');
				var str = haxe.io.Bytes.ofData(Krom.loadBlob(save)).toString();
				var files = str.split("\n");
				var mapbase = "";
				var mapnor = "";
				var mapocc = "";
				var maprough = "";
				var mapmet = "";
				var mapheight = "";
				// Import maps
				for (f in files) {
					if (f.length == 0) continue;
					f = StringTools.rtrim(f);
					var known = 
						StringTools.endsWith(f, ".jpg") ||
						StringTools.endsWith(f, ".png") ||
						StringTools.endsWith(f, ".tga") ||
						StringTools.endsWith(f, ".hdr");
					if (!known) continue;
					
					f = dropPath + sep + f;
					if (systemId == "Windows") f = StringTools.replace(f, "/", "\\");
					
					var base = f.substr(0, f.lastIndexOf(".")).toLowerCase();
					var valid = false;
					if (mapbase == "" && (StringTools.endsWith(base, "_albedo") ||
										  StringTools.endsWith(base, "_alb") ||
										  StringTools.endsWith(base, "_basecol") ||
										  StringTools.endsWith(base, "_basecolor") ||
										  StringTools.endsWith(base, "_diffuse") ||
										  StringTools.endsWith(base, "_base") ||
										  StringTools.endsWith(base, "_bc") ||
										  StringTools.endsWith(base, "_d") ||
										  StringTools.endsWith(base, "_col"))) {
						mapbase = f;
						valid = true;
					}
					if (mapnor == "" && (StringTools.endsWith(base, "_normal") ||
										 StringTools.endsWith(base, "_nor") ||
										 StringTools.endsWith(base, "_n") ||
										 StringTools.endsWith(base, "_nrm"))) {
						mapnor = f;
						valid = true;
					}
					if (mapocc == "" && (StringTools.endsWith(base, "_ao") ||
										 StringTools.endsWith(base, "_occlusion") ||
										 StringTools.endsWith(base, "_o") ||
										 StringTools.endsWith(base, "_occ"))) {
						mapocc = f;
						valid = true;
					}
					if (maprough == "" && (StringTools.endsWith(base, "_roughness") ||
										   StringTools.endsWith(base, "_roug") ||
										   StringTools.endsWith(base, "_r") ||
										   StringTools.endsWith(base, "_rough") ||
										   StringTools.endsWith(base, "_rgh"))) {
						maprough = f;
						valid = true;
					}
					if (mapmet == "" && (StringTools.endsWith(base, "_metallic") ||
										 StringTools.endsWith(base, "_metal") ||
										 StringTools.endsWith(base, "_metalness") ||
										 StringTools.endsWith(base, "_m") ||
										 StringTools.endsWith(base, "_met"))) {
						mapmet = f;
						valid = true;
					}
					if (mapheight == "" && (StringTools.endsWith(base, "_displacement") ||
										    StringTools.endsWith(base, "_height") ||
										    StringTools.endsWith(base, "_h") ||
											StringTools.endsWith(base, "_disp"))) {
						mapheight = f;
						valid = true;
					}

					if (valid) UITrait.inst.importAsset(f);
				}
				// Create material
				UITrait.inst.selectedMaterial = new MaterialSlot();
				UITrait.inst.materials.push(UITrait.inst.selectedMaterial);
				UINodes.inst.updateCanvasMap();
				var nodes = UINodes.inst.nodes;
				var canvas = UINodes.inst.canvas;
				var nout:Nodes.TNode = null;
				for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") { nout = n; break; }
				for (n in canvas.nodes) if (n.name == "RGB") { nodes.removeNode(n, canvas); break; }
				
				var pos = 0;
				if (mapbase != "") {
					var n = NodeCreator.createImageTexture();
					n.buttons[0].default_value = getAssetIndex(mapbase);
					n.buttons[0].data = mapEnum(getEnumTexts()[n.buttons[0].default_value]);
					n.x = 72;
					n.y = 192 + 160 * pos;
					pos++;
					var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 0 };
					canvas.links.push(l);
				}
				if (mapocc != "") {
					var n = NodeCreator.createImageTexture();
					n.buttons[0].default_value = getAssetIndex(mapocc);
					n.buttons[0].data = mapEnum(getEnumTexts()[n.buttons[0].default_value]);
					n.x = 72;
					n.y = 192 + 160 * pos;
					pos++;
					var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 2 };
					canvas.links.push(l);
				}
				if (maprough != "") {
					var n = NodeCreator.createImageTexture();
					n.buttons[0].default_value = getAssetIndex(maprough);
					n.buttons[0].data = mapEnum(getEnumTexts()[n.buttons[0].default_value]);
					n.x = 72;
					n.y = 192 + 160 * pos;
					pos++;
					var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 3 };
					canvas.links.push(l);
				}
				if (mapmet != "") {
					var n = NodeCreator.createImageTexture();
					n.buttons[0].default_value = getAssetIndex(mapmet);
					n.buttons[0].data = mapEnum(getEnumTexts()[n.buttons[0].default_value]);
					n.x = 72;
					n.y = 192 + 160 * pos;
					pos++;
					var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 4 };
					canvas.links.push(l);
				}
				if (mapnor != "") {
					var n = NodeCreator.createImageTexture();
					n.buttons[0].default_value = getAssetIndex(mapnor);
					n.buttons[0].data = mapEnum(getEnumTexts()[n.buttons[0].default_value]);
					n.x = 72;
					n.y = 192 + 160 * pos;
					pos++;
					var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 5 };
					canvas.links.push(l);
				}
				if (mapheight != "") {
					var n = NodeCreator.createImageTexture();
					n.buttons[0].default_value = getAssetIndex(mapheight);
					n.buttons[0].data = mapEnum(getEnumTexts()[n.buttons[0].default_value]);
					n.x = 72;
					n.y = 192 + 160 * pos;
					pos++;
					var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 7 };
					canvas.links.push(l);
				}
				iron.system.Tween.timer(0.01, function() {
					UINodes.inst.parsePaintMaterial();
					UITrait.inst.makeMaterialPreview();
				});
				#end
			}
			dropPath = "";
		}

		if (showFiles) updateFiles();
	}

	static function updateFiles() {
		var mouse = iron.system.Input.getMouse();
		if (mouse.released()) {
			var appw = kha.System.windowWidth();
			var apph = kha.System.windowHeight();
			var left = appw / 2 - modalW / 2;
			var right = appw / 2 + modalW / 2;
			var top = apph / 2 - modalH / 2;
			var bottom = apph / 2 + modalH / 2;
			if (mouse.x < left || mouse.x > right || mouse.y < top || mouse.y > bottom) {
				showFiles = false;
			}
		}
	}

	static function render(g:kha.graphics2.Graphics) {
		if (lastW >= 0 && arm.App.realw() > 0 && (lastW != arm.App.realw() || lastH != arm.App.realh())) {
			arm.App.resize();
		}
		lastW = arm.App.realw();
		lastH = arm.App.realh();

		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		uienabled = !showFiles;
		if (showFiles) renderFiles(g);
	}

	public static var path = '/';
	static function renderFiles(g:kha.graphics2.Graphics) {
		var appw = kha.System.windowWidth();
		var apph = kha.System.windowHeight();
		var left = Std.int(appw / 2 - modalW / 2);
		var right = Std.int(appw / 2 + modalW / 2);
		var top = Std.int(apph / 2 - modalH / 2);
		var bottom = Std.int(apph / 2 + modalH / 2);
		g.color = 0xff202020;
		g.fillRect(left, top, modalW, modalH);
		
		g.end();
		uimodal.begin(g);
		var pathHandle = Id.handle();
		if (uimodal.window(whandle, left, top, modalW, modalH - 50, true)) {
			pathHandle.text = uimodal.textInput(pathHandle, "Path");
			if (showFilename) filenameHandle.text = uimodal.textInput(filenameHandle, "File");
			path = zui.Ext.fileBrowser(uimodal, pathHandle, foldersOnly);
		}
		uimodal.end(false);
		g.begin(false);

		if (UITrait.checkImageFormat(path) || UITrait.checkMeshFormat(path) || UITrait.checkProjectFormat(path)) {
			showFiles = false;
			filesDone(path);
			var sep = kha.System.systemId == "Windows" ? "\\" : "/";
			pathHandle.text = pathHandle.text.substr(0, pathHandle.text.lastIndexOf(sep));
			whandle.redraws = 2;
			UITrait.inst.ddirty = 2;
		}

		uimodal.beginLayout(g, right - Std.int(uimodal.ELEMENT_W()), bottom - Std.int(uimodal.ELEMENT_H() * 1.2), Std.int(uimodal.ELEMENT_W()));
		if (uimodal.button("OK")) {
			showFiles = false;
			filesDone(path);
			UITrait.inst.ddirty = 2;
		}
		uimodal.endLayout(false);

		uimodal.beginLayout(g, right - Std.int(uimodal.ELEMENT_W() * 2), bottom - Std.int(uimodal.ELEMENT_H() * 1.2), Std.int(uimodal.ELEMENT_W()));
		if (uimodal.button("Cancel")) {
			showFiles = false;
			UITrait.inst.ddirty = 2;
		}
		uimodal.endLayout();

		g.begin(false);
	}
}
