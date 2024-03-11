
class SlotLayerRaw {
	id: i32 = 0;
	name: string;
	ext: string = "";
	visible: bool = true;
	parent: SlotLayerRaw = null; // Group (for layers) or layer (for masks)

	texpaint: image_t = null; // Base or mask
	///if is_paint
	texpaint_nor: image_t = null;
	texpaint_pack: image_t = null;
	texpaint_preview: image_t = null; // Layer preview
	///end

	mask_opacity: f32 = 1.0; // Opacity mask
	fill_layer: SlotMaterialRaw = null;
	show_panel: bool = true;
	blending = blend_type_t.MIX;
	object_mask: i32 = 0;
	scale: f32 = 1.0;
	angle: f32 = 0.0;
	uv_type = uv_type_t.UVMAP;
	paint_base: bool = true;
	paint_opac: bool = true;
	paint_occ: bool = true;
	paint_rough: bool = true;
	paint_met: bool = true;
	paint_nor: bool = true;
	paint_nor_blend: bool = true;
	paint_height: bool = true;
	paint_height_blend: bool = true;
	paint_emis: bool = true;
	paint_subs: bool = true;
	decal_mat: mat4_t = mat4_identity(); // Decal layer
}

class SlotLayer {

	static create(ext = "", type = layer_slot_type_t.LAYER, parent: SlotLayerRaw = null): SlotLayerRaw {
		let raw: SlotLayerRaw = new SlotLayerRaw();
		if (ext == "") {
			raw.id = 0;
			for (let l of project_layers) if (l.id >= raw.id) raw.id = l.id + 1;
			ext = raw.id + "";
		}
		raw.ext = ext;
		raw.parent = parent;

		if (type == layer_slot_type_t.GROUP) {
			raw.name = "Group " + (raw.id + 1);
		}
		else if (type == layer_slot_type_t.LAYER) {
			raw.name = "Layer " + (raw.id + 1);
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
			raw.name = "Mask " + (raw.id + 1);
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

	static delete = (raw: SlotLayerRaw) => {
		SlotLayer.unload(raw);

		if (SlotLayer.is_layer(raw)) {
			let masks: SlotLayerRaw[] = SlotLayer.get_masks(raw, false); // Prevents deleting group masks
			if (masks != null) for (let m of masks) SlotLayer.delete(m);
		}
		else if (SlotLayer.is_group(raw)) {
			let children: SlotLayerRaw[] = SlotLayer.get_children(raw);
			if (children != null) for (let c of children) SlotLayer.delete(c);
			let masks: SlotLayerRaw[] = SlotLayer.get_masks(raw);
			if (masks != null) for (let m of masks) SlotLayer.delete(m);
		}

		let lpos: i32 = project_layers.indexOf(raw);
		array_remove(project_layers, raw);
		// Undo can remove base layer and then restore it from undo layers
		if (project_layers.length > 0) {
			context_set_layer(project_layers[lpos > 0 ? lpos - 1 : 0]);
		}

		// Do not remove empty groups if the last layer is deleted as this prevents redo from working properly
	}

	static unload = (raw: SlotLayerRaw) => {
		if (SlotLayer.is_group(raw)) return;

		let _texpaint: image_t = raw.texpaint;
		///if is_paint
		let _texpaint_nor: image_t = raw.texpaint_nor;
		let _texpaint_pack: image_t = raw.texpaint_pack;
		let _texpaint_preview: image_t = raw.texpaint_preview;
		///end

		let _next = () => {
			image_unload(_texpaint);
			///if is_paint
			if (_texpaint_nor != null) image_unload(_texpaint_nor);
			if (_texpaint_pack != null) image_unload(_texpaint_pack);
			image_unload(_texpaint_preview);
			///end
		}
		base_notify_on_next_frame(_next);

		render_path_render_targets.delete("texpaint" + raw.ext);
		///if is_paint
		if (SlotLayer.is_layer(raw)) {
			render_path_render_targets.delete("texpaint_nor" + raw.ext);
			render_path_render_targets.delete("texpaint_pack" + raw.ext);
		}
		///end
	}

	static swap = (raw: SlotLayerRaw, other: SlotLayerRaw) => {
		if ((SlotLayer.is_layer(raw) || SlotLayer.is_mask(raw)) && (SlotLayer.is_layer(other) || SlotLayer.is_mask(other))) {
			render_path_render_targets.get("texpaint" + raw.ext)._image = other.texpaint;
			render_path_render_targets.get("texpaint" + other.ext)._image = raw.texpaint;
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
		if (SlotLayer.is_layer(raw) && SlotLayer.is_layer(other)) {
			render_path_render_targets.get("texpaint_nor" + raw.ext)._image = other.texpaint_nor;
			render_path_render_targets.get("texpaint_pack" + raw.ext)._image = other.texpaint_pack;
			render_path_render_targets.get("texpaint_nor" + other.ext)._image = raw.texpaint_nor;
			render_path_render_targets.get("texpaint_pack" + other.ext)._image = raw.texpaint_pack;
			let _texpaint_nor: image_t = raw.texpaint_nor;
			let _texpaint_pack: image_t = raw.texpaint_pack;
			raw.texpaint_nor = other.texpaint_nor;
			raw.texpaint_pack = other.texpaint_pack;
			other.texpaint_nor = _texpaint_nor;
			other.texpaint_pack = _texpaint_pack;
		}
		///end
	}

	static clear = (raw: SlotLayerRaw, baseColor = 0x00000000, baseImage: image_t = null, occlusion = 1.0, roughness = base_default_rough, metallic = 0.0) => {
		g4_begin(raw.texpaint);
		g4_clear(baseColor); // Base
		g4_end();
		if (baseImage != null) {
			g2_begin(raw.texpaint);
			g2_draw_scaled_image(baseImage, 0, 0, raw.texpaint.width, raw.texpaint.height);
			g2_end();
		}

		///if is_paint
		if (SlotLayer.is_layer(raw)) {
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

	static invert_mask = (raw: SlotLayerRaw) => {
		if (base_pipe_invert8 == null) base_make_pipe();
		let inverted: image_t = image_create_render_target(raw.texpaint.width, raw.texpaint.height, tex_format_t.RGBA32);
		g2_begin(inverted);
		g2_set_pipeline(base_pipe_invert8);
		g2_draw_image(raw.texpaint, 0, 0);
		g2_set_pipeline(null);
		g2_end();
		let _texpaint: image_t = raw.texpaint;
		let _next = () => {
			image_unload(_texpaint);
		}
		base_notify_on_next_frame(_next);
		raw.texpaint = render_path_render_targets.get("texpaint" + raw.id)._image = inverted;
		context_raw.layer_preview_dirty = true;
		context_raw.ddirty = 3;
	}

	static apply_mask = (raw: SlotLayerRaw) => {
		if (raw.parent.fill_layer != null) {
			SlotLayer.to_paint_layer(raw.parent);
		}
		if (SlotLayer.is_group(raw.parent)) {
			for (let c of SlotLayer.get_children(raw.parent)) {
				base_apply_mask(c, raw);
			}
		}
		else {
			base_apply_mask(raw.parent, raw);
		}
		SlotLayer.delete(raw);
	}

	static duplicate = (raw: SlotLayerRaw): SlotLayerRaw => {
		let layers: SlotLayerRaw[] = project_layers;
		let i: i32 = layers.indexOf(raw) + 1;
		let l: SlotLayerRaw = SlotLayer.create("", SlotLayer.is_layer(raw) ? layer_slot_type_t.LAYER : SlotLayer.is_mask(raw) ? layer_slot_type_t.MASK : layer_slot_type_t.GROUP, raw.parent);
		layers.splice(i, 0, l);

		if (base_pipe_merge == null) base_make_pipe();
		if (SlotLayer.is_layer(raw)) {
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
		else if (SlotLayer.is_mask(raw)) {
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

	static resize_and_set_bits = (raw: SlotLayerRaw) => {
		let res_x: i32 = config_get_texture_res_x();
		let res_y: i32 = config_get_texture_res_y();
		let rts: map_t<string, render_target_t> = render_path_render_targets;
		if (base_pipe_merge == null) base_make_pipe();

		if (SlotLayer.is_layer(raw)) {
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

			let _next = () => {
				image_unload(_texpaint);
				///if is_paint
				image_unload(_texpaint_nor);
				image_unload(_texpaint_pack);
				///end
			}
			base_notify_on_next_frame(_next);

			rts.get("texpaint" + raw.ext)._image = raw.texpaint;
			///if is_paint
			rts.get("texpaint_nor" + raw.ext)._image = raw.texpaint_nor;
			rts.get("texpaint_pack" + raw.ext)._image = raw.texpaint_pack;
			///end
		}
		else if (SlotLayer.is_mask(raw)) {
			let _texpaint: image_t = raw.texpaint;
			raw.texpaint = image_create_render_target(res_x, res_y, tex_format_t.RGBA32);

			g2_begin(raw.texpaint);
			g2_set_pipeline(base_pipe_copy8);
			g2_draw_scaled_image(_texpaint, 0, 0, res_x, res_y);
			g2_set_pipeline(null);
			g2_end();

			let _next = () => {
				image_unload(_texpaint);
			}
			base_notify_on_next_frame(_next);

			rts.get("texpaint" + raw.ext)._image = raw.texpaint;
		}
	}

	static to_fill_layer = (raw: SlotLayerRaw) => {
		context_set_layer(raw);
		raw.fill_layer = context_raw.material;
		base_update_fill_layer();
		let _next = () => {
			MakeMaterial.parse_paint_material();
			context_raw.layer_preview_dirty = true;
			ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		}
		base_notify_on_next_frame(_next);
	}

	static to_paint_layer = (raw: SlotLayerRaw) => {
		context_set_layer(raw);
		raw.fill_layer = null;
		MakeMaterial.parse_paint_material();
		context_raw.layer_preview_dirty = true;
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	}

	static is_visible = (raw: SlotLayerRaw): bool => {
		return raw.visible && (raw.parent == null || raw.parent.visible);
	}

	static get_children = (raw: SlotLayerRaw): SlotLayerRaw[] => {
		let children: SlotLayerRaw[] = null; // Child layers of a group
		for (let l of project_layers) {
			if (l.parent == raw && SlotLayer.is_layer(l)) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	static get_recursive_children = (raw: SlotLayerRaw): SlotLayerRaw[] => {
		let children: SlotLayerRaw[] = null;
		for (let l of project_layers) {
			if (l.parent == raw) { // Child layers and group masks
				if (children == null) children = [];
				children.push(l);
			}
			if (l.parent != null && l.parent.parent == raw) { // Layer masks
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	static get_masks = (raw: SlotLayerRaw, includeGroupMasks = true): SlotLayerRaw[] => {
		if (SlotLayer.is_mask(raw)) return null;

		let children: SlotLayerRaw[] = null;
		// Child masks of a layer
		for (let l of project_layers) {
			if (l.parent == raw && SlotLayer.is_mask(l)) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		// Child masks of a parent group
		if (includeGroupMasks) {
			if (raw.parent != null && SlotLayer.is_group(raw.parent)) {
				for (let l of project_layers) {
					if (l.parent == raw.parent && SlotLayer.is_mask(l)) {
						if (children == null) children = [];
						children.push(l);
					}
				}
			}
		}
		return children;
	}

	static has_masks = (raw: SlotLayerRaw, includeGroupMasks = true): bool => {
		// Layer mask
		for (let l of project_layers) {
			if (l.parent == raw && SlotLayer.is_mask(l)) {
				return true;
			}
		}
		// Group mask
		if (includeGroupMasks && raw.parent != null && SlotLayer.is_group(raw.parent)) {
			for (let l of project_layers) {
				if (l.parent == raw.parent && SlotLayer.is_mask(l)) {
					return true;
				}
			}
		}
		return false;
	}

	static get_opacity = (raw: SlotLayerRaw): f32 => {
		let f: f32 = raw.mask_opacity;
		if (SlotLayer.is_layer(raw) && raw.parent != null) f *= raw.parent.mask_opacity;
		return f;
	}

	static get_object_mask = (raw: SlotLayerRaw): i32 => {
		return SlotLayer.is_mask(raw) ? raw.parent.object_mask : raw.object_mask;
	}

	static is_layer = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor != null;
		///end
		///if is_sculpt
		return raw.texpaint != null;
		///end
	}

	static is_group = (raw: SlotLayerRaw): bool => {
		return raw.texpaint == null;
	}

	static get_containing_group = (raw: SlotLayerRaw): SlotLayerRaw => {
		if (raw.parent != null && SlotLayer.is_group(raw.parent))
			return raw.parent;
		else if (raw.parent != null && raw.parent.parent != null && SlotLayer.is_group(raw.parent.parent))
			return raw.parent.parent;
		else return null;
	}

	static is_mask = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor == null;
		///end
		///if is_sculpt
		return false;
		///end
	}

	static is_group_mask = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor == null && SlotLayer.is_group(raw.parent);
		///end
		///if is_sculpt
		return false;
		///end
	}

	static is_layer_mask = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor == null && SlotLayer.is_layer(raw.parent);
		///end
		///if is_sculpt
		return false;
		///end
	}

	static is_in_group = (raw: SlotLayerRaw): bool => {
		return raw.parent != null && (SlotLayer.is_group(raw.parent) || (raw.parent.parent != null && SlotLayer.is_group(raw.parent.parent)));
	}

	static can_move = (raw: SlotLayerRaw, to: i32): bool => {
		let old_index: i32 = project_layers.indexOf(raw);

		let delta: i32 = to - old_index; // If delta > 0 the layer is moved up, otherwise down
		if (to < 0 || to > project_layers.length - 1 || delta == 0) return false;

		// If the layer is moved up, all layers between the old position and the new one move one down.
		// The layers above the new position stay where they are.
		// If the new position is on top or on bottom no upper resp. lower layer exists.
		let new_upper_layer: SlotLayerRaw = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (new_upper_layer != null && !new_upper_layer.show_panel) {
			let children: SlotLayerRaw[] = SlotLayer.get_recursive_children(new_upper_layer);
			to -= children != null ? children.length : 0;
			delta = to - old_index;
			new_upper_layer = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];
		}

		let new_lower_layer: SlotLayerRaw = delta > 0 ? project_layers[to] : (to > 0 ? project_layers[to - 1] : null);

		if (SlotLayer.is_mask(raw)) {
			// Masks can not be on top.
			if (new_upper_layer == null) return false;
			// Masks should not be placed below a collapsed group. This condition can be savely removed.
			if (SlotLayer.is_in_group(new_upper_layer) && !SlotLayer.get_containing_group(new_upper_layer).show_panel) return false;
			// Masks should not be placed below a collapsed layer. This condition can be savely removed.
			if (SlotLayer.is_mask(new_upper_layer) && !new_upper_layer.parent.show_panel) return false;
		}

		if (SlotLayer.is_layer(raw)) {
			// Layers can not be moved directly below its own mask(s).
			if (new_upper_layer != null && SlotLayer.is_mask(new_upper_layer) && new_upper_layer.parent == raw) return false;
			// Layers can not be placed above a mask as the mask would be reparented.
			if (new_lower_layer != null && SlotLayer.is_mask(new_lower_layer)) return false;
		}

		// Currently groups can not be nested. Thus valid positions for groups are:
		if (SlotLayer.is_group(raw)) {
			// At the top.
			if (new_upper_layer == null) return true;
			// NOT below its own children.
			if (SlotLayer.get_containing_group(new_upper_layer) == raw) return false;
			// At the bottom.
			if (new_lower_layer == null) return true;
			// Above a group.
			if (SlotLayer.is_group(new_lower_layer)) return true;
			// Above a non-grouped layer.
			if (SlotLayer.is_layer(new_lower_layer) && !SlotLayer.is_in_group(new_lower_layer)) return true;
			else return false;
		}

		return true;
	}

	static move = (raw: SlotLayerRaw, to: i32) => {
		if (!SlotLayer.can_move(raw, to)) {
			return;
		}

		let pointers: map_t<SlotLayerRaw, i32> = TabLayers.init_layer_map();
		let old_index: i32 = project_layers.indexOf(raw);
		let delta: i32 = to - old_index;
		let new_upper_layer: SlotLayerRaw = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (new_upper_layer != null && !new_upper_layer.show_panel) {
			let children: SlotLayerRaw[] = SlotLayer.get_recursive_children(new_upper_layer);
			to -= children != null ? children.length : 0;
			delta = to - old_index;
			new_upper_layer = delta > 0 ? (to < project_layers.length - 1 ? project_layers[to + 1] : null) : project_layers[to];
		}

		context_set_layer(raw);
		history_order_layers(to);
		ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;

		array_remove(project_layers, raw);
		project_layers.splice(to, 0, raw);

		if (SlotLayer.is_layer(raw)) {
			let old_parent: SlotLayerRaw = raw.parent;

			if (new_upper_layer == null)
				raw.parent = null; // Placed on top.
			else if (SlotLayer.is_in_group(new_upper_layer) && !SlotLayer.get_containing_group(new_upper_layer).show_panel)
				raw.parent = null; // Placed below a collapsed group.
			else if (SlotLayer.is_layer(new_upper_layer))
				raw.parent = new_upper_layer.parent; // Placed below a layer, use the same parent.
			else if (SlotLayer.is_group(new_upper_layer))
				raw.parent = new_upper_layer; // Placed as top layer in a group.
			else if (SlotLayer.is_group_mask(new_upper_layer))
				raw.parent = new_upper_layer.parent; // Placed in a group below the lowest group mask.
			else if (SlotLayer.is_layer_mask(new_upper_layer))
				raw.parent = SlotLayer.get_containing_group(new_upper_layer); // Either the group the mask belongs to or null.

			// Layers can have masks as children. These have to be moved, too.
			let layer_masks: SlotLayerRaw[] = SlotLayer.get_masks(raw, false);
			if (layer_masks != null) {
				for (let idx: i32 = 0; idx < layer_masks.length; ++idx) {
					let mask: SlotLayerRaw = layer_masks[idx];
					array_remove(project_layers, mask);
					// If the masks are moved down each step increases the index below the layer by one.
					project_layers.splice(delta > 0 ? old_index + delta - 1 : old_index + delta + idx, 0, mask);
				}
			}

			// The layer is the last layer in the group, remove it. Notice that this might remove group masks.
			if (old_parent != null && SlotLayer.get_children(old_parent) == null)
				SlotLayer.delete(old_parent);
		}
		else if (SlotLayer.is_mask(raw)) {
			// Precondition newUpperLayer != null, ensured in canMove.
			if (SlotLayer.is_layer(new_upper_layer) || SlotLayer.is_group(new_upper_layer))
				raw.parent = new_upper_layer;
			else if (SlotLayer.is_mask(new_upper_layer)) { // Group mask or layer mask.
				raw.parent = new_upper_layer.parent;
			}
		}
		else if (SlotLayer.is_group(raw)) {
			let children: SlotLayerRaw[] = SlotLayer.get_recursive_children(raw);
			if (children != null) {
				for (let idx: i32 = 0; idx < children.length; ++idx) {
					let child: SlotLayerRaw = children[idx];
					array_remove(project_layers, child);
					// If the children are moved down each step increases the index below the layer by one.
					project_layers.splice(delta > 0 ? old_index + delta - 1 : old_index + delta + idx, 0, child);
				}
			}
		}

		for (let m of project_materials) TabLayers.remap_layer_pointers(m.canvas.nodes, TabLayers.fill_layer_map(pointers));
	}
}
