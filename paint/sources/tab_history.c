
#include "global.h"

void tab_history_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("History"), false, -1, false)) {
		for (i32 i = 0; i < history_steps->length; ++i) {
			i32 active = history_steps->length - 1 - history_redos;
			if (i == active) {
				ui_fill(0, 0, ui->_window_w, ui->ops->theme->ELEMENT_H, ui->ops->theme->HIGHLIGHT_COL);
			}
			ui_text(history_steps->buffer[i]->name, UI_ALIGN_LEFT, 0x00000000);
			if (ui->is_released) { // Jump to undo step
				i32 diff = i - active;
				while (diff > 0) {
					diff--;
					history_redo();
				}
				while (diff < 0) {
					diff++;
					history_undo();
				}
			}
			ui_fill(0, 0, (ui->_window_w / (float)UI_SCALE() - 2), 1 * UI_SCALE(), ui->ops->theme->SEPARATOR_COL);
		}
	}
}
