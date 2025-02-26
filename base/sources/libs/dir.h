#pragma once

#include <stdbool.h>

typedef struct directory {
	void *handle;
} directory;

typedef struct file {
	bool valid;
	char name[256];
} file;

directory open_dir(const char *dirname);
file read_next_file(directory *dir);
void close_dir(directory *dir);
