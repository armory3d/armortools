
///if (is_paint || is_sculpt)

class TabMaterials {

	static draw = (htab: HandleRaw) => {
		let mini = Config.raw.layout[LayoutSize.LayoutSidebarW] <= UIBase.sidebarMiniW;
		mini ? TabMaterials.drawMini(htab) : TabMaterials.drawFull(htab);
	}

	static drawMini = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		Zui.setHoveredTabName(tr("Materials"));

		Zui.beginSticky();
		Zui.separator(5);

		TabMaterials.buttonNodes();
		TabMaterials.buttonNew("+");

		Zui.endSticky();
		Zui.separator(3, false);
		TabMaterials.drawSlots(true);
	}

	static drawFull = (htab: HandleRaw) => {
		let ui = UIBase.ui;
		if (Zui.tab(htab, tr("Materials"))) {
			Zui.beginSticky();
			Zui.row([1 / 4, 1 / 4, 1 / 4]);

			TabMaterials.buttonNew(tr("New"));
			if (Zui.button(tr("Import"))) {
				Project.importMaterial();
			}
			TabMaterials.buttonNodes();

			Zui.endSticky();
			Zui.separator(3, false);
			TabMaterials.drawSlots(false);
		}
	}

	static buttonNodes = () => {
		let ui = UIBase.ui;
		if (Zui.button(tr("Nodes"))) {
			UIBase.showMaterialNodes();
		}
		else if (ui.isHovered) Zui.tooltip(tr("Show Node Editor") + ` (${Config.keymap.toggle_node_editor})`);
	}

	static drawSlots = (mini: bool) => {
		let ui = UIBase.ui;
		let slotw = Math.floor(51 * Zui.SCALE(ui));
		let num = Math.floor(Config.raw.layout[LayoutSize.LayoutSidebarW] / slotw);

		for (let row = 0; row < Math.floor(Math.ceil(Project.materials.length / num)); ++row) {
			let mult = Config.raw.show_asset_names ? 2 : 1;
			let ar = [];
			for (let i = 0; i < num * mult; ++i) ar.push(1 / num);
			Zui.row(ar);

			ui._x += 2;
			let off = Config.raw.show_asset_names ? Zui.ELEMENT_OFFSET(ui) * 10.0 : 6;
			if (row > 0) ui._y += off;

			for (let j = 0; j < num; ++j) {
				let imgw = Math.floor(50 * Zui.SCALE(ui));
				let i = j + row * num;
				if (i >= Project.materials.length) {
					Zui.endElement(imgw);
					if (Config.raw.show_asset_names) Zui.endElement(0);
					continue;
				}
				let img = Zui.SCALE(ui) > 1 ? Project.materials[i].image : Project.materials[i].imageIcon;
				let imgFull = Project.materials[i].image;

				// Highligh selected
				if (Context.raw.material == Project.materials[i]) {
					if (mini) {
						let w = ui._w / Zui.SCALE(ui);
						Zui.rect(0, -2, w - 2, w - 4, ui.t.HIGHLIGHT_COL, 3);
					}
					else {
						let off = row % 2 == 1 ? 1 : 0;
						let w = 50;
						if (Config.raw.window_scale > 1) w += Math.floor(Config.raw.window_scale * 2);
						Zui.fill(-1,         -2, w + 3,       2, ui.t.HIGHLIGHT_COL);
						Zui.fill(-1,    w - off, w + 3, 2 + off, ui.t.HIGHLIGHT_COL);
						Zui.fill(-1,         -2,     2,   w + 3, ui.t.HIGHLIGHT_COL);
						Zui.fill(w + 1,      -2,     2,   w + 4, ui.t.HIGHLIGHT_COL);
					}
				}

				///if krom_opengl
				ui.imageInvertY = Project.materials[i].previewReady;
				///end

				// Draw material icon
				let uix = ui._x;
				let uiy = ui._y;
				let tile = Zui.SCALE(ui) > 1 ? 100 : 50;
				let imgh: Null<f32> = mini ? UIBase.defaultSidebarMiniW * 0.85 * Zui.SCALE(ui) : null;
				let state = Project.materials[i].previewReady ?
					Zui.image(img, 0xffffffff, imgh) :
					Zui.image(Res.get("icons.k"), 0xffffffff, null, tile, tile, tile, tile);

				// Draw material numbers when selecting a material via keyboard shortcut
				let isTyping = ui.isTyping || UIView2D.ui.isTyping || UINodes.ui.isTyping;
				if (!isTyping) {
					if (i < 9 && Operator.shortcut(Config.keymap.select_material, ShortcutType.ShortcutDown)) {
						let number = String(i + 1);
						let width = font_width(ui.font, ui.fontSize, number) + 10;
						let height = font_height(ui.font, ui.fontSize);
						ui.g.color = ui.t.TEXT_COL;
						g2_fill_rect(uix, uiy, width, height);
						ui.g.color = ui.t.ACCENT_COL;
						g2_draw_string(number, uix + 5, uiy);
					}
				}

				// Select material
				if (state == State.Started && ui.inputY > ui._windowY) {
					if (Context.raw.material != Project.materials[i]) {
						Context.selectMaterial(i);
						///if is_paint
						if (Context.raw.tool == WorkspaceTool.ToolMaterial) {
							let _init = () => {
								Base.updateFillLayers();
							}
							App.notifyOnInit(_init);
						}
						///end
					}
					Base.dragOffX = -(mouse_x - uix - ui._windowX - 3);
					Base.dragOffY = -(mouse_y - uiy - ui._windowY + 1);
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
				if (ui.isHovered && ui.inputReleasedR) {
					Context.selectMaterial(i);
					let add = Project.materials.length > 1 ? 1 : 0;

					UIMenu.draw((ui: ZuiRaw) => {
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
							App.notifyOnInit(_init);
						}

						if (Project.materials.length > 1 && UIMenu.menuButton(ui, tr("Delete"), "delete")) {
							TabMaterials.deleteMaterial(m);
						}

						let baseHandle = Zui.nest(Zui.handle("tabmaterials_0"), m.id, {selected: m.paintBase});
						let opacHandle = Zui.nest(Zui.handle("tabmaterials_1"), m.id, {selected: m.paintOpac});
						let norHandle = Zui.nest(Zui.handle("tabmaterials_2"), m.id, {selected: m.paintNor});
						let occHandle = Zui.nest(Zui.handle("tabmaterials_3"), m.id, {selected: m.paintOcc});
						let roughHandle = Zui.nest(Zui.handle("tabmaterials_4"), m.id, {selected: m.paintRough});
						let metHandle = Zui.nest(Zui.handle("tabmaterials_5"), m.id, {selected: m.paintMet});
						let heightHandle = Zui.nest(Zui.handle("tabmaterials_6"), m.id, {selected: m.paintHeight});
						let emisHandle = Zui.nest(Zui.handle("tabmaterials_7"), m.id, {selected: m.paintEmis});
						let subsHandle = Zui.nest(Zui.handle("tabmaterials_8"), m.id, {selected: m.paintSubs});
						UIMenu.menuFill(ui);
						m.paintBase = Zui.check(baseHandle, tr("Base Color"));
						UIMenu.menuFill(ui);
						m.paintOpac = Zui.check(opacHandle, tr("Opacity"));
						UIMenu.menuFill(ui);
						m.paintNor = Zui.check(norHandle, tr("Normal"));
						UIMenu.menuFill(ui);
						m.paintOcc = Zui.check(occHandle, tr("Occlusion"));
						UIMenu.menuFill(ui);
						m.paintRough = Zui.check(roughHandle, tr("Roughness"));
						UIMenu.menuFill(ui);
						m.paintMet = Zui.check(metHandle, tr("Metallic"));
						UIMenu.menuFill(ui);
						m.paintHeight = Zui.check(heightHandle, tr("Height"));
						UIMenu.menuFill(ui);
						m.paintEmis = Zui.check(emisHandle, tr("Emission"));
						UIMenu.menuFill(ui);
						m.paintSubs = Zui.check(subsHandle, tr("Subsurface"));
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
				if (ui.isHovered) {
					Zui.tooltipImage(imgFull);
					if (i < 9) Zui.tooltip(Project.materials[i].canvas.name + " - (" + Config.keymap.select_material + " " + (i + 1) + ")");
					else Zui.tooltip(Project.materials[i].canvas.name);
				}

				if (Config.raw.show_asset_names) {
					ui._x = uix;
					ui._y += slotw * 0.9;
					Zui.text(Project.materials[i].canvas.name, Align.Center);
					if (ui.isHovered) {
						if (i < 9) Zui.tooltip(Project.materials[i].canvas.name + " - (" + Config.keymap.select_material + " " + (i + 1) + ")");
						else Zui.tooltip(Project.materials[i].canvas.name);
					}
					ui._y -= slotw * 0.9;
					if (i == Project.materials.length - 1) {
						ui._y += j == num - 1 ? imgw : imgw + Zui.ELEMENT_H(ui) + Zui.ELEMENT_OFFSET(ui);
					}
				}
			}

			ui._y += mini ? 0 : 6;

			///if krom_opengl
			ui.imageInvertY = false; // Material preview
			///end
		}

		let inFocus = ui.inputX > ui._windowX && ui.inputX < ui._windowX + ui._windowW &&
					  ui.inputY > ui._windowY && ui.inputY < ui._windowY + ui._windowH;
		if (inFocus && ui.isDeleteDown && Project.materials.length > 1) {
			ui.isDeleteDown = false;
			TabMaterials.deleteMaterial(Context.raw.material);
		}
	}

	static buttonNew = (text: string) => {
		let ui = UIBase.ui;
		if (Zui.button(text)) {
			g2_end(ui.g);
			Context.raw.material = SlotMaterial.create(Project.materials[0].data);
			Project.materials.push(Context.raw.material);
			TabMaterials.updateMaterial();
			g2_begin(ui.g, false);
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

	static updateMaterialPointers = (nodes: TNode[], i: i32) => {
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
