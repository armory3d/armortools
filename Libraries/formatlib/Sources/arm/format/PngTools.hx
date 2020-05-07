/*
 * format - haXe File Formats
 *
 * Copyright (c) 2008-2009, The haXe Project Contributors
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE HAXE PROJECT CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE HAXE PROJECT CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
package arm.format;
import arm.format.PngData;

class PngTools {

	/**
		Creates PNG data from bytes that contains one bytes (grey values) for each pixel.
	**/
	public static function buildGrey( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : PngData {
		var rgb = haxe.io.Bytes.alloc(width * height + height);
		// translate RGB to BGR and add filter byte
		var w = 0, r = 0;
		for( y in 0...height ) {
			rgb.set(w++,0); // no filter for this scanline
			for( x in 0...width )
				rgb.set(w++,data.get(r++));
		}
		var l = new List();
		l.add(CHeader({ width : width, height : height, colbits : 8, color : ColGrey(false), interlaced : false }));
		l.add(CData(deflate(rgb,level)));
		l.add(CEnd);
		return l;
	}

	/**
		Creates PNG data from bytes that contains three bytes (R,G and B values) for each pixel.
	**/
	public static function buildRGB( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : PngData {
		var rgb = haxe.io.Bytes.alloc(width * height * 3 + height);
		// translate RGB to BGR and add filter byte
		var w = 0, r = 0;
		for( y in 0...height ) {
			rgb.set(w++,0); // no filter for this scanline
			for( x in 0...width ) {
				rgb.set(w++,data.get(r+2));
				rgb.set(w++,data.get(r+1));
				rgb.set(w++,data.get(r));
				r += 3;
			}
		}
		var l = new List();
		l.add(CHeader({ width : width, height : height, colbits : 8, color : ColTrue(false), interlaced : false }));
		l.add(CData(deflate(rgb,level)));
		l.add(CEnd);
		return l;
	}

	/**
		Creates PNG data from bytes that contains four bytes in ARGB format for each pixel.
	**/
	public static function build32ARGB( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : PngData {
		var rgba = haxe.io.Bytes.alloc(width * height * 4 + height);
		// translate ARGB to RGBA and add filter byte
		var w = 0, r = 0;
		for( y in 0...height ) {
			rgba.set(w++,0); // no filter for this scanline
			for( x in 0...width ) {
				rgba.set(w++,data.get(r+1)); // r
				rgba.set(w++,data.get(r+2)); // g
				rgba.set(w++,data.get(r+3)); // b
				rgba.set(w++,data.get(r)); // a
				r += 4;
			}
		}
		var l = new List();
		l.add(CHeader({ width : width, height : height, colbits : 8, color : ColTrue(true), interlaced : false }));
		l.add(CData(deflate(rgba,level)));
		l.add(CEnd);
		return l;
	}

	/**
		Creates PNG data from bytes that contains four bytes in BGRA format for each pixel.
	**/
	public static function build32BGRA( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : PngData {
		var rgba = haxe.io.Bytes.alloc(width * height * 4 + height);
		// translate ARGB to RGBA and add filter byte
		var w = 0, r = 0;
		for( y in 0...height ) {
			rgba.set(w++,0); // no filter for this scanline
			for( x in 0...width ) {
				rgba.set(w++,data.get(r+2)); // r
				rgba.set(w++,data.get(r+1)); // g
				rgba.set(w++,data.get(r)); // b
				rgba.set(w++,data.get(r+3)); // a
				r += 4;
			}
		}
		var l = new List();
		l.add(CHeader({ width : width, height : height, colbits : 8, color : ColTrue(true), interlaced : false }));
		l.add(CData(deflate(rgba,level)));
		l.add(CEnd);
		return l;
	}

	public static function build32RGBA( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : PngData {
		var rgba = haxe.io.Bytes.alloc(width * height * 4 + height);
		var w = 0, r = 0;
		for( y in 0...height ) {
			rgba.set(w++,0); // no filter for this scanline
			for( x in 0...width ) {
				rgba.set(w++,data.get(r)); // r
				rgba.set(w++,data.get(r+1)); // g
				rgba.set(w++,data.get(r+2)); // b
				rgba.set(w++,data.get(r+3)); // a
				r += 4;
			}
		}
		var l = new List();
		l.add(CHeader({ width : width, height : height, colbits : 8, color : ColTrue(true), interlaced : false }));
		l.add(CData(deflate(rgba,level)));
		l.add(CEnd);
		return l;
	}

	// armory
	public static function build32RGB1( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : PngData {
		var rgba = haxe.io.Bytes.alloc(width * height * 4 + height);
		var w = 0, r = 0;
		for( y in 0...height ) {
			rgba.set(w++,0); // no filter for this scanline
			for( x in 0...width ) {
				rgba.set(w++,data.get(r)); // r
				rgba.set(w++,data.get(r+1)); // g
				rgba.set(w++,data.get(r+2)); // b
				rgba.set(w++,255); // 1
				r += 4;
			}
		}
		var l = new List();
		l.add(CHeader({ width : width, height : height, colbits : 8, color : ColTrue(true), interlaced : false }));
		l.add(CData(deflate(rgba,level)));
		l.add(CEnd);
		return l;
	}

	public static function build32RRR1( width : Int, height : Int, data : haxe.io.Bytes, off : Int, ?level = 9 ) : PngData {
		var rgba = haxe.io.Bytes.alloc(width * height * 4 + height);
		var w = 0, r = 0;
		for( y in 0...height ) {
			rgba.set(w++,0); // no filter for this scanline
			for( x in 0...width ) {
				rgba.set(w++,data.get(r+off)); // r
				rgba.set(w++,data.get(r+off)); // r
				rgba.set(w++,data.get(r+off)); // r
				rgba.set(w++,255); // 1
				r += 4;
			}
		}
		var l = new List();
		l.add(CHeader({ width : width, height : height, colbits : 8, color : ColTrue(true), interlaced : false }));
		l.add(CData(deflate(rgba,level)));
		l.add(CEnd);
		return l;
	}

	public static function deflate(b:haxe.io.Bytes, ?level = 9):haxe.io.Bytes {
		// return haxe.zip.Compress.run(b,level);
		var input = haxe.io.UInt8Array.fromBytes(b);
		var output = arm.format.pako.Pako.deflate(input);
		return output.view.buffer;
	}
}
