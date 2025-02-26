
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
#include <iron_gpu.h>
#include <iron_math.h>
#include "g5.h"

static VkSemaphore framebuffer_available;
static VkSemaphore relay_semaphore;
static bool wait_for_relay = false;
static void command_list_should_wait_for_framebuffer(void);
static VkDescriptorSetLayout compute_descriptor_layout;

#include "minivulkan.c.h"
#include "g5.c.h"
#include "g5_commandlist.c.h"
#include "g5_compute.c.h"
#include "g5_pipeline.c.h"
#include "g5_raytrace.c.h"
#include "g5_texture.c.h"
#include "g5_buffer.c.h"
