
class RenderPathPreview {

	static init = () => {

		{
			let t = RenderTarget.create();
			t.name = "texpreview";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "texpreview_icon";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}

		RenderPath.createDepthBuffer("mmain", "DEPTH24");

		{
			let t = RenderTarget.create();
			t.name = "mtex";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			///if krom_opengl
			t.depth_buffer = "mmain";
			///end
			RenderPath.createRenderTarget(t);
		}

		{
			let t = RenderTarget.create();
			t.name = "mgbuffer0";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			t.depth_buffer = "mmain";
			RenderPath.createRenderTarget(t);
		}

		{
			let t = RenderTarget.create();
			t.name = "mgbuffer1";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}

		{
			let t = RenderTarget.create();
			t.name = "mgbuffer2";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			RenderPath.createRenderTarget(t);
		}
	}

	static commandsPreview = () => {
		RenderPath.setTarget("mgbuffer2");
		RenderPath.clearTarget(0xff000000);

		///if (krom_metal)
		let clearColor = 0xffffffff;
		///else
		let clearColor: Null<i32> = null;
		///end

		RenderPath.setTarget("mgbuffer0");
		RenderPath.clearTarget(clearColor, 1.0);
		RenderPath.setTarget("mgbuffer0", ["mgbuffer1", "mgbuffer2"]);
		RenderPath.drawMeshes("mesh");

		// Deferred light
		RenderPath.setTarget("mtex");
		RenderPath.bindTarget("_mmain", "gbufferD");
		RenderPath.bindTarget("mgbuffer0", "gbuffer0");
		RenderPath.bindTarget("mgbuffer1", "gbuffer1");
		{
			RenderPath.bindTarget("empty_white", "ssaotex");
		}
		RenderPath.drawShader("shader_datas/deferred_light/deferred_light");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPath.setDepthFrom("mtex", "mgbuffer0"); // Bind depth for world pass
		///end

		RenderPath.setTarget("mtex"); // Re-binds depth
		RenderPath.drawSkydome("shader_datas/world_pass/world_pass");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPath.setDepthFrom("mtex", "mgbuffer1"); // Unbind depth
		///end

		let framebuffer = "texpreview";
		let selectedMat = Context.raw.material;
		RenderPath.renderTargets.get("texpreview").image = selectedMat.image;
		RenderPath.renderTargets.get("texpreview_icon").image = selectedMat.imageIcon;

		RenderPath.setTarget(framebuffer);
		RenderPath.bindTarget("mtex", "tex");
		RenderPath.drawShader("shader_datas/compositor_pass/compositor_pass");

		RenderPath.setTarget("texpreview_icon");
		RenderPath.bindTarget("texpreview", "tex");
		RenderPath.drawShader("shader_datas/supersample_resolve/supersample_resolve");
	}

	static commandsDecal = () => {
		RenderPath.setTarget("gbuffer2");
		RenderPath.clearTarget(0xff000000);

		///if (krom_metal)
		let clearColor = 0xffffffff;
		///else
		let clearColor: Null<i32> = null;
		///end

		RenderPath.setTarget("gbuffer0");
		RenderPath.clearTarget(clearColor, 1.0);
		RenderPath.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		RenderPath.drawMeshes("mesh");

		// Deferred light
		RenderPath.setTarget("tex");
		RenderPath.bindTarget("_main", "gbufferD");
		RenderPath.bindTarget("gbuffer0", "gbuffer0");
		RenderPath.bindTarget("gbuffer1", "gbuffer1");
		{
			RenderPath.bindTarget("empty_white", "ssaotex");
		}
		RenderPath.drawShader("shader_datas/deferred_light/deferred_light");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPath.setDepthFrom("tex", "gbuffer0"); // Bind depth for world pass
		///end

		RenderPath.setTarget("tex");
		RenderPath.drawSkydome("shader_datas/world_pass/world_pass");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		RenderPath.setDepthFrom("tex", "gbuffer1"); // Unbind depth
		///end

		let framebuffer = "texpreview";
		RenderPath.renderTargets.get("texpreview").image = Context.raw.decalImage;

		RenderPath.setTarget(framebuffer);

		RenderPath.bindTarget("tex", "tex");
		RenderPath.drawShader("shader_datas/compositor_pass/compositor_pass");
	}
}
