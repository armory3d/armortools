// .blend file parser
// Reference:
// https://github.com/fschutt/mystery-of-the-blend-backup
// https://web.archive.org/web/20170630054951/http://www.atmind.nl/blender/mystery_ot_blend.html
// Usage:
// let bl: BlendRaw = ParserBlend.init(blob: DataView);
// krom_log(ParserBlend.dir(bl, "Scene"));
// let scenes: any = ParserBlend.get(bl, "Scene");
// krom_log(BlHandle.get(BlHandle.get(scenes[0], "id"), "name"));

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
	map: Map<any, Block> = new Map(); // Map blocks by memory address
}

class ParserBlend {

	static init = (buffer: ArrayBuffer): BlendRaw => {
		let raw: BlendRaw = new BlendRaw();
		raw.view = new DataView(buffer);
		raw.pos = 0;
		if (ParserBlend.read_chars(raw, 7) != "BLENDER") {
			raw.view = new DataView(krom_inflate(buffer, false));
			raw.pos = 0;
			if (ParserBlend.read_chars(raw, 7) != "BLENDER") return null;
		}
		ParserBlend.parse(raw);
		return raw;
	}

	static dir = (raw: BlendRaw, type: string): string[] => {
		// Return structure fields
		let typeIndex: i32 = ParserBlend.get_type_index(raw.dna, type);
		if (typeIndex == -1) return null;
		let ds: DnaStruct = ParserBlend.get_struct(raw.dna, typeIndex);
		let fields: string[] = [];
		for (let i: i32 = 0; i < ds.field_names.length; ++i) {
			let nameIndex: i32 = ds.field_names[i];
			let typeIndex: i32 = ds.field_types[i];
			fields.push(raw.dna.types[typeIndex] + " " + raw.dna.names[nameIndex]);
		}
		return fields;
	}

	static get = (raw: BlendRaw, type: string): BlHandleRaw[] => {
		if (raw.dna == null) return null;
		// Return all structures of type
		let typeIndex: i32 = ParserBlend.get_type_index(raw.dna, type);
		if (typeIndex == -1) return null;
		let ds: DnaStruct = ParserBlend.get_struct(raw.dna, typeIndex);
		let handles: BlHandleRaw[] = [];
		for (let b of raw.blocks) {
			if (raw.dna.structs[b.sdna_index].type == typeIndex) {
				let h: BlHandleRaw = new BlHandleRaw();
				handles.push(h);
				h.block = b;
				h.ds = ds;
			}
		}
		return handles;
	}

	static get_struct = (dna: Dna, typeIndex: i32): DnaStruct => {
		for (let ds of dna.structs) if (ds.type == typeIndex) return ds;
		return null;
	}

	static get_type_index = (dna: Dna, type: string): i32 => {
		for (let i: i32 = 0; i < dna.types.length; ++i) if (type == dna.types[i]) return i;
		return -1;
	}

	static parse = (raw: BlendRaw) => {
		// Pointer size: _ 32bit, - 64bit
		raw.pointer_size = ParserBlend.read_char(raw) == "_" ? 4 : 8;

		// v - little endian, V - big endian
		raw.little_endian = ParserBlend.read_char(raw) == "v";

		raw.version = ParserBlend.read_chars(raw, 3);

		// Reading file blocks
		// Header - data
		while (raw.pos < raw.view.byteLength) {
			ParserBlend.align(raw);
			let b: Block = new Block();

			// Block type
			b.code = ParserBlend.read_chars(raw, 4);
			if (b.code == "ENDB") break;

			raw.blocks.push(b);
			b.blend = raw;

			// Total block length
			b.size = ParserBlend.read_i32(raw);

			// Memory address
			let addr: any = ParserBlend.read_pointer(raw);
			if (!raw.map.has(addr)) raw.map.set(addr, b);

			// Index of dna struct contained in this block
			b.sdna_index = ParserBlend.read_i32(raw);

			// Number of dna structs in this block
			b.count = ParserBlend.read_i32(raw);

			b.pos = raw.pos;

			// This block stores dna structures
			if (b.code == "DNA1") {
				raw.dna = new Dna();

				ParserBlend.read_chars(raw, 4); // SDNA
				ParserBlend.read_chars(raw, 4); // NAME
				let namesCount: i32 = ParserBlend.read_i32(raw);
				for (let i: i32 = 0; i < namesCount; ++i) {
					raw.dna.names.push(ParserBlend.read_string(raw));
				}
				ParserBlend.align(raw);

				ParserBlend.read_chars(raw, 4); // TYPE
				let typesCount: i32 = ParserBlend.read_i32(raw);
				for (let i: i32 = 0; i < typesCount; ++i) {
					raw.dna.types.push(ParserBlend.read_string(raw));
				}
				ParserBlend.align(raw);

				ParserBlend.read_chars(raw, 4); // TLEN
				for (let i: i32 = 0; i < typesCount; ++i) {
					raw.dna.types_length.push(ParserBlend.read_i16(raw));
				}
				ParserBlend.align(raw);

				ParserBlend.read_chars(raw, 4); // STRC
				let structCount: i32 = ParserBlend.read_i32(raw);
				for (let i: i32 = 0; i < structCount; ++i) {
					let ds: DnaStruct = new DnaStruct();
					raw.dna.structs.push(ds);
					ds.dna = raw.dna;
					ds.type = ParserBlend.read_i16(raw);
					let fieldCount: i32 = ParserBlend.read_i16(raw);
					if (fieldCount > 0) {
						ds.field_types = [];
						ds.field_names = [];
						for (let j: i32 = 0; j < fieldCount; ++j) {
							ds.field_types.push(ParserBlend.read_i16(raw));
							ds.field_names.push(ParserBlend.read_i16(raw));
						}
					}
				}
			}
			else {
				raw.pos += b.size;
			}
		}
	}

	static align = (raw: BlendRaw) => {
		// 4 bytes aligned
		let mod: i32 = raw.pos % 4;
		if (mod > 0) raw.pos += 4 - mod;
	}

	static read_i8 = (raw: BlendRaw): i32 => {
		let i: i32 = raw.view.getUint8(raw.pos);
		raw.pos += 1;
		return i;
	}

	static read_i16 = (raw: BlendRaw): i32 => {
		let i: i32 = raw.view.getInt16(raw.pos, raw.little_endian);
		raw.pos += 2;
		return i;
	}

	static read_i32 = (raw: BlendRaw): i32 => {
		let i: i32 = raw.view.getInt32(raw.pos, raw.little_endian);
		raw.pos += 4;
		return i;
	}

	static read_i64 = (raw: BlendRaw): any => {
		let aview: any = raw.view;
		let i: i32 = aview.getBigInt64(raw.pos, raw.little_endian);
		raw.pos += 8;
		return i;
	}

	static read_f32 = (raw: BlendRaw): f32 => {
		let f: f32 = raw.view.getFloat32(raw.pos, raw.little_endian);
		raw.pos += 4;
		return f;
	}

	static read_i8array = (raw: BlendRaw, len: i32): Int32Array => {
		let ar: Int32Array = new Int32Array(len);
		for (let i: i32 = 0; i < len; ++i) ar[i] = ParserBlend.read_i8(raw);
		return ar;
	}

	static read_i16array = (raw: BlendRaw, len: i32): Int32Array => {
		let ar: Int32Array = new Int32Array(len);
		for (let i: i32 = 0; i < len; ++i) ar[i] = ParserBlend.read_i16(raw);
		return ar;
	}

	static read_i32array = (raw: BlendRaw, len: i32): Int32Array => {
		let ar: Int32Array = new Int32Array(len);
		for (let i: i32 = 0; i < len; ++i) ar[i] = ParserBlend.read_i32(raw);
		return ar;
	}

	static read_f32array = (raw: BlendRaw, len: i32): Float32Array => {
		let ar: Float32Array = new Float32Array(len);
		for (let i: i32 = 0; i < len; ++i) ar[i] = ParserBlend.read_f32(raw);
		return ar;
	}

	static read_string = (raw: BlendRaw): string => {
		let s: string = "";
		while (true) {
			let ch: i32 = ParserBlend.read_i8(raw);
			if (ch == 0) break;
			s += String.fromCharCode(ch);
		}
		return s;
	}

	static read_chars = (raw: BlendRaw, len: i32): string => {
		let s: string = "";
		for (let i: i32 = 0; i < len; ++i) s += ParserBlend.read_char(raw);
		return s;
	}

	static read_char = (raw: BlendRaw): string => {
		return String.fromCharCode(ParserBlend.read_i8(raw));
	}

	static read_pointer = (raw: BlendRaw): any => {
		return raw.pointer_size == 4 ? ParserBlend.read_i32(raw) : ParserBlend.read_i64(raw);
	}
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

class BlHandle {
	static get_size = (raw: BlHandleRaw, index: i32): i32 => {
		let nameIndex: i32 = raw.ds.field_names[index];
		let typeIndex: i32 = raw.ds.field_types[index];
		let dna: Dna = raw.ds.dna;
		let n: string = dna.names[nameIndex];
		let size: i32 = 0;
		if (n.indexOf("*") >= 0) size = raw.block.blend.pointer_size;
		else size = dna.types_length[typeIndex];
		if (n.indexOf("[") > 0) size *= BlHandle.get_array_len(n);
		return size;
	}

	static base_name = (s: string): string => {
		while (s.charAt(0) == "*") s = s.substring(1, s.length);
		if (s.charAt(s.length - 1) == "]") s = s.substring(0, s.indexOf("["));
		return s;
	}

	static get_array_len = (s: string): i32 => {
		return parseInt(s.substring(s.indexOf("[") + 1, s.indexOf("]")));
	}

	static get = (raw: BlHandleRaw, name: string, index: i32 = 0, asType: string = null, arrayLen: i32 = 0): any => {
		// Return raw type or structure
		let dna: Dna = raw.ds.dna;
		for (let i: i32 = 0; i < raw.ds.field_names.length; ++i) {
			let nameIndex: i32 = raw.ds.field_names[i];
			let dnaName: string = dna.names[nameIndex];
			if (name == BlHandle.base_name(dnaName)) {
				let typeIndex: i32 = raw.ds.field_types[i];
				let type: string = dna.types[typeIndex];
				let newOffset: i32 = raw.offset;
				for (let j: i32 = 0; j < i; ++j) newOffset += BlHandle.get_size(raw, j);
				// Cast void * to type
				if (asType != null) {
					for (let i: i32 = 0; i < dna.types.length; ++i) {
						if (dna.types[i] == asType) {
							typeIndex = i;
							break;
						}
					}
				}
				// Raw type
				if (typeIndex < 12) {
					let blend: BlendRaw = raw.block.blend;
					blend.pos = raw.block.pos + newOffset;
					let isArray: bool = dnaName.charAt(dnaName.length - 1) == "]";
					let len: i32 = isArray ? (arrayLen > 0 ? arrayLen : this.get_array_len(dnaName)) : 1;
					switch (type) {
						case "int": return isArray ? ParserBlend.read_i32array(blend, len) : ParserBlend.read_i32(blend);
						case "char": return isArray ? ParserBlend.read_string(blend) : ParserBlend.read_i8(blend);
						case "uchar": return isArray ? ParserBlend.read_i8array(blend, len) : ParserBlend.read_i8(blend);
						case "short": return isArray ? ParserBlend.read_i16array(blend, len) : ParserBlend.read_i16(blend);
						case "ushort": return isArray ? ParserBlend.read_i16array(blend, len) : ParserBlend.read_i16(blend);
						case "float": return isArray ? ParserBlend.read_f32array(blend, len) : ParserBlend.read_f32(blend);
						case "double": return 0; //ParserBlend.readf64(blend);
						case "long": return isArray ? ParserBlend.read_i32array(blend, len) : ParserBlend.read_i32(blend);
						case "ulong": return isArray ? ParserBlend.read_i32array(blend, len) : ParserBlend.read_i32(blend);
						case "int64_t": return ParserBlend.read_i64(blend);
						case "uint64_t": return ParserBlend.read_i64(blend);
						case "void": if (dnaName.charAt(0) == "*") { return ParserBlend.read_i64(blend); };
					}
				}
				// Structure
				let h: BlHandleRaw = new BlHandleRaw();
				h.ds = ParserBlend.get_struct(dna, typeIndex);
				let isPointer: bool = dnaName.charAt(0) == "*";
				if (isPointer) {
					raw.block.blend.pos = raw.block.pos + newOffset;
					let addr: any = ParserBlend.read_pointer(raw.block.blend);
					if (raw.block.blend.map.has(addr)) {
						h.block = raw.block.blend.map.get(addr);
					}
					else h.block = raw.block;
					h.offset = 0;
				}
				else {
					h.block = raw.block;
					h.offset = newOffset;
				}
				h.offset += dna.types_length[typeIndex] * index;
				return h;
			}
		}
		return null;
	}
}
