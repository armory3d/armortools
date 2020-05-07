package arm.format.pako.zlib;

import haxe.io.Int32Array;
import haxe.io.UInt16Array;
import haxe.io.UInt8Array;
import arm.format.pako.utils.Common;
import arm.format.pako.zlib.Adler32;
import arm.format.pako.zlib.CRC32;
import arm.format.pako.zlib.InfFast;
import arm.format.pako.zlib.InfTrees;
import arm.format.pako.zlib.ZStream;
import arm.format.pako.zlib.Constants;


@:allow(arm.format.pako.zlib.InfFast)
class Inflate
{
  static inline var inflateInfo:String = 'pako inflate (from Nodeca project)';

  static inline var CODES = 0;
  static inline var LENS = 1;
  static inline var DISTS = 2;

  /* Public constants ==========================================================*/
  /* ===========================================================================*/


  /* Allowed flush values; see deflate() and inflate() below for details */
  /*
  //var Z_NO_FLUSH      = 0;
  //var Z_PARTIAL_FLUSH = 1;
  //var Z_SYNC_FLUSH    = 2;
  //var Z_FULL_FLUSH    = 3;
  var Z_FINISH        = 4;
  var Z_BLOCK         = 5;
  var Z_TREES         = 6;
  */

  /* Return codes for the compression/decompression functions. Negative values
   * are errors, positive values are used for special but normal events.
   */
  /*
  var Z_OK            = 0;
  var Z_STREAM_END    = 1;
  var Z_NEED_DICT     = 2;
  //var Z_ERRNO         = -1;
  var Z_STREAM_ERROR  = -2;
  var Z_DATA_ERROR    = -3;
  var Z_MEM_ERROR     = -4;
  var Z_BUF_ERROR     = -5;
  //var Z_VERSION_ERROR = -6;
  */

  /* The deflate compression method */
  //var Z_DEFLATED  = 8;


  /* STATES ====================================================================*/
  /* ===========================================================================*/


  static inline var    HEAD = 1;       /* i: waiting for magic header */
  static inline var    FLAGS = 2;      /* i: waiting for method and flags (gzip) */
  static inline var    TIME = 3;       /* i: waiting for modification time (gzip) */
  static inline var    OS = 4;         /* i: waiting for extra flags and operating system (gzip) */
  static inline var    EXLEN = 5;      /* i: waiting for extra length (gzip) */
  static inline var    EXTRA = 6;      /* i: waiting for extra bytes (gzip) */
  static inline var    NAME = 7;       /* i: waiting for end of file name (gzip) */
  static inline var    COMMENT = 8;    /* i: waiting for end of comment (gzip) */
  static inline var    HCRC = 9;       /* i: waiting for header crc (gzip) */
  static inline var    DICTID = 10;    /* i: waiting for dictionary check value */
  static inline var    DICT = 11;      /* waiting for inflateSetDictionary() call */
  static inline var        TYPE = 12;      /* i: waiting for type bits, including last-flag bit */
  static inline var        TYPEDO = 13;    /* i: same, but skip check to exit inflate on new block */
  static inline var        STORED = 14;    /* i: waiting for stored size (length and complement) */
  static inline var        COPY_ = 15;     /* i/o: same as COPY below, but only first time in */
  static inline var        COPY = 16;      /* i/o: waiting for input or output to copy stored block */
  static inline var        TABLE = 17;     /* i: waiting for dynamic block table lengths */
  static inline var        LENLENS = 18;   /* i: waiting for code length code lengths */
  static inline var        CODELENS = 19;  /* i: waiting for length/lit and distance code lengths */
  static inline var            LEN_ = 20;      /* i: same as LEN below, but only first time in */
  static inline var            LEN = 21;       /* i: waiting for length/lit/eob code */
  static inline var            LENEXT = 22;    /* i: waiting for length extra bits */
  static inline var            DIST = 23;      /* i: waiting for distance code */
  static inline var            DISTEXT = 24;   /* i: waiting for distance extra bits */
  static inline var            MATCH = 25;     /* o: waiting for output space to copy string */
  static inline var            LIT = 26;       /* o: waiting for output space to write literal */
  static inline var    CHECK = 27;     /* i: waiting for 32-bit check value */
  static inline var    LENGTH = 28;    /* i: waiting for 32-bit length (gzip) */
  static inline var    DONE = 29;      /* finished check, done -- remain here until reset */
  static inline var    BAD = 30;       /* got a data error -- remain here until reset */
  static inline var    MEM = 31;       /* got an inflate() memory error -- remain here until reset */
  static inline var    SYNC = 32;      /* looking for synchronization bytes to restart inflate() */

  /* ===========================================================================*/



  static inline var ENOUGH_LENS = 852;
  static inline var ENOUGH_DISTS = 592;
  //static inline var ENOUGH =  (ENOUGH_LENS+ENOUGH_DISTS);

  static inline var MAX_WBITS = 15;
  /* 32K LZ77 window */
  static inline var DEF_WBITS = MAX_WBITS;


  static inline function ZSWAP32(q:Int):Int {
    return  (((q >>> 24) & 0xff) +
            ((q >>> 8) & 0xff00) +
            ((q & 0xff00) << 8) +
            ((q & 0xff) << 24));
  }

  //NOTE(hx): InflateState moved to end of file

  static public function inflateResetKeep(strm:ZStream) {
    var state:InflateState;

    if (strm == null || strm.inflateState == null) { return ErrorStatus.Z_STREAM_ERROR; }
    state = strm.inflateState;
    strm.total_in = strm.total_out = state.total = 0;
    strm.msg = ''; /*Z_NULL*/
    if (state.wrap != 0) {       /* to support ill-conceived Java test suite */
      strm.adler = state.wrap & 1;
    }
    state.mode = HEAD;
    state.last = false;
    state.havedict = false;
    state.dmax = 32768;
    state.head = null/*Z_NULL*/;
    state.hold = 0;
    state.bits = 0;
    //state.lencode = state.distcode = state.next = state.codes;
    state.lencode = state.lendyn = new Int32Array(ENOUGH_LENS);
    state.distcode = state.distdyn = new Int32Array(ENOUGH_DISTS);

    state.sane = 1;
    state.back = -1;
    //Tracev((stderr, "inflate: reset\n"));
    return ErrorStatus.Z_OK;
  }

  static public function inflateReset(strm:ZStream) {
    var state;

    if (strm == null || strm.inflateState == null) { return ErrorStatus.Z_STREAM_ERROR; }
    state = strm.inflateState;
    state.wsize = 0;
    state.whave = 0;
    state.wnext = 0;
    return inflateResetKeep(strm);

  }

  static public function inflateReset2(strm:ZStream, windowBits) {
    var wrap;
    var state:InflateState;

    /* get the state */
    if (strm == null || strm.inflateState == null) { return ErrorStatus.Z_STREAM_ERROR; }
    state = strm.inflateState;

    /* extract wrap request from windowBits parameter */
    if (windowBits < 0) {
      wrap = 0;
      windowBits = -windowBits;
    }
    else {
      wrap = (windowBits >> 4) + 1;
      if (windowBits < 48) {
        windowBits &= 15;
      }
    }

    /* set number of window bits, free window if different */
    if (windowBits != 0 && (windowBits < 8 || windowBits > 15)) {
      return ErrorStatus.Z_STREAM_ERROR;
    }
    if (state.window != null && state.wbits != windowBits) {
      state.window = null;
    }

    /* update state and reset the rest of it */
    state.wrap = wrap;
    state.wbits = windowBits;
    return inflateReset(strm);
  }

  static public function inflateInit2(strm:ZStream, windowBits) {
    var ret;
    var state:InflateState;

    if (strm == null) { return ErrorStatus.Z_STREAM_ERROR; }
    //strm.msg = Z_NULL;                 /* in case we return an error */

    state = new InflateState();

    //if (state === Z_NULL) return Z_MEM_ERROR;
    //Tracev((stderr, "inflate: allocated\n"));
    strm.inflateState = state;
    state.window = null/*Z_NULL*/;
    ret = inflateReset2(strm, windowBits);
    if (ret != ErrorStatus.Z_OK) {
      strm.inflateState = null/*Z_NULL*/;
    }
    return ret;
  }

  static inline public function inflateInit(strm:ZStream) {
    return inflateInit2(strm, DEF_WBITS);
  }


  /*
   Return state with length and distance decoding tables and index sizes set to
   fixed code decoding.  Normally this returns fixed tables from inffixed.h.
   If BUILDFIXED is defined, then instead this routine builds the tables the
   first time it's called, and returns those tables the first time and
   thereafter.  This reduces the size of the code by about 2K bytes, in
   exchange for a little execution time.  However, BUILDFIXED should not be
   used for threaded applications, since the rewriting of the tables and virgin
   may not be thread-safe.
   */
  static var virgin:Bool = true;

  // We have no pointers in JS, so keep tables separate
  static var lenfix:Int32Array;
  static var distfix:Int32Array;

  static function fixedtables(state:InflateState) {
    /* build fixed huffman tables if first call (may not be thread safe) */
    if (virgin) {
      var sym;

      lenfix = new Int32Array(512);
      distfix = new Int32Array(32);

      /* literal/length table */
      sym = 0;
      while (sym < 144) { state.lens[sym++] = 8; }
      while (sym < 256) { state.lens[sym++] = 9; }
      while (sym < 280) { state.lens[sym++] = 7; }
      while (sym < 288) { state.lens[sym++] = 8; }

      InfTrees.inflate_table(LENS,  state.lens, 0, 288, lenfix,   0, state.work, {bits: 9});

      /* distance table */
      sym = 0;
      while (sym < 32) { state.lens[sym++] = 5; }

      InfTrees.inflate_table(DISTS, state.lens, 0, 32,   distfix, 0, state.work, {bits: 5});

      /* do this just once */
      virgin = false;
    }

    state.lencode = lenfix;
    state.lenbits = 9;
    state.distcode = distfix;
    state.distbits = 5;
  }


  /*
   Update the window with the last wsize (normally 32K) bytes written before
   returning.  If window does not exist yet, create it.  This is only called
   when a window is already in use, or when output has been written during this
   inflate call, but the end of the deflate stream has not been reached yet.
   It is also called to create a window for dictionary data when a dictionary
   is loaded.

   Providing output buffers larger than 32K to inflate() should provide a speed
   advantage, since only the last 32K of output is copied to the sliding window
   upon return from inflate(), and since all distances after the first 32K of
   output will fall in the output data, making match copies simpler and faster.
   The advantage may be dependent on the size of the processor's data caches.
   */
  static function updatewindow(strm:ZStream, src:UInt8Array, end, copy) {
    var dist;
    var state = strm.inflateState;

    /* if it hasn't been done already, allocate space for the window */
    if (state.window == null) {
      state.wsize = 1 << state.wbits;
      state.wnext = 0;
      state.whave = 0;

      state.window = new UInt8Array(state.wsize);
    }

    /* copy state->wsize or less output bytes into the circular window */
    if (copy >= state.wsize) {
      Common.arraySet(cast state.window,cast src, end - state.wsize, state.wsize, 0);
      state.wnext = 0;
      state.whave = state.wsize;
    }
    else {
      dist = state.wsize - state.wnext;
      if (dist > copy) {
        dist = copy;
      }
      //zmemcpy(state->window + state->wnext, end - copy, dist);
      Common.arraySet(cast state.window,cast src, end - copy, dist, state.wnext);
      copy -= dist;
      if (copy != 0) {
        //zmemcpy(state->window, end - copy, copy);
        Common.arraySet(cast state.window,cast src, end - copy, copy, 0);
        state.wnext = copy;
        state.whave = state.wsize;
      }
      else {
        state.wnext += dist;
        if (state.wnext == state.wsize) { state.wnext = 0; }
        if (state.whave < state.wsize) { state.whave += dist; }
      }
    }
    return 0;
  }

  static public function inflate(?strm:ZStream, flush:Int):Int {
    var state:InflateState;
    var input, output;          // input/output buffers
    var next;                   /* next input INDEX */
    var put;                    /* next output INDEX */
    var have, left;             /* available input and output */
    var hold = 0;                   /* bit buffer */
    var bits = 0;                   /* bits in bit buffer */
    var _in, _out;              /* save starting available input and output */
    var copy = 0;                   /* number of stored or match bytes to copy */
    var from;                   /* where to copy match bytes from */
    var from_source;
    var here = 0;               /* current decoding table entry */
    var here_bits = 0, here_op = 0, here_val = 0; // paked "here" denormalized (JS specific)
    //var last;                   /* parent table entry */
    var last_bits, last_op, last_val; // paked "last" denormalized (JS specific)
    var len = 0;                    /* length to copy for repeats, bits to drop */
    var ret:Int;                    /* return code */
    var hbuf = new UInt8Array(4);    /* buffer for gzip header crc calculation */
    var opts;

    var n; // temporary var for NEED_BITS

    var order = /* permutation of code lengths */
      [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15];


    if (strm == null || strm.inflateState == null || strm.output == null ||
        (strm.input == null && strm.avail_in != 0)) {
      return ErrorStatus.Z_STREAM_ERROR;
    }

    state = strm.inflateState;
    if (state.mode == TYPE) { state.mode = TYPEDO; }    /* skip check */


    //--- LOAD() ---
    put = strm.next_out;
    output = strm.output;
    left = strm.avail_out;
    next = strm.next_in;
    input = strm.input;
    have = strm.avail_in;
    hold = state.hold;
    bits = state.bits;
    //---

    _in = have;
    _out = left;
    ret = ErrorStatus.Z_OK;

    //NOTE(hx): this huge block is a state machine implemented with continue to emulate gotos.
    //A major difference with the js version is that, in haxe, switch cases don't have break,
    //so it's worth double checking this section
    var inf_leave = false;
    //inf_leave: // goto emulation
    while (true) {
      if (inf_leave) break;
      inf_leave = false;
      switch (state.mode) {
      case HEAD:
        if (state.wrap == 0) {
          state.mode = TYPEDO;
          continue; // inf_leave
        }
        //=== NEEDBITS(16);
        while (bits < 16) {
          if (have == 0) {
            inf_leave = true;
            break; // out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        if ((state.wrap & 2 != 0) && hold == 0x8b1f) {  /* gzip header */
          state.check = 0/*crc32(0L, Z_NULL, 0)*/;
          //=== CRC2(state.check, hold);
          hbuf[0] = hold & 0xff;
          hbuf[1] = (hold >>> 8) & 0xff;
          state.check = CRC32.crc32(state.check, hbuf, 2, 0);
          //===//

          //=== INITBITS();
          hold = 0;
          bits = 0;
          //===//
          state.mode = FLAGS;
          continue; //inf_leave
        }
        state.flags = 0;           /* expect zlib header */
        if (state.head != null) {
          state.head.done = false;
        }
        //NOTE(hx): check bitwise ops
        if (!(state.wrap & 1 == 1) ||   /* check if zlib header allowed */
          ((((hold & 0xff)/*BITS(8)*/ << 8) + (hold >> 8)) % 31) != 0) {
          strm.msg = 'incorrect header check';
          state.mode = BAD;
          continue; //inf_leave
        }
        if ((hold & 0x0f)/*BITS(4)*/ != Method.Z_DEFLATED) {
          strm.msg = 'unknown compression method';
          state.mode = BAD;
          continue; //inf_leave
        }
        //--- DROPBITS(4) ---//
        hold >>>= 4;
        bits -= 4;
        //---//
        len = (hold & 0x0f)/*BITS(4)*/ + 8;
        if (state.wbits == 0) {
          state.wbits = len;
        }
        else if (len > state.wbits) {
          strm.msg = 'invalid window size';
          state.mode = BAD;
          continue; //inf_leave
        }
        state.dmax = 1 << len;
        //Tracev((stderr, "inflate:   zlib header ok\n"));
        strm.adler = state.check = 1/*adler32(0L, Z_NULL, 0)*/;
        state.mode = hold & 0x200 != 0 ? DICTID : TYPE;
        //=== INITBITS();
        hold = 0;
        bits = 0;
        //===//
      case FLAGS:
        //=== NEEDBITS(16); */
        while (bits < 16) {
          if (have == 0) {
            inf_leave = true;
            break; // out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        state.flags = hold;
        if ((state.flags & 0xff) != Method.Z_DEFLATED) {
          strm.msg = 'unknown compression method';
          state.mode = BAD;
          continue; //inf_leave
        }
        if (state.flags & 0xe000 != 0) {
          strm.msg = 'unknown header flags set';
          state.mode = BAD;
          continue; //inf_leave
        }
        if (state.head != null) {
          state.head.text = ((hold >> 8) & 1 == 1);
        }
        if (state.flags & 0x0200 != 0) {
          //=== CRC2(state.check, hold);
          hbuf[0] = hold & 0xff;
          hbuf[1] = (hold >>> 8) & 0xff;
          state.check = CRC32.crc32(state.check, hbuf, 2, 0);
          //===//
        }
        //=== INITBITS();
        hold = 0;
        bits = 0;
        //===//
        state.mode = TIME;
        /* falls through */
      case TIME:
        //=== NEEDBITS(32); */
        while (bits < 32) {
          if (have == 0) {
            inf_leave = true;
            break; // out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        if (state.head != null) {
          state.head.time = hold;
        }
        if (state.flags & 0x0200 != 0) {
          //=== CRC4(state.check, hold)
          hbuf[0] = hold & 0xff;
          hbuf[1] = (hold >>> 8) & 0xff;
          hbuf[2] = (hold >>> 16) & 0xff;
          hbuf[3] = (hold >>> 24) & 0xff;
          state.check = CRC32.crc32(state.check, hbuf, 4, 0);
          //===
        }
        //=== INITBITS();
        hold = 0;
        bits = 0;
        //===//
        state.mode = OS;
        /* falls through */
      case OS:
        //=== NEEDBITS(16); */
        while (bits < 16) {
          if (have == 0) {
            inf_leave = true;
            break; // out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        if (state.head != null) {
          state.head.xflags = (hold & 0xff);
          state.head.os = (hold >> 8);
        }
        if (state.flags & 0x0200 != 0) {
          //=== CRC2(state.check, hold);
          hbuf[0] = hold & 0xff;
          hbuf[1] = (hold >>> 8) & 0xff;
          state.check = CRC32.crc32(state.check, hbuf, 2, 0);
          //===//
        }
        //=== INITBITS();
        hold = 0;
        bits = 0;
        //===//
        state.mode = EXLEN;
        /* falls through */
      case EXLEN:
        if (state.flags & 0x0400 != 0) {
          //=== NEEDBITS(16); */
          while (bits < 16) {
            if (have == 0) {
              inf_leave = true;
              break; // out of this while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
          }
          if (inf_leave) break; //inf_leave
          //===//
          state.length = hold;
          if (state.head != null) {
            state.head.extra_len = hold;
          }
          if (state.flags & 0x0200 != 0) {
            //=== CRC2(state.check, hold);
            hbuf[0] = hold & 0xff;
            hbuf[1] = (hold >>> 8) & 0xff;
            state.check = CRC32.crc32(state.check, hbuf, 2, 0);
            //===//
          }
          //=== INITBITS();
          hold = 0;
          bits = 0;
          //===//
        }
        else if (state.head != null) {
          state.head.extra = null/*Z_NULL*/;
        }
        state.mode = EXTRA;
        /* falls through */
      case EXTRA:
        if (state.flags & 0x0400 != 0) {
          copy = state.length;
          if (copy > have) { copy = have; }
          if (copy != 0) {
            if (state.head != null) {
              len = state.head.extra_len - state.length;
              if (state.head.extra == null) {
                // Use untyped array for more conveniend processing later
                state.head.extra = new UInt8Array(state.head.extra_len);
              }
              Common.arraySet(
                cast state.head.extra,
                cast input,
                next,
                // extra field is limited to 65536 bytes
                // - no need for additional size check
                copy,
                /*len + copy > state.head.extra_max - len ? state.head.extra_max : copy,*/
                len
              );
              //zmemcpy(state.head.extra + len, next,
              //        len + copy > state.head.extra_max ?
              //        state.head.extra_max - len : copy);
            }
            if (state.flags & 0x0200 != 0) {
              state.check = CRC32.crc32(state.check, input, copy, next);
            }
            have -= copy;
            next += copy;
            state.length -= copy;
          }
          if (state.length != 0) {
            inf_leave = true;
            break; //inf_leave
          }
        }
        state.length = 0;
        state.mode = NAME;
        /* falls through */
      case NAME:
        if (state.flags & 0x0800 != 0) {
          if (have == 0) {
            inf_leave = true;
            break; //inf_leave
          }
          copy = 0;
          do {
            // TODO: 2 or 1 bytes?
            len = input[next + copy++];
            /* use constant limit because in js we should not preallocate memory */
            if (state.head != null && len != 0 &&
                (state.length < 65536 /*state.head.name_max*/)) {
              state.head.name += String.fromCharCode(len);
            }
          } while (len != 0 && copy < have);

          if (state.flags & 0x0200 != 0) {
            state.check = CRC32.crc32(state.check, input, copy, next);
          }
          have -= copy;
          next += copy;
          if (len != 0) {
            inf_leave = true;
            break; //inf_leave
          }
        }
        else if (state.head != null) {
          state.head.name = null;
        }
        state.length = 0;
        state.mode = COMMENT;
        /* falls through */
      case COMMENT:
        if (state.flags & 0x1000 != 0) {
          if (have == 0) {
            inf_leave = true;
            break; //inf_leave
          }
          copy = 0;
          do {
            len = input[next + copy++];
            /* use constant limit because in js we should not preallocate memory */
            if (state.head != null && len != 0 &&
                (state.length < 65536 /*state.head.comm_max*/)) {
              state.head.comment += String.fromCharCode(len);
            }
          } while (len != 0 && copy < have);
          if (state.flags & 0x0200 != 0) {
            state.check = CRC32.crc32(state.check, input, copy, next);
          }
          have -= copy;
          next += copy;
          if (len != 0) {
            inf_leave = true;
            break; //inf_leave
          }
        }
        else if (state.head != null) {
          state.head.comment = null;
        }
        state.mode = HCRC;
        /* falls through */
      case HCRC:
        if (state.flags & 0x0200 != 0) {
          //=== NEEDBITS(16); */
          while (bits < 16) {
            if (have == 0) {
              inf_leave = true;
              break; //out of this while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
          }
          if (inf_leave) break; //inf_leave
          //===//
          if (hold != (state.check & 0xffff)) {
            strm.msg = 'header crc mismatch';
            state.mode = BAD;
            continue; //inf_leave
          }
          //=== INITBITS();
          hold = 0;
          bits = 0;
          //===//
        }
        if (state.head != null) {
          state.head.hcrc = ((state.flags >> 9) & 1);
          state.head.done = true;
        }
        strm.adler = state.check = 0 /*crc32(0L, Z_NULL, 0)*/;
        state.mode = TYPE;
      case DICTID:
        //=== NEEDBITS(32); */
        while (bits < 32) {
          if (have == 0) {
            inf_leave = true;
            break; //out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        strm.adler = state.check = ZSWAP32(hold);
        //=== INITBITS();
        hold = 0;
        bits = 0;
        //===//
        state.mode = DICT;
        /* falls through */
      case DICT:
        if (!state.havedict) {
          //--- RESTORE() ---
          strm.next_out = put;
          strm.avail_out = left;
          strm.next_in = next;
          strm.avail_in = have;
          state.hold = hold;
          state.bits = bits;
          //---
          return ErrorStatus.Z_NEED_DICT;
        }
        strm.adler = state.check = 1/*adler32(0L, Z_NULL, 0)*/;
        state.mode = TYPE;
        /* falls through */
      case TYPE:
        if (flush == Flush.Z_BLOCK || flush == Flush.Z_TREES) continue; //inf_leave
        state.mode = TYPEDO;
        /* falls through */
      case TYPEDO:
        if (state.last) {
          //--- BYTEBITS() ---//
          hold >>>= bits & 7;
          bits -= bits & 7;
          //---//
          state.mode = CHECK;
          continue; //inf_leave
        }
        //=== NEEDBITS(3); */
        while (bits < 3) {
          if (have == 0) {
            inf_leave = true;
            break; //out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        state.last = (hold & 0x01) == 1/*BITS(1)*/;
        //--- DROPBITS(1) ---//
        hold >>>= 1;
        bits -= 1;
        //---//

        switch ((hold & 0x03)/*BITS(2)*/) {
        case 0:                             /* stored block */
          //Tracev((stderr, "inflate:     stored block%s\n",
          //        state.last ? " (last)" : ""));
          state.mode = STORED;
        case 1:                             /* fixed block */
          fixedtables(state);
          //Tracev((stderr, "inflate:     fixed codes block%s\n",
          //        state.last ? " (last)" : ""));
          state.mode = LEN_;             /* decode codes */
          if (flush == Flush.Z_TREES) {
            //--- DROPBITS(2) ---//
            hold >>>= 2;
            bits -= 2;
            //---//
            inf_leave = true;
            break; //inf_leave
          }
        case 2:                             /* dynamic block */
          //Tracev((stderr, "inflate:     dynamic codes block%s\n",
          //        state.last ? " (last)" : ""));
          state.mode = TABLE;
        case 3:
          strm.msg = 'invalid block type';
          state.mode = BAD;
        }
        //--- DROPBITS(2) ---//
        hold >>>= 2;
        bits -= 2;
        //---//
      case STORED:
        //--- BYTEBITS() ---// /* go to byte boundary */
        hold >>>= bits & 7;
        bits -= bits & 7;
        //---//
        //=== NEEDBITS(32); */
        while (bits < 32) {
          if (have == 0) {
            inf_leave = true;
            break; //out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        if ((hold & 0xffff) != ((hold >>> 16) ^ 0xffff)) {
          strm.msg = 'invalid stored block lengths';
          state.mode = BAD;
          continue; //inf_leave
        }
        state.length = hold & 0xffff;
        //Tracev((stderr, "inflate:       stored length %u\n",
        //        state.length));
        //=== INITBITS();
        hold = 0;
        bits = 0;
        //===//
        state.mode = COPY_;
        if (flush == Flush.Z_TREES) {
          inf_leave = true;
          break; //inf_leave
        }
        /* falls through */
      case COPY_:
        state.mode = COPY;
        /* falls through */
      case COPY:
        copy = state.length;
        if (copy != 0) {
          if (copy > have) { copy = have; }
          if (copy > left) { copy = left; }
          if (copy == 0) {
            inf_leave = true;
            break; //inf_leave
          }
          //--- zmemcpy(put, next, copy); ---
          Common.arraySet(cast output, cast input, next, copy, put);
          //---//
          have -= copy;
          next += copy;
          left -= copy;
          put += copy;
          state.length -= copy;
          continue; //inf_leave
        }
        //Tracev((stderr, "inflate:       stored end\n"));
        state.mode = TYPE;
      case TABLE:
        //=== NEEDBITS(14); */
        while (bits < 14) {
          if (have == 0) {
            inf_leave = true;
            break; //out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
        }
        if (inf_leave) break; //inf_leave
        //===//
        state.nlen = (hold & 0x1f)/*BITS(5)*/ + 257;
        //--- DROPBITS(5) ---//
        hold >>>= 5;
        bits -= 5;
        //---//
        state.ndist = (hold & 0x1f)/*BITS(5)*/ + 1;
        //--- DROPBITS(5) ---//
        hold >>>= 5;
        bits -= 5;
        //---//
        state.ncode = (hold & 0x0f)/*BITS(4)*/ + 4;
        //--- DROPBITS(4) ---//
        hold >>>= 4;
        bits -= 4;
        //---//
  //#ifndef PKZIP_BUG_WORKAROUND
        if (state.nlen > 286 || state.ndist > 30) {
          strm.msg = 'too many length or distance symbols';
          state.mode = BAD;
          continue; //inf_leave
        }
  //#endif
        //Tracev((stderr, "inflate:       table sizes ok\n"));
        state.have = 0;
        state.mode = LENLENS;
        /* falls through */
      case LENLENS:
        while (state.have < state.ncode) {
          //=== NEEDBITS(3);
          while (bits < 3) {
            if (have == 0) {
              inf_leave = true;
              break; //out of inner while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
          }
          if (inf_leave) break; //out of outer while
          //===//
          state.lens[order[state.have++]] = (hold & 0x07);//BITS(3);
          //--- DROPBITS(3) ---//
          hold >>>= 3;
          bits -= 3;
          //---//
        }
        if (inf_leave) break; //inf_leave
        while (state.have < 19) {
          state.lens[order[state.have++]] = 0;
        }
        // We have separate tables & no pointers. 2 commented lines below not needed.
        //state.next = state.codes;
        //state.lencode = state.next;
        // Switch to use dynamic table
        state.lencode = state.lendyn;
        state.lenbits = 7;

        opts = {bits: state.lenbits};
        ret = InfTrees.inflate_table(CODES, state.lens, 0, 19, state.lencode, 0, state.work, opts);
        state.lenbits = opts.bits;

        if (ret != ErrorStatus.Z_OK) {
          strm.msg = 'invalid code lengths set';
          state.mode = BAD;
          continue; //inf_leave
        }
        //Tracev((stderr, "inflate:       code lengths ok\n"));
        state.have = 0;
        state.mode = CODELENS;
        /* falls through */
      case CODELENS:
        while (state.have < state.nlen + state.ndist) {
          while (true) {
            here = state.lencode[hold & ((1 << state.lenbits) - 1)];/*BITS(state.lenbits)*/
            here_bits = here >>> 24;
            here_op = (here >>> 16) & 0xff;
            here_val = here & 0xffff;

            if ((here_bits) <= bits) break; //out of inner while
            //--- PULLBYTE() ---//
            if (have == 0) {
              inf_leave = true;
              break; //out of inner while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
            //---//
          }
          if (inf_leave) break; //out of outer while
          if (here_val < 16) {
            //--- DROPBITS(here.bits) ---//
            hold = hold >>> here_bits;
            bits -= here_bits;
            //---//
            state.lens[state.have++] = here_val;
          }
          else {
            if (here_val == 16) {
              //=== NEEDBITS(here.bits + 2);
              n = here_bits + 2;
              while (bits < n) {
                if (have == 0) {
                  inf_leave = true;
                  break; //out of this while
                }
                have--;
                hold += input[next++] << bits;
                bits += 8;
              }
              if (inf_leave) break; //out of outer while
              //===//
              //--- DROPBITS(here.bits) ---//
              hold >>>= here_bits;
              bits -= here_bits;
              //---//
              if (state.have == 0) {
                strm.msg = 'invalid bit length repeat';
                state.mode = BAD;
                break; //out of outer while
              }
              len = state.lens[state.have - 1];
              copy = 3 + (hold & 0x03);//BITS(2);
              //--- DROPBITS(2) ---//
              hold >>>= 2;
              bits -= 2;
              //---//
            }
            else if (here_val == 17) {
              //=== NEEDBITS(here.bits + 3);
              n = here_bits + 3;
              while (bits < n) {
                if (have == 0) {
                  inf_leave = true;
                  break; //out of this while
                }
                have--;
                hold += input[next++] << bits;
                bits += 8;
              }
              if (inf_leave) break; //out of outer while
              //===//
              //--- DROPBITS(here.bits) ---//
              hold >>>= here_bits;
              bits -= here_bits;
              //---//
              len = 0;
              copy = 3 + (hold & 0x07);//BITS(3);
              //--- DROPBITS(3) ---//
              hold >>>= 3;
              bits -= 3;
              //---//
            }
            else {
              //=== NEEDBITS(here.bits + 7);
              n = here_bits + 7;
              while (bits < n) {
                if (have == 0) {
                  inf_leave = true;
                  break; //out of this while
                }
                have--;
                hold += input[next++] << bits;
                bits += 8;
              }
              if (inf_leave) break; //out of outer while
              //===//
              //--- DROPBITS(here.bits) ---//
              hold >>>= here_bits;
              bits -= here_bits;
              //---//
              len = 0;
              copy = 11 + (hold & 0x7f);//BITS(7);
              //--- DROPBITS(7) ---//
              hold >>>= 7;
              bits -= 7;
              //---//
            }
            if (state.have + copy > state.nlen + state.ndist) {
              strm.msg = 'invalid bit length repeat';
              state.mode = BAD;
              break; //out of outer while
            }
            while (copy-- != 0) {
              state.lens[state.have++] = len;
            }
          }
        }

        /* handle error breaks in while */
        if (inf_leave || state.mode == BAD) continue; //inf_leave

        /* check for end-of-block code (better have one) */
        if (state.lens[256] == 0) {
          strm.msg = 'invalid code -- missing end-of-block';
          state.mode = BAD;
          continue; //inf_leave
        }

        /* build code tables -- note: do not change the lenbits or distbits
           values here (9 and 6) without reading the comments in inftrees.h
           concerning the ENOUGH constants, which depend on those values */
        state.lenbits = 9;

        opts = {bits: state.lenbits};
        ret = InfTrees.inflate_table(LENS, state.lens, 0, state.nlen, state.lencode, 0, state.work, opts);
        // We have separate tables & no pointers. 2 commented lines below not needed.
        // state.next_index = opts.table_index;
        state.lenbits = opts.bits;
        // state.lencode = state.next;

        if (ret != ErrorStatus.Z_OK) {
          strm.msg = 'invalid literal/lengths set';
          state.mode = BAD;
          continue; //inf_leave
        }

        state.distbits = 6;
        //state.distcode.copy(state.codes);
        // Switch to use dynamic table
        state.distcode = state.distdyn;
        opts = {bits: state.distbits};
        ret = InfTrees.inflate_table(DISTS, state.lens, state.nlen, state.ndist, state.distcode, 0, state.work, opts);
        // We have separate tables & no pointers. 2 commented lines below not needed.
        // state.next_index = opts.table_index;
        state.distbits = opts.bits;
        // state.distcode = state.next;

        if (ret != ErrorStatus.Z_OK) {
          strm.msg = 'invalid distances set';
          state.mode = BAD;
          continue; //inf_leave
        }
        //Tracev((stderr, 'inflate:       codes ok\n'));
        state.mode = LEN_;
        if (flush == Flush.Z_TREES) {
          inf_leave = true;
          continue; //inf_leave
        }
        /* falls through */
      case LEN_:
        state.mode = LEN;
        /* falls through */
      case LEN:
        if (have >= 6 && left >= 258) {
          //--- RESTORE() ---
          strm.next_out = put;
          strm.avail_out = left;
          strm.next_in = next;
          strm.avail_in = have;
          state.hold = hold;
          state.bits = bits;
          //---
          InfFast.inflate_fast(strm, _out);
          //--- LOAD() ---
          put = strm.next_out;
          output = strm.output;
          left = strm.avail_out;
          next = strm.next_in;
          input = strm.input;
          have = strm.avail_in;
          hold = state.hold;
          bits = state.bits;
          //---

          if (state.mode == TYPE) {
            state.back = -1;
          }
          continue; //inf_leave
        }
        state.back = 0;
        while (true) {
          here = state.lencode[hold & ((1 << state.lenbits) -1)];  /*BITS(state.lenbits)*/
          here_bits = here >>> 24;
          here_op = (here >>> 16) & 0xff;
          here_val = here & 0xffff;

          if (here_bits <= bits) { break; }
          //--- PULLBYTE() ---//
          if (have == 0) {
            inf_leave = true;
            break; //out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
          //---//
        }
        if (inf_leave) continue; //inf_leave
        if (here_op != 0 && (here_op & 0xf0) == 0) {
          last_bits = here_bits;
          last_op = here_op;
          last_val = here_val;
          while (true) {
            here = state.lencode[last_val +
                    ((hold & ((1 << (last_bits + last_op)) -1))/*BITS(last.bits + last.op)*/ >> last_bits)];
            here_bits = here >>> 24;
            here_op = (here >>> 16) & 0xff;
            here_val = here & 0xffff;

            if ((last_bits + here_bits) <= bits) { break; }
            //--- PULLBYTE() ---//
            if (have == 0) {
              inf_leave = true;
              break; //out of this while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
            //---//
          }
          if (inf_leave) continue; //inf_leave
          //--- DROPBITS(last.bits) ---//
          hold >>>= last_bits;
          bits -= last_bits;
          //---//
          state.back += last_bits;
        }
        //--- DROPBITS(here.bits) ---//
        hold >>>= here_bits;
        bits -= here_bits;
        //---//
        state.back += here_bits;
        state.length = here_val;
        if (here_op == 0) {
          //Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
          //        "inflate:         literal '%c'\n" :
          //        "inflate:         literal 0x%02x\n", here.val));
          state.mode = LIT;
          continue; //inf_leave
        }
        if (here_op & 32 != 0) {
          //Tracevv((stderr, "inflate:         end of block\n"));
          state.back = -1;
          state.mode = TYPE;
          continue; //inf_leave
        }
        if (here_op & 64 != 0) {
          strm.msg = 'invalid literal/length code';
          state.mode = BAD;
          continue; //inf_leave
        }
        state.extra = here_op & 15;
        state.mode = LENEXT;
        /* falls through */
      case LENEXT:
        if (state.extra != 0) {
          //=== NEEDBITS(state.extra);
          n = state.extra;
          while (bits < n) {
            if (have == 0) {
              inf_leave = true;
              break; //out of this while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
          }
          if (inf_leave) continue; //inf_leave
          //===//
          state.length += hold & ((1 << state.extra) -1)/*BITS(state.extra)*/;
          //--- DROPBITS(state.extra) ---//
          hold >>>= state.extra;
          bits -= state.extra;
          //---//
          state.back += state.extra;
        }
        //Tracevv((stderr, "inflate:         length %u\n", state.length));
        state.was = state.length;
        state.mode = DIST;
        /* falls through */
      case DIST:
        while (true) {
          here = state.distcode[hold & ((1 << state.distbits) -1)];/*BITS(state.distbits)*/
          here_bits = here >>> 24;
          here_op = (here >>> 16) & 0xff;
          here_val = here & 0xffff;

          if ((here_bits) <= bits) { break; }
          //--- PULLBYTE() ---//
          if (have == 0) {
            inf_leave = true;
            break; //out of this while
          }
          have--;
          hold += input[next++] << bits;
          bits += 8;
          //---//
        }
        if (inf_leave) continue; //inf_leave
        if ((here_op & 0xf0) == 0) {
          last_bits = here_bits;
          last_op = here_op;
          last_val = here_val;
          while (true) {
            here = state.distcode[last_val +
                    ((hold & ((1 << (last_bits + last_op)) -1))/*BITS(last.bits + last.op)*/ >> last_bits)];
            here_bits = here >>> 24;
            here_op = (here >>> 16) & 0xff;
            here_val = here & 0xffff;

            if ((last_bits + here_bits) <= bits) { break; }
            //--- PULLBYTE() ---//
            if (have == 0) {
              inf_leave = true;
              break; //out of this while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
            //---//
          }
          if (inf_leave) continue; //inf_leave
          //--- DROPBITS(last.bits) ---//
          hold >>>= last_bits;
          bits -= last_bits;
          //---//
          state.back += last_bits;
        }
        //--- DROPBITS(here.bits) ---//
        hold >>>= here_bits;
        bits -= here_bits;
        //---//
        state.back += here_bits;
        if (here_op & 64 != 0) {
          strm.msg = 'invalid distance code';
          state.mode = BAD;
          continue; //inf_leave
        }
        state.offset = here_val;
        state.extra = (here_op) & 15;
        state.mode = DISTEXT;
        /* falls through */
      case DISTEXT:
        if (state.extra != 0) {
          //=== NEEDBITS(state.extra);
          n = state.extra;
          while (bits < n) {
            if (have == 0) {
              inf_leave = true;
              break; //out of this while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
          }
          if (inf_leave) continue; //inf_leave
          //===//
          state.offset += hold & ((1 << state.extra) -1)/*BITS(state.extra)*/;
          //--- DROPBITS(state.extra) ---//
          hold >>>= state.extra;
          bits -= state.extra;
          //---//
          state.back += state.extra;
        }
  //#ifdef INFLATE_STRICT
        if (state.offset > state.dmax) {
          strm.msg = 'invalid distance too far back';
          state.mode = BAD;
          continue; //inf_leave
        }
  //#endif
        //Tracevv((stderr, "inflate:         distance %u\n", state.offset));
        state.mode = MATCH;
        /* falls through */
      case MATCH:
        if (left == 0) {
          inf_leave = true;
          continue; //inf_leave
        }
        copy = _out - left;
        if (state.offset > copy) {         /* copy from window */
          copy = state.offset - copy;
          if (copy > state.whave) {
            if (state.sane != 0) {
              strm.msg = 'invalid distance too far back';
              state.mode = BAD;
              continue; //inf_leave
            }
  // (!) This block is disabled in zlib defailts,
  // don't enable it for binary compatibility
  //#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
  //          Trace((stderr, "inflate.c too far\n"));
  //          copy -= state.whave;
  //          if (copy > state.length) { copy = state.length; }
  //          if (copy > left) { copy = left; }
  //          left -= copy;
  //          state.length -= copy;
  //          do {
  //            output[put++] = 0;
  //          } while (--copy);
  //          if (state.length === 0) { state.mode = LEN; }
  //          break;
  //#endif
          }
          if (copy > state.wnext) {
            copy -= state.wnext;
            from = state.wsize - copy;
          }
          else {
            from = state.wnext - copy;
          }
          if (copy > state.length) { copy = state.length; }
          from_source = state.window;
        }
        else {                              /* copy from output */
          from_source = output;
          from = put - state.offset;
          copy = state.length;
        }
        if (copy > left) { copy = left; }
        left -= copy;
        state.length -= copy;
        do {
          output[put++] = from_source[from++];
        } while (--copy != 0);
        if (state.length == 0) { state.mode = LEN; }
      case LIT:
        if (left == 0) {
          inf_leave = true;
          continue; //inf_leave
        }
        output[put++] = state.length;
        left--;
        state.mode = LEN;
      case CHECK:
        if (state.wrap != 0) {
          //=== NEEDBITS(32);
          while (bits < 32) {
            if (have == 0) {
              inf_leave = true;
              break; //out of this while
            }
            have--;
            // Use '|' insdead of '+' to make sure that result is signed
            hold |= input[next++] << bits;
            bits += 8;
          }
          if (inf_leave) continue; //inf_leave
          //===//
          _out -= left;
          strm.total_out += _out;
          state.total += _out;
          if (_out != 0) {
            strm.adler = state.check =
                /*UPDATE(state.check, put - _out, _out);*/
                (state.flags != 0 ? CRC32.crc32(state.check, output, _out, put - _out) : Adler32.adler32(state.check, output, _out, put - _out));

          }
          _out = left;
          //NOTE(hx): force-clamp to 32bits (needed f.e. on python)
          hold = @:privateAccess haxe.Int32.clamp(hold);
          // NB: crc32 stored as signed 32-bit int, ZSWAP32 returns signed too
          if ((state.flags != 0 ? hold : ZSWAP32(hold)) != state.check) {
            strm.msg = 'incorrect data check';
            state.mode = BAD;
            continue; //inf_leave
          }
          //=== INITBITS();
          hold = 0;
          bits = 0;
          //===//
          //Tracev((stderr, "inflate:   check matches trailer\n"));
        }
        state.mode = LENGTH;
        /* falls through */
      case LENGTH:
        if (state.wrap != 0 && state.flags != 0) {
          //=== NEEDBITS(32);
          while (bits < 32) {
            if (have == 0) {
              inf_leave = true;
              break; //out of this while
            }
            have--;
            hold += input[next++] << bits;
            bits += 8;
          }
          if (inf_leave) continue; //inf_leave
          //===//
          if (hold != (state.total & 0xffffffff)) {
            strm.msg = 'incorrect length check';
            state.mode = BAD;
            continue; //inf_leave
          }
          //=== INITBITS();
          hold = 0;
          bits = 0;
          //===//
          //Tracev((stderr, "inflate:   length matches trailer\n"));
        }
        state.mode = DONE;
        /* falls through */
      case DONE:
        ret = ErrorStatus.Z_STREAM_END;
        inf_leave = true;
        continue; //inf_leave
      case BAD:
        ret = ErrorStatus.Z_DATA_ERROR;
        inf_leave = true;
        continue; //inf_leave
      case MEM:
        return ErrorStatus.Z_MEM_ERROR;
      case SYNC:
        return ErrorStatus.Z_STREAM_ERROR;
      default:
        return ErrorStatus.Z_STREAM_ERROR;
      }
    }

    // inf_leave <- here is real place for "goto inf_leave", emulated via inline and returns

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */

    //--- RESTORE() ---
    strm.next_out = put;
    strm.avail_out = left;
    strm.next_in = next;
    strm.avail_in = have;
    state.hold = hold;
    state.bits = bits;
    //---

    if (state.wsize != 0 || (_out != strm.avail_out && state.mode < BAD &&
                        (state.mode < CHECK || flush != Flush.Z_FINISH))) {
      if (updatewindow(strm, strm.output, strm.next_out, _out - strm.avail_out) != 0) {
        state.mode = MEM;
        return ErrorStatus.Z_MEM_ERROR;
      }
    }
    _in -= strm.avail_in;
    _out -= strm.avail_out;
    strm.total_in += _in;
    strm.total_out += _out;
    state.total += _out;
    if (state.wrap != 0 && _out != 0) {
      strm.adler = state.check = /*UPDATE(state.check, strm.next_out - _out, _out);*/
        (state.flags != 0 ? CRC32.crc32(state.check, output, _out, strm.next_out - _out) : Adler32.adler32(state.check, output, _out, strm.next_out - _out));
    }
    strm.data_type = state.bits + (state.last ? 64 : 0) +
                      (state.mode == TYPE ? 128 : 0) +
                      (state.mode == LEN_ || state.mode == COPY_ ? 256 : 0);
    if (((_in == 0 && _out == 0) || flush == Flush.Z_FINISH) && ret == ErrorStatus.Z_OK) {
      ret = ErrorStatus.Z_BUF_ERROR;
    }
    return ret;
  }

  static public function inflateEnd(?strm:ZStream) {

    if (strm == null || strm.inflateState == null /*|| strm->zfree == (free_func)0*/) {
      return ErrorStatus.Z_STREAM_ERROR;
    }

    var state = strm.inflateState;
    if (state.window != null) {
      state.window = null;
    }
    strm.inflateState = null;
    return ErrorStatus.Z_OK;
  }

  static public function inflateGetHeader(strm:ZStream, head) {
    var state:InflateState;

    /* check state */
    if (strm == null || strm.inflateState == null) { return ErrorStatus.Z_STREAM_ERROR; }
    state = strm.inflateState;
    if ((state.wrap & 2) == 0) { return ErrorStatus.Z_STREAM_ERROR; }

    /* save header structure */
    state.head = head;
    head.done = false;
    return ErrorStatus.Z_OK;
  }

  static public function inflateSetDictionary(strm:ZStream, dictionary:UInt8Array) {
    var dictLength = dictionary.length;

    var state:InflateState;
    var dictid:Int;
    var ret;

    /* check state */
    if (strm == null/* == Z_NULL */ || strm.inflateState == null/* == Z_NULL */) { return ErrorStatus.Z_STREAM_ERROR; }
    state = strm.inflateState;

    if (state.wrap != 0 && state.mode != DICT) {
      return ErrorStatus.Z_STREAM_ERROR;
    }

    /* check for correct dictionary identifier */
    if (state.mode == DICT) {
      dictid = 1; /* adler32(0, null, 0)*/
      /* dictid = adler32(dictid, dictionary, dictLength); */
      dictid = Adler32.adler32(dictid, dictionary, dictLength, 0);
      if (dictid != state.check) {
        return ErrorStatus.Z_DATA_ERROR;
      }
    }
    /* copy dictionary to window using updatewindow(), which will amend the
     existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary, dictLength, dictLength);
    if (ret != 0) {
      state.mode = MEM;
      return ErrorStatus.Z_MEM_ERROR;
    }
    state.havedict = true;
    // Tracev((stderr, "inflate:   dictionary set\n"));
    return ErrorStatus.Z_OK;
  }
}


@:allow(arm.format.pako.zlib.Inflate)
@:allow(arm.format.pako.zlib.InfFast)
class InflateState
{
  var mode:Int = 0;             /* current inflate mode */
  var last:Bool = false;          /* true if processing last block */
  var wrap:Int = 0;              /* bit 0 true for zlib, bit 1 true for gzip */
  var havedict:Bool = false;      /* true if dictionary provided */
  var flags:Int = 0;             /* gzip header method and flags (0 if zlib) */
  var dmax:Int = 0;              /* zlib header max distance (INFLATE_STRICT) */
  var check:Int = 0;             /* protected copy of check value */
  var total:Int = 0;             /* protected copy of output count */
  // TODO: may be {}
  var head:GZHeader = null;           /* where to save gzip header information */

  /* sliding window */
  var wbits:Int = 0;             /* log base 2 of requested window size */
  var wsize:Int = 0;             /* window size or zero if not using window */
  var whave:Int = 0;             /* valid bytes in the window */
  var wnext:Int = 0;             /* window write index */
  var window:UInt8Array= null;         /* allocated sliding window, if needed */

  /* bit accumulator */
  var hold:Int = 0;              /* input bit accumulator */
  var bits:Int = 0;              /* number of bits in "in" */

  /* for string and stored block copying */
  var length:Int = 0;            /* literal or length of data to copy */
  var offset:Int = 0;            /* distance back to copy string from */

  /* for table and code decoding */
  var extra:Int = 0;             /* extra bits needed */

  /* fixed and dynamic code tables */
  var lencode:Int32Array = null;          /* starting table for length/literal codes */
  var distcode:Int32Array = null;         /* starting table for distance codes */
  var lenbits:Int = 0;           /* index bits for lencode */
  var distbits:Int = 0;          /* index bits for distcode */

  /* dynamic table building */
  var ncode:Int = 0;             /* number of code length code lengths */
  var nlen:Int = 0;              /* number of length code lengths */
  var ndist:Int = 0;             /* number of distance code lengths */
  var have = 0;              /* number of code lengths in lens[] */
  var next = null;              /* next available space in codes[] */

  var lens:UInt16Array = new UInt16Array(320); /* temporary storage for code lengths */
  var work:UInt16Array = new UInt16Array(288); /* work area for code table building */

  /*
   because we don't have pointers in js, we use lencode and distcode directly
   as buffers so we don't need codes
  */
  //var codes = new Common.Buf32(ENOUGH);       /* space for code tables */
  var lendyn:Int32Array = null;              /* dynamic table for length/literal codes (JS specific) */
  var distdyn:Int32Array = null;             /* dynamic table for distance codes (JS specific) */
  var sane:Int = 0;                   /* if false, allow invalid distance too far */
  var back:Int = 0;                   /* bits back of last unprocessed length/lit */
  var was:Int = 0;                    /* initial length of match */

  function new() { }
}

/*exports.inflateReset = inflateReset;
exports.inflateReset2 = inflateReset2;
exports.inflateResetKeep = inflateResetKeep;
exports.inflateInit = inflateInit;
exports.inflateInit2 = inflateInit2;
exports.inflate = inflate;
exports.inflateEnd = inflateEnd;
exports.inflateGetHeader = inflateGetHeader;
exports.inflateSetDictionary = inflateSetDictionary;
exports.inflateInfo = 'pako inflate (from Nodeca project)';
*/

/* Not implemented
exports.inflateCopy = inflateCopy;
exports.inflateGetDictionary = inflateGetDictionary;
exports.inflateMark = inflateMark;
exports.inflatePrime = inflatePrime;
exports.inflateSync = inflateSync;
exports.inflateSyncPoint = inflateSyncPoint;
exports.inflateUndermine = inflateUndermine;
*/
