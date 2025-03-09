#pragma once

#include <iron_global.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define IRON_OUTDIR "out"
#ifdef IRON_ANDROID
struct AAsset;
struct __sFILE;
typedef struct __sFILE FILE;
#endif

#define IRON_FILE_TYPE_ASSET 0
#define IRON_FILE_TYPE_SAVE 1

typedef struct iron_file_reader {
	void *data; // A file handle or a more complex structure
	size_t size;
	size_t offset; // Needed by some implementations

	bool (*close)(struct iron_file_reader *reader);
	size_t (*read)(struct iron_file_reader *reader, void *data, size_t size);
	size_t (*pos)(struct iron_file_reader *reader);
	bool (*seek)(struct iron_file_reader *reader, size_t pos);
} iron_file_reader_t;

bool iron_file_reader_open(iron_file_reader_t *reader, const char *filepath, int type);
bool iron_file_reader_from_memory(iron_file_reader_t *reader, void *data, size_t size);
void iron_file_reader_set_callback(bool (*callback)(iron_file_reader_t *reader, const char *filename, int type));
bool iron_file_reader_close(iron_file_reader_t *reader);
size_t iron_file_reader_read(iron_file_reader_t *reader, void *data, size_t size);
size_t iron_file_reader_size(iron_file_reader_t *reader);
size_t iron_file_reader_pos(iron_file_reader_t *reader);
bool iron_file_reader_seek(iron_file_reader_t *reader, size_t pos);
float iron_read_f32le(uint8_t *data);
float iron_read_f32be(uint8_t *data);
uint64_t iron_read_u64le(uint8_t *data);
uint64_t iron_read_u64be(uint8_t *data);
int64_t iron_read_s64le(uint8_t *data);
int64_t iron_read_s64be(uint8_t *data);
uint32_t iron_read_u32le(uint8_t *data);
uint32_t iron_read_u32be(uint8_t *data);
int32_t iron_read_s32le(uint8_t *data);
int32_t iron_read_s32be(uint8_t *data);
uint16_t iron_read_u16le(uint8_t *data);
uint16_t iron_read_u16be(uint8_t *data);
int16_t iron_read_s16le(uint8_t *data);
int16_t iron_read_s16be(uint8_t *data);
uint8_t iron_read_u8(uint8_t *data);
int8_t iron_read_s8(uint8_t *data);

void iron_internal_set_files_location(char *dir);
char *iron_internal_get_files_location(void);
bool iron_internal_file_reader_callback(iron_file_reader_t *reader, const char *filename, int type);
bool iron_internal_file_reader_open(iron_file_reader_t *reader, const char *filename, int type);

typedef struct iron_file_writer {
	void *file;
	const char *filename;
} iron_file_writer_t;

bool iron_file_writer_open(iron_file_writer_t *writer, const char *filepath);
void iron_file_writer_write(iron_file_writer_t *writer, void *data, int size);
void iron_file_writer_close(iron_file_writer_t *writer);
