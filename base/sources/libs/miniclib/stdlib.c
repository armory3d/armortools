#include "stdlib.h"
#include "string.h"

int system(const char *string) {
	return 0;
}

void exit(int code) {
	exit(code);
}

int isdigit(int c) {
	return c >= '0' && c <= '9';
}

int isspace(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

long int strtol(const char *str, char **endptr, int base) {
	while (isspace(*str)) {
		str++;
	}
	int sign = (*str == '-') ? (str++, -1) : (*str == '+') ? (str++, 1) : 1;
	if (base == 0) {
		base = (*str == '0') ? (str[1] == 'x' || str[1] == 'X') ? (str += 2, 16) : (str++, 8) : 10;
	}
	if (base == 16 && *str == '0' && (str[1] == 'x' || str[1] == 'X')) {
		str += 2;
	}
	long val = 0;
	for (; *str; str++) {
		int digit = isdigit(*str) ? *str - '0' : (*str >= 'a' && *str <= 'z') ? *str - 'a' + 10 : (*str >= 'A' && *str <= 'Z') ? *str - 'A' + 10 : 999;
		if (digit >= base) {
			break;
		}
		val = val * base + digit;
	}
	if (endptr) {
		*endptr = (char *)str;
	}
	return sign * val;
}

float strtof(const char *str, char **endptr) {
	return (float)strtod(str, endptr);
}

double strtod(const char *str, char **endptr) {
	while (isspace(*str)) {
		str++;
	}
	int    sign = (*str == '-') ? (str++, -1) : (*str == '+') ? (str++, 1) : 1;
	double val  = 0;
	while (isdigit(*str)) {
		val = val * 10.0 + (*str++ - '0');
	}
	if (*str == '.') {
		double divisor = 10.0;
		for (str++; isdigit(*str); str++) {
			val += (*str - '0') / divisor;
			divisor *= 10.0;
		}
	}
	if (endptr) {
		*endptr = (char *)str;
	}
	return sign * val;
}

double atof(const char *str) {
	return strtod(str, NULL);
}

int abs(int n) {
	return n < 0 ? -n : n;
}

long long int llabs(long long int n) {
	return n < 0 ? -n : n;
}

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
	if (num < 2) {
		return;
	}
	char  *b = base;
	char   tmp[size];
	char  *pivot = b + (num / 2) * size;
	size_t i     = 0;
	size_t j     = num - 1;
	memcpy(tmp, pivot, size);
	while (i <= j) {
		while (compar(b + i * size, tmp) < 0) {
			i++;
		}
		while (j > 0 && compar(b + j * size, tmp) > 0) {
			j--;
		}
		if (i <= j) {
			if (i != j) {
				char t[size];
				memcpy(t, b + i * size, size);
				memcpy(b + i * size, b + j * size, size);
				memcpy(b + j * size, t, size);
			}
			i++;
			if (j == 0) {
				break;
			}
			j--;
		}
	}
	if (j > 0) {
		qsort(b, j + 1, size, compar);
	}
	if (i < num) {
		qsort(b + i * size, num - i, size, compar);
	}
}

static unsigned int seed = 1;

void srand(unsigned int s) {
	seed = s;
}

int rand() {
	seed = seed * 1103515245 + 12345;
	return (seed >> 16) & 0x7FFF;
}
