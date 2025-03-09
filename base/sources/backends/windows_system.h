#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef long HRESULT;
struct HMONITOR__;
struct HWND__;

void iron_microsoft_affirm(HRESULT result);
void iron_microsoft_affirm_message(HRESULT result, const char *format, ...);
void iron_microsoft_format(const char *format, va_list args, wchar_t *buffer);

int iron_windows_get_display_for_monitor(struct HMONITOR__ *monitor);
bool iron_windows_set_display_mode(int display_index, int width, int height, int bpp, int frequency);
void iron_windows_restore_display(int display_index);
void iron_windows_restore_displays();
void iron_windows_hide_windows();
void iron_windows_destroy_windows();
struct HWND__ *iron_windows_window_handle();
