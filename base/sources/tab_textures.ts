
let _tab_textures_draw_img: image_t;
let _tab_textures_draw_path: string;
let _tab_textures_draw_asset: asset_t;
let _tab_textures_draw_i: i32;
let _tab_textures_draw_is_packed: bool;

function tab_textures_draw(htab: ui_handle_t) {
	let ui: ui_t = ui_base_ui;
	let statush: i32 = config_raw.layout[layout_size_t.STATUS_H];
	if (ui_tab(htab, tr("Textures")) && statush > ui_status_default_status_h * ui_SCALE(ui)) {

		ui_begin_sticky();

		if (config_raw.touch_ui) {
			let row: f32[] = [1 / 4, 1 / 4];
			ui_row(row);
		}
		else {
			let row: f32[] = [1 / 14, 1 / 14];
			ui_row(row);
		}

		if (ui_button(tr("Import"))) {
			ui_files_show(string_array_join(path_texture_formats, ","), false, true, function (path: string) {
				import_asset_run(path, -1.0, -1.0, true, false);
				ui_base_hwnds[tab_area_t.STATUS].redraws = 2;
			});
		}
		if (ui.is_hovered) {
			ui_tooltip(tr("Import texture file") + " (" + map_get(config_keymap, "file_import_assets") + ")");
		}

		if (ui_button(tr("2D View"))) {
			ui_base_show_2d_view(view_2d_type_t.ASSET);
		}

		ui_end_sticky();

		if (project_assets.length > 0) {

			///if (is_paint || is_sculpt)
			let statusw: i32 = sys_width() - ui_toolbar_w - config_raw.layout[layout_size_t.SIDEBAR_W];
			///end
			///if is_lab
			let statusw: i32 = sys_width();
			///end

			let slotw: i32 = math_floor(52 * ui_SCALE(ui));
			let num: i32 = math_floor(statusw / slotw);

			for (let row: i32 = 0; row < math_floor(math_ceil(project_assets.length / num)); ++row) {
				let mult: i32 = config_raw.show_asset_names ? 2 : 1;
				let ar: f32[] = [];
				for (let i: i32 = 0; i < num * mult; ++i) {
					array_push(ar, 1 / num);
				}
				ui_row(ar);

				ui._x += 2;
				let off: f32 = config_raw.show_asset_names ? ui_ELEMENT_OFFSET(ui) * 10.0 : 6;
				if (row > 0) {
					ui._y += off;
				}

				for (let j: i32 = 0; j < num; ++j) {
					let imgw: i32 = math_floor(50 * ui_SCALE(ui));
					let i: i32 = j + row * num;
					if (i >= project_assets.length) {
						_ui_end_element(imgw);
						if (config_raw.show_asset_names) {
							_ui_end_element(0);
						}
						continue;
					}

					let asset: asset_t = project_assets[i];
					let img: image_t = project_get_image(asset);
					let uix: f32 = ui._x;
					let uiy: f32 = ui._y;
					let sw: i32 = img.height < img.width ? img.height : 0;
					if (_ui_image(img, 0xffffffff, slotw, 0, 0, sw, sw) == ui_state_t.STARTED && ui.input_y > ui._window_y) {
						base_drag_off_x = -(mouse_x - uix - ui._window_x - 3);
						base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
						base_drag_asset = asset;
						context_raw.texture = asset;

						if (time_time() - context_raw.select_time < 0.25) {
							ui_base_show_2d_view(view_2d_type_t.ASSET);
						}
						context_raw.select_time = time_time();
						ui_view2d_hwnd.redraws = 2;
					}

					if (asset == context_raw.texture) {
						let _uix: f32 = ui._x;
						let _uiy: f32 = ui._y;
						ui._x = uix;
						ui._y = uiy;
						let off: i32 = i % 2 == 1 ? 1 : 0;
						let w: i32 = 50;
						ui_fill(0,               0, w + 3,       2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(0,     w - off + 2, w + 3, 2 + off, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(0,               0,     2,   w + 3, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(w + 2,           0,     2,   w + 4, ui.ops.theme.HIGHLIGHT_COL);
						ui._x = _uix;
						ui._y = _uiy;
					}

					let is_packed: bool = project_raw.packed_assets != null && project_packed_asset_exists(project_raw.packed_assets, asset.file);

					if (ui.is_hovered) {
						_ui_tooltip_image(img, 256);
						if (is_packed) {
							ui_tooltip(asset.name + " " + tr("(packed)"));
						}
						else {
							ui_tooltip(asset.name);
						}
					}

					if (ui.is_hovered && ui.input_released_r) {
						context_raw.texture = asset;

						_tab_textures_draw_img = img;
						_tab_textures_draw_asset = asset;
						_tab_textures_draw_i = i;
						_tab_textures_draw_is_packed = is_packed;

						ui_menu_draw(function (ui: ui_t) {
							if (ui_menu_button(ui, tr("Export"))) {
								ui_files_show("png", true, false, function (path: string) {
									_tab_textures_draw_path = path;

									app_notify_on_next_frame(function () {
										let img: image_t = _tab_textures_draw_img;

										///if (is_paint || is_sculpt)
										if (base_pipe_merge == null) base_make_pipe();
										///end
										///if is_lab
										if (base_pipe_copy == null) base_make_pipe();
										///end

										let target: image_t = image_create_render_target(tab_textures_to_pow2(img.width), tab_textures_to_pow2(img.height));
										g2_begin(target);
										g2_set_pipeline(base_pipe_copy);
										g2_draw_scaled_image(img, 0, 0, target.width, target.height);
										g2_set_pipeline(null);
										g2_end();
										app_notify_on_next_frame(function (target: image_t) {
											let path: string = _tab_textures_draw_path;
											let f: string = ui_files_filename;
											if (f == "") {
												f = tr("untitled");
											}
											if (!ends_with(f, ".png")) {
												f += ".png";
											}
											iron_write_png(path + path_sep + f, image_get_pixels(target), target.width, target.height, 0);
											image_unload(target);
										}, target);
									});
								});
							}
							if (ui_menu_button(ui, tr("Reimport"))) {
								project_reimport_texture(_tab_textures_draw_asset);
							}

							///if (is_paint || is_sculpt)
							if (ui_menu_button(ui, tr("To Mask"))) {
								app_notify_on_next_frame(function () {
									base_create_image_mask(_tab_textures_draw_asset);
								});
							}
							///end

							if (ui_menu_button(ui, tr("Set as Envmap"))) {
								app_notify_on_next_frame(function () {
									import_envmap_run(_tab_textures_draw_asset.file, _tab_textures_draw_img);
								});
							}

							///if is_paint
							if (ui_menu_button(ui, tr("Set as Color ID Map"))) {
								context_raw.colorid_handle.position = _tab_textures_draw_i;
								context_raw.colorid_picked = false;
								ui_toolbar_handle.redraws = 1;
								if (context_raw.tool == workspace_tool_t.COLORID) {
									ui_header_handle.redraws = 2;
									context_raw.ddirty = 2;
								}
							}
							///end

							if (ui_menu_button(ui, tr("Delete"), "delete")) {
								tab_textures_delete_texture(_tab_textures_draw_asset);
							}
							if (!_tab_textures_draw_is_packed && ui_menu_button(ui, tr("Open Containing Directory..."))) {
								file_start(substring(_tab_textures_draw_asset.file, 0, string_last_index_of(_tab_textures_draw_asset.file, path_sep)));
							}
							if (!_tab_textures_draw_is_packed && ui_menu_button(ui, tr("Open in Browser"))) {
								tab_browser_show_directory(substring(_tab_textures_draw_asset.file, 0, string_last_index_of(_tab_textures_draw_asset.file, path_sep)));
							}
						});
					}

					if (config_raw.show_asset_names) {
						ui._x = uix;
						ui._y += slotw * 0.9;
						ui_text(project_assets[i].name, ui_align_t.CENTER);
						if (ui.is_hovered) {
							ui_tooltip(project_assets[i].name);
						}
						ui._y -= slotw * 0.9;
						if (i == project_assets.length - 1) {
							ui._y += j == num - 1 ? imgw : imgw + ui_ELEMENT_H(ui) + ui_ELEMENT_OFFSET(ui);
						}
					}
				}
			}
		}
		else {
			let img: image_t = resource_get("icons.k");
			let r: rect_t = resource_tile50(img, 0, 1);
			_ui_image(img, ui.ops.theme.BUTTON_COL, r.h, r.x, r.y, r.w, r.h);
			if (ui.is_hovered) {
				ui_tooltip(tr("Drag and drop files here"));
			}
		}

		let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
							 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (in_focus && ui.is_delete_down && project_assets.length > 0 && array_index_of(project_assets, context_raw.texture) >= 0) {
			ui.is_delete_down = false;
			tab_textures_delete_texture(context_raw.texture);
		}
	}
}

function tab_textures_to_pow2(i: i32): i32 {
	i--;
	i |= i >> 1;
	i |= i >> 2;
	i |= i >> 4;
	i |= i >> 8;
	i |= i >> 16;
	i++;
	return i;
}

function tab_textures_update_texture_pointers(nodes: ui_node_t[], i: i32) {
	for (let i: i32 = 0; i < nodes.length; ++i) {
		let n: ui_node_t = nodes[i];
		if (n.type == "TEX_IMAGE") {
			if (n.buttons[0].default_value[0] == i) {
				n.buttons[0].default_value[0] = 9999; // Texture deleted, use pink now
			}
			else if (n.buttons[0].default_value[0] > i) {
				n.buttons[0].default_value[0]--; // Offset by deleted texture
			}
		}
	}
}

function tab_textures_delete_texture(asset: asset_t) {
	let i: i32 = array_index_of(project_assets, asset);
	if (project_assets.length > 1) {
		context_raw.texture = project_assets[i == project_assets.length - 1 ? i - 1 : i + 1];
	}
	ui_base_hwnds[tab_area_t.STATUS].redraws = 2;

	///if is_paint
	if (context_raw.tool == workspace_tool_t.COLORID && i == context_raw.colorid_handle.position) {
		ui_header_handle.redraws = 2;
		context_raw.ddirty = 2;
		context_raw.colorid_picked = false;
		ui_toolbar_handle.redraws = 1;
	}
	///end

	data_delete_image(asset.file);
	map_delete(project_asset_map, asset.id);
	array_splice(project_assets, i, 1);
	array_splice(project_asset_names, i, 1);
	app_notify_on_next_frame(function () {
		make_material_parse_paint_material();

		///if (is_paint || is_sculpt)
		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		///end
	});

	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		tab_textures_update_texture_pointers(m.canvas.nodes, i);
	}
	///if (is_paint || is_sculpt)
	for (let i: i32 = 0; i < project_brushes.length; ++i) {
		let b: slot_brush_t = project_brushes[i];
		tab_textures_update_texture_pointers(b.canvas.nodes, i);
	}
	///end
}
