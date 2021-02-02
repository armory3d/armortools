package arm.format;

class ObjParser {

	public static var splitCode = "o".code; // Object split, "g" for groups, "u"semtl for materials
	public var posa: kha.arrays.Int16Array = null;
	public var nora: kha.arrays.Int16Array = null;
	public var texa: kha.arrays.Int16Array = null;
	public var inda: kha.arrays.Uint32Array = null;
	public var udims: Array<kha.arrays.Uint32Array> = null; // Indices split per udim tile
	public var udimsU = 1; // Number of horizontal udim tiles
	public var scalePos = 1.0;
	public var scaleTex = 1.0;
	public var name = "";
	public var hasNext = false; // File contains multiple objects
	public var pos = 0;
	var posTemp: Array<Float>;
	var uvTemp: Array<Float>;
	var norTemp: Array<Float>;
	var va: kha.arrays.Uint32Array;
	var ua: kha.arrays.Uint32Array;
	var na: kha.arrays.Uint32Array;
	var vi = 0;
	var ui = 0;
	var ni = 0;
	var buf: haxe.io.UInt8Array = null;

	static var vindOff = 0;
	static var tindOff = 0;
	static var nindOff = 0;
	static var bytes: haxe.io.Bytes = null;
	static var posFirst: Array<Float>;
	static var uvFirst: Array<Float>;
	static var norFirst: Array<Float>;

	public function new(blob: kha.Blob, startPos = 0, udim = false) {
		pos = startPos;
		var posIndices: Array<Int> = [];
		var uvIndices: Array<Int> = [];
		var norIndices: Array<Int> = [];
		var readingFaces = false;
		var readingObject = false;
		var fullAttrib = false;
		bytes = blob.bytes;

		posTemp = [];
		uvTemp = [];
		norTemp = [];
		va = new kha.arrays.Uint32Array(60);
		ua = new kha.arrays.Uint32Array(60);
		na = new kha.arrays.Uint32Array(60);
		buf = new haxe.io.UInt8Array(64);
		if (startPos == 0) vindOff = tindOff = nindOff = 0;

		if (splitCode == "u".code && startPos > 0) {
			posTemp = posFirst;
			norTemp = norFirst;
			uvTemp = uvFirst;
		}

		while (true) {
			if (pos >= bytes.length) break;

			var c0 = bytes.get(pos++);
			if (readingObject && readingFaces && (c0 == "v".code || c0 == splitCode)) {
				pos--;
				hasNext = true;
				break;
			}

			if (c0 == "v".code) {
				var c1 = bytes.get(pos++);
				if (c1 == " ".code) {
					if (bytes.get(pos) == " ".code) pos++; // Some exporters put additional space directly after "v"
					posTemp.push(readFloat());
					pos++; // Space
					posTemp.push(readFloat());
					pos++; // Space
					posTemp.push(readFloat());
				}
				else if (c1 == "t".code) {
					pos++; // Space
					uvTemp.push(readFloat());
					pos++; // Space
					uvTemp.push(readFloat());
					if (norTemp.length > 0) fullAttrib = true;
				}
				else if (c1 == "n".code) {
					pos++; // Space
					norTemp.push(readFloat());
					pos++; // Space
					norTemp.push(readFloat());
					pos++; // Space
					norTemp.push(readFloat());
					if (uvTemp.length > 0) fullAttrib = true;
				}
			}
			else if (c0 == "f".code) {
				pos++; // Space
				if (bytes.get(pos) == " ".code) pos++; // Some exporters put additional space directly after "f"
				readingFaces = true;
				vi = ui = ni = 0;
				fullAttrib ? readFaceFast() : readFace();

				if (vi <= 4) { // Convex, fan triangulation
					posIndices.push(va[0]);
					posIndices.push(va[1]);
					posIndices.push(va[2]);
					for (i in 3...vi) {
						posIndices.push(va[0]);
						posIndices.push(va[i - 1]);
						posIndices.push(va[i]);
					}
					if (uvTemp.length > 0) {
						uvIndices.push(ua[0]);
						uvIndices.push(ua[1]);
						uvIndices.push(ua[2]);
						for (i in 3...ui) {
							uvIndices.push(ua[0]);
							uvIndices.push(ua[i - 1]);
							uvIndices.push(ua[i]);
						}
					}
					if (norTemp.length > 0) {
						norIndices.push(na[0]);
						norIndices.push(na[1]);
						norIndices.push(na[2]);
						for (i in 3...ni) {
							norIndices.push(na[0]);
							norIndices.push(na[i - 1]);
							norIndices.push(na[i]);
						}
					}
				}
				else { // Convex or concave, ear clipping
					var vindOff = splitCode == "u".code ? 0 : vindOff;
					var nindOff = splitCode == "u".code ? 0 : nindOff;
					var nx = 0.0;
					var ny = 0.0;
					var nz = 0.0;
					if (norTemp.length > 0) {
						nx = norTemp[(na[0] - nindOff) * 3    ];
						ny = norTemp[(na[0] - nindOff) * 3 + 1];
						nz = norTemp[(na[0] - nindOff) * 3 + 2];
					}
					else {
						var n = MeshParser.calcNormal(
							new iron.math.Vec4(posTemp[(va[0] - vindOff) * 3], posTemp[(va[0] - vindOff) * 3 + 1], posTemp[(va[0] - vindOff) * 3 + 2]),
							new iron.math.Vec4(posTemp[(va[1] - vindOff) * 3], posTemp[(va[1] - vindOff) * 3 + 1], posTemp[(va[1] - vindOff) * 3 + 2]),
							new iron.math.Vec4(posTemp[(va[2] - vindOff) * 3], posTemp[(va[2] - vindOff) * 3 + 1], posTemp[(va[2] - vindOff) * 3 + 2])
						);
						nx = n.x;
						ny = n.y;
						nz = n.z;
					}
					var nxabs = Math.abs(nx);
					var nyabs = Math.abs(ny);
					var nzabs = Math.abs(nz);
					var flip = nx + ny + nz > 0;
					var axis = nxabs > nyabs && nxabs > nzabs ? 0 : nyabs > nxabs && nyabs > nzabs ? 1 : 2;
					var axis0 = axis == 0 ? (flip ? 2 : 1) : axis == 1 ? (flip ? 0 : 2) : (flip ? 1 : 0);
					var axis1 = axis == 0 ? (flip ? 1 : 2) : axis == 1 ? (flip ? 2 : 0) : (flip ? 0 : 1);

					var loops = 0;
					var i = -1;
					while (vi > 3 && loops++ < vi) {
						i = (i + 1) % vi;
						var i1 = (i + 1) % vi;
						var i2 = (i + 2) % vi;
						var vi0 = (va[i ] - vindOff) * 3;
						var vi1 = (va[i1] - vindOff) * 3;
						var vi2 = (va[i2] - vindOff) * 3;
						var v0x = posTemp[vi0 + axis0];
						var v0y = posTemp[vi0 + axis1];
						var v1x = posTemp[vi1 + axis0];
						var v1y = posTemp[vi1 + axis1];
						var v2x = posTemp[vi2 + axis0];
						var v2y = posTemp[vi2 + axis1];

						var e0x = v0x - v1x; // Not an interior vertex
						var e0y = v0y - v1y;
						var e1x = v2x - v1x;
						var e1y = v2y - v1y;
						var cross = e0x * e1y - e0y * e1x;
						if (cross <= 0) continue;

						var overlap = false; // Other vertex found inside this triangle
						for (j in 0...vi - 3) {
							var j0 = (va[(i + 3 + j) % vi] - vindOff) * 3;
							var px = posTemp[j0 + axis0];
							var py = posTemp[j0 + axis1];
							if (MeshParser.pnpoly(v0x, v0y, v1x, v1y, v2x, v2y, px, py)) {
								overlap = true;
								break;
							}
						}
						if (overlap) continue;

						posIndices.push(va[i ]); // Found ear
						posIndices.push(va[i1]);
						posIndices.push(va[i2]);
						if (uvTemp.length > 0) {
							uvIndices.push(ua[i ]);
							uvIndices.push(ua[i1]);
							uvIndices.push(ua[i2]);
						}
						if (norTemp.length > 0) {
							norIndices.push(na[i ]);
							norIndices.push(na[i1]);
							norIndices.push(na[i2]);
						}

						for (j in ((i + 1) % vi)...vi - 1) { // Consume vertex
							va[j] = va[j + 1];
							ua[j] = ua[j + 1];
							na[j] = na[j + 1];
						}
						vi--;
						i--;
						loops = 0;
					}
					posIndices.push(va[0]); // Last one
					posIndices.push(va[1]);
					posIndices.push(va[2]);
					if (uvTemp.length > 0) {
						uvIndices.push(ua[0]);
						uvIndices.push(ua[1]);
						uvIndices.push(ua[2]);
					}
					if (norTemp.length > 0) {
						norIndices.push(na[0]);
						norIndices.push(na[1]);
						norIndices.push(na[2]);
					}
				}
			}
			else if (c0 == splitCode) {
				if (splitCode == "u".code) pos += 5; // "u"semtl
				pos++; // Space
				if (!udim) readingObject = true;
				name = readString();
			}
			nextLine();
		}

		if (startPos > 0) {
			if (splitCode != "u".code) {
				for (i in 0...posIndices.length) posIndices[i] -= vindOff;
				for (i in 0...uvIndices.length) uvIndices[i] -= tindOff;
				for (i in 0...norIndices.length) norIndices[i] -= nindOff;
			}
		}
		else {
			if (splitCode == "u".code) {
				posFirst = posTemp;
				norFirst = norTemp;
				uvFirst = uvTemp;
			}
		}
		vindOff += Std.int(posTemp.length / 3); // Assumes separate vertex data per object
		tindOff += Std.int(uvTemp.length / 2);
		nindOff += Std.int(norTemp.length / 3);

		// Pack positions to (-1, 1) range
		scalePos = 0.0;
		for (i in 0...posTemp.length) {
			var f = Math.abs(posTemp[i]);
			if (scalePos < f) scalePos = f;
		}
		var inv = 32767 * (1 / scalePos);

		posa = new kha.arrays.Int16Array(posIndices.length * 4);
		inda = new kha.arrays.Uint32Array(posIndices.length);
		for (i in 0...posIndices.length) {
			posa[i * 4    ] = Std.int( posTemp[posIndices[i] * 3    ] * inv);
			posa[i * 4 + 1] = Std.int(-posTemp[posIndices[i] * 3 + 2] * inv);
			posa[i * 4 + 2] = Std.int( posTemp[posIndices[i] * 3 + 1] * inv);
			inda[i] = i;
		}

		if (norIndices.length > 0) {
			nora = new kha.arrays.Int16Array(norIndices.length * 2);
			for (i in 0...posIndices.length) {
				nora[i * 2    ] = Std.int( norTemp[norIndices[i] * 3    ] * 32767);
				nora[i * 2 + 1] = Std.int(-norTemp[norIndices[i] * 3 + 2] * 32767);
				posa[i * 4 + 3] = Std.int( norTemp[norIndices[i] * 3 + 1] * 32767);
			}
		}
		else {
			// Calc normals
			nora = new kha.arrays.Int16Array(inda.length * 2);
			var va = new iron.math.Vec4();
			var vb = new iron.math.Vec4();
			var vc = new iron.math.Vec4();
			var cb = new iron.math.Vec4();
			var ab = new iron.math.Vec4();
			for (i in 0...Std.int(inda.length / 3)) {
				var i1 = inda[i * 3    ];
				var i2 = inda[i * 3 + 1];
				var i3 = inda[i * 3 + 2];
				va.set(posa[i1 * 4], posa[i1 * 4 + 1], posa[i1 * 4 + 2]);
				vb.set(posa[i2 * 4], posa[i2 * 4 + 1], posa[i2 * 4 + 2]);
				vc.set(posa[i3 * 4], posa[i3 * 4 + 1], posa[i3 * 4 + 2]);
				cb.subvecs(vc, vb);
				ab.subvecs(va, vb);
				cb.cross(ab);
				cb.normalize();
				nora[i1 * 2    ] = Std.int(cb.x * 32767);
				nora[i1 * 2 + 1] = Std.int(cb.y * 32767);
				posa[i1 * 4 + 3] = Std.int(cb.z * 32767);
				nora[i2 * 2    ] = Std.int(cb.x * 32767);
				nora[i2 * 2 + 1] = Std.int(cb.y * 32767);
				posa[i2 * 4 + 3] = Std.int(cb.z * 32767);
				nora[i3 * 2    ] = Std.int(cb.x * 32767);
				nora[i3 * 2 + 1] = Std.int(cb.y * 32767);
				posa[i3 * 4 + 3] = Std.int(cb.z * 32767);
			}
		}

		if (uvIndices.length > 0) {
			if (udim) {
				// Find number of tiles
				var tilesU = 1;
				var tilesV = 1;
				for (i in 0...Std.int(uvTemp.length / 2)) {
					while (uvTemp[i * 2    ] > tilesU) tilesU++;
					while (uvTemp[i * 2 + 1] > tilesV) tilesV++;
				}

				function getTile(i1: Int, i2: Int, i3: Int): Int {
					var u1 = uvTemp[uvIndices[i1] * 2    ];
					var v1 = uvTemp[uvIndices[i1] * 2 + 1];
					var u2 = uvTemp[uvIndices[i2] * 2    ];
					var v2 = uvTemp[uvIndices[i2] * 2 + 1];
					var u3 = uvTemp[uvIndices[i3] * 2    ];
					var v3 = uvTemp[uvIndices[i3] * 2 + 1];
					var tileU = Std.int((u1 + u2 + u3) / 3);
					var tileV = Std.int((v1 + v2 + v3) / 3);
					return tileU + tileV * tilesU;
				}

				// Amount of indices pre tile
				var num = new kha.arrays.Uint32Array(tilesU * tilesV);
				for (i in 0...Std.int(inda.length / 3)) {
					var tile = getTile(inda[i * 3], inda[i * 3 + 1], inda[i * 3 + 2]);
					num[tile] += 3;
				}

				// Split indices per tile
				udims = [];
				udimsU = tilesU;
				for (i in 0...tilesU * tilesV) { udims.push(new kha.arrays.Uint32Array(num[i])); num[i] = 0; }

				for (i in 0...Std.int(inda.length / 3)) {
					var i1 = inda[i * 3    ];
					var i2 = inda[i * 3 + 1];
					var i3 = inda[i * 3 + 2];
					var tile = getTile(i1, i2, i3);
					udims[tile][num[tile]++] = i1;
					udims[tile][num[tile]++] = i2;
					udims[tile][num[tile]++] = i3;
				}

				// Normalize uvs to 0-1 range
				var uvtiles = new kha.arrays.Int16Array(uvTemp.length);
				for (i in 0...Std.int(inda.length / 3)) { // TODO: merge loops
					var i1 = inda[i * 3    ];
					var i2 = inda[i * 3 + 1];
					var i3 = inda[i * 3 + 2];
					var tile = getTile(i1, i2, i3);
					var tileU = tile % tilesU;
					var tileV = Std.int(tile / tilesU);
					uvtiles[uvIndices[i1] * 2    ] = tileU;
					uvtiles[uvIndices[i1] * 2 + 1] = tileV;
					uvtiles[uvIndices[i2] * 2    ] = tileU;
					uvtiles[uvIndices[i2] * 2 + 1] = tileV;
					uvtiles[uvIndices[i3] * 2    ] = tileU;
					uvtiles[uvIndices[i3] * 2 + 1] = tileV;
				}
				for (i in 0...uvtiles.length) uvTemp[i] -= uvtiles[i];
			}

			texa = new kha.arrays.Int16Array(uvIndices.length * 2);
			for (i in 0...posIndices.length) {
				var uvx = uvTemp[uvIndices[i] * 2];
				if (uvx > 1.0) uvx = uvx - Std.int(uvx);
				var uvy = uvTemp[uvIndices[i] * 2 + 1];
				if (uvy > 1.0) uvy = uvy - Std.int(uvy);
				texa[i * 2    ] = Std.int(       uvx  * 32767);
				texa[i * 2 + 1] = Std.int((1.0 - uvy) * 32767);
			}
		}
		bytes = null;
		if (!hasNext) { posFirst = norFirst = uvFirst = null; }
	}

	function readFaceFast() {
		while (true) {
			va[vi++] = readInt() - 1;
			pos++; // "/"
			ua[ui++] = readInt() - 1;
			pos++; // "/"
			na[ni++] = readInt() - 1;
			if (bytes.get(pos) == "\n".code || bytes.get(pos) == "\r".code) break;
			pos++; // " "
			// Some exporters put space at the end of "f" line
			if (vi >= 3 && (bytes.get(pos) == "\n".code || bytes.get(pos) == "\r".code)) break;
		}
	}

	function readFace() {
		while (true) {
			va[vi++] = readInt() - 1;
			if (uvTemp.length > 0 || norTemp.length > 0) {
				pos++; // "/"
			}
			if (uvTemp.length > 0) {
				ua[ui++] = readInt() - 1;
			}
			if (norTemp.length > 0) {
				pos++; // "/"
				na[ni++] = readInt() - 1;
			}
			if (bytes.get(pos) == "\n".code || bytes.get(pos) == "\r".code) break;
			pos++; // " "
			// Some exporters put space at the end of "f" line
			if (vi >= 3 && (bytes.get(pos) == "\n".code || bytes.get(pos) == "\r".code)) break;
		}
	}

	function readFloat(): Float {
		var bi = 0;
		while (true) { // Read into buffer
			var c = bytes.get(pos);
			if (c == " ".code || c == "\n".code || c == "\r".code) break;
			if (c == "E".code || c == "e".code) {
				pos++;
				var first = buf[0] == "-".code ? -(buf[1] - 48) : buf[0] - 48;
				var exp = readInt();
				var dec = 1;
				var loop = exp > 0 ? exp : -exp;
				for (i in 0...loop) dec *= 10;
				return exp > 0 ? first * dec : first / dec;
			}
			pos++;
			buf[bi++] = c;
		}
		var res = 0.0; // Parse buffer into float
		var dot = 1;
		var dec = 1;
		var off = buf[0] == "-".code ? 1 : 0;
		var len = bi - 1;
		for (i in 0...bi - off) {
			var c = buf[len - i];
			if (c == ".".code) { dot = dec; continue; }
			res += (c - 48) * dec;
			dec *= 10;
		}
		off > 0 ? res /= -dot : res /= dot;
		return res;
	}

	function readInt(): Int {
		var bi = 0;
		while (true) { // Read into buffer
			var c = bytes.get(pos);
			if (c == "/".code || c == "\n".code || c == "\r".code || c == " ".code) break;
			pos++;
			buf[bi++] = c;
		}
		var res = 0; // Parse buffer into int
		var dec = 1;
		var off = buf[0] == "-".code ? 1 : 0;
		var len = bi - 1;
		for (i in 0...bi - off) {
			res += (buf[len - i] - 48) * dec;
			dec *= 10;
		}
		if (off > 0) res *= -1;
		return res;
	}

	function readString(): String {
		var s = "";
		while (true) {
			var c = bytes.get(pos);
			if (c == "\n".code || c == "\r".code || c == " ".code) break;
			pos++;
			s += String.fromCharCode(c);
		}
		return s;
	}

	function nextLine() {
		while (true) {
			var c = bytes.get(pos++);
			if (c == "\n".code || pos >= bytes.length) break; // \n, \r\n
		}
	}
}
