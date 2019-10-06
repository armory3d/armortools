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
package arm.format;

enum FbxProp {
	PInt( v : Int );
	PFloat( v : Float );
	PString( v : String );
	PIdent( i : String );
	PInts( v : Array<Int> );
	PFloats( v : Array<Float> );
}

typedef FbxNode = {
	var name : String;
	var props : Array<FbxProp>;
	var childs : Array<FbxNode>;
}

class FbxTools {

	public static function get( n : FbxNode, path : String, opt = false ) {
		var parts = path.split(".");
		var cur = n;
		for( p in parts ) {
			var found = false;
			for( c in cur.childs )
				if( c.name == p ) {
					cur = c;
					found = true;
					break;
				}
			if( !found ) {
				if( opt )
					return null;
				throw n.name + " does not have " + path+" ("+p+" not found)";
			}
		}
		return cur;
	}

	public static function getAll( n : FbxNode, path : String ) {
		var parts = path.split(".");
		var cur = [n];
		for( p in parts ) {
			var out = [];
			for( n in cur )
				for( c in n.childs )
					if( c.name == p )
						out.push(c);
			cur = out;
			if( cur.length == 0 )
				return cur;
		}
		return cur;
	}

	public static function getInts( n : FbxNode ) {
		if( n.props.length != 1 )
			throw n.name + " has " + n.props + " props";
		switch( n.props[0] ) {
		case PInts(v):
			return v;
		default:
			throw n.name + " has " + n.props + " props";
		}
	}

	public static function getFloats( n : FbxNode ) {
		if( n.props.length != 1 )
			throw n.name + " has " + n.props + " props";
		switch( n.props[0] ) {
		case PFloats(v):
			return v;
		case PInts(i):
			var fl = new Array<Float>();
			for( x in i )
				fl.push(x);
			n.props[0] = PFloats(fl); // keep data synchronized
			// this is necessary for merging geometries since we are pushing directly into the
			// float buffer
			return fl;
		default:
			throw n.name + " has " + n.props + " props";
		}
	}

	public static function hasProp( n : FbxNode, p : FbxProp ) {
		for( p2 in n.props )
			if( Type.enumEq(p, p2) )
				return true;
		return false;
	}

	static inline function idToInt( f : Float ) {
		#if (neko || hl)
		// ids are unsigned
		f %= 4294967296.;
		if( f >= 2147483648. )
			f -= 4294967296.;
		else if( f < -2147483648. )
			f += 4294967296.;
		#end
		return Std.int(f);
	}

	public static function toInt( n : FbxProp ) {
		if( n == null ) throw "null prop";
		return switch( n ) {
		case PInt(v): v;
		case PFloat(f): idToInt(f);
		default: throw "Invalid prop " + n;
		}
	}

	public static function toFloat( n : FbxProp ) {
		if( n == null ) throw "null prop";
		return switch( n ) {
		case PInt(v): v * 1.0;
		case PFloat(v): v;
		default: throw "Invalid prop " + n;
		}
	}

	public static function toString( n : FbxProp ) {
		if( n == null ) throw "null prop";
		return switch( n ) {
		case PString(v): v;
		default: throw "Invalid prop " + n;
		}
	}

	public static function getId( n : FbxNode ) {
		if( n.props.length != 3 )
			throw n.name + " is not an object";
		return switch( n.props[0] ) {
		case PInt(id): id;
		case PFloat(id) : idToInt(id);
		default: throw n.name + " is not an object " + n.props;
		}
	}

	public static function getName( n : FbxNode ) {
		if( n.props.length != 3 )
			throw n.name + " is not an object";
		return switch( n.props[1] ) {
		case PString(n): n.split("::").pop();
		default: throw n.name + " is not an object";
		}
	}

	public static function getType( n : FbxNode ) {
		if( n.props.length != 3 )
			throw n.name + " is not an object";
		return switch( n.props[2] ) {
		case PString(n): n;
		default: throw n.name + " is not an object";
		}
	}

}

private enum Token {
	TIdent( s : String );
	TNode( s : String );
	TInt( s : String );
	TFloat( s : String );
	TString( s : String );
	TLength( v : Int );
	TBraceOpen;
	TBraceClose;
	TColon;
	TEof;
}

class Parser {

	var line : Int;
	var buf : String;
	var pos : Int;
	var token : Null<Token>;

	function new() {
	}

	function parseText( str ) : FbxNode {
		this.buf = str;
		this.pos = 0;
		this.line = 1;
		token = null;
		return {
			name : "Root",
			props : [PInt(0),PString("Root"),PString("Root")],
			childs : parseNodes(),
		};
	}

	function parseNodes() {
		var nodes = [];
		while( true ) {
			switch( peek() ) {
			case TEof, TBraceClose:
				return nodes;
			default:
			}
			nodes.push(parseNode());
		}
		return nodes;
	}

	function parseNode() : FbxNode {
		var t = next();
		var name = switch( t ) {
		case TNode(n): n;
		default: unexpected(t);
		};
		var props = [], childs = null;
		while( true ) {
			t = next();
			switch( t ) {
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
				var ints : Array<Int> = [];
				var floats : Array<Float> = null;
				var i = 0;
				while( i < v ) {
					t = next();
					switch( t ) {
					case TColon:
						continue;
					case TInt(s):
						i++;
						if( floats == null )
							ints.push(Std.parseInt(s));
						else
							floats.push(Std.parseInt(s));
					case TFloat(s):
						i++;
						if( floats == null ) {
							floats = [];
							for( i in ints )
								floats.push(i);
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
			switch( t ) {
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
		if( childs == null ) childs = [];
		return { name : name, props : props, childs : childs };
	}

	function except( except : Token ) {
		var t = next();
		if( !Type.enumEq(t, except) )
			error("Unexpected '" + tokenStr(t) + "' (" + tokenStr(except) + " expected)");
	}

	function peek() {
		if( token == null )
			token = nextToken();
		return token;
	}

	function next() {
		if( token == null )
			return nextToken();
		var tmp = token;
		token = null;
		return tmp;
	}

	function error( msg : String ) : Dynamic {
		throw msg + " (line " + line + ")";
		return null;
	}

	function unexpected( t : Token ) : Dynamic {
		return error("Unexpected "+tokenStr(t));
	}

	function tokenStr( t : Token ) {
		return switch( t ) {
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

	inline function getBuf( pos : Int, len : Int ) {
		return buf.substr(pos, len);
	}

	inline function isIdentChar(c) {
		return (c >= 'a'.code && c <= 'z'.code) || (c >= 'A'.code && c <= 'Z'.code) || (c >= '0'.code && c <= '9'.code) || c == '_'.code || c == '-'.code;
	}

	@:noDebug
	function nextToken() {
		var start = pos;
		while( true ) {
			var c = nextChar();
			switch( c ) {
			case ' '.code, '\r'.code, '\t'.code:
				start++;
			case '\n'.code:
				line++;
				start++;
			case ';'.code:
				while( true ) {
					var c = nextChar();
					if( StringTools.isEof(c) || c == '\n'.code ) {
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
				while( true ) {
					c = nextChar();
					if( c == '"'.code )
						break;
					if( StringTools.isEof(c) || c == '\n'.code )
						error("Unclosed string");
				}
				return TString(getBuf(start, pos - start - 1));
			case '*'.code:
				start = pos;
				do {
					c = nextChar();
				} while( c >= '0'.code && c <= '9'.code );
				pos--;
				return TLength(Std.parseInt(getBuf(start, pos - start)));
			default:
				if( (c >= 'a'.code && c <= 'z'.code) || (c >= 'A'.code && c <= 'Z'.code) || c == '_'.code ) {
					do {
						c = nextChar();
					} while( isIdentChar(c) );
					if( c == ':'.code )
						return TNode(getBuf(start, pos - start - 1));
					pos--;
					return TIdent(getBuf(start, pos - start));
				}
				if( (c >= '0'.code && c <= '9'.code) || c == '-'.code ) {
					do {
						c = nextChar();
					} while( c >= '0'.code && c <= '9'.code );
					if( c != '.'.code && c != 'E'.code && c != 'e'.code && pos - start < 10 ) {
						pos--;
						return TInt(getBuf(start, pos - start));
					}
					if( c == '.'.code ) {
						do {
							c = nextChar();
						} while( c >= '0'.code && c <= '9'.code );
					}
					if( c == 'e'.code || c == 'E'.code ) {
						c = nextChar();
						if( c != '-'.code && c != '+'.code )
							pos--;
						do {
							c = nextChar();
						} while( c >= '0'.code && c <= '9'.code );
					}
					pos--;
					return TFloat(getBuf(start, pos - start));
				}
				if( StringTools.isEof(c) ) {
					pos--;
					return TEof;
				}
				error("Unexpected char '" + String.fromCharCode(c) + "'");
			}
		}
	}

	public static function parse( text : String ) {
		return new Parser().parseText(text);
	}
}

class FbxLibrary {

	var root : FbxNode;
	var ids : Map<Int,FbxNode>;
	var connect : Map<Int,Array<Int>>;
	var namedConnect : Map<Int,Map<String,Int>>;
	var invConnect : Map<Int,Array<Int>>;
	// var uvAnims : Map<String, Array<{ t : Float, u : Float, v : Float }>>;
	// var animationEvents : Array<{ frame : Int, data : String }>;

	/**
		The FBX version that was decoded
	**/
	public var version : Float = 0.;

	public function new() {
		root = { name : "Root", props : [], childs : [] };
		reset();
	}

	function reset() {
		ids = new Map();
		connect = new Map();
		namedConnect = new Map();
		invConnect = new Map();
	}

	public function load( root : FbxNode ) {
		reset();
		this.root = root;

		version = FbxTools.toInt(FbxTools.get(root, "FBXHeaderExtension.FBXVersion").props[0]) / 1000;
		if( Std.int(version) != 7 )
			throw "FBX Version 7.x required : use FBX 2010 export";

		for( c in root.childs )
			init(c);

		// init properties
		// for( m in FbxTools.getAll(this.root, "Objects.Model") ) {
		// 	for( p in FbxTools.getAll(m, "Properties70.P") )
		// 		switch( FbxTools.toString(p.props[0]) ) {
		// 		case "UDP3DSMAX":
		// 			var userProps = FbxTools.toString(p.props[4]).split("&cr;&lf;");
		// 			for( p in userProps ) {
		// 				var pl = p.split("=");
		// 				var pname = StringTools.trim(pl.shift());
		// 				var pval = StringTools.trim(pl.join("="));
		// 				switch( pname ) {
		// 				case "UV" if( pval != "" ):
		// 					var xml = try Xml.parse(pval) catch( e : Dynamic ) throw "Invalid UV data in " + FbxTools.getName(m);
		// 					var frames = [for( f in new haxe.xml.Access(xml.firstElement()).elements ) { var f = f.innerData.split(" ");  { t : Std.parseFloat(f[0]) * 9622116.25, u : Std.parseFloat(f[1]), v : Std.parseFloat(f[2]) }} ];
		// 					if( uvAnims == null ) uvAnims = new Map();
		// 					uvAnims.set(FbxTools.getName(m), frames);
		// 				case "Events":
		// 					var xml = try Xml.parse(pval) catch( e : Dynamic ) throw "Invalid Events data in " + FbxTools.getName(m);
		// 					animationEvents = [for( f in new haxe.xml.Access(xml.firstElement()).elements ) { var f = f.innerData.split(" ");  { frame : Std.parseInt(f.shift()), data : StringTools.trim(f.join(" ")) }} ];
		// 				default:
		// 				}
		// 			}
		// 		default:
		// 		}
		// }
	}

	function init( n : FbxNode ) {
		switch( n.name ) {
		case "Connections":
			for( c in n.childs ) {
				if( c.name != "C" )
					continue;
				var child = FbxTools.toInt(c.props[1]);
				var parent = FbxTools.toInt(c.props[2]);

				// Maya exports invalid references
				if( ids.get(child) == null || ids.get(parent) == null ) continue;

				var name = c.props[3];

				if( name != null ) {
					var name = FbxTools.toString(name);
					var nc = namedConnect.get(parent);
					if( nc == null ) {
						nc = new Map();
						namedConnect.set(parent, nc);
					}
					nc.set(name, child);
					// don't register as a parent, since the target can also be the child of something else
					if( name == "LookAtProperty" ) continue;
				}

				var c = connect.get(parent);
				if( c == null ) {
					c = [];
					connect.set(parent, c);
				}
				c.push(child);

				if( parent == 0 )
					continue;

				var c = invConnect.get(child);
				if( c == null ) {
					c = [];
					invConnect.set(child, c);
				}
				c.push(parent);
			}
		case "Objects":
			for( c in n.childs )
				ids.set(FbxTools.getId(c), c);
		default:
		}
	}

	public function getGeometry( name : String = "" ) {
		var geom = null;
		for( g in FbxTools.getAll(root, "Objects.Geometry") )
			if( FbxTools.hasProp(g, PString("Geometry::" + name)) ) {
				geom = g;
				break;
			}
		if( geom == null )
			throw "Geometry " + name + " not found";
		return new Geometry(this, geom);
	}

	public function getFirstGeometry() {
		var geom = FbxTools.getAll(root, "Objects.Geometry")[0];
		return new Geometry(this, geom);
	}

	public function getAllGeometries() {
		var geoms = FbxTools.getAll(root, "Objects.Geometry");
		var res:Array<Geometry> = [];
		for (g in geoms) res.push(new Geometry(this, g));
		return res;
	}
}

class Geometry {

	var lib : FbxLibrary;
	var root : FbxNode;

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
		for( i in index ) {
			count++;
			if( i < 0 ) {
				index[pos] = -i - 1;
				var start = pos - count + 1;
				for( n in 0...count )
					vout.push(index[n + start]);
				for( n in 0...count - 2 ) {
					iout.push(start + n);
					iout.push(start + count - 1);
					iout.push(start + n + 1);
				}
				index[pos] = i; // restore
				count = 0;
			}
			pos++;
		}
		return { vidx : vout, idx : iout };
	}

	public function getNormals() {
		return processVectors("LayerElementNormal", "Normals");
	}

	public function getTangents( opt = false ) {
		return processVectors("LayerElementTangent", "Tangents", opt);
	}

	function processVectors( layer, name, opt = false ) {
		var vect = FbxTools.get(root, layer + "." + name, opt);
		if( vect == null ) return null;
		var nrm = FbxTools.getFloats(vect);
		// if by-vertice (Maya in some cases, unless maybe "Split per-Vertex Normals" is checked)
		// let's reindex based on polygon indexes
		if( FbxTools.toString(FbxTools.get(root, layer+".MappingInformationType").props[0]) == "ByVertice" ) {
			var nout = [];
			for( i in getPolygons() ) {
				var vid = i;
				if( vid < 0 ) vid = -vid - 1;
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
		return color == null ? null : { values : FbxTools.getFloats(FbxTools.get(color, "Colors")), index : FbxTools.getInts(FbxTools.get(color, "ColorIndex")) };
	}

	public function getUVs() {
		var uvs = [];
		for( v in FbxTools.getAll(root, "LayerElementUV") ) {
			var index = FbxTools.get(v, "UVIndex", true);
			var values = FbxTools.getFloats(FbxTools.get(v, "UV"));
			var index = if( index == null ) {
				// ByVertice/Direct (Maya sometimes...)
				[for( i in getPolygons() ) if( i < 0 ) -i - 1 else i];
			} else FbxTools.getInts(index);
			uvs.push({ values : values, index : index });
		}
		return uvs;
	}

	public function getBuffers(binary:Bool, p:FbxParser) {
		// triangulize indexes :
		// format is  A,B,...,-X : negative values mark the end of the polygon
		var pbuf = getVertices();
		var nbuf = getNormals();
		var tbuf = getUVs()[0];
		var polys = getPolygons();

		if (FbxParser.parseTransform) {
			for (i in 0...Std.int(pbuf.length / 3)) {
				pbuf[i * 3    ] *= p.sx;
				// q.fromEuler(p.rx, p.ry, p.rz);
				// v.applyQuat(q);
				pbuf[i * 3    ] += p.tx;
				pbuf[i * 3 + 1] *= p.sy;
				pbuf[i * 3 + 1] += p.ty;
				pbuf[i * 3 + 2] *= p.sz;
				pbuf[i * 3 + 2] += p.tz;
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
		var posa = new kha.arrays.Int16Array(vlen * 4);
		var nora = new kha.arrays.Int16Array(vlen * 2);
		var texa = tbuf != null ? new kha.arrays.Int16Array(vlen * 2) : null;
		var inda = new kha.arrays.Uint32Array(ilen);

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
						texa[vlen * 2    ] = Std.int(       tbuf.values[iuv * 2    ]  * 32767);
						texa[vlen * 2 + 1] = Std.int((1.0 - tbuf.values[iuv * 2 + 1]) * 32767);
					}
					vlen++;
				}
				// polygons are actually triangle fans
				for (n in 0...count - 2) {
					inda[ilen + 2] = start + n;
					inda[ilen + 1] = start + count - 1;
					inda[ilen    ] = start + n + 1;
					ilen += 3;
				}
				polys[pos] = i; // restore
				count = 0;
			}
			pos++;
		}

		return { posa: posa, nora: nora, texa: texa, inda: inda, scalePos: scalePos };
	}
}
