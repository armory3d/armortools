// Based on https://github.com/gorhill/lz4-wasm
// BSD 2-Clause License
// Copyright (c) 2018, Raymond Hill
// All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
package arm.format;

import haxe.io.Bytes;
import js.lib.Int32Array;
import js.lib.Uint8Array;

class Lz4 {

	static var hashTable: Int32Array = null;

	static inline function encodeBound(size: Int): Int {
		return untyped size > 0x7e000000 ? 0 : size + (size / 255 | 0) + 16;
	}

	public static function encode(b: Bytes): Bytes {
		var iBuf = new Uint8Array(cast b.getData());
		var iLen = iBuf.byteLength;
		if (iLen >= 0x7e000000) { trace("LZ4 range error"); return null; }

		// "The last match must start at least 12 bytes before end of block"
		var lastMatchPos = iLen - 12;

		// "The last 5 bytes are always literals"
		var lastLiteralPos = iLen - 5;

		if (hashTable == null) hashTable = new Int32Array(65536);
		hashTable.fill(-65536);

		var oLen = encodeBound(iLen);
		var oBuf = new Uint8Array(oLen);
		var iPos = 0;
		var oPos = 0;
		var anchorPos = 0;

		// Sequence-finding loop
		while (true) {
			var refPos = 0;
			var mOffset = 0;
			var sequence = iBuf[iPos] << 8 | iBuf[iPos + 1] << 16 | iBuf[iPos + 2] << 24;

			// Match-finding loop
			while (iPos <= lastMatchPos) {
				sequence = sequence >>> 8 | iBuf[iPos + 3] << 24;
				var hash = (sequence * 0x9e37 & 0xffff) + (sequence * 0x79b1 >>> 16) & 0xffff;
				refPos = hashTable[hash];
				hashTable[hash] = iPos;
				mOffset = iPos - refPos;
				if (mOffset < 65536 &&
					iBuf[refPos + 0] == ((sequence       ) & 0xff) &&
					iBuf[refPos + 1] == ((sequence >>>  8) & 0xff) &&
					iBuf[refPos + 2] == ((sequence >>> 16) & 0xff) &&
					iBuf[refPos + 3] == ((sequence >>> 24) & 0xff)
				) {
					break;
				}
				iPos += 1;
			}

			// No match found
			if (iPos > lastMatchPos) break;

			// Match found
			var lLen = iPos - anchorPos;
			var mLen = iPos;
			iPos += 4; refPos += 4;
			while (iPos < lastLiteralPos && iBuf[iPos] == iBuf[refPos]) {
				iPos += 1; refPos += 1;
			}
			mLen = iPos - mLen;
			var token = mLen < 19 ? mLen - 4 : 15;

			// Write token, length of literals if needed
			if (lLen >= 15) {
				oBuf[oPos++] = 0xf0 | token;
				var l = lLen - 15;
				while (l >= 255) {
					oBuf[oPos++] = 255;
					l -= 255;
				}
				oBuf[oPos++] = l;
			}
			else {
				oBuf[oPos++] = (lLen << 4) | token;
			}

			// Write literals
			while (lLen-- > 0) {
				oBuf[oPos++] = iBuf[anchorPos++];
			}

			if (mLen == 0) break;

			// Write offset of match
			oBuf[oPos + 0] = mOffset;
			oBuf[oPos + 1] = mOffset >>> 8;
			oPos += 2;

			// Write length of match if needed
			if (mLen >= 19) {
				var l = mLen - 19;
				while (l >= 255) {
					oBuf[oPos++] = 255;
					l -= 255;
				}
				oBuf[oPos++] = l;
			}

			anchorPos = iPos;
		}

		// Last sequence is literals only
		var lLen = iLen - anchorPos;
		if (lLen >= 15) {
			oBuf[oPos++] = 0xf0;
			var l = lLen - 15;
			while (l >= 255) {
				oBuf[oPos++] = 255;
				l -= 255;
			}
			oBuf[oPos++] = l;
		}
		else {
			oBuf[oPos++] = lLen << 4;
		}
		while (lLen-- > 0) {
			oBuf[oPos++] = iBuf[anchorPos++];
		}

		return Bytes.ofData(cast new Uint8Array(oBuf.buffer, 0, oPos));
	}

	public static function decode(b: Bytes, oLen: Int): Bytes {
		var iBuf: Uint8Array = new Uint8Array(cast b.getData());
		var iLen = iBuf.byteLength;
		var oBuf = new Uint8Array(oLen);
	    var iPos = 0;
	    var oPos = 0;

	    while (iPos < iLen) {
	        var token = iBuf[iPos++];

	        // Literals
	        var clen = token >>> 4;

	        // Length of literals
	        if (clen != 0) {
	            if (clen == 15) {
	                var l = 0;
	                while (true) {
	                    l = iBuf[iPos++];
	                    if (l != 255) break;
	                    clen += 255;
	                }
	                clen += l;
	            }

	            // Copy literals
	            var end = iPos + clen;
	            while (iPos < end) {
	                oBuf[oPos++] = iBuf[iPos++];
	            }
	            if (iPos == iLen) break;
	        }

	        // Match
	        var mOffset = iBuf[iPos + 0] | (iBuf[iPos + 1] << 8);
	        if (mOffset == 0 || mOffset > oPos) return null;
	        iPos += 2;

	        // Length of match
	        clen = (token & 0x0f) + 4;
	        if (clen == 19) {
	            var l = 0;
	            while (true) {
	                l = iBuf[iPos++];
	                if (l != 255) break;
	                clen += 255;
	            }
	            clen += l;
	        }

	        // Copy match
	        var mPos = oPos - mOffset;
	        var end = oPos + clen;
	        while (oPos < end) {
	            oBuf[oPos++] = oBuf[mPos++];
	        }
	    }

		return Bytes.ofData(cast oBuf.buffer);
	}
}
