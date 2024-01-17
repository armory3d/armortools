// Based on miniexr.cpp - public domain - 2013 Aras Pranckevicius / Unity Technologies
// https://github.com/aras-p/miniexr
// https://www.openexr.com/documentation/openexrfilelayout.pdf

class ParserExr {

	static writeString = (out: i32[], str: string) => {
		for (let i = 0; i < str.length; ++i) {
			out.push(str.charCodeAt(i));
		}
	}

	static run = (width: i32, height: i32, src: ArrayBuffer, bits = 16, type = 1, off = 0): ArrayBuffer => {
		let out = [];
		out.push(0x76); // magic
		out.push(0x2f);
		out.push(0x31);
		out.push(0x01);
		out.push(2); // version, scanline
		out.push(0);
		out.push(0);
		out.push(0);
		ParserExr.writeString(out, "channels");
		out.push(0);
		ParserExr.writeString(out, "chlist");
		out.push(0);

		out.push(55);
		out.push(0);
		out.push(0);
		out.push(0);

		let attrib = bits == 16 ? 1 : 2; // half, float

		out.push("B".charCodeAt(0)); // B
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

		out.push("G".charCodeAt(0)); // G
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

		out.push("R".charCodeAt(0)); // R
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

		ParserExr.writeString(out, "compression");
		out.push(0);
		ParserExr.writeString(out, "compression");
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0); // no compression

		ParserExr.writeString(out, "dataWindow");
		out.push(0);
		ParserExr.writeString(out, "box2i");
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

		let ww = width - 1;
		let hh = height - 1;

		out.push(ww & 0xff);
		out.push((ww >> 8) & 0xff);
		out.push((ww >> 16) & 0xff);
		out.push((ww >> 24) & 0xff);

		out.push(hh & 0xff);
		out.push((hh >> 8) & 0xff);
		out.push((hh >> 16) & 0xff);
		out.push((hh >> 24) & 0xff);

		ParserExr.writeString(out, "displayWindow");
		out.push(0);
		ParserExr.writeString(out, "box2i");
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

		ParserExr.writeString(out, "lineOrder");
		out.push(0);
		ParserExr.writeString(out, "lineOrder");
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0); // increasing Y

		ParserExr.writeString(out, "pixelAspectRatio");
		out.push(0);
		ParserExr.writeString(out, "float");
		out.push(0);

		out.push(4);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0); // 1.0f
		out.push(0);
		out.push(0x80);
		out.push(0x3f);

		ParserExr.writeString(out, "screenWindowCenter");
		out.push(0);

		ParserExr.writeString(out, "v2f");
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

		ParserExr.writeString(out, "screenWindowWidth");
		out.push(0);

		ParserExr.writeString(out, "float");
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

		let channels = 4;
		let byteSize = bits == 16 ? 2 : 4;
		let kHeaderSize = out.length;
		let kScanlineTableSize = 8 * height;
		let pixelRowSize = width * 3 * byteSize;
		let fullRowSize = pixelRowSize + 8;

		// line offset table
		let ofs = kHeaderSize + kScanlineTableSize;
		for (let y = 0; y < height; ++y) {
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
		let stride = channels * byteSize;
		let pos = 0;
		let srcView = new DataView(src);

		let writeLine16 = (bytePos: i32) => {
			for (let x = 0; x < width; ++x) {
				out.push(srcView.getUint8(bytePos    ));
				out.push(srcView.getUint8(bytePos + 1));
				bytePos += stride;
			}
		}

		let writeLine32 = (bytePos: i32) => {
			for (let x = 0; x < width; ++x) {
				out.push(srcView.getUint8(bytePos    ));
				out.push(srcView.getUint8(bytePos + 1));
				out.push(srcView.getUint8(bytePos + 2));
				out.push(srcView.getUint8(bytePos + 3));
				bytePos += stride;
			}
		}

		let writeLine = bits == 16 ? writeLine16 : writeLine32;

		let writeBGR = (off: i32) => {
			writeLine(pos + byteSize * 2);
			writeLine(pos + byteSize);
			writeLine(pos);
		}

		let writeSingle = (off: i32) => {
			writeLine(pos + off * byteSize);
			writeLine(pos + off * byteSize);
			writeLine(pos + off * byteSize);
		}

		let writeData = type == 1 ? writeBGR : writeSingle;

		for (let y = 0; y < height; ++y) {
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

		return Uint8Array.from(out).buffer;
	}
}
