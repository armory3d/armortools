
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
	let b = data_get_blob(path);
	let view = new DataView(b);
	pos = 0;
	// let header = input.read(80);
	pos += 80;
	// if (header.getString(0, 5) == "solid") {
		// return; // ascii not supported
	// }
	let faces = read_i32(view);
	let pos_temp = new Float32Array(faces * 9);
	let nor_temp = new Float32Array(faces * 3);
	for (let i = 0; i < faces; ++i) {
		let i3 = i * 3;
		nor_temp[i3    ] = read_f32(view);
		nor_temp[i3 + 1] = read_f32(view);
		nor_temp[i3 + 2] = read_f32(view);
		let i9 = i * 9;
		pos_temp[i9    ] = read_f32(view);
		pos_temp[i9 + 1] = read_f32(view);
		pos_temp[i9 + 2] = read_f32(view);
		pos_temp[i9 + 3] = read_f32(view);
		pos_temp[i9 + 4] = read_f32(view);
		pos_temp[i9 + 5] = read_f32(view);
		pos_temp[i9 + 6] = read_f32(view);
		pos_temp[i9 + 7] = read_f32(view);
		pos_temp[i9 + 8] = read_f32(view);
		read_i16(view); // attribute
	}

	let scale_pos = 0.0;
	for (let i = 0; i < pos_temp.length; ++i) {
		let f = Math.abs(pos_temp[i]);
		if (scale_pos < f) {
			scale_pos = f;
		}
	}
	let inv = 32767 * (1 / scale_pos);

	let verts = pos_temp.length / 3;
	let posa = new Int16Array(verts * 4);
	let nora = new Int16Array(verts * 2);
	let inda = new Uint32Array(verts);
	for (let i = 0; i < verts; ++i) {
		posa[i * 4    ] =  pos_temp[i * 3    ] * inv;
		posa[i * 4 + 1] = -pos_temp[i * 3 + 2] * inv;
		posa[i * 4 + 2] =  pos_temp[i * 3 + 1] * inv;
		nora[i * 2    ] =  nor_temp[i    ] * 32767;
		nora[i * 2 + 1] = -nor_temp[i + 2] * 32767;
		posa[i * 4 + 3] =  nor_temp[i + 1] * 32767;
		inda[i] = i;
	}

	data_delete_blob(path);
	let name = path.split("\\").pop().split("/").pop().split(".").shift();
	return {
		name: name,
		posa: posa,
		nora: nora,
		inda: inda,
		scale_pos: scale_pos,
		scale_tex: 1.0
	};
}

let plugin = plugin_create();
let formats = path_mesh_formats;
let importers = path_mesh_importers;
formats.push("stl");
importers.set("stl", import_stl);

plugin.delete = function() {
	formats.splice(formats.indexOf("stl"), 1);
	importers.delete("stl");
};
