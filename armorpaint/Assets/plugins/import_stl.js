
let pos = 0;

let read_i16 = function(view) {
	let i = view.getInt16(pos, true);
	pos += 2;
	return i;
}

let read_i32 = function(view) {
	let i = view.getInt32(pos, true);
	pos += 4;
	return i;
}

let read_f32 = function(view) {
	let f = view.getFloat32(pos, true);
	pos += 4;
	return f;
}

let import_stl = function(path, done) {
	Data.getBlob(path, function(b) {
		let view = new DataView(b);
		pos = 0;
		// let header = input.read(80);
		pos += 80;
		// if (header.getString(0, 5) == "solid") {
			// return; // ascii not supported
		// }
		let faces = read_i32(view);
		let posTemp = new Float32Array(faces * 9);
		let norTemp = new Float32Array(faces * 3);
		for (let i = 0; i < faces; ++i) {
			let i3 = i * 3;
			norTemp[i3    ] = read_f32(view);
			norTemp[i3 + 1] = read_f32(view);
			norTemp[i3 + 2] = read_f32(view);
			let i9 = i * 9;
			posTemp[i9    ] = read_f32(view);
			posTemp[i9 + 1] = read_f32(view);
			posTemp[i9 + 2] = read_f32(view);
			posTemp[i9 + 3] = read_f32(view);
			posTemp[i9 + 4] = read_f32(view);
			posTemp[i9 + 5] = read_f32(view);
			posTemp[i9 + 6] = read_f32(view);
			posTemp[i9 + 7] = read_f32(view);
			posTemp[i9 + 8] = read_f32(view);
			read_i16(view); // attribute
		}

		let scalePos = 0.0;
		for (let i = 0; i < posTemp.length; ++i) {
			let f = Math.abs(posTemp[i]);
			if (scalePos < f) scalePos = f;
		}
		let inv = 32767 * (1 / scalePos);

		let verts = posTemp.length / 3;
		let posa = new Int16Array(verts * 4);
		let nora = new Int16Array(verts * 2);
		let inda = new Uint32Array(verts);
		for (let i = 0; i < verts; ++i) {
			posa[i * 4    ] =  posTemp[i * 3    ] * inv;
			posa[i * 4 + 1] = -posTemp[i * 3 + 2] * inv;
			posa[i * 4 + 2] =  posTemp[i * 3 + 1] * inv;
			nora[i * 2    ] =  norTemp[i    ] * 32767;
			nora[i * 2 + 1] = -norTemp[i + 2] * 32767;
			posa[i * 4 + 3] =  norTemp[i + 1] * 32767;
			inda[i] = i;
		}

		let name = path.split("\\").pop().split("/").pop().split(".").shift();
		done({
			name: name,
			posa: posa,
			nora: nora,
			inda: inda,
			scale_pos: scalePos,
			scale_tex: 1.0
		});

		Data.deleteBlob(path);
	});
}

let plugin = Plugin.create();
let formats = Path.meshFormats;
let importers = Path.meshImporters;
formats.push("stl");
importers.set("stl", import_stl);

plugin.delete = function() {
	formats.splice(formats.indexOf("stl"), 1);
	importers.delete("stl");
};
