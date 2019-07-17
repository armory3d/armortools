hxPako
==========================================

[pako](https://github.com/nodeca/pako) v1.0.4 port to haxe, for cross-platform zlib functionality. 

### Features

 - Written in pure haxe (as of now works for js/hxnodejs/java/cs/cpp/hl/flash/neko).
 - Chunking support for big blobs.
 - Results are binary equal to well known [zlib](http://www.zlib.net/) (now v1.2.8 ported).

### API

```haxe
import pako.Pako;

// Deflate
//
var input = new UInt8Array();
//... fill input data here
var output = Pako.deflate(input);

// Inflate (simple wrapper can throw exception on broken stream)
//
var compressed = new UInt8Array();
//... fill data to uncompress here
try {
  var result = Pako.inflate(compressed);
} catch (err:Dynamic) {
  trace(err);
}

//
// Alternate interface for chunking & without exceptions
//

var inflator = new pako.Inflate();

inflator.push(chunk1, false);
inflator.push(chunk2, false);
...
inflator.push(chunkN, true); // true -> last chunk

if (inflator.err) {
  trace(inflator.msg);
}

var output = inflator.result;
```

For more info you can consult [pako documentation](http://nodeca.github.io/pako/).

### Notes
hxPako (like pako) does not contain some specific zlib functions:

- __deflate__ -  methods `deflateCopy`, `deflateBound`, `deflateParams`,
  `deflatePending`, `deflatePrime`, `deflateTune`.
- __inflate__ - `inflateGetDictionary`, `inflateCopy`, `inflateMark`,
  `inflatePrime`, `inflateSync`, `inflateSyncPoint`,
  `inflateUndermine`.

hxPako only supports `UInt8Array` (unlike pako, which also works with strings and arrays). But it's easy to extend to those too by using `UInt8Array.fromBytes()` and `UInt8Array.fromArray()`.

### Test Suite Timings
Current timings (`node.js` refers to the original suite from pako, which you can test by running `mocha` in the top folder). See [testAll.hxml](test/testAll.hxml).

| target     | release | debug |
|:-----------|--------:|------:|
|node.js     |    5.8s |     - |
|java        |    5.6s |   6.0s|
|cs          |    5.9s |  20.5s|
|hxnodejs    |    6.4s |   6.5s|
|cpp         |    6.7s |  62.1s|
|js          |    9.8s |  13.2s|
|hl (vm)     |   12.2s |  12.4s|
|flash       |   39.6s | 151.5s|
|pypy3.2 *   |   97.8s | 101.3s| * (2 failed tests)
|neko        |  168.6s | 181.2s|
  
### Authors
 - Andrey Tupitsin and Vitaly Puzrin (original pako lib)
 - Me (haxe port)

### License
MIT