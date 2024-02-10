
///if (is_paint || is_sculpt)

class TabMaterials {

	static draw = (htab: zui_handle_t) => {
		let mini = Config.raw.layout[LayoutSize.LayoutSidebarW] <= UIBase.sidebarMiniW;
		mini ? TabMaterials.drawMini(htab) : TabMaterials.drawFull(htab);
	}

	static drawMini = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		zui_set_hovered_tab_name(tr("Materials"));

		zui_begin_sticky();
		zui_separator(5);

		TabMaterials.buttonNodes();
		TabMaterials.buttonNew("+");

		zui_end_sticky();
		zui_separator(3, false);
		TabMaterials.drawSlots(true);
	}

	static drawFull = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("Materials"))) {
			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4, 1 / 4]);

			TabMaterials.buttonNew(tr("New"));
			if (zui_button(tr("Import"))) {
				Project.importMaterial();
			}
			TabMaterials.buttonNodes();

			zui_end_sticky();
			zui_separator(3, false);
			TabMaterials.drawSlots(false);
		}
	}

	static buttonNodes = () => {
		let ui = UIBase.ui;
		if (zui_button(tr("Nodes"))) {
			UIBase.showMaterialNodes();
		}
		else if (ui.is_hovered) zui_tooltip(tr("Show Node Editor") + ` (${Config.keymap.toggle_node_editor})`);
	}

	static drawSlots = (mini: bool) => {
		let ui = UIBase.ui;
		let slotw = Math.floor(51 * zui_SCALE(ui));
		let num = Math.floor(Config.raw.layout[LayoutSize.LayoutSidebarW] / slotw);

		for (let row = 0; row < Math.floor(Math.ceil(Project.materials.length / num)); ++row) {
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
				if (i >= Project.materials.length) {
					zui_end_element(imgw);
					if (Config.raw.show_asset_names) zui_end_element(0);
					continue;
				}
				let img = zui_SCALE(ui) > 1 ? Project.materials[i].image : Project.materials[i].imageIcon;
				let imgFull = Project.materials[i].image;

				// Highligh selected
				if (Context.raw.material == Project.materials[i]) {
					if (mini) {
						let w = ui._w / zui_SCALE(ui);
						zui_rect(0, -2, w - 2, w - 4, ui.t.HIGHLIGHT_COL, 3);
					}
					else {
						let off = row % 2 == 1 ? 1 : 0;
						let w = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						zui_fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						zui_fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						zui_fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}
				}

				///if krom_opengl
				ui.image_invert_y = Project.materials[i].previewReady;
				///end

				// Draw material icon
				let uix = ui._x;
				let uiy = ui._y;
				let tile = zui_SCALE(ui) > 1 ? 100 : 50;
				let imgh: Null<f32> = mini ? UIBase.defaultSidebarMiniW * 0.85 * zui_SCALE(ui) : null;
				let state = Project.materials[i].previewReady ?
					zui_image(img, 0xffffffff, imgh) :
					zui_image(Res.get("icons.k"), 0xffffffff, null, tile, tile, tile, tile);

				// Draw material numbers when selecting a material via keyboard shortcut
				let isTyping = ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;
				if (!isTyping) {
					if (i < 9 && Operator.shortcut(Config.keymap.select_material, ShortcutType.ShortcutDown)) {
						let number = String(i + 1);
						let width = g2_font_width(ui.font, ui.font_size, number) + 10;
						let height = g2_font_height(ui.font, ui.font_size);
						g2_set_color(ui.t.TEXT_COL);
						g2_fill_rect(uix, uiy, width, height);
						g2_set_color(ui.t.ACCENT_COL);
						g2_draw_string(number, uix + 5, uiy);
					}
				}

				// Select material
				if (state == State.Started && ui.input_y > ui._window_y) {
					if (Context.raw.material != Project.materials[i]) {
						Context.selectMaterial(i);
						///if is_paint
						if (Context.raw.tool == WorkspaceTool.ToolMaterial) {
							let _init = () => {
								Base.updateFillLayers();
							}
							app_notify_on_init(_init);
						}
						///end
					}
					Base.dragOffX = -(mouse_x - uix - ui._window_x - 3);
					Base.dragOffY = -(mouse_y - uiy - ui._window_y + 1);
					Base.dragMaterial = Context.raw.material;
					// Double click to show nodes
					if (time_time() - Context.raw.selectTime < 0.25) {
						UIBase.showMaterialNodes();
						Base.dragMaterial = null;
						Base.isDragging = false;
					}
					Context.raw.selectTime = time_time();
				}

				// Context menu
				if (ui.is_hovered && ui.input_released_r) {
					Context.selectMaterial(i);
					let add = Project.materials.length > 1 ? 1 : 0;

					UIMenu.draw((ui: zui_t) => {
						let m = Project.materials[i];

						if (UIMenu.menuButton(ui, tr("To Fill Layer"))) {
							Context.selectMaterial(i);
							Base.createFillLayer();
						}

						if (UIMenu.menuButton(ui, tr("Export"))) {
							Context.selectMaterial(i);
							BoxExport.showMaterial();
						}

						///if is_paint
						if (UIMenu.menuButton(ui, tr("Bake"))) {
							Context.selectMaterial(i);
							BoxExport.showBakeMaterial();
						}
						///end

						if (UIMenu.menuButton(ui, tr("Duplicate"))) {
							let _init = () => {
								Context.raw.material = SlotMaterial.create(Project.materials[0].data);
								Project.materials.push(Context.raw.material);
								let cloned = JSON.parse(JSON.stringify(Project.materials[i].canvas));
								Context.raw.material.canvas = cloned;
								TabMaterials.updateMaterial();
								History.duplicateMaterial();
							}
							app_notify_on_init(_init);
						}

						if (Project.materials.length > 1 && UIMenu.menuButton(ui, tr("Delete"), "delete")) {
							TabMaterials.deleteMaterial(m);
						}

						let baseHandle = zui_nest(zui_handle("tabmaterials_0"), m.id, {selected: m.paintBase});
						let opacHandle = zui_nest(zui_handle("tabmaterials_1"), m.id, {selected: m.paintOpac});
						let norHandle = zui_nest(zui_handle("tabmaterials_2"), m.id, {selected: m.paintNor});
						let occHandle = zui_nest(zui_handle("tabmaterials_3"), m.id, {selected: m.paintOcc});
						let roughHandle = zui_nest(zui_handle("tabmaterials_4"), m.id, {selected: m.paintRough});
						let metHandle = zui_nest(zui_handle("tabmaterials_5"), m.id, {selected: m.paintMet});
						let heightHandle = zui_nest(zui_handle("tabmaterials_6"), m.id, {selected: m.paintHeight});
						let emisHandle = zui_nest(zui_handle("tabmaterials_7"), m.id, {selected: m.paintEmis});
						let subsHandle = zui_nest(zui_handle("tabmaterials_8"), m.id, {selected: m.paintSubs});
						UIMenu.menuFill(ui);
						m.paintBase = zui_check(baseHandle, tr("Base Color"));
						UIMenu.menuFill(ui);
						m.paintOpac = zui_check(opacHandle, tr("Opacity"));
						UIMenu.menuFill(ui);
						m.paintNor = zui_check(norHandle, tr("Normal"));
						UIMenu.menuFill(ui);
						m.paintOcc = zui_check(occHandle, tr("Occlusion"));
						UIMenu.menuFill(ui);
						m.paintRough = zui_check(roughHandle, tr("Roughness"));
						UIMenu.menuFill(ui);
						m.paintMet = zui_check(metHandle, tr("Metallic"));
						UIMenu.menuFill(ui);
						m.paintHeight = zui_check(heightHandle, tr("Height"));
						UIMenu.menuFill(ui);
						m.paintEmis = zui_check(emisHandle, tr("Emission"));
						UIMenu.menuFill(ui);
						m.paintSubs = zui_check(subsHandle, tr("Subsurface"));
						if (baseHandle.changed ||
							opacHandle.changed ||
							norHandle.changed ||
							occHandle.changed ||
							roughHandle.changed ||
							metHandle.changed ||
							heightHandle.changed ||
							emisHandle.changed ||
							subsHandle.changed) {
							MakeMaterial.parsePaintMaterial();
							UIMenu.keepOpen = true;
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
					zui_text(Project.materials[i].canvas.name, Align.Center);
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

		let inFocus = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
					  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
		if (inFocus && ui.is_delete_down && Project.materials.length > 1) {
			ui.is_delete_down = false;
			TabMaterials.deleteMaterial(Context.raw.material);
		}
	}

	static buttonNew = (text: string) => {
		let ui = UIBase.ui;
		if (zui_button(text)) {
			let current = _g2_current;
			g2_end();
			Context.raw.material = SlotMaterial.create(Project.materials[0].data);
			Project.materials.push(Context.raw.material);
			TabMaterials.updateMaterial();
			g2_begin(current, false);
			History.newMaterial();
		}
	}

	static updateMaterial = () => {
		UIHeader.headerHandle.redraws = 2;
		UINodes.hwnd.redraws = 2;
		UINodes.groupStack = [];
		MakeMaterial.parsePaintMaterial();
		UtilRender.makeMaterialPreview();
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		if (decal) UtilRender.makeDecalPreview();
	}

	static updateMaterialPointers = (nodes: zui_node_t[], i: i32) => {
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

	static acceptSwatchDrag = (swatch: TSwatchColor) => {
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
		TabMaterials.updateMaterial();
		History.newMaterial();
	}

	static deleteMaterial = (m: SlotMaterialRaw) => {
		let i = Project.materials.indexOf(m);
		for (let l of Project.layers) if (l.fill_layer == m) l.fill_layer = null;
		History.deleteMaterial();
		Context.selectMaterial(i == Project.materials.length - 1 ? i - 1 : i + 1);
		Project.materials.splice(i, 1);
		UIBase.hwnds[1].redraws = 2;
		for (let m of Project.materials) TabMaterials.updateMaterialPointers(m.canvas.nodes, i);
	}
}

///end
