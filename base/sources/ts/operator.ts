

let operator_ops: map_t<string, ()=>void> = map_create();

function operator_register(name: string, call: ()=>void) {
	map_set(operator_ops, name, call);
}

function operator_run(name: string) {
	if (map_get(operator_ops, name) != null) {
		let cb: ()=>void = map_get(operator_ops, name);
		cb();
	}
}

function operator_update() {
	if (mouse_started_any() || keyboard_started_any()) {
		let keys: string[] = map_keys(config_keymap);
		for (let i: i32 = 0; i < keys.length; ++i) {
			let op: string = keys[i];
			if (operator_shortcut(map_get(config_keymap, op))) {
				operator_run(op);
			}
		}
	}
}

function operator_shortcut(s: string, type: shortcut_type_t = shortcut_type_t.STARTED): bool {
	if (s == "") {
		return false;
	}
	let shift: bool = string_index_of(s, "shift") >= 0;
	let ctrl: bool = string_index_of(s, "ctrl") >= 0;
	let alt: bool = string_index_of(s, "alt") >= 0;
	let flag: bool = shift == keyboard_down("shift") && ctrl == keyboard_down("control") && alt == keyboard_down("alt");

	if (string_index_of(s, "+") > 0) {
		s = substring(s, string_last_index_of(s, "+") + 1, s.length);
		if (s == "number") {
			return flag;
		}
	}
	else if (shift || ctrl || alt) {
		return flag;
	}

	let key: bool = false;
	if (s == "left" || s == "right" || s == "middle") {
		if (type == shortcut_type_t.DOWN) {
			key = mouse_down(s);
		}
		else {
			key = mouse_started(s);
		}
	}
	else if (type == shortcut_type_t.REPEAT) {
		key = keyboard_repeat(s);
	}
	else if (type == shortcut_type_t.DOWN) {
		key = keyboard_down(s);
	}
	else if (type == shortcut_type_t.RELEASED) {
		key = keyboard_released(s);
	}
	else {
		key = keyboard_started(s);
	}

	return flag && key;
}

enum shortcut_type_t {
	STARTED,
	REPEAT,
	DOWN,
	RELEASED,
}
