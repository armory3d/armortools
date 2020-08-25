package arm.filter;

import arm.ui.UISidebar;
import zui.Zui.Handle;

class FilterFrame extends FilterBase
{
    var filter: FilterBase = null;
    var h = new Handle();  // panel handle
    var ch = new Handle();  // combo handle

    public function new() {
        this.name = "None";
    }

    override public function draw() {
        var ui = UISidebar.inst.ui;
        var title = filter == null ? name : filter.name;
        if (ui.panel(h, tr(title))) {
            ui.indent(false);
            ui.row([4/5, 1/5]);

            // header
            // var ch = new Handle({position: FilterFactory.filterNames.indexOf(title)});
            ch.position = FilterFactory.filterNames.indexOf(title);
            ui.combo(ch, FilterFactory.filterNames, tr("Type"), true, zui.Zui.Align.Right);
            if (ch.changed) {
                filter = FilterFactory.CreateFilterByIndex(ch.position);
            }

            ui.button(tr("Remove"));
            if (filter != null) filter.draw();

            ui.unindent(false);
            ui.separator(4, true);
        }
    }

}
