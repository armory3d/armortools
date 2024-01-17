// .blend file parser
// Reference:
// https://github.com/fschutt/mystery-of-the-blend-backup
// https://web.archive.org/web/20170630054951/http://www.atmind.nl/blender/mystery_ot_blend.html
// Usage:
// let bl = new ParserBlend(blob: DataView);
// Krom.log(bl.dir("Scene"));
// let scenes = bl.get("Scene");
// Krom.log(scenes[0].get("id").get("name"));

class ParserBlend {

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

	constructor(buffer: ArrayBuffer) {
		this.view = new DataView(buffer);
		this.pos = 0;
		if (this.readChars(7) != "BLENDER") {
			this.view = new DataView(Krom.inflate(buffer, false));
			this.pos = 0;
			if (this.readChars(7) != "BLENDER") return;
		}
		this.parse();
	}

	dir = (type: string): string[] => {
		// Return structure fields
		let typeIndex = ParserBlend.getTypeIndex(this.dna, type);
		if (typeIndex == -1) return null;
		let ds = ParserBlend.getStruct(this.dna, typeIndex);
		let fields: string[] = [];
		for (let i = 0; i < ds.fieldNames.length; ++i) {
			let nameIndex = ds.fieldNames[i];
			let typeIndex = ds.fieldTypes[i];
			fields.push(this.dna.types[typeIndex] + " " + this.dna.names[nameIndex]);
		}
		return fields;
	}

	get = (type: string): BlHandle[] => {
		if (this.dna == null) return null;
		// Return all structures of type
		let typeIndex = ParserBlend.getTypeIndex(this.dna, type);
		if (typeIndex == -1) return null;
		let ds = ParserBlend.getStruct(this.dna, typeIndex);
		let handles: BlHandle[] = [];
		for (let b of this.blocks) {
			if (this.dna.structs[b.sdnaIndex].type == typeIndex) {
				let h = new BlHandle();
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

	parse = () => {
		// Pointer size: _ 32bit, - 64bit
		this.pointerSize = this.readChar() == "_" ? 4 : 8;

		// v - little endian, V - big endian
		this.littleEndian = this.readChar() == "v";

		this.version = this.readChars(3);

		// Reading file blocks
		// Header - data
		while (this.pos < this.view.byteLength) {
			this.align();
			let b = new Block();

			// Block type
			b.code = this.readChars(4);
			if (b.code == "ENDB") break;

			this.blocks.push(b);
			b.blend = this;

			// Total block length
			b.size = this.read32();

			// Memory address
			let addr = this.readPointer();
			if (!this.map.has(addr)) this.map.set(addr, b);

			// Index of dna struct contained in this block
			b.sdnaIndex = this.read32();

			// Number of dna structs in this block
			b.count = this.read32();

			b.pos = this.pos;

			// This block stores dna structures
			if (b.code == "DNA1") {
				this.dna = new Dna();

				let id = this.readChars(4); // SDNA
				let nameId = this.readChars(4); // NAME
				let namesCount = this.read32();
				for (let i = 0; i < namesCount; ++i) {
					this.dna.names.push(this.readString());
				}
				this.align();

				let typeId = this.readChars(4); // TYPE
				let typesCount = this.read32();
				for (let i = 0; i < typesCount; ++i) {
					this.dna.types.push(this.readString());
				}
				this.align();

				let lenId = this.readChars(4); // TLEN
				for (let i = 0; i < typesCount; ++i) {
					this.dna.typesLength.push(this.read16());
				}
				this.align();

				let structId = this.readChars(4); // STRC
				let structCount = this.read32();
				for (let i = 0; i < structCount; ++i) {
					let ds = new DnaStruct();
					this.dna.structs.push(ds);
					ds.dna = this.dna;
					ds.type = this.read16();
					let fieldCount = this.read16();
					if (fieldCount > 0) {
						ds.fieldTypes = [];
						ds.fieldNames = [];
						for (let j = 0; j < fieldCount; ++j) {
							ds.fieldTypes.push(this.read16());
							ds.fieldNames.push(this.read16());
						}
					}
				}
			}
			else {
				this.pos += b.size;
			}
		}
	}

	align = () => {
		// 4 bytes aligned
		let mod = this.pos % 4;
		if (mod > 0) this.pos += 4 - mod;
	}

	read8 = (): i32 => {
		let i = this.view.getUint8(this.pos);
		this.pos += 1;
		return i;
	}

	read16 = (): i32 => {
		let i = this.view.getInt16(this.pos, this.littleEndian);
		this.pos += 2;
		return i;
	}

	read32 = (): i32 => {
		let i = this.view.getInt32(this.pos, this.littleEndian);
		this.pos += 4;
		return i;
	}

	read64 = (): any => {
		let aview: any = this.view;
		let i = aview.getBigInt64(this.pos, this.littleEndian);
		this.pos += 8;
		return i;
	}

	readf32 = (): f32 => {
		let f = this.view.getFloat32(this.pos, this.littleEndian);
		this.pos += 4;
		return f;
	}

	read8array = (len: i32): Int32Array => {
		let ar = new Int32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = this.read8();
		return ar;
	}

	read16array = (len: i32): Int32Array => {
		let ar = new Int32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = this.read16();
		return ar;
	}

	read32array = (len: i32): Int32Array => {
		let ar = new Int32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = this.read32();
		return ar;
	}

	readf32array = (len: i32): Float32Array => {
		let ar = new Float32Array(len);
		for (let i = 0; i < len; ++i) ar[i] = this.readf32();
		return ar;
	}

	readString = (): string => {
		let s = "";
		while (true) {
			let ch = this.read8();
			if (ch == 0) break;
			s += String.fromCharCode(ch);
		}
		return s;
	}

	readChars = (len: i32): string => {
		let s = "";
		for (let i = 0; i < len; ++i) s += this.readChar();
		return s;
	}

	readChar = (): string => {
		return String.fromCharCode(this.read8());
	}

	readPointer = (): any => {
		return this.pointerSize == 4 ? this.read32() : this.read64();
	}
}

class Block {
	blend: ParserBlend;
	code: string;
	size: i32;
	sdnaIndex: i32;
	count: i32;
	pos: i32; // Byte pos of data start in blob
	constructor() {}
}

class Dna {
	names: string[] = [];
	types: string[] = [];
	typesLength: i32[] = [];
	structs: DnaStruct[] = [];
	constructor() {}
}

class DnaStruct {
	dna: Dna;
	type: i32; // Index in dna.types
	fieldTypes: i32[]; // Index in dna.types
	fieldNames: i32[]; // Index in dna.names
	constructor() {}
}

class BlHandle {
	block: Block;
	offset: i32 = 0; // Block data bytes offset
	ds: DnaStruct;

	constructor() {}

	getSize = (index: i32): i32 => {
		let nameIndex = this.ds.fieldNames[index];
		let typeIndex = this.ds.fieldTypes[index];
		let dna = this.ds.dna;
		let n = dna.names[nameIndex];
		let size = 0;
		if (n.indexOf("*") >= 0) size = this.block.blend.pointerSize;
		else size = dna.typesLength[typeIndex];
		if (n.indexOf("[") > 0) size *= this.getArrayLen(n);
		return size;
	}

	baseName = (s: string): string => {
		while (s.charAt(0) == "*") s = s.substring(1, s.length);
		if (s.charAt(s.length - 1) == "]") s = s.substring(0, s.indexOf("["));
		return s;
	}

	getArrayLen = (s: string): i32 => {
		return parseInt(s.substring(s.indexOf("[") + 1, s.indexOf("]")));
	}

	get = (name: string, index = 0, asType: string = null, arrayLen = 0): any => {
		// Return raw type or structure
		let dna = this.ds.dna;
		for (let i = 0; i < this.ds.fieldNames.length; ++i) {
			let nameIndex = this.ds.fieldNames[i];
			let dnaName = dna.names[nameIndex];
			if (name == this.baseName(dnaName)) {
				let typeIndex = this.ds.fieldTypes[i];
				let type = dna.types[typeIndex];
				let newOffset = this.offset;
				for (let j = 0; j < i; ++j) newOffset += this.getSize(j);
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
					let blend = this.block.blend;
					blend.pos = this.block.pos + newOffset;
					let isArray = dnaName.charAt(dnaName.length - 1) == "]";
					let len = isArray ? (arrayLen > 0 ? arrayLen : this.getArrayLen(dnaName)) : 1;
					switch (type) {
						case "int": return isArray ? blend.read32array(len) : blend.read32();
						case "char": return isArray ? blend.readString() : blend.read8();
						case "uchar": return isArray ? blend.read8array(len) : blend.read8();
						case "short": return isArray ? blend.read16array(len) : blend.read16();
						case "ushort": return isArray ? blend.read16array(len) : blend.read16();
						case "float": return isArray ? blend.readf32array(len) : blend.readf32();
						case "double": return 0; //blend.readf64();
						case "long": return isArray ? blend.read32array(len) : blend.read32();
						case "ulong": return isArray ? blend.read32array(len) : blend.read32();
						case "int64_t": return blend.read64();
						case "uint64_t": return blend.read64();
						case "void": if (dnaName.charAt(0) == "*") { return blend.read64(); };
					}
				}
				// Structure
				let h = new BlHandle();
				h.ds = ParserBlend.getStruct(dna, typeIndex);
				let isPointer = dnaName.charAt(0) == "*";
				if (isPointer) {
					this.block.blend.pos = this.block.pos + newOffset;
					let addr = this.block.blend.readPointer();
					if (this.block.blend.map.has(addr)) {
						h.block = this.block.blend.map.get(addr);
					}
					else h.block = this.block;
					h.offset = 0;
				}
				else {
					h.block = this.block;
					h.offset = newOffset;
				}
				h.offset += dna.typesLength[typeIndex] * index;
				return h;
			}
		}
		return null;
	}
}
