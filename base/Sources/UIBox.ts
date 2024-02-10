
class UIBox {

	static show = false;
	static draggable = true;
	static hwnd = zui_handle_create();
	static boxTitle = "";
	static boxText = "";
	static boxCommands: (ui: zui_t)=>void = null;
	static clickToHide = true;
	static modalW = 400;
	static modalH = 170;
	static modalOnHide: ()=>void = null;
	static draws = 0;
	static copyable = false;
	///if (krom_android || krom_ios)
	static tweenAlpha = 0.0;
	///end

	static render = () => {
		if (!UIMenu.show) {
			let ui = Base.uiBox;
			let inUse = ui.combo_selected_handle_ptr != null;
			let isEscape = keyboard_started("escape");
			if (UIBox.draws > 2 && (ui.input_released || isEscape) && !inUse && !ui.is_typing) {
				let appw = sys_width();
				let apph = sys_height();
				let mw = Math.floor(UIBox.modalW * zui_SCALE(ui));
				let mh = Math.floor(UIBox.modalH * zui_SCALE(ui));
				let left = (appw / 2 - mw / 2) + UIBox.hwnd.drag_x;
				let right = (appw / 2 + mw / 2) + UIBox.hwnd.drag_x;
				let top = (apph / 2 - mh / 2) + UIBox.hwnd.drag_y;
				let bottom = (apph / 2 + mh / 2) + UIBox.hwnd.drag_y;
				let mx = mouse_x;
				let my = mouse_y;
				if ((UIBox.clickToHide && (mx < left || mx > right || my < top || my > bottom)) || isEscape) {
					UIBox.hide();
				}
			}
		}

		if (Config.raw.touch_ui) { // Darken bg
			///if (krom_android || krom_ios)
			g2_set_color(color_from_floats(0, 0, 0, UIBox.tweenAlpha));
			///else
			g2_set_color(color_from_floats(0, 0, 0, 0.5));
			///end
			g2_fill_rect(0, 0, sys_width(), sys_height());
		}

		g2_end();

		let ui = Base.uiBox;
		let appw = sys_width();
		let apph = sys_height();
		let mw = Math.floor(UIBox.modalW * zui_SCALE(ui));
		let mh = Math.floor(UIBox.modalH * zui_SCALE(ui));
		if (mw > appw) mw = appw;
		if (mh > apph) mh = apph;
		let left = Math.floor(appw / 2 - mw / 2);
		let top = Math.floor(apph / 2 - mh / 2);

		if (UIBox.boxCommands == null) {
			zui_begin(ui);
			if (zui_window(UIBox.hwnd, left, top, mw, mh, UIBox.draggable)) {
				ui._y += 10;
				let tabVertical = Config.raw.touch_ui;
				if (zui_tab(zui_handle("uibox_0"), UIBox.boxTitle, tabVertical)) {
					let htext = zui_handle("uibox_1");
					htext.text = UIBox.boxText;
					UIBox.copyable ?
						zui_text_area(htext, Align.Left, false) :
						zui_text(UIBox.boxText);
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
						krom_copy_to_clipboard(UIBox.boxText);
					}
					///end
					if (zui_button(tr("OK"))) {
						UIBox.hide();
					}
				}
				UIBox.windowBorder(ui);
			}
			zui_end();
		}
		else {
			zui_begin(ui);
			if (zui_window(UIBox.hwnd, left, top, mw, mh, UIBox.draggable)) {
				ui._y += 10;
				UIBox.boxCommands(ui);
				UIBox.windowBorder(ui);
			}
			zui_end();
		}

		g2_begin(null, false);

		UIBox.draws++;
	}

	static showMessage = (title: string, text: string, copyable = false) => {
		UIBox.init();
		UIBox.modalW = 400;
		UIBox.modalH = 210;
		UIBox.boxTitle = title;
		UIBox.boxText = text;
		UIBox.boxCommands = null;
		UIBox.copyable = copyable;
		UIBox.draggable = true;
		///if (krom_android || krom_ios)
		UIBox.tweenIn();
		///end
	}

	static showCustom = (commands: (ui: zui_t)=>void = null, mw = 400, mh = 200, onHide: ()=>void = null, draggable = true) => {
		UIBox.init();
		UIBox.modalW = mw;
		UIBox.modalH = mh;
		UIBox.modalOnHide = onHide;
		UIBox.boxCommands = commands;
		UIBox.draggable = draggable;
		///if (krom_android || krom_ios)
		UIBox.tweenIn();
		///end
	}

	static hide = () => {
		///if (krom_android || krom_ios)
		UIBox.tweenOut();
		///else
		UIBox.hideInternal();
		///end
	}

	static hideInternal = () => {
		if (UIBox.modalOnHide != null) UIBox.modalOnHide();
		UIBox.show = false;
		Base.redrawUI();
	}

	///if (krom_android || krom_ios)
	static tweenIn = () => {
		tween_reset();
		tween_to({target: UIBox, props: { tweenAlpha: 0.5 }, duration: 0.2, ease: ease_t.EXPO_OUT});
		UIBox.hwnd.drag_y = Math.floor(sys_height() / 2);
		tween_to({target: UIBox.hwnd, props: { dragY: 0 }, duration: 0.2, ease: ease_t.EXPO_OUT, tick: () => { Base.redrawUI(); }});
	}

	static tweenOut = () => {
		tween_to({target: UIBox, props: { tweenAlpha: 0.0 }, duration: 0.2, ease: ease_t.EXPO_IN, done: UIBox.hideInternal});
		tween_to({target: UIBox.hwnd, props: { dragY: sys_height() / 2 }, duration: 0.2, ease: ease_t.EXPO_IN});
	}
	///end

	static init = () => {
		UIBox.hwnd.redraws = 2;
		UIBox.hwnd.drag_x = 0;
		UIBox.hwnd.drag_y = 0;
		UIBox.show = true;
		UIBox.draws = 0;
		UIBox.clickToHide = true;
	}

	static windowBorder = (ui: zui_t) => {
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
