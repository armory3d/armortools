
#include "ctype.h"

int tolower(int ch) {
	return (ch >= 'A' && ch <= 'Z') ? ch + 32 : ch;
}

int toupper(int ch) {
	return (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
}
