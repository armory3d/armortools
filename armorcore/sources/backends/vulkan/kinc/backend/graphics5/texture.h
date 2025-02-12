#pragma once

#include "minivulkan.h"

struct texture_object {
	VkImage image;
	VkImageLayout imageLayout;

	VkDeviceMemory mem;
	VkImageView view;
	int32_t tex_width;
	int32_t tex_height;
};

typedef struct {
	struct texture_object texture;
	VkDeviceSize deviceSize;

	uint8_t *conversionBuffer;
	int stride;
} Texture5Impl;
