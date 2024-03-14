
///if (is_paint || is_sculpt)

function tab_particles_draw(htab: zui_handle_t) {
	if (zui_tab(htab, tr("Particles"))) {
		zui_begin_sticky();
		zui_row([1 / 4, 1 / 4, 1 / 4]);
		if (zui_button(tr("New"))) {}
		if (zui_button(tr("Import"))) {}
		if (zui_button(tr("Nodes"))) {}
		zui_end_sticky();
	}
}

///end
