
let _tab_textures_draw_img: gpu_texture_t;
let _tab_textures_draw_path: string;
let _tab_textures_draw_asset: asset_t;
let _tab_textures_draw_i: i32;
let _tab_textures_draw_is_packed: bool;

function tab_textures_draw(htab: ui_handle_t) {

	if (ui_tab(htab, tr("Textures")) && ui._window_h > ui_statusbar_default_h * UI_SCALE()) {

		ui_begin_sticky();

		let row: f32[] = [ -100, -100 ];
		ui_row(row);

		if (ui_button(tr("Import"))) {
			ui_files_show(string_array_join(path_texture_formats, ","), false, true, function(path: string) {
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

			let slotw: i32 = math_floor(52 * UI_SCALE());
			let num: i32   = math_floor(ui._window_w / slotw);
			if (num == 0) {
				return;
			}

			for (let row: i32 = 0; row < math_floor(math_ceil(project_assets.length / num)); ++row) {
				let mult: i32 = config_raw.show_asset_names ? 2 : 1;
				let ar: f32[] = [];
				for (let i: i32 = 0; i < num * mult; ++i) {
					array_push(ar, 1 / num);
				}
				ui_row(ar);

				ui._x += 2;
				let off: f32 = config_raw.show_asset_names ? UI_ELEMENT_OFFSET() * 10.0 : 6;
				if (row > 0) {
					ui._y += off;
				}

				for (let j: i32 = 0; j < num; ++j) {
					let imgw: i32 = math_floor(50 * UI_SCALE());
					let i: i32    = j + row * num;
					if (i >= project_assets.length) {
						ui_end_element_of_size(imgw);
						if (config_raw.show_asset_names) {
							ui_end_element_of_size(0);
						}
						continue;
					}

					let asset: asset_t     = project_assets[i];
					let img: gpu_texture_t = project_get_image(asset);
					if (img == null) {
						let empty_rt: render_target_t = map_get(render_path_render_targets, "empty_black");
						img                           = empty_rt._image;
					}
					let uix: f32 = ui._x;
					let uiy: f32 = ui._y;
					let sw: i32  = img.height < img.width ? img.height : 0;
					if (ui_sub_image(img, 0xffffffff, slotw, 0, 0, sw, sw) == ui_state_t.STARTED && ui.input_y > ui._window_y) {
						base_drag_off_x     = -(mouse_x - uix - ui._window_x - 3);
						base_drag_off_y     = -(mouse_y - uiy - ui._window_y + 1);
						base_drag_asset     = asset;
						context_raw.texture = asset;

						if (sys_time() - context_raw.select_time < 0.2) {
							ui_base_show_2d_view(view_2d_type_t.ASSET);
						}
						context_raw.select_time = sys_time();
						ui_view2d_hwnd.redraws  = 2;
					}

					if (asset == context_raw.texture) {
						let _uix: f32 = ui._x;
						let _uiy: f32 = ui._y;
						ui._x         = uix;
						ui._y         = uiy;
						let off: i32  = i % 2 == 1 ? 1 : 0;
						let w: i32    = 50;
						ui_fill(0, 0, w + 3, 2, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(0, w - off + 2, w + 3, 2 + off, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(0, 0, 2, w + 3, ui.ops.theme.HIGHLIGHT_COL);
						ui_fill(w + 2, 0, 2, w + 4, ui.ops.theme.HIGHLIGHT_COL);
						ui._x = _uix;
						ui._y = _uiy;
					}

					let is_packed: bool = project_raw.packed_assets != null && project_packed_asset_exists(project_raw.packed_assets, asset.file);

					if (ui.is_hovered) {
						ui_tooltip_image(img, 256);
						if (is_packed) {
							ui_tooltip(asset.name + " " + tr("(packed)"));
						}
						else {
							ui_tooltip(asset.name);
						}
					}

					if (ui.is_hovered && ui.input_released_r) {
						context_raw.texture = asset;

						_tab_textures_draw_img       = img;
						_tab_textures_draw_asset     = asset;
						_tab_textures_draw_i         = i;
						_tab_textures_draw_is_packed = is_packed;

						ui_menu_draw(function() {
							if (ui_menu_button(tr("Export"))) {
								ui_files_show("png", true, false, function(path: string) {
									_tab_textures_draw_path = path;

									sys_notify_on_next_frame(function() {
										let img: gpu_texture_t    = _tab_textures_draw_img;
										let target: gpu_texture_t = gpu_create_render_target(img.width, img.height);
										draw_begin(target);
										draw_set_pipeline(pipes_copy);
										draw_scaled_image(img, 0, 0, target.width, target.height);
										draw_set_pipeline(null);
										draw_end();
										sys_notify_on_next_frame(function(target: gpu_texture_t) {
											let path: string = _tab_textures_draw_path;
											let f: string    = ui_files_filename;
											if (f == "") {
												f = tr("untitled");
											}
											if (!ends_with(f, ".png")) {
												f += ".png";
											}

											/// if IRON_BGRA
											let buf: buffer_t = export_arm_bgra_swap(gpu_get_texture_pixels(target));
											/// else
											let buf: buffer_t = gpu_get_texture_pixels(target);
											/// end

											iron_write_png(path + path_sep + f, buf, target.width, target.height, 0);
											gpu_delete_texture(target);
										}, target);
									});
								});
							}
							if (ui_menu_button(tr("Reimport"))) {
								project_reimport_texture(_tab_textures_draw_asset);
							}

							if (ui_menu_button(tr("To Mask"))) {
								sys_notify_on_next_frame(function() {
									layers_create_image_mask(_tab_textures_draw_asset);
								});
							}

							if (ui_menu_button(tr("Set as Envmap"))) {
								sys_notify_on_next_frame(function() {
									import_envmap_run(_tab_textures_draw_asset.file, _tab_textures_draw_img);
								});
							}

							if (ui_menu_button(tr("Set as Color ID Map"))) {
								context_raw.colorid_handle.i = _tab_textures_draw_i;
								context_raw.colorid_picked   = false;
								ui_toolbar_handle.redraws    = 1;
								if (context_raw.tool == tool_type_t.COLORID) {
									ui_header_handle.redraws = 2;
									context_raw.ddirty       = 2;
								}
							}

							if (ui_menu_button(tr("Delete"), "delete")) {
								tab_textures_delete_texture(_tab_textures_draw_asset);
							}
							if (!_tab_textures_draw_is_packed && ui_menu_button(tr("Open Containing Directory..."))) {
								file_start(substring(_tab_textures_draw_asset.file, 0, string_last_index_of(_tab_textures_draw_asset.file, path_sep)));
							}
							if (!_tab_textures_draw_is_packed && ui_menu_button(tr("Open in Browser"))) {
								tab_browser_show_directory(
								    substring(_tab_textures_draw_asset.file, 0, string_last_index_of(_tab_textures_draw_asset.file, path_sep)));
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
							ui._y += j == num - 1 ? imgw : imgw + UI_ELEMENT_H() + UI_ELEMENT_OFFSET();
						}
					}
				}
			}
		}
		else {
			let img: gpu_texture_t = resource_get("icons.k");
			let r: rect_t          = resource_tile50(img, 0, 1);
			ui_sub_image(img, ui.ops.theme.BUTTON_COL, r.h, r.x, r.y, r.w, r.h);
			if (ui.is_hovered) {
				ui_tooltip(tr("Drag and drop files here"));
			}
		}

		let in_focus: bool =
		    ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w && ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (in_focus && ui.is_delete_down && project_assets.length > 0 && array_index_of(project_assets, context_raw.texture) >= 0) {
			ui.is_delete_down = false;
			tab_textures_delete_texture(context_raw.texture);
		}
	}
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

	if (context_raw.tool == tool_type_t.COLORID && i == context_raw.colorid_handle.i) {
		ui_header_handle.redraws   = 2;
		context_raw.ddirty         = 2;
		context_raw.colorid_picked = false;
		ui_toolbar_handle.redraws  = 1;
	}

	if (data_get_image(asset.file) == scene_world._.envmap) {
		project_set_default_envmap();
	}

	if (project_raw.packed_assets != null) {
		for (let i: i32 = 0; i < project_raw.packed_assets.length; ++i) {
			let pa: packed_asset_t = project_raw.packed_assets[i];
			if (pa.name == asset.file) {
				array_splice(project_raw.packed_assets, i, 1);
				break;
			}
		}
	}

	data_delete_image(asset.file);
	map_delete(project_asset_map, asset.id);
	array_splice(project_assets, i, 1);
	array_splice(project_asset_names, i, 1);
	sys_notify_on_next_frame(function() {
		make_material_parse_paint_material();

		util_render_make_material_preview();
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	});

	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		tab_textures_update_texture_pointers(m.canvas.nodes, i);
	}

	for (let i: i32 = 0; i < project_brushes.length; ++i) {
		let b: slot_brush_t = project_brushes[i];
		tab_textures_update_texture_pointers(b.canvas.nodes, i);
	}
}
