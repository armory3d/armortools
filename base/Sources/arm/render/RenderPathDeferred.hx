package arm.render;

import iron.RenderPath;

class RenderPathDeferred {

	public static var path: RenderPath;

	public static function init(_path: RenderPath) {
		path = _path;
		path.createDepthBuffer("main", "DEPTH24");

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			t.depth_buffer = "main";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			#if kha_opengl
			t.depth_buffer = "main";
			#end
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "buf";
			t.width = 0;
			t.height = 0;
			#if (kha_direct3d12 || kha_vulkan)// || kha_metal)
			// Match raytrace_target format
			// Will cause "The render target format in slot 0 does not match that specified by the current pipeline state"
			t.format = "RGBA64";
			#else
			t.format = "RGBA32";
			#end
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "buf2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "taa2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "empty_white";
			t.width = 1;
			t.height = 1;
			t.format = "R8";
			var rt = new RenderTarget(t);
			var b = haxe.io.Bytes.alloc(1);
			b.set(0, 255);
			rt.image = kha.Image.fromBytes(b, t.width, t.height, kha.graphics4.TextureFormat.L8);
			path.renderTargets.set(t.name, rt);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "empty_black";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			var rt = new RenderTarget(t);
			var b = haxe.io.Bytes.alloc(4);
			b.set(0, 0);
			b.set(1, 0);
			b.set(2, 0);
			b.set(3, 0);
			rt.image = kha.Image.fromBytes(b, t.width, t.height, kha.graphics4.TextureFormat.RGBA32);
			path.renderTargets.set(t.name, rt);
		}

		#if is_sculpt
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer0_undo";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "gbufferD_undo";
			t.width = 0;
			t.height = 0;
			t.format = "R32";
			t.scale = RenderPathBase.getSuperSampling();
			path.createRenderTarget(t);
		}
		#end

		if (Config.raw.rp_ssao) {
			RenderPathBase.initSSAO();
		}

		path.loadShader("shader_datas/world_pass/world_pass");
		path.loadShader("shader_datas/deferred_light/deferred_light");
		path.loadShader("shader_datas/compositor_pass/compositor_pass");
		path.loadShader("shader_datas/copy_pass/copy_pass");
		path.loadShader("shader_datas/copy_pass/copyR8_pass");
		path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		path.loadShader("shader_datas/taa_pass/taa_pass");
		path.loadShader("shader_datas/supersample_resolve/supersample_resolve");
		// path.loadShader("shader_datas/motion_blur_pass/motion_blur_pass");
		// path.loadShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
		#if rp_voxels
		{
			RenderPathBase.initVoxels();
			path.loadShader("shader_datas/deferred_light/deferred_light_voxel");
		}
		#end

		RenderPathPaint.init(path);

		#if (is_paint || is_sculpt)
		RenderPathPreview.init(path);
		#end

		#if (kha_direct3d12 || kha_vulkan || kha_metal)
		RenderPathRaytrace.init(path);
		#end
	}

	public static function commands() {
		#if is_paint
		RenderPathPaint.liveBrushDirty();
		#end
		RenderPathBase.commands(drawDeferred);
	}

	public static function drawDeferred() {
		RenderPathBase.drawSSAO();
		#if rp_voxels
		RenderPathBase.drawVoxels();
		#end
		RenderPathBase.drawDeferredLight();
		RenderPathBase.drawBloom();
		RenderPathBase.drawSSR();
		// RenderPathBase.drawMotionBlur();
		// RenderPathBase.drawHistogram();

		path.setTarget("buf");
		path.bindTarget("tex", "tex");
		// path.bindTarget("histogram", "histogram");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");

		path.setTarget("buf");
		RenderPathBase.drawCompass(path.currentG);
		path.drawMeshes("overlay");

		RenderPathBase.drawTAA();
	}
}
