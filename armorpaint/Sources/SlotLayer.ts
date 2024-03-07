
class SlotLayerRaw {
	id = 0;
	name: string;
	ext = "";
	visible = true;
	parent: SlotLayerRaw = null; // Group (for layers) or layer (for masks)

	texpaint: image_t = null; // Base or mask
	///if is_paint
	texpaint_nor: image_t = null;
	texpaint_pack: image_t = null;
	texpaint_preview: image_t = null; // Layer preview
	///end

	maskOpacity = 1.0; // Opacity mask
	fill_layer: SlotMaterialRaw = null;
	show_panel = true;
	blending = blend_type_t.MIX;
	objectMask = 0;
	scale = 1.0;
	angle = 0.0;
	uvType = uv_type_t.UVMAP;
	paintBase = true;
	paintOpac = true;
	paintOcc = true;
	paintRough = true;
	paintMet = true;
	paintNor = true;
	paintNorBlend = true;
	paintHeight = true;
	paintHeightBlend = true;
	paintEmis = true;
	paintSubs = true;
	decalMat = mat4_identity(); // Decal layer
}

class SlotLayer {

	static create(ext = "", type = layer_slot_type_t.LAYER, parent: SlotLayerRaw = null): SlotLayerRaw {
		let raw = new SlotLayerRaw();
		if (ext == "") {
			raw.id = 0;
			for (let l of Project.layers) if (l.id >= raw.id) raw.id = l.id + 1;
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
			let format = Base.bits_handle.position == texture_bits_t.BITS8  ? "RGBA32" :
						 Base.bits_handle.position == texture_bits_t.BITS16 ? "RGBA64" :
						 									  			  "RGBA128";
			///end

			///if is_sculpt
			let format = "RGBA128";
			///end

			{
				let t = render_target_create();
				t.name = "texpaint" + ext;
				t.width = Config.get_texture_res_x();
				t.height = Config.get_texture_res_y();
				t.format = format;
				raw.texpaint = render_path_create_render_target(t)._image;
			}

			///if is_paint
			{
				let t = render_target_create();
				t.name = "texpaint_nor" + ext;
				t.width = Config.get_texture_res_x();
				t.height = Config.get_texture_res_y();
				t.format = format;
				raw.texpaint_nor = render_path_create_render_target(t)._image;
			}
			{
				let t = render_target_create();
				t.name = "texpaint_pack" + ext;
				t.width = Config.get_texture_res_x();
				t.height = Config.get_texture_res_y();
				t.format = format;
				raw.texpaint_pack = render_path_create_render_target(t)._image;
			}

			raw.texpaint_preview = image_create_render_target(UtilRender.layer_preview_size, UtilRender.layer_preview_size, tex_format_t.RGBA32);
			///end
		}

		///if is_paint
		else { // Mask
			raw.name = "Mask " + (raw.id + 1);
			let format = "RGBA32"; // Full bits for undo support, R8 is used
			raw.blending = blend_type_t.ADD;

			{
				let t = render_target_create();
				t.name = "texpaint" + ext;
				t.width = Config.get_texture_res_x();
				t.height = Config.get_texture_res_y();
				t.format = format;
				raw.texpaint = render_path_create_render_target(t)._image;
			}

			raw.texpaint_preview = image_create_render_target(UtilRender.layer_preview_size, UtilRender.layer_preview_size, tex_format_t.RGBA32);
		}
		///end

		return raw;
	}

	static delete = (raw: SlotLayerRaw) => {
		SlotLayer.unload(raw);

		if (SlotLayer.is_layer(raw)) {
			let masks = SlotLayer.get_masks(raw, false); // Prevents deleting group masks
			if (masks != null) for (let m of masks) SlotLayer.delete(m);
		}
		else if (SlotLayer.is_group(raw)) {
			let children = SlotLayer.get_children(raw);
			if (children != null) for (let c of children) SlotLayer.delete(c);
			let masks = SlotLayer.get_masks(raw);
			if (masks != null) for (let m of masks) SlotLayer.delete(m);
		}

		let lpos = Project.layers.indexOf(raw);
		array_remove(Project.layers, raw);
		// Undo can remove base layer and then restore it from undo layers
		if (Project.layers.length > 0) {
			Context.set_layer(Project.layers[lpos > 0 ? lpos - 1 : 0]);
		}

		// Do not remove empty groups if the last layer is deleted as this prevents redo from working properly
	}

	static unload = (raw: SlotLayerRaw) => {
		if (SlotLayer.is_group(raw)) return;

		let _texpaint = raw.texpaint;
		///if is_paint
		let _texpaint_nor = raw.texpaint_nor;
		let _texpaint_pack = raw.texpaint_pack;
		let _texpaint_preview = raw.texpaint_preview;
		///end

		let _next = () => {
			image_unload(_texpaint);
			///if is_paint
			if (_texpaint_nor != null) image_unload(_texpaint_nor);
			if (_texpaint_pack != null) image_unload(_texpaint_pack);
			image_unload(_texpaint_preview);
			///end
		}
		Base.notify_on_next_frame(_next);

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
			let _texpaint = raw.texpaint;
			raw.texpaint = other.texpaint;
			other.texpaint = _texpaint;

			///if is_paint
			let _texpaint_preview = raw.texpaint_preview;
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
			let _texpaint_nor = raw.texpaint_nor;
			let _texpaint_pack = raw.texpaint_pack;
			raw.texpaint_nor = other.texpaint_nor;
			raw.texpaint_pack = other.texpaint_pack;
			other.texpaint_nor = _texpaint_nor;
			other.texpaint_pack = _texpaint_pack;
		}
		///end
	}

	static clear = (raw: SlotLayerRaw, baseColor = 0x00000000, baseImage: image_t = null, occlusion = 1.0, roughness = Base.default_rough, metallic = 0.0) => {
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

		Context.raw.layer_preview_dirty = true;
		Context.raw.ddirty = 3;
	}

	static invert_mask = (raw: SlotLayerRaw) => {
		if (Base.pipe_invert8 == null) Base.make_pipe();
		let inverted = image_create_render_target(raw.texpaint.width, raw.texpaint.height, tex_format_t.RGBA32);
		g2_begin(inverted);
		g2_set_pipeline(Base.pipe_invert8);
		g2_draw_image(raw.texpaint, 0, 0);
		g2_set_pipeline(null);
		g2_end();
		let _texpaint = raw.texpaint;
		let _next = () => {
			image_unload(_texpaint);
		}
		Base.notify_on_next_frame(_next);
		raw.texpaint = render_path_render_targets.get("texpaint" + raw.id)._image = inverted;
		Context.raw.layer_preview_dirty = true;
		Context.raw.ddirty = 3;
	}

	static apply_mask = (raw: SlotLayerRaw) => {
		if (raw.parent.fill_layer != null) {
			SlotLayer.to_paint_layer(raw.parent);
		}
		if (SlotLayer.is_group(raw.parent)) {
			for (let c of SlotLayer.get_children(raw.parent)) {
				Base.apply_mask(c, raw);
			}
		}
		else {
			Base.apply_mask(raw.parent, raw);
		}
		SlotLayer.delete(raw);
	}

	static duplicate = (raw: SlotLayerRaw): SlotLayerRaw => {
		let layers = Project.layers;
		let i = layers.indexOf(raw) + 1;
		let l = SlotLayer.create("", SlotLayer.is_layer(raw) ? layer_slot_type_t.LAYER : SlotLayer.is_mask(raw) ? layer_slot_type_t.MASK : layer_slot_type_t.GROUP, raw.parent);
		layers.splice(i, 0, l);

		if (Base.pipe_merge == null) Base.make_pipe();
		if (SlotLayer.is_layer(raw)) {
			g2_begin(l.texpaint);
			g2_set_pipeline(Base.pipe_copy);
			g2_draw_image(raw.texpaint, 0, 0);
			g2_set_pipeline(null);
			g2_end();
			///if is_paint
			g2_begin(l.texpaint_nor);
			g2_set_pipeline(Base.pipe_copy);
			g2_draw_image(raw.texpaint_nor, 0, 0);
			g2_set_pipeline(null);
			g2_end();
			g2_begin(l.texpaint_pack);
			g2_set_pipeline(Base.pipe_copy);
			g2_draw_image(raw.texpaint_pack, 0, 0);
			g2_set_pipeline(null);
			g2_end();
			///end
		}
		else if (SlotLayer.is_mask(raw)) {
			g2_begin(l.texpaint);
			g2_set_pipeline(Base.pipe_copy8);
			g2_draw_image(raw.texpaint, 0, 0);
			g2_set_pipeline(null);
			g2_end();
		}

		///if is_paint
		g2_begin(l.texpaint_preview);
		g2_clear(0x00000000);
		g2_set_pipeline(Base.pipe_copy);
		g2_draw_scaled_image(raw.texpaint_preview, 0, 0, raw.texpaint_preview.width, raw.texpaint_preview.height);
		g2_set_pipeline(null);
		g2_end();
		///end

		l.visible = raw.visible;
		l.maskOpacity = raw.maskOpacity;
		l.fill_layer = raw.fill_layer;
		l.objectMask = raw.objectMask;
		l.blending = raw.blending;
		l.uvType = raw.uvType;
		l.scale = raw.scale;
		l.angle = raw.angle;
		l.paintBase = raw.paintBase;
		l.paintOpac = raw.paintOpac;
		l.paintOcc = raw.paintOcc;
		l.paintRough = raw.paintRough;
		l.paintMet = raw.paintMet;
		l.paintNor = raw.paintNor;
		l.paintNorBlend = raw.paintNorBlend;
		l.paintHeight = raw.paintHeight;
		l.paintHeightBlend = raw.paintHeightBlend;
		l.paintEmis = raw.paintEmis;
		l.paintSubs = raw.paintSubs;

		return l;
	}

	static resize_and_set_bits = (raw: SlotLayerRaw) => {
		let resX = Config.get_texture_res_x();
		let resY = Config.get_texture_res_y();
		let rts = render_path_render_targets;
		if (Base.pipe_merge == null) Base.make_pipe();

		if (SlotLayer.is_layer(raw)) {
			///if is_paint
			let format = Base.bits_handle.position == texture_bits_t.BITS8  ? tex_format_t.RGBA32 :
						 Base.bits_handle.position == texture_bits_t.BITS16 ? tex_format_t.RGBA64 :
						 									  			  tex_format_t.RGBA128;
			///end

			///if is_sculpt
			let format = tex_format_t.RGBA128;
			///end

			let _texpaint = raw.texpaint;
			raw.texpaint = image_create_render_target(resX, resY, format);
			g2_begin(raw.texpaint);
			g2_set_pipeline(Base.pipe_copy);
			g2_draw_scaled_image(_texpaint, 0, 0, resX, resY);
			g2_set_pipeline(null);
			g2_end();

			///if is_paint
			let _texpaint_nor = raw.texpaint_nor;
			let _texpaint_pack = raw.texpaint_pack;
			raw.texpaint_nor = image_create_render_target(resX, resY, format);
			raw.texpaint_pack = image_create_render_target(resX, resY, format);

			g2_begin(raw.texpaint_nor);
			g2_set_pipeline(Base.pipe_copy);
			g2_draw_scaled_image(_texpaint_nor, 0, 0, resX, resY);
			g2_set_pipeline(null);
			g2_end();

			g2_begin(raw.texpaint_pack);
			g2_set_pipeline(Base.pipe_copy);
			g2_draw_scaled_image(_texpaint_pack, 0, 0, resX, resY);
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
			Base.notify_on_next_frame(_next);

			rts.get("texpaint" + raw.ext)._image = raw.texpaint;
			///if is_paint
			rts.get("texpaint_nor" + raw.ext)._image = raw.texpaint_nor;
			rts.get("texpaint_pack" + raw.ext)._image = raw.texpaint_pack;
			///end
		}
		else if (SlotLayer.is_mask(raw)) {
			let _texpaint = raw.texpaint;
			raw.texpaint = image_create_render_target(resX, resY, tex_format_t.RGBA32);

			g2_begin(raw.texpaint);
			g2_set_pipeline(Base.pipe_copy8);
			g2_draw_scaled_image(_texpaint, 0, 0, resX, resY);
			g2_set_pipeline(null);
			g2_end();

			let _next = () => {
				image_unload(_texpaint);
			}
			Base.notify_on_next_frame(_next);

			rts.get("texpaint" + raw.ext)._image = raw.texpaint;
		}
	}

	static to_fill_layer = (raw: SlotLayerRaw) => {
		Context.set_layer(raw);
		raw.fill_layer = Context.raw.material;
		Base.update_fill_layer();
		let _next = () => {
			MakeMaterial.parse_paint_material();
			Context.raw.layer_preview_dirty = true;
			UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		}
		Base.notify_on_next_frame(_next);
	}

	static to_paint_layer = (raw: SlotLayerRaw) => {
		Context.set_layer(raw);
		raw.fill_layer = null;
		MakeMaterial.parse_paint_material();
		Context.raw.layer_preview_dirty = true;
		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	}

	static is_visible = (raw: SlotLayerRaw): bool => {
		return raw.visible && (raw.parent == null || raw.parent.visible);
	}

	static get_children = (raw: SlotLayerRaw): SlotLayerRaw[] => {
		let children: SlotLayerRaw[] = null; // Child layers of a group
		for (let l of Project.layers) {
			if (l.parent == raw && SlotLayer.is_layer(l)) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	static get_recursive_children = (raw: SlotLayerRaw): SlotLayerRaw[] => {
		let children: SlotLayerRaw[] = null;
		for (let l of Project.layers) {
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
		for (let l of Project.layers) {
			if (l.parent == raw && SlotLayer.is_mask(l)) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		// Child masks of a parent group
		if (includeGroupMasks) {
			if (raw.parent != null && SlotLayer.is_group(raw.parent)) {
				for (let l of Project.layers) {
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
		for (let l of Project.layers) {
			if (l.parent == raw && SlotLayer.is_mask(l)) {
				return true;
			}
		}
		// Group mask
		if (includeGroupMasks && raw.parent != null && SlotLayer.is_group(raw.parent)) {
			for (let l of Project.layers) {
				if (l.parent == raw.parent && SlotLayer.is_mask(l)) {
					return true;
				}
			}
		}
		return false;
	}

	static get_opacity = (raw: SlotLayerRaw): f32 => {
		let f = raw.maskOpacity;
		if (SlotLayer.is_layer(raw) && raw.parent != null) f *= raw.parent.maskOpacity;
		return f;
	}

	static get_object_mask = (raw: SlotLayerRaw): i32 => {
		return SlotLayer.is_mask(raw) ? raw.parent.objectMask : raw.objectMask;
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
		let oldIndex = Project.layers.indexOf(raw);

		let delta = to - oldIndex; // If delta > 0 the layer is moved up, otherwise down
		if (to < 0 || to > Project.layers.length - 1 || delta == 0) return false;

		// If the layer is moved up, all layers between the old position and the new one move one down.
		// The layers above the new position stay where they are.
		// If the new position is on top or on bottom no upper resp. lower layer exists.
		let newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			let children = SlotLayer.get_recursive_children(newUpperLayer);
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		let newLowerLayer = delta > 0 ? Project.layers[to] : (to > 0 ? Project.layers[to - 1] : null);

		if (SlotLayer.is_mask(raw)) {
			// Masks can not be on top.
			if (newUpperLayer == null) return false;
			// Masks should not be placed below a collapsed group. This condition can be savely removed.
			if (SlotLayer.is_in_group(newUpperLayer) && !SlotLayer.get_containing_group(newUpperLayer).show_panel) return false;
			// Masks should not be placed below a collapsed layer. This condition can be savely removed.
			if (SlotLayer.is_mask(newUpperLayer) && !newUpperLayer.parent.show_panel) return false;
		}

		if (SlotLayer.is_layer(raw)) {
			// Layers can not be moved directly below its own mask(s).
			if (newUpperLayer != null && SlotLayer.is_mask(newUpperLayer) && newUpperLayer.parent == raw) return false;
			// Layers can not be placed above a mask as the mask would be reparented.
			if (newLowerLayer != null && SlotLayer.is_mask(newLowerLayer)) return false;
		}

		// Currently groups can not be nested. Thus valid positions for groups are:
		if (SlotLayer.is_group(raw)) {
			// At the top.
			if (newUpperLayer == null) return true;
			// NOT below its own children.
			if (SlotLayer.get_containing_group(newUpperLayer) == raw) return false;
			// At the bottom.
			if (newLowerLayer == null) return true;
			// Above a group.
			if (SlotLayer.is_group(newLowerLayer)) return true;
			// Above a non-grouped layer.
			if (SlotLayer.is_layer(newLowerLayer) && !SlotLayer.is_in_group(newLowerLayer)) return true;
			else return false;
		}

		return true;
	}

	static move = (raw: SlotLayerRaw, to: i32) => {
		if (!SlotLayer.can_move(raw, to)) {
			return;
		}

		let pointers = TabLayers.init_layer_map();
		let oldIndex = Project.layers.indexOf(raw);
		let delta = to - oldIndex;
		let newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			let children = SlotLayer.get_recursive_children(newUpperLayer);
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		Context.set_layer(raw);
		History.order_layers(to);
		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;

		array_remove(Project.layers, raw);
		Project.layers.splice(to, 0, raw);

		if (SlotLayer.is_layer(raw)) {
			let oldParent = raw.parent;

			if (newUpperLayer == null)
				raw.parent = null; // Placed on top.
			else if (SlotLayer.is_in_group(newUpperLayer) && !SlotLayer.get_containing_group(newUpperLayer).show_panel)
				raw.parent = null; // Placed below a collapsed group.
			else if (SlotLayer.is_layer(newUpperLayer))
				raw.parent = newUpperLayer.parent; // Placed below a layer, use the same parent.
			else if (SlotLayer.is_group(newUpperLayer))
				raw.parent = newUpperLayer; // Placed as top layer in a group.
			else if (SlotLayer.is_group_mask(newUpperLayer))
				raw.parent = newUpperLayer.parent; // Placed in a group below the lowest group mask.
			else if (SlotLayer.is_layer_mask(newUpperLayer))
				raw.parent = SlotLayer.get_containing_group(newUpperLayer); // Either the group the mask belongs to or null.

			// Layers can have masks as children. These have to be moved, too.
			let layerMasks = SlotLayer.get_masks(raw, false);
			if (layerMasks != null) {
				for (let idx = 0; idx < layerMasks.length; ++idx) {
					let mask = layerMasks[idx];
					array_remove(Project.layers, mask);
					// If the masks are moved down each step increases the index below the layer by one.
					Project.layers.splice(delta > 0 ? oldIndex + delta - 1 : oldIndex + delta + idx, 0, mask);
				}
			}

			// The layer is the last layer in the group, remove it. Notice that this might remove group masks.
			if (oldParent != null && SlotLayer.get_children(oldParent) == null)
				SlotLayer.delete(oldParent);
		}
		else if (SlotLayer.is_mask(raw)) {
			// Precondition newUpperLayer != null, ensured in canMove.
			if (SlotLayer.is_layer(newUpperLayer) || SlotLayer.is_group(newUpperLayer))
				raw.parent = newUpperLayer;
			else if (SlotLayer.is_mask(newUpperLayer)) { // Group mask or layer mask.
				raw.parent = newUpperLayer.parent;
			}
		}
		else if (SlotLayer.is_group(raw)) {
			let children = SlotLayer.get_recursive_children(raw);
			if (children != null) {
				for (let idx = 0; idx < children.length; ++idx) {
					let child = children[idx];
					array_remove(Project.layers, child);
					// If the children are moved down each step increases the index below the layer by one.
					Project.layers.splice(delta > 0 ? oldIndex + delta - 1 : oldIndex + delta + idx, 0, child);
				}
			}
		}

		for (let m of Project.materials) TabLayers.remap_layer_pointers(m.canvas.nodes, TabLayers.fill_layer_map(pointers));
	}
}
