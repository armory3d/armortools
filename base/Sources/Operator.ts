
class Operator {

	static ops = new Map<string, any>();

	static register = (name: string, call: any) => {
		Operator.ops.set(name, call);
	}

	static run = (name: string) => {
		if (Operator.ops.get(name) != null) Operator.ops.get(name)();
	}

	static update = () => {
		if (Mouse.startedAny() || Keyboard.startedAny()) {
			for (let op in Config.keymap) {
				if (Operator.shortcut(Config.keymap[op])) Operator.run(op);
			}
		}
	}

	static shortcut = (s: string, type = ShortcutType.ShortcutStarted): bool => {
		if (s == "") return false;
		let shift = s.indexOf("shift") >= 0;
		let ctrl = s.indexOf("ctrl") >= 0;
		let alt = s.indexOf("alt") >= 0;
		let flag = shift == Keyboard.down("shift") &&
				   ctrl == Keyboard.down("control") &&
				   alt == Keyboard.down("alt");
		if (s.indexOf("+") > 0) {
			s = s.substr(s.lastIndexOf("+") + 1);
			if (s == "number") return flag;
		}
		else if (shift || ctrl || alt) return flag;
		let key = (s == "left" || s == "right" || s == "middle") ?
			// Mouse
			(type == ShortcutType.ShortcutDown ? Mouse.down(s) : Mouse.started(s)) :
			// Keyboard
			(type == ShortcutType.ShortcutRepeat ? Keyboard.repeat(s) : type == ShortcutType.ShortcutDown ? Keyboard.down(s) : type == ShortcutType.ShortcutReleased ? Keyboard.released(s) : Keyboard.started(s));
		return flag && key;
	}
}

enum ShortcutType {
	ShortcutStarted,
	ShortcutRepeat,
	ShortcutDown,
	ShortcutReleased,
}
