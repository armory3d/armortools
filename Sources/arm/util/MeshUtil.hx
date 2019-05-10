package arm.util;

import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.object.MeshObject;
import arm.ui.UITrait;

class MeshUtil {

	public static function mergeMesh() {
		var vlen = 0;
		var ilen = 0;
		var maxScale = 0.0;
		var paintObjects = UITrait.inst.paintObjects;
		for (i in 0...paintObjects.length) {
			vlen += paintObjects[i].data.raw.vertex_arrays[0].values.length;
			ilen += paintObjects[i].data.raw.index_arrays[0].values.length;
			if (paintObjects[i].data.scalePos > maxScale) maxScale = paintObjects[i].data.scalePos;
		}
		vlen = Std.int(vlen / 4);
		var va0 = new kha.arrays.Int16Array(vlen * 4);
		var va1 = new kha.arrays.Int16Array(vlen * 2);
		var va2 = new kha.arrays.Int16Array(vlen * 2);
		var ia = new kha.arrays.Uint32Array(ilen);

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

		var raw:TMeshData = {
			name: UITrait.inst.paintObject.name,
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

		new MeshData(raw, function(md:MeshData) {
			UITrait.inst.mergedObject = new MeshObject(md, UITrait.inst.paintObject.materials);
			UITrait.inst.mergedObject.name = UITrait.inst.paintObject.name;
			UITrait.inst.mergedObject.force_context = "paint";
			UITrait.inst.mainObject().addChild(UITrait.inst.mergedObject);
		});
	}
	
	public static function switchUpAxis(axisUp:Int) {
		for (p in UITrait.inst.paintObjects) {
			var g = p.data.geom;
			var vertices = g.vertexBuffer.lockInt16(); // posnortex
			var verticesDepth = g.vertexBufferMap.get("pos").lockInt16();
			if (axisUp == 2 || axisUp == 3) { // Y / -Y
				var sign = axisUp == 2 ? 1 : -1;
				for (i in 0...Std.int(vertices.length / 8)) {
					// Swap Y/Z
					var f = vertices[i * 8 + 1] * sign;
					vertices[i * 8 + 1] = vertices[i * 8 + 2] * sign;
					vertices[i * 8 + 2] = -f;

					f = vertices[i * 8 + 5] * sign;
					vertices[i * 8 + 5] = vertices[i * 8 + 3] * sign;
					vertices[i * 8 + 3] = -f;

					f = verticesDepth[i * 4 + 1] * sign;
					verticesDepth[i * 4 + 1] = verticesDepth[i * 4 + 2] * sign;
					verticesDepth[i * 4 + 2] = -f;
				}
			}
			else if (axisUp == 0 || axisUp == 1) { // Z / -Z
				var sign = axisUp == 0 ? 1 : -1;
				for (i in 0...Std.int(vertices.length / 8)) {
					var f = vertices[i * 8 + 1] * sign;
					vertices[i * 8 + 1] = -vertices[i * 8 + 2] * sign;
					vertices[i * 8 + 2] = f;

					f = vertices[i * 8 + 5] * sign;
					vertices[i * 8 + 5] = -vertices[i * 8 + 3] * sign;
					vertices[i * 8 + 3] = f;

					f = verticesDepth[i * 4 + 1] * sign;
					verticesDepth[i * 4 + 1] = -verticesDepth[i * 4 + 2] * sign;
					verticesDepth[i * 4 + 2] = f;
				}
			}
			g.vertexBuffer.unlock();
			g.vertexBufferMap.get("pos").unlock();
		}
	}

	public static function flipNormals() {
		for (p in UITrait.inst.paintObjects) {
			var g = p.data.geom;
			var vertices = g.vertexBuffer.lockInt16(); // posnortex
			for (i in 0...Std.int(vertices.length / 8)) {
				vertices[i * 8 + 3] = -vertices[i * 8 + 3];
				vertices[i * 8 + 4] = -vertices[i * 8 + 4];
				vertices[i * 8 + 5] = -vertices[i * 8 + 5];
			}
			g.vertexBuffer.unlock();
		}
	}

	public static function calcNormals() {
		var va = new iron.math.Vec4();
		var vb = new iron.math.Vec4();
		var vc = new iron.math.Vec4();
		var cb = new iron.math.Vec4();
		var ab = new iron.math.Vec4();
		for (p in UITrait.inst.paintObjects) {
			var g = p.data.geom;
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
	}
}
