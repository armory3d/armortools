package arm.render;

import iron.RenderPath;
import iron.Scene;

class RenderPathRaytrace {

	static var ready = false;
	static var frame = 0.0;
	static var f32 = new kha.arrays.Float32Array(21);
	static var helpMat = iron.math.Mat4.identity();

	public static function init() {
		iron.data.Data.getBlob("raytrace.cso", function(shader:kha.Blob) {		
			var md = Context.paintObject.data;
			var geom = md.geom;
			var count = Std.int(geom.positions.length / 4);
			var vb = new kha.arrays.Float32Array(count * 8);
			for (i in 0...count) {
				vb[i * 8    ] = (geom.positions[i * 4    ] / 32767) * md.scalePos;
				vb[i * 8 + 1] = (geom.positions[i * 4 + 1] / 32767) * md.scalePos;
				vb[i * 8 + 2] = (geom.positions[i * 4 + 2] / 32767) * md.scalePos;
				vb[i * 8 + 3] =  geom.normals  [i * 2    ] / 32767;
				vb[i * 8 + 4] =  geom.normals  [i * 2 + 1] / 32767;
				vb[i * 8 + 5] =  geom.positions[i * 4 + 3] / 32767;
				vb[i * 8 + 6] = (geom.uvs[i * 2    ] / 32767) * md.scaleTex;
				vb[i * 8 + 7] = (geom.uvs[i * 2 + 1] / 32767) * md.scaleTex;
			}
			var ib = geom.indices[0];

			untyped Krom.raytraceInit(shader.bytes.getData(), vb.buffer, ib.buffer, iron.App.w(), iron.App.h());
		});
	}

	public static function commands() {
		if (!ready) { ready = true; init(); }
		var cam = Scene.active.camera;
		var ct = cam.transform;
		helpMat.setFrom(cam.V);
		helpMat.multmat(cam.P);
		helpMat.getInverse(helpMat);
		f32[0] = ct.worldx();
		f32[1] = ct.worldy();
		f32[2] = ct.worldz();
		f32[3] = 1;
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
		f32[20] = frame;
		frame += 1.0;
		if (iron.system.Input.getMouse().started()) frame = 1.0;

		var path = RenderPathDeferred.path;
		var framebuffer = path.renderTargets.get("taa").image;

		untyped Krom.raytraceDispatchRays(framebuffer.renderTarget_, f32.buffer);

		Context.ddirty = 2;
	}
}
