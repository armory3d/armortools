package arm.format.pako.zlib;

import haxe.io.UInt8Array;


class GZHeader
{
  /* true if compressed data believed to be text */
  public var text:Bool       = false;
  /* modification time */
  public var time:Int        = 0;
  /* extra flags (not used when writing a gzip file) */
  public var xflags:Int      = 0;
  /* operating system */
  public var os:Int          = 0;
  /* pointer to extra field or Z_NULL if none */
  public var extra:UInt8Array = null;
  /* extra field length (valid if extra != Z_NULL) */
  public var extra_len:Int   = 0; // Actually, we don't need it in JS,
                       // but leave for few code modifications

  //
  // Setup limits is not necessary because in js we should not preallocate memory
  // for inflate use constant limit in 65536 bytes
  //

  /* space at extra (only when reading header) */
  // public var extra_max  = 0;
  /* pointer to zero-terminated file name or Z_NULL */
  public var name:String     = '';
  /* space at name (only when reading header) */
  // public var name_max   = 0;
  /* pointer to zero-terminated comment or Z_NULL */
  public var comment:String  = '';
  /* space at comment (only when reading header) */
  // public var comm_max   = 0;
  /* true if there was or will be a header crc */
  public var hcrc:Int        = 0;
  /* true when done reading gzip header (not used when writing a gzip file) */
  public var done:Bool       = false;

  //NOTE(hx): change GZHeader to accept a GZOptions typedef?
  public function new() { }
}