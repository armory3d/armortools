// Based on miniexr.cpp - public domain - 2013 Aras Pranckevicius / Unity Technologies
// https://github.com/aras-p/miniexr
// https://www.openexr.com/documentation/openexrfilelayout.pdf

let _parser_exr_width: i32;
let _parser_exr_stride: i32;
let _parser_exr_out: u8[];
let _parser_exr_src_view: buffer_t;
let _parser_exr_write_line: (byte_pos: i32)=> void;

function parser_exr_write_string(out: i32[], str: string) {
	for (let i: i32 = 0; i < str.length; ++i) {
		array_push(out, char_code_at(str, i));
	}
}

function parser_exr_write_line16(byte_pos: i32) {
	for (let x: i32 = 0; x < _parser_exr_width; ++x) {
		array_push(_parser_exr_out, buffer_get_u8(_parser_exr_src_view, byte_pos    ));
		array_push(_parser_exr_out, buffer_get_u8(_parser_exr_src_view, byte_pos + 1));
		byte_pos += _parser_exr_stride;
	}
}

function parser_exr_write_line32(byte_pos: i32) {
	for (let x: i32 = 0; x < _parser_exr_width; ++x) {
		array_push(_parser_exr_out, buffer_get_u8(_parser_exr_src_view, byte_pos    ));
		array_push(_parser_exr_out, buffer_get_u8(_parser_exr_src_view, byte_pos + 1));
		array_push(_parser_exr_out, buffer_get_u8(_parser_exr_src_view, byte_pos + 2));
		array_push(_parser_exr_out, buffer_get_u8(_parser_exr_src_view, byte_pos + 3));
		byte_pos += _parser_exr_stride;
	}
}

function parser_exr_write_bgr(off: i32, pos: i32, byte_size: i32) {
	_parser_exr_write_line(pos + byte_size * 2);
	_parser_exr_write_line(pos + byte_size);
	_parser_exr_write_line(pos);
}

function parser_exr_write_single(off: i32, pos: i32, byte_size: i32) {
	_parser_exr_write_line(pos + off * byte_size);
	_parser_exr_write_line(pos + off * byte_size);
	_parser_exr_write_line(pos + off * byte_size);
}

function parser_exr_run(width: i32, height: i32, src: buffer_t, bits: i32 = 16, type: i32 = 1, off: i32 = 0): buffer_t {
	let out: u8[] = [];
	array_push(out, 0x76); // magic
	array_push(out, 0x2f);
	array_push(out, 0x31);
	array_push(out, 0x01);
	array_push(out, 2); // version, scanline
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	parser_exr_write_string(out, "channels");
	array_push(out, 0);
	parser_exr_write_string(out, "chlist");
	array_push(out, 0);

	array_push(out, 55);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	let attrib: i32 = bits == 16 ? 1 : 2; // half, float

	array_push(out, char_code_at("B", 0)); // B
	array_push(out, 0);

	array_push(out, attrib);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, char_code_at("G", 0)); // G
	array_push(out, 0);

	array_push(out, attrib);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, char_code_at("R", 0)); // R
	array_push(out, 0);

	array_push(out, attrib);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);

	parser_exr_write_string(out, "compression");
	array_push(out, 0);
	parser_exr_write_string(out, "compression");
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0); // no compression

	parser_exr_write_string(out, "dataWindow");
	array_push(out, 0);
	parser_exr_write_string(out, "box2i");
	array_push(out, 0);

	array_push(out, 16);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	let ww: i32 = width - 1;
	let hh: i32 = height - 1;

	array_push(out, ww & 0xff);
	array_push(out, (ww >> 8) & 0xff);
	array_push(out, (ww >> 16) & 0xff);
	array_push(out, (ww >> 24) & 0xff);

	array_push(out, hh & 0xff);
	array_push(out, (hh >> 8) & 0xff);
	array_push(out, (hh >> 16) & 0xff);
	array_push(out, (hh >> 24) & 0xff);

	parser_exr_write_string(out, "displayWindow");
	array_push(out, 0);
	parser_exr_write_string(out, "box2i");
	array_push(out, 0);

	array_push(out, 16);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, ww & 0xff);
	array_push(out, (ww >> 8) & 0xff);
	array_push(out, (ww >> 16) & 0xff);
	array_push(out, (ww >> 24) & 0xff);

	array_push(out, hh & 0xff);
	array_push(out, (hh >> 8) & 0xff);
	array_push(out, (hh >> 16) & 0xff);
	array_push(out, (hh >> 24) & 0xff);

	parser_exr_write_string(out, "lineOrder");
	array_push(out, 0);
	parser_exr_write_string(out, "lineOrder");
	array_push(out, 0);

	array_push(out, 1);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0); // increasing Y

	parser_exr_write_string(out, "pixelAspectRatio");
	array_push(out, 0);
	parser_exr_write_string(out, "float");
	array_push(out, 0);

	array_push(out, 4);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0); // 1.0f
	array_push(out, 0);
	array_push(out, 0x80);
	array_push(out, 0x3f);

	parser_exr_write_string(out, "screenWindowCenter");
	array_push(out, 0);

	parser_exr_write_string(out, "v2f");
	array_push(out, 0);

	array_push(out, 8);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	parser_exr_write_string(out, "screenWindowWidth");
	array_push(out, 0);

	parser_exr_write_string(out, "float");
	array_push(out, 0);

	array_push(out, 4);
	array_push(out, 0);
	array_push(out, 0);
	array_push(out, 0);

	array_push(out, 0); // 1.0f
	array_push(out, 0);
	array_push(out, 0x80);
	array_push(out, 0x3f);

	array_push(out, 0); // end of header

	let channels: i32 = 4;
	let byte_size: i32 = bits == 16 ? 2 : 4;
	let k_header_size: i32 = out.length;
	let k_scanline_table_size: i32 = 8 * height;
	let pixel_row_size: i32 = width * 3 * byte_size;
	let full_row_size: i32 = pixel_row_size + 8;

	// line offset table
	let ofs: i32 = k_header_size + k_scanline_table_size;
	for (let y: i32 = 0; y < height; ++y) {
		array_push(out, ofs & 0xff);
		array_push(out, (ofs >> 8) & 0xff);
		array_push(out, (ofs >> 16) & 0xff);
		array_push(out, (ofs >> 24) & 0xff);
		array_push(out, 0);
		array_push(out, 0);
		array_push(out, 0);
		array_push(out, 0);
		ofs += full_row_size;
	}

	// scanline data
	let stride: i32 = channels * byte_size;
	let pos: i32 = 0;
	let src_view: buffer_t = src;

	_parser_exr_width = width;
	_parser_exr_stride = stride;
	_parser_exr_out = out;
	_parser_exr_src_view = src_view;

	_parser_exr_write_line = bits == 16 ? parser_exr_write_line16 : parser_exr_write_line32;
	let write_data: (off: i32, pos: i32, byte_size: i32)=>void = type == 1 ? parser_exr_write_bgr : parser_exr_write_single;

	for (let y: i32 = 0; y < height; ++y) {
		// coordinate
		array_push(out, y & 0xff);
		array_push(out, (y >> 8) & 0xff);
		array_push(out, (y >> 16) & 0xff);
		array_push(out, (y >> 24) & 0xff);
		// data size
		array_push(out, pixel_row_size & 0xff);
		array_push(out, (pixel_row_size >> 8) & 0xff);
		array_push(out, (pixel_row_size >> 16) & 0xff);
		array_push(out, (pixel_row_size >> 24) & 0xff);
		// data
		write_data(off, pos, byte_size);
		pos += width * stride;
	}

	return u8_array_create_from_array(out).buffer;
}
