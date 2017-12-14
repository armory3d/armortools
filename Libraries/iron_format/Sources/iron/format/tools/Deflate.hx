/*
 * format - haXe File Formats
 *
 * Copyright (c) 2008, The haXe Project Contributors
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
package iron.format.tools;

class Deflate {

	public static function run( b : haxe.io.Bytes, ?level = 9 ) : haxe.io.Bytes {
		// #if (haxe_ver >= 3.2)

		// return haxe.zip.Compress.run(b,level);
		
		var input = haxe.io.UInt8Array.fromBytes(b);
		var output = iron.format.pako.Pako.deflate(input);
		return output.view.buffer;

		// #else
		// // legacy
		// #if neko
		// return neko.zip.Compress.run(b,level);
		// #elseif flash9
		// var bytes = b.sub(0,b.length);
		// var data = bytes.getData();
		// data.compress();
		// return haxe.io.Bytes.ofData(data);
		// #elseif cpp
		// return cpp.zip.Compress.run(b,level);
		// #elseif ( java || php )
		// return haxe.zip.Compress.run(b,level);
		// #else
		// throw "Deflate is not supported on this platform";
		// return null;
		// #end

		// #end
	}

}
