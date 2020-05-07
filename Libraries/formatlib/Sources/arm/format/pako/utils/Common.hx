package arm.format.pako.utils;

import haxe.io.ArrayBufferView;
import haxe.io.UInt8Array;

class Common
{
  //NOTE(hx): blit (reset pos to respective offsets?)
  static inline public function arraySet(dest:ArrayBufferView, src:ArrayBufferView, src_offs:Int, len:Int, dest_offs:Int) {
    dest.buffer.blit(dest.byteOffset + dest_offs, src.buffer, src.byteOffset + src_offs, len);
  }

  //NOTE(hx): moved here from Trees and Deflate
  static inline public function zero(buf:ArrayBufferView) {
    var start = buf.byteOffset;
    var len = buf.byteLength;
    buf.buffer.fill(start, len, 0);
  }

  //NOTE(hx): if ArrayBufferView.EMULATED it will be a copy
  // reduce buffer size, avoiding mem copy
  static inline public function shrinkBuf(buf:UInt8Array, size:Int) {
    if (buf.length != size) {
      buf = cast buf.subarray(0, size);
    }
    return buf;
  }

  //NOTE(hx): blit
  // Join array of chunks to single array.
  static public function flattenChunks(chunks:Array<UInt8Array>) {
    var i, l, len, pos, chunk:UInt8Array, result:UInt8Array;

    // calculate data length
    len = 0;
    l = chunks.length;
    for (i in 0...l) {
      len += chunks[i].length;
    }

    // join chunks
    result = new UInt8Array(len);
    pos = 0;
    for (i in 0...l) {
      chunk = chunks[i];
      result.view.buffer.blit(pos, chunk.view.buffer, 0, chunk.length);
      pos += chunk.length;
    }

    return result;
  }
}