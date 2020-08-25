package arm.ui;

import arm.filter.FilterFrame;

class TabFilters {

	public static function draw() {
		var ui = UISidebar.inst.ui;
		var lay = Context.layer;

		if (ui.tab(UISidebar.inst.htab1, tr("Filters"))) {
			ui.beginSticky();

			if ( ui.button(tr("Add")) ) { lay.filters.push(new FilterFrame()); }

			// draw all filters
			for (i in 0...lay.filters.length) {
				var j = lay.filters.length - i - 1;
				lay.filters[j].draw();
			}

			ui.endSticky();
		}
	}
}
