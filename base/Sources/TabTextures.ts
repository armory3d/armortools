
class TabTextures {

	static draw = (htab: zui_handle_t) => {
		let ui: zui_t = UIBase.ui;
		let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
		if (zui_tab(htab, tr("Textures")) && statush > UIStatus.default_status_h * zui_SCALE(ui)) {

			zui_begin_sticky();

			if (config_raw.touch_ui) {
				zui_row([1 / 4, 1 / 4]);
			}
			else {
				zui_row([1 / 14, 1 / 14]);
			}

			if (zui_button(tr("Import"))) {
				UIFiles.show(path_texture_formats.join(","), false, true, (path: string) => {
					ImportAsset.run(path, -1.0, -1.0, true, false);
					UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
				});
			}
			if (ui.is_hovered) zui_tooltip(tr("Import texture file") + ` (${config_keymap.file_import_assets})`);

			if (zui_button(tr("2D View"))) UIBase.show_2d_view(view_2d_type_t.ASSET);

			zui_end_sticky();

			if (project_assets.length > 0) {

				///if (is_paint || is_sculpt)
				let statusw: i32 = sys_width() - UIToolbar.toolbar_w - config_raw.layout[layout_size_t.SIDEBAR_W];
				///end
				///if is_lab
				let statusw: i32 = sys_width();
				///end

				let slotw: i32 = Math.floor(52 * zui_SCALE(ui));
				let num: i32 = Math.floor(statusw / slotw);

				for (let row: i32 = 0; row < Math.floor(Math.ceil(project_assets.length / num)); ++row) {
					let mult: i32 = config_raw.show_asset_names ? 2 : 1;
					let ar: f32[] = [];
					for (let i: i32 = 0; i < num * mult; ++i) ar.push(1 / num);
					zui_row(ar);

					ui._x += 2;
					let off: f32 = config_raw.show_asset_names ? zui_ELEMENT_OFFSET(ui) * 10.0 : 6;
					if (row > 0) ui._y += off;

					for (let j: i32 = 0; j < num; ++j) {
						let imgw: i32 = Math.floor(50 * zui_SCALE(ui));
						let i: i32 = j + row * num;
						if (i >= project_assets.length) {
							zui_end_element(imgw);
							if (config_raw.show_asset_names) zui_end_element(0);
							continue;
						}

						let asset: asset_t = project_assets[i];
						let img: image_t = project_get_image(asset);
						let uix: f32 = ui._x;
						let uiy: f32 = ui._y;
						let sw: i32 = img.height < img.width ? img.height : 0;
						if (zui_image(img, 0xffffffff, slotw, 0, 0, sw, sw) == zui_state_t.STARTED && ui.input_y > ui._window_y) {
							base_drag_off_x = -(mouse_x - uix - ui._window_x - 3);
							base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
							base_drag_asset = asset;
							context_raw.texture = asset;

							if (time_time() - context_raw.select_time < 0.25) UIBase.show_2d_view(view_2d_type_t.ASSET);
							context_raw.select_time = time_time();
							UIView2D.hwnd.redraws = 2;
						}

						if (asset == context_raw.texture) {
							let _uix: f32 = ui._x;
							let _uiy: f32 = ui._y;
							ui._x = uix;
							ui._y = uiy;
							let off: i32 = i % 2 == 1 ? 1 : 0;
							let w: i32 = 50;
							zui_fill(0,               0, w + 3,       2, ui.t.HIGHLIGHT_COL);
							zui_fill(0,     w - off + 2, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
							zui_fill(0,               0,     2,   w + 3, ui.t.HIGHLIGHT_COL);
							zui_fill(w + 2,           0,     2,   w + 4, ui.t.HIGHLIGHT_COL);
							ui._x = _uix;
							ui._y = _uiy;
						}

						let is_packed: bool = project_raw.packed_assets != null && project_packed_asset_exists(project_raw.packed_assets, asset.file);

						if (ui.is_hovered) {
							zui_tooltip_image(img, 256);
							zui_tooltip(asset.name + (is_packed ? " " + tr("(packed)") : ""));
						}

						if (ui.is_hovered && ui.input_released_r) {
							context_raw.texture = asset;

							let count: i32 = 0;

							///if (is_paint || is_sculpt)
							count = is_packed ? 6 : 8;
							///end
							///if is_lab
							count = is_packed ? 6 : 6;
							///end

							UIMenu.draw((ui: zui_t) => {
								if (UIMenu.menu_button(ui, tr("Export"))) {
									UIFiles.show("png", true, false, (path: string) => {
										base_notify_on_next_frame(() => {
											///if (is_paint || is_sculpt)
											if (base_pipe_merge == null) base_make_pipe();
											///end
											///if is_lab
											if (base_pipe_copy == null) base_make_pipe();
											///end

											let target: image_t = image_create_render_target(TabTextures.to_pow2(img.width), TabTextures.to_pow2(img.height));
											g2_begin(target);
											g2_set_pipeline(base_pipe_copy);
											g2_draw_scaled_image(img, 0, 0, target.width, target.height);
											g2_set_pipeline(null);
											g2_end();
											base_notify_on_next_frame(() => {
												let f: string = UIFiles.filename;
												if (f == "") f = tr("untitled");
												if (!f.endsWith(".png")) f += ".png";
												krom_write_png(path + path_sep + f, image_get_pixels(target), target.width, target.height, 0);
												image_unload(target);
											});
										});
									});
								}
								if (UIMenu.menu_button(ui, tr("Reimport"))) {
									project_reimport_texture(asset);
								}

								///if (is_paint || is_sculpt)
								if (UIMenu.menu_button(ui, tr("To Mask"))) {
									base_notify_on_next_frame(() => {
										base_create_image_mask(asset);
									});
								}
								///end

								if (UIMenu.menu_button(ui, tr("Set as Envmap"))) {
									base_notify_on_next_frame(() => {
										ImportEnvmap.run(asset.file, img);
									});
								}

								///if is_paint
								if (UIMenu.menu_button(ui, tr("Set as Color ID Map"))) {
									context_raw.colorid_handle.position = i;
									context_raw.colorid_picked = false;
									UIToolbar.toolbar_handle.redraws = 1;
									if (context_raw.tool == workspace_tool_t.COLORID) {
										UIHeader.header_handle.redraws = 2;
										context_raw.ddirty = 2;
									}
								}
								///end

								if (UIMenu.menu_button(ui, tr("Delete"), "delete")) {
									TabTextures.delete_texture(asset);
								}
								if (!is_packed && UIMenu.menu_button(ui, tr("Open Containing Directory..."))) {
									file_start(asset.file.substr(0, asset.file.lastIndexOf(path_sep)));
								}
								if (!is_packed && UIMenu.menu_button(ui, tr("Open in Browser"))) {
									TabBrowser.show_directory(asset.file.substr(0, asset.file.lastIndexOf(path_sep)));
								}
							}, count);
						}

						if (config_raw.show_asset_names) {
							ui._x = uix;
							ui._y += slotw * 0.9;
							zui_text(project_assets[i].name, zui_align_t.CENTER);
							if (ui.is_hovered) zui_tooltip(project_assets[i].name);
							ui._y -= slotw * 0.9;
							if (i == project_assets.length - 1) {
								ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
							}
						}
					}
				}
			}
			else {
				let img: image_t = resource_get("icons.k");
				let r: rect_t = resource_tile50(img, 0, 1);
				zui_image(img, ui.t.BUTTON_COL, r.h, r.x, r.y, r.w, r.h);
				if (ui.is_hovered) zui_tooltip(tr("Drag and drop files here"));
			}

			let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						 	 	 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (in_focus && ui.is_delete_down && project_assets.length > 0 && project_assets.indexOf(context_raw.texture) >= 0) {
				ui.is_delete_down = false;
				TabTextures.delete_texture(context_raw.texture);
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

	static update_texture_pointers = (nodes: zui_node_t[], i: i32) => {
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

	static delete_texture = (asset: asset_t) => {
		let i: i32 = project_assets.indexOf(asset);
		if (project_assets.length > 1) {
			context_raw.texture = project_assets[i == project_assets.length - 1 ? i - 1 : i + 1];
		}
		UIBase.hwnds[tab_area_t.STATUS].redraws = 2;

		///if is_paint
		if (context_raw.tool == workspace_tool_t.COLORID && i == context_raw.colorid_handle.position) {
			UIHeader.header_handle.redraws = 2;
			context_raw.ddirty = 2;
			context_raw.colorid_picked = false;
			UIToolbar.toolbar_handle.redraws = 1;
		}
		///end

		data_delete_image(asset.file);
		project_asset_map.delete(asset.id);
		project_assets.splice(i, 1);
		project_asset_names.splice(i, 1);
		let _next = () => {
			MakeMaterial.parse_paint_material();

			///if (is_paint || is_sculpt)
			UtilRender.make_material_preview();
			UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
			///end
		}
		base_notify_on_next_frame(_next);

		for (let m of project_materials) TabTextures.update_texture_pointers(m.canvas.nodes, i);
		///if (is_paint || is_sculpt)
		for (let b of project_brushes) TabTextures.update_texture_pointers(b.canvas.nodes, i);
		///end
	}
}
