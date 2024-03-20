
let import_blend_mesh_eps: f32 = 1.0 / 32767;

function import_blend_mesh_run(path: string, replace_existing: bool = true) {
	let b: buffer_t = data_get_blob(path);
	let bl: BlendRaw = parser_blend_init(b);
	if (bl.dna == null) {
		console_error(strings_error3());
		return;
	}

	let obs: BlHandleRaw[] = parser_blend_get(bl, "Object");
	if (obs == null || obs.length == 0) {
		import_mesh_make_mesh(null, path);
		return;
	}

	let first: bool = true;
	for (let ob of obs) {
		if (bl_handle_get(ob, "type") != 1) {
			continue;
		}

		let name: string = bl_handle_get(bl_handle_get(ob, "id"), "name");
		name = substring(name, 2, name.length);

		let m: any = bl_handle_get(ob, "data", 0, "Mesh");
		if (m == null) {
			continue;
		}

		let totpoly: i32 = bl_handle_get(m, "totpoly");
		if (totpoly == 0) {
			continue;
		}

		let numtri: i32 = 0;
		for (let i: i32 = 0; i < totpoly; ++i) {
			let poly: any = bl_handle_get(m, "mpoly", i);
			let totloop: i32 = bl_handle_get(poly, "totloop");
			numtri += totloop - 2;
		}
		let inda: u32_array_t = u32_array_create(numtri * 3);
		for (let i: i32 = 0; i < inda.length; ++i) {
			inda[i] = i;
		}

		let posa32: f32_array_t = f32_array_create(numtri * 3 * 4);
		let posa: i16_array_t = i16_array_create(numtri * 3 * 4);
		let nora: i16_array_t = i16_array_create(numtri * 3 * 2);

		// pdata, 25 == CD_MPOLY
		// let vdata: any = get(m, "vdata");
		// let codata: any = null;
		// let codata_pos: i32 = 0;
		// for (let i: i32 = 0; i < get(vdata, "totlayer"); ++i) {
		// 	let l: any = get(vdata, "layers", i);
		// 	if (get(l, "type") == 0) { // CD_MVERT
		// 		let ptr: any = get(l, "data");
		// 		codata_pos = bl.get(map, ptr).pos;
		// 		codata = l;
		// 	}
		// }

		let ldata: any = bl_handle_get(m, "ldata");
		let uvdata: any = null;
		let uvdata_pos: i32 = 0;
		let coldata: any = null;
		let coldata_pos: i32 = 0;

		for (let i: i32 = 0; i < bl_handle_get(ldata, "totlayer"); ++i) {
			let l: any = bl_handle_get(ldata, "layers", i);
			if (bl_handle_get(l, "type") == 16) { // CD_MLOOPUV
				let ptr: any = bl_handle_get(l, "data");
				uvdata_pos = map_get(bl.map, ptr).pos;
				uvdata = l;
			}
			else if (bl_handle_get(l, "type") == 17) { // CD_PROP_BYTE_COLOR
				let ptr: any = bl_handle_get(l, "data");
				coldata_pos = map_get(bl.map, ptr).pos;
				coldata = l;
			}
			// CD_MLOOP == 26
		}

		let hasuv: bool = uvdata != null;
		let texa: i16_array_t = hasuv ? i16_array_create(numtri * 3 * 2) : null;
		let hascol: bool = context_raw.parse_vcols && coldata != null;
		let cola: i16_array_t = hascol ? i16_array_create(numtri * 3 * 4) : null;

		let tri: i32 = 0;
		let vec0: vec4_t = vec4_create();
		let vec1: vec4_t = vec4_create();
		let vec2: vec4_t = vec4_create();
		for (let i: i32 = 0; i < totpoly; ++i) {
			let poly: any = bl_handle_get(m, "mpoly", i);
			// let smooth: bool = get(poly, "flag") & 1 == 1; // ME_SMOOTH
			let smooth: bool = false; // TODO: fetch smooth normals
			let loopstart: i32 = bl_handle_get(poly, "loopstart");
			let totloop: i32 = bl_handle_get(poly, "totloop");
			if (totloop <= 4) { // Convex, fan triangulation
				let v0: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart + totloop - 1);
				let v1: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart);
				let co0: any = bl_handle_get(v0, "co");
				let co1: any = bl_handle_get(v1, "co");
				let no0: any = bl_handle_get(v0, "no");
				let no1: any = bl_handle_get(v1, "no");
				if (smooth) {
					vec4_normalize(vec4_set(vec0, no0[0] / 32767, no0[1] / 32767, no0[2] / 32767)); // shortmax
					vec4_normalize(vec4_set(vec1, no1[0] / 32767, no1[1] / 32767, no1[2] / 32767));
				}
				let uv0: f32_array_t = null;
				let uv1: f32_array_t = null;
				let uv2: f32_array_t = null;
				if (hasuv) {
					bl.pos = uvdata_pos + (loopstart + totloop - 1) * 4 * 3; // * 3 = x, y, flag
					uv0 = parser_blend_read_f32array(bl, 2);
					if (uv0[0] > 1.0 + import_blend_mesh_eps) {
						uv0[0] = uv0[0] - math_floor(uv0[0]);
					}
					if (uv0[1] > 1.0 + import_blend_mesh_eps) {
						uv0[1] = uv0[1] - math_floor(uv0[1]);
					}
					bl.pos = uvdata_pos + (loopstart) * 4 * 3;
					uv1 = parser_blend_read_f32array(bl, 2);
					if (uv1[0] > 1.0 + import_blend_mesh_eps) {
						uv1[0] = uv1[0] - math_floor(uv1[0]);
					}
					if (uv1[1] > 1.0 + import_blend_mesh_eps) {
						uv1[1] = uv1[1] - math_floor(uv1[1]);
					}
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
					col0r = parser_blend_read_i8(bl);
					col0g = parser_blend_read_i8(bl);
					col0b = parser_blend_read_i8(bl);
					bl.pos = coldata_pos + (loopstart) * 1 * 4;
					col1r = parser_blend_read_i8(bl);
					col1g = parser_blend_read_i8(bl);
					col1b = parser_blend_read_i8(bl);
				}
				for (let j: i32 = 0; j < totloop - 2; ++j) {
					let v2: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart + j + 1);
					let co2: any = bl_handle_get(v2, "co");
					let no2: any = bl_handle_get(v2, "no");
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
					posa[tri * 12 + 3] = math_floor(vec0.z * 32767);
					posa[tri * 12 + 7] = math_floor((smooth ? vec1.z : vec0.z) * 32767);
					posa[tri * 12 + 11] = math_floor((smooth ? vec2.z : vec0.z) * 32767);
					nora[tri * 6    ] = math_floor(vec0.x * 32767);
					nora[tri * 6 + 1] = math_floor(vec0.y * 32767);
					nora[tri * 6 + 2] = math_floor((smooth ? vec1.x : vec0.x) * 32767);
					nora[tri * 6 + 3] = math_floor((smooth ? vec1.y : vec0.y) * 32767);
					nora[tri * 6 + 4] = math_floor((smooth ? vec2.x : vec0.x) * 32767);
					nora[tri * 6 + 5] = math_floor((smooth ? vec2.y : vec0.y) * 32767);
					co1 = co2;
					no1 = no2;
					vec4_set_from(vec1, vec2);
					if (hasuv) {
						bl.pos = uvdata_pos + (loopstart + j + 1) * 4 * 3;
						uv2 = parser_blend_read_f32array(bl, 2);
						if (uv2[0] > 1.0 + import_blend_mesh_eps) {
							uv2[0] = uv2[0] - math_floor(uv2[0]);
						}
						if (uv2[1] > 1.0 + import_blend_mesh_eps) {
							uv2[1] = uv2[1] - math_floor(uv2[1]);
						}
						texa[tri * 6    ] = math_floor(uv0[0] * 32767);
						texa[tri * 6 + 1] = math_floor((1.0 - uv0[1]) * 32767);
						texa[tri * 6 + 2] = math_floor(uv1[0] * 32767);
						texa[tri * 6 + 3] = math_floor((1.0 - uv1[1]) * 32767);
						texa[tri * 6 + 4] = math_floor(uv2[0] * 32767);
						texa[tri * 6 + 5] = math_floor((1.0 - uv2[1]) * 32767);
						uv1 = uv2;
					}
					if (hascol) {
						bl.pos = coldata_pos + (loopstart + j + 1) * 1 * 4;
						col2r = parser_blend_read_i8(bl);
						col2g = parser_blend_read_i8(bl);
						col2b = parser_blend_read_i8(bl);
						cola[tri * 12    ] = col0r * 128;
						cola[tri * 12 + 1] = col0g * 128;
						cola[tri * 12 + 2] = col0b * 128;
						cola[tri * 12 + 3] = col1r * 128;
						cola[tri * 12 + 4] = col1g * 128;
						cola[tri * 12 + 5] = col1b * 128;
						cola[tri * 12 + 6] = col2r * 128;
						cola[tri * 12 + 7] = col2g * 128;
						cola[tri * 12 + 8] = col2b * 128;
						col1r = col2r;
						col1g = col2g;
						col1b = col2b;
					}
					tri++;
				}
			}
			else { // Convex or concave, ear clipping
				let va: i32[] = [];
				for (let i: i32 = 0; i < totloop; ++i) {
					array_push(va, loopstart + i);
				}
				let v0: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart);
				let v1: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart + 1);
				let v2: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart + 2);
				let co0: any = bl_handle_get(v0, "co");
				let co1: any = bl_handle_get(v1, "co");
				let co2: any = bl_handle_get(v2, "co");
				vec4_set(vec2, co2[0], co2[1], co2[2]);
				vec4_set(vec1, co1[0], co1[1], co1[2]);
				vec4_sub_vecs(vec0, vec2, vec1);
				vec4_set(vec2, co0[0], co0[1], co0[2]);
				vec4_sub_vecs(vec1, vec2, vec1);
				vec4_cross(vec0, vec1);
				vec4_normalize(vec0, );

				let nx: f32 = vec0.x;
				let ny: f32 = vec0.y;
				let nz: f32 = vec0.z;
				let nxabs: f32 = math_abs(nx);
				let nyabs: f32 = math_abs(ny);
				let nzabs: f32 = math_abs(nz);
				let flip: bool = nx + ny + nz > 0;
				let axis: i32 = nxabs > nyabs && nxabs > nzabs ? 0 : nyabs > nxabs && nyabs > nzabs ? 1 : 2;
				let axis0: i32 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
				let axis1: i32 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

				let winding: f32 = 0.0;
				for (let i: i32 = 0; i < totloop; ++i) {
					let v0: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart + i);
					let v1: BlHandleRaw = import_blend_mesh_get_mvert_v(m, loopstart + ((i + 1) % totloop));
					let co0: any = bl_handle_get(v0, "co");
					let co1: any = bl_handle_get(v1, "co");
					winding += (co1[axis0] - co0[axis0]) * (co1[axis1] + co0[axis1]);
				}
				flip = winding > 0 ? nx + ny + nz > 0 : nx + ny + nz < 0;
				axis0 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
				axis1 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

				let vi: i32 = totloop;
				let loops: i32 = 0;
				let i: i32 = -1;
				while (vi > 2 && loops++ < vi) {
					i = (i + 1) % vi;
					let i1: i32 = (i + 1) % vi;
					let i2: i32 = (i + 2) % vi;
					let v0: BlHandleRaw = import_blend_mesh_get_mvert_v(m, va[i ]);
					let v1: BlHandleRaw = import_blend_mesh_get_mvert_v(m, va[i1]);
					let v2: BlHandleRaw = import_blend_mesh_get_mvert_v(m, va[i2]);
					let co0: any = bl_handle_get(v0, "co");
					let co1: any = bl_handle_get(v1, "co");
					let co2: any = bl_handle_get(v2, "co");
					let v0x: f32 = co0[axis0];
					let v0y: f32 = co0[axis1];
					let v1x: f32 = co1[axis0];
					let v1y: f32 = co1[axis1];
					let v2x: f32 = co2[axis0];
					let v2y: f32 = co2[axis1];

					let e0x: f32 = v0x - v1x; // Not an interior vertex
					let e0y: f32 = v0y - v1y;
					let e1x: f32 = v2x - v1x;
					let e1y: f32 = v2y - v1y;
					let cross: f32 = e0x * e1y - e0y * e1x;
					if (cross <= 0) {
						continue;
					}

					let overlap: bool = false; // Other vertex found inside this triangle
					for (let j: i32 = 0; j < vi - 3; ++j) {
						let j0: i32 = (i + 3 + j) % vi;
						let v: BlHandleRaw = import_blend_mesh_get_mvert_v(m, va[j0]);
						let co: any = bl_handle_get(v, "co");
						let px: f32 = co[axis0];
						let py: f32 = co[axis1];

						if (util_mesh_pnpoly(v0x, v0y, v1x, v1y, v2x, v2y, px, py)) {
							overlap = true;
							break;
						}
					}
					if (overlap) {
						continue;
					}

					// Found ear
					{
						let no0: any = bl_handle_get(v0, "no");
						let no1: any = bl_handle_get(v1, "no");
						let no2: any = bl_handle_get(v2, "no");
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
						let uv0: f32_array_t = null;
						let uv1: f32_array_t = null;
						let uv2: f32_array_t = null;
						if (hasuv) {
							bl.pos = uvdata_pos + (va[i ]) * 4 * 3;
							uv0 = parser_blend_read_f32array(bl, 2);
							if (uv0[0] > 1.0 + import_blend_mesh_eps) {
								uv0[0] = uv0[0] - math_floor(uv0[0]);
							}
							if (uv0[1] > 1.0 + import_blend_mesh_eps) {
								uv0[1] = uv0[1] - math_floor(uv0[1]);
							}
							bl.pos = uvdata_pos + (va[i1]) * 4 * 3;
							uv1 = parser_blend_read_f32array(bl, 2);
							if (uv1[0] > 1.0 + import_blend_mesh_eps) {
								uv1[0] = uv1[0] - math_floor(uv1[0]);
							}
							if (uv1[1] > 1.0 + import_blend_mesh_eps) {
								uv1[1] = uv1[1] - math_floor(uv1[1]);
							}
							bl.pos = uvdata_pos + (va[i2]) * 4 * 3;
							uv2 = parser_blend_read_f32array(bl, 2);
							if (uv2[0] > 1.0 + import_blend_mesh_eps) {
								uv2[0] = uv2[0] - math_floor(uv2[0]);
							}
							if (uv2[1] > 1.0 + import_blend_mesh_eps) {
								uv2[1] = uv2[1] - math_floor(uv2[1]);
							}
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
							col0r = parser_blend_read_i8(bl);
							col0g = parser_blend_read_i8(bl);
							col0b = parser_blend_read_i8(bl);
							bl.pos = coldata_pos + (va[i1]) * 1 * 4;
							col1r = parser_blend_read_i8(bl);
							col1g = parser_blend_read_i8(bl);
							col1b = parser_blend_read_i8(bl);
							bl.pos = coldata_pos + (va[i2]) * 1 * 4;
							col2r = parser_blend_read_i8(bl);
							col2g = parser_blend_read_i8(bl);
							col2b = parser_blend_read_i8(bl);
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
						posa[tri * 12 + 3] = math_floor(vec0.z * 32767);
						posa[tri * 12 + 7] = math_floor((smooth ? vec1.z : vec0.z) * 32767);
						posa[tri * 12 + 11] = math_floor((smooth ? vec2.z : vec0.z) * 32767);
						nora[tri * 6    ] = math_floor(vec0.x * 32767);
						nora[tri * 6 + 1] = math_floor(vec0.y * 32767);
						nora[tri * 6 + 2] = math_floor((smooth ? vec1.x : vec0.x) * 32767);
						nora[tri * 6 + 3] = math_floor((smooth ? vec1.y : vec0.y) * 32767);
						nora[tri * 6 + 4] = math_floor((smooth ? vec2.x : vec0.x) * 32767);
						nora[tri * 6 + 5] = math_floor((smooth ? vec2.y : vec0.y) * 32767);
						if (hasuv) {
							texa[tri * 6    ] = math_floor(uv0[0] * 32767);
							texa[tri * 6 + 1] = math_floor((1.0 - uv0[1]) * 32767);
							texa[tri * 6 + 2] = math_floor(uv1[0] * 32767);
							texa[tri * 6 + 3] = math_floor((1.0 - uv1[1]) * 32767);
							texa[tri * 6 + 4] = math_floor(uv2[0] * 32767);
							texa[tri * 6 + 5] = math_floor((1.0 - uv2[1]) * 32767);
						}
						if (hascol) {
							cola[tri * 12    ] = col0r * 128;
							cola[tri * 12 + 1] = col0g * 128;
							cola[tri * 12 + 2] = col0b * 128;
							cola[tri * 12 + 3] = col1r * 128;
							cola[tri * 12 + 4] = col1g * 128;
							cola[tri * 12 + 5] = col1b * 128;
							cola[tri * 12 + 6] = col2r * 128;
							cola[tri * 12 + 7] = col2g * 128;
							cola[tri * 12 + 8] = col2b * 128;
						}
						tri++;
					}

					for (let j: i32 = ((i + 1) % vi); j < vi - 1; ++j) { // Consume vertex
						va[j] = va[j + 1];
					}
					vi--;
					i--;
					loops = 0;
				}
			}
		}

		// Apply world matrix
		let obmat: any = bl_handle_get(ob, "obmat", 0, "float", 16);
		let mat: mat4_t = mat4_transpose(mat4_from_f32_array(obmat));
		let v: vec4_t = vec4_create();
		for (let i: i32 = 0; i < math_floor(posa32.length / 3); ++i) {
			vec4_set(v, posa32[i * 3], posa32[i * 3 + 1], posa32[i * 3 + 2]);
			vec4_apply_mat4(v, mat);
			posa32[i * 3    ] = v.x;
			posa32[i * 3 + 1] = v.y;
			posa32[i * 3 + 2] = v.z;
		}
		mat4_get_inv(mat, mat);
		mat4_transpose3x3(mat);
		mat.m[12] = mat.m[13] = mat.m[14] = mat.m[15] = 0;
		for (let i: i32 = 0; i < math_floor(nora.length / 2); ++i) {
			vec4_set(v, nora[i * 2] / 32767, nora[i * 2 + 1] / 32767, posa[i * 4 + 3] / 32767);
			vec4_apply_mat(v, mat);
			vec4_normalize(v);
			nora[i * 2    ] = math_floor(v.x * 32767);
			nora[i * 2 + 1] = math_floor(v.y * 32767);
			posa[i * 4 + 3] = math_floor(v.z * 32767);
		}

		// Pack positions to (-1, 1) range
		let scale_pos: f32 = 0.0;
		for (let i: i32 = 0; i < posa32.length; ++i) {
			let f: f32 = math_abs(posa32[i]);
			if (scale_pos < f) {
				scale_pos = f;
			}
		}
		let inv: f32 = 1 / scale_pos;
		for (let i: i32 = 0; i < math_floor(posa32.length / 3); ++i) {
			posa[i * 4    ] = math_floor(posa32[i * 3    ] * 32767 * inv);
			posa[i * 4 + 1] = math_floor(posa32[i * 3 + 1] * 32767 * inv);
			posa[i * 4 + 2] = math_floor(posa32[i * 3 + 2] * 32767 * inv);
		}

		let obj: any = {
			posa: posa,
			nora: nora,
			texa: texa,
			cola: cola,
			inda: inda,
			name: name,
			scale_pos: scale_pos,
			scale_tex: 1.0
		};

		if (first && replace_existing) {
			import_mesh_make_mesh(obj, path);
		}
		else {
			import_mesh_add_mesh(obj);
		}
		first = false;
	}

	data_delete_blob(path);
}

function import_blend_mesh_get_mvert_v(m: BlHandleRaw, loopstart: i32) {
	return bl_handle_get(m, "mvert", bl_handle_get(bl_handle_get(m, "mloop", loopstart), "v"));
}
