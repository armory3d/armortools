package arm.filter;

import arm.ui.UISidebar;
import zui.Zui.Handle;
import arm.Context;
import arm.node.MakeMaterial;

class FilterFrame extends FilterBase
{
    var filter: FilterBase = null;
    public var h = new Handle();  // panel handle
    var ch = new Handle();  // combo handle
    static var delCount = 0;

    override function name() : String {
        return tr("None");
    }

    override public function draw() {
        var ui = UISidebar.inst.ui;
        var title = filter == null ? name() : filter.name();
        if (ui.panel(h, title)) {
            ui.indent(false);
            ui.row([4/5, 1/5]);

            // header
            ch.position = FilterFactory.getFilterNames().indexOf(title);
            ui.combo(ch, FilterFactory.getFilterNames());
            if (ch.changed) {
                filter = FilterFactory.CreateFilterByIndex(ch.position);
            }

            if (ui.button(tr("Remove"))) delete();
            if (filter != null) filter.draw();

            ui.unindent(false);
            ui.separator(4, true);
        }
    }

    override function getShaderText(color: String) : String {
        if (filter != null) return filter.getShaderText(color);
        return "";
    }

    public function delete() {
        // delete self from layer

        if (delCount != 0) {
            delCount = 0;
            return;
        }

        var lay = Context.layer;
        var i = lay.filters.indexOf(this) + 1;
        if (i <= 0) return;
        if (lay.filters.length != i) {
            for (j in i...lay.filters.length) {
                var f = cast(lay.filters[j], FilterFrame);
                if (!f.h.selected) continue;
                delCount = 10;
                break;
            }
        }

        lay.filters.remove(this);
		UISidebar.inst.hwnd1.redraws = 2;
        MakeMaterial.parseMeshMaterial();
    }

}
