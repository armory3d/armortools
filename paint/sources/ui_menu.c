
#include "global.h"

void ui_menu_render() {
	i32 menu_w = ui_menu_commands != NULL ? math_floor(base_default_element_w * UI_SCALE() * 2.3) : math_floor(UI_ELEMENT_W() * 2.3);

	i32 _FILL_BUTTON_BG            = ui->ops->theme->FILL_BUTTON_BG;
	ui->ops->theme->FILL_BUTTON_BG = false;
	i32 _ELEMENT_OFFSET            = ui->ops->theme->ELEMENT_OFFSET;
	ui->ops->theme->ELEMENT_OFFSET = 0;
	i32 _ELEMENT_H                 = ui->ops->theme->ELEMENT_H;
	ui->ops->theme->ELEMENT_H      = config_raw->touch_ui ? (28 + 2) : 28;

	if (ui_menu_nested) {
		ui_menu_show_first = true;
		ui_menu_nested     = false;
	}

	// First draw out of screen, then align the menu based on menu height
	if (ui_menu_show_first) {
		ui_menu_x -= iron_window_width() * 2;
		ui_menu_y -= iron_window_height() * 2;
	}

	draw_begin(NULL, false, 0);
	ui_begin_region(ui, ui_menu_x, ui_menu_y, menu_w);
	ui->input_enabled = ui->combo_selected_handle == NULL;
	ui_menu_begin();

	if (ui_menu_commands != NULL) {
		ui_menu_commands();
	}

	ui_menu_hide_flag = ui->combo_selected_handle == NULL && !ui_menu_keep_open && !ui_menu_show_first &&
	                    (ui->changed || ui->input_released || ui->input_released_r || ui->is_escape_down);
	ui_menu_keep_open = false;

	ui->ops->theme->FILL_BUTTON_BG = _FILL_BUTTON_BG;
	ui->ops->theme->ELEMENT_OFFSET = _ELEMENT_OFFSET;
	ui->ops->theme->ELEMENT_H      = _ELEMENT_H;
	ui_menu_end();
	ui_end_region();
	ui->input_enabled = true;
	draw_end();

	if (ui_menu_show_first) {
		ui_menu_show_first = false;
		ui_menu_keep_open  = true;
		ui_menu_h          = ui->_y - ui_menu_y;
		ui_menu_x += iron_window_width() * 2;
		ui_menu_y += iron_window_height() * 2;
		ui_menu_fit_to_screen();
		ui_menu_render(); // Render at correct position now
	}

	if (ui_menu_hide_flag) {
		ui_menu_hide();
		ui_menu_show_first = true;
		gc_unroot(ui_menu_commands);
		ui_menu_commands = NULL;
	}
}

void ui_menu_hide() {
	ui_menu_show = false;
	base_redraw_ui();
}

void ui_menu_draw(void (*commands)(void), i32 x, i32 y) {
	ui_end_input();
	if (ui_menu_show) {
		ui_menu_nested    = true;
		ui_menu_keep_open = true;
	}
	ui_menu_show = true;
	gc_unroot(ui_menu_commands);
	ui_menu_commands = commands;
	gc_root(ui_menu_commands);
	ui_menu_x = x > -1 ? x : math_floor(mouse_x + 1);
	ui_menu_y = y > -1 ? y : math_floor(mouse_y + 1);
	ui_menu_h = 0;
}

void ui_menu_fit_to_screen() {
	// Prevent the menu going out of screen
	f32 menu_w = base_default_element_w * UI_SCALE() * 2.3;
	if (ui_menu_x + menu_w > iron_window_width()) {
		if (ui_menu_x - menu_w > 0) {
			ui_menu_x = math_floor(ui_menu_x - menu_w);
		}
		else {
			ui_menu_x = math_floor(iron_window_width() - menu_w);
		}
	}
	if (ui_menu_y + ui_menu_h > iron_window_height()) {
		if (ui_menu_y - ui_menu_h > 0) {
			ui_menu_y = math_floor(ui_menu_y - ui_menu_h);
		}
		else {
			ui_menu_y = iron_window_height() - ui_menu_h;
		}
		ui_menu_x += 1; // Move out of mouse focus
	}
}

void ui_menu_separator() {
	ui->_y++;
	ui_fill(26, 0, ui->_w / (float)UI_SCALE() - 26, 1, ui->ops->theme->BUTTON_COL);
}

bool ui_menu_button(char *text, char *label, icon_t icon) {
	if (config_raw->touch_ui && !string_equals(label, ">")) {
		label = "";
	}
	i32  _x_left = ui->_x;
	i32  _y_top  = ui->_y;
	bool result  = ui_button(string("%s%s", config_button_spacing, text), config_button_align, label);
	if (string_equals(label, ">") && result) {
		ui_menu_keep_open = true;
	}

	if (icon != ICON_NONE) {
		i32            _y_bottom = ui->_y;
		gpu_texture_t *icons     = resource_get("icons05x.k");
		rect_t        *rect      = resource_tile50(icons, icon);
		i32            icon_h    = 25 * UI_SCALE();
		ui->_x                   = _x_left - 5 * UI_SCALE();
		ui->_y                   = _y_top - 1;
		if (config_raw->touch_ui) {
			ui->_x = _x_left - 2 * UI_SCALE();
			ui->_y = _y_top + 2 * UI_SCALE();
		}
		ui_sub_image(icons, ui->ops->theme->LABEL_COL - 0x00222222, icon_h, rect->x / 2.0, rect->y / 2.0, rect->w / 2.0, rect->h / 2.0);
		ui->_x = _x_left;
		ui->_y = _y_bottom;
	}

	return result;
}

u32 ui_menu_color_sub(u32 c, u32 s) {
	return c - (s + 0xff000000) > c ? 0xff000000 : c - s;
}

bool ui_icon_button(char *text, icon_t icon, ui_align_t align) {
	i32 _x_left = ui->_x;
	i32 _y_top  = ui->_y;
	i32 _w      = ui->_w;
	if (!string_equals(text, "")) {
		text = align == UI_ALIGN_LEFT ? string("        %s", text) : string("      %s", text);
	}

	char *tooltip = "";
	i32   textw   = draw_string_width(ui->ops->font, ui->font_size, text);
	f32   wmax    = config_raw->touch_ui ? 0.9 : 0.8;
	if (textw > _w * wmax) {
		tooltip = string_copy(text);
		text    = "";
		textw   = 0;
	}

	bool result = ui_button(text, align, "");

	if (ui->is_hovered && !string_equals(tooltip, "")) {
		ui_tooltip(tooltip);
	}

	if (icon != ICON_NONE) {
		i32            _x_right  = ui->_x;
		i32            _y_bottom = ui->_y;
		gpu_texture_t *icons     = resource_get("icons05x.k");
		rect_t        *rect      = resource_tile50(icons, icon);
		i32            icon_h    = 25 * UI_SCALE();
		ui->_x                   = align == UI_ALIGN_LEFT ? _x_left : _x_left + _w / 2.0 - textw / 2.0 - icon_h / 2.0;
		ui->_y                   = _y_top;

		if (config_raw->touch_ui) {
			ui->_x += 1 * UI_SCALE();
			if (!string_equals(text, "")) {
				ui->_x += 5 * UI_SCALE();
			}
			ui->_y = _y_top + 2 * UI_SCALE();
		}
		if (ui->current_ratio > -1) {
			ui->current_ratio--;
		}

		ui->image_scroll_align = false;
		ui_sub_image(icons, ui->enabled ? ui_menu_color_sub(ui->ops->theme->LABEL_COL, 0x00333333) : 0xffffffff, icon_h, rect->x / 2.0, rect->y / 2.0,
		             rect->w / 2.0, rect->h / 2.0);
		ui->image_scroll_align = true;

		ui->_x = _x_right;
		ui->_y = _y_bottom;
	}
	return result;
}

bool ui_menu_sub_button(ui_handle_t *handle, char *text) {
	ui->is_hovered = false;
	ui_menu_button(text, ">", ICON_NONE);
	if (ui->is_hovered) {
		gc_unroot(ui_menu_sub_handle);
		ui_menu_sub_handle = handle;
		gc_root(ui_menu_sub_handle);
	}
	else if (math_abs(ui->input_dy) > ui->input_dx && ui->input_x < ui->_x + ui->_w) {
		gc_unroot(ui_menu_sub_handle);
		ui_menu_sub_handle = NULL;
	}
	return ui_menu_sub_handle == handle;
}

void ui_menu_label(char *text, char *shortcut) {
	i32 _y                   = ui->_y;
	i32 _TEXT_COL            = ui->ops->theme->TEXT_COL;
	ui->ops->theme->TEXT_COL = ui->ops->theme->LABEL_COL;
	ui_text(text, UI_ALIGN_LEFT, 0x00000000);
	if (shortcut != NULL) {
		ui->_y = _y;
		ui_text(shortcut, UI_ALIGN_RIGHT, 0x00000000);
	}
	ui->ops->theme->TEXT_COL = _TEXT_COL;
}

void ui_menu_align() {
	if (!config_raw->touch_ui) {
		f32_array_t *row = f32_array_create_from_raw(
		    (f32[]){
		        12 / 100.0,
		        88 / 100.0,
		    },
		    2);
		ui_row(row);
		ui_end_element();
	}
}

void ui_menu_begin() {
	ui_draw_shadow(ui->_x, ui->_y, ui->_w, ui_menu_h);
	draw_set_color(ui->ops->theme->SEPARATOR_COL);
	ui_draw_rect(true, ui->_x, ui->_y, ui->_w, ui_menu_h);
	draw_set_color(0xffffffff);
}

void ui_menu_end() {}

void ui_menu_sub_begin(i32 items) {
	ui_menu_sub_x = ui->_x;
	ui_menu_sub_y = ui->_y;
	ui->_x += ui->_w + 2;
	ui->_y -= UI_ELEMENT_H();
	ui_draw_shadow(ui->_x, ui->_y, ui->_w, UI_ELEMENT_H() * items);
	draw_set_color(ui->ops->theme->SEPARATOR_COL);
	ui_draw_rect(true, ui->_x, ui->_y, ui->_w, UI_ELEMENT_H() * items);
	draw_set_color(0xffffffff);
}

void ui_menu_sub_end() {
	ui->_x = ui_menu_sub_x;
	ui->_y = ui_menu_sub_y;
}
