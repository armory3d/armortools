#pragma once

#include "minivulkan.h"

typedef struct {
	VkImage image;
	VkImageLayout imageLayout;
	VkDeviceMemory mem;
	VkImageView view;
	int32_t width;
	int32_t height;

	VkDeviceSize deviceSize;
	uint8_t *conversionBuffer;
	int stride;
} Texture5Impl;

typedef struct {
	VkImage sourceImage;
	VkDeviceMemory sourceMemory;
	VkImageView sourceView;

	VkImage depthImage;
	VkDeviceMemory depthMemory;
	VkImageView depthView;
	int depthBufferBits;

	VkFramebuffer framebuffer;

	VkFormat format;

	VkBuffer readbackBuffer;
	VkDeviceMemory readbackMemory;
	bool readbackBufferCreated;

	int stage;
	int stage_depth;
} RenderTarget5Impl;
