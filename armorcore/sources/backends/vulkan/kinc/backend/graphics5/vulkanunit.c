
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

#include "shaderhash.c.h"
#include "vulkan.c.h"
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
