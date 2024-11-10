#pragma once

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

void AndroidFileDialogOpen();
wchar_t *AndroidFileDialogSave();
void android_check_permissions();

#ifdef __cplusplus
}
#endif
