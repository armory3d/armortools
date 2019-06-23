package arm.ui;

import kha.System;
import kha.Image;
import kha.graphics4.PipelineState;
import kha.graphics4.VertexShader;
import kha.graphics4.FragmentShader;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.graphics4.BlendingFactor;
import zui.Zui;
import zui.Id;
import iron.system.Input;
import arm.util.UVUtil;
import arm.data.ConstData;

@:access(zui.Zui)
class UIView2D {

	public static var inst:UIView2D;
	public var show = false;
	public var type = 0; // Layer, texture
	public var wx:Int;
	public var wy:Int;
	public var ww:Int;
	public var wh:Int;
	public var ui:Zui;
	public var hwnd = Id.handle();
	public var panX = 0.0;
	public var panY = 0.0;
	public var panScale = 1.0;
	var pipe:PipelineState;
	var texType = 0;
	var uvmapShow = false;

	public function new() {
		inst = this;

		pipe = new PipelineState();
		pipe.vertexShader = VertexShader.fromSource(ConstData.layerViewVert);
		pipe.fragmentShader = FragmentShader.fromSource(ConstData.layerViewFrag);
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipe.inputLayout = [vs];
		pipe.blendSource = BlendingFactor.BlendOne;
		pipe.blendDestination = BlendingFactor.BlendZero;
		pipe.colorWriteMaskAlpha = false;
		pipe.compile();

		var t = Reflect.copy(App.theme);
		t.ELEMENT_H = 18;
		t.BUTTON_H = 16;
		var scale = Config.raw.window_scale;
		ui = new Zui({font: App.font, theme: t, color_wheel: App.color_wheel, scaleFactor: scale});
		ui.scrollEnabled = false;

		iron.App.notifyOnRender2D(render);
		iron.App.notifyOnUpdate(update);
	}

	function render(g:kha.graphics2.Graphics) {
		ww = Std.int(iron.App.w()) + UITrait.inst.toolbarw;
		var lay = Config.raw.ui_layout;
		wx = lay == 0 ? Std.int(iron.App.w()) : UITrait.inst.windowW;
		wx += UITrait.inst.toolbarw;
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
			wh = Std.int(iron.App.h() / 2);
		}
		if (ui.window(hwnd, wx, wy, ww, wh)) {

			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(UINodes.inst.grid, (panX * panScale) % 40 - 40, (panY * panScale) % 40 - 40);

			// Texture
			ui.g.pipeline = pipe;
			var l = Context.layer;
			var tex:Image = null;

			if (type == 0) { // Layer
				tex = texType == 0 ? l.texpaint : texType == 1 ? l.texpaint_nor : l.texpaint_pack;
				if (Context.layerIsMask) tex = l.texpaint_mask;
			}
			else { // Texture
				tex = UITrait.inst.getImage(Context.texture);
			}
			
			var th = tw;
			if (tex != null) {
				th = tw * (tex.height / tex.width);
				ui.g.drawScaledImage(tex, tx, ty, tw, th);
			}
			ui.g.pipeline = null;

			// UV map
			if (type == 0 && uvmapShow) {
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
			ui.fontSize = Std.int(22 * ui.SCALE);
			ui._x = ww - ui.ELEMENT_W() * 1.4;
			ui._y = wh - ui.ELEMENT_H() * 1.2;
			ui._w = Std.int(ui.ELEMENT_W() * 1.4);
			var h = Id.handle();
			
			if (type == 0) {
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
			if (type == 0) {
				var ew = Std.int(ui.ELEMENT_W());
				ui.g.color = ui.t.WINDOW_BG_COL;
				ui.g.fillRect(0, 0, ww, 24 * ui.SCALE);
				ui.g.color = 0xffffffff;
				ui._x = 3;
				ui._y = 3;
				ui._w = ew;
				texType = ui.combo(Id.handle({position: texType}), ["Base", "Normal Map", "ORM"], "Texture");
				ui._x += ew + 3;
				ui._y = 3;
				uvmapShow = ui.check(Id.handle({selected: uvmapShow}), "UV Map");
				ui._x += ew + 3;
				ui._y = 3;
			}
		}
		ui.end();
		g.begin(false);
	}

	function update() {
		var m = Input.getMouse();
		var kb = Input.getKeyboard();

		var headerh = ui.ELEMENT_H() * 1.4;
		UITrait.inst.paint2d = false;

		if (!App.uienabled ||
			!show ||
			m.x + App.x() < wx ||
			m.x + App.x() > wx + ww ||
			m.y + App.y() < wy + headerh ||
			m.y + App.y() > wy + wh) return;
		
		if (m.down("right") || m.down("middle")) {
			panX += m.movementX;
			panY += m.movementY;
		}
		if (m.wheelDelta != 0) {
			panScale -= m.wheelDelta / 10;
			if (panScale < 0.1) panScale = 0.1;
			if (panScale > 3.0) panScale = 3.0;
		}

		if (type == 0 && m.down("left")) {
			UITrait.inst.paint2d = true;
		}

		if (kb.started("left")) panX -= 5;
		else if (kb.started("right")) panX += 5;
		if (kb.started("up")) panY -= 5;
		else if (kb.started("down")) panY += 5;
	}
}
