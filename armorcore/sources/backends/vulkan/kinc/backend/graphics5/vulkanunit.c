#ifdef KINC_WINDOWS

// Windows 7
#define WINVER 0x0601
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
// #define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
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

// avoids a warning in the Windows headers
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#endif

#ifndef NDEBUG
#define VALIDATE
#endif

#include <vulkan/vulkan.h>

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/image.h>
#include <kinc/math/matrix.h>

#include "vulkan.h"

static VkSemaphore framebuffer_available;
static VkSemaphore relay_semaphore;
static bool wait_for_relay = false;
static void command_list_should_wait_for_framebuffer(void);
static VkDescriptorSetLayout compute_descriptor_layout;

#include "ShaderHash.c.h"
#include "Vulkan.c.h"
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