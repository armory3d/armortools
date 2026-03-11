
#include "global.h"

void ui_sidebar_render_ui() {

	// Expand button
	if (config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] == 0) {
		i32 width = math_floor(draw_string_width(ui->ops->font, ui->font_size, "<") + 25 * UI_SCALE());
		if (ui_window(ui_sidebar_hminimized, iron_window_width() - width, -1, width, math_floor(UI_ELEMENT_H() + 4 * UI_SCALE()), false)) {
			ui_fill(0, 0, ui->_window_w, ui->_window_h + 1, ui->ops->theme->SEPARATOR_COL);
			ui->_w                     = width;
			i32 _BUTTON_H              = ui->ops->theme->BUTTON_H;
			i32 _BUTTON_COL            = ui->ops->theme->BUTTON_COL;
			i32 _TEXT_COL              = ui->ops->theme->TEXT_COL;
			ui->ops->theme->BUTTON_H   = ui->ops->theme->ELEMENT_H;
			ui->ops->theme->BUTTON_COL = ui->ops->theme->SEPARATOR_COL;
			ui->ops->theme->TEXT_COL   = ui->ops->theme->HOVER_COL;

			if (ui_button("<", UI_ALIGN_CENTER, "")) {
				ui_sidebar_show(true);
			}
			ui->ops->theme->BUTTON_H   = _BUTTON_H;
			ui->ops->theme->BUTTON_COL = _BUTTON_COL;
			ui->ops->theme->TEXT_COL   = _TEXT_COL;
		}
		return;
	}

	// Tabs
	bool mini                 = config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] <= ui_sidebar_w_mini;
	i32  expand_button_offset = config_raw->touch_ui ? math_floor(UI_ELEMENT_H() + UI_ELEMENT_OFFSET()) : 0;
	ui_sidebar_tabx           = iron_window_width() - config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W];

	i32 _SCROLL_W = ui->ops->theme->SCROLL_W;
	if (mini) {
		ui->ops->theme->SCROLL_W = ui->ops->theme->SCROLL_MINI_W;
	}

	i32 sidebar_y = 0;

#ifdef IRON_IOS
	if (config_is_iphone()) {
		sidebar_y += UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
		ui_end();
		draw_begin(NULL, false, 0);
		draw_set_color(ui->ops->theme->PRESSED_COL);
		draw_filled_rect(ui_sidebar_tabx, 0, config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W], sidebar_y);
		draw_end();
		ui_begin(ui);
	}
#endif

	if (ui_window(ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0], ui_sidebar_tabx, sidebar_y, config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W],
	              config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0] - sidebar_y, false)) {
		tab_draw_t_array_t *tabs = ui_base_hwnd_tabs->buffer[TAB_AREA_SIDEBAR0];
		for (i32 i = 0; i < (mini ? 1 : tabs->length); ++i) {
			tabs->buffer[i]->f(ui_base_htabs->buffer[TAB_AREA_SIDEBAR0]);
		}

		if (ui_base_htabs->buffer[TAB_AREA_SIDEBAR0]->i < tabs->length) {
			ui_sidebar_last_tab = ui_base_htabs->buffer[TAB_AREA_SIDEBAR0]->i;
		}

		if (!config_raw->touch_ui && !mini) {
			if (ui_tab(ui_base_htabs->buffer[TAB_AREA_SIDEBAR0], ">", false, -2, false)) {
				ui_sidebar_show(false);
			}
		}
	}
	if (ui_window(ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1], ui_sidebar_tabx, config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H0],
	              config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W], config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_H1] - expand_button_offset, false)) {
		tab_draw_t_array_t *tabs = ui_base_hwnd_tabs->buffer[TAB_AREA_SIDEBAR1];
		for (i32 i = 0; i < (mini ? 1 : tabs->length); ++i) {
			tabs->buffer[i]->f(ui_base_htabs->buffer[TAB_AREA_SIDEBAR1]);
		}
	}

	ui_end_window();
	ui->ops->theme->SCROLL_W = _SCROLL_W;

	// Collapse / expand button for mini sidebar
	if (config_raw->touch_ui) {
		i32 width  = config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W];
		i32 height = math_floor(UI_ELEMENT_H() + UI_ELEMENT_OFFSET());
		if (ui_window(ui_handle(__ID__), iron_window_width() - width, iron_window_height() - height, width, height + 1, false)) {
			ui->_w                     = width;
			i32 _BUTTON_H              = ui->ops->theme->BUTTON_H;
			i32 _BUTTON_COL            = ui->ops->theme->BUTTON_COL;
			ui->ops->theme->BUTTON_H   = ui->ops->theme->ELEMENT_H;
			ui->ops->theme->BUTTON_COL = ui->ops->theme->WINDOW_BG_COL;
			if (ui_button(mini ? "<" : ">", UI_ALIGN_CENTER, "")) {
				config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] = mini ? ui_sidebar_default_w_full : ui_sidebar_default_w_mini;
				config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] = math_floor(config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] * UI_SCALE());
			}
			ui->ops->theme->BUTTON_H   = _BUTTON_H;
			ui->ops->theme->BUTTON_COL = _BUTTON_COL;
		}
	}
}

void ui_sidebar_show(bool b) {
	if (b) {
		config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W] =
		    context_raw->maximized_sidebar_width != 0 ? context_raw->maximized_sidebar_width : math_floor(ui_sidebar_default_w * config_raw->window_scale);
	}
	else {
		ui_base_htabs->buffer[TAB_AREA_SIDEBAR0]->i        = ui_sidebar_last_tab;
		config_raw->layout_tabs->buffer[TAB_AREA_SIDEBAR0] = ui_sidebar_last_tab;
		context_raw->maximized_sidebar_width               = config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W];
		config_raw->layout->buffer[LAYOUT_SIZE_SIDEBAR_W]  = 0;
	}
}
