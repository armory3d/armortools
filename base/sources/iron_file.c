
#include "iron_file.h"
#include <iron_system.h>
#ifdef IRON_ANDROID
#include <backends/android_system.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef IRON_WINDOWS
#include <malloc.h>
#include <memory.h>
#endif

#ifdef IRON_IOS
const char *iron_get_resource_path(void);
#endif

#ifdef IRON_MACOS
const char *iron_get_resource_path(void);
#endif

#ifdef IRON_WINDOWS
#include <backends/windows_mini.h>
#endif

static char *fileslocation = NULL;
#ifdef IRON_WINDOWS
static wchar_t wfilepath[1001];
#endif

void iron_internal_set_files_location(char *dir) {
	fileslocation = dir;
}

char *iron_internal_get_files_location(void) {
	return fileslocation;
}

#ifdef IRON_WINDOWS
static size_t iron_libc_file_reader_read(iron_file_reader_t *reader, void *data, size_t size) {
	DWORD readBytes = 0;
	if (ReadFile(reader->data, data, (DWORD)size, &readBytes, NULL)) {
		return (size_t)readBytes;
	}
	else {
		return 0;
	}
}

static bool iron_libc_file_reader_seek(iron_file_reader_t *reader, size_t pos) {
	// TODO: make this 64-bit compliant
	SetFilePointer(reader->data, (LONG)pos, NULL, FILE_BEGIN);
	return true;
}

static bool iron_libc_file_reader_close(iron_file_reader_t *reader) {
	CloseHandle(reader->data);
	return true;
}

static size_t iron_libc_file_reader_pos(iron_file_reader_t *reader) {
	// TODO: make this 64-bit compliant
	return (size_t)SetFilePointer(reader->data, 0, NULL, FILE_CURRENT);
}
#else
static size_t iron_libc_file_reader_read(iron_file_reader_t *reader, void *data, size_t size) {
	return fread(data, 1, size, (FILE *)reader->data);
}

static bool iron_libc_file_reader_seek(iron_file_reader_t *reader, size_t pos) {
	fseek((FILE *)reader->data, pos, SEEK_SET);
	return true;
}

static bool iron_libc_file_reader_close(iron_file_reader_t *reader) {
	if (reader->data != NULL) {
		fclose((FILE *)reader->data);
		reader->data = NULL;
	}
	return true;
}

static size_t iron_libc_file_reader_pos(iron_file_reader_t *reader) {
	return ftell((FILE *)reader->data);
}
#endif

bool iron_internal_file_reader_open(iron_file_reader_t *reader, const char *filename, int type) {
	char filepath[1001];
#ifdef IRON_IOS
	strcpy(filepath, type == IRON_FILE_TYPE_SAVE ? iron_internal_save_path() : iron_get_resource_path());
	if (type != IRON_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, IRON_OUTDIR);
		strcat(filepath, "/");
	}

	strcat(filepath, filename);
#endif
#ifdef IRON_MACOS
	strcpy(filepath, type == IRON_FILE_TYPE_SAVE ? iron_internal_save_path() : iron_get_resource_path());
	if (type != IRON_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, IRON_OUTDIR);
		strcat(filepath, "/");
	}
	strcat(filepath, filename);
#endif
#ifdef IRON_WINDOWS
	if (type == IRON_FILE_TYPE_SAVE) {
		strcpy(filepath, iron_internal_save_path());
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
#if defined(IRON_LINUX) || defined(IRON_ANDROID)
	if (type == IRON_FILE_TYPE_SAVE) {
		strcpy(filepath, iron_internal_save_path());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
#endif
#ifdef IRON_WASM
	strcpy(filepath, filename);
#endif

#ifdef IRON_WINDOWS
	// Drive letter or network
	bool isAbsolute = (filename[1] == ':' && filename[2] == '\\') || (filename[0] == '\\' && filename[1] == '\\');
#else
	bool isAbsolute = filename[0] == '/';
#endif

	if (isAbsolute) {
		strcpy(filepath, filename);
	}
	else if (fileslocation != NULL && type != IRON_FILE_TYPE_SAVE) {
		strcpy(filepath, fileslocation);
		strcat(filepath, "/");
		strcat(filepath, filename);
	}

#ifdef IRON_WINDOWS
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

#ifdef IRON_WINDOWS
	// TODO: make this 64-bit compliant
	reader->size = (size_t)GetFileSize(reader->data, NULL);
#else
	fseek((FILE *)reader->data, 0, SEEK_END);
	reader->size = ftell((FILE *)reader->data);
	fseek((FILE *)reader->data, 0, SEEK_SET);
#endif

	reader->read = iron_libc_file_reader_read;
	reader->seek = iron_libc_file_reader_seek;
	reader->close = iron_libc_file_reader_close;
	reader->pos = iron_libc_file_reader_pos;

	return true;
}

#if !defined(IRON_ANDROID)
bool iron_file_reader_open(iron_file_reader_t *reader, const char *filename, int type) {
	memset(reader, 0, sizeof(*reader));
	return iron_internal_file_reader_open(reader, filename, type);
}
#endif

size_t iron_file_reader_read(iron_file_reader_t *reader, void *data, size_t size) {
	return reader->read(reader, data, size);
}

bool iron_file_reader_seek(iron_file_reader_t *reader, size_t pos) {
	return reader->seek(reader, pos);
}

bool iron_file_reader_close(iron_file_reader_t *reader) {
	return reader->close(reader);
}

size_t iron_file_reader_pos(iron_file_reader_t *reader) {
	return reader->pos(reader);
}

size_t iron_file_reader_size(iron_file_reader_t *reader) {
	return reader->size;
}

float iron_read_f32le(uint8_t *data) {
	return *(float *)data;
}

uint64_t iron_read_u64le(uint8_t *data) {
	return *(uint64_t *)data;
}

int64_t iron_read_s64le(uint8_t *data) {
	return *(int64_t *)data;
}

uint32_t iron_read_u32le(uint8_t *data) {
	return *(uint32_t *)data;
}

int32_t iron_read_s32le(uint8_t *data) {
	return *(int32_t *)data;
}

uint16_t iron_read_u16le(uint8_t *data) {
	return *(uint16_t *)data;
}

int16_t iron_read_s16le(uint8_t *data) {
	return *(int16_t *)data;
}

uint8_t iron_read_u8(uint8_t *data) {
	return *data;
}

int8_t iron_read_s8(uint8_t *data) {
	return *(int8_t *)data;
}

bool iron_file_writer_open(iron_file_writer_t *writer, const char *filepath) {
	writer->file = NULL;

	char path[1001];
	strcpy(path, iron_internal_save_path());
	strcat(path, filepath);

#ifdef IRON_WINDOWS
	wchar_t wpath[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
	writer->file = CreateFileW(wpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
	writer->file = fopen(path, "wb");
#endif
	if (writer->file == NULL) {
		iron_log("Could not open file %s.", filepath);
		return false;
	}
	return true;
}

void iron_file_writer_close(iron_file_writer_t *writer) {
	if (writer->file != NULL) {
#ifdef IRON_WINDOWS
		CloseHandle(writer->file);
#else
		fclose((FILE *)writer->file);
#endif
		writer->file = NULL;
	}
}

void iron_file_writer_write(iron_file_writer_t *writer, void *data, int size) {
#ifdef IRON_WINDOWS
	DWORD written = 0;
	WriteFile(writer->file, data, (DWORD)size, &written, NULL);
#else
	fwrite(data, 1, size, (FILE *)writer->file);
#endif
}
