#pragma once

#include "MiniVulkan.h"

typedef struct {
	int _indexCount;
	VkCommandBuffer _buffer;
	VkFence fence;
} CommandList5Impl;
