
function export_obj_write_string(out: u8[], str: string) {
	for (let i: i32 = 0; i < str.length; ++i) {
		array_push(out, char_code_at(str, i));
	}
}

function export_obj_run(path: string, paint_objects: mesh_object_t[], apply_disp: bool = false) {
	let o: u8[] = [];
	export_obj_write_string(o, "# armorpaint.org\n");

	let poff: i32 = 0;
	let noff: i32 = 0;
	let toff: i32 = 0;
	for (let i: i32 = 0; i < paint_objects.length; ++i) {
		let p: mesh_object_t = paint_objects[i];
		let mesh: mesh_data_t = p.data;
		let inv: f32 = 1 / 32767;
		let sc: f32 = p.data.scale_pos * inv;
		let posa: i16_array_t = mesh.vertex_arrays[0].values;
		let nora: i16_array_t = mesh.vertex_arrays[1].values;
		let texa: i16_array_t = mesh.vertex_arrays[2].values;
		let len: i32 = math_floor(posa.length / 4);

		// Merge shared vertices and remap indices
		let posa2: i16_array_t = i16_array_create(len * 3);
		let nora2: i16_array_t = i16_array_create(len * 3);
		let texa2: i16_array_t = i16_array_create(len * 2);
		let posmap: i32_array_t = i32_array_create(len);
		let normap: i32_array_t = i32_array_create(len);
		let texmap: i32_array_t = i32_array_create(len);

		let pi: i32 = 0;
		let ni: i32 = 0;
		let ti: i32 = 0;
		for (let i: i32 = 0; i < len; ++i) {
			let found: bool = false;
			for (let j: i32 = 0; j < pi; ++j) {
				if (posa2[j * 3    ] == posa[i * 4    ] &&
					posa2[j * 3 + 1] == posa[i * 4 + 1] &&
					posa2[j * 3 + 2] == posa[i * 4 + 2]) {
					posmap[i] = j;
					found = true;
					break;
				}
			}
			if (!found) {
				posmap[i] = pi;
				posa2[pi * 3    ] = posa[i * 4    ];
				posa2[pi * 3 + 1] = posa[i * 4 + 1];
				posa2[pi * 3 + 2] = posa[i * 4 + 2];
				pi++;
			}

			found = false;
			for (let j: i32 = 0; j < ni; ++j) {
				if (nora2[j * 3    ] == nora[i * 2    ] &&
					nora2[j * 3 + 1] == nora[i * 2 + 1] &&
					nora2[j * 3 + 2] == posa[i * 4 + 3]) {
					normap[i] = j;
					found = true;
					break;
				}
			}
			if (!found) {
				normap[i] = ni;
				nora2[ni * 3    ] = nora[i * 2    ];
				nora2[ni * 3 + 1] = nora[i * 2 + 1];
				nora2[ni * 3 + 2] = posa[i * 4 + 3];
				ni++;
			}

			found = false;
			for (let j: i32 = 0; j < ti; ++j) {
				if (texa2[j * 2    ] == texa[i * 2    ] &&
					texa2[j * 2 + 1] == texa[i * 2 + 1]) {
					texmap[i] = j;
					found = true;
					break;
				}
			}
			if (!found) {
				texmap[i] = ti;
				texa2[ti * 2    ] = texa[i * 2    ];
				texa2[ti * 2 + 1] = texa[i * 2 + 1];
				ti++;
			}
		}

		if (apply_disp) {
			// let height: buffer_t = layers[0].texpaint_pack.getPixels();
			// let res: i32 = layers[0].texpaint_pack.width;
			// let strength: f32 = 0.1;
			// for (let i: i32 = 0; i < len; ++i) {
			// 	let x: i32 = math_floor(texa2[i * 2    ] / 32767 * res);
			// 	let y: i32 = math_floor((1.0 - texa2[i * 2 + 1] / 32767) * res);
			// 	let h: f32 = (1.0 - height.get((y * res + x) * 4 + 3) / 255) * strength;
			// 	posa2[i * 3    ] -= math_floor(nora2[i * 3    ] * inv * h / sc);
			// 	posa2[i * 3 + 1] -= math_floor(nora2[i * 3 + 1] * inv * h / sc);
			// 	posa2[i * 3 + 2] -= math_floor(nora2[i * 3 + 2] * inv * h / sc);
			// }
		}

		export_obj_write_string(o, "o " + p.base.name + "\n");
		for (let i: i32 = 0; i < pi; ++i) {
			export_obj_write_string(o, "v ");
			let f: f32 = posa2[i * 3] * sc;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, " ");
			f = posa2[i * 3 + 2] * sc;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, " ");
			f = -posa2[i * 3 + 1] * sc;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, "\n");
		}
		for (let i: i32 = 0; i < ni; ++i) {
			export_obj_write_string(o, "vn ");
			let f: f32 = nora2[i * 3] * inv;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, " ");
			f = nora2[i * 3 + 2] * inv;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, " ");
			f = -nora2[i * 3 + 1] * inv;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, "\n");
		}
		for (let i: i32 = 0; i < ti; ++i) {
			export_obj_write_string(o, "vt ");
			let f: f32 = texa2[i * 2] * inv;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, " ");
			f = 1.0 - texa2[i * 2 + 1] * inv;
			export_obj_write_string(o, f + "");
			export_obj_write_string(o, "\n");
		}

		let inda: u32_array_t = mesh.index_arrays[0].values;
		for (let i: i32 = 0; i < math_floor(inda.length / 3); ++i) {
			let pi1: i32 = posmap[inda[i * 3    ]] + 1 + poff;
			let pi2: i32 = posmap[inda[i * 3 + 1]] + 1 + poff;
			let pi3: i32 = posmap[inda[i * 3 + 2]] + 1 + poff;
			let ni1: i32 = normap[inda[i * 3    ]] + 1 + noff;
			let ni2: i32 = normap[inda[i * 3 + 1]] + 1 + noff;
			let ni3: i32 = normap[inda[i * 3 + 2]] + 1 + noff;
			let ti1: i32 = texmap[inda[i * 3    ]] + 1 + toff;
			let ti2: i32 = texmap[inda[i * 3 + 1]] + 1 + toff;
			let ti3: i32 = texmap[inda[i * 3 + 2]] + 1 + toff;
			export_obj_write_string(o, "f ");
			export_obj_write_string(o, pi1 + "");
			export_obj_write_string(o, "/");
			export_obj_write_string(o, ti1 + "");
			export_obj_write_string(o, "/");
			export_obj_write_string(o, ni1 + "");
			export_obj_write_string(o, " ");
			export_obj_write_string(o, pi2 + "");
			export_obj_write_string(o, "/");
			export_obj_write_string(o, ti2 + "");
			export_obj_write_string(o, "/");
			export_obj_write_string(o, ni2 + "");
			export_obj_write_string(o, " ");
			export_obj_write_string(o, pi3 + "");
			export_obj_write_string(o, "/");
			export_obj_write_string(o, ti3 + "");
			export_obj_write_string(o, "/");
			export_obj_write_string(o, ni3 + "");
			export_obj_write_string(o, "\n");
		}
		poff += pi;
		noff += ni;
		toff += ti;
	}

	if (!ends_with(path, ".obj")) {
		path += ".obj";
	}
	iron_file_save_bytes(path, o, 0);
}
