#pragma once

#include <kinc/global.h>
#include <stdbool.h>

/*! \file filewriter.h
    \brief Provides an API very similar to fwrite and friends but uses a directory that can actually used for persistent file storage. This can later be read
   using the kinc_file_reader-functions and KINC_FILE_TYPE_SAVE.
*/

typedef struct kinc_file_writer {
	void *file;
	const char *filename;
} kinc_file_writer_t;

bool kinc_file_writer_open(kinc_file_writer_t *writer, const char *filepath);
void kinc_file_writer_write(kinc_file_writer_t *writer, void *data, int size);
void kinc_file_writer_close(kinc_file_writer_t *writer);

#ifdef KINC_IMPLEMENTATION_IO
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include "filewriter.h"

#undef KINC_IMPLEMENTATION
#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/system.h>
#define KINC_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

#if defined(KINC_WINDOWS)
#include <kinc/backend/miniwindows.h>
#endif

bool kinc_file_writer_open(kinc_file_writer_t *writer, const char *filepath) {
	writer->file = NULL;

	char path[1001];
	strcpy(path, kinc_internal_save_path());
	strcat(path, filepath);

#ifdef KINC_WINDOWS
	wchar_t wpath[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
	writer->file = CreateFileW(wpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
	writer->file = fopen(path, "wb");
#endif
	if (writer->file == NULL) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not open file %s.", filepath);
		return false;
	}
	return true;
}

void kinc_file_writer_close(kinc_file_writer_t *writer) {
	if (writer->file != NULL) {
#ifdef KINC_WINDOWS
		CloseHandle(writer->file);
#else
		fclose((FILE *)writer->file);
#endif
		writer->file = NULL;
	}
}

void kinc_file_writer_write(kinc_file_writer_t *writer, void *data, int size) {
#ifdef KINC_WINDOWS
	DWORD written = 0;
	WriteFile(writer->file, data, (DWORD)size, &written, NULL);
#else
	fwrite(data, 1, size, (FILE *)writer->file);
#endif
}

#endif
