#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file filewriter.h
    \brief Provides an API very similar to fwrite and friends but uses a directory that can actually used for persistent file storage. This can later be read
   using the kinc_file_reader-functions and KINC_FILE_TYPE_SAVE.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_file_writer {
	void *file;
	const char *filename;
	bool mounted;
} kinc_file_writer_t;

/// <summary>
/// Opens a file for writing.
/// </summary>
/// <param name="reader">The writer to initialize for writing</param>
/// <param name="filepath">A filepath to identify a file</param>
/// <returns>Whether the file could be opened</returns>
bool kinc_file_writer_open(kinc_file_writer_t *writer, const char *filepath);

/// <summary>
/// Writes data to a file starting from the current writing-position and increases the writing-position accordingly.
/// </summary>
/// <param name="reader">The writer to write to</param>
/// <param name="data">A pointer to read the data from</param>
/// <param name="size">The amount of data to write in bytes</param>
void kinc_file_writer_write(kinc_file_writer_t *writer, void *data, int size);

/// <summary>
/// Closes a file.
/// </summary>
/// <param name="reader">The file to close</param>
void kinc_file_writer_close(kinc_file_writer_t *writer);

#ifdef KINC_IMPLEMENTATION_IO
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#if !defined(KINC_CONSOLE)

#include "filewriter.h"

#undef KINC_IMPLEMENTATION
#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/system.h>
#define KINC_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

#if defined(KINC_WINDOWS)
#include <kinc/backend/MiniWindows.h>
#endif

#if defined(KINC_PS4) || defined(KINC_SWITCH)
#define MOUNT_SAVES
bool mountSaveData(bool);
void unmountSaveData();
#endif

bool kinc_file_writer_open(kinc_file_writer_t *writer, const char *filepath) {
	writer->file = NULL;
	writer->mounted = false;
#ifdef MOUNT_SAVES
	if (!mountSaveData(true)) {
		return false;
	}
	writer->mounted = true;
#endif
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
#ifdef MOUNT_SAVES
	if (writer->mounted) {
		writer->mounted = false;
		unmountSaveData();
	}
#endif
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

#endif

#ifdef __cplusplus
}
#endif
