#include "string.h"

#include <stdlib.h>
#include <stdbool.h>

void *memset(void *ptr, int value, size_t num) {
	unsigned char *data = (unsigned char *)ptr;
	for (size_t i = 0; i < num; ++i) {
		data[i] = (unsigned char)value;
	}
	return ptr;
}

void *memcpy(void *destination, const void *source, size_t num) {
	unsigned char *s = (unsigned char *)source;
	unsigned char *d = (unsigned char *)destination;
	for (size_t i = 0; i < num; ++i) {
		d[i] = s[i];
	}
	return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
	unsigned char *p1 = (unsigned char *)ptr1;
	unsigned char *p2 = (unsigned char *)ptr2;
	for (size_t i = 0; i < num; ++i) {
		if (p1[i] != p2[i]) {
			return (int)p1[i] - (int)p2[i];
		}
	}
	return 0;
}

void *memmove(void *destination, const void *source, size_t num) {
	unsigned char *d = (unsigned char *)destination;
	unsigned char *s = (unsigned char *)source;
	if (d < s || d == s) {
		for (size_t i = 0; i < num; ++i) {
			d[i] = s[i];
		}
	}
	else {
		for (size_t i = num; i > 0; --i) {
			d[i - 1] = s[i - 1];
		}
	}
	return destination;
}

void *memchr(const void *str, int c, size_t num) {
	unsigned char *ptr    = (unsigned char *)str;
	unsigned char  target = (unsigned char)c;
	for (size_t i = 0; i < num; i++) {
		if (ptr[i] == target) {
			return (void *)(ptr + i);
		}
	}
	return NULL;
}

size_t strlen(const char *str) {
	size_t size = 0;
	while (true) {
		if (str[size] == 0) {
			return size;
		}
		++size;
	}
	return 0;
}

char *strcpy(char *destination, const char *source) {
	for (size_t i = 0;; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
}

char *strncpy(char *destination, const char *source, size_t num) {
	for (size_t i = 0; i < num; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
}

char *strcat(char *destination, const char *source) {
	size_t di = 0;
	while (destination[di] != 0) {
		++di;
	}
	for (size_t si = 0;; ++si) {
		destination[di] = source[si];
		if (source[si] == 0) {
			return destination;
		}
		++di;
	}
	return destination;
}

char *strdup(const char *str) {
	if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str) + 1;
    char *copy = (char *)malloc(len);
    return strcpy(copy, str);
}

char *strstr(const char *str1, const char *str2) {
	for (size_t i1 = 0;; ++i1) {
		if (str1[i1] == 0) {
			return NULL;
		}
		for (size_t i2 = 0;; ++i2) {
			if (str2[i2] == 0) {
				return (char *)&str1[i1];
			}
			if (str1[i1 + i2] != str2[i2]) {
				break;
			}
		}
	}
}

int strcmp(const char *str1, const char *str2) {
	for (size_t i = 0;; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
}

int strncmp(const char *str1, const char *str2, size_t num) {
	for (size_t i = 0; i < num; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
	return 0;
}

size_t wcslen(const wchar_t *str) {
	size_t size = 0;
	while (true) {
		if (str[size] == 0) {
			return size;
		}
		++size;
	}
	return 0;
}

wchar_t *wcscpy(wchar_t *destination, const wchar_t *source) {
	for (size_t i = 0;; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
}

wchar_t *wcsncpy(wchar_t *destination, const wchar_t *source, size_t num) {
	for (size_t i = 0; i < num; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
}

wchar_t *wcscat(wchar_t *destination, const wchar_t *source) {
	size_t di = 0;
	while (destination[di] != 0) {
		++di;
	}
	for (size_t si = 0;; ++si) {
		destination[di] = source[si];
		if (source[si] == 0) {
			return destination;
		}
		++di;
	}
	return destination;
}

wchar_t *wcsstr(wchar_t *str1, const wchar_t *str2) {
	for (size_t i1 = 0;; ++i1) {
		if (str1[i1] == 0) {
			return NULL;
		}
		for (size_t i2 = 0;; ++i2) {
			if (str2[i2] == 0) {
				return &str1[i1];
			}
			if (str1[i1 + i2] != str2[i2]) {
				break;
			}
		}
	}
}

int wcscmp(const wchar_t *str1, const wchar_t *str2) {
	for (size_t i = 0;; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
}

int wcsncmp(const wchar_t *str1, const wchar_t *str2, size_t num) {
	for (size_t i = 0; i < num; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
	return 0;
}

char *stpcpy(char *destination, const char *source) {
	for (size_t i = 0;; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return &destination[i];
		}
	}
}
