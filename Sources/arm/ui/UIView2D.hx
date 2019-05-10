package arm.ui;

import zui.Zui;
import zui.Id;
import arm.util.UVUtil;

@:access(zui.Zui)
class UIView2D extends iron.Trait {

	public static var inst:UIView2D;
	public var show = false;
	public var wx:Int;
	public var wy:Int;
	public var ww:Int;
	public var wh:Int;
	public var ui:Zui;
	public var hwnd = Id.handle();
	public var panX = 0.0;
	public var panY = 0.0;
	public var panScale = 1.0;
	var pipe:kha.graphics4.PipelineState;
	var texType = 0;
	var uvmapShow = false;

	public function new() {
		super();
		inst = this;

		pipe = new kha.graphics4.PipelineState();
		pipe.vertexShader = kha.graphics4.VertexShader.fromSource(ConstData.layerViewVert);
		pipe.fragmentShader = kha.graphics4.FragmentShader.fromSource(ConstData.layerViewFrag);
		var vs = new kha.graphics4.VertexStructure();
		vs.add("pos", kha.graphics4.VertexData.Float3);
		vs.add("tex", kha.graphics4.VertexData.Float2);
		vs.add("col", kha.graphics4.VertexData.Float4);
		pipe.inputLayout = [vs];
		pipe.blendSource = kha.graphics4.BlendingFactor.BlendOne;
		pipe.blendDestination = kha.graphics4.BlendingFactor.BlendZero;
		pipe.colorWriteMaskAlpha = false;
		pipe.compile();

		var t = Reflect.copy(arm.App.theme);
		t.ELEMENT_H = 18;
		t.BUTTON_H = 16;
		var scale = armory.data.Config.raw.window_scale;
		ui = new Zui({font: arm.App.font, theme: t, color_wheel: arm.App.color_wheel, scaleFactor: scale});
		ui.scrollEnabled = false;

		notifyOnRender2D(render);
		notifyOnUpdate(update);
	}

	function render(g:kha.graphics2.Graphics) {
		ww = Std.int(iron.App.w()) + UITrait.inst.toolbarw;
		var lay = App.C.ui_layout;
		wx = lay == 0 ? Std.int(iron.App.w()) : UITrait.inst.windowW;
		wx += UITrait.inst.toolbarw;
		wy = UITrait.inst.headerh * 2;

		if (!show) return;
		if (kha.System.windowWidth() == 0 || kha.System.windowHeight() == 0) return;

		if (UITrait.inst.pdirty >= 0) hwnd.redraws = 2; // Paint was active

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
			var l = UITrait.inst.selectedLayer;
			var tex = texType == 0 ? l.texpaint : texType == 1 ? l.texpaint_nor : l.texpaint_pack;
			if (UITrait.inst.selectedLayerIsMask) tex = l.texpaint_mask;
	 		ui.g.drawScaledImage(tex, tx, ty, tw, tw);
			ui.g.pipeline = null;

			// UV map
			if (uvmapShow) {
				ui.g.drawScaledImage(UVUtil.uvmap, tx, ty, tw, tw);
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
			h.text = l.name;
			l.name = ui.textInput(h, "", Right);
			if (h.changed) UITrait.inst.hwnd.redraws = 2;
			ui.t.ACCENT_COL = ACCENT_COL;
			ui.t.BUTTON_H = BUTTON_H;
			ui.t.ELEMENT_H = ELEMENT_H;
			ui.fontSize = FONT_SIZE;

			// Controls
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
		ui.end();
		g.begin(false);
	}

	function update() {
		var m = iron.system.Input.getMouse();
		var headerh = ui.ELEMENT_H() * 1.4;
		UITrait.inst.paint2d = false;

		if (!arm.App.uienabled ||
			!show ||
			m.x + App.x() < wx ||
			m.x + App.x() > wx + ww ||
			m.y + App.y() < wy + headerh ||
			m.y + App.y() > wy + wh) return;
		
		if (m.down("right")) {
			panX += m.movementX;
			panY += m.movementY;
		}
		if (m.wheelDelta != 0) {
			panScale -= m.wheelDelta / 10;
			if (panScale < 0.1) panScale = 0.1;
			if (panScale > 3.0) panScale = 3.0;
		}

		if (m.down("left")) {
			UITrait.inst.paint2d = true;
		}
	}
}
