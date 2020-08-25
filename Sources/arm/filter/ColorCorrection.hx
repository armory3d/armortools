package arm.filter;

import arm.ui.UISidebar;
import zui.Zui.Handle;

class ColorCorrection extends FilterBase
{

    public var gh = new Handle();
    public var ch = new Handle();
    public var bh = new Handle();
    public var sh = new Handle();
    public var hh = new Handle();
    public var oh = new Handle({value: 100});

    public function new() {
        this.name = "Color Correction";
    }

    override public function draw() {
        super.draw();
        var ui = UISidebar.inst.ui;

        ui.slider(gh, tr("Gama"), -3, 3, true);
        ui.slider(ch, tr("Contrast"), -2, 2, true);
        ui.slider(bh, tr("Brightness"), -1, 1, true);
        ui.slider(sh, tr("Saturation"), -1, 5, true);
        ui.slider(hh, tr("Hue Shift"), -180, 180, true);
        ui.slider(oh, tr("Opacity"), 0, 100, true);

        if (gh.changed) {}
        if (ch.changed) {}
        if (bh.changed) {}
        if (sh.changed) {}
        if (hh.changed) {}
        if (oh.changed) {}
    }

    override function getShaderText() : String {
        return super.getShaderText();
    }

}
