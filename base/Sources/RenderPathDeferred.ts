
class RenderPathDeferred {

	static path: RenderPath;

	static init = (_path: RenderPath) => {
		RenderPathDeferred.path = _path;
		RenderPathDeferred.path.createDepthBuffer("main", "DEPTH24");

		{
			let t = new RenderTargetRaw();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			t.depth_buffer = "main";
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			///if krom_opengl
			t.depth_buffer = "main";
			///end
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "buf";
			t.width = 0;
			t.height = 0;
			///if (krom_direct3d12 || krom_vulkan)// || krom_metal)
			// Match raytrace_target format
			// Will cause "The render target format in slot 0 does not match that specified by the current pipeline state"
			t.format = "RGBA64";
			///else
			t.format = "RGBA32";
			///end
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "buf2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "taa2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "empty_white";
			t.width = 1;
			t.height = 1;
			t.format = "R8";
			let rt = new RenderTarget(t);
			let b = new ArrayBuffer(1);
			let v = new DataView(b);
			v.setUint8(0, 255);
			rt.image = Image.fromBytes(b, t.width, t.height, TextureFormat.R8);
			RenderPathDeferred.path.renderTargets.set(t.name, rt);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "empty_black";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			let rt = new RenderTarget(t);
			let b = new ArrayBuffer(4);
			let v = new DataView(b);
			v.setUint8(0, 0);
			v.setUint8(1, 0);
			v.setUint8(2, 0);
			v.setUint8(3, 0);
			rt.image = Image.fromBytes(b, t.width, t.height, TextureFormat.RGBA32);
			RenderPathDeferred.path.renderTargets.set(t.name, rt);
		}

		///if is_sculpt
		{
			let t = new RenderTargetRaw();
			t.name = "gbuffer0_undo";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "gbufferD_undo";
			t.width = 0;
			t.height = 0;
			t.format = "R32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathDeferred.path.createRenderTarget(t);
		}
		///end

		if (Config.raw.rp_ssao) {
			RenderPathBase.initSSAO();
		}

		RenderPathDeferred.path.loadShader("shader_datas/world_pass/world_pass");
		RenderPathDeferred.path.loadShader("shader_datas/deferred_light/deferred_light");
		RenderPathDeferred.path.loadShader("shader_datas/compositor_pass/compositor_pass");
		RenderPathDeferred.path.loadShader("shader_datas/copy_pass/copy_pass");
		RenderPathDeferred.path.loadShader("shader_datas/copy_pass/copyR8_pass");
		RenderPathDeferred.path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		RenderPathDeferred.path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		RenderPathDeferred.path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		RenderPathDeferred.path.loadShader("shader_datas/taa_pass/taa_pass");
		RenderPathDeferred.path.loadShader("shader_datas/supersample_resolve/supersample_resolve");
		// RenderPathDeferred.path.loadShader("shader_datas/motion_blur_pass/motion_blur_pass");
		// RenderPathDeferred.path.loadShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
		///if arm_voxels
		{
			RenderPathBase.initVoxels();
			RenderPathDeferred.path.loadShader("shader_datas/deferred_light/deferred_light_voxel");
		}
		///end

		RenderPathPaint.init(RenderPathDeferred.path);

		///if (is_paint || is_sculpt)
		RenderPathPreview.init(RenderPathDeferred.path);
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.init(RenderPathDeferred.path);
		///end
	}

	static commands = () => {
		///if is_paint
		RenderPathPaint.liveBrushDirty();
		///end
		RenderPathBase.commands(RenderPathDeferred.drawDeferred);
	}

	static drawDeferred = () => {
		RenderPathBase.drawSSAO();
		///if arm_voxels
		RenderPathBase.drawVoxels();
		///end
		RenderPathBase.drawDeferredLight();
		RenderPathBase.drawBloom();
		RenderPathBase.drawSSR();
		// RenderPathBase.drawMotionBlur();
		// RenderPathBase.drawHistogram();

		RenderPathDeferred.path.setTarget("buf");
		RenderPathDeferred.path.bindTarget("tex", "tex");
		// RenderPathDeferred.path.bindTarget("histogram", "histogram");
		RenderPathDeferred.path.drawShader("shader_datas/compositor_pass/compositor_pass");

		RenderPathDeferred.path.setTarget("buf");
		RenderPathBase.drawCompass(RenderPathDeferred.path.currentG);
		RenderPathDeferred.path.drawMeshes("overlay");

		RenderPathBase.drawTAA();
	}
}
