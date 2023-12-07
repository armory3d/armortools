// Based on miniexr.cpp - public domain - 2013 Aras Pranckevicius / Unity Technologies
// https://github.com/aras-p/miniexr
// https://www.openexr.com/documentation/openexrfilelayout.pdf
package arm.format;

class ExrWriter {

	static function writeString(out: Array<Int>, str: String) {
		for (i in 0...str.length) {
			out.push(str.charCodeAt(i));
		}
	}

	public static function run(width: Int, height: Int, src: js.lib.ArrayBuffer, bits = 16, type = 1, off = 0): js.lib.ArrayBuffer {
		var out = [];
		out.push(0x76); // magic
		out.push(0x2f);
		out.push(0x31);
		out.push(0x01);
		out.push(2); // version, scanline
		out.push(0);
		out.push(0);
		out.push(0);
		writeString(out, "channels");
		out.push(0);
		writeString(out, "chlist");
		out.push(0);

		out.push(55);
		out.push(0);
		out.push(0);
		out.push(0);

		var attrib = bits == 16 ? 1 : 2; // half, float

		out.push("B".code); // B
		out.push(0);

		out.push(attrib);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push("G".code); // G
		out.push(0);

		out.push(attrib);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push("R".code); // R
		out.push(0);

		out.push(attrib);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);

		writeString(out, "compression");
		out.push(0);
		writeString(out, "compression");
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0); // no compression

		writeString(out, "dataWindow");
		out.push(0);
		writeString(out, "box2i");
		out.push(0);

		out.push(16);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		var ww = width - 1;
		var hh = height - 1;

		out.push(ww & 0xff);
		out.push((ww >> 8) & 0xff);
		out.push((ww >> 16) & 0xff);
		out.push((ww >> 24) & 0xff);

		out.push(hh & 0xff);
		out.push((hh >> 8) & 0xff);
		out.push((hh >> 16) & 0xff);
		out.push((hh >> 24) & 0xff);

		writeString(out, "displayWindow");
		out.push(0);
		writeString(out, "box2i");
		out.push(0);

		out.push(16);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(ww & 0xff);
		out.push((ww >> 8) & 0xff);
		out.push((ww >> 16) & 0xff);
		out.push((ww >> 24) & 0xff);

		out.push(hh & 0xff);
		out.push((hh >> 8) & 0xff);
		out.push((hh >> 16) & 0xff);
		out.push((hh >> 24) & 0xff);

		writeString(out, "lineOrder");
		out.push(0);
		writeString(out, "lineOrder");
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0); // increasing Y

		writeString(out, "pixelAspectRatio");
		out.push(0);
		writeString(out, "float");
		out.push(0);

		out.push(4);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0); // 1.0f
		out.push(0);
		out.push(0x80);
		out.push(0x3f);

		writeString(out, "screenWindowCenter");
		out.push(0);

		writeString(out, "v2f");
		out.push(0);

		out.push(8);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0);

		writeString(out, "screenWindowWidth");
		out.push(0);

		writeString(out, "float");
		out.push(0);

		out.push(4);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0); // 1.0f
		out.push(0);
		out.push(0x80);
		out.push(0x3f);

		out.push(0); // end of header

		var channels = 4;
		var byteSize = bits == 16 ? 2 : 4;
		var kHeaderSize = out.length;
		var kScanlineTableSize = 8 * height;
		var pixelRowSize = width * 3 * byteSize;
		var fullRowSize = pixelRowSize + 8;

		// line offset table
		var ofs = kHeaderSize + kScanlineTableSize;
		for (y in 0...height) {
			out.push(ofs & 0xff);
			out.push((ofs >> 8) & 0xff);
			out.push((ofs >> 16) & 0xff);
			out.push((ofs >> 24) & 0xff);
			out.push(0);
			out.push(0);
			out.push(0);
			out.push(0);
			ofs += fullRowSize;
		}

		// scanline data
		var stride = channels * byteSize;
		var pos = 0;
		var srcView = new js.lib.DataView(src);

		function writeLine16(bytePos: Int) {
			for (x in 0...width) {
				out.push(srcView.getUint8(bytePos    ));
				out.push(srcView.getUint8(bytePos + 1));
				bytePos += stride;
			}
		}

		function writeLine32(bytePos: Int) {
			for (x in 0...width) {
				out.push(srcView.getUint8(bytePos    ));
				out.push(srcView.getUint8(bytePos + 1));
				out.push(srcView.getUint8(bytePos + 2));
				out.push(srcView.getUint8(bytePos + 3));
				bytePos += stride;
			}
		}

		var writeLine = bits == 16 ? writeLine16 : writeLine32;

		function writeBGR(off: Int) {
			writeLine(pos + byteSize * 2);
			writeLine(pos + byteSize);
			writeLine(pos);
		}

		function writeSingle(off: Int) {
			writeLine(pos + off * byteSize);
			writeLine(pos + off * byteSize);
			writeLine(pos + off * byteSize);
		}

		var writeData = type == 1 ? writeBGR : writeSingle;

		for (y in 0...height) {
			// coordinate
			out.push(y & 0xff);
			out.push((y >> 8) & 0xff);
			out.push((y >> 16) & 0xff);
			out.push((y >> 24) & 0xff);
			// data size
			out.push(pixelRowSize & 0xff);
			out.push((pixelRowSize >> 8) & 0xff);
			out.push((pixelRowSize >> 16) & 0xff);
			out.push((pixelRowSize >> 24) & 0xff);
			// data
			writeData(off);
			pos += width * stride;
		}

		return js.lib.Uint8Array.from(out).buffer;
	}
}
