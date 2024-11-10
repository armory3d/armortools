
// Immediate Mode UI Library
// https://github.com/armory3d/armorcore

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <kinc/graphics2/g2.h>
#include "iron_armpack.h"

#define UI_LAYOUT_VERTICAL 0
#define UI_LAYOUT_HORIZONTAL 1
#define UI_ALIGN_LEFT 0
#define UI_ALIGN_CENTER 1
#define UI_ALIGN_RIGHT 2
#define UI_STATE_IDLE 0
#define UI_STATE_STARTED 1
#define UI_STATE_DOWN 2
#define UI_STATE_RELEASED 3
#define UI_STATE_HOVERED 4
#define UI_LINK_STYLE_LINE 0
#define UI_LINK_STYLE_CUBIC_BEZIER 1

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
} ui_theme_t;

typedef struct ui_options {
	g2_font_t *font;
	ui_theme_t *theme;
	float scale_factor;
	kinc_g4_texture_t *color_wheel;
	kinc_g4_texture_t *black_white_gradient;
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
	kinc_g4_render_target_t texture;
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
	char colored[128];
	char uncolored[128];
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
	char hovered_tab_name[64];
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
	char text_selected[256];
	ui_handle_t *submit_text_handle;
	char text_to_submit[256];
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
	kinc_g4_texture_t *tooltip_img;
	kinc_g4_render_target_t *tooltip_rt;
	int tooltip_img_max_width;
	bool tooltip_invert_y;
	float tooltip_x;
	float tooltip_y;
	bool tooltip_shown;
	bool tooltip_wait;
	double tooltip_time;
	char tab_names[16][64];
	uint32_t tab_colors[16];
	bool tab_enabled[16];
	int tab_count; // Number of tab calls since window begin
	ui_handle_t *tab_handle;
	float tab_scroll;
	bool tab_vertical;
	bool sticky;
	bool scissor;

	bool elements_baked;
	kinc_g4_render_target_t check_select_image;
	kinc_g4_render_target_t radio_image;
	kinc_g4_render_target_t radio_select_image;
	kinc_g4_render_target_t round_corner_image;
	kinc_g4_render_target_t filled_round_corner_image;
} ui_t;

void ui_init(ui_t *ui, ui_options_t *ops);
void ui_begin(ui_t *ui);
void ui_begin_sticky();
void ui_end_sticky();
void ui_begin_region(ui_t *ui, int x, int y, int w);
void ui_end_region(bool last);
bool ui_window(ui_handle_t *handle, int x, int y, int w, int h, bool drag); // Returns true if redraw is needed
bool ui_button(char *text, int align, char *label);
int ui_text(char *text, int align, int bg);
bool ui_tab(ui_handle_t *handle, char *text, bool vertical, uint32_t color);
bool ui_panel(ui_handle_t *handle, char *text, bool is_tree, bool filled);
int ui_sub_image(/*kinc_g4_texture_t kinc_g4_render_target_t*/ void *image, bool is_rt, uint32_t tint, int h, int sx, int sy, int sw, int sh);
int ui_image(/*kinc_g4_texture_t kinc_g4_render_target_t*/ void *image, bool is_rt, uint32_t tint, int h);
char *ui_text_input(ui_handle_t *handle, char *label, int align, bool editable, bool live_update);
bool ui_check(ui_handle_t *handle, char *text, char *label);
bool ui_radio(ui_handle_t *handle, int position, char *text, char *label);
int ui_combo(ui_handle_t *handle, char_ptr_array_t *texts, char *label, bool show_label, int align, bool search_bar);
float ui_slider(ui_handle_t *handle, char *text, float from, float to, bool filled, float precision, bool display_value, int align, bool text_edit);
void ui_row(f32_array_t *ratios);
void ui_separator(int h, bool fill);
void ui_tooltip(char *text);
void ui_tooltip_image(kinc_g4_texture_t *image, int max_width);
void ui_tooltip_render_target(kinc_g4_render_target_t *image, int max_width);
void ui_end(bool last);
void ui_end_window(bool bind_global_g);
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
#if defined(KINC_ANDROID) || defined(KINC_IOS)
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

#define UI_MAX_INSTANCES 8
extern ui_t *ui_instances[UI_MAX_INSTANCES];
extern int ui_instances_count;
extern bool ui_always_redraw_window;
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
