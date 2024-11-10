#pragma once

#include "MiniVulkan.h"

typedef struct {
	int count;
	int format;

	VkBuffer buf;
	VkDeviceMemory mem;
	VkMemoryAllocateInfo mem_alloc;
} IndexBuffer5Impl;
