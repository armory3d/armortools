
class ImportBlendMesh {

	static eps = 1.0 / 32767;

	static run = (path: string, replaceExisting = true) => {
		data_get_blob(path, (b: ArrayBuffer) => {
			let bl = ParserBlend.init(b);
			if (bl.dna == null) {
				Console.error(Strings.error3());
				return;
			}

			let obs = ParserBlend.get(bl, "Object");
			if (obs == null || obs.length == 0) { ImportMesh.makeMesh(null, path); return; }

			let first = true;
			for (let ob of obs) {
				if (BlHandle.get(ob, "type") != 1) continue;

				let name: string = BlHandle.get(BlHandle.get(ob, "id"), "name");
				name = name.substring(2, name.length);

				let m: any = BlHandle.get(ob, "data", 0, "Mesh");
				if (m == null) continue;

				let totpoly = BlHandle.get(m, "totpoly");
				if (totpoly == 0) continue;

				let numtri = 0;
				for (let i = 0; i < totpoly; ++i) {
					let poly = BlHandle.get(m, "mpoly", i);
					let totloop = BlHandle.get(poly, "totloop");
					numtri += totloop - 2;
				}
				let inda = new Uint32Array(numtri * 3);
				for (let i = 0; i < inda.length; ++i) inda[i] = i;

				let posa32 = new Float32Array(numtri * 3 * 4);
				let posa = new Int16Array(numtri * 3 * 4);
				let nora = new Int16Array(numtri * 3 * 2);

				// pdata, 25 == CD_MPOLY
				// let vdata: any = BlHandle.get(m, "vdata");
				// let codata: any = null;
				// let codata_pos = 0;
				// for (let i = 0; i < BlHandle.get(vdata, "totlayer"); ++i) {
				// 	let l = BlHandle.get(vdata, "layers", i);
				// 	if (BlHandle.get(l, "type") == 0) { // CD_MVERT
				// 		let ptr: any = BlHandle.get(l, "data");
				// 		codata_pos = bl.BlHandle.get(map, ptr).pos;
				// 		codata = l;
				// 	}
				// }

				let ldata: any = BlHandle.get(m, "ldata");
				let uvdata: any = null;
				let uvdata_pos = 0;
				let coldata: any = null;
				let coldata_pos = 0;

				for (let i = 0; i < BlHandle.get(ldata, "totlayer"); ++i) {
					let l = BlHandle.get(ldata, "layers", i);
					if (BlHandle.get(l, "type") == 16) { // CD_MLOOPUV
						let ptr: any = BlHandle.get(l, "data");
						uvdata_pos = bl.map.get(ptr).pos;
						uvdata = l;
					}
					else if (BlHandle.get(l, "type") == 17) { // CD_PROP_BYTE_COLOR
						let ptr: any = BlHandle.get(l, "data");
						coldata_pos = bl.map.get(ptr).pos;
						coldata = l;
					}
					// CD_MLOOP == 26
				}

				let hasuv = uvdata != null;
				let texa = hasuv ? new Int16Array(numtri * 3 * 2) : null;
				let hascol = Context.raw.parseVCols && coldata != null;
				let cola = hascol ? new Int16Array(numtri * 3 * 3) : null;

				let tri = 0;
				let vec0 = vec4_create();
				let vec1 = vec4_create();
				let vec2 = vec4_create();
				for (let i = 0; i < totpoly; ++i) {
					let poly = BlHandle.get(m, "mpoly", i);
					// let smooth = BlHandle.get(poly, "flag") & 1 == 1; // ME_SMOOTH
					let smooth = false; // TODO: fetch smooth normals
					let loopstart = BlHandle.get(poly, "loopstart");
					let totloop = BlHandle.get(poly, "totloop");
					if (totloop <= 4) { // Convex, fan triangulation
						let v0 = ImportBlendMesh.get_mvert_v(m, loopstart + totloop - 1);
						let v1 = ImportBlendMesh.get_mvert_v(m, loopstart);
						let co0 = BlHandle.get(v0, "co");
						let co1 = BlHandle.get(v1, "co");
						let no0 = BlHandle.get(v0, "no");
						let no1 = BlHandle.get(v1, "no");
						if (smooth) {
							vec4_normalize(vec4_set(vec0, no0[0] / 32767, no0[1] / 32767, no0[2] / 32767)); // shortmax
							vec4_normalize(vec4_set(vec1, no1[0] / 32767, no1[1] / 32767, no1[2] / 32767));
						}
						let uv0: Float32Array = null;
						let uv1: Float32Array = null;
						let uv2: Float32Array = null;
						if (hasuv) {
							bl.pos = uvdata_pos + (loopstart + totloop - 1) * 4 * 3; // * 3 = x, y, flag
							uv0 = ParserBlend.readf32array(bl, 2);
							if (uv0[0] > 1.0 + ImportBlendMesh.eps) uv0[0] = uv0[0] - Math.floor(uv0[0]);
							if (uv0[1] > 1.0 + ImportBlendMesh.eps) uv0[1] = uv0[1] - Math.floor(uv0[1]);
							bl.pos = uvdata_pos + (loopstart) * 4 * 3;
							uv1 = ParserBlend.readf32array(bl, 2);
							if (uv1[0] > 1.0 + ImportBlendMesh.eps) uv1[0] = uv1[0] - Math.floor(uv1[0]);
							if (uv1[1] > 1.0 + ImportBlendMesh.eps) uv1[1] = uv1[1] - Math.floor(uv1[1]);
						}
						let col0r: i32 = 0;
						let col0g: i32 = 0;
						let col0b: i32 = 0;
						let col1r: i32 = 0;
						let col1g: i32 = 0;
						let col1b: i32 = 0;
						let col2r: i32 = 0;
						let col2g: i32 = 0;
						let col2b: i32 = 0;
						if (hascol) {
							bl.pos = coldata_pos + (loopstart + totloop - 1) * 1 * 4; // * 4 = r, g, b, a
							col0r = ParserBlend.read8(bl);
							col0g = ParserBlend.read8(bl);
							col0b = ParserBlend.read8(bl);
							bl.pos = coldata_pos + (loopstart) * 1 * 4;
							col1r = ParserBlend.read8(bl);
							col1g = ParserBlend.read8(bl);
							col1b = ParserBlend.read8(bl);
						}
						for (let j = 0; j < totloop - 2; ++j) {
							let v2 = ImportBlendMesh.get_mvert_v(m, loopstart + j + 1);
							let co2 = BlHandle.get(v2, "co");
							let no2 = BlHandle.get(v2, "no");
							if (smooth) {
								vec4_normalize(vec4_set(vec2, no2[0] / 32767, no2[1] / 32767, no2[2] / 32767));
							}
							else {
								vec4_set(vec2, co2[0], co2[1], co2[2]);
								vec4_set(vec1, co1[0], co1[1], co1[2]);
								vec4_sub_vecs(vec0, vec2, vec1);
								vec4_set(vec2, co0[0], co0[1], co0[2]);
								vec4_sub_vecs(vec1, vec2, vec1);
								vec4_cross(vec0, vec1);
								vec4_normalize(vec0);
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
							posa[tri * 12 + 3] = Math.floor(vec0.z * 32767);
							posa[tri * 12 + 7] = Math.floor((smooth ? vec1.z : vec0.z) * 32767);
							posa[tri * 12 + 11] = Math.floor((smooth ? vec2.z : vec0.z) * 32767);
							nora[tri * 6    ] = Math.floor(vec0.x * 32767);
							nora[tri * 6 + 1] = Math.floor(vec0.y * 32767);
							nora[tri * 6 + 2] = Math.floor((smooth ? vec1.x : vec0.x) * 32767);
							nora[tri * 6 + 3] = Math.floor((smooth ? vec1.y : vec0.y) * 32767);
							nora[tri * 6 + 4] = Math.floor((smooth ? vec2.x : vec0.x) * 32767);
							nora[tri * 6 + 5] = Math.floor((smooth ? vec2.y : vec0.y) * 32767);
							co1 = co2;
							no1 = no2;
							vec4_set_from(vec1, vec2);
							if (hasuv) {
								bl.pos = uvdata_pos + (loopstart + j + 1) * 4 * 3;
								uv2 = ParserBlend.readf32array(bl, 2);
								if (uv2[0] > 1.0 + ImportBlendMesh.eps) uv2[0] = uv2[0] - Math.floor(uv2[0]);
								if (uv2[1] > 1.0 + ImportBlendMesh.eps) uv2[1] = uv2[1] - Math.floor(uv2[1]);
								texa[tri * 6    ] = Math.floor(uv0[0] * 32767);
								texa[tri * 6 + 1] = Math.floor((1.0 - uv0[1]) * 32767);
								texa[tri * 6 + 2] = Math.floor(uv1[0] * 32767);
								texa[tri * 6 + 3] = Math.floor((1.0 - uv1[1]) * 32767);
								texa[tri * 6 + 4] = Math.floor(uv2[0] * 32767);
								texa[tri * 6 + 5] = Math.floor((1.0 - uv2[1]) * 32767);
								uv1 = uv2;
							}
							if (hascol) {
								bl.pos = coldata_pos + (loopstart + j + 1) * 1 * 4;
								col2r = ParserBlend.read8(bl);
								col2g = ParserBlend.read8(bl);
								col2b = ParserBlend.read8(bl);
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
						let va: i32[] = [];
						for (let i = 0; i < totloop; ++i) va.push(loopstart + i);
						let v0 = ImportBlendMesh.get_mvert_v(m, loopstart);
						let v1 = ImportBlendMesh.get_mvert_v(m, loopstart + 1);
						let v2 = ImportBlendMesh.get_mvert_v(m, loopstart + 2);
						let co0 = BlHandle.get(v0, "co");
						let co1 = BlHandle.get(v1, "co");
						let co2 = BlHandle.get(v2, "co");
						vec4_set(vec2, co2[0], co2[1], co2[2]);
						vec4_set(vec1, co1[0], co1[1], co1[2]);
						vec4_sub_vecs(vec0, vec2, vec1);
						vec4_set(vec2, co0[0], co0[1], co0[2]);
						vec4_sub_vecs(vec1, vec2, vec1);
						vec4_cross(vec0, vec1);
						vec4_normalize(vec0, );

						let nx = vec0.x;
						let ny = vec0.y;
						let nz = vec0.z;
						let nxabs = Math.abs(nx);
						let nyabs = Math.abs(ny);
						let nzabs = Math.abs(nz);
						let flip = nx + ny + nz > 0;
						let axis = nxabs > nyabs && nxabs > nzabs ? 0 : nyabs > nxabs && nyabs > nzabs ? 1 : 2;
						let axis0 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
						let axis1 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

						let winding = 0.0;
						for (let i = 0; i < totloop; ++i) {
							let v0 = ImportBlendMesh.get_mvert_v(m, loopstart + i);
							let v1 = ImportBlendMesh.get_mvert_v(m, loopstart + ((i + 1) % totloop));
							let co0 = BlHandle.get(v0, "co");
							let co1 = BlHandle.get(v1, "co");
							winding += (co1[axis0] - co0[axis0]) * (co1[axis1] + co0[axis1]);
						}
						flip = winding > 0 ? nx + ny + nz > 0 : nx + ny + nz < 0;
						axis0 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
						axis1 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

						let vi = totloop;
						let loops = 0;
						let i = -1;
						while (vi > 2 && loops++ < vi) {
							i = (i + 1) % vi;
							let i1 = (i + 1) % vi;
							let i2 = (i + 2) % vi;
							let v0 = ImportBlendMesh.get_mvert_v(m, va[i ]);
							let v1 = ImportBlendMesh.get_mvert_v(m, va[i1]);
							let v2 = ImportBlendMesh.get_mvert_v(m, va[i2]);
							let co0 = BlHandle.get(v0, "co");
							let co1 = BlHandle.get(v1, "co");
							let co2 = BlHandle.get(v2, "co");
							let v0x = co0[axis0];
							let v0y = co0[axis1];
							let v1x = co1[axis0];
							let v1y = co1[axis1];
							let v2x = co2[axis0];
							let v2y = co2[axis1];

							let e0x = v0x - v1x; // Not an interior vertex
							let e0y = v0y - v1y;
							let e1x = v2x - v1x;
							let e1y = v2y - v1y;
							let cross = e0x * e1y - e0y * e1x;
							if (cross <= 0) continue;

							let overlap = false; // Other vertex found inside this triangle
							for (let j = 0; j < vi - 3; ++j) {
								let j0 = (i + 3 + j) % vi;
								let v = ImportBlendMesh.get_mvert_v(m, va[j0]);
								let co = BlHandle.get(v, "co");
								let px = co[axis0];
								let py = co[axis1];

								if (UtilMesh.pnpoly(v0x, v0y, v1x, v1y, v2x, v2y, px, py)) {
									overlap = true;
									break;
								}
							}
							if (overlap) continue;

							// Found ear
							{
								let no0 = BlHandle.get(v0, "no");
								let no1 = BlHandle.get(v1, "no");
								let no2 = BlHandle.get(v2, "no");
								if (smooth) {
									vec4_normalize(vec4_set(vec0, no0[0] / 32767, no0[1] / 32767, no0[2] / 32767)); // shortmax
									vec4_normalize(vec4_set(vec1, no1[0] / 32767, no1[1] / 32767, no1[2] / 32767));
									vec4_normalize(vec4_set(vec2, no2[0] / 32767, no2[1] / 32767, no2[2] / 32767));
								}
								else {
									vec4_set(vec2, co2[0], co2[1], co2[2]);
									vec4_set(vec1, co1[0], co1[1], co1[2]);
									vec4_sub_vecs(vec0, vec2, vec1);
									vec4_set(vec2, co0[0], co0[1], co0[2]);
									vec4_sub_vecs(vec1, vec2, vec1);
									vec4_cross(vec0, vec1);
									vec4_normalize(vec0, );
								}
								let uv0: Float32Array = null;
								let uv1: Float32Array = null;
								let uv2: Float32Array = null;
								if (hasuv) {
									bl.pos = uvdata_pos + (va[i ]) * 4 * 3;
									uv0 = ParserBlend.readf32array(bl, 2);
									if (uv0[0] > 1.0 + ImportBlendMesh.eps) uv0[0] = uv0[0] - Math.floor(uv0[0]);
									if (uv0[1] > 1.0 + ImportBlendMesh.eps) uv0[1] = uv0[1] - Math.floor(uv0[1]);
									bl.pos = uvdata_pos + (va[i1]) * 4 * 3;
									uv1 = ParserBlend.readf32array(bl, 2);
									if (uv1[0] > 1.0 + ImportBlendMesh.eps) uv1[0] = uv1[0] - Math.floor(uv1[0]);
									if (uv1[1] > 1.0 + ImportBlendMesh.eps) uv1[1] = uv1[1] - Math.floor(uv1[1]);
									bl.pos = uvdata_pos + (va[i2]) * 4 * 3;
									uv2 = ParserBlend.readf32array(bl, 2);
									if (uv2[0] > 1.0 + ImportBlendMesh.eps) uv2[0] = uv2[0] - Math.floor(uv2[0]);
									if (uv2[1] > 1.0 + ImportBlendMesh.eps) uv2[1] = uv2[1] - Math.floor(uv2[1]);
								}
								let col0r: i32 = 0;
								let col0g: i32 = 0;
								let col0b: i32 = 0;
								let col1r: i32 = 0;
								let col1g: i32 = 0;
								let col1b: i32 = 0;
								let col2r: i32 = 0;
								let col2g: i32 = 0;
								let col2b: i32 = 0;
								if (hascol) {
									bl.pos = coldata_pos + (va[i ]) * 1 * 4;
									col0r = ParserBlend.read8(bl);
									col0g = ParserBlend.read8(bl);
									col0b = ParserBlend.read8(bl);
									bl.pos = coldata_pos + (va[i1]) * 1 * 4;
									col1r = ParserBlend.read8(bl);
									col1g = ParserBlend.read8(bl);
									col1b = ParserBlend.read8(bl);
									bl.pos = coldata_pos + (va[i2]) * 1 * 4;
									col2r = ParserBlend.read8(bl);
									col2g = ParserBlend.read8(bl);
									col2b = ParserBlend.read8(bl);
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
								posa[tri * 12 + 3] = Math.floor(vec0.z * 32767);
								posa[tri * 12 + 7] = Math.floor((smooth ? vec1.z : vec0.z) * 32767);
								posa[tri * 12 + 11] = Math.floor((smooth ? vec2.z : vec0.z) * 32767);
								nora[tri * 6    ] = Math.floor(vec0.x * 32767);
								nora[tri * 6 + 1] = Math.floor(vec0.y * 32767);
								nora[tri * 6 + 2] = Math.floor((smooth ? vec1.x : vec0.x) * 32767);
								nora[tri * 6 + 3] = Math.floor((smooth ? vec1.y : vec0.y) * 32767);
								nora[tri * 6 + 4] = Math.floor((smooth ? vec2.x : vec0.x) * 32767);
								nora[tri * 6 + 5] = Math.floor((smooth ? vec2.y : vec0.y) * 32767);
								if (hasuv) {
									texa[tri * 6    ] = Math.floor(uv0[0] * 32767);
									texa[tri * 6 + 1] = Math.floor((1.0 - uv0[1]) * 32767);
									texa[tri * 6 + 2] = Math.floor(uv1[0] * 32767);
									texa[tri * 6 + 3] = Math.floor((1.0 - uv1[1]) * 32767);
									texa[tri * 6 + 4] = Math.floor(uv2[0] * 32767);
									texa[tri * 6 + 5] = Math.floor((1.0 - uv2[1]) * 32767);
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

							for (let j = ((i + 1) % vi); j < vi - 1; ++j) { // Consume vertex
								va[j] = va[j + 1];
							}
							vi--;
							i--;
							loops = 0;
						}
					}
				}

				// Apply world matrix
				let obmat = BlHandle.get(ob, "obmat", 0, "float", 16);
				let mat = mat4_transpose(mat4_from_f32_array(obmat));
				let v = vec4_create();
				for (let i = 0; i < Math.floor(posa32.length / 3); ++i) {
					vec4_set(v, posa32[i * 3], posa32[i * 3 + 1], posa32[i * 3 + 2]);
					vec4_apply_mat4(v, mat);
					posa32[i * 3    ] = v.x;
					posa32[i * 3 + 1] = v.y;
					posa32[i * 3 + 2] = v.z;
				}
				mat4_get_inv(mat, mat);
				mat4_transpose3x3(mat);
				mat.m[12] = mat.m[13] = mat.m[14] = mat.m[15] = 0;
				for (let i = 0; i < Math.floor(nora.length / 2); ++i) {
					vec4_set(v, nora[i * 2] / 32767, nora[i * 2 + 1] / 32767, posa[i * 4 + 3] / 32767);
					vec4_apply_mat(v, mat);
					vec4_normalize(v);
					nora[i * 2    ] = Math.floor(v.x * 32767);
					nora[i * 2 + 1] = Math.floor(v.y * 32767);
					posa[i * 4 + 3] = Math.floor(v.z * 32767);
				}

				// Pack positions to (-1, 1) range
				let scalePos = 0.0;
				for (let i = 0; i < posa32.length; ++i) {
					let f = Math.abs(posa32[i]);
					if (scalePos < f) scalePos = f;
				}
				let inv = 1 / scalePos;
				for (let i = 0; i < Math.floor(posa32.length / 3); ++i) {
					posa[i * 4    ] = Math.floor(posa32[i * 3    ] * 32767 * inv);
					posa[i * 4 + 1] = Math.floor(posa32[i * 3 + 1] * 32767 * inv);
					posa[i * 4 + 2] = Math.floor(posa32[i * 3 + 2] * 32767 * inv);
				}

				let obj = {posa: posa, nora: nora, texa: texa, cola: cola, inda: inda, name: name, scalePos: scalePos, scaleTes: 1.0};

				(first && replaceExisting) ? ImportMesh.makeMesh(obj, path) : ImportMesh.addMesh(obj);
				first = false;
			}

			data_delete_blob(path);
		});
	}

	static get_mvert_v(m: BlHandleRaw, loopstart: i32): BlHandleRaw {
		 return BlHandle.get(m, "mvert", BlHandle.get(BlHandle.get(m, "mloop", loopstart), "v"));
	}
}
