package arm.render;

import kha.System;
import iron.RenderPath;
import iron.Scene;
#if arm_painter
import arm.ui.UISidebar;
#end

class RenderPathDeferred {

	public static var path: RenderPath;

	#if rp_voxelao
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
			t.format = "RGBA64"; // Match raytrace_target format
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

		path.loadShader("world_pass/world_pass/world_pass");
		path.loadShader("deferred_light/deferred_light/deferred_light");
		path.loadShader("shader_datas/compositor_pass/compositor_pass");
		path.loadShader("shader_datas/copy_pass/copy_pass");
		path.loadShader("shader_datas/copy_pass/copyR8_pass");
		//path.loadShader("shader_datas/copy_pass/copyD32_pass");
		path.loadShader("shader_datas/smaa_edge_detect/smaa_edge_detect");
		path.loadShader("shader_datas/smaa_blend_weight/smaa_blend_weight");
		path.loadShader("shader_datas/smaa_neighborhood_blend/smaa_neighborhood_blend");
		path.loadShader("shader_datas/taa_pass/taa_pass");
		path.loadShader("shader_datas/supersample_resolve/supersample_resolve");

		#if arm_world
		{
			path.loadShader("water_pass/water_pass/water_pass");
			Scene.active.embedData("water_base.k", function() {});
			Scene.active.embedData("water_detail.k", function() {});
			Scene.active.embedData("water_foam.k", function() {});
			Scene.active.embedData("water_foam.k", function() {});
			Scene.active.embedData("clouds_base.raw", function() {});
			Scene.active.embedData("clouds_detail.raw", function() {});
			Scene.active.embedData("clouds_map.k", function() {});
		}
		#end

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
		#if rp_voxelao
		{
			Inc.initGI();
			path.loadShader("deferred_light/deferred_light/deferred_light_voxel");
		}
		#end

		#if arm_painter
		RenderPathPaint.init(path);
		RenderPathPreview.init(path);
		#end

		#if (kha_direct3d12 || kha_vulkan)
		RenderPathRaytrace.init(path);
		#end
	}

	@:access(iron.RenderPath)
	public static function commands() {

		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		#if arm_painter

		Inc.beginSplit();

		if (Inc.isCached()) return;

		// Match projection matrix jitter
		var skipTaa = Context.splitView;
		if (!skipTaa) {
			@:privateAccess Scene.active.camera.frame = RenderPathDeferred.taaFrame;
			@:privateAccess Scene.active.camera.projectionJitter();
		}
		Scene.active.camera.buildMatrix();

		RenderPathPaint.begin();

		drawSplit();
		#end // arm_painter

		drawGbuffer();

		#if arm_painter
		RenderPathPaint.draw();
		#end

		#if (kha_direct3d12 || kha_vulkan)
		if (Context.viewportMode == ViewPathTrace) {
			RenderPathRaytrace.draw();
			return;
		}
		#end

		drawDeferred();

		#if arm_painter
		RenderPathPaint.end();
		Inc.endSplit();
		#end

		taaFrame++;
	}

	public static function drawDeferred() {
		#if arm_painter
		var cameraType = Context.cameraType;
		var ddirty = Context.ddirty;
		#else
		var cameraType = CameraPerspective;
		var ddirty = 2;
		#end

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
		#if rp_voxelao
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
				var res = Inc.getVoxelRes();
				var voxtex = voxels;

				path.clearImage(voxtex, 0x00000000);
				path.setTarget("");
				path.setViewport(res, res);
				path.bindTarget(voxtex, "voxels");
				#if arm_painter
				if (arm.node.MaterialBuilder.heightUsed) {
					var tid = Project.layers[0].id;
					path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
				}
				#end
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
		#if rp_voxelao
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
			path.drawShader("deferred_light/deferred_light/deferred_light_voxel") :
			path.drawShader("deferred_light/deferred_light/deferred_light");

		#if arm_world
		{
			#if arm_creator
			var waterPass = Project.waterPass;
			#else
			var waterPass = true;
			#end

			if (waterPass) {
				path.setTarget("buf");
				path.bindTarget("tex", "tex");
				path.drawShader("shader_datas/copy_pass/copy_pass");
				path.setTarget("tex");
				path.bindTarget("_main", "gbufferD");
				path.bindTarget("buf", "tex");
				path.drawShader("water_pass/water_pass/water_pass");
			}
		}
		#end

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("tex", "gbuffer0"); // Bind depth for world pass
		#end

		path.setTarget("tex");
		path.drawSkydome("world_pass/world_pass/world_pass");

		#if (kha_direct3d11 || kha_direct3d12 || kha_metal || kha_vulkan)
		path.setDepthFrom("tex", "gbuffer1"); // Unbind depth
		#end

		if (Config.raw.rp_bloom != false) {

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
			path.bindTarget("tex", "tex");
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

			path.setTarget("tex");
			path.bindTarget("bloomtex2", "tex");
			path.drawShader("shader_datas/blur_gaus_pass/blur_gaus_pass_y_blend");
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
		#if arm_painter
		Inc.drawCompass(currentG);
		#end

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

		#if arm_painter
		var skipTaa = Context.splitView;
		#else
		var skipTaa = false;
		#end

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
		path.setTarget("gbuffer2");
		path.clearTarget(0xff000000);
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		var currentG = path.currentG;
		#if arm_painter
		RenderPathPaint.bindLayers();
		#end
		path.drawMeshes("mesh");
		#if arm_painter
		RenderPathPaint.unbindLayers();
		#end
		LineDraw.render(currentG);
	}

	static function drawSplit() {
		if (Context.splitView) {
			if (Context.pdirty > 0) {
				var cam = Scene.active.camera;

				Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[Context.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();

				drawGbuffer();

				#if (kha_direct3d12 || kha_vulkan)
				Context.viewportMode == ViewPathTrace ? RenderPathRaytrace.draw() : drawDeferred();
				#else
				drawDeferred();
				#end

				Context.viewIndex = Context.viewIndex == 0 ? 1 : 0;
				cam.transform.setMatrix(arm.plugin.Camera.inst.views[Context.viewIndex]);
				cam.buildMatrix();
				cam.buildProjection();
			}
		}
	}
}
