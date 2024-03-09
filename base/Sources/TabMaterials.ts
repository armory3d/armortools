
///if (is_paint || is_sculpt)

class TabMaterials {

	static draw = (htab: zui_handle_t) => {
		let mini: bool = config_raw.layout[layout_size_t.SIDEBAR_W] <= UIBase.sidebar_mini_w;
		mini ? TabMaterials.draw_mini(htab) : TabMaterials.draw_full(htab);
	}

	static draw_mini = (htab: zui_handle_t) => {
		zui_set_hovered_tab_name(tr("Materials"));

		zui_begin_sticky();
		zui_separator(5);

		TabMaterials.button_nodes();
		TabMaterials.button_new("+");

		zui_end_sticky();
		zui_separator(3, false);
		TabMaterials.draw_slots(true);
	}

	static draw_full = (htab: zui_handle_t) => {
		if (zui_tab(htab, tr("Materials"))) {
			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4, 1 / 4]);

			TabMaterials.button_new(tr("New"));
			if (zui_button(tr("Import"))) {
				project_import_material();
			}
			TabMaterials.button_nodes();

			zui_end_sticky();
			zui_separator(3, false);
			TabMaterials.draw_slots(false);
		}
	}

	static button_nodes = () => {
		let ui: zui_t = UIBase.ui;
		if (zui_button(tr("Nodes"))) {
			UIBase.show_material_nodes();
		}
		else if (ui.is_hovered) zui_tooltip(tr("Show Node Editor") + ` (${config_keymap.toggle_node_editor})`);
	}

	static draw_slots = (mini: bool) => {
		let ui: zui_t = UIBase.ui;
		let slotw: i32 = Math.floor(51 * zui_SCALE(ui));
		let num: i32 = Math.floor(config_raw.layout[layout_size_t.SIDEBAR_W] / slotw);

		for (let row: i32 = 0; row < Math.floor(Math.ceil(project_materials.length / num)); ++row) {
			let mult: i32 = config_raw.show_asset_names ? 2 : 1;
			let ar: f32[] = [];
			for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
			zui_row(ar);

			ui._x += 2;
			let off: f32 = config_raw.show_asset_names ? zui_ELEMENT_OFFSET(ui) * 10.0 : 6;
			if (row > 0) ui._y += off;

			for (let j: i32 = 0; j < num; ++j) {
				let imgw: i32 = Math.floor(50 * zui_SCALE(ui));
				let i: i32 = j + row * num;
				if (i >= project_materials.length) {
					zui_end_element(imgw);
					if (config_raw.show_asset_names) zui_end_element(0);
					continue;
				}
				let img: image_t = zui_SCALE(ui) > 1 ? project_materials[i].image : project_materials[i].image_icon;
				let imgFull: image_t = project_materials[i].image;

				// Highligh selected
				if (context_raw.material == project_materials[i]) {
					if (mini) {
						let w: f32 = ui._w / zui_SCALE(ui);
						zui_rect(0, -2, w - 2, w - 4, ui.t.HIGHLIGHT_COL, 3);
					}
					else {
						let off: i32 = row % 2 == 1 ? 1 : 0;
						let w: i32 = 50;
						if (config_raw.window_scale > 1) w += Math.floor(config_raw.window_scale * 2);
						zui_fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						zui_fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}
				}

				///if krom_opengl
				ui.image_invert_y = project_materials[i].preview_ready;
				///end

				// Draw material icon
				let uix: f32 = ui._x;
				let uiy: f32 = ui._y;
				let tile: i32 = zui_SCALE(ui) > 1 ? 100 : 50;
				let imgh: f32 = mini ? UIBase.default_sidebar_mini_w * 0.85 * zui_SCALE(ui) : -1.0;
				let state = project_materials[i].preview_ready ?
					zui_image(img, 0xffffffff, imgh) :
					zui_image(resource_get("icons.k"), 0xffffffff, -1.0, tile, tile, tile, tile);

				// Draw material numbers when selecting a material via keyboard shortcut
				let is_typing: bool = ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;
				if (!is_typing) {
					if (i < 9 && operator_shortcut(config_keymap.select_material, shortcut_type_t.DOWN)) {
						let number: string = String(i + 1);
						let width: i32 = g2_font_width(ui.font, ui.font_size, number) + 10;
						let height: i32 = g2_font_height(ui.font, ui.font_size);
						g2_set_color(ui.t.TEXT_COL);
						g2_fill_rect(uix, uiy, width, height);
						g2_set_color(ui.t.ACCENT_COL);
						g2_draw_string(number, uix + 5, uiy);
					}
				}

				// Select material
				if (state == zui_state_t.STARTED && ui.input_y > ui._window_y) {
					if (context_raw.material != project_materials[i]) {
						context_select_material(i);
						///if is_paint
						if (context_raw.tool == workspace_tool_t.MATERIAL) {
							let _init = () => {
								base_update_fill_layers();
							}
							app_notify_on_init(_init);
						}
						///end
					}
					base_drag_off_x = -(mouse_x - uix - ui._window_x - 3);
					base_drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
					base_drag_material = context_raw.material;
					// Double click to show nodes
					if (time_time() - context_raw.select_time < 0.25) {
						UIBase.show_material_nodes();
						base_drag_material = null;
						base_is_dragging = false;
					}
					context_raw.select_time = time_time();
				}

				// Context menu
				if (ui.is_hovered && ui.input_released_r) {
					context_select_material(i);
					let add: i32 = project_materials.length > 1 ? 1 : 0;

					UIMenu.draw((ui: zui_t) => {
						let m: SlotMaterialRaw = project_materials[i];

						if (UIMenu.menu_button(ui, tr("To Fill Layer"))) {
							context_select_material(i);
							base_create_fill_layer();
						}

						if (UIMenu.menu_button(ui, tr("Export"))) {
							context_select_material(i);
							BoxExport.show_material();
						}

						///if is_paint
						if (UIMenu.menu_button(ui, tr("Bake"))) {
							context_select_material(i);
							BoxExport.show_bake_material();
						}
						///end

						if (UIMenu.menu_button(ui, tr("Duplicate"))) {
							let _init = () => {
								context_raw.material = SlotMaterial.create(project_materials[0].data);
								project_materials.push(context_raw.material);
								let cloned: zui_node_canvas_t = json_parse(json_stringify(project_materials[i].canvas));
								context_raw.material.canvas = cloned;
								TabMaterials.update_material();
								history_duplicate_material();
							}
							app_notify_on_init(_init);
						}

						if (project_materials.length > 1 && UIMenu.menu_button(ui, tr("Delete"), "delete")) {
							TabMaterials.delete_material(m);
						}

						let base_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_0"), m.id, {selected: m.paint_base});
						let opac_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_1"), m.id, {selected: m.paint_opac});
						let nor_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_2"), m.id, {selected: m.paint_nor});
						let occ_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_3"), m.id, {selected: m.paint_occ});
						let rough_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_4"), m.id, {selected: m.paint_rough});
						let met_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_5"), m.id, {selected: m.paint_met});
						let height_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_6"), m.id, {selected: m.paint_height});
						let emis_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_7"), m.id, {selected: m.paint_emis});
						let subs_handle: zui_handle_t = zui_nest(zui_handle("tabmaterials_8"), m.id, {selected: m.paint_subs});
						UIMenu.menu_fill(ui);
						m.paint_base = zui_check(base_handle, tr("Base Color"));
						UIMenu.menu_fill(ui);
						m.paint_opac = zui_check(opac_handle, tr("Opacity"));
						UIMenu.menu_fill(ui);
						m.paint_nor = zui_check(nor_handle, tr("Normal"));
						UIMenu.menu_fill(ui);
						m.paint_occ = zui_check(occ_handle, tr("Occlusion"));
						UIMenu.menu_fill(ui);
						m.paint_rough = zui_check(rough_handle, tr("Roughness"));
						UIMenu.menu_fill(ui);
						m.paint_met = zui_check(met_handle, tr("Metallic"));
						UIMenu.menu_fill(ui);
						m.paint_height = zui_check(height_handle, tr("Height"));
						UIMenu.menu_fill(ui);
						m.paint_emis = zui_check(emis_handle, tr("Emission"));
						UIMenu.menu_fill(ui);
						m.paint_subs = zui_check(subs_handle, tr("Subsurface"));
						if (base_handle.changed ||
							opac_handle.changed ||
							nor_handle.changed ||
							occ_handle.changed ||
							rough_handle.changed ||
							met_handle.changed ||
							height_handle.changed ||
							emis_handle.changed ||
							subs_handle.changed) {
							MakeMaterial.parse_paint_material();
							UIMenu.keep_open = true;
						}
					}, 13 + add);
				}
				if (ui.is_hovered) {
					zui_tooltip_image(imgFull);
					if (i < 9) zui_tooltip(project_materials[i].canvas.name + " - (" + config_keymap.select_material + " " + (i + 1) + ")");
					else zui_tooltip(project_materials[i].canvas.name);
				}

				if (config_raw.show_asset_names) {
					ui._x = uix;
					ui._y += slotw * 0.9;
					zui_text(project_materials[i].canvas.name, zui_align_t.CENTER);
					if (ui.is_hovered) {
						if (i < 9) zui_tooltip(project_materials[i].canvas.name + " - (" + config_keymap.select_material + " " + (i + 1) + ")");
						else zui_tooltip(project_materials[i].canvas.name);
					}
					ui._y -= slotw * 0.9;
					if (i == project_materials.length - 1) {
						ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
					}
				}
			}

			ui._y += mini ? 0 : 6;

			///if krom_opengl
			ui.image_invert_y = false; // Material preview
			///end
		}

		let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
					    	 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (in_focus && ui.is_delete_down && project_materials.length > 1) {
			ui.is_delete_down = false;
			TabMaterials.delete_material(context_raw.material);
		}
	}

	static button_new = (text: string) => {
		if (zui_button(text)) {
			let current: image_t = _g2_current;
			g2_end();
			context_raw.material = SlotMaterial.create(project_materials[0].data);
			project_materials.push(context_raw.material);
			TabMaterials.update_material();
			g2_begin(current);
			history_new_material();
		}
	}

	static update_material = () => {
		UIHeader.header_handle.redraws = 2;
		UINodes.hwnd.redraws = 2;
		UINodes.group_stack = [];
		MakeMaterial.parse_paint_material();
		UtilRender.make_material_preview();
		let decal: bool = context_raw.tool == workspace_tool_t.DECAL || context_raw.tool == workspace_tool_t.TEXT;
		if (decal) UtilRender.make_decal_preview();
	}

	static update_material_pointers = (nodes: zui_node_t[], i: i32) => {
		for (let n of nodes) {
			if (n.type == "MATERIAL") {
				if (n.buttons[0].default_value == i) {
					n.buttons[0].default_value = 9999; // Material deleted
				}
				else if (n.buttons[0].default_value > i) {
					n.buttons[0].default_value--; // Offset by deleted material
				}
			}
		}
	}

	static accept_swatch_drag = (swatch: swatch_color_t) => {
		context_raw.material = SlotMaterial.create(project_materials[0].data);
		for (let node of context_raw.material.canvas.nodes) {
			if (node.type == "RGB" ) {
				node.outputs[0].default_value = [
					color_get_rb(swatch.base) / 255,
					color_get_gb(swatch.base) / 255,
					color_get_bb(swatch.base) / 255,
					color_get_ab(swatch.base) / 255
				];
			}
			else if (node.type == "OUTPUT_MATERIAL_PBR") {
				node.inputs[1].default_value = swatch.opacity;
				node.inputs[2].default_value = swatch.occlusion;
				node.inputs[3].default_value = swatch.roughness;
				node.inputs[4].default_value = swatch.metallic;
				node.inputs[7].default_value = swatch.height;
			}
		}
		project_materials.push(context_raw.material);
		TabMaterials.update_material();
		history_new_material();
	}

	static delete_material = (m: SlotMaterialRaw) => {
		let i: i32 = project_materials.indexOf(m);
		for (let l of project_layers) if (l.fill_layer == m) l.fill_layer = null;
		history_delete_material();
		context_select_material(i == project_materials.length - 1 ? i - 1 : i + 1);
		project_materials.splice(i, 1);
		UIBase.hwnds[1].redraws = 2;
		for (let m of project_materials) TabMaterials.update_material_pointers(m.canvas.nodes, i);
	}
}

///end
