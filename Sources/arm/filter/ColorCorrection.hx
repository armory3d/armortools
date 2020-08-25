package arm.filter;

import arm.ui.UISidebar;
import zui.Zui.Handle;
// import zui.Id;

class ColorCorrection extends FilterBase
{

    var gh = new Handle();
    var ch = new Handle();
    var bh = new Handle();
    var sh = new Handle();
    var hh = new Handle();
    var oh = new Handle({value: 100});

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
    }

}
