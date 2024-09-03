// .blend file parser
// Reference:
// https://github.com/fschutt/mystery-of-the-blend-backup
// https://web.archive.org/web/20170630054951/http://www.atmind.nl/blender/mystery_ot_blend.html

type blend_t = {
	pos?: i32;
	view?: u8_array_t;
	// Header
	version?: string;
	pointer_size?: i32;
	little_endian?: bool;
	// Data
	blocks?: block_t[];
	dna?: dna_t;
	map?: map_t<string, block_t>; // Map blocks by memory address
};

function parser_blend_init(buffer: buffer_t): blend_t {
	let raw: blend_t = {};
	raw.blocks = [];
	raw.map = map_create();
	raw.view = buffer;
	raw.pos = 0;
	if (parser_blend_read_chars(raw, 7) != "BLENDER") {
		raw.view = iron_inflate(buffer, false);
		raw.pos = 0;
		if (parser_blend_read_chars(raw, 7) != "BLENDER") {
			return null;
		}
	}
	parser_blend_parse(raw);
	return raw;
}

function parser_blend_dir(raw: blend_t, type: string): string[] {
	// Return structure fields
	let type_index: i32 = parser_blend_get_type_index(raw.dna, type);
	if (type_index == -1) {
		return null;
	}
	let ds: dna_struct_t = parser_blend_get_struct(raw.dna, type_index);
	let fields: string[] = [];
	for (let i: i32 = 0; i < ds.field_names.length; ++i) {
		let name_index: i32 = ds.field_names[i];
		let type_index: i32 = ds.field_types[i];
		array_push(fields, raw.dna.types[type_index] + " " + raw.dna.names[name_index]);
	}
	return fields;
}

function parser_blend_get(raw: blend_t, type: string): bl_handle_t[] {
	if (raw.dna == null) {
		return null;
	}
	// Return all structures of type
	let type_index: i32 = parser_blend_get_type_index(raw.dna, type);
	if (type_index == -1) {
		return null;
	}
	let ds: dna_struct_t = parser_blend_get_struct(raw.dna, type_index);
	let handles: bl_handle_t[] = [];
	for (let i: i32 = 0; i < raw.blocks.length; ++i) {
		let b: block_t = raw.blocks[i];
		let stype: i32 = raw.dna.structs[b.sdna_index].type;
		if (stype == type_index) {
			let h: bl_handle_t = {};
			h.offset = 0;
			array_push(handles, h);
			h.block = b;
			h.ds = ds;
		}
	}
	return handles;
}

function parser_blend_get_struct(dna: dna_t, type_index: i32): dna_struct_t {
	for (let i: i32 = 0; i < dna.structs.length; ++i) {
		let ds: dna_struct_t = dna.structs[i];
		if (ds.type == type_index) {
			return ds;
		}
	}
	return null;
}

function parser_blend_get_type_index(dna: dna_t, type: string): i32 {
	for (let i: i32 = 0; i < dna.types.length; ++i) {
		if (type == dna.types[i]) {
			return i;
		}
	}
	return -1;
}

function parser_blend_parse(raw: blend_t) {
	// Pointer size: _ 32bit, - 64bit
	raw.pointer_size = parser_blend_read_char(raw) == "_" ? 4 : 8;

	// v - little endian, V - big endian
	raw.little_endian = parser_blend_read_char(raw) == "v";

	raw.version = parser_blend_read_chars(raw, 3);

	// Reading file blocks
	// Header - data
	while (raw.pos < raw.view.length) {
		parser_blend_align(raw);
		let b: block_t = {};

		// Block type
		b.code = parser_blend_read_chars(raw, 4);
		if (b.code == "ENDB") {
			break;
		}

		array_push(raw.blocks, b);
		b.blend = raw;

		// Total block length
		b.size = parser_blend_read_i32(raw);

		// Memory address
		let addr: string = parser_blend_read_pointer(raw);
		if (map_get(raw.map, addr) == null) {
			map_set(raw.map, addr, b);
		}

		// Index of dna struct contained in this block
		b.sdna_index = parser_blend_read_i32(raw);

		// Number of dna structs in this block
		b.count = parser_blend_read_i32(raw);

		b.pos = raw.pos;

		// This block stores dna structures
		if (b.code == "DNA1") {
			raw.dna = {};
			raw.dna.names = [];
			raw.dna.types = [];
			raw.dna.types_length = [];
			raw.dna.structs = [];

			parser_blend_read_chars(raw, 4); // SDNA
			parser_blend_read_chars(raw, 4); // NAME
			let names_count: i32 = parser_blend_read_i32(raw);
			for (let i: i32 = 0; i < names_count; ++i) {
				array_push(raw.dna.names, parser_blend_read_string(raw));
			}
			parser_blend_align(raw);

			parser_blend_read_chars(raw, 4); // TYPE
			let types_count: i32 = parser_blend_read_i32(raw);
			for (let i: i32 = 0; i < types_count; ++i) {
				array_push(raw.dna.types, parser_blend_read_string(raw));
			}
			parser_blend_align(raw);

			parser_blend_read_chars(raw, 4); // TLEN
			for (let i: i32 = 0; i < types_count; ++i) {
				array_push(raw.dna.types_length, parser_blend_read_i16(raw));
			}
			parser_blend_align(raw);

			parser_blend_read_chars(raw, 4); // STRC
			let struct_count: i32 = parser_blend_read_i32(raw);
			for (let i: i32 = 0; i < struct_count; ++i) {
				let ds: dna_struct_t = {};
				array_push(raw.dna.structs, ds);
				ds.dna = raw.dna;
				ds.type = parser_blend_read_i16(raw);
				let field_count: i32 = parser_blend_read_i16(raw);
				if (field_count > 0) {
					ds.field_types = [];
					ds.field_names = [];
					for (let j: i32 = 0; j < field_count; ++j) {
						array_push(ds.field_types, parser_blend_read_i16(raw));
						array_push(ds.field_names, parser_blend_read_i16(raw));
					}
				}
			}
		}
		else {
			raw.pos += b.size;
		}
	}
}

function parser_blend_align(raw: blend_t) {
	// 4 bytes aligned
	let mod: i32 = raw.pos % 4;
	if (mod > 0) {
		raw.pos += 4 - mod;
	}
}

function parser_blend_read_i8(raw: blend_t): i32 {
	let i: i32 = buffer_get_i8(raw.view, raw.pos);
	raw.pos += 1;
	return i;
}

function parser_blend_read_i16(raw: blend_t): i32 {
	let i: i32 = buffer_get_i16(raw.view, raw.pos);
	raw.pos += 2;
	return i;
}

function parser_blend_read_i32(raw: blend_t): i32 {
	let i: i32 = buffer_get_i32(raw.view, raw.pos);
	raw.pos += 4;
	return i;
}

function parser_blend_read_i64(raw: blend_t): i64 {
	let i: i32 = buffer_get_i64(raw.view, raw.pos);
	raw.pos += 8;
	return i;
}

function parser_blend_read_f32(raw: blend_t): f32 {
	let f: f32 = buffer_get_f32(raw.view, raw.pos);
	raw.pos += 4;
	return f;
}

function parser_blend_read_f64(raw: blend_t): f64 {
	let f: f64 = buffer_get_f64(raw.view, raw.pos);
	raw.pos += 8;
	return f;
}

function parser_blend_read_i8array(raw: blend_t, len: i32): i32_array_t {
	let ar: i32_array_t = i32_array_create(len);
	for (let i: i32 = 0; i < len; ++i) {
		ar[i] = parser_blend_read_i8(raw);
	}
	return ar;
}

function parser_blend_read_i16array(raw: blend_t, len: i32): i32_array_t {
	let ar: i32_array_t = i32_array_create(len);
	for (let i: i32 = 0; i < len; ++i) {
		ar[i] = parser_blend_read_i16(raw);
	}
	return ar;
}

function parser_blend_read_i32array(raw: blend_t, len: i32): i32_array_t {
	let ar: i32_array_t = i32_array_create(len);
	for (let i: i32 = 0; i < len; ++i) {
		ar[i] = parser_blend_read_i32(raw);
	}
	return ar;
}

function parser_blend_read_f32array(raw: blend_t, len: i32): f32_array_t {
	let ar: f32_array_t = f32_array_create(len);
	for (let i: i32 = 0; i < len; ++i) {
		ar[i] = parser_blend_read_f32(raw);
	}
	return ar;
}

function parser_blend_read_f64array(raw: blend_t, len: i32): f32_array_t { // f64_array_t
	let ar: f32_array_t = f32_array_create(len);
	for (let i: i32 = 0; i < len; ++i) {
		ar[i] = parser_blend_read_f64(raw);
	}
	return ar;
}

function parser_blend_read_string(raw: blend_t): string {
	let s: string = "";
	while (true) {
		let ch: u8 = parser_blend_read_i8(raw);
		if (ch == 0) {
			break;
		}
		s += string_from_char_code(ch);
	}
	return s;
}

function parser_blend_read_chars(raw: blend_t, len: i32): string {
	let s: string = "";
	for (let i: i32 = 0; i < len; ++i) {
		s += parser_blend_read_char(raw);
	}
	return s;
}

function parser_blend_read_char(raw: blend_t): string {
	return string_from_char_code(parser_blend_read_i8(raw));
}

function parser_blend_read_pointer(raw: blend_t): string {
	let i: u64 = parser_blend_read_i64(raw);
	return u64_to_string(i);
}

type block_t = {
	blend?: blend_t;
	code?: string;
	size?: i32;
	sdna_index?: i32;
	count?: i32;
	pos?: i32; // Byte pos of data start in blob
};

type dna_t = {
	names?: string[];
	types?: string[];
	types_length?: i32[];
	structs?: dna_struct_t[];
};

type dna_struct_t = {
	dna?: dna_t;
	type?: i32; // Index in dna.types
	field_types?: i32[]; // Index in dna.types
	field_names?: i32[]; // Index in dna.names
};

type bl_handle_t = {
	block?: block_t;
	offset?: i32; // Block data bytes offset
	ds?: dna_struct_t;
};

function bl_handle_get_size(raw: bl_handle_t, index: i32): i32 {
	let name_index: i32 = raw.ds.field_names[index];
	let type_index: i32 = raw.ds.field_types[index];
	let dna: dna_t = raw.ds.dna;
	let n: string = dna.names[name_index];
	let size: i32 = 0;
	if (string_index_of(n, "*") >= 0) {
		size = raw.block.blend.pointer_size;
	}
	else {
		size = dna.types_length[type_index];
	}
	if (string_index_of(n, "[") > 0) {
		size *= bl_handle_get_array_len(n);
	}
	return size;
}

function bl_handle_base_name(s: string): string {
	while (char_at(s, 0) == "*") {
		s = substring(s, 1, s.length);
	}
	if (char_at(s, s.length - 1) == "]") {
		s = substring(s, 0, string_index_of(s, "["));
	}
	return s;
}

function bl_handle_get_array_len(s: string): i32 {
	return parse_int(substring(s, string_index_of(s, "[") + 1, string_index_of(s, "]")));
}

let bl_handle_tmp_i: i64;
let bl_handle_tmp_f: f64;

function bl_handle_get_f(raw: bl_handle_t, name: string): f64 {
	let p: f64_ptr = bl_handle_get(raw, name);
	return DEREFERENCE(p);
}

function bl_handle_get_i(raw: bl_handle_t, name: string): i64 {
	let p: i64_ptr = bl_handle_get(raw, name);
	return DEREFERENCE(p);
}

function bl_handle_get(raw: bl_handle_t, name: string, index: i32 = 0, as_type: string = null, array_len: i32 = 0): any {
	// Return raw type or structure
	let dna: dna_t = raw.ds.dna;
	for (let i: i32 = 0; i < raw.ds.field_names.length; ++i) {
		let name_index: i32 = raw.ds.field_names[i];
		let dna_name: string = dna.names[name_index];
		if (name == bl_handle_base_name(dna_name)) {

			let type_index: i32 = raw.ds.field_types[i];
			let type: string = dna.types[type_index];
			let new_offset: i32 = raw.offset;
			for (let j: i32 = 0; j < i; ++j) {
				new_offset += bl_handle_get_size(raw, j);
			}
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
				let blend: blend_t = raw.block.blend;
				blend.pos = raw.block.pos + new_offset;
				let is_array: bool = char_at(dna_name, dna_name.length - 1) == "]";
				let len: i32 = is_array ? (array_len > 0 ? array_len : bl_handle_get_array_len(dna_name)) : 1;

				if (type == "int") {
					if (is_array) {
						return parser_blend_read_i32array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_i = parser_blend_read_i32(blend);
						return ADDRESS(bl_handle_tmp_i);
					}
				}
				else if (type == "char") {
					if (is_array) {
						return parser_blend_read_string(blend);
					}
					else {
						bl_handle_tmp_i = parser_blend_read_i8(blend);
						return ADDRESS(bl_handle_tmp_i);
					}
				}
				else if (type == "uchar") {
					if (is_array) {
						return parser_blend_read_i8array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_i = parser_blend_read_i8(blend);
						return ADDRESS(bl_handle_tmp_i);
					}
				}
				else if (type == "short") {
					if (is_array) {
						return parser_blend_read_i16array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_i = parser_blend_read_i16(blend);
						return ADDRESS(bl_handle_tmp_i);
					}
				}
				else if (type == "ushort") {
					if (is_array) {
						return parser_blend_read_i16array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_i = parser_blend_read_i16(blend);
						return ADDRESS(bl_handle_tmp_i);
					}
				}
				else if (type == "float") {
					if (is_array) {
						return parser_blend_read_f32array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_f = parser_blend_read_f32(blend);
						return ADDRESS(bl_handle_tmp_f);
					}
				}
				else if (type == "double") {
					if (is_array) {
						return parser_blend_read_f64array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_f = parser_blend_read_f64(blend);
						return ADDRESS(bl_handle_tmp_f);
					}
				}
				else if (type == "long") {
					if (is_array) {
						return parser_blend_read_i32array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_i = parser_blend_read_i32(blend);
						return ADDRESS(bl_handle_tmp_i);
					}
				}
				else if (type == "ulong") {
					if (is_array) {
						return parser_blend_read_i32array(blend, len).buffer;
					}
					else {
						bl_handle_tmp_i = parser_blend_read_i32(blend);
						return ADDRESS(bl_handle_tmp_i);
					}
				}
				else if (type == "int64_t") {
					bl_handle_tmp_i = parser_blend_read_i64(blend);
					return ADDRESS(bl_handle_tmp_i);
				}
				else if (type == "uint64_t") {
					bl_handle_tmp_i = parser_blend_read_i64(blend);
					return ADDRESS(bl_handle_tmp_i);
				}
				else if (type == "void" && char_at(dna_name, 0) == "*") {
					bl_handle_tmp_i = parser_blend_read_i64(blend);
					return ADDRESS(bl_handle_tmp_i);
				}
			}

			// Structure
			let h: bl_handle_t = {};
			h.offset = 0;
			h.ds = parser_blend_get_struct(dna, type_index);
			let is_pointer: bool = char_at(dna_name, 0) == "*";
			if (is_pointer) {
				raw.block.blend.pos = raw.block.pos + new_offset;
				let addr: string = parser_blend_read_pointer(raw.block.blend);
				let bl: block_t = map_get(raw.block.blend.map, addr);
				if (bl != null) {
					h.block = bl;
				}
				else {
					h.block = raw.block;
				}
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
