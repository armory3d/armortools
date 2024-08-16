
let plugin = plugin_create();
let h1 = zui_handle_create();

plugin_notify_on_ui(plugin, function() {
	if (zui_panel(h1, "Converter")) {
		zui_row([1 / 2, 1 / 2]);
		if (zui_button(".arm to .json")) {
			ui_files_show("arm", false, true, function(path) {
				let b = data_get_blob(path);
				let parsed = armpack_decode(b);
				let out = string_to_buffer(JSON.stringify(parsed, null, "	"));
				krom_file_save_bytes(path.substr(0, path.length - 3) + "json", out);
			});
		}
		if (zui_button(".json to .arm")) {
			ui_files_show("json", false, true, function(path) {
				let b = data_get_blob(path);
				let parsed = JSON.parse(buffer_to_string(b));
				let out = armpack_encode(parsed);
				krom_file_save_bytes(path.substr(0, path.length - 4) + "arm", out);
			});
		}
	}
});

function buffer_to_string(b) {
	let str = "";
	let u8a = new Uint8Array(b);
	for (let i = 0; i < u8a.length; ++i) {
		str += String.fromCharCode(u8a[i]);
	}
	return str;
}

function string_to_buffer(str) {
	let u8a = new Uint8Array(str.length);
	for (let i = 0; i < str.length; ++i) {
		u8a[i] = str.charCodeAt(i);
	}
	return u8a.buffer;
}

let pos = 0;
let k_type = "";

function armpack_decode(b) {
	pos = 0;
	return read(new DataView(b));
}

function read_u8(v) {
	let i = v.getUint8(pos);
	pos++;
	return i;
}

function read_i16(v) {
	let i = v.getInt16(pos, true);
	pos += 2;
	return i;
}

function read_i32(v) {
	let i = v.getInt32(pos, true);
	pos += 4;
	return i;
}

function read_u32(v) {
	let i = v.getUint32(pos, true);
	pos += 4;
	return i;
}

function read_f32(v) {
	let f = v.getFloat32(pos, true);
	pos += 4;
	return f;
}

function read_string(v, len) {
	let s = "";
	for (let i = 0; i < len; ++i) {
		s += String.fromCharCode(read_u8(v));
	}
	return s;
}

function read(v) {
	let b = read_u8(v);
	switch (b) {
		case 0xc0: return null;
		case 0xc2: return false;
		case 0xc3: return true;
		case 0xca: { k_type = "__f32"; return read_f32(v); }
		case 0xd2: return read_i32(v);
		case 0xdb: return read_string(v, read_u32(v));
		case 0xdd: return read_array(v, read_i32(v));
		case 0xdf: return read_map(v, read_i32(v));
	}
	return 0;
}

function read_array(v, length) {
	let b = read_u8(v);

	if (b == 0xca) { // Typed f32
		let a = new Array(length);
		for (let x = 0; x < length; ++x) a[x] = read_f32(v);
		k_type = "__f32";
		return a;
	}
	else if (b == 0xd2) { // Typed i32
		let a = new Array(length);
		for (let x = 0; x < length; ++x) a[x] = read_i32(v);
		k_type = "__i32";
		return a;
	}
	else if (b == 0xd1) { // Typed i16
		let a = new Array(length);
		for (let x = 0; x < length; ++x) a[x] = read_i16(v);
		k_type = "__i16";
		return a;
	}
	else if (b == 0xc4) { // Typed u8
		let a = new Array(length);
		for (let x = 0; x < length; ++x) a[x] = read_u8(v);
		k_type = "__u8";
		return a;
	}
	else { // Dynamic type-value
		pos--;
		let a = new Array(length);
		for (let x = 0; x < length; ++x) a[x] = read(v);
		return a;
	}
}

function read_map(v, length) {
	let out = {};
	for (let n = 0; n < length; ++n) {
		let k = read(v);
		k_type = "";
		let val = read(v);
		k += k_type;
		k_type = "";
		out[k] = val;
	}
	return out;
}

function armpack_encode(d) {
	pos = 0;
	write_dummy(d);
	console.log(pos);
	let b = new ArrayBuffer(pos);
	let v = new DataView(b);
	pos = 0;
	write(v, d);
	console.log(b.byteLength);
	return b;
}

function write_u8(v, i) {
	v.setUint8(pos, i);
	pos += 1;
}

function write_i16(v, i) {
	v.setInt16(pos, i, true);
	pos += 2;
}

function write_i32(v, i) {
	v.setInt32(pos, i, true);
	pos += 4;
}

function write_f32(v, f) {
	v.setFloat32(pos, f, true);
	pos += 4;
}

function write_string(v, str) {
	for (let i = 0; i < str.length; ++i) {
		write_u8(v, str.charCodeAt(i));
	}
}

function write(v, d) {
	if (d == null) {
		write_u8(v, 0xc0);
	}
	else if (typeof d == "boolean") {
		write_u8(v, d ? 0xc3 : 0xc2);
	}
	else if (typeof d == "number") {
		if (Number.isInteger(d) && k_type != "__f32") {
			write_u8(v, 0xd2);
			write_i32(v, d);
		}
		else {
			write_u8(v, 0xca);
			write_f32(v, d);
		}
	}
	else if (typeof d == "string") {
		write_u8(v, 0xdb);
		write_i32(v, d.length);
		write_string(v, d);
	}
	else if (Array.isArray(d)) {
		write_u8(v, 0xdd);
		write_i32(v, d.length);
		if (k_type == "__u8") {
			write_u8(v, 0xc4);
			for (let i = 0; i < d.length; ++i) {
				write_u8(v, d[i]);
			}
		}
		else if (k_type == "__i16") {
			write_u8(v, 0xd1);
			for (let i = 0; i < d.length; ++i) {
				write_i16(v, d[i]);
			}
		}
		else if (k_type == "__f32") {
			write_u8(v, 0xca);
			for (let i = 0; i < d.length; ++i) {
				write_f32(v, d[i]);
			}
		}
		else if (k_type == "__i32") {
			write_u8(v, 0xd2);
			for (let i = 0; i < d.length; ++i) {
				write_i32(v, d[i]);
			}
		}
		else {
			for (let i = 0; i < d.length; ++i) {
				write(v, d[i]);
			}
		}
	}
	else {
		write_object(v, d);
	}
}

function write_object(v, d) {
	let f = Object.keys(d);
	write_u8(v, 0xdf);
	write_i32(v, f.length);
	for (let i = 0; i < f.length; ++i) {
		let k = f[i];

		k_type = "";
		if (k.endsWith("__f32")) {
			k_type = "__f32";
		}
		else if (k.endsWith("__i32")) {
			k_type = "__i32";
		}
		else if (k.endsWith("__i16")) {
			k_type = "__i16";
		}
		else if (k.endsWith("__u8")) {
			k_type = "__u8";
		}

		write_u8(v, 0xdb);
		write_i32(v, k.length - k_type.length);

		write_string(v, k.substring(0, k.length - k_type.length));
		write(v, d[k]);
		k_type = "";
	}
}

function write_dummy(d) {
	if (d == null) {
		pos += 1;
	}
	else if (typeof d == "boolean") {
		pos += 1;
	}
	else if (typeof d == "number") {
		pos += 1;
		pos += 4;
	}
	else if (typeof d == "string") {
		pos += 1;
		pos += 4;
		pos += d.length;
	}
	else if (Array.isArray(d)) {
		pos += 1;
		pos += 4;
		if (k_type == "__u8") {
			pos += 1;
			for (let i = 0; i < d.length; ++i) {
				pos += 1;
			}
		}
		else if (k_type == "__i16") {
			pos += 1;
			for (let i = 0; i < d.length; ++i) {
				pos += 2;
			}
		}
		else if (k_type == "__f32") {
			pos += 1;
			for (let i = 0; i < d.length; ++i) {
				pos += 4;
			}
		}
		else if (k_type == "__i32") {
			pos += 1;
			for (let i = 0; i < d.length; ++i) {
				pos += 4;
			}
		}
		else {
			for (let i = 0; i < d.length; ++i) {
				write_dummy(d[i]);
			}
		}
	}
	else {
		write_object_dummy(d);
	}
}

function write_object_dummy(d) {
	let f = Object.keys(d);
	pos += 1;
	pos += 4;
	for (let i = 0; i < f.length; ++i) {
		let k = f[i];
		pos += 1;
		pos += 4;

		k_type = "";
		if (k.endsWith("__f32")) {
			k_type = "__f32";
		}
		else if (k.endsWith("__i32")) {
			k_type = "__i32";
		}
		else if (k.endsWith("__i16")) {
			k_type = "__i16";
		}
		else if (k.endsWith("__u8")) {
			k_type = "__u8";
		}

		pos += k.length - k_type.length;
		write_dummy(d[k]);
		k_type = "";
	}
}
