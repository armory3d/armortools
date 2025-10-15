
type slot_layer_t = {
	id?: i32;
	name?: string;
	ext?: string;
	visible?: bool;
	parent?: slot_layer_t;    // Group (for layers) or layer (for masks)
	texpaint?: gpu_texture_t; // Base or mask
	texpaint_nor?: gpu_texture_t;
	texpaint_pack?: gpu_texture_t;
	texpaint_preview?: gpu_texture_t; // Layer preview
	mask_opacity?: f32;               // Opacity mask
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
	texpaint_sculpt?: gpu_texture_t;
};

function slot_layer_create(ext: string = "", type: layer_slot_type_t = layer_slot_type_t.LAYER, parent: slot_layer_t = null): slot_layer_t {
	let raw: slot_layer_t  = {};
	raw.id                 = 0;
	raw.ext                = "";
	raw.visible            = true;
	raw.mask_opacity       = 1.0; // Opacity mask
	raw.show_panel         = true;
	raw.blending           = blend_type_t.MIX;
	raw.object_mask        = 0;
	raw.scale              = 1.0;
	raw.angle              = 0.0;
	raw.uv_type            = uv_type_t.UVMAP;
	raw.paint_base         = true;
	raw.paint_opac         = true;
	raw.paint_occ          = true;
	raw.paint_rough        = true;
	raw.paint_met          = true;
	raw.paint_nor          = true;
	raw.paint_nor_blend    = true;
	raw.paint_height       = true;
	raw.paint_height_blend = true;
	raw.paint_emis         = true;
	raw.paint_subs         = true;
	raw.decal_mat          = mat4_identity(); // Decal layer

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
	raw.ext    = ext;
	raw.parent = parent;

	if (type == layer_slot_type_t.GROUP) {
		let id: i32 = (raw.id + 1);
		raw.name    = "Group " + id;
	}
	else if (type == layer_slot_type_t.LAYER) {
		let id: i32        = (raw.id + 1);
		raw.name           = "Layer " + id;
		let format: string = base_bits_handle.i == texture_bits_t.BITS8 ? "RGBA32" : base_bits_handle.i == texture_bits_t.BITS16 ? "RGBA64" : "RGBA128";

		{
			let t: render_target_t = render_target_create();
			t.name                 = "texpaint" + ext;
			t.width                = config_get_texture_res_x();
			t.height               = config_get_texture_res_y();
			t.format               = format;
			raw.texpaint           = render_path_create_render_target(t)._image;
		}

		{
			let t: render_target_t = render_target_create();
			t.name                 = "texpaint_nor" + ext;
			t.width                = config_get_texture_res_x();
			t.height               = config_get_texture_res_y();
			t.format               = format;
			raw.texpaint_nor       = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name                 = "texpaint_pack" + ext;
			t.width                = config_get_texture_res_x();
			t.height               = config_get_texture_res_y();
			t.format               = format;
			raw.texpaint_pack      = render_path_create_render_target(t)._image;
		}

		raw.texpaint_preview = gpu_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size, tex_format_t.RGBA32);
	}

	else { // Mask
		let id: i32        = (raw.id + 1);
		raw.name           = "Mask " + id;
		let format: string = "RGBA32"; // Full bits for undo support, R8 is used
		raw.blending       = blend_type_t.ADD;

		{
			let t: render_target_t = render_target_create();
			t.name                 = "texpaint" + ext;
			t.width                = config_get_texture_res_x();
			t.height               = config_get_texture_res_y();
			t.format               = format;
			raw.texpaint           = render_path_create_render_target(t)._image;
		}

		raw.texpaint_preview = gpu_create_render_target(util_render_layer_preview_size, util_render_layer_preview_size, tex_format_t.RGBA32);
	}

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

	let _texpaint: gpu_texture_t         = raw.texpaint;
	let _texpaint_nor: gpu_texture_t     = raw.texpaint_nor;
	let _texpaint_pack: gpu_texture_t    = raw.texpaint_pack;
	let _texpaint_preview: gpu_texture_t = raw.texpaint_preview;

	gpu_delete_texture(_texpaint);
	if (_texpaint_nor != null) {
		gpu_delete_texture(_texpaint_nor);
	}
	if (_texpaint_pack != null) {
		gpu_delete_texture(_texpaint_pack);
	}
	if (_texpaint_preview != null) {
		gpu_delete_texture(_texpaint_preview);
	}

	map_delete(render_path_render_targets, "texpaint" + raw.ext);
	if (slot_layer_is_layer(raw)) {
		map_delete(render_path_render_targets, "texpaint_nor" + raw.ext);
		map_delete(render_path_render_targets, "texpaint_pack" + raw.ext);
	}
}

function slot_layer_swap(raw: slot_layer_t, other: slot_layer_t) {
	if ((slot_layer_is_layer(raw) || slot_layer_is_mask(raw)) && (slot_layer_is_layer(other) || slot_layer_is_mask(other))) {
		let rt0: render_target_t     = map_get(render_path_render_targets, "texpaint" + raw.ext);
		let rt1: render_target_t     = map_get(render_path_render_targets, "texpaint" + other.ext);
		rt0._image                   = other.texpaint;
		rt1._image                   = raw.texpaint;
		let _texpaint: gpu_texture_t = raw.texpaint;
		raw.texpaint                 = other.texpaint;
		other.texpaint               = _texpaint;

		let _texpaint_preview: gpu_texture_t = raw.texpaint_preview;
		raw.texpaint_preview                 = other.texpaint_preview;
		other.texpaint_preview               = _texpaint_preview;
	}

	if (slot_layer_is_layer(raw) && slot_layer_is_layer(other)) {
		let nor0: render_target_t         = map_get(render_path_render_targets, "texpaint_nor" + raw.ext);
		nor0._image                       = other.texpaint_nor;
		let pack0: render_target_t        = map_get(render_path_render_targets, "texpaint_pack" + raw.ext);
		pack0._image                      = other.texpaint_pack;
		let nor1: render_target_t         = map_get(render_path_render_targets, "texpaint_nor" + other.ext);
		nor1._image                       = raw.texpaint_nor;
		let pack1: render_target_t        = map_get(render_path_render_targets, "texpaint_pack" + other.ext);
		pack1._image                      = raw.texpaint_pack;
		let _texpaint_nor: gpu_texture_t  = raw.texpaint_nor;
		let _texpaint_pack: gpu_texture_t = raw.texpaint_pack;
		raw.texpaint_nor                  = other.texpaint_nor;
		raw.texpaint_pack                 = other.texpaint_pack;
		other.texpaint_nor                = _texpaint_nor;
		other.texpaint_pack               = _texpaint_pack;
	}
}

function slot_layer_clear(raw: slot_layer_t, base_color: i32 = 0x00000000, base_image: gpu_texture_t = null, occlusion: f32 = 1.0,
                          roughness: f32 = layers_default_rough, metallic: f32 = 0.0) {
	// Base
	_gpu_begin(raw.texpaint, null, null, clear_flag_t.COLOR, base_color);
	gpu_end();
	if (base_image != null) {
		draw_begin(raw.texpaint);
		draw_scaled_image(base_image, 0, 0, raw.texpaint.width, raw.texpaint.height);
		draw_end();
	}

	if (slot_layer_is_layer(raw)) {
		// Nor
		_gpu_begin(raw.texpaint_nor, null, null, clear_flag_t.COLOR, color_from_floats(0.5, 0.5, 1.0, 0.0));
		gpu_end();
		// Occ, rough, met
		_gpu_begin(raw.texpaint_pack, null, null, clear_flag_t.COLOR, color_from_floats(occlusion, roughness, metallic, 0.0));
		gpu_end();
	}

	context_raw.layer_preview_dirty = true;
	context_raw.ddirty              = 3;
}

function slot_layer_invert_mask(raw: slot_layer_t) {
	let inverted: gpu_texture_t = gpu_create_render_target(raw.texpaint.width, raw.texpaint.height, tex_format_t.RGBA32);
	draw_begin(inverted);
	draw_set_pipeline(pipes_invert8);
	draw_image(raw.texpaint, 0, 0);
	draw_set_pipeline(null);
	draw_end();
	let _texpaint: gpu_texture_t = raw.texpaint;
	gpu_delete_texture(_texpaint);
	let rt: render_target_t = map_get(render_path_render_targets, "texpaint" + raw.id);
	raw.texpaint = rt._image        = inverted;
	context_raw.layer_preview_dirty = true;
	context_raw.ddirty              = 3;
}

function slot_layer_apply_mask(raw: slot_layer_t) {
	if (raw.parent.fill_layer != null) {
		slot_layer_to_paint_layer(raw.parent);
	}
	if (slot_layer_is_group(raw.parent)) {
		for (let i: i32 = 0; i < slot_layer_get_children(raw.parent).length; ++i) {
			let c: slot_layer_t = slot_layer_get_children(raw.parent)[i];
			layers_apply_mask(c, raw);
		}
	}
	else {
		layers_apply_mask(raw.parent, raw);
	}
	slot_layer_delete(raw);
}

function slot_layer_duplicate(raw: slot_layer_t): slot_layer_t {
	let layers: slot_layer_t[] = project_layers;
	let i: i32                 = array_index_of(layers, raw) + 1;
	let l: slot_layer_t        = slot_layer_create("",
                                            slot_layer_is_layer(raw)  ? layer_slot_type_t.LAYER
	                                               : slot_layer_is_mask(raw) ? layer_slot_type_t.MASK
	                                                                         : layer_slot_type_t.GROUP,
	                                               raw.parent);
	array_insert(layers, i, l);

	if (slot_layer_is_layer(raw)) {
		draw_begin(l.texpaint);
		draw_set_pipeline(pipes_copy);
		draw_image(raw.texpaint, 0, 0);
		draw_set_pipeline(null);
		draw_end();

		if (l.texpaint_nor != null) {
			draw_begin(l.texpaint_nor);
			draw_set_pipeline(pipes_copy);
			draw_image(raw.texpaint_nor, 0, 0);
			draw_set_pipeline(null);
			draw_end();
		}

		if (l.texpaint_pack != null) {
			draw_begin(l.texpaint_pack);
			draw_set_pipeline(pipes_copy);
			draw_image(raw.texpaint_pack, 0, 0);
			draw_set_pipeline(null);
			draw_end();
		}
	}
	else if (slot_layer_is_mask(raw)) {
		draw_begin(l.texpaint);
		draw_set_pipeline(pipes_copy8);
		draw_image(raw.texpaint, 0, 0);
		draw_set_pipeline(null);
		draw_end();
	}

	if (l.texpaint_preview != null) {
		draw_begin(l.texpaint_preview, true, 0x00000000);
		draw_set_pipeline(pipes_copy);
		draw_scaled_image(raw.texpaint_preview, 0, 0, raw.texpaint_preview.width, raw.texpaint_preview.height);
		draw_set_pipeline(null);
		draw_end();
	}

	l.visible            = raw.visible;
	l.mask_opacity       = raw.mask_opacity;
	l.fill_layer         = raw.fill_layer;
	l.object_mask        = raw.object_mask;
	l.blending           = raw.blending;
	l.uv_type            = raw.uv_type;
	l.scale              = raw.scale;
	l.angle              = raw.angle;
	l.paint_base         = raw.paint_base;
	l.paint_opac         = raw.paint_opac;
	l.paint_occ          = raw.paint_occ;
	l.paint_rough        = raw.paint_rough;
	l.paint_met          = raw.paint_met;
	l.paint_nor          = raw.paint_nor;
	l.paint_nor_blend    = raw.paint_nor_blend;
	l.paint_height       = raw.paint_height;
	l.paint_height_blend = raw.paint_height_blend;
	l.paint_emis         = raw.paint_emis;
	l.paint_subs         = raw.paint_subs;

	return l;
}

function slot_layer_resize_and_set_bits(raw: slot_layer_t) {
	let res_x: i32                          = config_get_texture_res_x();
	let res_y: i32                          = config_get_texture_res_y();
	let rts: map_t<string, render_target_t> = render_path_render_targets;

	if (slot_layer_is_layer(raw)) {
		let format: tex_format_t = base_bits_handle.i == texture_bits_t.BITS8    ? tex_format_t.RGBA32
		                           : base_bits_handle.i == texture_bits_t.BITS16 ? tex_format_t.RGBA64
		                                                                         : tex_format_t.RGBA128;

		let pipe: gpu_pipeline_t = format == tex_format_t.RGBA32 ? pipes_copy : format == tex_format_t.RGBA64 ? pipes_copy64 : pipes_copy128;

		let _texpaint: gpu_texture_t = raw.texpaint;
		raw.texpaint                 = gpu_create_render_target(res_x, res_y, format);
		draw_begin(raw.texpaint);
		draw_set_pipeline(pipe);
		draw_scaled_image(_texpaint, 0, 0, res_x, res_y);
		draw_set_pipeline(null);
		draw_end();

		let _texpaint_nor: gpu_texture_t = raw.texpaint_nor;
		if (raw.texpaint_nor != null) {
			raw.texpaint_nor = gpu_create_render_target(res_x, res_y, format);

			draw_begin(raw.texpaint_nor);
			draw_set_pipeline(pipe);
			draw_scaled_image(_texpaint_nor, 0, 0, res_x, res_y);
			draw_set_pipeline(null);
			draw_end();
		}

		let _texpaint_pack: gpu_texture_t = raw.texpaint_pack;
		if (raw.texpaint_pack != null) {
			raw.texpaint_pack = gpu_create_render_target(res_x, res_y, format);

			draw_begin(raw.texpaint_pack);
			draw_set_pipeline(pipe);
			draw_scaled_image(_texpaint_pack, 0, 0, res_x, res_y);
			draw_set_pipeline(null);
			draw_end();
		}

		gpu_delete_texture(_texpaint);
		if (_texpaint_nor != null) {
			gpu_delete_texture(_texpaint_nor);
		}
		if (_texpaint_pack != null) {
			gpu_delete_texture(_texpaint_pack);
		}

		let rt: render_target_t = map_get(rts, "texpaint" + raw.ext);
		rt._image               = raw.texpaint;

		if (raw.texpaint_nor != null) {
			let rt_nor: render_target_t = map_get(rts, "texpaint_nor" + raw.ext);
			rt_nor._image               = raw.texpaint_nor;
		}

		if (raw.texpaint_pack != null) {
			let rt_pack: render_target_t = map_get(rts, "texpaint_pack" + raw.ext);
			rt_pack._image               = raw.texpaint_pack;
		}
	}
	else if (slot_layer_is_mask(raw)) {
		let _texpaint: gpu_texture_t = raw.texpaint;
		raw.texpaint                 = gpu_create_render_target(res_x, res_y, tex_format_t.RGBA32);

		draw_begin(raw.texpaint);
		draw_set_pipeline(pipes_copy8);
		draw_scaled_image(_texpaint, 0, 0, res_x, res_y);
		draw_set_pipeline(null);
		draw_end();

		gpu_delete_texture(_texpaint);

		let rt: render_target_t = map_get(rts, "texpaint" + raw.ext);
		rt._image               = raw.texpaint;
	}
}

function slot_layer_to_fill_layer(raw: slot_layer_t) {
	context_set_layer(raw);
	raw.fill_layer = context_raw.material;
	layers_update_fill_layer();
	sys_notify_on_next_frame(function() {
		make_material_parse_paint_material();
		context_raw.layer_preview_dirty            = true;
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	});
}

function slot_layer_to_paint_layer(raw: slot_layer_t) {
	context_set_layer(raw);
	raw.fill_layer = null;
	make_material_parse_paint_material();
	context_raw.layer_preview_dirty            = true;
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
	return raw.texpaint != null && raw.texpaint_nor != null;
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
	return raw.texpaint != null && raw.texpaint_nor == null;
}

function slot_layer_is_group_mask(raw: slot_layer_t): bool {
	return raw.texpaint != null && raw.texpaint_nor == null && slot_layer_is_group(raw.parent);
}

function slot_layer_is_layer_mask(raw: slot_layer_t): bool {
	return raw.texpaint != null && raw.texpaint_nor == null && slot_layer_is_layer(raw.parent);
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
		delta           = to - old_index;
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
	let old_index: i32                     = array_index_of(project_layers, raw);
	let delta: i32                         = to - old_index;
	let new_upper_layer: slot_layer_t      = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];

	// Group or layer is collapsed so we check below and update the upper layer
	if (new_upper_layer != null && !new_upper_layer.show_panel) {
		let children: slot_layer_t[] = slot_layer_get_recursive_children(new_upper_layer);
		to -= children != null ? children.length : 0;
		delta           = to - old_index;
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

let layers_temp_image: gpu_texture_t = null;
let layers_expa: gpu_texture_t       = null;
let layers_expb: gpu_texture_t       = null;
let layers_expc: gpu_texture_t       = null;
let layers_default_base: f32         = 0.5;
let layers_default_rough: f32        = 0.4;
let layers_max_layers: i32 =
    /// if (arm_android || arm_ios)
    18;
/// else
255;
/// end

let _layers_uv_type: uv_type_t;
let _layers_decal_mat: mat4_t;
let _layers_position: i32;
let _layers_base_color: i32;
let _layers_occlusion: f32;
let _layers_roughness: f32;
let _layers_metallic: f32;

function layers_init() {
	slot_layer_clear(project_layers[0], color_from_floats(layers_default_base, layers_default_base, layers_default_base, 1.0));
}

function layers_resize() {
	let conf: config_t = config_raw;
	if (base_res_handle.i >= math_floor(texture_res_t.RES16384)) { // Save memory for >=16k
		conf.undo_steps = 1;
		if (context_raw.undo_handle != null) {
			context_raw.undo_handle.f = conf.undo_steps;
		}
		while (history_undo_layers.length > conf.undo_steps) {
			let l: slot_layer_t = array_pop(history_undo_layers);
			sys_notify_on_next_frame(function(l: slot_layer_t) {
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

	let blend0: render_target_t         = map_get(rts, "texpaint_blend0");
	let _texpaint_blend0: gpu_texture_t = blend0._image;
	gpu_delete_texture(_texpaint_blend0);
	blend0.width  = config_get_texture_res_x();
	blend0.height = config_get_texture_res_y();
	blend0._image = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);

	let blend1: render_target_t         = map_get(rts, "texpaint_blend1");
	let _texpaint_blend1: gpu_texture_t = blend1._image;
	gpu_delete_texture(_texpaint_blend1);
	blend1.width  = config_get_texture_res_x();
	blend1.height = config_get_texture_res_y();
	blend1._image = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);

	context_raw.brush_blend_dirty = true;

	let blur: render_target_t = map_get(rts, "texpaint_blur");
	if (blur != null) {
		let _texpaint_blur: gpu_texture_t = blur._image;
		gpu_delete_texture(_texpaint_blur);
		let size_x: f32 = math_floor(config_get_texture_res_x() * 0.95);
		let size_y: f32 = math_floor(config_get_texture_res_y() * 0.95);
		blur.width      = size_x;
		blur.height     = size_y;
		blur._image     = gpu_create_render_target(size_x, size_y);
	}
	if (render_path_paint_live_layer != null) {
		slot_layer_resize_and_set_bits(render_path_paint_live_layer);
	}
	render_path_raytrace_ready = false; // Rebuild baketex
	context_raw.ddirty         = 2;
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
	let l: slot_layer_t = project_layers[0];

	if (layers_temp_image != null &&
	    (layers_temp_image.width != l.texpaint.width || layers_temp_image.height != l.texpaint.height || layers_temp_image.format != l.texpaint.format)) {
		let _temptex0: render_target_t = map_get(render_path_render_targets, "temptex0");
		render_target_unload(_temptex0);
		map_delete(render_path_render_targets, "temptex0");
		layers_temp_image = null;
	}
	if (layers_temp_image == null) {
		let format: string = base_bits_handle.i == texture_bits_t.BITS8 ? "RGBA32" : base_bits_handle.i == texture_bits_t.BITS16 ? "RGBA64" : "RGBA128";

		let t: render_target_t  = render_target_create();
		t.name                  = "temptex0";
		t.width                 = l.texpaint.width;
		t.height                = l.texpaint.height;
		t.format                = format;
		let rt: render_target_t = render_path_create_render_target(t);
		layers_temp_image       = rt._image;
	}
}

function layers_make_temp_mask_img() {
	if (pipes_temp_mask_image != null &&
	    (pipes_temp_mask_image.width != config_get_texture_res_x() || pipes_temp_mask_image.height != config_get_texture_res_y())) {
		let _temp_mask_image: gpu_texture_t = pipes_temp_mask_image;
		gpu_delete_texture(_temp_mask_image);
		pipes_temp_mask_image = null;
	}
	if (pipes_temp_mask_image == null) {
		pipes_temp_mask_image = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);
	}
}

function layers_make_export_img() {
	let l: slot_layer_t = project_layers[0];

	if (layers_expa != null && (layers_expa.width != l.texpaint.width || layers_expa.height != l.texpaint.height || layers_expa.format != l.texpaint.format)) {
		let _expa: gpu_texture_t = layers_expa;
		let _expb: gpu_texture_t = layers_expb;
		let _expc: gpu_texture_t = layers_expc;
		gpu_delete_texture(_expa);
		gpu_delete_texture(_expb);
		gpu_delete_texture(_expc);
		layers_expa = null;
		layers_expb = null;
		layers_expc = null;
		map_delete(render_path_render_targets, "expa");
		map_delete(render_path_render_targets, "expb");
		map_delete(render_path_render_targets, "expc");
	}
	if (layers_expa == null) {
		let format: string = base_bits_handle.i == texture_bits_t.BITS8 ? "RGBA32" : base_bits_handle.i == texture_bits_t.BITS16 ? "RGBA64" : "RGBA128";

		{
			let t: render_target_t  = render_target_create();
			t.name                  = "expa";
			t.width                 = l.texpaint.width;
			t.height                = l.texpaint.height;
			t.format                = format;
			let rt: render_target_t = render_path_create_render_target(t);
			layers_expa             = rt._image;
		}

		{
			let t: render_target_t  = render_target_create();
			t.name                  = "expb";
			t.width                 = l.texpaint.width;
			t.height                = l.texpaint.height;
			t.format                = format;
			let rt: render_target_t = render_path_create_render_target(t);
			layers_expb             = rt._image;
		}

		{
			let t: render_target_t  = render_target_create();
			t.name                  = "expc";
			t.width                 = l.texpaint.width;
			t.height                = l.texpaint.height;
			t.format                = format;
			let rt: render_target_t = render_path_create_render_target(t);
			layers_expc             = rt._image;
		}
	}
}

function layers_apply_mask(l: slot_layer_t, m: slot_layer_t) {
	if (!slot_layer_is_layer(l) || !slot_layer_is_mask(m)) {
		return;
	}

	layers_make_temp_img();

	// Copy layer to temp
	draw_begin(layers_temp_image);
	draw_set_pipeline(pipes_copy);
	draw_image(l.texpaint, 0, 0);
	draw_set_pipeline(null);
	draw_end();

	// Apply mask
	_gpu_begin(l.texpaint);
	gpu_set_pipeline(pipes_apply_mask);
	gpu_set_texture(pipes_tex0_mask, layers_temp_image);
	gpu_set_texture(pipes_texa_mask, m.texpaint);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();
	gpu_end();
}

function layers_commands_merge_pack(pipe: gpu_pipeline_t, i0: gpu_texture_t, i1: gpu_texture_t, i1pack: gpu_texture_t, i1mask_opacity: f32,
                                    i1texmask: gpu_texture_t, i1blending: i32 = 101) {
	_gpu_begin(i0);
	gpu_set_pipeline(pipe);
	gpu_set_texture(pipes_tex0, i1);
	gpu_set_texture(pipes_tex1, i1pack);
	gpu_set_texture(pipes_texmask, i1texmask);
	gpu_set_texture(pipes_texa, layers_temp_image);
	gpu_set_float(pipes_opac, i1mask_opacity);
	gpu_set_float(pipes_tex1w, i1pack.width);
	gpu_set_int(pipes_blending, i1blending);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_draw();
	gpu_end();
}

function layers_is_fill_material(): bool {
	if (context_raw.tool == tool_type_t.MATERIAL) {
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
	let _layer: slot_layer_t   = context_raw.layer;
	let _tool: tool_type_t     = context_raw.tool;
	let _fill_type: i32        = context_raw.fill_type_handle.i;
	let current: gpu_texture_t = null;

	if (context_raw.tool == tool_type_t.MATERIAL) {
		if (render_path_paint_live_layer == null) {
			render_path_paint_live_layer = slot_layer_create("_live");
		}

		current = _draw_current;
		if (current != null)
			draw_end();

		context_raw.tool               = tool_type_t.FILL;
		context_raw.fill_type_handle.i = fill_type_t.OBJECT;
		render_path_paint_set_plane_mesh();
		make_material_parse_paint_material(false);
		context_raw.pdirty = 1;
		render_path_paint_use_live_layer(true);
		render_path_paint_commands_paint(false);
		render_path_paint_dilate(true, true);
		render_path_paint_use_live_layer(false);
		context_raw.tool               = _tool;
		context_raw.fill_type_handle.i = _fill_type;
		context_raw.pdirty             = 0;
		context_raw.rdirty             = 2;
		render_path_paint_restore_plane_mesh();
		make_material_parse_paint_material();
		ui_view2d_hwnd.redraws = 2;

		if (current != null)
			draw_begin(current);
		return;
	}

	let has_fill_layer: bool = false;
	let has_fill_mask: bool  = false;
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
		current = _draw_current;
		if (current != null) {
			draw_end();
		}
		context_raw.pdirty             = 1;
		context_raw.tool               = tool_type_t.FILL;
		context_raw.fill_type_handle.i = fill_type_t.OBJECT;

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

		context_raw.pdirty               = 0;
		context_raw.ddirty               = 2;
		context_raw.rdirty               = 2;
		context_raw.layers_preview_dirty = true; // Repaint all layer previews as multiple layers might have changed.
		if (current != null)
			draw_begin(current);
		context_raw.layer = _layer;
		layers_set_object_mask();
		context_raw.tool               = _tool;
		context_raw.fill_type_handle.i = _fill_type;
		make_material_parse_paint_material(false);
	}
}

function layers_update_fill_layer(parse_paint: bool = true) {
	let current: gpu_texture_t = _draw_current;
	let in_use: bool           = gpu_in_use;
	if (in_use)
		draw_end();

	let _tool: tool_type_t         = context_raw.tool;
	let _fill_type: i32            = context_raw.fill_type_handle.i;
	context_raw.tool               = tool_type_t.FILL;
	context_raw.fill_type_handle.i = fill_type_t.OBJECT;
	context_raw.pdirty             = 1;

	slot_layer_clear(context_raw.layer);

	if (parse_paint) {
		make_material_parse_paint_material(false);
	}
	render_path_paint_commands_paint(false);
	render_path_paint_dilate(true, true);

	context_raw.rdirty             = 2;
	context_raw.tool               = _tool;
	context_raw.fill_type_handle.i = _fill_type;
	if (in_use)
		draw_begin(current);
}

function layers_set_object_mask() {
	let ar: string[] = [ tr("None") ];
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
			let p: mesh_object_t  = project_paint_objects[i];
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
		context_raw.paint_object.skip_context  = "paint";
		context_raw.merged_object.base.visible = true;
	}
	util_uv_dilatemap_cached = false;
}

function layers_new_layer(clear: bool = true, position: i32 = -1): slot_layer_t {
	if (project_layers.length > layers_max_layers) {
		return null;
	}

	let l: slot_layer_t = slot_layer_create();
	l.object_mask       = context_raw.layer_filter;

	if (position == -1) {
		if (slot_layer_is_mask(context_raw.layer))
			context_set_layer(context_raw.layer.parent);
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
		sys_notify_on_next_frame(function(l: slot_layer_t) {
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
		sys_notify_on_next_frame(function(l: slot_layer_t) {
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
	if (context_raw.tool == tool_type_t.GIZMO) {
		return;
	}

	_layers_uv_type   = uv_type;
	_layers_decal_mat = decal_mat;
	_layers_position  = position;
	sys_notify_on_next_frame(function() {
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
	_layers_occlusion  = occlusion;
	_layers_roughness  = roughness;
	_layers_metallic   = metallic;
	_layers_position   = position;

	sys_notify_on_next_frame(function() {
		let l: slot_layer_t = layers_new_layer(false, _layers_position);
		history_new_layer();
		l.uv_type     = uv_type_t.UVMAP;
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
				m                   = slot_layer_duplicate(m);
				m.parent            = new_layer;
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
			let c: slot_layer_t         = slot_layer_get_children(l)[i];
			let masks: slot_layer_t[]   = slot_layer_get_masks(c, false);
			let new_layer: slot_layer_t = slot_layer_duplicate(c);
			new_layer.parent            = new_group;
			array_remove(project_layers, new_layer);
			array_insert(project_layers, array_index_of(project_layers, new_group), new_layer);
			if (masks != null) {
				for (let i: i32 = 0; i < masks.length; ++i) {
					let m: slot_layer_t        = masks[i];
					let new_mask: slot_layer_t = slot_layer_duplicate(m);
					new_mask.parent            = new_layer;
					array_remove(project_layers, new_mask);
					array_insert(project_layers, array_index_of(project_layers, new_layer), new_mask);
				}
			}
		}
		let group_masks: slot_layer_t[] = slot_layer_get_masks(l);
		if (group_masks != null) {
			for (let i: i32 = 0; i < group_masks.length; ++i) {
				let m: slot_layer_t        = group_masks[i];
				let new_mask: slot_layer_t = slot_layer_duplicate(m);
				new_mask.parent            = new_group;
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
	children[0].name   = l.name;
	if (children[0].fill_layer != null) {
		slot_layer_to_paint_layer(children[0]);
	}
	slot_layer_delete(l);
	return children[0];
}

function layers_merge_layer(l0: slot_layer_t, l1: slot_layer_t, use_mask: bool = false) {
	if (!l1.visible || slot_layer_is_group(l1)) {
		return;
	}

	layers_make_temp_img();

	draw_begin(layers_temp_image); // Copy to temp
	draw_set_pipeline(pipes_copy);
	draw_image(l0.texpaint, 0, 0);
	draw_set_pipeline(null);
	draw_end();

	let empty_rt: render_target_t = map_get(render_path_render_targets, "empty_white");
	let empty: gpu_texture_t      = empty_rt._image;
	let mask: gpu_texture_t       = empty;
	let l1masks: slot_layer_t[]   = use_mask ? slot_layer_get_masks(l1) : null;
	if (l1masks != null) {
		// for (let i: i32 = 1; i < l1masks.length - 1; ++i) {
		// 	merge_layer(l1masks[i + 1], l1masks[i]);
		// }
		mask = l1masks[0].texpaint;
	}

	if (slot_layer_is_mask(l1)) {
		_gpu_begin(l0.texpaint);
		gpu_set_pipeline(pipes_merge_mask);
		gpu_set_texture(pipes_tex0_merge_mask, l1.texpaint);
		gpu_set_texture(pipes_texa_merge_mask, layers_temp_image);
		gpu_set_float(pipes_opac_merge_mask, slot_layer_get_opacity(l1));
		gpu_set_int(pipes_blending_merge_mask, l1.blending);
		gpu_set_vertex_buffer(const_data_screen_aligned_vb);
		gpu_set_index_buffer(const_data_screen_aligned_ib);
		gpu_draw();
		gpu_end();
	}

	if (slot_layer_is_layer(l1)) {
		if (l1.paint_base) {
			_gpu_begin(l0.texpaint);
			gpu_set_pipeline(pipes_merge);
			gpu_set_texture(pipes_tex0, l1.texpaint);
			gpu_set_texture(pipes_tex1, empty);
			gpu_set_texture(pipes_texmask, mask);
			gpu_set_texture(pipes_texa, layers_temp_image);
			gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
			gpu_set_float(pipes_tex1w, empty.width);
			gpu_set_int(pipes_blending, l1.blending);
			gpu_set_vertex_buffer(const_data_screen_aligned_vb);
			gpu_set_index_buffer(const_data_screen_aligned_ib);
			gpu_draw();
			gpu_end();
		}

		if (l0.texpaint_nor != null) {
			draw_begin(layers_temp_image);
			draw_set_pipeline(pipes_copy);
			draw_image(l0.texpaint_nor, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			if (l1.paint_nor) {
				_gpu_begin(l0.texpaint_nor);
				gpu_set_pipeline(pipes_merge);
				gpu_set_texture(pipes_tex0, l1.texpaint);
				gpu_set_texture(pipes_tex1, l1.texpaint_nor);
				gpu_set_texture(pipes_texmask, mask);
				gpu_set_texture(pipes_texa, layers_temp_image);
				gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
				gpu_set_float(pipes_tex1w, l1.texpaint_nor.width);
				gpu_set_int(pipes_blending, l1.paint_nor_blend ? 102 : 101);
				gpu_set_vertex_buffer(const_data_screen_aligned_vb);
				gpu_set_index_buffer(const_data_screen_aligned_ib);
				gpu_draw();
				gpu_end();
			}
		}

		if (l0.texpaint_pack != null) {
			draw_begin(layers_temp_image);
			draw_set_pipeline(pipes_copy);
			draw_image(l0.texpaint_pack, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			if (l1.paint_occ || l1.paint_rough || l1.paint_met || l1.paint_height) {
				if (l1.paint_occ && l1.paint_rough && l1.paint_met && l1.paint_height) {
					layers_commands_merge_pack(pipes_merge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask,
					                           l1.paint_height_blend ? 103 : 101);
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
	if (layers == null) {
		layers = project_layers;
	}
	layers_make_temp_img();
	layers_make_export_img();

	let empty_rt: render_target_t = map_get(render_path_render_targets, "empty_white");
	let empty: gpu_texture_t      = empty_rt._image;

	// Clear export layer
	_gpu_begin(layers_expa, null, null, clear_flag_t.COLOR, color_from_floats(0.0, 0.0, 0.0, 0.0));
	gpu_end();
	_gpu_begin(layers_expb, null, null, clear_flag_t.COLOR, color_from_floats(0.5, 0.5, 1.0, 0.0));
	gpu_end();
	_gpu_begin(layers_expc, null, null, clear_flag_t.COLOR, color_from_floats(1.0, 0.0, 0.0, 0.0));
	gpu_end();

	// Flatten layers
	for (let i: i32 = 0; i < layers.length; ++i) {
		let l1: slot_layer_t = layers[i];
		if (!slot_layer_is_visible(l1)) {
			continue;
		}
		if (!slot_layer_is_layer(l1)) {
			continue;
		}

		let mask: gpu_texture_t     = empty;
		let l1masks: slot_layer_t[] = slot_layer_get_masks(l1);
		if (l1masks != null) {
			if (l1masks.length > 1) {
				layers_make_temp_mask_img();
				draw_begin(pipes_temp_mask_image, clear_flag_t.COLOR, 0x00000000);
				draw_end();
				let l1: slot_layer_t = {texpaint : pipes_temp_mask_image};
				for (let i: i32 = 0; i < l1masks.length; ++i) {
					layers_merge_layer(l1, l1masks[i]);
				}
				mask = pipes_temp_mask_image;
			}
			else {
				mask = l1masks[0].texpaint;
			}
		}

		if (l1.paint_base) {
			draw_begin(layers_temp_image); // Copy to temp
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expa, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			if (context_raw.tool == tool_type_t.GIZMO) {
				// Do not multiply basecol by alpha
				draw_begin(layers_expa); // Copy to temp
				draw_set_pipeline(pipes_copy);
				draw_image(l1.texpaint, 0, 0);
				draw_set_pipeline(null);
				draw_end();
			}
			else {
				_gpu_begin(layers_expa);
				gpu_set_pipeline(pipes_merge);
				gpu_set_texture(pipes_tex0, l1.texpaint);
				gpu_set_texture(pipes_tex1, empty);
				gpu_set_texture(pipes_texmask, mask);
				gpu_set_texture(pipes_texa, layers_temp_image);
				gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
				gpu_set_float(pipes_tex1w, empty.width);
				gpu_set_int(pipes_blending, layers.length > 1 ? l1.blending : 0);
				gpu_set_vertex_buffer(const_data_screen_aligned_vb);
				gpu_set_index_buffer(const_data_screen_aligned_ib);
				gpu_draw();
				gpu_end();
			}
		}

		if (l1.paint_nor) {
			draw_begin(layers_temp_image);
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expb, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			_gpu_begin(layers_expb);
			gpu_set_pipeline(pipes_merge);
			gpu_set_texture(pipes_tex0, l1.texpaint);
			gpu_set_texture(pipes_tex1, l1.texpaint_nor);
			gpu_set_texture(pipes_texmask, mask);
			gpu_set_texture(pipes_texa, layers_temp_image);
			gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
			gpu_set_float(pipes_tex1w, l1.texpaint_nor.width);
			gpu_set_int(pipes_blending, l1.paint_nor_blend ? 102 : 101);
			gpu_set_vertex_buffer(const_data_screen_aligned_vb);
			gpu_set_index_buffer(const_data_screen_aligned_ib);
			gpu_draw();
			gpu_end();
		}

		if (l1.paint_occ || l1.paint_rough || l1.paint_met || l1.paint_height) {
			draw_begin(layers_temp_image);
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expc, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			if (l1.paint_occ && l1.paint_rough && l1.paint_met && l1.paint_height) {
				layers_commands_merge_pack(pipes_merge, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask,
				                           l1.paint_height_blend ? 103 : 101);
			}
			else {
				if (l1.paint_occ) {
					layers_commands_merge_pack(pipes_merge_r, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				}
				if (l1.paint_rough) {
					layers_commands_merge_pack(pipes_merge_g, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				}
				if (l1.paint_met) {
					layers_commands_merge_pack(pipes_merge_b, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				}
			}
		}
	}

	let l0: slot_layer_t = {texpaint : layers_expa, texpaint_nor : layers_expb, texpaint_pack : layers_expc};

	// Merge height map into normal map
	if (height_to_normal && make_material_height_used) {

		draw_begin(layers_temp_image);
		draw_set_pipeline(pipes_copy);
		draw_image(l0.texpaint_nor, 0, 0);
		draw_set_pipeline(null);
		draw_end();

		_gpu_begin(l0.texpaint_nor);
		gpu_set_pipeline(pipes_merge);
		gpu_set_texture(pipes_tex0, layers_temp_image);
		gpu_set_texture(pipes_tex1, l0.texpaint_pack);
		gpu_set_texture(pipes_texmask, empty);
		gpu_set_texture(pipes_texa, empty);
		gpu_set_float(pipes_opac, 1.0);
		gpu_set_float(pipes_tex1w, l0.texpaint_pack.width);
		gpu_set_int(pipes_blending, 104);
		gpu_set_vertex_buffer(const_data_screen_aligned_vb);
		gpu_set_index_buffer(const_data_screen_aligned_ib);
		gpu_draw();
		gpu_end();
	}

	return l0;
}

function layers_on_resized() {
	sys_notify_on_next_frame(function() {
		layers_resize();
		let _layer: slot_layer_t       = context_raw.layer;
		let _material: slot_material_t = context_raw.material;
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (l.fill_layer != null) {
				context_raw.layer    = l;
				context_raw.material = l.fill_layer;
				layers_update_fill_layer();
			}
		}
		context_raw.layer    = _layer;
		context_raw.material = _material;
		make_material_parse_paint_material();
	});
	util_uv_uvmap              = null;
	util_uv_uvmap_cached       = false;
	util_uv_trianglemap        = null;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached   = false;
	render_path_raytrace_ready = false;
}
