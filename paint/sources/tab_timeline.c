
#include "global.h"

void tab_timeline_draw(ui_handle_t *htab) {
	if (ui_tab(htab, tr("Timeline"), false, -1, false) && ui->_window_h > ui_statusbar_default_h * UI_SCALE()) {
	}
}
