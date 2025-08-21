
// Immediate Mode UI Library

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "iron_draw.h"
#include "iron_armpack.h"

typedef enum {
	UI_LAYOUT_VERTICAL,
	UI_LAYOUT_HORIZONTAL
} _ui_layout_t;

typedef enum {
	UI_ALIGN_LEFT,
	UI_ALIGN_CENTER,
	UI_ALIGN_RIGHT
} _ui_align_t;

typedef enum {
	UI_STATE_IDLE,
	UI_STATE_STARTED,
	UI_STATE_DOWN,
	UI_STATE_RELEASED,
	UI_STATE_HOVERED
} _ui_state_t;

typedef enum {
	UI_LINK_STYLE_LINE,
	UI_LINK_STYLE_CUBIC_BEZIER
} _ui_link_style_t;

typedef struct ui_theme {
	int WINDOW_BG_COL;
	int HOVER_COL;
	int ACCENT_COL;
	int BUTTON_COL;
	int PRESSED_COL;
	int TEXT_COL;
	int LABEL_COL;
	int SEPARATOR_COL;
	int HIGHLIGHT_COL;
	int FONT_SIZE;
	int ELEMENT_W;
	int ELEMENT_H;
	int ELEMENT_OFFSET;
	int ARROW_SIZE;
	int BUTTON_H;
	int CHECK_SIZE;
	int CHECK_SELECT_SIZE;
	int SCROLL_W;
	int SCROLL_MINI_W;
	int TEXT_OFFSET;
	int TAB_W; // Indentation
	/*bool*/int FILL_WINDOW_BG;
	/*bool*/int FILL_BUTTON_BG;
	int LINK_STYLE;
	/*bool*/int FULL_TABS; // Make tabs take full window width
	/*bool*/int ROUND_CORNERS;
	/*bool*/int SHADOWS;
	int VIEWPORT_COL;
} ui_theme_t;

typedef struct ui_options {
	draw_font_t *font;
	ui_theme_t *theme;
	float scale_factor;
	gpu_texture_t *color_wheel;
	gpu_texture_t *black_white_gradient;
} ui_options_t;

typedef struct ui_handle_array {
	struct ui_handle **buffer;
	int length;
	int capacity;
} ui_handle_array_t;

typedef struct ui_handle {
	bool selected;
	int position;
	uint32_t color;
	float value;
	char *text;
	gpu_texture_t texture;
	int redraws;
	float scroll_offset;
	bool scroll_enabled;
	int layout;
	float last_max_x;
	float last_max_y;
	bool drag_enabled;
	int drag_x;
	int drag_y;
	bool changed;
	bool init;
	ui_handle_array_t *children;
} ui_handle_t;

typedef struct ui_text_extract {
	char colored[1024];
	char uncolored[1024];
} ui_text_extract_t;

typedef struct ui_coloring {
	uint32_t color;
	char_ptr_array_t *start;
	char *end;
	bool separated;
} ui_coloring_t;

typedef struct ui_coloring_array {
	ui_coloring_t **buffer;
	int length;
	int capacity;
} ui_coloring_array_t;

typedef struct ui_text_coloring {
	ui_coloring_array_t *colorings;
	uint32_t default_color;
} ui_text_coloring_t;

typedef struct ui {
	bool is_scrolling; // Use to limit other activities
	bool is_typing;
	bool enabled; // Current element state
	bool is_started;
	bool is_pushed;
	bool is_hovered;
	bool is_released;
	bool changed; // Global elements change check
	bool image_invert_y;
	bool scroll_enabled;
	bool always_redraw; // Hurts performance
	bool highlight_on_select; // Highlight text edit contents on selection
	bool tab_switch_enabled; // Allow switching focus to the next element by pressing tab
	ui_text_coloring_t *text_coloring; // Set coloring scheme for ui_draw_string() calls
	int window_border_top;
	int window_border_bottom;
	int window_border_left;
	int window_border_right;
	char hovered_tab_name[256];
	float hovered_tab_x;
	float hovered_tab_y;
	float hovered_tab_w;
	float hovered_tab_h;
	bool touch_hold_activated;
	bool slider_tooltip;
	float slider_tooltip_x;
	float slider_tooltip_y;
	float slider_tooltip_w;
	bool input_enabled;
	float input_x; // Input position
	float input_y;
	float input_started_x;
	float input_started_y;
	float input_dx; // Delta
	float input_dy;
	float input_wheel_delta;
	bool input_started; // Buttons
	bool input_started_r;
	bool input_released;
	bool input_released_r;
	bool input_down;
	bool input_down_r;
	bool pen_in_use;
	bool is_key_pressed; // Keys
	bool is_key_down;
	bool is_shift_down;
	bool is_ctrl_down;
	bool is_alt_down;
	bool is_a_down;
	bool is_backspace_down;
	bool is_delete_down;
	bool is_escape_down;
	bool is_return_down;
	bool is_tab_down;
	int key_code;
	int key_char;
	float input_started_time;
	int cursor_x; // Text input
	int highlight_anchor;
	f32_array_t *ratios; // Splitting rows
	int current_ratio;
	float x_before_split;
	int w_before_split;

	ui_options_t *ops;
	int font_size;
	float font_offset_y; // Precalculated offsets
	float arrow_offset_x;
	float arrow_offset_y;
	float title_offset_x;
	float button_offset_y;
	float check_offset_x;
	float check_offset_y;
	float check_select_offset_x;
	float check_select_offset_y;
	float radio_offset_x;
	float radio_offset_y;
	float radio_select_offset_x;
	float radio_select_offset_y;
	float scroll_align;
	bool image_scroll_align;

	float _x; // Cursor(stack) position
	float _y;
	int _w;
	int _h;

	float _window_x; // Window state
	float _window_y;
	float _window_w;
	float _window_h;
	ui_handle_t *current_window;
	bool window_ended;
	ui_handle_t *scroll_handle; // Window or slider being scrolled
	ui_handle_t *drag_handle; // Window being dragged
	ui_handle_t *drag_tab_handle; // Tab being dragged
	int drag_tab_position;
	float window_header_w;
	float window_header_h;
	float restore_x;
	float restore_y;

	ui_handle_t *text_selected_handle;
	char text_selected[1024];
	ui_handle_t *submit_text_handle;
	char text_to_submit[1024];
	bool tab_pressed;
	ui_handle_t *tab_pressed_handle;
	ui_handle_t *combo_selected_handle;
	ui_handle_t *combo_selected_window;
	int combo_selected_align;
	char_ptr_array_t *combo_selected_texts;
	char *combo_selected_label;
	int combo_selected_x;
	int combo_selected_y;
	int combo_selected_w;
	bool combo_search_bar;
	ui_handle_t *submit_combo_handle;
	int combo_to_submit;
	int combo_initial_value;
	char tooltip_text[512];
	gpu_texture_t *tooltip_img;
	int tooltip_img_max_width;
	bool tooltip_invert_y;
	float tooltip_x;
	float tooltip_y;
	bool tooltip_shown;
	bool tooltip_wait;
	double tooltip_time;
	char tab_names[16][256];
	uint32_t tab_colors[16];
	bool tab_enabled[16];
	int tab_count; // Number of tab calls since window begin
	ui_handle_t *tab_handle;
	float tab_scroll;
	bool tab_vertical;
	bool sticky;
	bool scissor;

	bool elements_baked;
	gpu_texture_t check_select_image;
	gpu_texture_t radio_image;
	gpu_texture_t radio_select_image;
	gpu_texture_t round_corner_image;
	gpu_texture_t filled_round_corner_image;
} ui_t;

void ui_init(ui_t *ui, ui_options_t *ops);
void ui_begin(ui_t *ui);
void ui_begin_sticky();
void ui_end_sticky();
void ui_begin_region(ui_t *ui, int x, int y, int w);
void ui_end_region();
bool ui_window(ui_handle_t *handle, int x, int y, int w, int h, bool drag); // Returns true if redraw is needed
bool ui_button(char *text, int align, char *label);
int ui_text(char *text, int align, int bg);
bool ui_tab(ui_handle_t *handle, char *text, bool vertical, uint32_t color);
bool ui_panel(ui_handle_t *handle, char *text, bool is_tree, bool filled);
int ui_sub_image(gpu_texture_t *image, uint32_t tint, int h, int sx, int sy, int sw, int sh);
int ui_image(gpu_texture_t *image, uint32_t tint, int h);
char *ui_text_input(ui_handle_t *handle, char *label, int align, bool editable, bool live_update);
bool ui_check(ui_handle_t *handle, char *text, char *label);
bool ui_radio(ui_handle_t *handle, int position, char *text, char *label);
int ui_combo(ui_handle_t *handle, char_ptr_array_t *texts, char *label, bool show_label, int align, bool search_bar);
float ui_slider(ui_handle_t *handle, char *text, float from, float to, bool filled, float precision, bool display_value, int align, bool text_edit);
void ui_row(f32_array_t *ratios);
void ui_row2();
void ui_row3();
void ui_row4();
void ui_row5();
void ui_row6();
void ui_row7();
void ui_separator(int h, bool fill);
void ui_tooltip(char *text);
void ui_tooltip_image(gpu_texture_t *image, int max_width);
void ui_end();
void ui_end_window();
char *ui_hovered_tab_name();
void ui_set_hovered_tab_name(char *name);
void ui_mouse_down(ui_t *ui, int button, int x, int y); // Input events
void ui_mouse_move(ui_t *ui, int x, int y, int movement_x, int movement_y);
void ui_mouse_up(ui_t *ui, int button, int x, int y);
void ui_mouse_wheel(ui_t *ui, int delta);
void ui_pen_down(ui_t *ui, int x, int y, float pressure);
void ui_pen_up(ui_t *ui, int x, int y, float pressure) ;
void ui_pen_move(ui_t *ui, int x, int y, float pressure);
void ui_key_down(ui_t *ui, int key_code);
void ui_key_up(ui_t *ui, int key_code);
void ui_key_press(ui_t *ui, unsigned character);
#if defined(IRON_ANDROID) || defined(IRON_IOS)
void ui_touch_down(ui_t *ui, int index, int x, int y);
void ui_touch_up(ui_t *ui, int index, int x, int y);
void ui_touch_move(ui_t *ui, int index, int x, int y);
#endif
char *ui_copy();
char *ui_cut();
void ui_paste(char *s);
void ui_theme_default(ui_theme_t *t);
ui_t *ui_get_current();
void ui_set_current(ui_t *current);
ui_handle_t *ui_handle_create();
ui_handle_t *ui_nest(ui_handle_t *handle, int pos);
void ui_set_scale(float factor);

bool ui_get_hover(float elem_h);
bool ui_get_released(float elem_h);
bool ui_input_in_rect(float x, float y, float w, float h);
void ui_fill(float x, float y, float w, float h, uint32_t color);
void ui_rect(float x, float y, float w, float h, uint32_t color, float strength);
int ui_line_count(char *str);
char *ui_extract_line(char *str, int line);
bool ui_is_visible(float elem_h);
void ui_end_element();
void ui_end_element_of_size(float element_size);
void ui_end_input();
void ui_end_frame();
void ui_fade_color(float alpha);
void ui_draw_string(char *text, float x_offset, float y_offset, int align, bool truncation);
void ui_draw_shadow(float x, float y, float w, float h);
void ui_draw_rect(bool fill, float x, float y, float w, float h);
void ui_draw_round_bottom(float x, float y, float w);
void ui_start_text_edit(ui_handle_t *handle, int align);
void ui_remove_char_at(char *str, int at);
void ui_remove_chars_at(char *str, int at, int count);
void ui_insert_char_at(char *str, int at, char c);
void ui_insert_chars_at(char *str, int at, char *cs);

float UI_SCALE();
float UI_ELEMENT_W();
float UI_ELEMENT_H();
float UI_ELEMENT_OFFSET();
float UI_ARROW_SIZE();
float UI_BUTTON_H();
float UI_CHECK_SIZE();
float UI_CHECK_SELECT_SIZE();
float UI_FONT_SIZE();
float UI_SCROLL_W();
float UI_TEXT_OFFSET();
float UI_TAB_W();
float UI_HEADER_DRAG_H();
float UI_FLASH_SPEED();
float UI_TOOLTIP_DELAY();

extern bool ui_touch_scroll;
extern bool ui_touch_hold;
extern bool ui_touch_tooltip;
extern bool ui_is_cut;
extern bool ui_is_copy;
extern bool ui_is_paste;
extern void (*ui_on_border_hover)(ui_handle_t *, int);
extern void (*ui_on_text_hover)(void);
extern void (*ui_on_deselect_text)(void);
extern void (*ui_on_tab_drop)(ui_handle_t *, int, ui_handle_t *, int);
extern const char *ui_theme_keys[];
extern int ui_theme_keys_count;

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
