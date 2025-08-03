#pragma once

#include <stdbool.h>

struct HWND__;

void iron_microsoft_format(const char *format, va_list args, wchar_t *buffer);
bool iron_windows_set_display_mode(int display_index, int width, int height, int bpp, int frequency);
void iron_windows_restore_display(int display_index);
void iron_windows_restore_displays();
void iron_windows_hide_windows();
void iron_windows_destroy_windows();
struct HWND__ *iron_windows_window_handle();
