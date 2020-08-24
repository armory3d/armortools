package arm.ui;

import arm.filter.FilterFrame;

class TabFilters {

	public static function draw() {
		var ui = UISidebar.inst.ui;

		if (ui.tab(UISidebar.inst.htab1, tr("Filters"))) {
			ui.beginSticky();
			if ( ui.button(tr("Add")) ) { Context.layer.filters.push(new FilterFrame()); }

			for (i in 0...Context.layer.filters.length)
			{
				var j = Context.layer.filters.length - i - 1;
				ui.button(tr("Add"));
			}
			ui.endSticky();
		}
	}
}
