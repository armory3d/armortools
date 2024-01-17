
class RenderPathPreview {

	static path: RenderPath;

	static init = (_path: RenderPath) => {
		RenderPathPreview.path = _path;

		{
			let t = new RenderTargetRaw();
			t.name = "texpreview";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPathPreview.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpreview_icon";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPathPreview.path.createRenderTarget(t);
		}

		RenderPathPreview.path.createDepthBuffer("mmain", "DEPTH24");

		{
			let t = new RenderTargetRaw();
			t.name = "mtex";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			///if krom_opengl
			t.depth_buffer = "mmain";
			///end
			RenderPathPreview.path.createRenderTarget(t);
		}

		{
			let t = new RenderTargetRaw();
			t.name = "mgbuffer0";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			t.depth_buffer = "mmain";
			RenderPathPreview.path.createRenderTarget(t);
		}

		{
			let t = new RenderTargetRaw();
			t.name = "mgbuffer1";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathPreview.path.createRenderTarget(t);
		}

		{
			let t = new RenderTargetRaw();
			t.name = "mgbuffer2";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPathPreview.path.createRenderTarget(t);
		}
	}

	static commandsPreview = () => {
		RenderPathPreview.path.setTarget("mgbuffer2");
		RenderPathPreview.path.clearTarget(0xff000000);

		///if (krom_metal)
		let clearColor = 0xffffffff;
		///else
		let clearColor: Null<i32> = null;
		///end

		RenderPathPreview.path.setTarget("mgbuffer0");
		RenderPathPreview.path.clearTarget(clearColor, 1.0);
		RenderPathPreview.path.setTarget("mgbuffer0", ["mgbuffer1", "mgbuffer2"]);
		RenderPathPreview.path.drawMeshes("mesh");

		// Deferred light
		RenderPathPreview.path.setTarget("mtex");
		RenderPathPreview.path.bindTarget("_mmain", "gbufferD");
		RenderPathPreview.path.bindTarget("mgbuffer0", "gbuffer0");
		RenderPathPreview.path.bindTarget("mgbuffer1", "gbuffer1");
		{
			RenderPathPreview.path.bindTarget("empty_white", "ssaotex");
		}
		RenderPathPreview.path.drawShader("shader_datas/deferred_light/deferred_light");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPathPreview.path.setDepthFrom("mtex", "mgbuffer0"); // Bind depth for world pass
		///end

		RenderPathPreview.path.setTarget("mtex"); // Re-binds depth
		RenderPathPreview.path.drawSkydome("shader_datas/world_pass/world_pass");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPathPreview.path.setDepthFrom("mtex", "mgbuffer1"); // Unbind depth
		///end

		let framebuffer = "texpreview";
		let selectedMat = Context.raw.material;
		RenderPath.active.renderTargets.get("texpreview").image = selectedMat.image;
		RenderPath.active.renderTargets.get("texpreview_icon").image = selectedMat.imageIcon;

		RenderPathPreview.path.setTarget(framebuffer);
		RenderPathPreview.path.bindTarget("mtex", "tex");
		RenderPathPreview.path.drawShader("shader_datas/compositor_pass/compositor_pass");

		RenderPathPreview.path.setTarget("texpreview_icon");
		RenderPathPreview.path.bindTarget("texpreview", "tex");
		RenderPathPreview.path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
	}

	static commandsDecal = () => {
		RenderPathPreview.path.setTarget("gbuffer2");
		RenderPathPreview.path.clearTarget(0xff000000);

		///if (krom_metal)
		let clearColor = 0xffffffff;
		///else
		let clearColor: Null<i32> = null;
		///end

		RenderPathPreview.path.setTarget("gbuffer0");
		RenderPathPreview.path.clearTarget(clearColor, 1.0);
		RenderPathPreview.path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		RenderPathPreview.path.drawMeshes("mesh");

		// Deferred light
		RenderPathPreview.path.setTarget("tex");
		RenderPathPreview.path.bindTarget("_main", "gbufferD");
		RenderPathPreview.path.bindTarget("gbuffer0", "gbuffer0");
		RenderPathPreview.path.bindTarget("gbuffer1", "gbuffer1");
		{
			RenderPathPreview.path.bindTarget("empty_white", "ssaotex");
		}
		RenderPathPreview.path.drawShader("shader_datas/deferred_light/deferred_light");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPathPreview.path.setDepthFrom("tex", "gbuffer0"); // Bind depth for world pass
		///end

		RenderPathPreview.path.setTarget("tex");
		RenderPathPreview.path.drawSkydome("shader_datas/world_pass/world_pass");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPathPreview.path.setDepthFrom("tex", "gbuffer1"); // Unbind depth
		///end

		let framebuffer = "texpreview";
		RenderPath.active.renderTargets.get("texpreview").image = Context.raw.decalImage;

		RenderPathPreview.path.setTarget(framebuffer);

		RenderPathPreview.path.bindTarget("tex", "tex");
		RenderPathPreview.path.drawShader("shader_datas/compositor_pass/compositor_pass");
	}
}
