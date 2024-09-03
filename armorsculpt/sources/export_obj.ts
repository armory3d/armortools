
function export_obj_write_string(out: u8[], str: string) {
	for (let i: i32 = 0; i < str.length; ++i) {
		array_push(out, char_code_at(str, i));
	}
}

function export_obj_run(path: string, paint_objects: mesh_object_t[], apply_disp: bool = false) {
	let o: u8[] = [];
	export_obj_write_string(o, "# armorsculpt.org\n");

	let texpaint: image_t = project_layers[0].texpaint;
	let pixels: buffer_t = image_get_pixels(texpaint);
	let mesh: mesh_data_t = paint_objects[0].data;
	let inda: u32_array_t = mesh.index_arrays[0].values;

	let posa: i16_array_t = i16_array_create(inda.length * 4);
	for (let i: i32 = 0; i < inda.length; ++i) {
		let index: i32 = inda[i];
		posa[index * 4    ] = math_floor(buffer_get_f32(pixels, i * 16    ) * 32767);
		posa[index * 4 + 1] = math_floor(buffer_get_f32(pixels, i * 16 + 4) * 32767);
		posa[index * 4 + 2] = math_floor(buffer_get_f32(pixels, i * 16 + 8) * 32767);
	}

	let poff: i32 = 0;
	// for (let i: i32 = 0; i < paint_objects.length; ++i) {
		let p: mesh_object_t = paint_objects[0];
		// let mesh = p.data.raw;
		let inv: f32 = 1 / 32767;
		let sc: f32 = p.data.scale_pos * inv;
		// let posa = mesh.vertex_arrays[0].values;
		let len: i32 = math_floor(posa.length / 4);
		// let len = math_floor(inda.length);

		// Merge shared vertices and remap indices
		let posa2: i16_array_t = i16_array_create(len * 3);
		let posmap: i32_array_t = i32_array_create(len);

		let pi: i32 = 0;
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
		}

		export_obj_write_string(o, "o " + p.base.name + "\n");
		for (let i: i32 = 0; i < pi; ++i) {
			export_obj_write_string(o, "v ");
			let f: f32 = posa2[i * 3] * sc;
			let vx: string = f + "";
			export_obj_write_string(o, substring(vx, 0, string_index_of(vx, ".") + 7));
			export_obj_write_string(o, " ");
			f = posa2[i * 3 + 2] * sc;
			let vy: string = f + "";
			export_obj_write_string(o, substring(vy, 0, string_index_of(vy, ".") + 7));
			export_obj_write_string(o, " ");
			f = -posa2[i * 3 + 1] * sc;
			let vz: string = f + "";
			export_obj_write_string(o, substring(vz, 0, string_index_of(vz, ".") + 7));
			export_obj_write_string(o, "\n");
		}

		// let inda = mesh.index_arrays[0].values;
		for (let i: i32 = 0; i < math_floor(inda.length / 3); ++i) {
			let pi1: i32 = posmap[inda[i * 3    ]] + 1 + poff;
			let pi2: i32 = posmap[inda[i * 3 + 1]] + 1 + poff;
			let pi3: i32 = posmap[inda[i * 3 + 2]] + 1 + poff;
			export_obj_write_string(o, "f ");
			export_obj_write_string(o, pi1 + "");
			export_obj_write_string(o, " ");
			export_obj_write_string(o, pi2 + "");
			export_obj_write_string(o, " ");
			export_obj_write_string(o, pi3 + "");
			export_obj_write_string(o, "\n");
		}
		poff += pi;
	// }

	if (!ends_with(path, ".obj")) {
		path += ".obj";
	}

	iron_file_save_bytes(path, o, 0);
}
