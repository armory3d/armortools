// Based on miniexr.cpp - public domain - 2013 Aras Pranckevicius / Unity Technologies
// https://github.com/aras-p/miniexr
// https://www.openexr.com/documentation/openexrfilelayout.pdf

class ParserExr {

	static write_string = (out: i32[], str: string) => {
		for (let i: i32 = 0; i < str.length; ++i) {
			out.push(str.charCodeAt(i));
		}
	}

	static run = (width: i32, height: i32, src: ArrayBuffer, bits: i32 = 16, type: i32 = 1, off: i32 = 0): ArrayBuffer => {
		let out: u8[] = [];
		out.push(0x76); // magic
		out.push(0x2f);
		out.push(0x31);
		out.push(0x01);
		out.push(2); // version, scanline
		out.push(0);
		out.push(0);
		out.push(0);
		ParserExr.write_string(out, "channels");
		out.push(0);
		ParserExr.write_string(out, "chlist");
		out.push(0);

		out.push(55);
		out.push(0);
		out.push(0);
		out.push(0);

		let attrib: i32 = bits == 16 ? 1 : 2; // half, float

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

		ParserExr.write_string(out, "compression");
		out.push(0);
		ParserExr.write_string(out, "compression");
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0); // no compression

		ParserExr.write_string(out, "dataWindow");
		out.push(0);
		ParserExr.write_string(out, "box2i");
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

		let ww: i32 = width - 1;
		let hh: i32 = height - 1;

		out.push(ww & 0xff);
		out.push((ww >> 8) & 0xff);
		out.push((ww >> 16) & 0xff);
		out.push((ww >> 24) & 0xff);

		out.push(hh & 0xff);
		out.push((hh >> 8) & 0xff);
		out.push((hh >> 16) & 0xff);
		out.push((hh >> 24) & 0xff);

		ParserExr.write_string(out, "displayWindow");
		out.push(0);
		ParserExr.write_string(out, "box2i");
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

		ParserExr.write_string(out, "lineOrder");
		out.push(0);
		ParserExr.write_string(out, "lineOrder");
		out.push(0);

		out.push(1);
		out.push(0);
		out.push(0);
		out.push(0);
		out.push(0); // increasing Y

		ParserExr.write_string(out, "pixelAspectRatio");
		out.push(0);
		ParserExr.write_string(out, "float");
		out.push(0);

		out.push(4);
		out.push(0);
		out.push(0);
		out.push(0);

		out.push(0); // 1.0f
		out.push(0);
		out.push(0x80);
		out.push(0x3f);

		ParserExr.write_string(out, "screenWindowCenter");
		out.push(0);

		ParserExr.write_string(out, "v2f");
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

		ParserExr.write_string(out, "screenWindowWidth");
		out.push(0);

		ParserExr.write_string(out, "float");
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

		let channels: i32 = 4;
		let byte_size: i32 = bits == 16 ? 2 : 4;
		let k_header_size: i32 = out.length;
		let k_scanline_table_size: i32 = 8 * height;
		let pixel_row_size: i32 = width * 3 * byte_size;
		let full_row_size: i32 = pixel_row_size + 8;

		// line offset table
		let ofs: i32 = k_header_size + k_scanline_table_size;
		for (let y: i32 = 0; y < height; ++y) {
			out.push(ofs & 0xff);
			out.push((ofs >> 8) & 0xff);
			out.push((ofs >> 16) & 0xff);
			out.push((ofs >> 24) & 0xff);
			out.push(0);
			out.push(0);
			out.push(0);
			out.push(0);
			ofs += full_row_size;
		}

		// scanline data
		let stride: i32 = channels * byte_size;
		let pos: i32 = 0;
		let srcView: DataView = new DataView(src);

		let write_line16 = (bytePos: i32) => {
			for (let x: i32 = 0; x < width; ++x) {
				out.push(srcView.getUint8(bytePos    ));
				out.push(srcView.getUint8(bytePos + 1));
				bytePos += stride;
			}
		}

		let write_line32 = (bytePos: i32) => {
			for (let x: i32 = 0; x < width; ++x) {
				out.push(srcView.getUint8(bytePos    ));
				out.push(srcView.getUint8(bytePos + 1));
				out.push(srcView.getUint8(bytePos + 2));
				out.push(srcView.getUint8(bytePos + 3));
				bytePos += stride;
			}
		}

		let write_line = bits == 16 ? write_line16 : write_line32;

		let write_bgr = (off: i32) => {
			write_line(pos + byte_size * 2);
			write_line(pos + byte_size);
			write_line(pos);
		}

		let write_single = (off: i32) => {
			write_line(pos + off * byte_size);
			write_line(pos + off * byte_size);
			write_line(pos + off * byte_size);
		}

		let write_data = type == 1 ? write_bgr : write_single;

		for (let y: i32 = 0; y < height; ++y) {
			// coordinate
			out.push(y & 0xff);
			out.push((y >> 8) & 0xff);
			out.push((y >> 16) & 0xff);
			out.push((y >> 24) & 0xff);
			// data size
			out.push(pixel_row_size & 0xff);
			out.push((pixel_row_size >> 8) & 0xff);
			out.push((pixel_row_size >> 16) & 0xff);
			out.push((pixel_row_size >> 24) & 0xff);
			// data
			write_data(off);
			pos += width * stride;
		}

		return Uint8Array.from(out).buffer;
	}
}
