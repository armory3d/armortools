
let _input_occupied: bool = false;
let _input_registered: bool = false;

function input_reset() {
	// _input_occupied = false;
	mouse_reset();
	pen_reset();
	keyboard_reset();
	gamepad_reset();
}

function input_end_frame() {
	mouse_end_frame();
	pen_end_frame();
	keyboard_end_frame();
	gamepad_end_frame();
}

function _input_on_foreground() {
	mouse_reset();
}

function input_register() {
	if (_input_registered) {
		return;
	}
	_input_registered = true;
	app_notify_on_end_frame(input_end_frame);
	app_notify_on_reset(input_reset);
	// Reset mouse delta on foreground
	sys_notify_on_app_state(_input_on_foreground, null, null, null, null);
	keyboard_reset();
	gamepad_reset();
}

let _mouse_buttons: string[] = ["left", "right", "middle", "side1", "side2"];
let _mouse_buttons_down: bool[] = [false, false, false, false, false];
let _mouse_buttons_started: bool[] = [false, false, false, false, false];
let _mouse_buttons_released: bool[] = [false, false, false, false, false];

let mouse_x: f32 = 0.0;
let mouse_y: f32 = 0.0;
let mouse_moved: bool = false;
let mouse_movement_x: f32 = 0.0;
let mouse_movement_y: f32 = 0.0;
let mouse_wheel_delta: f32 = 0.0;
let mouse_locked: bool = false;
let mouse_hidden: bool = false;
let mouse_last_x: f32 = -1.0;
let mouse_last_y: f32 = -1.0;

function mouse_end_frame() {
	_mouse_buttons_started[0] = _mouse_buttons_started[1] = _mouse_buttons_started[2] = _mouse_buttons_started[3] = _mouse_buttons_started[4] = false;
	_mouse_buttons_released[0] = _mouse_buttons_released[1] = _mouse_buttons_released[2] = _mouse_buttons_released[3] = _mouse_buttons_released[4] = false;
	mouse_moved = false;
	mouse_movement_x = 0;
	mouse_movement_y = 0;
	mouse_wheel_delta = 0;
}

function mouse_reset() {
	_mouse_buttons_down[0] = _mouse_buttons_down[1] = _mouse_buttons_down[2] = _mouse_buttons_down[3] = _mouse_buttons_down[4] = false;
	mouse_end_frame();
}

function mouse_button_index(button: string): i32 {
	for (let i: i32 = 0; i < _mouse_buttons.length; ++i) {
		if (_mouse_buttons[i] == button) {
			return i;
		}
	}
	return 0;
}

function mouse_down(button: string = "left"): bool {
	return _mouse_buttons_down[mouse_button_index(button)];
}

function mouse_down_any(): bool {
	return _mouse_buttons_down[0] || _mouse_buttons_down[1] || _mouse_buttons_down[2] || _mouse_buttons_down[3] || _mouse_buttons_down[4];
}

function mouse_started(button: string = "left"): bool {
	return _mouse_buttons_started[mouse_button_index(button)];
}

function mouse_started_any(): bool {
	return _mouse_buttons_started[0] || _mouse_buttons_started[1] || _mouse_buttons_started[2] || _mouse_buttons_started[3] || _mouse_buttons_started[4];
}

function mouse_released(button: string = "left"): bool {
	return _mouse_buttons_released[mouse_button_index(button)];
}

function mouse_lock() {
	if (sys_can_lock_mouse()) {
		sys_lock_mouse();
		mouse_locked = true;
		mouse_hidden = true;
	}
}

function mouse_unlock() {
	if (sys_can_lock_mouse()) {
		sys_unlock_mouse();
		mouse_locked = false;
		mouse_hidden = false;
	}
}

function mouse_hide() {
	sys_hide_system_cursor();
	mouse_hidden = true;
}

function mouse_show() {
	sys_show_system_cursor();
	mouse_hidden = false;
}

function mouse_down_listener(index: i32, x: i32, y: i32) {
	if (pen_in_use) {
		return;
	}

	_mouse_buttons_down[index] = true;
	_mouse_buttons_started[index] = true;
	mouse_x = x;
	mouse_y = y;
	///if (arm_android || arm_ios) // For movement delta using touch
	if (index == 0) {
		mouse_last_x = x;
		mouse_last_y = y;
	}
	///end
}

function mouse_up_listener(index: i32, x: i32, y: i32) {
	if (pen_in_use) {
		return;
	}

	_mouse_buttons_down[index] = false;
	_mouse_buttons_released[index] = true;
	mouse_x = x;
	mouse_y = y;
}

function mouse_move_listener(x: i32, y: i32, movement_x: i32, movement_y: i32) {
	if (mouse_last_x == -1.0 && mouse_last_y == -1.0) { // First frame init
		mouse_last_x = x;
		mouse_last_y = y;
	}
	if (mouse_locked) {
		// Can be called multiple times per frame
		mouse_movement_x += movement_x;
		mouse_movement_y += movement_y;
	}
	else {
		mouse_movement_x += x - mouse_last_x;
		mouse_movement_y += y - mouse_last_y;
	}
	mouse_last_x = x;
	mouse_last_y = y;
	mouse_x = x;
	mouse_y = y;
	mouse_moved = true;
}

function mouse_wheel_listener(delta: i32) {
	mouse_wheel_delta = delta;
}

///if (arm_android || arm_ios)
function mouse_on_touch_down(index: i32, x: i32, y: i32) {
	if (index == 1) { // Two fingers down - right mouse button
		_mouse_buttons_down[0] = false;
		mouse_down_listener(1, math_floor(mouse_x), math_floor(mouse_y));
		mouse_pinch_started = true;
		mouse_pinch_total = 0.0;
		mouse_pinch_dist = 0.0;
	}
	else if (index == 2) { // Three fingers down - middle mouse button
		_mouse_buttons_down[1] = false;
		mouse_down_listener(2, math_floor(mouse_x), math_floor(mouse_y));
	}
}

function mouse_on_touch_up(index: i32, x: i32, y: i32) {
	if (index == 1) {
		mouse_up_listener(1, math_floor(mouse_x), math_floor(mouse_y));
	}
	else if (index == 2) {
		mouse_up_listener(2, math_floor(mouse_x), math_floor(mouse_y));
	}
}

let mouse_pinch_dist: f32 = 0.0;
let mouse_pinch_total: f32 = 0.0;
let mouse_pinch_started: bool = false;

function mouse_on_touch_move(index: i32, x: i32, y: i32) {
	// Pinch to zoom - mouse wheel
	if (index == 1) {
		let last_dist: f32 = mouse_pinch_dist;
		let dx: f32 = mouse_x - x;
		let dy: f32 = mouse_y - y;
		mouse_pinch_dist = math_sqrt(dx * dx + dy * dy);
		mouse_pinch_total += last_dist != 0 ? last_dist - mouse_pinch_dist : 0;
		if (!mouse_pinch_started) {
			mouse_wheel_delta = mouse_pinch_total / 10;
			if (mouse_wheel_delta != 0) {
				mouse_pinch_total = 0.0;
			}
		}
		mouse_pinch_started = false;
	}
}
///end

function mouse_view_x(): f32 {
	return mouse_x - app_x();
}

function mouse_view_y(): f32 {
	return mouse_y - app_y();
}

let pen_buttons: string[] = ["tip"];
let pen_buttons_down: bool[] = [false];
let pen_buttons_started: bool[] = [false];
let pen_buttons_released: bool[] = [false];

let pen_x: f32 = 0.0;
let pen_y: f32 = 0.0;
let pen_moved: bool = false;
let pen_movement_x: f32 = 0.0;
let pen_movement_y: f32 = 0.0;
let pen_pressure: f32 = 0.0;
let pen_connected: bool = false;
let pen_in_use: bool = false;
let pen_last_x: f32 = -1.0;
let pen_last_y: f32 = -1.0;

function pen_end_frame() {
	pen_buttons_started[0] = false;
	pen_buttons_released[0] = false;
	pen_moved = false;
	pen_movement_x = 0;
	pen_movement_y = 0;
	pen_in_use = false;
}

function pen_reset() {
	pen_buttons_down[0] = false;
	pen_end_frame();
}

function pen_button_index(button: string): i32 {
	return 0;
}

function pen_down(button: string = "tip"): bool {
	return pen_buttons_down[pen_button_index(button)];
}

function pen_started(button: string = "tip"): bool {
	return pen_buttons_started[pen_button_index(button)];
}

function pen_released(button: string = "tip"): bool {
	return pen_buttons_released[pen_button_index(button)];
}

function pen_down_listener(x: i32, y: i32, pressure: f32) {
	pen_buttons_down[0] = true;
	pen_buttons_started[0] = true;
	pen_x = x;
	pen_y = y;
	pen_pressure = pressure;

	///if (!arm_android && !arm_ios)
	mouse_down_listener(0, x, y);
	///end
}

function pen_up_listener(x: i32, y: i32, pressure: f32) {
	///if (!arm_android && !arm_ios)
	if (pen_buttons_started[0]) {
		pen_buttons_started[0] = false;
		pen_in_use = true;
		return;
	}
	///end

	pen_buttons_down[0] = false;
	pen_buttons_released[0] = true;
	pen_x = x;
	pen_y = y;
	pen_pressure = pressure;

	///if (!arm_android && !arm_ios)
	mouse_up_listener(0, x, y);
	pen_in_use = true; // On pen release, additional mouse down & up events are fired at once - filter those out
	///end
}

function pen_move_listener(x: i32, y: i32, pressure: f32) {
	///if arm_ios
	// Listen to pen hover if no other input is active
	if (!pen_buttons_down[0] && pressure == 0.0) {
		if (!mouse_down_any()) {
			mouse_move_listener(x, y, 0, 0);
		}
		return;
	}
	///end

	if (pen_last_x == -1.0 && pen_last_y == -1.0) { // First frame init
		pen_last_x = x;
		pen_last_y = y;
	}
	pen_movement_x = x - pen_last_x;
	pen_movement_y = y - pen_last_y;
	pen_last_x = x;
	pen_last_y = y;
	pen_x = x;
	pen_y = y;
	pen_moved = true;
	pen_pressure = pressure;
	pen_connected = true;
}

function pen_view_x(): f32 {
	return pen_x - app_x();
}

function pen_view_y(): f32 {
	return pen_y - app_y();
}

let keyboard_keys: string[] = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "space", "backspace", "tab", "enter", "shift", "control", "alt", "win", "escape", "delete", "up", "down", "left", "right", "back", ",", ".", ":", ";", "<", "=", ">", "?", "!", "\"", "#", "$", "%", "&", "_", "(", ")", "*", "|", "{", "}", "[", "]", "~", "`", "/", "\\", "@", "+", "-", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12"];
let keyboard_keys_down: map_t<string, bool> = map_create();
let keyboard_keys_started: map_t<string, bool> = map_create();
let keyboard_keys_released: map_t<string, bool> = map_create();
let keyboard_keys_frame: string[] = [];
let keyboard_repeat_key: bool = false;
let keyboard_repeat_time: f32 = 0.0;

function keyboard_end_frame() {
	if (keyboard_keys_frame.length > 0) {
		for (let i: i32 = 0; i < keyboard_keys_frame.length; ++i) {
			let s: string = keyboard_keys_frame[i];
			map_set(keyboard_keys_started, s, false);
			map_set(keyboard_keys_released, s, false);
		}
		array_splice(keyboard_keys_frame, 0, keyboard_keys_frame.length);
	}

	if (time_time() - keyboard_repeat_time > 0.05) {
		keyboard_repeat_time = time_time();
		keyboard_repeat_key = true;
	}
	else {
		keyboard_repeat_key = false;
	}
}

function keyboard_reset() {
	// Use map_t for now..
	for (let i: i32 = 0; i < keyboard_keys.length; ++i) {
		let s: string = keyboard_keys[i];
		map_set(keyboard_keys_down, s, false);
		map_set(keyboard_keys_started, s, false);
		map_set(keyboard_keys_released, s, false);
	}
	keyboard_end_frame();
}

function keyboard_down(key: string): bool {
	return map_get(keyboard_keys_down, key);
}

function keyboard_started(key: string): bool {
	return map_get(keyboard_keys_started, key);
}

function keyboard_started_any(): bool {
	return keyboard_keys_frame.length > 0;
}

function keyboard_released(key: string): bool {
	return map_get(keyboard_keys_released, key);
}

function keyboard_repeat(key: string): bool {
	return map_get(keyboard_keys_started, key) || (keyboard_repeat_key && map_get(keyboard_keys_down, key));
}

function keyboard_key_code(key: key_code_t): string {
	if (key == key_code_t.SPACE) {
		return "space";
	}
	else if (key == key_code_t.BACKSPACE) {
		return "backspace";
	}
	else if (key == key_code_t.TAB) {
		return "tab";
	}
	else if (key == key_code_t.RETURN) {
		return "enter";
	}
	else if (key == key_code_t.SHIFT) {
		return "shift";
	}
	else if (key == key_code_t.CONTROL) {
		return "control";
	}
	///if arm_macos
	else if (key == key_code_t.META) {
		return "control";
	}
	///end
	else if (key == key_code_t.ALT) {
		return "alt";
	}
	else if (key == key_code_t.WIN) {
		return "win";
	}
	else if (key == key_code_t.ESCAPE) {
		return "escape";
	}
	else if (key == key_code_t.DELETE) {
		return "delete";
	}
	else if (key == key_code_t.UP) {
		return "up";
	}
	else if (key == key_code_t.DOWN) {
		return "down";
	}
	else if (key == key_code_t.LEFT) {
		return "left";
	}
	else if (key == key_code_t.RIGHT) {
		return "right";
	}
	else if (key == key_code_t.BACK) {
		return "back";
	}
	else if (key == key_code_t.COMMA) {
		return ",";
	}
	else if (key == key_code_t.PERIOD) {
		return ".";
	}
	else if (key == key_code_t.COLON) {
		return ":";
	}
	else if (key == key_code_t.SEMICOLON) {
		return ";";
	}
	else if (key == key_code_t.LESS_THAN) {
		return "<";
	}
	else if (key == key_code_t.EQUALS) {
		return "=";
	}
	else if (key == key_code_t.GREATER_THAN) {
		return ">";
	}
	else if (key == key_code_t.QUESTION_MARK) {
		return "?";
	}
	else if (key == key_code_t.EXCLAMATION) {
		return "!";
	}
	else if (key == key_code_t.DOUBLE_QUOTE) {
		return "\"";
	}
	else if (key == key_code_t.HASH) {
		return "#";
	}
	else if (key == key_code_t.DOLLAR) {
		return "$";
	}
	else if (key == key_code_t.PERCENT) {
		return "%";
	}
	else if (key == key_code_t.AMPERSAND) {
		return "&";
	}
	else if (key == key_code_t.UNDERSCORE) {
		return "_";
	}
	else if (key == key_code_t.OPEN_PAREN) {
		return "(";
	}
	else if (key == key_code_t.CLOSE_PAREN) {
		return ")";
	}
	else if (key == key_code_t.ASTERISK) {
		return "*";
	}
	else if (key == key_code_t.PIPE) {
		return "|";
	}
	else if (key == key_code_t.OPEN_CURLY_BRACKET) {
		return "{";
	}
	else if (key == key_code_t.CLOSE_CURLY_BRACKET) {
		return "}";
	}
	else if (key == key_code_t.OPEN_BRACKET) {
		return "[";
	}
	else if (key == key_code_t.CLOSE_BRACKET) {
		return "]";
	}
	else if (key == key_code_t.TILDE) {
		return "~";
	}
	else if (key == key_code_t.BACK_QUOTE) {
		return "`";
	}
	else if (key == key_code_t.SLASH) {
		return "/";
	}
	else if (key == key_code_t.BACK_SLASH) {
		return "\\";
	}
	else if (key == key_code_t.AT) {
		return "@";
	}
	else if (key == key_code_t.ADD) {
		return "+";
	}
	else if (key == key_code_t.PLUS) {
		return "+";
	}
	else if (key == key_code_t.SUBTRACT) {
		return "-";
	}
	else if (key == key_code_t.HYPHEN_MINUS) {
		return "-";
	}
	else if (key == key_code_t.MULTIPLY) {
		return "*";
	}
	else if (key == key_code_t.DIVIDE) {
		return "/";
	}
	else if (key == key_code_t.DECIMAL) {
		return ".";
	}
	else if (key == key_code_t.ZERO) {
		return "0";
	}
	else if (key == key_code_t.NUMPAD0) {
		return "0";
	}
	else if (key == key_code_t.ONE) {
		return "1";
	}
	else if (key == key_code_t.NUMPAD1) {
		return "1";
	}
	else if (key == key_code_t.TWO) {
		return "2";
	}
	else if (key == key_code_t.NUMPAD2) {
		return "2";
	}
	else if (key == key_code_t.THREE) {
		return "3";
	}
	else if (key == key_code_t.NUMPAD3) {
		return "3";
	}
	else if (key == key_code_t.FOUR) {
		return "4";
	}
	else if (key == key_code_t.NUMPAD4) {
		return "4";
	}
	else if (key == key_code_t.FIVE) {
		return "5";
	}
	else if (key == key_code_t.NUMPAD5) {
		return "5";
	}
	else if (key == key_code_t.SIX) {
		return "6";
	}
	else if (key == key_code_t.NUMPAD6) {
		return "6";
	}
	else if (key == key_code_t.SEVEN) {
		return "7";
	}
	else if (key == key_code_t.NUMPAD7) {
		return "7";
	}
	else if (key == key_code_t.EIGHT) {
		return "8";
	}
	else if (key == key_code_t.NUMPAD8) {
		return "8";
	}
	else if (key == key_code_t.NINE) {
		return "9";
	}
	else if (key == key_code_t.NUMPAD9) {
		return "9";
	}
	else if (key == key_code_t.F1) {
		return "f1";
	}
	else if (key == key_code_t.F2) {
		return "f2";
	}
	else if (key == key_code_t.F3) {
		return "f3";
	}
	else if (key == key_code_t.F4) {
		return "f4";
	}
	else if (key == key_code_t.F5) {
		return "f5";
	}
	else if (key == key_code_t.F6) {
		return "f6";
	}
	else if (key == key_code_t.F7) {
		return "f7";
	}
	else if (key == key_code_t.F8) {
		return "f8";
	}
	else if (key == key_code_t.F9) {
		return "f9";
	}
	else if (key == key_code_t.F10) {
		return "f10";
	}
	else if (key == key_code_t.F11) {
		return "f11";
	}
	else if (key == key_code_t.F12) {
		return "f12";
	}
	else {
		return to_lower_case(string_from_char_code(key));
	}
}

function keyboard_down_listener(code: key_code_t) {
	let s: string = keyboard_key_code(code);
	array_push(keyboard_keys_frame, s);
	map_set(keyboard_keys_started, s, true);
	map_set(keyboard_keys_down, s, true);
	keyboard_repeat_time = time_time() + 0.4;

	///if arm_android_rmb // Detect right mouse button on Android..
	if (code == key_code_t.BACK) {
		if (!_mouse_buttons_down[1]) {
			mouse_down_listener(1, math_floor(mouse_x), math_floor(mouse_y));
		}
	}
	///end
}

function keyboard_up_listener(code: key_code_t) {
	let s: string = keyboard_key_code(code);
	array_push(keyboard_keys_frame, s);
	map_set(keyboard_keys_released, s, true);
	map_set(keyboard_keys_down, s, false);

	///if arm_android_rmb
	if (code == key_code_t.BACK) {
		mouse_up_listener(1, math_floor(mouse_x), math_floor(mouse_y));
	}
	///end
}

function keyboard_press_listener(c: string) {}

let gamepad_buttons_ps: string[] = ["cross", "circle", "square", "triangle", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
let gamepad_buttons_xbox: string[] = ["a", "b", "x", "y", "l1", "r1", "l2", "r2", "share", "options", "l3", "r3", "up", "down", "left", "right", "home", "touchpad"];
let gamepad_buttons: string[] = gamepad_buttons_ps;
let gamepad_raws: gamepad_t[];

function gamepad_end_frame() {
	// TODO: check valgrind error
	// for (let i: i32 = 0; i < gamepad_raws.length; ++i) {
	// 	let g: gamepad_t = gamepad_raws[i];
	// 	if (g.buttons_frame.length > 0) {
	// 		for (let j: i32 = 0; j < g.buttons_frame.length; ++j) {
	// 			let b: i32 = g.buttons_frame[j];
	// 			g.buttons_started[b] = false;
	// 			g.buttons_released[b] = false;
	// 		}
	// 		array_splice(g.buttons_frame, 0, g.buttons_frame.length);
	// 	}
	// 	g.left_stick.moved = false;
	// 	g.left_stick.movement_x = 0;
	// 	g.left_stick.movement_y = 0;
	// 	g.right_stick.moved = false;
	// 	g.right_stick.movement_x = 0;
	// 	g.right_stick.movement_y = 0;
	// }
}

function gamepad_stick_create(): gamepad_stick_t {
	let raw: gamepad_stick_t = {};
	raw.x = 0.0;
	raw.y = 0.0;
	raw.last_x = 0.0;
	raw.last_y = 0.0;
	raw.moved = false;
	raw.movement_x = 0.0;
	raw.movement_y = 0.0;
	return raw;
}

function gamepad_create(): gamepad_t {
	let raw: gamepad_t = {};
	raw.buttons_down = [];
	raw.buttons_started = [];
	raw.buttons_released = [];
	raw.buttons_frame = [];
	raw.left_stick = gamepad_stick_create();
	raw.right_stick = gamepad_stick_create();
	return raw;
}

function gamepad_reset() {
	gamepad_raws = [];
	for (let i: i32 = 0; i < 4; ++i) {
		let g: gamepad_t = gamepad_create();
		array_push(gamepad_raws, g);
		for (let i: i32 = 0; i < gamepad_buttons.length; ++i) {
			array_push(g.buttons_down, 0.0);
			array_push(g.buttons_started, false);
			array_push(g.buttons_released, false);
		}
	}

	gamepad_end_frame();
}

function gamepad_key_code(button: i32): string {
	return gamepad_buttons[button];
}

function gamepad_button_index(button: string): i32 {
	for (let i: i32 = 0; i < gamepad_buttons.length; ++i) {
		if (gamepad_buttons[i] == button) {
			return i;
		}
	}
	return 0;
}

function gamepad_down(i: i32, button: string): f32 {
	return gamepad_raws[i].buttons_down[gamepad_button_index(button)];
}

function gamepad_started(i: i32, button: string): bool {
	return gamepad_raws[i].buttons_started[gamepad_button_index(button)];
}

function gamepad_released(i: i32, button: string): bool {
	return gamepad_raws[i].buttons_released[gamepad_button_index(button)];
}

function gamepad_axis_listener(i: i32, axis: i32, value: f32) {
	let stick: gamepad_stick_t = axis <= 1 ? gamepad_raws[i].left_stick : gamepad_raws[i].right_stick;

	if (axis == 0 || axis == 2) { // X
		stick.last_x = stick.x;
		stick.x = value;
		stick.movement_x = stick.x - stick.last_x;
	}
	else if (axis == 1 || axis == 3) { // Y
		stick.last_y = stick.y;
		stick.y = value;
		stick.movement_y = stick.y - stick.last_y;
	}
	stick.moved = true;
}

function gamepad_button_listener(i: i32, button: i32, value: f32) {
	// array_push(gamepad_raws[i].buttons_frame, button);

	// gamepad_raws[i].buttons_down[button] = value;
	// if (value > 0) {
	// 	gamepad_raws[i].buttons_started[button] = true; // Will trigger L2/R2 multiple times..
	// }
	// else {
	// 	gamepad_raws[i].buttons_released[button] = true;
	// }
}

type gamepad_stick_t = {
	x?: f32;
	y?: f32;
	last_x?: f32;
	last_y?: f32;
	moved?: bool;
	movement_x?: f32;
	movement_y?: f32;
};

type gamepad_t = {
	buttons_down?: f32[]; // Intensity 0 - 1
	buttons_started?: bool[];
	buttons_released?: bool[];
	buttons_frame?: i32[];
	left_stick?: gamepad_stick_t;
	right_stick?: gamepad_stick_t;
};

enum key_code_t {
	UNKNOWN = 0,
	BACK = 1, // Android
	CANCEL = 3,
	HELP = 6,
	BACKSPACE = 8,
	TAB = 9,
	RETURN = 13,
	SHIFT = 16,
	CONTROL = 17,
	ALT = 18,
	PAUSE = 19,
	CAPS_LOCK = 20,
	ESCAPE = 27,
	SPACE = 32,
	PAGE_UP = 33,
	PAGE_DOWN = 34,
	END = 35,
	HOME = 36,
	LEFT = 37,
	UP = 38,
	RIGHT = 39,
	DOWN = 40,
	PRINT_SCREEN = 44,
	INSERT = 45,
	DELETE = 46,
	ZERO = 48,
	ONE = 49,
	TWO = 50,
	THREE = 51,
	FOUR = 52,
	FIVE = 53,
	SIX = 54,
	SEVEN = 55,
	EIGHT = 56,
	NINE = 57,
	COLON = 58,
	SEMICOLON = 59,
	LESS_THAN = 60,
	EQUALS = 61,
	GREATER_THAN = 62,
	QUESTION_MARK = 63,
	AT = 64,
	A = 65,
	B = 66,
	C = 67,
	D = 68,
	E = 69,
	F = 70,
	G = 71,
	H = 72,
	I = 73,
	J = 74,
	K = 75,
	L = 76,
	M = 77,
	N = 78,
	O = 79,
	P = 80,
	Q = 81,
	R = 82,
	S = 83,
	T = 84,
	U = 85,
	V = 86,
	W = 87,
	X = 88,
	Y = 89,
	Z = 90,
	WIN = 91,
	CONTEXT_MENU = 93,
	SLEEP = 95,
	NUMPAD0 = 96,
	NUMPAD1 = 97,
	NUMPAD2 = 98,
	NUMPAD3 = 99,
	NUMPAD4 = 100,
	NUMPAD5 = 101,
	NUMPAD6 = 102,
	NUMPAD7 = 103,
	NUMPAD8 = 104,
	NUMPAD9 = 105,
	MULTIPLY = 106,
	ADD = 107,
	SEPARATOR = 108,
	SUBTRACT = 109,
	DECIMAL = 110,
	DIVIDE = 111,
	F1 = 112,
	F2 = 113,
	F3 = 114,
	F4 = 115,
	F5 = 116,
	F6 = 117,
	F7 = 118,
	F8 = 119,
	F9 = 120,
	F10 = 121,
	F11 = 122,
	F12 = 123,
	F13 = 124,
	F14 = 125,
	F15 = 126,
	F16 = 127,
	F17 = 128,
	F18 = 129,
	F19 = 130,
	F20 = 131,
	F21 = 132,
	F22 = 133,
	F23 = 134,
	F24 = 135,
	NUM_LOCK = 144,
	SCROLL_LOCK = 145,
	EXCLAMATION = 161,
	DOUBLE_QUOTE = 162,
	HASH = 163,
	DOLLAR = 164,
	PERCENT = 165,
	AMPERSAND = 166,
	UNDERSCORE = 167,
	OPEN_PAREN = 168,
	CLOSE_PAREN = 169,
	ASTERISK = 170,
	PLUS = 171,
	PIPE = 172,
	HYPHEN_MINUS = 173,
	OPEN_CURLY_BRACKET = 174,
	CLOSE_CURLY_BRACKET = 175,
	TILDE = 176,
	VOLUME_MUTE = 181,
	VOLUME_DOWN = 182,
	VOLUME_UP = 183,
	COMMA = 188,
	PERIOD = 190,
	SLASH = 191,
	BACK_QUOTE = 192,
	OPEN_BRACKET = 219,
	BACK_SLASH = 220,
	CLOSE_BRACKET = 221,
	QUOTE = 222,
	META = 224,
	ALTGR = 225,
}
