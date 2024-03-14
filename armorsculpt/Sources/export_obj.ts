
function export_obj_write_string(out: i32[], str: string) {
	for (let i = 0; i < str.length; ++i) {
		out.push(str.charCodeAt(i));
	}
}

function export_obj_run(path: string, paint_objects: mesh_object_t[], apply_displacement = false) {
	let o: i32[] = [];
	export_obj_write_string(o, "# armorsculpt.org\n");

	let texpaint = project_layers[0].texpaint;
	let pixels = image_get_pixels(texpaint);
	let pixelsView = new DataView(pixels);
	let mesh = paint_objects[0].data;
	let inda = mesh.index_arrays[0].values;

	let posa = new Int16Array(inda.length * 4);
	for (let i = 0; i < inda.length; ++i) {
		let index = inda[i];
		posa[index * 4    ] = math_floor(pixelsView.getFloat32(i * 16    , true) * 32767);
		posa[index * 4 + 1] = math_floor(pixelsView.getFloat32(i * 16 + 4, true) * 32767);
		posa[index * 4 + 2] = math_floor(pixelsView.getFloat32(i * 16 + 8, true) * 32767);
	}

	let poff = 0;
	// for (let p of paintObjects) {
		let p = paint_objects[0];
		// let mesh = p.data.raw;
		let inv = 1 / 32767;
		let sc = p.data.scale_pos * inv;
		// let posa = mesh.vertex_arrays[0].values;
		let len = math_floor(posa.length / 4);
		// let len = math_floor(inda.length);

		// Merge shared vertices and remap indices
		let posa2 = new Int16Array(len * 3);
		let posmap = new map_t<i32, i32>();

		let pi = 0;
		for (let i = 0; i < len; ++i) {
			let found = false;
			for (let j = 0; j < pi; ++j) {
				if (posa2[j * 3    ] == posa[i * 4    ] &&
					posa2[j * 3 + 1] == posa[i * 4 + 1] &&
					posa2[j * 3 + 2] == posa[i * 4 + 2]) {
					posmap.set(i, j);
					found = true;
					break;
				}
			}
			if (!found) {
				posmap.set(i, pi);
				posa2[pi * 3    ] = posa[i * 4    ];
				posa2[pi * 3 + 1] = posa[i * 4 + 1];
				posa2[pi * 3 + 2] = posa[i * 4 + 2];
				pi++;
			}
		}

		export_obj_write_string(o, "o " + p.base.name + "\n");
		for (let i = 0; i < pi; ++i) {
			export_obj_write_string(o, "v ");
			let vx = posa2[i * 3] * sc + "";
			export_obj_write_string(o, vx.substr(0, vx.indexOf(".") + 7));
			export_obj_write_string(o, " ");
			let vy = posa2[i * 3 + 2] * sc + "";
			export_obj_write_string(o, vy.substr(0, vy.indexOf(".") + 7));
			export_obj_write_string(o, " ");
			let vz = -posa2[i * 3 + 1] * sc + "";
			export_obj_write_string(o, vz.substr(0, vz.indexOf(".") + 7));
			export_obj_write_string(o, "\n");
		}

		// let inda = mesh.index_arrays[0].values;
		for (let i = 0; i < math_floor(inda.length / 3); ++i) {
			let pi1 = posmap.get(inda[i * 3    ]) + 1 + poff;
			let pi2 = posmap.get(inda[i * 3 + 1]) + 1 + poff;
			let pi3 = posmap.get(inda[i * 3 + 2]) + 1 + poff;
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

	if (!path.endsWith(".obj")) path += ".obj";

	let b = Uint8Array.from(o).buffer;
	krom_file_save_bytes(path, b, b.byteLength);
}
