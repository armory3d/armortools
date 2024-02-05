
class UIBox {

	static show = false;
	static draggable = true;
	static hwnd = Handle.create();
	static boxTitle = "";
	static boxText = "";
	static boxCommands: (ui: ZuiRaw)=>void = null;
	static clickToHide = true;
	static modalW = 400;
	static modalH = 170;
	static modalOnHide: ()=>void = null;
	static draws = 0;
	static copyable = false;
	///if (krom_android || krom_ios)
	static tweenAlpha = 0.0;
	///end

	static render = (g: g2_t) => {
		if (!UIMenu.show) {
			let ui = Base.uiBox;
			let inUse = ui.comboSelectedHandle_ptr != null;
			let isEscape = keyboard_started("escape");
			if (UIBox.draws > 2 && (ui.inputReleased || isEscape) && !inUse && !ui.isTyping) {
				let appw = sys_width();
				let apph = sys_height();
				let mw = Math.floor(UIBox.modalW * Zui.SCALE(ui));
				let mh = Math.floor(UIBox.modalH * Zui.SCALE(ui));
				let left = (appw / 2 - mw / 2) + UIBox.hwnd.dragX;
				let right = (appw / 2 + mw / 2) + UIBox.hwnd.dragX;
				let top = (apph / 2 - mh / 2) + UIBox.hwnd.dragY;
				let bottom = (apph / 2 + mh / 2) + UIBox.hwnd.dragY;
				let mx = mouse_x;
				let my = mouse_y;
				if ((UIBox.clickToHide && (mx < left || mx > right || my < top || my > bottom)) || isEscape) {
					UIBox.hide();
				}
			}
		}

		if (Config.raw.touch_ui) { // Darken bg
			///if (krom_android || krom_ios)
			g.color = color_from_floats(0, 0, 0, UIBox.tweenAlpha);
			///else
			g.color = color_from_floats(0, 0, 0, 0.5);
			///end
			g2_fill_rect(0, 0, sys_width(), sys_height());
		}

		g2_end(g);

		let ui = Base.uiBox;
		let appw = sys_width();
		let apph = sys_height();
		let mw = Math.floor(UIBox.modalW * Zui.SCALE(ui));
		let mh = Math.floor(UIBox.modalH * Zui.SCALE(ui));
		if (mw > appw) mw = appw;
		if (mh > apph) mh = apph;
		let left = Math.floor(appw / 2 - mw / 2);
		let top = Math.floor(apph / 2 - mh / 2);

		if (UIBox.boxCommands == null) {
			Zui.begin(ui, g);
			if (Zui.window(ui, UIBox.hwnd, left, top, mw, mh, UIBox.draggable)) {
				ui._y += 10;
				let tabVertical = Config.raw.touch_ui;
				if (Zui.tab(Zui.handle("uibox_0"), UIBox.boxTitle, tabVertical)) {
					let htext = Zui.handle("uibox_1");
					htext.text = UIBox.boxText;
					UIBox.copyable ?
						Zui.textArea(htext, Align.Left, false) :
						Zui.text(UIBox.boxText);
					Zui.endElement();

					///if (krom_windows || krom_linux || krom_darwin)
					if (UIBox.copyable) Zui.row([1 / 3, 1 / 3, 1 / 3]);
					else Zui.row([2 / 3, 1 / 3]);
					///else
					Zui.row([2 / 3, 1 / 3]);
					///end

					Zui.endElement();

					///if (krom_windows || krom_linux || krom_darwin)
					if (UIBox.copyable && Zui.button(tr("Copy"))) {
						Krom.copyToClipboard(UIBox.boxText);
					}
					///end
					if (Zui.button(tr("OK"))) {
						UIBox.hide();
					}
				}
				UIBox.windowBorder(ui);
			}
			Zui.end();
		}
		else {
			Zui.begin(ui, g);
			if (Zui.window(ui, UIBox.hwnd, left, top, mw, mh, UIBox.draggable)) {
				ui._y += 10;
				UIBox.boxCommands(ui);
				UIBox.windowBorder(ui);
			}
			Zui.end();
		}

		g2_begin(g, false);

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

	static showCustom = (commands: (ui: ZuiRaw)=>void = null, mw = 400, mh = 200, onHide: ()=>void = null, draggable = true) => {
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
		tween_to({target: UIBox, props: { tweenAlpha: 0.5 }, duration: 0.2, ease: Ease.ExpoOut});
		UIBox.hwnd.dragY = Math.floor(sys_height() / 2);
		tween_to({target: UIBox.hwnd, props: { dragY: 0 }, duration: 0.2, ease: Ease.ExpoOut, tick: () => { Base.redrawUI(); }});
	}

	static tweenOut = () => {
		tween_to({target: UIBox, props: { tweenAlpha: 0.0 }, duration: 0.2, ease: Ease.ExpoIn, done: UIBox.hideInternal});
		tween_to({target: UIBox.hwnd, props: { dragY: sys_height() / 2 }, duration: 0.2, ease: Ease.ExpoIn});
	}
	///end

	static init = () => {
		UIBox.hwnd.redraws = 2;
		UIBox.hwnd.dragX = 0;
		UIBox.hwnd.dragY = 0;
		UIBox.show = true;
		UIBox.draws = 0;
		UIBox.clickToHide = true;
	}

	static windowBorder = (ui: ZuiRaw) => {
		if (ui.scissor) {
			ui.scissor = false;
			g2_disable_scissor(ui.g);
		}
		// Border
		ui.g.color = ui.t.SEPARATOR_COL;
		g2_fill_rect(0, 0, 1, ui._windowH);
		g2_fill_rect(0 + ui._windowW - 1, 0, 1, ui._windowH);
		g2_fill_rect(0, 0 + ui._windowH - 1, ui._windowW, 1);
	}
}
