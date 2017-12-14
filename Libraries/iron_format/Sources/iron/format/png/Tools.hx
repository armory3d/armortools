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
package iron.format.png;
import iron.format.png.Data;

class Tools {

	/**
		Returns the PNG header informations. Throws an exception if no header found.
	**/
	public static function getHeader( d : Data ) : Header {
		for( c in d )
			switch( c ) {
			case CHeader(h): return h;
			default:
			}
		throw "Header not found";
	}

	/**
		Return the PNG palette colors, or null if no palette chunk was found
	**/
	public static function getPalette( d : Data ) : haxe.io.Bytes {
		for( c in d )
			switch( c )  {
			case CPalette(b): return b;
			default:
			}
		return null;
	}

	static inline function filter( data : #if flash10 iron.format.tools.MemoryBytes #else haxe.io.Bytes #end, x, y, stride, prev, p, numChannels=4 ) {
		var b = y == 0 ? 0 : data.get(p - stride);
		var c = x == 0 || y == 0  ? 0 : data.get(p - stride - numChannels);
		var k = prev + b - c;
		var pa = k - prev; if( pa < 0 ) pa = -pa;
		var pb = k - b; if( pb < 0 ) pb = -pb;
		var pc = k - c; if( pc < 0 ) pc = -pc;
		return (pa <= pb && pa <= pc) ? prev : (pb <= pc ? b : c);
	}

	/**
		Converts from BGRA to ARGB and the other way by reversing bytes.
	**/
	public static function reverseBytes( b : haxe.io.Bytes ) {
		#if flash10
		var bytes = b.getData();
		if( bytes.length < 1024 ) bytes.length = 1024;
		flash.Memory.select(bytes);
		#end
		inline function bget(p) {
			#if flash10
			return flash.Memory.getByte(p);
			#else
			return b.get(p);
			#end
		}
		inline function bset(p,v) {
			#if flash10
			flash.Memory.setByte(p,v);
			#else
			return b.set(p,v);
			#end
		}
		var p = 0;
		for( i in 0...b.length >> 2 ) {
			var b = bget(p);
			var g = bget(p + 1);
			var r = bget(p + 2);
			var a = bget(p + 3);
			bset(p++, a);
			bset(p++, r);
			bset(p++, g);
			bset(p++, b);
		}
	}

	/**
		Decode the greyscale PNG data and apply filters, extracting only the grey channel if alpha is present.
	**/
	@:noDebug
	public static function extractGrey( d : Data ) : haxe.io.Bytes {
		var h = getHeader(d);
		var grey = haxe.io.Bytes.alloc(h.width * h.height);
		var data = null;
		var fullData : haxe.io.BytesBuffer = null;
		for( c in d )
			switch( c ) {
			case CData(b):
				if( fullData != null )
					fullData.add(b);
				else if( data == null )
					data = b;
				else {
					fullData = new haxe.io.BytesBuffer();
					fullData.add(data);
					fullData.add(b);
					data = null;
				}
			default:
			}
		if( fullData != null )
			data = fullData.getBytes();
		if( data == null )
			throw "Data not found";
		data = iron.format.tools.Inflate.run(data);
		var r = 0, w = 0;
		switch( h.color ) {
		default:
			throw "Unsupported color mode";
		case ColGrey(alpha):
			if( h.colbits != 8 )
				throw "Unsupported color mode";
			var width = h.width;
			var stride = (alpha ? 2 : 1) * width + 1;
			if( data.length < h.height * stride ) throw "Not enough data";

			#if flash10
			var bytes = data.getData();
			var start = h.height * stride;
			bytes.length = start + h.width * h.height;
			if( bytes.length < 1024 ) bytes.length = 1024;
			flash.Memory.select(bytes);
			var realData = data, realGrey = grey;
			var data = iron.format.tools.MemoryBytes.make(0);
			var grey = iron.format.tools.MemoryBytes.make(start);
			#end

			var rinc = (alpha ? 2 : 1);
			for( y in 0...h.height ) {
				var f = data.get(r++);
				switch( f ) {
				case 0:
					for( x in 0...width ) {
						var v = data.get(r); r += rinc;
						grey.set(w++,v);
					}
				case 1:
					var cv = 0;
					for( x in 0...width ) {
						cv += data.get(r); r += rinc;
						grey.set(w++,cv);
					}
				case 2:
					var stride = y == 0 ? 0 : width;
					for( x in 0...width ) {
						var v = data.get(r) + grey.get(w - stride); r += rinc;
						grey.set(w++, v);
					}
				case 3:
					var cv = 0;
					var stride = y == 0 ? 0 : width;
					for( x in 0...width ) {
						cv = (data.get(r) + ((cv + grey.get(w - stride)) >> 1)) & 0xFF; r += rinc;
						grey.set(w++,cv);
					}
				case 4:
					var stride = width;
					var cv = 0;
					for( x in 0...width ) {
						cv = (filter(grey, x, y, stride, cv, w, 1) + data.get(r)) & 0xFF; r += rinc;
						grey.set(w++, cv);
					}
				default:
					throw "Invalid filter "+f;
				}
			}

			#if flash10
			var b = realGrey.getData();
			b.position = 0;
			b.writeBytes(realData.getData(), start, h.width * h.height);
			#end
		}
		return grey;
	}
	/**
		Decode the PNG data and apply filters. By default this will output BGRA low-endian format. You can use the [reverseBytes] function to inverse the bytes to ARGB big-endian format.
	**/
	@:noDebug
	public static function extract32( d : Data, ?bytes, ?flipY ) : haxe.io.Bytes {
		var h = getHeader(d);
		var bgra = bytes == null ? haxe.io.Bytes.alloc(h.width * h.height * 4) : bytes;
		var data = null;
		var fullData : haxe.io.BytesBuffer = null;
		for( c in d )
			switch( c ) {
			case CData(b):
				if( fullData != null )
					fullData.add(b);
				else if( data == null )
					data = b;
				else {
					fullData = new haxe.io.BytesBuffer();
					fullData.add(data);
					fullData.add(b);
					data = null;
				}
			default:
			}
		if( fullData != null )
			data = fullData.getBytes();
		if( data == null )
			throw "Data not found";
		data = iron.format.tools.Inflate.run(data);
		var r = 0, w = 0;
		var lineDelta = 0;
		if( flipY ) {
			lineDelta = -h.width * 8;
			w = (h.height - 1) * (h.width * 4);
		}
		var flipY = flipY ? -1 : 1;
		switch( h.color ) {
		case ColIndexed:
			var pal = getPalette(d);
			if( pal == null ) throw "PNG Palette is missing";

			// transparent palette extension
			var alpha = null;
			for( t in d )
				switch( t ) {
				case CUnknown("tRNS", data): alpha = data; break;
				default:
				}

			// if alpha is incomplete, pad with 0xFF
			if( alpha != null && alpha.length < 1 << h.colbits ) {
				var alpha2 = haxe.io.Bytes.alloc(1 << h.colbits);
				alpha2.blit(0,alpha,0,alpha.length);
				alpha2.fill(alpha.length, alpha2.length - alpha.length, 0xFF);
				alpha = alpha2;
			}

			var width = h.width;
			var stride = Math.ceil(width * h.colbits / 8) + 1;

			if( data.length < h.height * stride ) throw "Not enough data";

			#if flash10
			var bytes = data.getData();
			var start = h.height * stride;
			bytes.length = start + h.width * h.height * 4;
			if( bytes.length < 1024 ) bytes.length = 1024;
			flash.Memory.select(bytes);
			var realData = data, realRgba = bgra;
			var data = iron.format.tools.MemoryBytes.make(0);
			var bgra = iron.format.tools.MemoryBytes.make(start);
			#end

			var rline = (h.width * h.colbits) >> 3;
			for( y in 0...h.height ) {
				var f = data.get(r++);
				if( f == 0 ) {
					r += rline;
					continue;
				}
				switch( f ) {
				case 1:
					var c = 0;
					for( x in 0...width ) {
						var v = data.get(r);
						c += v;
						data.set(r++, c & 0xFF);
					}
				case 2:
					var stride = y == 0 ? 0 : (rline + 1);
					for( x in 0...width ) {
						var v = data.get(r);
						data.set(r, v + data.get(r - stride));
						r++;
					}
				case 3:
					var c = 0;
					var stride = y == 0 ? 0 : (rline + 1);
					for( x in 0...width ) {
						var v = data.get(r);
						c = (v + ((c + data.get(r - stride)) >> 1)) & 0xFF;
						data.set(r++, c);
					}
				case 4:
					var stride = rline + 1;
					var c = 0;
					for( x in 0...width ) {
						var v = data.get(r);
						c = (filter(data, x, y, stride, c, r, 1) + v) & 0xFF;
						data.set(r++, c);
					}
				default:
					throw "Invalid filter "+f;
				}
			}

			var r = 0;
			if( h.colbits == 8 ) {
				for( y in 0...h.height ) {
					r++;
					for( x in 0...h.width ) {
						var c = data.get(r++);
						bgra.set(w++, pal.get(c * 3 + 2));
						bgra.set(w++, pal.get(c * 3 + 1));
						bgra.set(w++, pal.get(c * 3));
						bgra.set(w++, if( alpha != null ) alpha.get(c) else 0xFF);
					}
					w += lineDelta;
				}
			} else if( h.colbits < 8 ) {
				var req = h.colbits;
				var mask = (1 << req) - 1;
				for( y in 0...h.height ) {
					r++;
					var bits = 0, nbits = 0, v;
					for( x in 0...h.width ) {
						if( nbits < req ) {
							bits = (bits << 8) | data.get(r++);
							nbits += 8;
						}
						var c = (bits >>> (nbits - req)) & mask;
						nbits -= req;
						bgra.set(w++, pal.get(c * 3 + 2));
						bgra.set(w++, pal.get(c * 3 + 1));
						bgra.set(w++, pal.get(c * 3));
						bgra.set(w++, if( alpha != null ) alpha.get(c) else 0xFF);
					}
					w += lineDelta;
				}
			} else
				throw h.colbits+" indexed bits per pixel not supported";

			#if flash10
			var b = realRgba.getData();
			b.position = 0;
			b.writeBytes(realData.getData(), start, h.width * h.height * 4);
			#end

		case ColGrey(alpha):
			if( h.colbits != 8 )
				throw "Unsupported color mode";
			var width = h.width;
			var stride = (alpha ? 2 : 1) * width + 1;
			if( data.length < h.height * stride ) throw "Not enough data";

			// transparent palette extension
			var alphvaIdx:Int = -1;
			if (!alpha)
				for( t in d )
					switch( t ) {
					case CUnknown("tRNS", data):
						if (data.length >= 2) alphvaIdx = data.get(1); // Since library supports only 8-bit greyscale, not bothered with conversions.
						break;
					default:
					}


			#if flash10
			var bytes = data.getData();
			var start = h.height * stride;
			bytes.length = start + h.width * h.height * 4;
			if( bytes.length < 1024 ) bytes.length = 1024;
			flash.Memory.select(bytes);
			var realData = data, realRgba = bgra;
			var data = iron.format.tools.MemoryBytes.make(0);
			var bgra = iron.format.tools.MemoryBytes.make(start);
			#end

			for( y in 0...h.height ) {
				var f = data.get(r++);
				switch( f ) {
				case 0:
					if( alpha )
						for( x in 0...width ) {
							var v = data.get(r++);
							bgra.set(w++,v);
							bgra.set(w++,v);
							bgra.set(w++,v);
							bgra.set(w++,data.get(r++));
						}
					else
						for( x in 0...width ) {
							var v = data.get(r++);
							bgra.set(w++,v);
							bgra.set(w++,v);
							bgra.set(w++,v);
							bgra.set(w++,v == alphvaIdx ? 0 : 0xFF);
						}
				case 1:
					var cv = 0, ca = 0;
					if( alpha )
						for( x in 0...width ) {
							cv += data.get(r++);
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							ca += data.get(r++);
							bgra.set(w++,ca);
						}
					else
						for( x in 0...width ) {
							cv += data.get(r++);
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							bgra.set(w++,cv == alphvaIdx ? 0 : 0xFF);
						}
				case 2:
					var stride = y == 0 ? 0 : width * 4 * flipY;
					if( alpha )
						for( x in 0...width ) {
							var v = data.get(r++) + bgra.get(w - stride);
							bgra.set(w++, v);
							bgra.set(w++, v);
							bgra.set(w++, v);
							bgra.set(w++, data.get(r++) + bgra.get(w - stride));
						}
					else
						for( x in 0...width ) {
							var v = data.get(r++) + bgra.get(w - stride);
							bgra.set(w++, v);
							bgra.set(w++, v);
							bgra.set(w++, v);
							bgra.set(w++, v == alphvaIdx ? 0 : 0xFF);
						}
				case 3:
					var cv = 0, ca = 0;
					var stride = y == 0 ? 0 : width * 4 * flipY;
					if( alpha )
						for( x in 0...width ) {
							cv = (data.get(r++) + ((cv + bgra.get(w - stride)) >> 1)) & 0xFF;
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							ca = (data.get(r++) + ((ca + bgra.get(w - stride)) >> 1)) & 0xFF;
							bgra.set(w++,ca);
						}
					else
						for( x in 0...width ) {
							cv = (data.get(r++) + ((cv + bgra.get(w - stride)) >> 1)) & 0xFF;
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							bgra.set(w++,cv);
							bgra.set(w++, cv == alphvaIdx ? 0 : 0xFF);
						}
				case 4:
					var stride = width * 4 * flipY;
					var cv = 0, ca = 0;
					if( alpha )
						for( x in 0...width ) {
							cv = (filter(bgra, x, y, stride, cv, w) + data.get(r++)) & 0xFF;
							bgra.set(w++, cv);
							bgra.set(w++, cv);
							bgra.set(w++, cv);
							ca = (filter(bgra, x, y, stride, ca, w) + data.get(r++)) & 0xFF;
							bgra.set(w++, ca);
						}
					else
						for( x in 0...width ) {
							cv = (filter(bgra, x, y, stride, cv, w) + data.get(r++)) & 0xFF;
							bgra.set(w++, cv);
							bgra.set(w++, cv);
							bgra.set(w++, cv);
							bgra.set(w++, cv == alphvaIdx ? 0 : 0xFF);
						}
				default:
					throw "Invalid filter "+f;
				}
				w += lineDelta;
			}

			#if flash10
			var b = realRgba.getData();
			b.position = 0;
			b.writeBytes(realData.getData(), start, h.width * h.height * 4);
			#end

		case ColTrue(alpha):
			if( h.colbits != 8 )
				throw "Unsupported color mode";
			var width = h.width;
			var stride = (alpha ? 4 : 3) * width + 1;
			if( data.length < h.height * stride ) throw "Not enough data";

			#if flash10
			var bytes = data.getData();
			var start = h.height * stride;
			bytes.length = start + h.width * h.height * 4;
			if( bytes.length < 1024 ) bytes.length = 1024;
			flash.Memory.select(bytes);
			var realData = data, realRgba = bgra;
			var data = iron.format.tools.MemoryBytes.make(0);
			var bgra = iron.format.tools.MemoryBytes.make(start);
			#end

			// transparent palette extension
			var alphaRed:Int = -1;
			var alphaGreen:Int = -1;
			var alphaBlue:Int = -1;
			if (!alpha)
				for( t in d )
					switch( t ) {
					case CUnknown("tRNS", data):
						if (data.length >= 6) {
							alphaRed = data.get(1);
							alphaGreen = data.get(3);
							alphaBlue = data.get(5);
						}
						break;
					default:
					}

			var cr = 0, cg = 0, cb = 0, ca = 0;
			inline function getAlphaValue():Int
			{
				return (cr == alphaRed && cg == alphaGreen && cb == alphaBlue) ? 0 : 0xff;
			}

			// PNG data is encoded as RGB[A]
			for( y in 0...h.height ) {
				var f = data.get(r++);
				switch( f ) {
				case 0:
					if( alpha )
						for( x in 0...width ) {
							bgra.set(w++,data.get(r+2));
							bgra.set(w++,data.get(r+1));
							bgra.set(w++,data.get(r));
							bgra.set(w++,data.get(r+3));
							r += 4;
						}
					else
						for( x in 0...width ) {
							bgra.set(w++,cb = data.get(r+2));
							bgra.set(w++,cg = data.get(r+1));
							bgra.set(w++,cr = data.get(r));
							bgra.set(w++,getAlphaValue());
							r += 3;
						}
				case 1:
					cr = cg = cb = ca = 0;
					if( alpha )
						for( x in 0...width ) {
							cb += data.get(r + 2);	bgra.set(w++,cb);
							cg += data.get(r + 1);	bgra.set(w++,cg);
							cr += data.get(r);		bgra.set(w++,cr);
							ca += data.get(r + 3);	bgra.set(w++,ca);
							r += 4;
						}
					else
						for( x in 0...width ) {
							cb += data.get(r + 2);	bgra.set(w++,cb);
							cg += data.get(r + 1);	bgra.set(w++,cg);
							cr += data.get(r);		bgra.set(w++,cr);
							bgra.set(w++,getAlphaValue());
							r += 3;
						}
				case 2:
					var stride = y == 0 ? 0 : width * 4 * flipY;
					if( alpha )
						for( x in 0...width ) {
							bgra.set(w, data.get(r + 2) + bgra.get(w - stride));	w++;
							bgra.set(w, data.get(r + 1) + bgra.get(w - stride));	w++;
							bgra.set(w, data.get(r) + bgra.get(w - stride));		w++;
							bgra.set(w, data.get(r + 3) + bgra.get(w - stride));	w++;
							r += 4;
						}
					else
						for( x in 0...width ) {
							bgra.set(w, cb = data.get(r + 2) + bgra.get(w - stride));	w++;
							bgra.set(w, cg = data.get(r + 1) + bgra.get(w - stride));	w++;
							bgra.set(w, cr = data.get(r) + bgra.get(w - stride));		w++;
							bgra.set(w++,getAlphaValue());
							r += 3;
						}
				case 3:
					cr = cg = cb = ca = 0;
					var stride = y == 0 ? 0 : width * 4 * flipY;
					if( alpha )
						for( x in 0...width ) {
							cb = (data.get(r + 2) + ((cb + bgra.get(w - stride)) >> 1)) & 0xFF;	bgra.set(w++, cb);
							cg = (data.get(r + 1) + ((cg + bgra.get(w - stride)) >> 1)) & 0xFF;	bgra.set(w++, cg);
							cr = (data.get(r + 0) + ((cr + bgra.get(w - stride)) >> 1)) & 0xFF;	bgra.set(w++, cr);
							ca = (data.get(r + 3) + ((ca + bgra.get(w - stride)) >> 1)) & 0xFF;	bgra.set(w++, ca);
							r += 4;
						}
					else
						for( x in 0...width ) {
							cb = (data.get(r + 2) + ((cb + bgra.get(w - stride)) >> 1)) & 0xFF;	bgra.set(w++, cb);
							cg = (data.get(r + 1) + ((cg + bgra.get(w - stride)) >> 1)) & 0xFF;	bgra.set(w++, cg);
							cr = (data.get(r + 0) + ((cr + bgra.get(w - stride)) >> 1)) & 0xFF;	bgra.set(w++, cr);
							bgra.set(w++,getAlphaValue());
							r += 3;
						}
				case 4:
					var stride = width * 4 * flipY;
					cr = cg = cb = ca = 0;
					if( alpha )
						for( x in 0...width ) {
							cb = (filter(bgra, x, y, stride, cb, w) + data.get(r + 2)) & 0xFF; bgra.set(w++, cb);
							cg = (filter(bgra, x, y, stride, cg, w) + data.get(r + 1)) & 0xFF; bgra.set(w++, cg);
							cr = (filter(bgra, x, y, stride, cr, w) + data.get(r + 0)) & 0xFF; bgra.set(w++, cr);
							ca = (filter(bgra, x, y, stride, ca, w) + data.get(r + 3)) & 0xFF; bgra.set(w++, ca);
							r += 4;
						}
					else
						for( x in 0...width ) {
							cb = (filter(bgra, x, y, stride, cb, w) + data.get(r + 2)) & 0xFF; bgra.set(w++, cb);
							cg = (filter(bgra, x, y, stride, cg, w) + data.get(r + 1)) & 0xFF; bgra.set(w++, cg);
							cr = (filter(bgra, x, y, stride, cr, w) + data.get(r + 0)) & 0xFF; bgra.set(w++, cr);
							bgra.set(w++,getAlphaValue());
							r += 3;
						}
				default:
					throw "Invalid filter "+f;
				}
				w += lineDelta;
			}

			#if flash10
			var b = realRgba.getData();
			b.position = 0;
			b.writeBytes(realData.getData(), start, h.width * h.height * 4);
			#end
		}
		return bgra;
	}

	/**
		Creates PNG data from bytes that contains one bytes (grey values) for each pixel.
	**/
	public static function buildGrey( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : Data {
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
		l.add(CData(iron.format.tools.Deflate.run(rgb,level)));
		l.add(CEnd);
		return l;
	}

	/**
		Creates PNG data from bytes that contains three bytes (R,G and B values) for each pixel.
	**/
	public static function buildRGB( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : Data {
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
		l.add(CData(iron.format.tools.Deflate.run(rgb,level)));
		l.add(CEnd);
		return l;
	}

	/**
		Creates PNG data from bytes that contains four bytes in ARGB format for each pixel.
	**/
	public static function build32ARGB( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : Data {
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
		l.add(CData(iron.format.tools.Deflate.run(rgba,level)));
		l.add(CEnd);
		return l;
	}

	/**
		Creates PNG data from bytes that contains four bytes in BGRA format for each pixel.
	**/
	public static function build32BGRA( width : Int, height : Int, data : haxe.io.Bytes, ?level = 9 ) : Data {
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
		l.add(CData(iron.format.tools.Deflate.run(rgba,level)));
		l.add(CEnd);
		return l;
	}

}
