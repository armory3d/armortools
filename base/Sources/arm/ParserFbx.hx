// Adapted fbx parser originally developed by Nicolas Cannasse
// https://github.com/HeapsIO/heaps/tree/master/hxd/fmt/fbx
// The MIT License (MIT)

// Copyright (c) 2013 Nicolas Cannasse

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package arm;

import iron.Mat4;
import iron.Vec4;
import iron.Quat;

class ParserFbx {

	public var posa: js.lib.Int16Array = null;
	public var nora: js.lib.Int16Array = null;
	public var texa: js.lib.Int16Array = null;
	public var cola: js.lib.Int16Array = null;
	public var inda: js.lib.Uint32Array = null;
	public var scalePos = 1.0;
	public var scaleTex = 1.0;
	public var name = "";

	public static var parseTransform = false;
	public static var parseVCols = false;

	// Transform
	public var tx = 0.0;
	public var ty = 0.0;
	public var tz = 0.0;
	public var rx = 0.0;
	public var ry = 0.0;
	public var rz = 0.0;
	public var sx = 1.0;
	public var sy = 1.0;
	public var sz = 1.0;

	var geoms: Array<Geometry>;
	var current = 0;
	var binary = true;

	public function new(buffer: js.lib.ArrayBuffer) {
		var view = new js.lib.DataView(buffer);
		var magic = "Kaydara FBX Binary\x20\x20\x00\x1a\x00";
		var s = "";
		for (i in 0...magic.length) s += String.fromCharCode(view.getUint8(i));
		binary = s == magic;

		var fbx = binary ? FbxBinaryParser.parse(view) : Parser.parse(untyped String.fromCharCode.apply(null, new js.lib.Uint8Array(buffer)));
		var lib = new FbxLibrary();
		try {
			lib.load(fbx);
		}
		catch (e: Dynamic) {
			Krom.log(e);
		}

		geoms = lib.getAllGeometries();
		next();
	}

	public function next(): Bool {
		if (current >= geoms.length) return false;
		var geom = geoms[current];
		var lib = geom.lib;

		tx = ty = tz = 0;
		rx = ry = rz = 0;
		sx = sy = sz = 1;
		if (parseTransform) {
			var connects = lib.invConnect.get(FbxTools.getId(geom.getRoot()));
			for (c in connects) {
				var node = lib.ids.get(c);
				for (p in FbxTools.getAll(node, "Properties70.P")) {
					switch (FbxTools.toString(p.props[0])) {
						case "Lcl Translation": {
							tx = FbxTools.toFloat(p.props[4]);
							ty = FbxTools.toFloat(p.props[5]);
							tz = FbxTools.toFloat(p.props[6]);
						}
						case "Lcl Rotation": {
							rx = FbxTools.toFloat(p.props[4]) * Math.PI / 180;
							ry = FbxTools.toFloat(p.props[5]) * Math.PI / 180;
							rz = FbxTools.toFloat(p.props[6]) * Math.PI / 180;
						}
						case "Lcl Scaling": {
							sx = FbxTools.toFloat(p.props[4]);
							sy = FbxTools.toFloat(p.props[5]);
							sz = FbxTools.toFloat(p.props[6]);
						}
					}
				}
			}
		}

		var res = geom.getBuffers(binary, this);
		scalePos = res.scalePos;
		posa = res.posa;
		nora = res.nora;
		texa = res.texa;
		cola = res.cola;
		inda = res.inda;
		name = FbxTools.getName(geom.getRoot());
		if (name.charCodeAt(0) == 0) name = name.substring(1); // null
		if (name.charCodeAt(0) == 1) name = name.substring(1); // start of heading
		if (name == "Geometry") name = "Object -Geometry";
		name = name.substring(0, name.length - 10); // -Geometry

		current++;
		return true;
	}
}

class FbxBinaryParser {

	var pos: Int;
	var view: js.lib.DataView;

	var version: Int;
	var is64: Bool;
	var root: FbxNode;

	function new(view: js.lib.DataView) {
		this.view = view;
		pos = 0;
		var magic = "Kaydara FBX Binary\x20\x20\x00\x1a\x00";
		var valid = readChars(magic.length) == magic;
		if (!valid) return;
		var version = read32();
		is64 = version >= 7500;
		root = {
			name: "Root",
			props: [PInt(0), PString("Root"), PString("Root")],
			childs: parseNodes()
		};
	}

	public static function parse(view: js.lib.DataView): FbxNode {
		return new FbxBinaryParser(view).root;
	}

	function parseArray(readVal: Void->Dynamic, isFloat = false): FbxProp {
		var len = read32();
		var encoding = read32();
		var compressedLen = read32();
		var endPos = pos + compressedLen;
		var _view = view;
		if (encoding != 0) {
			pos += 2;
			var input = view.buffer.slice(pos, pos + compressedLen);
			view = new js.lib.DataView(Krom.inflate(input, true));
			pos = 0;
		}
		var res = isFloat ? parseArrayf(readVal, len) : parseArrayi(readVal, len);
		if (encoding != 0) {
			pos = endPos;
			view = _view;
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
		var i = view.getUint8(pos);
		pos += 1;
		return i;
	}

	function read16(): Int {
		var i = view.getInt16(pos, true);
		pos += 2;
		return i;
	}

	function read32(): Int {
		var i = view.getInt32(pos, true);
		pos += 4;
		return i;
	}

	function read64(): Int {
		var i = untyped view.getBigInt64(pos, true);
		pos += 8;
		return i;
	}

	function readf32(): Float {
		var f = view.getFloat32(pos, true);
		pos += 4;
		return f;
	}

	function readf64(): Float {
		var f = view.getFloat64(pos, true);
		pos += 8;
		return f;
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

	function readBytes(len: Int): js.lib.DataView {
		var v = new js.lib.DataView(view.buffer, pos, len);
		pos += len;
		return v;
	}
}

enum FbxProp {
	PInt(v: Int);
	PFloat(v: Float);
	PString(v: String);
	PIdent(i: String);
	PInts(v: Array<Int>);
	PFloats(v: Array<Float>);
}

typedef FbxNode = {
	var name: String;
	var props: Array<FbxProp>;
	var childs: Array<FbxNode>;
}

class FbxTools {

	public static function get(n: FbxNode, path: String, opt = false) {
		var parts = path.split(".");
		var cur = n;
		for (p in parts) {
			var found = false;
			for (c in cur.childs) {
				if (c.name == p) {
					cur = c;
					found = true;
					break;
				}
			}
			if (!found) {
				if (opt) return null;
				throw n.name + " does not have " + path+" ("+p+" not found)";
			}
		}
		return cur;
	}

	public static function getAll(n: FbxNode, path: String) {
		var parts = path.split(".");
		var cur = [n];
		for (p in parts) {
			var out = [];
			for (n in cur) {
				for (c in n.childs) {
					if (c.name == p) {
						out.push(c);
					}
				}
			}
			cur = out;
			if (cur.length == 0) return cur;
		}
		return cur;
	}

	public static function getInts(n: FbxNode) {
		if (n.props.length != 1) {
			throw n.name + " has " + n.props + " props";
		}
		switch (n.props[0]) {
			case PInts(v):
				return v;
			default:
				throw n.name + " has " + n.props + " props";
		}
	}

	public static function getFloats(n: FbxNode) {
		if (n.props.length != 1) {
			throw n.name + " has " + n.props + " props";
		}
		switch(n.props[0]) {
			case PFloats(v):
				return v;
			case PInts(i):
				var fl = new Array<Float>();
				for(x in i) fl.push(x);
				n.props[0] = PFloats(fl); // keep data synchronized
				// this is necessary for merging geometries since we are pushing directly into the
				// float buffer
				return fl;
			default:
				throw n.name + " has " + n.props + " props";
		}
	}

	public static function hasProp(n: FbxNode, p: FbxProp) {
		for (p2 in n.props) {
			if (Type.enumEq(p, p2)) {
				return true;
			}
		}
		return false;
	}

	static inline function idToInt(f: Float) {
		return Std.int(f);
	}

	public static function toInt(n: FbxProp) {
		if (n == null) throw "null prop";
		return switch(n) {
			case PInt(v): v;
			case PFloat(f): idToInt(f);
			default: throw "Invalid prop " + n;
		}
	}

	public static function toFloat(n: FbxProp) {
		if (n == null) throw "null prop";
		return switch(n) {
			case PInt(v): v * 1.0;
			case PFloat(v): v;
			default: throw "Invalid prop " + n;
		}
	}

	public static function toString(n: FbxProp) {
		if (n == null) throw "null prop";
		return switch(n) {
			case PString(v): v;
			default: throw "Invalid prop " + n;
		}
	}

	public static function getId(n: FbxNode) {
		if (n.props.length != 3) throw n.name + " is not an object";
		return switch(n.props[0]) {
			case PInt(id): id;
			case PFloat(id): idToInt(id);
			default: throw n.name + " is not an object " + n.props;
		}
	}

	public static function getName(n: FbxNode) {
		if (n.props.length != 3) throw n.name + " is not an object";
		return switch(n.props[1]) {
			case PString(n): n.split("::").pop();
			default: throw n.name + " is not an object";
		}
	}

	public static function getType(n: FbxNode) {
		if (n.props.length != 3) throw n.name + " is not an object";
		return switch(n.props[2]) {
			case PString(n): n;
			default: throw n.name + " is not an object";
		}
	}
}

private enum Token {
	TIdent(s: String);
	TNode(s: String);
	TInt(s: String);
	TFloat(s: String);
	TString(s: String);
	TLength(v: Int);
	TBraceOpen;
	TBraceClose;
	TColon;
	TEof;
}

class Parser {

	var line: Int;
	var buf: String;
	var pos: Int;
	var token: Null<Token>;

	function new() {}

	function parseText(str): FbxNode {
		this.buf = str;
		this.pos = 0;
		this.line = 1;
		token = null;
		return {
			name: "Root",
			props: [PInt(0),PString("Root"),PString("Root")],
			childs: parseNodes(),
		};
	}

	function parseNodes() {
		var nodes = [];
		while (true) {
			switch(peek()) {
				case TEof, TBraceClose:
					return nodes;
				default:
			}
			nodes.push(parseNode());
		}
		return nodes;
	}

	function parseNode(): FbxNode {
		var t = next();
		var name = switch(t) {
			case TNode(n): n;
			default: unexpected(t);
		};
		var props = [], childs = null;
		while (true) {
			t = next();
			switch(t) {
				case TFloat(s):
					props.push(PFloat(Std.parseFloat(s)));
				case TInt(s):
					props.push(PInt(Std.parseInt(s)));
				case TString(s):
					props.push(PString(s));
				case TIdent(s):
					props.push(PIdent(s));
				case TBraceOpen, TBraceClose, TNode(_):
					token = t;
				case TLength(v):
					except(TBraceOpen);
					except(TNode("a"));
					var ints: Array<Int> = [];
					var floats: Array<Float> = null;
					var i = 0;
					while (i < v) {
						t = next();
						switch(t) {
							case TColon:
								continue;
							case TInt(s):
								i++;
								if (floats == null) {
									ints.push(Std.parseInt(s));
								}
								else {
									floats.push(Std.parseInt(s));
								}
							case TFloat(s):
								i++;
								if (floats == null) {
									floats = [];
									for(i in ints) {
										floats.push(i);
									}
									ints = null;
								}
								floats.push(Std.parseFloat(s));
							default:
								unexpected(t);
						}
					}
					props.push(floats == null ? PInts(ints) : PFloats(floats));
					except(TBraceClose);
					break;
				default:
					unexpected(t);
			}
			t = next();
			switch(t) {
				case TNode(_), TBraceClose:
					token = t; // next
					break;
				case TColon:
					// next prop
				case TBraceOpen:
					childs = parseNodes();
					except(TBraceClose);
					break;
				default:
					unexpected(t);
			}
		}
		if (childs == null) childs = [];
		return { name: name, props: props, childs: childs };
	}

	function except(except: Token) {
		var t = next();
		if (!Type.enumEq(t, except)) {
			error("Unexpected '" + tokenStr(t) + "' (" + tokenStr(except) + " expected)");
		}
	}

	function peek() {
		if (token == null) {
			token = nextToken();
		}
		return token;
	}

	function next() {
		if (token == null) {
			return nextToken();
		}
		var tmp = token;
		token = null;
		return tmp;
	}

	function error(msg: String): Dynamic {
		throw msg + " (line " + line + ")";
		return null;
	}

	function unexpected(t: Token): Dynamic {
		return error("Unexpected " + tokenStr(t));
	}

	function tokenStr(t: Token) {
		return switch(t) {
			case TEof: "<eof>";
			case TBraceOpen: '{';
			case TBraceClose: '}';
			case TIdent(i): i;
			case TNode(i): i+":";
			case TFloat(f): f;
			case TInt(i): i;
			case TString(s): '"' + s + '"';
			case TColon: ',';
			case TLength(l): '*' + l;
		};
	}

	inline function nextChar() {
		return StringTools.fastCodeAt(buf, pos++);
	}

	inline function getBuf(pos: Int, len: Int) {
		return buf.substr(pos, len);
	}

	inline function isIdentChar(c) {
		return (c >= 'a'.code && c <= 'z'.code) || (c >= 'A'.code && c <= 'Z'.code) || (c >= '0'.code && c <= '9'.code) || c == '_'.code || c == '-'.code;
	}

	@:noDebug
	function nextToken() {
		var start = pos;
		while (true) {
			var c = nextChar();
			switch(c) {
				case ' '.code, '\r'.code, '\t'.code:
					start++;
				case '\n'.code:
					line++;
					start++;
				case ';'.code:
					while (true) {
						var c = nextChar();
						if (StringTools.isEof(c) || c == '\n'.code) {
							pos--;
							break;
						}
					}
					start = pos;
				case ','.code:
					return TColon;
				case '{'.code:
					return TBraceOpen;
				case '}'.code:
					return TBraceClose;
				case '"'.code:
					start = pos;
					while (true) {
						c = nextChar();
						if (c == '"'.code) {
							break;
						}
						if (StringTools.isEof(c) || c == '\n'.code) {
							error("Unclosed string");
						}
					}
					return TString(getBuf(start, pos - start - 1));
				case '*'.code:
					start = pos;
					do {
						c = nextChar();
					} while (c >= '0'.code && c <= '9'.code);
					pos--;
					return TLength(Std.parseInt(getBuf(start, pos - start)));
				default:
					if ((c >= 'a'.code && c <= 'z'.code) || (c >= 'A'.code && c <= 'Z'.code) || c == '_'.code) {
						do {
							c = nextChar();
						} while (isIdentChar(c));
						if (c == ':'.code) {
							return TNode(getBuf(start, pos - start - 1));
						}
						pos--;
						return TIdent(getBuf(start, pos - start));
					}
					if ((c >= '0'.code && c <= '9'.code) || c == '-'.code) {
						do {
							c = nextChar();
						} while (c >= '0'.code && c <= '9'.code);
						if (c != '.'.code && c != 'E'.code && c != 'e'.code && pos - start < 10) {
							pos--;
							return TInt(getBuf(start, pos - start));
						}
						if (c == '.'.code) {
							do {
								c = nextChar();
							} while (c >= '0'.code && c <= '9'.code);
						}
						if (c == 'e'.code || c == 'E'.code) {
							c = nextChar();
							if (c != '-'.code && c != '+'.code)
								pos--;
							do {
								c = nextChar();
							} while (c >= '0'.code && c <= '9'.code);
						}
						pos--;
						return TFloat(getBuf(start, pos - start));
					}
					if (StringTools.isEof(c)) {
						pos--;
						return TEof;
					}
					error("Unexpected char '" + String.fromCharCode(c) + "'");
			}
		}
	}

	public static function parse(text: String) {
		return new Parser().parseText(text);
	}
}

class FbxLibrary {

	public var root: FbxNode;
	public var ids: Map<Int,FbxNode>;
	public var connect: Map<Int,Array<Int>>;
	public var namedConnect: Map<Int,Map<String,Int>>;
	public var invConnect: Map<Int,Array<Int>>;
	// var uvAnims: Map<String, Array<{ t : Float, u : Float, v : Float }>>;
	// var animationEvents: Array<{ frame : Int, data : String }>;

	/**
		The FBX version that was decoded
	**/
	public var version: Float = 0.;

	public function new() {
		root = { name: "Root", props: [], childs: [] };
		reset();
	}

	function reset() {
		ids = new Map();
		connect = new Map();
		namedConnect = new Map();
		invConnect = new Map();
	}

	public function load(root: FbxNode) {
		reset();
		this.root = root;

		version = FbxTools.toInt(FbxTools.get(root, "FBXHeaderExtension.FBXVersion").props[0]) / 1000;
		if (Std.int(version) != 7) {
			throw "FBX Version 7.x required : use FBX 2010 export";
		}

		for (c in root.childs) {
			init(c);
		}
	}

	function init(n: FbxNode) {
		switch (n.name) {
			case "Connections":
				for (c in n.childs) {
					if (c.name != "C") {
						continue;
					}
					var child = FbxTools.toInt(c.props[1]);
					var parent = FbxTools.toInt(c.props[2]);

					// Maya exports invalid references
					if (ids.get(child) == null || ids.get(parent) == null) continue;

					var name = c.props[3];

					if (name != null) {
						var name = FbxTools.toString(name);
						var nc = namedConnect.get(parent);
						if (nc == null) {
							nc = new Map();
							namedConnect.set(parent, nc);
						}
						nc.set(name, child);
						// don't register as a parent, since the target can also be the child of something else
						if (name == "LookAtProperty") continue;
					}

					var c = connect.get(parent);
					if (c == null) {
						c = [];
						connect.set(parent, c);
					}
					c.push(child);

					if (parent == 0) {
						continue;
					}

					var c = invConnect.get(child);
					if (c == null) {
						c = [];
						invConnect.set(child, c);
					}
					c.push(parent);
				}
			case "Objects":
				for (c in n.childs) {
					ids.set(FbxTools.getId(c), c);
				}
			default:
		}
	}

	public function getGeometry(name: String = "") {
		var geom = null;
		for (g in FbxTools.getAll(root, "Objects.Geometry")) {
			if (FbxTools.hasProp(g, PString("Geometry::" + name))) {
				geom = g;
				break;
			}
		}
		if (geom == null) {
			throw "Geometry " + name + " not found";
		}
		return new Geometry(this, geom);
	}

	public function getFirstGeometry() {
		var geom = FbxTools.getAll(root, "Objects.Geometry")[0];
		return new Geometry(this, geom);
	}

	public function getAllGeometries() {
		var geoms = FbxTools.getAll(root, "Objects.Geometry");
		var res: Array<Geometry> = [];
		for (g in geoms) res.push(new Geometry(this, g));
		return res;
	}
}

class Geometry {

	public var lib: FbxLibrary;
	public var root: FbxNode;

	static inline var eps = 1.0 / 32767;

	public function new(l, root) {
		this.lib = l;
		this.root = root;
	}

	public function getRoot() {
		return root;
	}

	public function getVertices() {
		return FbxTools.getFloats(FbxTools.get(root, "Vertices"));
	}

	public function getPolygons() {
		return FbxTools.getInts(FbxTools.get(root, "PolygonVertexIndex"));
	}

	/**
		Decode polygon informations into triangle indexes and vertices indexes.
		Returns vidx, which is the list of vertices indexes and iout which is the index buffer for the full vertex model
	**/
	public function getIndexes() {
		var count = 0, pos = 0;
		var index = getPolygons();
		var vout = [], iout = [];
		for (i in index) {
			count++;
			if (i < 0) {
				index[pos] = -i - 1;
				var start = pos - count + 1;
				for (n in 0...count) {
					vout.push(index[n + start]);
				}
				for (n in 0...count - 2) {
					iout.push(start + n);
					iout.push(start + count - 1);
					iout.push(start + n + 1);
				}
				index[pos] = i; // restore
				count = 0;
			}
			pos++;
		}
		return { vidx: vout, idx: iout };
	}

	public function getNormals() {
		return processVectors("LayerElementNormal", "Normals");
	}

	public function getTangents(opt = false) {
		return processVectors("LayerElementTangent", "Tangents", opt);
	}

	function processVectors(layer, name, opt = false) {
		var vect = FbxTools.get(root, layer + "." + name, opt);
		if (vect == null) return null;
		var nrm = FbxTools.getFloats(vect);
		// if by-vertice (Maya in some cases, unless maybe "Split per-Vertex Normals" is checked)
		// let's reindex based on polygon indexes
		if (FbxTools.toString(FbxTools.get(root, layer+".MappingInformationType").props[0]) == "ByVertice") {
			var nout = [];
			for (i in getPolygons()) {
				var vid = i;
				if (vid < 0) vid = -vid - 1;
				nout.push(nrm[vid * 3]);
				nout.push(nrm[vid * 3 + 1]);
				nout.push(nrm[vid * 3 + 2]);
			}
			nrm = nout;
		}
		return nrm;
	}

	public function getColors() {
		var color = FbxTools.get(root, "LayerElementColor",true);
		return color == null ? null : { values: FbxTools.getFloats(FbxTools.get(color, "Colors")), index: FbxTools.getInts(FbxTools.get(color, "ColorIndex")) };
	}

	public function getUVs() {
		var uvs = [];
		for (v in FbxTools.getAll(root, "LayerElementUV")) {
			var index = FbxTools.get(v, "UVIndex", true);
			var values = FbxTools.getFloats(FbxTools.get(v, "UV"));
			var index = if (index == null) {
				// ByVertice/Direct (Maya sometimes...)
				[for (i in getPolygons()) if (i < 0) -i - 1 else i];
			}
			else FbxTools.getInts(index);
			uvs.push({ values: values, index: index });
		}
		return uvs;
	}

	public function getBuffers(binary: Bool, p: ParserFbx) {
		// triangulize indexes :
		// format is  A,B,...,-X : negative values mark the end of the polygon
		var pbuf = getVertices();
		var nbuf = getNormals();
		var tbuf = getUVs()[0];
		var cbuf = ParserFbx.parseVCols ? getColors() : null;
		var polys = getPolygons();

		if (ParserFbx.parseTransform) {
			var m = Mat4.identity();
			var v = new Vec4(p.tx, p.ty, p.tz);
			var q = new Quat();
			q.fromEuler(p.rx, p.ry, p.rz);
			var sc = new Vec4(p.sx, p.sy, p.sz);
			m.compose(v, q, sc);

			for (i in 0...Std.int(pbuf.length / 3)) {
				v.set(pbuf[i * 3], pbuf[i * 3 + 1], pbuf[i * 3 + 2]);
				v.applymat(m);
				pbuf[i * 3    ] = v.x;
				pbuf[i * 3 + 1] = v.y;
				pbuf[i * 3 + 2] = v.z;
			}
			for (i in 0...Std.int(nbuf.length / 3)) {
				v.set(nbuf[i * 3], nbuf[i * 3 + 1], nbuf[i * 3 + 2]);
				v.applyQuat(q);
				nbuf[i * 3    ] = v.x;
				nbuf[i * 3 + 1] = v.y;
				nbuf[i * 3 + 2] = v.z;
			}
		}

		// Pack positions to (-1, 1) range
		var scalePos = 0.0;
		for (p in pbuf) {
			var f = Math.abs(p);
			if (scalePos < f) scalePos = f;
		}
		var inv = 32767 * (1 / scalePos);

		var pos = 0;
		var count = 0;
		var vlen = 0;
		var ilen = 0;
		for (i in polys) {
			count++;
			if (i < 0) {
				for (n in 0...count) vlen++;
				for (n in 0...count - 2) ilen += 3;
				count = 0;
			}
			pos++;
		}

		// Pack into 16bit
		var posa = new js.lib.Int16Array(vlen * 4);
		var nora = new js.lib.Int16Array(vlen * 2);
		var texa = tbuf != null ? new js.lib.Int16Array(vlen * 2) : null;
		var cola = cbuf != null ? new js.lib.Int16Array(vlen * 3) : null;
		var inda = new js.lib.Uint32Array(ilen);

		pos = 0;
		count = 0;
		vlen = 0;
		ilen = 0;
		for (i in polys) {
			count++;
			if (i < 0) {
				polys[pos] = -i - 1;
				var start = pos - count + 1;
				for (n in 0...count) {
					var k = n + start;
					var vidx = polys[k];
					posa[vlen * 4    ] = Std.int(pbuf[vidx * 3    ] * inv);
					posa[vlen * 4 + 1] = Std.int(pbuf[vidx * 3 + 1] * inv);
					posa[vlen * 4 + 2] = Std.int(pbuf[vidx * 3 + 2] * inv);
					posa[vlen * 4 + 3] = Std.int(nbuf[k * 3 + 2] * 32767);
					nora[vlen * 2    ] = Std.int(nbuf[k * 3    ] * 32767);
					nora[vlen * 2 + 1] = Std.int(nbuf[k * 3 + 1] * 32767);
					if (tbuf != null) {
						var iuv = tbuf.index[k];
						var uvx = tbuf.values[iuv * 2];
						if (uvx > 1.0 + eps) uvx = uvx - Std.int(uvx);
						var uvy = tbuf.values[iuv * 2 + 1];
						if (uvy > 1.0 + eps) uvy = uvy - Std.int(uvy);
						texa[vlen * 2    ] = Std.int(       uvx  * 32767);
						texa[vlen * 2 + 1] = Std.int((1.0 - uvy) * 32767);
					}
					if (cbuf != null) {
						var icol = cbuf.index[k];
						cola[vlen * 3    ] = Std.int(cbuf.values[icol * 4    ] * 32767);
						cola[vlen * 3 + 1] = Std.int(cbuf.values[icol * 4 + 1] * 32767);
						cola[vlen * 3 + 2] = Std.int(cbuf.values[icol * 4 + 2] * 32767);
						// cola[vlen * 4 + 3] = Std.int(cbuf.values[icol * 4 + 3] * 32767);
					}
					vlen++;
				}
				if (count <= 4) { // Convex, fan triangulation
					for (n in 0...count - 2) {
						inda[ilen + 2] = start + n;
						inda[ilen + 1] = start + count - 1;
						inda[ilen    ] = start + n + 1;
						ilen += 3;
					}
				}
				else { // Convex or concave, ear clipping
					var va: Array<Int> = [];
					for (i in 0...count) va.push(start + i);
					var nx = nbuf[start * 3    ];
					var ny = nbuf[start * 3 + 1];
					var nz = nbuf[start * 3 + 2];
					var nxabs = Math.abs(nx);
					var nyabs = Math.abs(ny);
					var nzabs = Math.abs(nz);
					var flip = nx + ny + nz > 0;
					var axis = nxabs > nyabs && nxabs > nzabs ? 0 : nyabs > nxabs && nyabs > nzabs ? 1 : 2;
					var axis0 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
					var axis1 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

					var vi = count;
					var loops = 0;
					var i = -1;
					while (vi > 3 && loops++ < vi) {
						i = (i + 1) % vi;
						var i1 = (i + 1) % vi;
						var i2 = (i + 2) % vi;
						var vi0 = polys[va[i ]] * 3;
						var vi1 = polys[va[i1]] * 3;
						var vi2 = polys[va[i2]] * 3;
						var v0x = pbuf[vi0 + axis0];
						var v0y = pbuf[vi0 + axis1];
						var v1x = pbuf[vi1 + axis0];
						var v1y = pbuf[vi1 + axis1];
						var v2x = pbuf[vi2 + axis0];
						var v2y = pbuf[vi2 + axis1];

						var e0x = v0x - v1x; // Not an interior vertex
						var e0y = v0y - v1y;
						var e1x = v2x - v1x;
						var e1y = v2y - v1y;
						var cross = e0x * e1y - e0y * e1x;
						if (cross <= 0) continue;

						var overlap = false; // Other vertex found inside this triangle
						for (j in 0...vi - 3) {
							var j0 = polys[va[(i + 3 + j) % vi]] * 3;
							var px = pbuf[j0 + axis0];
							var py = pbuf[j0 + axis1];
							if (UtilMesh.pnpoly(v0x, v0y, v1x, v1y, v2x, v2y, px, py)) {
								overlap = true;
								break;
							}
						}
						if (overlap) continue;

						inda[ilen++] = va[i ]; // Found ear
						inda[ilen++] = va[i1];
						inda[ilen++] = va[i2];

						for (j in ((i + 1) % vi)...vi - 1) { // Consume vertex
							va[j] = va[j + 1];
						}
						vi--;
						i--;
						loops = 0;
					}
					inda[ilen++] = va[0]; // Last one
					inda[ilen++] = va[1];
					inda[ilen++] = va[2];
				}
				polys[pos] = i; // restore
				count = 0;
			}
			pos++;
		}

		return { posa: posa, nora: nora, texa: texa, cola: cola, inda: inda, scalePos: scalePos };
	}
}

