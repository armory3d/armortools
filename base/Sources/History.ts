
class History {

	static steps: step_t[];
	static undo_i: i32 = 0; // Undo layer
	static undos: i32 = 0; // Undos available
	static redos: i32 = 0; // Redos available
	///if (is_paint || is_sculpt)
	static push_undo: bool = false; // Store undo on next paint
	static undo_layers: SlotLayerRaw[] = null;
	///end
	///if is_sculpt
	static push_undo2: bool = false;
	///end

	static undo = () => {
		if (History.undos > 0) {
			let active: i32 = History.steps.length - 1 - History.redos;
			let step: step_t = History.steps[active];

			if (step.name == tr("Edit Nodes")) {
				History.swap_canvas(step);
			}

			///if (is_paint || is_sculpt)
			else if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
				Context.raw.layer = Project.layers[step.layer];
				SlotLayer.delete(Context.raw.layer);
				Context.raw.layer = Project.layers[step.layer > 0 ? step.layer - 1 : 0];
			}
			else if (step.name == tr("New Group")) {
				Context.raw.layer = Project.layers[step.layer];
				// The layer below is the only layer in the group. Its layer masks are automatically unparented, too.
				Project.layers[step.layer - 1].parent = null;
				SlotLayer.delete(Context.raw.layer);
				Context.raw.layer = Project.layers[step.layer > 0 ? step.layer - 1 : 0];
			}
			else if (step.name == tr("Delete Layer")) {
				let parent: SlotLayerRaw = step.layer_parent > 0 ? Project.layers[step.layer_parent - 1] : null;
				let l: SlotLayerRaw = SlotLayer.create("", step.layer_type, parent);
				Project.layers.splice(step.layer, 0, l);
				Context.set_layer(l);
				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				SlotLayer.swap(l, lay);
				l.maskOpacity = step.layer_opacity;
				l.blending = step.layer_blending;
				l.objectMask = step.layer_object;
				MakeMaterial.parse_mesh_material();

				// Undo at least second time in order to avoid empty groups
				if (step.layer_type == layer_slot_type_t.GROUP) {
					Base.notify_on_next_frame(() => {
						// 1. Undo deleting group masks
						let n: i32 = 1;
						while (History.steps[active - n].layer_type == layer_slot_type_t.MASK) {
							History.undo();
							++n;
						}
						// 2. Undo a mask to have a non empty group
						History.undo();
					});
				}
			}
			else if (step.name == tr("Clear Layer")) {
				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				SlotLayer.swap(Context.raw.layer, lay);
				Context.raw.layer_preview_dirty = true;
			}
			else if (step.name == tr("Duplicate Layer")) {
				let children: SlotLayerRaw[] = SlotLayer.get_recursive_children(Project.layers[step.layer]);
				let position: i32 = step.layer + 1;
				if (children != null)
					position += children.length;

				Context.raw.layer = Project.layers[position];
				SlotLayer.delete(Context.raw.layer);
			}
			else if (step.name == tr("Order Layers")) {
				let target: SlotLayerRaw = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == tr("Merge Layers")) {
				Context.raw.layer = Project.layers[step.layer];
				SlotLayer.delete(Context.raw.layer);

				let parent: SlotLayerRaw = step.layer_parent > 0 ? Project.layers[step.layer_parent - 2] : null;
				let l: SlotLayerRaw = SlotLayer.create("", step.layer_type, parent);
				Project.layers.splice(step.layer, 0, l);
				Context.set_layer(l);

				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				SlotLayer.swap(Context.raw.layer, lay);

				l = SlotLayer.create("", step.layer_type, parent);
				Project.layers.splice(step.layer + 1, 0, l);
				Context.set_layer(l);

				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				lay = History.undo_layers[History.undo_i];
				SlotLayer.swap(Context.raw.layer, lay);

				Context.raw.layer.maskOpacity = step.layer_opacity;
				Context.raw.layer.blending = step.layer_blending;
				Context.raw.layer.objectMask = step.layer_object;
				Context.raw.layers_preview_dirty = true;
				MakeMaterial.parse_mesh_material();
			}
			else if (step.name == tr("Apply Mask")) {
				// First restore the layer(s)
				let maskPosition: i32 = step.layer;
				let currentLayer: SlotLayerRaw = null;
				// The layer at the old mask position is a mask, i.e. the layer had multiple masks before.
				if (SlotLayer.is_mask(Project.layers[maskPosition])) {
					currentLayer = Project.layers[maskPosition].parent;
				}
				else if (SlotLayer.is_layer(Project.layers[maskPosition]) || SlotLayer.is_group(Project.layers[maskPosition])) {
					currentLayer = Project.layers[maskPosition];
				}

				let layersToRestore: SlotLayerRaw[] = SlotLayer.is_group(currentLayer) ? SlotLayer.get_children(currentLayer) : [currentLayer];
				layersToRestore.reverse();

				for (let layer of layersToRestore) {
					// Replace the current layer's content with the old one
					Context.raw.layer = layer;
					History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
					let oldLayer: SlotLayerRaw = History.undo_layers[History.undo_i];
					SlotLayer.swap(Context.raw.layer, oldLayer);
				}

				// Now restore the applied mask
				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let mask: SlotLayerRaw = History.undo_layers[History.undo_i];
				Base.new_mask(false, currentLayer, maskPosition);
				SlotLayer.swap(Context.raw.layer, mask);
				Context.raw.layers_preview_dirty = true;
				Context.set_layer(Context.raw.layer);
			}
			else if (step.name == tr("Invert Mask")) {
				let _next = () => {
					Context.raw.layer = Project.layers[step.layer];
					SlotLayer.invert_mask(Context.raw.layer);
				}
				app_notify_on_init(_next);
			}
			else if (step.name == "Apply Filter") {
				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				Context.set_layer(Project.layers[step.layer]);
				SlotLayer.swap(Context.raw.layer, lay);
				Base.new_mask(false, Context.raw.layer);
				SlotLayer.swap(Context.raw.layer, lay);
				Context.raw.layer_preview_dirty = true;
			}
			else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
				SlotLayer.to_paint_layer(Context.raw.layer);
				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				SlotLayer.swap(Context.raw.layer, lay);
			}
			else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				SlotLayer.swap(Context.raw.layer, lay);
				Context.raw.layer.fill_layer = Project.materials[step.material];
			}
			else if (step.name == tr("Layer Opacity")) {
				Context.set_layer(Project.layers[step.layer]);
				let t: f32 = Context.raw.layer.maskOpacity;
				Context.raw.layer.maskOpacity = step.layer_opacity;
				step.layer_opacity = t;
				MakeMaterial.parse_mesh_material();
			}
			else if (step.name == tr("Layer Blending")) {
				Context.set_layer(Project.layers[step.layer]);
				let t: blend_type_t = Context.raw.layer.blending;
				Context.raw.layer.blending = step.layer_blending;
				step.layer_blending = t;
				MakeMaterial.parse_mesh_material();
			}
			else if (step.name == tr("Delete Node Group")) {
				Project.material_groups.splice(step.canvas_group, 0, { canvas: null, nodes: zui_nodes_create() });
				History.swap_canvas(step);
			}
			else if (step.name == tr("New Material")) {
				Context.raw.material = Project.materials[step.material];
				step.canvas = Context.raw.material.canvas;
				SlotMaterial.delete(Context.raw.material);
			}
			else if (step.name == tr("Delete Material")) {
				Context.raw.material = SlotMaterial.create(Project.materials[0].data);
				Project.materials.splice(step.material, 0, Context.raw.material);
				Context.raw.material.canvas = step.canvas;
				UINodes.canvas_changed();
				UINodes.hwnd.redraws = 2;
			}
			else if (step.name == tr("Duplicate Material")) {
				Context.raw.material = Project.materials[step.material];
				step.canvas = Context.raw.material.canvas;
				SlotMaterial.delete(Context.raw.material);
			}
			else { // Paint operation
				History.undo_i = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				Context.select_paint_object(Project.paint_objects[step.object]);
				Context.set_layer(Project.layers[step.layer]);
				SlotLayer.swap(Context.raw.layer, lay);
				Context.raw.layer_preview_dirty = true;
			}
			///end

			History.undos--;
			History.redos++;
			Context.raw.ddirty = 2;

			///if (is_paint || is_sculpt)
			UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
			UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
			if (UIView2D.show) {
				UIView2D.hwnd.redraws = 2;
			}

			if (Config.raw.touch_ui) {
				// Refresh undo & redo buttons
				UIMenubar.menu_handle.redraws = 2;
			}
			///end
		}
	}

	static redo = () => {
		if (History.redos > 0) {
			let active: i32 = History.steps.length - History.redos;
			let step: step_t = History.steps[active];

			if (step.name == tr("Edit Nodes")) {
				History.swap_canvas(step);
			}

			///if (is_paint || is_sculpt)
			else if (step.name == tr("New Layer") || step.name == tr("New Black Mask") || step.name == tr("New White Mask") || step.name == tr("New Fill Mask")) {
				let parent: SlotLayerRaw = step.layer_parent > 0 ? Project.layers[step.layer_parent - 1] : null;
				let l: SlotLayerRaw = SlotLayer.create("", step.layer_type, parent);
				Project.layers.splice(step.layer, 0, l);
				if (step.name == tr("New Black Mask")) {
					Base.notify_on_next_frame(() => {
						SlotLayer.clear(l, 0x00000000);
					});
				}
				else if (step.name == tr("New White Mask")) {
					Base.notify_on_next_frame(() => {
						SlotLayer.clear(l, 0xffffffff);
					});
				}
				else if (step.name == tr("New Fill Mask")) {
					Base.notify_on_next_frame(() => {
						Context.raw.material = Project.materials[step.material];
						SlotLayer.to_fill_layer(l);
					});
				}
				Context.raw.layer_preview_dirty = true;
				Context.set_layer(l);
			}
			else if (step.name == tr("New Group")) {
				let l: SlotLayerRaw = Project.layers[step.layer - 1];
				let group: SlotLayerRaw = Base.new_group();
				array_remove(Project.layers, group);
				Project.layers.splice(step.layer, 0, group);
				l.parent = group;
				Context.set_layer(group);
			}
			else if (step.name == tr("Delete Layer")) {
				Context.raw.layer = Project.layers[step.layer];
				History.swap_active();
				SlotLayer.delete(Context.raw.layer);

				// Redoing the last delete would result in an empty group
				// Redo deleting all group masks + the group itself
				if (step.layer_type == layer_slot_type_t.LAYER && History.steps.length >= active + 2 && (History.steps[active + 1].layer_type == layer_slot_type_t.GROUP || History.steps[active + 1].layer_type == layer_slot_type_t.MASK)) {
					let n: i32 = 1;
					while (History.steps[active + n].layer_type == layer_slot_type_t.MASK) {
						++n;
					}
					Base.notify_on_next_frame(() => {
						for (let i: i32 = 0; i < n; ++i) History.redo();
					});
				}
			}
			else if (step.name == tr("Clear Layer")) {
				Context.raw.layer = Project.layers[step.layer];
				History.swap_active();
				SlotLayer.clear(Context.raw.layer);
				Context.raw.layer_preview_dirty = true;
			}
			else if (step.name == tr("Duplicate Layer")) {
				Context.raw.layer = Project.layers[step.layer];
				let _next = () => {
					Base.duplicate_layer(Context.raw.layer);
				}
				Base.notify_on_next_frame(_next);
			}
			else if (step.name == tr("Order Layers")) {
				let target: SlotLayerRaw = Project.layers[step.prev_order];
				Project.layers[step.prev_order] = Project.layers[step.layer];
				Project.layers[step.layer] = target;
			}
			else if (step.name == tr("Merge Layers")) {
				Context.raw.layer = Project.layers[step.layer + 1];
				app_notify_on_init(History.redo_merge_layers);
				app_notify_on_init(Base.merge_down);
			}
			else if (step.name == tr("Apply Mask")) {
				Context.raw.layer = Project.layers[step.layer];
					if (SlotLayer.is_group_mask(Context.raw.layer)) {
						let group: SlotLayerRaw = Context.raw.layer.parent;
						let layers: SlotLayerRaw[] = SlotLayer.get_children(group);
						layers.splice(0, 0, Context.raw.layer);
						History.copy_merging_layers2(layers);
					}
					else History.copy_merging_layers2([Context.raw.layer, Context.raw.layer.parent]);

				let _next = () => {
					SlotLayer.apply_mask(Context.raw.layer);
					Context.set_layer(Context.raw.layer);
					Context.raw.layers_preview_dirty = true;
				}
				Base.notify_on_next_frame(_next);
			}
			else if (step.name == tr("Invert Mask")) {
				let _next = () => {
					Context.raw.layer = Project.layers[step.layer];
					SlotLayer.invert_mask(Context.raw.layer);
				}
				app_notify_on_init(_next);
			}
			else if (step.name == tr("Apply Filter")) {
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				Context.set_layer(Project.layers[step.layer]);
				SlotLayer.swap(Context.raw.layer, lay);
				Base.new_mask(false, lay);
				SlotLayer.swap(Context.raw.layer, lay);
				Context.raw.layer_preview_dirty = true;
				History.undo_i = (History.undo_i + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Fill Layer") || step.name == tr("To Fill Mask")) {
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				SlotLayer.swap(Context.raw.layer, lay);
				Context.raw.layer.fill_layer = Project.materials[step.material];
				History.undo_i = (History.undo_i + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("To Paint Layer") || step.name == tr("To Paint Mask")) {
				SlotLayer.to_paint_layer(Context.raw.layer);
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				SlotLayer.swap(Context.raw.layer, lay);
				History.undo_i = (History.undo_i + 1) % Config.raw.undo_steps;
			}
			else if (step.name == tr("Layer Opacity")) {
				Context.set_layer(Project.layers[step.layer]);
				let t: f32 = Context.raw.layer.maskOpacity;
				Context.raw.layer.maskOpacity = step.layer_opacity;
				step.layer_opacity = t;
				MakeMaterial.parse_mesh_material();
			}
			else if (step.name == tr("Layer Blending")) {
				Context.set_layer(Project.layers[step.layer]);
				let t: blend_type_t = Context.raw.layer.blending;
				Context.raw.layer.blending = step.layer_blending;
				step.layer_blending = t;
				MakeMaterial.parse_mesh_material();
			}
			else if (step.name == tr("Delete Node Group")) {
				History.swap_canvas(step);
				array_remove(Project.material_groups, Project.material_groups[step.canvas_group]);
			}
			else if (step.name == tr("New Material")) {
				Context.raw.material = SlotMaterial.create(Project.materials[0].data);
				Project.materials.splice(step.material, 0, Context.raw.material);
				Context.raw.material.canvas = step.canvas;
				UINodes.canvas_changed();
				UINodes.hwnd.redraws = 2;
			}
			else if (step.name == tr("Delete Material")) {
				Context.raw.material = Project.materials[step.material];
				step.canvas = Context.raw.material.canvas;
				SlotMaterial.delete(Context.raw.material);
			}
			else if (step.name == tr("Duplicate Material")) {
				Context.raw.material = SlotMaterial.create(Project.materials[0].data);
				Project.materials.splice(step.material, 0, Context.raw.material);
				Context.raw.material.canvas = step.canvas;
				UINodes.canvas_changed();
				UINodes.hwnd.redraws = 2;
			}
			else { // Paint operation
				let lay: SlotLayerRaw = History.undo_layers[History.undo_i];
				Context.select_paint_object(Project.paint_objects[step.object]);
				Context.set_layer(Project.layers[step.layer]);
				SlotLayer.swap(Context.raw.layer, lay);
				Context.raw.layer_preview_dirty = true;
				History.undo_i = (History.undo_i + 1) % Config.raw.undo_steps;
			}
			///end

			History.undos++;
			History.redos--;
			Context.raw.ddirty = 2;

			///if (is_paint || is_sculpt)
			UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
			UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
			if (UIView2D.show) UIView2D.hwnd.redraws = 2;

			if (Config.raw.touch_ui) {
				// Refresh undo & redo buttons
				UIMenubar.menu_handle.redraws = 2;
			}
			///end
		}
	}

	static reset = () => {
		///if (is_paint || is_sculpt)
		History.steps = [{name: tr("New"), layer: 0, layer_type: layer_slot_type_t.LAYER, layer_parent: -1, object: 0, material: 0, brush: 0}];
		///end
		///if is_lab
		History.steps = [{name: tr("New")}];
		///end

		History.undos = 0;
		History.redos = 0;
		History.undo_i = 0;
	}

	///if (is_paint || is_sculpt)
	static edit_nodes = (canvas: zui_node_canvas_t, canvas_type: i32, canvas_group: Null<i32> = null) => {
	///end
	///if is_lab
	static edit_nodes = (canvas: zui_node_canvas_t, canvas_group: Null<i32> = null) => {
	///end
		let step: step_t = History.push(tr("Edit Nodes"));
		step.canvas_group = canvas_group;
		///if (is_paint || is_sculpt)
		step.canvas_type = canvas_type;
		///end
		step.canvas = JSON.parse(JSON.stringify(canvas));
	}

	///if (is_paint || is_sculpt)
	static paint = () => {
		let isMask: bool = SlotLayer.is_mask(Context.raw.layer);
		History.copy_to_undo(Context.raw.layer.id, History.undo_i, isMask);

		History.push_undo = false;
		History.push(tr(UIToolbar.tool_names[Context.raw.tool]));
	}

	static new_layer = () => {
		History.push(tr("New Layer"));
	}

	static new_black_mask = () => {
		History.push(tr("New Black Mask"));
	}

	static new_white_mask = () => {
		History.push(tr("New White Mask"));
	}

	static new_fill_mask = () => {
		History.push(tr("New Fill Mask"));
	}

	static new_group = () => {
		History.push(tr("New Group"));
	}

	static duplicate_layer = () => {
		History.push(tr("Duplicate Layer"));
	}

	static delete_layer = () => {
		History.swap_active();
		History.push(tr("Delete Layer"));
	}

	static clear_layer = () => {
		History.swap_active();
		History.push(tr("Clear Layer"));
	}

	static order_layers = (prevOrder: i32) => {
		let step: step_t = History.push(tr("Order Layers"));
		step.prev_order = prevOrder;
	}

	static merge_layers = () => {
		History.copy_merging_layers();

		let step: step_t = History.push(tr("Merge Layers"));
		step.layer -= 1; // Merge down
		if (SlotLayer.has_masks(Context.raw.layer)) {
			step.layer -= SlotLayer.get_masks(Context.raw.layer).length;
		}
		History.steps.shift(); // Merge consumes 2 steps
		History.undos--;
		// TODO: use undo layer in app_merge_down to save memory
	}

	static apply_mask = () => {
		if (SlotLayer.is_group_mask(Context.raw.layer)) {
			let group: SlotLayerRaw = Context.raw.layer.parent;
			let layers: SlotLayerRaw[] = SlotLayer.get_children(group);
			layers.splice(0, 0, Context.raw.layer);
			History.copy_merging_layers2(layers);
		}
		else History.copy_merging_layers2([Context.raw.layer, Context.raw.layer.parent]);
		History.push(tr("Apply Mask"));
	}


	static invert_mask = () => {
		History.push(tr("Invert Mask"));
	}

	static apply_filter = () => {
		History.copy_to_undo(Context.raw.layer.id, History.undo_i, true);
		History.push(tr("Apply Filter"));
	}

	static to_fill_layer = () => {
		History.copy_to_undo(Context.raw.layer.id, History.undo_i, false);
		History.push(tr("To Fill Layer"));
	}

	static to_fill_mask = () => {
		History.copy_to_undo(Context.raw.layer.id, History.undo_i, true);
		History.push(tr("To Fill Mask"));
	}

	static to_paint_layer = () => {
		History.copy_to_undo(Context.raw.layer.id, History.undo_i, false);
		History.push(tr("To Paint Layer"));
	}

	static to_paint_mask = () => {
		History.copy_to_undo(Context.raw.layer.id, History.undo_i, true);
		History.push(tr("To Paint Mask"));
	}

	static layer_opacity = () => {
		History.push(tr("Layer Opacity"));
	}

	// static layer_object = () => {
	// 	History.push("Layer Object");
	// }

	static layer_blending = () => {
		History.push(tr("Layer Blending"));
	}

	static new_material = () => {
		let step: step_t = History.push(tr("New Material"));
		step.canvas_type = 0;
		step.canvas = JSON.parse(JSON.stringify(Context.raw.material.canvas));
	}

	static delete_material = () => {
		let step: step_t = History.push(tr("Delete Material"));
		step.canvas_type = 0;
		step.canvas = JSON.parse(JSON.stringify(Context.raw.material.canvas));
	}

	static duplicate_material = () => {
		let step: step_t = History.push(tr("Duplicate Material"));
		step.canvas_type = 0;
		step.canvas = JSON.parse(JSON.stringify(Context.raw.material.canvas));
	}

	static delete_material_group = (group: node_group_t) => {
		let step: step_t = History.push(tr("Delete Node Group"));
		step.canvas_type = canvas_type_t.MATERIAL;
		step.canvas_group = Project.material_groups.indexOf(group);
		step.canvas = JSON.parse(JSON.stringify(group.canvas));
	}
	///end

	static push = (name: string): step_t => {
		///if (krom_windows || krom_linux || krom_darwin)
		let filename: string = Project.filepath == "" ? UIFiles.filename : Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		sys_title_set(filename + "* - " + manifest_title);
		///end

		if (Config.raw.touch_ui) {
			// Refresh undo & redo buttons
			UIMenubar.menu_handle.redraws = 2;
		}

		if (History.undos < Config.raw.undo_steps) History.undos++;
		if (History.redos > 0) {
			for (let i: i32 = 0; i < History.redos; ++i) History.steps.pop();
			History.redos = 0;
		}

		///if (is_paint || is_sculpt)
		let opos: i32 = Project.paint_objects.indexOf(Context.raw.paint_object);
		let lpos: i32 = Project.layers.indexOf(Context.raw.layer);
		let mpos: i32 = Project.materials.indexOf(Context.raw.material);
		let bpos: i32 = Project.brushes.indexOf(Context.raw.brush);

		History.steps.push({
			name: name,
			layer: lpos,
			layer_type: SlotLayer.is_mask(Context.raw.layer) ? layer_slot_type_t.MASK : SlotLayer.is_group(Context.raw.layer) ? layer_slot_type_t.GROUP : layer_slot_type_t.LAYER,
			layer_parent: Context.raw.layer.parent == null ? -1 : Project.layers.indexOf(Context.raw.layer.parent),
			object: opos,
			material: mpos,
			brush: bpos,
			layer_opacity: Context.raw.layer.maskOpacity,
			layer_object: Context.raw.layer.objectMask,
			layer_blending: Context.raw.layer.blending
		});
		///end

		///if is_lab
		History.steps.push({
			name: name
		});
		///end

		while (History.steps.length > Config.raw.undo_steps + 1) History.steps.shift();
		return History.steps[History.steps.length - 1];
	}

	///if (is_paint || is_sculpt)
	static redo_merge_layers = () => {
		History.copy_merging_layers();
	}

	static copy_merging_layers = () => {
		let lay: SlotLayerRaw = Context.raw.layer;
		History.copy_to_undo(lay.id, History.undo_i, SlotLayer.is_mask(Context.raw.layer));

		let below: i32 = Project.layers.indexOf(lay) - 1;
		lay = Project.layers[below];
		History.copy_to_undo(lay.id, History.undo_i, SlotLayer.is_mask(Context.raw.layer));
	}

	static copy_merging_layers2 = (layers: SlotLayerRaw[]) => {
		for (let layer of layers)
		History.copy_to_undo(layer.id, History.undo_i, SlotLayer.is_mask(layer));
	}

	static swap_active = () => {
		let undoLayer: SlotLayerRaw = History.undo_layers[History.undo_i];
		SlotLayer.swap(undoLayer, Context.raw.layer);
		History.undo_i = (History.undo_i + 1) % Config.raw.undo_steps;
	}

	static copy_to_undo = (fromId: i32, toId: i32, isMask: bool) => {

		///if is_sculpt
		isMask = true;
		///end

		if (isMask) {
			render_path_set_target("texpaint_undo" + toId);
			render_path_bind_target("texpaint" + fromId, "tex");
			// render_path_draw_shader("shader_datas/copy_pass/copyR8_pass");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");
		}
		else {
			render_path_set_target("texpaint_undo" + toId, ["texpaint_nor_undo" + toId, "texpaint_pack_undo" + toId]);
			render_path_bind_target("texpaint" + fromId, "tex0");
			render_path_bind_target("texpaint_nor" + fromId, "tex1");
			render_path_bind_target("texpaint_pack" + fromId, "tex2");
			render_path_draw_shader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		}
		History.undo_i = (History.undo_i + 1) % Config.raw.undo_steps;
	}
	///end

	static get_canvas_owner = (step: step_t): any => {
		///if (is_paint || is_sculpt)
		return step.canvas_group == null ?
			Project.materials[step.material] :
			Project.material_groups[step.canvas_group];
		///end

		///if is_lab
		return null;
		///end
	}

	static swap_canvas = (step: step_t) => {
		///if (is_paint || is_sculpt)
		if (step.canvas_type == 0) {
			let _canvas: zui_node_canvas_t = History.get_canvas_owner(step).canvas;
			History.get_canvas_owner(step).canvas = step.canvas;
			step.canvas = _canvas;
			Context.raw.material = Project.materials[step.material];
		}
		else {
			let _canvas: zui_node_canvas_t = Project.brushes[step.brush].canvas;
			Project.brushes[step.brush].canvas = step.canvas;
			step.canvas = _canvas;
			Context.raw.brush = Project.brushes[step.brush];
		}
		///end

		///if is_lab
		let _canvas: zui_node_canvas_t = History.get_canvas_owner(step).canvas;
		History.get_canvas_owner(step).canvas = step.canvas;
		step.canvas = _canvas;
		///end

		UINodes.canvas_changed();
		UINodes.hwnd.redraws = 2;
	}
}

type step_t = {
	name?: string;
	canvas?: zui_node_canvas_t; // Node history
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
