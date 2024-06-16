
let ui_box_show: bool = false;
let ui_box_draggable: bool = true;
let ui_box_hwnd: zui_handle_t = zui_handle_create();
let ui_box_title: string = "";
let ui_box_text: string = "";
let ui_box_commands: (ui: zui_t)=>void = null;
let ui_box_click_to_hide: bool = true;
let ui_box_modalw: i32 = 400;
let ui_box_modalh: i32 = 170;
let ui_box_modal_on_hide: ()=>void = null;
let ui_box_draws: i32 = 0;
let ui_box_copyable: bool = false;
///if (krom_android || krom_ios)
let ui_box_tween_alpha: f32 = 0.0;
///end

function ui_box_init() {
	ui_box_hwnd.redraws = 2;
	ui_box_hwnd.drag_x = 0;
	ui_box_hwnd.drag_y = 0;
	ui_box_show = true;
	ui_box_draws = 0;
	ui_box_click_to_hide = true;
}

function ui_box_render() {
	if (!ui_menu_show) {
		let ui: zui_t = base_ui_box;
		let in_use: bool = ui.combo_selected_handle != null;
		let is_escape: bool = keyboard_started("escape");
		if (ui_box_draws > 2 && (ui.input_released || is_escape) && !in_use && !ui.is_typing) {
			let appw: i32 = sys_width();
			let apph: i32 = sys_height();
			let mw: i32 = math_floor(ui_box_modalw * zui_SCALE(ui));
			let mh: i32 = math_floor(ui_box_modalh * zui_SCALE(ui));
			let left: f32 = (appw / 2 - mw / 2) + ui_box_hwnd.drag_x;
			let right: f32 = (appw / 2 + mw / 2) + ui_box_hwnd.drag_x;
			let top: f32 = (apph / 2 - mh / 2) + ui_box_hwnd.drag_y;
			let bottom: f32 = (apph / 2 + mh / 2) + ui_box_hwnd.drag_y;
			let mx: i32 = mouse_x;
			let my: i32 = mouse_y;
			if ((ui_box_click_to_hide && (mx < left || mx > right || my < top || my > bottom)) || is_escape) {
				ui_box_hide();
			}
		}
	}

	if (config_raw.touch_ui) { // Darken bg
		///if (krom_android || krom_ios)
		g2_set_color(color_from_floats(0, 0, 0, ui_box_tween_alpha));
		///else
		g2_set_color(color_from_floats(0, 0, 0, 0.5));
		///end
		g2_fill_rect(0, 0, sys_width(), sys_height());
	}

	g2_end();

	let ui: zui_t = base_ui_box;
	let appw: i32 = sys_width();
	let apph: i32 = sys_height();
	let mw: i32 = math_floor(ui_box_modalw * zui_SCALE(ui));
	let mh: i32 = math_floor(ui_box_modalh * zui_SCALE(ui));
	if (mw > appw) {
		mw = appw;
	}
	if (mh > apph) {
		mh = apph;
	}
	let left: i32 = math_floor(appw / 2 - mw / 2);
	let top: i32 = math_floor(apph / 2 - mh / 2);

	if (ui_box_commands == null) {
		zui_begin(ui);
		if (zui_window(ui_box_hwnd, left, top, mw, mh, ui_box_draggable)) {
			ui._y += 10;
			let tab_vertical: bool = config_raw.touch_ui;
			if (zui_tab(zui_handle(__ID__), ui_box_title, tab_vertical)) {
				let htext: zui_handle_t = zui_handle(__ID__);
				htext.text = ui_box_text;
				if (ui_box_copyable) {
					zui_text_area(htext, zui_align_t.LEFT, false);
				}
				else {
					zui_text(ui_box_text);
				}
				_zui_end_element();

				///if (krom_windows || krom_linux || krom_macos)
				if (ui_box_copyable) {
					let row: f32[] = [1 / 3, 1 / 3, 1 / 3];
					zui_row(row);
				}
				else {
					let row: f32[] = [2 / 3, 1 / 3];
					zui_row(row);
				}
				///else
				let row: f32[] = [2 / 3, 1 / 3];
				zui_row(row);
				///end

				_zui_end_element();

				///if (krom_windows || krom_linux || krom_macos)
				if (ui_box_copyable && zui_button(tr("Copy"))) {
					krom_copy_to_clipboard(ui_box_text);
				}
				///end
				if (zui_button(tr("OK"))) {
					ui_box_hide();
				}
			}
			ui_box_window_border(ui);
		}
		zui_end();
	}
	else {
		zui_begin(ui);
		if (zui_window(ui_box_hwnd, left, top, mw, mh, ui_box_draggable)) {
			ui._y += 10;
			ui_box_commands(ui);
			ui_box_window_border(ui);
		}
		zui_end();
	}

	g2_begin(null);

	ui_box_draws++;
}

function ui_box_show_message(title: string, text: string, copyable: bool = false) {
	ui_box_init();
	ui_box_modalw = 400;
	ui_box_modalh = 210;
	ui_box_title = title;
	ui_box_text = text;
	ui_box_commands = null;
	ui_box_copyable = copyable;
	ui_box_draggable = true;
	///if (krom_android || krom_ios)
	ui_box_tween_in();
	///end
}

function ui_box_show_custom(commands: (ui: zui_t)=>void = null, mw: i32 = 400, mh: i32 = 200, on_hide: ()=>void = null, draggable: bool = true) {
	ui_box_init();
	ui_box_modalw = mw;
	ui_box_modalh = mh;
	ui_box_modal_on_hide = on_hide;
	ui_box_commands = commands;
	ui_box_draggable = draggable;
	///if (krom_android || krom_ios)
	ui_box_tween_in();
	///end
}

function ui_box_hide() {
	///if (krom_android || krom_ios)
	ui_box_tween_out();
	///else
	ui_box_hide_internal();
	///end
}

function ui_box_hide_internal() {
	if (ui_box_modal_on_hide != null) {
		ui_box_modal_on_hide();
	}
	ui_box_show = false;
	base_redraw_ui();
}

///if (krom_android || krom_ios)
function ui_box_tween_in() {
	// tween_reset();
	// tween_to({target: UIBox, props: { ui_box_tween_alpha: 0.5 }, duration: 0.2, ease: ease_t.EXPO_OUT});
	// ui_box_hwnd.drag_y = math_floor(sys_height() / 2);
	// tween_to({target: ui_box_hwnd, props: { dragY: 0 }, duration: 0.2, ease: ease_t.EXPO_OUT, tick: function () { base_redraw_ui(); }});
}

function ui_box_tween_out() {
	// tween_to({target: UIBox, props: { ui_box_tween_alpha: 0.0 }, duration: 0.2, ease: ease_t.EXPO_IN, done: ui_box_hide_internal});
	// tween_to({target: ui_box_hwnd, props: { dragY: sys_height() / 2 }, duration: 0.2, ease: ease_t.EXPO_IN});
}
///end

function ui_box_window_border(ui: zui_t) {
	if (ui.scissor) {
		ui.scissor = false;
		g2_disable_scissor();
	}
	// Border
	g2_set_color(ui.ops.theme.SEPARATOR_COL);
	g2_fill_rect(0, 0, 1, ui._window_h);
	g2_fill_rect(0 + ui._window_w - 1, 0, 1, ui._window_h);
	g2_fill_rect(0, 0 + ui._window_h - 1, ui._window_w, 1);
}
