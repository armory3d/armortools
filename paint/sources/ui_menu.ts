
let ui_menu_show: bool       = false;
let ui_menu_nested: bool     = false;
let ui_menu_x: i32           = 0;
let ui_menu_y: i32           = 0;
let ui_menu_h: i32           = 0;
let ui_menu_keep_open: bool  = false;
let ui_menu_commands: () => void    = null;
let ui_menu_show_first: bool        = true;
let ui_menu_hide_flag: bool         = false;
let ui_menu_sub_x: i32              = 0;
let ui_menu_sub_y: i32              = 0;
let ui_menu_sub_handle: ui_handle_t = null;

let _ui_menu_render_msg: string;

function ui_menu_render() {
	let menu_w: i32 = ui_menu_commands != null ? math_floor(base_default_element_w * UI_SCALE() * 2.3) : math_floor(UI_ELEMENT_W() * 2.3);

	let _FILL_BUTTON_BG: i32    = ui.ops.theme.FILL_BUTTON_BG;
	ui.ops.theme.FILL_BUTTON_BG = false;
	let _ELEMENT_OFFSET: i32    = ui.ops.theme.ELEMENT_OFFSET;
	ui.ops.theme.ELEMENT_OFFSET = 0;
	let _ELEMENT_H: i32         = ui.ops.theme.ELEMENT_H;
	ui.ops.theme.ELEMENT_H      = config_raw.touch_ui ? (28 + 2) : 28;

	if (ui_menu_nested) {
		ui_menu_show_first = true;
		ui_menu_nested     = false;
	}

	// First draw out of screen, then align the menu based on menu height
	if (ui_menu_show_first) {
		ui_menu_x -= iron_window_width() * 2;
		ui_menu_y -= iron_window_height() * 2;
	}

	draw_begin();
	ui_begin_region(ui, ui_menu_x, ui_menu_y, menu_w);
	ui.input_enabled = ui.combo_selected_handle == null;
	ui_menu_begin();

	if (ui_menu_commands != null) {
		ui_menu_commands();
	}

	ui_menu_hide_flag = ui.combo_selected_handle == null && !ui_menu_keep_open && !ui_menu_show_first &&
	                    (ui.changed || ui.input_released || ui.input_released_r || ui.is_escape_down);
	ui_menu_keep_open = false;

	ui.ops.theme.FILL_BUTTON_BG = _FILL_BUTTON_BG;
	ui.ops.theme.ELEMENT_OFFSET = _ELEMENT_OFFSET;
	ui.ops.theme.ELEMENT_H      = _ELEMENT_H;
	ui_menu_end();
	ui_end_region();
	ui.input_enabled = true;
	draw_end();

	if (ui_menu_show_first) {
		ui_menu_show_first = false;
		ui_menu_keep_open  = true;
		ui_menu_h          = ui._y - ui_menu_y;
		ui_menu_x += iron_window_width() * 2;
		ui_menu_y += iron_window_height() * 2;
		ui_menu_fit_to_screen();
		ui_menu_render(); // Render at correct position now
	}

	if (ui_menu_hide_flag) {
		ui_menu_hide();
		ui_menu_show_first = true;
		ui_menu_commands   = null;
	}
}

function ui_menu_hide() {
	ui_menu_show = false;
	base_redraw_ui();
}

function ui_menu_draw(commands: () => void = null, x: i32 = -1, y: i32 = -1) {
	ui_end_input();
	if (ui_menu_show) {
		ui_menu_nested    = true;
		ui_menu_keep_open = true;
	}
	ui_menu_show     = true;
	ui_menu_commands = commands;
	ui_menu_x        = x > -1 ? x : math_floor(mouse_x + 1);
	ui_menu_y        = y > -1 ? y : math_floor(mouse_y + 1);
	ui_menu_h        = 0;
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

function ui_menu_separator() {
	ui._y++;
	if (config_raw.touch_ui) {
		ui_fill(0, 0, ui._w / UI_SCALE(), 1, ui.ops.theme.BUTTON_COL);
	}
	else {
		ui_fill(26, 0, ui._w / UI_SCALE() - 26, 1, ui.ops.theme.BUTTON_COL);
	}
}

function ui_menu_button(text: string, label: string = "", icon: icon_t = icon_t.NONE): bool {
	if (config_raw.touch_ui) {
		label = "";
	}
	let _y_top: i32  = ui._y;
	let result: bool = ui_button(config_button_spacing + text, config_button_align, label);

	if (icon != icon_t.NONE) {
		let _y_bottom: i32 = ui._y;
		let icons: gpu_texture_t = resource_get("icons05x.k");
		let folder: rect_t       = resource_tile50(icons, icon);
		let icon_h: i32          = 25 * UI_SCALE();
		ui._y                    = _y_top - 1;
		ui._x -= 5 * UI_SCALE();
		ui_sub_image(icons, ui.ops.theme.LABEL_COL - 0x00222222, icon_h, folder.x / 2, folder.y / 2, folder.w / 2, folder.h / 2);
		ui._x += 5 * UI_SCALE();
		ui._y = _y_bottom;
	}
	return result;
}

function ui_menu_sub_button(handle: ui_handle_t, text: string): bool {
	ui.is_hovered = false;
	ui_menu_button(text, ">");
	if (ui.is_hovered) {
		ui_menu_sub_handle = handle;
	}
	else if (math_abs(ui.input_dy) > ui.input_dx && ui.input_x < ui._x + ui._w) {
		ui_menu_sub_handle = null;
	}
	return ui_menu_sub_handle == handle;
}

function ui_menu_label(text: string, shortcut: string = null) {
	let _y: i32           = ui._y;
	let _TEXT_COL: i32    = ui.ops.theme.TEXT_COL;
	ui.ops.theme.TEXT_COL = ui.ops.theme.LABEL_COL;
	ui_text(text);
	if (shortcut != null) {
		ui._y = _y;
		ui_text(shortcut, ui_align_t.RIGHT);
	}
	ui.ops.theme.TEXT_COL = _TEXT_COL;
}

function ui_menu_align() {
	if (!config_raw.touch_ui) {
		let row: f32[] = [ 12 / 100, 88 / 100 ];
		ui_row(row);
		ui_end_element();
	}
}

function ui_menu_begin() {
	ui_draw_shadow(ui._x, ui._y, ui._w, ui_menu_h);

	draw_set_color(ui.ops.theme.SEPARATOR_COL);
	ui_draw_rect(true, ui._x, ui._y, ui._w, ui_menu_h);
	draw_set_color(0xffffffff);
}

function ui_menu_end() {}

function ui_menu_sub_begin(items: i32) {
	ui_menu_sub_x = ui._x;
	ui_menu_sub_y = ui._y;
	ui._x += ui._w + 2;
	ui._y -= UI_ELEMENT_H();

	ui_draw_shadow(ui._x, ui._y, ui._w, UI_ELEMENT_H() * items);
	draw_set_color(ui.ops.theme.SEPARATOR_COL);
	ui_draw_rect(true, ui._x, ui._y, ui._w, UI_ELEMENT_H() * items);
	draw_set_color(0xffffffff);
}

function ui_menu_sub_end() {
	ui._x = ui_menu_sub_x;
	ui._y = ui_menu_sub_y;
}
