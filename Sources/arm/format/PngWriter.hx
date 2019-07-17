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

class PngWriter {

	var o : haxe.io.Output;

	public function new(o) {
		this.o = o;
		o.bigEndian = true;
	}

	public function write( png : PngData ) {
		for( b in [137,80,78,71,13,10,26,10] )
			o.writeByte(b);
		for( c in png )
			switch( c ) {
			case CHeader(h):
				var b = new haxe.io.BytesOutput();
				b.bigEndian = true;
				b.writeInt32(h.width);
				b.writeInt32(h.height);
				b.writeByte(h.colbits);
				b.writeByte(switch( h.color ) {
					case ColGrey(alpha): alpha ? 4 : 0;
					case ColTrue(alpha): alpha ? 6 : 2;
					case ColIndexed: 3;
				});
				b.writeByte(0);
				b.writeByte(0);
				b.writeByte(h.interlaced ? 1 : 0);
				writeChunk("IHDR",b.getBytes());
			case CEnd:
				writeChunk("IEND",haxe.io.Bytes.alloc(0));
			case CData(d):
				writeChunk("IDAT",d);
			case CPalette(b):
				writeChunk("PLTE",b);
			case CUnknown(id,data):
				writeChunk(id,data);
			}
	}

	function writeChunk( id : String, data : haxe.io.Bytes ) {
		o.writeInt32(data.length);
		o.writeString(id);
		o.write(data);
		// compute CRC
		var crc = new haxe.crypto.Crc32();
		for( i in 0...4 )
			crc.byte(id.charCodeAt(i));
		crc.update(data, 0, data.length);
		o.writeInt32(crc.get());
	}

}
