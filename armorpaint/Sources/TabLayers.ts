
class TabLayers {

	static layer_name_edit = -1;
	static layer_name_handle = zui_handle_create();
	static show_context_menu = false;

	static draw = (htab: zui_handle_t) => {
		let mini = Config.raw.layout[layout_size_t.SIDEBAR_W] <= UIBase.sidebar_mini_w;
		mini ? TabLayers.draw_mini(htab) : TabLayers.draw_full(htab);
	}

	static draw_mini = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		zui_set_hovered_tab_name(tr("Layers"));

		let _ELEMENT_H = ui.t.ELEMENT_H;
		ui.t.ELEMENT_H = Math.floor(UIBase.sidebar_mini_w / 2 / zui_SCALE(ui));

		zui_begin_sticky();
		zui_separator(5);

		TabLayers.combo_filter();
		TabLayers.button_2d_view();
		TabLayers.button_new("+");

		zui_end_sticky();
		ui._y += 2;

		TabLayers.highlight_odd_lines();
		TabLayers.draw_slots(true);

		ui.t.ELEMENT_H = _ELEMENT_H;
	}

	static draw_full = (htab: zui_handle_t) => {
		let ui = UIBase.ui;
		if (zui_tab(htab, tr("Layers"))) {
			zui_begin_sticky();
			zui_row([1 / 4, 1 / 4, 1 / 2]);

			TabLayers.button_new(tr("New"));
			TabLayers.button_2d_view();
			TabLayers.combo_filter();

			zui_end_sticky();
			ui._y += 2;

			TabLayers.highlight_odd_lines();
			TabLayers.draw_slots(false);
		}
	}

	static button_2d_view = () => {
		let ui = UIBase.ui;
		if (zui_button(tr("2D View"))) {
			UIBase.show_2d_view(view_2d_type_t.LAYER);
		}
		else if (ui.is_hovered) zui_tooltip(tr("Show 2D View") + ` (${Config.keymap.toggle_2d_view})`);
	}

	static draw_slots = (mini: bool) => {
		for (let i = 0; i < Project.layers.length; ++i) {
			if (i >= Project.layers.length) break; // Layer was deleted
			let j = Project.layers.length - 1 - i;
			let l = Project.layers[j];
			TabLayers.draw_layer_slot(l, j, mini);
		}
	}

	static highlight_odd_lines = () => {
		let ui = UIBase.ui;
		let step = ui.t.ELEMENT_H * 2;
		let fullH = ui._window_h - UIBase.hwnds[0].scroll_offset;
		for (let i = 0; i < Math.floor(fullH / step); ++i) {
			if (i % 2 == 0) {
				zui_fill(0, i * step, (ui._w / zui_SCALE(ui) - 2), step, ui.t.WINDOW_BG_COL - 0x00040404);
			}
		}
	}

	static button_new = (text: string) => {
		let ui = UIBase.ui;
		if (zui_button(text)) {
			UIMenu.draw((ui: zui_t) => {
				let l = Context.raw.layer;
				if (UIMenu.menu_button(ui, tr("Paint Layer"))) {
					Base.new_layer();
					History.new_layer();
				}
				if (UIMenu.menu_button(ui, tr("Fill Layer"))) {
					Base.create_fill_layer(uv_type_t.UVMAP);
				}
				if (UIMenu.menu_button(ui, tr("Decal Layer"))) {
					Base.create_fill_layer(uv_type_t.PROJECT);
				}
				if (UIMenu.menu_button(ui, tr("Black Mask"))) {
					if (SlotLayer.is_mask(l)) Context.set_layer(l.parent);
					// let l = Context.raw.layer;

					let m = Base.new_mask(false, l);
					let _next = () => {
						SlotLayer.clear(m, 0x00000000);
					}
					Base.notify_on_next_frame(_next);
					Context.raw.layer_preview_dirty = true;
					History.new_black_mask();
					Base.update_fill_layers();
				}
				if (UIMenu.menu_button(ui, tr("White Mask"))) {
					if (SlotLayer.is_mask(l)) Context.set_layer(l.parent);
					// let l = Context.raw.layer;

					let m = Base.new_mask(false, l);
					let _next = () => {
						SlotLayer.clear(m, 0xffffffff);
					}
					Base.notify_on_next_frame(_next);
					Context.raw.layer_preview_dirty = true;
					History.new_white_mask();
					Base.update_fill_layers();
				}
				if (UIMenu.menu_button(ui, tr("Fill Mask"))) {
					if (SlotLayer.is_mask(l)) Context.set_layer(l.parent);
					// let l = Context.raw.layer;

					let m = Base.new_mask(false, l);
					let _init = () => {
						SlotLayer.to_fill_layer(m);
					}
					app_notify_on_init(_init);
					Context.raw.layer_preview_dirty = true;
					History.new_fill_mask();
					Base.update_fill_layers();
				}
				ui.enabled = !SlotLayer.is_group(Context.raw.layer) && !SlotLayer.is_in_group(Context.raw.layer);
				if (UIMenu.menu_button(ui, tr("Group"))) {
					if (SlotLayer.is_group(l) || SlotLayer.is_in_group(l)) return;

					if (SlotLayer.is_layer_mask(l)) l = l.parent;

					let pointers = TabLayers.init_layer_map();
					let group = Base.new_group();
					Context.set_layer(l);
					array_remove(Project.layers, group);
					Project.layers.splice(Project.layers.indexOf(l) + 1, 0, group);
					l.parent = group;
					for (let m of Project.materials) TabLayers.remap_layer_pointers(m.canvas.nodes, TabLayers.fill_layer_map(pointers));
					Context.set_layer(group);
					History.new_group();
				}
				ui.enabled = true;
			}, 7);
		}
	}

	static combo_filter = () => {
		let ui = UIBase.ui;
		let ar = [tr("All")];
		for (let p of Project.paint_objects) ar.push(p.base.name);
		let atlases = Project.get_used_atlases();
		if (atlases != null) for (let a of atlases) ar.push(a);
		let filterHandle = zui_handle("tablayers_0");
		filterHandle.position = Context.raw.layer_filter;
		Context.raw.layer_filter = zui_combo(filterHandle, ar, tr("Filter"), false, zui_align_t.LEFT);
		if (filterHandle.changed) {
			for (let p of Project.paint_objects) {
				p.base.visible = Context.raw.layer_filter == 0 || p.base.name == ar[Context.raw.layer_filter] || Project.is_atlas_object(p);
			}
			if (Context.raw.layer_filter == 0 && Context.raw.merged_object_is_atlas) { // All
				UtilMesh.merge_mesh();
			}
			else if (Context.raw.layer_filter > Project.paint_objects.length) { // Atlas
				let visibles: mesh_object_t[] = [];
				for (let p of Project.paint_objects) if (p.base.visible) visibles.push(p);
				UtilMesh.merge_mesh(visibles);
			}
			Base.set_object_mask();
			UtilUV.uvmap_cached = false;
			Context.raw.ddirty = 2;
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			RenderPathRaytrace.ready = false;
			///end
		}
	}

	static remap_layer_pointers = (nodes: zui_node_t[], pointerMap: Map<i32, i32>) => {
		for (let n of nodes) {
			if (n.type == "LAYER" || n.type == "LAYER_MASK") {
				let i = n.buttons[0].default_value;
				if (pointerMap.has(i)) {
					n.buttons[0].default_value = pointerMap.get(i);
				}
			}
		}
	}

	static init_layer_map = (): Map<SlotLayerRaw, i32> => {
		let res: Map<SlotLayerRaw, i32> = new Map();
		for (let i = 0; i < Project.layers.length; ++i) res.set(Project.layers[i], i);
		return res;
	}

	static fill_layer_map = (map: Map<SlotLayerRaw, i32>): Map<i32, i32> => {
		let res: Map<i32, i32> = new Map();
		for (let l of map.keys()) res.set(map.get(l), Project.layers.indexOf(l) > -1 ? Project.layers.indexOf(l) : 9999);
		return res;
	}

	static set_drag_layer = (layer: SlotLayerRaw, offX: f32, offY: f32) => {
		Base.drag_off_x = offX;
		Base.drag_off_y = offY;
		Base.drag_layer = layer;
		Context.raw.drag_dest = Project.layers.indexOf(layer);
	}

	static draw_layer_slot = (l: SlotLayerRaw, i: i32, mini: bool) => {
		let ui = UIBase.ui;

		if (Context.raw.layer_filter > 0 &&
			SlotLayer.get_object_mask(l) > 0 &&
			SlotLayer.get_object_mask(l) != Context.raw.layer_filter) {
			return;
		}

		if (l.parent != null && !l.parent.show_panel) { // Group closed
			return;
		}
		if (l.parent != null && l.parent.parent != null && !l.parent.parent.show_panel) {
			return;
		}

		let step = ui.t.ELEMENT_H;
		let checkw = (ui._window_w / 100 * 8) / zui_SCALE(ui);

		// Highlight drag destination
		let absy = ui._window_y + ui._y;
		if (Base.is_dragging && Base.drag_layer != null && Context.in_layers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				let down = Project.layers.indexOf(Base.drag_layer) >= i;
				Context.raw.drag_dest = down ? i : i - 1;

				let ls = Project.layers;
				let dest = Context.raw.drag_dest;
				let toGroup = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
				let nestedGroup = SlotLayer.is_group(Base.drag_layer) && toGroup;
				if (!nestedGroup) {
					if (SlotLayer.can_move(Context.raw.layer, Context.raw.drag_dest)) {
						zui_fill(checkw, step * 2, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
					}
				}
			}
			else if (i == Project.layers.length - 1 && mouse_y < absy + step) {
				Context.raw.drag_dest = Project.layers.length - 1;
				if (SlotLayer.can_move(Context.raw.layer, Context.raw.drag_dest)) {
					zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
				}
			}
		}
		if (Base.is_dragging && (Base.drag_material != null || Base.drag_swatch != null) && Context.in_layers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				Context.raw.drag_dest = i;
				if (TabLayers.can_drop_new_layer(i))
					zui_fill(checkw, 2 * step, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
			}
			else if (i == Project.layers.length - 1 && mouse_y < absy + step) {
				Context.raw.drag_dest = Project.layers.length;
				if (TabLayers.can_drop_new_layer(Project.layers.length))
					zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
			}
		}

		mini ? TabLayers.draw_layer_slot_mini(l, i) : TabLayers.draw_layer_slot_full(l, i);

		TabLayers.draw_layer_highlight(l, mini);

		if (TabLayers.show_context_menu) {
			TabLayers.draw_layer_context_menu(l, mini);
		}
	}

	static draw_layer_slot_mini = (l: SlotLayerRaw, i: i32) => {
		let ui = UIBase.ui;

		zui_row([1, 1]);
		let uix = ui._x;
		let uiy = ui._y;
		let state = TabLayers.draw_layer_icon(l, i, uix, uiy, true);
		TabLayers.handle_layer_icon_state(l, i, state, uix, uiy);
		zui_end_element();

		ui._y += zui_ELEMENT_H(ui);
		ui._y -= zui_ELEMENT_OFFSET(ui);
	}

	static draw_layer_slot_full = (l: SlotLayerRaw, i: i32) => {
		let ui = UIBase.ui;

		let step = ui.t.ELEMENT_H;

		let hasPanel = SlotLayer.is_group(l) || (SlotLayer.is_layer(l) && SlotLayer.get_masks(l, false) != null);
		if (hasPanel) {
			zui_row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
		}
		else {
			zui_row([8 / 100, 16 / 100, 36 / 100, 30 / 100]);
		}

		// Draw eye icon
		let icons = Res.get("icons.k");
		let r = Res.tile18(icons, l.visible ? 0 : 1, 0);
		let center = (step / 2) * zui_SCALE(ui);
		ui._x += 2;
		ui._y += 3;
		ui._y += center;
		let col = ui.t.ACCENT_SELECT_COL;
		let parentHidden = l.parent != null && (!l.parent.visible || (l.parent.parent != null && !l.parent.parent.visible));
		if (parentHidden) col -= 0x99000000;

		if (zui_image(icons, col, -1.0, r.x, r.y, r.w, r.h) == zui_state_t.RELEASED) {
			TabLayers.layer_toggle_visible(l);
		}
		ui._x -= 2;
		ui._y -= 3;
		ui._y -= center;

		///if krom_opengl
		ui.image_invert_y = l.fill_layer != null;
		///end

		let uix = ui._x;
		let uiy = ui._y;
		ui._x += 2;
		ui._y += 3;
		if (l.parent != null) {
			ui._x += 10 * zui_SCALE(ui);
			if (l.parent.parent != null) ui._x += 10 * zui_SCALE(ui);
		}

		let state = TabLayers.draw_layer_icon(l, i, uix, uiy, false);

		ui._x -= 2;
		ui._y -= 3;

		if (Config.raw.touch_ui) {
			ui._x += 12 * zui_SCALE(ui);
		}

		///if krom_opengl
		ui.image_invert_y = false;
		///end

		TabLayers.handle_layer_icon_state(l, i, state, uix, uiy);

		// Draw layer name
		ui._y += center;
		if (TabLayers.layer_name_edit == l.id) {
			TabLayers.layer_name_handle.text = l.name;
			l.name = zui_text_input(TabLayers.layer_name_handle);
			if (ui.text_selected_handle_ptr != TabLayers.layer_name_handle.ptr) TabLayers.layer_name_edit = -1;
		}
		else {
			if (ui.enabled && ui.input_enabled && ui.combo_selected_handle_ptr == 0 &&
				ui.input_x > ui._window_x + ui._x && ui.input_x < ui._window_x + ui._window_w &&
				ui.input_y > ui._window_y + ui._y - center && ui.input_y < ui._window_y + ui._y - center + (step * zui_SCALE(ui)) * 2) {
				if (ui.input_started) {
					Context.set_layer(l);
					TabLayers.set_drag_layer(Context.raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
				}
				else if (ui.input_released_r) {
					Context.set_layer(l);
					TabLayers.show_context_menu = true;
				}
			}

			let state = zui_text(l.name);
			if (state == zui_state_t.RELEASED) {
				if (time_time() - Context.raw.select_time < 0.25) {
					TabLayers.layer_name_edit = l.id;
					TabLayers.layer_name_handle.text = l.name;
					zui_start_text_edit(TabLayers.layer_name_handle);
				}
				Context.raw.select_time = time_time();
			}

			let inFocus = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (inFocus && ui.is_delete_down && TabLayers.can_delete(Context.raw.layer)) {
				ui.is_delete_down = false;
				let _init = () => {
					TabLayers.delete_layer(Context.raw.layer);
				}
				app_notify_on_init(_init);
			}
		}
		ui._y -= center;

		if (l.parent != null) {
			ui._x -= 10 * zui_SCALE(ui);
			if (l.parent.parent != null) ui._x -= 10 * zui_SCALE(ui);
		}

		if (SlotLayer.is_group(l)) {
			zui_end_element();
		}
		else {
			if (SlotLayer.is_mask(l)) {
				ui._y += center;
			}

			TabLayers.combo_blending(ui, l);

			if (SlotLayer.is_mask(l)) {
				ui._y -= center;
			}
		}

		if (hasPanel) {
			ui._y += center;
			let layerPanel = zui_nest(zui_handle("tablayers_1"), l.id);
			layerPanel.selected = l.show_panel;
			l.show_panel = zui_panel(layerPanel, "", true, false, false);
			ui._y -= center;
		}

		if (SlotLayer.is_group(l) || SlotLayer.is_mask(l)) {
			ui._y -= zui_ELEMENT_OFFSET(ui);
			zui_end_element();
		}
		else {
			ui._y -= zui_ELEMENT_OFFSET(ui);

			zui_row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
			zui_end_element();
			zui_end_element();
			zui_end_element();

			if (Config.raw.touch_ui) {
				ui._x += 12 * zui_SCALE(ui);
			}

			TabLayers.combo_object(ui, l);
			zui_end_element();
		}

		ui._y -= zui_ELEMENT_OFFSET(ui);
	}

	static combo_object = (ui: zui_t, l: SlotLayerRaw, label = false): zui_handle_t => {
		let ar = [tr("Shared")];
		for (let p of Project.paint_objects) ar.push(p.base.name);
		let atlases = Project.get_used_atlases();
		if (atlases != null) for (let a of atlases) ar.push(a);
		let objectHandle = zui_nest(zui_handle("tablayers_2"), l.id);
		objectHandle.position = l.objectMask;
		l.objectMask = zui_combo(objectHandle, ar, tr("Object"), label, zui_align_t.LEFT);
		if (objectHandle.changed) {
			Context.set_layer(l);
			MakeMaterial.parse_mesh_material();
			if (l.fill_layer != null) { // Fill layer
				let _init = () => {
					Context.raw.material = l.fill_layer;
					SlotLayer.clear(l);
					Base.update_fill_layers();
				}
				app_notify_on_init(_init);
			}
			else {
				Base.set_object_mask();
			}
		}
		return objectHandle;
	}

	static combo_blending = (ui: zui_t, l: SlotLayerRaw, label = false): zui_handle_t => {
		let blendingHandle = zui_nest(zui_handle("tablayers_3"), l.id);
		blendingHandle.position = l.blending;
		zui_combo(blendingHandle, [
			tr("Mix"),
			tr("Darken"),
			tr("Multiply"),
			tr("Burn"),
			tr("Lighten"),
			tr("Screen"),
			tr("Dodge"),
			tr("Add"),
			tr("Overlay"),
			tr("Soft Light"),
			tr("Linear Light"),
			tr("Difference"),
			tr("Subtract"),
			tr("Divide"),
			tr("Hue"),
			tr("Saturation"),
			tr("Color"),
			tr("Value"),
		], tr("Blending"), label);
		if (blendingHandle.changed) {
			Context.set_layer(l);
			History.layer_blending();
			l.blending = blendingHandle.position;
			MakeMaterial.parse_mesh_material();
		}
		return blendingHandle;
	}

	static layer_toggle_visible = (l: SlotLayerRaw) => {
		l.visible = !l.visible;
		UIView2D.hwnd.redraws = 2;
		MakeMaterial.parse_mesh_material();
	}

	static draw_layer_highlight = (l: SlotLayerRaw, mini: bool) => {
		let ui = UIBase.ui;
		let step = ui.t.ELEMENT_H;

		// Separator line
		zui_fill(0, 0, (ui._w / zui_SCALE(ui) - 2), 1 * zui_SCALE(ui), ui.t.SEPARATOR_COL);

		// Highlight selected
		if (Context.raw.layer == l) {
			if (mini) {
				zui_rect(1, -step * 2, ui._w / zui_SCALE(ui) - 1, step * 2 + (mini ? -1 : 1), ui.t.HIGHLIGHT_COL, 3);
			}
			else {
				zui_rect(1, -step * 2 - 1, ui._w / zui_SCALE(ui) - 2, step * 2 + (mini ? -2 : 1), ui.t.HIGHLIGHT_COL, 2);
			}
		}
	}

	static handle_layer_icon_state = (l: SlotLayerRaw, i: i32, state: zui_state_t, uix: f32, uiy: f32) => {
		let ui = UIBase.ui;

		///if is_paint
		let texpaint_preview = l.texpaint_preview;
		///end
		///if is_sculpt
		let texpaint_preview = l.texpaint;
		///end

		TabLayers.show_context_menu = false;

		// Layer preview tooltip
		if (ui.is_hovered && texpaint_preview != null) {
			if (SlotLayer.is_mask(l)) {
				TabLayers.make_mask_preview_rgba32(l);
				zui_tooltip_image(Context.raw.mask_preview_rgba32);
			}
			else {
				zui_tooltip_image(texpaint_preview);
			}
			if (i < 9) zui_tooltip(l.name + " - (" + Config.keymap.select_layer + " " + (i + 1) + ")");
			else zui_tooltip(l.name);
		}

		// Show context menu
		if (ui.is_hovered && ui.input_released_r) {
			Context.set_layer(l);
			TabLayers.show_context_menu = true;
		}

		if (state == zui_state_t.STARTED) {
			Context.set_layer(l);
			TabLayers.set_drag_layer(Context.raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
		}
		else if (state == zui_state_t.RELEASED) {
			if (time_time() - Context.raw.select_time < 0.2) {
				UIBase.show_2d_view(view_2d_type_t.LAYER);
			}
			if (time_time() - Context.raw.select_time > 0.2) {
				Context.raw.select_time = time_time();
			}
			if (l.fill_layer != null) Context.set_material(l.fill_layer);
		}
	}

	static draw_layer_icon = (l: SlotLayerRaw, i: i32, uix: f32, uiy: f32, mini: bool) => {
		let ui = UIBase.ui;
		let icons = Res.get("icons.k");
		let iconH = (zui_ELEMENT_H(ui) - (mini ? 2 : 3)) * 2;

		if (mini && zui_SCALE(ui) > 1) {
			ui._x -= 1 * zui_SCALE(ui);
		}

		if (l.parent != null) {
			ui._x += (iconH - iconH * 0.9) / 2;
			iconH *= 0.9;
			if (l.parent.parent != null) {
				ui._x += (iconH - iconH * 0.9) / 2;
				iconH *= 0.9;
			}
		}

		if (!SlotLayer.is_group(l)) {
			///if is_paint
			let texpaint_preview = l.texpaint_preview;
			///end
			///if is_sculpt
			let texpaint_preview = l.texpaint;
			///end

			let icon = l.fill_layer == null ? texpaint_preview : l.fill_layer.image_icon;
			if (l.fill_layer == null) {
				// Checker
				let r = Res.tile50(icons, 4, 1);
				let _x = ui._x;
				let _y = ui._y;
				let _w = ui._w;
				zui_image(icons, 0xffffffff, iconH, r.x, r.y, r.w, r.h);
				ui.cur_ratio--;
				ui._x = _x;
				ui._y = _y;
				ui._w = _w;
			}
			if (l.fill_layer == null && SlotLayer.is_mask(l)) {
				g2_set_pipeline(UIView2D.pipe);
				///if krom_opengl
				krom_g4_set_pipeline(UIView2D.pipe.pipeline_);
				///end
				krom_g4_set_int(UIView2D.channel_location, 1);
			}

			let state = zui_image(icon, 0xffffffff, iconH);

			if (l.fill_layer == null && SlotLayer.is_mask(l)) {
				g2_set_pipeline(null);
			}

			// Draw layer numbers when selecting a layer via keyboard shortcut
			let isTyping = ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;
			if (!isTyping) {
				if (i < 9 && Operator.shortcut(Config.keymap.select_layer, ShortcutType.ShortcutDown)) {
					let number = String(i + 1) ;
					let width = g2_font_width(ui.font, ui.font_size, number) + 10;
					let height = g2_font_height(ui.font, ui.font_size);
					g2_set_color(ui.t.TEXT_COL);
					g2_fill_rect(uix, uiy, width, height);
					g2_set_color(ui.t.ACCENT_COL);
					g2_draw_string(number, uix + 5, uiy);
				}
			}

			return state;
		}
		else { // Group
			let folderClosed = Res.tile50(icons, 2, 1);
			let folderOpen = Res.tile50(icons, 8, 1);
			let folder = l.show_panel ? folderOpen : folderClosed;
			return zui_image(icons, ui.t.LABEL_COL - 0x00202020, iconH, folder.x, folder.y, folder.w, folder.h);
		}
	}

	static can_merge_down = (l: SlotLayerRaw) : bool => {
		let index = Project.layers.indexOf(l);
		// Lowest layer
		if (index == 0) return false;
		// Lowest layer that has masks
		if (SlotLayer.is_layer(l) && SlotLayer.is_mask(Project.layers[0]) && Project.layers[0].parent == l) return false;
		// The lowest toplevel layer is a group
		if (SlotLayer.is_group(l) && SlotLayer.is_in_group(Project.layers[0]) && SlotLayer.get_containing_group(Project.layers[0]) == l) return false;
		// Masks must be merged down to masks
		if (SlotLayer.is_mask(l) && !SlotLayer.is_mask(Project.layers[index - 1])) return false;
		return true;
	}

	static draw_layer_context_menu = (l: SlotLayerRaw, mini: bool) => {
		let add = 0;

		if (l.fill_layer == null) add += 1; // Clear
		if (l.fill_layer != null && !SlotLayer.is_mask(l)) add += 3;
		if (l.fill_layer != null && SlotLayer.is_mask(l)) add += 2;
		if (SlotLayer.is_mask(l)) add += 2;
		if (mini) {
			add += 1;
			if (!SlotLayer.is_group(l)) add += 1;
			if (SlotLayer.is_layer(l)) add += 1;
		}
		let menuElements = SlotLayer.is_group(l) ? 7 : (19 + add);

		UIMenu.draw((ui: zui_t) => {

			if (mini) {
				let visibleHandle = zui_handle("tablayers_4");
				visibleHandle.selected = l.visible;
				UIMenu.menu_fill(ui);
				zui_check(visibleHandle, tr("Visible"));
				if (visibleHandle.changed) {
					TabLayers.layer_toggle_visible(l);
					UIMenu.keep_open = true;
				}

				if (!SlotLayer.is_group(l)) {
					UIMenu.menu_fill(ui);
					if (TabLayers.combo_blending(ui, l, true).changed) {
						UIMenu.keep_open = true;
					}
				}
				if (SlotLayer.is_layer(l)) {
					UIMenu.menu_fill(ui);
					if (TabLayers.combo_object(ui, l, true).changed) {
						UIMenu.keep_open = true;
					}
				}
			}

			if (UIMenu.menu_button(ui, tr("Export"))) {
				if (SlotLayer.is_mask(l)) {
					UIFiles.show("png", true, false, (path: string) => {
						let f = UIFiles.filename;
						if (f == "") f = tr("untitled");
						if (!f.endsWith(".png")) f += ".png";
						krom_write_png(path + Path.sep + f, image_get_pixels(l.texpaint), l.texpaint.width, l.texpaint.height, 3); // RRR1
					});
				}
				else {
					///if is_paint
					Context.raw.layers_export = export_mode_t.SELECTED;
					BoxExport.show_textures();
					///end
				}
			}

			if (!SlotLayer.is_group(l)) {
				let toFillString = SlotLayer.is_layer(l) ? tr("To Fill Layer") : tr("To Fill Mask");
				let toPaintString = SlotLayer.is_layer(l) ? tr("To Paint Layer") : tr("To Paint Mask");

				if (l.fill_layer == null && UIMenu.menu_button(ui, toFillString)) {
					let _init = () => {
						SlotLayer.is_layer(l) ? History.to_fill_layer() : History.to_fill_mask();
						SlotLayer.to_fill_layer(l);
					}
					app_notify_on_init(_init);
				}
				if (l.fill_layer != null && UIMenu.menu_button(ui, toPaintString)) {
					let _init = () => {
						SlotLayer.is_layer(l) ? History.to_paint_layer() : History.to_paint_mask();
						SlotLayer.to_paint_layer(l);
					}
					app_notify_on_init(_init);
				}
			}

			ui.enabled = TabLayers.can_delete(l);
			if (UIMenu.menu_button(ui, tr("Delete"), "delete")) {
				let _init = () => {
					TabLayers.delete_layer(Context.raw.layer);
				}
				app_notify_on_init(_init);
			}
			ui.enabled = true;

			if (l.fill_layer == null && UIMenu.menu_button(ui, tr("Clear"))) {
				Context.set_layer(l);
				let _init = () => {
					if (!SlotLayer.is_group(l)) {
						History.clear_layer();
						SlotLayer.clear(l);
					}
					else {
						for (let c of SlotLayer.get_children(l)) {
							Context.raw.layer = c;
							History.clear_layer();
							SlotLayer.clear(c);
						}
						Context.raw.layers_preview_dirty = true;
						Context.raw.layer = l;
					}
				}
				app_notify_on_init(_init);
			}
			if (SlotLayer.is_mask(l) && l.fill_layer == null && UIMenu.menu_button(ui, tr("Invert"))) {
				let _init = () => {
					Context.set_layer(l);
					History.invert_mask();
					SlotLayer.invert_mask(l);
				}
				app_notify_on_init(_init);
			}
			if (SlotLayer.is_mask(l) && UIMenu.menu_button(ui, tr("Apply"))) {
				let _init = () => {
					Context.raw.layer = l;
					History.apply_mask();
					SlotLayer.apply_mask(l);
					Context.set_layer(l.parent);
					MakeMaterial.parse_mesh_material();
					Context.raw.layers_preview_dirty = true;
				}
				app_notify_on_init(_init);
			}
			if (SlotLayer.is_group(l) && UIMenu.menu_button(ui, tr("Merge Group"))) {
				let _init = () => {
					Base.merge_group(l);
				}
				app_notify_on_init(_init);
			}
			ui.enabled = TabLayers.can_merge_down(l);
			if (UIMenu.menu_button(ui, tr("Merge Down"))) {
				let _init = () => {
					Context.set_layer(l);
					History.merge_layers();
					Base.merge_down();
					if (Context.raw.layer.fill_layer != null) SlotLayer.to_paint_layer(Context.raw.layer);
				}
				app_notify_on_init(_init);
			}
			ui.enabled = true;
			if (UIMenu.menu_button(ui, tr("Duplicate"))) {
				let _init = () => {
					Context.set_layer(l);
					History.duplicate_layer();
					Base.duplicate_layer(l);
				}
				app_notify_on_init(_init);
			}

			UIMenu.menu_fill(ui);
			UIMenu.menu_align(ui);
			let layerOpacHandle = zui_nest(zui_handle("tablayers_5"), l.id);
			layerOpacHandle.value = l.maskOpacity;
			zui_slider(layerOpacHandle, tr("Opacity"), 0.0, 1.0, true);
			if (layerOpacHandle.changed) {
				if (ui.input_started) History.layer_opacity();
				l.maskOpacity = layerOpacHandle.value;
				MakeMaterial.parse_mesh_material();
				UIMenu.keep_open = true;
			}

			if (!SlotLayer.is_group(l)) {
				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let resHandleChangedLast = Base.res_handle.changed;
				///if (krom_android || krom_ios)
				let ar = ["128", "256", "512", "1K", "2K", "4K"];
				///else
				let ar = ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"];
				///end
				let _y = ui._y;
				Base.res_handle.value = Base.res_handle.position;
				Base.res_handle.position = Math.floor(zui_slider(Base.res_handle, ar[Base.res_handle.position], 0, ar.length - 1, false, 1, false, zui_align_t.LEFT, false));
				if (Base.res_handle.changed) {
					UIMenu.keep_open = true;
				}
				if (resHandleChangedLast && !Base.res_handle.changed) {
					Base.on_layers_resized();
				}
				ui._y = _y;
				zui_draw_string(tr("Res"), null, 0, zui_align_t.RIGHT);
				zui_end_element();

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				///if (krom_android || krom_ios)
				zui_inline_radio(Base.bits_handle, ["8bit"]);
				///else
				zui_inline_radio(Base.bits_handle, ["8bit", "16bit", "32bit"]);
				///end
				if (Base.bits_handle.changed) {
					app_notify_on_init(Base.set_layer_bits);
					UIMenu.keep_open = true;
				}
			}

			if (l.fill_layer != null) {
				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let scaleHandle = zui_nest(zui_handle("tablayers_6"), l.id);
				scaleHandle.value = l.scale;
				l.scale = zui_slider(scaleHandle, tr("UV Scale"), 0.0, 5.0, true);
				if (scaleHandle.changed) {
					Context.set_material(l.fill_layer);
					Context.set_layer(l);
					let _init = () => {
						Base.update_fill_layers();
					}
					app_notify_on_init(_init);
					UIMenu.keep_open = true;
				}

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let angleHandle = zui_nest(zui_handle("tablayers_7"), l.id);
				angleHandle.value = l.angle;
				l.angle = zui_slider(angleHandle, tr("Angle"), 0.0, 360, true, 1);
				if (angleHandle.changed) {
					Context.set_material(l.fill_layer);
					Context.set_layer(l);
					MakeMaterial.parse_paint_material();
					let _init = () => {
						Base.update_fill_layers();
					}
					app_notify_on_init(_init);
					UIMenu.keep_open = true;
				}

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let uvTypeHandle = zui_nest(zui_handle("tablayers_8"), l.id);
				uvTypeHandle.position = l.uvType;
				l.uvType = zui_inline_radio(uvTypeHandle, [tr("UV Map"), tr("Triplanar"), tr("Project")], zui_align_t.LEFT);
				if (uvTypeHandle.changed) {
					Context.set_material(l.fill_layer);
					Context.set_layer(l);
					MakeMaterial.parse_paint_material();
					let _init = () => {
						Base.update_fill_layers();
					}
					app_notify_on_init(_init);
					UIMenu.keep_open = true;
				}
			}

			if (!SlotLayer.is_group(l)) {
				let baseHandle = zui_nest(zui_handle("tablayers_9"), l.id);
				let opacHandle = zui_nest(zui_handle("tablayers_10"), l.id);
				let norHandle = zui_nest(zui_handle("tablayers_11"), l.id);
				let norBlendHandle = zui_nest(zui_handle("tablayers_12"), l.id);
				let occHandle = zui_nest(zui_handle("tablayers_13"), l.id);
				let roughHandle = zui_nest(zui_handle("tablayers_14"), l.id);
				let metHandle = zui_nest(zui_handle("tablayers_15"), l.id);
				let heightHandle = zui_nest(zui_handle("tablayers_16"), l.id);
				let heightBlendHandle = zui_nest(zui_handle("tablayers_17"), l.id);
				let emisHandle = zui_nest(zui_handle("tablayers_18"), l.id);
				let subsHandle = zui_nest(zui_handle("tablayers_19"), l.id);
				baseHandle.selected = l.paintBase;
				opacHandle.selected = l.paintOpac;
				norHandle.selected = l.paintNor;
				norBlendHandle.selected = l.paintNorBlend;
				occHandle.selected = l.paintOcc;
				roughHandle.selected = l.paintRough;
				metHandle.selected = l.paintMet;
				heightHandle.selected = l.paintHeight;
				heightBlendHandle.selected = l.paintHeightBlend;
				emisHandle.selected = l.paintEmis;
				subsHandle.selected = l.paintSubs;
				UIMenu.menu_fill(ui);
				l.paintBase = zui_check(baseHandle, tr("Base Color"));
				UIMenu.menu_fill(ui);
				l.paintOpac = zui_check(opacHandle, tr("Opacity"));
				UIMenu.menu_fill(ui);
				l.paintNor = zui_check(norHandle, tr("Normal"));
				UIMenu.menu_fill(ui);
				l.paintNorBlend = zui_check(norBlendHandle, tr("Normal Blending"));
				UIMenu.menu_fill(ui);
				l.paintOcc = zui_check(occHandle, tr("Occlusion"));
				UIMenu.menu_fill(ui);
				l.paintRough = zui_check(roughHandle, tr("Roughness"));
				UIMenu.menu_fill(ui);
				l.paintMet = zui_check(metHandle, tr("Metallic"));
				UIMenu.menu_fill(ui);
				l.paintHeight = zui_check(heightHandle, tr("Height"));
				UIMenu.menu_fill(ui);
				l.paintHeightBlend = zui_check(heightBlendHandle, tr("Height Blending"));
				UIMenu.menu_fill(ui);
				l.paintEmis = zui_check(emisHandle, tr("Emission"));
				UIMenu.menu_fill(ui);
				l.paintSubs = zui_check(subsHandle, tr("Subsurface"));
				if (baseHandle.changed ||
					opacHandle.changed ||
					norHandle.changed ||
					norBlendHandle.changed ||
					occHandle.changed ||
					roughHandle.changed ||
					metHandle.changed ||
					heightHandle.changed ||
					heightBlendHandle.changed ||
					emisHandle.changed ||
					subsHandle.changed) {
					MakeMaterial.parse_mesh_material();
					UIMenu.keep_open = true;
				}
			}
		}, menuElements);
	}

	static make_mask_preview_rgba32 = (l: SlotLayerRaw) => {
		///if is_paint
		if (Context.raw.mask_preview_rgba32 == null) {
			Context.raw.mask_preview_rgba32 = image_create_render_target(UtilRender.layer_preview_size, UtilRender.layer_preview_size);
		}
		// Convert from R8 to RGBA32 for tooltip display
		if (Context.raw.mask_preview_last != l) {
			Context.raw.mask_preview_last = l;
			app_notify_on_init(() => {
				g2_begin(Context.raw.mask_preview_rgba32);
				g2_set_pipeline(UIView2D.pipe);
				g4_set_int(UIView2D.channel_location, 1);
				g2_draw_image(l.texpaint_preview, 0, 0);
				g2_end();
				g2_set_pipeline(null);
			});
		}
		///end
	}

	static delete_layer = (l: SlotLayerRaw) => {
		let pointers = TabLayers.init_layer_map();

		if (SlotLayer.is_layer(l) && SlotLayer.has_masks(l, false)) {
			for (let m of SlotLayer.get_masks(l, false)) {
				Context.raw.layer = m;
				History.delete_layer();
				SlotLayer.delete(m);
			}
		}
		if (SlotLayer.is_group(l)) {
			for (let c of SlotLayer.get_children(l)) {
				if (SlotLayer.has_masks(c, false)) {
					for (let m of SlotLayer.get_masks(c, false)) {
						Context.raw.layer = m;
						History.delete_layer();
						SlotLayer.delete(m);
					}
				}
				Context.raw.layer = c;
				History.delete_layer();
				SlotLayer.delete(c);
			}
			if (SlotLayer.has_masks(l)) {
				for (let m of SlotLayer.get_masks(l)) {
					Context.raw.layer = m;
					History.delete_layer();
					SlotLayer.delete(m);
				}
			}
		}

		Context.raw.layer = l;
		History.delete_layer();
		SlotLayer.delete(l);

		if (SlotLayer.is_mask(l)) {
			Context.raw.layer = l.parent;
			Base.update_fill_layers();
		}

		// Remove empty group
		if (SlotLayer.is_in_group(l) && SlotLayer.get_children(SlotLayer.get_containing_group(l)) == null) {
			let g = SlotLayer.get_containing_group(l);
			// Maybe some group masks are left
			if (SlotLayer.has_masks(g)) {
				for (let m of SlotLayer.get_masks(g)) {
					Context.raw.layer = m;
					History.delete_layer();
					SlotLayer.delete(m);
				}
			}
			Context.raw.layer = l.parent;
			History.delete_layer();
			SlotLayer.delete(l.parent);
		}
		Context.raw.ddirty = 2;
		for (let m of Project.materials) TabLayers.remap_layer_pointers(m.canvas.nodes, TabLayers.fill_layer_map(pointers));
	}

	static can_delete = (l: SlotLayerRaw) => {
		let numLayers = 0;

		if (SlotLayer.is_mask(l)) return true;

		for (let slot of Project.layers) {
			if (SlotLayer.is_layer(slot)) ++numLayers;
		}

		// All layers are in one group
		if (SlotLayer.is_group(l) && SlotLayer.get_children(l).length == numLayers) return false;

		// Do not delete last layer
		return numLayers > 1;
	}

	static can_drop_new_layer = (position: i32) => {
		if (position > 0 && position < Project.layers.length && SlotLayer.is_mask(Project.layers[position - 1])) {
			// 1. The layer to insert is inserted in the middle
			// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
			return false;
		}
		return true;
	}
}
