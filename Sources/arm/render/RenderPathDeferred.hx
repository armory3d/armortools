package arm.render;

import kha.System;
import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;
import arm.node.MakeMesh;
import arm.Enums;

class RenderPathDeferred {

	public static var path: RenderPath;

	#if rp_voxels
	static var voxels = "voxels";
	static var voxelsLast = "voxels";
	public static var voxelFrame = 0;
	public static var voxelFreq = 6; // Revoxelizing frequency
	#end
	public static var taaFrame = 0;

	public static function init(_path: RenderPath) {

		path = _path;

		#if kha_metal
		{
			path.loadShader("clear_pass/clear_pass/clear_pass_color_depth_r8");
			path.loadShader("clear_pass/clear_pass/clear_pass_color_depth_rgba32");
			path.loadShader("clear_pass/clear_pass/clear_pass_color_depth_rgba64");
			path.loadShader("clear_pass/clear_pass/clear_pass_color_r8");
			path.loadShader("clear_pass/clear_pass/clear_pass_color_rgba32");
			path.loadShader("clear_pass/clear_pass/clear_pass_color_rgba64");
			path.loadShader("clear_pass/clear_pass/clear_pass_depth_r8");
			path.loadShader("clear_pass/clear_pass/clear_pass_depth_rgba32");
			path.loadShader("clear_pass/clear_pass/clear_pass_depth_rgba64");
			path.clearShader = "clear_pass/clear_pass/clear_pass";
		}
		#end

		path.createDepthBuffer("main", "DEPTH24");

		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer0";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			t.depth_buffer = "main";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer1";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "gbuffer2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "tex";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA64";
			t.scale = Inc.getSuperSampling();
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
			#if (kha_direct3d12 || kha_vulkan)
			// Match raytrace_target format
			// Will cause "The render target format in slot 0 does not match that specified by the current pipeline state"
			t.format = "RGBA64";
			#else
			t.format = "RGBA32";
			#end
			t.scale = Inc.getSuperSampling();
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
			t.name = "taa";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "taa2";
			t.width = 0;
			t.height = 0;
			t.format = "RGBA32";
			t.scale = Inc.getSuperSampling();
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

		path.loadShader("shader_datas/world_pass/world_pass");
		path.loadShader("shader_datas/deferred_light/deferred_light");
		path.loadShader("shader_datas/compositor_pass/compositor_pass");
		path.loadShader("shader_datas/copy_pass/copy_pass");
		path.loadShader("shader_datas/copy_pass/copyR8_pass");
		//path.loadShader("shader_datas/copy_pass/copyD32_pass");
		path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		path.loadShader("shader_datas/taa_pass/taa_pass");
		path.loadShader("shader_datas/supersample_resolve/supersample_resolve");

		#if (rp_motionblur == "Camera")
		{
			path.loadShader("shader_datas/motion_blur_pass/motion_blur_pass");
		}
		#end
		#if (rp_motionblur == "Object")
		{
			path.loadShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
		}
		#end
		#if rp_voxels
		{
			Inc.initGI();
			path.loadShader("shader_datas/deferred_light/deferred_light_voxel");
		}
		#end

		RenderPathPaint.init(path);
		RenderPathPreview.init(path);

		#if (kha_direct3d12 || kha_vulkan)
		RenderPathRaytrace.init(path);
		#end
	}

	@:access(iron.RenderPath)
	public static function commands() {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		Inc.beginSplit();
		if (Inc.isCached()) return;

		// Match projection matrix jitter
		var skipTaa = Context.splitView || ((Context.tool == ToolClone || Context.tool == ToolBlur) && Context.pdirty > 0);
		@:privateAccess Scene.active.camera.frame = skipTaa ? 0 : RenderPathDeferred.taaFrame;
		@:privateAccess Scene.active.camera.projectionJitter();
		Scene.active.camera.buildMatrix();

		RenderPathPaint.begin();
		drawSplit();
		drawGbuffer();
		RenderPathPaint.draw();

		#if (kha_direct3d12 || kha_vulkan)
		if (Context.viewportMode == ViewPathTrace) {
			var useLiveLayer = arm.ui.UIHeader.inst.worktab.position == SpaceMaterial;
			RenderPathRaytrace.draw(useLiveLayer);
			return;
		}
		#end

		drawDeferred();
		RenderPathPaint.end();
		Inc.end();
		taaFrame++;
	}

	public static function drawDeferred() {
		var cameraType = Context.cameraType;
		var ddirty = Context.ddirty;

		var ssgi = Config.raw.rp_ssgi != false && cameraType == CameraPerspective;
		if (ssgi && ddirty > 0 && taaFrame > 0) {
			if (path.renderTargets.get("singlea") == null) {
				{
					var t = new RenderTargetRaw();
					t.name = "singlea";
					t.width = 0;
					t.height = 0;
					t.format = "R8";
					t.scale = Inc.getSuperSampling();
					path.createRenderTarget(t);
				}
				{
					var t = new RenderTargetRaw();
					t.name = "singleb";
					t.width = 0;
					t.height = 0;
					t.format = "R8";
					t.scale = Inc.getSuperSampling();
					path.createRenderTarget(t);
				}
				path.loadShader("shader_datas/ssgi_pass/ssgi_pass");
				path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_x");
				path.loadShader("shader_datas/blur_edge_pass/blur_edge_pass_y");
			}

			path.setTarget("singlea");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/ssgi_pass/ssgi_pass");

			path.setTarget("singleb");
			path.bindTarget("singlea", "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_x");

			path.setTarget("singlea");
			path.bindTarget("singleb", "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_edge_pass/blur_edge_pass_y");
		}

		// Voxels
		#if rp_voxels
		if (Config.raw.rp_gi != false)
		{
			var voxelize = path.voxelize() && ddirty > 0 && taaFrame > 0;

			#if arm_voxelgi_temporal
			voxelize = ++voxelFrame % voxelFreq == 0;

			if (voxelize) {
				voxels = voxels == "voxels" ? "voxelsB" : "voxels";
				voxelsLast = voxels == "voxels" ? "voxelsB" : "voxels";
			}
			#end

			if (voxelize) {
				var res = 256;
				var voxtex = voxels;

				path.clearImage(voxtex, 0x00000000);
				path.setTarget("");
				path.setViewport(res, res);
				path.bindTarget(voxtex, "voxels");
				if (arm.node.MakeMaterial.heightUsed) {
					var tid = Project.layers[0].id;
					path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
				}
				path.drawMeshes("voxel");
				path.generateMipmaps(voxels);
			}
		}
		#end

		// ---
		// Deferred light
		// ---
		path.setTarget("tex");
		path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		var ssgi = Config.raw.rp_ssgi != false && cameraType == CameraPerspective;
		if (ssgi && taaFrame > 0) {
			path.bindTarget("singlea", "ssaotex");
		}
		else {
			path.bindTarget("empty_white", "ssaotex");
		}
		var voxelao_pass = false;
		#if rp_voxels
		if (Config.raw.rp_gi != false)
		{
			voxelao_pass = true;
			path.bindTarget(voxels, "voxels");
			#if arm_voxelgi_temporal
			{
				path.bindTarget(voxelsLast, "voxelsLast");
			}
			#end
		}
		#end

		voxelao_pass ?
			path.drawShader("shader_datas/deferred_light/deferred_light_voxel") :
			path.drawShader("shader_datas/deferred_light/deferred_light");

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("tex", "gbuffer0"); // Bind depth for world pass
		#end

		path.setTarget("tex");
		path.drawSkydome("shader_datas/world_pass/world_pass");

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("tex", "gbuffer1"); // Unbind depth
		#end

		if (Config.raw.rp_bloom != false) {
			commandsBloom();
		}

		if (Config.raw.rp_ssr != false) {
			if (@:privateAccess path.cachedShaderContexts.get("shader_datas/ssr_pass/ssr_pass") == null) {
				{
					var t = new RenderTargetRaw();
					t.name = "bufb";
					t.width = 0;
					t.height = 0;
					t.format = "RGBA64";
					path.createRenderTarget(t);
				}
				path.loadShader("shader_datas/ssr_pass/ssr_pass");
				path.loadShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_x");
				path.loadShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_y3_blend");
			}
			var targeta = "bufb";
			var targetb = "gbuffer1";

			path.setTarget(targeta);
			path.bindTarget("tex", "tex");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.bindTarget("gbuffer1", "gbuffer1");
			path.drawShader("shader_datas/ssr_pass/ssr_pass");

			path.setTarget(targetb);
			path.bindTarget(targeta, "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_x");

			path.setTarget("tex");
			path.bindTarget(targetb, "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/blur_adaptive_pass/blur_adaptive_pass_y3_blend");
		}

		#if ((rp_motionblur == "Camera") || (rp_motionblur == "Object"))
		{
			if (Config.raw.rp_motionblur != false) {
				path.setTarget("buf");
				path.bindTarget("tex", "tex");
				path.bindTarget("gbuffer0", "gbuffer0");
				#if (rp_motionblur == "Camera")
				{
					path.bindTarget("_main", "gbufferD");
					path.drawShader("shader_datas/motion_blur_pass/motion_blur_pass");
				}
				#else
				{
					path.bindTarget("gbuffer2", "sveloc");
					path.drawShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
				}
				#end
				path.setTarget("tex");
				path.bindTarget("buf", "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
			}
		}
		#end

		// Begin compositor
		#if rp_autoexposure
		{
			{
				var t = new RenderTargetRaw();
				t.name = "histogram";
				t.width = 1;
				t.height = 1;
				t.format = "RGBA64";
				path.createRenderTarget(t);
				path.loadShader("shader_datas/histogram_pass/histogram_pass");
			}

			path.setTarget("histogram");
			path.bindTarget("taa", "tex");
			path.drawShader("shader_datas/histogram_pass/histogram_pass");
		}
		#end

		path.setTarget("buf");
		path.bindTarget("tex", "tex");
		#if rp_autoexposure
		{
			path.bindTarget("histogram", "histogram");
		}
		#end
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
		// End compositor

		path.setTarget("buf");
		var currentG = path.currentG;
		path.drawMeshes("overlay");
		Inc.drawCompass(currentG);

		var current = taaFrame % 2 == 0 ? "bufa" : "taa2";
		var last = taaFrame % 2 == 0 ? "taa2" : "bufa";

		path.setTarget(current);
		path.clearTarget(0x00000000);
		path.bindTarget("buf", "colorTex");
		path.drawShader("shader_datas/smaa_edge_detect/smaa_edge_detect");

		path.setTarget("taa");
		path.clearTarget(0x00000000);
		path.bindTarget(current, "edgesTex");
		path.drawShader("shader_datas/smaa_blend_weight/smaa_blend_weight");

		path.setTarget(current);
		path.bindTarget("buf", "colorTex");
		path.bindTarget("taa", "blendTex");
		path.bindTarget("gbuffer2", "sveloc");
		path.drawShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");

		var skipTaa = Context.splitView;
		if (skipTaa) {
			path.setTarget("taa");
			path.bindTarget(current, "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			path.setTarget("taa");
			path.bindTarget(current, "tex");
			path.bindTarget(last, "tex2");
			path.bindTarget("gbuffer2", "sveloc");
			path.drawShader("shader_datas/taa_pass/taa_pass");
		}

		if (!Inc.ssaa4()) {
			path.setTarget("");
			path.bindTarget(taaFrame == 0 ? current : "taa", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
		}

		if (Inc.ssaa4()) {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
	}

	public static function drawGbuffer() {
		path.setTarget("gbuffer0"); // Only clear gbuffer0
		#if kha_metal
		path.clearTarget(0x00000000, 1.0);
		#else
		path.clearTarget(null, 1.0);
		#end
		if (MakeMesh.layerPassCount == 1) {
			path.setTarget("gbuffer2");
			path.clearTarget(0xff000000);
		}
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		var currentG = path.currentG;
		RenderPathPaint.bindLayers();
		path.drawMeshes("mesh");
		RenderPathPaint.unbindLayers();
		if (MakeMesh.layerPassCount > 1) {
			makeGbufferCopyTextures();
			for (i in 1...MakeMesh.layerPassCount) {
				var ping = i % 2 == 1 ? "_copy" : "";
				var pong = i % 2 == 1 ? "" : "_copy";
				if (i == MakeMesh.layerPassCount - 1) {
					path.setTarget("gbuffer2" + ping);
					path.clearTarget(0xff000000);
				}
				path.setTarget("gbuffer0" + ping, ["gbuffer1" + ping, "gbuffer2" + ping]);
				path.bindTarget("gbuffer0" + pong, "gbuffer0");
				path.bindTarget("gbuffer1" + pong, "gbuffer1");
				path.bindTarget("gbuffer2" + pong, "gbuffer2");
				RenderPathPaint.bindLayers();
				path.drawMeshes("mesh" + i);
				RenderPathPaint.unbindLayers();
			}
			if (MakeMesh.layerPassCount % 2 == 0) {
				copyToGbuffer();
			}
		}
		LineDraw.render(currentG);
	}

	public static function makeGbufferCopyTextures() {
		var copy = path.renderTargets.get("gbuffer0_copy");
		if (copy == null || copy.image.width != path.renderTargets.get("gbuffer0").image.width || copy.image.height != path.renderTargets.get("gbuffer0").image.height) {
			{
				var t = new RenderTargetRaw();
				t.name = "gbuffer0_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				t.depth_buffer = "main";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "gbuffer1_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "gbuffer2_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = Inc.getSuperSampling();
				path.createRenderTarget(t);
			}
		}
	}

	public static function copyToGbuffer() {
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		path.bindTarget("gbuffer0_copy", "tex0");
		path.bindTarget("gbuffer1_copy", "tex1");
		path.bindTarget("gbuffer2_copy", "tex2");
		path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
	}

	static function drawSplit() {
		if (Context.splitView && !Context.paint2dView) {
			#if (kha_metal || krom_android)
			Context.ddirty = 2;
			#else
			Context.ddirty = 1;
			#end
			var cam = Scene.active.camera;

			Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
			cam.transform.setMatrix(arm.Camera.inst.views[Context.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();

			drawGbuffer();

			#if (kha_direct3d12 || kha_vulkan)
			var useLiveLayer = arm.ui.UIHeader.inst.worktab.position == SpaceMaterial;
			Context.viewportMode == ViewPathTrace ? RenderPathRaytrace.draw(useLiveLayer) : drawDeferred();
			#else
			drawDeferred();
			#end

			Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
			cam.transform.setMatrix(arm.Camera.inst.views[Context.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();
		}
	}

	public static function commandsBloom(tex = "tex") {
		if (path.renderTargets.get("bloomtex") == null) {
			{
				var t = new RenderTargetRaw();
				t.name = "bloomtex";
				t.width = 0;
				t.height = 0;
				t.scale = 0.25;
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "bloomtex2";
				t.width = 0;
				t.height = 0;
				t.scale = 0.25;
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			path.loadShader("shader_datas/bloom_pass/bloom_pass");
			path.loadShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");
			path.loadShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");
			path.loadShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y_blend");
		}

		path.setTarget("bloomtex");
		path.bindTarget(tex, "tex");
		path.drawShader("shader_datas/bloom_pass/bloom_pass");

		path.setTarget("bloomtex2");
		path.bindTarget("bloomtex", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

		path.setTarget("bloomtex");
		path.bindTarget("bloomtex2", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");

		path.setTarget("bloomtex2");
		path.bindTarget("bloomtex", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

		path.setTarget("bloomtex");
		path.bindTarget("bloomtex2", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");

		path.setTarget("bloomtex2");
		path.bindTarget("bloomtex", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

		path.setTarget("bloomtex");
		path.bindTarget("bloomtex2", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y");

		path.setTarget("bloomtex2");
		path.bindTarget("bloomtex", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_x");

		path.setTarget(tex);
		path.bindTarget("bloomtex2", "tex");
		path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y_blend");
	}
}
