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
	static var path:RenderPath;
	static var first = true;
	static var f32 = new kha.arrays.Float32Array(24);
	static var helpMat = iron.math.Mat4.identity();
	static var vb:kha.arrays.Float32Array;
	static var ib:kha.arrays.Uint32Array;
	static var bsobol:kha.Image;
	static var bscramble:kha.Image;
	static var brank:kha.Image;
	static var raysTimer = 0.0;
	static var raysCounter = 0;
	static var lastLayer:kha.Image = null;
	static var lastEnvmap:kha.Image = null;
	static var isBake = false;
	static var lastBake = 0;

	public static function init(_path:RenderPath) {
		path = _path;
	}

	public static function commands() {
		if (!ready || isBake) {
			ready = true;
			isBake = false;
			raytraceInit("raytrace_brute.cso", iron.App.w(), iron.App.h());
			lastEnvmap = null;
			lastLayer = null;
		}

		var probe = Scene.active.world.probe;
		var savedEnvmap = UITrait.inst.showEnvmapBlur ? probe.radianceMipmaps[0] : probe.radiance;
		if (lastEnvmap != savedEnvmap) {
			lastEnvmap = savedEnvmap;
			lastLayer = null;

			var envrt = path.renderTargets.get("envrt").image;
			envrt.g2.begin(false);
			envrt.g2.drawScaledImage(savedEnvmap, 0, 0, envrt.width, envrt.height);
			envrt.g2.end();
		}

		var layer = Context.layer;
		if (lastLayer != layer.texpaint) {
			lastLayer = layer.texpaint;
			var envrt = path.renderTargets.get("envrt").image;
			Krom.raytraceSetTextures(layer.texpaint.renderTarget_, layer.texpaint_nor.renderTarget_, layer.texpaint_pack.renderTarget_, envrt.renderTarget_);
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
		if (!ready || !isBake) {
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
				t.format = "RGBA128";  // Match raytrace_target format
				path.createRenderTarget(t);
			}

			var _bakeType = UITrait.inst.bakeType;
			UITrait.inst.bakeType = -1;
			MaterialParser.parsePaintMaterial();
			for (i in 0...4) { // Jitter
				path.setTarget("baketex0", ["baketex1"]);
				path.drawMeshes("paint");
			}
			UITrait.inst.bakeType = _bakeType;
			// MaterialParser.parsePaintMaterial();

			raytraceInit(getBakeShaderName(), Config.getTextureRes(), Config.getTextureRes());

			return;
		}

		if (lastBake != UITrait.inst.bakeType) {
			lastBake = UITrait.inst.bakeType;
			raytraceInit(getBakeShaderName(), Config.getTextureRes(), Config.getTextureRes(), false);
			lastEnvmap = null;
		}

		var probe = Scene.active.world.probe;
		var savedEnvmap = UITrait.inst.showEnvmapBlur ? probe.radianceMipmaps[0] : probe.radiance;
		if (lastEnvmap != savedEnvmap || lastLayer != Context.layer.texpaint) {
			lastEnvmap = savedEnvmap;
			lastLayer = Context.layer.texpaint;

			var baketex0 = path.renderTargets.get("baketex0").image;
			var baketex1 = path.renderTargets.get("baketex1").image;
			var baketex2 = path.renderTargets.get("baketex2").image;
			var savedEnvmap = Scene.active.world.probe.radiance;
			var envrt = path.renderTargets.get("envrt").image;
			envrt.g2.begin(false);
			envrt.g2.drawScaledImage(savedEnvmap, 0, 0, envrt.width, envrt.height);
			envrt.g2.end();

			Krom.raytraceSetTextures(baketex0.renderTarget_, baketex1.renderTarget_, Context.layer.texpaint.renderTarget_, envrt.renderTarget_);
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

			var baketex2 = path.renderTargets.get("baketex2").image;
			Krom.raytraceDispatchRays(baketex2.renderTarget_, f32.buffer);

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

	static function raytraceInit(shaderName:String, targetW:Int, targetH:Int, build = true) {

		if (first) {
			Scene.active.embedData("bnoise_sobol.png", function() {});
			Scene.active.embedData("bnoise_scramble.png", function() {});
			Scene.active.embedData("bnoise_rank.png", function() {});

			{
				var t = new RenderTargetRaw();
				t.name = "bsobol";
				t.width = 256;
				t.height = 256;
				t.format = "RGBA32";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "bscramble";
				t.width = 128;
				t.height = 1024;
				t.format = "RGBA32";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "brank";
				t.width = 128;
				t.height = 1024;
				t.format = "RGBA32";
				path.createRenderTarget(t);
			}
			{
				var t = new RenderTargetRaw();
				t.name = "envrt";
				t.width = 4096;
				t.height = 2048;
				t.format = "RGBA128";
				path.createRenderTarget(t);
			}
		}

		iron.data.Data.getBlob(shaderName, function(shader:kha.Blob) {
			if (build) buildData();
			Krom.raytraceInit(
				shader.bytes.getData(), untyped vb.buffer, untyped ib.buffer, targetW, targetH,
				bsobol.renderTarget_, bscramble.renderTarget_, brank.renderTarget_);
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

		buildSobol();
	}

	static function buildSobol() {
		var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.png");
		var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.png");
		var bnoise_rank = Scene.active.embedded.get("bnoise_rank.png");

		bsobol = path.renderTargets.get("bsobol").image;
		bscramble = path.renderTargets.get("bscramble").image;
		brank = path.renderTargets.get("brank").image;

		bsobol.g2.begin(false);
		bsobol.g2.drawImage(bnoise_sobol, 0, 0);
		bsobol.g2.end();
		bscramble.g2.begin(false);
		bscramble.g2.drawImage(bnoise_scramble, 0, 0);
		bscramble.g2.end();
		brank.g2.begin(false);
		brank.g2.drawImage(bnoise_rank, 0, 0);
		brank.g2.end();
	}

	static function getBakeShaderName():String {
		return
			UITrait.inst.bakeType == 0 ? "raytrace_bake_ao.cso" :
			UITrait.inst.bakeType == 8 ? "raytrace_bake_light.cso" :
			UITrait.inst.bakeType == 9 ? "raytrace_bake_bent.cso" :
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
