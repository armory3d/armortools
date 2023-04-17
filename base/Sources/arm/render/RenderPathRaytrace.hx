package arm.render;

import iron.RenderPath;
import iron.Scene;

#if (kha_direct3d12 || kha_vulkan)

class RenderPathRaytrace {

	public static var frame = 0;
	public static var ready = false;
	public static var dirty = 0;
	public static var uvScale = 1.0;
	static var path: RenderPath;
	static var first = true;
	static var f32 = new kha.arrays.Float32Array(24);
	static var helpMat = iron.math.Mat4.identity();
	static var vb_scale = 1.0;
	static var vb: kha.graphics4.VertexBuffer;
	static var ib: kha.graphics4.IndexBuffer;

	static var lastEnvmap: kha.Image = null;
	static var isBake = false;

	#if kha_direct3d12
	public static inline var ext = ".cso";
	#else
	public static inline var ext = ".spirv";
	#end

	public static function init(_path: RenderPath) {
		path = _path;
	}

	static function commands(useLiveLayer: Bool) {
		if (!ready || isBake) {
			ready = true;
			isBake = false;
			var mode = Context.raw.pathTraceMode == TraceCore ? "core" : "full";
			raytraceInit("raytrace_brute_" + mode + ext);
			lastEnvmap = null;
		}

		if (!Context.raw.envmapLoaded) ContextBase.loadEnvmap();
		var probe = Scene.active.world.probe;
		var savedEnvmap = Context.raw.showEnvmapBlur ? probe.radianceMipmaps[0] : Context.raw.savedEnvmap;
		if (lastEnvmap != savedEnvmap) {
			lastEnvmap = savedEnvmap;
			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");
			var l = App.flatten(true);
			Krom.raytraceSetTextures(l.texpaint.renderTarget_, l.texpaint_nor.renderTarget_, l.texpaint_pack.renderTarget_, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		if (Context.raw.pdirty > 0 || dirty > 0) {
			App.flatten(true);
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
		f32[20] = Scene.active.world.probe.raw.strength * 1.5;
		if (!Context.raw.showEnvmap) f32[20] = -f32[20];
		f32[21] = Context.raw.envmapAngle;
		f32[22] = uvScale;

		var framebuffer = path.renderTargets.get("buf").image;
		Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

		if (Context.raw.ddirty == 1 || Context.raw.pdirty == 1) Context.raw.rdirty = 4;
		Context.raw.ddirty--;
		Context.raw.pdirty--;
		Context.raw.rdirty--;

		// Context.raw.ddirty = 1; // _RENDER
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
		if (Context.raw.mergedObject == null) arm.util.MeshUtil.mergeMesh();
		var mo = !Context.layerFilterUsed() ? Context.raw.mergedObject : Context.raw.paintObject;
		var md = mo.data;
		var geom = md.geom;
		var mo_scale = mo.transform.scale.x; // Uniform scale only
		vb_scale = mo.parent.transform.scale.x * md.scalePos * mo_scale;
		vb = geom.vertexBuffer;
		ib = geom.indexBuffers[0];
	}

	public static function draw(useLiveLayer: Bool) {
		var isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		if (Context.raw.ddirty > 1 || Context.raw.pdirty > 0 || isLive) frame = 0;

		commands(useLiveLayer);

		if (Config.raw.rp_bloom != false) {
			RenderPathBase.drawBloom("buf");
		}
		path.setTarget("buf");
		path.drawMeshes("overlay");
		path.setTarget("buf");
		RenderPathBase.drawCompass(path.currentG);
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
