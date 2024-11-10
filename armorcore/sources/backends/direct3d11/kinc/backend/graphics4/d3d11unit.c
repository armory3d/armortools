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

#include <kinc/graphics4/graphics.h>

#include <kinc/backend/SystemMicrosoft.h>

#pragma warning(disable : 4005)
#include <d3d11.h>

#include "Direct3D11.h"

#include <assert.h>
#include <malloc.h>
#include <stdint.h>

struct dx_context dx_ctx = {0};

static uint8_t vertexConstants[1024 * 4];
static uint8_t fragmentConstants[1024 * 4];
static uint8_t geometryConstants[1024 * 4];
static uint8_t tessControlConstants[1024 * 4];
static uint8_t tessEvalConstants[1024 * 4];
static uint8_t computeConstants[1024 * 4];

static D3D11_COMPARISON_FUNC get_comparison(kinc_g4_compare_mode_t compare) {
	switch (compare) {
	default:
	case KINC_G4_COMPARE_ALWAYS:
		return D3D11_COMPARISON_ALWAYS;
	case KINC_G4_COMPARE_NEVER:
		return D3D11_COMPARISON_NEVER;
	case KINC_G4_COMPARE_EQUAL:
		return D3D11_COMPARISON_EQUAL;
	case KINC_G4_COMPARE_NOT_EQUAL:
		return D3D11_COMPARISON_NOT_EQUAL;
	case KINC_G4_COMPARE_LESS:
		return D3D11_COMPARISON_LESS;
	case KINC_G4_COMPARE_LESS_EQUAL:
		return D3D11_COMPARISON_LESS_EQUAL;
	case KINC_G4_COMPARE_GREATER:
		return D3D11_COMPARISON_GREATER;
	case KINC_G4_COMPARE_GREATER_EQUAL:
		return D3D11_COMPARISON_GREATER_EQUAL;
	}
}

static size_t get_multiple_of_16(size_t value) {
	size_t ret = 16;
	while (ret < value) {
		ret += 16;
	}
	return ret;
}

#include "Direct3D11.c.h"
#include "ShaderHash.c.h"
#include "compute.c.h"
#include "indexbuffer.c.h"
#include "pipeline.c.h"
#include "rendertarget.c.h"
#include "shader.c.h"
#include "texture.c.h"
#include "vertexbuffer.c.h"
