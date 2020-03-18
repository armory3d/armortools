package arm.ui;

import zui.Zui;
import zui.Id;

#if arm_creator

@:access(zui.Zui)
class TabTraits {

	static var traits = ["PhysicsBody"];

	public static function draw() {
		var ui = UISidebar.inst.ui;
		if (ui.tab(UISidebar.inst.htab1, tr("Traits"))) {
			if (Context.object != null) {

				ui.row([1 / 4]);
				if (ui.button(tr("New"))) {
					UIMenu.draw(function(ui:Zui) {
						ui.text(tr("New"), Right, ui.t.HIGHLIGHT_COL);
						for (t in traits) {
							if (ui.button(t, Left)) {
								var cname = Type.resolveClass("arm.plugin." + t);
								if (cname == null) return;
								var inst = Type.createInstance(cname, []);
								Context.object.addTrait(inst);
							}
						}
					}, traits.length);
				}

				if (Context.object.traits.length > 0) {
					for (t in Context.object.traits) {
						ui.text(Type.getClassName(Type.getClass(t)));
						for (i in untyped 0...t.props.length) {
							var id = Id.handle();
							id.value = Reflect.field(t, untyped t.props[i]);
							ui.slider(id, untyped t.props[i]);
							if (id.changed) Reflect.setProperty(t, untyped t.props[i], id.value);
						}
					}
				}
			}
		}
	}
}

#end
