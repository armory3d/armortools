#pragma once

#include <wchar.h>

void IOSFileDialogOpen();
wchar_t *IOSFileDialogSave();
void IOSDeleteFile(const char *path);
