package arm.util;

import kha.arrays.Int16Array;
import kha.arrays.Uint32Array;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.object.MeshObject;
import iron.math.Vec4;
import arm.ui.UITrait;
import arm.Tool;

class MeshUtil {

	public static function mergeMesh() {
		var vlen = 0;
		var ilen = 0;
		var maxScale = 0.0;
		var paintObjects = Project.paintObjects;
		for (i in 0...paintObjects.length) {
			vlen += paintObjects[i].data.raw.vertex_arrays[0].values.length;
			ilen += paintObjects[i].data.raw.index_arrays[0].values.length;
			if (paintObjects[i].data.scalePos > maxScale) maxScale = paintObjects[i].data.scalePos;
		}
		vlen = Std.int(vlen / 4);
		var va0 = new Int16Array(vlen * 4);
		var va1 = new Int16Array(vlen * 2);
		var va2 = new Int16Array(vlen * 2);
		var ia = new Uint32Array(ilen);

		var voff = 0;
		var ioff = 0;
		for (i in 0...paintObjects.length) {
			var vas = paintObjects[i].data.raw.vertex_arrays;
			var ias = paintObjects[i].data.raw.index_arrays;
			var scale = paintObjects[i].data.scalePos;

			for (j in 0...vas[0].values.length) va0[j + voff * 4] = vas[0].values[j];
			for (j in 0...Std.int(va0.length / 4)) {
				va0[j * 4     + voff * 4] = Std.int((va0[j * 4     + voff * 4] * scale) / maxScale);
				va0[j * 4 + 1 + voff * 4] = Std.int((va0[j * 4 + 1 + voff * 4] * scale) / maxScale);
				va0[j * 4 + 2 + voff * 4] = Std.int((va0[j * 4 + 2 + voff * 4] * scale) / maxScale);
			}
			for (j in 0...vas[1].values.length) va1[j + voff * 2] = vas[1].values[j];
			for (j in 0...vas[2].values.length) va2[j + voff * 2] = vas[2].values[j];
			for (j in 0...ias[0].values.length) ia[j + ioff] = ias[0].values[j] + voff;

			voff += Std.int(vas[0].values.length / 4);
			ioff += Std.int(ias[0].values.length);
		}

		var raw: TMeshData = {
			name: Context.paintObject.name,
			vertex_arrays: [
				{ values: va0, attrib: "pos" },
				{ values: va1, attrib: "nor" },
				{ values: va2, attrib: "tex" }
			],
			index_arrays: [
				{ values: ia, material: 0 }
			],
			scale_pos: maxScale,
			scale_tex: 1.0
		};

		new MeshData(raw, function(md: MeshData) {
			Context.mergedObject = new MeshObject(md, Context.paintObject.materials);
			Context.mergedObject.name = Context.paintObject.name;
			Context.mergedObject.force_context = "paint";
			Context.mainObject().addChild(Context.mergedObject);
		});
	}

	public static function swapAxis(a: Int, b: Int) {
		var objects = UITrait.inst.worktab.position == SpaceScene ? [cast(Context.object, MeshObject)] : Project.paintObjects;
		for (o in objects) {
			// Remapping vertices, backle up
			// 0 - x, 1 - y, 2 - z
			var vas = o.data.raw.vertex_arrays;
			var pa  = vas[0].values;
			var na0 = a == 2 ? vas[0].values : vas[1].values;
			var na1 = b == 2 ? vas[0].values : vas[1].values;
			var c   = a == 2 ? 3 : a;
			var d   = b == 2 ? 3 : b;
			var e   = a == 2 ? 4 : 2;
			var f   = b == 2 ? 4 : 2;

			for (i in 0...Std.int(pa.length / 4)) {
				var t = pa[i * 4 + a];
				pa[i * 4 + a] = pa[i * 4 + b];
				pa[i * 4 + b] = -t;
				t = na0[i * e + c];
				na0[i * e + c] = na1[i * f + d];
				na1[i * f + d] = -t;
			}

			var g = o.data.geom;
			var posbuf = g.vertexBufferMap.get("pos");
			if (posbuf != null) { // Remove cache
				posbuf.delete();
				g.vertexBufferMap.remove("pos");
			}

			var vertices = g.vertexBuffer.lockInt16(); // posnortex
			for (i in 0...Std.int(vertices.length / 8)) {
				vertices[i * 8    ] = vas[0].values[i * 4    ];
				vertices[i * 8 + 1] = vas[0].values[i * 4 + 1];
				vertices[i * 8 + 2] = vas[0].values[i * 4 + 2];
				vertices[i * 8 + 3] = vas[0].values[i * 4 + 3];
				vertices[i * 8 + 4] = vas[1].values[i * 2    ];
				vertices[i * 8 + 5] = vas[1].values[i * 2 + 1];
			}
			g.vertexBuffer.unlock();
		}

		if (Context.mergedObject != null) {
			Context.mergedObject.remove();
			Data.deleteMesh(Context.mergedObject.data.handle);
			Context.mergedObject = null;
		}
		mergeMesh();

		#if kha_direct3d12
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}

	public static function flipNormals() {
		var objects = UITrait.inst.worktab.position == SpaceScene ? [cast(Context.object, MeshObject)] : Project.paintObjects;
		for (o in objects) {
			var g = o.data.geom;
			var vertices = g.vertexBuffer.lockInt16(); // posnortex
			for (i in 0...Std.int(vertices.length / 8)) {
				vertices[i * 8 + 3] = -vertices[i * 8 + 3];
				vertices[i * 8 + 4] = -vertices[i * 8 + 4];
				vertices[i * 8 + 5] = -vertices[i * 8 + 5];
			}
			g.vertexBuffer.unlock();
		}

		#if kha_direct3d12
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}

	public static function calcNormals() {
		var va = new Vec4();
		var vb = new Vec4();
		var vc = new Vec4();
		var cb = new Vec4();
		var ab = new Vec4();
		var objects = UITrait.inst.worktab.position == SpaceScene ? [cast(Context.object, MeshObject)] : Project.paintObjects;
		for (o in objects) {
			var g = o.data.geom;
			var inda = g.indices[0];
			var vertices = g.vertexBuffer.lockInt16(); // posnortex
			for (i in 0...Std.int(inda.length / 3)) {
				var i1 = inda[i * 3    ];
				var i2 = inda[i * 3 + 1];
				var i3 = inda[i * 3 + 2];
				va.set(vertices[i1 * 8], vertices[i1 * 8 + 1], vertices[i1 * 8 + 2]);
				vb.set(vertices[i2 * 8], vertices[i2 * 8 + 1], vertices[i2 * 8 + 2]);
				vc.set(vertices[i3 * 8], vertices[i3 * 8 + 1], vertices[i3 * 8 + 2]);
				cb.subvecs(vc, vb);
				ab.subvecs(va, vb);
				cb.cross(ab);
				cb.normalize();
				vertices[i1 * 8 + 4] = Std.int(cb.x * 32767);
				vertices[i1 * 8 + 5] = Std.int(cb.y * 32767);
				vertices[i1 * 8 + 3] = Std.int(cb.z * 32767);
				vertices[i2 * 8 + 4] = Std.int(cb.x * 32767);
				vertices[i2 * 8 + 5] = Std.int(cb.y * 32767);
				vertices[i2 * 8 + 3] = Std.int(cb.z * 32767);
				vertices[i3 * 8 + 4] = Std.int(cb.x * 32767);
				vertices[i3 * 8 + 5] = Std.int(cb.y * 32767);
				vertices[i3 * 8 + 3] = Std.int(cb.z * 32767);
			}
			g.vertexBuffer.unlock();
		}

		#if kha_direct3d12
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}
}
