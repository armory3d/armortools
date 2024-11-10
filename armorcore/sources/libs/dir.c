// Copyright (c) 2022 the Kongruent development team

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

//   1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.

//   2. Altered source versions must be plainly marked as such, and must not be
//      misrepresented as being the original software.

//   3. This notice may not be removed or altered from any source distribution.

#include "dir.h"
#include <stddef.h>
#include <stdio.h>

#ifdef _WIN32

#include <Windows.h>

directory open_dir(const char *dirname) {
	char pattern[1024];
	strcpy(pattern, dirname);
	strcat(pattern, "\\*");

	WIN32_FIND_DATAA data;
	directory dir;
	dir.handle = FindFirstFileA(pattern, &data);
	if (dir.handle == INVALID_HANDLE_VALUE) {
		return dir;
	}
	FindNextFileA(dir.handle, &data);
	return dir;
}

file read_next_file(directory *dir) {
	WIN32_FIND_DATAA data;
	file file;
	file.valid = FindNextFileA(dir->handle, &data) != 0;
	if (file.valid) {
		strcpy(file.name, data.cFileName);
	}
	else {
		file.name[0] = 0;
	}
	return file;
}

void close_dir(directory *dir) {
	FindClose(dir->handle);
}

#else

#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

directory open_dir(const char *dirname) {
	directory dir;
	dir.handle = opendir(dirname);
	return dir;
}

file read_next_file(directory *dir) {
	struct dirent *entry = readdir(dir->handle);

	while (entry != NULL && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
		entry = readdir(dir->handle);
	}

	file f;
	f.valid = entry != NULL;

	if (f.valid) {
		strcpy(f.name, entry->d_name);
	}

	return f;
}

void close_dir(directory *dir) {}

#endif
