package arm.render;

import iron.RenderPath;
import iron.Scene;
import arm.ui.UISidebar;
import arm.ui.UIHeader;
import arm.node.MakeMaterial;
import arm.Enums;

#if (kha_direct3d12 || kha_vulkan)

class RenderPathRaytrace {

	public static var frame = 0;
	public static var raysPix = 0;
	public static var raysSec = 0;
	public static var ready = false;
	public static var dirty = 0;
	static var path: RenderPath;
	static var first = true;
	static var f32 = new kha.arrays.Float32Array(24);
	static var helpMat = iron.math.Mat4.identity();
	static var vb_scale = 1.0;
	static var vb: kha.graphics4.VertexBuffer;
	static var ib: kha.graphics4.IndexBuffer;
	static var raysTimer = 0.0;
	static var raysCounter = 0;
	static var lastLayer: kha.Image = null;
	static var lastEnvmap: kha.Image = null;
	static var isBake = false;
	static var lastBake = 0;

	#if kha_direct3d12
	static inline var ext = ".cso";
	#else
	static inline var ext = ".spirv";
	#end

	public static function init(_path: RenderPath) {
		path = _path;
	}

	public static function commands() {
		if (!ready || isBake) {
			ready = true;
			isBake = false;
			var mode = Context.pathTraceMode == TraceCore ? "core" : "full";
			raytraceInit("raytrace_brute_" + mode + ext);
			lastEnvmap = null;
		}

		if (!Context.envmapLoaded) Context.loadEnvmap();
		var probe = Scene.active.world.probe;
		var savedEnvmap = Context.showEnvmapBlur ? probe.radianceMipmaps[0] : Context.savedEnvmap;
		var isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		var materialSpace = UIHeader.inst.worktab.position == SpaceMaterial;
		var layer = (isLive || materialSpace) ? RenderPathPaint.liveLayer : Context.layer;
		if (lastEnvmap != savedEnvmap) {
			lastEnvmap = savedEnvmap;
			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");
			Layers.makeExportImg();
			Layers.makeTempImg();
			flattenLayers();
			Krom.raytraceSetTextures(Layers.expa.renderTarget_, Layers.expb.renderTarget_, Layers.expc.renderTarget_, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		if (Context.pdirty > 0 || dirty > 0) {
			flattenLayers();
		}

		var cam = Scene.active.camera;
		var ct = cam.transform;
		helpMat.setFrom(cam.V);
		helpMat.multmat(cam.P);
		helpMat.getInverse(helpMat);
		f32[0] = ct.worldx();
		f32[1] = ct.worldy();
		f32[2] = ct.worldz();
		f32[3] = frame;
		frame = (frame % 4) + 1; // _PAINT
		// frame = frame + 1; // _RENDER
		f32[4] = helpMat._00;
		f32[5] = helpMat._01;
		f32[6] = helpMat._02;
		f32[7] = helpMat._03;
		f32[8] = helpMat._10;
		f32[9] = helpMat._11;
		f32[10] = helpMat._12;
		f32[11] = helpMat._13;
		f32[12] = helpMat._20;
		f32[13] = helpMat._21;
		f32[14] = helpMat._22;
		f32[15] = helpMat._23;
		f32[16] = helpMat._30;
		f32[17] = helpMat._31;
		f32[18] = helpMat._32;
		f32[19] = helpMat._33;
		f32[20] = Scene.active.world.probe.raw.strength;
		if (!Context.showEnvmap) f32[20] = -f32[20];
		f32[21] = Context.envmapAngle;
		f32[22] = Project.layers.length;

		var framebuffer = path.renderTargets.get("buf").image;
		Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

		if (Context.ddirty == 1 || Context.pdirty == 1) Context.rdirty = 4;
		Context.ddirty--;
		Context.pdirty--;
		Context.rdirty--;

		// Context.ddirty = 1; // _RENDER
	}

	static function flattenLayers() {
		var l = Project.layers[0];
		path.setTarget("expa", ["expb", "expc"]);
		path.bindTarget("texpaint" + l.id, "tex0");
		path.bindTarget("texpaint_nor" + l.id, "tex1");
		path.bindTarget("texpaint_pack" + l.id, "tex2");
		path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");

		var l0 = { texpaint: Layers.expa, texpaint_nor: Layers.expb, texpaint_pack: Layers.expc, texpaint_mask: l.texpaint_mask };
		if (l0.texpaint_mask != null) {
			Layers.applyMask(untyped l0);
		}

		if (Project.layers.length > 1) {
			for (i in 1...Project.layers.length) {
				Layers.mergeLayer(untyped l0, Project.layers[i], true);
			}
		}
	}

	public static function commandsBake() {
		if (!ready || !isBake || lastBake != Context.bakeType) {
			var rebuild = !(ready && isBake && lastBake != Context.bakeType);
			lastBake = Context.bakeType;
			ready = true;
			isBake = true;
			lastEnvmap = null;
			lastLayer = null;

			if (path.renderTargets.get("baketex0") != null) {
				path.renderTargets.get("baketex0").image.unload();
				path.renderTargets.get("baketex1").image.unload();
				path.renderTargets.get("baketex2").image.unload();
			}

			{
				var t = new RenderTargetRaw();
				t.name = "baketex0";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "baketex1";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "baketex2";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA64"; // Match raytrace_target format
				path.createRenderTarget(t);
			}

			var _bakeType = Context.bakeType;
			Context.bakeType = BakeInit;
			MakeMaterial.parsePaintMaterial();
			path.setTarget("baketex0");
			path.clearTarget(0x00000000); // Pixels with alpha of 0.0 are skipped during raytracing
			for (i in 0...4) { // Jitter
				path.setTarget("baketex0", ["baketex1"]);
				path.drawMeshes("paint");
			}
			Context.bakeType = _bakeType;
			function _next() {
				MakeMaterial.parsePaintMaterial();
			}
			App.notifyOnNextFrame(_next);

			raytraceInit(getBakeShaderName(), rebuild);

			return;
		}

		if (!Context.envmapLoaded) Context.loadEnvmap();
		var probe = Scene.active.world.probe;
		var savedEnvmap = Context.showEnvmapBlur ? probe.radianceMipmaps[0] : Context.savedEnvmap;
		if (lastEnvmap != savedEnvmap || lastLayer != Context.layer.texpaint) {
			lastEnvmap = savedEnvmap;
			lastLayer = Context.layer.texpaint;

			var baketex0 = path.renderTargets.get("baketex0").image;
			var baketex1 = path.renderTargets.get("baketex1").image;
			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");
			var texpaint_undo = RenderPath.active.renderTargets.get("texpaint_undo" + History.undoI).image;
			Krom.raytraceSetTextures(baketex0.renderTarget_, baketex1.renderTarget_, texpaint_undo.renderTarget_, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		if (Context.brushTime > 0) {
			Context.pdirty = 2;
			Context.rdirty = 2;
		}

		if (Context.pdirty > 0) {
			f32[0] = frame++;
			f32[1] = Context.bakeAoStrength;
			f32[2] = Context.bakeAoRadius;
			f32[3] = Context.bakeAoOffset;
			f32[4] = Scene.active.world.probe.raw.strength;
			f32[5] = Context.bakeUpAxis;
			f32[6] = Context.envmapAngle;

			var framebuffer = path.renderTargets.get("baketex2").image;
			Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

			path.setTarget("texpaint" + Context.layer.id);
			path.bindTarget("baketex2", "tex");
			path.drawShader("shader_datas/copy_pass/copy_pass");

			raysPix = frame * 64;
			raysCounter += 64;
			raysTimer += iron.system.Time.realDelta;
			if (raysTimer >= 1) {
				raysSec = raysCounter;
				raysTimer = 0;
				raysCounter = 0;
			}
			UIHeader.inst.headerHandle.redraws = 2;
		}
		else {
			frame = 0;
			raysTimer = 0;
			raysCounter = 0;
		}
	}

	static function raytraceInit(shaderName: String, build = true) {
		if (first) {
			first = false;
			Scene.active.embedData("bnoise_sobol.k", function() {});
			Scene.active.embedData("bnoise_scramble.k", function() {});
			Scene.active.embedData("bnoise_rank.k", function() {});
		}

		iron.data.Data.getBlob(shaderName, function(shader: kha.Blob) {
			if (build) buildData();
			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");
			Krom.raytraceInit(shader.bytes.getData(), untyped vb.buffer, untyped ib.buffer, vb_scale);
		});
	}

	static function buildData() {
		if (Context.mergedObject == null) arm.util.MeshUtil.mergeMesh();
		var mo = Context.layerFilter == 0 ? Context.mergedObject : Context.paintObject;
		var md = mo.data;
		var geom = md.geom;
		var mo_scale = mo.transform.scale.x; // Uniform scale only
		vb_scale = mo.parent.transform.scale.x * md.scalePos * mo_scale;
		vb = geom.vertexBuffer;
		ib = geom.indexBuffers[0];
	}

	static function getBakeShaderName(): String {
		return
			Context.bakeType == BakeAO  		? "raytrace_bake_ao" + ext :
			Context.bakeType == BakeLightmap 	? "raytrace_bake_light" + ext :
			Context.bakeType == BakeBentNormal  ? "raytrace_bake_bent" + ext :
												  "raytrace_bake_thick" + ext;
	}

	public static function draw() {
		var isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		if (Context.ddirty > 1 || Context.pdirty > 0 || isLive) frame = 0;

		commands();

		path.setTarget("buf");
		path.drawMeshes("overlay");
		path.setTarget("buf");
		Inc.drawCompass(path.currentG);
		path.setTarget("taa");
		path.bindTarget("buf", "tex");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
		path.setTarget("");
		path.bindTarget("taa", "tex");
		path.drawShader("shader_datas/copy_pass/copy_pass");
		if (Config.raw.brush_3d) {
			RenderPathPaint.commandsCursor();
		}
	}
}

#end
