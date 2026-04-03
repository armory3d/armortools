// File: bc7enc.h - Richard Geldreich, Jr. - MIT license or public domain (see end of bc7enc.c)
#pragma once
#include <stdlib.h>
#include <stdint.h>

#define BC7ENC_BLOCK_SIZE (16)
#define BC7ENC_MAX_PARTITIONS1 (64)
#define BC7ENC_MAX_UBER_LEVEL (4)

typedef uint8_t bc7enc_bool;
#define BC7ENC_TRUE (1)
#define BC7ENC_FALSE (0)

typedef struct
{
	// m_max_partitions_mode may range from 0 (disables mode 1) to BC7ENC_MAX_PARTITIONS1. The higher this value, the slower the compressor, but the higher the quality.
	uint32_t m_max_partitions_mode;

	// Relative RGBA or YCbCrA weights.
	uint32_t m_weights[4];

	// m_uber_level may range from 0 to BC7ENC_MAX_UBER_LEVEL. The higher this value, the slower the compressor, but the higher the quality.
	uint32_t m_uber_level;

	// If m_perceptual is true, colorspace error is computed in YCbCr space, otherwise RGB.
	bc7enc_bool m_perceptual;

	// Set m_try_least_squares to false for slightly faster/lower quality compression.
	bc7enc_bool m_try_least_squares;

	// When m_mode_partition_estimation_filterbank, the mode1 partition estimator skips lesser used partition patterns unless they are strongly predicted to be potentially useful.
	// There's a slight loss in quality with this enabled (around .08 dB RGB PSNR or .05 dB Y PSNR), but up to a 11% gain in speed depending on the other settings.
	bc7enc_bool m_mode_partition_estimation_filterbank;

	bc7enc_bool m_use_mode5_for_alpha;
	bc7enc_bool m_use_mode7_for_alpha;

} bc7enc_compress_block_params;

void bc7enc_compress_block_params_init_linear_weights(bc7enc_compress_block_params *p);

void bc7enc_compress_block_params_init_perceptual_weights(bc7enc_compress_block_params *p);

void bc7enc_compress_block_params_init(bc7enc_compress_block_params *p);

// bc7enc_compress_block_init() MUST be called before calling bc7enc_compress_block() (or you'll get artifacts).
void bc7enc_compress_block_init();

// Packs a single block of 16x16 RGBA pixels (R first in memory) to 128-bit BC7 block pBlock, using either mode 1 and/or 6.
// Alpha blocks will always use mode 6, and by default opaque blocks will use either modes 1 or 6.
// Returns BC7ENC_TRUE if the block had any pixels with alpha < 255, otherwise it return BC7ENC_FALSE. (This is not an error code - a block is always encoded.)
bc7enc_bool bc7enc_compress_block(void *pBlock, const void *pPixelsRGBA, const bc7enc_compress_block_params *pComp_params);
