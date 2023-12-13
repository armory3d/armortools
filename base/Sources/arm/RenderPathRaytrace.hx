package arm;

import iron.System;
import iron.Input;
import iron.RenderPath;
import iron.Scene;
import iron.Mat4;
import iron.Data;

#if (krom_direct3d12 || krom_vulkan || krom_metal)

class RenderPathRaytrace {

	public static var frame = 0;
	public static var ready = false;
	public static var dirty = 0;
	public static var uvScale = 1.0;
	public static var path: RenderPath;
	static var first = true;
	public static var f32 = new js.lib.Float32Array(24);
	static var helpMat = Mat4.identity();
	static var vb_scale = 1.0;
	static var vb: VertexBuffer;
	static var ib: IndexBuffer;

	public static var lastEnvmap: Image = null;
	public static var isBake = false;

	#if krom_direct3d12
	public static inline var ext = ".cso";
	#elseif krom_metal
	public static inline var ext = ".metal";
	#else
	public static inline var ext = ".spirv";
	#end

	#if is_lab
	static var lastTexpaint: Image = null;
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

		if (!Context.raw.envmapLoaded) {
			Context.loadEnvmap();
			Context.updateEnvmap();
		}

		var probe = Scene.active.world.probe;
		var savedEnvmap = Context.raw.showEnvmapBlur ? probe.radianceMipmaps[0] : Context.raw.savedEnvmap;

		if (lastEnvmap != savedEnvmap) {
			lastEnvmap = savedEnvmap;

			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");

			var l = Base.flatten(true);
			Krom.raytraceSetTextures(l.texpaint, l.texpaint_nor, l.texpaint_pack, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}

		#if is_lab
		var l = Base.flatten(true);
		if (l.texpaint != lastTexpaint) {
			lastTexpaint = l.texpaint;

			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");

			Krom.raytraceSetTextures(l.texpaint, l.texpaint_nor, l.texpaint_pack, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
		}
		#end

		if (Context.raw.pdirty > 0 || dirty > 0) {
			Base.flatten(true);
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
		#if krom_metal
		// frame = (frame % (16)) + 1; // _PAINT
		frame = frame + 1; // _RENDER
		#else
		frame = (frame % 4) + 1; // _PAINT
		// frame = frame + 1; // _RENDER
		#end
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
		#if is_lab
		f32[22] *= Scene.active.meshes[0].data.scaleTex;
		#end

		var framebuffer = path.renderTargets.get("buf").image;
		Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

		if (Context.raw.ddirty == 1 || Context.raw.pdirty == 1) {
			#if krom_metal
			Context.raw.rdirty = 128;
			#else
			Context.raw.rdirty = 4;
			#end
		}
		Context.raw.ddirty--;
		Context.raw.pdirty--;
		Context.raw.rdirty--;

		// Context.raw.ddirty = 1; // _RENDER
	}

	public static function raytraceInit(shaderName: String, build = true) {
		if (first) {
			first = false;
			Scene.active.embedData("bnoise_sobol.k", function() {});
			Scene.active.embedData("bnoise_scramble.k", function() {});
			Scene.active.embedData("bnoise_rank.k", function() {});
		}

		Data.getBlob(shaderName, function(shader: js.lib.ArrayBuffer) {
			if (build) buildData();
			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");
			Krom.raytraceInit(shader, vb.buffer_, ib.buffer_, vb_scale);
		});
	}

	static function buildData() {
		if (Context.raw.mergedObject == null) UtilMesh.mergeMesh();
		#if is_paint
		var mo = !Context.layerFilterUsed() ? Context.raw.mergedObject : Context.raw.paintObject;
		#else
		var mo = Scene.active.meshes[0];
		#end
		var md = mo.data;
		var mo_scale = mo.transform.scale.x; // Uniform scale only
		vb_scale = md.scalePos * mo_scale;
		if (mo.parent != null) vb_scale *= mo.parent.transform.scale.x;
		vb = md.vertexBuffer;
		ib = md.indexBuffers[0];
	}

	public static function draw(useLiveLayer: Bool) {
		var isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		if (Context.raw.ddirty > 1 || Context.raw.pdirty > 0 || isLive) frame = 0;

		#if krom_metal
		// Delay path tracing additional samples while painting
		var down = Input.getMouse().down() || Input.getPen().down();
		if (Context.inViewport() && down) frame = 0;
		#end

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
