package arm.render;

import iron.RenderPath;
import iron.Scene;
import arm.ui.UITrait;
import arm.node.MaterialParser;

#if kha_direct3d12

class RenderPathRaytrace {

	public static var frame = 0;
	public static var raysPix = 0;
	public static var raysSec = 0;
	public static var ready = false;
	static var path: RenderPath;
	static var first = true;
	static var f32 = new kha.arrays.Float32Array(24);
	static var helpMat = iron.math.Mat4.identity();
	static var vb: kha.arrays.Float32Array;
	static var ib: kha.arrays.Uint32Array;
	static var raysTimer = 0.0;
	static var raysCounter = 0;
	static var lastLayer: kha.Image = null;
	static var lastEnvmap: kha.Image = null;
	static var isBake = false;
	static var lastBake = 0;

	public static function init(_path: RenderPath) {
		path = _path;
	}

	public static function commands() {
		if (!ready || isBake) {
			ready = true;
			isBake = false;
			raytraceInit("raytrace_brute.cso");
			lastEnvmap = null;
			lastLayer = null;
		}

		var probe = Scene.active.world.probe;
		var savedEnvmap = UITrait.inst.showEnvmapBlur ? probe.radianceMipmaps[0] : probe.radiance;
		var layer = Context.layer;
		if (lastEnvmap != savedEnvmap || lastLayer != layer.texpaint) {
			lastEnvmap = savedEnvmap;
			lastLayer = layer.texpaint;
			var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.k");
			var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.k");
			var bnoise_rank = Scene.active.embedded.get("bnoise_rank.k");
			Krom.raytraceSetTextures(layer.texpaint.renderTarget_, layer.texpaint_nor.renderTarget_, layer.texpaint_pack.renderTarget_, savedEnvmap.texture_, bnoise_sobol.texture_, bnoise_scramble.texture_, bnoise_rank.texture_);
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
		f32[21] = UITrait.inst.showEnvmap ? 1.0 : 0.0;

		var framebuffer = path.renderTargets.get("buf").image;
		Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

		if (Context.ddirty == 1 || Context.pdirty == 1) Context.rdirty = 4;
		Context.ddirty--;
		Context.pdirty--;
		Context.rdirty--;

		// Context.ddirty = 1; // _RENDER
	}

	public static function commandsBake() {
		if (!ready || !isBake || lastBake != UITrait.inst.bakeType) {
			var rebuild = !(ready && isBake && lastBake != UITrait.inst.bakeType);
			lastBake = UITrait.inst.bakeType;
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
				t.width = Config.getTextureRes();
				t.height = Config.getTextureRes();
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "baketex1";
				t.width = Config.getTextureRes();
				t.height = Config.getTextureRes();
				t.format = "RGBA64";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "baketex2";
				t.width = Config.getTextureRes();
				t.height = Config.getTextureRes();
				t.format = "RGBA64"; // Match raytrace_target format
				path.createRenderTarget(t);
			}

			var _bakeType = UITrait.inst.bakeType;
			UITrait.inst.bakeType = BakeInit;
			MaterialParser.parsePaintMaterial();
			path.setTarget("baketex0");
			path.clearTarget(0x00000000); // Pixels with alpha of 0.0 are skipped during raytracing
			for (i in 0...4) { // Jitter
				path.setTarget("baketex0", ["baketex1"]);
				path.drawMeshes("paint");
			}
			UITrait.inst.bakeType = _bakeType;
			function _render(_) {
				MaterialParser.parsePaintMaterial();
				iron.App.removeRender(_render);
			}
			iron.App.notifyOnRender(_render);

			raytraceInit(getBakeShaderName(), rebuild);

			return;
		}

		var probe = Scene.active.world.probe;
		var savedEnvmap = UITrait.inst.showEnvmapBlur ? probe.radianceMipmaps[0] : probe.radiance;
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

		if (UITrait.inst.brushTime > 0) {
			Context.pdirty = 2;
			Context.rdirty = 2;
		}

		if (Context.pdirty > 0) {
			f32[0] = frame++;
			f32[1] = UITrait.inst.bakeAoStrength;
			f32[2] = UITrait.inst.bakeAoRadius;
			f32[3] = UITrait.inst.bakeAoOffset;
			f32[4] = Scene.active.world.probe.raw.strength;
			f32[5] = UITrait.inst.bakeUpAxis;

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
			UITrait.inst.headerHandle.redraws = 2;
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
			Krom.raytraceInit(shader.bytes.getData(), untyped vb.buffer, untyped ib.buffer);
		});
	}

	static function buildData() {
		if (Context.mergedObject == null) arm.util.MeshUtil.mergeMesh();
		var mo = Context.mergedObject;
		var sc = mo.parent.transform.scale.x * mo.data.scalePos;
		var md = mo.data;
		var geom = md.geom;
		var count = Std.int(geom.positions.length / 4);
		vb = new kha.arrays.Float32Array(count * 8);
		for (i in 0...count) {
			vb[i * 8    ] = (geom.positions[i * 4    ] * sc / 32767);
			vb[i * 8 + 1] = (geom.positions[i * 4 + 1] * sc / 32767);
			vb[i * 8 + 2] = (geom.positions[i * 4 + 2] * sc / 32767);
			vb[i * 8 + 3] =  geom.normals  [i * 2    ] / 32767;
			vb[i * 8 + 4] =  geom.normals  [i * 2 + 1] / 32767;
			vb[i * 8 + 5] =  geom.positions[i * 4 + 3] / 32767;
			vb[i * 8 + 6] = (geom.uvs[i * 2    ] / 32767);
			vb[i * 8 + 7] = (geom.uvs[i * 2 + 1] / 32767);
		}

		var indices = geom.indices[0];
		ib = new kha.arrays.Uint32Array(indices.length);
		for (i in 0...indices.length) ib[i] = indices[i];
	}

	static function getBakeShaderName(): String {
		return
			UITrait.inst.bakeType == BakeAO  		? "raytrace_bake_ao.cso" :
			UITrait.inst.bakeType == BakeLightmap 	? "raytrace_bake_light.cso" :
			UITrait.inst.bakeType == BakeBentNormal ? "raytrace_bake_bent.cso" :
													  "raytrace_bake_thick.cso";
	}

	public static function draw() {
		#if arm_painter
		if (Context.ddirty > 1) frame = 0;
		#else
		frame = 0;
		#end

		commands();

		path.setTarget("buf");
		Inc.drawCompass(path.currentG);
		path.setTarget("taa");
		path.bindTarget("buf", "tex");
		path.drawShader("shader_datas/compositor_pass/compositor_pass");
		path.setTarget("");
		path.bindTarget("taa", "tex");
		path.drawShader("shader_datas/copy_pass/copy_pass");
		#if arm_painter
		if (UITrait.inst.brush3d) {
			RenderPathPaint.commandsCursor();
		}
		#end
	}
}

#end
