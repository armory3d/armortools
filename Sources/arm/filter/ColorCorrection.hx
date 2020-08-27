package arm.filter;

import arm.ui.UISidebar;
import zui.Zui.Handle;

class ColorCorrection extends FilterBase
{

    public var gh = new Handle();
    public var ch = new Handle({value: 1});
    public var bh = new Handle({value: 1});
    public var sh = new Handle({value: 1});
    public var hh = new Handle();

    override function name() : String {
        return tr("Color Correction");
    }

    override public function draw() {
        super.draw();
        var ui = UISidebar.inst.ui;

        ui.slider(gh, tr("Gama"), -3, 3, true);
        ui.slider(bh, tr("Brightness"), 0, 3, true);
        ui.slider(ch, tr("Contrast"), 0, 3, true);
        ui.slider(sh, tr("Saturation"), 0, 5, true);
        ui.slider(hh, tr("Hue Shift"), 0, 1, true);

        if (gh.changed) { update(); }
        if (ch.changed) { update(); }
        if (bh.changed) { update(); }
        if (sh.changed) { update(); }
        if (hh.changed) { update(); }
    }

    override function getShaderText(color: String) : String {
        var ret = "";

        // hue & Brightness & Saturation
        var vec = 'vec4(${hh.value}, ${sh.value}, ${bh.value}, 0)';
        ret += '$color.rgb = hue_sat($color.rgb, $vec);';

        // Gama
        var gamma = (gh.value < 0.0) ? (-gh.value + 1.0) : (1.0 / (gh.value + 1.0));
        ret += '$color.rgb = pow($color.rgb, vec3($gamma, $gamma, $gamma));';

        //　Contrast
        ret += '$color.rgb = lerp(vec3(0.5, 0.5, 0.5), $color.rgb, ${ch.value});';

        return ret;
    }

}
