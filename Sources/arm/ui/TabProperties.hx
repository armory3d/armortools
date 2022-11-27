package arm.ui;

import kha.Color;
import zui.Zui;
import zui.Id;
import arm.node.MakeMaterial;
import arm.util.RenderUtil;


class TabProperties {

	@:access(zui.Zui)
	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab0, tr("Properties"))) {
            ui.text(Context.material.canvas.name);
			ui.separator(3, false);
            
			var nodes = Context.material.canvas.nodes;
            for (node in nodes) {
                if (node.type == "MATERIAL_INPUT") {
                    for (n in 0...node.outputs.length) {
                        
                        if (node.outputs[n].type == "RGBA") {
                            var h = Id.handle().nest(n);
                            var oldColor = Color.fromFloats(node.outputs[n].default_value[0],node.outputs[n].default_value[1],node.outputs[n].default_value[2]);
                            h.color = oldColor;
                            ui.row([1/3, 2/3]);
                            ui.text(node.outputs[n].name);
                            ui.text("", 0, h.color);
                            
                            if (ui.isHovered && ui.inputDown) {
                                UIMenu.draw(function(ui) {
                                    ui.fill(0, 0, ui._w / ui.ops.scaleFactor, ui.t.ELEMENT_H * 9, ui.t.SEPARATOR_COL);
                                    ui.changed = false;
                                    var color = zui.Ext.colorWheel(ui, h, false, null, 10 * ui.t.ELEMENT_H * ui.SCALE(), true);
                                    if (ui.changed) UIMenu.keepOpen = true;

                                    if (h.changed){
                                        node.outputs[n].default_value = [h.color.R, h.color.G, h.color.B, h.color.A];
                                        App.notifyOnNextFrame(function() {
                                            UINodes.inst.canvasChanged();
                                            TabMaterials.updateMaterial();
                                            UISidebar.inst.hwnd0.redraws = 1;
                                        });
                                    }
                                }, 10);
                            }
                        }
                        else if (node.outputs[n].type == "VALUE") {
                            var h = Id.handle().nest(n);
                            h.value = node.outputs[n].default_value;
                            var val = ui.slider(h,node.outputs[n].name,node.outputs[n].min,node.outputs[n].max, true,100.0,true, Align.Left);
                            
                            if (h.changed) {
                                node.outputs[n].default_value = val;
                                App.notifyOnNextFrame(function() {
                                    UINodes.inst.canvasChanged();
                                    TabMaterials.updateMaterial();
                                });
                            }
                        }
                        else if (node.outputs[n].type == "VECTOR") {
                            ui.text(node.outputs[n].name);
                            var val = [0.0,0.0,0.0];
                            var h1 = Id.handle().nest(n,{selected: node.outputs[n].default_value[0]});
                            h1.value = node.outputs[n].default_value[0];
                            val[0] = ui.slider(h1,"x",node.outputs[n].min,node.outputs[n].max, true, 100.0, true, Align.Left);
                            var h2 = Id.handle().nest(n,{selected: node.outputs[n].default_value[1]});
                            h2.value = node.outputs[n].default_value[1];
                            val[1] = ui.slider(h2,"y",node.outputs[n].min,node.outputs[n].max, true, 100.0, true, Align.Left);
                            var h3 = Id.handle().nest(n,{selected: node.outputs[n].default_value[2]});
                            h3.value = node.outputs[n].default_value[2];
                            val[2] = ui.slider(h3,"z",node.outputs[n].min,node.outputs[n].max, true, 100.0, true, Align.Left);
                            
                            if (h1.changed || h2.changed || h3.changed) {
                                node.outputs[n].default_value = val;
                                App.notifyOnNextFrame(function() {
                                    UINodes.inst.canvasChanged();
                                    TabMaterials.updateMaterial();
                                });
                            }
                        }
                        ui.separator(5, false);
                    }
                } 
            }

		}
	}
}