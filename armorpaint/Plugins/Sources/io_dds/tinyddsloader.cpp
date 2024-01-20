#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"
using namespace tinyddsloader;

static void *buf; /* Pointer to dds file data */
static size_t size; /* Size of the file data */

static uint8_t *pixels;
static uint32_t w;
static uint32_t h;
static uint32_t format;

uint8_t *io_dds_init(int bufSize) {
	size = bufSize;
	buf = (uint8_t *)malloc(sizeof(uint8_t) * bufSize);
	return (uint8_t *)buf;
}

void io_dds_parse() {
	if (pixels != NULL) {
		free(pixels);
	}

	DDSFile dds;

	if (dds.Load((uint8_t *)buf, size) != tinyddsloader::Result::Success) {
		return;
	}
	if (dds.GetTextureDimension() != DDSFile::TextureDimension::Texture2D) {
		return;
	}

	w = dds.GetWidth();
	h = dds.GetHeight();
	format = (uint32_t)dds.GetFormat(); // DDSFile::DXGIFormat::R8G8B8A8_Typeless

	// dds.Flip();
	// pixels = (uint8_t *)malloc(sizeof(uint8_t) * w * h * 4);
	pixels = (uint8_t *)dds.GetImageData(0, 0)->m_mem;
}

void io_dds_destroy() {
	free(buf);
	free(pixels);
}

uint8_t *io_dds_get_pixels() { return pixels; }
uint32_t io_dds_get_pixels_w() { return w; }
uint32_t io_dds_get_pixels_h() { return h; }
uint32_t io_dds_get_pixels_format() { return format; }
