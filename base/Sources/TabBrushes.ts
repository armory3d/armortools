
///if (is_paint || is_sculpt)

class TabBrushes {

	static draw = (htab: Handle) => {
		let ui = UIBase.ui;
		if (ui.tab(htab, tr("Brushes"))) {
			ui.beginSticky();
			ui.row([1 / 4, 1 / 4, 1 / 4]);
			if (ui.button(tr("New"))) {
				Context.raw.brush = SlotBrush.create();
				Project.brushes.push(Context.raw.brush);
				MakeMaterial.parseBrush();
				UINodes.hwnd.redraws = 2;
			}
			if (ui.button(tr("Import"))) {
				Project.importBrush();
			}
			if (ui.button(tr("Nodes"))) {
				UIBase.showBrushNodes();
			}
			ui.endSticky();
			ui.separator(3, false);

			let slotw = Math.floor(51 * ui.SCALE());
			let num = Math.floor(Config.raw.layout[LayoutSize.LayoutSidebarW] / slotw);

			for (let row = 0; row < Math.floor(Math.ceil(Project.brushes.length / num)); ++row) {
				let mult = Config.raw.show_asset_names ? 2 : 1;
				let ar = [];
				for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
				ui.row(ar);

				ui._x += 2;
				let off = Config.raw.show_asset_names ? ui.ELEMENT_OFFSET() * 10.0 : 6;
				if (row > 0) ui._y += off;

				for (let j = 0; j < num; ++j) {
					let imgw = Math.floor(50 * ui.SCALE());
					let i = j + row * num;
					if (i >= Project.brushes.length) {
						ui.endElement(imgw);
						if (Config.raw.show_asset_names) ui.endElement(0);
						continue;
					}
					let img = ui.SCALE() > 1 ? Project.brushes[i].image : Project.brushes[i].imageIcon;
					let imgFull = Project.brushes[i].image;

					if (Context.raw.brush == Project.brushes[i]) {
						// ui.fill(1, -2, img.width + 3, img.height + 3, ui.t.HIGHLIGHT_COL); // TODO
						let off = row % 2 == 1 ? 1 : 0;
						let w = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						ui.fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						ui.fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						ui.fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}

					let uix = ui._x;
					//let uiy = ui._y;
					let tile = ui.SCALE() > 1 ? 100 : 50;
					let state = Project.brushes[i].previewReady ? ui.image(img) : ui.image(Res.get("icons.k"), -1, null, tile * 5, tile, tile, tile);
					if (state == State.Started) {
						if (Context.raw.brush != Project.brushes[i]) Context.selectBrush(i);
						if (Time.time() - Context.raw.selectTime < 0.25) UIBase.showBrushNodes();
						Context.raw.selectTime = Time.time();
						// let mouse = Input.getMouse();
						// App.dragOffX = -(mouse.x - uix - ui._windowX - 3);
						// App.dragOffY = -(mouse.y - uiy - ui._windowY + 1);
						// App.dragBrush = Context.raw.brush;
					}
					if (ui.isHovered && ui.inputReleasedR) {
						Context.selectBrush(i);
						let add = Project.brushes.length > 1 ? 1 : 0;
						UIMenu.draw((ui: Zui) => {
							//let b = Project.brushes[i];

							if (UIMenu.menuButton(ui, tr("Export"))) {
								Context.selectBrush(i);
								BoxExport.showBrush();
							}

							if (UIMenu.menuButton(ui, tr("Duplicate"))) {
								let _init = () => {
									Context.raw.brush = SlotBrush.create();
									Project.brushes.push(Context.raw.brush);
									let cloned = JSON.parse(JSON.stringify(Project.brushes[i].canvas));
									Context.raw.brush.canvas = cloned;
									Context.setBrush(Context.raw.brush);
									UtilRender.makeBrushPreview();
								}
								App.notifyOnInit(_init);
							}

							if (Project.brushes.length > 1 && UIMenu.menuButton(ui, tr("Delete"), "delete")) {
								TabBrushes.deleteBrush(Project.brushes[i]);
							}
						}, 2 + add);
					}

					if (ui.isHovered) {
						if (imgFull == null) {
							App.notifyOnInit(() => {
								let _brush = Context.raw.brush;
								Context.raw.brush = Project.brushes[i];
								MakeMaterial.parseBrush();
								UtilRender.makeBrushPreview();
								Context.raw.brush = _brush;
							});
						}
						else {
							ui.tooltipImage(imgFull);
							ui.tooltip(Project.brushes[i].canvas.name);
						}
					}

					if (Config.raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						ui.text(Project.brushes[i].canvas.name, Align.Center);
						if (ui.isHovered) ui.tooltip(Project.brushes[i].canvas.name);
						ui._y -= slotw * 0.9;
						if (i == Project.brushes.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
						}
					}
				}

				ui._y += 6;
			}

			let inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
						  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
			if (inFocus && ui.isDeleteDown && Project.brushes.length > 1) {
				ui.isDeleteDown = false;
				TabBrushes.deleteBrush(Context.raw.brush);
			}
		}
	}

	static deleteBrush = (b: SlotBrushRaw) => {
		let i = Project.brushes.indexOf(b);
		Context.selectBrush(i == Project.brushes.length - 1 ? i - 1 : i + 1);
		Project.brushes.splice(i, 1);
		UIBase.hwnds[1].redraws = 2;
	}
}

///end
