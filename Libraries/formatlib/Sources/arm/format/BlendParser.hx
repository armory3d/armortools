// .blend file parser
// https://github.com/armory3d/blend
// Reference:
// https://github.com/fschutt/mystery-of-the-blend-backup
// https://web.archive.org/web/20170630054951/http://www.atmind.nl/blender/mystery_ot_blend.html
// Usage:
// var bl = new BlendParser(blob:kha.Blob);
// trace(bl.dir("Scene"));
// var scenes = bl.get("Scene");
// trace(scenes[0].get("id").get("name"));
package arm.format;

// https://github.com/Kode/Kha
import kha.Blob;

class BlendParser {

	public var pos: Int;
	var blob: Blob;

	// Header
	public var version: String;
	public var pointerSize: Int;
	public var littleEndian: Bool;
	// Data
	public var blocks: Array<Block> = [];
	public var dna: Dna = null;
	public var map = new Map<Int, Map<Int, Block>>(); // Map blocks by memory address

	public function new(blob: Blob) {
		this.blob = blob;
		this.pos = 0;
		if (readChars(7) != "BLENDER") {
			this.blob = Blob.fromBytes(haxe.io.Bytes.ofData(Krom.inflate(blob.toBytes().getData(), false)));
			this.pos = 0;
			if (readChars(7) != "BLENDER") return;
		}
		parse();
	}

	public function dir(type: String): Array<String> {
		// Return structure fields
		var typeIndex = getTypeIndex(dna, type);
		if (typeIndex == -1) return null;
		var ds = getStruct(dna, typeIndex);
		var fields: Array<String> = [];
		for (i in 0...ds.fieldNames.length) {
			var nameIndex = ds.fieldNames[i];
			var typeIndex = ds.fieldTypes[i];
			fields.push(dna.types[typeIndex] + " " + dna.names[nameIndex]);
		}
		return fields;
	}

	public function get(type: String): Array<Handle> {
		if (dna == null) return null;
		// Return all structures of type
		var typeIndex = getTypeIndex(dna, type);
		if (typeIndex == -1) return null;
		var ds = getStruct(dna, typeIndex);
		var handles: Array<Handle> = [];
		for (b in blocks) {
			if (dna.structs[b.sdnaIndex].type == typeIndex) {
				var h = new Handle();
				handles.push(h);
				h.block = b;
				h.ds = ds;
			}
		}
		return handles;
	}

	public static function getStruct(dna: Dna, typeIndex: Int): DnaStruct {
		for (ds in dna.structs) if (ds.type == typeIndex) return ds;
		return null;
	}

	public static function getTypeIndex(dna: Dna, type: String): Int {
		for (i in 0...dna.types.length) if (type == dna.types[i]) { return i; }
		return -1;
	}

	function parse() {
		// Pointer size: _ 32bit, - 64bit
		pointerSize = readChar() == "_" ? 4 : 8;

		// v - little endian, V - big endian
		littleEndian = readChar() == "v";
		if (littleEndian) {
			read16 = read16LE;
			read32 = read32LE;
			read64 = read64LE;
			readf32 = readf32LE;
		}
		else {
			read16 = read16BE;
			read32 = read32BE;
			read64 = read64BE;
			readf32 = readf32BE;
		}

		version = readChars(3);

		// Reading file blocks
		// Header - data
		while (pos < blob.length) {
			align();
			var b = new Block();

			// Block type
			b.code = readChars(4);
			if (b.code == "ENDB") break;

			blocks.push(b);
			b.blend = this;

			// Total block length
			b.size = read32();

			// Memory address
			var addr = readPointer();
			if (!map.exists(addr.high)) map.set(addr.high, new Map<Int, Block>());
			map.get(addr.high).set(addr.low, b);

			// Index of dna struct contained in this block
			b.sdnaIndex = read32();

			// Number of dna structs in this block
			b.count = read32();

			b.pos = pos;

			// This block stores dna structures
			if (b.code == "DNA1") {
				dna = new Dna();

				var id = readChars(4); // SDNA
				var nameId = readChars(4); // NAME
				var namesCount = read32();
				for (i in 0...namesCount) {
					dna.names.push(readString());
				}
				align();

				var typeId = readChars(4); // TYPE
				var typesCount = read32();
				for (i in 0...typesCount) {
					dna.types.push(readString());
				}
				align();

				var lenId = readChars(4); // TLEN
				for (i in 0...typesCount) {
					dna.typesLength.push(read16());
				}
				align();

				var structId = readChars(4); // STRC
				var structCount = read32();
				for (i in 0...structCount) {
					var ds = new DnaStruct();
					dna.structs.push(ds);
					ds.dna = dna;
					ds.type = read16();
					var fieldCount = read16();
					if (fieldCount > 0) {
						ds.fieldTypes = [];
						ds.fieldNames = [];
						for (j in 0...fieldCount) {
							ds.fieldTypes.push(read16());
							ds.fieldNames.push(read16());
						}
					}
				}
			}
			else {
				pos += b.size;
			}
		}
	}

	function align() {
		// 4 bytes aligned
		var mod = pos % 4;
		if (mod > 0) pos += 4 - mod;
	}

	public function read8(): Int {
		var i = blob.readU8(pos);
		pos += 1;
		return i;
	}

	public var read16: Void->Int;
	public var read32: Void->Int;
	public var read64: Void->haxe.Int64;
	public var readf32: Void->Float;

	function read16LE(): Int {
		var i = blob.readS16LE(pos);
		pos += 2;
		return i;
	}

	function read32LE(): Int {
		var i = blob.readS32LE(pos);
		pos += 4;
		return i;
	}

	function read64LE(): haxe.Int64 {
		return haxe.Int64.make(read32(), read32());
	}

	function readf32LE(): Float {
		var f = blob.readF32LE(pos);
		pos += 4;
		return f;
	}

	function read16BE(): Int {
		var i = blob.readS16BE(pos);
		pos += 2;
		return i;
	}

	function read32BE(): Int {
		var i = blob.readS32BE(pos);
		pos += 4;
		return i;
	}

	function read64BE(): haxe.Int64 {
		return haxe.Int64.make(read32(), read32());
	}

	function readf32BE(): Float {
		var f = blob.readF32BE(pos);
		pos += 4;
		return f;
	}

	public function read8array(len: Int): haxe.io.Int32Array {
		var ar = new haxe.io.Int32Array(len);
		for (i in 0...len) ar[i] = read8();
		return ar;
	}

	public function read16array(len: Int): haxe.io.Int32Array {
		var ar = new haxe.io.Int32Array(len);
		for (i in 0...len) ar[i] = read16();
		return ar;
	}

	public function read32array(len: Int): haxe.io.Int32Array {
		var ar = new haxe.io.Int32Array(len);
		for (i in 0...len) ar[i] = read32();
		return ar;
	}

	public function readf32array(len: Int): kha.arrays.Float32Array {
		var ar = new kha.arrays.Float32Array(len);
		for (i in 0...len) ar[i] = readf32();
		return ar;
	}

	public function readString(): String {
		var s = "";
		while (true) {
			var ch = read8();
			if (ch == 0) break;
			s += String.fromCharCode(ch);
		}
		return s;
	}

	public function readChars(len: Int): String {
		var s = "";
		for (i in 0...len) s += readChar();
		return s;
	}

	public function readChar(): String {
		return String.fromCharCode(read8());
	}

	public function readPointer(): haxe.Int64 {
		return pointerSize == 4 ? haxe.Int64.ofInt(read32()) : read64();
	}
}

class Block {
	public var blend: BlendParser;
	public var code: String;
	public var size: Int;
	public var sdnaIndex: Int;
	public var count: Int;
	public var pos: Int; // Byte pos of data start in blob
	public function new() {}
}

class Dna {
	public var names: Array<String> = [];
	public var types: Array<String> = [];
	public var typesLength: Array<Int> = [];
	public var structs: Array<DnaStruct> = [];
	public function new() {}
}

class DnaStruct {
	public var dna: Dna;
	public var type: Int; // Index in dna.types
	public var fieldTypes: Array<Int>; // Index in dna.types
	public var fieldNames: Array<Int>; // Index in dna.names
	public function new() {}
}

class Handle {
	public var block: Block;
	public var offset: Int = 0; // Block data bytes offset
	public var ds: DnaStruct;
	public function new() {}
	function getSize(index: Int): Int {
		var nameIndex = ds.fieldNames[index];
		var typeIndex = ds.fieldTypes[index];
		var dna = ds.dna;
		var n = dna.names[nameIndex];
		var size = 0;
		if (n.indexOf("*") >= 0) size = block.blend.pointerSize;
		else size = dna.typesLength[typeIndex];
		if (n.indexOf("[") > 0) size *= getArrayLen(n);
		return size;
	}
	function baseName(s: String): String {
		while (s.charAt(0) == "*") s = s.substring(1, s.length);
		if (s.charAt(s.length - 1) == "]") s = s.substring(0, s.indexOf("["));
		return s;
	}
	function getArrayLen(s: String): Int {
		return Std.parseInt(s.substring(s.indexOf("[") + 1, s.indexOf("]")));
	}
	public function get(name: String, index = 0, asType: String = null, arrayLen = 0): Dynamic {
		// Return raw type or structure
		var dna = ds.dna;
		for (i in 0...ds.fieldNames.length) {
			var nameIndex = ds.fieldNames[i];
			var dnaName = dna.names[nameIndex];
			if (name == baseName(dnaName)) {
				var typeIndex = ds.fieldTypes[i];
				var type = dna.types[typeIndex];
				var newOffset = offset;
				for (j in 0...i) newOffset += getSize(j);
				// Cast void* to type
				if (asType != null) {
					for (i in 0...dna.types.length) {
						if (dna.types[i] == asType) { typeIndex = i; break; }
					}
				}
				// Raw type
				if (typeIndex < 12) {
					var blend = block.blend;
					blend.pos = block.pos + newOffset;
					var isArray = dnaName.charAt(dnaName.length - 1) == "]";
					var len = isArray ? (arrayLen > 0 ? arrayLen : getArrayLen(dnaName)) : 1;
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
					case "void": return 0;
					}
				}
				// Structure
				var h = new Handle();
				h.ds = BlendParser.getStruct(dna, typeIndex);
				var isPointer = dnaName.charAt(0) == "*";
				if (isPointer) {
					block.blend.pos = block.pos + newOffset;
					var addr = block.blend.readPointer();
					if (block.blend.map.exists(addr.high)) {
						h.block = block.blend.map.get(addr.high).get(addr.low);
					}
					else h.block = block;
					h.offset = 0;
				}
				else {
					h.block = block;
					h.offset = newOffset;
				}
				h.offset += dna.typesLength[typeIndex] * index;
				return h;
			}
		}
		return null;
	}
}
