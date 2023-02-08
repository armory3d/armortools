package arm.util;

import kha.arrays.Int16Array;
import kha.arrays.Uint32Array;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.object.MeshObject;
import iron.math.Vec4;

class MeshUtil {

	public static var unwrappers: Map<String, Dynamic->Void> = [];

	public static function mergeMesh(paintObjects: Array<MeshObject> = null) {
		if (paintObjects == null) paintObjects = Project.paintObjects;
		if (paintObjects.length == 0) return;
		Context.mergedObjectIsAtlas = paintObjects.length < Project.paintObjects.length;
		var vlen = 0;
		var ilen = 0;
		var maxScale = 0.0;
		for (i in 0...paintObjects.length) {
			vlen += paintObjects[i].data.raw.vertex_arrays[0].values.length;
			ilen += paintObjects[i].data.raw.index_arrays[0].values.length;
			if (paintObjects[i].data.scalePos > maxScale) maxScale = paintObjects[i].data.scalePos;
		}
		vlen = Std.int(vlen / 4);
		var va0 = new Int16Array(vlen * 4);
		var va1 = new Int16Array(vlen * 2);
		var va2 = new Int16Array(vlen * 2);
		var va3 = paintObjects[0].data.raw.vertex_arrays.length > 3 ? new Int16Array(vlen * 3) : null; // +1 padding
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
			if (va3 != null) for (j in 0...vas[3].values.length) va3[j + voff * 3] = vas[3].values[j];
			for (j in 0...ias[0].values.length) ia[j + ioff] = ias[0].values[j] + voff;

			voff += Std.int(vas[0].values.length / 4);
			ioff += Std.int(ias[0].values.length);
		}

		var raw: TMeshData = {
			name: Context.paintObject.name,
			vertex_arrays: [
				{ values: va0, attrib: "pos", data: "short4norm" },
				{ values: va1, attrib: "nor", data: "short2norm" },
				{ values: va2, attrib: "tex", data: "short2norm" }
			],
			index_arrays: [
				{ values: ia, material: 0 }
			],
			scale_pos: maxScale,
			scale_tex: 1.0
		};
		if (va3 != null) raw.vertex_arrays.push({ values: va3, attrib: "col", data: "short4norm", padding: 1 });

		if (Context.mergedObject != null) {
			Context.mergedObject.remove();
			Data.deleteMesh(Context.mergedObject.data.handle);
		}

		new MeshData(raw, function(md: MeshData) {
			Context.mergedObject = new MeshObject(md, Context.paintObject.materials);
			Context.mergedObject.name = Context.paintObject.name + "_merged";
			Context.mergedObject.force_context = "paint";
			Context.mergedObject.setParent(Context.mainObject());
		});

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}

	public static function swapAxis(a: Int, b: Int) {
		var objects = Project.paintObjects;
		for (o in objects) {
			// Remapping vertices, buckle up
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
			var l = g.structLength;
			var vertices = g.vertexBuffer.lock(); // posnortex
			for (i in 0...Std.int(vertices.byteLength / 2 / l)) {
				vertices.setInt16((i * l    ) * 2, vas[0].values[i * 4    ]);
				vertices.setInt16((i * l + 1) * 2, vas[0].values[i * 4 + 1]);
				vertices.setInt16((i * l + 2) * 2, vas[0].values[i * 4 + 2]);
				vertices.setInt16((i * l + 3) * 2, vas[0].values[i * 4 + 3]);
				vertices.setInt16((i * l + 4) * 2, vas[1].values[i * 2    ]);
				vertices.setInt16((i * l + 5) * 2, vas[1].values[i * 2 + 1]);
			}
			g.vertexBuffer.unlock();
		}

		if (Context.mergedObject != null) {
			Context.mergedObject.remove();
			Data.deleteMesh(Context.mergedObject.data.handle);
			Context.mergedObject = null;
		}
		mergeMesh();
	}

	public static function flipNormals() {
		var objects = Project.paintObjects;
		for (o in objects) {
			var vas = o.data.raw.vertex_arrays;
			var va0 = vas[0].values;
			var va1 = vas[1].values;
			var g = o.data.geom;
			var l = g.structLength;
			var vertices = g.vertexBuffer.lock(); // posnortex
			for (i in 0...Std.int(vertices.byteLength / 2 / l)) {
				va0[i * 4 + 3] = -va0[i * 4 + 3];
				va1[i * 2] = -va1[i * 2];
				va1[i * 2 + 1] = -va1[i * 2 + 1];
				vertices.setInt16((i * l + 3) * 2, -vertices.getInt16((i * l + 3) * 2));
				vertices.setInt16((i * l + 4) * 2, -vertices.getInt16((i * l + 4) * 2));
				vertices.setInt16((i * l + 5) * 2, -vertices.getInt16((i * l + 5) * 2));
			}
			g.vertexBuffer.unlock();
		}

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}

	public static function calcNormals(smooth = false) {
		var va = new Vec4();
		var vb = new Vec4();
		var vc = new Vec4();
		var cb = new Vec4();
		var ab = new Vec4();
		var objects = Project.paintObjects;
		for (o in objects) {
			var g = o.data.geom;
			var l = g.structLength;
			var inda = g.indices[0];
			var vertices = g.vertexBuffer.lock(); // posnortex
			for (i in 0...Std.int(inda.length / 3)) {
				var i1 = inda[i * 3    ];
				var i2 = inda[i * 3 + 1];
				var i3 = inda[i * 3 + 2];
				va.set(vertices.getInt16((i1 * l) * 2), vertices.getInt16((i1 * l + 1) * 2), vertices.getInt16((i1 * l + 2) * 2));
				vb.set(vertices.getInt16((i2 * l) * 2), vertices.getInt16((i2 * l + 1) * 2), vertices.getInt16((i2 * l + 2) * 2));
				vc.set(vertices.getInt16((i3 * l) * 2), vertices.getInt16((i3 * l + 1) * 2), vertices.getInt16((i3 * l + 2) * 2));
				cb.subvecs(vc, vb);
				ab.subvecs(va, vb);
				cb.cross(ab);
				cb.normalize();
				vertices.setInt16((i1 * l + 4) * 2, Std.int(cb.x * 32767));
				vertices.setInt16((i1 * l + 5) * 2, Std.int(cb.y * 32767));
				vertices.setInt16((i1 * l + 3) * 2, Std.int(cb.z * 32767));
				vertices.setInt16((i2 * l + 4) * 2, Std.int(cb.x * 32767));
				vertices.setInt16((i2 * l + 5) * 2, Std.int(cb.y * 32767));
				vertices.setInt16((i2 * l + 3) * 2, Std.int(cb.z * 32767));
				vertices.setInt16((i3 * l + 4) * 2, Std.int(cb.x * 32767));
				vertices.setInt16((i3 * l + 5) * 2, Std.int(cb.y * 32767));
				vertices.setInt16((i3 * l + 3) * 2, Std.int(cb.z * 32767));
			}

			if (smooth) {
				var shared = new Uint32Array(1024);
				var sharedLen = 0;
				var found: Array<Int> = [];
				for (i in 0...(inda.length - 1)) {
					if (found.indexOf(i) >= 0) continue;
					var i1 = inda[i];
					sharedLen = 0;
					shared[sharedLen++] = i1;
					for (j in (i + 1)...inda.length) {
						var i2 = inda[j];
						var i1l = i1 * l;
						var i2l = i2 * l;
						if (vertices.getInt16((i1l    ) * 2) == vertices.getInt16((i2l    ) * 2) &&
							vertices.getInt16((i1l + 1) * 2) == vertices.getInt16((i2l + 1) * 2) &&
							vertices.getInt16((i1l + 2) * 2) == vertices.getInt16((i2l + 2) * 2)) {
							// if (n1.dot(n2) > 0)
							shared[sharedLen++] = i2;
							found.push(j);
							if (sharedLen >= 1024) break;
						}
					}
					if (sharedLen > 1) {
						va.set(0, 0, 0);
						for (j in 0...sharedLen) {
							var i1 = shared[j];
							var i1l = i1 * l;
							va.addf(vertices.getInt16((i1l + 4) * 2), vertices.getInt16((i1l + 5) * 2), vertices.getInt16((i1l + 3) * 2));
						}
						va.mult(1 / sharedLen);
						va.normalize();
						var vax = Std.int(va.x * 32767);
						var vay = Std.int(va.y * 32767);
						var vaz = Std.int(va.z * 32767);
						for (j in 0...sharedLen) {
							var i1 = shared[j];
							var i1l = i1 * l;
							vertices.setInt16((i1l + 4) * 2, vax);
							vertices.setInt16((i1l + 5) * 2, vay);
							vertices.setInt16((i1l + 3) * 2, vaz);
						}
					}
				}
			}
			g.vertexBuffer.unlock();

			var va0 = o.data.raw.vertex_arrays[0].values;
			var va1 = o.data.raw.vertex_arrays[1].values;
			for (i in 0...Std.int(vertices.byteLength / 4 / l)) {
				va1[i * 2    ] = vertices.getInt16((i * l + 4) * 2);
				va1[i * 2 + 1] = vertices.getInt16((i * l + 5) * 2);
				va0[i * 4 + 3] = vertices.getInt16((i * l + 3) * 2);
			}
		}

		#if (kha_direct3d12 || kha_vulkan)
		mergeMesh();
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}

	public static function toOrigin() {
		var dx = 0.0;
		var dy = 0.0;
		var dz = 0.0;
		for (o in Project.paintObjects) {
			var l = 4;
			var sc = o.data.scalePos / 32767;
			var va = o.data.raw.vertex_arrays[0].values;
			var minx = va[0];
			var maxx = va[0];
			var miny = va[1];
			var maxy = va[1];
			var minz = va[2];
			var maxz = va[2];
			for (i in 1...Std.int(va.length / l)) {
				if (va[i * l] < minx) minx = va[i * l];
				else if (va[i * l] > maxx) maxx = va[i * l];
				if (va[i * l + 1] < miny) miny = va[i * l + 1];
				else if (va[i * l + 1] > maxy) maxy = va[i * l + 1];
				if (va[i * l + 2] < minz) minz = va[i * l + 2];
				else if (va[i * l + 2] > maxz) maxz = va[i * l + 2];
			}
			dx += (minx + maxx) / 2 * sc;
			dy += (miny + maxy) / 2 * sc;
			dz += (minz + maxz) / 2 * sc;
		}
		dx /= Project.paintObjects.length;
		dy /= Project.paintObjects.length;
		dz /= Project.paintObjects.length;

		for (o in Project.paintObjects) {
			var g = o.data.geom;
			var sc = o.data.scalePos / 32767;
			var va = o.data.raw.vertex_arrays[0].values;
			var maxScale = 0.0;
			for (i in 0...Std.int(va.length / 4)) {
				if (Math.abs(va[i * 4    ] * sc - dx) > maxScale) maxScale = Math.abs(va[i * 4    ] * sc - dx);
				if (Math.abs(va[i * 4 + 1] * sc - dy) > maxScale) maxScale = Math.abs(va[i * 4 + 1] * sc - dy);
				if (Math.abs(va[i * 4 + 2] * sc - dz) > maxScale) maxScale = Math.abs(va[i * 4 + 2] * sc - dz);
			}
			o.transform.scaleWorld = o.data.scalePos = o.data.raw.scale_pos = maxScale;
			o.transform.buildMatrix();

			for (i in 0...Std.int(va.length / 4)) {
				va[i * 4    ] = Std.int((va[i * 4    ] * sc - dx) / maxScale * 32767);
				va[i * 4 + 1] = Std.int((va[i * 4 + 1] * sc - dy) / maxScale * 32767);
				va[i * 4 + 2] = Std.int((va[i * 4 + 2] * sc - dz) / maxScale * 32767);
			}

			var l = g.structLength;
			var vertices = g.vertexBuffer.lock(); // posnortex
			for (i in 0...Std.int(vertices.byteLength / 2 / l)) {
				vertices.setInt16((i * l    ) * 2, va[i * 4    ]);
				vertices.setInt16((i * l + 1) * 2, va[i * 4 + 1]);
				vertices.setInt16((i * l + 2) * 2, va[i * 4 + 2]);
			}
			g.vertexBuffer.unlock();
		}

		mergeMesh();
	}

	public static function applyDisplacement() {
		var height = Project.layers[0].texpaint_pack.getPixels();
		var res = Project.layers[0].texpaint_pack.width;
		var strength = 0.1;
		var o = Project.paintObjects[0];
		var g = o.data.geom;
		var l = g.structLength;
		var vertices = g.vertexBuffer.lock(); // posnortex
		for (i in 0...Std.int(vertices.byteLength / 2 / l)) {
			var x = Std.int(vertices.getInt16((i * l + 6) * 2) / 32767 * res);
			var y = Std.int(vertices.getInt16((i * l + 7) * 2) / 32767 * res);
			var h = (1.0 - height.get((y * res + x) * 4 + 3) / 255) * strength;
			vertices.setInt16((i * l    ) * 2, vertices.getInt16((i * l    ) * 2) - Std.int(vertices.getInt16((i * l + 4) * 2) * h));
			vertices.setInt16((i * l + 1) * 2, vertices.getInt16((i * l + 1) * 2) - Std.int(vertices.getInt16((i * l + 5) * 2) * h));
			vertices.setInt16((i * l + 2) * 2, vertices.getInt16((i * l + 2) * 2) - Std.int(vertices.getInt16((i * l + 3) * 2) * h));
		}
		g.vertexBuffer.unlock();

		var va0 = o.data.raw.vertex_arrays[0].values;
		var va1 = o.data.raw.vertex_arrays[1].values;
		for (i in 0...Std.int(vertices.byteLength / 4 / l)) {
			va0[i * 4    ] = vertices.getInt16((i * l    ) * 2);
			va0[i * 4 + 1] = vertices.getInt16((i * l + 1) * 2);
			va0[i * 4 + 2] = vertices.getInt16((i * l + 2) * 2);
		}
	}

	public static function equirectUnwrap(mesh: Dynamic) {
		var verts = Std.int(mesh.posa.length / 4);
		mesh.texa = new Int16Array(verts * 2);
		var n = new Vec4();
		for (i in 0...verts) {
			n.set(mesh.posa[i * 4] / 32767, mesh.posa[i * 4 + 1] / 32767, mesh.posa[i * 4 + 2] / 32767).normalize();
			// Sphere projection
			// mesh.texa[i * 2    ] = Math.atan2(n.x, n.y) / (Math.PI * 2) + 0.5;
			// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
			// Equirect
			mesh.texa[i * 2    ] = Std.int(((Math.atan2(-n.z, n.x) + Math.PI) / (Math.PI * 2)) * 32767);
			mesh.texa[i * 2 + 1] = Std.int((Math.acos(n.y) / Math.PI) * 32767);
		}
	}
}
