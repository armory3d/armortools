package arm.format;

import arm.format.FbxLibrary;

class FbxBinaryParser {

	var pos: Int;
	var blob: kha.Blob;

	var version: Int;
	var is64: Bool;
	var root: FbxNode;

	function new(blob: kha.Blob) {
		this.blob = blob;
		pos = 0;
		var magic = "Kaydara FBX Binary\x20\x20\x00\x1a\x00";
		var valid = readChars(magic.length) == magic;
		if (!valid) return;
		var version = read32();
		is64 = version >= 7500;
		root = {
			name : "Root",
			props : [PInt(0), PString("Root"), PString("Root")],
			childs : parseNodes()
		};
	}

	public static function parse(blob: kha.Blob): FbxNode {
		return new FbxBinaryParser(blob).root;
	}

	function parseArray(readVal: Void->Dynamic, isFloat = false): FbxProp {
		var len = read32();
		var encoding = read32();
		var compressedLen = read32();
		var endPos = pos + compressedLen;
		var _blob = blob;
		if (encoding != 0) {
			pos += 2;
			var input = blob.sub(pos, compressedLen).toBytes().getData();
			blob = kha.Blob.fromBytes(haxe.io.Bytes.ofData(Krom.inflate(input, true)));
			pos = 0;
		}
		var res = isFloat ? parseArrayf(readVal, len) : parseArrayi(readVal, len);
		if (encoding != 0) {
			pos = endPos;
			blob = _blob;
		}
		return res;
	}

	function parseArrayf(readVal: Void->Dynamic, len: Int): FbxProp {
		var res: Array<Float> = [];
		for (i in 0...len) res.push(readVal());
		return PFloats(res);
	}

	function parseArrayi(readVal: Void->Dynamic, len: Int): FbxProp {
		var res: Array<Int> = [];
		for (i in 0...len) res.push(readVal());
		return PInts(res);
	}

	function parseProp(): FbxProp {
		switch (readChar()) {
		case "C":
			return PString(readChar());
		case "Y":
			return PInt(read16());
		case "I":
			return PInt(read32());
		case "L":
			return PInt(read64());
		case "F":
			return PFloat(readf32());
		case "D":
			return PFloat(readf64());
		case "f":
			return parseArray(readf32, true);
		case "d":
			return parseArray(readf64, true);
		case "l":
			return parseArray(read64);
		case "i":
			return parseArray(read32);
		case "b":
			return parseArray(readBool);
		case "S":
			var len = read32();
			return PString(readChars(len));
		case "R":
			var b = readBytes(read32());
			return null;
		default:
			return null;
		}
	}

	function parseNode(): FbxNode {
		var endPos = 0;
		var numProps = 0;
		var propListLen = 0;

		if (is64) {
			endPos = read64();
			numProps = read64();
			propListLen = read64();
		}
		else {
			endPos = read32();
			numProps = read32();
			propListLen = read32();
		}

		var nameLen = read8();
		var name = nameLen == 0 ? "" : readChars(nameLen);
		if (endPos == 0) return null; // Null node

		var props: Array<FbxProp> = null;
		if (numProps > 0) props = [];
		for (i in 0...numProps) props.push(parseProp());

		var childs: Array<FbxNode> = null;
		var listLen = endPos - pos;
		if (listLen > 0) {
			childs = [];
			while (true) {
				var nested = parseNode();
				nested == null ? break : childs.push(nested);
			}
		}
		return { name: name, props: props, childs: childs };
	}

	function parseNodes(): Array<FbxNode> {
		var nodes = [];
		while (true) {
			var n = parseNode();
			n == null ? break : nodes.push(n);
		}
		return nodes;
	}

	function read8(): Int {
		var i = blob.readU8(pos);
		pos += 1;
		return i;
	}

	function read16(): Int {
		var i = blob.readS16LE(pos);
		pos += 2;
		return i;
	}

	function read32(): Int {
		var i = blob.bytes.getInt32(pos);
		// var i = blob.readS32LE(pos); // Result sometimes off by 1?
		pos += 4;
		return i;
	}

	function read64(): Int {
		var i1 = read32();
		var i2 = read32();
		// return cast haxe.Int64.make(i1, i2);
		return i1;
	}

	function readf32(): Float {
		var f = blob.readF32LE(pos);
		pos += 4;
		return f;
	}

	function readf64(): Float {
		var i1 = read32();
		var i2 = read32();
		return haxe.io.FPHelper.i64ToDouble(i1, i2); // LE
	}

	function readString(): String {
		var s = "";
		while (true) {
			var ch = read8();
			if (ch == 0) break;
			s += String.fromCharCode(ch);
		}
		return s;
	}

	function readChars(len: Int): String {
		var s = "";
		for (i in 0...len) s += readChar();
		return s;
	}

	function readChar(): String {
		return String.fromCharCode(read8());
	}

	function readBool(): Int {
		return read8(); // return read8() == 1;
	}

	function readBytes(len: Int): kha.Blob {
		var b = blob.sub(pos, len);
		pos += len;
		return b;
	}
}
