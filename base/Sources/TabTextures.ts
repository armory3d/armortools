
class TabTextures {

	static draw = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		let statush = Config.raw.layout[LayoutSize.LayoutStatusH];
		if (zui_tab(htab, tr("Textures")) && statush > UIStatus.defaultStatusH * zui_SCALE(ui)) {

			zui_begin_sticky();

			if (Config.raw.touch_ui) {
				zui_row([1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14]);
			}

			if (zui_button(tr("Import"))) {
				UIFiles.show(Path.textureFormats.join(","), false, true, (path: string) => {
					ImportAsset.run(path, -1.0, -1.0, true, false);
					UIBase.hwnds[TabArea.TabStatus].redraws = 2;
				});
			}
			if (ui.is_hovered) zui_tooltip(tr("Import texture file") + ` (${Config.keymap.file_import_assets})`);

			if (zui_button(tr("2D View"))) UIBase.show2DView(View2DType.View2DAsset);

			zui_end_sticky();

			if (Project.assets.length > 0) {

				///if (is_paint || is_sculpt)
				let statusw = sys_width() - UIToolbar.toolbarw - Config.raw.layout[LayoutSize.LayoutSidebarW];
				///end
				///if is_lab
				let statusw = sys_width();
				///end

				let slotw = Math.floor(52 * zui_SCALE(ui));
				let num = Math.floor(statusw / slotw);

				for (let row = 0; row < Math.floor(Math.ceil(Project.assets.length / num)); ++row) {
					let mult = Config.raw.show_asset_names ? 2 : 1;
					let ar = [];
					for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
					zui_row(ar);

					ui._x += 2;
					let off = Config.raw.show_asset_names ? zui_ELEMENT_OFFSET(ui) * 10.0 : 6;
					if (row > 0) ui._y += off;

					for (let j = 0; j < num; ++j) {
						let imgw = Math.floor(50 * zui_SCALE(ui));
						let i = j + row * num;
						if (i >= Project.assets.length) {
							zui_end_element(imgw);
							if (Config.raw.show_asset_names) zui_end_element(0);
							continue;
						}

						let asset = Project.assets[i];
						let img = Project.getImage(asset);
						let uix = ui._x;
						let uiy = ui._y;
						let sw = img.height < img.width ? img.height : 0;
						if (zui_image(img, 0xffffffff, slotw, 0, 0, sw, sw) == State.Started && ui.input_y > ui._window_y) {
							Base.dragOffX = -(mouse_x - uix - ui._window_x - 3);
							Base.dragOffY = -(mouse_y - uiy - ui._window_y + 1);
							Base.dragAsset = asset;
							Context.raw.texture = asset;

							if (time_time() - Context.raw.selectTime < 0.25) UIBase.show2DView(View2DType.View2DAsset);
							Context.raw.selectTime = time_time();
							UIView2D.hwnd.redraws = 2;
						}

						if (asset == Context.raw.texture) {
							let _uix = ui._x;
							let _uiy = ui._y;
							ui._x = uix;
							ui._y = uiy;
							let off = i % 2 == 1 ? 1 : 0;
							let w = 50;
							zui_fill(0,               0, w + 3,       2, ui.t.HIGHLIGHT_COL);
							zui_fill(0,     w - off + 2, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
							zui_fill(0,               0,     2,   w + 3, ui.t.HIGHLIGHT_COL);
							zui_fill(w + 2,           0,     2,   w + 4, ui.t.HIGHLIGHT_COL);
							ui._x = _uix;
							ui._y = _uiy;
						}

						let isPacked = Project.raw.packed_assets != null && Project.packedAssetExists(Project.raw.packed_assets, asset.file);

						if (ui.is_hovered) {
							zui_tooltip_image(img, 256);
							zui_tooltip(asset.name + (isPacked ? " " + tr("(packed)") : ""));
						}

						if (ui.is_hovered && ui.input_released_r) {
							Context.raw.texture = asset;

							let count = 0;

							///if (is_paint || is_sculpt)
							count = isPacked ? 6 : 8;
							///end
							///if is_lab
							count = isPacked ? 6 : 6;
							///end

							UIMenu.draw((ui: zui_t) => {
								if (UIMenu.menuButton(ui, tr("Export"))) {
									UIFiles.show("png", true, false, (path: string) => {
										Base.notifyOnNextFrame(() => {
											///if (is_paint || is_sculpt)
											if (Base.pipeMerge == null) Base.makePipe();
											///end
											///if is_lab
											if (Base.pipeCopy == null) Base.makePipe();
											///end

											let target = image_create_render_target(TabTextures.to_pow2(img.width), TabTextures.to_pow2(img.height));
											g2_begin(target, false);
											g2_set_pipeline(Base.pipeCopy);
											g2_draw_scaled_image(img, 0, 0, target.width, target.height);
											g2_set_pipeline(null);
											g2_end();
											Base.notifyOnNextFrame(() => {
												let f = UIFiles.filename;
												if (f == "") f = tr("untitled");
												if (!f.endsWith(".png")) f += ".png";
												krom_write_png(path + Path.sep + f, image_get_pixels(target), target.width, target.height, 0);
												image_unload(target);
											});
										});
									});
								}
								if (UIMenu.menuButton(ui, tr("Reimport"))) {
									Project.reimportTexture(asset);
								}

								///if (is_paint || is_sculpt)
								if (UIMenu.menuButton(ui, tr("To Mask"))) {
									Base.notifyOnNextFrame(() => {
										Base.createImageMask(asset);
									});
								}
								///end

								if (UIMenu.menuButton(ui, tr("Set as Envmap"))) {
									Base.notifyOnNextFrame(() => {
										ImportEnvmap.run(asset.file, img);
									});
								}

								///if is_paint
								if (UIMenu.menuButton(ui, tr("Set as Color ID Map"))) {
									Context.raw.colorIdHandle.position = i;
									Context.raw.colorIdPicked = false;
									UIToolbar.toolbarHandle.redraws = 1;
									if (Context.raw.tool == WorkspaceTool.ToolColorId) {
										UIHeader.headerHandle.redraws = 2;
										Context.raw.ddirty = 2;
									}
								}
								///end

								if (UIMenu.menuButton(ui, tr("Delete"), "delete")) {
									TabTextures.deleteTexture(asset);
								}
								if (!isPacked && UIMenu.menuButton(ui, tr("Open Containing Directory..."))) {
									File.start(asset.file.substr(0, asset.file.lastIndexOf(Path.sep)));
								}
								if (!isPacked && UIMenu.menuButton(ui, tr("Open in Browser"))) {
									TabBrowser.showDirectory(asset.file.substr(0, asset.file.lastIndexOf(Path.sep)));
								}
							}, count);
						}

						if (Config.raw.show_asset_names) {
							ui._x = uix;
							ui._y += slotw * 0.9;
							zui_text(Project.assets[i].name, Align.Center);
							if (ui.is_hovered) zui_tooltip(Project.assets[i].name);
							ui._y -= slotw * 0.9;
							if (i == Project.assets.length - 1) {
								ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
							}
						}
					}
				}
			}
			else {
				let img = Res.get("icons.k");
				let r = Res.tile50(img, 0, 1);
				zui_image(img, ui.t.BUTTON_COL, r.h, r.x, r.y, r.w, r.h);
				if (ui.is_hovered) zui_tooltip(tr("Drag and drop files here"));
			}

			let inFocus = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (inFocus && ui.is_delete_down && Project.assets.length > 0 && Project.assets.indexOf(Context.raw.texture) >= 0) {
				ui.is_delete_down = false;
				TabTextures.deleteTexture(Context.raw.texture);
			}
		}
	}

	static to_pow2 = (i: i32): i32 => {
		i--;
		i |= i >> 1;
		i |= i >> 2;
		i |= i >> 4;
		i |= i >> 8;
		i |= i >> 16;
		i++;
		return i;
	}

	static updateTexturePointers = (nodes: zui_node_t[], i: i32) => {
		for (let n of nodes) {
			if (n.type == "TEX_IMAGE") {
				if (n.buttons[0].default_value == i) {
					n.buttons[0].default_value = 9999; // Texture deleted, use pink now
				}
				else if (n.buttons[0].default_value > i) {
					n.buttons[0].default_value--; // Offset by deleted texture
				}
			}
		}
	}

	static deleteTexture = (asset: TAsset) => {
		let i = Project.assets.indexOf(asset);
		if (Project.assets.length > 1) {
			Context.raw.texture = Project.assets[i == Project.assets.length - 1 ? i - 1 : i + 1];
		}
		UIBase.hwnds[TabArea.TabStatus].redraws = 2;

		///if is_paint
		if (Context.raw.tool == WorkspaceTool.ToolColorId && i == Context.raw.colorIdHandle.position) {
			UIHeader.headerHandle.redraws = 2;
			Context.raw.ddirty = 2;
			Context.raw.colorIdPicked = false;
			UIToolbar.toolbarHandle.redraws = 1;
		}
		///end

		data_delete_image(asset.file);
		Project.assetMap.delete(asset.id);
		Project.assets.splice(i, 1);
		Project.assetNames.splice(i, 1);
		let _next = () => {
			MakeMaterial.parsePaintMaterial();

			///if (is_paint || is_sculpt)
			UtilRender.makeMaterialPreview();
			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			///end
		}
		Base.notifyOnNextFrame(_next);

		for (let m of Project.materials) TabTextures.updateTexturePointers(m.canvas.nodes, i);
		///if (is_paint || is_sculpt)
		for (let b of Project.brushes) TabTextures.updateTexturePointers(b.canvas.nodes, i);
		///end
	}
}
