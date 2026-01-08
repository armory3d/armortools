/*
 * This file is in the public domain.  Use it as you see fit.
 * Originally written by Tim Kientzle, March 2009.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <direct.h>
#endif

#define BLOCKSIZE    512
#define MAX_PATH_LEN 1024

static int system_mkdir(char *pathname, int mode) {
#if defined(_WIN32)
	(void)mode; /* UNUSED */
	return _mkdir(pathname);
#else
	return mkdir(pathname, mode);
#endif
}

/* Parse an octal number, ignoring leading and trailing nonsense. */
static size_t parseoct(const char *p, size_t n) {
	size_t i = 0;

	while ((*p < '0' || *p > '7') && n > 0) {
		++p;
		--n;
	}
	while (*p >= '0' && *p <= '7' && n > 0) {
		i *= 8;
		i += *p - '0';
		++p;
		--n;
	}
	return (i);
}

/* Returns true if this is 512 zero bytes. */
static int is_end_of_archive(const char *p) {
	int n;
	for (n = 0; n < BLOCKSIZE; ++n)
		if (p[n] != '\0')
			return (0);
	return (1);
}

/* Get the directory name from the path. */
static void get_dirname(const char *path, char *dir, size_t dir_size) {
	strncpy(dir, path, dir_size - 1);
	dir[dir_size - 1] = '\0';

	char *last_slash = NULL;
	char *q          = dir;
	while (*q) {
		if (*q == '/' || *q == '\\') {
			last_slash = q;
		}
		q++;
	}

	if (last_slash != NULL) {
		*last_slash = '\0';
	}
	else {
		strcpy(dir, ".");
	}
}

/* Create a directory using the full path, including parent directories as necessary. */
static void create_dir_full(char *fullpath, int mode) {
	char *p;
	int   r;

	/* Strip trailing '/' */
	if (fullpath[strlen(fullpath) - 1] == '/')
		fullpath[strlen(fullpath) - 1] = '\0';

	/* Try creating the directory. */
	r = system_mkdir(fullpath, mode);
	if (r != 0) {
		/* On failure, try creating parent directory. */
		p = strrchr(fullpath, '/');
		if (p == NULL) {
#if defined(_WIN32)
			p = strrchr(fullpath, '\\');
#endif
		}
		if (p != NULL && p != fullpath) {
			*p = '\0';
			create_dir_full(fullpath, 0755);
			*p = '/';
			r  = system_mkdir(fullpath, mode);
		}
	}
	if (r != 0)
		fprintf(stderr, "Could not create directory %s\n", fullpath);
}

/* Create a file using the full path, including parent directory as necessary. */
static FILE *create_file_full(char *fullpath, int mode) {
	FILE *f;

	f = fopen(fullpath, "wb+");
	if (f == NULL) {
		/* Try creating parent dir and then creating file. */
		char *p = strrchr(fullpath, '/');
		if (p == NULL) {
#if defined(_WIN32)
			p = strrchr(fullpath, '\\');
#endif
		}
		if (p != NULL) {
			*p = '\0';
			create_dir_full(fullpath, 0755);
			*p = '/';
			f  = fopen(fullpath, "wb+");
		}
	}
	return (f);
}

/* Verify the tar checksum. */
static int verify_checksum(const char *p) {
	int n, u = 0;
	for (n = 0; n < BLOCKSIZE; ++n) {
		if (n < 148 || n > 155)
			/* Standard tar checksum adds unsigned bytes. */
			u += ((unsigned char *)p)[n];
		else
			u += 0x20;
	}
	return (u == (int)parseoct(p + 148, 8));
}

/* Extract a tar archive. */
static void untar(FILE *a, const char *path) {
	char   buff[BLOCKSIZE];
	char   extract_dir[MAX_PATH_LEN];
	char   fullpath[MAX_PATH_LEN];
	FILE  *f = NULL;
	size_t bytes_read;
	size_t filesize;
	char  *override_name = NULL;
	char  *override_link = NULL;

	get_dirname(path, extract_dir, sizeof(extract_dir));

	for (;;) {
		bytes_read = fread(buff, 1, BLOCKSIZE, a);
		if (bytes_read < BLOCKSIZE) {
			fprintf(stderr, "Short read on %s: expected %d, got %d\n", path, BLOCKSIZE, (int)bytes_read);
			if (override_name)
				free(override_name);
			if (override_link)
				free(override_link);
			return;
		}
		if (is_end_of_archive(buff)) {
			if (override_name)
				free(override_name);
			if (override_link)
				free(override_link);
			return;
		}
		if (!verify_checksum(buff)) {
			fprintf(stderr, "Checksum failure\n");
			if (override_name)
				free(override_name);
			if (override_link)
				free(override_link);
			return;
		}

		char entry_name[MAX_PATH_LEN];
		entry_name[0] = '\0';
		char link_name[101];
		link_name[0]  = '\0';
		char typeflag = buff[156];
		filesize      = parseoct(buff + 124, 12);

		/* Handle override name from previous 'L' or 'K' */
		if (override_name) {
			strncpy(entry_name, override_name, MAX_PATH_LEN - 1);
			entry_name[MAX_PATH_LEN - 1] = '\0';
			free(override_name);
			override_name = NULL;
		}
		else {
			char short_name[101];
			memcpy(short_name, buff, 100);
			short_name[100] = '\0';
			strcpy(entry_name, short_name);

			/* Handle ustar prefix */
			if (memcmp(buff + 257, "ustar", 5) == 0) {
				char prefix[156];
				memcpy(prefix, buff + 345, 155);
				prefix[155] = '\0';
				if (prefix[0] != '\0') {
					strcpy(entry_name, prefix);
					strcat(entry_name, "/");
					strcat(entry_name, short_name);
				}
			}
		}

		if (override_link) {
			strncpy(link_name, override_link, sizeof(link_name) - 1);
			link_name[sizeof(link_name) - 1] = '\0';
			free(override_link);
			override_link = NULL;
		}
		else {
			memcpy(link_name, buff + 157, 100);
			link_name[100] = '\0';
		}

		/* Build fullpath */
		strcpy(fullpath, extract_dir);
		if (strcmp(extract_dir, ".") != 0 && extract_dir[0] != '\0') {
			strcat(fullpath, "/");
		}
		strcat(fullpath, entry_name);

		switch (typeflag) {
		case 'L': /* GNU long name */
		{
			char  *longname  = (char *)malloc(filesize + 1);
			char  *ptr       = longname;
			size_t remaining = filesize;
			while (remaining > 0) {
				bytes_read = fread(buff, 1, BLOCKSIZE, a);
				if (bytes_read < BLOCKSIZE && remaining >= BLOCKSIZE) {
					fprintf(stderr, "Short read on longname: expected %d, got %d\n", BLOCKSIZE, (int)bytes_read);
					free(longname);
					return;
				}
				size_t to_copy = (remaining < BLOCKSIZE) ? (size_t)remaining : BLOCKSIZE;
				memcpy(ptr, buff, to_copy);
				ptr += to_copy;
				remaining -= to_copy;
			}
			*ptr = '\0';
			/* Find the actual string length */
			longname[strnlen(longname, filesize)] = '\0';
			override_name                         = longname;
			continue; /* Skip data writing loop */
		}
		case '1': /* Hardlink */
			filesize = 0;
			break;
		case '2': /* Symlink */
			filesize = 0;
			break;
		case '3':
			filesize = 0;
			break;
		case '4':
			filesize = 0;
			break;
		case '5':
			create_dir_full(fullpath, (int)parseoct(buff + 100, 8));
			filesize = 0;
			break;
		case '6':
			filesize = 0;
			break;
		default:
			f = create_file_full(fullpath, (int)parseoct(buff + 100, 8));
			break;
		}
		while (filesize > 0) {
			bytes_read = fread(buff, 1, BLOCKSIZE, a);
			if (bytes_read < BLOCKSIZE) {
				fprintf(stderr, "Short read on %s: Expected %d, got %d\n", path, BLOCKSIZE, (int)bytes_read);
				if (f != NULL)
					fclose(f);
				if (override_name)
					free(override_name);
				if (override_link)
					free(override_link);
				return;
			}
			if (filesize < BLOCKSIZE)
				bytes_read = (size_t)filesize;
			if (f != NULL) {
				if (fwrite(buff, 1, bytes_read, f) != bytes_read) {
					fprintf(stderr, "Failed write to %s\n", fullpath);
					fclose(f);
					f = NULL;
				}
			}
			filesize -= bytes_read;
		}
		if (f != NULL) {
			fclose(f);
			f = NULL;
		}
	}
}

void untar_here(const char *path) {
	FILE *a = fopen(path, "rb");
	if (a == NULL)
		fprintf(stderr, "Unable to open %s\n", path);
	else {
		untar(a, path);
		fclose(a);
	}
}
