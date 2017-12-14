package iron.format.pako;

import haxe.io.UInt8Array;
import haxe.zip.FlushMode;
import iron.format.pako.Deflate.DeflateOptions;
import iron.format.pako.Inflate.InflateOptions;
import iron.format.pako.zlib.Deflate as ZlibDeflate;
import iron.format.pako.utils.Common;
import iron.format.pako.zlib.Messages;
import iron.format.pako.zlib.ZStream;
import iron.format.pako.zlib.Constants;
import iron.format.pako.zlib.Constants.CompressionLevel;
import iron.format.pako.zlib.GZHeader;


class Pako
{
  /** Version of ported pako lib (https://github.com/nodeca/pako). */
  inline static public var VERSION:String = "1.0.4";
  
  /**
   * deflate(data[, options]) -> Uint8Array|Array|String
   * - data (Uint8Array|Array|String): input data to compress.
   * - options (Object): zlib deflate options.
   *
   * Compress `data` with deflate alrorythm and `options`.
   *
   * Supported options are:
   *
   * - level
   * - windowBits
   * - memLevel
   * - strategy
   *
   * [http://zlib.net/manual.html#Advanced](http://zlib.net/manual.html#Advanced)
   * for more information on these.
   *
   * Sugar (options):
   *
   * - `raw` (Boolean) - say that we work with raw stream, if you don't wish to specify
   *   negative windowBits implicitly.
   * - `to` (String) - if equal to 'string', then result will be "binary string"
   *    (each char code [0..255])
   *
   * ##### Example:
   *
   * ```javascript
   * var pako = require('pako')
   *   , data = Uint8Array([1,2,3,4,5,6,7,8,9]);
   *
   * console.log(iron.format.pako.deflate(data));
   * ```
   **/
  static public function deflate(input:UInt8Array, ?options:DeflateOptions) {
    var deflator = new Deflate(options);

    //NOTE(hx): change here if we accept mode as Bool too
    deflator.push(input, Flush.Z_FINISH);

    // This should never happen, if you don't cheat with options :)
    if (deflator.err != 0) {
      throw (deflator.msg != '' ? deflator.msg : Messages.get(deflator.err));
    }

    return deflator.result;
  }


  /**
   * deflateRaw(data[, options]) -> Uint8Array|Array|String
   * - data (Uint8Array|Array|String): input data to compress.
   * - options (Object): zlib deflate options.
   *
   * The same as [[deflate]], but creates raw data, without wrapper
   * (header and adler32 crc).
   **/
  static public function deflateRaw(input:UInt8Array, ?options:DeflateOptions) {
    if (options == null) options = { };
    options.raw = true;
    return deflate(input, options);
  }


  /**
   * gzip(data[, options]) -> Uint8Array|Array|String
   * - data (Uint8Array|Array|String): input data to compress.
   * - options (Object): zlib deflate options.
   *
   * The same as [[deflate]], but create gzip wrapper instead of
   * deflate one.
   **/
  static public function gzip(input:UInt8Array, ?options:DeflateOptions) {
    if (options == null) options = { };
    options.gzip = true;
    return deflate(input, options);
  }
  /**
   * inflate(data[, options]) -> Uint8Array|Array|String
   * - data (Uint8Array|Array|String): input data to decompress.
   * - options (Object): zlib inflate options.
   *
   * Decompress `data` with inflate/ungzip and `options`. Autodetect
   * format via wrapper header by default. That's why we don't provide
   * separate `ungzip` method.
   *
   * Supported options are:
   *
   * - windowBits
   *
   * [http://zlib.net/manual.html#Advanced](http://zlib.net/manual.html#Advanced)
   * for more information.
   *
   * Sugar (options):
   *
   * - `raw` (Boolean) - say that we work with raw stream, if you don't wish to specify
   *   negative windowBits implicitly.
   * - `to` (String) - if equal to 'string', then result will be converted
   *   from utf8 to utf16 (javascript) string. When string output requested,
   *   chunk length can differ from `chunkSize`, depending on content.
   *
   *
   * ##### Example:
   *
   * ```javascript
   * var pako = require('pako')
   *   , input = iron.format.pako.deflate([1,2,3,4,5,6,7,8,9])
   *   , output;
   *
   * try {
   *   output = iron.format.pako.inflate(input);
   * } catch (err)
   *   console.log(err);
   * }
   * ```
   **/
  static public function inflate(input:UInt8Array, ?options:InflateOptions) {
    var inflator = new Inflate(options);

    inflator.push(input, true);

    // This should never happen, if you don't cheat with options :)
    if (inflator.err != ErrorStatus.Z_OK) {
      throw (inflator.msg != '' ? inflator.msg : Messages.get(inflator.err));
    }

    return inflator.result;
  }


  /**
   * inflateRaw(data[, options]) -> Uint8Array|Array|String
   * - data (Uint8Array|Array|String): input data to decompress.
   * - options (Object): zlib inflate options.
   *
   * The same as [[inflate]], but creates raw data, without wrapper
   * (header and adler32 crc).
   **/
  static public function inflateRaw(input:UInt8Array, ?options:InflateOptions) {
    if (options == null) options = { };
    options.raw = true;
    return inflate(input, options);
  }


  /**
   * ungzip(data[, options]) -> Uint8Array|Array|String
   * - data (Uint8Array|Array|String): input data to decompress.
   * - options (Object): zlib inflate options.
   *
   * Just shortcut to [[inflate]], because it autodetects format
   * by header.content. Done for convenience.
   **/
}

/*
exports.Deflate = Deflate;
exports.deflate = deflate;
exports.deflateRaw = deflateRaw;
exports.gzip = gzip;
*/