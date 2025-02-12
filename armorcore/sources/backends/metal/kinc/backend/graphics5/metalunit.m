#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

static id<MTLCommandBuffer> command_buffer = nil;
static id<MTLRenderCommandEncoder> render_command_encoder = nil;
static id<MTLComputeCommandEncoder> compute_command_encoder = nil;

static void start_render_pass(void);
static void end_render_pass(void);

#include "metal.m.h"
#include "commandlist.m.h"
#include "compute.m.h"
#include "constantbuffer.m.h"
#include "indexbuffer.m.h"
#include "pipeline.m.h"
#include "raytrace.m.h"
#include "rendertarget.m.h"
#include "sampler.m.h"
#include "shader.m.h"
#include "texture.m.h"
#include "vertexbuffer.m.h"
