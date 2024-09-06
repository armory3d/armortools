
let history_steps: step_t[];
let history_undo_i: i32 = 0; // Undo layer
let history_undos: i32 = 0; // Undos available
let history_redos: i32 = 0; // Redos available
///if (is_paint || is_sculpt)
let history_push_undo: bool = false; // Store undo on next paint
let history_undo_layers: slot_layer_t[] = null;
///end
///if is_sculpt
let history_push_undo2: bool = false;
///end

function history_undo() {
	if (history_undos > 0) {
		let active: i32 = history_steps.length - 1 - history_redos;
		let step: step_t = history_steps[active];

		if (step.name == tr("Edit Nodes")) {
			history_swap_canvas(step);
		}

		///if (is_paint || is_sculpt)
		else if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
			context_raw.layer = project_layers[step.layer];
			slot_layer_delete(context_raw.layer);
			context_raw.layer = project_layers[step.layer > 0 ? step.layer - 1 : 0];
		}
		else if (step.name == tr("New Group")) {
			context_raw.layer = project_layers[step.layer];
			// The layer below is the only layer in the group. Its layer masks are automatically unparented, too.
			project_layers[step.layer - 1].parent = null;
			slot_layer_delete(context_raw.layer);
			context_raw.layer = project_layers[step.layer > 0 ? step.layer - 1 : 0];
		}
		else if (step.name == tr("Delete Layer")) {
			let parent: slot_layer_t = step.layer_parent > 0 ? project_layers[step.layer_parent - 1] : null;
			let l: slot_layer_t = slot_layer_create("", step.layer_type, parent);
			array_insert(project_layers, step.layer, l);
			context_set_layer(l);
			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			slot_layer_swap(l, lay);
			l.mask_opacity = step.layer_opacity;
			l.blending = step.layer_blending;
			l.object_mask = step.layer_object;
			make_material_parse_mesh_material();

			// Undo at least second time in order to avoid empty groups
			if (step.layer_type == layer_slot_type_t.GROUP) {
				app_notify_on_next_frame(function () {
					let active: i32 = history_steps.length - 1 - history_redos;
					// 1. Undo deleting group masks
					let n: i32 = 1;
					while (history_steps[active - n].layer_type == layer_slot_type_t.MASK) {
						history_undo();
						++n;
					}
					// 2. Undo a mask to have a non empty group
					history_undo();
				});
			}
		}
		else if (step.name == tr("Clear Layer")) {
			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			slot_layer_swap(context_raw.layer, lay);
			context_raw.layer_preview_dirty = true;
		}
		else if (step.name == tr("Duplicate Layer")) {
			let children: slot_layer_t[] = slot_layer_get_recursive_children(project_layers[step.layer]);
			let position: i32 = step.layer + 1;
			if (children != null) {
				position += children.length;
			}

			context_raw.layer = project_layers[position];
			slot_layer_delete(context_raw.layer);
		}
		else if (step.name == tr("Order Layers")) {
			let target: slot_layer_t = project_layers[step.prev_order];
			project_layers[step.prev_order] = project_layers[step.layer];
			project_layers[step.layer] = target;
		}
		else if (step.name == tr("Merge Layers")) {
			context_raw.layer = project_layers[step.layer];
			slot_layer_delete(context_raw.layer);

			let parent: slot_layer_t = step.layer_parent > 0 ? project_layers[step.layer_parent - 2] : null;
			let l: slot_layer_t = slot_layer_create("", step.layer_type, parent);
			array_insert(project_layers, step.layer, l);
			context_set_layer(l);

			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			slot_layer_swap(context_raw.layer, lay);

			l = slot_layer_create("", step.layer_type, parent);
			array_insert(project_layers, step.layer + 1, l);
			context_set_layer(l);

			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			lay = history_undo_layers[history_undo_i];
			slot_layer_swap(context_raw.layer, lay);

			context_raw.layer.mask_opacity = step.layer_opacity;
			context_raw.layer.blending = step.layer_blending;
			context_raw.layer.object_mask = step.layer_object;
			context_raw.layers_preview_dirty = true;
			make_material_parse_mesh_material();
		}
		else if (step.name == tr("Apply Mask")) {
			// First restore the layer(s)
			let mask_pos: i32 = step.layer;
			let current_layer: slot_layer_t = null;
			// The layer at the old mask position is a mask, i.e. the layer had multiple masks before.
			if (slot_layer_is_mask(project_layers[mask_pos])) {
				current_layer = project_layers[mask_pos].parent;
			}
			else if (slot_layer_is_layer(project_layers[mask_pos]) || slot_layer_is_group(project_layers[mask_pos])) {
				current_layer = project_layers[mask_pos];
			}

			let layers_to_restore: slot_layer_t[];
			if (slot_layer_is_group(current_layer)) {
				layers_to_restore = slot_layer_get_children(current_layer);
			}
			else {
				layers_to_restore = [current_layer];
			}
			array_reverse(layers_to_restore);

			for (let i: i32 = 0; i < layers_to_restore.length; ++i) {
				let layer: slot_layer_t = layers_to_restore[i];
				// Replace the current layer's content with the old one
				context_raw.layer = layer;
				history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
				let old_layer: slot_layer_t = history_undo_layers[history_undo_i];
				slot_layer_swap(context_raw.layer, old_layer);
			}

			// Now restore the applied mask
			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let mask: slot_layer_t = history_undo_layers[history_undo_i];
			base_new_mask(false, current_layer, mask_pos);
			slot_layer_swap(context_raw.layer, mask);
			context_raw.layers_preview_dirty = true;
			context_set_layer(context_raw.layer);
		}
		else if (step.name == tr("Invert Mask")) {
			app_notify_on_init(function (step: step_t) {
				context_raw.layer = project_layers[step.layer];
				slot_layer_invert_mask(context_raw.layer);
			}, step);
		}
		else if (step.name == "Apply Filter") {
			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			context_set_layer(project_layers[step.layer]);
			slot_layer_swap(context_raw.layer, lay);
			base_new_mask(false, context_raw.layer);
			slot_layer_swap(context_raw.layer, lay);
			context_raw.layer_preview_dirty = true;
		}
		else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
			slot_layer_to_paint_layer(context_raw.layer);
			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			slot_layer_swap(context_raw.layer, lay);
		}
		else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			slot_layer_swap(context_raw.layer, lay);
			context_raw.layer.fill_layer = project_materials[step.material];
		}
		else if (step.name == tr("Layer Opacity")) {
			context_set_layer(project_layers[step.layer]);
			let t: f32 = context_raw.layer.mask_opacity;
			context_raw.layer.mask_opacity = step.layer_opacity;
			step.layer_opacity = t;
			make_material_parse_mesh_material();
		}
		else if (step.name == tr("Layer Blending")) {
			context_set_layer(project_layers[step.layer]);
			let t: blend_type_t = context_raw.layer.blending;
			context_raw.layer.blending = step.layer_blending;
			step.layer_blending = t;
			make_material_parse_mesh_material();
		}
		else if (step.name == tr("Delete Node Group")) {
			let ng: node_group_t = {
				canvas: null,
				nodes: ui_nodes_create()
			};
			array_insert(project_material_groups, step.canvas_group, ng);
			history_swap_canvas(step);
		}
		else if (step.name == tr("New Material")) {
			context_raw.material = project_materials[step.material];
			step.canvas = context_raw.material.canvas;
			slot_material_delete(context_raw.material);
		}
		else if (step.name == tr("Delete Material")) {
			context_raw.material = slot_material_create(project_materials[0].data);
			array_insert(project_materials, step.material, context_raw.material);
			context_raw.material.canvas = step.canvas;
			ui_nodes_canvas_changed();
			ui_nodes_hwnd.redraws = 2;
		}
		else if (step.name == tr("Duplicate Material")) {
			context_raw.material = project_materials[step.material];
			step.canvas = context_raw.material.canvas;
			slot_material_delete(context_raw.material);
		}
		else { // Paint operation
			history_undo_i = history_undo_i - 1 < 0 ? config_raw.undo_steps - 1 : history_undo_i - 1;
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			context_select_paint_object(project_paint_objects[step.object]);
			context_set_layer(project_layers[step.layer]);
			slot_layer_swap(context_raw.layer, lay);
			context_raw.layer_preview_dirty = true;
		}
		///end

		history_undos--;
		history_redos++;
		context_raw.ddirty = 2;

		///if (is_paint || is_sculpt)
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		if (ui_view2d_show) {
			ui_view2d_hwnd.redraws = 2;
		}

		if (config_raw.touch_ui) {
			// Refresh undo & redo buttons
			ui_menubar_menu_handle.redraws = 2;
		}
		///end
	}
}

function history_redo() {
	if (history_redos > 0) {
		let active: i32 = history_steps.length - history_redos;
		let step: step_t = history_steps[active];

		if (step.name == tr("Edit Nodes")) {
			history_swap_canvas(step);
		}

		///if (is_paint || is_sculpt)
		else if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
			let parent: slot_layer_t = step.layer_parent > 0 ? project_layers[step.layer_parent - 1] : null;
			let l: slot_layer_t = slot_layer_create("", step.layer_type, parent);
			array_insert(project_layers, step.layer, l);
			if (step.name == tr("New Black Mask")) {
				app_notify_on_next_frame(function (l: slot_layer_t) {
					slot_layer_clear(l, 0x00000000);
				}, l);
			}
			else if (step.name == tr("New White Mask")) {
				app_notify_on_next_frame(function (l: slot_layer_t) {
					slot_layer_clear(l, 0xffffffff);
				}, l);
			}
			else if (step.name == tr("New Fill Mask")) {
				context_raw.material = project_materials[step.material];
				app_notify_on_next_frame(function (l: slot_layer_t) {
					slot_layer_to_fill_layer(l);
				}, l);
			}
			context_raw.layer_preview_dirty = true;
			context_set_layer(l);
		}
		else if (step.name == tr("New Group")) {
			let l: slot_layer_t = project_layers[step.layer - 1];
			let group: slot_layer_t = base_new_group();
			array_remove(project_layers, group);
			array_insert(project_layers, step.layer, group);
			l.parent = group;
			context_set_layer(group);
		}
		else if (step.name == tr("Delete Layer")) {
			context_raw.layer = project_layers[step.layer];
			history_swap_active();
			slot_layer_delete(context_raw.layer);

			// Redoing the last delete would result in an empty group
			// Redo deleting all group masks + the group itself
			if (step.layer_type == layer_slot_type_t.LAYER && history_steps.length >= active + 2 && (history_steps[active + 1].layer_type == layer_slot_type_t.GROUP || history_steps[active + 1].layer_type == layer_slot_type_t.MASK)) {
				app_notify_on_next_frame(function () {
					let active: i32 = history_steps.length - history_redos;
					let n: i32 = 1;
					while (history_steps[active + n].layer_type == layer_slot_type_t.MASK) {
						++n;
					}

					for (let i: i32 = 0; i < n; ++i) {
						history_redo();
					}
				});
			}
		}
		else if (step.name == tr("Clear Layer")) {
			context_raw.layer = project_layers[step.layer];
			history_swap_active();
			slot_layer_clear(context_raw.layer);
			context_raw.layer_preview_dirty = true;
		}
		else if (step.name == tr("Duplicate Layer")) {
			context_raw.layer = project_layers[step.layer];
			app_notify_on_next_frame(function () {
				base_duplicate_layer(context_raw.layer);
			});
		}
		else if (step.name == tr("Order Layers")) {
			let target: slot_layer_t = project_layers[step.prev_order];
			project_layers[step.prev_order] = project_layers[step.layer];
			project_layers[step.layer] = target;
		}
		else if (step.name == tr("Merge Layers")) {
			context_raw.layer = project_layers[step.layer + 1];
			app_notify_on_init(history_redo_merge_layers);
			app_notify_on_init(base_merge_down);
		}
		else if (step.name == tr("Apply Mask")) {
			context_raw.layer = project_layers[step.layer];
			if (slot_layer_is_group_mask(context_raw.layer)) {
				let group: slot_layer_t = context_raw.layer.parent;
				let layers: slot_layer_t[] = slot_layer_get_children(group);
				array_insert(layers, 0, context_raw.layer);
				history_copy_merging_layers2(layers);
			}
			else {
				let layers: slot_layer_t[] = [context_raw.layer, context_raw.layer.parent];
				history_copy_merging_layers2(layers);
			}

			app_notify_on_next_frame(function () {
				slot_layer_apply_mask(context_raw.layer);
				context_set_layer(context_raw.layer);
				context_raw.layers_preview_dirty = true;
			});
		}
		else if (step.name == tr("Invert Mask")) {
			app_notify_on_init(function (step: step_t) {
				context_raw.layer = project_layers[step.layer];
				slot_layer_invert_mask(context_raw.layer);
			}, step);
		}
		else if (step.name == tr("Apply Filter")) {
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			context_set_layer(project_layers[step.layer]);
			slot_layer_swap(context_raw.layer, lay);
			base_new_mask(false, lay);
			slot_layer_swap(context_raw.layer, lay);
			context_raw.layer_preview_dirty = true;
			history_undo_i = (history_undo_i + 1) % config_raw.undo_steps;
		}
		else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			slot_layer_swap(context_raw.layer, lay);
			context_raw.layer.fill_layer = project_materials[step.material];
			history_undo_i = (history_undo_i + 1) % config_raw.undo_steps;
		}
		else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
			slot_layer_to_paint_layer(context_raw.layer);
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			slot_layer_swap(context_raw.layer, lay);
			history_undo_i = (history_undo_i + 1) % config_raw.undo_steps;
		}
		else if (step.name == tr("Layer Opacity")) {
			context_set_layer(project_layers[step.layer]);
			let t: f32 = context_raw.layer.mask_opacity;
			context_raw.layer.mask_opacity = step.layer_opacity;
			step.layer_opacity = t;
			make_material_parse_mesh_material();
		}
		else if (step.name == tr("Layer Blending")) {
			context_set_layer(project_layers[step.layer]);
			let t: blend_type_t = context_raw.layer.blending;
			context_raw.layer.blending = step.layer_blending;
			step.layer_blending = t;
			make_material_parse_mesh_material();
		}
		else if (step.name == tr("Delete Node Group")) {
			history_swap_canvas(step);
			array_remove(project_material_groups, project_material_groups[step.canvas_group]);
		}
		else if (step.name == tr("New Material")) {
			context_raw.material = slot_material_create(project_materials[0].data);
			array_insert(project_materials, step.material, context_raw.material);
			context_raw.material.canvas = step.canvas;
			ui_nodes_canvas_changed();
			ui_nodes_hwnd.redraws = 2;
		}
		else if (step.name == tr("Delete Material")) {
			context_raw.material = project_materials[step.material];
			step.canvas = context_raw.material.canvas;
			slot_material_delete(context_raw.material);
		}
		else if (step.name == tr("Duplicate Material")) {
			context_raw.material = slot_material_create(project_materials[0].data);
			array_insert(project_materials, step.material, context_raw.material);
			context_raw.material.canvas = step.canvas;
			ui_nodes_canvas_changed();
			ui_nodes_hwnd.redraws = 2;
		}
		else { // Paint operation
			let lay: slot_layer_t = history_undo_layers[history_undo_i];
			context_select_paint_object(project_paint_objects[step.object]);
			context_set_layer(project_layers[step.layer]);
			slot_layer_swap(context_raw.layer, lay);
			context_raw.layer_preview_dirty = true;
			history_undo_i = (history_undo_i + 1) % config_raw.undo_steps;
		}
		///end

		history_undos++;
		history_redos--;
		context_raw.ddirty = 2;

		///if (is_paint || is_sculpt)
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		if (ui_view2d_show) {
			ui_view2d_hwnd.redraws = 2;
		}

		if (config_raw.touch_ui) {
			// Refresh undo & redo buttons
			ui_menubar_menu_handle.redraws = 2;
		}
		///end
	}
}

function history_reset() {
	///if (is_paint || is_sculpt)
	history_steps = [
		{
			name: tr("New", null),
			layer: 0,
			layer_type: layer_slot_type_t.LAYER,
			layer_parent: -1,
			object: 0,
			material: 0,
			brush: 0
		}
	];
	///end
	///if is_lab
	history_steps = [
		{
			name: tr("New", null)
		}
	];
	///end

	history_undos = 0;
	history_redos = 0;
	history_undo_i = 0;
}

///if (is_paint || is_sculpt)
function history_edit_nodes(canvas: ui_node_canvas_t, canvas_type: i32, canvas_group: i32 = -1) {
///end
///if is_lab
function history_edit_nodes(canvas: ui_node_canvas_t, canvas_group: i32 = -1) {
///end
	let step: step_t = history_push(tr("Edit Nodes"));
	step.canvas_group = canvas_group;
	///if (is_paint || is_sculpt)
	step.canvas_type = canvas_type;
	///end
	step.canvas = util_clone_canvas(canvas);
}

///if (is_paint || is_sculpt)
function history_paint() {
	let is_mask: bool = slot_layer_is_mask(context_raw.layer);
	history_copy_to_undo(context_raw.layer.id, history_undo_i, is_mask);

	history_push_undo = false;
	history_push(tr(ui_toolbar_tool_names[context_raw.tool]));
}

function history_new_layer() {
	history_push(tr("New Layer"));
}

function history_new_black_mask() {
	history_push(tr("New Black Mask"));
}

function history_new_white_mask() {
	history_push(tr("New White Mask"));
}

function history_new_fill_mask() {
	history_push(tr("New Fill Mask"));
}

function history_new_group() {
	history_push(tr("New Group"));
}

function history_duplicate_layer() {
	history_push(tr("Duplicate Layer"));
}

function history_delete_layer() {
	history_swap_active();
	history_push(tr("Delete Layer"));
}

function history_clear_layer() {
	history_swap_active();
	history_push(tr("Clear Layer"));
}

function history_order_layers(prevOrder: i32) {
	let step: step_t = history_push(tr("Order Layers"));
	step.prev_order = prevOrder;
}

function history_merge_layers() {
	history_copy_merging_layers();

	let step: step_t = history_push(tr("Merge Layers"));
	step.layer -= 1; // Merge down
	if (slot_layer_has_masks(context_raw.layer)) {
		step.layer -= slot_layer_get_masks(context_raw.layer).length;
	}
	array_shift(history_steps); // Merge consumes 2 steps
	history_undos--;
	// TODO: use undo layer in app_merge_down to save memory
}

function history_apply_mask() {
	if (slot_layer_is_group_mask(context_raw.layer)) {
		let group: slot_layer_t = context_raw.layer.parent;
		let layers: slot_layer_t[] = slot_layer_get_children(group);
		array_insert(layers, 0, context_raw.layer);
		history_copy_merging_layers2(layers);
	}
	else {
		let layers: slot_layer_t[] = [context_raw.layer, context_raw.layer.parent];
		history_copy_merging_layers2(layers);
	}
	history_push(tr("Apply Mask"));
}


function history_invert_mask() {
	history_push(tr("Invert Mask"));
}

function history_apply_filter() {
	history_copy_to_undo(context_raw.layer.id, history_undo_i, true);
	history_push(tr("Apply Filter"));
}

function history_to_fill_layer() {
	history_copy_to_undo(context_raw.layer.id, history_undo_i, false);
	history_push(tr("To Fill Layer"));
}

function history_to_fill_mask() {
	history_copy_to_undo(context_raw.layer.id, history_undo_i, true);
	history_push(tr("To Fill Mask"));
}

function history_to_paint_layer() {
	history_copy_to_undo(context_raw.layer.id, history_undo_i, false);
	history_push(tr("To Paint Layer"));
}

function history_to_paint_mask() {
	history_copy_to_undo(context_raw.layer.id, history_undo_i, true);
	history_push(tr("To Paint Mask"));
}

function history_layer_opacity() {
	history_push(tr("Layer Opacity"));
}

// function history_layer_object() {
// 	history_push("Layer Object");
// }

function history_layer_blending() {
	history_push(tr("Layer Blending"));
}

function history_new_material() {
	let step: step_t = history_push(tr("New Material"));
	step.canvas_type = 0;
	step.canvas = util_clone_canvas(context_raw.material.canvas);
}

function history_delete_material() {
	let step: step_t = history_push(tr("Delete Material"));
	step.canvas_type = 0;
	step.canvas = util_clone_canvas(context_raw.material.canvas);
}

function history_duplicate_material() {
	let step: step_t = history_push(tr("Duplicate Material"));
	step.canvas_type = 0;
	step.canvas = util_clone_canvas(context_raw.material.canvas);
}

function history_delete_material_group(group: node_group_t) {
	let step: step_t = history_push(tr("Delete Node Group"));
	step.canvas_type = canvas_type_t.MATERIAL;
	step.canvas_group = array_index_of(project_material_groups, group);
	step.canvas = util_clone_canvas(group.canvas);
}
///end

function history_push(name: string): step_t {
	///if (arm_windows || arm_linux || arm_macos)
	let filename: string = project_filepath == "" ? ui_files_filename : substring(project_filepath, string_last_index_of(project_filepath, path_sep) + 1, project_filepath.length - 4);
	sys_title_set(filename + "* - " + manifest_title);
	///end

	if (config_raw.touch_ui) {
		// Refresh undo & redo buttons
		ui_menubar_menu_handle.redraws = 2;
	}

	if (history_undos < config_raw.undo_steps) {
		history_undos++;
	}
	if (history_redos > 0) {
		for (let i: i32 = 0; i < history_redos; ++i) {
			array_pop(history_steps);
		}
		history_redos = 0;
	}

	///if (is_paint || is_sculpt)
	let opos: i32 = array_index_of(project_paint_objects, context_raw.paint_object);
	let lpos: i32 = array_index_of(project_layers, context_raw.layer);
	let mpos: i32 = array_index_of(project_materials, context_raw.material);
	let bpos: i32 = array_index_of(project_brushes, context_raw.brush);

	let step: step_t = {
		name: name,
		layer: lpos,
		layer_type: slot_layer_is_mask(context_raw.layer) ? layer_slot_type_t.MASK : slot_layer_is_group(context_raw.layer) ? layer_slot_type_t.GROUP : layer_slot_type_t.LAYER,
		layer_parent: context_raw.layer.parent == null ? -1 : array_index_of(project_layers, context_raw.layer.parent),
		object: opos,
		material: mpos,
		brush: bpos,
		layer_opacity: context_raw.layer.mask_opacity,
		layer_object: context_raw.layer.object_mask,
		layer_blending: context_raw.layer.blending
	};

	array_push(history_steps, step);
	///end

	///if is_lab
	let step: step_t = {
		name: name
	};
	array_push(history_steps, step);
	///end

	while (history_steps.length > config_raw.undo_steps + 1) {
		array_shift(history_steps);
	}
	return history_steps[history_steps.length - 1];
}

///if (is_paint || is_sculpt)
function history_redo_merge_layers() {
	history_copy_merging_layers();
}

function history_copy_merging_layers() {
	let lay: slot_layer_t = context_raw.layer;
	history_copy_to_undo(lay.id, history_undo_i, slot_layer_is_mask(context_raw.layer));

	let below: i32 = array_index_of(project_layers, lay) - 1;
	lay = project_layers[below];
	history_copy_to_undo(lay.id, history_undo_i, slot_layer_is_mask(context_raw.layer));
}

function history_copy_merging_layers2(layers: slot_layer_t[]) {
	for (let i: i32 = 0; i < layers.length; ++i) {
		let layer: slot_layer_t = layers[i];
		history_copy_to_undo(layer.id, history_undo_i, slot_layer_is_mask(layer));
	}
}

function history_swap_active() {
	let undo_layer: slot_layer_t = history_undo_layers[history_undo_i];
	slot_layer_swap(undo_layer, context_raw.layer);
	history_undo_i = (history_undo_i + 1) % config_raw.undo_steps;
}

function history_copy_to_undo(from_id: i32, to_id: i32, is_mask: bool) {

	///if is_sculpt
	is_mask = true;
	///end

	if (is_mask) {
		render_path_set_target("texpaint_undo" + to_id);
		render_path_bind_target("texpaint" + from_id, "tex");
		// render_path_draw_shader("shader_datas/copy_pass/copyR8_pass");
		render_path_draw_shader("shader_datas/copy_pass/copy_pass");
	}
	else {
		let additional: string[] = ["texpaint_nor_undo" + to_id, "texpaint_pack_undo" + to_id];
		render_path_set_target("texpaint_undo" + to_id, additional);
		render_path_bind_target("texpaint" + from_id, "tex0");
		render_path_bind_target("texpaint_nor" + from_id, "tex1");
		render_path_bind_target("texpaint_pack" + from_id, "tex2");
		render_path_draw_shader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
	}
	history_undo_i = (history_undo_i + 1) % config_raw.undo_steps;
}
///end

function history_get_canvas(step: step_t): ui_node_canvas_t {
	///if (is_paint || is_sculpt)
	if (step.canvas_group == -1) {
		return project_materials[step.material].canvas;
	}
	else {
		return project_material_groups[step.canvas_group].canvas;
	}
	///end

	///if is_lab
	return null;
	///end
}

function history_set_canvas(step: step_t, canvas: ui_node_canvas_t) {
	///if (is_paint || is_sculpt)
	if (step.canvas_group == -1) {
		project_materials[step.material].canvas = canvas;
	}
	else {
		project_material_groups[step.canvas_group].canvas = canvas;
	}
	///end
}

function history_swap_canvas(step: step_t) {
	///if (is_paint || is_sculpt)
	if (step.canvas_type == 0) {
		let _canvas: ui_node_canvas_t = history_get_canvas(step);
		history_set_canvas(step, step.canvas);
		step.canvas = _canvas;
		context_raw.material = project_materials[step.material];
	}
	else {
		let _canvas: ui_node_canvas_t = project_brushes[step.brush].canvas;
		project_brushes[step.brush].canvas = step.canvas;
		step.canvas = _canvas;
		context_raw.brush = project_brushes[step.brush];
	}
	///end

	///if is_lab
	let _canvas: ui_node_canvas_t = history_get_canvas(step);
	history_set_canvas(step, step.canvas);
	step.canvas = _canvas;
	///end

	ui_nodes_canvas_changed();
	ui_nodes_hwnd.redraws = 2;
}

type step_t = {
	name?: string;
	canvas?: ui_node_canvas_t; // Node history
	canvas_group?: i32;
	///if (is_paint || is_sculpt)
	layer?: i32;
	layer_type?: layer_slot_type_t;
	layer_parent?: i32;
	object?: i32;
	material?: i32;
	brush?: i32;
	layer_opacity?: f32;
	layer_object?: i32;
	layer_blending?: i32;
	prev_order?: i32; // Previous layer position
	canvas_type?: i32;
	///end
};
