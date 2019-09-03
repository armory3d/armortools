package arm.render;

import iron.RenderPath;
import iron.Scene;

#if kha_direct3d12

class RenderPathRaytrace {

	public static var frame = 1.0;
	public static var ready = false;
	static var f32 = new kha.arrays.Float32Array(20);
	static var helpMat = iron.math.Mat4.identity();

	public static function init() {
		iron.data.Data.getBlob("raytrace.cso", function(shader:kha.Blob) {
			if (Context.mergedObject == null) arm.util.MeshUtil.mergeMesh();
			var mo = Context.mergedObject;
			var sc = mo.transform.scale;
			var md = mo.data;
			var geom = md.geom;
			var count = Std.int(geom.positions.length / 4);
			var vb = new kha.arrays.Float32Array((count + 4) * 8);
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

			vb[count * 8    ] = -1; // Light
			vb[count * 8 + 1] =  0.25;
			vb[count * 8 + 2] =  1;
			vb[count * 8 + 3] =  0;
			vb[count * 8 + 4] =  0;
			vb[count * 8 + 5] = -1;
			vb[count * 8 + 6] =  0;
			vb[count * 8 + 7] =  0;

			vb[count * 8 + 8] =  1;
			vb[count * 8 + 9] = -0.25;
			vb[count * 8 + 10] =  1;
			vb[count * 8 + 11] =  0;
			vb[count * 8 + 12] =  0;
			vb[count * 8 + 13] = -1;
			vb[count * 8 + 14] =  0;
			vb[count * 8 + 15] =  0;

			vb[count * 8 + 16] = -1;
			vb[count * 8 + 17] = -0.25;
			vb[count * 8 + 18] =  1;
			vb[count * 8 + 19] =  0;
			vb[count * 8 + 20] =  0;
			vb[count * 8 + 21] = -1;
			vb[count * 8 + 22] =  0;
			vb[count * 8 + 23] =  0;

			vb[count * 8 + 24] =  1;
			vb[count * 8 + 25] =  0.25;
			vb[count * 8 + 26] =  1;
			vb[count * 8 + 27] =  0;
			vb[count * 8 + 28] =  0;
			vb[count * 8 + 29] = -1;
			vb[count * 8 + 30] =  0;
			vb[count * 8 + 31] =  0;

			var indices = geom.indices[0];
			var ib = new kha.arrays.Uint32Array(indices.length + 6);
			for (i in 0...indices.length) ib[i] = indices[i];

			ib[indices.length    ] = count    ; // Light
			ib[indices.length + 1] = count + 1;
			ib[indices.length + 2] = count + 2;
			ib[indices.length + 3] = count    ;
			ib[indices.length + 4] = count + 3;
			ib[indices.length + 5] = count + 1;

			var layer = Context.layer;
			var savedEnvmap = Scene.active.world.probe.radiance;

			Krom.raytraceInit(
				shader.bytes.getData(), vb.buffer, ib.buffer, iron.App.w(), iron.App.h(),
				layer.texpaint.renderTarget_, layer.texpaint_nor.renderTarget_, layer.texpaint_pack.renderTarget_,
				savedEnvmap.texture_);
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
		frame += 1.0;
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
			if (Context.mergedObject == null) arm.util.MeshUtil.mergeMesh();
			var mo = Context.mergedObject;
			var sc = mo.transform.scale;
			var md = mo.data;
			var geom = md.geom;
			var count = Std.int(geom.positions.length / 4);
			var vb = new kha.arrays.Float32Array((count + 4) * 8);
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
			var ib = new kha.arrays.Uint32Array(indices.length + 6);
			for (i in 0...indices.length) ib[i] = indices[i];

			var path = RenderPathDeferred.path;
			var baketex0 = path.renderTargets.get("baketex0").image;
			var baketex1 = path.renderTargets.get("baketex1").image;
			var baketex2 = path.renderTargets.get("baketex2").image;
			var savedEnvmap = Scene.active.world.probe.radiance;
			var layer = Context.layer;

			Krom.raytraceInit(
				shader.bytes.getData(), vb.buffer, ib.buffer, layer.texpaint.width, layer.texpaint.height,
				baketex0.renderTarget_, baketex1.renderTarget_, baketex2.renderTarget_,
				savedEnvmap.texture_);
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
}

#end
