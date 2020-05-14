/*
 * format - haXe File Formats
 *
 *  JPG File Format
 *  Copyright (C) 2007-2009 Thibault Imbert, AS3-to-haXe by Michel Oster
 *
 * Copyright (c) 2009, The haXe Project Contributors
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

class JpgWriter {
	var ZigZag: Array<Int>;

	// Static table initialization
	function initZigZag() {
		ZigZag = [
			 0, 1, 5, 6,14,15,27,28,
			 2, 4, 7,13,16,26,29,42,
			 3, 8,12,17,25,30,41,43,
			 9,11,18,24,31,40,44,53,
			10,19,23,32,39,45,52,54,
			20,22,33,38,46,51,55,60,
			21,34,37,47,50,56,59,61,
			35,36,48,49,57,58,62,63
		];
	}

	var YTable: Array<Int>;     // = new Array(64);
	var UVTable: Array<Int>;    // = new Array(64);
	var fdtbl_Y: Array<Float>;    // = new Array(64);
	var fdtbl_UV: Array<Float>;   // = new Array(64);

	function initQuantTables(sf: Int) {
		var YQT: Array<Int> = [
			16, 11, 10, 16, 24, 40, 51, 61,
			12, 12, 14, 19, 26, 58, 60, 55,
			14, 13, 16, 24, 40, 57, 69, 56,
			14, 17, 22, 29, 51, 87, 80, 62,
			18, 22, 37, 56, 68,109,103, 77,
			24, 35, 55, 64, 81,104,113, 92,
			49, 64, 78, 87,103,121,120,101,
			72, 92, 95, 98,112,100,103, 99
		];
		for (i in 0...64) {
			var t: Int = Math.floor( (YQT[i] * sf + 50) / 100 );
			if( t < 1 ) t = 1;
			else if( t > 255 ) t = 255;
			YTable[ ZigZag[i] ] = t;
		}
		var UVQT: Array<Int> = [
			17, 18, 24, 47, 99, 99, 99, 99,
			18, 21, 26, 66, 99, 99, 99, 99,
			24, 26, 56, 99, 99, 99, 99, 99,
			47, 66, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99
		];
		for( j in 0...64 ) {
			var u: Int = Math.floor( (UVQT[j] * sf + 50) / 100 );
			if( u < 1 ) u = 1;
			else if( u > 255 ) u = 255;
			UVTable[ ZigZag[j] ] = u;
		}
		var aasf: Array<Float> = [
			1.0, 1.387039845, 1.306562965, 1.175875602,
			1.0, 0.785694958, 0.541196100, 0.275899379
		];
		var k = 0;
		for( row in 0...8 ) {
			for( col in 0...8 ) {
				fdtbl_Y[k]  = (1.0 / (YTable [ZigZag[k]] * aasf[row] * aasf[col] * 8.0));
				fdtbl_UV[k] = (1.0 / (UVTable[ZigZag[k]] * aasf[row] * aasf[col] * 8.0));
				k++;
			}
		}
	}

	var std_dc_luminance_nrcodes: Array<Int>;
	var std_dc_luminance_values: haxe.io.Bytes;
	var std_ac_luminance_nrcodes: Array<Int>;
	var std_ac_luminance_values: haxe.io.Bytes;

	function initLuminance() {
		std_dc_luminance_nrcodes = [0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0];
		std_dc_luminance_values = strIntsToBytes( '0,1,2,3,4,5,6,7,8,9,10,11' );
		std_ac_luminance_nrcodes = [0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d];
		std_ac_luminance_values = strIntsToBytes(
			'0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,' +
			'0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,' +
			'0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,' +
			'0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,' +
			'0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,' +
			'0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,' +
			'0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,' +
			'0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,' +
			'0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,' +
			'0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,' +
			'0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,' +
			'0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,' +
			'0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,' +
			'0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,' +
			'0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,' +
			'0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,' +
			'0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,' +
			'0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,' +
			'0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,' +
			'0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,' +
			'0xf9,0xfa'
		);
	}

	function strIntsToBytes( s: String ) {
		var len = s.length;
		var b = new haxe.io.BytesBuffer();
		var val = 0;
		var i = 0;
		for( j in 0...len ) {
			if( s.charAt( j ) == ',' ) {
				val = Std.parseInt( s.substr(i, j - i) );
				b.addByte( val );
				i = j + 1;
			}
		}
		if( i < len ) {
			val = Std.parseInt( s.substr(i) );
			b.addByte( val );
		}
		return b.getBytes();
	}

	var std_dc_chrominance_nrcodes: Array<Int>;
	var std_dc_chrominance_values: haxe.io.Bytes;
	var std_ac_chrominance_nrcodes: Array<Int>;
	var std_ac_chrominance_values: haxe.io.Bytes;

	function initChrominance() {
		std_dc_chrominance_nrcodes = [0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0];
		std_dc_chrominance_values = strIntsToBytes( '0,1,2,3,4,5,6,7,8,9,10,11' );
		std_ac_chrominance_nrcodes = [0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77];
		std_ac_chrominance_values = strIntsToBytes(
			'0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,' +
			'0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,' +
			'0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,' +
			'0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,' +
			'0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,' +
			'0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,' +
			'0x27,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,' +
			'0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,' +
			'0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,' +
			'0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,' +
			'0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,' +
			'0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,' +
			'0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,' +
			'0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,' +
			'0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,' +
			'0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,' +
			'0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,' +
			'0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,' +
			'0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,' +
			'0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,' +
			'0xf9,0xfa'
		);
	}

	var YDC_HT: Map<Int,BitString>;
	var UVDC_HT: Map<Int,BitString>;
	var YAC_HT: Map<Int,BitString>;
	var UVAC_HT: Map<Int,BitString>;

	function initHuffmanTbl() {
		YDC_HT = computeHuffmanTbl(std_dc_luminance_nrcodes, std_dc_luminance_values);
		UVDC_HT = computeHuffmanTbl(std_dc_chrominance_nrcodes, std_dc_chrominance_values);
		YAC_HT = computeHuffmanTbl(std_ac_luminance_nrcodes, std_ac_luminance_values);
		UVAC_HT = computeHuffmanTbl(std_ac_chrominance_nrcodes, std_ac_chrominance_values);
	}

	function computeHuffmanTbl(nrcodes: Array<Int>, std_table: haxe.io.Bytes): Map<Int,BitString> {
		var codevalue = 0;
		var pos_in_table = 0;
		var HT: Map<Int,BitString> = new Map();
		for( k in 1...17 ) {
			var end = nrcodes[k];
			for( j in 0...end ) {
				var idx: Int = std_table.get( pos_in_table );
				HT.set( idx, new BitString( k, codevalue ) );
				pos_in_table++;
				codevalue++;
			}
			codevalue *= 2;
		}
		return HT;
	}

	var bitcode: Map<Int,BitString>;
	var category: Map<Int,Int>;

	function initCategoryNumber() {
		var nrlower = 1;
		var nrupper = 2;
		var idx: Int;
		for (cat in 1...16) {
			//Positive numbers
			for( nr in nrlower...nrupper ) {
				idx = 32767 + nr;
				category.set( idx, cat );
				bitcode.set( idx, new BitString( cat, nr ) );
			}
			//Negative numbers
			var nrneg: Int = -(nrupper - 1);
			while( nrneg <= -nrlower ) {
				idx = 32767 + nrneg;
				category.set( idx, cat );
				bitcode.set( idx, new BitString( cat, nrupper - 1 + nrneg ) );
				nrneg++;
			}
			nrlower <<= 1;
			nrupper <<= 1;
		}
	}

	// IO functions
	var byteout: haxe.io.Output;
	var bytenew: Int;
	var bytepos: Int;

	function writeBits(bs: BitString) {
		var value: Int = bs.val;
		var posval: Int = bs.len - 1;
		while( posval >= 0 ) {
			//if (value & uint(1 << posval) ) {
			if( (value & (1 << posval)) != 0 ) {  //<- CORRECT ?
				//bytenew |= uint(1 << bytepos);
				bytenew |= (1 << bytepos);
			}
			posval--;
			bytepos--;
			if( bytepos < 0 ) {
				if( bytenew == 0xFF ) {
					b(0xFF);
					b(0);
				}
				else {
					b(bytenew);
				}
				bytepos = 7;
				bytenew = 0;
			}
		}
	}

	function writeWord( val: Int ) {
		b( (val >> 8) & 0xFF );
		b( val & 0xFF );
	}

	// DCT & quantization core

	function fDCTQuant(data: Array<Float>, fdtbl: Array<Float>): Array<Float> {
		/* Pass 1:  process rows. */
		var dataOff = 0;
		for (i in 0...8) {
			var tmp0: Float = data[dataOff + 0] + data[dataOff + 7];
			var tmp7: Float = data[dataOff + 0] - data[dataOff + 7];
			var tmp1: Float = data[dataOff + 1] + data[dataOff + 6];
			var tmp6: Float = data[dataOff + 1] - data[dataOff + 6];
			var tmp2: Float = data[dataOff + 2] + data[dataOff + 5];
			var tmp5: Float = data[dataOff + 2] - data[dataOff + 5];
			var tmp3: Float = data[dataOff + 3] + data[dataOff + 4];
			var tmp4: Float = data[dataOff + 3] - data[dataOff + 4];

			/* Even part */
			var tmp10: Float = tmp0 + tmp3;	/* phase 2 */
			var tmp13: Float = tmp0 - tmp3;
			var tmp11: Float = tmp1 + tmp2;
			var tmp12: Float = tmp1 - tmp2;

			data[dataOff + 0] = tmp10 + tmp11; /* phase 3 */
			data[dataOff + 4] = tmp10 - tmp11;

			var z1: Float = (tmp12 + tmp13) * 0.707106781; /* c4 */
			data[dataOff + 2] = tmp13 + z1; /* phase 5 */
			data[dataOff + 6] = tmp13 - z1;

			/* Odd part */
			tmp10 = tmp4 + tmp5; /* phase 2 */
			tmp11 = tmp5 + tmp6;
			tmp12 = tmp6 + tmp7;

			/* The rotator is modified from fig 4-8 to avoid extra negations. */
			var z5: Float = (tmp10 - tmp12) * 0.382683433; /* c6 */
			var z2: Float = 0.541196100 * tmp10 + z5; /* c2-c6 */
			var z4: Float = 1.306562965 * tmp12 + z5; /* c2+c6 */
			var z3: Float = tmp11 * 0.707106781; /* c4 */

			var z11: Float = tmp7 + z3;	/* phase 5 */
			var z13: Float = tmp7 - z3;

			data[dataOff + 5] = z13 + z2;	/* phase 6 */
			data[dataOff + 3] = z13 - z2;
			data[dataOff + 1] = z11 + z4;
			data[dataOff + 7] = z11 - z4;

			dataOff += 8; /* advance pointer to next row */
		}

		/* Pass 2:  process columns. */
		dataOff = 0;
		for (j in 0...8) {
			var tmp0p2: Float = data[dataOff+ 0] + data[dataOff+56];
			var tmp7p2: Float = data[dataOff+ 0] - data[dataOff+56];
			var tmp1p2: Float = data[dataOff+ 8] + data[dataOff+48];
			var tmp6p2: Float = data[dataOff+ 8] - data[dataOff+48];
			var tmp2p2: Float = data[dataOff+16] + data[dataOff+40];
			var tmp5p2: Float = data[dataOff+16] - data[dataOff+40];
			var tmp3p2: Float = data[dataOff+24] + data[dataOff+32];
			var tmp4p2: Float = data[dataOff+24] - data[dataOff+32];

			/* Even part */
			var tmp10p2: Float = tmp0p2 + tmp3p2;	/* phase 2 */
			var tmp13p2: Float = tmp0p2 - tmp3p2;
			var tmp11p2: Float = tmp1p2 + tmp2p2;
			var tmp12p2: Float = tmp1p2 - tmp2p2;

			data[dataOff+ 0] = tmp10p2 + tmp11p2; /* phase 3 */
			data[dataOff+32] = tmp10p2 - tmp11p2;

			var z1p2: Float = (tmp12p2 + tmp13p2) * 0.707106781; /* c4 */
			data[dataOff+16] = tmp13p2 + z1p2; /* phase 5 */
			data[dataOff+48] = tmp13p2 - z1p2;

			/* Odd part */
			tmp10p2 = tmp4p2 + tmp5p2; /* phase 2 */
			tmp11p2 = tmp5p2 + tmp6p2;
			tmp12p2 = tmp6p2 + tmp7p2;

			/* The rotator is modified from fig 4-8 to avoid extra negations. */
			var z5p2: Float = (tmp10p2 - tmp12p2) * 0.382683433; /* c6 */
			var z2p2: Float = 0.541196100 * tmp10p2 + z5p2; /* c2-c6 */
			var z4p2: Float = 1.306562965 * tmp12p2 + z5p2; /* c2+c6 */
			var z3p2: Float= tmp11p2 * 0.707106781; /* c4 */

			var z11p2: Float = tmp7p2 + z3p2;	/* phase 5 */
			var z13p2: Float = tmp7p2 - z3p2;

			data[dataOff+40] = z13p2 + z2p2; /* phase 6 */
			data[dataOff+24] = z13p2 - z2p2;
			data[dataOff+ 8] = z11p2 + z4p2;
			data[dataOff+56] = z11p2 - z4p2;

			dataOff++; /* advance pointer to next column */
		}

		// Quantize/descale the coefficients
		for (k in 0...64) {
			// Apply the quantization and scaling factor & Round to nearest integer
			data[k] = Math.round(data[k] * fdtbl[k]);
		}
		return data;
	}

	// Chunk writing

	inline function b(v) {
		byteout.writeByte(v);
	}

	function writeAPP0() {
		b(0xFF); b(0xE0);  //<- marker 0xFFE0
		b(0); b(16);  //<- length
		b("J".code);  // J
		b("F".code);
		b("I".code);
		b("F".code);
		b(0);
		b(1);  // versionhi
		b(1);  // versionlo
		b(0);  // xyunits
		b(0); b(1);  // xdensity
		b(0); b(1);  // ydensity
		b(0);  // thumbnwidth
		b(0);  // thumbnheight
	}
	function writeDQT() {
		b(0xFF); b(0xDB);  //<- marker 0xFFDB
		b(0); b(132);   //<- length
		b(0);
		for( j in 0...64 )
			b(YTable[j]);
		b(1);
		for( j in 0...64 )
			b(UVTable[j]);
	}
	function writeSOF0(width: Int, height: Int) {
		b(0xFF); b(0xC0);  //<- marker 0xFFC0
		b(0);  b(17);    //<- length, truecolor YUV JPG
		b(8);  // precision
		b( (height>>8) & 0xFF );
		b(  height & 0xFF );
		b(  (width>>8) & 0xFF );
		b(  width & 0xFF );
		b(3);     // nrofcomponents
		b(1);     // IdY
		b(0x11);  // HVY
		b(0);     // QTY
		b(2);     // IdU
		b(0x11);  // HVU
		b(1);     // QTU
		b(3);     // IdV
		b(0x11);  // HVV
		b(1);     // QTV
	}

	function writeDHT() {
		b(0xFF); b(0xC4);  //<- marker 0xFFC4
		b(0x01); b(0xA2);   //<- length
		b(0);  // HTYDCinfo
		for( j in 1...17 )
			b(std_dc_luminance_nrcodes[j]);
		byteout.write(std_dc_luminance_values);

		b(0x10);  // HTYACinfo
		for( j in 1...17 )
			b(std_ac_luminance_nrcodes[j]);
		byteout.write(std_ac_luminance_values);

		b(1);  // HTUDCinfo
		for( j in 1...17 )
			b(std_dc_chrominance_nrcodes[j]);
		byteout.write(std_dc_chrominance_values);

		b(0x11);  // HTUACinfo
		for( j in 1...17 )
			b(std_ac_chrominance_nrcodes[j]);
		byteout.write(std_ac_chrominance_values);
	}

	function writeSOS() {
		b(0xFF); b(0xDA);  //<- marker 0xFFDA
		b(0); b(12);   //<- length
		b(3);     // nrofcomponents
		b(1);     // IdY
		b(0);     // HTY
		b(2);     // IdU
		b(0x11);  // HTU
		b(3);     // IdV
		b(0x11);  // HTV
		b(0);     // Ss
		b(0x3F);  // Se
		b(0);     // Bf
	}

	// Core processing
	var DU: Array<Float>;  //<- initialized in function new JPEGEncoder()

	function processDU(CDU: Array<Float>, fdtbl: Array<Float>, DC: Float, HTDC: Map<Int,BitString>, HTAC: Map<Int,BitString>): Float {
		var EOB: BitString = HTAC.get( 0x00 );
		var M16zeroes: BitString = HTAC.get( 0xF0 );

		var DU_DCT: Array<Float> = fDCTQuant(CDU, fdtbl);
		//ZigZag reorder
		for (i in 0...64) {
			DU[ ZigZag[i] ] = DU_DCT[i];
		}
		var idx: Int;
		var Diff = Std.int( DU[0] - DC );
		DC = DU[0];
		//Encode DC
		if( Diff == 0 ) {
			writeBits( HTDC.get(0) ); // Diff might be 0
		} else {
			idx = 32767 + Diff;
			writeBits(HTDC.get( category.get( idx ) ));
			writeBits( bitcode.get( idx ) );
		}

		//Encode ACs
		var end0pos = 63;
		//for (; (end0pos>0)&&(DU[end0pos]==0); end0pos--) {  };
		while( (end0pos > 0) && ( DU[end0pos] == 0.0 ) ) end0pos--;

		//end0pos = first element in reverse order !=0
		if ( end0pos == 0 ) {
			writeBits(EOB);
			return DC;
		}
		var i = 1;
		while ( i <= end0pos ) {
			var startpos = i;
			//for (; (DU[i]==0) && (i<=end0pos); i++) {  };  <- it's a 'while' loop
			while( ( DU[i] == 0.0 ) && ( i <= end0pos ) ) i++;

			var nrzeroes: Int = i - startpos;
			if ( nrzeroes >= 16 ) {
				//for (var nrmarker: Int=1; nrmarker <= nrzeroes/16; nrmarker++) {
				for( nrmarker in 0...(nrzeroes >> 4) ) writeBits(M16zeroes);
				nrzeroes &= 0xF;
			}
			idx = 32767 + Std.int( DU[i] );  //<- line added
			writeBits( HTAC.get( nrzeroes * 16 + category.get( idx ) ) );
			writeBits( bitcode.get( idx ) );
			i++;
		}
		if( end0pos != 63 ) writeBits(EOB);
		return DC;
	}

	var YDU: Array<Float>;
	var UDU: Array<Float>;
	var VDU: Array<Float>;

	function ARGB2YUV(img: haxe.io.Bytes, width : Int, xpos: Int, ypos: Int) {
		var pos = 0;
		for( y in 0...8 ) {
			var offset = ((y + ypos) * width + xpos) << 2;
			for( x in 0...8 ) {
				offset++; // skip alpha
				var R = img.get(offset++);
				var G = img.get(offset++);
				var B = img.get(offset++);
				YDU[pos] = ((( 0.29900) * R + ( 0.58700) * G + ( 0.11400) * B)) -128;
				UDU[pos] = (((-0.16874) * R + (-0.33126) * G + ( 0.50000) * B));
				VDU[pos] = ((( 0.50000) * R + (-0.41869) * G + (-0.08131) * B));
				pos++;
			}
		}
	}

	public function new( out : haxe.io.Output ) {
	//begin : lines added to initialize variables
		YTable = new Array<Int>();
		UVTable = new Array<Int>();
		fdtbl_Y = new Array<Float>();
		fdtbl_UV = new Array<Float>();
		for (i in 0...64) {
			YTable.push(0); UVTable.push(0);
			fdtbl_Y.push(0.0); fdtbl_UV.push(0.0);
		}

		bitcode = new Map();  //<- 65535 elements <BitString>
		category = new Map();  //<- 65535 elements <Int>
		byteout = out;
		bytenew = 0;
		bytepos = 7;

		YDC_HT = new Map();
		UVDC_HT = new Map();
		YAC_HT = new Map();
		UVAC_HT = new Map();

		YDU = new Array<Float>();  //<- 64 elements
		UDU = new Array<Float>();
		VDU = new Array<Float>();
		DU = new Array<Float>();
		for (i in 0...64) {
			YDU.push(0.0); UDU.push(0.0); VDU.push(0.0); DU.push(0.0);
		}
		initZigZag();
		initLuminance();
		initChrominance();
	//end : lines added to initialize variables

		// Create tables
		initHuffmanTbl();
		initCategoryNumber();
	}

	public function write( image : JpgData, type = 0, off = 0, swapRG = false ) {
		// init quality table
		var quality = image.quality;
		if( quality <= 0 ) quality = 1;
		if( quality > 100 ) quality = 100;
		var sf =
			if( quality < 50 ) Std.int( 5000 / quality )
			else Std.int( 200 - quality * 2 );
		initQuantTables(sf);

		// Initialize bit writer
		bytenew = 0;
		bytepos = 7;

		var width = image.width;
		var height = image.height;
		// Add JPEG headers
		writeWord(0xFFD8); // SOI
		writeAPP0();
		writeDQT();
		writeSOF0( width, height );
		writeDHT();
		writeSOS();

		// Encode 8x8 macroblocks
		var DCY = 0.0;
		var DCU = 0.0;
		var DCV = 0.0;
		bytenew = 0;
		bytepos = 7;
		var ypos = 0;
		if (type == 0) {
			while( ypos < height ) {
				var xpos = 0;
				while( xpos < width ) {
					ARGB2YUV(image.pixels, width, xpos, ypos);
					DCY = processDU(YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
					DCU = processDU(UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
					DCV = processDU(VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
					xpos += 8;
				}
				ypos += 8;
			}
		}
		else if (type == 1 && !swapRG) {
			while( ypos < height ) {
				var xpos = 0;
				while( xpos < width ) {
					RGBA2YUV(image.pixels, width, xpos, ypos);
					DCY = processDU(YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
					DCU = processDU(UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
					DCV = processDU(VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
					xpos += 8;
				}
				ypos += 8;
			}
		}
		else if (type == 1 && swapRG) {
			while( ypos < height ) {
				var xpos = 0;
				while( xpos < width ) {
					BGRA2YUV(image.pixels, width, xpos, ypos);
					DCY = processDU(YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
					DCU = processDU(UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
					DCV = processDU(VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
					xpos += 8;
				}
				ypos += 8;
			}
		}
		else if (type == 2) {
			while( ypos < height ) {
				var xpos = 0;
				while( xpos < width ) {
					RRR2YUV(image.pixels, width, xpos, ypos, off);
					DCY = processDU(YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
					DCU = processDU(UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
					DCV = processDU(VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
					xpos += 8;
				}
				ypos += 8;
			}
		}

		// Do the bit alignment of the EOI marker
		if( bytepos >= 0 ) {
			var fillbits = new BitString( bytepos + 1, ( 1 << (bytepos + 1) ) - 1 );
			writeBits(fillbits);
		}

		writeWord(0xFFD9); //EOI
	}

	// armory
	function RGBA2YUV(img: haxe.io.Bytes, width : Int, xpos: Int, ypos: Int) {
		var pos = 0;
		for( y in 0...8 ) {
			var offset = ((y + ypos) * width + xpos) << 2;
			for( x in 0...8 ) {
				var R = img.get(offset++);
				var G = img.get(offset++);
				var B = img.get(offset++);
				offset++; // skip alpha
				YDU[pos] = ((( 0.29900) * R + ( 0.58700) * G + ( 0.11400) * B)) -128;
				UDU[pos] = (((-0.16874) * R + (-0.33126) * G + ( 0.50000) * B));
				VDU[pos] = ((( 0.50000) * R + (-0.41869) * G + (-0.08131) * B));
				pos++;
			}
		}
	}
	function BGRA2YUV(img: haxe.io.Bytes, width : Int, xpos: Int, ypos: Int) {
		var pos = 0;
		for( y in 0...8 ) {
			var offset = ((y + ypos) * width + xpos) << 2;
			for( x in 0...8 ) {
				var B = img.get(offset++);
				var G = img.get(offset++);
				var R = img.get(offset++);
				offset++; // skip alpha
				YDU[pos] = ((( 0.29900) * R + ( 0.58700) * G + ( 0.11400) * B)) -128;
				UDU[pos] = (((-0.16874) * R + (-0.33126) * G + ( 0.50000) * B));
				VDU[pos] = ((( 0.50000) * R + (-0.41869) * G + (-0.08131) * B));
				pos++;
			}
		}
	}
	function RRR2YUV(img: haxe.io.Bytes, width : Int, xpos: Int, ypos: Int, off: Int) {
		var pos = 0;
		for( y in 0...8 ) {
			var offset = ((y + ypos) * width + xpos) << 2;
			for( x in 0...8 ) {
				var R = img.get(offset + off);
				offset+=4;
				YDU[pos] = ((( 0.29900) * R + ( 0.58700) * R + ( 0.11400) * R)) -128;
				UDU[pos] = (((-0.16874) * R + (-0.33126) * R + ( 0.50000) * R));
				VDU[pos] = ((( 0.50000) * R + (-0.41869) * R + (-0.08131) * R));
				pos++;
			}
		}
	}
}

class BitString {
	public var len: Int;
	public var val: Int;

	public function new(l: Int, v: Int) {
		len = l;
		val = v;
	}
}
