#include <kinc/global.h>

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

#ifdef KINC_WINDOWS
#include <d3d12.h>
#else
#include <kinc/backend/d3d12_special_edition.h>
#endif
#ifdef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
struct DXGI_SWAP_CHAIN_DESC1;
#else
#include <dxgi.h>
#endif

#include "d3d12mini.h"

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

extern "C" {
ID3D12Device *device = NULL;
}
static ID3D12RootSignature *globalRootSignature = NULL;
static ID3D12RootSignature *globalComputeRootSignature = NULL;
// extern ID3D12GraphicsCommandList* commandList;

#include <stdbool.h>

#define MAXIMUM_WINDOWS 16

struct dx_ctx {
	int current_window;
	struct dx_window windows[MAXIMUM_WINDOWS];
};

static struct dx_ctx dx_ctx = {0};

inline struct dx_window *kinc_dx_current_window() {
	return &dx_ctx.windows[dx_ctx.current_window];
}

static bool compute_pipeline_set = false;

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>

#include "Direct3D12.c.h"
#include "ShaderHash.c.h"
#include "commandlist.c.h"
#include "compute.c.h"
#include "constantbuffer.c.h"
#include "indexbuffer.c.h"
#include "pipeline.c.h"
#include "raytrace.c.h"
#include "rendertarget.c.h"
#include "sampler.c.h"
#include "shader.c.h"
#include "texture.c.h"
#include "vertexbuffer.c.h"
