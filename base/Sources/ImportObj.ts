
class ImportObj {

	static run = (path: string, replaceExisting = true) => {
		let i = Context.raw.splitBy;
		let isUdim = i == SplitType.SplitUdim;
		let splitCode =
			(i == SplitType.SplitObject || isUdim) ? "o".charCodeAt(0) :
			 i == SplitType.SplitGroup 			   ? "g".charCodeAt(0) :
			 				 			   			 "u".charCodeAt(0); // usemtl

		data_get_blob(path, (b: ArrayBuffer) => {

			if (isUdim) {
				let part = krom_io_obj_parse(b, splitCode, 0, isUdim);
				let name = part.name;
				for (let i = 0; i < part.udims.length; ++i) {
					if (part.udims[i].length == 0) continue;
					let u = i % part.udims_u;
					let v = Math.floor(i / part.udims_u);
					part.name = name + "." + (1000 + v * 10 + u + 1);
					part.inda = part.udims[i];
					i == 0 ? (replaceExisting ? ImportMesh.makeMesh(part, path) : ImportMesh.addMesh(part)) : ImportMesh.addMesh(part);
				}
			}
			else {
				let parts: any[] = [];
				let part = krom_io_obj_parse(b, splitCode, 0, false);
				parts.push(part);
				while (part.has_next) {
					part = krom_io_obj_parse(b, splitCode, part.pos, false);
					// This part does not contain faces (may contain lines only)
					if (part.inda.length == 0) {
						continue;
					}
					parts.push(part);
				}
				if (Context.raw.splitBy == SplitType.SplitMaterial) {
					let posa0;
					let posa1;
					let nora0;
					let nora1;
					let texa0;
					let texa1;
					let inda0;
					let inda1;
					// Merge to single object per material
					for (let i = 0; i < parts.length; ++i) {
						let j = i + 1;
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
								let voff = Math.floor(posa0.length / 4);
								// Repack merged positions
								let posa32 = new Float32Array(Math.floor(posa0.length / 4) * 3 + Math.floor(posa1.length / 4) * 3);
								for (let k = 0; k < Math.floor(posa0.length / 4); ++k) {
									posa32[k * 3    ] = posa0[k * 4    ] / 32767 * parts[i].scalePos;
									posa32[k * 3 + 1] = posa0[k * 4 + 1] / 32767 * parts[i].scalePos;
									posa32[k * 3 + 2] = posa0[k * 4 + 2] / 32767 * parts[i].scalePos;
								}
								for (let k = 0; k < Math.floor(posa1.length / 4); ++k) {
									posa32[voff * 3 + k * 3    ] = posa1[k * 4    ] / 32767 * parts[j].scalePos;
									posa32[voff * 3 + k * 3 + 1] = posa1[k * 4 + 1] / 32767 * parts[j].scalePos;
									posa32[voff * 3 + k * 3 + 2] = posa1[k * 4 + 2] / 32767 * parts[j].scalePos;
								}
								let scalePos = 0.0;
								for (let k = 0; k < posa32.length; ++k) {
									let f = Math.abs(posa32[k]);
									if (scalePos < f) scalePos = f;
								}
								let inv = 32767 * (1 / scalePos);
								let posa = new Int16Array(posa0.length + posa1.length);
								for (let k = 0; k < Math.floor(posa.length / 4); ++k) {
									posa[k * 4    ] = Math.floor(posa32[k * 3    ] * inv);
									posa[k * 4 + 1] = Math.floor(posa32[k * 3 + 1] * inv);
									posa[k * 4 + 2] = Math.floor(posa32[k * 3 + 2] * inv);
								}
								for (let k = 0; k < Math.floor(posa0.length / 4); ++k) posa[k * 4 + 3] = posa0[k * 4 + 3];
								for (let k = 0; k < Math.floor(posa1.length / 4); ++k) posa[posa0.length + k * 4 + 3] = posa1[k * 4 + 3];
								// Merge normals and uvs
								let nora = new Int16Array(nora0.length + nora1.length);
								let texa = (texa0 != null && texa1 != null) ? new Int16Array(texa0.length + texa1.length) : null;
								let inda = new Uint32Array(inda0.length + inda1.length);
								nora.set(nora0);
								nora.set(nora1, nora0.length);
								if (texa != null) {
									texa.set(texa0);
									texa.set(texa1, texa0.length);
								}
								inda.set(inda0);
								for (let k = 0; k < inda1.length; ++k) inda[k + inda0.length] = inda1[k] + voff;
								parts[i].posa = posa;
								parts[i].nora = nora;
								parts[i].texa = texa;
								parts[i].inda = inda;
								parts[i].scalePos = scalePos;
								parts.splice(j, 1);
							}
							else j++;
						}
					}
				}
				replaceExisting ? ImportMesh.makeMesh(parts[0], path) : ImportMesh.addMesh(parts[0]);
				for (let i = 1; i < parts.length; ++i) {
					ImportMesh.addMesh(parts[i]);
				}
			}
			data_delete_blob(path);
		});
	}
}
