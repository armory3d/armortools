#include "string.h"

#include <stdbool.h>

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

char *strstr(const char *str1, const char *str2) {
	for (size_t i1 = 0;; ++i1) {
		if (str1[i1] == 0) {
			return NULL;
		}
		for (size_t i2 = 0;; ++i2) {
			if (str2[i2] == 0) {
				return (char*)&str1[i1];
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
