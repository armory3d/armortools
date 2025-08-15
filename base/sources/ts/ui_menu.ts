
let ui_menu_show: bool = false;
let ui_menu_nested: bool = false;
let ui_menu_x: i32 = 0;
let ui_menu_y: i32 = 0;
let ui_menu_h: i32 = 0;
let ui_menu_keep_open: bool = false;
let ui_menu_commands: (ui: ui_t)=>void = null;
let ui_menu_show_first: bool = true;
let ui_menu_hide_flag: bool = false;

let _ui_menu_render_msg: string;

function ui_menu_render() {
	let ui: ui_t = ui_base_ui;
	let menu_w: i32 = ui_menu_commands != null ?
		math_floor(base_default_element_w * UI_SCALE() * 2.3) :
		math_floor(UI_ELEMENT_W() * 2.3);

	let _FILL_BUTTON_BG: i32 = ui.ops.theme.FILL_BUTTON_BG;
	ui.ops.theme.FILL_BUTTON_BG = false;
	let _ELEMENT_OFFSET: i32 = ui.ops.theme.ELEMENT_OFFSET;
	ui.ops.theme.ELEMENT_OFFSET = 0;
	let _ELEMENT_H: i32 = ui.ops.theme.ELEMENT_H;
	ui.ops.theme.ELEMENT_H = config_raw.touch_ui ? (28 + 2) : 28;

	if (ui_menu_nested) {
		ui_menu_show_first = true;
		ui_menu_nested = false;
	}

	// First draw out of screen, then align the menu based on menu height
	if (ui_menu_show_first) {
		ui_menu_x -= iron_window_width() * 2;
		ui_menu_y -= iron_window_height() * 2;
	}

	draw_begin();
	ui_begin_region(ui, ui_menu_x, ui_menu_y, menu_w);
	ui_menu_start(ui);

	if (ui_menu_commands != null) {
		ui_menu_commands(ui);
	}

	ui_menu_hide_flag = ui.combo_selected_handle == null && !ui_menu_keep_open && !ui_menu_show_first && (ui.changed || ui.input_released || ui.input_released_r || ui.is_escape_down);
	ui_menu_keep_open = false;

	ui.ops.theme.FILL_BUTTON_BG = _FILL_BUTTON_BG;
	ui.ops.theme.ELEMENT_OFFSET = _ELEMENT_OFFSET;
	ui.ops.theme.ELEMENT_H = _ELEMENT_H;
	ui_menu_end(ui);
	ui_end_region();
	draw_end();

	if (ui_menu_show_first) {
		ui_menu_show_first = false;
		ui_menu_keep_open = true;
		ui_menu_h = ui._y - ui_menu_y;
		ui_menu_x += iron_window_width() * 2;
		ui_menu_y += iron_window_height() * 2;
		ui_menu_fit_to_screen();
		ui_menu_render(); // Render at correct position now
	}

	if (ui_menu_hide_flag) {
		ui_menu_hide();
		ui_menu_show_first = true;
		ui_menu_commands = null;
	}
}

function ui_menu_hide() {
	ui_menu_show = false;
	base_redraw_ui();
}

function ui_menu_draw(commands: (ui: ui_t)=>void = null, x: i32 = -1, y: i32 = -1) {
	ui_end_input();
	if (ui_menu_show) {
		ui_menu_nested = true;
		ui_menu_keep_open = true;
	}
	ui_menu_show = true;
	ui_menu_commands = commands;
	ui_menu_x = x > -1 ? x : math_floor(mouse_x + 1);
	ui_menu_y = y > -1 ? y : math_floor(mouse_y + 1);
	ui_menu_h = 0;
}

function ui_menu_fit_to_screen() {
	// Prevent the menu going out of screen
	let menu_w: f32 = base_default_element_w * UI_SCALE() * 2.3;
	if (ui_menu_x + menu_w > iron_window_width()) {
		if (ui_menu_x - menu_w > 0) {
			ui_menu_x = math_floor(ui_menu_x - menu_w);
		}
		else {
			ui_menu_x = math_floor(iron_window_width() - menu_w);
		}
	}
	if (ui_menu_y + ui_menu_h > iron_window_height()) {
		if (ui_menu_y - ui_menu_h > 0) {
			ui_menu_y = math_floor(ui_menu_y - ui_menu_h);
		}
		else {
			ui_menu_y = iron_window_height() - ui_menu_h;
		}
		ui_menu_x += 1; // Move out of mouse focus
	}
}

function ui_menu_separator(ui: ui_t) {
	ui._y++;
	if (config_raw.touch_ui) {
		ui_fill(0, 0, ui._w / UI_SCALE(), 1, ui.ops.theme.BUTTON_COL);
	}
	else {
		ui_fill(26, 0, ui._w / UI_SCALE() - 26, 1, ui.ops.theme.BUTTON_COL);
	}
}

function ui_menu_button(text: string, label: string = ""): bool {
	if (config_raw.touch_ui) {
		label = "";
	}
	return ui_button(config_button_spacing + text, config_button_align, label);
}

function ui_menu_align(ui: ui_t) {
	if (!config_raw.touch_ui) {
		let row: f32[] = [12 / 100, 88 / 100];
		ui_row(row);
		ui_end_element();
	}
}

function ui_menu_start(ui: ui_t) {
	ui_draw_shadow(ui._x, ui._y, ui._w, ui_menu_h);

	draw_set_color(ui.ops.theme.SEPARATOR_COL);
	ui_draw_rect(true, ui._x, ui._y, ui._w, ui_menu_h);
	draw_set_color(0xffffffff);
}

function ui_menu_end(ui: ui_t) {
}
