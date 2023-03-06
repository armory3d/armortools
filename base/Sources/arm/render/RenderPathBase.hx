package arm.render;

import kha.System;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.math.Quat;
import iron.object.MeshObject;
import iron.system.Input;
import iron.RenderPath;
import iron.Scene;
import arm.shader.MakeMesh;

class RenderPathBase {

	public static var taaFrame = 0;
	static var path: RenderPath;
	static var superSample = 1.0;
	static var lastX = -1.0;
	static var lastY = -1.0;
	static var bloomMipmaps: Array<RenderTarget>;
	static var bloomCurrentMip = 0;
	static var bloomSampleScale: Float;
	#if rp_voxels
	static var voxelsCreated = false;
	#end

	public static function init(_path: RenderPath) {
		path = _path;
		superSample = Config.raw.rp_supersample;
	}

	#if rp_voxels
	public static function initVoxels(targetName = "voxels") {
		if (Config.raw.rp_gi != true || voxelsCreated) return;
		voxelsCreated = true;

		{
			var t = new RenderTargetRaw();
			t.name = targetName;
			t.format = "R8";
			t.width = 256;
			t.height = 256;
			t.depth = 256;
			t.is_image = true;
			t.mipmaps = true;
			path.createRenderTarget(t);
		}
	}
	#end

	public static function applyConfig() {
		if (superSample != Config.raw.rp_supersample) {
			superSample = Config.raw.rp_supersample;
			for (rt in path.renderTargets) {
				if (rt.raw.width == 0 && rt.raw.scale != null) {
					rt.raw.scale = superSample;
				}
			}
			path.resize();
		}
		#if rp_voxels
		if (!voxelsCreated) initVoxels();
		#end
	}

	public static inline function getSuperSampling(): Float {
		return superSample;
	}

	public static function drawCompass(currentG: kha.graphics4.Graphics) {
		if (Context.raw.showCompass) {
			var scene = Scene.active;
			var cam = scene.camera;
			var compass: MeshObject = cast scene.getChild(".Compass");

			var _visible = compass.visible;
			var _parent = compass.parent;
			var _loc = compass.transform.loc;
			var _rot = compass.transform.rot;
			var crot = cam.transform.rot;
			var ratio = iron.App.w() / iron.App.h();
			var _P = cam.P;
			cam.P = Mat4.ortho(-8 * ratio, 8 * ratio, -8, 8, -2, 2);
			compass.visible = true;
			compass.parent = cam;
			compass.transform.loc = new Vec4(7.4 * ratio, 7.0, -1);
			compass.transform.rot = new Quat(-crot.x, -crot.y, -crot.z, crot.w);
			compass.transform.scale.set(0.4, 0.4, 0.4);
			compass.transform.buildMatrix();
			compass.frustumCulling = false;
			compass.render(currentG, "overlay", []);

			cam.P = _P;
			compass.visible = _visible;
			compass.parent = _parent;
			compass.transform.loc = _loc;
			compass.transform.rot = _rot;
			compass.transform.buildMatrix();
		}
	}

	public static function begin() {
		// Begin split
		if (Context.raw.splitView && !Context.raw.paint2dView) {
			if (Context.raw.viewIndexLast == -1 && Context.raw.viewIndex == -1) {
				// Begin split, draw right viewport first
				Context.raw.viewIndex = 1;
			}
			else {
				// Set current viewport
				Context.raw.viewIndex = Input.getMouse().viewX > arm.App.w() / 2 ? 1 : 0;
			}

			var cam = Scene.active.camera;
			if (Context.raw.viewIndexLast > -1) {
				// Save current viewport camera
				arm.Camera.inst.views[Context.raw.viewIndexLast].setFrom(cam.transform.local);
			}

			var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;

			if (Context.raw.viewIndexLast != Context.raw.viewIndex || decal || !Config.raw.brush_3d) {
				// Redraw on current viewport change
				Context.raw.ddirty = 1;
			}

			cam.transform.setMatrix(arm.Camera.inst.views[Context.raw.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();
		}

		// Match projection matrix jitter
		var skipTaa = Context.raw.splitView || ((Context.raw.tool == ToolClone || Context.raw.tool == ToolBlur) && Context.raw.pdirty > 0);
		@:privateAccess Scene.active.camera.frame = skipTaa ? 0 : RenderPathBase.taaFrame;
		@:privateAccess Scene.active.camera.projectionJitter();
		Scene.active.camera.buildMatrix();
	}

	public static function end() {
		// End split
		Context.raw.viewIndexLast = Context.raw.viewIndex;
		Context.raw.viewIndex = -1;

		if (Context.raw.foregroundEvent && !iron.system.Input.getMouse().down()) {
			Context.raw.foregroundEvent = false;
			Context.raw.pdirty = 0;
		}

		taaFrame++;
	}

	public static inline function ssaa4(): Bool {
		return Config.raw.rp_supersample == 4;
	}

	public static function isCached(): Bool {
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return true;

		var mouse = Input.getMouse();
		var mx = lastX;
		var my = lastY;
		lastX = mouse.viewX;
		lastY = mouse.viewY;

		if (Context.raw.ddirty <= 0 && Context.raw.rdirty <= 0 && Context.raw.pdirty <= 0) {
			if (mx != lastX || my != lastY || mouse.locked) Context.raw.ddirty = 0;
			#if (kha_metal || krom_android)
			if (Context.raw.ddirty > -6) {
			#else
			if (Context.raw.ddirty > -2) {
			#end
				path.setTarget("");
				path.bindTarget("taa", "tex");
				ssaa4() ?
					path.drawShader("shader_datas/supersample_resolve/supersample_resolve") :
					path.drawShader("shader_datas/copy_pass/copy_pass");
				RenderPathPaint.commandsCursor();
				if (Context.raw.ddirty <= 0) Context.raw.ddirty--;
			}
			end();
			return true;
		}
		return false;
	}

	public static function commands(drawCommands: Void->Void) {
		if (isCached()) return;
		begin();

		RenderPathPaint.begin();
		drawSplit(drawCommands);
		drawGbuffer();
		RenderPathPaint.draw();

		#if (kha_direct3d12 || kha_vulkan)
		if (Context.raw.viewportMode == ViewPathTrace) {
			var useLiveLayer = arm.ui.UIHeader.inst.worktab.position == SpaceMaterial;
			RenderPathRaytrace.draw(useLiveLayer);
			return;
		}
		#end

		drawCommands();
		RenderPathPaint.end();
		end();
	}

	public static function drawBloom(tex = "tex") {
		if (Config.raw.rp_bloom != false) {
			if (bloomMipmaps == null) {
				bloomMipmaps = [];

				var prevScale = 1.0;
				for (i in 0...10) {
					var t = new RenderTargetRaw();
					t.name = "bloom_mip_" + i;
					t.width = 0;
					t.height = 0;
					t.scale = (prevScale *= 0.5);
					t.format = "RGBA64";
					bloomMipmaps.push(path.createRenderTarget(t));
				}

				path.loadShader("shader_datas/bloom_pass/bloom_downsample_pass");
				path.loadShader("shader_datas/bloom_pass/bloom_upsample_pass");

				iron.object.Uniforms.externalIntLinks.push(function(_, _, link: String) {
					if (link == "_bloomCurrentMip") return bloomCurrentMip;
					return null;
				});
				iron.object.Uniforms.externalFloatLinks.push(function(_, _, link: String) {
					if (link == "_bloomSampleScale") return bloomSampleScale;
					return null;
				});
			}

			var bloomRadius = 6.5;
			var minDim = Math.min(path.currentW, path.currentH);
			var logMinDim = Math.max(1.0, js.lib.Math.log2(minDim) + (bloomRadius - 8.0));
			var numMips = Std.int(logMinDim);
			bloomSampleScale = 0.5 + logMinDim - numMips;

			for (i in 0...numMips) {
				bloomCurrentMip = i;
				path.setTarget(bloomMipmaps[i].raw.name);
				path.clearTarget();
				path.bindTarget(i == 0 ? tex : bloomMipmaps[i - 1].raw.name, "tex");
				path.drawShader("shader_datas/bloom_pass/bloom_downsample_pass");
			}
			for (i in 0...numMips) {
				var mipLevel = numMips - 1 - i;
				bloomCurrentMip = mipLevel;
				path.setTarget(mipLevel == 0 ? tex : bloomMipmaps[mipLevel - 1].raw.name);
				path.bindTarget(bloomMipmaps[mipLevel].raw.name, "tex");
				path.drawShader("shader_datas/bloom_pass/bloom_upsample_pass");
			}
		}
	}

	public static function drawSplit(drawCommands: Void->Void) {
		if (Context.raw.splitView && !Context.raw.paint2dView) {
			Context.raw.ddirty = 2;
			var cam = Scene.active.camera;

			Context.raw.viewIndex = Context.raw.viewIndex == 0 ? 1 : 0;
			cam.transform.setMatrix(arm.Camera.inst.views[Context.raw.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();

			drawGbuffer();

			#if (kha_direct3d12 || kha_vulkan)
			var useLiveLayer = arm.ui.UIHeader.inst.worktab.position == SpaceMaterial;
			Context.raw.viewportMode == ViewPathTrace ? RenderPathRaytrace.draw(useLiveLayer) : drawCommands();
			#else
			drawCommands();
			#end

			Context.raw.viewIndex = Context.raw.viewIndex == 0 ? 1 : 0;
			cam.transform.setMatrix(arm.Camera.inst.views[Context.raw.viewIndex]);
			cam.buildMatrix();
			cam.buildProjection();
		}
	}

	#if rp_voxels
	public static function drawVoxels() {
		if (Config.raw.rp_gi != false) {
			var voxelize = Context.raw.ddirty > 0 && taaFrame > 0;
			if (voxelize) {
				path.clearImage("voxels", 0x00000000);
				path.setTarget("");
				path.setViewport(256, 256);
				path.bindTarget("voxels", "voxels");
				if (arm.shader.MakeMaterial.heightUsed) {
					var tid = 0; // Project.layers[0].id;
					path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
				}
				path.drawMeshes("voxel");
				path.generateMipmaps("voxels");
			}
		}
	}
	#end

	public static function initSSAO() {
		{
			var t = new RenderTargetRaw();
			t.name = "singlea";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = getSuperSampling();
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "singleb";
			t.width = 0;
			t.height = 0;
			t.format = "R8";
			t.scale = getSuperSampling();
			path.createRenderTarget(t);
		}
		path.loadShader("shader_datas/ssao_pass/ssao_pass");
		path.loadShader("shader_datas/ssao_blur_pass/ssao_blur_pass_x");
		path.loadShader("shader_datas/ssao_blur_pass/ssao_blur_pass_y");
	}

	public static function drawSSAO() {
		var ssao = Config.raw.rp_ssao != false && Context.raw.cameraType == CameraPerspective;
		if (ssao && Context.raw.ddirty > 0 && taaFrame > 0) {
			if (path.renderTargets.get("singlea") == null) {
				initSSAO();
			}

			path.setTarget("singlea");
			path.bindTarget("_main", "gbufferD");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/ssao_pass/ssao_pass");

			path.setTarget("singleb");
			path.bindTarget("singlea", "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/ssao_blur_pass/ssao_blur_pass_x");

			path.setTarget("singlea");
			path.bindTarget("singleb", "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/ssao_blur_pass/ssao_blur_pass_y");
		}
	}

	public static function drawDeferredLight() {
		path.setTarget("tex");
		path.bindTarget("_main", "gbufferD");
		path.bindTarget("gbuffer0", "gbuffer0");
		path.bindTarget("gbuffer1", "gbuffer1");
		var ssao = Config.raw.rp_ssao != false && Context.raw.cameraType == CameraPerspective;
		if (ssao && taaFrame > 0) {
			path.bindTarget("singlea", "ssaotex");
		}
		else {
			path.bindTarget("empty_white", "ssaotex");
		}

		var voxelao_pass = false;
		#if rp_voxels
		if (Config.raw.rp_gi != false) {
			voxelao_pass = true;
			path.bindTarget("voxels", "voxels");
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
	}

	public static function drawSSR() {
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
				path.loadShader("shader_datas/ssr_blur_pass/ssr_blur_pass_x");
				path.loadShader("shader_datas/ssr_blur_pass/ssr_blur_pass_y3_blend");
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
			path.drawShader("shader_datas/ssr_blur_pass/ssr_blur_pass_x");

			path.setTarget("tex");
			path.bindTarget(targetb, "tex");
			path.bindTarget("gbuffer0", "gbuffer0");
			path.drawShader("shader_datas/ssr_blur_pass/ssr_blur_pass_y3_blend");
		}
	}

	// public static function drawMotionBlur() {
	// 	if (Config.raw.rp_motionblur != false) {
	// 		path.setTarget("buf");
	// 		path.bindTarget("tex", "tex");
	// 		path.bindTarget("gbuffer0", "gbuffer0");
	// 		#if (rp_motionblur == "Camera")
	// 		{
	// 			path.bindTarget("_main", "gbufferD");
	// 			path.drawShader("shader_datas/motion_blur_pass/motion_blur_pass");
	// 		}
	// 		#else
	// 		{
	// 			path.bindTarget("gbuffer2", "sveloc");
	// 			path.drawShader("shader_datas/motion_blur_veloc_pass/motion_blur_veloc_pass");
	// 		}
	// 		#end
	// 		path.setTarget("tex");
	// 		path.bindTarget("buf", "tex");
	// 		path.drawShader("shader_datas/copy_pass/copy_pass");
	// 	}
	// }

	// public static function drawHistogram() {
	// 	{
	// 		var t = new RenderTargetRaw();
	// 		t.name = "histogram";
	// 		t.width = 1;
	// 		t.height = 1;
	// 		t.format = "RGBA64";
	// 		path.createRenderTarget(t);

	// 		path.loadShader("shader_datas/histogram_pass/histogram_pass");
	// 	}

	// 	path.setTarget("histogram");
	// 	path.bindTarget("taa", "tex");
	// 	path.drawShader("shader_datas/histogram_pass/histogram_pass");
	// }

	public static function drawTAA() {
		var current = taaFrame % 2 == 0 ? "buf2" : "taa2";
		var last = taaFrame % 2 == 0 ? "taa2" : "buf2";

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

		var skipTaa = Context.raw.splitView;
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

		if (ssaa4()) {
			path.setTarget("");
			path.bindTarget(taaFrame % 2 == 0 ? "taa2" : "taa", "tex");
			path.drawShader("shader_datas/supersample_resolve/supersample_resolve");
		}
		else {
			path.setTarget("");
			path.bindTarget(taaFrame == 0 ? current : "taa", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");
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

		var hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutDown) || iron.system.Input.getKeyboard().down("control");
		var isDecal = App.isDecalLayer();
		if (isDecal && !hide) LineDraw.render(currentG, Context.raw.layer.decalMat);
	}

	static function makeGbufferCopyTextures() {
		var copy = path.renderTargets.get("gbuffer0_copy");
		if (copy == null || copy.image.width != path.renderTargets.get("gbuffer0").image.width || copy.image.height != path.renderTargets.get("gbuffer0").image.height) {
			{
				var t = new RenderTargetRaw();
				t.name = "gbuffer0_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.getSuperSampling();
				t.depth_buffer = "main";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "gbuffer1_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.getSuperSampling();
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "gbuffer2_copy";
				t.width = 0;
				t.height = 0;
				t.format = "RGBA64";
				t.scale = RenderPathBase.getSuperSampling();
				path.createRenderTarget(t);
			}

			#if kha_metal
			// TODO: Fix depth attach for gbuffer0_copy on metal
			// Use resize to re-create buffers from scratch for now
			path.resize();
			#end
		}
	}

	static function copyToGbuffer() {
		path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
		path.bindTarget("gbuffer0_copy", "tex0");
		path.bindTarget("gbuffer1_copy", "tex1");
		path.bindTarget("gbuffer2_copy", "tex2");
		path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
	}
}
