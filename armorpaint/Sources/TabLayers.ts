
class TabLayers {

	static layer_name_edit: i32 = -1;
	static layer_name_handle: zui_handle_t = zui_handle_create();
	static show_context_menu: bool = false;

	static draw = (htab: zui_handle_t) => {
		let mini: bool = config_raw.layout[layout_size_t.SIDEBAR_W] <= UIBase.sidebar_mini_w;
		mini ? TabLayers.draw_mini(htab) : TabLayers.draw_full(htab);
	}

	static draw_mini = (htab: zui_handle_t) => {
		let ui: zui_t = UIBase.ui;
		zui_set_hovered_tab_name(tr("Layers"));

		let _ELEMENT_H: i32 = ui.t.ELEMENT_H;
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
		let ui: zui_t = UIBase.ui;
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
		let ui: zui_t = UIBase.ui;
		if (zui_button(tr("2D View"))) {
			UIBase.show_2d_view(view_2d_type_t.LAYER);
		}
		else if (ui.is_hovered) zui_tooltip(tr("Show 2D View") + ` (${config_keymap.toggle_2d_view})`);
	}

	static draw_slots = (mini: bool) => {
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			if (i >= project_layers.length) break; // Layer was deleted
			let j: i32 = project_layers.length - 1 - i;
			let l: SlotLayerRaw = project_layers[j];
			TabLayers.draw_layer_slot(l, j, mini);
		}
	}

	static highlight_odd_lines = () => {
		let ui: zui_t = UIBase.ui;
		let step: i32 = ui.t.ELEMENT_H * 2;
		let full_h: i32 = ui._window_h - UIBase.hwnds[0].scroll_offset;
		for (let i: i32 = 0; i < Math.floor(full_h / step); ++i) {
			if (i % 2 == 0) {
				zui_fill(0, i * step, (ui._w / zui_SCALE(ui) - 2), step, ui.t.WINDOW_BG_COL - 0x00040404);
			}
		}
	}

	static button_new = (text: string) => {
		if (zui_button(text)) {
			UIMenu.draw((ui: zui_t) => {
				let l: SlotLayerRaw = context_raw.layer;
				if (UIMenu.menu_button(ui, tr("Paint Layer"))) {
					base_new_layer();
					history_new_layer();
				}
				if (UIMenu.menu_button(ui, tr("Fill Layer"))) {
					base_create_fill_layer(uv_type_t.UVMAP);
				}
				if (UIMenu.menu_button(ui, tr("Decal Layer"))) {
					base_create_fill_layer(uv_type_t.PROJECT);
				}
				if (UIMenu.menu_button(ui, tr("Black Mask"))) {
					if (SlotLayer.is_mask(l)) context_set_layer(l.parent);
					// let l: SlotLayerRaw = raw.layer;

					let m: SlotLayerRaw = base_new_mask(false, l);
					let _next = () => {
						SlotLayer.clear(m, 0x00000000);
					}
					base_notify_on_next_frame(_next);
					context_raw.layer_preview_dirty = true;
					history_new_black_mask();
					base_update_fill_layers();
				}
				if (UIMenu.menu_button(ui, tr("White Mask"))) {
					if (SlotLayer.is_mask(l)) context_set_layer(l.parent);
					// let l: SlotLayerRaw = raw.layer;

					let m: SlotLayerRaw = base_new_mask(false, l);
					let _next = () => {
						SlotLayer.clear(m, 0xffffffff);
					}
					base_notify_on_next_frame(_next);
					context_raw.layer_preview_dirty = true;
					history_new_white_mask();
					base_update_fill_layers();
				}
				if (UIMenu.menu_button(ui, tr("Fill Mask"))) {
					if (SlotLayer.is_mask(l)) context_set_layer(l.parent);
					// let l: SlotLayerRaw = raw.layer;

					let m: SlotLayerRaw = base_new_mask(false, l);
					let _init = () => {
						SlotLayer.to_fill_layer(m);
					}
					app_notify_on_init(_init);
					context_raw.layer_preview_dirty = true;
					history_new_fill_mask();
					base_update_fill_layers();
				}
				ui.enabled = !SlotLayer.is_group(context_raw.layer) && !SlotLayer.is_in_group(context_raw.layer);
				if (UIMenu.menu_button(ui, tr("Group"))) {
					if (SlotLayer.is_group(l) || SlotLayer.is_in_group(l)) return;

					if (SlotLayer.is_layer_mask(l)) l = l.parent;

					let pointers: map_t<SlotLayerRaw, i32> = TabLayers.init_layer_map();
					let group = base_new_group();
					context_set_layer(l);
					array_remove(project_layers, group);
					project_layers.splice(project_layers.indexOf(l) + 1, 0, group);
					l.parent = group;
					for (let m of project_materials) TabLayers.remap_layer_pointers(m.canvas.nodes, TabLayers.fill_layer_map(pointers));
					context_set_layer(group);
					history_new_group();
				}
				ui.enabled = true;
			}, 7);
		}
	}

	static combo_filter = () => {
		let ar: string[] = [tr("All")];
		for (let p of project_paint_objects) ar.push(p.base.name);
		let atlases: string[] = project_get_used_atlases();
		if (atlases != null) for (let a of atlases) ar.push(a);
		let filter_handle: zui_handle_t = zui_handle("tablayers_0");
		filter_handle.position = context_raw.layer_filter;
		context_raw.layer_filter = zui_combo(filter_handle, ar, tr("Filter"), false, zui_align_t.LEFT);
		if (filter_handle.changed) {
			for (let p of project_paint_objects) {
				p.base.visible = context_raw.layer_filter == 0 || p.base.name == ar[context_raw.layer_filter] || project_is_atlas_object(p);
			}
			if (context_raw.layer_filter == 0 && context_raw.merged_object_is_atlas) { // All
				UtilMesh.merge_mesh();
			}
			else if (context_raw.layer_filter > project_paint_objects.length) { // Atlas
				let visibles: mesh_object_t[] = [];
				for (let p of project_paint_objects) if (p.base.visible) visibles.push(p);
				UtilMesh.merge_mesh(visibles);
			}
			base_set_object_mask();
			UtilUV.uvmap_cached = false;
			context_raw.ddirty = 2;
			///if (krom_direct3d12 || krom_vulkan || krom_metal)
			RenderPathRaytrace.ready = false;
			///end
		}
	}

	static remap_layer_pointers = (nodes: zui_node_t[], pointerMap: Map<i32, i32>) => {
		for (let n of nodes) {
			if (n.type == "LAYER" || n.type == "LAYER_MASK") {
				let i: any = n.buttons[0].default_value;
				if (pointerMap.has(i)) {
					n.buttons[0].default_value = pointerMap.get(i);
				}
			}
		}
	}

	static init_layer_map = (): Map<SlotLayerRaw, i32> => {
		let res: Map<SlotLayerRaw, i32> = new Map();
		for (let i: i32 = 0; i < project_layers.length; ++i) res.set(project_layers[i], i);
		return res;
	}

	static fill_layer_map = (map: Map<SlotLayerRaw, i32>): Map<i32, i32> => {
		let res: Map<i32, i32> = new Map();
		for (let l of map.keys()) res.set(map.get(l), project_layers.indexOf(l) > -1 ? project_layers.indexOf(l) : 9999);
		return res;
	}

	static set_drag_layer = (layer: SlotLayerRaw, offX: f32, offY: f32) => {
		base_drag_off_x = offX;
		base_drag_off_y = offY;
		base_drag_layer = layer;
		context_raw.drag_dest = project_layers.indexOf(layer);
	}

	static draw_layer_slot = (l: SlotLayerRaw, i: i32, mini: bool) => {
		let ui: zui_t = UIBase.ui;

		if (context_raw.layer_filter > 0 &&
			SlotLayer.get_object_mask(l) > 0 &&
			SlotLayer.get_object_mask(l) != context_raw.layer_filter) {
			return;
		}

		if (l.parent != null && !l.parent.show_panel) { // Group closed
			return;
		}
		if (l.parent != null && l.parent.parent != null && !l.parent.parent.show_panel) {
			return;
		}

		let step: i32 = ui.t.ELEMENT_H;
		let checkw: f32 = (ui._window_w / 100 * 8) / zui_SCALE(ui);

		// Highlight drag destination
		let absy: f32 = ui._window_y + ui._y;
		if (base_is_dragging && base_drag_layer != null && context_in_layers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				let down: bool = project_layers.indexOf(base_drag_layer) >= i;
				context_raw.drag_dest = down ? i : i - 1;

				let ls: SlotLayerRaw[] = project_layers;
				let dest: i32 = context_raw.drag_dest;
				let to_group: bool = down ? dest > 0 && ls[dest - 1].parent != null && ls[dest - 1].parent.show_panel : dest < ls.length && ls[dest].parent != null && ls[dest].parent.show_panel;
				let nested_group: bool = SlotLayer.is_group(base_drag_layer) && to_group;
				if (!nested_group) {
					if (SlotLayer.can_move(context_raw.layer, context_raw.drag_dest)) {
						zui_fill(checkw, step * 2, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
					}
				}
			}
			else if (i == project_layers.length - 1 && mouse_y < absy + step) {
				context_raw.drag_dest = project_layers.length - 1;
				if (SlotLayer.can_move(context_raw.layer, context_raw.drag_dest)) {
					zui_fill(checkw, 0, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
				}
			}
		}
		if (base_is_dragging && (base_drag_material != null || base_drag_swatch != null) && context_in_layers()) {
			if (mouse_y > absy + step && mouse_y < absy + step * 3) {
				context_raw.drag_dest = i;
				if (TabLayers.can_drop_new_layer(i))
					zui_fill(checkw, 2 * step, (ui._window_w / zui_SCALE(ui) - 2) - checkw, 2 * zui_SCALE(ui), ui.t.HIGHLIGHT_COL);
			}
			else if (i == project_layers.length - 1 && mouse_y < absy + step) {
				context_raw.drag_dest = project_layers.length;
				if (TabLayers.can_drop_new_layer(project_layers.length))
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
		let uix: f32 = ui._x;
		let uiy: f32 = ui._y;
		let state: zui_state_t = TabLayers.draw_layer_icon(l, i, uix, uiy, true);
		TabLayers.handle_layer_icon_state(l, i, state, uix, uiy);
		zui_end_element();

		ui._y += zui_ELEMENT_H(ui);
		ui._y -= zui_ELEMENT_OFFSET(ui);
	}

	static draw_layer_slot_full = (l: SlotLayerRaw, i: i32) => {
		let ui: zui_t = UIBase.ui;

		let step: i32 = ui.t.ELEMENT_H;

		let has_panel: bool = SlotLayer.is_group(l) || (SlotLayer.is_layer(l) && SlotLayer.get_masks(l, false) != null);
		if (has_panel) {
			zui_row([8 / 100, 16 / 100, 36 / 100, 30 / 100, 10 / 100]);
		}
		else {
			zui_row([8 / 100, 16 / 100, 36 / 100, 30 / 100]);
		}

		// Draw eye icon
		let icons: image_t = resource_get("icons.k");
		let r: rect_t = resource_tile18(icons, l.visible ? 0 : 1, 0);
		let center: f32 = (step / 2) * zui_SCALE(ui);
		ui._x += 2;
		ui._y += 3;
		ui._y += center;
		let col: i32 = ui.t.ACCENT_SELECT_COL;
		let parent_hidden: bool = l.parent != null && (!l.parent.visible || (l.parent.parent != null && !l.parent.parent.visible));
		if (parent_hidden) col -= 0x99000000;

		if (zui_image(icons, col, -1.0, r.x, r.y, r.w, r.h) == zui_state_t.RELEASED) {
			TabLayers.layer_toggle_visible(l);
		}
		ui._x -= 2;
		ui._y -= 3;
		ui._y -= center;

		///if krom_opengl
		ui.image_invert_y = l.fill_layer != null;
		///end

		let uix: f32 = ui._x;
		let uiy: f32 = ui._y;
		ui._x += 2;
		ui._y += 3;
		if (l.parent != null) {
			ui._x += 10 * zui_SCALE(ui);
			if (l.parent.parent != null) ui._x += 10 * zui_SCALE(ui);
		}

		let state: zui_state_t = TabLayers.draw_layer_icon(l, i, uix, uiy, false);

		ui._x -= 2;
		ui._y -= 3;

		if (config_raw.touch_ui) {
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
					context_set_layer(l);
					TabLayers.set_drag_layer(context_raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
				}
				else if (ui.input_released_r) {
					context_set_layer(l);
					TabLayers.show_context_menu = true;
				}
			}

			let state: zui_state_t = zui_text(l.name);
			if (state == zui_state_t.RELEASED) {
				if (time_time() - context_raw.select_time < 0.25) {
					TabLayers.layer_name_edit = l.id;
					TabLayers.layer_name_handle.text = l.name;
					zui_start_text_edit(TabLayers.layer_name_handle);
				}
				context_raw.select_time = time_time();
			}

			let in_focus: bool = ui.input_x > ui._window_x && ui.input_x < ui._window_x + ui._window_w &&
						  		 ui.input_y > ui._window_y && ui.input_y < ui._window_y + ui._window_h;
			if (in_focus && ui.is_delete_down && TabLayers.can_delete(context_raw.layer)) {
				ui.is_delete_down = false;
				let _init = () => {
					TabLayers.delete_layer(context_raw.layer);
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

		if (has_panel) {
			ui._y += center;
			let layer_panel: zui_handle_t = zui_nest(zui_handle("tablayers_1"), l.id);
			layer_panel.selected = l.show_panel;
			l.show_panel = zui_panel(layer_panel, "", true, false, false);
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

			if (config_raw.touch_ui) {
				ui._x += 12 * zui_SCALE(ui);
			}

			TabLayers.combo_object(ui, l);
			zui_end_element();
		}

		ui._y -= zui_ELEMENT_OFFSET(ui);
	}

	static combo_object = (ui: zui_t, l: SlotLayerRaw, label = false): zui_handle_t => {
		let ar: string[] = [tr("Shared")];
		for (let p of project_paint_objects) ar.push(p.base.name);
		let atlases: string[] = project_get_used_atlases();
		if (atlases != null) for (let a of atlases) ar.push(a);
		let object_handle: zui_handle_t = zui_nest(zui_handle("tablayers_2"), l.id);
		object_handle.position = l.object_mask;
		l.object_mask = zui_combo(object_handle, ar, tr("Object"), label, zui_align_t.LEFT);
		if (object_handle.changed) {
			context_set_layer(l);
			MakeMaterial.parse_mesh_material();
			if (l.fill_layer != null) { // Fill layer
				let _init = () => {
					context_raw.material = l.fill_layer;
					SlotLayer.clear(l);
					base_update_fill_layers();
				}
				app_notify_on_init(_init);
			}
			else {
				base_set_object_mask();
			}
		}
		return object_handle;
	}

	static combo_blending = (ui: zui_t, l: SlotLayerRaw, label = false): zui_handle_t => {
		let blending_handle: zui_handle_t = zui_nest(zui_handle("tablayers_3"), l.id);
		blending_handle.position = l.blending;
		zui_combo(blending_handle, [
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
		if (blending_handle.changed) {
			context_set_layer(l);
			history_layer_blending();
			l.blending = blending_handle.position;
			MakeMaterial.parse_mesh_material();
		}
		return blending_handle;
	}

	static layer_toggle_visible = (l: SlotLayerRaw) => {
		l.visible = !l.visible;
		UIView2D.hwnd.redraws = 2;
		MakeMaterial.parse_mesh_material();
	}

	static draw_layer_highlight = (l: SlotLayerRaw, mini: bool) => {
		let ui: zui_t = UIBase.ui;
		let step: i32 = ui.t.ELEMENT_H;

		// Separator line
		zui_fill(0, 0, (ui._w / zui_SCALE(ui) - 2), 1 * zui_SCALE(ui), ui.t.SEPARATOR_COL);

		// Highlight selected
		if (context_raw.layer == l) {
			if (mini) {
				zui_rect(1, -step * 2, ui._w / zui_SCALE(ui) - 1, step * 2 + (mini ? -1 : 1), ui.t.HIGHLIGHT_COL, 3);
			}
			else {
				zui_rect(1, -step * 2 - 1, ui._w / zui_SCALE(ui) - 2, step * 2 + (mini ? -2 : 1), ui.t.HIGHLIGHT_COL, 2);
			}
		}
	}

	static handle_layer_icon_state = (l: SlotLayerRaw, i: i32, state: zui_state_t, uix: f32, uiy: f32) => {
		let ui: zui_t = UIBase.ui;

		///if is_paint
		let texpaint_preview: image_t = l.texpaint_preview;
		///end
		///if is_sculpt
		let texpaint_preview: image_t = l.texpaint;
		///end

		TabLayers.show_context_menu = false;

		// Layer preview tooltip
		if (ui.is_hovered && texpaint_preview != null) {
			if (SlotLayer.is_mask(l)) {
				TabLayers.make_mask_preview_rgba32(l);
				zui_tooltip_image(context_raw.mask_preview_rgba32);
			}
			else {
				zui_tooltip_image(texpaint_preview);
			}
			if (i < 9) zui_tooltip(l.name + " - (" + config_keymap.select_layer + " " + (i + 1) + ")");
			else zui_tooltip(l.name);
		}

		// Show context menu
		if (ui.is_hovered && ui.input_released_r) {
			context_set_layer(l);
			TabLayers.show_context_menu = true;
		}

		if (state == zui_state_t.STARTED) {
			context_set_layer(l);
			TabLayers.set_drag_layer(context_raw.layer, -(mouse_x - uix - ui._window_x - 3), -(mouse_y - uiy - ui._window_y + 1));
		}
		else if (state == zui_state_t.RELEASED) {
			if (time_time() - context_raw.select_time < 0.2) {
				UIBase.show_2d_view(view_2d_type_t.LAYER);
			}
			if (time_time() - context_raw.select_time > 0.2) {
				context_raw.select_time = time_time();
			}
			if (l.fill_layer != null) context_set_material(l.fill_layer);
		}
	}

	static draw_layer_icon = (l: SlotLayerRaw, i: i32, uix: f32, uiy: f32, mini: bool) => {
		let ui: zui_t = UIBase.ui;
		let icons: image_t = resource_get("icons.k");
		let icon_h: i32 = (zui_ELEMENT_H(ui) - (mini ? 2 : 3)) * 2;

		if (mini && zui_SCALE(ui) > 1) {
			ui._x -= 1 * zui_SCALE(ui);
		}

		if (l.parent != null) {
			ui._x += (icon_h - icon_h * 0.9) / 2;
			icon_h *= 0.9;
			if (l.parent.parent != null) {
				ui._x += (icon_h - icon_h * 0.9) / 2;
				icon_h *= 0.9;
			}
		}

		if (!SlotLayer.is_group(l)) {
			///if is_paint
			let texpaint_preview: image_t = l.texpaint_preview;
			///end
			///if is_sculpt
			let texpaint_preview: image_t = l.texpaint;
			///end

			let icon: image_t = l.fill_layer == null ? texpaint_preview : l.fill_layer.image_icon;
			if (l.fill_layer == null) {
				// Checker
				let r: rect_t = resource_tile50(icons, 4, 1);
				let _x: f32 = ui._x;
				let _y: f32 = ui._y;
				let _w: f32 = ui._w;
				zui_image(icons, 0xffffffff, icon_h, r.x, r.y, r.w, r.h);
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

			let state: zui_state_t = zui_image(icon, 0xffffffff, icon_h);

			if (l.fill_layer == null && SlotLayer.is_mask(l)) {
				g2_set_pipeline(null);
			}

			// Draw layer numbers when selecting a layer via keyboard shortcut
			let is_typing: bool = ui.is_typing || UIView2D.ui.is_typing || UINodes.ui.is_typing;
			if (!is_typing) {
				if (i < 9 && operator_shortcut(config_keymap.select_layer, shortcut_type_t.DOWN)) {
					let number: string = String(i + 1) ;
					let width: i32 = g2_font_width(ui.font, ui.font_size, number) + 10;
					let height: i32 = g2_font_height(ui.font, ui.font_size);
					g2_set_color(ui.t.TEXT_COL);
					g2_fill_rect(uix, uiy, width, height);
					g2_set_color(ui.t.ACCENT_COL);
					g2_draw_string(number, uix + 5, uiy);
				}
			}

			return state;
		}
		else { // Group
			let folder_closed: rect_t = resource_tile50(icons, 2, 1);
			let folder_open: rect_t = resource_tile50(icons, 8, 1);
			let folder: rect_t = l.show_panel ? folder_open : folder_closed;
			return zui_image(icons, ui.t.LABEL_COL - 0x00202020, icon_h, folder.x, folder.y, folder.w, folder.h);
		}
	}

	static can_merge_down = (l: SlotLayerRaw) : bool => {
		let index: i32 = project_layers.indexOf(l);
		// Lowest layer
		if (index == 0) return false;
		// Lowest layer that has masks
		if (SlotLayer.is_layer(l) && SlotLayer.is_mask(project_layers[0]) && project_layers[0].parent == l) return false;
		// The lowest toplevel layer is a group
		if (SlotLayer.is_group(l) && SlotLayer.is_in_group(project_layers[0]) && SlotLayer.get_containing_group(project_layers[0]) == l) return false;
		// Masks must be merged down to masks
		if (SlotLayer.is_mask(l) && !SlotLayer.is_mask(project_layers[index - 1])) return false;
		return true;
	}

	static draw_layer_context_menu = (l: SlotLayerRaw, mini: bool) => {
		let add: i32 = 0;

		if (l.fill_layer == null) add += 1; // Clear
		if (l.fill_layer != null && !SlotLayer.is_mask(l)) add += 3;
		if (l.fill_layer != null && SlotLayer.is_mask(l)) add += 2;
		if (SlotLayer.is_mask(l)) add += 2;
		if (mini) {
			add += 1;
			if (!SlotLayer.is_group(l)) add += 1;
			if (SlotLayer.is_layer(l)) add += 1;
		}
		let menu_elements: i32 = SlotLayer.is_group(l) ? 7 : (19 + add);

		UIMenu.draw((ui: zui_t) => {

			if (mini) {
				let visible_handle: zui_handle_t = zui_handle("tablayers_4");
				visible_handle.selected = l.visible;
				UIMenu.menu_fill(ui);
				zui_check(visible_handle, tr("Visible"));
				if (visible_handle.changed) {
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
						let f: string = UIFiles.filename;
						if (f == "") f = tr("untitled");
						if (!f.endsWith(".png")) f += ".png";
						krom_write_png(path + path_sep + f, image_get_pixels(l.texpaint), l.texpaint.width, l.texpaint.height, 3); // RRR1
					});
				}
				else {
					///if is_paint
					context_raw.layers_export = export_mode_t.SELECTED;
					BoxExport.show_textures();
					///end
				}
			}

			if (!SlotLayer.is_group(l)) {
				let to_fill_string: string = SlotLayer.is_layer(l) ? tr("To Fill Layer") : tr("To Fill Mask");
				let to_paint_string: string = SlotLayer.is_layer(l) ? tr("To Paint Layer") : tr("To Paint Mask");

				if (l.fill_layer == null && UIMenu.menu_button(ui, to_fill_string)) {
					let _init = () => {
						SlotLayer.is_layer(l) ? history_to_fill_layer() : history_to_fill_mask();
						SlotLayer.to_fill_layer(l);
					}
					app_notify_on_init(_init);
				}
				if (l.fill_layer != null && UIMenu.menu_button(ui, to_paint_string)) {
					let _init = () => {
						SlotLayer.is_layer(l) ? history_to_paint_layer() : history_to_paint_mask();
						SlotLayer.to_paint_layer(l);
					}
					app_notify_on_init(_init);
				}
			}

			ui.enabled = TabLayers.can_delete(l);
			if (UIMenu.menu_button(ui, tr("Delete"), "delete")) {
				let _init = () => {
					TabLayers.delete_layer(context_raw.layer);
				}
				app_notify_on_init(_init);
			}
			ui.enabled = true;

			if (l.fill_layer == null && UIMenu.menu_button(ui, tr("Clear"))) {
				context_set_layer(l);
				let _init = () => {
					if (!SlotLayer.is_group(l)) {
						history_clear_layer();
						SlotLayer.clear(l);
					}
					else {
						for (let c of SlotLayer.get_children(l)) {
							context_raw.layer = c;
							history_clear_layer();
							SlotLayer.clear(c);
						}
						context_raw.layers_preview_dirty = true;
						context_raw.layer = l;
					}
				}
				app_notify_on_init(_init);
			}
			if (SlotLayer.is_mask(l) && l.fill_layer == null && UIMenu.menu_button(ui, tr("Invert"))) {
				let _init = () => {
					context_set_layer(l);
					history_invert_mask();
					SlotLayer.invert_mask(l);
				}
				app_notify_on_init(_init);
			}
			if (SlotLayer.is_mask(l) && UIMenu.menu_button(ui, tr("Apply"))) {
				let _init = () => {
					context_raw.layer = l;
					history_apply_mask();
					SlotLayer.apply_mask(l);
					context_set_layer(l.parent);
					MakeMaterial.parse_mesh_material();
					context_raw.layers_preview_dirty = true;
				}
				app_notify_on_init(_init);
			}
			if (SlotLayer.is_group(l) && UIMenu.menu_button(ui, tr("Merge Group"))) {
				let _init = () => {
					base_merge_group(l);
				}
				app_notify_on_init(_init);
			}
			ui.enabled = TabLayers.can_merge_down(l);
			if (UIMenu.menu_button(ui, tr("Merge Down"))) {
				let _init = () => {
					context_set_layer(l);
					history_merge_layers();
					base_merge_down();
					if (context_raw.layer.fill_layer != null) SlotLayer.to_paint_layer(context_raw.layer);
				}
				app_notify_on_init(_init);
			}
			ui.enabled = true;
			if (UIMenu.menu_button(ui, tr("Duplicate"))) {
				let _init = () => {
					context_set_layer(l);
					history_duplicate_layer();
					base_duplicate_layer(l);
				}
				app_notify_on_init(_init);
			}

			UIMenu.menu_fill(ui);
			UIMenu.menu_align(ui);
			let layer_opac_handle: zui_handle_t = zui_nest(zui_handle("tablayers_5"), l.id);
			layer_opac_handle.value = l.mask_opacity;
			zui_slider(layer_opac_handle, tr("Opacity"), 0.0, 1.0, true);
			if (layer_opac_handle.changed) {
				if (ui.input_started) history_layer_opacity();
				l.mask_opacity = layer_opac_handle.value;
				MakeMaterial.parse_mesh_material();
				UIMenu.keep_open = true;
			}

			if (!SlotLayer.is_group(l)) {
				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let res_handle_changed_last: bool = base_res_handle.changed;
				///if (krom_android || krom_ios)
				let ar: string[] = ["128", "256", "512", "1K", "2K", "4K"];
				///else
				let ar: string[] = ["128", "256", "512", "1K", "2K", "4K", "8K", "16K"];
				///end
				let _y: i32 = ui._y;
				base_res_handle.value = base_res_handle.position;
				base_res_handle.position = Math.floor(zui_slider(base_res_handle, ar[base_res_handle.position], 0, ar.length - 1, false, 1, false, zui_align_t.LEFT, false));
				if (base_res_handle.changed) {
					UIMenu.keep_open = true;
				}
				if (res_handle_changed_last && !base_res_handle.changed) {
					base_on_layers_resized();
				}
				ui._y = _y;
				zui_draw_string(tr("Res"), null, 0, zui_align_t.RIGHT);
				zui_end_element();

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				///if (krom_android || krom_ios)
				zui_inline_radio(base_bits_handle, ["8bit"]);
				///else
				zui_inline_radio(base_bits_handle, ["8bit", "16bit", "32bit"]);
				///end
				if (base_bits_handle.changed) {
					app_notify_on_init(base_set_layer_bits);
					UIMenu.keep_open = true;
				}
			}

			if (l.fill_layer != null) {
				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let scale_handle: zui_handle_t = zui_nest(zui_handle("tablayers_6"), l.id);
				scale_handle.value = l.scale;
				l.scale = zui_slider(scale_handle, tr("UV Scale"), 0.0, 5.0, true);
				if (scale_handle.changed) {
					context_set_material(l.fill_layer);
					context_set_layer(l);
					let _init = () => {
						base_update_fill_layers();
					}
					app_notify_on_init(_init);
					UIMenu.keep_open = true;
				}

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let angle_handle: zui_handle_t = zui_nest(zui_handle("tablayers_7"), l.id);
				angle_handle.value = l.angle;
				l.angle = zui_slider(angle_handle, tr("Angle"), 0.0, 360, true, 1);
				if (angle_handle.changed) {
					context_set_material(l.fill_layer);
					context_set_layer(l);
					MakeMaterial.parse_paint_material();
					let _init = () => {
						base_update_fill_layers();
					}
					app_notify_on_init(_init);
					UIMenu.keep_open = true;
				}

				UIMenu.menu_fill(ui);
				UIMenu.menu_align(ui);
				let uv_type_handle: zui_handle_t = zui_nest(zui_handle("tablayers_8"), l.id);
				uv_type_handle.position = l.uv_type;
				l.uv_type = zui_inline_radio(uv_type_handle, [tr("UV Map"), tr("Triplanar"), tr("Project")], zui_align_t.LEFT);
				if (uv_type_handle.changed) {
					context_set_material(l.fill_layer);
					context_set_layer(l);
					MakeMaterial.parse_paint_material();
					let _init = () => {
						base_update_fill_layers();
					}
					app_notify_on_init(_init);
					UIMenu.keep_open = true;
				}
			}

			if (!SlotLayer.is_group(l)) {
				let base_handle: zui_handle_t = zui_nest(zui_handle("tablayers_9"), l.id);
				let opac_handle: zui_handle_t = zui_nest(zui_handle("tablayers_10"), l.id);
				let nor_handle: zui_handle_t = zui_nest(zui_handle("tablayers_11"), l.id);
				let nor_blend_handle: zui_handle_t = zui_nest(zui_handle("tablayers_12"), l.id);
				let occ_handle: zui_handle_t = zui_nest(zui_handle("tablayers_13"), l.id);
				let rough_handle: zui_handle_t = zui_nest(zui_handle("tablayers_14"), l.id);
				let met_handle: zui_handle_t = zui_nest(zui_handle("tablayers_15"), l.id);
				let height_handle: zui_handle_t = zui_nest(zui_handle("tablayers_16"), l.id);
				let height_blend_handle: zui_handle_t = zui_nest(zui_handle("tablayers_17"), l.id);
				let emis_handle: zui_handle_t = zui_nest(zui_handle("tablayers_18"), l.id);
				let subs_handle: zui_handle_t = zui_nest(zui_handle("tablayers_19"), l.id);
				base_handle.selected = l.paint_base;
				opac_handle.selected = l.paint_opac;
				nor_handle.selected = l.paint_nor;
				nor_blend_handle.selected = l.paint_nor_blend;
				occ_handle.selected = l.paint_occ;
				rough_handle.selected = l.paint_rough;
				met_handle.selected = l.paint_met;
				height_handle.selected = l.paint_height;
				height_blend_handle.selected = l.paint_height_blend;
				emis_handle.selected = l.paint_emis;
				subs_handle.selected = l.paint_subs;
				UIMenu.menu_fill(ui);
				l.paint_base = zui_check(base_handle, tr("Base Color"));
				UIMenu.menu_fill(ui);
				l.paint_opac = zui_check(opac_handle, tr("Opacity"));
				UIMenu.menu_fill(ui);
				l.paint_nor = zui_check(nor_handle, tr("Normal"));
				UIMenu.menu_fill(ui);
				l.paint_nor_blend = zui_check(nor_blend_handle, tr("Normal Blending"));
				UIMenu.menu_fill(ui);
				l.paint_occ = zui_check(occ_handle, tr("Occlusion"));
				UIMenu.menu_fill(ui);
				l.paint_rough = zui_check(rough_handle, tr("Roughness"));
				UIMenu.menu_fill(ui);
				l.paint_met = zui_check(met_handle, tr("Metallic"));
				UIMenu.menu_fill(ui);
				l.paint_height = zui_check(height_handle, tr("Height"));
				UIMenu.menu_fill(ui);
				l.paint_height_blend = zui_check(height_blend_handle, tr("Height Blending"));
				UIMenu.menu_fill(ui);
				l.paint_emis = zui_check(emis_handle, tr("Emission"));
				UIMenu.menu_fill(ui);
				l.paint_subs = zui_check(subs_handle, tr("Subsurface"));
				if (base_handle.changed ||
					opac_handle.changed ||
					nor_handle.changed ||
					nor_blend_handle.changed ||
					occ_handle.changed ||
					rough_handle.changed ||
					met_handle.changed ||
					height_handle.changed ||
					height_blend_handle.changed ||
					emis_handle.changed ||
					subs_handle.changed) {
					MakeMaterial.parse_mesh_material();
					UIMenu.keep_open = true;
				}
			}
		}, menu_elements);
	}

	static make_mask_preview_rgba32 = (l: SlotLayerRaw) => {
		///if is_paint
		if (context_raw.mask_preview_rgba32 == null) {
			context_raw.mask_preview_rgba32 = image_create_render_target(UtilRender.layer_preview_size, UtilRender.layer_preview_size);
		}
		// Convert from R8 to RGBA32 for tooltip display
		if (context_raw.mask_preview_last != l) {
			context_raw.mask_preview_last = l;
			app_notify_on_init(() => {
				g2_begin(context_raw.mask_preview_rgba32);
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
		let pointers: map_t<SlotLayerRaw, i32> = TabLayers.init_layer_map();

		if (SlotLayer.is_layer(l) && SlotLayer.has_masks(l, false)) {
			for (let m of SlotLayer.get_masks(l, false)) {
				context_raw.layer = m;
				history_delete_layer();
				SlotLayer.delete(m);
			}
		}
		if (SlotLayer.is_group(l)) {
			for (let c of SlotLayer.get_children(l)) {
				if (SlotLayer.has_masks(c, false)) {
					for (let m of SlotLayer.get_masks(c, false)) {
						context_raw.layer = m;
						history_delete_layer();
						SlotLayer.delete(m);
					}
				}
				context_raw.layer = c;
				history_delete_layer();
				SlotLayer.delete(c);
			}
			if (SlotLayer.has_masks(l)) {
				for (let m of SlotLayer.get_masks(l)) {
					context_raw.layer = m;
					history_delete_layer();
					SlotLayer.delete(m);
				}
			}
		}

		context_raw.layer = l;
		history_delete_layer();
		SlotLayer.delete(l);

		if (SlotLayer.is_mask(l)) {
			context_raw.layer = l.parent;
			base_update_fill_layers();
		}

		// Remove empty group
		if (SlotLayer.is_in_group(l) && SlotLayer.get_children(SlotLayer.get_containing_group(l)) == null) {
			let g: SlotLayerRaw = SlotLayer.get_containing_group(l);
			// Maybe some group masks are left
			if (SlotLayer.has_masks(g)) {
				for (let m of SlotLayer.get_masks(g)) {
					context_raw.layer = m;
					history_delete_layer();
					SlotLayer.delete(m);
				}
			}
			context_raw.layer = l.parent;
			history_delete_layer();
			SlotLayer.delete(l.parent);
		}
		context_raw.ddirty = 2;
		for (let m of project_materials) TabLayers.remap_layer_pointers(m.canvas.nodes, TabLayers.fill_layer_map(pointers));
	}

	static can_delete = (l: SlotLayerRaw) => {
		let num_layers: i32 = 0;

		if (SlotLayer.is_mask(l)) return true;

		for (let slot of project_layers) {
			if (SlotLayer.is_layer(slot)) ++num_layers;
		}

		// All layers are in one group
		if (SlotLayer.is_group(l) && SlotLayer.get_children(l).length == num_layers) return false;

		// Do not delete last layer
		return num_layers > 1;
	}

	static can_drop_new_layer = (position: i32) => {
		if (position > 0 && position < project_layers.length && SlotLayer.is_mask(project_layers[position - 1])) {
			// 1. The layer to insert is inserted in the middle
			// 2. The layer below is a mask, i.e. the layer would have to be a (group) mask, too.
			return false;
		}
		return true;
	}
}
