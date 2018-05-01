package arm;

import zui.*;

class UIView2D extends iron.Trait {

	public static var inst:UIView2D;

	public var show = false;
	public var wx:Int;
	public var wy:Int;
	public var ww:Int;

	var pipe:kha.graphics4.PipelineState;

	public function new() {
		super();
		inst = this;

		pipe = new kha.graphics4.PipelineState();
		pipe.fragmentShader = kha.Shaders.view2d_frag;
		pipe.vertexShader = kha.Shaders.view2d_vert;
		var vs = new kha.graphics4.VertexStructure();
		vs.add("vertexPosition", kha.graphics4.VertexData.Float3);
		vs.add("texPosition", kha.graphics4.VertexData.Float2);
		vs.add("vertexColor", kha.graphics4.VertexData.Float4);
		pipe.inputLayout = [vs];
		pipe.compile();

		notifyOnRender2D(render2D);
	}

	function render2D(g:kha.graphics2.Graphics) {
		ww = Std.int(iron.App.w());
		wx = Std.int(iron.App.w());
		wy = 0;

		if (!show) return;
		
		g.end();
		if (UINodes.inst.grid == null) UINodes.inst.drawGrid();
		g.begin(false);

		g.color = 0xffffffff;
		g.drawImage(UINodes.inst.grid, wx, wy);

		var tw = Std.int(iron.App.w() * 0.95);
		var tx = Std.int(iron.App.w() + (iron.App.w() - tw) / 2);
		var ty = Std.int(iron.App.h() / 2 - tw / 2);
		g.pipeline = pipe;
		g.drawScaledImage(UITrait.inst.texpaint, tx, ty, tw, tw);
		g.pipeline = null;
	}
}
