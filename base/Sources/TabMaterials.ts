
///if (is_paint || is_sculpt)

class TabMaterials {

	static draw = (htab: zui_handle_t) => {
		let mini: bool = Config.raw.layout[layout_size_t.SIDEBAR_W] <= UIBase.sidebar_mini_w;
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
				Project.import_material();
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
		else if (ui.is_hovered) zui_tooltip(tr("Show Node Editor") + ` (${Config.keymap.toggle_node_editor})`);
	}

	static draw_slots = (mini: bool) => {
		let ui: zui_t = UIBase.ui;
		let slotw: i32 = Math.floor(51 * zui_SCALE(ui));
		let num: i32 = Math.floor(Config.raw.layout[layout_size_t.SIDEBAR_W] / slotw);

		for (let row: i32 = 0; row < Math.floor(Math.ceil(Project.materials.length / num)); ++row) {
			let mult: i32 = Config.raw.show_asset_names ? 2 : 1;
			let ar: f32[] = [];
			for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
			zui_row(ar);

			ui._x += 2;
			let off: f32 = Config.raw.show_asset_names ? zui_ELEMENT_OFFSET(ui) * 10.0 : 6;
			if (row > 0) ui._y += off;

			for (let j: i32 = 0; j < num; ++j) {
				let imgw: i32 = Math.floor(50 * zui_SCALE(ui));
				let i: i32 = j + row * num;
				if (i >= Project.materials.length) {
					zui_end_element(imgw);
					if (Config.raw.show_asset_names) zui_end_element(0);
					continue;
				}
				let img: image_t = zui_SCALE(ui) > 1 ? Project.materials[i].image : Project.materials[i].image_icon;
				let imgFull: image_t = Project.materials[i].image;

				// Highligh selected
				if (Context.raw.material == Project.materials[i]) {
					if (mini) {
						let w: f32 = ui._w / zui_SCALE(ui);
						zui_rect(0, -2, w - 2, w - 4, ui.t.HIGHLIGHT_COL, 3);
					}
					else {
						let off: i32 = row % 2 == 1 ? 1 : 0;
						let w: i32 = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						zui_fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						zui_fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}
				}

				///if krom_opengl
				ui.image_invert_y = Project.materials[i].preview_ready;
				///end

				// Draw material icon
				let uix: f32 = ui._x;
				let uiy: f32 = ui._y;
				let tile: i32 = zui_SCALE(ui) > 1 ? 100 : 50;
				let imgh: f32 = mini ? UIBase.default_sidebar_mini_w * 0.85 * zui_SCALE(ui) : -1.0;
				let state = Project.materials[i].preview_ready ?
					zui_image(img, 0xffffffff, imgh) :
					zui_image(Res.get("icons.k"), 0xffffffff, -1.0, tile, tile, tile, tile);

				// Draw material numbers when selecting a material via keyboard shortcut
				let isTyping: bool = ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;
				if (!isTyping) {
					if (i < 9 && Operator.shortcut(Config.keymap.select_material, ShortcutType.ShortcutDown)) {
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
					if (Context.raw.material != Project.materials[i]) {
						Context.select_material(i);
						///if is_paint
						if (Context.raw.tool == workspace_tool_t.MATERIAL) {
							let _init = () => {
								Base.update_fill_layers();
							}
							app_notify_on_init(_init);
						}
						///end
					}
					Base.drag_off_x = -(mouse_x - uix - ui._window_x - 3);
					Base.drag_off_y = -(mouse_y - uiy - ui._window_y + 1);
					Base.drag_material = Context.raw.material;
					// Double click to show nodes
					if (time_time() - Context.raw.select_time < 0.25) {
						UIBase.show_material_nodes();
						Base.drag_material = null;
						Base.is_dragging = false;
					}
					Context.raw.select_time = time_time();
				}

				// Context menu
				if (ui.is_hovered && ui.input_released_r) {
					Context.select_material(i);
					let add: i32 = Project.materials.length > 1 ? 1 : 0;

					UIMenu.draw((ui: zui_t) => {
						let m: SlotMaterialRaw = Project.materials[i];

						if (UIMenu.menu_button(ui, tr("To Fill Layer"))) {
							Context.select_material(i);
							Base.create_fill_layer();
						}

						if (UIMenu.menu_button(ui, tr("Export"))) {
							Context.select_material(i);
							BoxExport.show_material();
						}

						///if is_paint
						if (UIMenu.menu_button(ui, tr("Bake"))) {
							Context.select_material(i);
							BoxExport.show_bake_material();
						}
						///end

						if (UIMenu.menu_button(ui, tr("Duplicate"))) {
							let _init = () => {
								Context.raw.material = SlotMaterial.create(Project.materials[0].data);
								Project.materials.push(Context.raw.material);
								let cloned: zui_node_canvas_t = json_parse(json_stringify(Project.materials[i].canvas));
								Context.raw.material.canvas = cloned;
								TabMaterials.update_material();
								History.duplicate_material();
							}
							app_notify_on_init(_init);
						}

						if (Project.materials.length > 1 && UIMenu.menu_button(ui, tr("Delete"), "delete")) {
							TabMaterials.delete_material(m);
						}

						let baseHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_0"), m.id, {selected: m.paint_base});
						let opacHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_1"), m.id, {selected: m.paint_opac});
						let norHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_2"), m.id, {selected: m.paint_nor});
						let occHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_3"), m.id, {selected: m.paint_occ});
						let roughHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_4"), m.id, {selected: m.paint_rough});
						let metHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_5"), m.id, {selected: m.paint_met});
						let heightHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_6"), m.id, {selected: m.paint_height});
						let emisHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_7"), m.id, {selected: m.paint_emis});
						let subsHandle: zui_handle_t = zui_nest(zui_handle("tabmaterials_8"), m.id, {selected: m.paint_subs});
						UIMenu.menu_fill(ui);
						m.paint_base = zui_check(baseHandle, tr("Base Color"));
						UIMenu.menu_fill(ui);
						m.paint_opac = zui_check(opacHandle, tr("Opacity"));
						UIMenu.menu_fill(ui);
						m.paint_nor = zui_check(norHandle, tr("Normal"));
						UIMenu.menu_fill(ui);
						m.paint_occ = zui_check(occHandle, tr("Occlusion"));
						UIMenu.menu_fill(ui);
						m.paint_rough = zui_check(roughHandle, tr("Roughness"));
						UIMenu.menu_fill(ui);
						m.paint_met = zui_check(metHandle, tr("Metallic"));
						UIMenu.menu_fill(ui);
						m.paint_height = zui_check(heightHandle, tr("Height"));
						UIMenu.menu_fill(ui);
						m.paint_emis = zui_check(emisHandle, tr("Emission"));
						UIMenu.menu_fill(ui);
						m.paint_subs = zui_check(subsHandle, tr("Subsurface"));
						if (baseHandle.changed ||
							opacHandle.changed ||
							norHandle.changed ||
							occHandle.changed ||
							roughHandle.changed ||
							metHandle.changed ||
							heightHandle.changed ||
							emisHandle.changed ||
							subsHandle.changed) {
							MakeMaterial.parse_paint_material();
							UIMenu.keep_open = true;
						}
					}, 13 + add);
				}
				if (ui.is_hovered) {
					zui_tooltip_image(imgFull);
					if (i < 9) zui_tooltip(Project.materials[i].canvas.name + " - (" + Config.keymap.select_material + " " + (i + 1) + ")");
					else zui_tooltip(Project.materials[i].canvas.name);
				}

				if (Config.raw.show_asset_names) {
					ui._x = uix;
					ui._y += slotw * 0.9;
					zui_text(Project.materials[i].canvas.name, zui_align_t.CENTER);
					if (ui.is_hovered) {
						if (i < 9) zui_tooltip(Project.materials[i].canvas.name + " - (" + Config.keymap.select_material + " " + (i + 1) + ")");
						else zui_tooltip(Project.materials[i].canvas.name);
					}
					ui._y -= slotw * 0.9;
					if (i == Project.materials.length - 1) {
						ui._y += j == num - 1 ? imgw : imgw + zui_ELEMENT_H(ui) + zui_ELEMENT_OFFSET(ui);
					}
				}
			}

			ui._y += mini ? 0 : 6;

			///if krom_opengl
			ui.image_invert_y = false; // Material preview
			///end
		}

		let inFocus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
					  		ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (inFocus && ui.is_delete_down && Project.materials.length > 1) {
			ui.is_delete_down = false;
			TabMaterials.delete_material(Context.raw.material);
		}
	}

	static button_new = (text: string) => {
		if (zui_button(text)) {
			let current: image_t = _g2_current;
			g2_end();
			Context.raw.material = SlotMaterial.create(Project.materials[0].data);
			Project.materials.push(Context.raw.material);
			TabMaterials.update_material();
			g2_begin(current);
			History.new_material();
		}
	}

	static update_material = () => {
		UIHeader.header_handle.redraws = 2;
		UINodes.hwnd.redraws = 2;
		UINodes.group_stack = [];
		MakeMaterial.parse_paint_material();
		UtilRender.make_material_preview();
		let decal: bool = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
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
		Context.raw.material = SlotMaterial.create(Project.materials[0].data);
		for (let node of Context.raw.material.canvas.nodes) {
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
		Project.materials.push(Context.raw.material);
		TabMaterials.update_material();
		History.new_material();
	}

	static delete_material = (m: SlotMaterialRaw) => {
		let i: i32 = Project.materials.indexOf(m);
		for (let l of Project.layers) if (l.fill_layer == m) l.fill_layer = null;
		History.delete_material();
		Context.select_material(i == Project.materials.length - 1 ? i - 1 : i + 1);
		Project.materials.splice(i, 1);
		UIBase.hwnds[1].redraws = 2;
		for (let m of Project.materials) TabMaterials.update_material_pointers(m.canvas.nodes, i);
	}
}

///end
