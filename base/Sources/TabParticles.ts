
///if (is_paint || is_sculpt)

class TabParticles {

	static draw = (htab: Handle) => {
		let ui = UIBase.ui;
		if (ui.tab(htab, tr("Particles"))) {
			ui.beginSticky();
			ui.row([1 / 4, 1 / 4, 1 / 4]);
			if (ui.button(tr("New"))) {}
			if (ui.button(tr("Import"))) {}
			if (ui.button(tr("Nodes"))) {}
			ui.endSticky();
		}
	}
}

///end
