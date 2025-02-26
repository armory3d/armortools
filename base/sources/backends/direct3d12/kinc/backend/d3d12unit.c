#include <iron_global.h>

// Windows 7
#define WINVER 0x0601
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0601

#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
#define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
#define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
// #define NOMSG
#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
// #define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <d3d12.h>
#include <dxgi.h>
#include "d3d12mini.h"

ID3D12Device *device = NULL;
static ID3D12RootSignature *globalRootSignature = NULL;
static ID3D12RootSignature *globalComputeRootSignature = NULL;
// extern ID3D12GraphicsCommandList* commandList;

#include <stdbool.h>

#define MAXIMUM_WINDOWS 1

struct dx_ctx {
	struct dx_window windows[MAXIMUM_WINDOWS];
};

static struct dx_ctx dx_ctx = {0};

inline struct dx_window *kinc_dx_current_window() {
	return &dx_ctx.windows[0];
}

static bool compute_pipeline_set = false;

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>

#include "g5.c.h"
#include "g5_commandlist.c.h"
#include "g5_compute.c.h"
#include "g5_pipeline.c.h"
#include "g5_raytrace.c.h"
#include "g5_texture.c.h"
#include "g5_buffer.c.h"
