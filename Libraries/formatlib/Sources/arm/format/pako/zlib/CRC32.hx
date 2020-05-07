package arm.format.pako.zlib;

import haxe.ds.Vector;
import haxe.io.UInt8Array;

// Note: we can't get significant speed boost here.
// So write code to minimize size - no pregenerated tables
// and array tools dependencies.

class CRC32
{
  // Use ordinary array, since untyped makes no boost here
  static function makeTable():Vector<Int> {
    var c, table = new Vector<Int>(256);

    for (n in 0...256) {
      c = n;
      for (k in 0...8) {
        c = ((c&1 == 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
      }
      table[n] = c;
    }

    return table;
  }

  // Create table on load. Just 255 signed longs. Not a problem.
  static var crcTable:Vector<Int> = makeTable();


  static public function crc32(crc:Int, buf:UInt8Array, len:Int, pos:Int):Int {
    var t = crcTable,
        end = pos + len;

    crc = crc ^ (-1);

    for (i in pos...end) {
      crc = (crc >>> 8) ^ t[(crc ^ buf[i]) & 0xFF];
    }

    return (crc ^ (-1)); // >>> 0;
  }
}