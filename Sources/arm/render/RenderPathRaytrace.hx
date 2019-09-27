package arm.render;

import iron.RenderPath;
import iron.Scene;

#if kha_direct3d12

class RenderPathRaytrace {

	public static var frame = 0.0;
	public static var ready = false;
	static var f32 = new kha.arrays.Float32Array(20);
	static var helpMat = iron.math.Mat4.identity();
	static var vb:kha.arrays.Float32Array;
	static var ib:kha.arrays.Uint32Array;
	static var bsobol:kha.Image;
	static var bscramble:kha.Image;
	static var brank:kha.Image;

	static function init() {
		iron.data.Data.getBlob("raytrace_brute.cso", function(shader:kha.Blob) {
			buildData();

			var layer = Context.layer;
			var savedEnvmap = Scene.active.world.probe.radiance;

			Krom.raytraceInit(
				shader.bytes.getData(), untyped vb.buffer, untyped ib.buffer, iron.App.w(), iron.App.h(),
				layer.texpaint.renderTarget_, layer.texpaint_nor.renderTarget_, layer.texpaint_pack.renderTarget_,
				savedEnvmap.texture_, bsobol.renderTarget_, bscramble.renderTarget_, brank.renderTarget_);
		});
	}

	public static function commands() {
		if (!ready) { ready = true; init(); return; }
		var cam = Scene.active.camera;
		var ct = cam.transform;
		helpMat.setFrom(cam.V);
		helpMat.multmat(cam.P);
		helpMat.getInverse(helpMat);
		f32[0] = ct.worldx();
		f32[1] = ct.worldy();
		f32[2] = ct.worldz();
		f32[3] = frame;
		frame = (Std.int(frame) % 4) + 1;
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

		var path = RenderPathDeferred.path;
		var framebuffer = path.renderTargets.get("taa").image;

		Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

		Context.ddirty = 1;
		// Context.ddirty--;
		Context.pdirty--;
		Context.rdirty--;
	}

	public static function initBake() {
		iron.data.Data.getBlob("raytrace_bake.cso", function(shader:kha.Blob) {
			buildData();

			var path = RenderPathDeferred.path;
			var baketex0 = path.renderTargets.get("baketex0").image;
			var baketex1 = path.renderTargets.get("baketex1").image;
			var baketex2 = path.renderTargets.get("baketex2").image;
			var savedEnvmap = Scene.active.world.probe.radiance;
			var layer = Context.layer;

			Krom.raytraceInit(
				shader.bytes.getData(), untyped vb.buffer, untyped ib.buffer, layer.texpaint.width, layer.texpaint.height,
				baketex0.renderTarget_, baketex1.renderTarget_, baketex2.renderTarget_,
				savedEnvmap.texture_, bsobol.renderTarget_, bscramble.renderTarget_, brank.renderTarget_);
		});
	}

	public static function commandsBake() {
		if (!ready) { ready = true; initBake(); return; }
		f32[0] = frame;
		frame += 1.0;

		var path = RenderPathDeferred.path;
		var baketex2 = path.renderTargets.get("baketex2").image;

		Krom.raytraceDispatchRays(baketex2.renderTarget_, f32.buffer);

		Context.ddirty = 1;
		// Context.ddirty--;
		Context.pdirty--;
		// Context.rdirty--;
		Context.rdirty = 2;
	}

	static function buildData() {
		if (Context.mergedObject == null) arm.util.MeshUtil.mergeMesh();
		var mo = Context.mergedObject;
		var sc = mo.transform.scale;
		var md = mo.data;
		var geom = md.geom;
		var count = Std.int(geom.positions.length / 4);
		vb = new kha.arrays.Float32Array(count * 8);
		for (i in 0...count) {
			vb[i * 8    ] = (geom.positions[i * 4    ] / 32767);
			vb[i * 8 + 1] = (geom.positions[i * 4 + 1] / 32767);
			vb[i * 8 + 2] = (geom.positions[i * 4 + 2] / 32767);
			vb[i * 8 + 3] =  geom.normals  [i * 2    ] / 32767;
			vb[i * 8 + 4] =  geom.normals  [i * 2 + 1] / 32767;
			vb[i * 8 + 5] =  geom.positions[i * 4 + 3] / 32767;
			vb[i * 8 + 6] = (geom.uvs[i * 2    ] / 32767);
			vb[i * 8 + 7] = (geom.uvs[i * 2 + 1] / 32767);
		}

		var indices = geom.indices[0];
		ib = new kha.arrays.Uint32Array(indices.length);
		for (i in 0...indices.length) ib[i] = indices[i];

		var bnoise_sobol = Scene.active.embedded.get("bnoise_sobol.png");
		var bnoise_scramble = Scene.active.embedded.get("bnoise_scramble.png");
		var bnoise_rank = Scene.active.embedded.get("bnoise_rank.png");

		var path = RenderPathDeferred.path;
		bsobol = path.renderTargets.get("bsobol").image;
		bscramble = path.renderTargets.get("bscramble").image;
		brank = path.renderTargets.get("brank").image;

		bsobol.g2.begin();
		bsobol.g2.drawImage(bnoise_sobol, 0, 0);
		bsobol.g2.end();
		bscramble.g2.begin();
		bscramble.g2.drawImage(bnoise_scramble, 0, 0);
		bscramble.g2.end();
		brank.g2.begin();
		brank.g2.drawImage(bnoise_rank, 0, 0);
		brank.g2.end();
	}
}

#end
