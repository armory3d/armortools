
class UIBox {

	static show: bool = false;
	static draggable: bool = true;
	static hwnd: zui_handle_t = zui_handle_create();
	static box_title: string = "";
	static box_text: string = "";
	static box_commands: (ui: zui_t)=>void = null;
	static click_to_hide: bool = true;
	static modalw: i32 = 400;
	static modalh: i32 = 170;
	static modal_on_hide: ()=>void = null;
	static draws: i32 = 0;
	static copyable: bool = false;
	///if (krom_android || krom_ios)
	static tween_alpha: f32 = 0.0;
	///end

	static render = () => {
		if (!UIMenu.show) {
			let ui: zui_t = base_ui_box;
			let in_use: bool = ui.combo_selected_handle_ptr != 0;
			let is_escape: bool = keyboard_started("escape");
			if (UIBox.draws > 2 && (ui.input_released || is_escape) && !in_use && !ui.is_typing) {
				let appw: i32 = sys_width();
				let apph: i32 = sys_height();
				let mw: i32 = Math.floor(UIBox.modalw * zui_SCALE(ui));
				let mh: i32 = Math.floor(UIBox.modalh * zui_SCALE(ui));
				let left: f32 = (appw / 2 - mw / 2) + UIBox.hwnd.drag_x;
				let right: f32 = (appw / 2 + mw / 2) + UIBox.hwnd.drag_x;
				let top: f32 = (apph / 2 - mh / 2) + UIBox.hwnd.drag_y;
				let bottom: f32 = (apph / 2 + mh / 2) + UIBox.hwnd.drag_y;
				let mx: i32 = mouse_x;
				let my: i32 = mouse_y;
				if ((UIBox.click_to_hide && (mx < left || mx > right || my < top || my > bottom)) || is_escape) {
					UIBox.hide();
				}
			}
		}

		if (Config.raw.touch_ui) { // Darken bg
			///if (krom_android || krom_ios)
			g2_set_color(color_from_floats(0, 0, 0, UIBox.tween_alpha));
			///else
			g2_set_color(color_from_floats(0, 0, 0, 0.5));
			///end
			g2_fill_rect(0, 0, sys_width(), sys_height());
		}

		g2_end();

		let ui: zui_t = base_ui_box;
		let appw: i32 = sys_width();
		let apph: i32 = sys_height();
		let mw: i32 = Math.floor(UIBox.modalw * zui_SCALE(ui));
		let mh: i32 = Math.floor(UIBox.modalh * zui_SCALE(ui));
		if (mw > appw) mw = appw;
		if (mh > apph) mh = apph;
		let left: i32 = Math.floor(appw / 2 - mw / 2);
		let top: i32 = Math.floor(apph / 2 - mh / 2);

		if (UIBox.box_commands == null) {
			zui_begin(ui);
			if (zui_window(UIBox.hwnd, left, top, mw, mh, UIBox.draggable)) {
				ui._y += 10;
				let tab_vertical: bool = Config.raw.touch_ui;
				if (zui_tab(zui_handle("uibox_0"), UIBox.box_title, tab_vertical)) {
					let htext: zui_handle_t = zui_handle("uibox_1");
					htext.text = UIBox.box_text;
					UIBox.copyable ?
						zui_text_area(htext, zui_align_t.LEFT, false) :
						zui_text(UIBox.box_text);
					zui_end_element();

					///if (krom_windows || krom_linux || krom_darwin)
					if (UIBox.copyable) zui_row([1 / 3, 1 / 3, 1 / 3]);
					else zui_row([2 / 3, 1 / 3]);
					///else
					zui_row([2 / 3, 1 / 3]);
					///end

					zui_end_element();

					///if (krom_windows || krom_linux || krom_darwin)
					if (UIBox.copyable && zui_button(tr("Copy"))) {
						krom_copy_to_clipboard(UIBox.box_text);
					}
					///end
					if (zui_button(tr("OK"))) {
						UIBox.hide();
					}
				}
				UIBox.window_border(ui);
			}
			zui_end();
		}
		else {
			zui_begin(ui);
			if (zui_window(UIBox.hwnd, left, top, mw, mh, UIBox.draggable)) {
				ui._y += 10;
				UIBox.box_commands(ui);
				UIBox.window_border(ui);
			}
			zui_end();
		}

		g2_begin(null);

		UIBox.draws++;
	}

	static show_message = (title: string, text: string, copyable: bool = false) => {
		UIBox.init();
		UIBox.modalw = 400;
		UIBox.modalh = 210;
		UIBox.box_title = title;
		UIBox.box_text = text;
		UIBox.box_commands = null;
		UIBox.copyable = copyable;
		UIBox.draggable = true;
		///if (krom_android || krom_ios)
		UIBox.tween_in();
		///end
	}

	static show_custom = (commands: (ui: zui_t)=>void = null, mw: i32 = 400, mh: i32 = 200, onHide: ()=>void = null, draggable: bool = true) => {
		UIBox.init();
		UIBox.modalw = mw;
		UIBox.modalh = mh;
		UIBox.modal_on_hide = onHide;
		UIBox.box_commands = commands;
		UIBox.draggable = draggable;
		///if (krom_android || krom_ios)
		UIBox.tween_in();
		///end
	}

	static hide = () => {
		///if (krom_android || krom_ios)
		UIBox.tween_out();
		///else
		UIBox.hide_internal();
		///end
	}

	static hide_internal = () => {
		if (UIBox.modal_on_hide != null) UIBox.modal_on_hide();
		UIBox.show = false;
		base_redraw_ui();
	}

	///if (krom_android || krom_ios)
	static tween_in = () => {
		tween_reset();
		tween_to({target: UIBox, props: { tweenAlpha: 0.5 }, duration: 0.2, ease: ease_t.EXPO_OUT});
		UIBox.hwnd.drag_y = Math.floor(sys_height() / 2);
		tween_to({target: UIBox.hwnd, props: { dragY: 0 }, duration: 0.2, ease: ease_t.EXPO_OUT, tick: () => { base_redraw_ui(); }});
	}

	static tween_out = () => {
		tween_to({target: UIBox, props: { tweenAlpha: 0.0 }, duration: 0.2, ease: ease_t.EXPO_IN, done: UIBox.hide_internal});
		tween_to({target: UIBox.hwnd, props: { dragY: sys_height() / 2 }, duration: 0.2, ease: ease_t.EXPO_IN});
	}
	///end

	static init = () => {
		UIBox.hwnd.redraws = 2;
		UIBox.hwnd.drag_x = 0;
		UIBox.hwnd.drag_y = 0;
		UIBox.show = true;
		UIBox.draws = 0;
		UIBox.click_to_hide = true;
	}

	static window_border = (ui: zui_t) => {
		if (ui.scissor) {
			ui.scissor = false;
			g2_disable_scissor();
		}
		// Border
		g2_set_color(ui.t.SEPARATOR_COL);
		g2_fill_rect(0, 0, 1, ui._window_h);
		g2_fill_rect(0 + ui._window_w - 1, 0, 1, ui._window_h);
		g2_fill_rect(0, 0 + ui._window_h - 1, ui._window_w, 1);
	}
}
