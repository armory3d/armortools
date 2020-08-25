package arm.filter;

import arm.ui.UISidebar;

class ColorCorrection extends FilterBase
{

    public function new() {
        this.name = "Color Correction";
    }

    override public function draw() {
        super.draw();
        var ui = UISidebar.inst.ui;
        ui.button("Color Correction");
    }

}
