
#include "global.h"

i32 ui_statusbar_last_tab = 0;

void ui_statusbar_init() {}

i32 ui_statusbar_width() {
	return iron_window_width() - g_config->layout->buffer[LAYOUT_SIZE_SIDEBAR_W];
}

void ui_statusbar_render_ui() {
	i32 statush = g_config->layout->buffer[LAYOUT_SIZE_STATUS_H];
	if (ui_window(ui_base_hwnds->buffer[TAB_AREA_STATUS], 0, iron_window_height() - statush, ui_statusbar_width(), statush, false)) {
		ui->_y += 2;
		draw_set_color(ui->ops->theme->SEPARATOR_COL);
		draw_filled_rect(0, 0, 1, ui->_window_h);
		draw_filled_rect(ui->_window_w - 1, 0, 1, ui->_window_h);
		tab_draw_t_array_t *hwnd_draws = ui_base_hwnd_tabs->buffer[TAB_AREA_STATUS];
		ui_handle_t        *htab       = ui_base_htabs->buffer[TAB_AREA_STATUS];
		for (i32 i = 0; i < hwnd_draws->length; ++i) {
			tab_draw_t *draw = hwnd_draws->buffer[i];
			draw->f(htab);
		}
		if (ui_base_htabs->buffer[TAB_AREA_STATUS]->i < hwnd_draws->length) {
			ui_statusbar_last_tab = ui_base_htabs->buffer[TAB_AREA_STATUS]->i;
		}
		if (!g_config->touch_ui) {
			bool minimized = g_config->layout->buffer[LAYOUT_SIZE_STATUS_H] <= (ui_statusbar_default_h * g_config->window_scale);
			if (ui_tab(ui_base_htabs->buffer[TAB_AREA_STATUS], minimized ? "<" : ">", false, -2, false)) {
				ui_base_htabs->buffer[TAB_AREA_STATUS]->i        = ui_statusbar_last_tab;
				g_config->layout_tabs->buffer[TAB_AREA_STATUS] = ui_statusbar_last_tab;
			}
		}
		bool minimized = statush <= ui_statusbar_default_h * g_config->window_scale;
		if (htab->changed && (htab->i == g_context->last_status_position || minimized)) {
			ui_base_toggle_browser();
		}
		g_context->last_status_position = htab->i;
	}
}

void ui_statusbar_draw_version_tab(ui_handle_t *htab) {
	if (!g_config->touch_ui) {
		ui->enabled = false;
		ui_tab(ui_base_htabs->buffer[TAB_AREA_STATUS], manifest_version, false, -1, false);
		ui->enabled = true;
	}
}
