package arm.render;

import iron.RenderPath;
import arm.util.RenderUtil;
import arm.ui.UIHeader;
import arm.Enums;

@:access(iron.RenderPath)
class RenderPathPreview {

	public static var path: RenderPath;

	public static function init(_path: RenderPath) {
		path = _path;

		{
			var t = new RenderTargetRaw();
			t.name = "texpreview";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpreview_icon";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}

		{
			path.createDepthBuffer("mmain", "DEPTH24");

			var t = new RenderTargetRaw();
			t.name = "mtex";
			t.width = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.height = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			#if kha_opengl
			t.depth_buffer = "mmain";
			#end
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "mgbuffer0";
			t.width = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.height = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			t.depth_buffer = "mmain";
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "mgbuffer1";
			t.width = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.height = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		{
			var t = new RenderTargetRaw();
			t.name = "mgbuffer2";
			t.width = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.height = Std.int(RenderUtil.matPreviewSize * 2.0);
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}

		//
		// path.loadShader("shader_datas/deferred_light/deferred_light");
		// path.loadShader("shader_datas/world_pass/world_pass");
		// path.loadShader("shader_datas/compositor_pass/compositor_pass");
		// path.loadShader("shader_datas/supersample_resolve/supersample_resolve");
		//
	}

	public static function commandsPreview() {
		path.setTarget("mgbuffer2");
		path.clearTarget(0xff000000);

		#if (kha_metal)
		var clearColor = 0xffffffff;
		#else
		var clearColor: Null<Int> = null;
		#end

		path.setTarget("mgbuffer0");
		path.clearTarget(clearColor, 1.0);
		path.setTarget("mgbuffer0", ["mgbuffer1", "mgbuffer2"]);
		path.drawMeshes("mesh");

		// ---
		// Deferred light
		// ---
		path.setTarget("mtex");
		path.bindTarget("_mmain", "gbufferD");
		path.bindTarget("mgbuffer0", "gbuffer0");
		path.bindTarget("mgbuffer1", "gbuffer1");
		{
			path.bindTarget("empty_white", "ssaotex");
		}
		path.drawShader("shader_datas/deferred_light/deferred_light");

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("mtex", "mgbuffer0"); // Bind depth for world pass
		#end

		path.setTarget("mtex"); // Re-binds depth
		path.drawSkydome("shader_datas/world_pass/world_pass");

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("mtex", "mgbuffer1"); // Unbind depth
		#end

		var framebuffer = "texpreview";
		var selectedMat = Context.material;
		RenderPath.active.renderTargets.get("texpreview").image = selectedMat.image;
		RenderPath.active.renderTargets.get("texpreview_icon").image = selectedMat.imageIcon;

		path.setTarget(framebuffer);
		path.bindTarget("mtex", "tex");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");

		path.setTarget("texpreview_icon");
		path.bindTarget("texpreview", "tex");
		path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
	}

	public static function commandsDecal() {
		path.setTarget("gbuffer2");
		path.clearTarget(0xff000000);

		#if (kha_metal)
		var clearColor = 0xffffffff;
		#else
		var clearColor: Null<Int> = null;
		#end

		path.setTarget("gbuffer0");
		path.clearTarget(clearColor, 1.0);
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		path.drawMeshes("mesh");

		// ---
		// Deferred light
		// ---
		path.setTarget("tex");
		path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		{
			path.bindTarget("empty_white", "ssaotex");
		}
		path.drawShader("shader_datas/deferred_light/deferred_light");

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("tex", "gbuffer0"); // Bind depth for world pass
		#end

		path.setTarget("tex");
		path.drawSkydome("shader_datas/world_pass/world_pass");

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("tex", "gbuffer1"); // Unbind depth
		#end

		var framebuffer = "texpreview";
		RenderPath.active.renderTargets.get("texpreview").image = Context.decalImage;

		path.setTarget(framebuffer);

		path.bindTarget("tex", "tex");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
	}
}
