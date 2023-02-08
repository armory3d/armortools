package arm.io;

import kha.Blob;
import kha.arrays.Uint32Array;
import kha.arrays.Float32Array;
import kha.arrays.Int16Array;
import iron.data.Data;
import iron.math.Vec4;
import arm.format.BlendParser;
import zui.Nodes;
import arm.ui.UISidebar;
import arm.ui.UINodes;
import arm.sys.Path;

class ImportBlend {

	static inline var eps = 1.0 / 32767;

	public static function run(path: String, replaceExisting = true) {
		Data.getBlob(path, function(b: Blob) {
			var bl = new BlendParser(b);
			if (bl.dna == null) {
				Console.error(Strings.error3());
				return;
			}

			var obs = bl.get("Object");
			if (obs == null || obs.length == 0) { ImportMesh.makeMesh(null, path); return; }

			var first = true;
			for (ob in obs) {
				if (ob.get("type") != 1) continue;

				var name: String = ob.get("id").get("name");
				name = name.substring(2, name.length);

				var m: Dynamic = ob.get("data", 0, "Mesh");
				if (m == null) continue;

				var totpoly = m.get("totpoly");
				if (totpoly == 0) continue;

				var numtri = 0;
				for (i in 0...totpoly) {
					var poly = m.get("mpoly", i);
					var totloop = poly.get("totloop");
					numtri += totloop - 2;
				}
				var inda = new Uint32Array(numtri * 3);
				for (i in 0...inda.length) inda[i] = i;

				var posa32 = new Float32Array(numtri * 3 * 4);
				var posa = new Int16Array(numtri * 3 * 4);
				var nora = new Int16Array(numtri * 3 * 2);
				var hasuv = m.get("mloopuv") != null;
				var texa = hasuv ? new Int16Array(numtri * 3 * 2) : null;
				var hascol = Context.parseVCols && m.get("mloopcol") != null;
				var cola = hascol ? new Int16Array(numtri * 3 * 3) : null;

				var tri = 0;
				var vec0 = new Vec4();
				var vec1 = new Vec4();
				var vec2 = new Vec4();
				for (i in 0...totpoly) {
					var poly = m.get("mpoly", i);
					var smooth = poly.get("flag") & 1 == 1; // ME_SMOOTH
					var loopstart = poly.get("loopstart");
					var totloop = poly.get("totloop");
					if (totloop <= 4) { // Convex, fan triangulation
						var v0 = m.get("mvert", m.get("mloop", loopstart + totloop - 1).get("v"));
						var v1 = m.get("mvert", m.get("mloop", loopstart).get("v"));
						var co0 = v0.get("co");
						var co1 = v1.get("co");
						var no0 = v0.get("no");
						var no1 = v1.get("no");
						if (smooth) {
							vec0.set(no0[0] / 32767, no0[1] / 32767, no0[2] / 32767).normalize(); // shortmax
							vec1.set(no1[0] / 32767, no1[1] / 32767, no1[2] / 32767).normalize();
						}
						var uv0: Float32Array = null;
						var uv1: Float32Array = null;
						var uv2: Float32Array = null;
						if (hasuv) {
							uv0 = m.get("mloopuv", loopstart + totloop - 1).get("uv");
							if (uv0[0] > 1.0 + eps) uv0[0] = uv0[0] - Std.int(uv0[0]);
							if (uv0[1] > 1.0 + eps) uv0[1] = uv0[1] - Std.int(uv0[1]);
							uv1 = m.get("mloopuv", loopstart).get("uv");
							if (uv1[0] > 1.0 + eps) uv1[0] = uv1[0] - Std.int(uv1[0]);
							if (uv1[1] > 1.0 + eps) uv1[1] = uv1[1] - Std.int(uv1[1]);
						}
						var col0r: Int = 0;
						var col0g: Int = 0;
						var col0b: Int = 0;
						var col1r: Int = 0;
						var col1g: Int = 0;
						var col1b: Int = 0;
						var col2r: Int = 0;
						var col2g: Int = 0;
						var col2b: Int = 0;
						if (hascol) {
							var loop = m.get("mloopcol", loopstart + totloop - 1);
							col0r = loop.get("r");
							col0g = loop.get("g");
							col0b = loop.get("b");
							loop = m.get("mloopcol", loopstart);
							col1r = loop.get("r");
							col1g = loop.get("g");
							col1b = loop.get("b");
						}
						for (j in 0...totloop - 2) {
							var v2 = m.get("mvert", m.get("mloop", loopstart + j + 1).get("v"));
							var co2 = v2.get("co");
							var no2 = v2.get("no");
							if (smooth) {
								vec2.set(no2[0] / 32767, no2[1] / 32767, no2[2] / 32767).normalize();
							}
							else {
								vec2.set(co2[0], co2[1], co2[2]);
								vec1.set(co1[0], co1[1], co1[2]);
								vec0.subvecs(vec2, vec1);
								vec2.set(co0[0], co0[1], co0[2]);
								vec1.subvecs(vec2, vec1);
								vec0.cross(vec1);
								vec0.normalize();
							}
							posa32[tri * 9    ] = co0[0];
							posa32[tri * 9 + 1] = co0[1];
							posa32[tri * 9 + 2] = co0[2];
							posa32[tri * 9 + 3] = co1[0];
							posa32[tri * 9 + 4] = co1[1];
							posa32[tri * 9 + 5] = co1[2];
							posa32[tri * 9 + 6] = co2[0];
							posa32[tri * 9 + 7] = co2[1];
							posa32[tri * 9 + 8] = co2[2];
							posa[tri * 12 + 3] = Std.int(vec0.z * 32767);
							posa[tri * 12 + 7] = Std.int((smooth ? vec1.z : vec0.z) * 32767);
							posa[tri * 12 + 11] = Std.int((smooth ? vec2.z : vec0.z) * 32767);
							nora[tri * 6    ] = Std.int(vec0.x * 32767);
							nora[tri * 6 + 1] = Std.int(vec0.y * 32767);
							nora[tri * 6 + 2] = Std.int((smooth ? vec1.x : vec0.x) * 32767);
							nora[tri * 6 + 3] = Std.int((smooth ? vec1.y : vec0.y) * 32767);
							nora[tri * 6 + 4] = Std.int((smooth ? vec2.x : vec0.x) * 32767);
							nora[tri * 6 + 5] = Std.int((smooth ? vec2.y : vec0.y) * 32767);
							co1 = co2;
							no1 = no2;
							vec1.setFrom(vec2);
							if (hasuv) {
								uv2 = m.get("mloopuv", loopstart + j + 1).get("uv");
								if (uv2[0] > 1.0 + eps) uv2[0] = uv2[0] - Std.int(uv2[0]);
								if (uv2[1] > 1.0 + eps) uv2[1] = uv2[1] - Std.int(uv2[1]);
								texa[tri * 6    ] = Std.int(uv0[0] * 32767);
								texa[tri * 6 + 1] = Std.int((1.0 - uv0[1]) * 32767);
								texa[tri * 6 + 2] = Std.int(uv1[0] * 32767);
								texa[tri * 6 + 3] = Std.int((1.0 - uv1[1]) * 32767);
								texa[tri * 6 + 4] = Std.int(uv2[0] * 32767);
								texa[tri * 6 + 5] = Std.int((1.0 - uv2[1]) * 32767);
								uv1 = uv2;
							}
							if (hascol) {
								var loop = m.get("mloopcol", loopstart + j + 1);
								col2r = loop.get("r");
								col2g = loop.get("g");
								col2b = loop.get("b");
								cola[tri * 9    ] = col0r * 128;
								cola[tri * 9 + 1] = col0g * 128;
								cola[tri * 9 + 2] = col0b * 128;
								cola[tri * 9 + 3] = col1r * 128;
								cola[tri * 9 + 4] = col1g * 128;
								cola[tri * 9 + 5] = col1b * 128;
								cola[tri * 9 + 6] = col2r * 128;
								cola[tri * 9 + 7] = col2g * 128;
								cola[tri * 9 + 8] = col2b * 128;
								col1r = col2r;
								col1g = col2g;
								col1b = col2b;
							}
							tri++;
						}
					}
					else { // Convex or concave, ear clipping
						var va: Array<Int> = [];
						for (i in 0...totloop) va.push(loopstart + i);
						var co0 = m.get("mvert", m.get("mloop", loopstart).get("v")).get("co");
						var co1 = m.get("mvert", m.get("mloop", loopstart + 1).get("v")).get("co");
						var co2 = m.get("mvert", m.get("mloop", loopstart + 2).get("v")).get("co");
						vec2.set(co2[0], co2[1], co2[2]);
						vec1.set(co1[0], co1[1], co1[2]);
						vec0.subvecs(vec2, vec1);
						vec2.set(co0[0], co0[1], co0[2]);
						vec1.subvecs(vec2, vec1);
						vec0.cross(vec1);
						vec0.normalize();

						var nx = vec0.x;
						var ny = vec0.y;
						var nz = vec0.z;
						var nxabs = Math.abs(nx);
						var nyabs = Math.abs(ny);
						var nzabs = Math.abs(nz);
						var flip = nx + ny + nz > 0;
						var axis = nxabs > nyabs && nxabs > nzabs ? 0 : nyabs > nxabs && nyabs > nzabs ? 1 : 2;
						var axis0 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
						var axis1 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

						var winding = 0.0;
						for (i in 0...totloop) {
							var co0 = m.get("mvert", m.get("mloop", loopstart + i).get("v")).get("co");
							var co1 = m.get("mvert", m.get("mloop", loopstart + ((i + 1) % totloop)).get("v")).get("co");
							winding += (co1[axis0] - co0[axis0]) * (co1[axis1] + co0[axis1]);
						}
						var flip = winding > 0 ? nx + ny + nz > 0 : nx + ny + nz < 0;
						var axis0 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
						var axis1 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

						var vi = totloop;
						var loops = 0;
						var i = -1;
						while (vi > 2 && loops++ < vi) {
							i = (i + 1) % vi;
							var i1 = (i + 1) % vi;
							var i2 = (i + 2) % vi;
							var v0 = m.get("mvert", m.get("mloop", va[i ]).get("v"));
							var v1 = m.get("mvert", m.get("mloop", va[i1]).get("v"));
							var v2 = m.get("mvert", m.get("mloop", va[i2]).get("v"));
							var co0 = v0.get("co");
							var co1 = v1.get("co");
							var co2 = v2.get("co");
							var v0x = co0[axis0];
							var v0y = co0[axis1];
							var v1x = co1[axis0];
							var v1y = co1[axis1];
							var v2x = co2[axis0];
							var v2y = co2[axis1];

							var e0x = v0x - v1x; // Not an interior vertex
							var e0y = v0y - v1y;
							var e1x = v2x - v1x;
							var e1y = v2y - v1y;
							var cross = e0x * e1y - e0y * e1x;
							if (cross <= 0) continue;

							var overlap = false; // Other vertex found inside this triangle
							for (j in 0...vi - 3) {
								var j0 = (i + 3 + j) % vi;
								var co = m.get("mvert", m.get("mloop", va[j0]).get("v")).get("co");
								var px = co[axis0];
								var py = co[axis1];

								if (arm.format.MeshParser.pnpoly(v0x, v0y, v1x, v1y, v2x, v2y, px, py)) {
									overlap = true;
									break;
								}
							}
							if (overlap) continue;

							// Found ear
							{
								var no0 = v0.get("no");
								var no1 = v1.get("no");
								var no2 = v2.get("no");
								if (smooth) {
									vec0.set(no0[0] / 32767, no0[1] / 32767, no0[2] / 32767).normalize(); // shortmax
									vec1.set(no1[0] / 32767, no1[1] / 32767, no1[2] / 32767).normalize();
									vec2.set(no2[0] / 32767, no2[1] / 32767, no2[2] / 32767).normalize();
								}
								else {
									vec2.set(co2[0], co2[1], co2[2]);
									vec1.set(co1[0], co1[1], co1[2]);
									vec0.subvecs(vec2, vec1);
									vec2.set(co0[0], co0[1], co0[2]);
									vec1.subvecs(vec2, vec1);
									vec0.cross(vec1);
									vec0.normalize();
								}
								var uv0: Float32Array = null;
								var uv1: Float32Array = null;
								var uv2: Float32Array = null;
								if (hasuv) {
									uv0 = m.get("mloopuv", va[i ]).get("uv");
									if (uv0[0] > 1.0 + eps) uv0[0] = uv0[0] - Std.int(uv0[0]);
									if (uv0[1] > 1.0 + eps) uv0[1] = uv0[1] - Std.int(uv0[1]);
									uv1 = m.get("mloopuv", va[i1]).get("uv");
									if (uv1[0] > 1.0 + eps) uv1[0] = uv1[0] - Std.int(uv1[0]);
									if (uv1[1] > 1.0 + eps) uv1[1] = uv1[1] - Std.int(uv1[1]);
									uv2 = m.get("mloopuv", va[i2]).get("uv");
									if (uv2[0] > 1.0 + eps) uv2[0] = uv2[0] - Std.int(uv2[0]);
									if (uv2[1] > 1.0 + eps) uv2[1] = uv2[1] - Std.int(uv2[1]);
								}
								var col0r: Int = 0;
								var col0g: Int = 0;
								var col0b: Int = 0;
								var col1r: Int = 0;
								var col1g: Int = 0;
								var col1b: Int = 0;
								var col2r: Int = 0;
								var col2g: Int = 0;
								var col2b: Int = 0;
								if (hascol) {
									var loop = m.get("mloopcol", va[i ]);
									col0r = loop.get("r");
									col0g = loop.get("g");
									col0b = loop.get("b");
									loop = m.get("mloopcol", va[i1]);
									col1r = loop.get("r");
									col1g = loop.get("g");
									col1b = loop.get("b");
									loop = m.get("mloopcol", va[i2]);
									col2r = loop.get("r");
									col2g = loop.get("g");
									col2b = loop.get("b");
								}
								posa32[tri * 9    ] = co0[0];
								posa32[tri * 9 + 1] = co0[1];
								posa32[tri * 9 + 2] = co0[2];
								posa32[tri * 9 + 3] = co1[0];
								posa32[tri * 9 + 4] = co1[1];
								posa32[tri * 9 + 5] = co1[2];
								posa32[tri * 9 + 6] = co2[0];
								posa32[tri * 9 + 7] = co2[1];
								posa32[tri * 9 + 8] = co2[2];
								posa[tri * 12 + 3] = Std.int(vec0.z * 32767);
								posa[tri * 12 + 7] = Std.int((smooth ? vec1.z : vec0.z) * 32767);
								posa[tri * 12 + 11] = Std.int((smooth ? vec2.z : vec0.z) * 32767);
								nora[tri * 6    ] = Std.int(vec0.x * 32767);
								nora[tri * 6 + 1] = Std.int(vec0.y * 32767);
								nora[tri * 6 + 2] = Std.int((smooth ? vec1.x : vec0.x) * 32767);
								nora[tri * 6 + 3] = Std.int((smooth ? vec1.y : vec0.y) * 32767);
								nora[tri * 6 + 4] = Std.int((smooth ? vec2.x : vec0.x) * 32767);
								nora[tri * 6 + 5] = Std.int((smooth ? vec2.y : vec0.y) * 32767);
								if (hasuv) {
									texa[tri * 6    ] = Std.int(uv0[0] * 32767);
									texa[tri * 6 + 1] = Std.int((1.0 - uv0[1]) * 32767);
									texa[tri * 6 + 2] = Std.int(uv1[0] * 32767);
									texa[tri * 6 + 3] = Std.int((1.0 - uv1[1]) * 32767);
									texa[tri * 6 + 4] = Std.int(uv2[0] * 32767);
									texa[tri * 6 + 5] = Std.int((1.0 - uv2[1]) * 32767);
								}
								if (hascol) {
									cola[tri * 9    ] = col0r * 128;
									cola[tri * 9 + 1] = col0g * 128;
									cola[tri * 9 + 2] = col0b * 128;
									cola[tri * 9 + 3] = col1r * 128;
									cola[tri * 9 + 4] = col1g * 128;
									cola[tri * 9 + 5] = col1b * 128;
									cola[tri * 9 + 6] = col2r * 128;
									cola[tri * 9 + 7] = col2g * 128;
									cola[tri * 9 + 8] = col2b * 128;
								}
								tri++;
							}

							for (j in ((i + 1) % vi)...vi - 1) { // Consume vertex
								va[j] = va[j + 1];
							}
							vi--;
							i--;
							loops = 0;
						}
					}
				}

				// Apply world matrix
				var obmat = ob.get("obmat", 0, "float", 16);
				var mat = iron.math.Mat4.fromFloat32Array(obmat).transpose();
				var v = new iron.math.Vec4();
				for (i in 0...Std.int(posa32.length / 3)) {
					v.set(posa32[i * 3], posa32[i * 3 + 1], posa32[i * 3 + 2]);
					v.applymat4(mat);
					posa32[i * 3    ] = v.x;
					posa32[i * 3 + 1] = v.y;
					posa32[i * 3 + 2] = v.z;
				}
				mat.getInverse(mat);
				mat.transpose3x3();
				mat._30 = mat._31 = mat._32 = mat._33 = 0;
				for (i in 0...Std.int(nora.length / 2)) {
					v.set(nora[i * 2] / 32767, nora[i * 2 + 1] / 32767, posa[i * 4 + 3] / 32767);
					v.applymat(mat);
					v.normalize();
					nora[i * 2    ] = Std.int(v.x * 32767);
					nora[i * 2 + 1] = Std.int(v.y * 32767);
					posa[i * 4 + 3] = Std.int(v.z * 32767);
				}

				// Pack positions to (-1, 1) range
				var scalePos = 0.0;
				for (i in 0...posa32.length) {
					var f = Math.abs(posa32[i]);
					if (scalePos < f) scalePos = f;
				}
				var inv = 1 / scalePos;
				for (i in 0...Std.int(posa32.length / 3)) {
					posa[i * 4    ] = Std.int(posa32[i * 3    ] * 32767 * inv);
					posa[i * 4 + 1] = Std.int(posa32[i * 3 + 1] * 32767 * inv);
					posa[i * 4 + 2] = Std.int(posa32[i * 3 + 2] * 32767 * inv);
				}

				var obj = {posa: posa, nora: nora, texa: texa, cola: cola, inda: inda, name: name, scalePos: scalePos, scaleTes: 1.0};

				(first && replaceExisting) ? ImportMesh.makeMesh(obj, path) : ImportMesh.addMesh(obj);
				first = false;
			}

			Data.deleteBlob(path);
		});
	}
}
