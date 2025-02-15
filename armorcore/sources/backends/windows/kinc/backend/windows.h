#pragma once

#include <stdbool.h>

struct HMONITOR__;
struct HWND__;

int kinc_windows_get_display_for_monitor(struct HMONITOR__ *monitor);
bool kinc_windows_set_display_mode(int display_index, int width, int height, int bpp, int frequency);
void kinc_windows_restore_display(int display_index);
void kinc_windows_restore_displays();
void kinc_windows_hide_windows();
void kinc_windows_destroy_windows();
struct HWND__ *kinc_windows_window_handle();
