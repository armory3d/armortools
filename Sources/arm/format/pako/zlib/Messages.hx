package arm.format.pako.zlib;

import haxe.ds.IntMap;
import arm.format.pako.zlib.Constants;

//NOTE(hx): might as well refactor Error to be over string
class Messages
{
  static var map:IntMap<String> = [
    ErrorStatus.Z_NEED_DICT =>      'need dictionary',     /* Z_NEED_DICT       2  */
    ErrorStatus.Z_STREAM_END =>     'stream end',          /* Z_STREAM_END      1  */
    ErrorStatus.Z_OK =>             '',                    /* Z_OK              0  */
    ErrorStatus.Z_ERRNO =>          'file error',          /* Z_ERRNO         (-1) */
    ErrorStatus.Z_STREAM_ERROR =>   'stream error',        /* Z_STREAM_ERROR  (-2) */
    ErrorStatus.Z_DATA_ERROR =>     'data error',          /* Z_DATA_ERROR    (-3) */
    ErrorStatus.Z_MEM_ERROR =>      'insufficient memory', /* Z_MEM_ERROR     (-4) */
    ErrorStatus.Z_BUF_ERROR =>      'buffer error',        /* Z_BUF_ERROR     (-5) */
    ErrorStatus.Z_VERSION_ERROR =>  'incompatible version' /* Z_VERSION_ERROR (-6) */
  ];

  static public function get(error:Int) {
    return "ERROR: " + map.get(error);
  }
}
