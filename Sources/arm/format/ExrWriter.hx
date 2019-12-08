// Based on miniexr.cpp - public domain - 2013 Aras Pranckevicius / Unity Technologies
// https://github.com/aras-p/miniexr
// https://www.openexr.com/documentation/openexrfilelayout.pdf
package arm.format;

class ExrWriter {

	public function new(out: haxe.io.BytesOutput, width: Int, height: Int, src: haxe.io.Bytes, bits = 16, type = 1, off = 0) {
		out.writeByte(0x76); // magic
		out.writeByte(0x2f);
		out.writeByte(0x31);
		out.writeByte(0x01);
		out.writeByte(2); // version, scanline
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeString("channels");
		out.writeByte(0);
		out.writeString("chlist");
		out.writeByte(0);

		out.writeByte(55);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		var attrib = bits == 16 ? 1 : 2; // half, float

		out.writeByte("B".code); // B
		out.writeByte(0);

		out.writeByte(attrib);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte("G".code); // G
		out.writeByte(0);

		out.writeByte(attrib);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte("R".code); // R
		out.writeByte(0);

		out.writeByte(attrib);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);

		out.writeString("compression");
		out.writeByte(0);
		out.writeString("compression");
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0); // no compression

		out.writeString("dataWindow");
		out.writeByte(0);
		out.writeString("box2i");
		out.writeByte(0);

		out.writeByte(16);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		var ww = width - 1;
		var hh = height - 1;

		out.writeByte(ww & 0xff);
		out.writeByte((ww >> 8) & 0xff);
		out.writeByte((ww >> 16) & 0xff);
		out.writeByte((ww >> 24) & 0xff);

		out.writeByte(hh & 0xff);
		out.writeByte((hh >> 8) & 0xff);
		out.writeByte((hh >> 16) & 0xff);
		out.writeByte((hh >> 24) & 0xff);

		out.writeString("displayWindow");
		out.writeByte(0);
		out.writeString("box2i");
		out.writeByte(0);

		out.writeByte(16);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(ww & 0xff);
		out.writeByte((ww >> 8) & 0xff);
		out.writeByte((ww >> 16) & 0xff);
		out.writeByte((ww >> 24) & 0xff);

		out.writeByte(hh & 0xff);
		out.writeByte((hh >> 8) & 0xff);
		out.writeByte((hh >> 16) & 0xff);
		out.writeByte((hh >> 24) & 0xff);

		out.writeString("lineOrder");
		out.writeByte(0);
		out.writeString("lineOrder");
		out.writeByte(0);

		out.writeByte(1);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0); // increasing Y

		out.writeString("pixelAspectRatio");
		out.writeByte(0);
		out.writeString("float");
		out.writeByte(0);

		out.writeByte(4);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0); // 1.0f
		out.writeByte(0);
		out.writeByte(0x80);
		out.writeByte(0x3f);

		out.writeString("screenWindowCenter");
		out.writeByte(0);

		out.writeString("v2f");
		out.writeByte(0);

		out.writeByte(8);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeString("screenWindowWidth");
		out.writeByte(0);

		out.writeString("float");
		out.writeByte(0);

		out.writeByte(4);
		out.writeByte(0);
		out.writeByte(0);
		out.writeByte(0);

		out.writeByte(0); // 1.0f
		out.writeByte(0);
		out.writeByte(0x80);
		out.writeByte(0x3f);

		out.writeByte(0); // end of header

		var channels = 4;
		var byteSize = bits == 16 ? 2 : 4;
		var kHeaderSize = out.length;
		var kScanlineTableSize = 8 * height;
		var pixelRowSize = width * 3 * byteSize;
		var fullRowSize = pixelRowSize + 8;

		// line offset table
		var ofs = kHeaderSize + kScanlineTableSize;
		for (y in 0...height) {
			out.writeByte(ofs & 0xff);
			out.writeByte((ofs >> 8) & 0xff);
			out.writeByte((ofs >> 16) & 0xff);
			out.writeByte((ofs >> 24) & 0xff);
			out.writeByte(0);
			out.writeByte(0);
			out.writeByte(0);
			out.writeByte(0);
			ofs += fullRowSize;
		}

		// scanline data
		var stride = channels * byteSize;
		var pos = 0;

		function writeLine16(bytePos: Int) {
			for (x in 0...width) {
				out.writeByte(src.get(bytePos    ));
				out.writeByte(src.get(bytePos + 1));
				bytePos += stride;
			}
		}

		function writeLine32(bytePos: Int) {
			for (x in 0...width) {
				out.writeByte(src.get(bytePos    ));
				out.writeByte(src.get(bytePos + 1));
				out.writeByte(src.get(bytePos + 2));
				out.writeByte(src.get(bytePos + 3));
				bytePos += stride;
			}
		}

		var writeLine = bits == 16 ? writeLine16 : writeLine32;

		function writeBGR(off: Int) {
			writeLine(pos + byteSize * 2);
			writeLine(pos + byteSize);
			writeLine(pos    );
		}

		function writeSingle(off: Int) {
			writeLine(pos + off * byteSize);
			writeLine(pos + off * byteSize);
			writeLine(pos + off * byteSize);
		}

		var writeData = type == 1 ? writeBGR : writeSingle;

		for (y in 0...height) {
			// coordinate
			out.writeByte(y & 0xff);
			out.writeByte((y >> 8) & 0xff);
			out.writeByte((y >> 16) & 0xff);
			out.writeByte((y >> 24) & 0xff);
			// data size
			out.writeByte(pixelRowSize & 0xff);
			out.writeByte((pixelRowSize >> 8) & 0xff);
			out.writeByte((pixelRowSize >> 16) & 0xff);
			out.writeByte((pixelRowSize >> 24) & 0xff);
			// data
			writeData(off);
			pos += width * stride;
		}
	}
}
