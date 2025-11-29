#include "iron_string.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *gc_alloc(size_t size);
void *gc_leaf(void *ptr);

char *string_alloc(int size) {
	char *r = gc_alloc(size);
	gc_leaf(r);
	return r;
}

char *string_join(char *a, char *b) {
	char *r = string_alloc(strlen(a) + strlen(b) + 1);
	strcpy(r, a);
	strcat(r, b);
	return r;
}

char *string_copy(char *a) {
	if (a == NULL) {
		return NULL;
	}
	char *r = string_alloc(strlen(a) + 1);
	strcpy(r, a);
	return r;
}

int string_length(char *str) {
	return strlen(str);
}

bool string_equals(char *a, char *b) {
	if (a == NULL || b == NULL) {
		return false;
	}
	return strcmp(a, b) == 0;
}

char *i32_to_string(int32_t i) {
	int   l = snprintf(NULL, 0, "%d", i);
	char *r = string_alloc(l + 1);
	sprintf(r, "%d", i);
	return r;
}

char *i32_to_string_hex(int32_t i) {
	int   l = snprintf(NULL, 0, "%X", i);
	char *r = string_alloc(l + 1);
	sprintf(r, "%X", i);
	return r;
}

char *i64_to_string(int64_t i) {
	int   l = snprintf(NULL, 0, "%ld", i);
	char *r = string_alloc(l + 1);
	sprintf(r, "%ld", i);
	return r;
}

char *u64_to_string(uint64_t i) {
	int   l = snprintf(NULL, 0, "%lu", i);
	char *r = string_alloc(l + 1);
	sprintf(r, "%lu", i);
	return r;
}

char *f32_to_string_with_zeros(float f) {
	int   l = snprintf(NULL, 0, "%f", f);
	char *r = string_alloc(l + 1);
	sprintf(r, "%f", f);
	return r;
}

char *f32_to_string(float f) {
	char *r = f32_to_string_with_zeros(f);
	string_strip_trailing_zeros(r);
	return r;
}

float f32_from_string(const char *const s) {
	return atof(s);
}

void string_strip_trailing_zeros(char *str) {
	int len = strlen(str);
	while (str[--len] == '0') {
		str[len] = '\0';
	}
	if (str[len] == '.') {
		str[len] = '\0';
	}
}

int32_t string_index_of_pos(char *s, char *search, int pos) {
	char *found = strstr(s + pos, search);
	if (found != NULL) {
		return found - s;
	}
	return -1;
}

int32_t string_index_of(char *s, char *search) {
	return string_index_of_pos(s, search, 0);
}

int32_t string_last_index_of(char *str, char *search) {
	char *s     = str;
	char *found = NULL;
	while (1) {
		char *p = strstr(s, search);
		if (p == NULL) {
			break;
		}
		found = p;
		s     = p + 1;
	}
	if (found != NULL) {
		return found - str;
	}
	return -1;
}

any_array_t *string_split(char *s, char *sep) {
	any_array_t *a       = gc_alloc(sizeof(any_array_t));
	int          sep_len = strlen(sep);
	char        *pos     = s;
	while (true) {
		char *next = strstr(pos, sep);
		if (next == NULL) {
			any_array_push(a, string_copy(pos));
			break;
		}
		int   part_len = next - pos;
		char *part     = string_alloc(part_len + 1);
		strncpy(part, pos, part_len);
		part[part_len] = '\0';
		any_array_push(a, part);
		pos = next + sep_len;
	}
	return a;
}

char *string_array_join(any_array_t *a, char *separator) {
	int len     = 0;
	int len_sep = strlen(separator);
	for (int i = 0; i < a->length; ++i) {
		len += strlen(a->buffer[i]);
		if (i < a->length - 1) {
			len += len_sep;
		}
	}

	char *r = string_alloc(len + 1);
	for (int i = 0; i < a->length; ++i) {
		strcat(r, a->buffer[i]);
		if (i < a->length - 1) {
			strcat(r, separator);
		}
	}
	return r;
}

char *string_replace_all(char *s, char *search, char *replace) {
	char  *buffer      = string_alloc(1024);
	char  *buffer_pos  = buffer;
	size_t search_len  = strlen(search);
	size_t replace_len = strlen(replace);
	while (1) {
		char *p = strstr(s, search);
		if (p == NULL) {
			strcpy(buffer_pos, s);
			break;
		}
		memcpy(buffer_pos, s, p - s);
		buffer_pos += p - s;
		memcpy(buffer_pos, replace, replace_len);
		buffer_pos += replace_len;
		s = p + search_len;
	}
	return buffer;
}

char *substring(char *s, int32_t start, int32_t end) {
	char *buffer = string_alloc(end - start + 1);
	for (int i = 0; i < end - start; ++i) {
		buffer[i] = s[start + i];
	}
	return buffer;
}

char *string_from_char_code(int32_t c) {
	char *r = string_alloc(2);
	r[0]    = c;
	r[1]    = '\0';
	return r;
}

int32_t char_code_at(char *s, int32_t i) {
	return s[i];
}

char *char_at(char *s, int32_t i) {
	char *r = string_alloc(2);
	r[0]    = s[i];
	r[1]    = '\0';
	return r;
}

bool starts_with(char *s, char *start) {
	return strncmp(start, s, strlen(start)) == 0;
}

bool ends_with(char *s, char *end) {
	size_t len_s   = strlen(s);
	size_t len_end = strlen(end);
	return strncmp(s + len_s - len_end, end, len_end) == 0;
}

char *to_lower_case(char *s) {
	char *r = string_alloc(strlen(s) + 1);
	strcpy(r, s);
	int len = string_length(r);
	for (int i = 0; i < len; ++i) {
		r[i] = tolower(r[i]);
	}
	return r;
}

char *to_upper_case(char *s) {
	char *r = string_alloc(strlen(s) + 1);
	strcpy(r, s);
	int len = string_length(r);
	for (int i = 0; i < len; ++i) {
		r[i] = toupper(r[i]);
	}
	return r;
}

char *trim_end(char *str) {
	int pos = string_length(str) - 1;
	while (pos >= 0 && (str[pos] == ' ' || str[pos] == '\n' || str[pos] == '\r')) {
		pos--;
	}
	return substring(str, 0, pos + 1);
}

// Per Lowgren, CC BY-SA 3.0
// https://stackoverflow.com/a/35332046
#define is_unicode(c) (((c) & 0xc0) == 0xc0)
int string_utf8_decode(const char *str, int *i) {
	const unsigned char *s = (const unsigned char *)str;
	int                  u = *s, l = 1;
	if (is_unicode(u)) {
		int a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;
		if (a < 6 || !(u & 0x02)) {
			int b, p = 0;
			u = ((u << (a + 1)) & 0xff) >> (a + 1);
			for (b = 1; b < a; ++b)
				u = (u << 6) | (s[l++] & 0x3f);
		}
	}
	if (i)
		*i += l;
	return u;
}
