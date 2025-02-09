
let layers_temp_image: image_t = null;
let layers_expa: image_t = null;
let layers_expb: image_t = null;
let layers_expc: image_t = null;
let layers_default_base: f32 = 0.5;
let layers_default_rough: f32 = 0.4;
let layers_max_layers: i32 =
///if (arm_android || arm_ios)
	18;
///else
	255;
///end

let _layers_uv_type: uv_type_t;
let _layers_decal_mat: mat4_t;
let _layers_position: i32;
let _layers_base_color: i32;
let _layers_occlusion: f32;
let _layers_roughness: f32;
let _layers_metallic: f32;

function layers_init() {
	///if (is_paint || is_sculpt)
	slot_layer_clear(project_layers[0], color_from_floats(layers_default_base, layers_default_base, layers_default_base, 1.0));
	///end

	///if is_lab
	let texpaint: render_target_t = map_get(render_path_render_targets, "texpaint");
	let texpaint_nor: render_target_t = map_get(render_path_render_targets, "texpaint_nor");
	let texpaint_pack: render_target_t = map_get(render_path_render_targets, "texpaint_pack");
	g2_begin(texpaint._image);
	g2_draw_scaled_image(resource_get("placeholder.k"), 0, 0, config_get_texture_res_x(), config_get_texture_res_y()); // Base
	g2_end();
	g4_begin(texpaint_nor._image);
	g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
	g4_end();
	g4_begin(texpaint_pack._image);
	g4_clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
	g4_end();
	let texpaint_nor_empty: render_target_t = map_get(render_path_render_targets, "texpaint_nor_empty");
	let texpaint_pack_empty: render_target_t = map_get(render_path_render_targets, "texpaint_pack_empty");
	g4_begin(texpaint_nor_empty._image);
	g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
	g4_end();
	g4_begin(texpaint_pack_empty._image);
	g4_clear(color_from_floats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
	g4_end();
	///end
}

function layers_resize() {
	let conf: config_t = config_raw;
	if (base_res_handle.position >= math_floor(texture_res_t.RES16384)) { // Save memory for >=16k
		conf.undo_steps = 1;
		if (context_raw.undo_handle != null) {
			context_raw.undo_handle.value = conf.undo_steps;
		}
		while (history_undo_layers.length > conf.undo_steps) {
			let l: slot_layer_t = array_pop(history_undo_layers);
			app_notify_on_next_frame(function (l: slot_layer_t) {
				slot_layer_unload(l);
			}, l);
		}
	}
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		slot_layer_resize_and_set_bits(l);
	}
	for (let i: i32 = 0; i < history_undo_layers.length; ++i) {
		let l: slot_layer_t = history_undo_layers[i];
		slot_layer_resize_and_set_bits(l);
	}

	let rts: map_t<string, render_target_t> = render_path_render_targets;

	let blend0: render_target_t = map_get(rts, "texpaint_blend0");
	let _texpaint_blend0: image_t = blend0._image;
	app_notify_on_next_frame(function (_texpaint_blend0: image_t) {
		image_unload(_texpaint_blend0);
	}, _texpaint_blend0);
	blend0.width = config_get_texture_res_x();
	blend0.height = config_get_texture_res_y();
	blend0._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);

	let blend1: render_target_t = map_get(rts, "texpaint_blend1");
	let _texpaint_blend1: image_t = blend1._image;
	app_notify_on_next_frame(function (_texpaint_blend1: image_t) {
		image_unload(_texpaint_blend1);
	}, _texpaint_blend1);
	blend1.width = config_get_texture_res_x();
	blend1.height = config_get_texture_res_y();
	blend1._image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);

	context_raw.brush_blend_dirty = true;

	let blur: render_target_t = map_get(rts, "texpaint_blur");
	if (blur != null) {
		let _texpaint_blur: image_t = blur._image;
		app_notify_on_next_frame(function (_texpaint_blur: image_t) {
			image_unload(_texpaint_blur);
		}, _texpaint_blur);
		let size_x: f32 = math_floor(config_get_texture_res_x() * 0.95);
		let size_y: f32 = math_floor(config_get_texture_res_y() * 0.95);
		blur.width = size_x;
		blur.height = size_y;
		blur._image = image_create_render_target(size_x, size_y);
	}
	if (render_path_paint_live_layer != null) {
		slot_layer_resize_and_set_bits(render_path_paint_live_layer);
	}
	render_path_raytrace_ready = false; // Rebuild baketex
	context_raw.ddirty = 2;
}

function layers_set_bits() {
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		slot_layer_resize_and_set_bits(l);
	}
	for (let i: i32 = 0; i < history_undo_layers.length; ++i) {
		let l: slot_layer_t = history_undo_layers[i];
		slot_layer_resize_and_set_bits(l);
	}
}

function layers_make_temp_img() {
	///if (is_paint || is_sculpt)
	let l: slot_layer_t = project_layers[0];
	///end
	///if is_lab
	let l: brush_output_node_t = context_raw.brush_output_node_inst;
	///end

	if (layers_temp_image != null && (layers_temp_image.width != l.texpaint.width || layers_temp_image.height != l.texpaint.height || layers_temp_image.format != l.texpaint.format)) {
		let _temptex0: render_target_t = map_get(render_path_render_targets, "temptex0");
		app_notify_on_next_frame(function (_temptex0: render_target_t) {
			render_target_unload(_temptex0);
		}, _temptex0);
		map_delete(render_path_render_targets, "temptex0");
		layers_temp_image = null;
	}
	if (layers_temp_image == null) {
		let format: string = base_bits_handle.position == texture_bits_t.BITS8  ? "RGBA32" :
							 base_bits_handle.position == texture_bits_t.BITS16 ? "RGBA64" :
																				  "RGBA128";
		///if is_lab
		format = "RGBA32";
		///end

		let t: render_target_t = render_target_create();
		t.name = "temptex0";
		t.width = l.texpaint.width;
		t.height = l.texpaint.height;
		t.format = format;
		let rt: render_target_t = render_path_create_render_target(t);
		layers_temp_image = rt._image;
	}
}

function layers_make_temp_mask_img() {
	if (pipes_temp_mask_image != null && (pipes_temp_mask_image.width != config_get_texture_res_x() || pipes_temp_mask_image.height != config_get_texture_res_y())) {
		let _temp_mask_image: image_t = pipes_temp_mask_image;
		app_notify_on_next_frame(function (_temp_mask_image: image_t) {
			image_unload(_temp_mask_image);
		}, _temp_mask_image);
		pipes_temp_mask_image = null;
	}
	if (pipes_temp_mask_image == null) {
		pipes_temp_mask_image = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);
	}
}

function layers_make_export_img() {
	///if (is_paint || is_sculpt)
	let l: slot_layer_t = project_layers[0];
	///end
	///if is_lab
	let l: brush_output_node_t = context_raw.brush_output_node_inst;
	///end

	if (layers_expa != null && (layers_expa.width != l.texpaint.width || layers_expa.height != l.texpaint.height || layers_expa.format != l.texpaint.format)) {
		let _expa: image_t = layers_expa;
		let _expb: image_t = layers_expb;
		let _expc: image_t = layers_expc;
		app_notify_on_next_frame(function (_expa: image_t) {
			image_unload(_expa);
		}, _expa);
		app_notify_on_next_frame(function (_expb: image_t) {
			image_unload(_expb);
		}, _expb);
		app_notify_on_next_frame(function (_expc: image_t) {
			image_unload(_expc);
		}, _expc);
		layers_expa = null;
		layers_expb = null;
		layers_expc = null;
		map_delete(render_path_render_targets, "expa");
		map_delete(render_path_render_targets, "expb");
		map_delete(render_path_render_targets, "expc");
	}
	if (layers_expa == null) {
		let format: string = base_bits_handle.position == texture_bits_t.BITS8  ? "RGBA32" :
							 base_bits_handle.position == texture_bits_t.BITS16 ? "RGBA64" :
																				  "RGBA128";

		///if is_lab
		format = "RGBA32";
		///end

		{
			let t: render_target_t = render_target_create();
			t.name = "expa";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt: render_target_t = render_path_create_render_target(t);
			layers_expa = rt._image;
		}

		{
			let t: render_target_t = render_target_create();
			t.name = "expb";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt: render_target_t = render_path_create_render_target(t);
			layers_expb = rt._image;
		}

		{
			let t: render_target_t = render_target_create();
			t.name = "expc";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			let rt: render_target_t = render_path_create_render_target(t);
			layers_expc = rt._image;
		}
	}
}

function layers_apply_mask(l: slot_layer_t, m: slot_layer_t) {
	if (!slot_layer_is_layer(l) || !slot_layer_is_mask(m)) {
		return;
	}

	layers_make_temp_img();

	// Copy layer to temp
	g2_begin(layers_temp_image);
	g2_set_pipeline(pipes_copy);
	g2_draw_image(l.texpaint, 0, 0);
	g2_set_pipeline(null);
	g2_end();

	// Apply mask
	g4_begin(l.texpaint);
	g4_set_pipeline(pipes_apply_mask);
	g4_set_tex(pipes_tex0_mask, layers_temp_image);
	g4_set_tex(pipes_texa_mask, m.texpaint);
	g4_set_vertex_buffer(const_data_screen_aligned_vb);
	g4_set_index_buffer(const_data_screen_aligned_ib);
	g4_draw();
	g4_end();
}

function layers_commands_merge_pack(pipe: pipeline_t, i0: image_t, i1: image_t, i1pack: image_t, i1mask_opacity: f32, i1texmask: image_t, i1blending: i32 = -1) {
	g4_begin(i0);
	g4_set_pipeline(pipe);
	g4_set_tex(pipes_tex0, i1);
	g4_set_tex(pipes_tex1, i1pack);
	g4_set_tex(pipes_texmask, i1texmask);
	g4_set_tex(pipes_texa, layers_temp_image);
	g4_set_float(pipes_opac, i1mask_opacity);
	g4_set_int(pipes_blending, i1blending);
	g4_set_vertex_buffer(const_data_screen_aligned_vb);
	g4_set_index_buffer(const_data_screen_aligned_ib);
	g4_draw();
	g4_end();
}

function layers_is_fill_material(): bool {
	if (context_raw.tool == workspace_tool_t.MATERIAL) {
		return true;
	}

	let m: slot_material_t = context_raw.material;
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (l.fill_layer == m) {
			return true;
		}
	}
	return false;
}

function layers_update_fill_layers() {
	let _layer: slot_layer_t = context_raw.layer;
	let _tool: workspace_tool_t = context_raw.tool;
	let _fill_type: i32 = context_raw.fill_type_handle.position;
	let current: image_t = null;

	if (context_raw.tool == workspace_tool_t.MATERIAL) {
		if (render_path_paint_live_layer == null) {
			render_path_paint_live_layer = slot_layer_create("_live");
		}

		current = _g2_current;
		if (current != null) g2_end();

		context_raw.tool = workspace_tool_t.FILL;
		context_raw.fill_type_handle.position = fill_type_t.OBJECT;
		make_material_parse_paint_material(false);
		context_raw.pdirty = 1;
		render_path_paint_use_live_layer(true);
		render_path_paint_commands_paint(false);
		render_path_paint_dilate(true, true);
		render_path_paint_use_live_layer(false);
		context_raw.tool = _tool;
		context_raw.fill_type_handle.position = _fill_type;
		context_raw.pdirty = 0;
		context_raw.rdirty = 2;

		if (current != null) g2_begin(current);
		return;
	}

	let has_fill_layer: bool = false;
	let has_fill_mask: bool = false;
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (slot_layer_is_layer(l) && l.fill_layer == context_raw.material) {
			has_fill_layer = true;
		}
	}
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (slot_layer_is_mask(l) && l.fill_layer == context_raw.material) {
			has_fill_mask = true;
		}
	}

	if (has_fill_layer || has_fill_mask) {
		current = _g2_current;
		if (current != null) {
			g2_end();
		}
		context_raw.pdirty = 1;
		context_raw.tool = workspace_tool_t.FILL;
		context_raw.fill_type_handle.position = fill_type_t.OBJECT;

		if (has_fill_layer) {
			let first: bool = true;
			for (let i: i32 = 0; i < project_layers.length; ++i) {
				let l: slot_layer_t = project_layers[i];
				if (slot_layer_is_layer(l) && l.fill_layer == context_raw.material) {
					context_raw.layer = l;
					if (first) {
						first = false;
						make_material_parse_paint_material(false);
					}
					layers_set_object_mask();
					slot_layer_clear(l);
					render_path_paint_commands_paint(false);
					render_path_paint_dilate(true, true);
				}
			}
		}
		if (has_fill_mask) {
			let first: bool = true;
			for (let i: i32 = 0; i < project_layers.length; ++i) {
				let l: slot_layer_t = project_layers[i];
				if (slot_layer_is_mask(l) && l.fill_layer == context_raw.material) {
					context_raw.layer = l;
					if (first) {
						first = false;
						make_material_parse_paint_material(false);
					}
					layers_set_object_mask();
					slot_layer_clear(l);
					render_path_paint_commands_paint(false);
					render_path_paint_dilate(true, true);
				}
			}
		}

		context_raw.pdirty = 0;
		context_raw.ddirty = 2;
		context_raw.rdirty = 2;
		context_raw.layers_preview_dirty = true; // Repaint all layer previews as multiple layers might have changed.
		if (current != null) g2_begin(current);
		context_raw.layer = _layer;
		layers_set_object_mask();
		context_raw.tool = _tool;
		context_raw.fill_type_handle.position = _fill_type;
		make_material_parse_paint_material(false);
	}
}

function layers_update_fill_layer(parse_paint: bool = true) {
	let current: image_t = _g2_current;
	let g2_in_use: bool = _g2_in_use;
	if (g2_in_use) g2_end();

	let _tool: workspace_tool_t = context_raw.tool;
	let _fill_type: i32 = context_raw.fill_type_handle.position;
	context_raw.tool = workspace_tool_t.FILL;
	context_raw.fill_type_handle.position = fill_type_t.OBJECT;
	context_raw.pdirty = 1;

	slot_layer_clear(context_raw.layer);

	if (parse_paint) {
		make_material_parse_paint_material(false);
	}
	render_path_paint_commands_paint(false);
	render_path_paint_dilate(true, true);

	context_raw.rdirty = 2;
	context_raw.tool = _tool;
	context_raw.fill_type_handle.position = _fill_type;
	if (g2_in_use) g2_begin(current);
}

function layers_set_object_mask() {
	///if is_sculpt
	return;
	///end

	let ar: string[] = [tr("None")];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		array_push(ar, p.base.name);
	}

	let mask: i32 = context_object_mask_used() ? slot_layer_get_object_mask(context_raw.layer) : 0;
	if (context_layer_filter_used()) {
		mask = context_raw.layer_filter;
	}
	if (mask > 0) {
		if (context_raw.merged_object != null) {
			context_raw.merged_object.base.visible = false;
		}
		let o: mesh_object_t = project_paint_objects[0];
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			let p: mesh_object_t = project_paint_objects[i];
			let mask_name: string = ar[mask];
			if (p.base.name == mask_name) {
				o = p;
				break;
			}
		}
		context_select_paint_object(o);
	}
	else {
		let is_atlas: bool = slot_layer_get_object_mask(context_raw.layer) > 0 && slot_layer_get_object_mask(context_raw.layer) <= project_paint_objects.length;
		if (context_raw.merged_object == null || is_atlas || context_raw.merged_object_is_atlas) {
			let visibles: mesh_object_t[] = is_atlas ? project_get_atlas_objects(slot_layer_get_object_mask(context_raw.layer)) : null;
			util_mesh_merge(visibles);
		}
		context_select_paint_object(context_main_object());
		context_raw.paint_object.skip_context = "paint";
		context_raw.merged_object.base.visible = true;
	}
	util_uv_dilatemap_cached = false;
}

function layers_new_layer(clear: bool = true, position: i32 = -1): slot_layer_t {
	if (project_layers.length > layers_max_layers) {
		return null;
	}

	let l: slot_layer_t = slot_layer_create();
	l.object_mask = context_raw.layer_filter;

	if (position == -1) {
		if (slot_layer_is_mask(context_raw.layer)) context_set_layer(context_raw.layer.parent);
		array_insert(project_layers, array_index_of(project_layers, context_raw.layer) + 1, l);
	}
	else {
		array_insert(project_layers, position, l);
	}

	context_set_layer(l);
	let li: i32 = array_index_of(project_layers, context_raw.layer);
	if (li > 0) {
		let below: slot_layer_t = project_layers[li - 1];
		if (slot_layer_is_layer(below)) {
			context_raw.layer.parent = below.parent;
		}
	}
	if (clear) {
		app_notify_on_init(function (l: slot_layer_t) {
			slot_layer_clear(l);
		}, l);
	}
	context_raw.layer_preview_dirty = true;
	return l;
}

function layers_new_mask(clear: bool = true, parent: slot_layer_t, position: i32 = -1): slot_layer_t {
	if (project_layers.length > layers_max_layers) {
		return null;
	}
	let l: slot_layer_t = slot_layer_create("", layer_slot_type_t.MASK, parent);
	if (position == -1) {
		position = array_index_of(project_layers, parent);
	}
	array_insert(project_layers, position, l);
	context_set_layer(l);
	if (clear) {
		app_notify_on_init(function (l: slot_layer_t) {
			slot_layer_clear(l);
		}, l);
	}
	context_raw.layer_preview_dirty = true;
	return l;
}

function layers_new_group(): slot_layer_t {
	if (project_layers.length > layers_max_layers) {
		return null;
	}
	let l: slot_layer_t = slot_layer_create("", layer_slot_type_t.GROUP);
	array_push(project_layers, l);
	context_set_layer(l);
	return l;
}

function layers_create_fill_layer(uv_type: uv_type_t = uv_type_t.UVMAP, decal_mat: mat4_t = mat4nan, position: i32 = -1) {
	///if is_forge
	return;
	///end
	_layers_uv_type = uv_type;
	_layers_decal_mat = decal_mat;
	_layers_position = position;
	app_notify_on_init(function () {
		let l: slot_layer_t = layers_new_layer(false, _layers_position);
		history_new_layer();
		l.uv_type = _layers_uv_type;
		if (!mat4_isnan(_layers_decal_mat)) {
			l.decal_mat = _layers_decal_mat;
		}
		l.object_mask = context_raw.layer_filter;
		history_to_fill_layer();
		slot_layer_to_fill_layer(l);
	});
}

function layers_create_image_mask(asset: asset_t) {
	let l: slot_layer_t = context_raw.layer;
	if (slot_layer_is_mask(l) || slot_layer_is_group(l)) {
		return;
	}

	history_new_layer();
	let m: slot_layer_t = layers_new_mask(false, l);
	slot_layer_clear(m, 0x00000000, project_get_image(asset));
	context_raw.layer_preview_dirty = true;
}

function layers_create_color_layer(base_color: i32, occlusion: f32 = 1.0, roughness: f32 = layers_default_rough, metallic: f32 = 0.0, position: i32 = -1) {
	_layers_base_color = base_color;
	_layers_occlusion = occlusion;
	_layers_roughness = roughness;
	_layers_metallic = metallic;
	_layers_position = position;

	app_notify_on_init(function () {
		let l: slot_layer_t = layers_new_layer(false, _layers_position);
		history_new_layer();
		l.uv_type = uv_type_t.UVMAP;
		l.object_mask = context_raw.layer_filter;
		slot_layer_clear(l, _layers_base_color, null, _layers_occlusion, _layers_roughness, _layers_metallic);
	});
}

function layers_duplicate_layer(l: slot_layer_t) {
	if (!slot_layer_is_group(l)) {
		let new_layer: slot_layer_t = slot_layer_duplicate(l);
		context_set_layer(new_layer);
		let masks: slot_layer_t[] = slot_layer_get_masks(l, false);
		if (masks != null) {
			for (let i: i32 = 0; i < masks.length; ++i) {
				let m: slot_layer_t = masks[i];
				m = slot_layer_duplicate(m);
				m.parent = new_layer;
				array_remove(project_layers, m);
				array_insert(project_layers, array_index_of(project_layers, new_layer), m);
			}
		}
		context_set_layer(new_layer);
	}
	else {
		let new_group: slot_layer_t = layers_new_group();
		array_remove(project_layers, new_group);
		array_insert(project_layers, array_index_of(project_layers, l) + 1, new_group);
		// group.show_panel = true;
		for (let i: i32 = 0; i < slot_layer_get_children(l).length; ++i) {
			let c: slot_layer_t = slot_layer_get_children(l)[i];
			let masks: slot_layer_t[] = slot_layer_get_masks(c, false);
			let new_layer: slot_layer_t = slot_layer_duplicate(c);
			new_layer.parent = new_group;
			array_remove(project_layers, new_layer);
			array_insert(project_layers, array_index_of(project_layers, new_group), new_layer);
			if (masks != null) {
				for (let i: i32 = 0; i < masks.length; ++i) {
					let m: slot_layer_t = masks[i];
					let new_mask: slot_layer_t = slot_layer_duplicate(m);
					new_mask.parent = new_layer;
					array_remove(project_layers, new_mask);
					array_insert(project_layers, array_index_of(project_layers, new_layer), new_mask);
				}
			}
		}
		let group_masks: slot_layer_t[] = slot_layer_get_masks(l);
		if (group_masks != null) {
			for (let i: i32 = 0; i < group_masks.length; ++i) {
				let m: slot_layer_t = group_masks[i];
				let new_mask: slot_layer_t = slot_layer_duplicate(m);
				new_mask.parent = new_group;
				array_remove(project_layers, new_mask);
				array_insert(project_layers, array_index_of(project_layers, new_group), new_mask);
			}
		}
		context_set_layer(new_group);
	}
}

function layers_apply_masks(l: slot_layer_t) {
	let masks: slot_layer_t[] = slot_layer_get_masks(l);

	if (masks != null) {
		for (let i: i32 = 0; i < masks.length - 1; ++i) {
			layers_merge_layer(masks[i + 1], masks[i]);
			slot_layer_delete(masks[i]);
		}
		slot_layer_apply_mask(masks[masks.length - 1]);
		context_raw.layer_preview_dirty = true;
	}
}

function layers_merge_down() {
	let l1: slot_layer_t = context_raw.layer;

	if (slot_layer_is_group(l1)) {
		l1 = layers_merge_group(l1);
	}
	else if (slot_layer_has_masks(l1)) { // It is a layer
		layers_apply_masks(l1);
		context_set_layer(l1);
	}

	let l0: slot_layer_t = project_layers[array_index_of(project_layers, l1) - 1];

	if (slot_layer_is_group(l0)) {
		l0 = layers_merge_group(l0);
	}
	else if (slot_layer_has_masks(l0)) { // It is a layer
		layers_apply_masks(l0);
		context_set_layer(l0);
	}

	layers_merge_layer(l0, l1);
	slot_layer_delete(l1);
	context_set_layer(l0);
	context_raw.layer_preview_dirty = true;
}

function layers_merge_group(l: slot_layer_t): slot_layer_t {
	if (!slot_layer_is_group(l)) {
		return null;
	}

	let children: slot_layer_t[] = slot_layer_get_children(l);

	if (children.length == 1 && slot_layer_has_masks(children[0], false)) {
		layers_apply_masks(children[0]);
	}

	for (let i: i32 = 0; i < children.length - 1; ++i) {
		context_set_layer(children[children.length - 1 - i]);
		history_merge_layers();
		layers_merge_down();
	}

	// Now apply the group masks
	let masks: slot_layer_t[] = slot_layer_get_masks(l);
	if (masks != null) {
		for (let i: i32 = 0; i < masks.length - 1; ++i) {
			layers_merge_layer(masks[i + 1], masks[i]);
			slot_layer_delete(masks[i]);
		}
		layers_apply_mask(children[0], masks[masks.length - 1]);
	}

	children[0].parent = null;
	children[0].name = l.name;
	if (children[0].fill_layer != null) {
		slot_layer_to_paint_layer(children[0]);
	}
	slot_layer_delete(l);
	return children[0];
}

function layers_merge_layer(l0 : slot_layer_t, l1: slot_layer_t, use_mask: bool = false) {
	if (!l1.visible || slot_layer_is_group(l1)) {
		return;
	}

	layers_make_temp_img();

	g2_begin(layers_temp_image); // Copy to temp
	g2_set_pipeline(pipes_copy);
	g2_draw_image(l0.texpaint, 0, 0);
	g2_set_pipeline(null);
	g2_end();

	let empty_rt: render_target_t = map_get(render_path_render_targets, "empty_white");
	let empty: image_t = empty_rt._image;
	let mask: image_t = empty;
	let l1masks: slot_layer_t[] =  use_mask ? slot_layer_get_masks(l1) : null;
	if (l1masks != null) {
		// for (let i: i32 = 1; i < l1masks.length - 1; ++i) {
		// 	merge_layer(l1masks[i + 1], l1masks[i]);
		// }
		mask = l1masks[0].texpaint;
	}

	if (slot_layer_is_mask(l1)) {
		g4_begin(l0.texpaint);
		g4_set_pipeline(pipes_merge_mask);
		g4_set_tex(pipes_tex0_merge_mask, l1.texpaint);
		g4_set_tex(pipes_texa_merge_mask, layers_temp_image);
		g4_set_float(pipes_opac_merge_mask, slot_layer_get_opacity(l1));
		g4_set_int(pipes_blending_merge_mask, l1.blending);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_draw();
		g4_end();
	}

	if (slot_layer_is_layer(l1)) {
		if (l1.paint_base) {
			g4_begin(l0.texpaint);
			g4_set_pipeline(pipes_merge);
			g4_set_tex(pipes_tex0, l1.texpaint);
			g4_set_tex(pipes_tex1, empty);
			g4_set_tex(pipes_texmask, mask);
			g4_set_tex(pipes_texa, layers_temp_image);
			g4_set_float(pipes_opac, slot_layer_get_opacity(l1));
			g4_set_int(pipes_blending, l1.blending);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			g4_end();
		}

		if (l0.texpaint_nor != null) {
			g2_begin(layers_temp_image);
			g2_set_pipeline(pipes_copy);
			g2_draw_image(l0.texpaint_nor, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			if (l1.paint_nor) {
				g4_begin(l0.texpaint_nor);
				g4_set_pipeline(pipes_merge);
				g4_set_tex(pipes_tex0, l1.texpaint);
				g4_set_tex(pipes_tex1, l1.texpaint_nor);
				g4_set_tex(pipes_texmask, mask);
				g4_set_tex(pipes_texa, layers_temp_image);
				g4_set_float(pipes_opac, slot_layer_get_opacity(l1));
				g4_set_int(pipes_blending, l1.paint_nor_blend ? -2 : -1);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}
		}

		if (l0.texpaint_pack != null) {
			g2_begin(layers_temp_image);
			g2_set_pipeline(pipes_copy);
			g2_draw_image(l0.texpaint_pack, 0, 0);
			g2_set_pipeline(null);
			g2_end();

			if (l1.paint_occ || l1.paint_rough || l1.paint_met || l1.paint_height) {
				if (l1.paint_occ && l1.paint_rough && l1.paint_met && l1.paint_height) {
					layers_commands_merge_pack(pipes_merge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask, l1.paint_height_blend ? -3 : -1);
				}
				else {
					if (l1.paint_occ) {
						layers_commands_merge_pack(pipes_merge_r, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
					}
					if (l1.paint_rough) {
						layers_commands_merge_pack(pipes_merge_g, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
					}
					if (l1.paint_met) {
						layers_commands_merge_pack(pipes_merge_b, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
					}
				}
			}
		}
	}
}

function layers_flatten(height_to_normal: bool = false, layers: slot_layer_t[] = null): slot_layer_t {
	return layers_ext_flatten(height_to_normal, layers);
}

function layers_on_resized() {
	layers_ext_on_resized();
}
