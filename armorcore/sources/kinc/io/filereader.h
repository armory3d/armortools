#pragma once

#include <kinc/global.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*! \file filereader.h
    \brief Provides an API very similar to fread and friends but handles the intricacies of where files are actually hidden on each supported system.
*/

#ifndef KINC_DEBUGDIR
#define KINC_DEBUGDIR "out"
#endif

#ifdef KINC_ANDROID
struct AAsset;
struct __sFILE;
typedef struct __sFILE FILE;
#endif

#define KINC_FILE_TYPE_ASSET 0
#define KINC_FILE_TYPE_SAVE 1

typedef struct kinc_file_reader {
	void *data; // A file handle or a more complex structure
	size_t size;
	size_t offset; // Needed by some implementations

	bool (*close)(struct kinc_file_reader *reader);
	size_t (*read)(struct kinc_file_reader *reader, void *data, size_t size);
	size_t (*pos)(struct kinc_file_reader *reader);
	bool (*seek)(struct kinc_file_reader *reader, size_t pos);
} kinc_file_reader_t;

bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filepath, int type);
bool kinc_file_reader_from_memory(kinc_file_reader_t *reader, void *data, size_t size);
void kinc_file_reader_set_callback(bool (*callback)(kinc_file_reader_t *reader, const char *filename, int type));
bool kinc_file_reader_close(kinc_file_reader_t *reader);
size_t kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size);
size_t kinc_file_reader_size(kinc_file_reader_t *reader);
size_t kinc_file_reader_pos(kinc_file_reader_t *reader);
bool kinc_file_reader_seek(kinc_file_reader_t *reader, size_t pos);
float kinc_read_f32le(uint8_t *data);
float kinc_read_f32be(uint8_t *data);
uint64_t kinc_read_u64le(uint8_t *data);
uint64_t kinc_read_u64be(uint8_t *data);
int64_t kinc_read_s64le(uint8_t *data);
int64_t kinc_read_s64be(uint8_t *data);
uint32_t kinc_read_u32le(uint8_t *data);
uint32_t kinc_read_u32be(uint8_t *data);
int32_t kinc_read_s32le(uint8_t *data);
int32_t kinc_read_s32be(uint8_t *data);
uint16_t kinc_read_u16le(uint8_t *data);
uint16_t kinc_read_u16be(uint8_t *data);
int16_t kinc_read_s16le(uint8_t *data);
int16_t kinc_read_s16be(uint8_t *data);
uint8_t kinc_read_u8(uint8_t *data);
int8_t kinc_read_s8(uint8_t *data);

void kinc_internal_set_files_location(char *dir);
char *kinc_internal_get_files_location(void);
bool kinc_internal_file_reader_callback(kinc_file_reader_t *reader, const char *filename, int type);
bool kinc_internal_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type);

#ifdef KINC_IMPLEMENTATION_IO
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include "filereader.h"

#undef KINC_IMPLEMENTATION
#include <kinc/system.h>
#define KINC_IMPLEMENTATION

#ifdef KINC_ANDROID
#include <kinc/backend/Android.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef KINC_WINDOWS
#include <malloc.h>
#include <memory.h>
#endif

static bool memory_close_callback(kinc_file_reader_t *reader) {
	return true;
}

static size_t memory_read_callback(kinc_file_reader_t *reader, void *data, size_t size) {
	size_t read_size = reader->size - reader->offset < size ? reader->size - reader->offset : size;
	memcpy(data, (uint8_t *)reader->data + reader->offset, read_size);
	reader->offset += read_size;
	return read_size;
}

static size_t memory_pos_callback(kinc_file_reader_t *reader) {
	return reader->offset;
}

static bool memory_seek_callback(kinc_file_reader_t *reader, size_t pos) {
	reader->offset = pos;
	return true;
}

bool kinc_file_reader_from_memory(kinc_file_reader_t *reader, void *data, size_t size)
{
	memset(reader, 0, sizeof(kinc_file_reader_t));
	reader->data = data;
	reader->size = size;
	reader->read = memory_read_callback;
	reader->pos = memory_pos_callback;
	reader->seek = memory_seek_callback;
	reader->close = memory_close_callback;
	return true;
}

#ifdef KINC_IOS
const char *iphonegetresourcepath(void);
#endif

#ifdef KINC_MACOS
const char *macgetresourcepath(void);
#endif

#if defined(KINC_WINDOWS)
#include <kinc/backend/miniwindows.h>
#endif

static char *fileslocation = NULL;
static bool (*file_reader_callback)(kinc_file_reader_t *reader, const char *filename, int type) = NULL;
#ifdef KINC_WINDOWS
static wchar_t wfilepath[1001];
#endif

void kinc_internal_set_files_location(char *dir) {
	fileslocation = dir;
}

char *kinc_internal_get_files_location(void) {
	return fileslocation;
}

bool kinc_internal_file_reader_callback(kinc_file_reader_t *reader, const char *filename, int type) {
	return file_reader_callback ? file_reader_callback(reader, filename, type) : false;
}

#if defined(KINC_WINDOWS)
static size_t kinc_libc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size) {
	DWORD readBytes = 0;
	if (ReadFile(reader->data, data, (DWORD)size, &readBytes, NULL)) {
		return (size_t)readBytes;
	}
	else {
		return 0;
	}
}

static bool kinc_libc_file_reader_seek(kinc_file_reader_t *reader, size_t pos) {
	// TODO: make this 64-bit compliant
	SetFilePointer(reader->data, (LONG)pos, NULL, FILE_BEGIN);
	return true;
}

static bool kinc_libc_file_reader_close(kinc_file_reader_t *reader) {
	CloseHandle(reader->data);
	return true;
}

static size_t kinc_libc_file_reader_pos(kinc_file_reader_t *reader) {
	// TODO: make this 64-bit compliant
	return (size_t)SetFilePointer(reader->data, 0, NULL, FILE_CURRENT);
}
#else
static size_t kinc_libc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size) {
	return fread(data, 1, size, (FILE *)reader->data);
}

static bool kinc_libc_file_reader_seek(kinc_file_reader_t *reader, size_t pos) {
	fseek((FILE *)reader->data, pos, SEEK_SET);
	return true;
}

static bool kinc_libc_file_reader_close(kinc_file_reader_t *reader) {
	if (reader->data != NULL) {
		fclose((FILE *)reader->data);
		reader->data = NULL;
	}
	return true;
}

static size_t kinc_libc_file_reader_pos(kinc_file_reader_t *reader) {
	return ftell((FILE *)reader->data);
}
#endif

bool kinc_internal_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type) {
	char filepath[1001];
#ifdef KINC_IOS
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? kinc_internal_save_path() : iphonegetresourcepath());
	if (type != KINC_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, KINC_DEBUGDIR);
		strcat(filepath, "/");
	}

	strcat(filepath, filename);
#endif
#ifdef KINC_MACOS
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? kinc_internal_save_path() : macgetresourcepath());
	if (type != KINC_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, KINC_DEBUGDIR);
		strcat(filepath, "/");
	}
	strcat(filepath, filename);
#endif
#ifdef KINC_WINDOWS
	if (type == KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, kinc_internal_save_path());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
	size_t filepathlength = strlen(filepath);
	for (size_t i = 0; i < filepathlength; ++i)
		if (filepath[i] == '/')
			filepath[i] = '\\';
#endif
#if defined(KINC_LINUX) || defined(KINC_ANDROID)
	if (type == KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, kinc_internal_save_path());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
#endif
#ifdef KINC_WASM
	strcpy(filepath, filename);
#endif

#ifdef KINC_WINDOWS
	// Drive letter or network
	bool isAbsolute = (filename[1] == ':' && filename[2] == '\\') || (filename[0] == '\\' && filename[1] == '\\');
#else
	bool isAbsolute = filename[0] == '/';
#endif

	if (isAbsolute) {
		strcpy(filepath, filename);
	}
	else if (fileslocation != NULL && type != KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, fileslocation);
		strcat(filepath, "/");
		strcat(filepath, filename);
	}

#ifdef KINC_WINDOWS
	MultiByteToWideChar(CP_UTF8, 0, filepath, -1, wfilepath, 1000);
	reader->data = CreateFileW(wfilepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (reader->data == INVALID_HANDLE_VALUE) {
		return false;
	}
#else
	reader->data = fopen(filepath, "rb");
	if (reader->data == NULL) {
		return false;
	}
#endif

#ifdef KINC_WINDOWS
	// TODO: make this 64-bit compliant
	reader->size = (size_t)GetFileSize(reader->data, NULL);
#else
	fseek((FILE *)reader->data, 0, SEEK_END);
	reader->size = ftell((FILE *)reader->data);
	fseek((FILE *)reader->data, 0, SEEK_SET);
#endif

	reader->read = kinc_libc_file_reader_read;
	reader->seek = kinc_libc_file_reader_seek;
	reader->close = kinc_libc_file_reader_close;
	reader->pos = kinc_libc_file_reader_pos;

	return true;
}

#if !defined(KINC_ANDROID)
bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type) {
	memset(reader, 0, sizeof(*reader));
	return kinc_internal_file_reader_callback(reader, filename, type) ||
	       kinc_internal_file_reader_open(reader, filename, type);
}
#endif

void kinc_file_reader_set_callback(bool (*callback)(kinc_file_reader_t *reader, const char *filename, int type)) {
	file_reader_callback = callback;
}

size_t kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size) {
	return reader->read(reader, data, size);
}

bool kinc_file_reader_seek(kinc_file_reader_t *reader, size_t pos) {
	return reader->seek(reader, pos);
}

bool kinc_file_reader_close(kinc_file_reader_t *reader) {
	return reader->close(reader);
}

size_t kinc_file_reader_pos(kinc_file_reader_t *reader) {
	return reader->pos(reader);
}

size_t kinc_file_reader_size(kinc_file_reader_t *reader) {
	return reader->size;
}

float kinc_read_f32le(uint8_t *data) {
#ifdef KINC_LITTLE_ENDIAN // speed optimization
	return *(float *)data;
#else // works on all architectures
	int i = (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return *(float *)&i;
#endif
}

uint64_t kinc_read_u64le(uint8_t *data) {
#ifdef KINC_LITTLE_ENDIAN
	return *(uint64_t *)data;
#else
	return ((uint64_t)data[0] << 0) | ((uint64_t)data[1] << 8) | ((uint64_t)data[2] << 16) | ((uint64_t)data[3] << 24) | ((uint64_t)data[4] << 32) |
	       ((uint64_t)data[5] << 40) | ((uint64_t)data[6] << 48) | ((uint64_t)data[7] << 56);
#endif
}

int64_t kinc_read_s64le(uint8_t *data) {
#ifdef KINC_LITTLE_ENDIAN
	return *(int64_t *)data;
#else
	return ((int64_t)data[0] << 0) | ((int64_t)data[1] << 8) | ((int64_t)data[2] << 16) | ((int64_t)data[3] << 24) | ((int64_t)data[4] << 32) |
	       ((int64_t)data[5] << 40) | ((int64_t)data[6] << 48) | ((int64_t)data[7] << 56);
#endif
}

uint32_t kinc_read_u32le(uint8_t *data) {
#ifdef KINC_LITTLE_ENDIAN
	return *(uint32_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

int32_t kinc_read_s32le(uint8_t *data) {
#ifdef KINC_LITTLE_ENDIAN
	return *(int32_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

uint16_t kinc_read_u16le(uint8_t *data) {
#ifdef KINC_LITTLE_ENDIAN
	return *(uint16_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

int16_t kinc_read_s16le(uint8_t *data) {
#ifdef KINC_LITTLE_ENDIAN
	return *(int16_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

uint8_t kinc_read_u8(uint8_t *data) {
	return *data;
}

int8_t kinc_read_s8(uint8_t *data) {
	return *(int8_t *)data;
}

#endif
