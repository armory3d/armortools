package arm;

import armory.object.Object;
import armory.system.Cycles;
import zui.*;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MaterialData;

class UINodesBrush extends armory.Trait {

	public static var show = false;

	public static var wx:Int;
	public static var wy:Int;

	var ui:Zui;
	var drawMenu = false;
	var showMenu = false;
	var hideMenu = false;
	var popupX = 0;
	var popupY = 0;

	static var font:kha.Font;

	public function new() {
		super();

		// Load font for UI labels
		iron.data.Data.getFont('droid_sans.ttf', function(f:kha.Font) {

			iron.data.Data.getBlob('default_brush.json', function(b:kha.Blob) {

				canvas = haxe.Json.parse(b.toString());
				parseBrush();

				font = f;

				var t = Reflect.copy(zui.Themes.dark);
				t.FILL_WINDOW_BG = true;
				t.ELEMENT_H = 18;
				t.BUTTON_H = 16;
				ui = new Zui({font: f, theme: t});
				ui.scrollEnabled = false;
				armory.Scene.active.notifyOnInit(sceneInit);
			});
		});
	}

	function sceneInit() {
		// Store references to cube and plane objects
		notifyOnRender2D(render2D);
		notifyOnUpdate(update);
	}

	var wh = 300;

	function update() {
		if (!show) return;
		if (!UITrait.uienabled) return;
		var mouse = iron.system.Input.getMouse();
		var keyboard = iron.system.Input.getKeyboard();

		wx = 200;
		wy = iron.App.h() - wh;
		if (mouse.x < wx || mouse.y < wy) return;

		if (ui.isTyping) return;

		if (mouse.released("right") || keyboard.started("a")) {
			showMenu = true;
			popupX = Std.int(mouse.x);
			popupY = Std.int(mouse.y);
		}
		else if (mouse.released()) {
			hideMenu = true;
		}

		if (keyboard.started("x")) {
			var n = uinodes.nodeSelected;
			var i = 0;      
			while (i < canvas.links.length) {
				var l = canvas.links[i];
			    if (l.from_id == n.id || l.to_id == n.id) {
			        canvas.links.splice(i, 1);
			    }
			    else i++;
			}
			canvas.nodes.remove(n);
		}

		if (keyboard.started("p")) {
			trace(haxe.Json.stringify(canvas));
		}
	}

	static var uinodes = new Nodes();

	public static var canvas:TNodeCanvas = null;

	static var bg:kha.Image = null;

	function render2D(g:kha.graphics2.Graphics) {
		if (!show) return;

		if (!UITrait.uienabled && ui.inputRegistered) ui.unregisterInput();
		if (UITrait.uienabled && !ui.inputRegistered) ui.registerInput();
		
		g.end();

		if (bg == null) {
			var w = iron.App.w() - 200;
			var h = wh;
			bg = kha.Image.createRenderTarget(w, h);
			bg.g2.begin(true, 0xff141414);
			for (i in 0...Std.int(h / 40) + 1) {
				bg.g2.color = 0xff303030;
				bg.g2.drawLine(0, i * 40, w, i * 40);
				bg.g2.color = 0xff202020;
				bg.g2.drawLine(0, i * 40 + 20, w, i * 40 + 20);
			}
			for (i in 0...Std.int(w / 40) + 1) {
				bg.g2.color = 0xff303030;
				bg.g2.drawLine(i * 40, 0, i * 40, h);
				bg.g2.color = 0xff202020;
				bg.g2.drawLine(i * 40 + 20, 0, i * 40 + 20, h);
			}
			bg.g2.end();
		}

		// Start with UI
		ui.begin(g);
		
		// Make window
		var mouse = iron.system.Input.getMouse();
		wx = 200;
		wy = iron.App.h() - wh;
		var hwin = Id.handle();
		if (ui.window(hwin, wx, wy, iron.App.w() - 200, wh)) {

			ui.g.color = 0xffffffff;
			ui.g.drawImage(bg, 0, 0);

			ui.g.font = font;
			ui.g.fontSize = 42;
			var title = "Brush";
			// var title = "Brush (right-click to add node)";
			var titlew = ui.g.font.width(42, title);
			var titleh = ui.g.font.height(42);
			ui.g.drawString(title, iron.App.w() - 200 - titlew - 20, wh - titleh - 10);
			
			uinodes.nodeCanvas(ui, canvas);
		}

		ui.endWindow();

		if (drawMenu) {
			ui.beginLayout(g, popupX, popupY, 120);
			
			if (ui.button("On Brush")) {
				var node_id = uinodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "On Brush",
					type: "OnBrushNode",
					x: mouse.x - wx,
					y: mouse.y - wy,
					color: 0xff4982a0,
					inputs: [],
					outputs: [
						{
							id: uinodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Out",
							type: "ACTION",
							color: 0xff63c763,
							default_value: null
						},
						{
							id: uinodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Position",
							type: "VECTOR",
							color: 0xff63c763,
							default_value: null
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				uinodes.nodeDrag = n;
				uinodes.nodeSelected = n;
			}
			if (ui.button("Paint")) {
				var node_id = uinodes.getNodeId(canvas.nodes);
				var n:TNode = {
					id: node_id,
					name: "Paint",
					type: "PaintNode",
					x: mouse.x - wx,
					y: mouse.y - wy,
					color: 0xff4982a0,
					inputs: [
						{
							id: uinodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "In",
							type: "ACTION",
							color: 0xff63c763,
							default_value: null
						},
						{
							id: uinodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Position",
							type: "VECTOR",
							color: 0xff63c763,
							default_value: null
						}
					],
					outputs: [
						{
							id: uinodes.getSocketId(canvas.nodes),
							node_id: node_id,
							name: "Out",
							type: "ACTION",
							color: 0xff63c763,
							default_value: null
						}
					],
					buttons: []
				};
				canvas.nodes.push(n);
				uinodes.nodeDrag = n;
				uinodes.nodeSelected = n;
			}

			if (ui.button("Parse")) {
				parseBrush();
			}

			ui.endLayout();
		}

		ui.end();

		g.begin(false);

		if (showMenu) {
			showMenu = false;
			drawMenu = true;
			
		}
		if (hideMenu) {
			hideMenu = false;
			drawMenu = false;
		}
	}

	function parseBrush() {
		armory.system.Logic.packageName = "arm.logicnode";
		var tree = armory.system.Logic.parse(canvas, false);
	}
}
