
function import_obj_run(path: string, replace_existing: bool = true) {
	let i: split_type_t = context_raw.split_by;
	let is_udim: bool = i == split_type_t.UDIM;
	let split_code: i32 =
		(i == split_type_t.OBJECT || is_udim) ? char_code_at("o", 0) :
		 i == split_type_t.GROUP 		  	  ? char_code_at("g", 0) :
		 										char_code_at("u", 0); // usemtl

	let b: buffer_t = data_get_blob(path);

	if (is_udim) {
		let part: raw_mesh_t = iron_obj_parse(b, split_code, 0, is_udim);
		let name: string = part.name;
		for (let i: i32 = 0; i < part.udims.length; ++i) {
			let a: u32_array_t = part.udims[i];
			if (a.length == 0) {
				continue;
			}
			let u: i32 = i % part.udims_u;
			let v: i32 = math_floor(i / part.udims_u);
			let id: i32 = (1000 + v * 10 + u + 1);
			part.name = name + "." + id;
			part.inda = a;
			if (i == 0) {
				if (replace_existing) {
					import_mesh_make_mesh(part);
				}
				else {
					import_mesh_add_mesh(part);
				}
			}
			else {
				import_mesh_add_mesh(part);
			}
		}
	}
	else {
		let parts: raw_mesh_t[] = [];
		let part: raw_mesh_t = iron_obj_parse(b, split_code, 0, false);
		array_push(parts, part);
		while (part.has_next) {
			part = iron_obj_parse(b, split_code, part.pos, false);
			// This part does not contain faces (may contain lines only)
			if (part.inda.length == 0) {
				continue;
			}
			array_push(parts, part);
		}
		if (context_raw.split_by == split_type_t.MATERIAL) {
			let posa0: i16_array_t;
			let posa1: i16_array_t;
			let nora0: i16_array_t;
			let nora1: i16_array_t;
			let texa0: i16_array_t;
			let texa1: i16_array_t;
			let inda0: u32_array_t;
			let inda1: u32_array_t;
			// Merge to single object per material
			for (let i: i32 = 0; i < parts.length; ++i) {
				let j: i32 = i + 1;
				while (j < parts.length) {
					let iname: string = parts[i].name;
					let jname: string = parts[j].name;
					if (iname == jname) {
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
						let posa32: f32_array_t = f32_array_create(math_floor(posa0.length / 4) * 3 + math_floor(posa1.length / 4) * 3);
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
							if (scale_pos < f) {
								scale_pos = f;
							}
						}
						let inv: f32 = 32767 * (1 / scale_pos);
						let posa: i16_array_t = i16_array_create(posa0.length + posa1.length);
						for (let k: i32 = 0; k < math_floor(posa.length / 4); ++k) {
							posa[k * 4    ] = math_floor(posa32[k * 3    ] * inv);
							posa[k * 4 + 1] = math_floor(posa32[k * 3 + 1] * inv);
							posa[k * 4 + 2] = math_floor(posa32[k * 3 + 2] * inv);
						}
						for (let k: i32 = 0; k < math_floor(posa0.length / 4); ++k) {
							posa[k * 4 + 3] = posa0[k * 4 + 3];
						}
						for (let k: i32 = 0; k < math_floor(posa1.length / 4); ++k) {
							posa[posa0.length + k * 4 + 3] = posa1[k * 4 + 3];
						}
						// Merge normals and uvs
						let nora: i16_array_t = i16_array_create(nora0.length + nora1.length);
						let texa: i16_array_t = (texa0 != null && texa1 != null) ? i16_array_create(texa0.length + texa1.length) : null;
						let inda: u32_array_t = u32_array_create(inda0.length + inda1.length);

						for (let k: i32 = 0; k < nora0.length; ++k) {
							nora[k] = nora0[k];
						}
						for (let k: i32 = 0; k < nora1.length; ++k) {
							nora[k + nora0.length] = nora1[k];
						}

						if (texa != null) {
							for (let k: i32 = 0; k < texa0.length; ++k) {
								texa[k] = texa0[k];
							}
							for (let k: i32 = 0; k < texa1.length; ++k) {
								texa[k + texa0.length] = texa1[k];
							}
						}

						for (let k: i32 = 0; k < inda0.length; ++k) {
							inda[k] = inda0[k];
						}
						for (let k: i32 = 0; k < inda1.length; ++k) {
							inda[k + inda0.length] = inda1[k] + voff;
						}

						parts[i].posa = posa;
						parts[i].nora = nora;
						parts[i].texa = texa;
						parts[i].inda = inda;
						parts[i].scale_pos = scale_pos;
						array_splice(parts, j, 1);
					}
					else {
						j++;
					}
				}
			}
		}
		replace_existing ? import_mesh_make_mesh(parts[0]) : import_mesh_add_mesh(parts[0]);
		for (let i: i32 = 1; i < parts.length; ++i) {
			import_mesh_add_mesh(parts[i]);
		}
	}
	data_delete_blob(path);
}
