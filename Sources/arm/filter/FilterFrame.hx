package arm.filter;

// import arm.format.BlendParser.Handle;
import arm.ui.UISidebar;
import zui.Zui.Handle;
import zui.Id;

class FilterFrame extends FilterBase
{
    public static var filterNamesTr: Array<String> = [];
    public var filter: FilterBase = null;
    var ch = Id.handle();

    public function new() {
        this.name = "None";

        // tr names
        if (filterNamesTr.length == 0) {
            for (i in 0...FilterFactory.filterNames.length) {
                filterNamesTr.push(tr(FilterFactory.filterNames[i]));
            }
        }
    }

    override public function draw() {
        var ui = UISidebar.inst.ui;
        var title = filter == null ? name : filter.name;
        if (ui.panel(h, tr(title))) {
            ui.row([4/5, 1/5]);

            // header
            // var ch = new Handle({position: FilterFactory.filterNames.indexOf(title)});
            ch.position = FilterFactory.filterNames.indexOf(title);
            ui.combo(ch, filterNamesTr, tr("Type"), true, zui.Zui.Align.Right);
            if (ch.changed) {
                filter = FilterFactory.CreateFilterByIndex(ch.position);
            }

            ui.button(tr("Remove"));
            if (filter != null) filter.draw();
        }
    }

}
