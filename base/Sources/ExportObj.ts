
class ExportObj {

	static writeString = (out: i32[], str: string) => {
		for (let i = 0; i < str.length; ++i) {
			out.push(str.charCodeAt(i));
		}
	}

	static run = (path: string, paintObjects: mesh_object_t[], applyDisplacement = false) => {
		let o: i32[] = [];
		ExportObj.writeString(o, "# armorpaint.org\n");

		let poff = 0;
		let noff = 0;
		let toff = 0;
		for (let p of paintObjects) {
			let mesh = p.data;
			let inv = 1 / 32767;
			let sc = p.data.scale_pos * inv;
			let posa = mesh.vertex_arrays[0].values;
			let nora = mesh.vertex_arrays[1].values;
			let texa = mesh.vertex_arrays[2].values;
			let len = Math.floor(posa.length / 4);

			// Merge shared vertices and remap indices
			let posa2 = new Int16Array(len * 3);
			let nora2 = new Int16Array(len * 3);
			let texa2 = new Int16Array(len * 2);
			let posmap = new Map<i32, i32>();
			let normap = new Map<i32, i32>();
			let texmap = new Map<i32, i32>();

			let pi = 0;
			let ni = 0;
			let ti = 0;
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

				found = false;
				for (let j = 0; j < ni; ++j) {
					if (nora2[j * 3    ] == nora[i * 2    ] &&
						nora2[j * 3 + 1] == nora[i * 2 + 1] &&
						nora2[j * 3 + 2] == posa[i * 4 + 3]) {
						normap.set(i, j);
						found = true;
						break;
					}
				}
				if (!found) {
					normap.set(i, ni);
					nora2[ni * 3    ] = nora[i * 2    ];
					nora2[ni * 3 + 1] = nora[i * 2 + 1];
					nora2[ni * 3 + 2] = posa[i * 4 + 3];
					ni++;
				}

				found = false;
				for (let j = 0; j < ti; ++j) {
					if (texa2[j * 2    ] == texa[i * 2    ] &&
						texa2[j * 2 + 1] == texa[i * 2 + 1]) {
						texmap.set(i, j);
						found = true;
						break;
					}
				}
				if (!found) {
					texmap.set(i, ti);
					texa2[ti * 2    ] = texa[i * 2    ];
					texa2[ti * 2 + 1] = texa[i * 2 + 1];
					ti++;
				}
			}

			if (applyDisplacement) {
				// let height = Project.layers[0].texpaint_pack.getPixels();
				// let res = Project.layers[0].texpaint_pack.width;
				// let strength = 0.1;
				// for (let i = 0; i < len; ++i) {
				// 	let x = Math.floor(texa2[i * 2    ] / 32767 * res);
				// 	let y = Math.floor((1.0 - texa2[i * 2 + 1] / 32767) * res);
				// 	let h = (1.0 - height.get((y * res + x) * 4 + 3) / 255) * strength;
				// 	posa2[i * 3    ] -= Math.floor(nora2[i * 3    ] * inv * h / sc);
				// 	posa2[i * 3 + 1] -= Math.floor(nora2[i * 3 + 1] * inv * h / sc);
				// 	posa2[i * 3 + 2] -= Math.floor(nora2[i * 3 + 2] * inv * h / sc);
				// }
			}

			ExportObj.writeString(o, "o " + p.base.name + "\n");
			for (let i = 0; i < pi; ++i) {
				ExportObj.writeString(o, "v ");
				ExportObj.writeString(o, posa2[i * 3] * sc + "");
				ExportObj.writeString(o, " ");
				ExportObj.writeString(o, posa2[i * 3 + 2] * sc + "");
				ExportObj.writeString(o, " ");
				ExportObj.writeString(o, -posa2[i * 3 + 1] * sc + "");
				ExportObj.writeString(o, "\n");
			}
			for (let i = 0; i < ni; ++i) {
				ExportObj.writeString(o, "vn ");
				ExportObj.writeString(o, nora2[i * 3] * inv + "");
				ExportObj.writeString(o, " ");
				ExportObj.writeString(o, nora2[i * 3 + 2] * inv + "");
				ExportObj.writeString(o, " ");
				ExportObj.writeString(o, -nora2[i * 3 + 1] * inv + "");
				ExportObj.writeString(o, "\n");
			}
			for (let i = 0; i < ti; ++i) {
				ExportObj.writeString(o, "vt ");
				ExportObj.writeString(o, texa2[i * 2] * inv + "");
				ExportObj.writeString(o, " ");
				ExportObj.writeString(o, 1.0 - texa2[i * 2 + 1] * inv + "");
				ExportObj.writeString(o, "\n");
			}

			let inda = mesh.index_arrays[0].values;
			for (let i = 0; i < Math.floor(inda.length / 3); ++i) {
				let pi1 = posmap.get(inda[i * 3    ]) + 1 + poff;
				let pi2 = posmap.get(inda[i * 3 + 1]) + 1 + poff;
				let pi3 = posmap.get(inda[i * 3 + 2]) + 1 + poff;
				let ni1 = normap.get(inda[i * 3    ]) + 1 + noff;
				let ni2 = normap.get(inda[i * 3 + 1]) + 1 + noff;
				let ni3 = normap.get(inda[i * 3 + 2]) + 1 + noff;
				let ti1 = texmap.get(inda[i * 3    ]) + 1 + toff;
				let ti2 = texmap.get(inda[i * 3 + 1]) + 1 + toff;
				let ti3 = texmap.get(inda[i * 3 + 2]) + 1 + toff;
				ExportObj.writeString(o, "f ");
				ExportObj.writeString(o, pi1 + "");
				ExportObj.writeString(o, "/");
				ExportObj.writeString(o, ti1 + "");
				ExportObj.writeString(o, "/");
				ExportObj.writeString(o, ni1 + "");
				ExportObj.writeString(o, " ");
				ExportObj.writeString(o, pi2 + "");
				ExportObj.writeString(o, "/");
				ExportObj.writeString(o, ti2 + "");
				ExportObj.writeString(o, "/");
				ExportObj.writeString(o, ni2 + "");
				ExportObj.writeString(o, " ");
				ExportObj.writeString(o, pi3 + "");
				ExportObj.writeString(o, "/");
				ExportObj.writeString(o, ti3 + "");
				ExportObj.writeString(o, "/");
				ExportObj.writeString(o, ni3 + "");
				ExportObj.writeString(o, "\n");
			}
			poff += pi;
			noff += ni;
			toff += ti;
		}

		if (!path.endsWith(".obj")) path += ".obj";

		let b = Uint8Array.from(o).buffer;
		krom_file_save_bytes(path, b, b.byteLength);
	}
}
