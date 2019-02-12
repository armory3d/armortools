package arm;

import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.object.MeshObject;

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

			// position, normals

			var vertices = g.vertexBuffer.lock(); // posnortex
			var verticesDepth = g.vertexBufferMap.get("pos").lock();
			if (!g.vertexBufferMap.exists("posnor")) g.get([{name: "pos", data: 'short4norm'}, {name: "nor", data: 'short2norm'}]);
			var verticesVox = g.vertexBufferMap.get("posnor").lock();
			if (axisUp == 1) { // Y
				for (i in 0...Std.int(vertices.length / g.structLength)) {
					var f = vertices[i * g.structLength + 1];
					vertices[i * g.structLength + 1] = vertices[i * g.structLength + 2];
					vertices[i * g.structLength + 2] = -f;
					f = vertices[i * g.structLength + 4];
					vertices[i * g.structLength + 4] = vertices[i * g.structLength + 5];
					vertices[i * g.structLength + 5] = -f;

					f = verticesDepth[i * 3 + 1];
					verticesDepth[i * 3 + 1] = verticesDepth[i * 3 + 2];
					verticesDepth[i * 3 + 2] = -f;

					f = verticesVox[i * 6 + 1];
					verticesVox[i * 6 + 1] = verticesVox[i * 6 + 2];
					verticesVox[i * 6 + 2] = -f;
				}
			}
			else { // Z
				for (i in 0...Std.int(vertices.length / g.structLength)) {
					var f = vertices[i * g.structLength + 1];
					vertices[i * g.structLength + 1] = -vertices[i * g.structLength + 2];
					vertices[i * g.structLength + 2] = f;
					f = vertices[i * g.structLength + 4];
					vertices[i * g.structLength + 4] = -vertices[i * g.structLength + 5];
					vertices[i * g.structLength + 5] = f;

					f = verticesDepth[i * 3 + 1];
					verticesDepth[i * 3 + 1] = -verticesDepth[i * 3 + 2];
					verticesDepth[i * 3 + 2] = f;

					f = verticesVox[i * 6 + 1];
					verticesVox[i * 6 + 1] = -verticesVox[i * 6 + 2];
					verticesVox[i * 6 + 2] = f;
				}
			}
			g.vertexBuffer.unlock();
			g.vertexBufferMap.get("pos").unlock();
			g.vertexBufferMap.get("posnor").unlock();
		}
	}
}
