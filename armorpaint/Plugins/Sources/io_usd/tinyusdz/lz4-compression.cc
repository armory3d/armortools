#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif 

#include "lz4-compression.hh"

#include <cstring>
#include <cstdlib>
#include <memory>
#include <algorithm>



// LZ4Compression based on USD's TfFastCompression class

//
// Copyright 2017 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//

#include "pxrLZ4/lz4.h"

using namespace pxr_lz4;

namespace tinyusdz {

size_t LZ4Compression::GetMaxInputSize() {
  return 127 * static_cast<size_t>(LZ4_MAX_INPUT_SIZE);
}

size_t LZ4Compression::GetCompressedBufferSize(size_t inputSize) {
  if (inputSize > GetMaxInputSize()) return 0;

  // If it fits in one chunk then it's just the compress bound plus 1.
  if (inputSize <= LZ4_MAX_INPUT_SIZE) {
    return size_t(LZ4_compressBound(int(inputSize))) + 1;
  }
  size_t nWholeChunks = inputSize / LZ4_MAX_INPUT_SIZE;
  size_t partChunkSz = inputSize % LZ4_MAX_INPUT_SIZE;
  size_t sz = 1 + nWholeChunks *
                      (size_t(LZ4_compressBound(LZ4_MAX_INPUT_SIZE)) + sizeof(int32_t));
  if (partChunkSz) sz += size_t(LZ4_compressBound(int(partChunkSz))) + sizeof(int32_t);
  return sz;
}

size_t LZ4Compression::CompressToBuffer(char const *input, char *compressed,
                                        size_t inputSize, std::string *err) {
  if (inputSize > GetMaxInputSize()) {
    if (err) {
      (*err) = "Attempted to compress a buffer of " +
               std::to_string(inputSize) +
               " bytes, "
               "more than the maximum supported " +
               std::to_string(GetMaxInputSize()) + "\n";
    }
    return 0;
  }

  // If it fits in one chunk, just do it.
  char const *const origCompressed = compressed;
  if (inputSize <= LZ4_MAX_INPUT_SIZE) {
    compressed[0] = 0;  // < zero byte means one chunk.
    compressed += 1 + LZ4_compress_default(input, compressed + 1, int(inputSize),
                                           int(GetCompressedBufferSize(inputSize)));
  } else {
    size_t nWholeChunks = inputSize / LZ4_MAX_INPUT_SIZE;
    size_t partChunkSz = inputSize % LZ4_MAX_INPUT_SIZE;
    size_t numChunks = nWholeChunks + (partChunkSz ? 1 : 0);

    if (numChunks > 127) {
      if (err) {
        (*err) = "# of chunks must be less than 127 but got " + std::to_string(numChunks) + "\n";
      }
      return 0;
    }
    *compressed++ = char(numChunks);
    auto writeChunk = [](char const *&_input, char *&_output, size_t size) {
      char *o = _output;
      _output += sizeof(int32_t);
      int32_t n =
          LZ4_compress_default(_input, _output, int(size), LZ4_compressBound(int(size)));
      memcpy(o, &n, sizeof(n));
      _output += n;
      _input += size;
    };
    for (size_t chunk = 0; chunk != nWholeChunks; ++chunk) {
      writeChunk(input, compressed, LZ4_MAX_INPUT_SIZE);
    }
    if (partChunkSz) {
      writeChunk(input, compressed, partChunkSz);
    }
  }

  return size_t(compressed - origCompressed);
}

size_t LZ4Compression::DecompressFromBuffer(char const *compressed,
                                               char *output,
                                               size_t compressedSize,
                                               size_t maxOutputSize,
                                               std::string *err) {
  if (compressedSize <= 1) {
      if (err) {
        (*err) =
            "Invalid compressedSize.\n";
      }
    return 0;
  }

  // Check first byte for # chunks.
  int nChunks = *compressed++;
  if (nChunks == 0) {
    // Just one.
    int nDecompressed = LZ4_decompress_safe(compressed, output,
                                            int(compressedSize - 1), int(maxOutputSize));
    if (nDecompressed < 0) {
      if (err) {
        (*err) =
            "Failed to decompress data, possibly corrupt? "
            "LZ4 error code: " +
            std::to_string(nDecompressed) + "\n";
      }
      return 0;
    }
    return size_t(nDecompressed);
  } else {
    // Do each chunk.
    size_t totalDecompressed = 0;
    for (int i = 0; i != nChunks; ++i) {
      int32_t chunkSize = 0;
      memcpy(&chunkSize, compressed, sizeof(chunkSize));
      compressed += sizeof(chunkSize);
      int nDecompressed = LZ4_decompress_safe(
          compressed, output, chunkSize,
          int(std::min<size_t>(LZ4_MAX_INPUT_SIZE, maxOutputSize)));
      if (nDecompressed < 0) {
        if (err) {
          (*err) =
              "Failed to decompress data, possibly corrupt? "
              "LZ4 error code: " +
              std::to_string(nDecompressed) + "\n";
        }
        return 0;
      }
      compressed += chunkSize;
      output += nDecompressed;
      maxOutputSize -= size_t(nDecompressed);
      totalDecompressed += size_t(nDecompressed);
    }
    return totalDecompressed;
  }
  // unreachable.
}

}  // namespace tinyusdz
