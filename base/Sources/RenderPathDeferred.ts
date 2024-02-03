
class RenderPathDeferred {

	static init = () => {
		RenderPath.createDepthBuffer("main", "DEPTH24");

		{
			let t = RenderTarget.create();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			t.depth_buffer = "main";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			///if krom_opengl
			t.depth_buffer = "main";
			///end
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
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
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "buf2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "taa2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "empty_white";
			t.width = 1;
			t.height = 1;
			t.format = "R8";
			let b = new ArrayBuffer(1);
			let v = new DataView(b);
			v.setUint8(0, 255);
			t.image = Image.fromBytes(b, t.width, t.height, TextureFormat.R8);
			RenderPath.renderTargets.set(t.name, t);
		}
		{
			let t = RenderTarget.create();
			t.name = "empty_black";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			let b = new ArrayBuffer(4);
			let v = new DataView(b);
			v.setUint8(0, 0);
			v.setUint8(1, 0);
			v.setUint8(2, 0);
			v.setUint8(3, 0);
			t.image = Image.fromBytes(b, t.width, t.height, TextureFormat.RGBA32);
			RenderPath.renderTargets.set(t.name, t);
		}

		///if is_sculpt
		{
			let t = RenderTarget.create();
			t.name = "gbuffer0_undo";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "gbufferD_undo";
			t.width = 0;
			t.height = 0;
			t.format = "R32";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
		///end

		if (Config.raw.rp_ssao) {
			RenderPathBase.initSSAO();
		}

		RenderPath.loadShader("shader_datas/world_pass/world_pass");
		RenderPath.loadShader("shader_datas/deferred_light/deferred_light");
		RenderPath.loadShader("shader_datas/compositor_pass/compositor_pass");
		RenderPath.loadShader("shader_datas/copy_pass/copy_pass");
		RenderPath.loadShader("shader_datas/copy_pass/copyR8_pass");
		RenderPath.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		RenderPath.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		RenderPath.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		RenderPath.loadShader("shader_datas/taa_pass/taa_pass");
		RenderPath.loadShader("shader_datas/supersample_resolve/supersample_resolve");
		// RenderPath.loadShader("shader_datas/motion_blur_pass/motion_blur_pass");
		// RenderPath.loadShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
		///if arm_voxels
		{
			RenderPathBase.initVoxels();
			RenderPath.loadShader("shader_datas/deferred_light/deferred_light_voxel");
		}
		///end

		RenderPathPaint.init();

		///if (is_paint || is_sculpt)
		RenderPathPreview.init();
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.init();
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

		RenderPath.setTarget("buf");
		RenderPath.bindTarget("tex", "tex");
		// RenderPath.bindTarget("histogram", "histogram");
		RenderPath.drawShader("shader_datas/compositor_pass/compositor_pass");

		RenderPath.setTarget("buf");
		RenderPathBase.drawCompass(RenderPath.currentG);
		RenderPath.drawMeshes("overlay");

		RenderPathBase.drawTAA();
	}
}
