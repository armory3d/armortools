
class ExportObj {

	static writeString = (out: i32[], str: string) => {
		for (let i = 0; i < str.length; ++i) {
			out.push(str.charCodeAt(i));
		}
	}

	static run = (path: string, paintObjects: MeshObject[], applyDisplacement = false) => {
		let o: i32[] = [];
		writeString(o, "# armorsculpt.org\n");

		let texpaint = Project.layers[0].texpaint;
		let pixels = texpaint.getPixels();
		let pixelsView = new DataView(pixels);
		let mesh = paintObjects[0].data.raw;
		let inda = mesh.index_arrays[0].values;

		let posa = new Int16Array(inda.length * 4);
		for (let i = 0; i < inda.length; ++i) {
			let index = inda[i];
			posa[index * 4    ] = Math.floor(pixelsView.getFloat32(i * 16    , true) * 32767);
			posa[index * 4 + 1] = Math.floor(pixelsView.getFloat32(i * 16 + 4, true) * 32767);
			posa[index * 4 + 2] = Math.floor(pixelsView.getFloat32(i * 16 + 8, true) * 32767);
		}

		let poff = 0;
		// for (let p of paintObjects) {
			let p = paintObjects[0];
			// let mesh = p.data.raw;
			let inv = 1 / 32767;
			let sc = p.data.scalePos * inv;
			// let posa = mesh.vertex_arrays[0].values;
			let len = Math.floor(posa.length / 4);
			// let len = Math.floor(inda.length);

			// Merge shared vertices and remap indices
			let posa2 = new Int16Array(len * 3);
			let posmap = new Map<i32, i32>();

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

			writeString(o, "o " + p.name + "\n");
			for (let i = 0; i < pi; ++i) {
				writeString(o, "v ");
				let vx = posa2[i * 3] * sc + "";
				writeString(o, vx.substr(0, vx.indexOf(".") + 7));
				writeString(o, " ");
				let vy = posa2[i * 3 + 2] * sc + "";
				writeString(o, vy.substr(0, vy.indexOf(".") + 7));
				writeString(o, " ");
				let vz = -posa2[i * 3 + 1] * sc + "";
				writeString(o, vz.substr(0, vz.indexOf(".") + 7));
				writeString(o, "\n");
			}

			// let inda = mesh.index_arrays[0].values;
			for (let i = 0; i < Math.floor(inda.length / 3); ++i) {
				let pi1 = posmap.get(inda[i * 3    ]) + 1 + poff;
				let pi2 = posmap.get(inda[i * 3 + 1]) + 1 + poff;
				let pi3 = posmap.get(inda[i * 3 + 2]) + 1 + poff;
				writeString(o, "f ");
				writeString(o, pi1 + "");
				writeString(o, " ");
				writeString(o, pi2 + "");
				writeString(o, " ");
				writeString(o, pi3 + "");
				writeString(o, "\n");
			}
			poff += pi;
		// }

		if (!path.endsWith(".obj")) path += ".obj";

		let b = Uint8Array.from(o).buffer;
		Krom.fileSaveBytes(path, b, b.byteLength);
	}
}
