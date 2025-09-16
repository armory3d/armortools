#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "iron_array.h"
#include "iron_gpu.h"
void *gpu_create_texture_from_bytes(void *buffer, int width, int height, int format);
void console_info(char *s);

typedef struct {
	char name[256];
	int pixel_type;
} channel_t;

void *io_exr_parse(uint8_t *buf) {
	if (buf[0] != 0x76 || buf[1] != 0x2f || buf[2] != 0x31 || buf[3] != 0x01) {
		return NULL;
	}
	size_t pos = 0;
	pos += 4;
	pos += 4; // version

	int width = 0;
	int height = 0;
	int bits = 16;
	int pixel_type = 0;
	channel_t channels[4];
	int num_channels = 0;

	while (1) {
		char name[256];
		int i = 0;
		while (buf[pos] != 0) {
			name[i++] = (char)buf[pos];
			pos++;
		}
		name[i] = 0;
		pos++; // null

		if (strlen(name) == 0) {
			break; // end of header
		}

		char attr_type[256];
		i = 0;
		while (buf[pos] != 0) {
			attr_type[i++] = (char)buf[pos];
			pos++;
		}
		attr_type[i] = 0;
		pos++; // null

		uint32_t attr_size = *(uint32_t *)(buf + pos);
		pos += 4;

		if (strcmp(name, "channels") == 0 && strcmp(attr_type, "chlist") == 0) {
			size_t chpos = pos;
			while (1) {
				char chname[256];
				i = 0;
				while (buf[chpos] != 0) {
					chname[i++] = (char)buf[chpos];
					chpos++;
				}
				chname[i] = 0;
				chpos++; // null
				if (strlen(chname) == 0) {
					break;
				}

				int32_t chpixel_type = *(int32_t *)(buf + chpos); chpos += 4;
				chpos += 4; // pLinear
				chpos += 4; // reserved
				chpos += 4; // xSampling
				chpos += 4; // ySampling
				strcpy(channels[num_channels].name, chname);
				channels[num_channels].pixel_type = chpixel_type;
				num_channels++;
			}
		}
		else if (strcmp(name, "dataWindow") == 0 || strcmp(name, "displayWindow") == 0) {
			int32_t xMin = *(int32_t *)(buf + pos);
			int32_t yMin = *(int32_t *)(buf + pos + 4);
			int32_t xMax = *(int32_t *)(buf + pos + 8);
			int32_t yMax = *(int32_t *)(buf + pos + 12);
			if (strcmp(name, "dataWindow") == 0) {
				width = xMax - xMin + 1;
				height = yMax - yMin + 1;
			}
		}
		else if (strcmp(name, "compression") == 0) {
			uint8_t comp = buf[pos];
			if (comp != 0) {
				console_info("Error: Compressed exr files not yet implemented");
				return NULL;
			}
		}
		pos += attr_size;
	}

	pixel_type = channels[0].pixel_type;
	bits = (pixel_type == 1) ? 16 : 32;

	uint32_t *line_offset_table = (uint32_t *)malloc(height * sizeof(uint32_t));
	for (int y = 0; y < height; y++) {
		uint32_t lo = *(uint32_t *)(buf + pos);
		line_offset_table[y] = lo;
		pos += 8;
	}

	int is_bgr = (num_channels == 3 && strcmp(channels[0].name, "B") == 0 && strcmp(channels[1].name, "G") == 0 && strcmp(channels[2].name, "R") == 0);
	int is_16bit = (bits == 16);
	int channel_bytes = is_16bit ? 2 : 4;
	size_t image_size = (size_t)width * height * 4 * channel_bytes;
	uint8_t *pixels = (uint8_t *)malloc(image_size);

	for (int y = 0; y < height; y++) {
		uint32_t scan_line_pos = line_offset_table[y];
		uint32_t off = scan_line_pos + 8;

		for (int x = 0; x < width; x++) {
			size_t outi = ((size_t)y * width + x) * 4 * channel_bytes;
			float r = 0.0f;
			float g = 0.0f;
			float b = 0.0f;
			float a = 1.0f;

			if (is_bgr) {
				if (is_16bit) {
					uint16_t b_h = *(uint16_t *)(buf + off); off += 2;
					uint16_t g_h = *(uint16_t *)(buf + off); off += 2;
					uint16_t r_h = *(uint16_t *)(buf + off); off += 2;
					*(uint16_t *)(pixels + outi + 0 * channel_bytes) = r_h;
					*(uint16_t *)(pixels + outi + 1 * channel_bytes) = g_h;
					*(uint16_t *)(pixels + outi + 2 * channel_bytes) = b_h;
					*(uint16_t *)(pixels + outi + 3 * channel_bytes) = 0x3c00;
				}
				else {
					float b = *(float *)(buf + off); off += 4;
					float g = *(float *)(buf + off); off += 4;
					float r = *(float *)(buf + off); off += 4;
					*(float *)(pixels + outi + 0) = r;
					*(float *)(pixels + outi + 4) = g;
					*(float *)(pixels + outi + 8) = b;
					*(float *)(pixels + outi + 12) = a;
				}
			}
			else {
				if (is_16bit) {
					uint16_t v_h = *(uint16_t *)(buf + off); off += 2;
					*(uint16_t *)(pixels + outi + 0 * channel_bytes) = v_h;
					*(uint16_t *)(pixels + outi + 1 * channel_bytes) = v_h;
					*(uint16_t *)(pixels + outi + 2 * channel_bytes) = v_h;
					*(uint16_t *)(pixels + outi + 3 * channel_bytes) = 0x3c00;
				}
				else {
					float v = *(float *)(buf + off); off += 4;
					*(float *)(pixels + outi + 0) = v;
					*(float *)(pixels + outi + 4) = v;
					*(float *)(pixels + outi + 8) = v;
					*(float *)(pixels + outi + 12) = a;
				}
			}
		}
	}

	free(line_offset_table);

	buffer_t *b = (buffer_t *)malloc(sizeof(buffer_t));
	b->buffer = pixels;
	b->length = b->capacity = (uint32_t)image_size;
	int format = is_16bit ? GPU_TEXTURE_FORMAT_RGBA64 : GPU_TEXTURE_FORMAT_RGBA128;
	return gpu_create_texture_from_bytes(b, width, height, format);
}
