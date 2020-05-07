package arm.format.pako.zlib;

//NOTE(hx): reconvert these to enum abstracts over Int (watching out for recursion and comparisons)

class Constants { }

/* Allowed flush values; see deflate() and inflate() below for details */
class Flush
{
  static inline public var Z_NO_FLUSH:Int =         0;
  static inline public var Z_PARTIAL_FLUSH:Int =    1;
  static inline public var Z_SYNC_FLUSH:Int =       2;
  static inline public var Z_FULL_FLUSH:Int =       3;
  static inline public var Z_FINISH:Int =           4;
  static inline public var Z_BLOCK:Int =            5;
  static inline public var Z_TREES:Int =            6;
}

/* Return codes for the compression/decompression functions. Negative values
* are errors, positive values are used for special but normal events.
*/
class ErrorStatus
{
  static inline public var Z_OK:Int =               0;
  static inline public var Z_STREAM_END:Int =       1;
  static inline public var Z_NEED_DICT:Int =        2;
  static inline public var Z_ERRNO:Int =           -1;
  static inline public var Z_STREAM_ERROR:Int =    -2;
  static inline public var Z_DATA_ERROR:Int =      -3;
  static inline public var Z_MEM_ERROR:Int =       -4; //NOTE(hx): commented out in pako.js
  static inline public var Z_BUF_ERROR:Int =       -5;
  static inline public var Z_VERSION_ERROR:Int =   -6; //NOTE(hx): commented out in pako.js
}

/* compression levels */
class CompressionLevel
{
  static inline public var Z_NO_COMPRESSION:Int =         0;
  static inline public var Z_BEST_SPEED:Int =             1;
  static inline public var Z_BEST_COMPRESSION:Int =       9;
  static inline public var Z_DEFAULT_COMPRESSION:Int =   -1;
}

class Strategy
{
  static inline public var Z_FILTERED:Int =               1;
  static inline public var Z_HUFFMAN_ONLY:Int =           2;
  static inline public var Z_RLE:Int =                    3;
  static inline public var Z_FIXED:Int =                  4;
  static inline public var Z_DEFAULT_STRATEGY:Int =       0;
}

/* Possible values of the data_type field (though see inflate()) */
class DataType
{
  static inline public var Z_BINARY:Int =                 0;
  static inline public var Z_TEXT:Int =                   1;
  //static inline public var Z_ASCII:Int =                1; // = Z_TEXT (deprecated)
  static inline public var Z_UNKNOWN:Int =                2;
}

/* The deflate compression method */
class Method
{
  static inline public var Z_DEFLATED:Int =               8;
  //static inline public var Z_NULL:Int =                 null; // Use -1 or null inline, depending on variable type
}
