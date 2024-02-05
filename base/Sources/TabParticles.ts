
///if (is_paint || is_sculpt)

class TabParticles {

	static draw = (htab: HandleRaw) => {
		if (Zui.tab(htab, tr("Particles"))) {
			Zui.beginSticky();
			Zui.row([1 / 4, 1 / 4, 1 / 4]);
			if (Zui.button(tr("New"))) {}
			if (Zui.button(tr("Import"))) {}
			if (Zui.button(tr("Nodes"))) {}
			Zui.endSticky();
		}
	}
}

///end
