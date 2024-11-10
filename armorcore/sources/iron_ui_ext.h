#pragma once

#include "iron_ui.h"

float ui_float_input(ui_handle_t *handle, char *label, int align, float precision);
char *ui_file_browser(ui_handle_t *handle, bool folders_only);
int ui_inline_radio(ui_handle_t *handle, char_ptr_array_t *texts, int align);
int ui_color_wheel(ui_handle_t *handle, bool alpha, float w, float h, bool color_preview, void (*picker)(void *), void *data);
char *ui_text_area(ui_handle_t *handle, int align, bool editable, char *label, bool word_wrap);
void ui_begin_menu();
void ui_end_menu();
bool _ui_menu_button(char *text);
void ui_hsv_to_rgb(float cr, float cg, float cb, float *out);
void ui_rgb_to_hsv(float cr, float cg, float cb, float *out);

uint8_t ui_color_r(uint32_t color);
uint8_t ui_color_g(uint32_t color);
uint8_t ui_color_b(uint32_t color);
uint8_t ui_color_a(uint32_t color);
uint32_t ui_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

extern bool ui_text_area_line_numbers;
extern bool ui_text_area_scroll_past_end;
extern ui_text_coloring_t *ui_text_area_coloring;
