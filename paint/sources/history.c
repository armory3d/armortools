void history_undo() {
	if (history_undos > 0) {
		i32             active = history_steps->length - 1 - history_redos;
		history_step_t *step   = history_steps->buffer[active];

		if (string_equals(step->name, tr("Edit Nodes", null))) {
			history_swap_canvas(step);
		}
		else if (string_equals(step->name, tr("New Layer", null)) || string_equals(step->name, tr("New Black Mask", null)) ||
		         string_equals(step->name, tr("New White Mask", null)) || string_equals(step->name, tr("New Fill Mask", null))) {
			context_raw->layer = project_layers->buffer[step->layer];
			slot_layer_delete(context_raw->layer);
			context_raw->layer = project_layers->buffer[step->layer > 0 ? step->layer - 1 : 0];
		}
		else if (string_equals(step->name, tr("New Group", null))) {
			context_raw->layer = project_layers->buffer[step->layer];
			// The layer below is the only layer in the group. Its layer masks are automatically unparented, too.
			project_layers->buffer[step->layer - 1]->parent = null;
			slot_layer_delete(context_raw->layer);
			context_raw->layer = project_layers->buffer[step->layer > 0 ? step->layer - 1 : 0];
		}
		else if (string_equals(step->name, tr("Delete Layer", null))) {
			slot_layer_t *parent = step->layer_parent > 0 ? project_layers->buffer[step->layer_parent - 1] : null;
			slot_layer_t *l      = slot_layer_create("", step->layer_type, parent);
			array_insert(project_layers, step->layer, l);
			context_set_layer(l);
			history_undo_i    = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(l, lay);
			l->mask_opacity = step->layer_opacity;
			l->blending     = step->layer_blending;
			l->object_mask  = step->layer_object;
			make_material_parse_mesh_material();

			// Undo at least second time in order to avoid empty groups
			if (step->layer_type == LAYER_SLOT_TYPE_GROUP) {
				sys_notify_on_next_frame(&history_undo_76821, null);
			}
		}
		else if (string_equals(step->name, tr("Clear Layer", null))) {
			history_undo_i    = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(context_raw->layer, lay);
			context_raw->layer_preview_dirty = true;
		}
		else if (string_equals(step->name, tr("Duplicate Layer", null))) {
			slot_layer_t_array_t *children = slot_layer_get_recursive_children(project_layers->buffer[step->layer]);
			i32                   position = step->layer + 1;
			if (children != null) {
				position += children->length;
			}

			context_raw->layer = project_layers->buffer[position];
			slot_layer_delete(context_raw->layer);
		}
		else if (string_equals(step->name, tr("Order Layers", null))) {
			slot_layer_t *target                     = project_layers->buffer[step->prev_order];
			project_layers->buffer[step->prev_order] = project_layers->buffer[step->layer];
			project_layers->buffer[step->layer]      = target;
		}
		else if (string_equals(step->name, tr("Merge Layers", null))) {
			context_raw->layer = project_layers->buffer[step->layer];
			slot_layer_delete(context_raw->layer);

			slot_layer_t *parent = step->layer_parent > 0 ? project_layers->buffer[step->layer_parent - 2] : null;
			slot_layer_t *l      = slot_layer_create("", step->layer_type, parent);
			array_insert(project_layers, step->layer, l);
			context_set_layer(l);

			history_undo_i    = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(context_raw->layer, lay);

			l = slot_layer_create("", step->layer_type, parent);
			array_insert(project_layers, step->layer + 1, l);
			context_set_layer(l);

			history_undo_i = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			lay            = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(context_raw->layer, lay);

			context_raw->layer->mask_opacity  = step->layer_opacity;
			context_raw->layer->blending      = step->layer_blending;
			context_raw->layer->object_mask   = step->layer_object;
			context_raw->layers_preview_dirty = true;
			make_material_parse_mesh_material();
		}
		else if (string_equals(step->name, tr("Apply Mask", null))) {
			// First restore the layer(s)
			i32           mask_pos      = step->layer;
			slot_layer_t *current_layer = null;
			// The layer at the old mask position is a mask, i.e. the layer had multiple masks before.
			if (slot_layer_is_mask(project_layers->buffer[mask_pos])) {
				current_layer = project_layers->buffer[mask_pos]->parent;
			}
			else if (slot_layer_is_layer(project_layers->buffer[mask_pos]) || slot_layer_is_group(project_layers->buffer[mask_pos])) {
				current_layer = project_layers->buffer[mask_pos];
			}

			slot_layer_t_array_t *layers_to_restore;
			if (slot_layer_is_group(current_layer)) {
				layers_to_restore = slot_layer_get_children(current_layer);
			}
			else {
				layers_to_restore = any_array_create_from_raw(
				    (any[]){
				        current_layer,
				    },
				    1);
			}
			array_reverse(layers_to_restore);

			for (i32 i = 0; i < layers_to_restore->length; ++i) {
				slot_layer_t *layer = layers_to_restore->buffer[i];
				// Replace the current layer's content with the old one
				context_raw->layer      = layer;
				history_undo_i          = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
				slot_layer_t *old_layer = history_undo_layers->buffer[history_undo_i];
				slot_layer_swap(context_raw->layer, old_layer);
			}

			// Now restore the applied mask
			history_undo_i     = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *mask = history_undo_layers->buffer[history_undo_i];
			layers_new_mask(false, current_layer, mask_pos);
			slot_layer_swap(context_raw->layer, mask);
			context_raw->layers_preview_dirty = true;
			context_set_layer(context_raw->layer);
		}
		else if (string_equals(step->name, tr("Invert Mask", null))) {
			sys_notify_on_next_frame(&history_undo_77442, step);
		}
		else if (string_equals(step->name, "Apply Filter")) {
			history_undo_i    = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			context_set_layer(project_layers->buffer[step->layer]);
			slot_layer_swap(context_raw->layer, lay);
			layers_new_mask(false, context_raw->layer, -1);
			slot_layer_swap(context_raw->layer, lay);
			context_raw->layer_preview_dirty = true;
		}
		else if (string_equals(step->name, tr("To Fill Layer", null)) || string_equals(step->name, tr("To Fill Mask", null))) {
			slot_layer_to_paint_layer(context_raw->layer);
			history_undo_i    = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(context_raw->layer, lay);
		}
		else if (string_equals(step->name, tr("To Paint Layer", null)) || string_equals(step->name, tr("To Paint Mask", null))) {
			history_undo_i    = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(context_raw->layer, lay);
			context_raw->layer->fill_layer = project_materials->buffer[step->material];
		}
		else if (string_equals(step->name, tr("Layer Opacity", null))) {
			context_set_layer(project_layers->buffer[step->layer]);
			f32 t                            = context_raw->layer->mask_opacity;
			context_raw->layer->mask_opacity = step->layer_opacity;
			step->layer_opacity              = t;
			make_material_parse_mesh_material();
		}
		else if (string_equals(step->name, tr("Layer Blending", null))) {
			context_set_layer(project_layers->buffer[step->layer]);
			blend_type_t t               = context_raw->layer->blending;
			context_raw->layer->blending = step->layer_blending;
			step->layer_blending         = t;
			make_material_parse_mesh_material();
		}
		else if (string_equals(step->name, tr("Delete Node Group", null))) {
			node_group_t *ng = GC_ALLOC_INIT(node_group_t, {.canvas = null, .nodes = ui_nodes_create()});
			array_insert(project_material_groups, step->canvas_group, ng);
			history_swap_canvas(step);
		}
		else if (string_equals(step->name, tr("New Material", null))) {
			context_raw->material = project_materials->buffer[step->material];
			step->canvas          = context_raw->material->canvas;
			slot_material_delete(context_raw->material);
		}
		else if (string_equals(step->name, tr("Delete Material", null))) {
			context_raw->material = slot_material_create(project_materials->buffer[0]->data, null);
			array_insert(project_materials, step->material, context_raw->material);
			context_raw->material->canvas = step->canvas;
			ui_nodes_canvas_changed();
			ui_nodes_hwnd->redraws = 2;
		}
		else if (string_equals(step->name, tr("Duplicate Material", null))) {
			context_raw->material = project_materials->buffer[step->material];
			step->canvas          = context_raw->material->canvas;
			slot_material_delete(context_raw->material);
		}
		else { // Paint operation
			history_undo_i    = history_undo_i - 1 < 0 ? config_raw->undo_steps - 1 : history_undo_i - 1;
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			context_select_paint_object(project_paint_objects->buffer[step->object]);
			context_set_layer(project_layers->buffer[step->layer]);
			slot_layer_swap(context_raw->layer, lay);
			context_raw->layer_preview_dirty = true;
		}

		history_undos--;
		history_redos++;
		context_raw->ddirty = 2;

		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
		if (ui_view2d_show) {
			ui_view2d_hwnd->redraws = 2;
		}

		if (config_raw->touch_ui) {
			// Refresh undo & redo buttons
			ui_menubar_menu_handle->redraws = 2;
		}
	}
}

void history_undo_77442(history_step_t *step) {
	context_raw->layer = project_layers->buffer[step->layer];
	slot_layer_invert_mask(context_raw->layer);
}

void history_undo_76821(any _) {
	i32 active = history_steps->length - 1 - history_redos;
	// 1. Undo deleting group masks
	i32 n = 1;
	while (history_steps->buffer[active - n]->layer_type == LAYER_SLOT_TYPE_MASK) {
		history_undo();
		++n;
	}
	// 2. Undo a mask to have a non empty group
	history_undo();
}

void history_redo() {
	if (history_redos > 0) {
		i32             active = history_steps->length - history_redos;
		history_step_t *step   = history_steps->buffer[active];

		if (string_equals(step->name, tr("Edit Nodes", null))) {
			history_swap_canvas(step);
		}
		else if (string_equals(step->name, tr("New Layer", null)) || string_equals(step->name, tr("New Black Mask", null)) ||
		         string_equals(step->name, tr("New White Mask", null)) || string_equals(step->name, tr("New Fill Mask", null))) {
			slot_layer_t *parent = step->layer_parent > 0 ? project_layers->buffer[step->layer_parent - 1] : null;
			slot_layer_t *l      = slot_layer_create("", step->layer_type, parent);
			array_insert(project_layers, step->layer, l);
			if (string_equals(step->name, tr("New Black Mask", null))) {
				sys_notify_on_next_frame(&history_redo_78106, l);
			}
			else if (string_equals(step->name, tr("New White Mask", null))) {
				sys_notify_on_next_frame(&history_redo_78139, l);
			}
			else if (string_equals(step->name, tr("New Fill Mask", null))) {
				context_raw->material = project_materials->buffer[step->material];
				sys_notify_on_next_frame(&history_redo_78179, l);
			}
			context_raw->layer_preview_dirty = true;
			context_set_layer(l);
		}
		else if (string_equals(step->name, tr("New Group", null))) {
			slot_layer_t *l     = project_layers->buffer[step->layer - 1];
			slot_layer_t *group = layers_new_group();
			array_remove(project_layers, group);
			array_insert(project_layers, step->layer, group);
			l->parent = group;
			context_set_layer(group);
		}
		else if (string_equals(step->name, tr("Delete Layer", null))) {
			context_raw->layer = project_layers->buffer[step->layer];
			history_swap_active();
			slot_layer_delete(context_raw->layer);

			// Redoing the last delete would result in an empty group
			// Redo deleting all group masks + the group itself
			if (step->layer_type == LAYER_SLOT_TYPE_LAYER && history_steps->length >= active + 2 &&
			    (history_steps->buffer[active + 1]->layer_type == LAYER_SLOT_TYPE_GROUP ||
			     history_steps->buffer[active + 1]->layer_type == LAYER_SLOT_TYPE_MASK)) {
				sys_notify_on_next_frame(&history_redo_78330, null);
			}
		}
		else if (string_equals(step->name, tr("Clear Layer", null))) {
			context_raw->layer = project_layers->buffer[step->layer];
			history_swap_active();
			slot_layer_clear(context_raw->layer, 0x00000000, null, 1.0, layers_default_rough, 0.0);
			context_raw->layer_preview_dirty = true;
		}
		else if (string_equals(step->name, tr("Duplicate Layer", null))) {
			context_raw->layer = project_layers->buffer[step->layer];
			sys_notify_on_next_frame(&history_redo_78447, null);
		}
		else if (string_equals(step->name, tr("Order Layers", null))) {
			slot_layer_t *target                     = project_layers->buffer[step->prev_order];
			project_layers->buffer[step->prev_order] = project_layers->buffer[step->layer];
			project_layers->buffer[step->layer]      = target;
		}
		else if (string_equals(step->name, tr("Merge Layers", null))) {
			context_raw->layer = project_layers->buffer[step->layer + 1];
			sys_notify_on_next_frame(&history_redo_78524, null);
			sys_notify_on_next_frame(&history_redo_78540, null);
		}
		else if (string_equals(step->name, tr("Apply Mask", null))) {
			context_raw->layer = project_layers->buffer[step->layer];
			if (slot_layer_is_group_mask(context_raw->layer)) {
				slot_layer_t         *group  = context_raw->layer->parent;
				slot_layer_t_array_t *layers = slot_layer_get_children(group);
				array_insert(layers, 0, context_raw->layer);
				history_copy_merging_layers2(layers);
			}
			else {
				slot_layer_t_array_t *layers = any_array_create_from_raw(
				    (any[]){
				        context_raw->layer,
				        context_raw->layer->parent,
				    },
				    2);
				history_copy_merging_layers2(layers);
			}

			sys_notify_on_next_frame(&history_redo_78638, null);
		}
		else if (string_equals(step->name, tr("Invert Mask", null))) {
			sys_notify_on_next_frame(&history_redo_78676, step);
		}
		else if (string_equals(step->name, tr("Apply Filter", null))) {
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			context_set_layer(project_layers->buffer[step->layer]);
			slot_layer_swap(context_raw->layer, lay);
			layers_new_mask(false, lay, -1);
			slot_layer_swap(context_raw->layer, lay);
			context_raw->layer_preview_dirty = true;
			history_undo_i                   = (history_undo_i + 1) % config_raw->undo_steps;
		}
		else if (string_equals(step->name, tr("To Fill Layer", null)) || string_equals(step->name, tr("To Fill Mask", null))) {
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(context_raw->layer, lay);
			context_raw->layer->fill_layer = project_materials->buffer[step->material];
			history_undo_i                 = (history_undo_i + 1) % config_raw->undo_steps;
		}
		else if (string_equals(step->name, tr("To Paint Layer", null)) || string_equals(step->name, tr("To Paint Mask", null))) {
			slot_layer_to_paint_layer(context_raw->layer);
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			slot_layer_swap(context_raw->layer, lay);
			history_undo_i = (history_undo_i + 1) % config_raw->undo_steps;
		}
		else if (string_equals(step->name, tr("Layer Opacity", null))) {
			context_set_layer(project_layers->buffer[step->layer]);
			f32 t                            = context_raw->layer->mask_opacity;
			context_raw->layer->mask_opacity = step->layer_opacity;
			step->layer_opacity              = t;
			make_material_parse_mesh_material();
		}
		else if (string_equals(step->name, tr("Layer Blending", null))) {
			context_set_layer(project_layers->buffer[step->layer]);
			blend_type_t t               = context_raw->layer->blending;
			context_raw->layer->blending = step->layer_blending;
			step->layer_blending         = t;
			make_material_parse_mesh_material();
		}
		else if (string_equals(step->name, tr("Delete Node Group", null))) {
			history_swap_canvas(step);
			array_remove(project_material_groups, project_material_groups->buffer[step->canvas_group]);
		}
		else if (string_equals(step->name, tr("New Material", null))) {
			context_raw->material = slot_material_create(project_materials->buffer[0]->data, null);
			array_insert(project_materials, step->material, context_raw->material);
			context_raw->material->canvas = step->canvas;
			ui_nodes_canvas_changed();
			ui_nodes_hwnd->redraws = 2;
		}
		else if (string_equals(step->name, tr("Delete Material", null))) {
			context_raw->material = project_materials->buffer[step->material];
			step->canvas          = context_raw->material->canvas;
			slot_material_delete(context_raw->material);
		}
		else if (string_equals(step->name, tr("Duplicate Material", null))) {
			context_raw->material = slot_material_create(project_materials->buffer[0]->data, null);
			array_insert(project_materials, step->material, context_raw->material);
			context_raw->material->canvas = step->canvas;
			ui_nodes_canvas_changed();
			ui_nodes_hwnd->redraws = 2;
		}
		else { // Paint operation
			slot_layer_t *lay = history_undo_layers->buffer[history_undo_i];
			context_select_paint_object(project_paint_objects->buffer[step->object]);
			context_set_layer(project_layers->buffer[step->layer]);
			slot_layer_swap(context_raw->layer, lay);
			context_raw->layer_preview_dirty = true;
			history_undo_i                   = (history_undo_i + 1) % config_raw->undo_steps;
		}

		history_undos++;
		history_redos--;
		context_raw->ddirty = 2;

		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR0]->redraws = 2;
		ui_base_hwnds->buffer[TAB_AREA_SIDEBAR1]->redraws = 2;
		if (ui_view2d_show) {
			ui_view2d_hwnd->redraws = 2;
		}

		if (config_raw->touch_ui) {
			// Refresh undo & redo buttons
			ui_menubar_menu_handle->redraws = 2;
		}
	}
}

void history_redo_78676(history_step_t *step) {
	context_raw->layer = project_layers->buffer[step->layer];
	slot_layer_invert_mask(context_raw->layer);
}

void history_redo_78638(any _) {
	slot_layer_apply_mask(context_raw->layer);
	context_set_layer(context_raw->layer);
	context_raw->layers_preview_dirty = true;
}

void history_redo_78540(any _) {
	layers_merge_down();
}

void history_redo_78524(any _) {
	history_redo_merge_layers();
}

void history_redo_78447(any _) {
	layers_duplicate_layer(context_raw->layer);
}

void history_redo_78330(any _) {
	i32 active = history_steps->length - history_redos;
	i32 n      = 1;
	while (history_steps->buffer[active + n]->layer_type == LAYER_SLOT_TYPE_MASK) {
		++n;
	}
	for (i32 i = 0; i < n; ++i) {
		history_redo();
	}
}

void history_redo_78179(slot_layer_t *l) {
	slot_layer_to_fill_layer(l);
}

void history_redo_78139(slot_layer_t *l) {
	slot_layer_clear(l, 0xffffffff, null, 1.0, layers_default_rough, 0.0);
}

void history_redo_78106(slot_layer_t *l) {
	slot_layer_clear(l, 0x00000000, null, 1.0, layers_default_rough, 0.0);
}

void history_reset() {
	gc_unroot(history_steps);
	history_steps = any_array_create_from_raw(
	    (any[]){
	        GC_ALLOC_INIT(
	            history_step_t,
	            {.name = tr("New", null), .layer = 0, .layer_type = LAYER_SLOT_TYPE_LAYER, .layer_parent = -1, .object = 0, .material = 0, .brush = 0}),
	    },
	    1);
	gc_root(history_steps);
	history_undos  = 0;
	history_redos  = 0;
	history_undo_i = 0;
}

void history_edit_nodes(ui_node_canvas_t *canvas, i32 canvas_type, i32 canvas_group) {
	history_step_t *step = history_push(tr("Edit Nodes", null));
	step->canvas_group   = canvas_group;
	step->canvas_type    = canvas_type;
	step->canvas         = util_clone_canvas(canvas);
}

void history_paint() {
	bool is_mask = slot_layer_is_mask(context_raw->layer);
	history_copy_to_undo(context_raw->layer->id, history_undo_i, is_mask);

	history_push_undo = false;
	history_push(tr(ui_toolbar_tool_names->buffer[context_raw->tool], null));
}

void history_new_layer() {
	history_push(tr("New Layer", null));
}

void history_new_black_mask() {
	history_push(tr("New Black Mask", null));
}

void history_new_white_mask() {
	history_push(tr("New White Mask", null));
}

void history_new_fill_mask() {
	history_push(tr("New Fill Mask", null));
}

void history_new_group() {
	history_push(tr("New Group", null));
}

void history_duplicate_layer() {
	history_push(tr("Duplicate Layer", null));
}

void history_delete_layer() {
	history_swap_active();
	history_push(tr("Delete Layer", null));
}

void history_clear_layer() {
	history_swap_active();
	history_push(tr("Clear Layer", null));
}

void history_order_layers(i32 prev_order) {
	history_step_t *step = history_push(tr("Order Layers", null));
	step->prev_order     = prev_order;
}

void history_merge_layers() {
	history_copy_merging_layers();

	history_step_t *step = history_push(tr("Merge Layers", null));
	step->layer -= 1; // Merge down
	if (slot_layer_has_masks(context_raw->layer, true)) {
		step->layer -= slot_layer_get_masks(context_raw->layer, true)->length;
	}
	array_shift(history_steps); // Merge consumes 2 steps
	history_undos--;
	// TODO: use undo layer in app_merge_down to save memory
}

void history_apply_mask() {
	if (slot_layer_is_group_mask(context_raw->layer)) {
		slot_layer_t         *group  = context_raw->layer->parent;
		slot_layer_t_array_t *layers = slot_layer_get_children(group);
		array_insert(layers, 0, context_raw->layer);
		history_copy_merging_layers2(layers);
	}
	else {
		slot_layer_t_array_t *layers = any_array_create_from_raw(
		    (any[]){
		        context_raw->layer,
		        context_raw->layer->parent,
		    },
		    2);
		history_copy_merging_layers2(layers);
	}
	history_push(tr("Apply Mask", null));
}

void history_invert_mask() {
	history_push(tr("Invert Mask", null));
}

void history_apply_filter() {
	history_copy_to_undo(context_raw->layer->id, history_undo_i, true);
	history_push(tr("Apply Filter", null));
}

void history_to_fill_layer() {
	history_copy_to_undo(context_raw->layer->id, history_undo_i, false);
	history_push(tr("To Fill Layer", null));
}

void history_to_fill_mask() {
	history_copy_to_undo(context_raw->layer->id, history_undo_i, true);
	history_push(tr("To Fill Mask", null));
}

void history_to_paint_layer() {
	history_copy_to_undo(context_raw->layer->id, history_undo_i, false);
	history_push(tr("To Paint Layer", null));
}

void history_to_paint_mask() {
	history_copy_to_undo(context_raw->layer->id, history_undo_i, true);
	history_push(tr("To Paint Mask", null));
}

void history_layer_opacity() {
	history_push(tr("Layer Opacity", null));
}

// void history_layer_object() {
// 	history_push("Layer Object");
// }

void history_layer_blending() {
	history_push(tr("Layer Blending", null));
}

void history_new_material() {
	history_step_t *step = history_push(tr("New Material", null));
	step->canvas_type    = 0;
	step->canvas         = util_clone_canvas(context_raw->material->canvas);
}

void history_delete_material() {
	history_step_t *step = history_push(tr("Delete Material", null));
	step->canvas_type    = 0;
	step->canvas         = util_clone_canvas(context_raw->material->canvas);
}

void history_duplicate_material() {
	history_step_t *step = history_push(tr("Duplicate Material", null));
	step->canvas_type    = 0;
	step->canvas         = util_clone_canvas(context_raw->material->canvas);
}

void history_delete_material_group(node_group_t *group) {
	history_step_t *step = history_push(tr("Delete Node Group", null));
	step->canvas_type    = CANVAS_TYPE_MATERIAL;
	step->canvas_group   = array_index_of(project_material_groups, group);
	step->canvas         = util_clone_canvas(group->canvas);
}

history_step_t *history_push(string_t *name) {
	#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	string_t *filename = string_equals(project_filepath, "")
	                         ? ui_files_filename
	                         : substring(project_filepath, string_last_index_of(project_filepath, PATH_SEP) + 1, string_length(project_filepath) - 4);
	sys_title_set(string_join(string_join(filename, "* - "), manifest_title));
	#endif

	if (config_raw->touch_ui) {
		// Refresh undo & redo buttons
		ui_menubar_menu_handle->redraws = 2;
	}

	if (history_undos < config_raw->undo_steps) {
		history_undos++;
	}
	if (history_redos > 0) {
		for (i32 i = 0; i < history_redos; ++i) {
			array_pop(history_steps);
		}
		history_redos = 0;
	}

	i32 opos = array_index_of(project_paint_objects, context_raw->paint_object);
	i32 lpos = array_index_of(project_layers, context_raw->layer);
	i32 mpos = array_index_of(project_materials, context_raw->material);
	i32 bpos = array_index_of(project_brushes, context_raw->brush);

	history_step_t *step =
	    GC_ALLOC_INIT(history_step_t, {.name           = name,
	                                   .layer          = lpos,
	                                   .layer_type     = slot_layer_is_mask(context_raw->layer)    ? LAYER_SLOT_TYPE_MASK
	                                                     : slot_layer_is_group(context_raw->layer) ? LAYER_SLOT_TYPE_GROUP
	                                                                                               : LAYER_SLOT_TYPE_LAYER,
	                                   .layer_parent   = context_raw->layer->parent == null ? -1 : array_index_of(project_layers, context_raw->layer->parent),
	                                   .object         = opos,
	                                   .material       = mpos,
	                                   .brush          = bpos,
	                                   .layer_opacity  = context_raw->layer->mask_opacity,
	                                   .layer_object   = context_raw->layer->object_mask,
	                                   .layer_blending = context_raw->layer->blending});

	any_array_push(history_steps, step);

	while (history_steps->length > config_raw->undo_steps + 1) {
		array_shift(history_steps);
	}
	return history_steps->buffer[history_steps->length - 1];
}

void history_redo_merge_layers() {
	history_copy_merging_layers();
}

void history_copy_merging_layers() {
	slot_layer_t *lay = context_raw->layer;
	history_copy_to_undo(lay->id, history_undo_i, slot_layer_is_mask(context_raw->layer));

	i32 below = array_index_of(project_layers, lay) - 1;
	lay       = project_layers->buffer[below];
	history_copy_to_undo(lay->id, history_undo_i, slot_layer_is_mask(context_raw->layer));
}

void history_copy_merging_layers2(slot_layer_t_array_t *layers) {
	for (i32 i = 0; i < layers->length; ++i) {
		slot_layer_t *layer = layers->buffer[i];
		history_copy_to_undo(layer->id, history_undo_i, slot_layer_is_mask(layer));
	}
}

void history_swap_active() {
	slot_layer_t *undo_layer = history_undo_layers->buffer[history_undo_i];
	slot_layer_swap(undo_layer, context_raw->layer);
	history_undo_i = (history_undo_i + 1) % config_raw->undo_steps;
}

void history_copy_to_undo(i32 from_id, i32 to_id, bool is_mask) {
	if (is_mask) {
		render_path_set_target(string_join("texpaint_undo", i32_to_string(to_id)), null, null, GPU_CLEAR_NONE, 0, 0.0);
		render_path_bind_target(string_join("texpaint", i32_to_string(from_id)), "tex");
		// render_path_draw_shader("Scene/copy_pass/copyR8_pass");
		render_path_draw_shader("Scene/copy_pass/copy_pass");
	}
	else if (context_raw->layer->texpaint_sculpt != null) {
		render_path_set_target(string_join("texpaint_sculpt_undo", i32_to_string(to_id)), null, null, GPU_CLEAR_NONE, 0, 0.0);
		render_path_bind_target(string_join("texpaint_sculpt", i32_to_string(from_id)), "tex");
		render_path_draw_shader("Scene/copy_pass/copyRGBA128_pass");
	}
	else {
		string_t_array_t *additional = any_array_create_from_raw(
		    (any[]){
		        string_join("texpaint_nor_undo", i32_to_string(to_id)),
		        string_join("texpaint_pack_undo", i32_to_string(to_id)),
		    },
		    2);
		render_path_set_target(string_join("texpaint_undo", i32_to_string(to_id)), additional, null, GPU_CLEAR_NONE, 0, 0.0);
		render_path_bind_target(string_join("texpaint", i32_to_string(from_id)), "tex0");
		render_path_bind_target(string_join("texpaint_nor", i32_to_string(from_id)), "tex1");
		render_path_bind_target(string_join("texpaint_pack", i32_to_string(from_id)), "tex2");

		gpu_texture_format_t format = base_bits_handle->i == TEXTURE_BITS_BITS8    ? GPU_TEXTURE_FORMAT_RGBA32
		                              : base_bits_handle->i == TEXTURE_BITS_BITS16 ? GPU_TEXTURE_FORMAT_RGBA64
		                                                                           : GPU_TEXTURE_FORMAT_RGBA128;

		string_t *pipe = format == GPU_TEXTURE_FORMAT_RGBA32   ? "copy_mrt3_pass"
		                 : format == GPU_TEXTURE_FORMAT_RGBA64 ? "copy_mrt3RGBA64_pass"
		                                                       : "copy_mrt3RGBA128_pass";
		render_path_draw_shader(string_join("Scene/copy_mrt3_pass/", pipe));
	}
	history_undo_i = (history_undo_i + 1) % config_raw->undo_steps;
}

ui_node_canvas_t *history_get_canvas(history_step_t *step) {
	if (step->canvas_group == -1) {
		return project_materials->buffer[step->material]->canvas;
	}
	else {
		return project_material_groups->buffer[step->canvas_group]->canvas;
	}
}

void history_set_canvas(history_step_t *step, ui_node_canvas_t *canvas) {
	if (step->canvas_group == -1) {
		project_materials->buffer[step->material]->canvas = canvas;
	}
	else {
		project_material_groups->buffer[step->canvas_group]->canvas = canvas;
	}
}

void history_swap_canvas(history_step_t *step) {
	if (step->canvas_type == 0) {
		ui_node_canvas_t *_canvas = history_get_canvas(step);
		history_set_canvas(step, step->canvas);
		step->canvas          = _canvas;
		context_raw->material = project_materials->buffer[step->material];
	}
	else {
		ui_node_canvas_t *_canvas                    = project_brushes->buffer[step->brush]->canvas;
		project_brushes->buffer[step->brush]->canvas = step->canvas;
		step->canvas                                 = _canvas;
		context_raw->brush                           = project_brushes->buffer[step->brush];
	}

	ui_nodes_t *nodes                = ui_nodes_get_nodes();
	nodes->nodes_selected_id->length = 0;

	ui_nodes_canvas_changed();
	ui_nodes_hwnd->redraws = 2;
}
