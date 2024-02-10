// .blend file parser
// Reference:
// https://github.com/fschutt/mystery-of-the-blend-backup
// https://web.archive.org/web/20170630054951/http://www.atmind.nl/blender/mystery_ot_blend.html
// Usage:
// let bl = ParserBlend.init(blob: DataView);
// krom_log(ParserBlend.dir(bl, "Scene"));
// let scenes = ParserBlend.get(bl, "Scene");
// krom_log(BlHandle.get(BlHandle.get(scenes[0], "id"), "name"));

class BlendRaw {
	pos: i32;
	view: DataView;

	// Header
	version: string;
	pointerSize: i32;
	littleEndian: bool;
	// Data
	blocks: Block[] = [];
	dna: Dna = null;
	map = new Map<any, Block>(); // Map blocks by memory address
}

class ParserBlend {

	static init = (buffer: ArrayBuffer): BlendRaw => {
		let raw = new BlendRaw();
		raw.view = new DataView(buffer);
		raw.pos = 0;
		if (ParserBlend.readChars(raw, 7) != "BLENDER") {
			raw.view = new DataView(krom_inflate(buffer, false));
			raw.pos = 0;
			if (ParserBlend.readChars(raw, 7) != "BLENDER") return null;
		}
		ParserBlend.parse(raw);
		return raw;
	}

	static dir = (raw: BlendRaw, type: string): string[] => {
		// Return structure fields
		let typeIndex = ParserBlend.getTypeIndex(raw.dna, type);
		if (typeIndex == -1) return null;
		let ds = ParserBlend.getStruct(raw.dna, typeIndex);
		let fields: string[] = [];
		for (let i = 0; i < ds.fieldNames.length; ++i) {
			let nameIndex = ds.fieldNames[i];
			let typeIndex = ds.fieldTypes[i];
			fields.push(raw.dna.types[typeIndex] + " " + raw.dna.names[nameIndex]);
		}
		return fields;
	}

	static get = (raw: BlendRaw, type: string): BlHandleRaw[] => {
		if (raw.dna == null) return null;
		// Return all structures of type
		let typeIndex = ParserBlend.getTypeIndex(raw.dna, type);
		if (typeIndex == -1) return null;
		let ds = ParserBlend.getStruct(raw.dna, typeIndex);
		let handles: BlHandleRaw[] = [];
		for (let b of raw.blocks) {
			if (raw.dna.structs[b.sdnaIndex].type == typeIndex) {
				let h = new BlHandleRaw();
				handles.push(h);
				h.block = b;
				h.ds = ds;
			}
		}
		return handles;
	}

	static getStruct = (dna: Dna, typeIndex: i32): DnaStruct => {
		for (let ds of dna.structs) if (ds.type == typeIndex) return ds;
		return null;
	}

	static getTypeIndex = (dna: Dna, type: string): i32 => {
		for (let i = 0; i < dna.types.length; ++i) if (type == dna.types[i]) return i;
		return -1;
	}

	static parse = (raw: BlendRaw) => {
		// Pointer size: _ 32bit, - 64bit
		raw.pointerSize = ParserBlend.readChar(raw) == "_" ? 4 : 8;

		// v - little endian, V - big endian
		raw.littleEndian = ParserBlend.readChar(raw) == "v";

		raw.version = ParserBlend.readChars(raw, 3);

		// Reading file blocks
		// Header - data
		while (raw.pos < raw.view.byteLength) {
			ParserBlend.align(raw);
			let b = new Block();

			// Block type
			b.code = ParserBlend.readChars(raw, 4);
			if (b.code == "ENDB") break;

			raw.blocks.push(b);
			b.blend = raw;

			// Total block length
			b.size = ParserBlend.read32(raw);

			// Memory address
			let addr = ParserBlend.readPointer(raw);
			if (!raw.map.has(addr)) raw.map.set(addr, b);

			// Index of dna struct contained in this block
			b.sdnaIndex = ParserBlend.read32(raw);

			// Number of dna structs in this block
			b.count = ParserBlend.read32(raw);

			b.pos = raw.pos;

			// This block stores dna structures
			if (b.code == "DNA1") {
				raw.dna = new Dna();

				let id = ParserBlend.readChars(raw, 4); // SDNA
				let nameId = ParserBlend.readChars(raw, 4); // NAME
				let namesCount = ParserBlend.read32(raw);
				for (let i = 0; i < namesCount; ++i) {
					raw.dna.names.push(ParserBlend.readString(raw));
				}
				ParserBlend.align(raw);

				let typeId = ParserBlend.readChars(raw, 4); // TYPE
				let typesCount = ParserBlend.read32(raw);
				for (let i = 0; i < typesCount; ++i) {
					raw.dna.types.push(ParserBlend.readString(raw));
				}
				ParserBlend.align(raw);

				let lenId = ParserBlend.readChars(raw, 4); // TLEN
				for (let i = 0; i < typesCount; ++i) {
					raw.dna.typesLength.push(ParserBlend.read16(raw));
				}
				ParserBlend.align(raw);

				let structId = ParserBlend.readChars(raw, 4); // STRC
				let structCount = ParserBlend.read32(raw);
				for (let i = 0; i < structCount; ++i) {
					let ds = new DnaStruct();
					raw.dna.structs.push(ds);
					ds.dna = raw.dna;
					ds.type = ParserBlend.read16(raw);
					let fieldCount = ParserBlend.read16(raw);
					if (fieldCount > 0) {
						ds.fieldTypes = [];
						ds.fieldNames = [];
						for (let j = 0; j < fieldCount; ++j) {
							ds.fieldTypes.push(ParserBlend.read16(raw));
							ds.fieldNames.push(ParserBlend.read16(raw));
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
		let mod = raw.pos % 4;
		if (mod > 0) raw.pos += 4 - mod;
	}

	static read8 = (raw: BlendRaw): i32 => {
		let i = raw.view.getUint8(raw.pos);
		raw.pos += 1;
		return i;
	}

	static read16 = (raw: BlendRaw): i32 => {
		let i = raw.view.getInt16(raw.pos, raw.littleEndian);
		raw.pos += 2;
		return i;
	}

	static read32 = (raw: BlendRaw): i32 => {
		let i = raw.view.getInt32(raw.pos, raw.littleEndian);
		raw.pos += 4;
		return i;
	}

	static read64 = (raw: BlendRaw): any => {
		let aview: any = raw.view;
		let i = aview.getBigInt64(raw.pos, raw.littleEndian);
		raw.pos += 8;
		return i;
	}

	static readf32 = (raw: BlendRaw): f32 => {
		let f = raw.view.getFloat32(raw.pos, raw.littleEndian);
		raw.pos += 4;
		return f;
	}

	static read8array = (raw: BlendRaw, len: i32): Int32Array => {
		let ar = new Int32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = ParserBlend.read8(raw);
		return ar;
	}

	static read16array = (raw: BlendRaw, len: i32): Int32Array => {
		let ar = new Int32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = ParserBlend.read16(raw);
		return ar;
	}

	static read32array = (raw: BlendRaw, len: i32): Int32Array => {
		let ar = new Int32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = ParserBlend.read32(raw);
		return ar;
	}

	static readf32array = (raw: BlendRaw, len: i32): Float32Array => {
		let ar = new Float32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = ParserBlend.readf32(raw);
		return ar;
	}

	static readString = (raw: BlendRaw): string => {
		let s = "";
		while (true) {
			let ch = ParserBlend.read8(raw);
			if (ch == 0) break;
			s += String.fromCharCode(ch);
		}
		return s;
	}

	static readChars = (raw: BlendRaw, len: i32): string => {
		let s = "";
		for (let i = 0; i < len; ++i) s += ParserBlend.readChar(raw);
		return s;
	}

	static readChar = (raw: BlendRaw): string => {
		return String.fromCharCode(ParserBlend.read8(raw));
	}

	static readPointer = (raw: BlendRaw): any => {
		return raw.pointerSize == 4 ? ParserBlend.read32(raw) : ParserBlend.read64(raw);
	}
}

class Block {
	blend: BlendRaw;
	code: string;
	size: i32;
	sdnaIndex: i32;
	count: i32;
	pos: i32; // Byte pos of data start in blob
}

class Dna {
	names: string[] = [];
	types: string[] = [];
	typesLength: i32[] = [];
	structs: DnaStruct[] = [];
}

class DnaStruct {
	dna: Dna;
	type: i32; // Index in dna.types
	fieldTypes: i32[]; // Index in dna.types
	fieldNames: i32[]; // Index in dna.names
}

class BlHandleRaw {
	block: Block;
	offset: i32 = 0; // Block data bytes offset
	ds: DnaStruct;
}

class BlHandle {
	static getSize = (raw: BlHandleRaw, index: i32): i32 => {
		let nameIndex = raw.ds.fieldNames[index];
		let typeIndex = raw.ds.fieldTypes[index];
		let dna = raw.ds.dna;
		let n = dna.names[nameIndex];
		let size = 0;
		if (n.indexOf("*") >= 0) size = raw.block.blend.pointerSize;
		else size = dna.typesLength[typeIndex];
		if (n.indexOf("[") > 0) size *= BlHandle.getArrayLen(n);
		return size;
	}

	static baseName = (s: string): string => {
		while (s.charAt(0) == "*") s = s.substring(1, s.length);
		if (s.charAt(s.length - 1) == "]") s = s.substring(0, s.indexOf("["));
		return s;
	}

	static getArrayLen = (s: string): i32 => {
		return parseInt(s.substring(s.indexOf("[") + 1, s.indexOf("]")));
	}

	static get = (raw: BlHandleRaw, name: string, index = 0, asType: string = null, arrayLen = 0): any => {
		// Return raw type or structure
		let dna = raw.ds.dna;
		for (let i = 0; i < raw.ds.fieldNames.length; ++i) {
			let nameIndex = raw.ds.fieldNames[i];
			let dnaName = dna.names[nameIndex];
			if (name == BlHandle.baseName(dnaName)) {
				let typeIndex = raw.ds.fieldTypes[i];
				let type = dna.types[typeIndex];
				let newOffset = raw.offset;
				for (let j = 0; j < i; ++j) newOffset += BlHandle.getSize(raw, j);
				// Cast void * to type
				if (asType != null) {
					for (let i = 0; i < dna.types.length; ++i) {
						if (dna.types[i] == asType) {
							typeIndex = i;
							break;
						}
					}
				}
				// Raw type
				if (typeIndex < 12) {
					let blend = raw.block.blend;
					blend.pos = raw.block.pos + newOffset;
					let isArray = dnaName.charAt(dnaName.length - 1) == "]";
					let len = isArray ? (arrayLen > 0 ? arrayLen : this.getArrayLen(dnaName)) : 1;
					switch (type) {
						case "int": return isArray ? ParserBlend.read32array(blend, len) : ParserBlend.read32(blend);
						case "char": return isArray ? ParserBlend.readString(blend) : ParserBlend.read8(blend);
						case "uchar": return isArray ? ParserBlend.read8array(blend, len) : ParserBlend.read8(blend);
						case "short": return isArray ? ParserBlend.read16array(blend, len) : ParserBlend.read16(blend);
						case "ushort": return isArray ? ParserBlend.read16array(blend, len) : ParserBlend.read16(blend);
						case "float": return isArray ? ParserBlend.readf32array(blend, len) : ParserBlend.readf32(blend);
						case "double": return 0; //ParserBlend.readf64(blend);
						case "long": return isArray ? ParserBlend.read32array(blend, len) : ParserBlend.read32(blend);
						case "ulong": return isArray ? ParserBlend.read32array(blend, len) : ParserBlend.read32(blend);
						case "int64_t": return ParserBlend.read64(blend);
						case "uint64_t": return ParserBlend.read64(blend);
						case "void": if (dnaName.charAt(0) == "*") { return ParserBlend.read64(blend); };
					}
				}
				// Structure
				let h = new BlHandleRaw();
				h.ds = ParserBlend.getStruct(dna, typeIndex);
				let isPointer = dnaName.charAt(0) == "*";
				if (isPointer) {
					raw.block.blend.pos = raw.block.pos + newOffset;
					let addr = ParserBlend.readPointer(raw.block.blend);
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
				h.offset += dna.typesLength[typeIndex] * index;
				return h;
			}
		}
		return null;
	}
}
