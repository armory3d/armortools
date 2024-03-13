
function import_obj_run(path: string, replace_existing: bool = true) {
	let i: split_type_t = context_raw.split_by;
	let is_udim: bool = i == split_type_t.UDIM;
	let split_code: i32 =
		(i == split_type_t.OBJECT || is_udim) ? "o".charCodeAt(0) :
		 i == split_type_t.GROUP 		  	  ? "g".charCodeAt(0) :
												"u".charCodeAt(0); // usemtl

	let b: ArrayBuffer = data_get_blob(path);

	if (is_udim) {
		let part: any = krom_io_obj_parse(b, split_code, 0, is_udim);
		let name: string = part.name;
		for (let i: i32 = 0; i < part.udims.length; ++i) {
			if (part.udims[i].length == 0) continue;
			let u: i32 = i % part.udims_u;
			let v: i32 = math_floor(i / part.udims_u);
			part.name = name + "." + (1000 + v * 10 + u + 1);
			part.inda = part.udims[i];
			i == 0 ? (replace_existing ? import_mesh_make_mesh(part, path) : import_mesh_add_mesh(part)) : import_mesh_add_mesh(part);
		}
	}
	else {
		let parts: any[] = [];
		let part: any = krom_io_obj_parse(b, split_code, 0, false);
		parts.push(part);
		while (part.has_next) {
			part = krom_io_obj_parse(b, split_code, part.pos, false);
			// This part does not contain faces (may contain lines only)
			if (part.inda.length == 0) {
				continue;
			}
			parts.push(part);
		}
		if (context_raw.split_by == split_type_t.MATERIAL) {
			let posa0: Int16Array;
			let posa1: Int16Array;
			let nora0: Int16Array;
			let nora1: Int16Array;
			let texa0: Int16Array;
			let texa1: Int16Array;
			let inda0: Uint32Array;
			let inda1: Uint32Array;
			// Merge to single object per material
			for (let i: i32 = 0; i < parts.length; ++i) {
				let j: i32 = i + 1;
				while (j < parts.length) {
					if (parts[i].name == parts[j].name) {
						posa0 = parts[i].posa;
						posa1 = parts[j].posa;
						nora0 = parts[i].nora;
						nora1 = parts[j].nora;
						texa0 = parts[i].texa != null ? parts[i].texa : null;
						texa1 = parts[j].texa != null ? parts[j].texa : null;
						inda0 = parts[i].inda;
						inda1 = parts[j].inda;
						let voff: i32 = math_floor(posa0.length / 4);
						// Repack merged positions
						let posa32: Float32Array = new Float32Array(math_floor(posa0.length / 4) * 3 + math_floor(posa1.length / 4) * 3);
						for (let k: i32 = 0; k < math_floor(posa0.length / 4); ++k) {
							posa32[k * 3    ] = posa0[k * 4    ] / 32767 * parts[i].scale_pos;
							posa32[k * 3 + 1] = posa0[k * 4 + 1] / 32767 * parts[i].scale_pos;
							posa32[k * 3 + 2] = posa0[k * 4 + 2] / 32767 * parts[i].scale_pos;
						}
						for (let k: i32 = 0; k < math_floor(posa1.length / 4); ++k) {
							posa32[voff * 3 + k * 3    ] = posa1[k * 4    ] / 32767 * parts[j].scale_pos;
							posa32[voff * 3 + k * 3 + 1] = posa1[k * 4 + 1] / 32767 * parts[j].scale_pos;
							posa32[voff * 3 + k * 3 + 2] = posa1[k * 4 + 2] / 32767 * parts[j].scale_pos;
						}
						let scale_pos: f32 = 0.0;
						for (let k: i32 = 0; k < posa32.length; ++k) {
							let f: f32 = math_abs(posa32[k]);
							if (scale_pos < f) scale_pos = f;
						}
						let inv: f32 = 32767 * (1 / scale_pos);
						let posa: Int16Array = new Int16Array(posa0.length + posa1.length);
						for (let k: i32 = 0; k < math_floor(posa.length / 4); ++k) {
							posa[k * 4    ] = math_floor(posa32[k * 3    ] * inv);
							posa[k * 4 + 1] = math_floor(posa32[k * 3 + 1] * inv);
							posa[k * 4 + 2] = math_floor(posa32[k * 3 + 2] * inv);
						}
						for (let k: i32 = 0; k < math_floor(posa0.length / 4); ++k) posa[k * 4 + 3] = posa0[k * 4 + 3];
						for (let k: i32 = 0; k < math_floor(posa1.length / 4); ++k) posa[posa0.length + k * 4 + 3] = posa1[k * 4 + 3];
						// Merge normals and uvs
						let nora: Int16Array = new Int16Array(nora0.length + nora1.length);
						let texa: Int16Array = (texa0 != null && texa1 != null) ? new Int16Array(texa0.length + texa1.length) : null;
						let inda: Uint32Array = new Uint32Array(inda0.length + inda1.length);
						nora.set(nora0);
						nora.set(nora1, nora0.length);
						if (texa != null) {
							texa.set(texa0);
							texa.set(texa1, texa0.length);
						}
						inda.set(inda0);
						for (let k: i32 = 0; k < inda1.length; ++k) inda[k + inda0.length] = inda1[k] + voff;
						parts[i].posa = posa;
						parts[i].nora = nora;
						parts[i].texa = texa;
						parts[i].inda = inda;
						parts[i].scale_pos = scale_pos;
						parts.splice(j, 1);
					}
					else j++;
				}
			}
		}
		replace_existing ? import_mesh_make_mesh(parts[0], path) : import_mesh_add_mesh(parts[0]);
		for (let i: i32 = 1; i < parts.length; ++i) {
			import_mesh_add_mesh(parts[i]);
		}
	}
	data_delete_blob(path);
}
