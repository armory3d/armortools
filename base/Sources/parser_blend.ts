// .blend file parser
// Reference:
// https://github.com/fschutt/mystery-of-the-blend-backup
// https://web.archive.org/web/20170630054951/http://www.atmind.nl/blender/mystery_ot_blend.html
// Usage:
// let bl: BlendRaw = parser_blend_init(blob: DataView);
// krom_log(parser_blend_dir(bl, "Scene"));
// let scenes: any = parser_blend_get(bl, "Scene");
// krom_log(get(get(scenes[0], "id"), "name"));

class BlendRaw {
	pos: i32;
	view: DataView;

	// Header
	version: string;
	pointer_size: i32;
	little_endian: bool;
	// Data
	blocks: Block[] = [];
	dna: Dna = null;
	map: map_t<any, Block> = map_create(); // Map blocks by memory address
}

function parser_blend_init(buffer: ArrayBuffer): BlendRaw {
	let raw: BlendRaw = new BlendRaw();
	raw.view = new DataView(buffer);
	raw.pos = 0;
	if (parser_blend_read_chars(raw, 7) != "BLENDER") {
		raw.view = new DataView(krom_inflate(buffer, false));
		raw.pos = 0;
		if (parser_blend_read_chars(raw, 7) != "BLENDER") return null;
	}
	parser_blend_parse(raw);
	return raw;
}

function parser_blend_dir(raw: BlendRaw, type: string): string[] {
	// Return structure fields
	let type_index: i32 = parser_blend_get_type_index(raw.dna, type);
	if (type_index == -1) return null;
	let ds: DnaStruct = parser_blend_get_struct(raw.dna, type_index);
	let fields: string[] = [];
	for (let i: i32 = 0; i < ds.field_names.length; ++i) {
		let name_index: i32 = ds.field_names[i];
		let type_index: i32 = ds.field_types[i];
		fields.push(raw.dna.types[type_index] + " " + raw.dna.names[name_index]);
	}
	return fields;
}

function parser_blend_get(raw: BlendRaw, type: string): BlHandleRaw[] {
	if (raw.dna == null) return null;
	// Return all structures of type
	let type_index: i32 = parser_blend_get_type_index(raw.dna, type);
	if (type_index == -1) return null;
	let ds: DnaStruct = parser_blend_get_struct(raw.dna, type_index);
	let handles: BlHandleRaw[] = [];
	for (let b of raw.blocks) {
		if (raw.dna.structs[b.sdna_index].type == type_index) {
			let h: BlHandleRaw = new BlHandleRaw();
			handles.push(h);
			h.block = b;
			h.ds = ds;
		}
	}
	return handles;
}

function parser_blend_get_struct(dna: Dna, typeIndex: i32): DnaStruct {
	for (let ds of dna.structs) if (ds.type == typeIndex) return ds;
	return null;
}

function parser_blend_get_type_index(dna: Dna, type: string): i32 {
	for (let i: i32 = 0; i < dna.types.length; ++i) if (type == dna.types[i]) return i;
	return -1;
}

function parser_blend_parse(raw: BlendRaw) {
	// Pointer size: _ 32bit, - 64bit
	raw.pointer_size = parser_blend_read_char(raw) == "_" ? 4 : 8;

	// v - little endian, V - big endian
	raw.little_endian = parser_blend_read_char(raw) == "v";

	raw.version = parser_blend_read_chars(raw, 3);

	// Reading file blocks
	// Header - data
	while (raw.pos < raw.view.byteLength) {
		parser_blend_align(raw);
		let b: Block = new Block();

		// Block type
		b.code = parser_blend_read_chars(raw, 4);
		if (b.code == "ENDB") break;

		raw.blocks.push(b);
		b.blend = raw;

		// Total block length
		b.size = parser_blend_read_i32(raw);

		// Memory address
		let addr: any = parser_blend_read_pointer(raw);
		if (!raw.map.has(addr)) raw.map.set(addr, b);

		// Index of dna struct contained in this block
		b.sdna_index = parser_blend_read_i32(raw);

		// Number of dna structs in this block
		b.count = parser_blend_read_i32(raw);

		b.pos = raw.pos;

		// This block stores dna structures
		if (b.code == "DNA1") {
			raw.dna = new Dna();

			parser_blend_read_chars(raw, 4); // SDNA
			parser_blend_read_chars(raw, 4); // NAME
			let names_count: i32 = parser_blend_read_i32(raw);
			for (let i: i32 = 0; i < names_count; ++i) {
				raw.dna.names.push(parser_blend_read_string(raw));
			}
			parser_blend_align(raw);

			parser_blend_read_chars(raw, 4); // TYPE
			let types_count: i32 = parser_blend_read_i32(raw);
			for (let i: i32 = 0; i < types_count; ++i) {
				raw.dna.types.push(parser_blend_read_string(raw));
			}
			parser_blend_align(raw);

			parser_blend_read_chars(raw, 4); // TLEN
			for (let i: i32 = 0; i < types_count; ++i) {
				raw.dna.types_length.push(parser_blend_read_i16(raw));
			}
			parser_blend_align(raw);

			parser_blend_read_chars(raw, 4); // STRC
			let struct_count: i32 = parser_blend_read_i32(raw);
			for (let i: i32 = 0; i < struct_count; ++i) {
				let ds: DnaStruct = new DnaStruct();
				raw.dna.structs.push(ds);
				ds.dna = raw.dna;
				ds.type = parser_blend_read_i16(raw);
				let field_count: i32 = parser_blend_read_i16(raw);
				if (field_count > 0) {
					ds.field_types = [];
					ds.field_names = [];
					for (let j: i32 = 0; j < field_count; ++j) {
						ds.field_types.push(parser_blend_read_i16(raw));
						ds.field_names.push(parser_blend_read_i16(raw));
					}
				}
			}
		}
		else {
			raw.pos += b.size;
		}
	}
}

function parser_blend_align(raw: BlendRaw) {
	// 4 bytes aligned
	let mod: i32 = raw.pos % 4;
	if (mod > 0) raw.pos += 4 - mod;
}

function parser_blend_read_i8(raw: BlendRaw): i32 {
	let i: i32 = raw.view.getUint8(raw.pos);
	raw.pos += 1;
	return i;
}

function parser_blend_read_i16(raw: BlendRaw): i32 {
	let i: i32 = raw.view.getInt16(raw.pos, raw.little_endian);
	raw.pos += 2;
	return i;
}

function parser_blend_read_i32(raw: BlendRaw): i32 {
	let i: i32 = raw.view.getInt32(raw.pos, raw.little_endian);
	raw.pos += 4;
	return i;
}

function parser_blend_read_i64(raw: BlendRaw): any {
	let aview: any = raw.view;
	let i: i32 = aview.getBigInt64(raw.pos, raw.little_endian);
	raw.pos += 8;
	return i;
}

function parser_blend_read_f32(raw: BlendRaw): f32 {
	let f: f32 = raw.view.getFloat32(raw.pos, raw.little_endian);
	raw.pos += 4;
	return f;
}

function parser_blend_read_i8array(raw: BlendRaw, len: i32): Int32Array {
	let ar: Int32Array = new Int32Array(len);
	for (let i: i32 = 0; i < len; ++i) ar[i] = parser_blend_read_i8(raw);
	return ar;
}

function parser_blend_read_i16array(raw: BlendRaw, len: i32): Int32Array {
	let ar: Int32Array = new Int32Array(len);
	for (let i: i32 = 0; i < len; ++i) ar[i] = parser_blend_read_i16(raw);
	return ar;
}

function parser_blend_read_i32array(raw: BlendRaw, len: i32): Int32Array {
	let ar: Int32Array = new Int32Array(len);
	for (let i: i32 = 0; i < len; ++i) ar[i] = parser_blend_read_i32(raw);
	return ar;
}

function parser_blend_read_f32array(raw: BlendRaw, len: i32): Float32Array {
	let ar: Float32Array = new Float32Array(len);
	for (let i: i32 = 0; i < len; ++i) ar[i] = parser_blend_read_f32(raw);
	return ar;
}

function parser_blend_read_string(raw: BlendRaw): string {
	let s: string = "";
	while (true) {
		let ch: i32 = parser_blend_read_i8(raw);
		if (ch == 0) break;
		s += String.fromCharCode(ch);
	}
	return s;
}

function parser_blend_read_chars(raw: BlendRaw, len: i32): string {
	let s: string = "";
	for (let i: i32 = 0; i < len; ++i) s += parser_blend_read_char(raw);
	return s;
}

function parser_blend_read_char(raw: BlendRaw): string {
	return String.fromCharCode(parser_blend_read_i8(raw));
}

function parser_blend_read_pointer(raw: BlendRaw): any {
	return raw.pointer_size == 4 ? parser_blend_read_i32(raw) : parser_blend_read_i64(raw);
}

class Block {
	blend: BlendRaw;
	code: string;
	size: i32;
	sdna_index: i32;
	count: i32;
	pos: i32; // Byte pos of data start in blob
}

class Dna {
	names: string[] = [];
	types: string[] = [];
	types_length: i32[] = [];
	structs: DnaStruct[] = [];
}

class DnaStruct {
	dna: Dna;
	type: i32; // Index in dna.types
	field_types: i32[]; // Index in dna.types
	field_names: i32[]; // Index in dna.names
}

class BlHandleRaw {
	block: Block;
	offset: i32 = 0; // Block data bytes offset
	ds: DnaStruct;
}

function bl_handle_get_size(raw: BlHandleRaw, index: i32): i32 {
	let name_index: i32 = raw.ds.field_names[index];
	let type_index: i32 = raw.ds.field_types[index];
	let dna: Dna = raw.ds.dna;
	let n: string = dna.names[name_index];
	let size: i32 = 0;
	if (n.indexOf("*") >= 0) size = raw.block.blend.pointer_size;
	else size = dna.types_length[type_index];
	if (n.indexOf("[") > 0) size *= bl_handle_get_array_len(n);
	return size;
}

function bl_handle_base_name(s: string): string {
	while (s.charAt(0) == "*") s = s.substring(1, s.length);
	if (s.charAt(s.length - 1) == "]") s = s.substring(0, s.indexOf("["));
	return s;
}

function bl_handle_get_array_len(s: string): i32 {
	return parseInt(s.substring(s.indexOf("[") + 1, s.indexOf("]")));
}

function bl_handle_get(raw: BlHandleRaw, name: string, index: i32 = 0, as_type: string = null, array_len: i32 = 0): any {
	// Return raw type or structure
	let dna: Dna = raw.ds.dna;
	for (let i: i32 = 0; i < raw.ds.field_names.length; ++i) {
		let name_index: i32 = raw.ds.field_names[i];
		let dna_name: string = dna.names[name_index];
		if (name == bl_handle_base_name(dna_name)) {
			let type_index: i32 = raw.ds.field_types[i];
			let type: string = dna.types[type_index];
			let new_offset: i32 = raw.offset;
			for (let j: i32 = 0; j < i; ++j) new_offset += bl_handle_get_size(raw, j);
			// Cast void * to type
			if (as_type != null) {
				for (let i: i32 = 0; i < dna.types.length; ++i) {
					if (dna.types[i] == as_type) {
						type_index = i;
						break;
					}
				}
			}
			// Raw type
			if (type_index < 12) {
				let blend: BlendRaw = raw.block.blend;
				blend.pos = raw.block.pos + new_offset;
				let is_array: bool = dna_name.charAt(dna_name.length - 1) == "]";
				let len: i32 = is_array ? (array_len > 0 ? array_len : this.bl_handle_get_array_len(dna_name)) : 1;
				switch (type) {
					case "int": return is_array ? parser_blend_read_i32array(blend, len) : parser_blend_read_i32(blend);
					case "char": return is_array ? parser_blend_read_string(blend) : parser_blend_read_i8(blend);
					case "uchar": return is_array ? parser_blend_read_i8array(blend, len) : parser_blend_read_i8(blend);
					case "short": return is_array ? parser_blend_read_i16array(blend, len) : parser_blend_read_i16(blend);
					case "ushort": return is_array ? parser_blend_read_i16array(blend, len) : parser_blend_read_i16(blend);
					case "float": return is_array ? parser_blend_read_f32array(blend, len) : parser_blend_read_f32(blend);
					case "double": return 0; //readf64(blend);
					case "long": return is_array ? parser_blend_read_i32array(blend, len) : parser_blend_read_i32(blend);
					case "ulong": return is_array ? parser_blend_read_i32array(blend, len) : parser_blend_read_i32(blend);
					case "int64_t": return parser_blend_read_i64(blend);
					case "uint64_t": return parser_blend_read_i64(blend);
					case "void": if (dna_name.charAt(0) == "*") { return parser_blend_read_i64(blend); };
				}
			}
			// Structure
			let h: BlHandleRaw = new BlHandleRaw();
			h.ds = parser_blend_get_struct(dna, type_index);
			let is_pointer: bool = dna_name.charAt(0) == "*";
			if (is_pointer) {
				raw.block.blend.pos = raw.block.pos + new_offset;
				let addr: any = parser_blend_read_pointer(raw.block.blend);
				if (raw.block.blend.map.has(addr)) {
					h.block = raw.block.blend.map.get(addr);
				}
				else h.block = raw.block;
				h.offset = 0;
			}
			else {
				h.block = raw.block;
				h.offset = new_offset;
			}
			h.offset += dna.types_length[type_index] * index;
			return h;
		}
	}
	return null;
}
