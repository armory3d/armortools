#include "vulkan.h"
#include <kinc/graphics5/texture.h>
#include <kinc/image.h>
#include <kinc/log.h>

bool use_staging_buffer = false;

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
void set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);

static void prepare_texture_image(uint8_t *tex_colors, uint32_t tex_width, uint32_t tex_height, struct texture_object *tex_obj, VkImageTiling tiling,
                                  VkImageUsageFlags usage, VkFlags required_props, VkDeviceSize *deviceSize, VkFormat tex_format) {
	VkResult err;
	bool pass;

	tex_obj->tex_width = tex_width;
	tex_obj->tex_height = tex_height;

	VkImageCreateInfo image_create_info = {0};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext = NULL;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = tex_format;
	image_create_info.extent.width = tex_width;
	image_create_info.extent.height = tex_height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = tiling;
	image_create_info.usage = usage;
	image_create_info.flags = 0;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	VkMemoryAllocateInfo mem_alloc = {0};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements mem_reqs;

	err = vkCreateImage(vk_ctx.device, &image_create_info, NULL, &tex_obj->image);
	assert(!err);

	vkGetImageMemoryRequirements(vk_ctx.device, tex_obj->image, &mem_reqs);

	*deviceSize = mem_alloc.allocationSize = mem_reqs.size;
	pass = memory_type_from_properties(mem_reqs.memoryTypeBits, required_props, &mem_alloc.memoryTypeIndex);
	assert(pass);

	// allocate memory
	err = vkAllocateMemory(vk_ctx.device, &mem_alloc, NULL, &tex_obj->mem);
	assert(!err);

	// bind memory
	err = vkBindImageMemory(vk_ctx.device, tex_obj->image, tex_obj->mem, 0);
	assert(!err);

	if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && tex_colors != NULL) {
		VkImageSubresource subres = {0};
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = 0;
		subres.arrayLayer = 0;

		VkSubresourceLayout layout;
		uint8_t *data;

		vkGetImageSubresourceLayout(vk_ctx.device, tex_obj->image, &subres, &layout);

		err = vkMapMemory(vk_ctx.device, tex_obj->mem, 0, mem_alloc.allocationSize, 0, (void **)&data);
		assert(!err);

		if (tex_format == VK_FORMAT_R8_UNORM) {
			for (uint32_t y = 0; y < tex_height; y++) {
				for (uint32_t x = 0; x < tex_width; x++) {
					data[y * layout.rowPitch + x] = tex_colors[y * tex_width + x];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R32_SFLOAT) {
			uint32_t *data32 = (uint32_t *)data;
			uint32_t *tex_colors32 = (uint32_t *)tex_colors;
			for (uint32_t y = 0; y < tex_height; y++) {
				for (uint32_t x = 0; x < tex_width; x++) {
					data32[y * (layout.rowPitch / 4) + x] = tex_colors32[y * tex_width + x];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R16_SFLOAT) {
			uint16_t *data16 = (uint16_t *)data;
			uint16_t *tex_colors16 = (uint16_t *)tex_colors;
			for (uint32_t y = 0; y < tex_height; y++) {
				for (uint32_t x = 0; x < tex_width; x++) {
					data16[y * (layout.rowPitch / 4) + x] = tex_colors16[y * tex_width + x];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R32G32B32A32_SFLOAT) {
			uint32_t *data32 = (uint32_t *)data;
			uint32_t *tex_colors32 = (uint32_t *)tex_colors;
			for (uint32_t y = 0; y < tex_height; y++) {
				for (uint32_t x = 0; x < tex_width; x++) {
					data32[y * (layout.rowPitch / 4) + x * 4 + 0] = tex_colors32[y * tex_width * 4 + x * 4 + 0];
					data32[y * (layout.rowPitch / 4) + x * 4 + 1] = tex_colors32[y * tex_width * 4 + x * 4 + 1];
					data32[y * (layout.rowPitch / 4) + x * 4 + 2] = tex_colors32[y * tex_width * 4 + x * 4 + 2];
					data32[y * (layout.rowPitch / 4) + x * 4 + 3] = tex_colors32[y * tex_width * 4 + x * 4 + 3];
				}
			}
		}
		else if (tex_format == VK_FORMAT_R16G16B16A16_SFLOAT) {
			uint16_t *data16 = (uint16_t *)data;
			uint16_t *tex_colors16 = (uint16_t *)tex_colors;
			for (uint32_t y = 0; y < tex_height; y++) {
				for (uint32_t x = 0; x < tex_width; x++) {
					data16[y * (layout.rowPitch / 4) + x * 4 + 0] = tex_colors16[y * tex_width * 4 + x * 4 + 0];
					data16[y * (layout.rowPitch / 4) + x * 4 + 1] = tex_colors16[y * tex_width * 4 + x * 4 + 1];
					data16[y * (layout.rowPitch / 4) + x * 4 + 2] = tex_colors16[y * tex_width * 4 + x * 4 + 2];
					data16[y * (layout.rowPitch / 4) + x * 4 + 3] = tex_colors16[y * tex_width * 4 + x * 4 + 3];
				}
			}
		}
		else if (tex_format == VK_FORMAT_B8G8R8A8_UNORM) {
			for (uint32_t y = 0; y < tex_height; y++) {
				// uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
				for (uint32_t x = 0; x < tex_width; x++) {
					data[y * layout.rowPitch + x * 4 + 0] = tex_colors[y * tex_width * 4 + x * 4 + 2];
					data[y * layout.rowPitch + x * 4 + 1] = tex_colors[y * tex_width * 4 + x * 4 + 1];
					data[y * layout.rowPitch + x * 4 + 2] = tex_colors[y * tex_width * 4 + x * 4 + 0];
					data[y * layout.rowPitch + x * 4 + 3] = tex_colors[y * tex_width * 4 + x * 4 + 3];
					// row[x] = tex_colors[(x & 1) ^ (y & 1)];
				}
			}
		}
		else {
			for (uint32_t y = 0; y < tex_height; y++) {
				// uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
				for (uint32_t x = 0; x < tex_width; x++) {
					data[y * layout.rowPitch + x * 4 + 0] = tex_colors[y * tex_width * 4 + x * 4 + 0];
					data[y * layout.rowPitch + x * 4 + 1] = tex_colors[y * tex_width * 4 + x * 4 + 1];
					data[y * layout.rowPitch + x * 4 + 2] = tex_colors[y * tex_width * 4 + x * 4 + 2];
					data[y * layout.rowPitch + x * 4 + 3] = tex_colors[y * tex_width * 4 + x * 4 + 3];
					// row[x] = tex_colors[(x & 1) ^ (y & 1)];
				}
			}
		}

		vkUnmapMemory(vk_ctx.device, tex_obj->mem);
	}

	if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
		tex_obj->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	else {
		tex_obj->imageLayout = VK_IMAGE_LAYOUT_GENERAL; // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	set_image_layout(tex_obj->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, tex_obj->imageLayout);
	// setting the image layout does not reference the actual memory so no need to add a mem ref
}

static void destroy_texture_image(struct texture_object *tex_obj) {
	// clean up staging resources
	vkDestroyImage(vk_ctx.device, tex_obj->image, NULL);
	vkFreeMemory(vk_ctx.device, tex_obj->mem, NULL);
}

static VkFormat convert_image_format(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case KINC_IMAGE_FORMAT_RGB24:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case KINC_IMAGE_FORMAT_A32:
		return VK_FORMAT_R32_SFLOAT;
	case KINC_IMAGE_FORMAT_A16:
		return VK_FORMAT_R16_SFLOAT;
	case KINC_IMAGE_FORMAT_GREY8:
		return VK_FORMAT_R8_UNORM;
	case KINC_IMAGE_FORMAT_BGRA32:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case KINC_IMAGE_FORMAT_RGBA32:
		return VK_FORMAT_R8G8B8A8_UNORM;
	default:
		return VK_FORMAT_B8G8R8A8_UNORM;
	}
}

static int format_byte_size(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_RGB24:
		return 4;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
		return 4;
	default:
		return 4;
	}
}

static void update_stride(kinc_g5_texture_t *texture) {
	VkImageSubresource subres = {0};
	subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subres.mipLevel = 0;
	subres.arrayLayer = 0;

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(vk_ctx.device, texture->impl.texture.image, &subres, &layout);

	texture->impl.stride = (int)layout.rowPitch;
}

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image) {
	texture->texWidth = image->width;
	texture->texHeight = image->height;

	const VkFormat tex_format = convert_image_format(image->format);
	VkFormatProperties props;
	VkResult err;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !use_staging_buffer) {
		// Device can texture using linear textures
		prepare_texture_image((uint8_t *)image->data, (uint32_t)image->width, (uint32_t)image->height, &texture->impl.texture, VK_IMAGE_TILING_LINEAR,
		                      VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize,
		                      tex_format);

		flush_init_cmd();
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
		// Must use staging buffer to copy linear texture to optimized
		struct texture_object staging_texture;

		memset(&staging_texture, 0, sizeof(staging_texture));
		prepare_texture_image((uint8_t *)image->data, (uint32_t)image->width, (uint32_t)image->height, &staging_texture, VK_IMAGE_TILING_LINEAR,
		                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize, tex_format);
		prepare_texture_image((uint8_t *)image->data, (uint32_t)image->width, (uint32_t)image->height, &texture->impl.texture, VK_IMAGE_TILING_OPTIMAL,
		                      (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/),
		                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->impl.deviceSize, tex_format);
		set_image_layout(staging_texture.image, VK_IMAGE_ASPECT_COLOR_BIT, staging_texture.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		set_image_layout(texture->impl.texture.image, VK_IMAGE_ASPECT_COLOR_BIT, texture->impl.texture.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageCopy copy_region = {0};
		copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.srcSubresource.mipLevel = 0;
		copy_region.srcSubresource.baseArrayLayer = 0;
		copy_region.srcSubresource.layerCount = 1;
		copy_region.srcOffset.x = copy_region.srcOffset.y = copy_region.srcOffset.z = 0;
		copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.dstSubresource.mipLevel = 0;
		copy_region.dstSubresource.baseArrayLayer = 0;
		copy_region.dstSubresource.layerCount = 1;
		copy_region.dstOffset.x = copy_region.dstOffset.y = copy_region.dstOffset.z = 0;
		copy_region.extent.width = (uint32_t)staging_texture.tex_width;
		copy_region.extent.height = (uint32_t)staging_texture.tex_height;
		copy_region.extent.depth = 1;

		vkCmdCopyImage(vk_ctx.setup_cmd, staging_texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->impl.texture.image,
		               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

		set_image_layout(texture->impl.texture.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->impl.texture.imageLayout);

		flush_init_cmd();

		destroy_texture_image(&staging_texture);
	}
	else {
		assert(!"No support for B8G8R8A8_UNORM as texture image format");
	}

	update_stride(texture);

	VkImageViewCreateInfo view = {0};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = tex_format;
	view.components.r = VK_COMPONENT_SWIZZLE_R;
	view.components.g = VK_COMPONENT_SWIZZLE_G;
	view.components.b = VK_COMPONENT_SWIZZLE_B;
	view.components.a = VK_COMPONENT_SWIZZLE_A;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.flags = 0;

	view.image = texture->impl.texture.image;
	err = vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.texture.view);
	assert(!err);
}

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->texWidth = width;
	texture->texHeight = height;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;
	VkResult err;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	// Device can texture using linear textures
	prepare_texture_image(NULL, (uint32_t)width, (uint32_t)height, &texture->impl.texture, VK_IMAGE_TILING_LINEAR,
	                      VK_IMAGE_USAGE_SAMPLED_BIT /*| VK_IMAGE_USAGE_STORAGE_BIT*/, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->impl.deviceSize,
	                      tex_format);

	flush_init_cmd();

	update_stride(texture);

	VkImageViewCreateInfo view = {0};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = tex_format;
	view.components.r = VK_COMPONENT_SWIZZLE_R;
	view.components.g = VK_COMPONENT_SWIZZLE_G;
	view.components.b = VK_COMPONENT_SWIZZLE_B;
	view.components.a = VK_COMPONENT_SWIZZLE_A;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.flags = 0;

	view.image = texture->impl.texture.image;
	err = vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.texture.view);
	assert(!err);
}

void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->texWidth = width;
	texture->texHeight = height;

	const VkFormat tex_format = convert_image_format(format);
	VkFormatProperties props;
	VkResult err;

	vkGetPhysicalDeviceFormatProperties(vk_ctx.gpu, tex_format, &props);

	// Device can texture using linear textures
	prepare_texture_image(NULL, (uint32_t)width, (uint32_t)height, &texture->impl.texture, VK_IMAGE_TILING_OPTIMAL,
	                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->impl.deviceSize,
	                      tex_format);

	flush_init_cmd();

	update_stride(texture);

	VkImageViewCreateInfo view = {0};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = tex_format;
	view.components.r = VK_COMPONENT_SWIZZLE_R;
	view.components.g = VK_COMPONENT_SWIZZLE_G;
	view.components.b = VK_COMPONENT_SWIZZLE_B;
	view.components.a = VK_COMPONENT_SWIZZLE_A;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;
	view.flags = 0;

	view.image = texture->impl.texture.image;
	err = vkCreateImageView(vk_ctx.device, &view, NULL, &texture->impl.texture.view);
	assert(!err);
}

void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {
	vkDestroyImageView(vk_ctx.device, texture->impl.texture.view, NULL);
	destroy_texture_image(&texture->impl.texture);
}

void kinc_g5_internal_texture_set(kinc_g5_texture_t *texture, int unit) {}

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	return texture->impl.stride;
}

uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture) {
	void *data;

	VkResult err = vkMapMemory(vk_ctx.device, texture->impl.texture.mem, 0, texture->impl.deviceSize, 0, &data);
	assert(!err);

	return (uint8_t *)data;
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *texture) {
	vkUnmapMemory(vk_ctx.device, texture->impl.texture.mem);
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_image_t *mipmap, int level) {}
