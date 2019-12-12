package arm.ui;

import kha.System;
import kha.Image;
import kha.graphics4.PipelineState;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.graphics4.BlendingFactor;
import zui.Zui;
import zui.Id;
import iron.system.Input;
import arm.util.UVUtil;
import arm.Tool;

@:access(zui.Zui)
class UIView2D {

	public static var inst: UIView2D;
	public var show = false;
	public var type = View2DLayer;
	public var wx: Int;
	public var wy: Int;
	public var ww: Int;
	public var wh: Int;
	public var ui: Zui;
	public var hwnd = Id.handle();
	public var panX = 0.0;
	public var panY = 0.0;
	public var panScale = 1.0;
	var pipe: PipelineState;
	var texType = TexBase;
	var uvmapShow = false;

	public function new() {
		inst = this;

		pipe = new PipelineState();
		pipe.vertexShader = Reflect.field(kha.Shaders, "layer_view_vert");
		pipe.fragmentShader = Reflect.field(kha.Shaders, "layer_view_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipe.inputLayout = [vs];
		pipe.blendSource = BlendingFactor.BlendOne;
		pipe.blendDestination = BlendingFactor.BlendZero;
		pipe.colorWriteMaskAlpha = false;
		pipe.compile();

		var scale = Config.raw.window_scale;
		ui = new Zui({font: App.font, theme: App.theme, color_wheel: App.color_wheel, scaleFactor: scale});
		ui.scrollEnabled = false;

		iron.App.notifyOnRender2D(render);
		iron.App.notifyOnUpdate(update);
	}

	function render(g: kha.graphics2.Graphics) {
		if (UINodes.inst.defaultWindowW == 0) UINodes.inst.defaultWindowW = Std.int(iron.App.w() / 2);
		if (UINodes.inst.defaultWindowH == 0) UINodes.inst.defaultWindowH = Std.int(iron.App.h() / 2);
		ww = UINodes.inst.defaultWindowW;
		wx = Std.int(iron.App.w()) + UITrait.inst.toolbarw;
		wy = UITrait.inst.headerh * 2;

		if (!show) return;
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (Context.pdirty >= 0) hwnd.redraws = 2; // Paint was active

		var tw = iron.App.w() * 0.95 * panScale;
		var tx = iron.App.w() / 2 - tw / 2 + panX;
		var ty = iron.App.h() / 2 - tw / 2 + panY;

		g.end();

		// Cache grid
		if (UINodes.inst.grid == null) UINodes.inst.drawGrid();

		// Ensure UV map is drawn
		if (uvmapShow) UVUtil.cacheUVMap();

		ui.begin(g);
		wh = iron.App.h();
		if (UINodes.inst.show) {
			wh -= UINodes.inst.defaultWindowH;
		}
		if (ui.window(hwnd, wx, wy, ww, wh)) {

			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(UINodes.inst.grid, (panX * panScale) % 40 - 40, (panY * panScale) % 40 - 40);

			// Texture
			ui.g.pipeline = pipe;
			var l = Context.layer;
			var tex: Image = null;

			if (type == View2DLayer) {
				tex = texType == TexBase ? l.texpaint : texType == TexNormal ? l.texpaint_nor : l.texpaint_pack;
				if (Context.layerIsMask) tex = l.texpaint_mask;
			}
			else { // View2DAsset
				tex = UITrait.inst.getImage(Context.texture);
			}

			var th = tw;
			if (tex != null) {
				th = tw * (tex.height / tex.width);
				if (!UITrait.inst.textureFilter) {
					ui.g.imageScaleQuality = kha.graphics2.ImageScaleQuality.Low;
				}
				ui.g.drawScaledImage(tex, tx, ty, tw, th);
				if (!UITrait.inst.textureFilter) {
					ui.g.imageScaleQuality = kha.graphics2.ImageScaleQuality.High;
				}
			}
			ui.g.pipeline = null;

			// UV map
			if (type == View2DLayer && uvmapShow) {
				ui.g.drawScaledImage(UVUtil.uvmap, tx, ty, tw, th);
			}

			// Editable layer name
			var ACCENT_COL = ui.t.ACCENT_COL;
			var BUTTON_H = ui.t.BUTTON_H;
			var ELEMENT_H = ui.t.ELEMENT_H;
			var FONT_SIZE = ui.fontSize;
			ui.t.ACCENT_COL = 0x00000000;
			ui.t.BUTTON_H = 30;
			ui.t.ELEMENT_H = 30;
			ui.fontSize = Std.int(22 * ui.SCALE());
			ui._x = ww - ui.ELEMENT_W() * 1.4;
			ui._y = wh - ui.ELEMENT_H() * 1.2;
			ui._w = Std.int(ui.ELEMENT_W() * 1.4);
			var h = Id.handle();

			if (type == View2DLayer) {
				h.text = l.name;
				l.name = ui.textInput(h, "", Right);
			}
			else {
				var asset = Context.texture;
				if (asset != null) {
					var assetNames = Project.assetNames;
					var i = assetNames.indexOf(asset.name);
					h.text = asset.name;
					asset.name = ui.textInput(h, "", Right);
					assetNames[i] = asset.name;
				}
			}

			if (h.changed) UITrait.inst.hwnd.redraws = 2;
			ui.t.ACCENT_COL = ACCENT_COL;
			ui.t.BUTTON_H = BUTTON_H;
			ui.t.ELEMENT_H = ELEMENT_H;
			ui.fontSize = FONT_SIZE;

			// Controls
			if (type == View2DLayer) {
				var ew = Std.int(ui.ELEMENT_W());
				ui.g.color = ui.t.WINDOW_BG_COL;
				ui.g.fillRect(0, 0, ww, ui.ELEMENT_H() + ui.ELEMENT_OFFSET());
				ui.g.color = 0xffffffff;
				ui._x = 2;
				ui._y = 2;
				ui._w = ew;
				texType = ui.combo(Id.handle({position: texType}), ["Base", "Normal Map", "ORM"], "Texture");
				ui._x += ew + 3;
				ui._y = 2;
				uvmapShow = ui.check(Id.handle({selected: uvmapShow}), "UV Map");
				ui._x += ew + 3;
				ui._y = 2;
			}

			if (Context.tool == ToolPicker) {
				var cursorImg = Res.get("cursor.k");
				ui.g.drawScaledImage(cursorImg, tx + tw * UITrait.inst.uvxPicked - 16, ty + th * UITrait.inst.uvyPicked - 16, 32, 32);
			}
		}
		ui.end();
		g.begin(false);
	}

	function update() {
		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var headerh = ui.ELEMENT_H() * 1.4;
		UITrait.inst.paint2d = false;

		if (!App.uienabled ||
			!show ||
			mouse.x < wx ||
			mouse.x > wx + ww ||
			mouse.y < wy + headerh ||
			mouse.y > wy + wh) {
			return;
		}

		if (mouse.down("right") || mouse.down("middle")) {
			panX += mouse.movementX;
			panY += mouse.movementY;
		}
		if (mouse.wheelDelta != 0) {
			panScale -= mouse.wheelDelta / 10;
			if (panScale < 0.1) panScale = 0.1;
			if (panScale > 3.0) panScale = 3.0;
		}

		if (type == View2DLayer &&
			(Operator.shortcut(Config.keymap.action_paint) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint))) {
			UITrait.inst.paint2d = true;
		}

		if (ui.isTyping) return;

		if (kb.started("left")) panX -= 5;
		else if (kb.started("right")) panX += 5;
		if (kb.started("up")) panY -= 5;
		else if (kb.started("down")) panY += 5;
	}
}
