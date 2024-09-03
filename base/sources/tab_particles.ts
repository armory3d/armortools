
///if (is_paint || is_sculpt)

function tab_particles_draw(htab: ui_handle_t) {
	if (ui_tab(htab, tr("Particles"))) {
		ui_begin_sticky();
		let row: f32[] = [1 / 4, 1 / 4, 1 / 4];
		ui_row(row);
		if (ui_button(tr("New"))) {

		}
		if (ui_button(tr("Import"))) {

		}
		if (ui_button(tr("Nodes"))) {

		}
		ui_end_sticky();
	}
}

///end
