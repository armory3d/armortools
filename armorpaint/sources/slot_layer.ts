
type slot_layer_t = {
	id?: i32;
	name?: string;
	ext?: string;
	visible?: bool;
	parent?: slot_layer_t; // Group (for layers) or layer (for masks)
	texpaint?: image_t; // Base or mask
	////if is_paint
	texpaint_nor?: image_t;
	texpaint_pack?: image_t;
	texpaint_preview?: image_t; // Layer preview
	////end
	mask_opacity?: f32; // Opacity mask
	fill_layer?: slot_material_t;
	show_panel?: bool;
	blending?: blend_type_t;
	object_mask?: i32;
	scale?: f32;
	angle?: f32;
	uv_type?: uv_type_t;
	paint_base?: bool;
	paint_opac?: bool;
	paint_occ?: bool;
	paint_rough?: bool;
	paint_met?: bool;
	paint_nor?: bool;
	paint_nor_blend?: bool;
	paint_height?: bool;
	paint_height_blend?: bool;
	paint_emis?: bool;
	paint_subs?: bool;
	decal_mat?: mat4_t; // Decal layer
};

function slot_layer_create(ext: string = "", type: layer_slot_type_t = layer_slot_type_t.LAYER, parent: slot_layer_t = null): slot_layer_t {
	let raw: slot_layer_t = {};
	raw.id = 0;
	raw.ext = "";
	raw.visible = true;
	raw.mask_opacity = 1.0; // Opacity mask
	raw.show_panel = true;
	raw.blending = blend_type_t.MIX;
	raw.object_mask = 0;
	raw.scale = 1.0;
	raw.angle = 0.0;
	raw.uv_type = uv_type_t.UVMAP;
	raw.paint_base = true;
	raw.paint_opac = true;
	raw.paint_occ = true;
	raw.paint_rough = true;
	raw.paint_met = true;
	raw.paint_nor = true;
	raw.paint_nor_blend = true;
	raw.paint_height = true;
	raw.paint_height_blend = true;
	raw.paint_emis = true;
	raw.paint_subs = true;
	raw.decal_mat = mat4_identity(); // Decal layer

	if (ext == "") {
		raw.id = 0;
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (l.id >= raw.id) {
				raw.id = l.id + 1;
			}
		}
		ext = raw.id + "";
	}
	raw.ext = ext;
	raw.parent = parent;

	if (type == layer_slot_type_t.GROUP) {
		let id: i32 = (raw.id + 1);
		raw.name = "Group " + id;
	}
	else if (type == layer_slot_type_t.LAYER) {
		let id: i32 = (raw.id + 1);
		raw.name = "Layer " + id;
		///if is_paint
		let format: string = base_bits_handle.position == texture_bits_t.BITS8  ? "RGBA32" :
							 base_bits_handle.position == texture_bits_t.BITS16 ? "RGBA64" :
																				  "RGBA128";
		///end

		///if is_sculpt
		let format: string = "RGBA128";
		///end

		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint" + ext;
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = format;
			raw.texpaint = render_path_create_render_target(t)._image;
		}

		///if is_paint
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_nor" + ext;
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = format;
			raw.texpaint_nor = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_pack" + ext;
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = format;
			raw.texpaint_pack = render_path_create_render_target(t)._image;
		}

		raw.texpaint_preview = image_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size, tex_format_t.RGBA32);
		///end
	}

	///if is_paint
	else { // Mask
		let id: i32 = (raw.id + 1);
		raw.name = "Mask " + id;
		let format: string = "RGBA32"; // Full bits for undo support, R8 is used
		raw.blending = blend_type_t.ADD;

		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint" + ext;
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = format;
			raw.texpaint = render_path_create_render_target(t)._image;
		}

		raw.texpaint_preview = image_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size, tex_format_t.RGBA32);
	}
	///end

	return raw;
}

function slot_layer_delete(raw: slot_layer_t) {
	slot_layer_unload(raw);

	if (slot_layer_is_layer(raw)) {
		let masks: slot_layer_t[] = slot_layer_get_masks(raw, false); // Prevents deleting group masks
		if (masks != null) {
			for (let i: i32 = 0; i < masks.length; ++i) {
				let m: slot_layer_t = masks[i];
				slot_layer_delete(m);
			}
		}
	}
	else if (slot_layer_is_group(raw)) {
		let children: slot_layer_t[] = slot_layer_get_children(raw);
		if (children != null) {
			for (let i: i32 = 0; i < children.length; ++i) {
				let c: slot_layer_t = children[i];
				slot_layer_delete(c);
			}
		}
		let masks: slot_layer_t[] = slot_layer_get_masks(raw);
		if (masks != null) {
			for (let i: i32 = 0; i < masks.length; ++i) {
				let m: slot_layer_t = masks[i];
				slot_layer_delete(m);
			}
		}
	}

	let lpos: i32 = array_index_of(project_layers, raw);
	array_remove(project_layers, raw);
	// Undo can remove base layer and then restore it from undo layers
	if (project_layers.length > 0) {
		context_set_layer(project_layers[lpos > 0 ? lpos - 1 : 0]);
	}

	// Do not remove empty groups if the last layer is deleted as this prevents redo from working properly
}

function slot_layer_unload(raw: slot_layer_t) {
	if (slot_layer_is_group(raw)) {
		return;
	}

	let _texpaint: image_t = raw.texpaint;
	///if is_paint
	let _texpaint_nor: image_t = raw.texpaint_nor;
	let _texpaint_pack: image_t = raw.texpaint_pack;
	let _texpaint_preview: image_t = raw.texpaint_preview;
	///end

	app_notify_on_next_frame(function (_texpaint: image_t) {
		image_unload(_texpaint);
	}, _texpaint);

	///if is_paint
	if (_texpaint_nor != null) {
		app_notify_on_next_frame(function (_texpaint_nor: image_t) {
			image_unload(_texpaint_nor);
		}, _texpaint_nor);
	}
	if (_texpaint_pack != null) {
		app_notify_on_next_frame(function (_texpaint_pack: image_t) {
			image_unload(_texpaint_pack);
		}, _texpaint_pack);
	}
	image_unload(_texpaint_preview);
	///end

	map_delete(render_path_render_targets, "texpaint" + raw.ext);
	///if is_paint
	if (slot_layer_is_layer(raw)) {
		map_delete(render_path_render_targets, "texpaint_nor" + raw.ext);
		map_delete(render_path_render_targets, "texpaint_pack" + raw.ext);
	}
	///end
}

function slot_layer_swap(raw: slot_layer_t, other: slot_layer_t) {
	if ((slot_layer_is_layer(raw) || slot_layer_is_mask(raw)) && (slot_layer_is_layer(other) || slot_layer_is_mask(other))) {
		let rt0: render_target_t = map_get(render_path_render_targets, "texpaint" + raw.ext);
		let rt1: render_target_t = map_get(render_path_render_targets, "texpaint" + other.ext);
		rt0._image = other.texpaint;
		rt1._image = raw.texpaint;
		let _texpaint: image_t = raw.texpaint;
		raw.texpaint = other.texpaint;
		other.texpaint = _texpaint;

		///if is_paint
		let _texpaint_preview: image_t = raw.texpaint_preview;
		raw.texpaint_preview = other.texpaint_preview;
		other.texpaint_preview = _texpaint_preview;
		///end
	}

	///if is_paint
	if (slot_layer_is_layer(raw) && slot_layer_is_layer(other)) {
		let nor0: render_target_t = map_get(render_path_render_targets, "texpaint_nor" + raw.ext);
		nor0._image = other.texpaint_nor;
		let pack0: render_target_t = map_get(render_path_render_targets, "texpaint_pack" + raw.ext);
		pack0._image = other.texpaint_pack;
		let nor1: render_target_t = map_get(render_path_render_targets, "texpaint_nor" + other.ext);
		nor1._image = raw.texpaint_nor;
		let pack1: render_target_t = map_get(render_path_render_targets, "texpaint_pack" + other.ext);
		pack1._image = raw.texpaint_pack;
		let _texpaint_nor: image_t = raw.texpaint_nor;
		let _texpaint_pack: image_t = raw.texpaint_pack;
		raw.texpaint_nor = other.texpaint_nor;
		raw.texpaint_pack = other.texpaint_pack;
		other.texpaint_nor = _texpaint_nor;
		other.texpaint_pack = _texpaint_pack;
	}
	///end
}

function slot_layer_clear(raw: slot_layer_t, base_color: i32 = 0x00000000, base_image: image_t = null, occlusion: f32 = 1.0, roughness: f32 = base_default_rough, metallic: f32 = 0.0) {
	g4_begin(raw.texpaint);
	g4_clear(base_color); // Base
	g4_end();
	if (base_image != null) {
		g2_begin(raw.texpaint);
		g2_draw_scaled_image(base_image, 0, 0, raw.texpaint.width, raw.texpaint.height);
		g2_end();
	}

	///if is_paint
	if (slot_layer_is_layer(raw)) {
		g4_begin(raw.texpaint_nor);
		g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
		g4_end();
		g4_begin(raw.texpaint_pack);
		g4_clear(color_from_floats(occlusion, roughness, metallic, 0.0)); // Occ, rough, met
		g4_end();
	}
	///end

	context_raw.layer_preview_dirty = true;
	context_raw.ddirty = 3;
}

function slot_layer_invert_mask(raw: slot_layer_t) {
	if (base_pipe_invert8 == null) {
		base_make_pipe();
	}
	let inverted: image_t = image_create_render_target(raw.texpaint.width, raw.texpaint.height, tex_format_t.RGBA32);
	g2_begin(inverted);
	g2_set_pipeline(base_pipe_invert8);
	g2_draw_image(raw.texpaint, 0, 0);
	g2_set_pipeline(null);
	g2_end();
	let _texpaint: image_t = raw.texpaint;
	app_notify_on_next_frame(function (_texpaint: image_t) {
		image_unload(_texpaint);
	}, _texpaint);
	let rt: render_target_t = map_get(render_path_render_targets, "texpaint" + raw.id);
	raw.texpaint = rt._image = inverted;
	context_raw.layer_preview_dirty = true;
	context_raw.ddirty = 3;
}

function slot_layer_apply_mask(raw: slot_layer_t) {
	if (raw.parent.fill_layer != null) {
		slot_layer_to_paint_layer(raw.parent);
	}
	if (slot_layer_is_group(raw.parent)) {
		for (let i: i32 = 0; i < slot_layer_get_children(raw.parent).length; ++i) {
			let c: slot_layer_t = slot_layer_get_children(raw.parent)[i];
			base_apply_mask(c, raw);
		}
	}
	else {
		base_apply_mask(raw.parent, raw);
	}
	slot_layer_delete(raw);
}

function slot_layer_duplicate(raw: slot_layer_t): slot_layer_t {
	let layers: slot_layer_t[] = project_layers;
	let i: i32 = array_index_of(layers, raw) + 1;
	let l: slot_layer_t = slot_layer_create("", slot_layer_is_layer(raw) ? layer_slot_type_t.LAYER : slot_layer_is_mask(raw) ? layer_slot_type_t.MASK : layer_slot_type_t.GROUP, raw.parent);
	array_insert(layers, i, l);

	if (base_pipe_merge == null) {
		base_make_pipe();
	}
	if (slot_layer_is_layer(raw)) {
		g2_begin(l.texpaint);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_image(raw.texpaint, 0, 0);
		g2_set_pipeline(null);
		g2_end();
		///if is_paint
		g2_begin(l.texpaint_nor);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_image(raw.texpaint_nor, 0, 0);
		g2_set_pipeline(null);
		g2_end();
		g2_begin(l.texpaint_pack);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_image(raw.texpaint_pack, 0, 0);
		g2_set_pipeline(null);
		g2_end();
		///end
	}
	else if (slot_layer_is_mask(raw)) {
		g2_begin(l.texpaint);
		g2_set_pipeline(base_pipe_copy8);
		g2_draw_image(raw.texpaint, 0, 0);
		g2_set_pipeline(null);
		g2_end();
	}

	///if is_paint
	g2_begin(l.texpaint_preview);
	g2_clear(0x00000000);
	g2_set_pipeline(base_pipe_copy);
	g2_draw_scaled_image(raw.texpaint_preview, 0, 0, raw.texpaint_preview.width, raw.texpaint_preview.height);
	g2_set_pipeline(null);
	g2_end();
	///end

	l.visible = raw.visible;
	l.mask_opacity = raw.mask_opacity;
	l.fill_layer = raw.fill_layer;
	l.object_mask = raw.object_mask;
	l.blending = raw.blending;
	l.uv_type = raw.uv_type;
	l.scale = raw.scale;
	l.angle = raw.angle;
	l.paint_base = raw.paint_base;
	l.paint_opac = raw.paint_opac;
	l.paint_occ = raw.paint_occ;
	l.paint_rough = raw.paint_rough;
	l.paint_met = raw.paint_met;
	l.paint_nor = raw.paint_nor;
	l.paint_nor_blend = raw.paint_nor_blend;
	l.paint_height = raw.paint_height;
	l.paint_height_blend = raw.paint_height_blend;
	l.paint_emis = raw.paint_emis;
	l.paint_subs = raw.paint_subs;

	return l;
}

function slot_layer_resize_and_set_bits(raw: slot_layer_t) {
	let res_x: i32 = config_get_texture_res_x();
	let res_y: i32 = config_get_texture_res_y();
	let rts: map_t<string, render_target_t> = render_path_render_targets;
	if (base_pipe_merge == null) {
		base_make_pipe();
	}

	if (slot_layer_is_layer(raw)) {
		///if is_paint
		let format: tex_format_t = base_bits_handle.position == texture_bits_t.BITS8  ? tex_format_t.RGBA32 :
			base_bits_handle.position == texture_bits_t.BITS16 ? tex_format_t.RGBA64 :
			tex_format_t.RGBA128;
		///end

		///if is_sculpt
		let format: tex_format_t = tex_format_t.RGBA128;
		///end

		let _texpaint: image_t = raw.texpaint;
		raw.texpaint = image_create_render_target(res_x, res_y, format);
		g2_begin(raw.texpaint);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_scaled_image(_texpaint, 0, 0, res_x, res_y);
		g2_set_pipeline(null);
		g2_end();

		///if is_paint
		let _texpaint_nor: image_t = raw.texpaint_nor;
		let _texpaint_pack: image_t = raw.texpaint_pack;
		raw.texpaint_nor = image_create_render_target(res_x, res_y, format);
		raw.texpaint_pack = image_create_render_target(res_x, res_y, format);

		g2_begin(raw.texpaint_nor);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_scaled_image(_texpaint_nor, 0, 0, res_x, res_y);
		g2_set_pipeline(null);
		g2_end();

		g2_begin(raw.texpaint_pack);
		g2_set_pipeline(base_pipe_copy);
		g2_draw_scaled_image(_texpaint_pack, 0, 0, res_x, res_y);
		g2_set_pipeline(null);
		g2_end();
		///end

		app_notify_on_next_frame(function (_texpaint: image_t) {
			image_unload(_texpaint);
		}, _texpaint);

		///if is_paint
		app_notify_on_next_frame(function (_texpaint_nor: image_t) {
			image_unload(_texpaint_nor);
		}, _texpaint_nor);

		app_notify_on_next_frame(function (_texpaint_pack: image_t) {
			image_unload(_texpaint_pack);
		}, _texpaint_pack);
		///end

		let rt: render_target_t = map_get(rts, "texpaint" + raw.ext);
		rt._image = raw.texpaint;
		///if is_paint
		let rt_nor: render_target_t = map_get(rts, "texpaint_nor" + raw.ext);
		rt_nor._image = raw.texpaint_nor;
		let rt_pack: render_target_t = map_get(rts, "texpaint_pack" + raw.ext);
		rt_pack._image = raw.texpaint_pack;
		///end
	}
	else if (slot_layer_is_mask(raw)) {
		let _texpaint: image_t = raw.texpaint;
		raw.texpaint = image_create_render_target(res_x, res_y, tex_format_t.RGBA32);

		g2_begin(raw.texpaint);
		g2_set_pipeline(base_pipe_copy8);
		g2_draw_scaled_image(_texpaint, 0, 0, res_x, res_y);
		g2_set_pipeline(null);
		g2_end();

		app_notify_on_next_frame(function (_texpaint: image_t) {
			image_unload(_texpaint);
		}, _texpaint);

		let rt: render_target_t = map_get(rts, "texpaint" + raw.ext);
		rt._image = raw.texpaint;
	}
}

function slot_layer_to_fill_layer(raw: slot_layer_t) {
	context_set_layer(raw);
	raw.fill_layer = context_raw.material;
	base_update_fill_layer();
	app_notify_on_next_frame(function () {
		make_material_parse_paint_material();
		context_raw.layer_preview_dirty = true;
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	});
}

function slot_layer_to_paint_layer(raw: slot_layer_t) {
	context_set_layer(raw);
	raw.fill_layer = null;
	make_material_parse_paint_material();
	context_raw.layer_preview_dirty = true;
	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
}

function slot_layer_is_visible(raw: slot_layer_t): bool {
	return raw.visible && (raw.parent == null || raw.parent.visible);
}

function slot_layer_get_children(raw: slot_layer_t): slot_layer_t[] {
	let children: slot_layer_t[] = null; // Child layers of a group
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (l.parent == raw && slot_layer_is_layer(l)) {
			if (children == null) {
				children = [];
			}
			array_push(children, l);
		}
	}
	return children;
}

function slot_layer_get_recursive_children(raw: slot_layer_t): slot_layer_t[] {
	let children: slot_layer_t[] = null;
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (l.parent == raw) { // Child layers and group masks
			if (children == null) {
				children = [];
			}
			array_push(children, l);
		}
		if (l.parent != null && l.parent.parent == raw) { // Layer masks
			if (children == null) {
				children = [];
			}
			array_push(children, l);
		}
	}
	return children;
}

function slot_layer_get_masks(raw: slot_layer_t, include_group_masks: bool = true): slot_layer_t[] {
	if (slot_layer_is_mask(raw)) {
		return null;
	}

	let children: slot_layer_t[] = null;
	// Child masks of a layer
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (l.parent == raw && slot_layer_is_mask(l)) {
			if (children == null) {
				children = [];
			}
			array_push(children, l);
		}
	}
	// Child masks of a parent group
	if (include_group_masks) {
		if (raw.parent != null && slot_layer_is_group(raw.parent)) {
			for (let i: i32 = 0; i < project_layers.length; ++i) {
				let l: slot_layer_t = project_layers[i];
				if (l.parent == raw.parent && slot_layer_is_mask(l)) {
					if (children == null) {
						children = [];
					}
					array_push(children, l);
				}
			}
		}
	}
	return children;
}

function slot_layer_has_masks(raw: slot_layer_t, include_group_masks: bool = true): bool {
	// Layer mask
	for (let i: i32 = 0; i < project_layers.length; ++i) {
		let l: slot_layer_t = project_layers[i];
		if (l.parent == raw && slot_layer_is_mask(l)) {
			return true;
		}
	}
	// Group mask
	if (include_group_masks && raw.parent != null && slot_layer_is_group(raw.parent)) {
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (l.parent == raw.parent && slot_layer_is_mask(l)) {
				return true;
			}
		}
	}
	return false;
}

function slot_layer_get_opacity(raw: slot_layer_t): f32 {
	let f: f32 = raw.mask_opacity;
	if (slot_layer_is_layer(raw) && raw.parent != null) {
		f *= raw.parent.mask_opacity;
	}
	return f;
}

function slot_layer_get_object_mask(raw: slot_layer_t): i32 {
	return slot_layer_is_mask(raw) ? raw.parent.object_mask : raw.object_mask;
}

function slot_layer_is_layer(raw: slot_layer_t): bool {
	///if is_paint
	return raw.texpaint != null && raw.texpaint_nor != null;
	///end
	///if is_sculpt
	return raw.texpaint != null;
	///end
}

function slot_layer_is_group(raw: slot_layer_t): bool {
	return raw.texpaint == null;
}

function slot_layer_get_containing_group(raw: slot_layer_t): slot_layer_t {
	if (raw.parent != null && slot_layer_is_group(raw.parent)) {
		return raw.parent;
	}
	else if (raw.parent != null && raw.parent.parent != null && slot_layer_is_group(raw.parent.parent)) {
		return raw.parent.parent;
	}
	else {
		return null;
	}
}

function slot_layer_is_mask(raw: slot_layer_t): bool {
	///if is_paint
	return raw.texpaint != null && raw.texpaint_nor == null;
	///end
	///if is_sculpt
	return false;
	///end
}

function slot_layer_is_group_mask(raw: slot_layer_t): bool {
	///if is_paint
	return raw.texpaint != null && raw.texpaint_nor == null && slot_layer_is_group(raw.parent);
	///end
	///if is_sculpt
	return false;
	///end
}

function slot_layer_is_layer_mask(raw: slot_layer_t): bool {
	///if is_paint
	return raw.texpaint != null && raw.texpaint_nor == null && slot_layer_is_layer(raw.parent);
	///end
	///if is_sculpt
	return false;
	///end
}

function slot_layer_is_in_group(raw: slot_layer_t): bool {
	return raw.parent != null && (slot_layer_is_group(raw.parent) || (raw.parent.parent != null && slot_layer_is_group(raw.parent.parent)));
}

function slot_layer_can_move(raw: slot_layer_t, to: i32): bool {
	let old_index: i32 = array_index_of(project_layers, raw);

	let delta: i32 = to - old_index; // If delta > 0 the layer is moved up, otherwise down
	if (to < 0 || to > project_layers.length - 1 || delta == 0) {
		return false;
	}

	// If the layer is moved up, all layers between the old position and the new one move one down
	// The layers above the new position stay where they are
	// If the new position is on top or on bottom no upper resp. lower layer exists
	let new_upper_layer: slot_layer_t = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];

	// Group or layer is collapsed so we check below and update the upper layer
	if (new_upper_layer != null && !new_upper_layer.show_panel) {
		let children: slot_layer_t[] = slot_layer_get_recursive_children(new_upper_layer);
		to -= children != null ? children.length : 0;
		delta = to - old_index;
		new_upper_layer = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];
	}

	let new_lower_layer: slot_layer_t = delta > 0 ? project_layers[to] : (to > 0 ? project_layers[to - 1] : null);

	if (slot_layer_is_mask(raw)) {
		// Masks can not be on top
		if (new_upper_layer == null) {
			return false;
		}
		// Masks should not be placed below a collapsed group - this condition can be savely removed
		if (slot_layer_is_in_group(new_upper_layer) && !slot_layer_get_containing_group(new_upper_layer).show_panel) {
			return false;
		}
		// Masks should not be placed below a collapsed layer - this condition can be savely removed
		if (slot_layer_is_mask(new_upper_layer) && !new_upper_layer.parent.show_panel) {
			return false;
		}
	}

	if (slot_layer_is_layer(raw)) {
		// Layers can not be moved directly below its own mask(s)
		if (new_upper_layer != null && slot_layer_is_mask(new_upper_layer) && new_upper_layer.parent == raw) {
			return false;
		}
		// Layers can not be placed above a mask as the mask would be reparented
		if (new_lower_layer != null && slot_layer_is_mask(new_lower_layer)) {
			return false;
		}
	}

	// Currently groups can not be nested - thus valid positions for groups are:
	if (slot_layer_is_group(raw)) {
		// At the top
		if (new_upper_layer == null) {
			return true;
		}
		// NOT below its own children
		if (slot_layer_get_containing_group(new_upper_layer) == raw) {
			return false;
		}
		// At the bottom
		if (new_lower_layer == null) {
			return true;
		}
		// Above a group
		if (slot_layer_is_group(new_lower_layer)) {
			return true;
		}
		// Above a non-grouped layer
		if (slot_layer_is_layer(new_lower_layer) && !slot_layer_is_in_group(new_lower_layer)) {
			return true;
		}
		else {
			return false;
		}
	}

	return true;
}

function slot_layer_move(raw: slot_layer_t, to: i32) {
	if (!slot_layer_can_move(raw, to)) {
		return;
	}

	let pointers: map_t<slot_layer_t, i32> = tab_layers_init_layer_map();
	let old_index: i32 = array_index_of(project_layers, raw);
	let delta: i32 = to - old_index;
	let new_upper_layer: slot_layer_t = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];

	// Group or layer is collapsed so we check below and update the upper layer
	if (new_upper_layer != null && !new_upper_layer.show_panel) {
		let children: slot_layer_t[] = slot_layer_get_recursive_children(new_upper_layer);
		to -= children != null ? children.length : 0;
		delta = to - old_index;
		new_upper_layer = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];
	}

	context_set_layer(raw);
	history_order_layers(to);
	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;

	array_remove(project_layers, raw);
	array_insert(project_layers, to, raw);

	if (slot_layer_is_layer(raw)) {
		let old_parent: slot_layer_t = raw.parent;

		if (new_upper_layer == null) {
			raw.parent = null; // Placed on top
		}
		else if (slot_layer_is_in_group(new_upper_layer) && !slot_layer_get_containing_group(new_upper_layer).show_panel) {
			raw.parent = null; // Placed below a collapsed group
		}
		else if (slot_layer_is_layer(new_upper_layer)) {
			raw.parent = new_upper_layer.parent; // Placed below a layer, use the same parent
		}
		else if (slot_layer_is_group(new_upper_layer)) {
			raw.parent = new_upper_layer; // Placed as top layer in a group
		}
		else if (slot_layer_is_group_mask(new_upper_layer)) {
			raw.parent = new_upper_layer.parent; // Placed in a group below the lowest group mask
		}
		else if (slot_layer_is_layer_mask(new_upper_layer)) {
			raw.parent = slot_layer_get_containing_group(new_upper_layer); // Either the group the mask belongs to or null
		}

		// Layers can have masks as children
		// These have to be moved, too
		let layer_masks: slot_layer_t[] = slot_layer_get_masks(raw, false);
		if (layer_masks != null) {
			for (let idx: i32 = 0; idx < layer_masks.length; ++idx) {
				let mask: slot_layer_t = layer_masks[idx];
				array_remove(project_layers, mask);
				// If the masks are moved down each step increases the index below the layer by one.
				array_insert(project_layers, delta > 0 ? old_index + delta - 1 : old_index + delta + idx, mask);
			}
		}

		// The layer is the last layer in the group, remove it
		// Notice that this might remove group masks
		if (old_parent != null && slot_layer_get_children(old_parent) == null) {
			slot_layer_delete(old_parent);
		}
	}
	else if (slot_layer_is_mask(raw)) {
		// Precondition new_upper_layer != null, ensured in can_move
		if (slot_layer_is_layer(new_upper_layer) || slot_layer_is_group(new_upper_layer)) {
			raw.parent = new_upper_layer;
		}
		else if (slot_layer_is_mask(new_upper_layer)) { // Group mask or layer mask
			raw.parent = new_upper_layer.parent;
		}
	}
	else if (slot_layer_is_group(raw)) {
		let children: slot_layer_t[] = slot_layer_get_recursive_children(raw);
		if (children != null) {
			for (let idx: i32 = 0; idx < children.length; ++idx) {
				let child: slot_layer_t = children[idx];
				array_remove(project_layers, child);
				// If the children are moved down each step increases the index below the layer by one
				array_insert(project_layers, delta > 0 ? old_index + delta - 1 : old_index + delta + idx, child);
			}
		}
	}

	for (let i: i32 = 0; i < project_materials.length; ++i) {
		let m: slot_material_t = project_materials[i];
		tab_layers_remap_layer_pointers(m.canvas.nodes, tab_layers_fill_layer_map(pointers));
	}
}
