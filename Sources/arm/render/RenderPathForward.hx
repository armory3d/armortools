package arm.render;

import kha.System;
import iron.RenderPath;

class RenderPathForward {

	public static var path:RenderPath;

	public static function init(_path:RenderPath) {
		path = _path;

		path.loadShader("world_pass/world_pass/world_pass");

		path.createDepthBuffer("main", "DEPTH24");

		var t = new RenderTargetRaw();
		t.name = "lbuffer0";
		t.width = 0;
		t.height = 0;
		t.format = "RGBA64";
		t.scale = Inc.getSuperSampling();
		t.depth_buffer = "main";
		path.createRenderTarget(t);

		path.loadShader("shader_datas/compositor_pass/compositor_pass");
		path.loadShader("shader_datas/copy_pass/copy_pass");
		path.loadShader("shader_datas/supersample_resolve/supersample_resolve");

		{
			var t = new RenderTargetRaw();
			t.name = "buf";
			t.width = 0;
			t.height = 0;
			t.format = 'RGBA32';
			t.scale = Inc.getSuperSampling();
			t.depth_buffer = "main";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "bufa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "bufb";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		path.loadShader("shader_datas/taa_pass/taa_pass");

		#if arm_painter
		RenderPathPaint.init(path);
		RenderPathPreview.init(path);
		#end

		#if kha_direct3d12
		RenderPathRaytrace.init(path);
		#end
	}

	@:access(iron.RenderPath)
	public static function commands() {

		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		path.setTarget("lbuffer0");
		path.clearTarget(null, 1.0);
		path.drawMeshes("mesh");

		path.setTarget("lbuffer0");
		path.drawSkydome("world_pass/world_pass/world_pass");

		var framebuffer = Inc.ssaa4() ? "buf" : "";

		path.setTarget("buf");
		path.bindTarget("lbuffer0", "tex");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");

		{
			path.setTarget("bufa");
			path.clearTarget(0x00000000);
			path.bindTarget("buf", "colorTex");
			path.drawShader("shader_datas/smaa_edge_detect/smaa_edge_detect");

			path.setTarget("bufb");
			path.clearTarget(0x00000000);
			path.bindTarget("bufa", "edgesTex");
			path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

			path.setTarget(framebuffer);
			path.bindTarget("buf", "colorTex");
			path.bindTarget("bufb", "blendTex");
			path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		}

		if (Inc.ssaa4()) {
			path.setTarget("");
			path.bindTarget(framebuffer, "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
	}
}
