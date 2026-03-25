
#include "ctype.h"

int tolower(int ch) {
	return (ch >= 'A' && ch <= 'Z') ? ch + 32 : ch;
}

int toupper(int ch) {
	return (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
}

int isspace(int ch) {
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\v';
}

int isxdigit(int ch) {
	return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

int isdigit(int ch) {
	return ch >= '0' && ch <= '9';
}

int isalpha(int ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int isalnum(int ch) {
	return isalpha(ch) || isdigit(ch);
}
