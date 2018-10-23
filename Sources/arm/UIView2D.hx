package arm;

import zui.*;

@:access(zui.Zui)
class UIView2D extends iron.Trait {

	public static var inst:UIView2D;

	public var show = false;
	public var wx:Int;
	public var wy:Int;
	public var ww:Int;

	var panX = 0.0;
	var panY = 0.0;
	var panScale = 1.0;

	var pipe:kha.graphics4.PipelineState;
	var texType = 0;
	var uvmap:kha.Image = null;
	var uvmapShow = false;
	public var uvmapCached = false;
	public var ui:Zui;

	public function new() {
		super();
		inst = this;

		var frag = "
#version 330
uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;
void main() {
	vec4 texcolor = texture(tex, texCoord) * color;
	FragColor = texcolor;
}
		";
		var vert = "
#version 330
in vec3 vertexPosition;
in vec2 texPosition;
in vec4 vertexColor;
uniform mat4 projectionMatrix;
out vec2 texCoord;
out vec4 color;
void main() {
	gl_Position = projectionMatrix * vec4(vertexPosition, 1.0);
	texCoord = texPosition;
	color = vertexColor;
}
		";

		pipe = new kha.graphics4.PipelineState();
		pipe.fragmentShader = kha.graphics4.FragmentShader.fromSource(frag);
		pipe.vertexShader = kha.graphics4.VertexShader.fromSource(vert);
		var vs = new kha.graphics4.VertexStructure();
		vs.add("vertexPosition", kha.graphics4.VertexData.Float3);
		vs.add("texPosition", kha.graphics4.VertexData.Float2);
		vs.add("vertexColor", kha.graphics4.VertexData.Float4);
		pipe.inputLayout = [vs];
		pipe.compile();

		var t = Reflect.copy(arm.App.theme);
		t.ELEMENT_H = 18;
		t.BUTTON_H = 16;
		var scale = armory.data.Config.raw.window_scale;
		ui = new Zui({font: arm.App.font, theme: t, color_wheel: arm.App.color_wheel, scaleFactor: scale});
		ui.scrollEnabled = false;

		notifyOnRender2D(render2D);
		notifyOnUpdate(update);
	}

	public var hwnd = Id.handle();

	function render2D(g:kha.graphics2.Graphics) {
		ww = Std.int(iron.App.w());
		var lay = UITrait.inst.apconfig.ui_layout;
		wx = lay == 0 ? Std.int(iron.App.w()) : UITrait.inst.windowW;
		wy = 0;

		if (!show) return;
		if (arm.App.realw() == 0 || arm.App.realh() == 0) return;

		if (UITrait.inst.paintDirty()) hwnd.redraws = 2;

		var tw = iron.App.w() * 0.95;
		var tx = iron.App.w() / 2 - tw / 2;
		var ty = iron.App.h() / 2 - tw / 2;

		tx += panX;
		ty += panY;
		tw *= panScale;

		g.end();
		
		// Cache grid
		if (UINodes.inst.grid == null) UINodes.inst.drawGrid();

		// Cache UV map
		if (uvmap == null && uvmapShow) {
			uvmap = kha.Image.createRenderTarget(2048, 2048);
		}
		if (!uvmapCached && uvmapShow) {
			uvmapCached = true;
			var mesh = UITrait.inst.currentObject.data.raw;
			var texa = mesh.vertex_arrays[2].values;
			var inda = mesh.index_arrays[0].values;
			uvmap.g2.begin(true, 0x00000000);
			uvmap.g2.color = 0xffffffff;
			for (i in 0...Std.int(inda.length / 3)) {
				var x1 = texa[inda[i * 3 + 0] * 2 + 0] * uvmap.width;
				var x2 = texa[inda[i * 3 + 1] * 2 + 0] * uvmap.width;
				var x3 = texa[inda[i * 3 + 2] * 2 + 0] * uvmap.width;
				var y1 = texa[inda[i * 3 + 0] * 2 + 1] * uvmap.width;
				var y2 = texa[inda[i * 3 + 1] * 2 + 1] * uvmap.width;
				var y3 = texa[inda[i * 3 + 2] * 2 + 1] * uvmap.width;
				uvmap.g2.drawLine(x1, y1, x2, y2);
				uvmap.g2.drawLine(x2, y2, x3, y3);
				uvmap.g2.drawLine(x3, y3, x1, y1);
			}
			uvmap.g2.end();
		}
		
		ui.begin(g);
		if (ui.window(hwnd, wx, wy, ww, iron.App.h())) {

			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(UINodes.inst.grid, (panX * panScale) % 40 - 40, (panY * panScale) % 40 - 40);

			// Texture
			ui.g.pipeline = pipe;
			var l = UITrait.inst.selectedLayer;
			var tex = texType == 0 ? l.texpaint : texType == 1 ? l.texpaint_nor : l.texpaint_pack;
	 		ui.g.drawScaledImage(tex, tx, ty, tw, tw);
			ui.g.pipeline = null;

			// UV map
			if (uvmapShow) {
				ui.g.drawScaledImage(uvmap, tx, ty, tw, tw);
			}

			// Controls
			ui.g.color = ui.t.WINDOW_BG_COL;
			ui.g.fillRect(0, 0, ww, 24);
			ui.g.color = 0xffffffff;
			ui._x = 3;
			ui._y = 3;
			ui._w = 105;
			texType = ui.combo(Id.handle({position: texType}), ["Base", "Normal", "ORM"], "Texture");
			ui._x += 105 + 3;
			ui._y = 3;
			uvmapShow = ui.check(Id.handle({selected: uvmapShow}), "UV Map");
			ui._x += 105 + 3;
			ui._y = 3;
		}
		ui.end();
		g.begin(false);
	}

	function update() {
		if (!arm.App.uienabled) return;
		var m = iron.system.Input.getMouse();
		if (!show) return;
		if (m.x + App.x() < wx || m.x + App.x() > wx + ww) return;
		
		if (m.down("right")) {
			panX += m.movementX;
			panY += m.movementY;
		}
		if (m.wheelDelta != 0) {
			panScale -= m.wheelDelta / 10;
			if (panScale < 0.1) panScale = 0.1;
			if (panScale > 3.0) panScale = 3.0;
		}
	}
}
