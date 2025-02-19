#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

static id<MTLCommandBuffer> command_buffer = nil;
static id<MTLRenderCommandEncoder> render_command_encoder = nil;
static id<MTLComputeCommandEncoder> compute_command_encoder = nil;

static void start_render_pass(void);
static void end_render_pass(void);

#include "metal.m.h"
#include "g5_commandlist.m.h"
#include "g5_compute.m.h"
#include "g5_pipeline.m.h"
#include "g5_raytrace.m.h"
#include "g5_texture.m.h"
#include "g5_buffer.m.h"
