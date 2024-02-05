
class RenderPathPreview {

	static init = () => {

		{
			let t = render_target_create();
			t.name = "texpreview";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpreview_icon";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}

		render_path_create_depth_buffer("mmain", "DEPTH24");

		{
			let t = render_target_create();
			t.name = "mtex";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			///if krom_opengl
			t.depth_buffer = "mmain";
			///end
			render_path_create_render_target(t);
		}

		{
			let t = render_target_create();
			t.name = "mgbuffer0";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			t.depth_buffer = "mmain";
			render_path_create_render_target(t);
		}

		{
			let t = render_target_create();
			t.name = "mgbuffer1";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			render_path_create_render_target(t);
		}

		{
			let t = render_target_create();
			t.name = "mgbuffer2";
			t.width = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.height = Math.floor(UtilRender.materialPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = RenderPathBase.getSuperSampling();
			render_path_create_render_target(t);
		}
	}

	static commandsPreview = () => {
		render_path_set_target("mgbuffer2");
		render_path_clear_target(0xff000000);

		///if (krom_metal)
		let clearColor = 0xffffffff;
		///else
		let clearColor: Null<i32> = null;
		///end

		render_path_set_target("mgbuffer0");
		render_path_clear_target(clearColor, 1.0);
		render_path_set_target("mgbuffer0", ["mgbuffer1", "mgbuffer2"]);
		render_path_draw_meshes("mesh");

		// Deferred light
		render_path_set_target("mtex");
		render_path_bind_target("_mmain", "gbufferD");
		render_path_bind_target("mgbuffer0", "gbuffer0");
		render_path_bind_target("mgbuffer1", "gbuffer1");
		{
			render_path_bind_target("empty_white", "ssaotex");
		}
		render_path_draw_shader("shader_datas/deferred_light/deferred_light");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		render_path_set_depth_from("mtex", "mgbuffer0"); // Bind depth for world pass
		///end

		render_path_set_target("mtex"); // Re-binds depth
		render_path_draw_skydome("shader_datas/world_pass/world_pass");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		render_path_set_depth_from("mtex", "mgbuffer1"); // Unbind depth
		///end

		let framebuffer = "texpreview";
		let selectedMat = Context.raw.material;
		render_path_render_targets.get("texpreview").image = selectedMat.image;
		render_path_render_targets.get("texpreview_icon").image = selectedMat.imageIcon;

		render_path_set_target(framebuffer);
		render_path_bind_target("mtex", "tex");
		render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");

		render_path_set_target("texpreview_icon");
		render_path_bind_target("texpreview", "tex");
		render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");
	}

	static commandsDecal = () => {
		render_path_set_target("gbuffer2");
		render_path_clear_target(0xff000000);

		///if (krom_metal)
		let clearColor = 0xffffffff;
		///else
		let clearColor: Null<i32> = null;
		///end

		render_path_set_target("gbuffer0");
		render_path_clear_target(clearColor, 1.0);
		render_path_set_target("gbuffer0", ["gbuffer1", "gbuffer2"]);
		render_path_draw_meshes("mesh");

		// Deferred light
		render_path_set_target("tex");
		render_path_bind_target("_main", "gbufferD");
		render_path_bind_target("gbuffer0", "gbuffer0");
		render_path_bind_target("gbuffer1", "gbuffer1");
		{
			render_path_bind_target("empty_white", "ssaotex");
		}
		render_path_draw_shader("shader_datas/deferred_light/deferred_light");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		render_path_set_depth_from("tex", "gbuffer0"); // Bind depth for world pass
		///end

		render_path_set_target("tex");
		render_path_draw_skydome("shader_datas/world_pass/world_pass");

		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		render_path_set_depth_from("tex", "gbuffer1"); // Unbind depth
		///end

		let framebuffer = "texpreview";
		render_path_render_targets.get("texpreview").image = Context.raw.decalImage;

		render_path_set_target(framebuffer);

		render_path_bind_target("tex", "tex");
		render_path_draw_shader("shader_datas/compositor_pass/compositor_pass");
	}
}
