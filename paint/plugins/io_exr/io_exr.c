#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "iron_array.h"
#include "iron_gpu.h"
void *gpu_create_texture_from_bytes(void *buffer, int width, int height, int format);
void console_info(char *s);
buffer_t *iron_inflate(buffer_t *bytes, bool raw);

typedef struct {
	char name[256];
	int pixel_type;
} channel_t;

void *io_exr_parse(uint8_t *buf, size_t buf_size) {
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
	int compression = 0;

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
				uint8_t pLinear = buf[chpos]; chpos += 1;  // 1 byte
				chpos += 3;  // Skip reserved (3 bytes)
				int32_t xSampling = *(int32_t *)(buf + chpos); chpos += 4;
				int32_t ySampling = *(int32_t *)(buf + chpos); chpos += 4;

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
			compression = buf[pos];
			if (compression != 0 && compression != 2) {
				console_info("Error: This exr compression type is not yet implemented");
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

	int r_idx = -1;
	int g_idx = -1;
	int b_idx = -1;
	int a_idx = -1;
	for (int c = 0; c < num_channels; c++) {
		if (strcmp(channels[c].name, "R") == 0) r_idx = c;
		else if (strcmp(channels[c].name, "G") == 0) g_idx = c;
		else if (strcmp(channels[c].name, "B") == 0) b_idx = c;
		else if (strcmp(channels[c].name, "A") == 0) a_idx = c;
	}

	bool is_16bit = bits == 16;
	int channel_bytes = is_16bit ? 2 : 4;
	size_t image_size = (size_t)width * height * 4 * channel_bytes;
	uint8_t *pixels = (uint8_t *)malloc(image_size);
	uint8_t *reordered = NULL;

	for (int y = 0; y < height; y++) {
		uint32_t scan_line_pos = line_offset_table[y];
		uint32_t compressed_len = *(uint32_t *)(buf + scan_line_pos + 4);
		uint32_t off = scan_line_pos + 8;
		uint8_t *line_data = NULL;
		buffer_t *decomp;

		if (compression == 0) { // None
			line_data = buf + off;
		}
		// else if (compression == 1) {} // RLE
		else if (compression == 2) { // ZIPS
			buffer_t compressed;
			compressed.buffer = buf + off;
			compressed.length = compressed.capacity = compressed_len;
			decomp = iron_inflate(&compressed, false);
			line_data = decomp->buffer;

			uint8_t *t = line_data + 1;
			uint8_t *stop = line_data + decomp->length;
			int p = line_data[0];
			while (t < stop) {
				int d = *t;
				int orig = (d - 128 + p) & 0xFF;
				p = orig;
				*t = (uint8_t)orig;
				++t;
			}

			if (reordered == NULL) {
				reordered = malloc(decomp->length);
			}
			size_t half = (decomp->length + 1) / 2;
			for (size_t i = 0; i < half; i++) {
				reordered[i * 2] = decomp->buffer[i];
				if (i * 2 + 1 < decomp->length) {
					reordered[i * 2 + 1] = decomp->buffer[half + i];
				}
			}
			line_data = reordered;
		}
		// else if (compression == 3) {} // ZIP

		if (line_data) {
			uint8_t *plane_starts[4];
			size_t plane_size = (size_t)width * channel_bytes;
			for (int c = 0; c < num_channels; c++) {
				plane_starts[c] = line_data + (size_t)c * plane_size;
			}

			for (int x = 0; x < width; x++) {
				size_t outi = ((size_t)y * width + x) * 4 * channel_bytes;

				if (is_16bit) {
					uint16_t vals[4] = {0};
					for (int c = 0; c < num_channels; c++) {
						vals[c] = *(uint16_t *)(plane_starts[c] + (size_t)x * channel_bytes);
					}
					uint16_t r_h = (r_idx >= 0 ? vals[r_idx] : (num_channels > 0 ? vals[0] : 0));
					uint16_t g_h = (g_idx >= 0 ? vals[g_idx] : r_h);
					uint16_t b_h = (b_idx >= 0 ? vals[b_idx] : r_h);
					uint16_t a_h = (a_idx >= 0 ? vals[a_idx] : 0x3c00);

					*(uint16_t *)(pixels + outi + 0 * channel_bytes) = r_h;
					*(uint16_t *)(pixels + outi + 1 * channel_bytes) = g_h;
					*(uint16_t *)(pixels + outi + 2 * channel_bytes) = b_h;
					*(uint16_t *)(pixels + outi + 3 * channel_bytes) = a_h;
				}
				else {
					float vals_f[4] = {0.0f};
					for (int c = 0; c < num_channels; c++) {
						vals_f[c] = *(float *)(plane_starts[c] + (size_t)x * channel_bytes);
					}
					float r = (r_idx >= 0 ? vals_f[r_idx] : (num_channels > 0 ? vals_f[0] : 0.0f));
					float g = (g_idx >= 0 ? vals_f[g_idx] : r);
					float b = (b_idx >= 0 ? vals_f[b_idx] : r);
					float a = (a_idx >= 0 ? vals_f[a_idx] : 1.0f);

					*(float *)(pixels + outi + 0) = r;
					*(float *)(pixels + outi + 4) = g;
					*(float *)(pixels + outi + 8) = b;
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
