#pragma once

#include <stdio.h>

typedef long HRESULT;

void kinc_microsoft_affirm(HRESULT result);
void kinc_microsoft_affirm_message(HRESULT result, const char *format, ...);
void kinc_microsoft_format(const char *format, va_list args, wchar_t *buffer);
