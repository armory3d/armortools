package arm.filter;

import arm.ui.UISidebar;
import zui.Zui.Handle;
import arm.node.MakeMaterial;

class ColorCorrection extends FilterBase
{

    public var gh = new Handle();
    public var ch = new Handle({value: 1});
    public var bh = new Handle({value: 1});
    public var sh = new Handle({value: 1});
    public var hh = new Handle();

    public function new() {
        this.name = "Color Correction";
    }

    override public function draw() {
        super.draw();
        var ui = UISidebar.inst.ui;

        ui.slider(gh, tr("Gama"), -3, 3, true);
        ui.slider(bh, tr("Brightness"), 0, 3, true);
        ui.slider(ch, tr("Contrast"), 0, 3, true);
        ui.slider(sh, tr("Saturation"), 0, 5, true);
        ui.slider(hh, tr("Hue Shift"), 0, 1, true);

        if (gh.changed) { MakeMaterial.parseMeshMaterial(); }
        if (ch.changed) { MakeMaterial.parseMeshMaterial(); }
        if (bh.changed) { MakeMaterial.parseMeshMaterial(); }
        if (sh.changed) { MakeMaterial.parseMeshMaterial(); }
        if (hh.changed) { MakeMaterial.parseMeshMaterial(); }
    }

    override function getShaderText(color: String) : String {
        var ret = "";

        // hue
        // var root3 = "vec3(0.57735, 0.57735, 0.57735)";
        // var half_angle = '0.5 * radians(${hh.value})';
        // var quat = 'vec4( ($root3 * sin($half_angle)), cos($half_angle) )';
        // var diag = '0.5 - ($quat.rgb * $quat.rgb)';
        // var a = '($quat.gbr * $quat.brg) + ($quat.a * $quat.rgb)';
        // var b = '($quat.gbr * $quat.brg) - ($quat.a * $quat.rgb)';

        // var rot_matrix = 'vec3x3(
        //                         2.0*vec3($diag.x, $b.z, $a,y), 
        //                         2.0*vec3($a.z, $diag.y, $b.x), 
        //                         2.0*vec3($b.y, $a.x, $diag.z))';
        // ret += '$color.rgb = mul($rot_matrix, $color.rgb);';
        var vec = 'vec4(${hh.value}, ${sh.value}, ${bh.value}, 0)';
        ret += '$color.rgb = hue_sat($color.rgb, $vec);';

        // Gama
        var gamma = (gh.value < 0.0) ? (-gh.value + 1.0) : (1.0 / (gh.value + 1.0));
        ret += '$color.rgb = pow($color.rgb, vec3($gamma, $gamma, $gamma));';

        //　Contrast
        ret += '$color.rgb = lerp(vec3(0.5, 0.5, 0.5), $color.rgb, ${ch.value});';

        // Brightness
        // ret += '$color.rgb += ${bh.value};';

        // Saturation
        // var luminance = '$color.r * 0.2125 + $color.g * 0.7154 + $color.b * 0.0721';
        // var luminanceColor = 'vec3($luminance, $luminance, $luminance)';
        // ret += '$color.rgb = lerp($luminanceColor, $color.rgb, ${sh.value});';

        return ret;
    }

}
