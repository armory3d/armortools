#ifndef MINIH264_H
#define MINIH264_H
/*
    https://github.com/lieff/minih264
    To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide.
    This software is distributed without any warranty.
    See <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef H264E_SVC_API
#   define H264E_SVC_API 1
#endif

#ifndef H264E_MAX_THREADS
#   define H264E_MAX_THREADS 4
#endif

/**
*   API return error codes
*/
#define H264E_STATUS_SUCCESS                0
#define H264E_STATUS_BAD_ARGUMENT           1
#define H264E_STATUS_BAD_PARAMETER          2
#define H264E_STATUS_BAD_FRAME_TYPE         3
#define H264E_STATUS_SIZE_NOT_MULTIPLE_16   4
#define H264E_STATUS_SIZE_NOT_MULTIPLE_2    5
#define H264E_STATUS_BAD_LUMA_ALIGN         6
#define H264E_STATUS_BAD_LUMA_STRIDE        7
#define H264E_STATUS_BAD_CHROMA_ALIGN       8
#define H264E_STATUS_BAD_CHROMA_STRIDE      9

/**
*   Frame type definitions
*   - Sequence must start with key (IDR) frame.
*   - P (Predicted) frames are most efficiently coded
*   - Dropable frames may be safely removed from bitstream, and used
*     for frame rate scalability
*   - Golden and Recovery frames used for error recovery. These
*     frames uses "long-term reference" for prediction, and
*     can be decoded if P frames sequence is interrupted.
*     They acts similarly to key frame, but coded more efficiently.
*
*   Type        Refers to   Saved as long-term  Saved as short-term
*   ---------------------------------------------------------------
*   Key (IDR) : N/A         Yes                 Yes                |
*   Golden    : long-term   Yes                 Yes                |
*   Recovery  : long-term   No                  Yes                |
*   P         : short-term  No                  Yes                |
*   Droppable : short-term  No                  No                 |
*                                                                  |
*   Example sequence:        K   P   P   G   D   P   R   D   K     |
*   long-term reference       1K  1K  1K  4G  4G  4G  4G  4G  9K   |
*                             /         \ /         \         /    |
*   coded frame             1K  2P  3P  4G  5D  6P  7R  8D  9K     |
*                             \ / \ / \   \ /   / \   \ /     \    |
*   short-term reference      1K  2P  3P  4G  4G  6P  7R  7R  9K   |
*
*/
#define H264E_FRAME_TYPE_DEFAULT    0       // Frame type set according to GOP size
#define H264E_FRAME_TYPE_KEY        6       // Random access point: SPS+PPS+Intra frame
#define H264E_FRAME_TYPE_I          5       // Intra frame: updates long & short-term reference
#define H264E_FRAME_TYPE_GOLDEN     4       // Use and update long-term reference
#define H264E_FRAME_TYPE_RECOVERY   3       // Use long-term reference, updates short-term reference
#define H264E_FRAME_TYPE_P          2       // Use and update short-term reference
#define H264E_FRAME_TYPE_DROPPABLE  1       // Use short-term reference, don't update anything
#define H264E_FRAME_TYPE_CUSTOM     99      // Application specifies reference frame

/**
*   Speed preset index.
*   Currently used values are 0, 1, 8 and 9
*/
#define H264E_SPEED_SLOWEST         0       // All coding tools enabled, including denoise filter
#define H264E_SPEED_BALANCED        5
#define H264E_SPEED_FASTEST         10      // Minimum tools enabled

/**
*   Creations parameters
*/
typedef struct H264E_create_param_tag
{
    // Frame width: must be multiple of 16
    int width;

    // Frame height: must be multiple of 16
    int height;

    // GOP size == key frame period
    // If 0: no key frames generated except 1st frame (infinite GOP)
    // If 1: Only intra-frames produced
    int gop;

    // Video Buffer Verifier size, bits
    // If 0: VBV model would be disabled
    // Note, that this value defines Level,
    int vbv_size_bytes;

    // If set: transparent frames produced on VBV overflow
    // If not set: VBV overflow ignored, produce bitrate bigger than specified
    int vbv_overflow_empty_frame_flag;

    // If set: keep minimum bitrate using stuffing, prevent VBV underflow
    // If not set: ignore VBV underflow, produce bitrate smaller than specified
    int vbv_underflow_stuffing_flag;

    // If set: control bitrate at macroblock-level (better bitrate precision)
    // If not set: control bitrate at frame-level (better quality)
    int fine_rate_control_flag;

    // If set: don't change input, but allocate additional frame buffer
    // If not set: use input as a scratch
    int const_input_flag;

    // If 0: golden, recovery, and custom frames are disabled
    // If >0: Specifies number of persistent frame buffer's used
    int max_long_term_reference_frames;

    int enableNEON;

    // If set: enable temporal noise suppression
    int temporal_denoise_flag;

    int sps_id;

#if H264E_SVC_API
    //          SVC extension
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Number of SVC layers:
    // 1 = AVC
    // 2 = SVC with 2-layers of spatial scalability
    int num_layers;

    // If set, SVC extension layer will use predictors from base layer
    // (sometimes can slightly increase efficiency)
    int inter_layer_pred_flag;
#endif

#if H264E_MAX_THREADS
    //           Multi-thread extension
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Maximum threads, supported by the callback
    int max_threads;

    // Opaque token, passed to callback
    void *token;

    // Application-supplied callback function.
    // This callback runs given jobs, by calling provided job_func(), passing
    // job_data[i] to each one.
    //
    // The h264e_thread_pool_run() can be used here, example:
    //
    //      int max_threads = 4;
    //      void *thread_pool = h264e_thread_pool_init(max_threads);
    //
    //      H264E_create_param_t par;
    //      par.max_threads = max_threads;
    //      par.token = thread_pool;
    //      par.run_func_in_thread = h264e_thread_pool_run;
    //
    // The reason to use double callbacks is to avoid mixing portable and
    // system-dependent code, and to avoid close() function in the encoder API.
    //
    void (*run_func_in_thread)(void *token, void (*job_func)(void*), void *job_data[], int njobs);
#endif

} H264E_create_param_t;

/**
*   Run-time parameters
*/
typedef struct H264E_run_param_tag
{
    // Variable, indicating speed/quality tradeoff
    // 0 means best quality
    int encode_speed;

    // Frame type override: one of H264E_FRAME_TYPE_* values
    // if 0: GOP pattern defined by create_param::gop value
    int frame_type;

    // Used only if frame_type == H264E_FRAME_TYPE_CUSTOM
    // Reference long-term frame index [1..max_long_term_reference_frames]
    // 0 = use previous frame (short-term)
    // -1 = IDR frame, kill all long-term frames
    int long_term_idx_use;

    // Used only if frame_type == H264E_FRAME_TYPE_CUSTOM
    // Store decoded frame in long-term buffer with given index in the
    // range [1..max_long_term_reference_frames]
    // 0 = save to short-term buffer
    // -1 = Don't save frame (dropable)
    int long_term_idx_update;

    // Target frame size. Typically = bitrate/framerate
    int desired_frame_bytes;

    // Minimum quantizer value, 10 indicates good quality
    // range: [10; qp_max]
    int qp_min;

    // Maximum quantizer value, 51 indicates very bad quality
    // range: [qp_min; 51]
    int qp_max;

    // Desired NALU size. NALU produced as soon as it's size exceed this value
    // if 0: frame would be coded with a single NALU
    int desired_nalu_bytes;

    // Optional NALU notification callback, called by the encoder
    // as soon as NALU encoding complete.
    void (*nalu_callback)(
        const unsigned char *nalu_data, // Coded NALU data, w/o start code
        int sizeof_nalu_data,           // Size of NALU data
        void *token                     // optional transparent token
        );

    // token to pass to NALU callback
    void *nalu_callback_token;

} H264E_run_param_t;

/**
*    Planar YUV420 descriptor
*/
typedef struct H264E_io_yuv_tag
{
    // Pointers to 3 pixel planes of YUV image
    unsigned char *yuv[3];
    // Stride for each image plane
    int stride[3];
} H264E_io_yuv_t;

typedef struct H264E_persist_tag H264E_persist_t;
typedef struct H264E_scratch_tag H264E_scratch_t;

/**
*   Return persistent and scratch memory requirements
*   for given encoding options.
*
*   Return value:
*       -zero in case of success
*       -error code (H264E_STATUS_*), if fails
*
*   example:
*
*   int sizeof_persist, sizeof_scratch, error;
*   H264E_persist_t * enc;
*   H264E_scratch_t * scratch;
*
*   error = H264E_sizeof(param, &sizeof_persist, &sizeof_scratch);
*   if (!error)
*   {
*       enc     = malloc(sizeof_persist);
*       scratch = malloc(sizeof_scratch);
*       error = H264E_init(enc, param);
*   }
*/
int H264E_sizeof(
    const H264E_create_param_t *param,  ///< Encoder creation parameters
    int *sizeof_persist,                ///< [OUT] Size of persistent RAM
    int *sizeof_scratch                 ///< [OUT] Size of scratch RAM
);

/**
*   Initialize encoding session
*
*   Return value:
*       -zero in case of success
*       -error code (H264E_STATUS_*), if fails
*/
int H264E_init(
    H264E_persist_t *enc,               ///< Encoder object
    const H264E_create_param_t *param   ///< Encoder creation parameters
);

/**
*   Encode single video frame
*
*   Output buffer is in the scratch RAM
*
*   Return value:
*       -zero in case of success
*       -error code (H264E_STATUS_*), if fails
*/
int H264E_encode(
    H264E_persist_t *enc,               ///< Encoder object
    H264E_scratch_t *scratch,           ///< Scratch memory
    const H264E_run_param_t *run_param, ///< run-time parameters
    H264E_io_yuv_t *frame,              ///< Input video frame
    unsigned char **coded_data,         ///< [OUT] Pointer to coded data
    int *sizeof_coded_data              ///< [OUT] Size of coded data
);

/**
*   This is a "hack" function to set internal rate-control state
*   Note that encoder allows application to completely override rate-control,
*   so this function should be used only by lazy coders, who just want to change
*   VBV size, without implementing custom rate-control.
*
*   Note that H.264 level defined by VBV size on initialization.
*/
void H264E_set_vbv_state(
    H264E_persist_t *enc,               ///< Encoder object
    int vbv_size_bytes,                 ///< New VBV size
    int vbv_fullness_bytes              ///< New VBV fulness, -1 = no change
);

#ifdef __cplusplus
}
#endif

#endif //MINIH264_H

#if defined(MINIH264_IMPLEMENTATION) && !defined(MINIH264_IMPLEMENTATION_GUARD)
#define MINIH264_IMPLEMENTATION_GUARD

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/************************************************************************/
/*      Build configuration                                             */
/************************************************************************/
#ifndef H264E_ENABLE_DENOISE
#define H264E_ENABLE_DENOISE 1 // Build-in noise supressor
#endif

#ifndef MAX_LONG_TERM_FRAMES
#define MAX_LONG_TERM_FRAMES 8 // Max long-term frames count
#endif

#if !defined(MINIH264_ONLY_SIMD) && (defined(_M_X64) || defined(_M_ARM64) || defined(__x86_64__) || defined(__aarch64__))
/* x64 always have SSE2, arm64 always have neon, no need for generic code */
#define MINIH264_ONLY_SIMD
#endif /* SIMD checks... */

#if (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))) || ((defined(__i386__) || defined(__x86_64__)) && defined(__SSE2__))
#define H264E_ENABLE_SSE2 1
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <emmintrin.h>
#endif
#elif defined(__ARM_NEON) || defined(__aarch64__)
#define H264E_ENABLE_NEON 1
#include <arm_neon.h>
#else
#ifdef MINIH264_ONLY_SIMD
#error MINIH264_ONLY_SIMD used, but SSE/NEON not enabled
#endif
#endif

#ifndef MINIH264_ONLY_SIMD
#define H264E_ENABLE_PLAIN_C 1
#endif

#define H264E_CONFIGS_COUNT ((H264E_ENABLE_SSE2) + (H264E_ENABLE_PLAIN_C) + (H264E_ENABLE_NEON))

#if defined(__ARMCC_VERSION) || defined(_WIN32) || defined(__EMSCRIPTEN__)
#define __BYTE_ORDER 0
#define __BIG_ENDIAN 1
#elif defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define __BYTE_ORDER BYTE_ORDER
#define __BIG_ENDIAN BIG_ENDIAN
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
#else
#error platform not supported
#endif

#if defined(__aarch64__) && defined(__clang__)
// uintptr_t broken with aarch64 clang on ubuntu 18
#define uintptr_t unsigned long
#endif
#if defined(__arm__) && defined(__clang__)
#include <arm_acle.h>
#elif defined(__arm__) && defined(__GNUC__) && !defined(__ARMCC_VERSION)
static inline unsigned int __usad8(unsigned int val1, unsigned int val2)
{
    unsigned int result;
    __asm__ volatile ("usad8 %0, %1, %2\n\t"
                      : "=r" (result)
                      : "r" (val1), "r" (val2));
    return result;
}

static inline unsigned int __usada8(unsigned int val1, unsigned int val2, unsigned int val3)
{
    unsigned int result;
    __asm__ volatile ("usada8 %0, %1, %2, %3\n\t"
                      : "=r" (result)
                      : "r" (val1), "r" (val2), "r" (val3));
    return result;
}

static inline unsigned int __sadd16(unsigned int val1, unsigned int val2)
{
    unsigned int result;
    __asm__ volatile ("sadd16 %0, %1, %2\n\t"
                      : "=r" (result)
                      : "r" (val1), "r" (val2));
    return result;
}

static inline unsigned int __ssub16(unsigned int val1, unsigned int val2)
{
    unsigned int result;
    __asm__ volatile ("ssub16 %0, %1, %2\n\t"
                      : "=r" (result)
                      : "r" (val1), "r" (val2));
    return result;
}

static inline unsigned int __clz(unsigned int val1)
{
    unsigned int result;
    __asm__ volatile ("clz %0, %1\n\t"
                      : "=r" (result)
                      : "r" (val1));
    return result;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#if defined(_MSC_VER) && _MSC_VER >= 1400
#   define h264e_restrict __restrict
#elif defined(__arm__)
#   define h264e_restrict __restrict
#else
#   define h264e_restrict
#endif
#if defined(_MSC_VER)
#   define ALIGN(n) __declspec(align(n))
#   define ALIGN2(n)
#else
#   define ALIGN(n)
#   define ALIGN2(n) __attribute__((aligned(n)))
#endif

#if __GNUC__ || __clang__
typedef int int_u __attribute__ ((__aligned__ (1)));
#else
typedef int int_u;
#endif

#ifndef MAX
#   define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#   define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef ABS
#   define ABS(x)    ((x) >= 0 ? (x) : -(x))
#endif

#define IS_ALIGNED(p, n) (!((uintptr_t)(p) & (uintptr_t)((n) - 1)))

// bit-stream
#if __BYTE_ORDER == __BIG_ENDIAN
#   define SWAP32(x) (uint32_t)(x)
#else
#ifdef _MSC_VER
#   define SWAP32(x) _byteswap_ulong(x)
#elif defined(__GNUC__) || defined(__clang__)
#   define SWAP32(x) __builtin_bswap32(x)
#else
#   define SWAP32(x) (uint32_t)((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | ((x & 0xFF) << 24))
#endif
#endif

#define BS_OPEN(bs) uint32_t cache = bs->cache; int shift = bs->shift; uint32_t *buf = bs->buf;
#define BS_CLOSE(bs) bs->cache = cache; bs->shift = shift; bs->buf = buf;
#define BS_PUT(n, val)      \
if ((shift -= n) < 0)       \
{                           \
    cache |= val >> -shift; \
    *buf++ = SWAP32(cache); \
    shift += 32;            \
    cache = 0;              \
}                           \
cache |= (uint32_t)val << shift;

// Quantizer-dequantizer modes
#define QDQ_MODE_INTRA_4   2       // intra 4x4
#define QDQ_MODE_INTER     8       // inter
#define QDQ_MODE_INTRA_16  (8 + 1) // intra 16x61
#define QDQ_MODE_CHROMA    (4 + 1) // chroma

// put most frequently used bits to lsb, to use these as look-up tables
#define AVAIL_TR    8
#define AVAIL_TL    4
#define AVAIL_L     2
#define AVAIL_T     1

typedef uint8_t     pix_t;
typedef uint32_t    bs_item_t;

/**
*   Output bitstream
*/
typedef struct
{
    int         shift;  // bit position in the cache
    uint32_t    cache;  // bit cache
    bs_item_t    *buf;  // current position
    bs_item_t  *origin; // initial position
} bs_t;

/**
*   Tuple for motion vector, or height/width representation
*/
typedef union
{
    struct
    {
        int16_t x;      // horizontal or width
        int16_t y;      // vertical or height
    } s;
    int32_t u32;        // packed representation
} point_t;

/**
*   Rectangle
*/
typedef struct
{
    point_t tl;         // top-left corner
    point_t br;         // bottom-right corner
} rectangle_t;

/**
*   Quantized/dequantized representation for 4x4 block
*/
typedef struct
{
    int16_t qv[16];     // quantized coefficient
    int16_t dq[16];     // dequantized
} quant_t;

/**
*   Scratch RAM, used only for current MB encoding
*/
typedef struct H264E_scratch_tag
{
    pix_t mb_pix_inp[256];          // Input MB (cached)
    pix_t mb_pix_store[4*256];      // Prediction variants

    // Quantized/dequantized
    int16_t dcy[16];                // Y DC
    quant_t qy[16];                 // Y 16x4x4 blocks

    int16_t dcu[16];                // U DC: 4 used + align
    quant_t qu[4];                  // U 4x4x4 blocks

    int16_t dcv[16];                // V DC: 4 used + align
    quant_t qv[4];                  // V 4x4x4 blocks

    // Quantized DC:
    int16_t quant_dc[16];           // Y
    int16_t quant_dc_u[4];          // U
    int16_t quant_dc_v[4];          // V

    uint16_t nz_mask;               // Bit flags for non-zero 4x4 blocks
} scratch_t;

/**
*   Deblock filter frame context
*/
typedef struct
{
    // Motion vectors for 4x4 MB internal sub-blocks, top and left border,
    // 5x5 array without top-left cell:
    //     T0 T1 T2 T4
    //  L0 i0 i1 i2 i3
    //  L1 ...
    //  ......
    //
    point_t df_mv[5*5 - 1];         // MV for current macroblock and neighbors
    uint8_t *df_qp;                 // QP for current row of macroblocks
    int8_t *mb_type;                // Macroblock type for current row of macroblocks
    uint32_t nzflag;                // Bit flags for non-zero 4x4 blocks (left neighbors)

    // Huffman and deblock uses different nnz...
    uint8_t *df_nzflag;             // Bit flags for non-zero 4x4 blocks (top neighbors), only 4 bits used
} deblock_filter_t;

/**
*    Deblock filter parameters for current MB
*/
typedef struct
{
    uint32_t strength32[4*2];       // Strength for 4 colums and 4 rows
    uint8_t tc0[16*2];              // TC0 parameter for 4 colums and 4 rows
    uint8_t alpha[2*2];             // alpha for border/internals
    uint8_t beta[2*2];              // beta for border/internals
} deblock_params_t;

/**
*   Persistent RAM
*/
typedef struct H264E_persist_tag
{
    H264E_create_param_t param;     // Copy of create parameters
    H264E_io_yuv_t inp;             // Input picture

    struct
    {
        int pic_init_qp;            // Initial QP
    } sps;

    struct
    {
        int num;                    // Frame number
        int nmbx;                   // Frame width, macroblocks
        int nmby;                   // Frame height, macroblocks
        int nmb;                    // Number of macroblocks in frame
        int w;                      // Frame width, pixels
        int h;                      // Frame height, pixels
        rectangle_t mv_limit;       // Frame MV limits = frame + border extension
        rectangle_t mv_qpel_limit;  // Reduced MV limits for qpel interpolation filter
        int cropping_flag;          // Cropping indicator
    } frame;

    struct
    {
        int type;                   // Current slice type (I/P)
        int start_mb_num;           // # of 1st MB in the current slice
    } slice;

    struct
    {
        int x;                      // MB x position (in MB's)
        int y;                      // MB y position (in MB's)
        int num;                    // MB number
        int skip_run;               // Skip run count

        // according to table 7-13
        // -1 = skip, 0 = P16x16, 1 = P16x8, 2=P8x16, 3 = P8x8, 5 = I4x4, >=6 = I16x16
        int type;                   // MB type

        struct
        {
            int pred_mode_luma;     // Intra 16x16 prediction mode
        } i16;

        int8_t i4x4_mode[16];       // Intra 4x4 prediction modes

        int cost;                   // Best coding cost
        int avail;                  // Neighbor availability flags
        point_t mvd[16];            // Delta-MV for each 4x4 sub-part
        point_t mv[16];             // MV for each 4x4 sub-part

        point_t mv_skip_pred;       // Skip MV predictor
    } mb;

    H264E_io_yuv_t ref;             // Current reference picture
    H264E_io_yuv_t dec;             // Reconstructed current macroblock
#if H264E_ENABLE_DENOISE
    H264E_io_yuv_t denoise;         // Noise suppression filter
#endif

    unsigned char *lt_yuv[MAX_LONG_TERM_FRAMES][3]; // Long-term reference pictures
    unsigned char lt_used[MAX_LONG_TERM_FRAMES];    // Long-term "used" flags

    struct
    {
        int qp;                     // Current QP
        int vbv_bits;               // Current VBV fullness, bits
        int qp_smooth;              // Averaged QP
        int dqp_smooth;             // Adaptive QP adjustment, account for "compressibility"
        int max_dqp;                // Worst-case DQP, for long-term reference QP adjustment

        int bit_budget;             // Frame bit budget
        int prev_qp;                // Previous MB QP
        int prev_err;               // Accumulated coded size error
        int stable_count;           // Stable/not stable state machine

        int vbv_target_level;       // Desired VBV fullness after frame encode

        // Quantizer data, passed to low-level functions
        // layout:
        // multiplier_quant0, multiplier_dequant0,
        // multiplier_quant2, multiplier_dequant2,
        // multiplier_quant1, multiplier_dequant1,
        // rounding_factor_pos,
        // zero_thr_inter
        // zero_thr_inter2
        // ... and same data for chroma
        //uint16_t qdat[2][(6 + 4)];
#define OFFS_RND_INTER 6
#define OFFS_RND_INTRA 7
#define OFFS_THR_INTER 8
#define OFFS_THR2_INTER 9
#define OFFS_THR_1_OFF 10
#define OFFS_THR_2_OFF 18
#define OFFS_QUANT_VECT 26
#define OFFS_DEQUANT_VECT 34
        //struct
        //{
        //    uint16_t qdq[6];
        //    uint16_t rnd[2]; // inter/intra
        //    uint16_t thr[2]; // thresholds
        //    uint16_t zero_thr[2][8];
        //    uint16_t qfull[8];
        //    uint16_t dqfull[8];
        //} qdat[2];
        uint16_t qdat[2][6 + 2 + 2 + 8 + 8 + 8 + 8];
    } rc;

    deblock_filter_t df;            // Deblock filter

    // Speed/quality trade-off
    struct
    {
        int disable_deblock;        // Disable deblock filter flags
    } speed;

    int most_recent_ref_frame_idx;  // Last updated long-term reference

    // predictors contexts
    point_t *mv_pred;               // MV for left&top 4x4 blocks
    uint8_t *nnz;                   // Number of non-zero coeffs per 4x4 block for left&top
    int32_t *i4x4mode;              // Intra 4x4 mode for left&top
    pix_t *top_line;                // left&top neighbor pixels

    // output data
    uint8_t *out;                   // Output data storage (pointer to scratch RAM!)
    unsigned int out_pos;           // Output byte position
    bs_t bs[1];                     // Output bitbuffer

    scratch_t *scratch;             // Pointer to scratch RAM
#if H264E_MAX_THREADS > 1
    scratch_t *scratch_store[H264E_MAX_THREADS];   // Pointer to scratch RAM
    int sizeof_scaratch;
#endif
    H264E_run_param_t run_param;    // Copy of run-time parameters

    // Consecutive IDR's must have different idr_pic_id,
    // unless there are some P between them
    uint8_t next_idr_pic_id;

    pix_t *pbest;                   // Macroblock best predictor
    pix_t *ptest;                   // Macroblock predictor under test

    point_t mv_clusters[2];         // MV clusterization for prediction

    // Flag to track short-term reference buffer, for MMCO 1 command
    int short_term_used;

#if H264E_SVC_API
    //svc ext
    int   current_layer;
    int   adaptive_base_mode_flag;
    void *enc_next;
#endif

} h264e_enc_t;

#ifdef __cplusplus
}
#endif //__cplusplus
/************************************************************************/
/*      Constants                                                       */
/************************************************************************/

// Tunable constants can be adjusted by the "training" application
#ifndef ADJUSTABLE
#   define ADJUSTABLE static const
#endif

// Huffman encode tables
#define CODE8(val, len) (uint8_t)((val << 4) + len)
#define CODE(val, len) (uint8_t)((val << 4) + (len - 1))

const uint8_t h264e_g_run_before[57] =
{
    15, 17, 20, 24, 29, 35, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    /**** Table #  0 size  2 ****/
    CODE8(1, 1), CODE8(0, 1),
    /**** Table #  1 size  3 ****/
    CODE8(1, 1), CODE8(1, 2), CODE8(0, 2),
    /**** Table #  2 size  4 ****/
    CODE8(3, 2), CODE8(2, 2), CODE8(1, 2), CODE8(0, 2),
    /**** Table #  3 size  5 ****/
    CODE8(3, 2), CODE8(2, 2), CODE8(1, 2), CODE8(1, 3), CODE8(0, 3),
    /**** Table #  4 size  6 ****/
    CODE8(3, 2), CODE8(2, 2), CODE8(3, 3), CODE8(2, 3), CODE8(1, 3), CODE8(0, 3),
    /**** Table #  5 size  7 ****/
    CODE8(3, 2), CODE8(0, 3), CODE8(1, 3), CODE8(3, 3), CODE8(2, 3), CODE8(5, 3), CODE8(4, 3),
    /**** Table #  6 size 15 ****/
    CODE8(7, 3), CODE8(6, 3), CODE8(5, 3), CODE8(4, 3), CODE8(3, 3), CODE8(2,  3), CODE8(1,  3), CODE8(1, 4),
    CODE8(1, 5), CODE8(1, 6), CODE8(1, 7), CODE8(1, 8), CODE8(1, 9), CODE8(1, 10), CODE8(1, 11),
};

const uint8_t h264e_g_total_zeros_cr_2x2[12] =
{
    3, 7, 10,
    /**** Table #  0 size  4 ****/
    CODE8(1, 1), CODE8(1, 2), CODE8(1, 3), CODE8(0, 3),
    /**** Table #  1 size  3 ****/
    CODE8(1, 1), CODE8(1, 2), CODE8(0, 2),
    /**** Table #  2 size  2 ****/
    CODE8(1, 1), CODE8(0, 1),
};

const uint8_t h264e_g_total_zeros[150] =
{
    15, 31, 46, 60, 73, 85, 96, 106, 115, 123, 130, 136, 141, 145, 148,
    /**** Table #  0 size 16 ****/
    CODE8(1, 1), CODE8(3, 3), CODE8(2, 3), CODE8(3, 4), CODE8(2, 4), CODE8(3, 5), CODE8(2, 5), CODE8(3, 6),
    CODE8(2, 6), CODE8(3, 7), CODE8(2, 7), CODE8(3, 8), CODE8(2, 8), CODE8(3, 9), CODE8(2, 9), CODE8(1, 9),
    /**** Table #  1 size 15 ****/
    CODE8(7, 3), CODE8(6, 3), CODE8(5, 3), CODE8(4, 3), CODE8(3, 3), CODE8(5, 4), CODE8(4, 4), CODE8(3, 4),
    CODE8(2, 4), CODE8(3, 5), CODE8(2, 5), CODE8(3, 6), CODE8(2, 6), CODE8(1, 6), CODE8(0, 6),
    /**** Table #  2 size 14 ****/
    CODE8(5, 4), CODE8(7, 3), CODE8(6, 3), CODE8(5, 3), CODE8(4, 4), CODE8(3, 4), CODE8(4, 3), CODE8(3, 3),
    CODE8(2, 4), CODE8(3, 5), CODE8(2, 5), CODE8(1, 6), CODE8(1, 5), CODE8(0, 6),
    /**** Table #  3 size 13 ****/
    CODE8(3, 5), CODE8(7, 3), CODE8(5, 4), CODE8(4, 4), CODE8(6, 3), CODE8(5, 3), CODE8(4, 3), CODE8(3, 4),
    CODE8(3, 3), CODE8(2, 4), CODE8(2, 5), CODE8(1, 5), CODE8(0, 5),
    /**** Table #  4 size 12 ****/
    CODE8(5, 4), CODE8(4, 4), CODE8(3, 4), CODE8(7, 3), CODE8(6, 3), CODE8(5, 3), CODE8(4, 3), CODE8(3, 3),
    CODE8(2, 4), CODE8(1, 5), CODE8(1, 4), CODE8(0, 5),
    /**** Table #  5 size 11 ****/
    CODE8(1, 6), CODE8(1, 5), CODE8(7, 3), CODE8(6, 3), CODE8(5, 3), CODE8(4, 3), CODE8(3, 3), CODE8(2, 3),
    CODE8(1, 4), CODE8(1, 3), CODE8(0, 6),
    /**** Table #  6 size 10 ****/
    CODE8(1, 6), CODE8(1, 5), CODE8(5, 3), CODE8(4, 3), CODE8(3, 3), CODE8(3, 2), CODE8(2, 3), CODE8(1, 4),
    CODE8(1, 3), CODE8(0, 6),
    /**** Table #  7 size  9 ****/
    CODE8(1, 6), CODE8(1, 4), CODE8(1, 5), CODE8(3, 3), CODE8(3, 2), CODE8(2, 2), CODE8(2, 3), CODE8(1, 3),
    CODE8(0, 6),
    /**** Table #  8 size  8 ****/
    CODE8(1, 6), CODE8(0, 6), CODE8(1, 4), CODE8(3, 2), CODE8(2, 2), CODE8(1, 3), CODE8(1, 2), CODE8(1, 5),
    /**** Table #  9 size  7 ****/
    CODE8(1, 5), CODE8(0, 5), CODE8(1, 3), CODE8(3, 2), CODE8(2, 2), CODE8(1, 2), CODE8(1, 4),
    /**** Table # 10 size  6 ****/
    CODE8(0, 4), CODE8(1, 4), CODE8(1, 3), CODE8(2, 3), CODE8(1, 1), CODE8(3, 3),
    /**** Table # 11 size  5 ****/
    CODE8(0, 4), CODE8(1, 4), CODE8(1, 2), CODE8(1, 1), CODE8(1, 3),
    /**** Table # 12 size  4 ****/
    CODE8(0, 3), CODE8(1, 3), CODE8(1, 1), CODE8(1, 2),
    /**** Table # 13 size  3 ****/
    CODE8(0, 2), CODE8(1, 2), CODE8(1, 1),
    /**** Table # 14 size  2 ****/
    CODE8(0, 1), CODE8(1, 1),
};

const uint8_t h264e_g_coeff_token[277 + 18] =
{
    17 + 18, 17 + 18,
    82 + 18, 82 + 18,
    147 + 18, 147 + 18, 147 + 18, 147 + 18,
    212 + 18, 212 + 18, 212 + 18, 212 + 18, 212 + 18, 212 + 18, 212 + 18, 212 + 18, 212 + 18,
    0 + 18,
    /**** Table #  4 size 17 ****/     // offs: 0
    CODE(1, 2), CODE(1, 1), CODE(1, 3), CODE(5, 6), CODE(7, 6), CODE(6, 6), CODE(2, 7), CODE(0, 7), CODE(4, 6),
    CODE(3, 7), CODE(2, 8), CODE(0, 0), CODE(3, 6), CODE(3, 8), CODE(0, 0), CODE(0, 0), CODE(2, 6),
    /**** Table #  0 size 65 ****/     // offs: 17
    CODE( 1,  1), CODE( 1,  2), CODE( 1,  3), CODE( 3,  5), CODE( 5,  6), CODE( 4,  6), CODE( 5,  7), CODE( 3,  6),
    CODE( 7,  8), CODE( 6,  8), CODE( 5,  8), CODE( 4,  7), CODE( 7,  9), CODE( 6,  9), CODE( 5,  9), CODE( 4,  8),
    CODE( 7, 10), CODE( 6, 10), CODE( 5, 10), CODE( 4,  9), CODE( 7, 11), CODE( 6, 11), CODE( 5, 11), CODE( 4, 10),
    CODE(15, 13), CODE(14, 13), CODE(13, 13), CODE( 4, 11), CODE(11, 13), CODE(10, 13), CODE( 9, 13), CODE(12, 13),
    CODE( 8, 13), CODE(14, 14), CODE(13, 14), CODE(12, 14), CODE(15, 14), CODE(10, 14), CODE( 9, 14), CODE( 8, 14),
    CODE(11, 14), CODE(14, 15), CODE(13, 15), CODE(12, 15), CODE(15, 15), CODE(10, 15), CODE( 9, 15), CODE( 8, 15),
    CODE(11, 15), CODE( 1, 15), CODE(13, 16), CODE(12, 16), CODE(15, 16), CODE(14, 16), CODE( 9, 16), CODE( 8, 16),
    CODE(11, 16), CODE(10, 16), CODE( 5, 16), CODE( 0,  0), CODE( 7, 16), CODE( 6, 16), CODE( 0,  0), CODE( 0,  0), CODE( 4, 16),
    /**** Table #  1 size 65 ****/     // offs: 82
    CODE( 3,  2), CODE( 2,  2), CODE( 3,  3), CODE( 5,  4), CODE(11,  6), CODE( 7,  5), CODE( 9,  6), CODE( 4,  4),
    CODE( 7,  6), CODE(10,  6), CODE( 5,  6), CODE( 6,  5), CODE( 7,  7), CODE( 6,  6), CODE( 5,  7), CODE( 8,  6),
    CODE( 7,  8), CODE( 6,  7), CODE( 5,  8), CODE( 4,  6), CODE( 4,  8), CODE( 6,  8), CODE( 5,  9), CODE( 4,  7),
    CODE( 7,  9), CODE( 6,  9), CODE(13, 11), CODE( 4,  9), CODE(15, 11), CODE(14, 11), CODE( 9, 11), CODE(12, 11),
    CODE(11, 11), CODE(10, 11), CODE(13, 12), CODE( 8, 11), CODE(15, 12), CODE(14, 12), CODE( 9, 12), CODE(12, 12),
    CODE(11, 12), CODE(10, 12), CODE(13, 13), CODE(12, 13), CODE( 8, 12), CODE(14, 13), CODE( 9, 13), CODE( 8, 13),
    CODE(15, 13), CODE(10, 13), CODE( 6, 13), CODE( 1, 13), CODE(11, 13), CODE(11, 14), CODE(10, 14), CODE( 4, 14),
    CODE( 7, 13), CODE( 8, 14), CODE( 5, 14), CODE( 0,  0), CODE( 9, 14), CODE( 6, 14), CODE( 0,  0), CODE( 0,  0), CODE( 7, 14),
    /**** Table #  2 size 65 ****/     // offs: 147
    CODE(15,  4), CODE(14,  4), CODE(13,  4), CODE(12,  4), CODE(15,  6), CODE(15,  5), CODE(14,  5), CODE(11,  4),
    CODE(11,  6), CODE(12,  5), CODE(11,  5), CODE(10,  4), CODE( 8,  6), CODE(10,  5), CODE( 9,  5), CODE( 9,  4),
    CODE(15,  7), CODE( 8,  5), CODE(13,  6), CODE( 8,  4), CODE(11,  7), CODE(14,  6), CODE( 9,  6), CODE(13,  5),
    CODE( 9,  7), CODE(10,  6), CODE(13,  7), CODE(12,  6), CODE( 8,  7), CODE(14,  7), CODE(10,  7), CODE(12,  7),
    CODE(15,  8), CODE(14,  8), CODE(13,  8), CODE(12,  8), CODE(11,  8), CODE(10,  8), CODE( 9,  8), CODE( 8,  8),
    CODE(15,  9), CODE(14,  9), CODE(13,  9), CODE(12,  9), CODE(11,  9), CODE(10,  9), CODE( 9,  9), CODE(10, 10),
    CODE( 8,  9), CODE( 7,  9), CODE(11, 10), CODE( 6, 10), CODE(13, 10), CODE(12, 10), CODE( 7, 10), CODE( 2, 10),
    CODE( 9, 10), CODE( 8, 10), CODE( 3, 10), CODE( 0,  0), CODE( 5, 10), CODE( 4, 10), CODE( 0,  0), CODE( 0,  0), CODE( 1, 10),
    /**** Table #  3 size 65 ****/     // offs: 212
     3,  1,  6, 11,  0,  5, 10, 15,  4,  9, 14, 19,  8, 13, 18, 23, 12, 17, 22, 27, 16, 21, 26, 31, 20, 25, 30, 35,
    24, 29, 34, 39, 28, 33, 38, 43, 32, 37, 42, 47, 36, 41, 46, 51, 40, 45, 50, 55, 44, 49, 54, 59, 48, 53, 58, 63,
    52, 57, 62,  0, 56, 61,  0,  0, 60
};

/*
    Block scan order
    0 1 4 5
    2 3 6 7
    8 9 C D
    A B E F
*/
static const uint8_t decode_block_scan[16] = { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };

static const uint8_t qpy2qpc[52] = {  // todo: [0 - 9] not used
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
   13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
   26, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 34, 35,
   35, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 39,
};

/**
*   Rate-control LUT for intra/inter macroblocks: number of bits per macroblock for given QP
*   Estimated experimentally
*/
static const uint16_t bits_per_mb[2][42 - 1] =
{
    // 10                                                          20                                                          30                                                          40                                                          50
    { 664,  597,  530,  484,  432,  384,  341,  297,  262,  235,  198,  173,  153,  131,  114,  102,   84,   74,   64,   54,   47,   42,   35,   31,   26,   22,   20,   17,   15,   13,   12,   10,    9,    9,    7,    7,    6,    5,    4,    1,    1}, // P
    {1057,  975,  925,  868,  803,  740,  694,  630,  586,  547,  496,  457,  420,  378,  345,  318,  284,  258,  234,  210,  190,  178,  155,  141,  129,  115,  102,   95,   82,   75,   69,   60,   55,   51,   45,   41,   40,   35,   31,   28,   24}  // I
};

/**
*   Deblock filter constants:
*   <alpha> <thr[1]> <thr[2]> <thr[3]> <beta>
*/
static const uint8_t g_a_tc0_b[52 - 10][5] = {
    {  0,  0,  0,  0,  0},  // 10
    {  0,  0,  0,  0,  0},  // 11
    {  0,  0,  0,  0,  0},  // 12
    {  0,  0,  0,  0,  0},  // 13
    {  0,  0,  0,  0,  0},  // 14
    {  0,  0,  0,  0,  0},  // 15
    {  4,  0,  0,  0,  2},
    {  4,  0,  0,  1,  2},
    {  5,  0,  0,  1,  2},
    {  6,  0,  0,  1,  3},
    {  7,  0,  0,  1,  3},
    {  8,  0,  1,  1,  3},
    {  9,  0,  1,  1,  3},
    { 10,  1,  1,  1,  4},
    { 12,  1,  1,  1,  4},
    { 13,  1,  1,  1,  4},
    { 15,  1,  1,  1,  6},
    { 17,  1,  1,  2,  6},
    { 20,  1,  1,  2,  7},
    { 22,  1,  1,  2,  7},
    { 25,  1,  1,  2,  8},
    { 28,  1,  2,  3,  8},
    { 32,  1,  2,  3,  9},
    { 36,  2,  2,  3,  9},
    { 40,  2,  2,  4, 10},
    { 45,  2,  3,  4, 10},
    { 50,  2,  3,  4, 11},
    { 56,  3,  3,  5, 11},
    { 63,  3,  4,  6, 12},
    { 71,  3,  4,  6, 12},
    { 80,  4,  5,  7, 13},
    { 90,  4,  5,  8, 13},
    {101,  4,  6,  9, 14},
    {113,  5,  7, 10, 14},
    {127,  6,  8, 11, 15},
    {144,  6,  8, 13, 15},
    {162,  7, 10, 14, 16},
    {182,  8, 11, 16, 16},
    {203,  9, 12, 18, 17},
    {226, 10, 13, 20, 17},
    {255, 11, 15, 23, 18},
    {255, 13, 17, 25, 18},
};

/************************************************************************/
/*  Adjustable encoder parameters. Initial MIN_QP values never used     */
/************************************************************************/

ADJUSTABLE uint16_t g_rnd_inter[] = {
    11665, 11665, 11665, 11665, 11665, 11665, 11665, 11665, 11665, 11665,
    11665, 12868, 14071, 15273, 16476,
    17679, 17740, 17801, 17863, 17924,
    17985, 17445, 16904, 16364, 15823,
    15283, 15198, 15113, 15027, 14942,
    14857, 15667, 16478, 17288, 18099,
    18909, 19213, 19517, 19822, 20126,
    20430, 16344, 12259, 8173, 4088,
    4088, 4088, 4088, 4088, 4088,
    4088, 4088,
};

ADJUSTABLE uint16_t g_thr_inter[] = {
    31878, 31878, 31878, 31878, 31878, 31878, 31878, 31878, 31878, 31878,
    31878, 33578, 35278, 36978, 38678,
    40378, 41471, 42563, 43656, 44748,
    45841, 46432, 47024, 47615, 48207,
    48798, 49354, 49911, 50467, 51024,
    51580, 51580, 51580, 51580, 51580,
    51580, 52222, 52864, 53506, 54148,
    54790, 45955, 37120, 28286, 19451,
    10616, 9326, 8036, 6745, 5455,
    4165, 4165,
};

ADJUSTABLE uint16_t g_thr_inter2[] = {
    45352, 45352, 45352, 45352, 45352, 45352, 45352, 45352, 45352, 45352,
    45352, 41100, 36848, 32597, 28345,
    24093, 25904, 27715, 29525, 31336,
    33147, 33429, 33711, 33994, 34276,
    34558, 32902, 31246, 29590, 27934,
    26278, 26989, 27700, 28412, 29123,
    29834, 29038, 28242, 27445, 26649,
    25853, 23440, 21028, 18615, 16203,
    13790, 11137, 8484, 5832, 3179,
    526, 526,
};

ADJUSTABLE uint16_t g_skip_thr_inter[52] =
{
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    45, 45, 45, 44, 44,
    44, 40, 37, 33, 30,
    26, 32, 38, 45, 51,
    57, 58, 58, 59, 59,
    60, 66, 73, 79, 86,
    92, 95, 98, 100, 103,
    106, 200, 300, 400, 500,
    600, 700, 800, 900, 1000,
    1377, 1377,
};

ADJUSTABLE uint16_t g_lambda_q4[52] =
{
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 13, 11, 10, 8,
    7, 11, 15, 20, 24,
    28, 30, 31, 33, 34,
    36, 48, 60, 71, 83,
    95, 95, 95, 96, 96,
    96, 113, 130, 147, 164,
    181, 401, 620, 840, 1059,
    1279, 1262, 1246, 1229, 1213,
    1196, 1196,
};
ADJUSTABLE uint16_t g_lambda_mv_q4[52] =
{
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 14, 15, 15, 16,
    17, 18, 20, 21, 23,
    24, 28, 32, 37, 41,
    45, 53, 62, 70, 79,
    87, 105, 123, 140, 158,
    176, 195, 214, 234, 253,
    272, 406, 541, 675, 810,
    944, 895, 845, 796, 746,
    697, 697,
};

ADJUSTABLE uint16_t g_skip_thr_i4x4[52] =
{
    0,1,2,3,4,5,6,7,8,9,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
    100, 100,
};

ADJUSTABLE uint16_t g_deadzonei[] = {
    3419, 3419, 3419, 3419, 3419, 3419, 3419, 3419, 3419, 3419,
    30550, 8845, 14271, 19698, 25124,
    30550, 29556, 28562, 27569, 26575,
    25581, 25284, 24988, 24691, 24395,
    24098, 24116, 24134, 24153, 24171,
    24189, 24010, 23832, 23653, 23475,
    23296, 23569, 23842, 24115, 24388,
    24661, 19729, 14797, 9865, 4933,
    24661, 3499, 6997, 10495, 13993,
    17491, 17491,
};

ADJUSTABLE uint16_t g_lambda_i4_q4[] = {
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 31, 34, 38, 41,
    45, 76, 106, 137, 167,
    198, 220, 243, 265, 288,
    310, 347, 384, 421, 458,
    495, 584, 673, 763, 852,
    941, 1053, 1165, 1276, 1388,
    1500, 1205, 910, 614, 319,
    5000, 1448, 2872, 4296, 5720,
    7144, 7144,
};

ADJUSTABLE uint16_t g_lambda_i16_q4[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 3, 7, 10, 14,
    17, 14, 10, 7, 3,
    50, 20, 39, 59, 78,
    98, 94, 89, 85, 80,
    76, 118, 161, 203, 246,
    288, 349, 410, 470, 531,
    592, 575, 558, 540, 523,
    506, 506,
};

const uint8_t g_diff_to_gainQ8[256] =
{
    0, 16, 25, 32, 37, 41, 44, 48, 50, 53, 55, 57, 59, 60, 62, 64, 65,
    66, 67, 69, 70, 71, 72, 73, 74, 75, 76, 76, 77, 78, 79, 80, 80,
    81, 82, 82, 83, 83, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89,
    90, 90, 91, 91, 92, 92, 92, 93, 93, 94, 94, 94, 95, 95, 96, 96,
    96, 97, 97, 97, 98, 98, 98, 99, 99, 99, 99, 100, 100, 100, 101, 101,
    101, 102, 102, 102, 102, 103, 103, 103, 103, 104, 104, 104, 104, 105, 105, 105,
    105, 106, 106, 106, 106, 106, 107, 107, 107, 107, 108, 108, 108, 108, 108, 109,
    109, 109, 109, 109, 110, 110, 110, 110, 110, 111, 111, 111, 111, 111, 112, 112,
    112, 112, 112, 112, 113, 113, 113, 113, 113, 113, 114, 114, 114, 114, 114, 114,
    115, 115, 115, 115, 115, 115, 115, 116, 116, 116, 116, 116, 116, 117, 117, 117,
    117, 117, 117, 117, 118, 118, 118, 118, 118, 118, 118, 118, 119, 119, 119, 119,
    119, 119, 119, 119, 120, 120, 120, 120, 120, 120, 120, 120, 121, 121, 121, 121,
    121, 121, 121, 121, 122, 122, 122, 122, 122, 122, 122, 122, 122, 123, 123, 123,
    123, 123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 124, 124, 124, 124, 125,
    125, 125, 125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126,
    126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 128,
};

#if H264E_ENABLE_SSE2 && !defined(MINIH264_ASM)
#define BS_BITS 32

static void h264e_bs_put_bits_sse2(bs_t *bs, unsigned n, unsigned val)
{
    assert(!(val >> n));
    bs->shift -= n;
    assert((unsigned)n <= 32);
    if (bs->shift < 0)
    {
        assert(-bs->shift < 32);
        bs->cache |= val >> -bs->shift;
        *bs->buf++ = SWAP32(bs->cache);
        bs->shift = 32 + bs->shift;
        bs->cache = 0;
    }
    bs->cache |= val << bs->shift;
}

static void h264e_bs_flush_sse2(bs_t *bs)
{
    *bs->buf = SWAP32(bs->cache);
}

static unsigned h264e_bs_get_pos_bits_sse2(const bs_t *bs)
{
    unsigned pos_bits = (unsigned)((bs->buf - bs->origin)*BS_BITS);
    pos_bits += BS_BITS - bs->shift;
    assert((int)pos_bits >= 0);
    return pos_bits;
}

static unsigned h264e_bs_byte_align_sse2(bs_t *bs)
{
    int pos = h264e_bs_get_pos_bits_sse2(bs);
    h264e_bs_put_bits_sse2(bs, -pos & 7, 0);
    return pos + (-pos & 7);
}

/**
*   Golomb code
*   0 => 1
*   1 => 01 0
*   2 => 01 1
*   3 => 001 00
*   4 => 001 01
*
*   [0]     => 1
*   [1..2]  => 01x
*   [3..6]  => 001xx
*   [7..14] => 0001xxx
*
*/
static void h264e_bs_put_golomb_sse2(bs_t *bs, unsigned val)
{
    int size;
#if defined(_MSC_VER)
    unsigned long nbit;
    _BitScanReverse(&nbit, val + 1);
    size = 1 + nbit;
#else
    size = 32 - __builtin_clz(val + 1);
#endif
    h264e_bs_put_bits_sse2(bs, 2*size - 1, val + 1);
}

/**
*   signed Golomb code.
*   mapping to unsigned code:
*       0 => 0
*       1 => 1
*      -1 => 2
*       2 => 3
*      -2 => 4
*       3 => 5
*      -3 => 6
*/
static void h264e_bs_put_sgolomb_sse2(bs_t *bs, int val)
{
    val = 2*val - 1;
    val ^= val >> 31;
    h264e_bs_put_golomb_sse2(bs, val);
}

static void h264e_bs_init_bits_sse2(bs_t *bs, void *data)
{
    bs->origin = data;
    bs->buf = bs->origin;
    bs->shift = BS_BITS;
    bs->cache = 0;
}

static unsigned __clz_cavlc(unsigned v)
{
#if defined(_MSC_VER)
    unsigned long nbit;
    _BitScanReverse(&nbit, v);
    return 31 - nbit;
#else
    return __builtin_clz(v);
#endif
}

static void h264e_vlc_encode_sse2(bs_t *bs, int16_t *quant, int maxNumCoeff, uint8_t *nz_ctx)
{
    int nnz_context, nlevels, nnz; // nnz = nlevels + trailing_ones
    unsigned trailing_ones = 0;
    unsigned trailing_ones_sign = 0;
    uint8_t runs[16];
    uint8_t *prun = runs;
    int16_t *levels;
    int cloop = maxNumCoeff;
    int v, drun;
    unsigned zmask;
    BS_OPEN(bs)

    ALIGN(16) int16_t zzquant[16] ALIGN2(16);
    levels = zzquant + ((maxNumCoeff == 4) ? 4 : 16);
    if (maxNumCoeff != 4)
    {
        __m128i y0, y1;
        __m128i x0 = _mm_load_si128((__m128i *)quant);
        __m128i x1 = _mm_load_si128((__m128i *)(quant + 8));
#define SWAP_XMM(x, i, j)     { int t0 = _mm_extract_epi16(x, i); int t1 = _mm_extract_epi16(x, j); x = _mm_insert_epi16(x, t0, j); x = _mm_insert_epi16(x, t1, i); }
#define SWAP_XMM2(x, y, i, j) { int t0 = _mm_extract_epi16(x, i); int t1 = _mm_extract_epi16(y, j); y = _mm_insert_epi16(y, t0, j); x = _mm_insert_epi16(x, t1, i); }
        SWAP_XMM(x0, 3, 4);
        SWAP_XMM(x1, 3, 4);
        SWAP_XMM2(x0, x1, 5, 2);
        x0 = _mm_shufflelo_epi16(x0, 0 + (3 << 2) + (1 << 4) + (2 << 6));
        x0 = _mm_shufflehi_epi16(x0, 2 + (0 << 2) + (3 << 4) + (1 << 6));
        x1 = _mm_shufflelo_epi16(x1, 2 + (0 << 2) + (3 << 4) + (1 << 6));
        x1 = _mm_shufflehi_epi16(x1, 1 + (2 << 2) + (0 << 4) + (3 << 6));
        y0 = _mm_unpacklo_epi64(x0, x1);
        y1 = _mm_unpackhi_epi64(x0, x1);
        y0 = _mm_slli_epi16(y0, 1);
        y1 = _mm_slli_epi16(y1, 1);
        zmask = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_packs_epi16(y0, y1), _mm_setzero_si128()));
        _mm_store_si128((__m128i *)zzquant, y0);
        _mm_store_si128((__m128i *)(zzquant + 8), y1);

        if (maxNumCoeff == 15)
            zmask |= 1;
        zmask = (~zmask) << 16;

        v = 15;
        drun = (maxNumCoeff == 16) ? 1 : 0;
    } else
    {
        __m128i x0 = _mm_loadl_epi64((__m128i *)quant);
        x0 = _mm_slli_epi16(x0, 1);
        zmask = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_packs_epi16(x0, x0), _mm_setzero_si128()));
        _mm_storel_epi64((__m128i *)zzquant, x0);
        zmask = (~zmask) << 28;
        drun = 1;
        v = 3;
    }

    if (zmask)
    {
        do
        {
            int i = __clz_cavlc(zmask);
            *--levels = zzquant[v -= i];
            *prun++ = (uint8_t)(v + drun);
            zmask <<= (i + 1);
            v--;
        } while(zmask);
        quant = zzquant + ((maxNumCoeff == 4) ? 4 : 16);
        nnz = (int)(quant - levels);

        cloop = MIN(3, nnz);
        levels = quant - 1;
        do
        {
            if ((unsigned)(*levels + 2) > 4u)
            {
                break;
            }
            trailing_ones_sign = (trailing_ones_sign << 1) | (*levels-- < 0);
            trailing_ones++;
        } while (--cloop);
    } else
    {
        nnz = trailing_ones = 0;
    }
    nlevels = nnz - trailing_ones;

    nnz_context = nz_ctx[-1] + nz_ctx[1];

    nz_ctx[0] = (uint8_t)nnz;
    if (nnz_context <= 34)
    {
        nnz_context = (nnz_context + 1) >> 1;
    }
    nnz_context &= 31;

    // 9.2.1 Parsing process for total number of transform coefficient levels and trailing ones
    {
        int off = h264e_g_coeff_token[nnz_context];
        unsigned n = 6, val = h264e_g_coeff_token[off + trailing_ones + 4*nlevels];
        if (off != 230)
        {
            n = (val & 15) + 1;
            val >>= 4;
        }
        BS_PUT(n, val);
    }

    if (nnz)
    {
        if (trailing_ones)
        {
            BS_PUT(trailing_ones, trailing_ones_sign);
        }
        if (nlevels)
        {
            int vlcnum = 1;
            int sym_len, prefix_len;

            int sym = *levels-- - 2;
            if (sym < 0) sym = -3 - sym;
            if (sym >= 6) vlcnum++;
            if (trailing_ones < 3)
            {
                sym -= 2;
                if (nnz > 10)
                {
                    sym_len = 1;
                    prefix_len = sym >> 1;
                    if (prefix_len >= 15)
                    {
                        // or vlcnum = 1;  goto escape;
                        prefix_len = 15;
                        sym_len = 12;
                    }
                    sym -= prefix_len << 1;
                    // bypass vlcnum advance due to sym -= 2; above
                    goto loop_enter;
                }
            }

            if (sym < 14)
            {
                prefix_len = sym;
                sym = 0; // to avoid side effect in bitbuf
                sym_len = 0;
            } else if (sym < 30)
            {
                prefix_len = 14;
                sym_len = 4;
                sym -= 14;
            } else
            {
                vlcnum = 1;
                goto escape;
            }
            goto loop_enter;

            for (;;)
            {
                sym_len = vlcnum;
                prefix_len = sym >> vlcnum;
                if (prefix_len >= 15)
                {
escape:
                    prefix_len = 15;
                    sym_len = 12;
                }
                sym -= prefix_len << vlcnum;

                if (prefix_len >= 3 && vlcnum < 6) vlcnum++;
loop_enter:
                sym |= 1 << sym_len;
                sym_len += prefix_len+1;
                BS_PUT(sym_len, (unsigned)sym);
                if (!--nlevels) break;
                sym = *levels-- - 2;
                if (sym < 0) sym = -3 - sym;
            }
        }

        if (nnz < maxNumCoeff)
        {
            const uint8_t *vlc = (maxNumCoeff == 4) ? h264e_g_total_zeros_cr_2x2 : h264e_g_total_zeros;
            uint8_t *run = runs;
            int run_prev = *run++;
            int nzeros = run_prev - nnz;
            int zeros_left = 2*nzeros - 1;
            int ctx = nnz - 1;
            run[nnz - 1] = (uint8_t)maxNumCoeff; // terminator
            for(;;)
            {
                int t;
                //encode_huff8(bs, vlc, ctx, nzeros);

                unsigned val = vlc[vlc[ctx] + nzeros];
                unsigned n = val & 15;
                val >>= 4;
                BS_PUT(n, val);

                zeros_left -= nzeros;
                if (zeros_left < 0)
                {
                    break;
                }

                t = *run++;
                nzeros = run_prev - t - 1;
                if (nzeros < 0)
                {
                    break;
                }
                run_prev = t;
                assert(zeros_left < 14);
                vlc = h264e_g_run_before;
                ctx = zeros_left;
            }
        }
    }
    BS_CLOSE(bs);
}

#define MM_LOAD_8TO16_2(p) _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(p)), _mm_setzero_si128())
static __inline __m128i subabs128_16(__m128i a, __m128i b)
{
    return _mm_or_si128(_mm_subs_epu16(a, b), _mm_subs_epu16(b, a));
}
static __inline __m128i clone2x16(const void *p)
{
    __m128i tmp = MM_LOAD_8TO16_2(p);
    return _mm_unpacklo_epi16(tmp, tmp);
}
static __inline __m128i subabs128(__m128i a, __m128i b)
{
    return _mm_or_si128(_mm_subs_epu8(a, b), _mm_subs_epu8(b, a));
}

static void transpose8x8_sse(uint8_t *dst, int dst_stride, uint8_t *src, int src_stride)
{
    __m128i a = _mm_loadl_epi64((__m128i *)(src));
    __m128i b = _mm_loadl_epi64((__m128i *)(src += src_stride));
    __m128i c = _mm_loadl_epi64((__m128i *)(src += src_stride));
    __m128i d = _mm_loadl_epi64((__m128i *)(src += src_stride));
    __m128i e = _mm_loadl_epi64((__m128i *)(src += src_stride));
    __m128i f = _mm_loadl_epi64((__m128i *)(src += src_stride));
    __m128i g = _mm_loadl_epi64((__m128i *)(src += src_stride));
    __m128i h = _mm_loadl_epi64((__m128i *)(src += src_stride));

    __m128i p0 = _mm_unpacklo_epi8(a,b);  // b7 a7 b6 a6 ... b0 a0
    __m128i p1 = _mm_unpacklo_epi8(c,d);  // d7 c7 d6 c6 ... d0 c0
    __m128i p2 = _mm_unpacklo_epi8(e,f);  // f7 e7 f6 e6 ... f0 e0
    __m128i p3 = _mm_unpacklo_epi8(g,h);  // h7 g7 h6 g6 ... h0 g0

    __m128i q0 = _mm_unpacklo_epi16(p0, p1);  // d3c3 b3a3 ... d0c0 b0a0
    __m128i q1 = _mm_unpackhi_epi16(p0, p1);  // d7c7 b7a7 ... d4c4 b4a4
    __m128i q2 = _mm_unpacklo_epi16(p2, p3);  // h3g3 f3e3 ... h0g0 f0e0
    __m128i q3 = _mm_unpackhi_epi16(p2, p3);  // h7g7 f7e7 ... h4g4 f4e4

    __m128i r0 = _mm_unpacklo_epi32(q0, q2);  // h1g1f1e1 d1c1b1a1 h0g0f0e0 d0c0b0a0
    __m128i r1 = _mm_unpackhi_epi32(q0, q2);  // h3g3f3e3 d3c3b3a3 h2g2f2e2 d2c2b2a2
    __m128i r2 = _mm_unpacklo_epi32(q1, q3);
    __m128i r3 = _mm_unpackhi_epi32(q1, q3);
    _mm_storel_epi64((__m128i *)(dst), r0); dst += dst_stride; _mm_storel_epi64((__m128i *)(dst), _mm_unpackhi_epi64(r0, r0)); dst += dst_stride;
    _mm_storel_epi64((__m128i *)(dst), r1); dst += dst_stride; _mm_storel_epi64((__m128i *)(dst), _mm_unpackhi_epi64(r1, r1)); dst += dst_stride;
    _mm_storel_epi64((__m128i *)(dst), r2); dst += dst_stride; _mm_storel_epi64((__m128i *)(dst), _mm_unpackhi_epi64(r2, r2)); dst += dst_stride;
    _mm_storel_epi64((__m128i *)(dst), r3); dst += dst_stride; _mm_storel_epi64((__m128i *)(dst), _mm_unpackhi_epi64(r3, r3)); dst += dst_stride;
}

static void deblock_chroma_h_s4_sse(uint8_t *pq0, int stride, const void* threshold, int alpha, int beta, uint32_t argstr)
{
    __m128i thr, str, d;
    __m128i p1 = MM_LOAD_8TO16_2(pq0 - 2*stride);
    __m128i p0 = MM_LOAD_8TO16_2(pq0 - stride);
    __m128i q0 = MM_LOAD_8TO16_2(pq0);
    __m128i q1 = MM_LOAD_8TO16_2(pq0 + stride);
    __m128i zero = _mm_setzero_si128();
    __m128i _alpha = _mm_set1_epi16((short)alpha);
    __m128i _beta = _mm_set1_epi16((short)beta);
    __m128i tmp;

    str =                    _mm_cmplt_epi16(subabs128_16(p0, q0), _alpha);
    str = _mm_and_si128(str, _mm_cmplt_epi16(_mm_max_epi16(subabs128_16(p1, p0), subabs128_16(q1, q0)), _beta));

    if ((uint8_t)argstr != 4)
    {
        d = _mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_sub_epi16(q0, p0), 2), p1), q1),_mm_set1_epi16(4)), 3);
        thr = _mm_add_epi16(clone2x16(threshold), _mm_set1_epi16(1));
        d = _mm_min_epi16(_mm_max_epi16(d, _mm_sub_epi16(zero, thr)), thr);

        tmp = _mm_unpacklo_epi8(_mm_cvtsi32_si128(argstr), _mm_setzero_si128());
        tmp = _mm_unpacklo_epi16(tmp, tmp);

//        str = _mm_and_si128(str, _mm_cmpgt_epi16(clone2x16(strength), zero));
        str = _mm_and_si128(str, _mm_cmpgt_epi16(tmp, zero));
        d = _mm_and_si128(str, d);
        p0 = _mm_add_epi16(p0, d);
        q0 = _mm_sub_epi16(q0, d);
    } else
    {
        __m128i pq = _mm_add_epi16(p1, q1);
        __m128i newp = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(pq, p1), p0), 1);
        __m128i newq = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(pq, q1), q0), 1);
        p0 = _mm_xor_si128(_mm_and_si128(_mm_xor_si128(_mm_avg_epu16(newp,zero), p0), str), p0);
        q0 = _mm_xor_si128(_mm_and_si128(_mm_xor_si128(_mm_avg_epu16(newq,zero), q0), str), q0);
    }
    _mm_storel_epi64((__m128i*)(pq0 - stride), _mm_packus_epi16(p0, zero));
    _mm_storel_epi64((__m128i*)(pq0         ), _mm_packus_epi16(q0, zero));
}

static void deblock_chroma_v_s4_sse(uint8_t *pix, int stride, const void* threshold, int alpha, int beta, uint32_t str)
{
    uint8_t t8x4[8*4];
    int i;
    uint8_t *p = pix - 2;
    __m128i t0 =_mm_unpacklo_epi16(
        _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int_u*)p),              _mm_cvtsi32_si128(*(int_u*)(p + stride))),
        _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int_u*)(p + 2*stride)), _mm_cvtsi32_si128(*(int_u*)(p + 3*stride)))
        );
    __m128i t1 =_mm_unpacklo_epi16(
        _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int_u*)(p + 4*stride)), _mm_cvtsi32_si128(*(int_u*)(p + 5*stride))),
        _mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int_u*)(p + 6*stride)), _mm_cvtsi32_si128(*(int_u*)(p + 7*stride)))
        );
    __m128i p1 = _mm_unpacklo_epi32(t0, t1);
    __m128i p0 = _mm_shuffle_epi32 (p1, 0x4E); // 01001110b
    __m128i q0 = _mm_unpackhi_epi32(t0, t1);
    __m128i q1 = _mm_shuffle_epi32 (q0, 0x4E);
    _mm_storel_epi64((__m128i*)(t8x4), p1);
    _mm_storel_epi64((__m128i*)(t8x4 + 8), p0);
    _mm_storel_epi64((__m128i*)(t8x4 + 16), q0);
    _mm_storel_epi64((__m128i*)(t8x4 + 24), q1);
    deblock_chroma_h_s4_sse(t8x4 + 16, 8, threshold, alpha, beta, str);

    for (i = 0; i < 8; i++)
    {
        pix[-1] = t8x4[8  + i];
        pix[ 0] = t8x4[16 + i];
        pix += stride;
    }
}

#define CMP_BETA(p, q, beta)   _mm_cmpeq_epi8(_mm_subs_epu8(_mm_subs_epu8(p, q), beta), _mm_subs_epu8(_mm_subs_epu8(q, p), beta))
#define CMP_1(p, q, beta)     (_mm_subs_epu8(subabs128(p, q), beta))

static void deblock_luma_h_s4_sse(uint8_t *pix, int stride, int alpha, int beta)
{
    int ccloop = 2;
    do
    {
        __m128i p3 = MM_LOAD_8TO16_2(pix - 4*stride);
        __m128i p2 = MM_LOAD_8TO16_2(pix - 3*stride);
        __m128i p1 = MM_LOAD_8TO16_2(pix - 2*stride);
        __m128i p0 = MM_LOAD_8TO16_2(pix - stride);
        __m128i q0 = MM_LOAD_8TO16_2(pix);
        __m128i q1 = MM_LOAD_8TO16_2(pix + stride);
        __m128i q2 = MM_LOAD_8TO16_2(pix + 2*stride);
        __m128i q3 = MM_LOAD_8TO16_2(pix + 3*stride);
        __m128i zero = _mm_setzero_si128();
        __m128i _alpha = _mm_set1_epi16((short)alpha);
        __m128i _quarteralpha = _mm_set1_epi16((short)((alpha >> 2) + 2));
        __m128i _beta = _mm_set1_epi16((short)beta);
        __m128i ap_less_beta;
        __m128i aq_less_beta;
        __m128i str;
        __m128i pq;
        __m128i short_p;
        __m128i short_q;
        __m128i long_p;
        __m128i long_q;
        __m128i t;
        __m128i p0q0_less__quarteralpha;

        __m128i absdif_p0_q0 = subabs128_16(p0, q0);
        __m128i p0_plus_q0 = _mm_add_epi16(_mm_add_epi16(p0, q0), _mm_set1_epi16(2));

        // if (abs_p0_q0 < alpha && abs_p1_p0 < beta && abs_q1_q0 < beta)
        str = _mm_cmplt_epi16(absdif_p0_q0, _alpha);
        //str = _mm_and_si128(str, _mm_cmplt_epi16(subabs128_16(p1, p0), _beta));
        //str = _mm_and_si128(str, _mm_cmplt_epi16(subabs128_16(q1, q0), _beta));
        str = _mm_and_si128(str, _mm_cmplt_epi16(_mm_max_epi16(subabs128_16(p1, p0), subabs128_16(q1, q0)), _beta));
        p0q0_less__quarteralpha = _mm_and_si128(_mm_cmplt_epi16(absdif_p0_q0, _quarteralpha), str);

        //int short_p = (2*p1 + p0 + q1 + 2);
        //int short_q = (2*q1 + q0 + p1 + 2);
        short_p = _mm_avg_epu8(_mm_avg_epu8(p0, q1),p1);
        pq = _mm_add_epi16(_mm_add_epi16(p1, q1), _mm_set1_epi16(2));
        short_p = _mm_add_epi16(_mm_add_epi16(pq, p1), p0);
        short_q = _mm_add_epi16(_mm_add_epi16(pq, q1), q0);

        ap_less_beta = _mm_and_si128(_mm_cmplt_epi16(subabs128_16(p2, p0), _beta), p0q0_less__quarteralpha);
        t = _mm_add_epi16(_mm_add_epi16(p2, p1), p0_plus_q0);
        // short_p += t - p1 + q0;
        long_p = _mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_mm_add_epi16(short_p, t), p1), q0), 1);

        _mm_storel_epi64((__m128i*)(pix - 2*stride), _mm_packus_epi16(_mm_or_si128(_mm_and_si128(ap_less_beta, _mm_srai_epi16(t, 2)), _mm_andnot_si128(ap_less_beta, p1)), zero));
        t = _mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(p3, p2), 1), t), _mm_set1_epi16(2));
        _mm_storel_epi64((__m128i*)(pix - 3*stride), _mm_packus_epi16(_mm_or_si128(_mm_and_si128(ap_less_beta, _mm_srai_epi16(t, 3)), _mm_andnot_si128(ap_less_beta, p2)), zero));

        aq_less_beta = _mm_and_si128(_mm_cmplt_epi16(subabs128_16(q2, q0), _beta), p0q0_less__quarteralpha);
        t = _mm_add_epi16(_mm_add_epi16(q2, q1), p0_plus_q0);
        long_q = _mm_srai_epi16(_mm_add_epi16(_mm_sub_epi16(_mm_add_epi16(short_q, t), q1), p0), 1);
        _mm_storel_epi64((__m128i*)(pix + 1*stride), _mm_packus_epi16(_mm_or_si128(_mm_and_si128(aq_less_beta, _mm_srai_epi16(t, 2)), _mm_andnot_si128(aq_less_beta, q1)), zero));

        t = _mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(q3, q2), 1), t), _mm_set1_epi16(2));
        _mm_storel_epi64((__m128i*)(pix + 2*stride), _mm_packus_epi16(_mm_or_si128(_mm_and_si128(aq_less_beta, _mm_srai_epi16(t, 3)), _mm_andnot_si128(aq_less_beta, q2)), zero));

        short_p = _mm_srai_epi16(_mm_or_si128(_mm_and_si128(ap_less_beta, long_p), _mm_andnot_si128(ap_less_beta, short_p)), 2);
        short_q = _mm_srai_epi16(_mm_or_si128(_mm_and_si128(aq_less_beta, long_q), _mm_andnot_si128(aq_less_beta, short_q)), 2);

        _mm_storel_epi64((__m128i*)(pix - stride), _mm_packus_epi16(_mm_or_si128(_mm_and_si128(str, short_p), _mm_andnot_si128(str, p0)), zero));
        _mm_storel_epi64((__m128i*)(pix         ), _mm_packus_epi16(_mm_or_si128(_mm_and_si128(str, short_q), _mm_andnot_si128(str, q0)), zero));

        pix += 8;
    } while (--ccloop);
}

static void deblock_luma_v_s4_sse(uint8_t *pix, int stride, int alpha, int beta)
{
    __m128i scratch[8];
    uint8_t *s = pix - 4;
    uint8_t *dst = (uint8_t *)scratch;
    int cloop = 2;
    do
    {
        transpose8x8_sse(dst, 16, s, stride);
        s += 8*stride;
        dst += 8;
    } while(--cloop);

    deblock_luma_h_s4_sse((uint8_t *)(scratch+4), 16, alpha, beta);
    s = pix - 4;
    dst = (uint8_t *)scratch;
    cloop = 2;
    do
    {
        transpose8x8_sse(s, stride, dst, 16);
        s += 8*stride;
        dst += 8;
    } while(--cloop);
}

// (a-b) >> 1s == ((a + ~b + 1) >> 1u) - 128;
//
// delta = (((q0-p0)<<2) + (p1-q1) + 4) >> 3 =
//          (4*q0 - 4*p0 + p1 - q1 + 4) >> 3 =
//          ((p1-p0) - (q1-q0) - 3*(p0-q0) + 4) >> 3
//          ((p1-p0) - (q1-q0) - 3*p0 + 3*q0) + 4) >> 3
//          (((p1-p0)-p0)>>1 - ((q1-q0)-q0)>>1 - p0 + q0) + 2) >> 2
//          ((((p1-p0)-p0)>>1 - p0)>>1 - (((q1-q0)-q0)>>1 - q0)>>1) + 1) >> 1
static void deblock_luma_h_s3_sse(uint8_t *h264e_restrict pix, int stride, int alpha, int beta, const void* threshold, uint32_t strength)
{
    __m128i p1 = _mm_loadu_si128((__m128i *)(pix - 2*stride));
    __m128i p0 = _mm_loadu_si128((__m128i *)(pix - stride));
    __m128i q0 = _mm_loadu_si128((__m128i *)pix);
    __m128i q1 = _mm_loadu_si128((__m128i *)(pix + stride));
    __m128i maskp, maskq, zeromask, thr;
    __m128i tc0tmp, p2, q2, p0q0avg, _beta;

#define HALFSUM(x, y) _mm_sub_epi8(_mm_avg_epu8(x, y), _mm_and_si128(_mm_xor_si128(y, x), _mm_set1_epi8(1)))

    // if (ABS(p0-q0) - alpha) ...
    zeromask = _mm_subs_epu8(subabs128(p0, q0), _mm_set1_epi8((int8_t)(alpha - 1)));
    //  & (ABS(p1-p0) - beta) & (ABS(q1-q0) - beta)
    _beta = _mm_set1_epi8((int8_t)(beta - 1));
    zeromask = _mm_or_si128(zeromask, _mm_subs_epu8(_mm_max_epu8(subabs128(p1, p0), subabs128(q1, q0)), _beta));
    zeromask = _mm_cmpeq_epi8(zeromask, _mm_setzero_si128());

    {
        __m128i str_x = _mm_cvtsi32_si128(strength);
        str_x = _mm_unpacklo_epi8(str_x, str_x);
        str_x = _mm_cmpgt_epi8(_mm_unpacklo_epi8(str_x, str_x), _mm_setzero_si128());
        zeromask = _mm_and_si128(zeromask, str_x);
    }

    thr = _mm_cvtsi32_si128(*(int*)threshold);//_mm_loadl_epi64((__m128i *)(threshold));
    thr = _mm_unpacklo_epi8(thr, thr);
    thr = _mm_unpacklo_epi8(thr, thr);
    thr = _mm_and_si128(thr, zeromask);

    p2 = _mm_loadu_si128((__m128i *)(pix - 3*stride));
    maskp = CMP_BETA(p2, p0, _beta);
    tc0tmp = _mm_and_si128(thr, maskp);
    p0q0avg = _mm_avg_epu8(p0, q0);     // (p0+q0+1)>>1
    _mm_storeu_si128((__m128i *)(pix - 2*stride), _mm_min_epu8(_mm_max_epu8(HALFSUM(p2, p0q0avg), _mm_subs_epu8(p1, tc0tmp)), _mm_adds_epu8(p1, tc0tmp)));

    q2 = _mm_loadu_si128((__m128i *)(pix + 2*stride));
    maskq = CMP_BETA(q2, q0, _beta);
    tc0tmp = _mm_and_si128(thr, maskq);
    _mm_storeu_si128((__m128i *)(pix + stride),  _mm_min_epu8(_mm_max_epu8(HALFSUM(q2, p0q0avg), _mm_subs_epu8(q1, tc0tmp)), _mm_adds_epu8(q1, tc0tmp)));

    thr = _mm_sub_epi8(thr, maskp);
    thr = _mm_sub_epi8(thr, maskq);
    thr = _mm_and_si128(thr, zeromask);

    {
    __m128i ff = _mm_set1_epi8(0xff);
    __m128i part1 = _mm_avg_epu8(q0, _mm_xor_si128(p0, ff));
    __m128i part2 = _mm_avg_epu8(p1, _mm_xor_si128(q1, ff));
    __m128i carry = _mm_and_si128(_mm_xor_si128(p0, q0), _mm_set1_epi8(1));
    __m128i d = _mm_adds_epu8(part1, _mm_avg_epu8(_mm_avg_epu8(part2, _mm_set1_epi8(3)), carry));
    __m128i delta_p = _mm_subs_epu8(d, _mm_set1_epi8((char)(128 + 33)));
    __m128i delta_n = _mm_subs_epu8(_mm_set1_epi8((char)(128 + 33)), d);
    delta_p = _mm_min_epu8(delta_p, thr);
    delta_n = _mm_min_epu8(delta_n, thr);

    q0 =  _mm_adds_epu8(_mm_subs_epu8(q0, delta_p), delta_n);
    p0 =  _mm_subs_epu8(_mm_adds_epu8(p0, delta_p), delta_n);

    _mm_storeu_si128 ((__m128i *)(pix - stride), p0);
    _mm_storeu_si128 ((__m128i *)pix,            q0);
    }
}

static void deblock_luma_v_s3_sse(uint8_t *pix, int stride, int alpha, int beta, const void* thr, uint32_t strength)
{
    __m128i scratch[8];
    uint8_t *s = pix - 4;
    uint8_t *dst = (uint8_t *)scratch;
    int cloop = 2;
    do
    {
        transpose8x8_sse(dst, 16, s, stride);
        s += 8*stride;
        dst += 8;
    } while(--cloop);

    deblock_luma_h_s3_sse((uint8_t*)(scratch + 4), 16, alpha, beta, thr, strength);
    s = pix - 4;
    dst = (uint8_t *)scratch;
    cloop = 2;
    do
    {
        transpose8x8_sse(s, stride, dst, 16);
        s += 8*stride;
        dst += 8;
    } while(--cloop);
}

static void h264e_deblock_chroma_sse2(uint8_t *pix, int32_t stride, const deblock_params_t *par)
{
    const uint8_t *alpha = par->alpha;
    const uint8_t *beta = par->beta;
    const uint8_t *thr = par->tc0;
    const uint8_t *strength = (uint8_t *)par->strength32;
    int a, b, x, y;
    a = alpha[0];
    b = beta[0];
    for (x = 0; x < 16; x += 8)
    {
        uint32_t str = *(uint32_t*)&strength[x];
        if (str && a)
        {
            deblock_chroma_v_s4_sse(pix + (x >> 1), stride, thr + x, a, b, str);
        }
        a = alpha[1];
        b = beta[1];
    }
    thr += 16;
    strength += 16;
    a = alpha[2];
    b = beta[2];
    for (y = 0; y < 16; y += 8)
    {
        uint32_t str = *(uint32_t*)&strength[y];
        if (str && a)
        {
            deblock_chroma_h_s4_sse(pix, stride, thr + y, a, b, str);
        }
        pix += 4*stride;
        a = alpha[3];
        b = beta[3];
    }
}

static void h264e_deblock_luma_sse2(uint8_t *pix, int32_t stride, const deblock_params_t *par)
{
    const uint8_t *alpha = par->alpha;
    const uint8_t *beta = par->beta;
    const uint8_t *thr = par->tc0;
    const uint8_t *strength = (uint8_t *)par->strength32;
    int a, b, x, y;
    a = alpha[0];
    b = beta[0];
    for (x = 0; x < 16; x += 4)
    {
        uint32_t str = *(uint32_t*)&strength[x];
        if ((uint8_t)str == 4)
        {
            deblock_luma_v_s4_sse(pix + x, stride, a, b);
        } else if (str && a)
        {
            deblock_luma_v_s3_sse(pix + x, stride, a, b, thr + x, str);
        }
        a = alpha[1];
        b = beta[1];
    }
    thr += 16;
    strength += 16;
    a = alpha[2];
    b = beta[2];
    for (y = 0; y < 16; y += 4)
    {
        uint32_t str = *(uint32_t*)&strength[y];
        if ((uint8_t)str == 4)
        {
            deblock_luma_h_s4_sse(pix, stride, a, b);
        } else if (str && a)
        {
            deblock_luma_h_s3_sse(pix, stride, a, b, thr + y, str);
        }
        a = alpha[3];
        b = beta[3];
        pix += 4*stride;
    }
}

static void h264e_denoise_run_sse2(unsigned char *frm, unsigned char *frmprev, int w, int h_arg, int stride_frm, int stride_frmprev)
{
#define MM_LOAD_8TO16(p) _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(p)), zero)
    int cloop, h = h_arg;
    __m128i zero = _mm_setzero_si128();
    __m128i exp  = _mm_set1_epi32(0x7F800000);

    w -= 2;
    h -= 2;
    if (w <= 2 || h <= 2)
    {
        return;
    }

    do
    {
        unsigned char *pf = frm += stride_frm;
        unsigned char *pp = frmprev += stride_frmprev;
        cloop = w >> 3;
        pp[-stride_frmprev] = *pf++;
        pp++;

        while (cloop--)
        {
            __m128 float_val;
            __m128i log_neighbour, log_d;
            __m128i log_neighbour_h, log_neighbour_l, log_d_h, log_d_l;
            __m128i a, b;
            __m128i gain;
            __m128i abs_d, abs_neighbour;
            a = MM_LOAD_8TO16(pf);
            b = MM_LOAD_8TO16(pp);
            abs_d   = _mm_or_si128(_mm_subs_epu16(a, b), _mm_subs_epu16(b, a));
            a = MM_LOAD_8TO16(pf-stride_frm);
            a = _mm_add_epi16(a, MM_LOAD_8TO16(pf - 1));
            a = _mm_add_epi16(a, MM_LOAD_8TO16(pf + 1));
            a = _mm_add_epi16(a, MM_LOAD_8TO16(pf + stride_frm));
            b = MM_LOAD_8TO16(pp-stride_frmprev);
            b = _mm_add_epi16(b, MM_LOAD_8TO16(pp - 1));
            b = _mm_add_epi16(b, MM_LOAD_8TO16(pp + 1));
            b = _mm_add_epi16(b, MM_LOAD_8TO16(pp + stride_frmprev));

            abs_neighbour = _mm_or_si128(_mm_subs_epu16(a, b), _mm_subs_epu16(b, a));

            abs_neighbour = _mm_srai_epi16(abs_neighbour, 2);

            abs_d = _mm_add_epi16(abs_d, _mm_set1_epi16(1));
            abs_neighbour = _mm_add_epi16(abs_neighbour, _mm_set1_epi16(1));

            float_val = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(abs_neighbour, zero), 16), 16));
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            log_neighbour_l  = _mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(_mm_castps_si128(float_val), exp), 23), _mm_set1_epi32(127));

            float_val = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(abs_neighbour, zero), 16), 16));
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            log_neighbour_h  = _mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(_mm_castps_si128(float_val), exp), 23), _mm_set1_epi32(127));

            float_val = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(abs_d, zero), 16), 16));
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            log_d_l = _mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(_mm_castps_si128(float_val), exp), 23), _mm_set1_epi32(127));

            float_val = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(abs_d, zero), 16), 16));
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            float_val = _mm_mul_ps(float_val, float_val);
            log_d_h = _mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(_mm_castps_si128(float_val), exp), 23), _mm_set1_epi32(127));

            log_d = _mm_packs_epi32(log_d_l, log_d_h);
            log_neighbour = _mm_packs_epi32(log_neighbour_l, log_neighbour_h);

            log_neighbour = _mm_slli_epi16(log_neighbour, 8);
            log_neighbour = _mm_adds_epu16(log_neighbour, log_neighbour);
            log_neighbour = _mm_adds_epu16(log_neighbour, log_neighbour);
            log_neighbour = _mm_srli_epi16(log_neighbour, 8);

            log_neighbour = _mm_subs_epu16(_mm_set1_epi16(255), log_neighbour);
            log_d = _mm_subs_epu16(_mm_set1_epi16(255), log_d);

            gain = _mm_mullo_epi16(log_d, log_neighbour);

            a = MM_LOAD_8TO16(pf);
            b = MM_LOAD_8TO16(pp);
{
            __m128i s;
            __m128i gain_inv;
            gain_inv = _mm_sub_epi16(_mm_set1_epi8((char)255), gain);
            s = _mm_add_epi16(_mm_mulhi_epu16(a, gain_inv), _mm_mulhi_epu16(b, gain));
            b = _mm_mullo_epi16(b, gain);
            a = _mm_mullo_epi16(a, gain_inv);
            a = _mm_sub_epi16(_mm_avg_epu16(a, b), _mm_and_si128(_mm_xor_si128(a, b), _mm_set1_epi16(1)));
            a = _mm_avg_epu16(_mm_srli_epi16(a, 14), _mm_set1_epi16(0));
            a = _mm_add_epi16(a, s);
            _mm_storel_epi64((__m128i *)(pp-stride_frmprev), _mm_packus_epi16(a,zero));
}
            pf += 8;
            pp += 8;
        }

        cloop = w & 7;
        while (cloop--)
        {
            int d, neighbourhood;
            unsigned g, gd, gn, out_val;
            d = pf[0] - pp[0];
            neighbourhood  = pf[-1]      - pp[-1];
            neighbourhood += pf[+1]      - pp[+1];
            neighbourhood += pf[-stride_frm] - pp[-stride_frmprev];
            neighbourhood += pf[+stride_frm] - pp[+stride_frmprev];

            if (d < 0)
            {
                d = -d;
            }
            if (neighbourhood < 0)
            {
                neighbourhood = -neighbourhood;
            }
            neighbourhood >>= 2;

            gd = g_diff_to_gainQ8[d];
            gn = g_diff_to_gainQ8[neighbourhood];

            gn <<= 2;
            if (gn > 255)
            {
                gn = 255;
            }

            gn = 255 - gn;
            gd = 255 - gd;
            g = gn*gd;  // Q8*Q8 = Q16;

            //out_val = ((pp[0]*g ) >> 16) + (((0xffff-g)*pf[0] ) >> 16);
            out_val = (pp[0]*g + (0xffff-g)*pf[0]  + (1<<15)) >> 16;

            assert(out_val <= 255);

            pp[-stride_frmprev] = (unsigned char)out_val;

            pf++, pp++;
        }
        pp[-stride_frmprev] = *pf++;
    } while(--h);

    memcpy(frmprev + stride_frmprev, frm + stride_frm, w+2);
    h = h_arg - 2;
    do
    {
        memcpy(frmprev, frmprev - stride_frmprev, w+2);
        frmprev -= stride_frmprev;
    } while(--h);
    memcpy(frmprev, frm - stride_frm*(h_arg-2), w+2);
}

#define IS_NULL(p) ((p) < (pix_t *)(uintptr_t)32)

static uint32_t intra_predict_dc_sse(const pix_t *left, const pix_t *top, int log_side)
{
    unsigned dc = 0, side = 1u << log_side, round = 0;
    __m128i sum = _mm_setzero_si128();
    if (!IS_NULL(left))
    {
        int cloop = side;
        round += side >> 1;
        do
        {
            sum = _mm_add_epi64(sum, _mm_sad_epu8(_mm_cvtsi32_si128(*(int*)left), _mm_setzero_si128()));
            left += 4;
        } while (cloop -= 4);
    }
    if (!IS_NULL(top))
    {
        int cloop = side;
        round += side >> 1;
        do
        {
            sum = _mm_add_epi64(sum, _mm_sad_epu8(_mm_cvtsi32_si128(*(int*)top), _mm_setzero_si128()));
            top += 4;
        } while (cloop -= 4);
    }
    dc = _mm_cvtsi128_si32(sum);
    dc += round;
    if (round == side) dc >>= 1;
    dc >>= log_side;
    if (!round) dc = 128;
    return dc * 0x01010101;
}

/*
 * Note: To make the code more readable we refer to the neighboring pixels
 * in variables named as below:
 *
 *    UL U0 U1 U2 U3 U4 U5 U6 U7
 *    L0 xx xx xx xx
 *    L1 xx xx xx xx
 *    L2 xx xx xx xx
 *    L3 xx xx xx xx
 */
#define UL edge[-1]
#define U0 edge[0]
#define T1 edge[1]
#define U2 edge[2]
#define U3 edge[3]
#define U4 edge[4]
#define U5 edge[5]
#define U6 edge[6]
#define U7 edge[7]
#define L0 edge[-2]
#define L1 edge[-3]
#define L2 edge[-4]
#define L3 edge[-5]

static void h264e_intra_predict_16x16_sse2(pix_t *predict,  const pix_t *left, const pix_t *top, int mode)
{
    int cloop = 16;
    if (mode < 1)
    {
        __m128i a = _mm_load_si128((__m128i *)top);
        do
        {
            _mm_store_si128((__m128i *)predict, a);
            predict += 16;
        } while(--cloop);
    } else if (mode == 1)
    {
        const __m128i c1111 = _mm_set1_epi8(1);
        do
        {
            _mm_store_si128((__m128i *)predict, _mm_shuffle_epi32(_mm_mul_epu32(_mm_cvtsi32_si128(*left++), c1111), 0));
            predict += 16;
        } while(--cloop);
    } else //if (mode == 2)
    {
        __m128i dc128;
        int dc = intra_predict_dc_sse(left, top, 4);
        dc128 = _mm_shuffle_epi32(_mm_cvtsi32_si128(dc), 0);
        do
        {
            _mm_store_si128((__m128i *)predict, dc128);
            predict += 16;
        } while(--cloop);
    }
}

static void h264e_intra_predict_chroma_sse2(pix_t *predict, const pix_t *left, const pix_t *top, int mode)
{
    int cloop = 8;
    if (mode < 1)
    {
        __m128i a = _mm_load_si128((__m128i *)top);
        do
        {
            _mm_store_si128((__m128i *)predict, a);
            predict += 16;
        } while(--cloop);
    } else if (mode == 1)
    {
        do
        {
            __m128i t = _mm_unpacklo_epi32(_mm_cvtsi32_si128(left[0]*0x01010101u), _mm_cvtsi32_si128(left[8]*0x01010101u));
            t = _mm_unpacklo_epi32(t, t);
            _mm_store_si128((__m128i *)predict, t);
            left++;
            predict += 16;
        } while(--cloop);
    } else //if (mode == 2)
    {
        // chroma
        uint32_t *d = (uint32_t*)predict;
        __m128i *d128 = (__m128i *)predict;
        __m128i tmp;
        cloop = 2;
        do
        {
            d[0] = d[1] = d[16] = intra_predict_dc_sse(left, top, 2);
            d[17] = intra_predict_dc_sse(left + 4, top + 4, 2);
            if (!IS_NULL(top))
            {
                d[1] = intra_predict_dc_sse(NULL, top + 4, 2);
            }
            if (!IS_NULL(left))
            {
                d[16] = intra_predict_dc_sse(NULL, left + 4, 2);
            }
            d += 2;
            left += 8;
            top += 8;
        } while(--cloop);
        tmp = _mm_load_si128(d128++);
        _mm_store_si128(d128++, tmp);
        _mm_store_si128(d128++, tmp);
        _mm_store_si128(d128++, tmp);
        tmp = _mm_load_si128(d128++);
        _mm_store_si128(d128++, tmp);
        _mm_store_si128(d128++, tmp);
        _mm_store_si128(d128++, tmp);
    }
}

static int h264e_intra_choose_4x4_sse2(const pix_t *blockin, pix_t *blockpred, int avail, const pix_t *edge, int mpred, int penalty)
{
    int best_m = 0;
    int sad, best_sad = 0x10000;

    __m128i b0 = _mm_loadl_epi64((__m128i *)blockin);
    __m128i b1 = _mm_loadl_epi64((__m128i *)(blockin + 16));
    __m128i b2 = _mm_loadl_epi64((__m128i *)(blockin + 32));
    __m128i b3 = _mm_loadl_epi64((__m128i *)(blockin + 48));
    __m128i c  = _mm_unpacklo_epi32(b0, b1);
    __m128i d  = _mm_unpacklo_epi32(b2, b3);
    __m128i sse_blockin = _mm_unpacklo_epi64(c, d);
    __m128i t, t0, t1, t2, res, sad128, best128;

#define TEST(mode) sad128 = _mm_sad_epu8(res, sse_blockin);                 \
            sad128 = _mm_adds_epu16 (sad128, _mm_shuffle_epi32(sad128, 2)); \
            sad  = _mm_cvtsi128_si32(sad128);                               \
            if (mode != mpred) sad += penalty;                              \
            if (sad < best_sad)                                             \
            {                                                               \
                best128 = res;                                              \
                best_sad = sad;                                             \
                best_m = mode;                                              \
            }

    __m128i border = _mm_loadu_si128((__m128i *)(&L3));
    int topright = 0x01010101u*U7;

    if (!(avail & AVAIL_TR))
    {
        topright = 0x01010101u*U3;
        //border = _mm_insert_epi32 (border, topright, 2);
        border = _mm_insert_epi16 (border, topright, 4);
        border = _mm_insert_epi16 (border, topright, 5);
    }
    //border = _mm_insert_epi32 (border, topright, 3);
    border = _mm_insert_epi16 (border, topright, 6);
    border = _mm_insert_epi16 (border, topright, 7);

    // DC
    {
        unsigned dc = 0, round = 0;

        if (avail & AVAIL_L)
        {
            dc += _mm_cvtsi128_si32(_mm_sad_epu8(_mm_and_si128(border, _mm_set_epi32(0, 0, 0, ~0)), _mm_setzero_si128()));
            round += 2;
        }
        if (avail & AVAIL_T)
        {
            dc += _mm_cvtsi128_si32(_mm_sad_epu8(_mm_and_si128(_mm_srli_si128(border, 5), _mm_set_epi32(0, 0, 0, ~0)), _mm_setzero_si128()));
            round += 2;
        }
        dc += round;
        if (round == 4) dc >>= 1;
        dc >>= 2;
        if (!round) dc = 128;
        t = _mm_cvtsi32_si128(dc * 0x01010101);
        t = _mm_unpacklo_epi32(t, t);
        best128 =_mm_unpacklo_epi32(t, t);

        //TEST(2)
        sad128 = _mm_sad_epu8(best128, sse_blockin);
        sad128 = _mm_adds_epu16 (sad128, _mm_shuffle_epi32(sad128, 2));
        best_sad = _mm_cvtsi128_si32(sad128);

        if (2 != mpred) best_sad += penalty;
        best_m = 2;
    }

    if (avail & AVAIL_T)
    {
        t = _mm_srli_si128(border, 5);
        t = _mm_unpacklo_epi32(t, t);
        res =  _mm_unpacklo_epi32(t, t);
        TEST(0)

        t0 = _mm_srli_si128(border, 5);
        t1 = _mm_srli_si128(border, 6);
        t2 = _mm_srli_si128(border, 7);
        t = _mm_sub_epi8(_mm_avg_epu8(t0, t2), _mm_and_si128(_mm_xor_si128(t0, t2), _mm_set1_epi8(1)));
        t = _mm_avg_epu8(t, t1);
        t2 = _mm_unpacklo_epi32(t, _mm_srli_si128(t, 1));

        res = _mm_unpacklo_epi64(t2, _mm_unpacklo_epi32(_mm_srli_si128(t, 2), _mm_srli_si128(t, 3)));
        TEST(3)

        t0 = _mm_avg_epu8(t0,t1);
        t0  = _mm_unpacklo_epi32(t0, _mm_srli_si128(t0, 1));
        res = _mm_unpacklo_epi32(t0, t2);
        TEST(7)
    }

    if (avail & AVAIL_L)
    {
        int ext;
        t = _mm_unpacklo_epi8(border, border);
        t = _mm_shufflelo_epi16(t, 3 + (2 << 2) + (1 << 4) + (0 << 6));
        res = _mm_unpacklo_epi8(t, t);
        TEST(1)

        t0 = _mm_unpacklo_epi8(border, _mm_setzero_si128());
        t0 = _mm_shufflelo_epi16(t0, 3 + (2 << 2) + (1 << 4) + (0 << 6));
        t0 = _mm_packus_epi16(t0, t0);       // 0 1 2 3

        t1 = _mm_unpacklo_epi8(t0, t0);      // 0 0 1 1 2 2 3 3

        ext = _mm_extract_epi16(t1, 3);
        t0 = _mm_insert_epi16 (t0, ext, 2);  // 0 1 2 3 3 3
        t1 = _mm_insert_epi16 (t1, ext, 4);  // 0 0 1 1 2 2 3 3 33
        t2 = _mm_slli_si128(t0, 2);          // x x 0 1 2 3 3 3
        t = _mm_sub_epi8(_mm_avg_epu8(t0, t2), _mm_and_si128(_mm_xor_si128(t0, t2), _mm_set1_epi8(1)));
        // 0 1 2 3 3 3
        // x x 0 1 2 3
        t = _mm_unpacklo_epi8(t2, t);
        // 0   1   2   3   3   3
        // x   x   0   1   2   3
        // x   x   0   1   2   3
        t = _mm_avg_epu8(t, _mm_slli_si128(t1, 2));
        // 0 0 1 1 2 2 3 3

        res = _mm_unpacklo_epi32(_mm_srli_si128(t, 4), _mm_srli_si128(t, 6));
        //res = _mm_insert_epi32 (res, ext|(ext<<16),3);
        res = _mm_insert_epi16 (res, ext, 6);
        res = _mm_insert_epi16 (res, ext, 7);
        TEST(8)
    }

    if ((avail & (AVAIL_T | AVAIL_L | AVAIL_TL)) == (AVAIL_T | AVAIL_L | AVAIL_TL))
    {
        int t16;
        t0 = _mm_srli_si128(border, 1);
        t1 = _mm_srli_si128(border, 2);
        t = _mm_sub_epi8(_mm_avg_epu8(border, t1), _mm_and_si128(_mm_xor_si128(border, t1), _mm_set1_epi8(1)));
        t = _mm_avg_epu8(t, t0);

        res = _mm_unpacklo_epi64(_mm_unpacklo_epi32(_mm_srli_si128(t, 3), _mm_srli_si128(t, 2)), _mm_unpacklo_epi32(_mm_srli_si128(t, 1), t));
        TEST(4)

        t1 = _mm_unpacklo_epi8(t2 = _mm_avg_epu8(t0,border), t);
        t1 = _mm_unpacklo_epi32(t1, _mm_srli_si128(t1, 2));
        res = _mm_shuffle_epi32(t1, 3 | (2 << 2) | (1 << 4) | (0 << 6));
        res = _mm_insert_epi16 (res, _mm_extract_epi16 (t, 2), 1);
        TEST(6)

        t = _mm_srli_si128(t, 1);
        res = _mm_unpacklo_epi32(_mm_srli_si128(t2, 4), _mm_srli_si128(t, 2));
        t2 =  _mm_insert_epi16 (t2, t16 = _mm_extract_epi16 (t, 0), 1);
        t  =  _mm_insert_epi16 (t, (t16 << 8), 0);
        res = _mm_unpacklo_epi64(res, _mm_unpacklo_epi32(_mm_srli_si128(t2, 3), _mm_srli_si128(t, 1)));
        TEST(5)
    }

    ((uint32_t *)blockpred)[ 0] = _mm_extract_epi16(best128, 0) | ((unsigned)_mm_extract_epi16(best128, 1) << 16);
    ((uint32_t *)blockpred)[ 4] = _mm_extract_epi16(best128, 2) | ((unsigned)_mm_extract_epi16(best128, 3) << 16);
    ((uint32_t *)blockpred)[ 8] = _mm_extract_epi16(best128, 4) | ((unsigned)_mm_extract_epi16(best128, 5) << 16);
    ((uint32_t *)blockpred)[12] = _mm_extract_epi16(best128, 6) | ((unsigned)_mm_extract_epi16(best128, 7) << 16);

    return best_m + (best_sad << 4);    // pack result
}

#define MM_LOAD_8TO16(p) _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(p)), zero)
#define MM_LOAD_REG(p, sh) _mm_unpacklo_epi8(_mm_srli_si128(p, sh), zero)
#define __inline
static __inline void copy_wh_sse(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    assert(h % 4 == 0);
    if (w == 16)
    {
        do
        {
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
            _mm_store_si128((__m128i *)dst, _mm_loadu_si128((__m128i *)src)); src += src_stride; dst += 16;
        } while(h -= 8);
    } else //if (w == 8)
    {
        do
        {
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
        } while(h -= 4);
    }
}

static __inline void hpel_lpf_diag_sse(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    ALIGN(16) int16_t scratch[21 * 16] ALIGN2(16);  /* 21 rows by 16 pixels per row */

    /*
     * Intermediate values will be 1/2 pel at Horizontal direction
     * Starting at (0.5, -2) at top extending to (0.5, height + 3) at bottom
     * scratch contains a 2D array of size (w)X(h + 5)
     */
    __m128i zero = _mm_setzero_si128();
    __m128i c32,c5 = _mm_set1_epi16(5);
    int cloop = h + 5;
    int16_t *h264e_restrict dst16 = scratch;
    const int16_t *src16 = scratch + 2*16;
    src -= 2*src_stride;
    if (w == 8)
    {
        src16 = scratch + 2*8;
        do
        {
            __m128i inp = _mm_loadu_si128((__m128i*)(src - 2));
            _mm_store_si128((__m128i*)dst16, _mm_add_epi16(
                _mm_mullo_epi16(
                    _mm_sub_epi16(
                        _mm_slli_epi16(
                            _mm_add_epi16(MM_LOAD_REG(inp, 2), MM_LOAD_REG(inp, 3)),
                            2),
                        _mm_add_epi16(MM_LOAD_REG(inp, 1), MM_LOAD_REG(inp, 4))),
                    c5),
                _mm_add_epi16(_mm_unpacklo_epi8(inp, zero), MM_LOAD_REG(inp, 5))
            ));
            src += src_stride;
            dst16 += 8;
        } while (--cloop);

        c32 = _mm_set1_epi16(32);
        cloop = h;
        do
        {
            // (20*x2 - 5*x1 + x0 + 512) >> 10 =>
            // (16*x2 + 4*x2 - 4*x1 - x1 + x0 + 512) >> 10 =>
            // ((((x0 - x1) >> 2) + (x2 - x1)) >> 2) + x2 + 32 >> 6
            __m128i x1 = _mm_add_epi16(_mm_load_si128((__m128i*)(src16 - 1*8)), _mm_load_si128((__m128i*)(src16 + 2*8)));
            __m128i x2 = _mm_add_epi16(_mm_load_si128((__m128i*)(src16 - 0*8)), _mm_load_si128((__m128i*)(src16 + 1*8)));
            _mm_storel_epi64((__m128i*)dst,
                _mm_packus_epi16(
                    _mm_srai_epi16(
                        _mm_add_epi16(
                            _mm_srai_epi16(
                                _mm_sub_epi16(
                                    _mm_srai_epi16(
                                        _mm_sub_epi16(
                                            _mm_add_epi16(_mm_load_si128((__m128i*)(src16 - 2*8)), _mm_load_si128((__m128i*)(src16 + 3*8))),
                                            x1),
                                        2),
                                    _mm_sub_epi16(x1, x2)),
                                2),
                            _mm_add_epi16(x2, c32)),
                        6),
                    zero));
            src16 += 8;
            dst += 16;
        } while(--cloop);
    } else
    {
        do
        {
            _mm_store_si128((__m128i*)dst16, _mm_add_epi16(
                _mm_mullo_epi16(
                    _mm_sub_epi16(
                        _mm_slli_epi16(
                            _mm_add_epi16(MM_LOAD_8TO16(src - 0), MM_LOAD_8TO16(src + 1)),
                            2),
                        _mm_add_epi16(MM_LOAD_8TO16(src - 1), MM_LOAD_8TO16(src + 2))),
                    c5),
                _mm_add_epi16(MM_LOAD_8TO16(src - 2), MM_LOAD_8TO16(src + 3))
            ));
            _mm_store_si128((__m128i*)(dst16 + 8), _mm_add_epi16(
                _mm_mullo_epi16(
                    _mm_sub_epi16(
                        _mm_slli_epi16(
                            _mm_add_epi16(MM_LOAD_8TO16(src + 8 - 0), MM_LOAD_8TO16(src + 8 + 1)),
                            2),
                        _mm_add_epi16(MM_LOAD_8TO16(src + 8 - 1), MM_LOAD_8TO16(src + 8 + 2))),
                    c5),
                _mm_add_epi16(MM_LOAD_8TO16(src + 8 - 2), MM_LOAD_8TO16(src + 8 + 3))
            ));
            src += src_stride;
            dst16 += 8*2;
        } while (--cloop);

        c32 = _mm_set1_epi16(32);
        cloop = 2*h;
        do
        {
            // (20*x2 - 5*x1 + x0 + 512) >> 10 =>
            // (16*x2 + 4*x2 - 4*x1 - x1 + x0 + 512) >> 10 =>
            // ((((x0 - x1) >> 2) + (x2 - x1)) >> 2) + x2 + 32 >> 6
            __m128i x1 = _mm_add_epi16(_mm_load_si128((__m128i*)(src16 - 1*16)), _mm_load_si128((__m128i*)(src16 + 2*16)));
            __m128i x2 = _mm_add_epi16(_mm_load_si128((__m128i*)(src16 - 0*16)), _mm_load_si128((__m128i*)(src16 + 1*16)));
            _mm_storel_epi64((__m128i*)dst,
                _mm_packus_epi16(
                    _mm_srai_epi16(
                        _mm_add_epi16(
                            _mm_srai_epi16(
                                _mm_sub_epi16(
                                    _mm_srai_epi16(
                                        _mm_sub_epi16(
                                            _mm_add_epi16(_mm_load_si128((__m128i*)(src16 - 2*16)), _mm_load_si128((__m128i*)(src16 + 3*16))),
                                            x1),
                                        2),
                                    _mm_sub_epi16(x1, x2)),
                                2),
                            _mm_add_epi16(x2, c32)),
                        6),
                    zero));
            src16 += 8;
            dst += 8;
        } while(--cloop);
    }
}

static __inline void hpel_lpf_hor_sse(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    __m128i zero = _mm_setzero_si128();
    const __m128i five = _mm_set1_epi16(5);
    if (w == 8)
    {
        do
        {
            __m128i inp = _mm_loadu_si128((__m128i*)(src - 2));
            _mm_storel_epi64((__m128i*)dst, _mm_packus_epi16(
                _mm_srai_epi16(
                    _mm_add_epi16(
                        _mm_add_epi16(
                            _mm_mullo_epi16(
                                _mm_sub_epi16(
                                    _mm_slli_epi16(_mm_add_epi16(MM_LOAD_REG(inp, 2), MM_LOAD_REG(inp, 3)), 2),
                                    _mm_add_epi16(MM_LOAD_REG(inp, 1), MM_LOAD_REG(inp, 4))),
                                 five),
                            _mm_add_epi16(_mm_unpacklo_epi8(inp, zero), MM_LOAD_REG(inp, 5))),
                        _mm_set1_epi16(16)),
                    5),
                zero));
            src += src_stride;
            dst += 16;
        } while (--h);
    } else do
    {
        __m128i inp = _mm_loadu_si128((__m128i*)(src - 2));
        _mm_storel_epi64((__m128i*)dst, _mm_packus_epi16(
            _mm_srai_epi16(
                _mm_add_epi16(
                    _mm_add_epi16(
                        _mm_mullo_epi16(
                            _mm_sub_epi16(
                                _mm_slli_epi16(_mm_add_epi16(MM_LOAD_REG(inp, 2), MM_LOAD_REG(inp, 3)), 2),
                                _mm_add_epi16(MM_LOAD_REG(inp, 1), MM_LOAD_REG(inp, 4))),
                             five),
                        _mm_add_epi16(_mm_unpacklo_epi8(inp, zero), MM_LOAD_REG(inp, 5))),
                    _mm_set1_epi16(16)),
                5),
            zero));
        inp = _mm_loadu_si128((__m128i*)(src + 8 - 2));
        _mm_storel_epi64((__m128i*)(dst + 8), _mm_packus_epi16(
            _mm_srai_epi16(
                _mm_add_epi16(
                    _mm_add_epi16(
                        _mm_mullo_epi16(
                            _mm_sub_epi16(
                                _mm_slli_epi16(_mm_add_epi16(MM_LOAD_REG(inp, 2), MM_LOAD_REG(inp, 3)), 2),
                                _mm_add_epi16(MM_LOAD_REG(inp, 1), MM_LOAD_REG(inp, 4))),
                             five),
                        _mm_add_epi16(_mm_unpacklo_epi8(inp, zero), MM_LOAD_REG(inp, 5))),
                    _mm_set1_epi16(16)),
                5),
            zero));
        src += src_stride;
        dst += 16;
    } while (--h);
}

static __inline void hpel_lpf_ver_sse(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    __m128i zero = _mm_setzero_si128();
    __m128i five = _mm_set1_epi16(5);
    __m128i const16 = _mm_set1_epi16(16);

    do
    {
        int cloop = h;
        do
        {
            _mm_storel_epi64((__m128i*)dst, _mm_packus_epi16(
                _mm_srai_epi16(
                    _mm_add_epi16(
                        _mm_add_epi16(
                            _mm_mullo_epi16(
                                _mm_sub_epi16(
                                     _mm_slli_epi16(_mm_add_epi16(MM_LOAD_8TO16(src - 0*src_stride), MM_LOAD_8TO16(src + 1*src_stride)), 2),
                                    _mm_add_epi16(MM_LOAD_8TO16(src - 1*src_stride), MM_LOAD_8TO16(src + 2*src_stride))),
                                five),
                            _mm_add_epi16(MM_LOAD_8TO16(src - 2*src_stride), MM_LOAD_8TO16(src + 3*src_stride))),
                        const16),
                    5),
                zero));
            src += src_stride;
            dst += 16;
        } while(--cloop);
        src += 8 - src_stride*h;
        dst += 8 - 16*h;
    } while ((w -= 8) > 0);
}

static void average_16x16_unalign_sse(uint8_t *dst, const uint8_t *src, int src_stride)
{
    __m128i *d = (__m128i *)dst;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
    _mm_store_si128(d, _mm_avg_epu8(_mm_load_si128(d), _mm_loadu_si128((__m128i *)src))); src += src_stride; d++;
}

static void h264e_qpel_average_wh_align_sse2(const uint8_t *src0, const uint8_t *src1, uint8_t *h264e_restrict dst, point_t wh)
{
    int w = wh.s.x;
    int h = wh.s.y;
    __m128i *d = (__m128i *)dst;
    const __m128i *s0 = (const __m128i *)src0;
    const __m128i *s1 = (const __m128i *)src1;
    if (w == 16)
    {
        do
        {
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
            _mm_store_si128(d++, _mm_avg_epu8(_mm_load_si128(s0++), _mm_load_si128(s1++)));
        } while((h -= 8) > 0);
    } else
    {
        do
        {
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
            _mm_storel_epi64(d++, _mm_avg_epu8(_mm_loadl_epi64(s0++), _mm_loadl_epi64(s1++)));
        } while((h -= 8) > 0);
    }
}

static void h264e_qpel_interpolate_luma_sse2(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, point_t wh, point_t dxdy)
{
    ALIGN(16) uint8_t scratch[16*16] ALIGN2(16);
//    src += ((dx + 1) >> 2) + ((dy + 1) >> 2)*src_stride;            // dx == 3 ? next row; dy == 3 ? next line
//    dxdy              actions: Horizontal, Vertical, Diagonal, Average
//    0 1 2 3 +1        -   ha    h    ha+
//    1                 va  hva   hda  hv+a
//    2                 v   vda   d    v+da
//    3                 va+ h+va h+da  h+v+a
//    +stride
    int32_t pos = 1 << (dxdy.s.x + 4*dxdy.s.y);
    uint8_t *h264e_restrict dst0 = dst;

    if (pos == 1)
    {
        copy_wh_sse(src, src_stride, dst, wh.s.x, wh.s.y);
        return;
    }
    if (pos & 0xe0ee) // 1110 0000 1110 1110
    {
        hpel_lpf_hor_sse(src + ((dxdy.s.y + 1) >> 2)*src_stride, src_stride, dst, wh.s.x, wh.s.y);
        dst = scratch;
    }
    if (pos & 0xbbb0) // 1011 1011 1011 0000
    {
        hpel_lpf_ver_sse(src + ((dxdy.s.x + 1) >> 2), src_stride, dst, wh.s.x, wh.s.y);
        dst = scratch;
    }
    if (pos & 0x4e40) // 0100 1110 0100 0000
    {
        hpel_lpf_diag_sse(src, src_stride, dst, wh.s.x, wh.s.y);
        dst = scratch;
    }
    if (pos & 0xfafa) // 1111 1010 1111 1010
    {
        assert(wh.s.x == 16 && wh.s.y == 16);
        if (pos & 0xeae0)// 1110 1010 1110 0000
        {
            point_t p;
            p.u32 = 16 + (16 << 16);
            h264e_qpel_average_wh_align_sse2(scratch, dst0, dst0, p);
        } else
        {
            src += ((dxdy.s.x + 1) >> 2) + ((dxdy.s.y + 1) >> 2)*src_stride;
            average_16x16_unalign_sse(dst0, src, src_stride);
        }
    }
}

static void h264e_qpel_interpolate_chroma_sse2(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, point_t wh, point_t dxdy)
{
    __m128i zero = _mm_setzero_si128();
    int w = wh.s.x;
    int h = wh.s.y;
    __m128i a, b, c, d;

//        __m128i a = _mm_set1_epi16((short)((8-dx) * (8-dy)));
//        __m128i b = _mm_set1_epi16((short)(dx * (8-dy)));
//        __m128i c = _mm_set1_epi16((short)((8-dx) * dy));
//        __m128i d = _mm_set1_epi16((short)(dx * dy));
    __m128i c8 = _mm_set1_epi16(8);
    __m128i y,x = _mm_cvtsi32_si128(dxdy.u32);
    x = _mm_unpacklo_epi16(x, x);
    x = _mm_unpacklo_epi32(x, x);
    y = _mm_unpackhi_epi64(x, x);
    x = _mm_unpacklo_epi64(x, x);
    a = _mm_mullo_epi16(_mm_sub_epi16(c8, x), _mm_sub_epi16(c8, y));
    b = _mm_mullo_epi16(x, _mm_sub_epi16(c8, y));
    c = _mm_mullo_epi16(_mm_sub_epi16(c8, x), y);
    d = _mm_mullo_epi16(x, y);

    if (!dxdy.u32)
    {
        // 10%
        if (w == 8) do
        {
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
            _mm_storel_epi64((__m128i *)dst, _mm_loadl_epi64((__m128i *)src)); src += src_stride; dst += 16;
        } while(h -= 4);
        else
        {
            do
            {
                *(int *)dst = *(int_u *)src; src += src_stride; dst += 16;
                *(int *)dst = *(int_u *)src; src += src_stride; dst += 16;
                *(int *)dst = *(int_u *)src; src += src_stride; dst += 16;
                *(int *)dst = *(int_u *)src; src += src_stride; dst += 16;
            } while(h -= 4);
        }
    } else
    if (!dxdy.s.x || !dxdy.s.y)
    {
        // 40%
        int dsrc = dxdy.s.x?1:src_stride;
        c = _mm_or_si128(c,b);

        if (w==8)
        {
            do
            {
                _mm_storel_epi64((__m128i *)dst,
                _mm_packus_epi16(
                    _mm_srai_epi16(
                        _mm_add_epi16(
                            _mm_add_epi16(
                                    _mm_mullo_epi16(a, MM_LOAD_8TO16(src)),
                                    _mm_mullo_epi16(c, MM_LOAD_8TO16(src + dsrc))),
                            _mm_set1_epi16(32)),
                        6),
                    zero)) ;
                dst += 16;
                src += src_stride;
            } while (--h);
        } else
        {
            do
            {
                *(int* )(dst) = _mm_cvtsi128_si32 (
                _mm_packus_epi16(
                    _mm_srai_epi16(
                        _mm_add_epi16(
                            _mm_add_epi16(
                                    _mm_mullo_epi16(a, MM_LOAD_8TO16(src)),
                                    _mm_mullo_epi16(c, MM_LOAD_8TO16(src + dsrc))),
                            _mm_set1_epi16(32)),
                        6),
                    zero));
                dst += 16;
                src += src_stride;
            } while (--h);
        }
    } else
    {
        // 50%
        if (w == 8)
        {
            __m128i x1,x0;
            x0 = _mm_loadl_epi64((__m128i*)(src));
            x1 = _mm_loadl_epi64((__m128i*)(src + 1));
            x0 = _mm_unpacklo_epi8(x0, zero);
            x1 = _mm_unpacklo_epi8(x1, zero);
            do
            {
                __m128i y0, y1;
                src += src_stride;
                y0 = _mm_loadl_epi64((__m128i*)(src));
                y1 = _mm_loadl_epi64((__m128i*)(src + 1));
                y0 = _mm_unpacklo_epi8(y0, zero);
                y1 = _mm_unpacklo_epi8(y1, zero);
                _mm_storel_epi64((__m128i *)dst,
                    _mm_packus_epi16(
                        _mm_srai_epi16(
                            _mm_add_epi16(
                                _mm_add_epi16(
                                    _mm_add_epi16(
                                        _mm_mullo_epi16(x0, a),
                                        _mm_mullo_epi16(x1, b)),
                                    _mm_add_epi16(
                                        _mm_mullo_epi16(y0, c),
                                        _mm_mullo_epi16(y1, d))),
                                _mm_set1_epi16(32)),
                            6),
                        zero));
                x0 = y0;
                x1 = y1;
                dst += 16;
            } while (--h);
        } else
        {
            // TODO: load 32!
            __m128i x1, x0 = MM_LOAD_8TO16(src);
            do
            {
                src += src_stride;
                x1 = MM_LOAD_8TO16(src);
                *(int*)(dst) = _mm_cvtsi128_si32(
                    _mm_packus_epi16(
                        _mm_srai_epi16(
                            _mm_add_epi16(
                                _mm_add_epi16(
                                    _mm_add_epi16(
                                        _mm_mullo_epi16(x0, a),
                                        _mm_mullo_epi16(_mm_srli_si128(x0, 2), b)),
                                    _mm_add_epi16(
                                        _mm_mullo_epi16(x1, c),
                                        _mm_mullo_epi16(_mm_srli_si128(x1, 2), d))),
                                _mm_set1_epi16(32)),
                            6),
                        zero));
                x0 = x1;
                dst += 16;
            } while (--h);
        }
    }
}

static int h264e_sad_mb_unlaign_8x8_sse2(const pix_t *a, int a_stride, const pix_t *b, int sad[4])
{
    __m128i *mb = (__m128i *)b;
    __m128i s01, s23;
    s01 = _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++)); a += a_stride;
    s01 = _mm_add_epi64(s01, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s01 = _mm_add_epi64(s01, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s01 = _mm_add_epi64(s01, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s01 = _mm_add_epi64(s01, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s01 = _mm_add_epi64(s01, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s01 = _mm_add_epi64(s01, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s01 = _mm_add_epi64(s01, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;

    s23 = _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++)); a += a_stride;
    s23 = _mm_add_epi64(s23, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s23 = _mm_add_epi64(s23, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s23 = _mm_add_epi64(s23, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s23 = _mm_add_epi64(s23, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s23 = _mm_add_epi64(s23, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s23 = _mm_add_epi64(s23, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s23 = _mm_add_epi64(s23, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;

    sad[0] = _mm_cvtsi128_si32(s01);
    sad[1] = _mm_extract_epi16(s01, 4);
    sad[2] = _mm_cvtsi128_si32(s23);
    sad[3] = _mm_extract_epi16(s23, 4);
    return sad[0] + sad[1] + sad[2] + sad[3];
}


static int h264e_sad_mb_unlaign_wh_sse2(const pix_t *a, int a_stride, const pix_t *b, point_t wh)
{
    __m128i *mb = (__m128i *)b;
    __m128i s;

    assert(wh.s.x == 8 || wh.s.x == 16);
    assert(wh.s.y == 8 || wh.s.y == 16);

    if (wh.s.x == 8)
    {
        s =                  _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++));  a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;

        if (wh.s.y == 16)
        {
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
            s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadl_epi64((__m128i *)a), _mm_loadl_epi64(mb++))); a += a_stride;
        }
        return _mm_extract_epi16 (s, 0);
    }

    s =                  _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++));  a += a_stride;
    s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;

    if (wh.s.y == 16)
    {
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
        s = _mm_add_epi16(s, _mm_sad_epu8(_mm_loadu_si128((__m128i *)a), _mm_loadu_si128(mb++))); a += a_stride;
    }

    s = _mm_adds_epu16(s, _mm_shuffle_epi32(s, 2));
    return _mm_cvtsi128_si32(s);
}

static void h264e_copy_8x8_sse2(pix_t *d, int d_stride, const pix_t *s)
{
    assert(IS_ALIGNED(d, 8));
    assert(IS_ALIGNED(s, 8));
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s))); s += 16; d += d_stride;
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s))); s += 16; d += d_stride;
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s))); s += 16; d += d_stride;
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s))); s += 16; d += d_stride;
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s))); s += 16; d += d_stride;
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s))); s += 16; d += d_stride;
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s))); s += 16; d += d_stride;
    _mm_storel_epi64((__m128i*)(d), _mm_loadl_epi64((__m128i*)(s)));
}

static void h264e_copy_16x16_sse2(pix_t *d, int d_stride, const pix_t *s, int s_stride)
{
    assert(IS_ALIGNED(d, 8));
    assert(IS_ALIGNED(s, 8));
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s))); s += s_stride; d += d_stride;
    _mm_storeu_si128((__m128i*)(d), _mm_loadu_si128((__m128i*)(s)));
}

static void h264e_copy_borders_sse2(unsigned char *pic, int w, int h, int guard)
{
    int rowbytes = w + 2*guard;
    int topbot = 2;
    pix_t *s = pic;
    pix_t *d = pic - guard*rowbytes;
    assert(guard == 8 || guard == 16);
    assert((w % 8) == 0);
    do
    {
        int cloop = w;
        do
        {
            __m128i t = _mm_loadu_si128((__m128i*)(s));
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            if (guard == 16)
            {
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
                _mm_storeu_si128((__m128i*)d, t); d += rowbytes;
            }
            s += 16;
            d += 16 - guard*rowbytes;
        } while((cloop -= 16) > 0);
        s = pic + (h - 1)*rowbytes;
        d = s + rowbytes;
    } while(--topbot);

    {
        pix_t *s0 = pic - guard*rowbytes;
        pix_t *s1 = pic - guard*rowbytes + w - 1;
        int cloop = 2*guard + h;
        if (guard == 8) do
        {
            _mm_storel_epi64((__m128i*)(s0-8), _mm_set1_epi8(*s0));
            _mm_storel_epi64((__m128i*)(s1+1), _mm_set1_epi8(*s1));
            s0 += rowbytes;
            s1 += rowbytes;
        } while(--cloop); else do
        {
            _mm_storeu_si128((__m128i*)(s0-16), _mm_set1_epi8(*s0));
            _mm_storeu_si128((__m128i*)(s1+1), _mm_set1_epi8(*s1));
            s0 += rowbytes;
            s1 += rowbytes;
        } while(--cloop);

    }
}

static void hadamar4_2d_sse(int16_t *x)
{
    __m128i a = _mm_loadl_epi64((__m128i*)x);
    __m128i b = _mm_loadl_epi64((__m128i*)(x + 4));
    __m128i c = _mm_loadl_epi64((__m128i*)(x + 8));
    __m128i d = _mm_loadl_epi64((__m128i*)(x + 12));

    __m128i u0 = _mm_add_epi16(a, c);
    __m128i u1 = _mm_sub_epi16(a, c);
    __m128i u2 = _mm_add_epi16(b, d);
    __m128i u3 = _mm_sub_epi16(b, d);
    __m128i v0 = _mm_add_epi16(u0, u2);
    __m128i v3 = _mm_sub_epi16(u0, u2);
    __m128i v1 = _mm_add_epi16(u1, u3);
    __m128i v2 = _mm_sub_epi16(u1, u3);

    //    v0: a0 a1 a2 a3
    //    v1: b0 ......
    //    v2: c0 ......
    //    v4: d0 d1 .. d3
    //
    __m128i t0 = _mm_unpacklo_epi16(v0, v1);    // a0, b0, a1, b1, a2, b2, a3, b3
    __m128i t2 = _mm_unpacklo_epi16(v2, v3);    // c0, d0, c1, d1, c2, d2, c3, d3
    a = _mm_unpacklo_epi32(t0, t2);    // a0, b0, c0, d0, a1, b1, c1, d1
    c = _mm_unpackhi_epi32(t0, t2);    // a2, b2, c2, d2, a3, b3, c3, d3
    u0 = _mm_add_epi16(a, c); // u0 u2
    u1 = _mm_sub_epi16(a, c); // u1 u3
    v0 = _mm_unpacklo_epi64(u0, u1); // u0 u1
    v1 = _mm_unpackhi_epi64(u0, u1); // u2 u3
    u0 = _mm_add_epi16(v0, v1); // v0 v1
    u1 = _mm_sub_epi16(v0, v1); // v3 v2

    v1 = _mm_shuffle_epi32(u1, 0x4e); // u2 u3      01001110
    _mm_store_si128((__m128i*)x, u0);
    _mm_store_si128((__m128i*)(x + 8), v1);

}

static void dequant_dc_sse(quant_t *q, int16_t *qval, int dequant, int n)
{
    do q++->dq[0] = (int16_t)(*qval++*(int16_t)dequant); while (--n);
}

static void quant_dc_sse(int16_t *qval, int16_t *deq, int16_t quant, int n, int round_q18)
{
    int r_minus = (1 << 18) - round_q18;
    do
    {
        int v = *qval;
        int r = v < 0 ? r_minus : round_q18;
        *deq++ = *qval++ = (v * quant + r) >> 18;
    } while (--n);
}

static void hadamar2_2d_sse(int16_t *x)
{
    int a = x[0];
    int b = x[1];
    int c = x[2];
    int d = x[3];
    x[0] = (int16_t)(a + b + c + d);
    x[1] = (int16_t)(a - b + c - d);
    x[2] = (int16_t)(a + b - c - d);
    x[3] = (int16_t)(a - b - c + d);
}

static void h264e_quant_luma_dc_sse2(quant_t *q, int16_t *deq, const uint16_t *qdat)
{
    int16_t *tmp = ((int16_t*)q) - 16;
    hadamar4_2d_sse(tmp);
    quant_dc_sse(tmp, deq, qdat[0], 16, 0x20000);//0x15555);
    hadamar4_2d_sse(tmp);
    assert(!(qdat[1] & 3));
    // dirty trick here: shift w/o rounding, since it have no effect  for qp >= 10 (or, to be precise, for qp => 9)
    dequant_dc_sse(q, tmp, qdat[1] >> 2, 16);
}

static int h264e_quant_chroma_dc_sse2(quant_t *q, int16_t *deq, const uint16_t *qdat)
{
    int16_t *tmp = ((int16_t*)q) - 16;
    hadamar2_2d_sse(tmp);
    quant_dc_sse(tmp, deq, (int16_t)(qdat[0] << 1), 4, 0xAAAA);
    hadamar2_2d_sse(tmp);
    assert(!(qdat[1] & 1));
    dequant_dc_sse(q, tmp, qdat[1] >> 1, 4);
    return !!(tmp[0] | tmp[1] | tmp[2] | tmp[3]);
}

static int is_zero_sse(const int16_t *dat, int i0, const uint16_t *thr)
{
    __m128i t = _mm_loadu_si128((__m128i*)(thr));
    __m128i d = _mm_load_si128((__m128i*)(dat));
    __m128i z = _mm_setzero_si128();
    __m128i m, sign;
    if (i0) d = _mm_insert_epi16 (d, 0, 0);

    sign = _mm_cmpgt_epi16(z, d);
    d = _mm_sub_epi16(_mm_xor_si128(d, sign), sign);

    m = _mm_cmpgt_epi16(d, t);
    d = _mm_loadu_si128((__m128i*)(dat + 8));
    sign = _mm_cmpgt_epi16(z, d);
    d = _mm_sub_epi16(_mm_xor_si128(d, sign), sign);
    m = _mm_or_si128(m, _mm_cmpgt_epi16(d, t));
    return !_mm_movemask_epi8(m);
}

static int is_zero4_sse(const quant_t *q, int i0, const uint16_t *thr)
{
    return is_zero_sse(q[0].dq, i0, thr) &&
           is_zero_sse(q[1].dq, i0, thr) &&
           is_zero_sse(q[4].dq, i0, thr) &&
           is_zero_sse(q[5].dq, i0, thr);
}

static int h264e_transform_sub_quant_dequant_sse2(const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q, const uint16_t *qdat)
{
    int crow = mode >> 1;
    int ccol = crow;
    int i, i0 = mode & 1;
    int nz_block_mask = 0;
    int zmask = 0;
    quant_t *q_0 = q;

    int y, x;
    for (y = 0; y < crow; y++)
    {
        for (x = 0; x < ccol; x += 2)
        {
            const pix_t *pinp  = inp  + inp_stride*4*y + 4*x;
            const pix_t *ppred = pred +         16*4*y + 4*x;

            __m128i d0, d1, d2, d3;
            __m128i t0, t1, t2, t3;
            __m128i q0, q1, q2, q3;
            __m128i zero = _mm_setzero_si128();
            __m128i inp8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)pinp),  zero);
            __m128i pred8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)ppred), zero);

            d0 =_mm_sub_epi16(inp8, pred8);
            pinp += inp_stride;
            inp8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)pinp),  zero);
            pred8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(ppred + 16)), zero);
            d1 =_mm_sub_epi16(inp8, pred8);
            pinp += inp_stride;
            inp8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)pinp),  zero);
            pred8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(ppred + 32)), zero);
            d2 =_mm_sub_epi16(inp8, pred8);
            pinp += inp_stride;
            inp8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)pinp),  zero);
            pred8 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(ppred + 48)), zero);
            d3 =_mm_sub_epi16(inp8, pred8);
            t0 = _mm_add_epi16(d0, d3);
            t1 = _mm_sub_epi16(d0, d3);
            t2 = _mm_add_epi16(d1, d2);
            t3 = _mm_sub_epi16(d1, d2);
            q0 = _mm_add_epi16(t0, t2);
            q1 = _mm_add_epi16(_mm_add_epi16(t1, t1), t3);
            q2 = _mm_sub_epi16(t0, t2);
            q3 = _mm_sub_epi16(t1, _mm_add_epi16(t3, t3));

            //    q0: a0 a1 ....... a7
            //    q1: b0 .............
            //    q2: c0 .............
            //    q3: d0 d1 ....... d7
            //
            t0 = _mm_unpacklo_epi16(q0, q1);    // a0, b0, a1, b1, a2, b2, a3, b3
            t1 = _mm_unpackhi_epi16(q0, q1);    // a4, b4, a5, b5, a6, b6, a7, b7
            t2 = _mm_unpacklo_epi16(q2, q3);    // c0, d0
            t3 = _mm_unpackhi_epi16(q2, q3);    // c4, d4

            q0 = _mm_unpacklo_epi32(t0, t2);    // a0, b0, c0, d0, a1, b1, c1, d1
            q1 = _mm_unpackhi_epi32(t0, t2);    // a2, b2,
            q2 = _mm_unpacklo_epi32(t1, t3);    // a4, b4
            q3 = _mm_unpackhi_epi32(t1, t3);    // a6, b6

            d0 = _mm_unpacklo_epi64(q0, q2);    // a0, b0, c0, d0, a4, b4, c4, d4
            d1 = _mm_unpackhi_epi64(q0, q2);    // a1, b1, c1, d1
            d2 = _mm_unpacklo_epi64(q1, q3);    // a2, b2,
            d3 = _mm_unpackhi_epi64(q1, q3);    // a3, b3,

            t0 = _mm_add_epi16(d0, d3);
            t1 = _mm_sub_epi16(d0, d3);
            t2 = _mm_add_epi16(d1, d2);
            t3 = _mm_sub_epi16(d1, d2);
            q0 = _mm_add_epi16(t0, t2);
            q1 = _mm_add_epi16(_mm_add_epi16(t1, t1), t3);
            q2 = _mm_sub_epi16(t0, t2);
            q3 = _mm_sub_epi16(t1, _mm_add_epi16(t3, t3));

            _mm_storel_epi64((__m128i*)(q[0].dq    ), q0);
            _mm_storel_epi64((__m128i*)(q[0].dq + 4), q1);
            _mm_storel_epi64((__m128i*)(q[0].dq + 8), q2);
            _mm_storel_epi64((__m128i*)(q[0].dq + 12), q3);
            if (ccol > 1)
            {
                q0 = _mm_unpackhi_epi64(q0, q0); _mm_storel_epi64((__m128i*)(q[1].dq    ), q0);
                q1 = _mm_unpackhi_epi64(q1, q1); _mm_storel_epi64((__m128i*)(q[1].dq + 4), q1);
                q2 = _mm_unpackhi_epi64(q2, q2); _mm_storel_epi64((__m128i*)(q[1].dq + 8), q2);
                q3 = _mm_unpackhi_epi64(q3, q3); _mm_storel_epi64((__m128i*)(q[1].dq + 12), q3);
            }
            q += 2;
        }
    }
    q = q_0;
    crow = mode >> 1;
    ccol = crow;

    if (mode & 1) // QDQ_MODE_INTRA_16 || QDQ_MODE_CHROMA
    {
        int cloop = (mode >> 1)*(mode >> 1);
        short *dc = ((short *)q) - 16;
        quant_t *pq = q;
        do
        {
            *dc++ = pq->dq[0];
            pq++;
        } while (--cloop);
    }

    if (mode == QDQ_MODE_INTER || mode == QDQ_MODE_CHROMA)
    {
        for (i = 0; i < crow*ccol; i++)
        {
            if (is_zero_sse(q[i].dq, i0, qdat + OFFS_THR_1_OFF))
            {
                zmask |= (1 << i);
            }
        }

        if (mode == QDQ_MODE_INTER)
        {
            if ((~zmask & 0x0033) && is_zero4_sse(q +  0, i0, qdat + OFFS_THR_2_OFF)) zmask |= 0x33;
            if ((~zmask & 0x00CC) && is_zero4_sse(q +  2, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 2);
            if ((~zmask & 0x3300) && is_zero4_sse(q +  8, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 8);
            if ((~zmask & 0xCC00) && is_zero4_sse(q + 10, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 10);
        }
    }

    do
    {
        do
        {
            int nz_mask = 0;
            if (zmask & 1)
            {
                _mm_store_si128((__m128i*)(q->qv),     _mm_setzero_si128());
                _mm_store_si128((__m128i*)(q->qv) + 1, _mm_setzero_si128());
            } else
            {
                int16_t *qv_tmp = q->qv;//[16];
                __m128i t;
                const __m128i const_q  = _mm_loadu_si128((__m128i*)(qdat + OFFS_QUANT_VECT));
                const __m128i const_dq = _mm_loadu_si128((__m128i*)(qdat + OFFS_DEQUANT_VECT));

                __m128i src = _mm_load_si128((__m128i*)(q[0].dq));
                __m128i r = _mm_xor_si128(_mm_set1_epi16(qdat[OFFS_RND_INTER]), _mm_cmpgt_epi16(_mm_setzero_si128(), src));
                __m128i lo = _mm_mullo_epi16(src, const_q);
                __m128i hi = _mm_mulhi_epi16(src, const_q);
                __m128i dst0 = _mm_unpacklo_epi16(lo, hi);
                __m128i dst1 = _mm_unpackhi_epi16(lo, hi);
                dst0 = _mm_srai_epi32(_mm_add_epi32(dst0, _mm_unpacklo_epi16(r, _mm_setzero_si128())), 16);
                dst1 = _mm_srai_epi32(_mm_add_epi32(dst1, _mm_unpackhi_epi16(r, _mm_setzero_si128())), 16);
                dst0 = _mm_packs_epi32(dst0, dst1);
                _mm_store_si128((__m128i*)(qv_tmp), dst0);

                t = _mm_cmpeq_epi16(_mm_setzero_si128(), dst0);
                nz_mask = _mm_movemask_epi8( _mm_packs_epi16(t, t)) & 0xff;
                dst0 = _mm_mullo_epi16(dst0, const_dq);
                _mm_store_si128((__m128i*)(q[0].dq), dst0);


                src = _mm_load_si128((__m128i*)(q[0].dq + 8));
                r = _mm_xor_si128(_mm_set1_epi16(qdat[OFFS_RND_INTER]), _mm_cmpgt_epi16(_mm_setzero_si128(), src));
                lo = _mm_mullo_epi16(src, const_q);
                hi = _mm_mulhi_epi16(src, const_q);
                dst0 = _mm_unpacklo_epi16(lo, hi);
                dst1 = _mm_unpackhi_epi16(lo, hi);

                dst0 = _mm_srai_epi32(_mm_add_epi32(dst0, _mm_unpacklo_epi16(r, _mm_setzero_si128())), 16);
                dst1 = _mm_srai_epi32(_mm_add_epi32(dst1, _mm_unpackhi_epi16(r, _mm_setzero_si128())), 16);
                dst0 = _mm_packs_epi32(dst0, dst1);
                _mm_store_si128((__m128i*)(qv_tmp + 8), dst0);

                t = _mm_cmpeq_epi16(_mm_setzero_si128(), dst0);
                nz_mask |= _mm_movemask_epi8( _mm_packs_epi16(t, t)) << 8;
                dst0 = _mm_mullo_epi16(dst0, const_dq);
                _mm_store_si128((__m128i*)(q[0].dq + 8), dst0);
                nz_mask = ~nz_mask & 0xffff;
                if (i0)
                {
                    nz_mask &= ~1;
                }
            }

            zmask >>= 1;
            nz_block_mask <<= 1;
            if (nz_mask)
                nz_block_mask |= 1;
            q++;
        } while (--ccol);
        ccol = mode >> 1;
    } while (--crow);
    return nz_block_mask;
}

static void h264e_transform_add_sse2(pix_t *out, int out_stride, const pix_t *pred, quant_t *q, int side, int32_t mask)
{
    int crow = side;
    int ccol = crow;

    assert(IS_ALIGNED(out, 4));
    assert(IS_ALIGNED(pred, 4));
    assert(!(out_stride % 4));

    do
    {
        do
        {
            if (mask >= 0)
            {
                // copy 4x4
                pix_t *dst = out;
                *(uint32_t*)dst = *(uint32_t*)(pred + 0 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 1 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 2 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 3 * 16);
            }
            else
            {
                __m128i zero = _mm_setzero_si128();
                __m128i c32 = _mm_set1_epi16(32);
                __m128i d0, d1, d2, d3;
                __m128i e0, e1, e2, e3;
                d0 = _mm_load_si128((__m128i*)(q->dq + 0));
                d2 = _mm_load_si128((__m128i*)(q->dq + 8));
                d1 = _mm_unpackhi_epi64(d0, d2);
                d3 = _mm_unpackhi_epi64(d2, d0);

                e0 = _mm_add_epi16(d0, d2);
                e1 = _mm_sub_epi16(d0, d2);

                e2 = _mm_srai_epi16(d1, 1);
                e2 = _mm_sub_epi16(e2, d3);
                e3 = _mm_srai_epi16(d3, 1);
                e3 = _mm_add_epi16(e3, d1);

                d0 = _mm_add_epi16(e0, e3);
                d1 = _mm_add_epi16(e1, e2);
                d2 = _mm_sub_epi16(e1, e2);
                d3 = _mm_sub_epi16(e0, e3);

                e1 = _mm_unpacklo_epi16(d0, d1);    // a0, b0, a1, b1, a2, b2, a3, b3
                e3 = _mm_unpacklo_epi16(d2, d3);    // c0, d0

                e0 = _mm_unpacklo_epi32(e1, e3);    // a0, b0, c0, d0, a1, b1, c1, d1
                e2 = _mm_unpackhi_epi32(e1, e3);    // a2, b2,

                e1 = _mm_unpackhi_epi64(e0, e2);
                e3 = _mm_unpackhi_epi64(e2, e0);

                d0 = _mm_add_epi16(e0, e2);
                d1 = _mm_sub_epi16(e0, e2);
                d2 = _mm_srai_epi16(e1, 1);
                d2 = _mm_sub_epi16(d2, e3);
                d3 = _mm_srai_epi16(e3, 1);
                d3 = _mm_add_epi16(d3, e1);

                // Pack 4x64 to 2x128
                e0 = _mm_unpacklo_epi64(d0, d1);
                e1 = _mm_unpacklo_epi64(d3, d2);

                e0 = _mm_add_epi16(e0, c32);
                d0 = _mm_srai_epi16(_mm_add_epi16(e0, e1), 6);
                d3 = _mm_srai_epi16(_mm_sub_epi16(e0, e1), 6);
                // Unpack back to 4x64
                d1 = _mm_unpackhi_epi64(d0, zero);
                d2 = _mm_unpackhi_epi64(d3, zero);

                *(int* )(out)                = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_add_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(pred +  0)), zero), d0), zero));
                *(int* )(out + 1*out_stride) = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_add_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(pred + 16)), zero), d1), zero));
                *(int* )(out + 2*out_stride) = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_add_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(pred + 32)), zero), d2), zero));
                *(int* )(out + 3*out_stride) = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_add_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int*)(pred + 48)), zero), d3), zero));

            }
            mask = (uint32_t)mask << 1;
            q++;
            out += 4;
            pred += 4;
        } while (--ccol);
        ccol = side;
        out += 4*(out_stride - ccol);
        pred += 4*(16 - ccol);
    } while (--crow);
}
#endif

#if H264E_ENABLE_NEON && !defined(MINIH264_ASM)
#define TR32(x, y) tr0 = vtrnq_u32(vreinterpretq_u32_u8(x), vreinterpretq_u32_u8(y)); x = vreinterpretq_u8_u32(tr0.val[0]); y = vreinterpretq_u8_u32(tr0.val[1]);
#define TR16(x, y) tr1 = vtrnq_u16(vreinterpretq_u16_u8(x), vreinterpretq_u16_u8(y)); x = vreinterpretq_u8_u16(tr1.val[0]); y = vreinterpretq_u8_u16(tr1.val[1]);
#define TR8(x, y)  tr2 = vtrnq_u8((x), (y)); x = (tr2.val[0]); y = (tr2.val[1]);

static void deblock_luma_v_neon(uint8_t *pix, int stride, int alpha, int beta, const uint8_t *pthr, const uint8_t *pstr)
{
    uint8x16_t q0, q1, q2, q3, q4, q5, q6, q7, q8, q9, q10, q11, q12, q13, q14, q15;
    uint8x16_t tmp;
    uint32x4x2_t tr0;
    uint16x8x2_t tr1;
    uint8x16x2_t tr2;
    q8 = vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q9 = vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q10= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q11= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q12= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q13= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q14= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q15= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;

    TR32(q8,  q12);
    TR32(q9,  q13);
    TR32(q10, q14);
    TR32(q11, q15);
    TR16(q8,  q10);
    TR16(q9,  q11);
    TR16(q12, q14);
    TR16(q13, q15);
    TR8(q8,   q9 );
    TR8(q10,  q11);
    TR8(q12,  q13);
    TR8(q14,  q15);

    q1  = vabdq_u8(q11, q12);
    q2  = vcltq_u8(q1, vdupq_n_u8(alpha));
    q1  = vcltq_u8(vmaxq_u8(vabdq_u8(q11, q10), vabdq_u8(q12, q13)), vdupq_n_u8(beta));
    q2  = vandq_u8(q2, q1);

    tmp = vreinterpretq_u8_u32(vdupq_n_u32(*(uint32_t*)pstr));
    tmp = vzipq_u8(tmp, tmp).val[0];
    tmp = vzipq_u8(tmp, tmp).val[0];
    q1  = tmp;

    q1  = vcgtq_s8(vreinterpretq_s8_u8(q1), vdupq_n_s8(0));
    q2  = vandq_u8(q2, q1);
    q7 = vhsubq_u8(q10, q13);
    q7 = vreinterpretq_u8_s8(vshrq_n_s8(vreinterpretq_s8_u8(q7), 1));
    q0 = veorq_u8(q12, q11);
    q6 = vandq_u8(vdupq_n_u8(1), q0);

    q0 = vhsubq_u8(q12, q11);// ;(q0-p0))>>1

    q7 = vreinterpretq_u8_s8(vrhaddq_s8(vreinterpretq_s8_u8(q7), vreinterpretq_s8_u8(q6))); //((p1-q1)>>2 + carry + 1) >> 1
    q7 = vreinterpretq_u8_s8(vqaddq_s8(vreinterpretq_s8_u8(q0),  vreinterpretq_s8_u8(q7))); //=delta = (((q0-p0)<<2) + (p1-q1) + 4) >> 3;
    q7 = vandq_u8(q7, q2);

    tmp = vreinterpretq_u8_u32(vdupq_n_u32(*(uint32_t*)pthr));
    tmp = vzipq_u8(tmp, tmp).val[0];
    tmp = vzipq_u8(tmp, tmp).val[0];
    q1  = tmp;

    q1  = vandq_u8(q2, q1);

    q0 = vabdq_u8(q9,  q11); // ap = ABS(p2 - p0);
    q0 = vcltq_u8(q0,  vdupq_n_u8(beta)); //sp = (ap - beta) >> 31;
    q4 = vandq_u8(q0,  q2);  // & sp
    q0 = vabdq_u8(q14, q12); //aq = ABS(q2 - q0);
    q0 = vcltq_u8(q0,  vdupq_n_u8(beta))  ;//sq = (aq - beta) >> 31;
    q3 = vandq_u8(q0,  q2);  //  & sq

    q0  = vrhaddq_u8(q11, q12);//((p0+q0+1)>>1)
    q0  = vhaddq_u8 (q0,  q9 );//((p2 + ((p0+q0+1)>>1))>>1)
    q5  = vandq_u8  (q1,  q4 );
    q6  = vqaddq_u8 (q10, q5 );//{p1+thr}
    q0  = vminq_u8  (q0,  q6 );
    q6  = vqsubq_u8 (q10, q5 );//{p1-thr}
    q10 = vmaxq_u8  (q0,  q6 );

    q0  = vrhaddq_u8(q11, q12);// ;((p0+q0+1)>>1)
    q0  = vhaddq_u8 (q0,  q14);// ;((q2 + ((p0+q0+1)>>1))>>1)
    q5  = vandq_u8  (q1,  q3 );
    q6  = vqaddq_u8 (q13, q5 );// ;{q1+thr}
    q0  = vminq_u8  (q0,  q6 );
    q6  = vqsubq_u8 (q13, q5 );// ;{q1-thr}
    q13 = vmaxq_u8  (q0,  q6 );

    q1  = vreinterpretq_u8_s8(vsubq_s8(vreinterpretq_s8_u8(q1), vreinterpretq_s8_u8(q3)));
    q1  = vreinterpretq_u8_s8(vsubq_s8(vreinterpretq_s8_u8(q1), vreinterpretq_s8_u8(q4))); //tC = thr - sp - sq;
    q1  = vandq_u8(q1, q2);// ; set thr = 0 if str==0

    q6  = veorq_u8(q6, q6);
    q5  = vreinterpretq_u8_s8(vmaxq_s8(vreinterpretq_s8_u8(q6), vreinterpretq_s8_u8(q7))); //delta > 0
    q7  = vreinterpretq_u8_s8(vsubq_s8(vreinterpretq_s8_u8(q6), vreinterpretq_s8_u8(q7)));
    q6  = vreinterpretq_u8_s8(vmaxq_s8(vreinterpretq_s8_u8(q6), vreinterpretq_s8_u8(q7))); //-(delta < 0)
    q5  =  vminq_u8(q1, q5);
    q6  =  vminq_u8(q1, q6);

    q11 = vqaddq_u8(q11, q5);
    q11 = vqsubq_u8(q11, q6);
    q12 = vqsubq_u8(q12, q5);
    q12 = vqaddq_u8(q12, q6);

    TR8(q8,   q9 );
    TR8(q10,  q11);
    TR8(q12,  q13);
    TR8(q14,  q15);
    TR16(q8,  q10);
    TR16(q9,  q11);
    TR16(q12, q14);
    TR16(q13, q15);
    TR32(q8,  q12);
    TR32(q9,  q13);
    TR32(q10, q14);
    TR32(q11, q15);

    pix -= 8*stride + 4;
    vst1_u8(pix, vget_low_u8(q8));  pix += stride;
    vst1_u8(pix, vget_low_u8(q9));  pix += stride;
    vst1_u8(pix, vget_low_u8(q10)); pix += stride;
    vst1_u8(pix, vget_low_u8(q11)); pix += stride;
    vst1_u8(pix, vget_low_u8(q12)); pix += stride;
    vst1_u8(pix, vget_low_u8(q13)); pix += stride;
    vst1_u8(pix, vget_low_u8(q14)); pix += stride;
    vst1_u8(pix, vget_low_u8(q15)); pix += stride;

    vst1_u8(pix, vget_high_u8(q8)); pix += stride;
    vst1_u8(pix, vget_high_u8(q9)); pix += stride;
    vst1_u8(pix, vget_high_u8(q10)); pix += stride;
    vst1_u8(pix, vget_high_u8(q11)); pix += stride;
    vst1_u8(pix, vget_high_u8(q12)); pix += stride;
    vst1_u8(pix, vget_high_u8(q13)); pix += stride;
    vst1_u8(pix, vget_high_u8(q14)); pix += stride;
    vst1_u8(pix, vget_high_u8(q15)); pix += stride;
}

static void deblock_luma_h_s4_neon(uint8_t *pix, int stride, int alpha, int beta)
{
    uint8x16_t q0, q1, q2, q3, q4, q5, q6, q7, q8, q9, q10, q11, q12, q13, q14, q15, vspill0, vspill1;
    q8  = vld1q_u8(pix - 4*stride);
    q9  = vld1q_u8(pix - 3*stride);
    q10 = vld1q_u8(pix - 2*stride);
    q11 = vld1q_u8(pix - 1*stride);
    q12 = vld1q_u8(pix);
    q13 = vld1q_u8(pix + 1*stride);
    q14 = vld1q_u8(pix + 2*stride);
    q15 = vld1q_u8(pix + 3*stride);
    q0  = vabdq_u8(q11, q12);
    q2  = vcltq_u8(q0, vdupq_n_u8(alpha));
    q2  = vandq_u8(q2, vcltq_u8(vabdq_u8(q11, q10), vdupq_n_u8(beta)));
    q2  = vandq_u8(q2, vcltq_u8(vabdq_u8(q12, q13), vdupq_n_u8(beta)));
    q1  = vandq_u8(q2, vcltq_u8(q0, vdupq_n_u8(((alpha >> 2) + 2))));
    q0  = vandq_u8(q1, vcltq_u8(vabdq_u8(q9,  q11), vdupq_n_u8(beta)));
    q3  = vandq_u8(q1, vcltq_u8(vabdq_u8(q14, q12), vdupq_n_u8(beta)));
    q4 = vhaddq_u8(q9,  q10);
    q5 = vhaddq_u8(q11, q12);
    q6 = vsubq_u8(vrhaddq_u8(q9,  q10), q4);
    q7 = vsubq_u8(vrhaddq_u8(q11, q12), q5);
    q6 = vhaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q4, q8);
    q4 = vhaddq_u8(q4, q8);
    q7 = vsubq_u8(q7, q4);
    q6 = vaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q5, q9);
    q5 = vhaddq_u8(q5, q9);
    q7 = vsubq_u8(q7, q5);
    q6 = vhaddq_u8(q6, q7);

    q7 = vrhaddq_u8(q4, q5);
    q4 = vhaddq_u8(q4, q5);
    q7 = vsubq_u8(q7, q4);
    q6 = vrhaddq_u8(q6, q7);
    q4 = vaddq_u8(q4, q6);
    vspill0 =  vbslq_u8(q0, q4, q9);   // VMOV        q6,     q9   VBIT        q6,     q4,     q0

    q4 = vhaddq_u8(q14, q13);
    q5 = vhaddq_u8(q12, q11);
    q6 = vsubq_u8(vrhaddq_u8(q14, q13), q4);
    q7 = vsubq_u8(vrhaddq_u8(q12, q11), q5);
    q6 = vhaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q4, q15);
    q4 = vhaddq_u8(q4, q15);
    q7 = vsubq_u8(q7, q4);
    q6 = vaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q5, q14);
    q5 = vhaddq_u8(q5, q14);
    q7 = vsubq_u8(q7, q5);
    q6 = vhaddq_u8(q6, q7);

    q7 = vrhaddq_u8(q4, q5);
    q4 = vhaddq_u8(q4, q5);
    q7 = vsubq_u8(q7, q4);
    q6 = vrhaddq_u8(q6, q7);
    q4 = vaddq_u8(q4, q6);
    vspill1 =  vbslq_u8(q3, q4, q14);   //     VMOV        q6,     q14    VBIT        q6,     q4,     q3

    q1 = vhaddq_u8 (q9,  q13);
    q4 = vrhaddq_u8(q1,  q10);
    q5 = vrhaddq_u8(q11, q12);
    q6 = vhaddq_u8 (q1,  q10);
    q7 = vhaddq_u8 (q11, q12);
    q4 = vhaddq_u8 (q4,  q5);
    q6 = vrhaddq_u8(q6,  q7);
    q1 = vrhaddq_u8(q4,  q6);
    q4 = vrhaddq_u8(q9,  q10);
    q5 = vrhaddq_u8(q11, q12);
    q6 = vhaddq_u8 (q9,  q10);
    q7 = vhaddq_u8 (q11, q12);
    q4 = vhaddq_u8 (q4,  q5);
    q6 = vrhaddq_u8(q6,  q7);
    q4 = vrhaddq_u8(q4,  q6);
    q5 = vhaddq_u8 (q11, q13);
    q5 = vrhaddq_u8(q5,  q10);

    q1 = vbslq_u8(q0, q1, q5); //VBIF        q1,     q5,     q0
    q0 = vbslq_u8(q0, q4, q10);//VBSL        q0,     q4,     q10

    q7 = vhaddq_u8 (q14, q10);
    q4 = vrhaddq_u8(q7,  q13);
    q5 = vrhaddq_u8(q11, q12);
    q6 = vhaddq_u8 (q7,  q13);
    q7 = vhaddq_u8 (q11, q12);
    q4 = vhaddq_u8 (q4,  q5 );
    q6 = vrhaddq_u8(q6,  q7 );
    q4 = vrhaddq_u8(q4,  q6 );
    q6 = vrhaddq_u8(q14, q13);
    q5 = vrhaddq_u8(q11, q12);
    q5 = vhaddq_u8 (q6,  q5 );
    q6 = vhaddq_u8 (q14, q13);
    q7 = vhaddq_u8 (q11, q12);
    q6 = vrhaddq_u8(q6,  q7 );
    q5 = vrhaddq_u8(q5,  q6 );
    q6 = vhaddq_u8 (q12, q10);
    q6 = vrhaddq_u8(q6,  q13);

    q4 = vbslq_u8(q3, q4, q6); //    VBIF        q4,     q6,     q3    ;q0
    q3 = vbslq_u8(q3, q5, q13);//    VBSL        q3,     q5,     q13   ;q1

    q10 = vbslq_u8(q2, q0, q10);
    q11 = vbslq_u8(q2, q1, q11);
    q12 = vbslq_u8(q2, q4, q12);
    q13 = vbslq_u8(q2, q3, q13);

    vst1q_u8(pix - 3*stride, vspill0);
    vst1q_u8(pix - 2*stride, q10);
    vst1q_u8(pix - 1*stride, q11);
    vst1q_u8(pix           , q12);
    vst1q_u8(pix + 1*stride, q13);
    vst1q_u8(pix + 2*stride, vspill1);

}

static void deblock_luma_v_s4_neon(uint8_t *pix, int stride, int alpha, int beta)
{
    uint32x4x2_t tr0;
    uint16x8x2_t tr1;
    uint8x16x2_t tr2;
    uint8x16_t q0, q1, q2, q3, q4, q5, q6, q7, q8, q9, q10, q11, q12, q13, q14, q15, vspill0, vspill1;
    q8 = vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q9 = vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q10= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q11= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q12= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q13= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q14= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;
    q15= vcombine_u8(vld1_u8(pix - 4), vld1_u8(pix - 4 + 8*stride)); pix += stride;

    TR32(q8,  q12);
    TR32(q9,  q13);
    TR32(q10, q14);
    TR32(q11, q15);
    TR16(q8,  q10);
    TR16(q9,  q11);
    TR16(q12, q14);
    TR16(q13, q15);
    TR8(q8,   q9 );
    TR8(q10,  q11);
    TR8(q12,  q13);
    TR8(q14,  q15);

    q0 = vabdq_u8(q11, q12);
    q2 = vcltq_u8(q0, vdupq_n_u8(alpha));
    q2 = vandq_u8(q2, vcltq_u8(vabdq_u8(q11,    q10), vdupq_n_u8(beta)));
    q2 = vandq_u8(q2, vcltq_u8(vabdq_u8(q12,    q13), vdupq_n_u8(beta)));
    q1 = vandq_u8(q2, vcltq_u8(q0, vdupq_n_u8(((alpha >> 2) + 2))));
    q0 = vandq_u8(q1, vcltq_u8(vabdq_u8(q9,     q11), vdupq_n_u8(beta)));
    q3 = vandq_u8(q1, vcltq_u8(vabdq_u8(q14,    q12), vdupq_n_u8(beta)));
    q4 = vhaddq_u8(q9,  q10);
    q5 = vhaddq_u8(q11, q12);
    q6 = vsubq_u8(vrhaddq_u8(q9,  q10), q4);
    q7 = vsubq_u8(vrhaddq_u8(q11, q12), q5);
    q6 = vhaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q4, q8);
    q4 = vhaddq_u8(q4, q8);
    q7 = vsubq_u8(q7, q4);
    q6 = vaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q5, q9);
    q5 = vhaddq_u8(q5, q9);
    q7 = vsubq_u8(q7, q5);
    q6 = vhaddq_u8(q6, q7);

    q7 = vrhaddq_u8(q4, q5);
    q4 = vhaddq_u8(q4, q5);
    q7 = vsubq_u8(q7, q4);
    q6 = vrhaddq_u8(q6, q7);
    q4 = vaddq_u8(q4, q6);
    vspill0 =  vbslq_u8(q0, q4, q9);   // VMOV        q6,     q9   VBIT        q6,     q4,     q0

    q4 = vhaddq_u8(q14, q13);
    q5 = vhaddq_u8(q12, q11);
    q6 = vsubq_u8(vrhaddq_u8(q14, q13), q4);
    q7 = vsubq_u8(vrhaddq_u8(q12, q11), q5);
    q6 = vhaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q4, q15);
    q4 = vhaddq_u8(q4, q15);
    q7 = vsubq_u8(q7, q4);
    q6 = vaddq_u8(q6, q7);
    q7 = vrhaddq_u8(q5, q14);
    q5 = vhaddq_u8(q5, q14);
    q7 = vsubq_u8(q7, q5);
    q6 = vhaddq_u8(q6, q7);

    q7 = vrhaddq_u8(q4, q5);
    q4 = vhaddq_u8(q4, q5);
    q7 = vsubq_u8(q7, q4);
    q6 = vrhaddq_u8(q6, q7);
    q4 = vaddq_u8(q4, q6);
    vspill1 =  vbslq_u8(q3, q4, q14);   //     VMOV        q6,     q14    VBIT        q6,     q4,     q3

    q1 = vhaddq_u8 (q9,  q13);
    q4 = vrhaddq_u8(q1,  q10);
    q5 = vrhaddq_u8(q11, q12);
    q6 = vhaddq_u8 (q1,  q10);
    q7 = vhaddq_u8 (q11, q12);
    q4 = vhaddq_u8 (q4,  q5);
    q6 = vrhaddq_u8(q6,  q7);
    q1 = vrhaddq_u8(q4,  q6);
    q4 = vrhaddq_u8(q9,  q10);
    q5 = vrhaddq_u8(q11, q12);
    q6 = vhaddq_u8 (q9,  q10);
    q7 = vhaddq_u8 (q11, q12);
    q4 = vhaddq_u8 (q4,  q5);
    q6 = vrhaddq_u8(q6,  q7);
    q4 = vrhaddq_u8(q4,  q6);
    q5 = vhaddq_u8 (q11, q13);
    q5 = vrhaddq_u8(q5,  q10);

    q1 = vbslq_u8(q0, q1, q5); //VBIF        q1,     q5,     q0
    q0 = vbslq_u8(q0, q4, q10);//VBSL        q0,     q4,     q10

    q7 = vhaddq_u8 (q14, q10);
    q4 = vrhaddq_u8(q7,  q13);
    q5 = vrhaddq_u8(q11, q12);
    q6 = vhaddq_u8 (q7,  q13);
    q7 = vhaddq_u8 (q11, q12);
    q4 = vhaddq_u8 (q4,  q5 );
    q6 = vrhaddq_u8(q6,  q7 );
    q4 = vrhaddq_u8(q4,  q6 );
    q6 = vrhaddq_u8(q14, q13);
    q5 = vrhaddq_u8(q11, q12);
    q5 = vhaddq_u8 (q6,  q5 );
    q6 = vhaddq_u8 (q14, q13);
    q7 = vhaddq_u8 (q11, q12);
    q6 = vrhaddq_u8(q6,  q7 );
    q5 = vrhaddq_u8(q5,  q6 );
    q6 = vhaddq_u8 (q12, q10);
    q6 = vrhaddq_u8(q6,  q13);

    q4 = vbslq_u8(q3, q4, q6); //    VBIF        q4,     q6,     q3    ;q0
    q3 = vbslq_u8(q3, q5, q13);//    VBSL        q3,     q5,     q13   ;q1

    q10 = vbslq_u8(q2,q0, q10);
    q11 = vbslq_u8(q2,q1, q11);
    q12 = vbslq_u8(q2,q4, q12);
    q13 = vbslq_u8(q2,q3, q13);

    q9 = vspill0;
    q14 = vspill1;

    TR8(q8,   q9 );
    TR8(q10,  q11);
    TR8(q12,  q13);
    TR8(q14,  q15);
    TR16(q8,  q10);
    TR16(q9,  q11);
    TR16(q12, q14);
    TR16(q13, q15);
    TR32(q8,  q12);
    TR32(q9,  q13);
    TR32(q10, q14);
    TR32(q11, q15);

    pix -= 8*stride + 4;
    vst1_u8(pix, vget_low_u8(q8)); pix += stride;
    vst1_u8(pix, vget_low_u8(q9)); pix += stride;
    vst1_u8(pix, vget_low_u8(q10)); pix += stride;
    vst1_u8(pix, vget_low_u8(q11)); pix += stride;
    vst1_u8(pix, vget_low_u8(q12)); pix += stride;
    vst1_u8(pix, vget_low_u8(q13)); pix += stride;
    vst1_u8(pix, vget_low_u8(q14)); pix += stride;
    vst1_u8(pix, vget_low_u8(q15)); pix += stride;

    vst1_u8(pix, vget_high_u8(q8)); pix += stride;
    vst1_u8(pix, vget_high_u8(q9)); pix += stride;
    vst1_u8(pix, vget_high_u8(q10)); pix += stride;
    vst1_u8(pix, vget_high_u8(q11)); pix += stride;
    vst1_u8(pix, vget_high_u8(q12)); pix += stride;
    vst1_u8(pix, vget_high_u8(q13)); pix += stride;
    vst1_u8(pix, vget_high_u8(q14)); pix += stride;
    vst1_u8(pix, vget_high_u8(q15)); pix += stride;
}

static void deblock_luma_h_neon(uint8_t *pix, int stride, int alpha, int beta, const uint8_t *pthr, const uint8_t *pstr)
{
    uint8x16_t q0, q1, q2, q3, q4, q5, q6, q7, q9, q10, q11, q12, q13, q14;
    uint8x16_t tmp;

    q9  = vld1q_u8(pix - 3*stride);
    q10 = vld1q_u8(pix - 2*stride);
    q11 = vld1q_u8(pix - 1*stride);
    q12 = vld1q_u8(pix);
    q13 = vld1q_u8(pix + 1*stride);
    q14 = vld1q_u8(pix + 2*stride);

    q1  = vabdq_u8(q11, q12);
    q2  = vcltq_u8(q1, vdupq_n_u8(alpha));
    q1  = vcltq_u8(vmaxq_u8(vabdq_u8(q11, q10), vabdq_u8(q12, q13)), vdupq_n_u8(beta));
    q2  = vandq_u8(q2, q1);

    tmp = vreinterpretq_u8_u32(vdupq_n_u32(*(uint32_t*)pstr));
    tmp = vzipq_u8(tmp, tmp).val[0];
    tmp = vzipq_u8(tmp, tmp).val[0];
    q1  = tmp;

    q1  = vcgtq_s8(vreinterpretq_s8_u8(q1), vdupq_n_s8(0));
    q2  = vandq_u8(q2, q1);
    q7 = vhsubq_u8(q10, q13);
    q7 = vreinterpretq_u8_s8(vshrq_n_s8(vreinterpretq_s8_u8(q7), 1));
    q0 = veorq_u8(q12, q11);
    q6 = vandq_u8(vdupq_n_u8(1), q0);

    q0 = vhsubq_u8(q12, q11);// ;(q0-p0))>>1

    q7 = vreinterpretq_u8_s8(vrhaddq_s8(vreinterpretq_s8_u8(q7), vreinterpretq_s8_u8(q6))); //((p1-q1)>>2 + carry + 1) >> 1
    q7 = vreinterpretq_u8_s8(vqaddq_s8(vreinterpretq_s8_u8(q0),  vreinterpretq_s8_u8(q7))); //=delta = (((q0-p0)<<2) + (p1-q1) + 4) >> 3;
    q7 = vandq_u8(q7, q2);

    tmp = vreinterpretq_u8_u32(vdupq_n_u32(*(uint32_t*)pthr));
    tmp = vzipq_u8(tmp, tmp).val[0];
    tmp = vzipq_u8(tmp, tmp).val[0];
    q1  = tmp;

    q1  = vandq_u8(q2, q1);

    q0 = vabdq_u8(q9,  q11); // ap = ABS(p2 - p0);
    q0 = vcltq_u8(q0,  vdupq_n_u8(beta)); //sp = (ap - beta) >> 31;
    q4 = vandq_u8(q0,  q2); // & sp
    q0 = vabdq_u8(q14, q12);//aq = ABS(q2 - q0);
    q0 = vcltq_u8(q0,  vdupq_n_u8(beta));//sq = (aq - beta) >> 31;
    q3 = vandq_u8(q0,  q2); // & sq

    q0  = vrhaddq_u8(q11, q12);//((p0+q0+1)>>1)
    q0  = vhaddq_u8 (q0,  q9 );//((p2 + ((p0+q0+1)>>1))>>1)
    q5  = vandq_u8  (q1,  q4 );
    q6  = vqaddq_u8 (q10, q5 );//{p1+thr}
    q0  = vminq_u8  (q0,  q6 );
    q6  = vqsubq_u8 (q10, q5 );//{p1-thr}
    q10 = vmaxq_u8  (q0,  q6 );

    q0   = vrhaddq_u8(q11, q12);// ;((p0+q0+1)>>1)
    q0   = vhaddq_u8 (q0,  q14);// ;((q2 + ((p0+q0+1)>>1))>>1)
    q5   = vandq_u8  (q1,  q3 );
    q6   = vqaddq_u8 (q13, q5 );// ;{q1+thr}
    q0   = vminq_u8  (q0,  q6 );
    q6   = vqsubq_u8 (q13, q5 );// ;{q1-thr}
    q13  = vmaxq_u8  (q0,  q6 );

    q1  = vreinterpretq_u8_s8(vsubq_s8(vreinterpretq_s8_u8(q1), vreinterpretq_s8_u8(q3)));
    q1  = vreinterpretq_u8_s8(vsubq_s8(vreinterpretq_s8_u8(q1), vreinterpretq_s8_u8(q4))); //tC = thr - sp - sq;
    q1  = vandq_u8(q1, q2);// ; set thr = 0 if str==0

    q6  = veorq_u8(q6, q6);
    q5  = vreinterpretq_u8_s8(vmaxq_s8(vreinterpretq_s8_u8(q6), vreinterpretq_s8_u8(q7))); //delta > 0
    q7  = vreinterpretq_u8_s8(vsubq_s8(vreinterpretq_s8_u8(q6), vreinterpretq_s8_u8(q7)));
    q6  = vreinterpretq_u8_s8(vmaxq_s8(vreinterpretq_s8_u8(q6), vreinterpretq_s8_u8(q7))); //-(delta < 0)
    q5  =  vminq_u8(q1, q5);
    q6  =  vminq_u8(q1, q6);

    q11 = vqaddq_u8(q11, q5);
    q11 = vqsubq_u8(q11, q6);
    q12 = vqsubq_u8(q12, q5);
    q12 = vqaddq_u8(q12, q6);

    vst1q_u8(pix - 2*stride, q10);
    vst1q_u8(pix - 1*stride, q11);
    vst1q_u8(pix           , q12);
    vst1q_u8(pix + 1*stride, q13);
}

static void deblock_chroma_v_neon(uint8_t *pix, int32_t stride, int a, int b, const uint8_t *thr, const uint8_t *str)
{
    int32x2_t d16 = vld1_s32((int32_t*)(pix - 2 + 0*stride));
    int32x2_t d18 = vld1_s32((int32_t*)(pix - 2 + 1*stride));
    int32x2_t d20 = vld1_s32((int32_t*)(pix - 2 + 2*stride));
    int32x2_t d22 = vld1_s32((int32_t*)(pix - 2 + 3*stride));
    int32x2_t d17 = vld1_s32((int32_t*)(pix - 2 + 4*stride));
    int32x2_t d19 = vld1_s32((int32_t*)(pix - 2 + 5*stride));
    int32x2_t d21 = vld1_s32((int32_t*)(pix - 2 + 6*stride));
    int32x2_t d23 = vld1_s32((int32_t*)(pix - 2 + 7*stride));
    int32x2x2_t tr0 = vtrn_s32(d16, d17);
    int32x2x2_t tr1 = vtrn_s32(d18, d19);
    int32x2x2_t tr2 = vtrn_s32(d20, d21);
    int32x2x2_t tr3 = vtrn_s32(d22, d23);
    int16x8x2_t tr4 = vtrnq_s16(vreinterpretq_s16_s32(vcombine_s32(tr0.val[0], tr0.val[1])), vreinterpretq_s16_s32(vcombine_s32(tr2.val[0], tr2.val[1])));
    int16x8x2_t tr5 = vtrnq_s16(vreinterpretq_s16_s32(vcombine_s32(tr1.val[0], tr1.val[1])), vreinterpretq_s16_s32(vcombine_s32(tr3.val[0], tr3.val[1])));
    uint8x16x2_t tr6 = vtrnq_u8(vreinterpretq_u8_s16(tr4.val[0]), vreinterpretq_u8_s16(tr5.val[0]));
    uint8x16x2_t tr7 = vtrnq_u8(vreinterpretq_u8_s16(tr4.val[1]), vreinterpretq_u8_s16(tr5.val[1]));

{
    uint8x16_t q8  = tr6.val[0];
    uint8x16_t q9  = tr6.val[1];
    uint8x16_t q10 = tr7.val[0];
    uint8x16_t q11 = tr7.val[1];

    uint8x16_t q1  = vabdq_u8(q9, q10);
    uint8x16_t q2  = vcltq_u8(q1, vdupq_n_u8(a));
    uint8x16_t q4  = vmaxq_u8(vabdq_u8(q10, q11), vabdq_u8(q8, q9));
    uint8x16_t q0;
    uint8x16_t q3;
    uint8x16_t q6;
     int8x16_t q4s;
     int8x16_t q7;
    uint8x16_t q7u;
    uint8x16_t q5;
    uint8x16_t vstr = vld1q_u8(str);
    uint8x16_t vthr = vld1q_u8(thr);

    q4 = vcltq_u8(q4, vdupq_n_u8(b));
    q2 = vandq_u8(q2, q4);
    q1 = vzipq_u8(vstr, vstr).val[0];
    q3 = vcgtq_s8(vreinterpretq_s8_u8(q1), vdupq_n_s8(0));
    q1 = vshrq_n_u8(q1, 2);
    q1 = vcgtq_s8(vreinterpretq_s8_u8(q1), vdupq_n_s8(0));
    q2 = vandq_u8(q2, q3);

    q0 = vzipq_u8(vthr, vthr).val[0];
    q0 = vaddq_u8(q0, vdupq_n_u8(1));
    q0 = vandq_u8(q0, q2);

    q7 = vshrq_n_s8(vreinterpretq_s8_u8(vhsubq_u8(q8, q11)), 1);
    q6 = vandq_u8(vdupq_n_u8(1), veorq_u8(q10, q9));
    q4 = vhsubq_u8(q10, q9);
    q7 = vrhaddq_s8(q7, vreinterpretq_s8_u8(q6));
    q7 = vqaddq_s8(vreinterpretq_s8_u8(q4), q7);

    q4s = vdupq_n_s8(0);
    q5 = vreinterpretq_u8_s8(vmaxq_s8(q4s,               q7));
    q4 = vreinterpretq_u8_s8(vmaxq_s8(q4s, vsubq_s8(q4s, q7)));
    q5 = vminq_u8(q0, q5);
    q4 = vminq_u8(q0, q4);

    q0 = vqaddq_u8(q9,  q5);
    q0 = vqsubq_u8(q0,  q4);
    q3 = vqsubq_u8(q10, q5);
    q3 = vqaddq_u8(q3,  q4);

    q6  = vrhaddq_u8(vhaddq_u8(q9, q11), q8);
    q7u = vrhaddq_u8(vhaddq_u8(q8, q10), q11);

    q0 = vbslq_u8(q1,  q6, q0 );
    q3 = vbslq_u8(q1, q7u, q3 );
    q9 = vbslq_u8(q2,  q0, q9 );
    q10= vbslq_u8(q2,  q3, q10);

    tr6 = vtrnq_u8(q8,  q9);
    tr7 = vtrnq_u8(q10, q11);

    tr4 = vtrnq_s16(vreinterpretq_s16_u8(tr6.val[0]), vreinterpretq_s16_u8(tr7.val[0]));
    tr5 = vtrnq_s16(vreinterpretq_s16_u8(tr6.val[1]), vreinterpretq_s16_u8(tr7.val[1]));

    tr0 = vtrn_s32(vget_low_s32(vreinterpretq_s32_s16(tr4.val[0])), vget_high_s32(vreinterpretq_s32_s16(tr4.val[0])));
    tr1 = vtrn_s32(vget_low_s32(vreinterpretq_s32_s16(tr5.val[0])), vget_high_s32(vreinterpretq_s32_s16(tr5.val[0])));
    tr2 = vtrn_s32(vget_low_s32(vreinterpretq_s32_s16(tr4.val[1])), vget_high_s32(vreinterpretq_s32_s16(tr4.val[1])));
    tr3 = vtrn_s32(vget_low_s32(vreinterpretq_s32_s16(tr5.val[1])), vget_high_s32(vreinterpretq_s32_s16(tr5.val[1])));

#if 0
    // unaligned store fools Android NDK 15 optimizer
    *(int32_t*)(uint8_t*)(pix - 2 + 0*stride) = vget_lane_s32(tr0.val[0], 0);
    *(int32_t*)(uint8_t*)(pix - 2 + 1*stride) = vget_lane_s32(tr1.val[0], 0);
    *(int32_t*)(uint8_t*)(pix - 2 + 2*stride) = vget_lane_s32(tr2.val[0], 0);
    *(int32_t*)(uint8_t*)(pix - 2 + 3*stride) = vget_lane_s32(tr3.val[0], 0);
    *(int32_t*)(uint8_t*)(pix - 2 + 4*stride) = vget_lane_s32(tr0.val[1], 0);
    *(int32_t*)(uint8_t*)(pix - 2 + 5*stride) = vget_lane_s32(tr1.val[1], 0);
    *(int32_t*)(uint8_t*)(pix - 2 + 6*stride) = vget_lane_s32(tr2.val[1], 0);
    *(int32_t*)(uint8_t*)(pix - 2 + 7*stride) = vget_lane_s32(tr3.val[1], 0);
#else
    vst1_lane_s16((int16_t*)(pix - 2 + 0*stride),     vreinterpret_s16_s32(tr0.val[0]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 0*stride) + 1, vreinterpret_s16_s32(tr0.val[0]), 1);
    vst1_lane_s16((int16_t*)(pix - 2 + 1*stride),     vreinterpret_s16_s32(tr1.val[0]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 1*stride) + 1, vreinterpret_s16_s32(tr1.val[0]), 1);
    vst1_lane_s16((int16_t*)(pix - 2 + 2*stride),     vreinterpret_s16_s32(tr2.val[0]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 2*stride) + 1, vreinterpret_s16_s32(tr2.val[0]), 1);
    vst1_lane_s16((int16_t*)(pix - 2 + 3*stride),     vreinterpret_s16_s32(tr3.val[0]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 3*stride) + 1, vreinterpret_s16_s32(tr3.val[0]), 1);
    vst1_lane_s16((int16_t*)(pix - 2 + 4*stride),     vreinterpret_s16_s32(tr0.val[1]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 4*stride) + 1, vreinterpret_s16_s32(tr0.val[1]), 1);
    vst1_lane_s16((int16_t*)(pix - 2 + 5*stride),     vreinterpret_s16_s32(tr1.val[1]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 5*stride) + 1, vreinterpret_s16_s32(tr1.val[1]), 1);
    vst1_lane_s16((int16_t*)(pix - 2 + 6*stride),     vreinterpret_s16_s32(tr2.val[1]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 6*stride) + 1, vreinterpret_s16_s32(tr2.val[1]), 1);
    vst1_lane_s16((int16_t*)(pix - 2 + 7*stride),     vreinterpret_s16_s32(tr3.val[1]), 0);
    vst1_lane_s16((int16_t*)(pix - 2 + 7*stride) + 1, vreinterpret_s16_s32(tr3.val[1]), 1);
#endif
}
}

static void deblock_chroma_h_neon(uint8_t *pix, int32_t stride, int a, int b, const uint8_t *thr, const uint8_t *str)
{
    uint8x16_t q0;
    uint8x16_t q8  = vld1q_u8(pix - 2*stride);
    uint8x16_t q9  = vld1q_u8(pix - 1*stride);
    uint8x16_t q10 = vld1q_u8(pix);
    uint8x16_t q11 = vld1q_u8(pix + stride);
    uint8x16_t q1  = vabdq_u8(q9, q10);
    uint8x16_t q2  = vcltq_u8(q1, vdupq_n_u8(a));
    uint8x16_t q4  = vmaxq_u8(vabdq_u8(q10, q11), vabdq_u8(q8, q9));
    uint8x16_t q3;
    uint8x16_t q6;
     int8x16_t q4s;
     int8x16_t q7;
    uint8x16_t q7u;
    uint8x16_t q5;
    uint8x16_t vstr = vld1q_u8(str);
    uint8x16_t vthr = vld1q_u8(thr);

    q4 = vcltq_u8(q4, vdupq_n_u8(b));
    q2 = vandq_u8(q2, q4);
    q1 = vzipq_u8(vstr, vstr).val[0];
    q3 = vcgtq_s8(vreinterpretq_s8_u8(q1), vdupq_n_s8(0));
    q1 = vshrq_n_u8(q1, 2);
    q1 = vcgtq_s8(vreinterpretq_s8_u8(q1), vdupq_n_s8(0));
    q2 = vandq_u8(q2, q3);

    q0 = vzipq_u8(vthr, vthr).val[0];
    q0 = vaddq_u8(q0, vdupq_n_u8(1));
    q0 = vandq_u8(q0, q2);

    q7 = vshrq_n_s8(vreinterpretq_s8_u8(vhsubq_u8(q8, q11)), 1);
    q6 = vandq_u8(vdupq_n_u8(1), veorq_u8(q10, q9));
    q4 = vhsubq_u8(q10, q9);
    q7 = vrhaddq_s8(q7, vreinterpretq_s8_u8(q6));
    q7 = vqaddq_s8(vreinterpretq_s8_u8(q4), q7);

    q4s = vdupq_n_s8(0);
    q5 = vreinterpretq_u8_s8(vmaxq_s8(q4s,               q7));
    q4 = vreinterpretq_u8_s8(vmaxq_s8(q4s, vsubq_s8(q4s, q7)));
    q5 = vminq_u8(q0, q5);
    q4 = vminq_u8(q0, q4);

    q0 = vqaddq_u8(q9,  q5);
    q0 = vqsubq_u8(q0,  q4);
    q3 = vqsubq_u8(q10, q5);
    q3 = vqaddq_u8(q3,  q4);

    q6  = vrhaddq_u8(vhaddq_u8(q9, q11), q8);
    q7u = vrhaddq_u8(vhaddq_u8(q8, q10), q11);

    q0 = vbslq_u8(q1,  q6, q0 );
    q3 = vbslq_u8(q1, q7u, q3 );
    q9 = vbslq_u8(q2,  q0, q9 );
    q10= vbslq_u8(q2,  q3, q10);

    vst1_u8(pix - stride, vget_low_u8(q9));
    vst1_u8(pix,          vget_low_u8(q10));
}

static void h264e_deblock_chroma_neon(uint8_t *pix, int32_t stride, const deblock_params_t *par)
{
    const uint8_t *alpha = par->alpha;
    const uint8_t *beta = par->beta;
    const uint8_t *thr = par->tc0;
    const uint8_t *strength = (uint8_t *)par->strength32;
    int a, b, x, y;
    a = alpha[0];
    b = beta[0];
    for (x = 0; x < 16; x += 8)
    {
        uint32_t str = *(uint32_t*)&strength[x];
        if (str && a)
        {
            deblock_chroma_v_neon(pix + (x >> 1), stride, a, b, thr + x, strength + x);
        }
        a = alpha[1];
        b = beta[1];
    }
    thr += 16;
    strength += 16;
    a = alpha[2];
    b = beta[2];
    for (y = 0; y < 16; y += 8)
    {
        uint32_t str = *(uint32_t*)&strength[y];
        if (str && a)
        {
            deblock_chroma_h_neon(pix, stride, a, b, thr + y, strength + y);
        }
        pix += 4*stride;
        a = alpha[3];
        b = beta[3];
    }
}

static void h264e_deblock_luma_neon(uint8_t *pix, int32_t stride, const deblock_params_t *par)
{
    const uint8_t *alpha = par->alpha;
    const uint8_t *beta = par->beta;
    const uint8_t *thr = par->tc0;
    const uint8_t *strength = (uint8_t *)par->strength32;
    int a = alpha[0];
    int b = beta[0];
    int x, y;
    for (x = 0; x < 16; x += 4)
    {
        uint32_t str = *(uint32_t*)&strength[x];
        if ((uint8_t)str == 4)
        {
            deblock_luma_v_s4_neon(pix + x, stride, a, b);
        } else if (str && a)
        {
            deblock_luma_v_neon(pix + x, stride, a, b, thr + x, strength + x);
        }
        a = alpha[1];
        b = beta[1];
    }
    a = alpha[2];
    b = beta[2];
    thr += 16;
    strength += 16;
    for (y = 0; y < 16; y += 4)
    {
        uint32_t str = *(uint32_t*)&strength[y];
        if ((uint8_t)str == 4)
        {
            deblock_luma_h_s4_neon(pix, stride, a, b);
        } else if (str && a)
        {
            deblock_luma_h_neon(pix, stride, a, b, thr + y, strength + y);
        }
        a = alpha[3];
        b = beta[3];
        pix += 4*stride;
    }
}

static void h264e_denoise_run_neon(unsigned char *frm, unsigned char *frmprev, int w, int h_arg, int stride_frm, int stride_frmprev)
{
    int cloop, h = h_arg;
    if (w <= 2 || h <= 2)
    {
        return;
    }
    w -= 2;
    h -= 2;

    do
    {
        unsigned char *pf = frm += stride_frm;
        unsigned char *pp = frmprev += stride_frmprev;
        cloop = w;
        pp[-stride_frmprev] = *pf++;
        pp++;

        for (;cloop >= 8; cloop -= 8, pf += 8, pp += 8)
        {
            uint16x8_t vp0w;
            uint32x4_t vpr0;
            uint32x4_t vpr1;
            uint16x8_t vf0w;
            int16x8_t vcls, vt, vcl, vgn, vgd;
            uint16x8_t vg;
            uint8x8_t vf0 = vld1_u8(pf);
            uint8x8_t vft = vld1_u8(pf - stride_frm);
            uint8x8_t vfb = vld1_u8(pf + stride_frm);
            uint8x8_t vfl = vld1_u8(pf - 1);
            uint8x8_t vfr = vld1_u8(pf + 1);
            uint8x8_t vp0 = vld1_u8(pp);
            uint8x8_t vpt = vld1_u8(pp - stride_frmprev);
            uint8x8_t vpb = vld1_u8(pp + stride_frmprev);
            uint8x8_t vpl = vld1_u8(pp - 1);
            uint8x8_t vpr = vld1_u8(pp + 1);
            uint16x8_t vd  = vabdl_u8(vf0, vp0);
            uint16x8_t vfs = vaddw_u8(vaddw_u8(vaddl_u8(vft, vfb), vfl), vfr);
            uint16x8_t vps = vaddw_u8(vaddw_u8(vaddl_u8(vpt, vpb), vpl), vpr);
            uint16x8_t vneighbourhood = vshrq_n_u16(vabdq_u16(vfs, vps), 2);

            vt = vaddq_s16(vreinterpretq_s16_u16(vd), vdupq_n_s16(1));

            vt = vqshlq_n_s16(vt, 7);
            vcls = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcls);
            vt = vqdmulhq_s16(vt,vt);                             // 1

            vcl = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);
            vt = vqdmulhq_s16(vt,vt);                             // 2
            vcl = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);
            vt = vqdmulhq_s16(vt,vt);                             // 3
            vcl = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);
            vt = vqdmulhq_s16(vt,vt);                             // 4
            vcl = vclsq_s16(vt);
            // vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);

            vgd = vsubq_s16(vdupq_n_s16(127), vcls);

            // same as above 
            vt = vaddq_s16(vreinterpretq_s16_u16(vneighbourhood), vdupq_n_s16(1));
            
            vt = vqshlq_n_s16(vt, 7);
            vcls = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcls);
            vt = vqdmulhq_s16(vt,vt);                             // 1
            vcl = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);
            vt = vqdmulhq_s16(vt,vt);                             // 2
            vcl = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);
            vt = vqdmulhq_s16(vt,vt);                             // 3
            vcl = vclsq_s16(vt);
            vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);
            vt = vqdmulhq_s16(vt,vt);                             // 4
            vcl = vclsq_s16(vt);
            // vt = vshlq_s16(vt, vcl);
            vcls = vaddq_s16(vaddq_s16(vcls, vcls), vcl);

            vgn = vsubq_s16(vdupq_n_s16(127), vcls);

            vgn = vreinterpretq_s16_u16(vshrq_n_u16(vqshlq_n_u16(vreinterpretq_u16_s16(vgn), 10), 8));            // <<=2, saturated

            vgd = vsubq_s16(vdupq_n_s16(255), vgd);
            vgn = vsubq_s16(vdupq_n_s16(255), vgn);

            //vst1_u8(pp - stride_frmprev, vreinterpret_u8_s8(vmovn_s16(vgn)));
            //vst1_u8(pp - stride_frmprev, vreinterpret_u8_s8(vmovn_s16(vreinterpretq_s16_u16(vneighbourhood))));
            //vst1_u8(pp - stride_frmprev, vp0);

            vg  = vmulq_u16(vreinterpretq_u16_s16(vgn), vreinterpretq_u16_s16(vgd));

            vp0w = vmovl_u8(vp0);
            vpr0 = vmull_u16(vget_low_u16(vp0w), vget_low_u16(vg));
            vpr1 = vmull_u16(vget_high_u16(vp0w), vget_high_u16(vg));
            vg = vreinterpretq_u16_s16(vsubq_s16(vreinterpretq_s16_u8(vdupq_n_u8(255)), vreinterpretq_s16_u16(vg)));

            vf0w = vmovl_u8(vf0);
            vpr0 = vmlal_u16(vpr0, vget_low_u16(vf0w), vget_low_u16(vg));
            vpr1 = vmlal_u16(vpr1, vget_high_u16(vf0w), vget_high_u16(vg));

            vst1_u8(pp - stride_frmprev, vmovn_u16(vcombine_u16(vrshrn_n_u32(vpr0, 16), vrshrn_n_u32(vpr1, 16))));
        }                    

        while (cloop--)
        {
            int d, neighbourhood;
            unsigned g, gd, gn, out_val;
            d = pf[0] - pp[0];
            neighbourhood  = pf[-1] - pp[-1];
            neighbourhood += pf[+1] - pp[+1];
            neighbourhood += pf[-stride_frm] - pp[-stride_frmprev];
            neighbourhood += pf[+stride_frm] - pp[+stride_frmprev];

            if (d < 0) 
            {
                d = -d;
            }
            if (neighbourhood < 0) 
            {
                neighbourhood = -neighbourhood;
            }
            neighbourhood >>= 2;

            gd = g_diff_to_gainQ8[d];
            gn = g_diff_to_gainQ8[neighbourhood];

            gn <<= 2;
            if (gn > 255) 
            {
                gn = 255;
            }

            gn = 255 - gn;
            gd = 255 - gd;
            g = gn*gd;  // Q8*Q8 = Q16;

            //out_val = ((pp[0]*g ) >> 16) + (((0xffff-g)*pf[0] ) >> 16);
            //out_val = ((pp[0]*g + (1<<15)) >> 16) + (((0xffff-g)*pf[0]  + (1<<15)) >> 16);
            out_val = (pp[0]*g + (0xffff - g)*pf[0]  + (1 << 15)) >> 16;
            
            assert(out_val <= 255);
            
            pp[-stride_frmprev] = (unsigned char)out_val;
            //pp[-stride_frmprev] = gn;
            //pp[-stride_frmprev] = neighbourhood;
            //pp[-stride_frmprev] = pp[0];

            pf++, pp++;
        } 

        pp[-stride_frmprev] = *pf++;
    } while(--h);

    memcpy(frmprev + stride_frmprev, frm + stride_frm, w + 2);
    h = h_arg - 2;
    do
    {
        memcpy(frmprev, frmprev - stride_frmprev, w + 2);
        frmprev -= stride_frmprev;
    } while(--h);
    memcpy(frmprev, frm - stride_frm*(h_arg - 2), w + 2);
}

#undef IS_NULL
#define IS_NULL(p) ((p) < (pix_t *)32)

static uint32_t intra_predict_dc4_neon(const pix_t *left, const pix_t *top)
{
    unsigned dc = 0, side = 4, round = 0;
    uint32x2_t s = vdup_n_u32(0);

    if (!IS_NULL(left))
    {
        s = vpaddl_u16(vpaddl_u8(vld1_u8(left)));
        round += side >> 1;
    }
    if (!IS_NULL(top))
    {
        s = vadd_u32(s, vpaddl_u16(vpaddl_u8(vld1_u8(top))));
        round += side >> 1;
    }
    dc = vget_lane_u32(s, 0);

    dc += round;
    if (round == side) dc >>= 1;
    dc >>= 2;
    if (!round) dc = 128;
    return dc * 0x01010101;
}

static uint8x16_t intra_predict_dc16_neon(const pix_t *left, const pix_t *top)
{
    unsigned dc = 0, side = 16, round = 0;

    if (!IS_NULL(left))
    {
        uint8x16_t v = vld1q_u8(left);
        uint64x2_t s = vpaddlq_u32(vpaddlq_u16(vpaddlq_u8(v)));
        uint64x1_t q = vadd_u64(vget_high_u64(s), vget_low_u64(s));
        dc += vget_lane_u32(vreinterpret_u32_u64(q), 0);
        round += side >> 1;
    }
    if (!IS_NULL(top))
    {
        uint8x16_t v = vld1q_u8(top);
        uint64x2_t s = vpaddlq_u32(vpaddlq_u16(vpaddlq_u8(v)));
        uint64x1_t q = vadd_u64(vget_high_u64(s), vget_low_u64(s));
        dc += vget_lane_u32(vreinterpret_u32_u64(q), 0);
        round += side >> 1;
    }
    dc += round;
    if (round == side) dc >>= 1;
    dc >>= 4;
    if (!round) dc = 128;
    return vdupq_n_u8(dc);
}

/*
 * Note: To make the code more readable we refer to the neighboring pixels
 * in variables named as below:
 *
 *    UL U0 U1 U2 U3 U4 U5 U6 U7
 *    L0 xx xx xx xx
 *    L1 xx xx xx xx
 *    L2 xx xx xx xx
 *    L3 xx xx xx xx
 */
#define UL edge[-1] 
#define U0 edge[0] 
#define T1 edge[1] 
#define U2 edge[2] 
#define U3 edge[3] 
#define U4 edge[4] 
#define U5 edge[5] 
#define U6 edge[6] 
#define U7 edge[7] 
#define L0 edge[-2]
#define L1 edge[-3]
#define L2 edge[-4]
#define L3 edge[-5]

static void h264e_intra_predict_16x16_neon(pix_t *predict, const pix_t *left, const pix_t *top, int mode)
{
    int cloop = 4;
    uint32_t *d = (uint32_t*)predict;
    uint32x4_t v;
    assert(IS_ALIGNED(predict, 4));
    assert(IS_ALIGNED(top, 4));
    if (mode != 1)
    {
        if (mode < 1)
        {
            v = vld1q_u32((uint32_t*)top);
        } else //(mode == 2)
        {
            v = vreinterpretq_u32_u8(intra_predict_dc16_neon(left, top));
        }
        do
        {
            vst1q_u32(d, v); d += 4;
            vst1q_u32(d, v); d += 4;
            vst1q_u32(d, v); d += 4;
            vst1q_u32(d, v); d += 4;
        } while (--cloop);
    } else //if (mode == 1)
    {
        do
        {
            vst1q_u8((uint8_t*)d, vdupq_n_u8(*left++)); d += 4;
            vst1q_u8((uint8_t*)d, vdupq_n_u8(*left++)); d += 4;
            vst1q_u8((uint8_t*)d, vdupq_n_u8(*left++)); d += 4;
            vst1q_u8((uint8_t*)d, vdupq_n_u8(*left++)); d += 4;
        } while (--cloop);
    }
}

static void h264e_intra_predict_chroma_neon(pix_t *predict, const pix_t *left, const pix_t *top, int mode)
{
    int cloop = 8;
    uint32_t *d = (uint32_t*)predict;
    uint32x4_t v;
    assert(IS_ALIGNED(predict, 4));
    assert(IS_ALIGNED(top, 4));
    if (mode < 1)
    {
        v = vld1q_u32((uint32_t*)top);
        vst1q_u32(d, v); d += 4;
        vst1q_u32(d, v); d += 4;
        vst1q_u32(d, v); d += 4;
        vst1q_u32(d, v); d += 4;
        vst1q_u32(d, v); d += 4;
        vst1q_u32(d, v); d += 4;
        vst1q_u32(d, v); d += 4;
        vst1q_u32(d, v); d += 4;
    } else if (mode == 1)
    {
        do 
        {
            v = vreinterpretq_u32_u8(vcombine_u8(vdup_n_u8(left[0]), vdup_n_u8(left[8])));
            vst1q_u32(d, v); d += 4;
            left++;
        } while(--cloop);
    } else //if (mode == 2)
    {
        int ccloop = 2;
        cloop = 2;
        do
        {
            d[0] = d[1] = d[16] = intra_predict_dc4_neon(left, top);
            d[17] = intra_predict_dc4_neon(left + 4, top + 4);
            if (!IS_NULL(top))
            {
                d[1] = intra_predict_dc4_neon(NULL, top + 4);
            }
            if (!IS_NULL(left))
            {
                d[16] = intra_predict_dc4_neon(NULL, left + 4);
            }
            d += 2;
            left += 8;
            top += 8;
        } while(--cloop);

        do
        {
            v = vld1q_u32(d - 4);
            vst1q_u32(d, v); d += 4;
            vst1q_u32(d, v); d += 4;
            vst1q_u32(d, v); d += 4;
            d += 4;
        } while(--ccloop);
    }
}

static __inline int vsad_neon(uint8x16_t a, uint8x16_t b)
{
    uint64x2_t s = vpaddlq_u32(vpaddlq_u16(vpaddlq_u8(vabdq_u8(a, b))));
    uint64x1_t q = vadd_u64(vget_high_u64(s), vget_low_u64(s));
    return vget_lane_u32(vreinterpret_u32_u64(q), 0);
}

static int h264e_intra_choose_4x4_neon(const pix_t *blockin, pix_t *blockpred, int avail, const pix_t *edge, int mpred, int penalty)
{
    int sad, best_sad, best_m = 2;

    uint32_t r0, r1, r2, r3;
    uint8x16_t vx, vt, vr, vpred, v1, v2, v8, v9, q1, q2, q10, q11, q12;
    uint8x8_t d2, d3;

    r0 = ((uint32_t *)blockin)[ 0];
    r1 = ((uint32_t *)blockin)[ 4];
    r2 = ((uint32_t *)blockin)[ 8];
    r3 = ((uint32_t *)blockin)[12];
    vr = vcombine_u8(vcreate_u8(((uint64_t)r1 << 32) | r0), vcreate_u8(((uint64_t)r3 << 32) | r2));

#define VTEST(mode) sad = vsad_neon(vr,vx);    \
            if (mode != mpred) sad += penalty; \
            if (sad < best_sad)                \
            {                                  \
                vpred = vx;                    \
                best_sad = sad;                \
                best_m = mode;                 \
            }

    // DC
    vx = vdupq_n_u8(intra_predict_dc4_neon((avail & AVAIL_L) ? &L3 : 0, (avail & AVAIL_T) ? &U0 : 0));

    best_sad = vsad_neon(vx, vr);
    if (2 != mpred) 
    {   
        best_sad += penalty;
    }
    vpred = vx;

    vt = vld1q_u8(&L3);
    vt = vreinterpretq_u8_u32(vsetq_lane_u32(U7*0x01010101, vreinterpretq_u32_u8(vt), 3));
    if (avail & AVAIL_T)
    {
        uint32x2_t t2;
        if (!(avail & AVAIL_TR))
        {
            vt = vcombine_u8(vget_low_u8(vt), vdup_n_u8(U3));
        }

        vx =  vreinterpretq_u8_u32(vdupq_n_u32(*(uint32_t*)&U0));
        VTEST(0);

        vx = vt;
        vx = vrhaddq_u8(vhaddq_u8(vextq_u8(vx, vx, 5), vextq_u8(vx, vx, 7)), vextq_u8(vx, vx, 6));

        v1 = vextq_u8(vx, vx, 1);
        d2 = vext_u8(vget_low_u8(vx), vget_low_u8(vx), 2);
        d3 = vext_u8(vget_low_u8(vx), vget_low_u8(vx), 3);
        vx = vreinterpretq_u8_u32(vcombine_u32(
            t2 = vzip_u32(vreinterpret_u32_u8(vget_low_u8(vx)), vreinterpret_u32_u8(vget_low_u8(v1))).val[0], 
            vzip_u32(vreinterpret_u32_u8(d2), vreinterpret_u32_u8(d3)).val[0]));
        VTEST(3);

        vx = vt;
        vx = vrhaddq_u8(vextq_u8(vt, vt, 5), vextq_u8(vt, vt, 6));
        vx = vreinterpretq_u8_u32(vzipq_u32(vreinterpretq_u32_u8(vx), vreinterpretq_u32_u8(vextq_u8(vx, vx, 1))).val[0]);
        vx = vreinterpretq_u8_u32(vzipq_u32(vreinterpretq_u32_u8(vx), 
        vreinterpretq_u32_u8(vcombine_u8(vreinterpret_u8_u32(t2), vget_high_u8(vextq_u8(vt, vt, 7))))).val[0]);

        VTEST(7);
    }

    if (avail & AVAIL_L)
    {
        vx = vrev32q_u8(vt);
        vx = vzipq_u8(vx, vx).val[0];
        vx = vzipq_u8(vx, vx).val[0];
        VTEST(1);

        v2 = vrev32q_u8(vt);
        v8 = vrev32q_u8(vt);
        vx = vrev32q_u8(vt);
        v8 = vzipq_u8(vx, vx).val[0];
        {
            int tmp = vgetq_lane_u16(vreinterpretq_u16_u8(v8), 3);
            v2 = vreinterpretq_u8_u16(vsetq_lane_u16(tmp, vreinterpretq_u16_u8(v2), 2));
            v8 = vreinterpretq_u8_u16(vsetq_lane_u16(tmp, vreinterpretq_u16_u8(v8), 4));
            v9 = vextq_u8(v2, v2, 14);
            v9 = vzipq_u8(v9, vhaddq_u8(v9, v2)).val[0]; 
            v9 = vrhaddq_u8(v9, vextq_u8(v8, v8, 14));
            tmp |= tmp << 16;
            vx = vreinterpretq_u8_u32(vzipq_u32(vreinterpretq_u32_u8(vextq_u8(v9, v9, 4)),
                                                vreinterpretq_u32_u8(vextq_u8(v9, v9, 6))).val[0]);
            vx = vreinterpretq_u8_u32(vsetq_lane_u32(tmp, vreinterpretq_u32_u8(vx), 3));
        }
        VTEST(8);
    }

    if ((avail & (AVAIL_T | AVAIL_L | AVAIL_TL)) == (AVAIL_T | AVAIL_L | AVAIL_TL))
    {
        uint32x2x2_t pair;
        uint8x8_t d4, d6;
        int lr;
        q11 = q2 = vrhaddq_u8(vhaddq_u8(vt, vextq_u8(vt, vt, 2)), q10 = vextq_u8(vt, vt, 1));
        d4 = vget_low_u8(q2);
        d6 = vreinterpret_u8_u32(vzip_u32(vreinterpret_u32_u8(vext_u8(d4, d4, 3)), vreinterpret_u32_u8(vext_u8(d4, d4, 1))).val[0]);
        d4 = vreinterpret_u8_u32(vzip_u32(vreinterpret_u32_u8(vext_u8(d4, d4, 2)), vreinterpret_u32_u8(d4)).val[0]);
        pair = vzip_u32(vreinterpret_u32_u8(d6), vreinterpret_u32_u8(d4));
        vx = vcombine_u8(vreinterpret_u8_u32(pair.val[0]), vreinterpret_u8_u32(pair.val[1]));
        VTEST(4);

        vx  = q12 = vrhaddq_u8(vt, q10);
        q1  = vzipq_u8(vx, q11).val[0];
        q1  = vreinterpretq_u8_u32(vzipq_u32(vreinterpretq_u32_u8(q1), vreinterpretq_u32_u8(vextq_u8(q1, q1, 2))).val[0]);
        q1  = vreinterpretq_u8_u32(vrev64q_u32(vreinterpretq_u32_u8(q1)));
        vx  = vcombine_u8(vget_high_u8(q1), vget_low_u8(q1));
        vx = vreinterpretq_u8_u16(
            vsetq_lane_u16(vgetq_lane_u16(vreinterpretq_u16_u8(q11), 2), vreinterpretq_u16_u8(vx), 1));
        VTEST(6);

        q11 = vextq_u8(q11, q11, 1);
        q1  = vextq_u8(q12, q12, 4);
        q2  = vextq_u8(q11, q11, 2);
        q1  = vreinterpretq_u8_u32(vzipq_u32(vreinterpretq_u32_u8(q1), vreinterpretq_u32_u8(q2)).val[0]);
        q12 = vreinterpretq_u8_u16(vsetq_lane_u16(lr = vgetq_lane_u16(vreinterpretq_u16_u8(q11), 0), vreinterpretq_u16_u8(q12), 1));
        q11 = vreinterpretq_u8_u16(vsetq_lane_u16((lr << 8) & 0xffff, vreinterpretq_u16_u8(q11), 0));
        vx = vcombine_u8(vget_low_u8(q1), vreinterpret_u8_u32(vzip_u32(
            vreinterpret_u32_u8(vext_u8(vget_low_u8(q12), vget_low_u8(q12), 3)),
            vreinterpret_u32_u8(vext_u8(vget_low_u8(q11), vget_low_u8(q11), 1))
            ).val[0]));
        VTEST(5);
    }

    vst1q_lane_u32(((uint32_t *)blockpred) + 0, vreinterpretq_u32_u8(vpred ), 0);
    vst1q_lane_u32(((uint32_t *)blockpred) + 4, vreinterpretq_u32_u8(vpred ), 1);
    vst1q_lane_u32(((uint32_t *)blockpred) + 8, vreinterpretq_u32_u8(vpred ), 2);
    vst1q_lane_u32(((uint32_t *)blockpred) +12, vreinterpretq_u32_u8(vpred ), 3);
    return best_m + (best_sad << 4); // pack result
}

static void copy_wh_neon(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    if (w == 4)
    {
        do
        {
            *(int32_t*)dst = *(int32_t*)src; dst += 16; src += src_stride;
            *(int32_t*)dst = *(int32_t*)src; dst += 16; src += src_stride;
            *(int32_t*)dst = *(int32_t*)src; dst += 16; src += src_stride;
            *(int32_t*)dst = *(int32_t*)src; dst += 16; src += src_stride;
        } while (h -= 4);
    } else if (w == 8)
    {
        do
        {
            vst1_u8(dst, vld1_u8(src)); dst += 16; src += src_stride;
            vst1_u8(dst, vld1_u8(src)); dst += 16; src += src_stride;
            vst1_u8(dst, vld1_u8(src)); dst += 16; src += src_stride;
            vst1_u8(dst, vld1_u8(src)); dst += 16; src += src_stride;
        } while (h -= 4);
    } else
    {
        do
        {
            uint8x16_t v0, v1, v2, v3;
            v0 = vld1q_u8(src); src += src_stride;
            v1 = vld1q_u8(src); src += src_stride;
            v2 = vld1q_u8(src); src += src_stride;
            v3 = vld1q_u8(src); src += src_stride;

            vst1q_u8(dst, v0); dst += 16; 
            vst1q_u8(dst, v1); dst += 16; 
            vst1q_u8(dst, v2); dst += 16; 
            vst1q_u8(dst, v3); dst += 16; 
        } while (h -= 4);
    }
}

static void hpel_lpf_hor_neon(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    uint8x8_t c5 = vdup_n_u8(5);
    uint8x8_t c20 = vshl_n_u8(c5, 2);
    if (w == 16)
    {
        do
        {
            uint8x16_t s0 = vld1q_u8(src - 2);
            uint8x16_t s1 = vld1q_u8(src - 2 + 16);
            uint8x16_t v0 = s0;
            uint8x16_t v1 = vextq_u8(s0, s1, 1);
            uint8x16_t v2 = vextq_u8(s0, s1, 2);
            uint8x16_t v3 = vextq_u8(s0, s1, 3);
            uint8x16_t v4 = vextq_u8(s0, s1, 4);
            uint8x16_t v5 = vextq_u8(s0, s1, 5);

            uint16x8_t q, s = vaddl_u8(vget_low_u8(v0), vget_low_u8(v5));
            s = vmlsl_u8(s, vget_low_u8(v1), c5);
            s = vmlsl_u8(s, vget_low_u8(v4), c5);
            s = vmlal_u8(s, vget_low_u8(v2), c20);
            s = vmlal_u8(s, vget_low_u8(v3), c20);

            q = vaddl_u8(vget_high_u8(v0), vget_high_u8(v5));
            q = vmlsl_u8(q, vget_high_u8(v1), c5);
            q = vmlsl_u8(q, vget_high_u8(v4), c5);
            q = vmlal_u8(q, vget_high_u8(v2), c20);
            q = vmlal_u8(q, vget_high_u8(v3), c20);

            vst1q_u8(dst, vcombine_u8(
                vqrshrun_n_s16(vreinterpretq_s16_u16(s), 5),
                vqrshrun_n_s16(vreinterpretq_s16_u16(q), 5)));

            dst += 16;
            src += src_stride;
        } while (--h);
    } else
    {
        do
        {
            uint8x16_t line = vld1q_u8(src - 2);
            uint8x8_t s0 = vget_low_u8(line);
            uint8x8_t s1 = vget_high_u8(line);
            uint8x8_t v0 = s0;
            uint8x8_t v1 = vext_u8(s0, s1, 1);
            uint8x8_t v2 = vext_u8(s0, s1, 2);
            uint8x8_t v3 = vext_u8(s0, s1, 3);
            uint8x8_t v4 = vext_u8(s0, s1, 4);
            uint8x8_t v5 = vext_u8(s0, s1, 5);

            uint16x8_t s = vaddl_u8(v0, v5);
            s = vmlsl_u8(s, v1, c5);
            s = vmlsl_u8(s, v4, c5);
            s = vmlal_u8(s, v2, c20);
            s = vmlal_u8(s, v3, c20);

            vst1_u8(dst, vqrshrun_n_s16(vreinterpretq_s16_u16(s), 5));

            dst += 16;
            src += src_stride;
        } while (--h);
    }
}

static void hpel_lpf_hor16_neon(const uint8_t *src, int src_stride, int16_t *h264e_restrict dst, int w, int h)
{
    uint8x8_t c5 = vdup_n_u8(5);
    uint8x8_t c20 = vshl_n_u8(c5, 2);
    if (w == 16)
    {
        do
        {
            uint8x16_t s0 = vld1q_u8(src - 2);
            uint8x16_t s1 = vld1q_u8(src - 2 + 16);
            uint8x16_t v0 = s0;
            uint8x16_t v1 = vextq_u8(s0, s1, 1);
            uint8x16_t v2 = vextq_u8(s0, s1, 2);
            uint8x16_t v3 = vextq_u8(s0, s1, 3);
            uint8x16_t v4 = vextq_u8(s0, s1, 4);
            uint8x16_t v5 = vextq_u8(s0, s1, 5);

            uint16x8_t q, s = vaddl_u8(vget_low_u8(v0), vget_low_u8(v5));
            s = vmlsl_u8(s, vget_low_u8(v1), c5);
            s = vmlsl_u8(s, vget_low_u8(v4), c5);
            s = vmlal_u8(s, vget_low_u8(v2), c20);
            s = vmlal_u8(s, vget_low_u8(v3), c20);

            q = vaddl_u8(vget_high_u8(v0), vget_high_u8(v5));
            q = vmlsl_u8(q, vget_high_u8(v1), c5);
            q = vmlsl_u8(q, vget_high_u8(v4), c5);
            q = vmlal_u8(q, vget_high_u8(v2), c20);
            q = vmlal_u8(q, vget_high_u8(v3), c20);

            vst1q_s16(dst, vreinterpretq_s16_u16(s));
            vst1q_s16(dst + 8, vreinterpretq_s16_u16(q));

            dst += 16;
            src += src_stride;
        } while (--h);
    } else
    {
        do
        {
            uint8x16_t line = vld1q_u8(src - 2);
            uint8x8_t s0 = vget_low_u8(line);
            uint8x8_t s1 = vget_high_u8(line);
            uint8x8_t v0 = s0;
            uint8x8_t v1 = vext_u8(s0, s1,  1);
            uint8x8_t v2 = vext_u8(s0, s1, 2);
            uint8x8_t v3 = vext_u8(s0, s1, 3);
            uint8x8_t v4 = vext_u8(s0, s1, 4);
            uint8x8_t v5 = vext_u8(s0, s1, 5);

            uint16x8_t s = vaddl_u8(v0, v5);
            s = vmlsl_u8(s, v1, c5);
            s = vmlsl_u8(s, v4, c5);
            s = vmlal_u8(s, v2, c20);
            s = vmlal_u8(s, v3, c20);

            vst1q_s16(dst, vreinterpretq_s16_u16(s));

            dst += 16;
            src += src_stride;
        } while (--h);
    }
}

static void hpel_lpf_ver_neon(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    uint8x8_t c5 = vdup_n_u8(5);
    uint8x8_t c20 = vshl_n_u8(c5, 2);

    if (w == 16)
    {
        uint8x16_t v0 = vld1q_u8(src - 2*src_stride);
        uint8x16_t v1 = vld1q_u8(src - 1*src_stride);
        uint8x16_t v2 = vld1q_u8(src);
        uint8x16_t v3 = vld1q_u8(src + 1*src_stride);
        uint8x16_t v4 = vld1q_u8(src + 2*src_stride);
        do
        {
            uint8x16_t v5 = vld1q_u8(src + 3*src_stride);
            uint16x8_t q, s = vaddl_u8(vget_low_u8(v0), vget_low_u8(v5));
            s = vmlsl_u8(s, vget_low_u8(v1), c5);
            s = vmlsl_u8(s, vget_low_u8(v4), c5);
            s = vmlal_u8(s, vget_low_u8(v2), c20);
            s = vmlal_u8(s, vget_low_u8(v3), c20);

            q = vaddl_u8(vget_high_u8(v0), vget_high_u8(v5));
            q = vmlsl_u8(q, vget_high_u8(v1), c5);
            q = vmlsl_u8(q, vget_high_u8(v4), c5);
            q = vmlal_u8(q, vget_high_u8(v2), c20);
            q = vmlal_u8(q, vget_high_u8(v3), c20);

            vst1q_u8(dst, vcombine_u8(
                vqrshrun_n_s16(vreinterpretq_s16_u16(s), 5),
                vqrshrun_n_s16(vreinterpretq_s16_u16(q), 5)));
            dst += 16;
            src += src_stride;
            v0 = v1;
            v1 = v2;
            v2 = v3;
            v3 = v4;
            v4 = v5;
        } while (--h);
    } else
    {
        uint8x8_t v0 = vld1_u8(src - 2*src_stride);
        uint8x8_t v1 = vld1_u8(src - 1*src_stride);
        uint8x8_t v2 = vld1_u8(src);
        uint8x8_t v3 = vld1_u8(src + 1*src_stride);
        uint8x8_t v4 = vld1_u8(src + 2*src_stride);
        do
        {
            uint8x8_t v5 = vld1_u8(src + 3*src_stride);
            uint16x8_t s = vaddl_u8(v0, v5);
            s = vmlsl_u8(s, v1, c5);
            s = vmlsl_u8(s, v4, c5);
            s = vmlal_u8(s, v2, c20);
            s = vmlal_u8(s, v3, c20);

            vst1_u8(dst, vqrshrun_n_s16(vreinterpretq_s16_u16(s), 5));
            dst += 16;
            src += src_stride;
            v0 = v1;
            v1 = v2;
            v2 = v3;
            v3 = v4;
            v4 = v5;
        } while (--h);
    }
}

static void hpel_lpf_ver16_neon(const int16_t *src, uint8_t *h264e_restrict dst, int w, int h)
{
    do
    {
        int cloop = h;
        int16x8_t v0 = vld1q_s16(src);
        int16x8_t v1 = vld1q_s16(src + 16);
        int16x8_t v2 = vld1q_s16(src + 16*2);
        int16x8_t v3 = vld1q_s16(src + 16*3);
        int16x8_t v4 = vld1q_s16(src + 16*4);
        do
        {
            int16x8_t v5 = vld1q_s16(src+16*5);

            int16x8_t s0 = vaddq_s16(v0, v5);
            int16x8_t s1 = vaddq_s16(v1, v4);
            int16x8_t s2 = vaddq_s16(v2, v3);

            int16x8_t vs = vshrq_n_s16(vsubq_s16(s0, s1), 2);
            int16x8_t vq = vsubq_s16(s2, s1);
            s0 = vshrq_n_s16(vaddq_s16(vq, vs), 2);
            s0 = vaddq_s16(s0, s2);

            vst1_u8(dst, vqrshrun_n_s16(s0, 6));

            dst += 16;
            src += 16;
            v0 = v1;
            v1 = v2;
            v2 = v3;
            v3 = v4;
            v4 = v5;
        } while (--cloop);

        src -= 16*h - 8;
        dst -= 16*h - 8;
    } while (w -= 8);
}

static void hpel_lpf_diag_neon(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    ALIGN(16) int16_t scratch[21 * 16] ALIGN2(16);  /* 21 rows by 16 pixels per row */

    /*
     * Intermediate values will be 1/2 pel at Horizontal direction
     * Starting at (0.5, -2) at top extending to (0.5, height + 3) at bottom
     * scratch contains a 2D array of size (w)X(h + 5)
     */
    hpel_lpf_hor16_neon(src - 2*src_stride, src_stride, scratch, w, h + 5);
    hpel_lpf_ver16_neon(scratch, dst, w, h);
}

static void average_16x16_unalign_neon(uint8_t *dst, const uint8_t *src, int src_stride)
{
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
    vst1q_u8(dst, vrhaddq_u8(vld1q_u8(dst), vld1q_u8(src)));  src += src_stride; dst += 16;
}

static void h264e_qpel_average_wh_align_neon(const uint8_t *src0, const uint8_t *src1, uint8_t *dst, point_t wh)
{
    int w = wh.s.x;
    int h = wh.s.y;
    int cloop = h;
    if (w == 8)
    {
        do
        {
            vst1_u8(dst, vrhadd_u8(vld1_u8(src0), vld1_u8(src1)));
            dst += 16;
            src0 += 16;
            src1 += 16;
        } while (--cloop);
    } else
    {
        do
        {
            vst1q_u8(dst, vrhaddq_u8(vld1q_u8(src0), vld1q_u8(src1)));
            dst += 16;
            src0 += 16;
            src1 += 16;
        } while (--cloop);
    }
}

static void h264e_qpel_interpolate_luma_neon(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, point_t wh, point_t dxdy)
{
//    src += ((dx + 1) >> 2) + ((dy + 1) >> 2)*src_stride;            // dx == 3 ? next row; dy == 3 ? next line
//    dxdy              actions: Horizontal, Vertical, Diagonal, Average
//    0 1 2 3 +1        -   ha    h    ha+
//    1                 va  hva   hda  hv+a
//    2                 v   vda   d    v+da
//    3                 va+ h+va h+da  h+v+a
//    +stride
    int32_t pos = 1 << (dxdy.s.x + 4*dxdy.s.y);

    if (pos == 1)
    {
        copy_wh_neon(src, src_stride, dst, wh.s.x, wh.s.y);
    } else
    {
        ALIGN(16) uint8_t scratch[16*16] ALIGN2(16);
        int dstused = 0;
        if (pos & 0xe0ee)// 1110 0000 1110 1110
        {
            hpel_lpf_hor_neon(src + ((pos & 0xe000) ? src_stride : 0), src_stride, dst, wh.s.x, wh.s.y);
            dstused++;
        }
        if (pos & 0xbbb0)// 1011 1011 1011 0000
        {
            hpel_lpf_ver_neon(src + ((pos & 0x8880) ? 1 : 0), src_stride, dstused ? scratch : dst, wh.s.x, wh.s.y);
            dstused++;
        }
        if (pos & 0x4e40)// 0100 1110 0100 0000
        {
            hpel_lpf_diag_neon(src, src_stride, dstused ? scratch : dst, wh.s.x, wh.s.y);
            dstused++;
        }
        if (pos & 0xfafa)// 1111 1010 1111 1010
        {
            assert(wh.s.x == 16 && wh.s.y == 16);
            if (dstused == 2)
            {
                point_t p;

                src = scratch;
                src_stride = 16;
                p.u32 = 16 + (16 << 16);

                h264e_qpel_average_wh_align_neon(src, dst, dst, p);
            } else
            {
                src += ((dxdy.s.x + 1) >> 2) + ((dxdy.s.y + 1) >> 2)*src_stride;
                average_16x16_unalign_neon(dst, src, src_stride);
            }
        }
    }
}

static void h264e_qpel_interpolate_chroma_neon(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, point_t wh, point_t dxdy)
{
    /* if fractionl mv is not (0, 0) */
    if (dxdy.u32)
    {
        uint8x8_t v8 = vdup_n_u8(8);
        uint8x8_t vx = vdup_n_u8(dxdy.s.x);
        uint8x8_t vy = vdup_n_u8(dxdy.s.y);
        uint8x8_t v8x = vsub_u8(v8, vx);
        uint8x8_t v8y = vsub_u8(v8, vy);
        uint8x8_t va = vmul_u8(v8x, v8y);
        uint8x8_t vb = vmul_u8(vx, v8y);
        uint8x8_t vc = vmul_u8(v8x, vy);
        uint8x8_t vd = vmul_u8(vx, vy);
        int h = wh.s.y;
        if (wh.s.x == 8)
        {
            uint8x16_t vt0 = vld1q_u8(src);
            uint8x16_t vt1 = vextq_u8(vt0, vt0, 1);
            src += src_stride;
            do
            {
                uint8x16_t vb0 = vld1q_u8(src);
                uint8x16_t vb1 = vextq_u8(vb0, vb0, 1);
                uint16x8_t vs = vmull_u8(vget_low_u8(vt0), va);
                vs = vmlal_u8(vs, vget_low_u8(vt1), vb);
                vs = vmlal_u8(vs, vget_low_u8(vb0), vc);
                vs = vmlal_u8(vs, vget_low_u8(vb1), vd);
                vst1_u8(dst, vqrshrun_n_s16(vreinterpretq_s16_u16(vs), 6));
                vt0 = vb0;
                vt1 = vb1;
                dst += 16;
                src += src_stride;
             } while(--h);
         } else
         {
            uint8x8_t vt0 = vld1_u8(src);
            uint8x8_t vt1 = vext_u8(vt0, vt0, 1);
            src += src_stride;
            do
            {
                uint8x8_t vb0 = vld1_u8(src);
                uint8x8_t vb1 = vext_u8(vb0, vb0, 1);
                uint16x8_t vs = vmull_u8(vt0, va);
                vs = vmlal_u8(vs, vt1, vb);
                vs = vmlal_u8(vs, vb0, vc);
                vs = vmlal_u8(vs, vb1, vd);
                *(int32_t*)dst = vget_lane_s32(vreinterpret_s32_u8(vqrshrun_n_s16(vreinterpretq_s16_u16(vs), 6)), 0);
                vt0 = vb0;
                vt1 = vb1;
                dst += 16;
                src += src_stride;
             } while(--h);
         }
    } else
    {
        copy_wh_neon(src, src_stride, dst, wh.s.x, wh.s.y);
    }
}

static int h264e_sad_mb_unlaign_8x8_neon(const pix_t *a, int a_stride, const pix_t *b, int _sad[4])
{
    uint16x8_t s0, s1;
    uint8x16_t va, vb;
    int cloop = 2, sum = 0;
    do
    {
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabdl_u8(    vget_low_u8(va), vget_low_u8(vb));   s1 = vabdl_u8(    vget_high_u8(va), vget_high_u8(vb)); 
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
        va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
        s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
        {
        uint32x4_t v0 = vpaddlq_u16(s0);
        uint64x2_t v1 = vpaddlq_u32(v0);
        sum += _sad[0] = (int)(vgetq_lane_u64(v1, 0)+vgetq_lane_u64(v1, 1));
        v0 = vpaddlq_u16(s1);
        v1 = vpaddlq_u32(v0);
        sum += _sad[1] = (int)(vgetq_lane_u64(v1, 0)+vgetq_lane_u64(v1, 1));
        _sad += 2;
        }
    } while(--cloop);
    return sum;
}

static int h264e_sad_mb_unlaign_wh_neon(const pix_t *a, int a_stride, const pix_t *b, point_t wh)
{
    uint16x8_t s0, s1;
    uint8x16_t va, vb;
    int cloop = wh.s.y/8, sum = 0;
    if (wh.s.x == 16)
    {
        do
        {
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabdl_u8(    vget_low_u8(va), vget_low_u8(vb));   s1 = vabdl_u8(    vget_high_u8(va), vget_high_u8(vb)); 
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));   s1 = vabal_u8(s1, vget_high_u8(va), vget_high_u8(vb)); 

            uint32x4_t v0 = vpaddlq_u16(s0);
            uint64x2_t v1 = vpaddlq_u32(v0);
            sum += vgetq_lane_u64(v1, 0) + vgetq_lane_u64(v1, 1);

            v0 = vpaddlq_u16(s1);
            v1 = vpaddlq_u32(v0);
            sum += vgetq_lane_u64(v1, 0) + vgetq_lane_u64(v1, 1);
        } while(--cloop);
    } else
    {
        do
        {
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabdl_u8(    vget_low_u8(va), vget_low_u8(vb));
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));
            va = vld1q_u8(a), vb = vld1q_u8(b);  a += a_stride, b += 16;
            s0 = vabal_u8(s0, vget_low_u8(va), vget_low_u8(vb));

            uint32x4_t v0 = vpaddlq_u16(s0);
            uint64x2_t v1 = vpaddlq_u32(v0);
            sum += vgetq_lane_u64(v1, 0) + vgetq_lane_u64(v1, 1);
        } while(--cloop);
    }
    return sum;
}

static void h264e_copy_8x8_neon(pix_t *d, int d_stride, const pix_t *s)
{
    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;
    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;
    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;
    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;

    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;
    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;
    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;
    vst1_u8(d, vld1_u8(s)); s += 16;  d += d_stride;
}


static void h264e_copy_16x16_neon(pix_t *d, int d_stride, const pix_t *s, int s_stride)
{
    assert(!((unsigned)d & 7));
    assert(!((unsigned)s & 7));
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;

    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;

    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;

    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
    vst1q_u8(d, vld1q_u8(s)); s += s_stride; d += d_stride;
}

// Keep intermediate data in transposed format.
// Save transpose for vectorized implementation
// TODO: TRANSPOSE_BLOCK==0 broken
#define TRANSPOSE_BLOCK     0
#define UNZIGSAG_IN_QUANT   0

#define SUM_DIF(a, b) { int t = a + b; b = a - b; a = t; }

static void hadamar4_2d_neon(int16_t *x)
{
    int16x8_t q0 = vld1q_s16(x);
    int16x8_t q1 = vld1q_s16(x + 8);
    int16x8_t s = vaddq_s16(q0, q1);
    int16x8_t d = vsubq_s16(q0, q1);
    int16x8_t q2 = vcombine_s16(vget_low_s16(s), vget_low_s16(d));
    int16x8_t q3 = vcombine_s16(vget_high_s16(s), vget_high_s16(d));
    q0 = vaddq_s16(q2, q3);
    d  = vsubq_s16(q2, q3);
    q1 = vcombine_s16(vget_high_s16(d), vget_low_s16(d));
{
    int16x4x2_t t0 = vtrn_s16(vget_low_s16(q0), vget_high_s16(q0));
    int16x4x2_t t1 = vtrn_s16(vget_low_s16(q1), vget_high_s16(q1));
    int32x4x2_t tq = vtrnq_s32(vreinterpretq_s32_s16(vcombine_s16(t0.val[0], t0.val[1])), vreinterpretq_s32_s16(vcombine_s16(t1.val[0], t1.val[1])));

    q0 = vcombine_s16(vget_low_s16(vreinterpretq_s16_s32(tq.val[0])), vget_high_s16(vreinterpretq_s16_s32(tq.val[0])));
    q1 = vcombine_s16(vget_low_s16(vreinterpretq_s16_s32(tq.val[1])), vget_high_s16(vreinterpretq_s16_s32(tq.val[1])));

    s = vaddq_s16(q0, q1);
    d = vsubq_s16(q0, q1);
    q2 = vcombine_s16(vget_low_s16(s), vget_low_s16(d));
    q3 = vcombine_s16(vget_high_s16(s), vget_high_s16(d));
    q0 = vaddq_s16(q2, q3);
    d = vsubq_s16(q2, q3);
    q1 = vcombine_s16(vget_high_s16(d), vget_low_s16(d));
    vst1q_s16(x, q0);
    vst1q_s16(x + 8, q1);
}
}

static void dequant_dc_neon(quant_t *q, int16_t *qval, int dequant, int n)
{
    do q++->dq[0] = (int16_t)(*qval++*(int16_t)dequant); while (--n);
}

static void quant_dc_neon(int16_t *qval, int16_t *deq, int16_t quant, int n, int round_q18)
{
#if 1
    int r_minus =  (1 << 18) - round_q18;
    static const uint8_t iscan16[16] = {0, 2, 3, 9, 1, 4, 8, 10, 5, 7, 11, 14, 6, 12, 13, 15};
    static const uint8_t iscan4[4] = {0, 1, 2, 3};
    const uint8_t *scan = n == 4 ? iscan4 : iscan16;
    do
    {
        int v = *qval;
        int r = v < 0 ? r_minus : round_q18;
        deq[*scan++] = *qval++ = (v * quant + r) >> 18;
    } while (--n);
#else
    int r_minus =  (1 << 18) - round_q18;
    do
    {
        int v = *qval;
        int r = v < 0 ? r_minus : round_q18;
        *deq++ = *qval++ = (v * quant + r) >> 18;
    } while (--n);
#endif
}

static void hadamar2_2d_neon(int16_t *x)
{
    int a = x[0];
    int b = x[1];
    int c = x[2];
    int d = x[3];
    x[0] = (int16_t)(a + b + c + d);
    x[1] = (int16_t)(a - b + c - d);
    x[2] = (int16_t)(a + b - c - d);
    x[3] = (int16_t)(a - b - c + d);
}

static void h264e_quant_luma_dc_neon(quant_t *q, int16_t *deq, const uint16_t *qdat)
{
    int16_t *tmp = ((int16_t*)q) - 16;
    hadamar4_2d_neon(tmp);
    quant_dc_neon(tmp, deq, qdat[0], 16, 0x20000);//0x15555);
    hadamar4_2d_neon(tmp);
    assert(!(qdat[1] & 3));
    // dirty trick here: shift w/o rounding, since it have no effect  for qp >= 10 (or, to be precise, for qp => 9)
    dequant_dc_neon(q, tmp, qdat[1] >> 2, 16);
}

static int h264e_quant_chroma_dc_neon(quant_t *q, int16_t *deq, const uint16_t *qdat)
{
    int16_t *tmp = ((int16_t*)q) - 16;
    hadamar2_2d_neon(tmp);
    quant_dc_neon(tmp, deq, (int16_t)(qdat[0] << 1), 4, 0xAAAA);
    hadamar2_2d_neon(tmp);
    assert(!(qdat[1] & 1));
    dequant_dc_neon(q, tmp, qdat[1] >> 1, 4);
    return !!(tmp[0] | tmp[1] | tmp[2] | tmp[3]);
}

#define TRANSFORM(x0, x1, x2, x3, p, s) { \
    int t0 = x0 + x3;                     \
    int t1 = x0 - x3;                     \
    int t2 = x1 + x2;                     \
    int t3 = x1 - x2;                     \
    (p)[  0] = (int16_t)(t0 + t2);        \
    (p)[  s] = (int16_t)(t1*2 + t3);      \
    (p)[2*s] = (int16_t)(t0 - t2);        \
    (p)[3*s] = (int16_t)(t1 - t3*2);      \
}

static void FwdTransformResidual4x42_neon(const uint8_t *inp, const uint8_t *pred, uint32_t inp_stride, int16_t *out)
{
#if TRANSPOSE_BLOCK
    int i;
    int16_t tmp[16];
    // Transform columns
    for (i = 0; i < 4; i++, pred++, inp++)
    {
        int f0 = inp[0] - pred[0];
        int f1 = inp[1*inp_stride] - pred[1*16];
        int f2 = inp[2*inp_stride] - pred[2*16];
        int f3 = inp[3*inp_stride] - pred[3*16];
        TRANSFORM(f0, f1, f2, f3, tmp + i*4, 1);
    }
    // Transform rows
    for (i = 0; i < 4; i++)
    {
        int d0 = tmp[i + 0];
        int d1 = tmp[i + 4];
        int d2 = tmp[i + 8];
        int d3 = tmp[i + 12];
        TRANSFORM(d0, d1, d2, d3, out + i, 4);
    }
#else
    /* Transform rows */
    uint8x8_t inp0  = vreinterpret_u8_s32(vtrn_s32(vreinterpret_s32_u8(vld1_u8(inp)),  vreinterpret_s32_u8(vld1_u8(inp + inp_stride))).val[0]);
    uint8x8_t inp1  = vreinterpret_u8_s32(vtrn_s32(vreinterpret_s32_u8(vld1_u8(inp + 2*inp_stride)), vreinterpret_s32_u8(vld1_u8(inp + 3*inp_stride))).val[0]);
    uint8x8_t pred0 = vreinterpret_u8_s32(vtrn_s32(vreinterpret_s32_u8(vld1_u8(pred)),  vreinterpret_s32_u8(vld1_u8(pred + 16))).val[0]);
    uint8x8_t pred1 = vreinterpret_u8_s32(vtrn_s32(vreinterpret_s32_u8(vld1_u8(pred + 2*16)), vreinterpret_s32_u8(vld1_u8(pred + 3*16))).val[0]);
    int16x8_t q0 = vreinterpretq_s16_u16(vsubl_u8(inp0, pred0));
    int16x8_t q1 = vreinterpretq_s16_u16(vsubl_u8(inp1, pred1));

    int16x4x2_t  t0 = vtrn_s16(vget_low_s16(q0), vget_high_s16(q0));
    int16x4x2_t  t1 = vtrn_s16(vget_low_s16(q1), vget_high_s16(q1));
    int32x4x2_t  tq = vtrnq_s32(vreinterpretq_s32_s16(vcombine_s16(t0.val[0], t0.val[1])), vreinterpretq_s32_s16(vcombine_s16(t1.val[0], t1.val[1])));

    int16x4_t d4 = vadd_s16(vget_low_s16(vreinterpretq_s16_s32(tq.val[0])), vget_high_s16(vreinterpretq_s16_s32(tq.val[1])));
    int16x4_t d5 = vsub_s16(vget_low_s16(vreinterpretq_s16_s32(tq.val[0])), vget_high_s16(vreinterpretq_s16_s32(tq.val[1])));
    int16x4_t d6 = vadd_s16(vget_high_s16(vreinterpretq_s16_s32(tq.val[0])), vget_low_s16(vreinterpretq_s16_s32(tq.val[1])));
    int16x4_t d7 = vsub_s16(vget_high_s16(vreinterpretq_s16_s32(tq.val[0])), vget_low_s16(vreinterpretq_s16_s32(tq.val[1])));
    int16x8_t q2 = vcombine_s16(d4, d5);
    int16x8_t q3 = vcombine_s16(d6, d7);
    q0 = vaddq_s16(q2, q3);
    q0 = vcombine_s16(vget_low_s16(q0), vadd_s16(vget_high_s16(q0), d5));
    q1 = vsubq_s16(q2, q3);
    q1 = vcombine_s16(vget_low_s16(q1), vsub_s16(vget_high_s16(q1), d7));

    t0 = vtrn_s16(vget_low_s16(q0), vget_high_s16(q0));
    t1 = vtrn_s16(vget_low_s16(q1), vget_high_s16(q1));
    tq = vtrnq_s32(vreinterpretq_s32_s16(vcombine_s16(t0.val[0], t0.val[1])), vreinterpretq_s32_s16(vcombine_s16(t1.val[0], t1.val[1])));

    d4 = vadd_s16(vget_low_s16(vreinterpretq_s16_s32(tq.val[0])), vget_high_s16(vreinterpretq_s16_s32(tq.val[1])));
    d5 = vsub_s16(vget_low_s16(vreinterpretq_s16_s32(tq.val[0])), vget_high_s16(vreinterpretq_s16_s32(tq.val[1])));
    d6 = vadd_s16(vget_high_s16(vreinterpretq_s16_s32(tq.val[0])), vget_low_s16(vreinterpretq_s16_s32(tq.val[1])));
    d7 = vsub_s16(vget_high_s16(vreinterpretq_s16_s32(tq.val[0])), vget_low_s16(vreinterpretq_s16_s32(tq.val[1])));
    q2 = vcombine_s16(d4, d5);
    q3 = vcombine_s16(d6, d7);
    q0 = vaddq_s16(q2, q3);
    q0 = vcombine_s16(vget_low_s16(q0), vadd_s16(vget_high_s16(q0), d5));
    q1 = vsubq_s16(q2, q3);
    q1 = vcombine_s16(vget_low_s16(q1), vsub_s16(vget_high_s16(q1), d7));

    vst1q_s16(out, q0);
    vst1q_s16(out + 8, q1);
#endif
}

static void TransformResidual4x4_neon(const int16_t *pSrc, const pix_t *pred, pix_t *out, int out_stride)
{
    int16x4_t e0, e1, e2, e3;
    int16x4_t f0, f1, f2, f3;
    int16x4_t g0, g1, g2, g3;
    int16x4_t h0, h1, h2, h3;
    int16x4_t d0 = vld1_s16(pSrc);
    int16x4_t d1 = vld1_s16(pSrc + 4);
    int16x4_t d2 = vld1_s16(pSrc + 8);
    int16x4_t d3 = vld1_s16(pSrc + 12);
    int16x4x2_t dd0 = vtrn_s16(d0, d1);
    int16x4x2_t dd1 = vtrn_s16(d2, d3);
    int32x4x2_t d = vtrnq_s32(vreinterpretq_s32_s16(vcombine_s16(dd0.val[0], dd0.val[1])), vreinterpretq_s32_s16(vcombine_s16(dd1.val[0], dd1.val[1])));
    d0 = vreinterpret_s16_s32(vget_low_s32(d.val[0]));
    d1 = vreinterpret_s16_s32(vget_high_s32(d.val[0]));
    d2 = vreinterpret_s16_s32(vget_low_s32(d.val[1]));
    d3 = vreinterpret_s16_s32(vget_high_s32(d.val[1]));

    e0 = vadd_s16(d0, d2);
    e1 = vsub_s16(d0, d2);
    e2 = vsub_s16(vshr_n_s16(d1, 1), d3);
    e3 = vadd_s16(d1, vshr_n_s16(d3, 1));
    f0 = vadd_s16(e0, e3);
    f1 = vadd_s16(e1, e2);
    f2 = vsub_s16(e1, e2);
    f3 = vsub_s16(e0, e3);

    dd0 = vtrn_s16(f0, f1);
    dd1 = vtrn_s16(f2, f3);
    d = vtrnq_s32(vreinterpretq_s32_s16(vcombine_s16(dd0.val[0], dd0.val[1])), vreinterpretq_s32_s16(vcombine_s16(dd1.val[0], dd1.val[1])));
    f0 = vreinterpret_s16_s32(vget_low_s32(d.val[0]));
    f1 = vreinterpret_s16_s32(vget_high_s32(d.val[0]));
    f2 = vreinterpret_s16_s32(vget_low_s32(d.val[1]));
    f3 = vreinterpret_s16_s32(vget_high_s32(d.val[1]));

    g0 = vadd_s16(f0, f2);
    g1 = vsub_s16(f0, f2);
    g2 = vsub_s16(vshr_n_s16(f1, 1), f3);
    g3 = vadd_s16(f1, vshr_n_s16(f3, 1));
    h0 = vadd_s16(g0, g3);
    h1 = vadd_s16(g1, g2);
    h2 = vsub_s16(g1, g2);
    h3 = vsub_s16(g0, g3);

    {
        uint8x8_t inp0 = vreinterpret_u8_s32(vtrn_s32(vreinterpret_s32_u8(vld1_u8(pred)),  vreinterpret_s32_u8(vld1_u8(pred + 16))).val[0]);
        uint8x8_t inp1 = vreinterpret_u8_s32(vtrn_s32(vreinterpret_s32_u8(vld1_u8(pred + 2*16)), vreinterpret_s32_u8(vld1_u8(pred + 3*16))).val[0]);
        int16x8_t a0 = vaddq_s16(vcombine_s16(h0, h1), vreinterpretq_s16_u16(vshll_n_u8(inp0, 6)));
        int16x8_t a1 = vaddq_s16(vcombine_s16(h2, h3), vreinterpretq_s16_u16(vshll_n_u8(inp1, 6)));
        uint8x8_t r0 = vqrshrun_n_s16(a0, 6);
        uint8x8_t r1 = vqrshrun_n_s16(a1, 6);
        *(uint32_t*)(&out[0*out_stride]) = vget_lane_u32(vreinterpret_u32_u8(r0), 0);
        *(uint32_t*)(&out[1*out_stride]) = vget_lane_u32(vreinterpret_u32_u8(r0), 1);
        *(uint32_t*)(&out[2*out_stride]) = vget_lane_u32(vreinterpret_u32_u8(r1), 0);
        *(uint32_t*)(&out[3*out_stride]) = vget_lane_u32(vreinterpret_u32_u8(r1), 1);
    }
}

static int is_zero_neon(const int16_t *dat, int i0, const uint16_t *thr)
{
    static const uint16x8_t g_ign_first = { 0, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
    int16x8_t v0 = vabsq_s16(*(int16x8_t *)dat);
    int16x8_t v1 = vabsq_s16(*(int16x8_t *)(dat + 8));
    int16x8_t t = *(int16x8_t *)thr;
    uint16x8_t m0 = vcgtq_s16(v0, t);
    uint16x8_t m1 = vcgtq_s16(v1, t);
    if (i0)
        m0 = vandq_u16(m0, g_ign_first);
    m0 = vorrq_u16(m0, m1);
    uint16x4_t m4 = vorr_u16(vget_low_u16(m0), vget_high_u16(m0));
    return !(vget_lane_u32(vreinterpret_u32_u16(m4), 0) | vget_lane_u32(vreinterpret_u32_u16(m4), 1));
}

static int is_zero4_neon(const quant_t *q, int i0, const uint16_t *thr)
{
    return is_zero_neon(q[0].dq, i0, thr) &&
           is_zero_neon(q[1].dq, i0, thr) &&
           is_zero_neon(q[4].dq, i0, thr) &&
           is_zero_neon(q[5].dq, i0, thr);
}

static int zero_smallq_neon(quant_t *q, int mode, const uint16_t *qdat)
{
    int zmask = 0;
    int i, i0 = mode & 1, n = mode >> 1;
    if (mode == QDQ_MODE_INTER || mode == QDQ_MODE_CHROMA)
    {
        for (i = 0; i < n*n; i++)
        {
            if (is_zero_neon(q[i].dq, i0, qdat + OFFS_THR_1_OFF))
            {
                zmask |= (1 << i); //9.19
            }
        }
        if (mode == QDQ_MODE_INTER)   //8.27
        {
            if ((~zmask & 0x0033) && is_zero4_neon(q +  0, i0, qdat + OFFS_THR_2_OFF)) zmask |= 0x33;
            if ((~zmask & 0x00CC) && is_zero4_neon(q +  2, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 2);
            if ((~zmask & 0x3300) && is_zero4_neon(q +  8, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 8);
            if ((~zmask & 0xCC00) && is_zero4_neon(q + 10, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 10);
        }
    }
    return zmask;
}

static int quantize_neon(quant_t *q, int mode, const uint16_t *qdat, int zmask)
{
#if UNZIGSAG_IN_QUANT
#if TRANSPOSE_BLOCK
    //         ; Zig-zag scan      Transposed zig-zag
    //         ;    0 1 5 6        0 2 3 9
    //         ;    2 4 7 C        1 4 8 A
    //         ;    3 8 B D        5 7 B E
    //         ;    9 A E F        6 C D F
    static const unsigned char iscan16[16] = {0, 2, 3, 9, 1, 4, 8, 10, 5, 7, 11, 14, 6, 12, 13, 15};
#else
    static const unsigned char iscan16[16] = {0, 1, 5, 6, 2, 4, 7, 12, 3, 8, 11, 13, 9, 10, 14, 15};
#endif
#endif
    int ccol, crow, nz_block_mask = 0;
    ccol = mode >> 1;
    crow = ccol;
    do
    {
        do
        {
            int nz_mask = 0;

            if (zmask & 1)
            {
                int32_t *p = (int32_t *)q->qv;
                *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
                *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
            } else
            {
                static const uint8_t iscan16_neon [] = {
                    0x00,0x01,0x02,0x03,0x08,0x09,0x10,0x11,
                    0x0A,0x0B,0x04,0x05,0x06,0x07,0x0C,0x0D,
                    0x12,0x13,0x18,0x19,0x1A,0x1B,0x14,0x15,
                    0x0E,0x0F,0x16,0x17,0x1C,0x1D,0x1E,0x1F};
                static const uint16_t imask16_neon [] = {
                    0x0001,0x0002,0x0004,0x0008,
                    0x0010,0x0020,0x0040,0x0080,
                    0x0100,0x0200,0x0400,0x0800,
                    0x1000,0x2000,0x4000,0x8000};
                short save = 0;
                uint8x16_t q8,q9;
                int16x8_t q0 = vld1q_s16(q->dq);
                int16x8_t q1 = vld1q_s16(q->dq + 8);
                uint16x8_t r =  vdupq_n_u16(qdat[OFFS_RND_INTER]);
                uint16x8_t r0 = veorq_u16(r, vcltq_s16(q0, vdupq_n_s16(0)));
                uint16x8_t r1 = veorq_u16(r, vcltq_s16(q1, vdupq_n_s16(0)));
                int16x4_t d4, d5, d6, d7;
                int16x4_t d22, d23, d24, d25;
                int16x4_t d26, d27, d28, d29;

                d4 = d6 = vdup_n_s16(qdat[2]);
                d5 = d7 = vdup_n_s16(qdat[3]);
                d4 = vset_lane_s16(qdat[0], d4, 0);
                d4 = vset_lane_s16(qdat[0], d4, 2);
                d5 = vset_lane_s16(qdat[1], d5, 0);
                d5 = vset_lane_s16(qdat[1], d5, 2);
                d6 = vset_lane_s16(qdat[4], d6, 1);
                d6 = vset_lane_s16(qdat[4], d6, 3);
                d7 = vset_lane_s16(qdat[5], d7, 1);
                d7 = vset_lane_s16(qdat[5], d7, 3);

                d22 = vqshrn_n_s32(vreinterpretq_s32_u32(vaddw_u16(vreinterpretq_u32_s32(vmull_s16(vget_low_s16(q0), d4)), vget_low_u16(r0))), 16);
                d26 = vmul_s16(d22, d5);
                d23 = vqshrn_n_s32(vreinterpretq_s32_u32(vaddw_u16(vreinterpretq_u32_s32(vmull_s16(vget_high_s16(q0), d6)), vget_high_u16(r0))), 16);
                d27 = vmul_s16(d23, d7);
                d24 = vqshrn_n_s32(vreinterpretq_s32_u32(vaddw_u16(vreinterpretq_u32_s32(vmull_s16(vget_low_s16(q1), d4)), vget_low_u16(r1))), 16);
                d28 = vmul_s16(d24, d5);
                d25 = vqshrn_n_s32(vreinterpretq_s32_u32(vaddw_u16(vreinterpretq_u32_s32(vmull_s16(vget_high_s16(q1), d6)), vget_high_u16(r1))), 16);
                d29 = vmul_s16(d25, d7);
                if (mode & 1)
                {
                    save = q->dq[0];
                }
                vst1q_s16(q->dq,     vcombine_s16(d26, d27));
                vst1q_s16(q->dq + 8, vcombine_s16(d28, d29));
                if (mode & 1)
                {
                    q->dq[0] = save;
                }

                if (mode & 1)
                {
                    save = q->qv[0];
                }
                q8 = vld1q_u8(iscan16_neon);
                q9 = vld1q_u8(iscan16_neon + 16);

                {
// vtbl4_u8 is marked unavailable for iOS arm64, use wider versions there.
#if defined(__APPLE__) && defined(__aarch64__) &&  defined(__apple_build_version__)
                uint8x16x2_t vlut;
                vlut.val[0] = vreinterpretq_u8_s16(vcombine_s16(d22, d23));
                vlut.val[1] = vreinterpretq_u8_s16(vcombine_s16(d24, d25));
                vst1_s16(q->qv + 0, d4 = vreinterpret_s16_u8(vtbl2q_u8(vlut, vget_low_u8(q8))));
                vst1_s16(q->qv + 4, d5 = vreinterpret_s16_u8(vtbl2q_u8(vlut, vget_high_u8(q8))));
                vst1_s16(q->qv + 8, d6 = vreinterpret_s16_u8(vtbl2q_u8(vlut, vget_low_u8(q9))));
                vst1_s16(q->qv +12, d7 = vreinterpret_s16_u8(vtbl2q_u8(vlut, vget_high_u8(q9))));
#else
                uint8x8x4_t vlut;
                vlut.val[0] = vreinterpret_u8_s16(d22);
                vlut.val[1] = vreinterpret_u8_s16(d23);
                vlut.val[2] = vreinterpret_u8_s16(d24);
                vlut.val[3] = vreinterpret_u8_s16(d25);
                vst1_s16(q->qv + 0, d4 = vreinterpret_s16_u8(vtbl4_u8(vlut, vget_low_u8(q8))));
                vst1_s16(q->qv + 4, d5 = vreinterpret_s16_u8(vtbl4_u8(vlut, vget_high_u8(q8))));
                vst1_s16(q->qv + 8, d6 = vreinterpret_s16_u8(vtbl4_u8(vlut, vget_low_u8(q9))));
                vst1_s16(q->qv +12, d7 = vreinterpret_s16_u8(vtbl4_u8(vlut, vget_high_u8(q9))));
#endif
                }
                {
                    uint16x8_t bm0 = vld1q_u16(imask16_neon);
                    uint16x8_t bm1 = vld1q_u16(imask16_neon + 8);
                    uint16x4_t m;
                    bm0 = vandq_u16(bm0, vceqq_s16(vcombine_s16(d4, d5), vdupq_n_s16(0)));
                    bm1 = vandq_u16(bm1, vceqq_s16(vcombine_s16(d6, d7), vdupq_n_s16(0)));
                    bm0 = vorrq_u16(bm0, bm1);
                    m = vorr_u16(vget_low_u16(bm0), vget_high_u16(bm0));
                    m = vpadd_u16(m, m);
                    m = vpadd_u16(m, m);
                    nz_mask = vget_lane_u16(vmvn_u16(m), 0);
                }

                if (mode & 1)
                {
                    q->qv[0] = save;
                    nz_mask &= ~1;
                }
            }

            zmask >>= 1;
            nz_block_mask <<= 1;
            if (nz_mask)
                nz_block_mask |= 1;
            q++;
        } while (--ccol);
        ccol = mode >> 1;
    } while (--crow);
    return nz_block_mask;
}

static void transform_neon(const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q)
{
    int crow = mode >> 1;
    int ccol = crow;

    do
    {
        do
        {
            FwdTransformResidual4x42_neon(inp, pred, inp_stride, q->dq);
            q++;
            inp += 4;
            pred += 4;
        } while (--ccol);
        ccol = mode >> 1;
        inp += 4*(inp_stride - ccol);
        pred += 4*(16 - ccol);
    } while (--crow);
}

static int h264e_transform_sub_quant_dequant_neon(const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q, const uint16_t *qdat)
{
    int zmask;
    transform_neon(inp, pred, inp_stride, mode, q);
    if (mode & 1) // QDQ_MODE_INTRA_16 || QDQ_MODE_CHROMA
    {
        int cloop = (mode >> 1)*(mode >> 1);
        short *dc = ((short *)q) - 16;
        quant_t *pq = q;
        do
        {
            *dc++ = pq->dq[0];
            pq++;
        } while (--cloop);
    }
    zmask = zero_smallq_neon(q, mode, qdat);
    return quantize_neon(q, mode, qdat, zmask);
}

static void h264e_transform_add_neon(pix_t *out, int out_stride, const pix_t *pred, quant_t *q, int side, int32_t mask)
{
    int crow = side;
    int ccol = crow;

    assert(!((unsigned)out % 4));
    assert(!((unsigned)pred % 4));
    assert(!(out_stride % 4));
    do
    {
        do
        {
            if (mask >= 0)
            {
                // copy 4x4
                pix_t *dst = out;
                *(uint32_t*)dst = *(uint32_t*)(pred + 0 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 1 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 2 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 3 * 16);
            } else
            {
                TransformResidual4x4_neon(q->dq, pred, out, out_stride);
            }
            mask <<= 1;
            q++;
            out += 4;
            pred += 4;
        } while (--ccol);
        ccol = side;
        out += 4*(out_stride - ccol);
        pred += 4*(16 - ccol);
    } while (--crow);
}
#endif

#if H264E_ENABLE_PLAIN_C

static uint8_t byteclip_deblock(int x)
{
    if (x > 255)
    {
        return 255;
    }
    if (x < 0)
    {
        return 0;
    }
    return (uint8_t)x;
}

static int clip_range(int range, int src)
{
    if (src > range)
    {
        src = range;
    }
    if (src < -range)
    {
        src = -range;
    }
    return src;
}

static void deblock_chroma(uint8_t *pix, int stride, int alpha, int beta, int thr, int strength)
{
    int p1, p0, q0, q1;
    int delta;

    if (strength == 0)
    {
        return;
    }

    p1 = pix[-2*stride];
    p0 = pix[-1*stride];
    q0 = pix[ 0*stride];
    q1 = pix[ 1*stride];

    if (ABS(p0 - q0) >= alpha || ABS(p1 - p0) >= beta || ABS(q1 - q0) >= beta)
    {
        return;
    }

    if (strength < 4)
    {
        int tC = thr + 1;
        delta = (((q0 - p0)*4) + (p1 - q1) + 4) >> 3;
        delta = clip_range(tC, delta);
        pix[-1*stride] = byteclip_deblock(p0 + delta);
        pix[ 0*stride] = byteclip_deblock(q0 - delta);
    } else
    {
        pix[-1*stride] = (pix_t)((2*p1 + p0 + q1 + 2) >> 2);
        pix[ 0*stride] = (pix_t)((2*q1 + q0 + p1 + 2) >> 2);
    }
}

static void deblock_luma_v(uint8_t *pix, int stride, int alpha, int beta, const uint8_t *pthr, const uint8_t *pstr)
{
    int p2, p1, p0, q0, q1, q2, thr;
    int ap, aq, delta, cloop, i;
    for (i = 0; i < 4; i++)
    {
        cloop = 4;
        if (pstr[i])
        {
            thr = pthr[i];
            do
            {
                p1 = pix[-2];
                p0 = pix[-1];
                q0 = pix[ 0];
                q1 = pix[ 1];

                //if (ABS(p0 - q0) < alpha && ABS(p1 - p0) < beta && ABS(q1 - q0) < beta)
                if (((ABS(p0 - q0) - alpha) & (ABS(p1 - p0) - beta) & (ABS(q1 - q0) - beta)) < 0)
                {
                    int tC = thr;
                    // avoid conditons
                    int sp, sq, d2;
                    p2 = pix[-3];
                    q2 = pix[ 2];
                    ap = ABS(p2 - p0);
                    aq = ABS(q2 - q0);
                    delta = (((q0 - p0)*4) + (p1 - q1) + 4) >> 3;

                    sp = (ap - beta) >> 31;
                    sq = (aq - beta) >> 31;
                    d2 = (((p2 + ((p0 + q0 + 1) >> 1)) >> 1) - p1) & sp;
                    d2 = clip_range(thr, d2);
                    pix[-2] = (pix_t)(p1 + d2);
                    d2 = (((q2 + ((p0 + q0 + 1) >> 1)) >> 1) - q1) & sq;
                    d2 = clip_range(thr, d2);
                    pix[ 1] = (pix_t)(q1 + d2);
                    tC = thr - sp - sq;
                    delta = clip_range(tC, delta);
                    pix[-1] = byteclip_deblock(p0 + delta);
                    pix[ 0] = byteclip_deblock(q0 - delta);
                }
                pix += stride;
            } while (--cloop);
        } else
        {
                pix += 4*stride;
        }
    }
}

static void deblock_luma_h_s4(uint8_t *pix, int stride, int alpha, int beta)
{
    int p3, p2, p1, p0, q0, q1, q2, q3;
    int ap, aq, cloop = 16;
    do
    {
        int abs_p0_q0, abs_p1_p0, abs_q1_q0;
        p1 = pix[-2*stride];
        p0 = pix[-1*stride];
        q0 = pix[ 0*stride];
        q1 = pix[ 1*stride];
        abs_p0_q0 = ABS(p0 - q0);
        abs_p1_p0 = ABS(p1 - p0);
        abs_q1_q0 = ABS(q1 - q0);
        if (abs_p0_q0 < alpha && abs_p1_p0 < beta && abs_q1_q0 < beta)
        {
            int short_p = (2*p1 + p0 + q1 + 2);
            int short_q = (2*q1 + q0 + p1 + 2);

            if (abs_p0_q0 < ((alpha>>2)+2))
            {
                p2 = pix[-3*stride];
                q2 = pix[ 2*stride];
                ap = ABS(p2 - p0);
                aq = ABS(q2 - q0);
                if (ap < beta)
                {
                    int t = p2 + p1 + p0 + q0 + 2;
                    p3 = pix[-4*stride];
                    short_p += t - p1 + q0; //(p2 + 2*p1 + 2*p0 + 2*q0 + q1 + 4) >> 3);
                    short_p >>= 1;
                    pix[-2*stride] = (pix_t)(t >> 2);
                    pix[-3*stride] = (pix_t)((2*p3 + 2*p2 + t + 2) >> 3); //(2*p3 + 3*p2 + p1 + p0 + q0 + 4) >> 3);
                }
                if (aq < beta)
                {
                    int t = q2 + q1 + p0 + q0 + 2;
                    q3 = pix[ 3*stride];
                    short_q += (t - q1 + p0);//(q2 + 2*q1 + 2*q0 + 2*p0 + p1 + 4)>>3);
                    short_q >>= 1;
                    pix[ 1*stride] = (pix_t)(t >> 2);
                    pix[ 2*stride] = (pix_t)((2*q3 + 2*q2 + t + 2) >> 3); //((2*q3 + 3*q2 + q1 + q0 + p0 + 4) >> 3);
                }
            }
            pix[-1*stride] = (pix_t)(short_p >> 2);
            pix[ 0*stride] = (pix_t)(short_q >> 2);
        }
        pix += 1;
    } while (--cloop);
}

static void deblock_luma_v_s4(uint8_t *pix, int stride, int alpha, int beta)
{
    int p3, p2, p1, p0, q0, q1, q2, q3;
    int ap, aq, cloop = 16;
    do
    {
        p2 = pix[-3];
        p1 = pix[-2];
        p0 = pix[-1];
        q0 = pix[ 0];
        q1 = pix[ 1];
        q2 = pix[ 2];
        if (ABS(p0 - q0) < alpha && ABS(p1 - p0) < beta && ABS(q1 - q0) < beta)
        {
            ap = ABS(p2 - p0);
            aq = ABS(q2 - q0);

            if (ap < beta && ABS(p0 - q0) < ((alpha >> 2) + 2))
            {
                p3 = pix[-4];
                pix[-1] = (pix_t)((p2 + 2*p1 + 2*p0 + 2*q0 + q1 + 4) >> 3);
                pix[-2] = (pix_t)((p2 + p1 + p0 + q0 + 2) >> 2);
                pix[-3] = (pix_t)((2*p3 + 3*p2 + p1 + p0 + q0 + 4) >> 3);
            } else
            {
                pix[-1] = (pix_t)((2*p1 + p0 + q1 + 2) >> 2);
            }

            if (aq < beta && ABS(p0 - q0) < ((alpha >> 2) + 2))
            {
                q3 = pix[ 3];
                pix[ 0] = (pix_t)((q2 + 2*q1 + 2*q0 + 2*p0 + p1 + 4) >> 3);
                pix[ 1] = (pix_t)((q2 + q1 + p0 + q0 + 2) >> 2);
                pix[ 2] = (pix_t)((2*q3 + 3*q2 + q1 + q0 + p0 + 4) >> 3);
            } else
            {
                pix[ 0] = (pix_t)((2*q1 + q0 + p1 + 2) >> 2);
            }
        }
        pix += stride;
    } while (--cloop);
}

static void deblock_luma_h(uint8_t *pix, int stride, int alpha, int beta, const uint8_t *pthr, const uint8_t *pstr)
{
    int p2, p1, p0, q0, q1, q2;
    int ap, aq, delta, i;
    for (i = 0; i < 4; i++)
    {
        if (pstr[i])
        {
            int cloop = 4;
            int thr = pthr[i];
            do
            {
                p1 = pix[-2*stride];
                p0 = pix[-1*stride];
                q0 = pix[ 0*stride];
                q1 = pix[ 1*stride];

                //if (ABS(p0-q0) < alpha && ABS(p1-p0) < beta && ABS(q1-q0) < beta)
                if (((ABS(p0-q0) - alpha) & (ABS(p1-p0) - beta) & (ABS(q1-q0) - beta)) < 0)
                {
                    int tC = thr;
                    int sp, sq, d2;
                    p2 = pix[-3*stride];
                    q2 = pix[ 2*stride];
                    ap = ABS(p2 - p0);
                    aq = ABS(q2 - q0);
                    delta = (((q0 - p0)*4) + (p1 - q1) + 4) >> 3;

                    sp = (ap - beta) >> 31;
                    d2 = (((p2 + ((p0 + q0 + 1) >> 1)) >> 1) - p1) & sp;
                    d2 = clip_range(thr, d2);
                    pix[-2*stride] = (pix_t)(p1 + d2);

                    sq = (aq - beta) >> 31;
                    d2 = (((q2 + ((p0 + q0 + 1) >> 1)) >> 1) - q1) & sq;
                    d2 = clip_range(thr, d2);
                    pix[ 1*stride] = (pix_t)(q1 + d2);

                    tC = thr - sp - sq;
                    delta = clip_range(tC, delta);

                    pix[-1*stride] = byteclip_deblock(p0 + delta);
                    pix[ 0*stride] = byteclip_deblock(q0 - delta);
                }
                pix += 1;
            } while (--cloop);
        } else
        {
            pix += 4;
        }
    }
}

static void deblock_chroma_v(uint8_t *pix, int32_t stride, int a, int b, const uint8_t *thr, const uint8_t *str)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        deblock_chroma(pix, 1, a, b, thr[i >> 1], str[i >> 1]);
        pix += stride;
    }
}

static void deblock_chroma_h(uint8_t *pix, int32_t stride, int a, int b, const uint8_t *thr, const uint8_t *str)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        deblock_chroma(pix, stride, a, b, thr[i >> 1], str[i >> 1]);
        pix += 1;
    }
}

static void h264e_deblock_chroma(uint8_t *pix, int32_t stride, const deblock_params_t *par)
{
    const uint8_t *alpha = par->alpha;
    const uint8_t *beta  = par->beta;
    const uint8_t *thr   = par->tc0;
    const uint8_t *strength = (uint8_t *)par->strength32;
    int a,b,x,y;
    a = alpha[0];
    b = beta[0];
    for (x = 0; x < 16; x += 8)
    {
        uint32_t str = *(uint32_t*)&strength[x];
        if (str && a)
        {
            deblock_chroma_v(pix + (x >> 1), stride, a, b, thr + x, strength + x);
        }
        a = alpha[1];
        b = beta[1];
    }
    thr += 16;
    strength += 16;
    a = alpha[2];
    b = beta[2];
    for (y = 0; y < 16; y += 8)
    {
        uint32_t str = *(uint32_t*)&strength[y];
        if (str && a)
        {
            deblock_chroma_h(pix, stride, a, b, thr + y, strength + y);
        }
        pix += 4*stride;
        a = alpha[3];
        b = beta[3];
    }
}

static void h264e_deblock_luma(uint8_t *pix, int32_t stride, const deblock_params_t *par)
{
    const uint8_t *alpha = par->alpha;
    const uint8_t *beta  = par->beta;
    const uint8_t *thr   = par->tc0;
    const uint8_t *strength = (uint8_t *)par->strength32;
    int a = alpha[0];
    int b = beta[0];
    int x, y;
    for (x = 0; x < 16; x += 4)
    {
        uint32_t str = *(uint32_t*)&strength[x];
        if ((uint8_t)str == 4)
        {
            deblock_luma_v_s4(pix + x, stride, a, b);
        } else if (str && a)
        {
            deblock_luma_v(pix + x, stride, a, b, thr + x, strength + x);
        }
        a = alpha[1];
        b = beta[1];
    }
    a = alpha[2];
    b = beta[2];
    thr += 16;
    strength += 16;
    for (y = 0; y < 16; y += 4)
    {
        uint32_t str = *(uint32_t*)&strength[y];
        if ((uint8_t)str == 4)
        {
            deblock_luma_h_s4(pix, stride, a, b);
        } else if (str && a)
        {
            deblock_luma_h(pix, stride, a, b, thr + y, strength + y);
        }
        a = alpha[3];
        b = beta[3];
        pix += 4*stride;
    }
}

static void h264e_denoise_run(unsigned char *frm, unsigned char *frmprev, int w, int h_arg, int stride_frm, int stride_frmprev)
{
    int cloop, h = h_arg;
    if (w <= 2 || h <= 2)
    {
        return;
    }
    w -= 2;
    h -= 2;

    do
    {
        unsigned char *pf = frm += stride_frm;
        unsigned char *pp = frmprev += stride_frmprev;
        cloop = w;
        pp[-stride_frmprev] = *pf++;
        pp++;
        do
        {
            int d, neighbourhood;
            unsigned g, gd, gn, out_val;
            d = pf[0] - pp[0];
            neighbourhood  = pf[-1]      - pp[-1];
            neighbourhood += pf[+1]      - pp[+1];
            neighbourhood += pf[-stride_frm] - pp[-stride_frmprev];
            neighbourhood += pf[+stride_frm] - pp[+stride_frmprev];

            if (d < 0)
            {
                d = -d;
            }
            if (neighbourhood < 0)
            {
                neighbourhood = -neighbourhood;
            }
            neighbourhood >>= 2;

            gd = g_diff_to_gainQ8[d];
            gn = g_diff_to_gainQ8[neighbourhood];

            gn <<= 2;
            if (gn > 255)
            {
                gn = 255;
            }

            gn = 255 - gn;
            gd = 255 - gd;
            g = gn*gd;  // Q8*Q8 = Q16;

            //out_val = ((pp[0]*g ) >> 16) + (((0xffff-g)*pf[0] ) >> 16);
            //out_val = ((pp[0]*g + (1<<15)) >> 16) + (((0xffff-g)*pf[0]  + (1<<15)) >> 16);
            out_val = (pp[0]*g + (0xffff - g)*pf[0]  + (1 << 15)) >> 16;

            assert(out_val <= 255);

            pp[-stride_frmprev] = (unsigned char)out_val;
            //pp[-stride_frmprev] = gd;//(unsigned char)((neighbourhood+1)>255?255:(neighbourhood+1));

            pf++, pp++;
        } while (--cloop);

        pp[-stride_frmprev] = *pf++;
    } while(--h);

    memcpy(frmprev + stride_frmprev, frm + stride_frm, w + 2);
    h = h_arg - 2;
    do
    {
        memcpy(frmprev, frmprev - stride_frmprev, w + 2);
        frmprev -= stride_frmprev;
    } while(--h);
    memcpy(frmprev, frm - stride_frm*(h_arg - 2), w + 2);
}

#undef IS_NULL
#define IS_NULL(p) ((p) < (pix_t *)32)

static uint32_t intra_predict_dc(const pix_t *left, const pix_t *top, int log_side)
{
    unsigned dc = 0, side = 1u << log_side, round = 0;
    do
    {
        if (!IS_NULL(left))
        {
            int cloop = side;
            round += side >> 1;
            do
            {
                dc += *left++;
                dc += *left++;
                dc += *left++;
                dc += *left++;
            } while(cloop -= 4);
        }
        left = top;
        top = NULL;
    } while (left);
    dc += round;
    if (round == side)
        dc >>= 1;
    dc >>= log_side;
    if (!round) dc = 128;
    return dc * 0x01010101;
}

/*
 * Note: To make the code more readable we refer to the neighboring pixels
 * in variables named as below:
 *
 *    UL U0 U1 U2 U3 U4 U5 U6 U7
 *    L0 xx xx xx xx
 *    L1 xx xx xx xx
 *    L2 xx xx xx xx
 *    L3 xx xx xx xx
 */
#define UL edge[-1]
#define U0 edge[0]
#define T1 edge[1]
#define U2 edge[2]
#define U3 edge[3]
#define U4 edge[4]
#define U5 edge[5]
#define U6 edge[6]
#define U7 edge[7]
#define L0 edge[-2]
#define L1 edge[-3]
#define L2 edge[-4]
#define L3 edge[-5]

static void h264e_intra_predict_16x16(pix_t *predict,  const pix_t *left, const pix_t *top, int mode)
{
    int cloop = 16;
    uint32_t *d = (uint32_t*)predict;
    assert(IS_ALIGNED(predict, 4));
    assert(IS_ALIGNED(top, 4));
    if (mode != 1)
    {
        uint32_t t0, t1, t2, t3;
        if (mode < 1)
        {
            t0 = ((uint32_t*)top)[0];
            t1 = ((uint32_t*)top)[1];
            t2 = ((uint32_t*)top)[2];
            t3 = ((uint32_t*)top)[3];
        } else //(mode == 2)
        {
            t0 = t1 = t2 = t3 = intra_predict_dc(left, top, 4);
        }
        do
        {
            *d++ = t0;
            *d++ = t1;
            *d++ = t2;
            *d++ = t3;
        } while (--cloop);
    } else //if (mode == 1)
    {
        do
        {
            uint32_t val = *left++ * 0x01010101u;
            *d++ = val;
            *d++ = val;
            *d++ = val;
            *d++ = val;
        } while (--cloop);
    }
}

static void h264e_intra_predict_chroma(pix_t *predict, const pix_t *left, const pix_t *top, int mode)
{
    int cloop = 8;
    uint32_t *d = (uint32_t*)predict;
    assert(IS_ALIGNED(predict, 4));
    assert(IS_ALIGNED(top, 4));
    if (mode < 1)
    {
        uint32_t t0, t1, t2, t3;
        t0 = ((uint32_t*)top)[0];
        t1 = ((uint32_t*)top)[1];
        t2 = ((uint32_t*)top)[2];
        t3 = ((uint32_t*)top)[3];
        do
        {
            *d++ = t0;
            *d++ = t1;
            *d++ = t2;
            *d++ = t3;
        } while (--cloop);
    } else if (mode == 1)
    {
        do
        {
            uint32_t u = left[0] * 0x01010101u;
            uint32_t v = left[8] * 0x01010101u;
            d[0] = u;
            d[1] = u;
            d[2] = v;
            d[3] = v;
            d += 4;
            left++;
        } while(--cloop);
    } else //if (mode == 2)
    {
        int ccloop = 2;
        cloop = 2;
        do
        {
            d[0] = d[1] = d[16] = intra_predict_dc(left, top, 2);
            d[17] = intra_predict_dc(left + 4, top + 4, 2);
            if (!IS_NULL(top))
            {
                d[1] = intra_predict_dc(NULL, top + 4, 2);
            }
            if (!IS_NULL(left))
            {
                d[16] = intra_predict_dc(NULL, left + 4, 2);
            }
            d += 2;
            left += 8;
            top += 8;
        } while(--cloop);

        do
        {
            cloop = 12;
            do
            {
                *d = d[-4];
                d++;
            } while(--cloop);
            d += 4;
        } while(--ccloop);
    }
}

static int pix_sad_4(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3,
                     uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3)
{
#if defined(__arm__)
    int sad = __usad8(r0, x0);
    sad = __usada8(r1, x1, sad);
    sad = __usada8(r2, x2, sad);
    sad = __usada8(r3, x3, sad);
    return sad;
#else
    int c, sad = 0;
    for (c = 0; c < 4; c++)
    {
        int d = (r0 & 0xff) - (x0 & 0xff); r0 >>= 8; x0 >>= 8;
        sad += ABS(d);
    }
    for (c = 0; c < 4; c++)
    {
        int d = (r1 & 0xff) - (x1 & 0xff); r1 >>= 8; x1 >>= 8;
        sad += ABS(d);
    }
    for (c = 0; c < 4; c++)
    {
        int d = (r2 & 0xff) - (x2 & 0xff); r2 >>= 8; x2 >>= 8;
        sad += ABS(d);
    }
    for (c = 0; c < 4; c++)
    {
        int d = (r3 & 0xff) - (x3 & 0xff); r3 >>= 8; x3 >>= 8;
        sad += ABS(d);
    }
    return sad;
#endif
}

static int h264e_intra_choose_4x4(const pix_t *blockin, pix_t *blockpred, int avail, const pix_t *edge, int mpred, int penalty)
{
    int sad, best_sad, best_m = 2;

    uint32_t r0, r1, r2, r3;
    uint32_t x0, x1, x2, x3, x;

    r0 = ((uint32_t *)blockin)[ 0];
    r1 = ((uint32_t *)blockin)[ 4];
    r2 = ((uint32_t *)blockin)[ 8];
    r3 = ((uint32_t *)blockin)[12];
#undef TEST
#define TEST(mode) sad = pix_sad_4(r0, r1, r2, r3, x0, x1, x2, x3); \
        if (mode != mpred) sad += penalty;    \
        if (sad < best_sad)                   \
        {                                     \
            ((uint32_t *)blockpred)[ 0] = x0; \
            ((uint32_t *)blockpred)[ 4] = x1; \
            ((uint32_t *)blockpred)[ 8] = x2; \
            ((uint32_t *)blockpred)[12] = x3; \
            best_sad = sad;                   \
            best_m = mode;                    \
        }

    // DC
    x0 = x1 = x2 = x3 = intra_predict_dc((avail & AVAIL_L) ? &L3 : 0, (avail & AVAIL_T) ? &U0 : 0, 2);
    best_sad = pix_sad_4(r0, r1, r2, r3, x0, x1, x2, x3);
    if (2 != mpred)
    {
        best_sad += penalty;
    }
    ((uint32_t *)blockpred)[ 0] = x0;
    ((uint32_t *)blockpred)[ 4] = x1;
    ((uint32_t *)blockpred)[ 8] = x2;
    ((uint32_t *)blockpred)[12] = x3;


    if (avail & AVAIL_T)
    {
        uint32_t save = *(uint32_t*)&U4;
        if (!(avail & AVAIL_TR))
        {
            *(uint32_t*)&U4 = U3*0x01010101u;
        }

        x0 = x1 = x2 = x3 = *(uint32_t*)&U0;
        TEST(0)

        x  = ((U6 + 3u*U7      + 2u) >> 2) << 24;
        x |= ((U5 + 2u*U6 + U7 + 2u) >> 2) << 16;
        x |= ((U4 + 2u*U5 + U6 + 2u) >> 2) << 8;
        x |= ((U3 + 2u*U4 + U5 + 2u) >> 2);

        x3 = x;
        x = (x << 8) | ((U2 + 2u*U3 + U4 + 2u) >> 2);
        x2 = x;
        x = (x << 8) | ((T1 + 2u*U2 + U3 + 2u) >> 2);
        x1 = x;
        x = (x << 8) | ((U0 + 2u*T1 + U2 + 2u) >> 2);
        x0 = x;
        TEST(3)

        x3 = x1;
        x1 = x0;

        x  = ((U4 + U5 + 1u) >> 1) << 24;
        x |= ((U3 + U4 + 1u) >> 1) << 16;
        x |= ((U2 + U3 + 1u) >> 1) << 8;
        x |= ((T1 + U2 + 1u) >> 1);
        x2 = x;
        x = (x << 8) | ((U0 + T1 + 1) >> 1);
        x0 = x;
        TEST(7)

        *(uint32_t*)&U4 = save;
    }

    if (avail & AVAIL_L)
    {
        x0 = 0x01010101u * L0;
        x1 = 0x01010101u * L1;
        x2 = 0x01010101u * L2;
        x3 = 0x01010101u * L3;
        TEST(1)

        x = x3;
        x <<= 16;
        x |= ((L2 + 3u*L3 + 2u) >> 2) << 8;
        x |= ((L2 + L3 + 1u) >> 1);
        x2 = x;
        x <<= 16;
        x |= ((L1 + 2u*L2 + L3 + 2u) >> 2) << 8;
        x |= ((L1 + L2 + 1u) >> 1);
        x1 = x;
        x <<= 16;
        x |= ((L0 + 2u*L1 + L2 + 2u) >> 2) << 8;
        x |= ((L0 + L1 + 1u) >> 1);
        x0 = x;
        TEST(8)
    }

    if ((avail & (AVAIL_T | AVAIL_L | AVAIL_TL)) == (AVAIL_T | AVAIL_L | AVAIL_TL))
    {
        uint32_t line0, line3;
        x  = ((U3 + 2u*U2 + T1 + 2u) >> 2) << 24;
        x |= ((U2 + 2u*T1 + U0 + 2u) >> 2) << 16;
        x |= ((T1 + 2u*U0 + UL + 2u) >> 2) << 8;
        x |= ((U0 + 2u*UL + L0 + 2u) >> 2);
        line0 = x;
        x0 = x;
        x = (x << 8) | ((UL + 2u*L0 + L1 + 2u) >> 2);
        x1 = x;
        x = (x << 8) | ((L0 + 2u*L1 + L2 + 2u) >> 2);
        x2 = x;
        x = (x << 8) | ((L1 + 2u*L2 + L3 + 2u) >> 2);
        x3 = x;
        line3 = x;
        TEST(4)

        x = x0 << 8;
        x |= ((UL + L0 + 1u) >> 1);
        x0 = x;
        x <<= 8;
        x |= (line3 >> 16) & 0xff;
        x <<= 8;
        x |= ((L0 + L1 + 1u) >> 1);
        x1 = x;
        x <<= 8;
        x |= (line3 >> 8) & 0xff;
        x <<= 8;
        x |= ((L1 + L2 + 1u) >> 1);
        x2 = x;
        x <<= 8;
        x |= line3 & 0xff;
        x <<= 8;
        x |= ((L2 + L3 + 1u) >> 1);
        x3 = x;
        TEST(6)

        x1 = line0;
        x3 = (x1 << 8) | ((line3 >> 8) & 0xFF);

        x  = ((U2 + U3 + 1u) >> 1) << 24;
        x |= ((T1 + U2 + 1u) >> 1) << 16;
        x |= ((U0 + T1 + 1u) >> 1) << 8;
        x |= ((UL + U0 + 1u) >> 1);
        x0 = x;
        x = (x << 8) | ((line3 >> 16) & 0xFF);
        x2 = x;
        TEST(5)
    }
    return best_m + (best_sad << 4);
}

static uint8_t byteclip(int x)
{
    if (x > 255) x = 255;
    if (x < 0) x = 0;
    return (uint8_t)x;
}

static int hpel_lpf(const uint8_t *p, int s)
{
    return p[0] - 5*p[s] + 20*p[2*s] + 20*p[3*s] - 5*p[4*s] + p[5*s];
}

static void copy_wh(const uint8_t *src, int src_stride, uint8_t *dst, int w, int h)
{
    int x, y;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            dst [x] = src [x];
        }
        dst += 16;
        src += src_stride;
    }
}

static void hpel_lpf_diag(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    ALIGN(16) int16_t scratch[21 * 16] ALIGN2(16);  /* 21 rows by 16 pixels per row */

    /*
     * Intermediate values will be 1/2 pel at Horizontal direction
     * Starting at (0.5, -2) at top extending to (0.5, height + 3) at bottom
     * scratch contains a 2D array of size (w)X(h + 5)
     */
    int y, x;
    for (y = 0; y < h + 5; y++)
    {
        for (x = 0; x < w; x++)
        {
            scratch[y * w + x] = (int16_t)hpel_lpf(src + (y - 2) * src_stride + (x - 2), 1);
        }
    }

    /* Vertical interpolate */
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            int pos = y * w + x;
            int HalfCoeff =
                scratch [pos] -
                5 * scratch [pos + 1 * w] +
                20 * scratch [pos + 2 * w] +
                20 * scratch [pos + 3 * w] -
                5 * scratch [pos + 4 * w] +
                scratch [pos + 5 * w];

            HalfCoeff = byteclip((HalfCoeff + 512) >> 10);

            dst [y * 16 + x] = (uint8_t)HalfCoeff;
        }
    }
}

static void hpel_lpf_hor(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    int x, y;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            dst [y * 16 + x] = byteclip((hpel_lpf(src + y * src_stride + (x - 2), 1) + 16) >> 5);
        }
    }
}

static void hpel_lpf_ver(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, int w, int h)
{
    int y, x;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            dst [y * 16 + x] = byteclip((hpel_lpf(src + (y - 2) * src_stride + x, src_stride) + 16) >> 5);
        }
    }
}

static void average_16x16_unalign(uint8_t *dst, const uint8_t *src1, int src1_stride)
{
    int x, y;
    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            dst[y * 16 + x] = (uint8_t)(((uint32_t)dst [y * 16 + x] + src1[y*src1_stride + x] + 1) >> 1);
        }
    }
}

static void h264e_qpel_average_wh_align(const uint8_t *src0, const uint8_t *src1, uint8_t *dst, point_t wh)
{
    int w = wh.s.x;
    int h = wh.s.y;
    int x, y;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            dst[y * 16 + x] = (uint8_t)((src0[y * 16 + x] + src1[y * 16 + x] + 1) >> 1);
        }
    }
}

static void h264e_qpel_interpolate_luma(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, point_t wh, point_t dxdy)
{
    ALIGN(16) uint8_t scratch[16*16] ALIGN2(16);
    //  src += ((dx + 1) >> 2) + ((dy + 1) >> 2)*src_stride;            // dx == 3 ? next row; dy == 3 ? next line
    //  dxdy              actions: Horizontal, Vertical, Diagonal, Average
    //  0 1 2 3 +1        -   ha    h    ha+
    //  1                 va  hva   hda  hv+a
    //  2                 v   vda   d    v+da
    //  3                 va+ h+va h+da  h+v+a
    //  +stride
    int32_t pos = 1 << (dxdy.s.x + 4*dxdy.s.y);
    int dstused = 0;

    if (pos == 1)
    {
        copy_wh(src, src_stride, dst, wh.s.x, wh.s.y);
        return;
    }
    if (pos & 0xe0ee)// 1110 0000 1110 1110
    {
        hpel_lpf_hor(src + ((pos & 0xe000) ? src_stride : 0), src_stride, dst, wh.s.x, wh.s.y);
        dstused++;
    }
    if (pos & 0xbbb0)// 1011 1011 1011 0000
    {
        hpel_lpf_ver(src + ((pos & 0x8880) ? 1 : 0), src_stride, dstused ? scratch : dst, wh.s.x, wh.s.y);
        dstused++;
    }
    if (pos & 0x4e40)// 0100 1110 0100 0000
    {
        hpel_lpf_diag(src, src_stride, dstused ? scratch : dst, wh.s.x, wh.s.y);
        dstused++;
    }
    if (pos & 0xfafa)// 1111 1010 1111 1010
    {
        assert(wh.s.x == 16 && wh.s.y == 16);
        if (dstused == 2)
        {
            point_t p;

            src = scratch;
            src_stride = 16;
            p.u32 = 16 + (16<<16);

            h264e_qpel_average_wh_align(src, dst, dst, p);
            return;
        } else
        {
            src += ((dxdy.s.x + 1) >> 2) + ((dxdy.s.y + 1) >> 2)*src_stride;
        }
        average_16x16_unalign(dst, src, src_stride);
    }
}

static void h264e_qpel_interpolate_chroma(const uint8_t *src, int src_stride, uint8_t *h264e_restrict dst, point_t wh, point_t dxdy)
{
    /* if fractionl mv is not (0, 0) */
    if (dxdy.u32)
    {
        int a = (8 - dxdy.s.x) * (8 - dxdy.s.y);
        int b = dxdy.s.x * (8 - dxdy.s.y);
        int c = (8 - dxdy.s.x) * dxdy.s.y;
        int d = dxdy.s.x * dxdy.s.y;
        int h = wh.s.y;
        do
        {
            int x;
            for (x = 0; x < wh.s.x; x++)
            {
                dst[x] = (uint8_t)((
                   a * src[             x] + b * src[             x + 1] +
                   c * src[src_stride + x] + d * src[src_stride + x + 1] +
                   32) >> 6);
            }
            dst += 16;
            src += src_stride;
        } while (--h);
    } else
    {
        copy_wh(src, src_stride, dst, wh.s.x, wh.s.y);
    }
}

static int sad_block(const pix_t *a, int a_stride, const pix_t *b, int b_stride, int w, int h)
{
    int r, c, sad = 0;
    for (r = 0; r < h; r++)
    {
        for (c = 0; c < w; c++)
        {
            int d = a[c] - b[c];
            sad += ABS(d);
        }
        a += a_stride;
        b += b_stride;
    }
    return sad;
}

static int h264e_sad_mb_unlaign_8x8(const pix_t *a, int a_stride, const pix_t *b, int sad[4])
{
    sad[0] = sad_block(a,     a_stride, b,     16, 8, 8);
    sad[1] = sad_block(a + 8, a_stride, b + 8, 16, 8, 8);
    a += 8*a_stride;
    b += 8*16;
    sad[2] = sad_block(a,     a_stride, b,     16, 8, 8);
    sad[3] = sad_block(a + 8, a_stride, b + 8, 16, 8, 8);
    return sad[0] + sad[1] + sad[2] + sad[3];
}

static int h264e_sad_mb_unlaign_wh(const pix_t *a, int a_stride, const pix_t *b, point_t wh)
{
    return sad_block(a, a_stride, b, 16, wh.s.x, wh.s.y);
}

static void h264e_copy_8x8(pix_t *d, int d_stride, const pix_t *s)
{
    int cloop = 8;
    assert(IS_ALIGNED(d, 8));
    assert(IS_ALIGNED(s, 8));
    do
    {
        int a = ((const int*)s)[0];
        int b = ((const int*)s)[1];
        ((int*)d)[0] = a;
        ((int*)d)[1] = b;
        s += 16;
        d += d_stride;
    } while(--cloop);
}

static void h264e_copy_16x16(pix_t *d, int d_stride, const pix_t *s, int s_stride)
{
    int cloop = 16;
    assert(IS_ALIGNED(d, 8));
    assert(IS_ALIGNED(s, 8));
    do
    {
        int a = ((const int*)s)[0];
        int b = ((const int*)s)[1];
        int x = ((const int*)s)[2];
        int y = ((const int*)s)[3];
        ((int*)d)[0] = a;
        ((int*)d)[1] = b;
        ((int*)d)[2] = x;
        ((int*)d)[3] = y;
        s += s_stride;
        d += d_stride;
    } while(--cloop);
}
#endif /* H264E_ENABLE_PLAIN_C */

#if H264E_ENABLE_PLAIN_C || (H264E_ENABLE_NEON && !defined(MINIH264_ASM))
static void h264e_copy_borders(unsigned char *pic, int w, int h, int guard)
{
    int r, rowbytes = w + 2*guard;
    unsigned char *d = pic - guard;
    for (r = 0; r < h; r++, d += rowbytes)
    {
        memset(d, d[guard], guard);
        memset(d + rowbytes - guard, d[rowbytes - guard - 1], guard);
    }
    d = pic - guard - guard*rowbytes;
    for (r = 0; r < guard; r++)
    {
        memcpy(d, pic - guard, rowbytes);
        memcpy(d + (guard + h)*rowbytes, pic - guard + (h - 1)*rowbytes, rowbytes);
        d += rowbytes;
    }
}
#endif /* H264E_ENABLE_PLAIN_C || (H264E_ENABLE_NEON && !defined(MINIH264_ASM)) */

#if H264E_ENABLE_PLAIN_C
#undef TRANSPOSE_BLOCK
#define TRANSPOSE_BLOCK     1
#define UNZIGSAG_IN_QUANT   0
#define SUM_DIF(a, b) { int t = a + b; b = a - b; a = t; }

static int clip_byte(int x)
{
    if (x > 255)
    {
        x = 255;
    } else if (x < 0)
    {
        x = 0;
    }
    return x;
}

static void hadamar4_2d(int16_t *x)
{
    int s = 1;
    int sback = 1;
    int16_t tmp[16];
    int16_t *out = tmp;
    int16_t *p = x;
    do
    {
        int cloop = 4;
        do
        {
            int a, b, c, d;
            a = *p; p += 4;//s;
            b = *p; p += 4;//s;
            c = *p; p += 4;//s;
            d = *p; p -= 11;//sback;
            SUM_DIF(a, c);
            SUM_DIF(b, d);
            SUM_DIF(a, b);
            SUM_DIF(c, d);

            *out = (int16_t)a; out += s;
            *out = (int16_t)c; out += s;
            *out = (int16_t)d; out += s;
            *out = (int16_t)b; out += sback;
        } while (--cloop);
        s = 5 - s;
        sback = -11;
        out = x;
        p = tmp;
    } while (s != 1);
}

static void dequant_dc(quant_t *q, int16_t *qval, int dequant, int n)
{
    do q++->dq[0] = (int16_t)(*qval++ * (int16_t)dequant); while (--n);
}

static void quant_dc(int16_t *qval, int16_t *deq, int16_t quant, int n, int round_q18)
{
#if UNZIGSAG_IN_QUANT
    int r_minus =  (1 << 18) - round_q18;
    static const uint8_t iscan16[16] = {0, 1, 5, 6, 2, 4, 7, 12, 3, 8, 11, 13, 9, 10, 14, 15};
    static const uint8_t iscan4[4] = {0, 1, 2, 3};
    const uint8_t *scan = n == 4 ? iscan4 : iscan16;
    do
    {
        int v = *qval;
        int r = v < 0 ? r_minus : round_q18;
        deq[*scan++] = *qval++ = (v * quant + r) >> 18;
    } while (--n);
#else
    int r_minus =  (1<<18) - round_q18;
    do
    {
        int v = *qval;
        int r = v < 0 ? r_minus : round_q18;
        *deq++ = *qval++ = (v * quant + r) >> 18;
    } while (--n);
#endif
}

static void hadamar2_2d(int16_t *x)
{
    int a = x[0];
    int b = x[1];
    int c = x[2];
    int d = x[3];
    x[0] = (int16_t)(a + b + c + d);
    x[1] = (int16_t)(a - b + c - d);
    x[2] = (int16_t)(a + b - c - d);
    x[3] = (int16_t)(a - b - c + d);
}

static void h264e_quant_luma_dc(quant_t *q, int16_t *deq, const uint16_t *qdat)
{
    int16_t *tmp = ((int16_t*)q) - 16;
    hadamar4_2d(tmp);
    quant_dc(tmp, deq, qdat[0], 16, 0x20000);//0x15555);
    hadamar4_2d(tmp);
    assert(!(qdat[1] & 3));
    // dirty trick here: shift w/o rounding, since it have no effect  for qp >= 10 (or, to be precise, for qp => 9)
    dequant_dc(q, tmp, qdat[1] >> 2, 16);
}

static int h264e_quant_chroma_dc(quant_t *q, int16_t *deq, const uint16_t *qdat)
{
    int16_t *tmp = ((int16_t*)q) - 16;
    hadamar2_2d(tmp);
    quant_dc(tmp, deq, (int16_t)(qdat[0] << 1), 4, 0xAAAA);
    hadamar2_2d(tmp);
    assert(!(qdat[1] & 1));
    dequant_dc(q, tmp, qdat[1] >> 1, 4);
    return !!(tmp[0] | tmp[1] | tmp[2] | tmp[3]);
}

static const uint8_t g_idx2quant[16] =
{
    0, 2, 0, 2,
    2, 4, 2, 4,
    0, 2, 0, 2,
    2, 4, 2, 4
};

#define TRANSFORM(x0, x1, x2, x3, p, s) { \
    int t0 = x0 + x3;                     \
    int t1 = x0 - x3;                     \
    int t2 = x1 + x2;                     \
    int t3 = x1 - x2;                     \
    (p)[  0] = (int16_t)(t0 + t2);        \
    (p)[  s] = (int16_t)(t1*2 + t3);      \
    (p)[2*s] = (int16_t)(t0 - t2);        \
    (p)[3*s] = (int16_t)(t1 - t3*2);      \
}

static void FwdTransformResidual4x42(const uint8_t *inp, const uint8_t *pred,
    uint32_t inp_stride, int16_t *out)
{
    int i;
    int16_t tmp[16];

#if TRANSPOSE_BLOCK
    // Transform columns
    for (i = 0; i < 4; i++, pred++, inp++)
    {
        int f0 = inp[0] - pred[0];
        int f1 = inp[1*inp_stride] - pred[1*16];
        int f2 = inp[2*inp_stride] - pred[2*16];
        int f3 = inp[3*inp_stride] - pred[3*16];
        TRANSFORM(f0, f1, f2, f3, tmp + i*4, 1);
    }
    // Transform rows
    for (i = 0; i < 4; i++)
    {
        int d0 = tmp[i + 0];
        int d1 = tmp[i + 4];
        int d2 = tmp[i + 8];
        int d3 = tmp[i + 12];
        TRANSFORM(d0, d1, d2, d3, out + i, 4);
    }

#else
    /* Transform rows */
    for (i = 0; i < 16; i += 4)
    {
        int d0 = inp[0] - pred[0];
        int d1 = inp[1] - pred[1];
        int d2 = inp[2] - pred[2];
        int d3 = inp[3] - pred[3];
        TRANSFORM(d0, d1, d2, d3, tmp + i, 1);
        pred += 16;
        inp += inp_stride;
    }

    /* Transform columns */
    for (i = 0; i < 4; i++)
    {
        int f0 = tmp[i + 0];
        int f1 = tmp[i + 4];
        int f2 = tmp[i + 8];
        int f3 = tmp[i + 12];
        TRANSFORM(f0, f1, f2, f3, out + i, 4);
    }
#endif
}

static void TransformResidual4x4(int16_t *pSrc)
{
    int i;
    int16_t tmp[16];

    /* Transform rows */
    for (i = 0; i < 16; i += 4)
    {
#if TRANSPOSE_BLOCK
        int d0 = pSrc[(i >> 2) + 0];
        int d1 = pSrc[(i >> 2) + 4];
        int d2 = pSrc[(i >> 2) + 8];
        int d3 = pSrc[(i >> 2) + 12];
#else
        int d0 = pSrc[i + 0];
        int d1 = pSrc[i + 1];
        int d2 = pSrc[i + 2];
        int d3 = pSrc[i + 3];
#endif
        int e0 = d0 + d2;
        int e1 = d0 - d2;
        int e2 = (d1 >> 1) - d3;
        int e3 = d1 + (d3 >> 1);
        int f0 = e0 + e3;
        int f1 = e1 + e2;
        int f2 = e1 - e2;
        int f3 = e0 - e3;
        tmp[i + 0] = (int16_t)f0;
        tmp[i + 1] = (int16_t)f1;
        tmp[i + 2] = (int16_t)f2;
        tmp[i + 3] = (int16_t)f3;
    }

    /* Transform columns */
    for (i = 0; i < 4; i++)
    {
        int f0 = tmp[i + 0];
        int f1 = tmp[i + 4];
        int f2 = tmp[i + 8];
        int f3 = tmp[i + 12];
        int g0 = f0 + f2;
        int g1 = f0 - f2;
        int g2 = (f1 >> 1) - f3;
        int g3 = f1 + (f3 >> 1);
        int h0 = g0 + g3;
        int h1 = g1 + g2;
        int h2 = g1 - g2;
        int h3 = g0 - g3;
        pSrc[i + 0] = (int16_t)((h0 + 32) >> 6);
        pSrc[i + 4] = (int16_t)((h1 + 32) >> 6);
        pSrc[i + 8] = (int16_t)((h2 + 32) >> 6);
        pSrc[i + 12] = (int16_t)((h3 + 32) >> 6);
    }
}

static int is_zero(const int16_t *dat, int i0, const uint16_t *thr)
{
    int i;
    for (i = i0; i < 16; i++)
    {
        if ((unsigned)(dat[i] + thr[i & 7]) > (unsigned)2*thr[i & 7])
        {
            return 0;
        }
    }
    return 1;
}

static int is_zero4(const quant_t *q, int i0, const uint16_t *thr)
{
    return is_zero(q[0].dq, i0, thr) &&
           is_zero(q[1].dq, i0, thr) &&
           is_zero(q[4].dq, i0, thr) &&
           is_zero(q[5].dq, i0, thr);
}

static int zero_smallq(quant_t *q, int mode, const uint16_t *qdat)
{
    int zmask = 0;
    int i, i0 = mode & 1, n = mode >> 1;
    if (mode == QDQ_MODE_INTER || mode == QDQ_MODE_CHROMA)
    {
        for (i = 0; i < n*n; i++)
        {
            if (is_zero(q[i].dq, i0, qdat + OFFS_THR_1_OFF))
            {
                zmask |= (1 << i); //9.19
            }
        }
        if (mode == QDQ_MODE_INTER)   //8.27
        {
            if ((~zmask & 0x0033) && is_zero4(q +  0, i0, qdat + OFFS_THR_2_OFF)) zmask |= 0x33;
            if ((~zmask & 0x00CC) && is_zero4(q +  2, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 2);
            if ((~zmask & 0x3300) && is_zero4(q +  8, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 8);
            if ((~zmask & 0xCC00) && is_zero4(q + 10, i0, qdat + OFFS_THR_2_OFF)) zmask |= (0x33 << 10);
        }
    }
    return zmask;
}

static int quantize(quant_t *q, int mode, const uint16_t *qdat, int zmask)
{
#if UNZIGSAG_IN_QUANT
#if TRANSPOSE_BLOCK
    // ; Zig-zag scan      Transposed zig-zag
    // ;    0 1 5 6        0 2 3 9
    // ;    2 4 7 C        1 4 8 A
    // ;    3 8 B D        5 7 B E
    // ;    9 A E F        6 C D F
    static const unsigned char iscan16[16] = { 0, 2, 3, 9, 1, 4, 8, 10, 5, 7, 11, 14, 6, 12, 13, 15 };
#else
    static const unsigned char iscan16[16] = { 0, 1, 5, 6, 2, 4, 7, 12, 3, 8, 11, 13, 9, 10, 14, 15 };
#endif
#endif
    int i, i0 = mode & 1, ccol, crow;
    int nz_block_mask = 0;
    ccol = mode >> 1;
    crow = ccol;
    do
    {
        do
        {
            int nz_mask = 0;

            if (zmask & 1)
            {
                int32_t *p = (int32_t *)q->qv;
                *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
                *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
            } else
            {
                for (i = i0; i < 16; i++)
                {
                    int off = g_idx2quant[i];
                    int v, round = qdat[OFFS_RND_INTER];

                    if (q->dq[i] < 0) round = 0xFFFF - round;

                    v = (q->dq[i]*qdat[off] + round) >> 16;
#if UNZIGSAG_IN_QUANT
                    if (v)
                        nz_mask |= 1 << iscan16[i];
                    q->qv[iscan16[i]] = (int16_t)v;
#else
                    if (v)
                        nz_mask |= 1 << i;
                    q->qv[i] = (int16_t)v;
#endif
                    q->dq[i] = (int16_t)(v*qdat[off + 1]);
                }
            }

            zmask >>= 1;
            nz_block_mask <<= 1;
            if (nz_mask)
                nz_block_mask |= 1;
            q++;
        } while (--ccol);
        ccol = mode >> 1;
    } while (--crow);
    return nz_block_mask;
}

static void transform(const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q)
{
    int crow = mode >> 1;
    int ccol = crow;

    do
    {
        do
        {
            FwdTransformResidual4x42(inp, pred, inp_stride, q->dq);
            q++;
            inp += 4;
            pred += 4;
        } while (--ccol);
        ccol = mode >> 1;
        inp += 4*(inp_stride - ccol);
        pred += 4*(16 - ccol);
    } while (--crow);
}

static int h264e_transform_sub_quant_dequant(const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q, const uint16_t *qdat)
{
    int zmask;
    transform(inp, pred, inp_stride, mode, q);
    if (mode & 1) // QDQ_MODE_INTRA_16 || QDQ_MODE_CHROMA
    {
        int cloop = (mode >> 1)*(mode >> 1);
        short *dc = ((short *)q) - 16;
        quant_t *pq = q;
        do
        {
            *dc++ = pq->dq[0];
            pq++;
        } while (--cloop);
    }
    zmask = zero_smallq(q, mode, qdat);
    return quantize(q, mode, qdat, zmask);
}

static void h264e_transform_add(pix_t *out, int out_stride, const pix_t *pred, quant_t *q, int side, int32_t mask)
{
    int crow = side;
    int ccol = crow;

    assert(IS_ALIGNED(out, 4));
    assert(IS_ALIGNED(pred, 4));
    assert(!(out_stride % 4));

    do
    {
        do
        {
            if (mask >= 0)
            {
                // copy 4x4
                pix_t *dst = out;
                *(uint32_t*)dst = *(uint32_t*)(pred + 0 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 1 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 2 * 16); dst += out_stride;
                *(uint32_t*)dst = *(uint32_t*)(pred + 3 * 16);
            } else
            {
                int i, j;
                TransformResidual4x4(q->dq);
                for (j = 0; j < 4; j++)
                {
                    for (i = 0; i < 4; i++)
                    {
                        int Value = q->dq[j * 4 + i] + pred[j * 16 + i];
                        out[j * out_stride + i] = (pix_t)clip_byte(Value);
                    }
                }
            }
            mask = (uint32_t)mask << 1;
            q++;
            out += 4;
            pred += 4;
        } while (--ccol);
        ccol = side;
        out += 4*(out_stride - ccol);
        pred += 4*(16 - ccol);
    } while (--crow);
}
#endif /* H264E_ENABLE_PLAIN_C */

#if H264E_ENABLE_PLAIN_C || (H264E_ENABLE_NEON && !defined(MINIH264_ASM))

#define BS_BITS 32

static void h264e_bs_put_bits(bs_t *bs, unsigned n, unsigned val)
{
    assert(!(val >> n));
    bs->shift -= n;
    assert((unsigned)n <= 32);
    if (bs->shift < 0)
    {
        assert(-bs->shift < 32);
        bs->cache |= val >> -bs->shift;
        *bs->buf++ = SWAP32(bs->cache);
        bs->shift = 32 + bs->shift;
        bs->cache = 0;
    }
    bs->cache |= val << bs->shift;
}

static void h264e_bs_flush(bs_t *bs)
{
    *bs->buf = SWAP32(bs->cache);
}

static unsigned h264e_bs_get_pos_bits(const bs_t *bs)
{
    unsigned pos_bits = (unsigned)((bs->buf - bs->origin)*BS_BITS);
    pos_bits += BS_BITS - bs->shift;
    assert((int)pos_bits >= 0);
    return pos_bits;
}

static unsigned h264e_bs_byte_align(bs_t *bs)
{
    int pos = h264e_bs_get_pos_bits(bs);
    h264e_bs_put_bits(bs, -pos & 7, 0);
    return pos + (-pos & 7);
}

/**
*   Golomb code
*   0 => 1
*   1 => 01 0
*   2 => 01 1
*   3 => 001 00
*   4 => 001 01
*
*   [0]     => 1
*   [1..2]  => 01x
*   [3..6]  => 001xx
*   [7..14] => 0001xxx
*
*/
static void h264e_bs_put_golomb(bs_t *bs, unsigned val)
{
#ifdef __arm__
    int size = 32 - __clz(val + 1);
#else
    int size = 0;
    unsigned t = val + 1;
    do
    {
        size++;
    } while (t >>= 1);
#endif
    h264e_bs_put_bits(bs, 2*size - 1, val + 1);
}

/**
*   signed Golomb code.
*   mapping to unsigned code:
*       0 => 0
*       1 => 1
*      -1 => 2
*       2 => 3
*      -2 => 4
*       3 => 5
*      -3 => 6
*/
static void h264e_bs_put_sgolomb(bs_t *bs, int val)
{
    val = 2*val - 1;
    val ^= val >> 31;
    h264e_bs_put_golomb(bs, val);
}

static void h264e_bs_init_bits(bs_t *bs, void *data)
{
    bs->origin = data;
    bs->buf = bs->origin;
    bs->shift = BS_BITS;
    bs->cache = 0;
}

static void h264e_vlc_encode(bs_t *bs, int16_t *quant, int maxNumCoeff, uint8_t *nz_ctx)
{
    int nnz_context, nlevels, nnz; // nnz = nlevels + trailing_ones
    int trailing_ones = 0;
    int trailing_ones_sign = 0;
    uint8_t runs[16];
    uint8_t *prun = runs;
    int16_t *levels;
    int cloop = maxNumCoeff;
    BS_OPEN(bs)

#if H264E_ENABLE_SSE2 || (H264E_ENABLE_PLAIN_C && !H264E_ENABLE_NEON)
    // this branch used with SSE + C configuration
    int16_t zzquant[16];
    levels = zzquant + ((maxNumCoeff == 4) ? 4 : 16);
    if (maxNumCoeff != 4)
    {
        int v;
        if (maxNumCoeff == 16)
        {
            v = quant[15]*2; if (v) *--levels = (int16_t)v, *prun++ = 16;
            v = quant[11]*2; if (v) *--levels = (int16_t)v, *prun++ = 15;
            v = quant[14]*2; if (v) *--levels = (int16_t)v, *prun++ = 14;
            v = quant[13]*2; if (v) *--levels = (int16_t)v, *prun++ = 13;
            v = quant[10]*2; if (v) *--levels = (int16_t)v, *prun++ = 12;
            v = quant[ 7]*2; if (v) *--levels = (int16_t)v, *prun++ = 11;
            v = quant[ 3]*2; if (v) *--levels = (int16_t)v, *prun++ = 10;
            v = quant[ 6]*2; if (v) *--levels = (int16_t)v, *prun++ =  9;
            v = quant[ 9]*2; if (v) *--levels = (int16_t)v, *prun++ =  8;
            v = quant[12]*2; if (v) *--levels = (int16_t)v, *prun++ =  7;
            v = quant[ 8]*2; if (v) *--levels = (int16_t)v, *prun++ =  6;
            v = quant[ 5]*2; if (v) *--levels = (int16_t)v, *prun++ =  5;
            v = quant[ 2]*2; if (v) *--levels = (int16_t)v, *prun++ =  4;
            v = quant[ 1]*2; if (v) *--levels = (int16_t)v, *prun++ =  3;
            v = quant[ 4]*2; if (v) *--levels = (int16_t)v, *prun++ =  2;
            v = quant[ 0]*2; if (v) *--levels = (int16_t)v, *prun++ =  1;
        } else
        {
            v = quant[15]*2; if (v) *--levels = (int16_t)v, *prun++ = 15;
            v = quant[11]*2; if (v) *--levels = (int16_t)v, *prun++ = 14;
            v = quant[14]*2; if (v) *--levels = (int16_t)v, *prun++ = 13;
            v = quant[13]*2; if (v) *--levels = (int16_t)v, *prun++ = 12;
            v = quant[10]*2; if (v) *--levels = (int16_t)v, *prun++ = 11;
            v = quant[ 7]*2; if (v) *--levels = (int16_t)v, *prun++ = 10;
            v = quant[ 3]*2; if (v) *--levels = (int16_t)v, *prun++ =  9;
            v = quant[ 6]*2; if (v) *--levels = (int16_t)v, *prun++ =  8;
            v = quant[ 9]*2; if (v) *--levels = (int16_t)v, *prun++ =  7;
            v = quant[12]*2; if (v) *--levels = (int16_t)v, *prun++ =  6;
            v = quant[ 8]*2; if (v) *--levels = (int16_t)v, *prun++ =  5;
            v = quant[ 5]*2; if (v) *--levels = (int16_t)v, *prun++ =  4;
            v = quant[ 2]*2; if (v) *--levels = (int16_t)v, *prun++ =  3;
            v = quant[ 1]*2; if (v) *--levels = (int16_t)v, *prun++ =  2;
            v = quant[ 4]*2; if (v) *--levels = (int16_t)v, *prun++ =  1;
        }
    } else
    {
        int v;
        v = quant[ 3]*2; if (v) *--levels = (int16_t)v, *prun++ = 4;
        v = quant[ 2]*2; if (v) *--levels = (int16_t)v, *prun++ = 3;
        v = quant[ 1]*2; if (v) *--levels = (int16_t)v, *prun++ = 2;
        v = quant[ 0]*2; if (v) *--levels = (int16_t)v, *prun++ = 1;
    }
    quant = zzquant + ((maxNumCoeff == 4) ? 4 : 16);
    nnz = (int)(quant - levels);
#else
    quant += (maxNumCoeff == 4) ? 4 : 16;
    levels = quant;
    do
    {
        int v = *--quant;
        if (v)
        {
            *--levels = v*2;
            *prun++ = cloop;
        }
    } while (--cloop);
    quant += maxNumCoeff;
    nnz = quant - levels;
#endif

    if (nnz)
    {
        cloop = MIN(3, nnz);
        levels = quant - 1;
        do
        {
            if ((unsigned)(*levels + 2) > 4u)
            {
                break;
            }
            trailing_ones_sign = (trailing_ones_sign << 1) | (*levels-- < 0);
            trailing_ones++;
        } while (--cloop);
    }
    nlevels = nnz - trailing_ones;

    nnz_context = nz_ctx[-1] + nz_ctx[1];

    nz_ctx[0] = (uint8_t)nnz;
    if (nnz_context <= 34)
    {
        nnz_context = (nnz_context + 1) >> 1;
    }
    nnz_context &= 31;

    // 9.2.1 Parsing process for total number of transform coefficient levels and trailing ones
    {
        int off = h264e_g_coeff_token[nnz_context];
        int n = 6, val = h264e_g_coeff_token[off + trailing_ones + 4*nlevels];
        if (off != 230)
        {
            n = (val & 15) + 1;
            val >>= 4;
        }
        BS_PUT(n, val);
    }

    if (nnz)
    {
        if (trailing_ones)
        {
            BS_PUT(trailing_ones, trailing_ones_sign);
        }
        if (nlevels)
        {
            int vlcnum = 1;
            int sym_len, prefix_len;

            int sym = *levels-- - 2;
            if (sym < 0) sym = -3 - sym;
            if (sym >= 6) vlcnum++;
            if (trailing_ones < 3)
            {
                sym -= 2;
                if (nnz > 10)
                {
                    sym_len = 1;
                    prefix_len = sym >> 1;
                    if (prefix_len >= 15)
                    {
                        // or vlcnum = 1;  goto escape;
                        prefix_len = 15;
                        sym_len = 12;
                    }
                    sym -= prefix_len << 1;
                    // bypass vlcnum advance due to sym -= 2; above
                    goto loop_enter;
                }
            }

            if (sym < 14)
            {
                prefix_len = sym;
                sym = 0; // to avoid side effect in bitbuf
                sym_len = 0;
            } else if (sym < 30)
            {
                prefix_len = 14;
                sym_len = 4;
                sym -= 14;
            } else
            {
                vlcnum = 1;
                goto escape;
            }
            goto loop_enter;

            for (;;)
            {
                sym_len = vlcnum;
                prefix_len = sym >> vlcnum;
                if (prefix_len >= 15)
                {
escape:
                    prefix_len = 15;
                    sym_len = 12;
                }
                sym -= prefix_len << vlcnum;

                if (prefix_len >= 3 && vlcnum < 6)
                    vlcnum++;
loop_enter:
                sym |= 1 << sym_len;
                sym_len += prefix_len + 1;
                BS_PUT(sym_len, sym);
                if (!--nlevels) break;
                sym = *levels-- - 2;
                if (sym < 0) sym = -3 - sym;
            }
        }

        if (nnz < maxNumCoeff)
        {
            const uint8_t *vlc = (maxNumCoeff == 4) ? h264e_g_total_zeros_cr_2x2 : h264e_g_total_zeros;
            uint8_t *run = runs;
            int run_prev = *run++;
            int nzeros = run_prev - nnz;
            int zeros_left = 2*nzeros - 1;
            int ctx = nnz - 1;
            run[nnz - 1] = (uint8_t)maxNumCoeff; // terminator
            for (;;)
            {
                int t;

                int val = vlc[vlc[ctx] + nzeros];
                int n = val & 15;
                val >>= 4;
                BS_PUT(n, val);

                zeros_left -= nzeros;
                if (zeros_left < 0)
                {
                    break;
                }

                t = *run++;
                nzeros = run_prev - t - 1;
                if (nzeros < 0)
                {
                    break;
                }
                run_prev = t;
                assert(zeros_left < 14);
                vlc = h264e_g_run_before;
                ctx = zeros_left;
            }
        }
    }
    BS_CLOSE(bs);
}
#endif /* H264E_ENABLE_PLAIN_C || (H264E_ENABLE_NEON && !defined(MINIH264_ASM)) */

#if H264E_SVC_API
static uint32_t udiv32(uint32_t n, uint32_t d)
{
    uint32_t q = 0, r = n, N = 16;
    do
    {
        N--;
        if ((r >> N) >= d)
        {
            r -= (d << N);
            q += (1 << N);
        }
    } while (N);
    return q;
}

static void h264e_copy_8x8_s(pix_t *d, int d_stride, const pix_t *s, int s_stride)
{
    int cloop = 8;
    assert(!((unsigned)(uintptr_t)d & 7));
    assert(!((unsigned)(uintptr_t)s & 7));
    do
    {
        int a = ((const int*)s)[0];
        int b = ((const int*)s)[1];
        ((int*)d)[0] = a;
        ((int*)d)[1] = b;
        s += s_stride;
        d += d_stride;
    } while(--cloop);
}

static void h264e_frame_downsampling(uint8_t *out, int wo, int ho,
    const uint8_t *src, int wi, int hi, int wo_Crop, int ho_Crop, int wi_Crop, int hi_Crop)
{
#define Q_BILIN 12
#define ONE_BILIN (1<<Q_BILIN)
    int r, c;
    int scaleh = udiv32(hi_Crop<<Q_BILIN, ho_Crop);
    int scalew = udiv32(wi_Crop<<Q_BILIN, wo_Crop);

    for (r = 0; r < ho_Crop; r++)
    {
        int dy = r*scaleh + (scaleh >> 2);
        int y = dy >> Q_BILIN;
        dy = dy & (ONE_BILIN - 1);

        for (c = 0; c < wo_Crop; c++)
        {
            int dx = c*scalew + (scalew >> 2);
            //          int dx = c*scalew;
            int x = dx >> Q_BILIN;
            const uint8_t *s0, *s1;
            uint8_t s00, s01, s10, s11;
            dx &= (ONE_BILIN - 1);


            s1 = s0 = src + x + y*wi;
            if (y < hi - 1)
            {
                s1 = s0 + wi;
            }

            s00 = s01 = s0[0];
            s10 = s11 = s1[0];
            if (x < wi - 1)
            {
                s01 = s0[1];
                s11 = s1[1];
            }

            *out++ =(uint8_t) ((((s11*dx + s10*(ONE_BILIN - dx)) >> (Q_BILIN - 1))*dy +
                ((s01*dx + s00*(ONE_BILIN - dx)) >> (Q_BILIN - 1))*(ONE_BILIN - dy) + (1 << (Q_BILIN + 1 - 1))) >> (Q_BILIN + 1));
        }
        if (wo > wo_Crop) //copy border
        {
            int cloop = wo - wo_Crop;
            uint8_t border = out[-1];
            do
            {
                *out++ = border;
            } while(--cloop);
        }
    }

    // copy bottom
    {
        int cloop = (ho - ho_Crop) * wo;
        if (cloop > 0)
        {
            do
            {
                *out = out[-wo];
                out++;
            } while(--cloop);
        }
    }
}

static int clip(int val, int max)
{
    if (val < 0) return 0;
    if (val > max) return max;
    return val;
}

static const int8_t g_filter16_luma[16][4] =
{
    {  0, 32,  0,  0 },
    { -1, 32,  2, -1 },
    { -2, 31,  4, -1 },
    { -3, 30,  6, -1 },
    { -3, 28,  8, -1 },
    { -4, 26, 11, -1 },
    { -4, 24, 14, -2 },
    { -3, 22, 16, -3 },
    { -3, 19, 19, -3 },
    { -3, 16, 22, -3 },
    { -2, 14, 24, -4 },
    { -1, 11, 26, -4 },
    { -1,  8, 28, -3 },
    { -1,  6, 30, -3 },
    { -1,  4, 31, -2 },
    { -1,  2, 32, -1 }
};

static void h264e_intra_upsampling(int srcw, int srch, int dstw, int dsth, int is_chroma,
    const uint8_t *arg_src, int src_stride, uint8_t *arg_dst, int dst_stride)
{
    int i, j;
    //===== set position calculation parameters =====
    int shift_x = 16;//(m_iLevelIdc <= 30 ? 16 : 31 - CeilLog2(iBaseW));
    int shift_y = 16;//(m_iLevelIdc <= 30 ? 16 : 31 - CeilLog2(iBaseH));
    int step_x  = udiv32(((unsigned int)srcw << shift_x) + (dstw >> 1), dstw);
    int step_y  = udiv32(((unsigned int)srch << shift_y) + (dsth >> 1), dsth);
    int start_x = udiv32((srcw << (shift_x - 1 - is_chroma)) + (dstw >> 1), dstw) + (1 << (shift_x - 5));
    int start_y = udiv32((srch << (shift_y - 1 - is_chroma)) + (dsth >> 1), dsth) + (1 << (shift_y - 5));
    int16_t *temp16 = (short*)(arg_dst + dst_stride*dsth) + 4;  // malloc(( iBaseH )*sizeof(short)); //ref frame have border =1 mb

    if (is_chroma)
    {
        int xpos = start_x - (4 << 12);
        for (i = 0; i < dstw; i++, xpos += step_x)
        {
            const uint8_t* src = arg_src;
            int xfrac  = (xpos >> 12) & 15;
            int xint = xpos >> 16;
            int m0 = clip(xint + 0, srcw - 1);
            int m1 = clip(xint + 1, srcw - 1);
            for( j = 0; j < srch ; j++ )
            {
                temp16[j] = (int16_t)(src[m1]*xfrac + src[m0]*(16 - xfrac));
                src += src_stride;
            }
            temp16[-1] = temp16[0];
            temp16[srch] = temp16[srch-1];

            //========== vertical upsampling ===========
            {
                int16_t* src16 = temp16;
                uint8_t* dst = arg_dst + i;
                int ypos = start_y - (4 << 12);
                for (j = 0; j < dsth; j++)
                {
                    int yfrac = (ypos >> 12) & 15;
                    int yint  = (ypos >> 16);
                    int acc = yfrac*src16[yint + 1] + (16 - yfrac)*src16[yint + 0];
                    acc = (acc + 128) >> 8;
                    *dst = (int8_t)acc;
                    dst += dst_stride;
                    ypos += step_y;
                }
            }
        }
    } else
    {
        int xpos = start_x - (8 << 12);
        for (i = 0; i < dstw; i++, xpos += step_x)
        {
            const uint8_t *src = arg_src;
            int xfrac    = (xpos >> 12) & 15;
            int xint   = xpos >> 16;
            int m0 = clip(xint - 1, srcw - 1);
            int m1 = clip(xint    , srcw - 1);
            int m2 = clip(xint + 1, srcw - 1);
            int m3 = clip(xint + 2, srcw - 1);
            //========== horizontal upsampling ===========
            for( j = 0; j < srch ; j++ )
            {
                int acc = 0;
                acc += g_filter16_luma[xfrac][0] * src[m0];
                acc += g_filter16_luma[xfrac][1] * src[m1];
                acc += g_filter16_luma[xfrac][2] * src[m2];
                acc += g_filter16_luma[xfrac][3] * src[m3];
                temp16[j] = (int16_t)acc;
                src += src_stride;
            }
            temp16[-2] = temp16[-1] = temp16[0];
            temp16[srch + 1] = temp16[srch] = temp16[srch - 1];

            //========== vertical upsampling ===========
            {
                int16_t *src16 = temp16;
                uint8_t *dst = arg_dst + i;
                int ypos = start_y - (8 << 12);

                for (j = 0; j < dsth; j++)
                {
                    int yfrac = (ypos >> 12) & 15;
                    int yint = ypos >> 16;
                    int acc = 512;
                    acc += g_filter16_luma[yfrac][0] * src16[yint + 0 - 1];
                    acc += g_filter16_luma[yfrac][1] * src16[yint + 1 - 1];
                    acc += g_filter16_luma[yfrac][2] * src16[yint + 2 - 1];
                    acc += g_filter16_luma[yfrac][3] * src16[yint + 3 - 1];
                    acc >>= 10;
                    if (acc < 0)
                    {
                        acc = 0;
                    }
                    if (acc > 255)
                    {
                        acc = 255;
                    }
                    *dst = (int8_t)acc;
                    dst += dst_stride;
                    ypos += step_y;
                }
            }
        }
    }
}
#endif /* H264E_SVC_API */

// Experimental code branch:
// Rate-control takes into account that long-term references compresses worser than short-term
#define H264E_RATE_CONTROL_GOLDEN_FRAMES 1

/************************************************************************/
/*      Constants (can't be changed)                                    */
/************************************************************************/

#define MIN_QP          10   // Minimum QP

#define MVPRED_MEDIAN   1
#define MVPRED_L        2
#define MVPRED_U        3
#define MVPRED_UR       4
#define MV_NA           0x8000
#define AVAIL(mv)       ((mv).u32 != MV_NA)

#define SLICE_TYPE_P    0
#define SLICE_TYPE_I    2

#define NNZ_NA          64

#define MAX_MV_CAND     20

#define STARTCODE_4BYTES 4

#define SCALABLE_BASELINE 83

/************************************************************************/
/*      Hardcoded params (can be changed at compile time)               */
/************************************************************************/
#define ALPHA_OFS       0       // Deblock alpha offset
#define BETA_OFS        0       // Deblock beta offset
#define DQP_CHROMA      0       // chroma delta QP

#define MV_RANGE        32      // Motion vector search range, pixels
#define MV_GUARD        14      // Out-of-frame MV's restriction, pixels

/************************************************************************/
/*      Code shortcuts                                                  */
/************************************************************************/
#define U(n,v) h264e_bs_put_bits(enc->bs, n, v)
#define U1(v)  h264e_bs_put_bits(enc->bs, 1, v)
#define UE(v)  h264e_bs_put_golomb(enc->bs, v)
#define SE(v)  h264e_bs_put_sgolomb(enc->bs, v)
#define SWAP(datatype, a, b) { datatype _ = a; a = b; b = _; }
#define SQR(x) ((x)*(x))
#define SQRP(pnt) SQR(pnt.s.x) + SQR(pnt.s.y)
#define SMOOTH(smth, p) smth.s.x = (63*smth.s.x + p.s.x + 32) >> 6;  smth.s.y = (63*smth.s.y + p.s.y + 32) >> 6;
#define MUL_LAMBDA(x, lambda) ((x)*(lambda) >> 4)

/************************************************************************/
/*      Optimized code fallback                                         */
/************************************************************************/

#if defined(MINIH264_ASM)
#include "asm/minih264e_asm.h"
#endif
#if H264E_ENABLE_NEON && defined(MINIH264_ASM)
#   define h264e_bs_put_bits_neon      h264e_bs_put_bits_arm11
#   define h264e_bs_flush_neon         h264e_bs_flush_arm11
#   define h264e_bs_get_pos_bits_neon  h264e_bs_get_pos_bits_arm11
#   define h264e_bs_byte_align_neon    h264e_bs_byte_align_arm11
#   define h264e_bs_put_golomb_neon    h264e_bs_put_golomb_arm11
#   define h264e_bs_put_sgolomb_neon   h264e_bs_put_sgolomb_arm11
#   define h264e_bs_init_bits_neon     h264e_bs_init_bits_arm11
#   define h264e_vlc_encode_neon       h264e_vlc_encode_arm11
#elif H264E_ENABLE_NEON
#   define h264e_bs_put_bits_neon      h264e_bs_put_bits
#   define h264e_bs_flush_neon         h264e_bs_flush
#   define h264e_bs_get_pos_bits_neon  h264e_bs_get_pos_bits
#   define h264e_bs_byte_align_neon    h264e_bs_byte_align
#   define h264e_bs_put_golomb_neon    h264e_bs_put_golomb
#   define h264e_bs_put_sgolomb_neon   h264e_bs_put_sgolomb
#   define h264e_bs_init_bits_neon     h264e_bs_init_bits
#   define h264e_vlc_encode_neon       h264e_vlc_encode
#   define h264e_copy_borders_neon     h264e_copy_borders
#endif

/************************************************************************/
/*      Declare exported functions for each configuration               */
/************************************************************************/
#if !H264E_CONFIGS_COUNT
#   error no build configuration defined
#elif H264E_CONFIGS_COUNT == 1
//  Exactly one configuration: append config suffix to exported names
#   if H264E_ENABLE_NEON
#       define MAP_NAME(name) name##_neon
#   endif
#   if H264E_ENABLE_SSE2
#       define MAP_NAME(name) name##_sse2
#   endif
#else //if H264E_CONFIGS_COUNT > 1
//  Several configurations: use Virtual Functions Table (VFT)
typedef struct
{
#   define  H264E_API(type, name, args) type (*name) args;
// h264e_qpel
H264E_API(void, h264e_qpel_interpolate_chroma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_interpolate_luma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_average_wh_align, (const uint8_t *p0, const uint8_t *p1, uint8_t *h264e_restrict d, point_t wh))
// h264e_deblock
H264E_API(void, h264e_deblock_chroma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
H264E_API(void, h264e_deblock_luma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
// h264e_intra
H264E_API(void, h264e_intra_predict_chroma,  (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(void, h264e_intra_predict_16x16, (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(int,  h264e_intra_choose_4x4, (const pix_t *blockin, pix_t *blockpred, int avail, const pix_t *edge, int mpred, int penalty))
// h264e_cavlc
H264E_API(void,     h264e_bs_put_bits, (bs_t *bs, unsigned n, unsigned val))
H264E_API(void,     h264e_bs_flush, (bs_t *bs))
H264E_API(unsigned, h264e_bs_get_pos_bits, (const bs_t *bs))
H264E_API(unsigned, h264e_bs_byte_align, (bs_t *bs))
H264E_API(void,     h264e_bs_put_golomb, (bs_t *bs, unsigned val))
H264E_API(void,     h264e_bs_put_sgolomb, (bs_t *bs, int val))
H264E_API(void,     h264e_bs_init_bits, (bs_t *bs, void *data))
H264E_API(void,     h264e_vlc_encode, (bs_t *bs, int16_t *quant, int maxNumCoeff, uint8_t *nz_ctx))
// h264e_sad
H264E_API(int,  h264e_sad_mb_unlaign_8x8, (const pix_t *a, int a_stride, const pix_t *b, int sad[4]))
H264E_API(int,  h264e_sad_mb_unlaign_wh, (const pix_t *a, int a_stride, const pix_t *b, point_t wh))
H264E_API(void, h264e_copy_8x8, (pix_t *d, int d_stride, const pix_t *s))
H264E_API(void, h264e_copy_16x16, (pix_t *d, int d_stride, const pix_t *s, int s_stride))
H264E_API(void, h264e_copy_borders, (unsigned char *pic, int w, int h, int guard))
// h264e_transform
H264E_API(void, h264e_transform_add, (pix_t *out, int out_stride, const pix_t *pred, quant_t *q, int side, int32_t mask))
H264E_API(int,  h264e_transform_sub_quant_dequant, (const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q, const uint16_t *qdat))
H264E_API(void, h264e_quant_luma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
H264E_API(int,  h264e_quant_chroma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
// h264e_denoise
H264E_API(void, h264e_denoise_run, (unsigned char *frm, unsigned char *frmprev, int w, int h, int stride_frm, int stride_frmprev))
#   undef H264E_API
} vft_t;

// non-const VFT, run-time initialized
static const vft_t *g_vft;

// const VFT for each supported build config
#if H264E_ENABLE_PLAIN_C
static const vft_t g_vft_plain_c =
{
#define  H264E_API(type, name, args) name,
// h264e_qpel
H264E_API(void, h264e_qpel_interpolate_chroma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_interpolate_luma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_average_wh_align, (const uint8_t *p0, const uint8_t *p1, uint8_t *h264e_restrict d, point_t wh))
// h264e_deblock
H264E_API(void, h264e_deblock_chroma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
H264E_API(void, h264e_deblock_luma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
// h264e_intra
H264E_API(void, h264e_intra_predict_chroma,  (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(void, h264e_intra_predict_16x16, (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(int,  h264e_intra_choose_4x4, (const pix_t *blockin, pix_t *blockpred, int avail, const pix_t *edge, int mpred, int penalty))
// h264e_cavlc
H264E_API(void,     h264e_bs_put_bits, (bs_t *bs, unsigned n, unsigned val))
H264E_API(void,     h264e_bs_flush, (bs_t *bs))
H264E_API(unsigned, h264e_bs_get_pos_bits, (const bs_t *bs))
H264E_API(unsigned, h264e_bs_byte_align, (bs_t *bs))
H264E_API(void,     h264e_bs_put_golomb, (bs_t *bs, unsigned val))
H264E_API(void,     h264e_bs_put_sgolomb, (bs_t *bs, int val))
H264E_API(void,     h264e_bs_init_bits, (bs_t *bs, void *data))
H264E_API(void,     h264e_vlc_encode, (bs_t *bs, int16_t *quant, int maxNumCoeff, uint8_t *nz_ctx))
// h264e_sad
H264E_API(int,  h264e_sad_mb_unlaign_8x8, (const pix_t *a, int a_stride, const pix_t *b, int sad[4]))
H264E_API(int,  h264e_sad_mb_unlaign_wh, (const pix_t *a, int a_stride, const pix_t *b, point_t wh))
H264E_API(void, h264e_copy_8x8, (pix_t *d, int d_stride, const pix_t *s))
H264E_API(void, h264e_copy_16x16, (pix_t *d, int d_stride, const pix_t *s, int s_stride))
H264E_API(void, h264e_copy_borders, (unsigned char *pic, int w, int h, int guard))
// h264e_transform
H264E_API(void, h264e_transform_add, (pix_t *out, int out_stride, const pix_t *pred, quant_t *q, int side, int32_t mask))
H264E_API(int,  h264e_transform_sub_quant_dequant, (const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q, const uint16_t *qdat))
H264E_API(void, h264e_quant_luma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
H264E_API(int,  h264e_quant_chroma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
// h264e_denoise
H264E_API(void, h264e_denoise_run, (unsigned char *frm, unsigned char *frmprev, int w, int h, int stride_frm, int stride_frmprev))
#undef H264E_API
};
#endif
#if H264E_ENABLE_NEON
static const vft_t g_vft_neon =
{
#define  H264E_API(type, name, args) name##_neon,
// h264e_qpel
H264E_API(void, h264e_qpel_interpolate_chroma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_interpolate_luma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_average_wh_align, (const uint8_t *p0, const uint8_t *p1, uint8_t *h264e_restrict d, point_t wh))
// h264e_deblock
H264E_API(void, h264e_deblock_chroma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
H264E_API(void, h264e_deblock_luma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
// h264e_intra
H264E_API(void, h264e_intra_predict_chroma,  (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(void, h264e_intra_predict_16x16, (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(int,  h264e_intra_choose_4x4, (const pix_t *blockin, pix_t *blockpred, int avail, const pix_t *edge, int mpred, int penalty))
// h264e_cavlc
H264E_API(void,     h264e_bs_put_bits, (bs_t *bs, unsigned n, unsigned val))
H264E_API(void,     h264e_bs_flush, (bs_t *bs))
H264E_API(unsigned, h264e_bs_get_pos_bits, (const bs_t *bs))
H264E_API(unsigned, h264e_bs_byte_align, (bs_t *bs))
H264E_API(void,     h264e_bs_put_golomb, (bs_t *bs, unsigned val))
H264E_API(void,     h264e_bs_put_sgolomb, (bs_t *bs, int val))
H264E_API(void,     h264e_bs_init_bits, (bs_t *bs, void *data))
H264E_API(void,     h264e_vlc_encode, (bs_t *bs, int16_t *quant, int maxNumCoeff, uint8_t *nz_ctx))
// h264e_sad
H264E_API(int,  h264e_sad_mb_unlaign_8x8, (const pix_t *a, int a_stride, const pix_t *b, int sad[4]))
H264E_API(int,  h264e_sad_mb_unlaign_wh, (const pix_t *a, int a_stride, const pix_t *b, point_t wh))
H264E_API(void, h264e_copy_8x8, (pix_t *d, int d_stride, const pix_t *s))
H264E_API(void, h264e_copy_16x16, (pix_t *d, int d_stride, const pix_t *s, int s_stride))
H264E_API(void, h264e_copy_borders, (unsigned char *pic, int w, int h, int guard))
// h264e_transform
H264E_API(void, h264e_transform_add, (pix_t *out, int out_stride, const pix_t *pred, quant_t *q, int side, int32_t mask))
H264E_API(int,  h264e_transform_sub_quant_dequant, (const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q, const uint16_t *qdat))
H264E_API(void, h264e_quant_luma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
H264E_API(int,  h264e_quant_chroma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
// h264e_denoise
H264E_API(void, h264e_denoise_run, (unsigned char *frm, unsigned char *frmprev, int w, int h, int stride_frm, int stride_frmprev))
#undef H264E_API
};
#endif
#if H264E_ENABLE_SSE2
static const vft_t g_vft_sse2 =
{
#define  H264E_API(type, name, args) name##_sse2,
// h264e_qpel
H264E_API(void, h264e_qpel_interpolate_chroma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_interpolate_luma, (const uint8_t *src,int src_stride, uint8_t *h264e_restrict dst,point_t wh, point_t dxdy))
H264E_API(void, h264e_qpel_average_wh_align, (const uint8_t *p0, const uint8_t *p1, uint8_t *h264e_restrict d, point_t wh))
// h264e_deblock
H264E_API(void, h264e_deblock_chroma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
H264E_API(void, h264e_deblock_luma, (uint8_t *pSrcDst, int32_t srcdstStep, const deblock_params_t *par))
// h264e_intra
H264E_API(void, h264e_intra_predict_chroma,  (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(void, h264e_intra_predict_16x16, (pix_t *predict, const pix_t *left, const pix_t *top, int mode))
H264E_API(int,  h264e_intra_choose_4x4, (const pix_t *blockin, pix_t *blockpred, int avail, const pix_t *edge, int mpred, int penalty))
// h264e_cavlc
H264E_API(void,     h264e_bs_put_bits, (bs_t *bs, unsigned n, unsigned val))
H264E_API(void,     h264e_bs_flush, (bs_t *bs))
H264E_API(unsigned, h264e_bs_get_pos_bits, (const bs_t *bs))
H264E_API(unsigned, h264e_bs_byte_align, (bs_t *bs))
H264E_API(void,     h264e_bs_put_golomb, (bs_t *bs, unsigned val))
H264E_API(void,     h264e_bs_put_sgolomb, (bs_t *bs, int val))
H264E_API(void,     h264e_bs_init_bits, (bs_t *bs, void *data))
H264E_API(void,     h264e_vlc_encode, (bs_t *bs, int16_t *quant, int maxNumCoeff, uint8_t *nz_ctx))
// h264e_sad
H264E_API(int,  h264e_sad_mb_unlaign_8x8, (const pix_t *a, int a_stride, const pix_t *b, int sad[4]))
H264E_API(int,  h264e_sad_mb_unlaign_wh, (const pix_t *a, int a_stride, const pix_t *b, point_t wh))
H264E_API(void, h264e_copy_8x8, (pix_t *d, int d_stride, const pix_t *s))
H264E_API(void, h264e_copy_16x16, (pix_t *d, int d_stride, const pix_t *s, int s_stride))
H264E_API(void, h264e_copy_borders, (unsigned char *pic, int w, int h, int guard))
// h264e_transform
H264E_API(void, h264e_transform_add, (pix_t *out, int out_stride, const pix_t *pred, quant_t *q, int side, int32_t mask))
H264E_API(int,  h264e_transform_sub_quant_dequant, (const pix_t *inp, const pix_t *pred, int inp_stride, int mode, quant_t *q, const uint16_t *qdat))
H264E_API(void, h264e_quant_luma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
H264E_API(int,  h264e_quant_chroma_dc, (quant_t *q, int16_t *deq, const uint16_t *qdat))
// h264e_denoise
H264E_API(void, h264e_denoise_run, (unsigned char *frm, unsigned char *frmprev, int w, int h, int stride_frm, int stride_frmprev))
#undef H264E_API
};
#endif

/************************************************************************/
/*      Code to detect CPU features and init VFT                        */
/************************************************************************/

#if H264E_ENABLE_SSE2
#if defined(_MSC_VER)
#define minih264_cpuid __cpuid
#else
static __inline__ __attribute__((always_inline)) void minih264_cpuid(int CPUInfo[], const int InfoType)
{
#if defined(__PIC__)
    __asm__ __volatile__(
#if defined(__x86_64__)
        "push %%rbx\n"
        "cpuid\n"
        "xchgl %%ebx, %1\n"
        "pop  %%rbx\n"
#else /* defined(__x86_64__) */
        "xchgl %%ebx, %1\n"
        "cpuid\n"
        "xchgl %%ebx, %1\n"
#endif /* defined(__x86_64__) */
        : "=a" (CPUInfo[0]), "=r" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3])
        : "a" (InfoType));
#else /* defined(__PIC__) */
    __asm__ __volatile__(
        "cpuid"
        : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3])
        : "a" (InfoType));
#endif /* defined(__PIC__)*/
}
#endif /* defined(_MSC_VER) */

static int CPU_have_SSE2()
{
    int CPUInfo[4];
    minih264_cpuid(CPUInfo, 0);
    if (CPUInfo[0] > 0)
    {
        minih264_cpuid(CPUInfo, 1);
        if (CPUInfo[3] & (1 << 26))
            return 1;
    }
    return 0;
}
#endif

static void init_vft(int enableNEON)
{
#if H264E_ENABLE_PLAIN_C
    g_vft = &g_vft_plain_c;
#endif
    (void)enableNEON;
#if H264E_ENABLE_NEON
    if (enableNEON)
        g_vft = &g_vft_neon;
    else
        g_vft = &g_vft_plain_c;
#endif
#if H264E_ENABLE_SSE2
    if (CPU_have_SSE2())
    {
        g_vft = &g_vft_sse2;
    }
#endif
}

#define MAP_NAME(name) g_vft->name

#endif

#ifdef MAP_NAME
#   define h264e_qpel_interpolate_chroma     MAP_NAME(h264e_qpel_interpolate_chroma)
#   define h264e_qpel_interpolate_luma       MAP_NAME(h264e_qpel_interpolate_luma)
#   define h264e_qpel_average_wh_align       MAP_NAME(h264e_qpel_average_wh_align)
#   define h264e_deblock_chroma              MAP_NAME(h264e_deblock_chroma)
#   define h264e_deblock_luma                MAP_NAME(h264e_deblock_luma)
#   define h264e_intra_predict_chroma        MAP_NAME(h264e_intra_predict_chroma)
#   define h264e_intra_predict_16x16         MAP_NAME(h264e_intra_predict_16x16)
#   define h264e_intra_choose_4x4            MAP_NAME(h264e_intra_choose_4x4)
#   define h264e_bs_put_bits                 MAP_NAME(h264e_bs_put_bits)
#   define h264e_bs_flush                    MAP_NAME(h264e_bs_flush)
#   define h264e_bs_get_pos_bits             MAP_NAME(h264e_bs_get_pos_bits)
#   define h264e_bs_byte_align               MAP_NAME(h264e_bs_byte_align)
#   define h264e_bs_put_golomb               MAP_NAME(h264e_bs_put_golomb)
#   define h264e_bs_put_sgolomb              MAP_NAME(h264e_bs_put_sgolomb)
#   define h264e_bs_init_bits                MAP_NAME(h264e_bs_init_bits)
#   define h264e_vlc_encode                  MAP_NAME(h264e_vlc_encode)
#   define h264e_sad_mb_unlaign_8x8          MAP_NAME(h264e_sad_mb_unlaign_8x8)
#   define h264e_sad_mb_unlaign_wh           MAP_NAME(h264e_sad_mb_unlaign_wh)
#   define h264e_copy_8x8                    MAP_NAME(h264e_copy_8x8)
#   define h264e_copy_16x16                  MAP_NAME(h264e_copy_16x16)
#   define h264e_copy_borders                MAP_NAME(h264e_copy_borders)
#   define h264e_transform_add               MAP_NAME(h264e_transform_add)
#   define h264e_transform_sub_quant_dequant MAP_NAME(h264e_transform_sub_quant_dequant)
#   define h264e_quant_luma_dc               MAP_NAME(h264e_quant_luma_dc)
#   define h264e_quant_chroma_dc             MAP_NAME(h264e_quant_chroma_dc)
#   define h264e_denoise_run                 MAP_NAME(h264e_denoise_run)
#endif

/************************************************************************/
/*      Arithmetics                                                     */
/************************************************************************/

#ifndef __arm__
/**
*   Count of leading zeroes
*/
static unsigned __clz(unsigned v)
{
#if defined(_MSC_VER)
    unsigned long nbit;
    _BitScanReverse(&nbit, v);
    return 31 - nbit;
#elif defined(__GNUC__) || defined(__clang__) || defined(__aarch64__)
    return __builtin_clz(v);
#else
    unsigned clz = 32;
    assert(v);
    do
    {
        clz--;
    } while (v >>= 1);
    return clz;
#endif
}
#endif

/**
*   Size of unsigned Golomb code
*/
static int bitsize_ue(int v)
{
    return 2*(32 - __clz(v + 1)) - 1;
}

/**
*   Size of signed Golomb code
*/
static int bits_se(int v)
{
    v = 2*v - 1;
    v ^= v >> 31;
    return bitsize_ue(v);
}

/**
*   Multiply 32x32 Q16
*/
static uint32_t mul32x32shr16(uint32_t x, uint32_t y)
{
    uint32_t r = (x >> 16) * (y & 0xFFFFu) + x * (y >> 16) + ((y & 0xFFFFu) * (x & 0xFFFFu) >> 16);
    //assert(r == (uint32_t)((__int64)x*y>>16));
    return r;
}

/**
*   Integer division, producing Q16 output
*/
static uint32_t div_q16(uint32_t numer, uint32_t denum)
{
    unsigned f = 1 << __clz(denum);
    do
    {
        denum = denum * f >> 16;
        numer = mul32x32shr16(numer, f);
        f = ((1 << 17) - denum);
    } while (denum  != 0xffff);
    return numer;
}

/************************************************************************/
/*      Motion Vector arithmetics                                       */
/************************************************************************/

static point_t point(int x, int y)
{
    point_t p;
    p.u32 = ((unsigned)y << 16) | ((unsigned)x & 0xFFFF);    // assumes little-endian
    return p;
}

static int mv_is_zero(point_t p)
{
    return !p.u32;
}

static int mv_equal(point_t p0, point_t p1)
{
    return (p0.u32 == p1.u32);
}

/**
*   check that difference between given MV's components is greater than 3
*/
static int mv_differs3(point_t p0, point_t p1)
{
    return ABS(p0.s.x - p1.s.x) > 3 || ABS(p0.s.y - p1.s.y) > 3;
}

static point_t mv_add(point_t a, point_t b)
{
#if defined(__arm__)
    a.u32 = __sadd16(a.u32, b.u32);
#elif H264E_ENABLE_SSE2 && (H264E_CONFIGS_COUNT == 1)
    a.u32 = _mm_cvtsi128_si32(_mm_add_epi16(_mm_cvtsi32_si128(a.u32), _mm_cvtsi32_si128(b.u32)));
#else
    a.s.x += b.s.x;
    a.s.y += b.s.y;
#endif
    return a;
}

static point_t mv_sub(point_t a, point_t b)
{
#if defined(__arm__)
    a.u32 = __ssub16(a.u32, b.u32);
#elif H264E_ENABLE_SSE2 && (H264E_CONFIGS_COUNT == 1)
    a.u32 = _mm_cvtsi128_si32(_mm_sub_epi16(_mm_cvtsi32_si128(a.u32), _mm_cvtsi32_si128(b.u32)));
#else
    a.s.x -= b.s.x;
    a.s.y -= b.s.y;
#endif
    return a;
}

static void mv_clip(point_t *h264e_restrict p, const rectangle_t *range)
{
    p->s.x = MAX(p->s.x, range->tl.s.x);
    p->s.x = MIN(p->s.x, range->br.s.x);
    p->s.y = MAX(p->s.y, range->tl.s.y);
    p->s.y = MIN(p->s.y, range->br.s.y);
}

static int mv_in_rect(point_t p, const rectangle_t *r)
{
    return (p.s.y >= r->tl.s.y && p.s.y <= r->br.s.y && p.s.x >= r->tl.s.x && p.s.x <= r->br.s.x);
}

static point_t mv_round_qpel(point_t p)
{
    return point((p.s.x + 1) & ~3, (p.s.y + 1) & ~3);
}

/************************************************************************/
/*      Misc macroblock helper functions                                */
/************************************************************************/
/**
*   @return current macroblock input luma pixels
*/
static pix_t *mb_input_luma(h264e_enc_t *enc)
{
    return enc->inp.yuv[0] + (enc->mb.x + enc->mb.y*enc->inp.stride[0])*16;
}

/**
*   @return current macroblock input chroma pixels
*/
static pix_t *mb_input_chroma(h264e_enc_t *enc, int uv)
{
    return enc->inp.yuv[uv] + (enc->mb.x + enc->mb.y*enc->inp.stride[uv])*8;
}

/**
*   @return absolute MV for current macroblock for given MV
*/
static point_t mb_abs_mv(h264e_enc_t *enc, point_t mv)
{
    return mv_add(mv, point(enc->mb.x*64, enc->mb.y*64));
}

/************************************************************************/
/*      Pixel copy functions                                            */
/************************************************************************/
/**
*   Copy incomplete (cropped) macroblock pixels with borders extension
*/
static void pix_copy_cropped_mb(pix_t *d, int d_stride, const pix_t *s, int s_stride, int w, int h)
{
    int nbottom = d_stride - h; // assume dst = square d_strideXd_stride
    s_stride -= w;
    do
    {
        int cloop = w;
        pix_t last = 0;
        do
        {
            last = *s++;
            *d++ = last;
        } while (--cloop);
        cloop = d_stride - w;
        if (cloop) do
        {
            *d++ = last;    // extend row
        } while (--cloop);
        s += s_stride;
    } while (--h);
    s = d - d_stride;
    if (nbottom) do
    {
        memcpy(d, s, d_stride);  // extend columns
        d += d_stride;
    } while (--nbottom);
}

/**
*   Copy one image component
*/
static void pix_copy_pic(pix_t *dst, int dst_stride, pix_t *src, int src_stride, int w, int h)
{
    do
    {
        memcpy(dst, src, w);
        dst += dst_stride;
        src += src_stride;
    } while (--h);
}

/**
*   Copy reconstructed frame to reference buffer, with borders extensionn
*/
static void pix_copy_recon_pic_to_ref(h264e_enc_t *enc)
{
    int c, h = enc->frame.h, w = enc->frame.w, guard = 16;
    for (c = 0; c < 3; c++)
    {
        if (enc->param.const_input_flag)
        {
            SWAP(pix_t*, enc->ref.yuv[c], enc->dec.yuv[c]);
        } else
        {
            pix_copy_pic(enc->ref.yuv[c], w + 2*guard, enc->dec.yuv[c], w, w, h);
        }

        h264e_copy_borders(enc->ref.yuv[c], w, h, guard);
        if (!c) guard >>= 1, w >>= 1, h >>= 1;
    }
}

/************************************************************************/
/*      Median MV predictor                                             */
/************************************************************************/

/**
*   @return neighbors availability flags for current macroblock
*/
static int mb_avail_flag(const h264e_enc_t *enc)
{
    int nmb = enc->mb.num;
    int flag = nmb >= enc->slice.start_mb_num + enc->frame.nmbx;
    if (nmb >= enc->slice.start_mb_num + enc->frame.nmbx - 1 && enc->mb.x != enc->frame.nmbx-1)
    {
        flag += AVAIL_TR;
    }
    if (nmb != enc->slice.start_mb_num && enc->mb.x)
    {
        flag += AVAIL_L;
    }
    if (nmb > enc->slice.start_mb_num + enc->frame.nmbx && enc->mb.x)
    {
        flag += AVAIL_TL;
    }
    return flag;
}

/**
*   @return median of 3 given integers
*/
#if !(H264E_ENABLE_SSE2 && (H264E_CONFIGS_COUNT == 1))
static int me_median_of_3(int a, int b, int c)
{
    return MAX(MIN(MAX(a, b), c), MIN(a, b));
}
#endif

/**
*   @return median of 3 given motion vectors
*/
static point_t point_median_of_3(point_t a, point_t b, point_t c)
{
#if H264E_ENABLE_SSE2 && (H264E_CONFIGS_COUNT == 1)
    __m128i a2 = _mm_cvtsi32_si128(a.u32);
    __m128i b2 = _mm_cvtsi32_si128(b.u32);
    point_t med;
    med.u32 = _mm_cvtsi128_si32(_mm_max_epi16(_mm_min_epi16(_mm_max_epi16(a2, b2), _mm_cvtsi32_si128(c.u32)), _mm_min_epi16(a2, b2)));
    return med;
#else
    return point(me_median_of_3(a.s.x, b.s.x, c.s.x),
                 me_median_of_3(a.s.y, b.s.y, c.s.y));
#endif
}

/**
*   Save state of the MV predictor
*/
static void me_mv_medianpredictor_save_ctx(h264e_enc_t *enc, point_t *ctx)
{
    int i;
    point_t *mvtop = enc->mv_pred + 8 + enc->mb.x*4;
    for (i = 0; i < 4; i++)
    {
        *ctx++ = enc->mv_pred[i];
        *ctx++ = enc->mv_pred[4 + i];
        *ctx++ = mvtop[i];
    }
}

/**
*   Restore state of the MV predictor
*/
static void me_mv_medianpredictor_restore_ctx(h264e_enc_t *enc, const point_t *ctx)
{
    int i;
    point_t *mvtop = enc->mv_pred + 8 + enc->mb.x*4;
    for (i = 0; i < 4; i++)
    {
        enc->mv_pred[i] = *ctx++;
        enc->mv_pred[4 + i] = *ctx++;
        mvtop[i] = *ctx++;
    }
}

/**
*   Put motion vector to the deblock filter matrix.
*   x,y,w,h refers to 4x4 blocks within 16x16 macroblock, and should be in the range [0,4]
*/
static void me_mv_dfmatrix_put(point_t *dfmv, int x, int y, int w, int h, point_t mv)
{
    int i;
    assert(y < 4 && x < 4);

    dfmv += y*5 + x + 5;   // 5x5 matrix without left-top cell
    do
    {
        for (i = 0; i < w; i++)
        {
            dfmv[i] = mv;
        }
        dfmv += 5;
    } while (--h);
}

/**
*   Use given motion vector for prediction
*/
static void me_mv_medianpredictor_put(h264e_enc_t *enc, int x, int y, int w, int h, point_t mv)
{
    int i;
    point_t *mvtop = enc->mv_pred + 8 + enc->mb.x*4;
    assert(y < 4 && x < 4);

    enc->mv_pred[4 + y] = mvtop[x + w-1]; // top-left corner = top-right corner
    for (i = 1; i < h; i++)
    {
        enc->mv_pred[4 + y + i] = mv;     // top-left corner(s) for next row(s) = this
    }
    for (i = 0; i < h; i++)
    {
        enc->mv_pred[y + i] = mv;         // left = this
    }
    for (i = 0; i < w; i++)
    {
        mvtop[x + i] = mv;                // top = this
    }
}

/**
*   Motion vector median predictor for non-skip macroblock, as defined in the standard
*/
static point_t me_mv_medianpredictor_get(const h264e_enc_t *enc, point_t xy, point_t wh)
{
    int x = xy.s.x >> 2;
    int y = xy.s.y >> 2;
    int w = wh.s.x >> 2;
    int h = wh.s.y >> 2;
    int mvPredType = MVPRED_MEDIAN;
    point_t a, b, c, d, ret = point(0, 0);
    point_t *mvtop = enc->mv_pred + 8 + enc->mb.x*4;
    int flag = enc->mb.avail;

    assert(y < 4);
    assert(x < 4);
    assert(w <= 4);
    assert(h <= 4);

    a = enc->mv_pred[y];
    b = mvtop[x];
    c = mvtop[x + w];
    d = enc->mv_pred[4 + y];

    if (!x)
    {
        if (!(flag & AVAIL_L))
        {
            a.u32 = MV_NA;
        }
        if (!(flag & AVAIL_TL))
        {
            d.u32 = MV_NA;
        }
    }
    if (!y)
    {
        if (!(flag & AVAIL_T))
        {
            b.u32 = MV_NA;
            if (x + w < 4)
            {
                c.u32 = MV_NA;
            }
            if (x > 0)
            {
                d.u32 = MV_NA;
            }
        }
        if (!(flag & AVAIL_TL) && !x)
        {
            d.u32 = MV_NA;
        }
        if (!(flag & AVAIL_TR) && x + w == 4)
        {
            c.u32 = MV_NA;
        }
    }

    if (x + w == 4 && (!(flag & AVAIL_TR) || y))
    {
        c = d;
    }

    if (AVAIL(a) && !AVAIL(b) && !AVAIL(c))
    {
        mvPredType = MVPRED_L;
    } else if (!AVAIL(a) && AVAIL(b) && !AVAIL(c))
    {
        mvPredType = MVPRED_U;
    } else if (!AVAIL(a) && !AVAIL(b) && AVAIL(c))
    {
        mvPredType = MVPRED_UR;
    }

    // Directional predictions
    if (w == 2 && h == 4)
    {
        if (x == 0)
        {
            if (AVAIL(a))
            {
                mvPredType = MVPRED_L;
            }
        } else
        {
            if (AVAIL(c))
            {
                mvPredType = MVPRED_UR;
            }
        }
    } else if (w == 4 && h == 2)
    {
        if (y == 0)
        {
            if (AVAIL(b))
            {
                mvPredType = MVPRED_U;
            }
        } else
        {
            if (AVAIL(a))
            {
                mvPredType = MVPRED_L;
            }
        }
    }

    switch(mvPredType)
    {
    default:
    case MVPRED_MEDIAN:
        if (!(AVAIL(b) || AVAIL(c)))
        {
            if (AVAIL(a))
            {
                ret = a;
            }
        } else
        {
            if (!AVAIL(a))
            {
                a = ret;
            }
            if (!AVAIL(b))
            {
                b = ret;
            }
            if (!AVAIL(c))
            {
                c = ret;
            }
            ret = point_median_of_3(a, b, c);
        }
        break;
    case MVPRED_L:
        if (AVAIL(a))
        {
            ret = a;
        }
        break;
    case MVPRED_U:
        if (AVAIL(b))
        {
            ret = b;
        }
        break;
    case MVPRED_UR:
        if (AVAIL(c))
        {
            ret = c;
        }
        break;
    }
    return ret;
}

/**
*   Motion vector median predictor for skip macroblock
*/
static point_t me_mv_medianpredictor_get_skip(h264e_enc_t *enc)
{
    point_t pred_16x16 = me_mv_medianpredictor_get(enc, point(0, 0),  point(16, 16));
    enc->mb.mv_skip_pred = point(0, 0);
    if (!(~enc->mb.avail & (AVAIL_L | AVAIL_T)))
    {
        point_t *mvtop = enc->mv_pred + 8 + enc->mb.x*4;
        if (!mv_is_zero(enc->mv_pred[0]) && !mv_is_zero(mvtop[0]))
        {
            enc->mb.mv_skip_pred = pred_16x16;
        }
    }
    return pred_16x16;
}

/**
*   Get starting points candidates for MV search
*/
static int me_mv_medianpredictor_get_cand(const h264e_enc_t *enc, point_t *mv)
{
    point_t *mv0 = mv;
    point_t *mvtop = enc->mv_pred + 8 + enc->mb.x*4;
    int flag = enc->mb.avail;
    *mv++ = point(0, 0);
    if ((flag & AVAIL_L) && AVAIL(enc->mv_pred[0]))
    {
        *mv++ = enc->mv_pred[0];
    }
    if ((flag & AVAIL_T) && AVAIL(mvtop[0]))
    {
        *mv++ = mvtop[0];
    }
    if ((flag & AVAIL_TR) && AVAIL(mvtop[4]))
    {
        *mv++ = mvtop[4];
    }
    return (int)(mv - mv0);
}


/************************************************************************/
/*      NAL encoding                                                    */
/************************************************************************/

/**
*   Count ## of escapes, i.e. binary strings 0000 0000  0000 0000  0000 00xx
*   P(escape) = 2^-22
*   E(run_between_escapes) = 2^21 ~= 2 MB
*/
static int nal_count_esc(const uint8_t *s, int n)
{
    int i, cnt_esc = 0, cntz = 0;
    for (i = 0; i < n; i++)
    {
        uint8_t byte = *s++;
        if (cntz == 2 && byte <= 3)
        {
            cnt_esc++;
            cntz = 0;
        }

        if (byte)
        {
            cntz = 0;
        } else
        {
            cntz++;
        }
    }
    return cnt_esc;
}

/**
*   Put NAL escape codes to the output bitstream
*/
static int nal_put_esc(uint8_t *d, const uint8_t *s, int n)
{
    int i, j = 0, cntz = 0;
    for (i = 0; i < n; i++)
    {
        uint8_t byte = *s++;
        if (cntz == 2 && byte <= 3)
        {
            d[j++] = 3;
            cntz = 0;
        }

        if (byte)
        {
            cntz = 0;
        } else
        {
            cntz++;
        }
        d[j++] = byte;
    }
    assert(d + j <= s);
    return j;
}

/**
*   Init NAL encoding
*/
static void nal_start(h264e_enc_t *enc, int nal_hdr)
{
    uint8_t *d = enc->out + enc->out_pos;
    d[0] = d[1] = d[2] = 0; d[3] = 1; // start code
    enc->out_pos += STARTCODE_4BYTES;
    d += STARTCODE_4BYTES + (-(int)enc->out_pos & 3);   // 4-bytes align for bitbuffer
    assert(IS_ALIGNED(d, 4));
    h264e_bs_init_bits(enc->bs, d);
    U(8, nal_hdr);
}

/**
*   Finalize NAL encoding
*/
static void nal_end(h264e_enc_t *enc)
{
    int cnt_esc, bs_bytes;
    uint8_t *nal = enc->out + enc->out_pos;

    U1(1); // stop bit
    bs_bytes = h264e_bs_byte_align(enc->bs) >> 3;
    h264e_bs_flush(enc->bs);

    // count # of escape bytes to insert
    cnt_esc = nal_count_esc((unsigned char*)enc->bs->origin, bs_bytes);

    if ((uint8_t *)enc->bs->origin != nal + cnt_esc)
    {
        // make free space for escapes and remove align bytes
        memmove(nal + cnt_esc, enc->bs->origin, bs_bytes);
    }
    if (cnt_esc)
    {
        // insert escape bytes
        bs_bytes = nal_put_esc(nal, nal + cnt_esc, bs_bytes);
    }
    if (enc->run_param.nalu_callback)
    {
        // Call application-supplied callback
        enc->run_param.nalu_callback(nal, bs_bytes, enc->run_param.nalu_callback_token);
    }
    enc->out_pos += bs_bytes;
}


/************************************************************************/
/*      Top-level syntax elements (SPS,PPS,Slice)                       */
/************************************************************************/

/**
*   Encode Sequence Parameter Set (SPS)
*   ref: [1] 7.3.2.1.1
*/

//temp global
#define dependency_id 1
#define quality_id 0
#define default_base_mode_flag 0
#define log2_max_frame_num_minus4 1

static void encode_sps(h264e_enc_t *enc, int profile_idc)
{
    struct limit_t
    {
        uint8_t level;
        uint8_t constrains;
        uint16_t max_fs;
        uint16_t max_vbvdiv5;
        uint32_t max_dpb;
    };
    static const struct limit_t limit [] = {
        {10, 0xE0, 99,    175/5, 396},
        {10, 0xF0, 99,    350/5, 396},
        {11, 0xE0, 396,   500/5, 900},
        {12, 0xE0, 396,   1000/5, 2376},
        {13, 0xE0, 396,   2000/5, 2376},
        {20, 0xE0, 396,   2000/5, 2376},
        {21, 0xE0, 792,   4000/5, 4752},
        {22, 0xE0, 1620,  4000/5, 8100},
        {30, 0xE0, 1620,  10000/5, 8100},
        {31, 0xE0, 3600,  14000/5, 18000},
        {32, 0xE0, 5120,  20000/5, 20480},
        {40, 0xE0, 8192,  25000/5, 32768},
        {41, 0xE0, 8192,  62500/5, 32768},
        {42, 0xE0, 8704,  62500/5, 34816},
        {50, 0xE0, 22080, 135000/5, 110400},
        {51, 0xE0, 36864, 240000/5, 184320}
    };
    const struct limit_t *plim = limit;

    while (plim->level < 51 && (enc->frame.nmb > plim->max_fs ||
        enc->param.vbv_size_bytes > plim->max_vbvdiv5*(5*1000/8) ||
        (unsigned)(enc->frame.nmb*(enc->param.max_long_term_reference_frames + 1)) > plim->max_dpb))
    {
        plim++;
    }

    nal_start(enc, 0x67 | (profile_idc == SCALABLE_BASELINE)*8);
    U(8, profile_idc);  // profile, 66 = baseline
    U(8, plim->constrains & ((profile_idc!= SCALABLE_BASELINE)*4));     // no constrains
    U(8, plim->level);
    //U(5, 0x1B);       // sps_id|log2_max_frame_num_minus4|pic_order_cnt_type
    //UE(0);  // sps_id 1
    UE(enc->param.sps_id);

#if H264E_SVC_API
    if(profile_idc== SCALABLE_BASELINE)
    {
        UE(1); //chroma_format_idc
        UE(0); //bit_depth_luma_minus8
        UE(0); //bit_depth_chroma_minus8)
        U1(0); //qpprime_y_zero_transform_bypass_flag
        U1(0); //seq_scaling_matrix_present_flag
    }
#endif
    UE(log2_max_frame_num_minus4);  // log2_max_frame_num_minus4  1 UE(0);  // log2_max_frame_num_minus4  1
    UE(2);  // pic_order_cnt_type         011
    UE(1 + enc->param.max_long_term_reference_frames);  // num_ref_frames
    U1(0);                                      // gaps_in_frame_num_value_allowed_flag);
    UE(((enc->param.width + 15) >> 4) - 1);     // pic_width_in_mbs_minus1
    UE(((enc->param.height + 15) >> 4) - 1);    // pic_height_in_map_units_minus1
    U(3, 6 + enc->frame.cropping_flag);         // frame_mbs_only_flag|direct_8x8_inference_flag|frame_cropping_flag
//    U1(1);  // frame_mbs_only_flag
//    U1(1);  // direct_8x8_inference_flag
//    U1(frame_cropping_flag);  // frame_cropping_flag
    if (enc->frame.cropping_flag)
    {
        UE(0);                                          // frame_crop_left_offset
        UE((enc->frame.w - enc->param.width) >> 1);     // frame_crop_right_offset
        UE(0);                                          // frame_crop_top_offset
        UE((enc->frame.h - enc->param.height) >> 1);    // frame_crop_bottom_offset
    }
    U1(0);      // vui_parameters_present_flag

#if H264E_SVC_API
    if(profile_idc == SCALABLE_BASELINE)
    {
        U1(1);  //(inter_layer_deblocking_filter_control_present_flag); //inter_layer_deblocking_filter_control_present_flag
        U(2,0); //extended_spatial_scalability
        U1(0);  //chroma_phase_x_plus1_flag
        U(2,0); //chroma_phase_y_plus1

    /*    if( sps->sps_ext.extended_spatial_scalability == 1 )
        {
            //if( ChromaArrayType > 0 )
            {
                put_bits( s, 1,0);
                put_bits( s, 2,0); ///
            }
            put_bits_se( s, sps->sps_ext.seq_scaled_ref_layer_left_offset );
            put_bits_se( s, sps->sps_ext.seq_scaled_ref_layer_top_offset );
            put_bits_se( s, sps->sps_ext.seq_scaled_ref_layer_right_offset );
            put_bits_se( s, sps->sps_ext.seq_scaled_ref_layer_bottom_offset );
        }*/
        U1(0); //seq_tcoeff_level_prediction_flag
        U1(1); //slice_header_restriction_flag
        U1(0); //svc_vui_parameters_present_flag
        U1(0); //additional_extension2_flag
    }
#endif
    nal_end(enc);
}

/**
*   Encode Picture Parameter Set (SPS)
*   ref: [1] 7.3.2.2
*/
static void encode_pps(h264e_enc_t *enc, int pps_id)
{
    nal_start(enc, 0x68);
 //   U(10, 0x338);       // constant shortcut:
    UE(enc->param.sps_id*4 + pps_id);  // pic_parameter_set_id         1
    UE(enc->param.sps_id);  // seq_parameter_set_id         1
    U1(0);  // entropy_coding_mode_flag     0
    U1(0);  // pic_order_present_flag       0
    UE(0);  // num_slice_groups_minus1      1
    UE(0);  // num_ref_idx_l0_active_minus1 1
    UE(0);  // num_ref_idx_l1_active_minus1 1
    U1(0);  // weighted_pred_flag           0
    U(2,0); // weighted_bipred_idc          00
    SE(enc->sps.pic_init_qp - 26);  // pic_init_qp_minus26
#if DQP_CHROMA
    SE(0);  // pic_init_qs_minus26                    1
    SE(DQP_CHROMA);  // chroma_qp_index_offset        1
    U1(1);  // deblocking_filter_control_present_flag 1
    U1(0);  // constrained_intra_pred_flag            0
    U1(0);  // redundant_pic_cnt_present_flag         0
#else
    U(5, 0x1C);         // constant shortcut:
//     SE(0);  // pic_init_qs_minus26                    1
//     SE(0);  // chroma_qp_index_offset                 1
//     U1(1);  // deblocking_filter_control_present_flag 1
//     U1(0);  // constrained_intra_pred_flag            0
//     U1(0);  // redundant_pic_cnt_present_flag         0
#endif
    nal_end(enc);
}

/**
*   Encode Slice Header
*   ref: [1] 7.3.3
*/
static void encode_slice_header(h264e_enc_t *enc, int frame_type, int long_term_idx_use, int long_term_idx_update, int pps_id, int enc_type)
{
    // slice reset
    enc->slice.start_mb_num = enc->mb.num;
    enc->mb.skip_run = 0;
    memset(enc->i4x4mode, -1, (enc->frame.nmbx + 1)*4);
    memset(enc->nnz, NNZ_NA, (enc->frame.nmbx + 1)*8);    // DF ignore slice borders, but uses it's own nnz's

    if (enc_type == 0)
    {
#if H264E_SVC_API
        if (enc->param.num_layers > 1)
        {
            //need prefix nal for compatibility base layer with h264
            nal_start(enc, 14 | 0x40);
            //if((nal_unit_type == NAL_UNIT_TYPE_PREFIX_SCALABLE_EXT ) ||nal_unit_type == NAL_UNIT_TYPE_RBSP_SCALABLE_EXT))
            {
                //reserved_one_bit = 1    idr_flag                    priority_id
                U(8, (1 << 7) | ((frame_type == H264E_FRAME_TYPE_KEY) << 6) | 0);
                U1(1);   //no_inter_layer_pred_flag
                U(3, 0); //dependency_id
                U(4, quality_id); //quality_id
                //reserved_three_2bits = 3!
                U(3, 0); //temporal_id
                U1(1); //use_ref_base_pic_flag
                U1(0); //discardable_flag
                U1(1); //output_flag
                U(2, 3);

                U1(0); //store_ref_base_pic_flag
                if (!(frame_type == H264E_FRAME_TYPE_KEY))
                {
                    U1(0); //adaptive_ref_base_pic_marking_mode_flag  u(1)
                }

                U1(0); //prefix_nal_unit_additional_extension_flag 2 u(1)

                //put_bits_rbsp_trailing( s );
            }
            nal_end(enc);
        }
#endif //#if H264E_SVC_API
        nal_start(enc, (frame_type == H264E_FRAME_TYPE_KEY ? 5 : 1) | (long_term_idx_update >= 0 ? 0x60 : 0));
    }
#if H264E_SVC_API
    else
    {
        nal_start(enc, (20 | (long_term_idx_update >= 0 ? 0x60 : 0)));  //RBSP_SCALABLE_EXT = 20
        //nal_unit_type 20 or 14
        {
            //reserved_one_bit = 1    idr_flag                    priority_id
            U(8, (1 << 7) | ((frame_type == H264E_FRAME_TYPE_KEY) << 6) | 0);
            U1(!enc->param.inter_layer_pred_flag); //no_inter_layer_pred_flag
            U(3, dependency_id); //dependency_id
            U(4, quality_id);    //quality_id
            //reserved_three_2bits = 3!!!
            U(3, 0); //temporal_id
            U1(0); //use_ref_base_pic_flag
            U1(1); //discardable_flag
            U1(1); //output_flag
            U(2, 3);
        }
    }
#endif

    UE(enc->slice.start_mb_num);        // first_mb_in_slice
    UE(enc->slice.type);                // slice_type
    //U(1+4, 16 + (enc->frame.num&15));   // pic_parameter_set_id | frame_num
    UE(pps_id);                           // pic_parameter_set_id
    U(4 + log2_max_frame_num_minus4, enc->frame.num & ((1 << (log2_max_frame_num_minus4 + 4)) - 1)); // frame_num U(4, enc->frame.num&15);            // frame_num
    if (frame_type == H264E_FRAME_TYPE_KEY)
    {
        UE(enc->next_idr_pic_id);       // idr_pic_id
    }
    //!!!  if !quality_id && enc->slice.type == SLICE_TYPE_P  put_bit(s, 0); // num_ref_idx_active_override_flag = 0
    if(!quality_id)
    {
        if (((enc_type != 0)) && enc->slice.type == SLICE_TYPE_P)
        {
            //U1(0);
        }
        if (enc->slice.type == SLICE_TYPE_P)// if( slice_type == P  | |  slice_type ==  SP  | |  slice_type  = =  B )
        {
            int ref_pic_list_modification_flag_l0 = long_term_idx_use > 0;
            //U1(0);                      // num_ref_idx_active_override_flag
            // ref_pic_list_modification()
            U(2, ref_pic_list_modification_flag_l0); // num_ref_idx_active_override_flag | ref_pic_list_modification_flag_l0
            if (ref_pic_list_modification_flag_l0)
            {
                // Table 7-7
                UE(2);      // long_term_pic_num is present and specifies the long-term picture number for a reference picture
                UE(long_term_idx_use - 1); // long_term_pic_num
                UE(3);      // End loop
            }
        }

        if (long_term_idx_update >= 0)
        {
            //dec_ref_pic_marking( )
            if (frame_type == H264E_FRAME_TYPE_KEY)
            {
                //U1(0);                                      // no_output_of_prior_pics_flag
                //U1(enc->param.enable_golden_frames_flag);   // long_term_reference_flag
                U(2, enc->param.max_long_term_reference_frames > 0);   // no_output_of_prior_pics_flag | long_term_reference_flag
            } else
            {
                int adaptive_ref_pic_marking_mode_flag = long_term_idx_update > 0;//(frame_type == H264E_FRAME_TYPE_GOLDEN);
                U1(adaptive_ref_pic_marking_mode_flag);
                if (adaptive_ref_pic_marking_mode_flag)
                {
                    // Table 7-9
                    if (enc->short_term_used)
                    {
                        UE(1);  // unmark short
                        UE(0);  // unmark short
                    }
                    if (enc->lt_used[long_term_idx_update - 1])
                    {
                        UE(2);  // Mark a long-term reference picture as "unused for reference"
                        UE(long_term_idx_update - 1); // index
                    } else
                    {
                        UE(4);  // Specify the maximum long-term frame index
                        UE(enc->param.max_long_term_reference_frames);    // [0,max-1]+1
                    }
                    UE(6);  // Mark the current picture as "used for long-term reference"
                    UE(long_term_idx_update - 1);   // index
                    UE(0);  // End loop
                }
            }
        }
    }
    SE(enc->rc.prev_qp - enc->sps.pic_init_qp);     // slice_qp_delta
#if H264E_MAX_THREADS
    if (enc->param.max_threads > 1)
    {
        UE(enc->speed.disable_deblock ? 1 : 2);
    } else
#endif
    {
        UE(enc->speed.disable_deblock);             // disable deblock
    }

    if (enc->speed.disable_deblock != 1)
    {
#if ALPHA_OFS || BETA_OFS
        SE(ALPHA_OFS/2);                            // slice_alpha_c0_offset_div2
        SE(BETA_OFS/2);                             // slice_beta_offset_div2
#else
        U(2, 3);
#endif
    }

#if H264E_SVC_API
    if (enc_type != 0)
    {
        enc->adaptive_base_mode_flag = enc->param.inter_layer_pred_flag;
        if (enc->param.inter_layer_pred_flag && !quality_id)
        {
            UE(16*(dependency_id - 1));
            //if(1)//(inter_layer_deblocking_filter_control_present_flag)
            {
                UE(0);//disable_inter_layer_deblocking_filter_idc
                UE(0);
                UE(0);
            }
            /*if( sh->disable_inter_layer_deblocking_filter_idc != 1 )
            {
                put_bits_se(s, sh->slice_alpha_c0_offset_div2);
                put_bits_se(s, sh->slice_beta_offset_div2);
            }*/
            U1(0); // constrained_intra_resampling_flag 2 u(1)
        }
        if (enc->param.inter_layer_pred_flag)
        {
            U1(0); //slice_skip_flag u(1)
            {
                U1(enc->adaptive_base_mode_flag); // 2 u(1)
                if (!enc->adaptive_base_mode_flag)
                    U1(default_base_mode_flag); // 2 u(1)
                if (!default_base_mode_flag)
                {
                    U1(0); //adaptive_motion_prediction_flag) // 2 u(1)
                    U1(0); //sh->default_motion_prediction_flag// 2 u(1)
                }
                U1(0); //adaptive_residual_prediction_flag // 2 u(1)
                U1(0); //default_residual_prediction_flag // 2 u(1)
            }
        }
    }
#endif // #if H264E_SVC_API
}

/**
*   Macroblock transform, quantization and bitstream encoding
*/
static void mb_write(h264e_enc_t *enc, int enc_type, int base_mode)
{
    int i, uv, mb_type, cbpc, cbpl, cbp;
    scratch_t *qv = enc->scratch;
    //int base_mode = enc_type > 0 ? 1 : 0;
    int mb_type_svc = base_mode ? -2 : enc->mb.type;
    int intra16x16_flag = mb_type_svc >= 6;// && !base_mode;
    uint8_t nz[9];
    uint8_t *nnz_top = enc->nnz + 8 + enc->mb.x*8;
    uint8_t *nnz_left = enc->nnz;

    if (enc->mb.type != 5)
    {
        enc->i4x4mode[0] = enc->i4x4mode[enc->mb.x + 1] = 0x02020202;
    }

    enc->df.nzflag = ((enc->df.nzflag >> 4) & 0x84210) | enc->df.df_nzflag[enc->mb.x];
    for (i = 0; i < 4; i++)
    {
        nz[5 + i] = nnz_top[i];
        nnz_top[i] = 0;
        nz[3 - i] = nnz_left[i];
        nnz_left[i] = 0;
    }

l_skip:
    if (enc->mb.type == -1)
    {
        // encode skip macroblock
        assert(enc->slice.type != SLICE_TYPE_I);

        // Increment run count
        enc->mb.skip_run++;

        // Update predictors
        *(uint32_t*)(nnz_top + 4) = *(uint32_t*)(nnz_left + 4) = 0; // set chroma NNZ to 0
        me_mv_medianpredictor_put(enc, 0, 0, 4, 4, enc->mb.mv[0]);
        me_mv_dfmatrix_put(enc->df.df_mv, 0, 0, 4, 4, enc->mb.mv[0]);

        // Update reference with reconstructed pixels
        h264e_copy_16x16(enc->dec.yuv[0], enc->dec.stride[0], enc->pbest, 16);
        h264e_copy_8x8(enc->dec.yuv[1], enc->dec.stride[1], enc->ptest);
        h264e_copy_8x8(enc->dec.yuv[2], enc->dec.stride[2], enc->ptest + 8);
    } else
    {
        if (enc->mb.type != 5)
        {
            unsigned nz_mask;
            nz_mask = h264e_transform_sub_quant_dequant(qv->mb_pix_inp, enc->pbest, 16, intra16x16_flag ? QDQ_MODE_INTRA_16 : QDQ_MODE_INTER, qv->qy, enc->rc.qdat[0]);
            enc->scratch->nz_mask = (uint16_t)nz_mask;
            if (intra16x16_flag)
            {
                h264e_quant_luma_dc(qv->qy, qv->quant_dc, enc->rc.qdat[0]);
                nz_mask = 0xFFFF;
            }
            h264e_transform_add(enc->dec.yuv[0], enc->dec.stride[0], enc->pbest, qv->qy, 4, nz_mask << 16);
        }

        // Coded Block Pattern for luma
        cbpl = 0;
        if (enc->scratch->nz_mask & 0xCC00) cbpl |= 1;
        if (enc->scratch->nz_mask & 0x3300) cbpl |= 2;
        if (enc->scratch->nz_mask & 0x00CC) cbpl |= 4;
        if (enc->scratch->nz_mask & 0x0033) cbpl |= 8;

        // Coded Block Pattern for chroma
        cbpc = 0;
        for (uv = 1; uv < 3; uv++)
        {
            pix_t *pred = enc->ptest + (uv - 1)*8;
            pix_t *pix_mb_uv = mb_input_chroma(enc, uv);
            int dc_flag, inp_stride = enc->inp.stride[uv];
            unsigned nz_mask;
            quant_t *pquv = (uv == 1) ? qv->qu : qv->qv;

            if (enc->frame.cropping_flag && ((enc->mb.x + 1)*16  > enc->param.width || (enc->mb.y + 1)*16  > enc->param.height))
            {
                pix_copy_cropped_mb(enc->scratch->mb_pix_inp, 8, pix_mb_uv, enc->inp.stride[uv],
                    MIN(8, enc->param.width/2  - enc->mb.x*8),
                    MIN(8, enc->param.height/2 - enc->mb.y*8)
                    );
                pix_mb_uv = enc->scratch->mb_pix_inp;
                inp_stride = 8;
            }

            nz_mask = h264e_transform_sub_quant_dequant(pix_mb_uv, pred, inp_stride, QDQ_MODE_CHROMA, pquv, enc->rc.qdat[1]);

            if (nz_mask)
            {
                cbpc = 2;
            }

            cbpc |= dc_flag = h264e_quant_chroma_dc(pquv, uv == 1 ? qv->quant_dc_u : qv->quant_dc_v, enc->rc.qdat[1]);

            if (!(dc_flag | nz_mask))
            {
                h264e_copy_8x8(enc->dec.yuv[uv], enc->dec.stride[uv], pred);
            } else
            {
                if (dc_flag)
                {
                    for (i = 0; i < 4; i++)
                    {
                        if (~nz_mask & (8 >> i))
                        {
                            memset(pquv[i].dq + 1, 0, (16 - 1)*sizeof(int16_t));
                        }
                    }
                    nz_mask = 15;
                }
                h264e_transform_add(enc->dec.yuv[uv], enc->dec.stride[uv], pred, pquv, 2, nz_mask << 28);
            }
        }
        cbpc = MIN(cbpc, 2);

        // Rollback to skip
        if (!(enc->mb.type | cbpl | cbpc) && // Inter prediction, all-zero after quantization
            mv_equal(enc->mb.mv[0], enc->mb.mv_skip_pred)) // MV == MV preditor for skip
        {
            enc->mb.type = -1;
            goto l_skip;
        }

        mb_type = enc->mb.type;
        if (mb_type_svc >= 6)   // intra 16x16
        {
            if (cbpl)
            {
                cbpl = 15;
            }
            mb_type += enc->mb.i16.pred_mode_luma + cbpc*4 + (cbpl ? 12 : 0);
        }
        if (mb_type >= 5 && enc->slice.type == SLICE_TYPE_I)    // Intra in I slice
        {
            mb_type -= 5;
        }

        if (enc->slice.type != SLICE_TYPE_I)
        {
            UE(enc->mb.skip_run);
            enc->mb.skip_run = 0;
        }

        (void)enc_type;
#if H264E_SVC_API
        if (enc->adaptive_base_mode_flag && enc_type > 0)
            U1(base_mode);
#endif

        if (!base_mode)
            UE(mb_type);

        if (enc->mb.type == 3) // 8x8
        {
            for (i = 0; i < 4; i++)
            {
                UE(0);
            }
            // 0 = 8x8
            // 1 = 8x4
            // 2 = 4x8
            // 3 = 4x4
        }

        if (!base_mode)
        {
            if (enc->mb.type >= 5)   // intra
            {
                int pred_mode_chroma;
                if (enc->mb.type == 5)  // intra 4x4
                {
                    for (i = 0; i < 16; i++)
                    {
                        int m = enc->mb.i4x4_mode[decode_block_scan[i]];
                        int nbits =  4;
                        if (m < 0)
                        {
                            m = nbits = 1;
                        }
                        U(nbits, m);
                    }
                }
                pred_mode_chroma = enc->mb.i16.pred_mode_luma;
                if (!(pred_mode_chroma&1))
                {
                    pred_mode_chroma ^= 2;
                }
                UE(pred_mode_chroma);
                me_mv_medianpredictor_put(enc, 0, 0, 4, 4, point(MV_NA,0));
            } else
            {
                int part, x = 0, y = 0;
                int dx = (enc->mb.type & 2) ? 2 : 4;
                int dy = (enc->mb.type & 1) ? 2 : 4;
                for (part = 0;;part++)
                {
                    SE(enc->mb.mvd[part].s.x);
                    SE(enc->mb.mvd[part].s.y);
                    me_mv_medianpredictor_put(enc, x, y, dx, dy, enc->mb.mv[part]);
                    me_mv_dfmatrix_put(enc->df.df_mv, x, y, dx, dy, enc->mb.mv[part]);
                    x = (x + dx) & 3;
                    if (!x)
                    {
                        y = (y + dy) & 3;
                        if (!y)
                        {
                            break;
                        }
                    }
                }
            }
        }
        cbp = cbpl + (cbpc << 4);
        /*temp for test up-sample filter*/
        /*if(base_mode)
        {
            cbp = 0;
            cbpl=0;
            cbpc = 0;
        }*/
        if (mb_type_svc < 6)
        {
            // encode cbp 9.1.2 Mapping process for coded block pattern
            static const uint8_t cbp2code[2][48] = {
                {3, 29, 30, 17, 31, 18, 37,  8, 32, 38, 19,  9, 20, 10, 11,  2, 16, 33, 34, 21, 35, 22, 39,  4,
                36, 40, 23,  5, 24,  6,  7,  1, 41, 42, 43, 25, 44, 26, 46, 12, 45, 47, 27, 13, 28, 14, 15,  0},
                {0,  2,  3,  7,  4,  8, 17, 13,  5, 18,  9, 14, 10, 15, 16, 11,  1, 32, 33, 36, 34, 37, 44, 40,
                35, 45, 38, 41, 39, 42, 43, 19,  6, 24, 25, 20, 26, 21, 46, 28, 27, 47, 22, 29, 23, 30, 31, 12}
            };
            UE(cbp2code[mb_type_svc < 5][cbp]);
        }

        if (cbp || (mb_type_svc >= 6))
        {
            SE(enc->rc.qp - enc->rc.prev_qp);
            enc->rc.prev_qp = enc->rc.qp;
        }

        // *** Huffman encoding ***

        // 1. Encode Luma DC (intra 16x16 only)
        if (intra16x16_flag)
        {
            h264e_vlc_encode(enc->bs, qv->quant_dc, 16, nz + 4);
        }

        // 2. Encode luma residual (only if CBP non-zero)
        if (cbpl)
        {
            for (i = 0; i < 16; i++)
            {
                int j = decode_block_scan[i];
                if (cbp & (1 << (i >> 2)))
                {
                    uint8_t *pnz = nz + 4 + (j & 3) - (j >> 2);
                    h264e_vlc_encode(enc->bs, qv->qy[j].qv, 16 - intra16x16_flag, pnz);
                    if (*pnz)
                    {
                        enc->df.nzflag |= 1 << (5 + (j & 3) + 5*(j >> 2));
                    }
                } else
                {
                    nz[4 + (j & 3) - (j >> 2)] = 0;
                }
            }
            for (i = 0; i < 4; i++)
            {
                nnz_top[i] = nz[1 + i];
                nnz_left[i] = nz[7 - i];
            }
        }

        // 2. Encode chroma
        if (cbpc)
        {
            uint8_t nzcdc[3];
            nzcdc[0] = nzcdc[2] = 17;   // dummy neighbors, indicating chroma DC
            // 2.1. Encode chroma DC
            for (uv = 1; uv < 3; uv++)
            {
                h264e_vlc_encode(enc->bs, uv == 1 ? qv->quant_dc_u : qv->quant_dc_v, 4, nzcdc + 1);
            }

            // 2.2. Encode chroma residual
            if (cbpc > 1)
            {
                for (uv = 1; uv < 3; uv++)
                {
                    uint8_t nzc[5];
                    int nnz_off = (uv == 1 ? 4 : 6);
                    quant_t *pquv = uv == 1 ? qv->qu : qv->qv;
                    for (i = 0; i < 2; i++)
                    {
                        nzc[3 + i] = nnz_top[nnz_off + i] ;
                        nzc[1 - i] = nnz_left[nnz_off + i];
                    }
                    for (i = 0; i < 4; i++)
                    {
                        int k = 2 + (i & 1) - (i >> 1);
                        h264e_vlc_encode(enc->bs, pquv[i].qv, 15, nzc + k);
                    }
                    for (i = 0; i < 2; i++)
                    {
                        nnz_top[nnz_off + i]  = nzc[1 + i];
                        nnz_left[nnz_off + i] = nzc[3 - i];
                    }
                }
            }
        }
        if (cbpc !=2)
        {
            *(uint32_t*)(nnz_top+4) = *(uint32_t*)(nnz_left+4) = 0; // set chroma NNZ to 0
        }
    }

    // Save top & left lines
    for (uv = 0; uv < 3; uv++)
    {
        int off = 0, n = uv ? 8 : 16;
        pix_t *top = enc->top_line + 48 + enc->mb.x*32;
        pix_t *left = enc->top_line;
        pix_t *mb = enc->dec.yuv[uv];

        if (uv)
        {
            off = 8 + uv*8;
        }
        top  += off;
        left += off;

        enc->top_line[32 + uv] = top[n - 1];
        for (i = 0; i < n; i++)
        {
            left[i] = mb[n - 1 + i*enc->dec.stride[uv]];
            top[i] = mb[(n - 1)*enc->dec.stride[uv] + i];
        }
    }
}

/************************************************************************/
/*      Intra mode encoding                                             */
/************************************************************************/
/**
*   Estimate cost of 4x4 intra predictor
*/
static void intra_choose_4x4(h264e_enc_t *enc)
{
    int i, n, a, nz_mask = 0, avail = mb_avail_flag(enc);
    scratch_t *qv = enc->scratch;
    pix_t *mb_dec = enc->dec.yuv[0];
    pix_t *dec = enc->ptest;
    int cost =  g_lambda_i4_q4[enc->rc.qp];// + MUL_LAMBDA(16, g_lambda_q4[enc->rc.qp]);    // 4x4 cost: at least 16 bits + penalty

    uint32_t edge_store[(3 + 16 + 1 + 16 + 4)/4 + 2]; // pad for SSE
    pix_t *edge = ((pix_t*)edge_store) + 3 + 16 + 1;
    uint32_t *edge32 = (uint32_t *)edge;              // alias
    const uint32_t *top32 = (const uint32_t*)(enc->top_line + 48 + enc->mb.x*32);
    pix_t *left = enc->top_line;

    edge[-1] = enc->top_line[32];
    for (i = 0; i < 16; i++)
    {
        edge[-2 - i] = left[i];
    }
    for (i = 0; i < 4; i++)
    {
        edge32[i] = top32[i];
    }
    edge32[4] = top32[8];

    for (n = 0; n < 16; n++)
    {
        static const uint8_t block2avail[16] = {
            0x07, 0x23, 0x23, 0x2b, 0x9b, 0x77, 0xff, 0x77, 0x9b, 0xff, 0xff, 0x77, 0x9b, 0x77, 0xff, 0x77,
        };
        pix_t *block;
        pix_t *blockin;
        int sad, mpred, mode;
        int r = n >> 2;
        int c = n & 3;
        int8_t *ctx_l = (int8_t *)enc->i4x4mode + r;
        int8_t *ctx_t = (int8_t *)enc->i4x4mode + 4 + enc->mb.x*4 + c;
        edge = ((pix_t*)edge_store) + 3 + 16 + 1 + 4*c - 4*r;

        a = avail;
        a &= block2avail[n];
        a |= block2avail[n] >> 4;

        if (!(block2avail[n] & AVAIL_TL)) // TL replace
        {
            if ((n <= 3 && (avail & AVAIL_T)) ||
                (n  > 3 && (avail & AVAIL_L)))
            {
                a |= AVAIL_TL;
            }
        }
        if (n < 3 && (avail & AVAIL_T))
        {
            a |= AVAIL_TR;
        }

        blockin = enc->scratch->mb_pix_inp + (c + r*16)*4;
        block = dec + (c + r*16)*4;

        mpred = MIN(*ctx_l, *ctx_t);
        if (mpred < 0)
        {
            mpred = 2;
        }

        sad = h264e_intra_choose_4x4(blockin, block, a, edge, mpred, MUL_LAMBDA(3, g_lambda_q4[enc->rc.qp]));
        mode = sad & 15;
        sad >>= 4;

        *ctx_l = *ctx_t = (int8_t)mode;
        if (mode == mpred)
        {
            mode = -1;
        } else if (mode > mpred)
        {
            mode--;
        }
        enc->mb.i4x4_mode[n] = (int8_t)mode;

        nz_mask <<= 1;
        if (sad > g_skip_thr_i4x4[enc->rc.qp])
        {
            //  skip transform on low SAD gains just about 2% for all-intra coding at QP40,
            //  for other QP gain is minimal, so SAD check do not used
            nz_mask |= h264e_transform_sub_quant_dequant(blockin, block, 16, QDQ_MODE_INTRA_4, qv->qy + n, enc->rc.qdat[0]);

            if (nz_mask & 1)
            {
                h264e_transform_add(block, 16, block, qv->qy + n, 1, ~0);
            }
        } else
        {
            memset((qv->qy+n), 0, sizeof(qv->qy[0]));
        }

        cost += sad;

        edge[2] = block[3];
        edge[1] = block[3 + 16];
        edge[0] = block[3 + 16*2];
        *(uint32_t*)&edge[-4] = *(uint32_t*)&block[16*3];
    }
    enc->scratch->nz_mask = (uint16_t)nz_mask;

    if (cost < enc->mb.cost)
    {
        enc->mb.cost = cost;
        enc->mb.type = 5;   // intra 4x4
        h264e_copy_16x16(mb_dec, enc->dec.stride[0], dec, 16);  // restore reference
    }
}

/**
*   Choose 16x16 prediction mode, most suitable for given gradient
*/
static int intra_estimate_16x16(pix_t *p, int s, int avail, int qp)
{
    static const uint8_t mode_i16x16_valid[8] = { 4, 5, 6, 7, 4, 5, 6, 15 };
    pix_t p00 = p[0];
    pix_t p01 = p[15];
    pix_t p10 = p[15*s + 0];
    pix_t p11 = p[15*s + 15];
    int v = mode_i16x16_valid[avail & (AVAIL_T + AVAIL_L + AVAIL_TL)];
    // better than above on low bitrates
    int dx = ABS(p00 - p01) + ABS(p10 - p11) + ABS(p[8*s] - p[8*s + 15]);
    int dy = ABS(p00 - p10) + ABS(p01 - p11) + ABS(p[8] - p[15*s + 8]);

    if ((dx > 30 + 3*dy && dy < (100 + 50 - qp)
        //|| (/*dx < 50 &&*/ dy <= 12)
        ) && (v & 1))
        return 0;
    else if (dy > 30 + 3*dx && dx < (100 + 50 - qp) && (v & (1 << 1)))
        return 1;
    else
        return 2;
}

/**
*   Estimate cost of 16x16 intra predictor
*
*   for foreman@qp10
*
*   12928 - [0-3], [0]
*   12963 - [0-2], [0]
*   12868 - [0-2], [0-3]
*   12878 - [0-2], [0-2]
*   12834 - [0-3], [0-3]
*sad
*   13182
*heuristic
*   13063
*
*/
static void intra_choose_16x16(h264e_enc_t *enc, pix_t *left, pix_t *top, int avail)
{
    int sad, sad4[4];
    // heuristic mode decision
    enc->mb.i16.pred_mode_luma = intra_estimate_16x16(enc->scratch->mb_pix_inp, 16, avail, enc->rc.qp);

    // run chosen predictor
    h264e_intra_predict_16x16(enc->ptest, left, top, enc->mb.i16.pred_mode_luma);

    // coding cost
    sad = h264e_sad_mb_unlaign_8x8(enc->scratch->mb_pix_inp, 16, enc->ptest, sad4)        // SAD
        + MUL_LAMBDA(bitsize_ue(enc->mb.i16.pred_mode_luma + 1), g_lambda_q4[enc->rc.qp]) // side-info penalty
        + g_lambda_i16_q4[enc->rc.qp];                                                    // block kind penalty

    if (sad < enc->mb.cost)
    {
        enc->mb.cost = sad;
        enc->mb.type = 6;
        SWAP(pix_t*, enc->pbest, enc->ptest);
    }
}

/************************************************************************/
/*      Inter mode encoding                                             */
/************************************************************************/

/**
*   Sub-pel luma interpolation
*/
static void interpolate_luma(const pix_t *ref, int stride, point_t mv, point_t wh, pix_t *dst)
{
    ref += (mv.s.y >> 2) * stride + (mv.s.x >> 2);
    mv.u32 &= 0x000030003;
    h264e_qpel_interpolate_luma(ref, stride, dst, wh, mv);
}

/**
*   Sub-pel chroma interpolation
*/
static void interpolate_chroma(h264e_enc_t *enc, point_t mv)
{
    int i;
    for (i = 1; i < 3; i++)
    {
        point_t wh;
        int part = 0, x = 0, y = 0;
        wh.s.x = (enc->mb.type & 2) ? 4 : 8;
        wh.s.y = (enc->mb.type & 1) ? 4 : 8;
        if (enc->mb.type == -1) // skip
        {
            wh.s.x = wh.s.y = 8;
        }

        for (;;part++)
        {
            pix_t *ref;
            mv = mb_abs_mv(enc, enc->mb.mv[part]);
            ref = enc->ref.yuv[i] + ((mv.s.y >> 3) + y)*enc->ref.stride[i] + (mv.s.x >> 3) + x;
            mv.u32 &= 0x00070007;
            h264e_qpel_interpolate_chroma(ref, enc->ref.stride[i], enc->ptest + (i - 1)*8 + 16*y + x, wh, mv);
            x = (x + wh.s.x) & 7;
            if (!x)
            {
                y = (y + wh.s.y) & 7;
                if (!y)
                {
                    break;
                }
            }
        }
    }
}

/**
*   RD cost of given MV
*/
static int me_mv_cost(point_t mv, point_t mv_pred, int qp)
{
    int nb = bits_se(mv.s.x - mv_pred.s.x) + bits_se(mv.s.y - mv_pred.s.y);
    return MUL_LAMBDA(nb, g_lambda_mv_q4[qp]);
}

/**
*   RD cost of given MV candidate (TODO)
*/
#define me_mv_cand_cost me_mv_cost
//static int me_mv_cand_cost(point_t mv, point_t mv_pred, int qp)
//{
//    int nb = bits_se(mv.s.x - mv_pred.s.x) + bits_se(mv.s.y - mv_pred.s.y);
//    return MUL_LAMBDA(nb, g_lambda_mv_q4[qp]);
//}


/**
*   Modified full-pel motion search with small diamond algorithm
*   note: diamond implemented with small modifications, trading speed for precision
*/
static int me_search_diamond(h264e_enc_t *enc, const pix_t *ref, const pix_t *b, int rowbytes, point_t *mv,
    const rectangle_t *range, int qp, point_t mv_pred, int min_sad, point_t wh, pix_t *scratch, pix_t **ppbest, int store_bytes)
{
    // cache map           cache moves
    //      3              0   x->1
    //      *              1   x->0
    //  1 * x * 0          2   x->3
    //      *              3   x->2
    //      2                   ^1

    //   cache double moves:
    //           prev               prev
    //      x ->   0   ->   3   ==>   3   =>   1
    //      x ->   0   ->   2   ==>   2   =>   1
    //      x ->   0   ->   0   ==>   0   =>   1
    //      x ->   0   ->   1   - impossible
    //   prev SAD(n) is (n+4)
    //

    static const point_t dir2mv[] = {{{4, 0}},{{-4, 0}},{{0, 4}},{{0, -4}}};
    union
    {
        uint16_t cache[8];
        uint32_t cache32[4];
    } sad;

    int dir, cloop, dir_prev, cost;
    point_t v;

    assert(mv_in_rect(*mv, range));

restart:
    dir = 0;                // start gradient descend with direction dir2mv[0]
    cloop = 4;              // try 4 directions
    dir_prev = -1;          // not yet moved

    // reset SAD cache
    sad.cache32[0] = sad.cache32[1] = sad.cache32[2] = sad.cache32[3] = ~0u;

    // 1. Full-pel ME with small diamond modification:
    // center point moved immediately as soon as new minimum found
    do
    {
        assert(dir >= 0 && dir < 4);

        // Try next point. Avoid out-of-range moves
        v = mv_add(*mv, dir2mv[dir]);
        //if (mv_in_rect(v, range) && sad.cache[dir] == (uint16_t)~0u)
        if (mv_in_rect(v, range) && sad.cache[dir] == 0xffffu)
        {
            cost = h264e_sad_mb_unlaign_wh(ref + ((v.s.y*rowbytes + v.s.x) >> 2), rowbytes, b, wh);
            //cost += me_mv_cost(*mv, mv_pred, qp);
            cost += me_mv_cost(v, mv_pred, qp);
            sad.cache[dir] = (uint16_t)cost;
            if (cost < min_sad)
            {
                // This point is better than center: move this point to center and continue
                int corner = ~0;
                if (dir_prev >= 0)                      // have previous move
                {                                       // save cache point, which can be used in next iteration
                    corner = sad.cache[4 + dir];        // see "cache double moves" above
                }
                sad.cache32[2] = sad.cache32[0];        // save current cache to 'previous'
                sad.cache32[3] = sad.cache32[1];
                sad.cache32[0] = sad.cache32[1] = ~0u;  // reset current cache
                if (dir_prev >= 0)                      // but if have previous move
                {                                       // one cache point can be reused from previous iteration
                    sad.cache[dir_prev^1] = (uint16_t)corner; // see "cache double moves" above
                }
                sad.cache[dir^1] = (uint16_t)min_sad;   // previous center become a neighbor's
                dir_prev = dir;                         // save this direction
                dir--;                                  // start next iteration with the same direction
                cloop = 4 + 1;                          // and try 4 directions (+1 for do-while loop)
                *mv = v;                                // Save best point found
                min_sad = cost;                         // and it's SAD
            }
        }
        dir = (dir + 1) & 3;                            // cycle search directions
    } while(--cloop);

    // 2. Optional: Try diagonal step
    //if (1)
    {
        int primary_dir   = sad.cache[3] >= sad.cache[2] ? 2 : 3;
        int secondary_dir = sad.cache[1] >= sad.cache[0] ? 0 : 1;
        if (sad.cache[primary_dir] < sad.cache[secondary_dir])
        {
            SWAP(int, secondary_dir, primary_dir);
        }

        v = mv_add(dir2mv[secondary_dir], dir2mv[primary_dir]);
        v = mv_add(*mv, v);
        //cost = (uint16_t)~0u;
        if (mv_in_rect(v, range))
        {
            cost = h264e_sad_mb_unlaign_wh(ref + ((v.s.y*rowbytes + v.s.x) >> 2), rowbytes, b, wh);
            cost += me_mv_cost(v, mv_pred, qp);
            if (cost < min_sad)
            {
                *mv = v;//mv_add(*mv, v);
                min_sad = cost;
                goto restart;
            }
        }
    }

    interpolate_luma(ref, rowbytes, *mv, wh, scratch);    // Plain NxM copy can be used
    *ppbest = scratch;

    // 3. Fractional pel search
    if (enc->run_param.encode_speed < 9 && mv_in_rect(*mv, &enc->frame.mv_qpel_limit))
    {
        point_t vbest = *mv;
        pix_t *pbest = scratch;
        pix_t *hpel  = scratch + store_bytes;
        pix_t *hpel1 = scratch + ((store_bytes == 8) ? 256 : 2*store_bytes);
        pix_t *hpel2 = hpel1 + store_bytes;

        int i, sad_test;
        point_t primary_qpel, secondary_qpel, vdiag;

        unsigned minsad1 = sad.cache[1];
        unsigned minsad2 = sad.cache[3];
        secondary_qpel = point(-1, 0);
        primary_qpel = point(0, -1);
        if (sad.cache[3] >= sad.cache[2])
            primary_qpel = point(0, 1), minsad2 = sad.cache[2];
        if (sad.cache[1] >= sad.cache[0])
            secondary_qpel = point(1, 0), minsad1 = sad.cache[0];

        if (minsad2 > minsad1)
        {
            SWAP(point_t, secondary_qpel, primary_qpel);
        }

        //     ============> primary
        //     |00 01 02
        //     |10 11 12
        //     |20    22
        //     V
        //     secondary
        vdiag = mv_add(primary_qpel, secondary_qpel);

        for (i = 0; i < 7; i++)
        {
            pix_t *ptest;
            switch(i)
            {
            case 0:
                // 02 = interpolate primary half-pel
                v = mv_add(*mv, mv_add(primary_qpel, primary_qpel));
                interpolate_luma(ref, rowbytes, v, wh, ptest = hpel1);
                break;
            case 1:
                // 01 q-pel = (00 + 02)/2
                v = mv_add(*mv, primary_qpel);
                h264e_qpel_average_wh_align(scratch, hpel1, ptest = hpel, wh);
                break;
            case 2:
                // 20 = interpolate secondary half-pel
                v = mv_add(*mv, mv_add(secondary_qpel, secondary_qpel));
                interpolate_luma(ref, rowbytes, v, wh, ptest = hpel2);
                break;
            case 3:
                // 10 q-pel = (00 + 20)/2
                hpel  = scratch + store_bytes; if (pbest == hpel) hpel = scratch;
                v = mv_add(*mv, secondary_qpel);
                h264e_qpel_average_wh_align(scratch, hpel2, ptest = hpel, wh);
                break;
            case 4:
                // 11 q-pel = (02 + 20)/2
                hpel  = scratch + store_bytes; if (pbest == hpel) hpel = scratch;
                v = mv_add(*mv, vdiag);
                h264e_qpel_average_wh_align(hpel1, hpel2, ptest = hpel, wh);
                break;
            case 5:
                // 22 = interpolate center half-pel
                if (pbest == hpel2) hpel2 = scratch, hpel = scratch + store_bytes;
                v = mv_add(*mv, mv_add(vdiag, vdiag));
                interpolate_luma(ref, rowbytes, v, wh, ptest = hpel2);
                break;
            case 6:
            default:
                // 12 q-pel = (02 + 22)/2
                hpel  = scratch + store_bytes; if (pbest == hpel) hpel = scratch;
                v = mv_add(*mv, mv_add(primary_qpel, vdiag));
                h264e_qpel_average_wh_align(hpel2, hpel1, ptest = hpel, wh);
                break;
            }

            sad_test = h264e_sad_mb_unlaign_wh(ptest, 16, b, wh) + me_mv_cost(v, mv_pred, qp);
            if (sad_test < min_sad)
            {
                min_sad = sad_test;
                vbest = v;
                pbest = ptest;
            }
        }

        *mv = vbest;
        *ppbest = pbest;
    }
    return min_sad;
}

/**
*   Set range for MV search
*/
static void me_mv_set_range(point_t *pnt, rectangle_t *range, const rectangle_t *mv_limit, int mby)
{
    // clip start point
    rectangle_t r = *mv_limit;
    r.tl.s.y = (int16_t)(MAX(r.tl.s.y, mby - 63*4));
    r.br.s.y = (int16_t)(MIN(r.br.s.y, mby + 63*4));
    mv_clip(pnt, &r);
    range->tl = mv_add(*pnt, point(-MV_RANGE*4, -MV_RANGE*4));
    range->br = mv_add(*pnt, point(+MV_RANGE*4, +MV_RANGE*4));
    // clip search range
    mv_clip(&range->tl, &r);
    mv_clip(&range->br, &r);
}

/**
*   Remove duplicates from MV candidates list
*/
static int me_mv_refine_cand(point_t *p, int n)
{
    int i, j, k;
    p[0] = mv_round_qpel(p[0]);
    for (j = 1, k = 1; j < n; j++)
    {
        point_t mv = mv_round_qpel(p[j]);
        for (i = 0; i < k; i++)
        {
            // TODO
            //if (!mv_differs3(mv, p[i], 3*4))
            //if (!mv_differs3(mv, p[i], 1*4))
            //if (!mv_differs3(mv, p[i], 3))
            if (mv_equal(mv, p[i]))
                break;
        }
        if (i == k)
            p[k++] = mv;
    }
    return k;
}

/**
*   Choose candidates for inter MB partitioning (16x8,8x16 or 8x8),
*   using SAD's for 8x8 sub-blocks
*/
static void mb_inter_partition(/*const */int sad[4], int mode[4])
{
/*
    slope
        |[ 1  1]| _ |[ 1 -1]|
        |[-1 -1]|   |[ 1 -1]|
        indicates v/h gradient: big negative = vertical prediction; big positive = horizontal

    skew
        |[ 1  0]| _ |[ 0 -1]|
        |[ 0 -1]|   |[ 1  0]|
        indicates diagonal gradient: big negative = diagonal down right
*/
    int p00 = sad[0];
    int p01 = sad[1];
    int p10 = sad[2];
    int p11 = sad[3];
    int sum = p00 + p01 + p10 + p11;
    int slope = ABS((p00 - p10) + (p01 - p11)) - ABS((p00 - p01) + (p10 - p11));
    int skew = ABS(p11 - p00) - ABS(p10 - p01);

    if (slope >  (sum >> 4))
    {
        mode[1] = 1;    // try 8x16 partition
    }
    if (slope < -(sum >> 4))
    {
        mode[2] = 1;    // try 16x8 partition
    }
    if (ABS(skew) > (sum >> 4) && ABS(slope) <= (sum >> 4))
    {
        mode[3] = 1;    // try 8x8 partition
    }
}

/**
*   Online MV clustering to "long" and "short" clusters
*   Estimate mean "long" and "short" vectors
*/
static void mv_clusters_update(h264e_enc_t *enc, point_t mv)
{
    int mv_norm = SQRP(mv);
    int n0 = SQRP(enc->mv_clusters[0]);
    int n1 = SQRP(enc->mv_clusters[1]);
    if (mv_norm < n1)
    {
        // "short" is shorter than "long"
        SMOOTH(enc->mv_clusters[0], mv);
    }
    if (mv_norm >= n0)
    {
        // "long" is longer than "short"
        SMOOTH(enc->mv_clusters[1], mv);
    }
}

/**
*   Choose inter mode: skip/coded, ME partition, find MV
*/
static void inter_choose_mode(h264e_enc_t *enc)
{
    int prefered_modes[4] = { 1, 0, 0, 0 };
    point_t mv_skip, mv_skip_a, mv_cand[MAX_MV_CAND];
    point_t mv_pred_16x16 = me_mv_medianpredictor_get_skip(enc);
    point_t mv_best = point(MV_NA, 0); // avoid warning

    int sad, sad_skip = 0x7FFFFFFF, sad_best = 0x7FFFFFFF;
    int off, i, j = 0, ncand = 0;
    int cand_sad4[MAX_MV_CAND][4];
    const pix_t *ref_yuv = enc->ref.yuv[0];
    int ref_stride = enc->ref.stride[0];
    int mv_cand_cost_best = 0;
    mv_skip = enc->mb.mv_skip_pred;
    mv_skip_a = mb_abs_mv(enc, mv_skip);

    for (i = 0; i < 4; i++)
    {
        enc->df.df_mv[4 + 5*i].u32 = enc->mv_pred[i].u32;
        enc->df.df_mv[i].u32       = enc->mv_pred[8 + 4*enc->mb.x + i].u32;
    }

    // Try skip mode
    if (mv_in_rect(mv_skip_a, &enc->frame.mv_qpel_limit))
    {
        int *sad4 = cand_sad4[0];
        interpolate_luma(ref_yuv, ref_stride, mv_skip_a, point(16, 16), enc->ptest);
        sad_skip = h264e_sad_mb_unlaign_8x8(enc->scratch->mb_pix_inp, 16, enc->ptest, sad4);

        if (MAX(MAX(sad4[0], sad4[1]), MAX(sad4[2], sad4[3])) < g_skip_thr_inter[enc->rc.qp])
        {
            int uv, sad_uv;

            SWAP(pix_t*, enc->pbest, enc->ptest);
            enc->mb.type = -1;
            enc->mb.mv[0] = mv_skip;
            enc->mb.cost = 0;
            interpolate_chroma(enc, mv_skip_a);

            // Check that chroma SAD is not too big for the skip
            for (uv = 1; uv <= 2; uv++)
            {
                pix_t *pred = enc->ptest + (uv - 1)*8;
                pix_t *pix_mb_uv = mb_input_chroma(enc, uv);
                int inp_stride = enc->inp.stride[uv];

                if (enc->frame.cropping_flag && ((enc->mb.x + 1)*16  > enc->param.width || (enc->mb.y + 1)*16  > enc->param.height))
                {
                    // Speculative read beyond frame borders: make local copy of the macroblock.
                    // TODO: same code used in mb_write() and mb_encode()
                    pix_copy_cropped_mb(enc->scratch->mb_pix_store, 8, pix_mb_uv, enc->inp.stride[uv],
                        MIN(8, enc->param.width/2  - enc->mb.x*8),
                        MIN(8, enc->param.height/2 - enc->mb.y*8));
                    pix_mb_uv = enc->scratch->mb_pix_store;
                    inp_stride = 8;
                }

                sad_uv = h264e_sad_mb_unlaign_wh(pix_mb_uv, inp_stride, pred, point(8, 8));
                if (sad_uv >= g_skip_thr_inter[enc->rc.qp])
                {
                    break;
                }
            }
            if (uv == 3)
            {
                return;
            }
        }

        if (enc->run_param.encode_speed < 1) // enable 8x16, 16x8 and 8x8 partitions
        {
            mb_inter_partition(sad4, prefered_modes);
        }

        //sad_skip += me_mv_cost(mv_skip, mv_pred_16x16, enc->rc.qp);

        // Too big skip SAD. Use skip predictor as a diamond start point candidate
        mv_best = mv_cand[ncand++] = mv_round_qpel(mv_skip);
        if (!((mv_skip.s.x | mv_skip.s.y) & 3))
        {
            sad_best = sad_skip;//+ me_mv_cost(mv_best, mv_pred_16x16, enc->rc.qp)
            mv_cand_cost_best = me_mv_cand_cost(mv_skip, mv_pred_16x16, enc->rc.qp);
            //mv_cand_cost_best = me_mv_cand_cost(mv_skip, point(0,0), enc->rc.qp);
            j = 1;
        }
    }

    mv_cand[ncand++] = mv_pred_16x16;
    ncand += me_mv_medianpredictor_get_cand(enc, mv_cand + ncand);

    if (enc->mb.x <= 0)
    {
        mv_cand[ncand++] = point(8*4, 0);
    }
    if (enc->mb.y <= 0)
    {
        mv_cand[ncand++] = point(0, 8*4);
    }

    mv_cand[ncand++] = enc->mv_clusters[0];
    mv_cand[ncand++] = enc->mv_clusters[1];

    assert(ncand <= MAX_MV_CAND);
    ncand = me_mv_refine_cand(mv_cand, ncand);

    for (/*j = 0*/; j < ncand; j++)
    {
        point_t mv = mb_abs_mv(enc, mv_cand[j]);
        if (mv_in_rect(mv, &enc->frame.mv_limit))
        {
            int mv_cand_cost = me_mv_cand_cost(mv_cand[j], mv_pred_16x16, enc->rc.qp);

            int *sad4 = cand_sad4[j];
            off = ((mv.s.y + 0) >> 2)*ref_stride + ((mv.s.x + 0) >> 2);
            sad = h264e_sad_mb_unlaign_8x8(ref_yuv + off, ref_stride, enc->scratch->mb_pix_inp, sad4);

            if (enc->run_param.encode_speed < 1) // enable 8x16, 16x8 and 8x8 partitions
            {
                mb_inter_partition(sad4, prefered_modes);
            }

            if (sad + mv_cand_cost < sad_best + mv_cand_cost_best)
            //if (sad < sad_best)
            {
                mv_cand_cost_best = mv_cand_cost;
                sad_best = sad;
                mv_best = mv_cand[j];
            }
        }
    }

    sad_best += me_mv_cost(mv_best, mv_pred_16x16, enc->rc.qp);

    {
        int mb_type;
        point_t wh, part, mvpred_ctx[12], part_mv[4][16], part_mvd[4][16];
        pix_t *store = enc->scratch->mb_pix_store;
        pix_t *pred_best = store, *pred_test = store + 256;

#define MAX8X8_MODES 4
        me_mv_medianpredictor_save_ctx(enc, mvpred_ctx);
        enc->mb.cost = 0xffffff;
        for (mb_type = 0; mb_type < MAX8X8_MODES; mb_type++)
        {
            static const int nbits[4] = { 1, 4, 4, 12 };
            int imv = 0;
            int part_sad = MUL_LAMBDA(nbits[mb_type], g_lambda_q4[enc->rc.qp]);

            if (!prefered_modes[mb_type]) continue;

            wh.s.x = (mb_type & 2) ? 8 : 16;
            wh.s.y = (mb_type & 1) ? 8 : 16;
            part = point(0, 0);
            for (;;)
            {
                rectangle_t range;
                pix_t *diamond_out;
                point_t mv, mv_pred, mvabs = mb_abs_mv(enc, mv_best);
                me_mv_set_range(&mvabs, &range, &enc->frame.mv_limit, enc->mb.y*16*4 + part.s.y*4);

                mv_pred = me_mv_medianpredictor_get(enc, part, wh);

                if (mb_type)
                {
                    mvabs = mv_round_qpel(mb_abs_mv(enc, mv_pred));
                    me_mv_set_range(&mvabs, &range, &enc->frame.mv_limit, enc->mb.y*16*4 + part.s.y*4);
                    off = ((mvabs.s.y >> 2) + part.s.y)*ref_stride + ((mvabs.s.x >> 2) + part.s.x);
                    sad_best = h264e_sad_mb_unlaign_wh(ref_yuv + off, ref_stride, enc->scratch->mb_pix_inp + part.s.y*16 + part.s.x, wh)
                        + me_mv_cost(mvabs,
                        //mv_pred,
                        mb_abs_mv(enc, mv_pred),
                        enc->rc.qp);
                }

                part_sad += me_search_diamond(enc, ref_yuv + part.s.y*ref_stride + part.s.x,
                    enc->scratch->mb_pix_inp + part.s.y*16 + part.s.x, ref_stride, &mvabs, &range, enc->rc.qp,
                    mb_abs_mv(enc, mv_pred), sad_best, wh,
                    store, &diamond_out, mb_type ? (mb_type == 2 ? 8 : 128) : 256);

                if (!mb_type)
                {
                    pred_test = diamond_out;
                    if (pred_test < store + 2*256)
                    {
                        pred_best = (pred_test == store ? store + 256 : store);
                        store += 2*256;
                    } else
                    {
                        pred_best = (pred_test == (store + 512) ? store + 512 + 256 : store + 512);
                    }
                } else
                {
                    h264e_copy_8x8(pred_test + part.s.y*16 + part.s.x, 16, diamond_out);
                    if (mb_type < 3)
                    {
                        int part_off = (wh.s.x >> 4)*8 + (wh.s.y >> 4)*8*16;
                        h264e_copy_8x8(pred_test + part_off + part.s.y*16 + part.s.x, 16, diamond_out + part_off);
                    }
                }

                mv = mv_sub(mvabs, point(enc->mb.x*16*4, enc->mb.y*16*4));

                part_mvd[mb_type][imv] = mv_sub(mv, mv_pred);
                part_mv[mb_type][imv++] = mv;

                me_mv_medianpredictor_put(enc, part.s.x >> 2, part.s.y >> 2, wh.s.x >> 2, wh.s.y >> 2, mv);

                part.s.x = (part.s.x + wh.s.x) & 15;
                if (!part.s.x)
                {
                    part.s.y = (part.s.y + wh.s.y) & 15;
                    if (!part.s.y) break;
                }
            }

            me_mv_medianpredictor_restore_ctx(enc, mvpred_ctx);

            if (part_sad < enc->mb.cost)
            {
                SWAP(pix_t*, pred_best, pred_test);
                enc->mb.cost = part_sad;
                enc->mb.type = mb_type;
            }
        }
        enc->pbest = pred_best;
        enc->ptest = pred_test;
        memcpy(enc->mb.mv,  part_mv [enc->mb.type], 16*sizeof(point_t));
        memcpy(enc->mb.mvd, part_mvd[enc->mb.type], 16*sizeof(point_t));

        if (enc->mb.cost > sad_skip)
        {
            enc->mb.type = 0;
            enc->mb.cost = sad_skip + me_mv_cand_cost(mv_skip, mv_pred_16x16, enc->rc.qp);
            enc->mb.mv [0] = mv_skip;
            enc->mb.mvd[0] = mv_sub(mv_skip, mv_pred_16x16);

            assert(mv_in_rect(mv_skip_a, &enc->frame.mv_qpel_limit)) ;
            interpolate_luma(ref_yuv, ref_stride, mv_skip_a, point(16, 16), enc->pbest);
            interpolate_chroma(enc, mv_skip_a);
        }
    }
}

/************************************************************************/
/*      Deblock filter                                                  */
/************************************************************************/
#define MB_FLAG_SVC_INTRA 1
#define MB_FLAG_SLICE_START_DEBLOCK_2 2

/**
*   Set deblock filter strength
*/
static void df_strength(deblock_filter_t *df, int mb_type, int mbx, uint8_t *strength, int IntraBLFlag)
{
    uint8_t *sv = strength;
    uint8_t *sh = strength + 16;
    int flag = df->nzflag;
    df->df_nzflag[mbx] = (uint8_t)(flag >> 20);
    /*
        nzflag represents macroblock and it's neighbors with 24 bit flags:
        0 1 2 3
      4 5 6 7 8
      A B C D E
      F G H I J
      K L K N O
    */
    (void)IntraBLFlag;
#if H264E_SVC_API
    if (IntraBLFlag & MB_FLAG_SVC_INTRA)
    {
        int ccloop = 4;
        do
        {
            int cloop = 4;
            do
            {
                int v = 0;
                if (flag & 3 << 4)
                {
                    v = 1;
                }

                *sv = (uint8_t)v; sv += 4;

                v = 0;
                if (flag & 33)
                {
                    v = 1;
                }

                *sh++ = (uint8_t)v;

                flag >>= 1;

            } while(--cloop);
            flag >>= 1;
            sv -= 15;

        } while(--ccloop);
    } else
#endif
    {
        if (mb_type < 5)
        {
            int ccloop = 4;
            point_t *mv = df->df_mv;
            do
            {
                int cloop = 4;
                do
                {
                    int v = 0;
                    if (flag & 3 << 4)
                    {
                        v = 2;
                    } else if (mv_differs3(mv[4], mv[5]))
                    {
                        v = 1;
                    }
                    *sv = (uint8_t)v; sv += 4;

                    v = 0;
                    if (flag & 33)
                    {
                        v = 2;
                    } else if (mv_differs3(mv[0], mv[5]))
                    {
                        v = 1;
                    }
                    *sh++ = (uint8_t)v;

                    flag >>= 1;
                    mv++;
                } while(--cloop);
                flag >>= 1;
                sv -= 15;
                mv++;
            } while(--ccloop);
        } else
        {
            // Deblock mode #3 (intra)
            ((uint32_t*)(sv))[1] = ((uint32_t*)(sv))[2] = ((uint32_t*)(sv))[3] =             // for inner columns
            ((uint32_t*)(sh))[1] = ((uint32_t*)(sh))[2] = ((uint32_t*)(sh))[3] = 0x03030303; // for inner rows
        }
        if ((mb_type >= 5 || df->mb_type[mbx - 1] >= 5)) // speculative read
        {
            ((uint32_t*)(strength))[0] = 0x04040404;    // Deblock mode #4 (strong intra) for left column
        }
        if ((mb_type >= 5 || df->mb_type[mbx    ] >= 5))
        {
            ((uint32_t*)(strength))[4] = 0x04040404;    // Deblock mode #4 (strong intra) for top row
        }
    }
    df->mb_type[mbx] = (int8_t)mb_type;
}

/**
*   Run deblock for current macroblock
*/
static void mb_deblock(deblock_filter_t *df, int mb_type, int qp_this, int mbx, int mby, H264E_io_yuv_t *mbyuv, int IntraBLFlag)
{
    int i, cr, qp, qp_left, qp_top;
    deblock_params_t par;
    uint8_t *alpha = par.alpha; //[2*2];
    uint8_t *beta  = par.beta;  //[2*2];
    uint32_t *strength32  = par.strength32; //[4*2]; // == uint8_t strength[16*2];
    uint8_t *strength = (uint8_t *)strength32;
    uint8_t *tc0 = par.tc0; //[16*2];

    df_strength(df, mb_type, mbx, strength, IntraBLFlag);
    if (!mbx || (IntraBLFlag & MB_FLAG_SLICE_START_DEBLOCK_2))
    {
        strength32[0] = 0;
    }

    if (!mby)
    {
        strength32[4] = 0;
    }

    qp_top = df->df_qp[mbx];
    qp_left = df->df_qp[mbx - 1];
    df->df_qp[mbx] = (uint8_t)qp_this;

    cr = 0;
    for (;;)
    {
        const uint8_t *lut;
        if (*((uint32_t*)strength))
        {
            qp = (qp_left + qp_this + 1) >> 1;
            lut = g_a_tc0_b[-10 + qp + ALPHA_OFS];
            alpha[0] = lut[0];
            beta[0]  = lut[4 + (BETA_OFS - ALPHA_OFS)*5];
            for (i = 0; i < 4; i++) tc0[i] = lut[strength[i]];
        }
        if (*((uint32_t*)(strength + 16)))
        {
            qp = (qp_top + qp_this + 1) >> 1;
            lut = g_a_tc0_b[-10 + qp + ALPHA_OFS];

            alpha[2]  = lut[0];
            beta[2] = lut[4 + (BETA_OFS - ALPHA_OFS)*5];
            for (i = 0; i < 4; i++) tc0[16 + i] = lut[strength[16 + i]];
        }

        lut = g_a_tc0_b[-10 + qp_this + ALPHA_OFS];
        alpha[3] = alpha[1] = lut[0];
        beta[3] = beta[1] = lut[4 + (BETA_OFS - ALPHA_OFS)*5];
        for (i = 4; i < 16; i++)
        {
            tc0[i] = lut[strength[i]];
            tc0[16 + i] = lut[strength[16 + i]];
        }
        if (cr)
        {
            int *t = (int *)tc0;
            t[1] = t[2];         // TODO: need only for OMX
            t[5] = t[6];
            i = 2;
            do
            {
                h264e_deblock_chroma(mbyuv->yuv[i], mbyuv->stride[i], &par);
            } while (--i);
            break;
        }
        h264e_deblock_luma(mbyuv->yuv[0], mbyuv->stride[0], &par);

        qp_this = qpy2qpc[qp_this + DQP_CHROMA];
        qp_left = qpy2qpc[qp_left + DQP_CHROMA];
        qp_top = qpy2qpc[qp_top + DQP_CHROMA];
        cr++;
    }
}

/************************************************************************/
/*      Macroblock encoding                                             */
/************************************************************************/
/**
*   Macroblock encoding
*/
static void mb_encode(h264e_enc_t *enc, int enc_type)
{
    pix_t *top = enc->top_line + 48 + enc->mb.x*32;
    pix_t *left = enc->top_line;
    int avail = enc->mb.avail = mb_avail_flag(enc);
    int base_mode = 0;

    if (enc->frame.cropping_flag && ((enc->mb.x + 1)*16 > enc->param.width || (enc->mb.y + 1)*16 > enc->param.height))
    {
        pix_copy_cropped_mb(enc->scratch->mb_pix_inp, 16, mb_input_luma(enc), enc->inp.stride[0],
             MIN(16, enc->param.width  - enc->mb.x*16),
             MIN(16, enc->param.height - enc->mb.y*16));
    } else
    {
        // cache input macroblock
        h264e_copy_16x16(enc->scratch->mb_pix_inp, 16, mb_input_luma(enc), enc->inp.stride[0]);
    }

    if (!(avail & AVAIL_L)) left = NULL;
    if (!(avail & AVAIL_T)) top  = NULL;

    enc->pbest = enc->scratch->mb_pix_store;
    enc->ptest = enc->pbest + 256;
    enc->mb.type = 0;
    enc->mb.cost = 0x7FFFFFFF;

    if (enc->slice.type == SLICE_TYPE_P)
    {
        inter_choose_mode(enc);
    }
#if H264E_SVC_API
    else if (enc_type > 0 && enc->param.inter_layer_pred_flag)
    {
        base_mode = 1;
        enc->mb.type = 6;
        h264e_copy_16x16(enc->pbest, 16, (enc->ref.yuv[0] + (enc->mb.x + enc->mb.y*enc->ref.stride[0])*16), enc->ref.stride[0]);
        h264e_copy_8x8_s(enc->ptest, 16, (enc->ref.yuv[1] + (enc->mb.x + enc->mb.y*enc->ref.stride[1])*8), enc->ref.stride[1]);
        h264e_copy_8x8_s(enc->ptest + 8, 16, (enc->ref.yuv[2] + (enc->mb.x + enc->mb.y*enc->ref.stride[2])*8), enc->ref.stride[2]);

        goto _WRITE_MB;
    }
#endif

    if (enc->mb.type >= 0)
    {
        intra_choose_16x16(enc, left, top, avail);
        if (enc->run_param.encode_speed < 2 || enc->slice.type != SLICE_TYPE_P) // enable intra4x4 on P slices
        {
            intra_choose_4x4(enc);
        }
    }

    if (enc->mb.type < 5)
    {
        mv_clusters_update(enc, enc->mb.mv[0]);
    }

    if (enc->mb.type >= 5)
    {
        pix_t *pred = enc->ptest;
        h264e_intra_predict_chroma(pred, left + 16, top + 16, enc->mb.i16.pred_mode_luma);
    } else
    {
        interpolate_chroma(enc, mb_abs_mv(enc, enc->mb.mv[0]));
    }

#if H264E_SVC_API
_WRITE_MB:
#endif
    mb_write(enc, enc_type, base_mode);

    if (!enc->speed.disable_deblock)
    {
        int mbx = enc->mb.x;
        int mby = enc->mb.y;
#if H264E_MAX_THREADS
        if (enc->param.max_threads > 1)
        {   // Avoid deblock across slice border
            if (enc->mb.num < enc->slice.start_mb_num + enc->frame.nmbx)
                mby = 0;
            if (enc->mb.num == enc->slice.start_mb_num)
            {
                base_mode |= MB_FLAG_SLICE_START_DEBLOCK_2;
            }
        }
#endif
        mb_deblock(&enc->df, enc->mb.type, enc->rc.prev_qp, mbx, mby, &enc->dec, base_mode);
    }
}


/************************************************************************/
/*      Rate-control                                                    */
/************************************************************************/

/**
*   @return zero threshold for given rounding offset
*/
static uint16_t rc_rnd2thr(int round, int q)
{
    int b, thr = 0;
    for (b = 0x8000; b; b >>= 1)
    {
        int t = (thr | b)*q;
        if (t <= 0x10000 - round)  // TODO: error: < !!!!!!!
        {
            thr |= b;
        }
    }
    return (uint16_t)thr;
}

/**
*   Set quantizer constants (deadzone and rounding) for given QP
*/
static void rc_set_qp(h264e_enc_t *enc, int qp)
{
    qp = MIN(qp, enc->run_param.qp_max);
    qp = MAX(qp, enc->run_param.qp_min);
    qp = MIN(qp, 51);   // avoid VC2010 static analyzer warning

    if (enc->rc.qp != qp)
    {
        static const int16_t g_quant_coeff[6*6] =
        {
            //    0         2         1
            13107, 10, 8066, 13, 5243, 16,
            11916, 11, 7490, 14, 4660, 18,
            10082, 13, 6554, 16, 4194, 20,
             9362, 14, 5825, 18, 3647, 23,
             8192, 16, 5243, 20, 3355, 25,
             7282, 18, 4559, 23, 2893, 29
            // 0 2 0 2
            // 2 1 2 1
            // 0 2 0 2
            // 2 1 2 1
        };

        int cloop = 2;
        enc->rc.qp = qp;

        do
        {
            uint16_t *qdat0 = enc->rc.qdat[2 - cloop];
            uint16_t *qdat  = enc->rc.qdat[2 - cloop];
            int qp_div6 = qp*86 >> 9;
            int qp_mod6 = qp - qp_div6*6;
            const int16_t *quant_coeff = g_quant_coeff + qp_mod6*6; // TODO: need calculate qp%6*6
            int i = 3;

            // Quant/dequant multiplier
            do
            {
                *qdat++ = *quant_coeff++ << 1 >> qp_div6;
                *qdat++ = *quant_coeff++ << qp_div6;
            } while(--i);

            // quantizer deadzone for P & chroma
            *qdat++ = enc->slice.type == SLICE_TYPE_P ? g_rnd_inter[qp] : g_deadzonei[qp];
            // quantizer deadzone for I
            *qdat++ = g_deadzonei[qp];

            *qdat++ = g_thr_inter[qp]  - 0x7fff;
            *qdat++ = g_thr_inter2[qp] - 0x7fff;

            qdat[0] = qdat[2] = rc_rnd2thr(g_thr_inter[qp] - 0x7fff, qdat0[0]);
            qdat[1] = qdat[3] =
            qdat[4] = qdat[6] = rc_rnd2thr(g_thr_inter[qp] - 0x7fff, qdat0[2]);
            qdat[5] = qdat[7] = rc_rnd2thr(g_thr_inter[qp] - 0x7fff, qdat0[4]);
            qdat += 8;
            qdat[0] = qdat[2] = rc_rnd2thr(g_thr_inter2[qp] - 0x7fff, qdat0[0]);
            qdat[1] = qdat[3] =
            qdat[4] = qdat[6] = rc_rnd2thr(g_thr_inter2[qp] - 0x7fff, qdat0[2]);
            qdat[5] = qdat[7] = rc_rnd2thr(g_thr_inter2[qp] - 0x7fff, qdat0[4]);
            qdat += 8;
            qdat[0] = qdat[2] = qdat0[0];
            qdat[1] = qdat[3] =
            qdat[4] = qdat[6] = qdat0[2];
            qdat[5] = qdat[7] = qdat0[4];
            qdat += 8;
            qdat[0] = qdat[2] = qdat0[1];
            qdat[1] = qdat[3] =
            qdat[4] = qdat[6] = qdat0[3];
            qdat[5] = qdat[7] = qdat0[5];

            qp = qpy2qpc[qp + DQP_CHROMA];
        } while (--cloop);
    }
}

/**
*   Estimate frame bit budget and QP
*
*   How bit budget allocated?
*   ~~~~~~~~~~~~~~~~~~~~~~~~~
*   1. Estimate target size of I and P macroblock, assuming same quality
*   2. Estimate I peak size
*   3. Estimate desired stationary VBV level
*
*/
static int rc_frame_start(h264e_enc_t *enc, int is_intra, int is_refers_to_long_term)
{
    unsigned np = MIN(enc->param.gop - 1u, 63u);
    int nmb = enc->frame.nmb;

    int qp = -1, add_bits, bit_budget = enc->run_param.desired_frame_bytes*8;
    int nominal_p, gop_bits, stationary_vbv_level;
    uint32_t peak_factor_q16;

    // Estimate QP
    do
    {
        qp++;
        gop_bits = bits_per_mb[0][qp]*np + bits_per_mb[1][qp];
    } while (gop_bits*nmb > (int)(np + 1)*enc->run_param.desired_frame_bytes*8 && qp < 40);

    /*
    *   desired*gop = i + p*(gop-1);   i/p = alpha;
    *   p = desired * gop / (gop-1+alpha) and i = p*alpha or i = (desired-p)*gop + p;
    */
    peak_factor_q16 = div_q16(bits_per_mb[1][qp] << 16, bits_per_mb[0][qp] << 16);
    if (np)
    {
        uint32_t ratio_q16 = div_q16((np + 1) << 16, (np << 16) + peak_factor_q16);
        nominal_p = mul32x32shr16(enc->run_param.desired_frame_bytes*8, ratio_q16);
    } else
    {
        nominal_p = 0;
    }

    stationary_vbv_level = MIN(enc->param.vbv_size_bytes*8 >> 4, enc->run_param.desired_frame_bytes*8);

    if (is_intra)
    {
        int nominal_i = mul32x32shr16(nominal_p, peak_factor_q16);
        add_bits = nominal_i - bit_budget;
    }
#if H264E_RATE_CONTROL_GOLDEN_FRAMES
    else if (is_refers_to_long_term)
    {
        int d_qp = enc->rc.max_dqp - enc->rc.dqp_smooth;
        unsigned peak_factor_golden_q16;
        int nominal_golden;
        d_qp = MAX(d_qp, 2);
        d_qp = MIN(d_qp, 12);
        d_qp = d_qp * 4 * 85 >> 8;//* 16 / 12;

        peak_factor_golden_q16 = (peak_factor_q16 - (1 << 16)) * d_qp >> 4;
        nominal_golden = nominal_p + mul32x32shr16(nominal_p, peak_factor_golden_q16);
        add_bits = nominal_golden - bit_budget;
    }
#endif
    else
    {
        add_bits = nominal_p - bit_budget;

        // drift to stationary level
        if (enc->param.vbv_size_bytes)
        {
            add_bits += (enc->rc.vbv_target_level - enc->rc.vbv_bits) >> 4;
        }
    }
    if (enc->param.vbv_size_bytes)
    {
        add_bits = MIN(add_bits, (enc->param.vbv_size_bytes*8*7 >> 3) - enc->rc.vbv_bits);
    }

    bit_budget += add_bits;
    bit_budget = MIN(bit_budget, enc->run_param.desired_frame_bytes*8*16);
    bit_budget = MAX(bit_budget, enc->run_param.desired_frame_bytes*8 >> 2);

#if H264E_RATE_CONTROL_GOLDEN_FRAMES
    if (is_intra || is_refers_to_long_term)
#else
    if (is_intra)
#endif
    {
        // Increase VBV target level due to to I-frame load: this avoids QP adaptation after I-frame
        enc->rc.vbv_target_level = enc->rc.vbv_bits + bit_budget - enc->run_param.desired_frame_bytes*8;
    }

    // Slow drift of VBV target to stationary level...
    enc->rc.vbv_target_level -= enc->run_param.desired_frame_bytes*8 - nominal_p;

    // ...until stationary level reached
    enc->rc.vbv_target_level = MAX(enc->rc.vbv_target_level, stationary_vbv_level);

    enc->rc.bit_budget = bit_budget;

    if (enc->param.fine_rate_control_flag && enc->frame.num)
    {
        qp = enc->rc.qp_smooth >> 8;
    } else
    {

#if H264E_RATE_CONTROL_GOLDEN_FRAMES
        if (is_refers_to_long_term)
        {
            for (qp = 0; qp < 42 - 1; qp++)
            {
                //if (((bits_per_mb[0][qp] + bits_per_mb[1][qp]) >> 1)*nmb < bit_budget)
                if (((bits_per_mb[0][qp] + bits_per_mb[1][qp]) >> 1)*nmb < bit_budget)
                    break;
            }
        } else
#endif
        {
            const uint16_t *bits = bits_per_mb[!!is_intra];
            for (qp = 0; qp < 42 - 1; qp++)
            {
                if (bits[qp]*nmb < bit_budget)
                {
                    break;
                }
            }
        }
        qp += MIN_QP;

#if H264E_RATE_CONTROL_GOLDEN_FRAMES
        if (is_refers_to_long_term)
        {
            int dqp = MAX(enc->rc.max_dqp, enc->rc.dqp_smooth);
            dqp  = MIN(dqp, enc->rc.dqp_smooth + 6);
            qp += dqp;
            qp = MAX(enc->rc.prev_qp, qp);
        } else
#endif
        {
            qp += enc->rc.dqp_smooth;
        }

        // If reference frame has high qp, motion compensation is less effective, so qp should be increased
        if (enc->rc.prev_qp > qp + 1)
        {
            qp = (enc->rc.prev_qp + qp + 1)/2;
        }
    }

    enc->rc.qp = 0; // force
    rc_set_qp(enc, qp);
    qp = enc->rc.qp;

    enc->rc.qp_smooth = qp << 8;
    enc->rc.prev_qp = qp;

    return (enc->rc.vbv_bits > enc->param.vbv_size_bytes*8);
}

/**
*   Update rate-control state after frame encode
*/
static void rc_frame_end(h264e_enc_t *enc, int intra_flag, int skip_flag, int is_refers_to_long_term)
{
    // 1. Update QP offset adaptive adjustment
    if (!skip_flag /*&& !is_refers_to_long_term*/)
    {
        int qp, nmb = enc->frame.nmb;
        // a posterior qp estimation
        for (qp = 0; qp != 41 && bits_per_mb[intra_flag][qp]*nmb > (int)enc->out_pos*8 - 32; qp++) {/*no action*/}

        qp += MIN_QP;

        if (!is_refers_to_long_term)
        {
            if ((enc->rc.qp_smooth >> 8) - enc->rc.dqp_smooth < qp - 1)
            {
                enc->rc.dqp_smooth--;
            } else if ((enc->rc.qp_smooth >> 8) - enc->rc.dqp_smooth > qp + 1)
            {
                enc->rc.dqp_smooth++;
            }
        }
        if (intra_flag || is_refers_to_long_term)
        {
            enc->rc.max_dqp = enc->rc.dqp_smooth;
        } else
        {
            enc->rc.max_dqp = MAX(enc->rc.max_dqp, (enc->rc.qp_smooth >> 8) - qp);
        }
    }

    // 2. Update VBV model state
    enc->rc.vbv_bits += enc->out_pos*8 - enc->run_param.desired_frame_bytes*8;

    // 3. If VBV model used, handle overflow/underflow
    if (enc->param.vbv_size_bytes)
    {
        if (enc->rc.vbv_bits < 0)       // VBV underflow
        {
            if (enc->param.vbv_underflow_stuffing_flag)
            {
                // put stuffing ('filler data')
                nal_start(enc, 12); // filler_data_rbsp
                do
                {
                    U(8, 0xFF);
                    enc->rc.vbv_bits += 8;
                } while (enc->rc.vbv_bits < 0);
                nal_end(enc);
            } else
            {
                // ignore underflow
                enc->rc.vbv_bits = 0;
            }
        }
        if (enc->rc.vbv_bits > enc->param.vbv_size_bytes*8) // VBV overflow
        {
            if (!enc->param.vbv_overflow_empty_frame_flag)
            {
                // ignore overflow
                enc->rc.vbv_bits = enc->param.vbv_size_bytes*8;
            }
        }
    } else
    {
        enc->rc.vbv_bits = 0;
    }
}

/**
*   Update rate-control state after macroblock encode, set QP for next MB
*/
static void rc_mb_end(h264e_enc_t *enc)
{
    // used / ncoded = budget/total
    int bits_coded = h264e_bs_get_pos_bits(enc->bs) +  enc->out_pos*8 + 1;
    int mb_coded = enc->mb.num; // after increment: 1, 2....
    int err = bits_coded *enc->frame.nmb - enc->rc.bit_budget*mb_coded;
    int d_err = err - enc->rc.prev_err;
    int qp = enc->rc.qp;
    assert(enc->mb.num);
    enc->rc.prev_err = err;

    if (err > 0 && d_err > 0)
    {   // Increasing risk of overflow
        if (enc->rc.stable_count < 3)
        {
            qp++;                       // State not stable: increase QP
        }
        enc->rc.stable_count = 0;       // Set state to "not stable"
    } else if (err < 0 && d_err < 0)
    {   // Increasing risk of underlow
        if (enc->rc.stable_count < 3)
        {
            qp--;
        }
        enc->rc.stable_count = 0;
    } else
    {   // Stable state
        enc->rc.stable_count++;
    }
    enc->rc.qp_smooth += qp - (enc->rc.qp_smooth >> 8);
    qp = MIN(qp, enc->rc.prev_qp + 3);
    qp = MAX(qp, enc->rc.prev_qp - 3);
    rc_set_qp(enc, qp);
}

/************************************************************************/
/*      Top-level API                                                   */
/************************************************************************/

#define ALIGN_128BIT(p) (void *)((uintptr_t)(((char*)(p)) + 15) & ~(uintptr_t)15)
#define ALLOC(ptr, size) p = ALIGN_128BIT(p); if (enc) ptr = (void *)p; p += size;

/**
*   Internal allocator for persistent RAM
*/
static int enc_alloc(h264e_enc_t *enc, const H264E_create_param_t *par, unsigned char *p, int inp_buf_flag)
{
    unsigned char *p0 = p;
    int nmbx = (par->width  + 15) >> 4;
    int nmby = (par->height + 15) >> 4;
    int nref_frames = 1 + par->max_long_term_reference_frames + par->const_input_flag;
#if H264E_ENABLE_DENOISE
    nref_frames += !!par->temporal_denoise_flag;
#endif
    ALLOC(enc->ref.yuv[0], ((nmbx + 2) * (nmby + 2) * 384) * nref_frames);
    (void)inp_buf_flag;
#if H264E_SVC_API
    if (inp_buf_flag)
    {
        ALLOC(enc->inp.yuv[0], ((nmbx)*(nmby)*384)); /* input buffer for base laeyr */
    }
#endif
    return (int)((p - p0) + 15) & ~15u;
}

/**
*   Internal allocator for scratch RAM
*/
static int enc_alloc_scratch(h264e_enc_t *enc, const H264E_create_param_t *par, unsigned char *p)
{
    unsigned char *p0 = p;
    int nmbx = (par->width  + 15) >> 4;
    int nmby = (par->height + 15) >> 4;
    ALLOC(enc->scratch, sizeof(scratch_t));
    ALLOC(enc->out, nmbx * nmby * (384 + 2 + 10) * 3/2);

    ALLOC(enc->nnz, nmbx*8 + 8);
    ALLOC(enc->mv_pred, (nmbx*4 + 8)*sizeof(point_t));
    ALLOC(enc->i4x4mode, nmbx*4 + 4);
    ALLOC(enc->df.df_qp, nmbx);
    ALLOC(enc->df.mb_type, nmbx);
    ALLOC(enc->df.df_nzflag, nmbx);
    ALLOC(enc->top_line, nmbx*32 + 32 + 16);
    return (int)(p - p0);
}

/**
*   Setup H264E_io_yuv_t structures
*/
static pix_t *io_yuv_set_pointers(pix_t *base, H264E_io_yuv_t *frm, int w, int h)
{
    int s = w + (16 + 16);    // guards
    int i, guard = 16;
    for (i = 0; i < 3; i++)
    {
        frm->stride[i] = s;
        frm->yuv[i] = base + (s + 1)*guard;
        base += s*(h + 2*guard);
        if (!i) guard >>= 1, s >>= 1, h >>= 1;
    }
    return base;
}

/**
*   Verify encoder creation parameters. Return error code, or 0 if prameters
*/
static int enc_check_create_params(const H264E_create_param_t *par)
{
    if (!par)
    {
        return H264E_STATUS_BAD_ARGUMENT;   // NULL argument
    }
    if ((int)(par->vbv_size_bytes | par->gop) < 0)
    {
        return H264E_STATUS_BAD_PARAMETER;  // negative GOP or VBV size
    }
    if (par->width <= 0 || par->height <= 0)
    {
        return H264E_STATUS_BAD_PARAMETER;  // non-positive frame size
    }
    if ((unsigned)(par->const_input_flag | par->fine_rate_control_flag |
        par->vbv_overflow_empty_frame_flag | par->vbv_underflow_stuffing_flag) > 1)
    {
        return H264E_STATUS_BAD_PARAMETER;  // Any flag is not 0 or 1
    }
    if ((unsigned)par->max_long_term_reference_frames > MAX_LONG_TERM_FRAMES)
    {
        return H264E_STATUS_BAD_PARAMETER;  // Too many long-term reference frames requested
    }
    if ((par->width | par->height) & 1)
    {
        return H264E_STATUS_SIZE_NOT_MULTIPLE_2; // frame size must be multiple of 2
    }
    if (((par->width | par->height) & 15) && !par->const_input_flag)
    {
        // if input buffer reused as scratch (par->const_input_flag == 0)
        // frame size must be multiple of 16
        return H264E_STATUS_SIZE_NOT_MULTIPLE_16;
    }
    return H264E_STATUS_SUCCESS;
};

static int H264E_sizeof_one(const H264E_create_param_t *par, int *sizeof_persist, int *sizeof_scratch, int inp_buf_flag)
{
    int error = enc_check_create_params(par);
    if (!sizeof_persist || !sizeof_scratch)
    {
        error = H264E_STATUS_BAD_ARGUMENT;
    }
    if (error)
    {
        return error;
    }

    *sizeof_persist = enc_alloc(NULL, par, (void*)(uintptr_t)1, inp_buf_flag) + sizeof(h264e_enc_t);
#if H264E_MAX_THREADS > 1
    *sizeof_scratch = enc_alloc_scratch(NULL, par, (void*)(uintptr_t)1) * (par->max_threads + 1);
#else
    *sizeof_scratch = enc_alloc_scratch(NULL, par, (void*)(uintptr_t)1);
#endif
    return error;
}

static int H264E_init_one(h264e_enc_t *enc, const H264E_create_param_t *opt, int inp_buf_flag)
{
    pix_t *base;
#if H264E_CONFIGS_COUNT > 1
    init_vft(opt->enableNEON);
#endif
    memset(enc, 0, sizeof(*enc));

    enc->frame.nmbx = (opt->width  + 15) >> 4;
    enc->frame.nmby = (opt->height + 15) >> 4;
    enc->frame.nmb = enc->frame.nmbx*enc->frame.nmby;
    enc->frame.w = enc->frame.nmbx*16;
    enc->frame.h = enc->frame.nmby*16;
    enc->frame.mv_limit.tl = point(-MV_GUARD*4, -MV_GUARD*4);
    enc->frame.mv_qpel_limit.tl = mv_add(enc->frame.mv_limit.tl, point(4*4, 4*4));
    enc->frame.mv_limit.br = point((enc->frame.nmbx*16 - (16 - MV_GUARD))*4, (enc->frame.nmby*16 - (16 - MV_GUARD))*4);
    enc->frame.mv_qpel_limit.br = mv_add(enc->frame.mv_limit.br, point(-4*4, -4*4));
    enc->frame.cropping_flag = !!((opt->width | opt->height) & 15);
    enc->param = *opt;

    enc_alloc(enc, opt, (void*)(enc + 1), inp_buf_flag);

#if H264E_SVC_API
    if (inp_buf_flag)
    {
        enc->inp.yuv[1] = enc->inp.yuv[0] + enc->frame.w*enc->frame.h;
        enc->inp.yuv[2] = enc->inp.yuv[1] + enc->frame.w*enc->frame.h/4;
        enc->inp.stride[0] = enc->frame.w;
        enc->inp.stride[1] = enc->frame.w/2;
        enc->inp.stride[2] = enc->frame.w/2;
        enc->dec = enc->inp;
    }
#endif

    base = io_yuv_set_pointers(enc->ref.yuv[0], &enc->ref, enc->frame.nmbx*16, enc->frame.nmby*16);
#if H264E_ENABLE_DENOISE
    if (enc->param.temporal_denoise_flag)
    {
        pix_t *p = base;
        base = io_yuv_set_pointers(base, &enc->denoise, enc->frame.nmbx*16, enc->frame.nmby*16);
        while (p < base) *p++ = 0;
    }
#endif
    if (enc->param.const_input_flag)
    {
        base = io_yuv_set_pointers(base, &enc->dec, enc->frame.nmbx*16, enc->frame.nmby*16);
    }
    if (enc->param.max_long_term_reference_frames)
    {
        H264E_io_yuv_t t;
        int i;
        for (i = 0; i < enc->param.max_long_term_reference_frames; i++)
        {
            base = io_yuv_set_pointers(base, &t, enc->frame.nmbx*16, enc->frame.nmby*16);
            enc->lt_yuv[i][0] = t.yuv[0];
            enc->lt_yuv[i][1] = t.yuv[1];
            enc->lt_yuv[i][2] = t.yuv[2];
        }
    }
    return H264E_STATUS_SUCCESS;
}

/**
*   Encoder initialization
*   See header file for details.
*/
int H264E_init(h264e_enc_t *enc, const H264E_create_param_t *opt)
{
    h264e_enc_t *enc_curr = enc;
    int i, ret;
    (void)i;

    ret = H264E_init_one(enc_curr, opt, 0);

#if H264E_SVC_API
    for (i = opt->num_layers; i > 1; i--)
    {
        H264E_create_param_t opt_next = enc_curr->param;
        int sizeof_persist = 0, sizeof_scratch = 0;

        opt_next.const_input_flag = 0;
        opt_next.temporal_denoise_flag = 0;
        opt_next.width =  opt_next.width >> 1;
        opt_next.width += opt_next.width & 1;
        opt_next.height = opt_next.height >> 1;
        opt_next.height+= opt_next.height & 1;

        opt_next.vbv_size_bytes <<= 2;

        H264E_sizeof_one(&enc_curr->param, &sizeof_persist, &sizeof_scratch, 1);
        enc_curr = enc_curr->enc_next = (char *)enc_curr + sizeof_persist;

        ret = H264E_init_one(enc_curr, &opt_next, 1);
        if (ret)
            break;
    }
#endif
    return ret;
}

static void encode_slice(h264e_enc_t *enc, int frame_type, int long_term_idx_use, int long_term_idx_update, int pps_id, int enc_type)
{
    int i, k;
    encode_slice_header(enc, frame_type, long_term_idx_use, long_term_idx_update, pps_id,enc_type);
    // encode frame
    do
    {   // encode row
        do
        {   // encode macroblock
            if (enc->run_param.desired_nalu_bytes &&
                h264e_bs_get_pos_bits(enc->bs) > enc->run_param.desired_nalu_bytes*8u)
            {
                // start new slice
                nal_end(enc);
                encode_slice_header(enc, frame_type, long_term_idx_use, long_term_idx_update, pps_id, enc_type);
            }

            mb_encode(enc, enc_type);

            enc->dec.yuv[0] += 16;
            enc->dec.yuv[1] += 8;
            enc->dec.yuv[2] += 8;

            enc->mb.num++;  // before rc_mb_end
            if (enc->param.fine_rate_control_flag)
            {
                rc_mb_end(enc);
            }
        } while (++enc->mb.x < enc->frame.nmbx);

        for (i = 0, k = 16; i < 3; i++, k = 8)
        {
            enc->dec.yuv[i] += k*(enc->dec.stride[i] - enc->frame.nmbx);
        }

        // start new row
        enc->mb.x = 0;
        *((uint32_t*)(enc->nnz)) = *((uint32_t*)(enc->nnz + 4)) = 0x01010101 * NNZ_NA; // left edge of NNZ predictor
        enc->i4x4mode[0] = -1;

    } while (++enc->mb.y < enc->frame.nmby);

    if (enc->mb.skip_run)
    {
        UE(enc->mb.skip_run);
    }

    nal_end(enc);
    for (i = 0, k = 16; i < 3; i++, k = 8)
    {
        enc->dec.yuv[i] -= k*enc->dec.stride[i]*enc->frame.nmby;
    }
}

#if H264E_MAX_THREADS
typedef struct
{
    H264E_persist_t *enc;
    int frame_type, long_term_idx_use, long_term_idx_update, pps_id, enc_type;
} h264_enc_slice_thread_params_t;

static void encode_slice_thread_simple(void *arg)
{
    h264_enc_slice_thread_params_t *h = (h264_enc_slice_thread_params_t*)arg;
    encode_slice(h->enc, h->frame_type, h->long_term_idx_use, h->long_term_idx_update, h->pps_id, h->enc_type);
}
#endif

static int H264E_encode_one(H264E_persist_t *enc, const H264E_run_param_t *opt,
    int long_term_idx_use, int is_refers_to_long_term, int long_term_idx_update,
    int frame_type, int pps_id, int enc_type)
{
    int i, k;
    // slice reset
    enc->slice.type = (long_term_idx_use < 0 ? SLICE_TYPE_I : SLICE_TYPE_P);
    rc_frame_start(enc, (long_term_idx_use < 0) ? 1 : 0, is_refers_to_long_term);

    enc->mb.x = enc->mb.y = enc->mb.num = 0;

    if (long_term_idx_use > 0)
    {
        // Activate long-term reference buffer
        for (i = 0; i < 3; i++)
        {
            SWAP(pix_t*, enc->ref.yuv[i], enc->lt_yuv[long_term_idx_use - 1][i]);
        }
    }

    if (enc->param.vbv_size_bytes && !long_term_idx_use && long_term_idx_update <= 0 &&
        enc->rc.vbv_bits - enc->run_param.desired_frame_bytes*8 > enc->param.vbv_size_bytes*8)
    {
        // encode transparent frame on VBV overflow
        encode_slice_header(enc, frame_type, long_term_idx_use, long_term_idx_update, pps_id,enc_type);
        enc->mb.skip_run = enc->frame.nmb;
        UE(enc->mb.skip_run);
        nal_end(enc);
        for (i = 0, k = 16; i < 3; i++, k = 8)
        {
            pix_copy_pic(enc->dec.yuv[i], enc->dec.stride[i], enc->ref.yuv[i], enc->ref.stride[i], enc->frame.nmbx*k, enc->frame.nmby*k);
        }
    } else
    {
#if H264E_MAX_THREADS
        if (enc->param.max_threads > 1)
        {
            H264E_persist_t enc_thr[H264E_MAX_THREADS];
            int sizeof_scratch = enc_alloc_scratch(NULL, &enc->param, (void*)(uintptr_t)1);
            unsigned char *scratch_base = ((unsigned char*)enc->scratch) + sizeof_scratch;
            int mby = 0;
            int ithr;
            int nmby = enc->frame.nmby;
            void *savep[3];
            for (i = 0; i < 3; i++)
            {
                savep[i] = enc->dec.yuv[i];
            }

            for (ithr = 0; ithr < enc->param.max_threads; ithr++)
            {
                enc_thr[ithr] = *enc;
                enc_thr[ithr].mb.y = mby;
                enc_thr[ithr].mb.num = mby*enc->frame.nmbx;
                mby += (enc->frame.nmby - mby) / (enc->param.max_threads - ithr);
                enc_thr[ithr].frame.nmby = mby;
                enc_thr[ithr].rc.bit_budget /= enc->param.max_threads;
                enc_thr[ithr].frame.nmb = enc_thr[ithr].frame.nmbx * enc_thr[ithr].frame.nmby;

                for (i = 0, k = 16; i < 3; i++, k = 8)
                {
                    enc_thr[ithr].dec.yuv[i] += k*enc->dec.stride[i]*enc_thr[ithr].mb.y;
                }

                //enc_alloc_scratch(enc_thr + ithr, &enc->param, (unsigned char*)(scratch_thr[ithr]));
                scratch_base += enc_alloc_scratch(enc_thr + ithr, &enc->param, scratch_base);
                enc_thr[ithr].out_pos = 0;
                h264e_bs_init_bits(enc_thr[ithr].bs, enc_thr[ithr].out);
            }

            {
                h264_enc_slice_thread_params_t thread_par[H264E_MAX_THREADS];
                void *args[H264E_MAX_THREADS];
                for (i = 0; i < enc->param.max_threads; i++)
                {
                    thread_par[i].enc = enc_thr + i;
                    thread_par[i].frame_type = frame_type;
                    thread_par[i].long_term_idx_use = long_term_idx_use;
                    thread_par[i].long_term_idx_update = long_term_idx_update;
                    thread_par[i].pps_id = pps_id;
                    thread_par[i].enc_type = enc_type;
                    args[i] = thread_par + i;
                }
                enc->param.run_func_in_thread(enc->param.token, encode_slice_thread_simple, args, enc->param.max_threads);
            }

            for (i = 0; i < enc->param.max_threads; i++)
            {
                memcpy(enc->out + enc->out_pos, enc_thr[i].out, enc_thr[i].out_pos);
                enc->out_pos += enc_thr[i].out_pos;
            }
            enc->frame.nmby = nmby;
            for (i = 0; i < 3; i++)
            {
                enc->dec.yuv[i] = savep[i];
            }
        } else
#endif
        {
            encode_slice(enc, frame_type, long_term_idx_use, long_term_idx_update, pps_id, enc_type);
        }
    }

    // Set flags for AMM state machine for standard compliance
    if (frame_type == H264E_FRAME_TYPE_KEY)
    {
        // Reset long-term reference frames
        memset(enc->lt_used, 0, sizeof(enc->lt_used));
        // Assume that this frame is not short-term (have effect only if AMM used)
        enc->short_term_used = 0;
    }
    if (long_term_idx_update > 0)
    {
        enc->lt_used[long_term_idx_update - 1] = 1;
    } else if (long_term_idx_update == 0)
    {
        enc->short_term_used = 1;
    }

    rc_frame_end(enc, long_term_idx_use == -1, enc->mb.skip_run == enc->frame.nmb, is_refers_to_long_term);

    if (long_term_idx_use > 0)
    {
        // deactivate long-term reference
        for (i = 0; i < 3; i++)
        {
            SWAP(pix_t*, enc->ref.yuv[i], enc->lt_yuv[long_term_idx_use - 1][i]);
        }
    }

    if (long_term_idx_update != -1)
    {
        pix_copy_recon_pic_to_ref(enc);

        if (++enc->frame.num >= enc->param.gop && enc->param.gop && (opt->frame_type == H264E_FRAME_TYPE_DEFAULT))
        {
            enc->frame.num = 0;     // trigger to encode IDR on next call
        }

        if (long_term_idx_update > 0)
        {
            for (i = 0; i < 3; i++)
            {
                SWAP(pix_t*, enc->ref.yuv[i], enc->lt_yuv[long_term_idx_update - 1][i]);
            }
        }
    }

    return H264E_STATUS_SUCCESS;
}

static int check_parameters_align(const H264E_create_param_t *opt, const H264E_io_yuv_t *in)
{
    int i;
    int min_align = 0;
#if H264E_ENABLE_NEON || H264E_ENABLE_SSE2
    min_align = 7;
#endif
    if (opt->const_input_flag && opt->temporal_denoise_flag)
    {
        min_align = 0;
    }
    for (i = 0; i < 3; i++)
    {
        if (((uintptr_t)in->yuv[i]) & min_align)
        {
            return i ? H264E_STATUS_BAD_CHROMA_ALIGN : H264E_STATUS_BAD_LUMA_ALIGN;
        }
        if (in->stride[i] & min_align)
        {
            return i ? H264E_STATUS_BAD_CHROMA_STRIDE : H264E_STATUS_BAD_LUMA_STRIDE;
        }
    }
    return H264E_STATUS_SUCCESS;
}

/**
*   Top-level encode function
*   See header file for details.
*/
int H264E_encode(H264E_persist_t *enc, H264E_scratch_t *scratch, const H264E_run_param_t *opt,
    H264E_io_yuv_t *in, unsigned char **coded_data, int *sizeof_coded_data)
{
    int i;
    int frame_type;
    int long_term_idx_use;
    int long_term_idx_update;
    int is_refers_to_long_term;
    int error;

    error = check_parameters_align(&enc->param, in);
    if (error)
    {
        return error;
    }
    (void)i;
    i = enc_alloc_scratch(enc, &enc->param, (unsigned char*)scratch);
#if H264E_SVC_API
    {
        H264E_persist_t *e = enc->enc_next;
        while (e)
        {
            i += enc_alloc_scratch(e, &enc->param, ((unsigned char*)scratch) + i);
            e = e->enc_next;
        }
    }
#endif

    enc->inp = *in;

#if H264E_ENABLE_DENOISE
    // 1. Run optional denoise filter
    if (enc->param.temporal_denoise_flag && opt->encode_speed < 2)
    {
        int sh = 0;
        for (i = 0; i < 3; i++)
        {
            h264e_denoise_run(in->yuv[i], enc->denoise.yuv[i],  enc->param.width >> sh, enc->param.height >> sh, in->stride[i], enc->denoise.stride[i]);
            enc->inp.yuv[i] = enc->denoise.yuv[i];
            enc->inp.stride[i] = enc->denoise.stride[i];
            sh = 1;
        }
    }
#endif

    enc->out_pos = 0;   // reset output bitbuffer position

    if (opt)
    {
        enc->run_param = *opt;  // local copy of run-time parameters
    }
    opt = &enc->run_param;      // refer to local copy

    // silently fix invalid QP without warning
    if (!enc->run_param.qp_max || enc->run_param.qp_max > 51)
    {
        enc->run_param.qp_max = 51;
    }
    if (!enc->run_param.qp_min || enc->run_param.qp_min < MIN_QP)
    {
        enc->run_param.qp_min = MIN_QP;
    }

    enc->speed.disable_deblock = (opt->encode_speed == 8 || opt->encode_speed == 10);

    if (!enc->param.const_input_flag)
    {
        // if input frame can be re-used as a scratch, set reconstructed frame to the input
        enc->dec = *in;
    }

    // Set default frame type
    frame_type = opt->frame_type;
    if (frame_type == H264E_FRAME_TYPE_DEFAULT)
    {
        frame_type = enc->frame.num ? H264E_FRAME_TYPE_P : H264E_FRAME_TYPE_KEY;
    }
    // Estimate long-term indexes from frame type
    // index 0 means "short-term" reference
    // index -1 means "not used"
    switch (frame_type)
    {
    default:
    case H264E_FRAME_TYPE_I:        long_term_idx_use = -1; long_term_idx_update = 0; break;
    case H264E_FRAME_TYPE_KEY:      long_term_idx_use = -1; long_term_idx_update = enc->param.max_long_term_reference_frames > 0; break;
    case H264E_FRAME_TYPE_GOLDEN:   long_term_idx_use =  1; long_term_idx_update = 1; break;
    case H264E_FRAME_TYPE_RECOVERY: long_term_idx_use =  1; long_term_idx_update = 0; break;
    case H264E_FRAME_TYPE_P:        long_term_idx_use =  enc->most_recent_ref_frame_idx; long_term_idx_update =  0; break;
    case H264E_FRAME_TYPE_DROPPABLE:long_term_idx_use =  enc->most_recent_ref_frame_idx; long_term_idx_update = -1; break;
    case H264E_FRAME_TYPE_CUSTOM:   long_term_idx_use =  opt->long_term_idx_use; long_term_idx_update = opt->long_term_idx_update;
        if (!long_term_idx_use)
        {
            long_term_idx_use = enc->most_recent_ref_frame_idx;
        }
        if (long_term_idx_use < 0)
        {
            // hack: redefine frame type, always encode IDR
            frame_type = H264E_FRAME_TYPE_KEY;
        }
        break;
    }

#if H264E_RATE_CONTROL_GOLDEN_FRAMES
    is_refers_to_long_term = (long_term_idx_use != enc->most_recent_ref_frame_idx && long_term_idx_use >= 0);
#else
    is_refers_to_long_term = 0;
#endif

    if (long_term_idx_update >= 0)
    {
        enc->most_recent_ref_frame_idx = long_term_idx_update;
    }
    if (frame_type == H264E_FRAME_TYPE_KEY)
    {
        int pic_init_qp = 30;
        pic_init_qp = MIN(pic_init_qp, enc->run_param.qp_max);
        pic_init_qp = MAX(pic_init_qp, enc->run_param.qp_min);

        //temp only two layers!
        enc->sps.pic_init_qp = pic_init_qp;
        enc->next_idr_pic_id ^= 1;
        enc->frame.num = 0;

#if H264E_SVC_API
        if (enc->param.num_layers > 1)
        {
            H264E_persist_t *enc_base = enc->enc_next;
            enc_base->sps.pic_init_qp = pic_init_qp;
            enc_base->next_idr_pic_id ^= 1;
            enc_base->frame.num = 0;

            enc_base->out = enc->out;
            enc_base->out_pos = 0;
            encode_sps(enc_base, 66);
            encode_pps(enc_base, 0);

            enc->out_pos += enc_base->out_pos;
            encode_sps(enc, 83);
            encode_pps(enc, 1);
        } else
#endif
        {
            encode_sps(enc, 66);
            encode_pps(enc, 0);
        }
    } else
    {
        if (!enc->sps.pic_init_qp)
        {
            return H264E_STATUS_BAD_FRAME_TYPE;
        }
        if (long_term_idx_use > enc->param.max_long_term_reference_frames ||
            long_term_idx_update > enc->param.max_long_term_reference_frames ||
            long_term_idx_use > MAX_LONG_TERM_FRAMES)
        {
            return H264E_STATUS_BAD_FRAME_TYPE;
        }
    }

#if H264E_SVC_API
    if (enc->param.num_layers > 1)
    {
        H264E_persist_t *enc_base = enc->enc_next;
        int sh = 0;

        enc_base->run_param = enc->run_param;
        enc_base->run_param.desired_frame_bytes = enc->run_param.desired_frame_bytes >> 2;

        for (i = 0; i < 3; i++)
        {
            h264e_frame_downsampling(enc_base->inp.yuv[i], enc_base->inp.stride[i], enc_base->frame.h >> sh,
                in->yuv[i], in->stride[i], enc->param.height >> sh, enc_base->param.width >> sh,
                enc_base->param.height >> sh, enc->param.width >> sh, enc->param.height >> sh);
            sh = 1;
        }

        enc_base->scratch = enc->scratch;
        enc_base->out = enc->out + enc->out_pos;
        enc_base->out_pos = 0;

        H264E_encode_one(enc_base, &enc_base->run_param, long_term_idx_use, is_refers_to_long_term, long_term_idx_update,
            frame_type, enc->param.sps_id*4 + 0, 0);

        enc->out_pos += enc_base->out_pos;

        if ((frame_type == H264E_FRAME_TYPE_I || frame_type == H264E_FRAME_TYPE_KEY) && enc->param.inter_layer_pred_flag)
        {
            for (i = 0, sh = 0; i < 3; i++, sh = 1)
            {
                h264e_intra_upsampling(enc_base->frame.w >> sh, enc_base->frame.h >> sh, enc->frame.w >> sh, enc->frame.h >> sh,
                    sh, enc_base->dec.yuv[i], enc_base->dec.stride[i], enc->ref.yuv[i], enc->ref.stride[i]);
            }
        }

        memset(enc->df.df_nzflag, 0, enc->frame.nmbx);
        H264E_encode_one(enc, opt, long_term_idx_use, is_refers_to_long_term, long_term_idx_update,
            frame_type, enc->param.sps_id*4 + 1, 20);
    } else
#endif // H264E_SVC_API
    {
        H264E_encode_one(enc, opt, long_term_idx_use, is_refers_to_long_term, long_term_idx_update,
            frame_type, enc->param.sps_id*4 + 0, 0);
    }

    *sizeof_coded_data = enc->out_pos;
    *coded_data = enc->out;
    return H264E_STATUS_SUCCESS;
}

/**
*   Return persistent and scratch memory requirements
*   for given encoding options.
*   See header file for details.
*/
int H264E_sizeof(const H264E_create_param_t *par, int *sizeof_persist, int *sizeof_scratch)
{
    int i;
    int error = H264E_sizeof_one(par, sizeof_persist, sizeof_scratch, 0);
    (void)i;
#if H264E_SVC_API
    for (i = par->num_layers; i > 1; i--)
    {
        H264E_create_param_t opt_next = *par;
        opt_next.const_input_flag = 1;
        opt_next.temporal_denoise_flag = 0;
        opt_next.width   = opt_next.width >> 1;
        opt_next.width  += opt_next.width & 1;
        opt_next.height  = opt_next.height >> 1;
        opt_next.height += opt_next.height & 1;
        *sizeof_persist += enc_alloc(NULL, par, (void*)(uintptr_t)1, 1) + sizeof(h264e_enc_t);
#if H264E_MAX_THREADS > 1
        *sizeof_scratch += enc_alloc_scratch(NULL, par, (void*)(uintptr_t)1) * (H264E_MAX_THREADS + 1);
#else
        *sizeof_scratch += enc_alloc_scratch(NULL, par, (void*)(uintptr_t)1);
#endif
    }
#endif
    return error;
}

/**
*   Set VBV size and fullness
*   See header file for details.
*/
void H264E_set_vbv_state(
    H264E_persist_t *enc,
    int vbv_size_bytes,     //< New VBV size
    int vbv_fullness_bytes  //< New VBV fulness, -1 = no change
)
{
    if (enc)
    {
        enc->param.vbv_size_bytes = vbv_size_bytes;
        if (vbv_fullness_bytes >= 0)
        {
            enc->rc.vbv_bits = vbv_fullness_bytes*8;
            enc->rc.vbv_target_level = enc->rc.vbv_bits;
        }
    }
}
#endif
