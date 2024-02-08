
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
	blending = BlendType.BlendMix;
	objectMask = 0;
	scale = 1.0;
	angle = 0.0;
	uvType = UVType.UVMap;
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

	static create(ext = "", type = LayerSlotType.SlotLayer, parent: SlotLayerRaw = null): SlotLayerRaw {
		let raw = new SlotLayerRaw();
		if (ext == "") {
			raw.id = 0;
			for (let l of Project.layers) if (l.id >= raw.id) raw.id = l.id + 1;
			ext = raw.id + "";
		}
		raw.ext = ext;
		raw.parent = parent;

		if (type == LayerSlotType.SlotGroup) {
			raw.name = "Group " + (raw.id + 1);
		}
		else if (type == LayerSlotType.SlotLayer) {
			raw.name = "Layer " + (raw.id + 1);
			///if is_paint
			let format = Base.bitsHandle.position == TextureBits.Bits8  ? "RGBA32" :
						 Base.bitsHandle.position == TextureBits.Bits16 ? "RGBA64" :
						 									  			  "RGBA128";
			///end

			///if is_sculpt
			let format = "RGBA128";
			///end

			{
				let t = render_target_create();
				t.name = "texpaint" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				raw.texpaint = render_path_create_render_target(t).image;
			}

			///if is_paint
			{
				let t = render_target_create();
				t.name = "texpaint_nor" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				raw.texpaint_nor = render_path_create_render_target(t).image;
			}
			{
				let t = render_target_create();
				t.name = "texpaint_pack" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				raw.texpaint_pack = render_path_create_render_target(t).image;
			}

			raw.texpaint_preview = image_create_render_target(UtilRender.layerPreviewSize, UtilRender.layerPreviewSize, tex_format_t.RGBA32);
			///end
		}

		///if is_paint
		else { // Mask
			raw.name = "Mask " + (raw.id + 1);
			let format = "RGBA32"; // Full bits for undo support, R8 is used
			raw.blending = BlendType.BlendAdd;

			{
				let t = render_target_create();
				t.name = "texpaint" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				raw.texpaint = render_path_create_render_target(t).image;
			}

			raw.texpaint_preview = image_create_render_target(UtilRender.layerPreviewSize, UtilRender.layerPreviewSize, tex_format_t.RGBA32);
		}
		///end

		return raw;
	}

	static delete = (raw: SlotLayerRaw) => {
		SlotLayer.unload(raw);

		if (SlotLayer.isLayer(raw)) {
			let masks = SlotLayer.getMasks(raw, false); // Prevents deleting group masks
			if (masks != null) for (let m of masks) SlotLayer.delete(m);
		}
		else if (SlotLayer.isGroup(raw)) {
			let children = SlotLayer.getChildren(raw);
			if (children != null) for (let c of children) SlotLayer.delete(c);
			let masks = SlotLayer.getMasks(raw);
			if (masks != null) for (let m of masks) SlotLayer.delete(m);
		}

		let lpos = Project.layers.indexOf(raw);
		array_remove(Project.layers, raw);
		// Undo can remove base layer and then restore it from undo layers
		if (Project.layers.length > 0) {
			Context.setLayer(Project.layers[lpos > 0 ? lpos - 1 : 0]);
		}

		// Do not remove empty groups if the last layer is deleted as this prevents redo from working properly
	}

	static unload = (raw: SlotLayerRaw) => {
		if (SlotLayer.isGroup(raw)) return;

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
		Base.notifyOnNextFrame(_next);

		render_path_render_targets.delete("texpaint" + raw.ext);
		///if is_paint
		if (SlotLayer.isLayer(raw)) {
			render_path_render_targets.delete("texpaint_nor" + raw.ext);
			render_path_render_targets.delete("texpaint_pack" + raw.ext);
		}
		///end
	}

	static swap = (raw: SlotLayerRaw, other: SlotLayerRaw) => {
		if ((SlotLayer.isLayer(raw) || SlotLayer.isMask(raw)) && (SlotLayer.isLayer(other) || SlotLayer.isMask(other))) {
			render_path_render_targets.get("texpaint" + raw.ext).image = other.texpaint;
			render_path_render_targets.get("texpaint" + other.ext).image = raw.texpaint;
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
		if (SlotLayer.isLayer(raw) && SlotLayer.isLayer(other)) {
			render_path_render_targets.get("texpaint_nor" + raw.ext).image = other.texpaint_nor;
			render_path_render_targets.get("texpaint_pack" + raw.ext).image = other.texpaint_pack;
			render_path_render_targets.get("texpaint_nor" + other.ext).image = raw.texpaint_nor;
			render_path_render_targets.get("texpaint_pack" + other.ext).image = raw.texpaint_pack;
			let _texpaint_nor = raw.texpaint_nor;
			let _texpaint_pack = raw.texpaint_pack;
			raw.texpaint_nor = other.texpaint_nor;
			raw.texpaint_pack = other.texpaint_pack;
			other.texpaint_nor = _texpaint_nor;
			other.texpaint_pack = _texpaint_pack;
		}
		///end
	}

	static clear = (raw: SlotLayerRaw, baseColor = 0x00000000, baseImage: image_t = null, occlusion = 1.0, roughness = Base.defaultRough, metallic = 0.0) => {
		g4_begin(raw.texpaint);
		g4_clear(baseColor); // Base
		g4_end();
		if (baseImage != null) {
			g2_begin(raw.texpaint, false);
			g2_draw_scaled_image(baseImage, 0, 0, raw.texpaint.width, raw.texpaint.height);
			g2_end();
		}

		///if is_paint
		if (SlotLayer.isLayer(raw)) {
			g4_begin(raw.texpaint_nor);
			g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
			g4_end();
			g4_begin(raw.texpaint_pack);
			g4_clear(color_from_floats(occlusion, roughness, metallic, 0.0)); // Occ, rough, met
			g4_end();
		}
		///end

		Context.raw.layerPreviewDirty = true;
		Context.raw.ddirty = 3;
	}

	static invertMask = (raw: SlotLayerRaw) => {
		if (Base.pipeInvert8 == null) Base.makePipe();
		let inverted = image_create_render_target(raw.texpaint.width, raw.texpaint.height, tex_format_t.RGBA32);
		g2_begin(inverted, false);
		g2_set_pipeline(Base.pipeInvert8);
		g2_draw_image(raw.texpaint, 0, 0);
		g2_set_pipeline(null);
		g2_end();
		let _texpaint = raw.texpaint;
		let _next = () => {
			image_unload(_texpaint);
		}
		Base.notifyOnNextFrame(_next);
		raw.texpaint = render_path_render_targets.get("texpaint" + raw.id).image = inverted;
		Context.raw.layerPreviewDirty = true;
		Context.raw.ddirty = 3;
	}

	static applyMask = (raw: SlotLayerRaw) => {
		if (raw.parent.fill_layer != null) {
			SlotLayer.toPaintLayer(raw.parent);
		}
		if (SlotLayer.isGroup(raw.parent)) {
			for (let c of SlotLayer.getChildren(raw.parent)) {
				Base.applyMask(c, raw);
			}
		}
		else {
			Base.applyMask(raw.parent, raw);
		}
		SlotLayer.delete(raw);
	}

	static duplicate = (raw: SlotLayerRaw): SlotLayerRaw => {
		let layers = Project.layers;
		let i = layers.indexOf(raw) + 1;
		let l = SlotLayer.create("", SlotLayer.isLayer(raw) ? LayerSlotType.SlotLayer : SlotLayer.isMask(raw) ? LayerSlotType.SlotMask : LayerSlotType.SlotGroup, raw.parent);
		layers.splice(i, 0, l);

		if (Base.pipeMerge == null) Base.makePipe();
		if (SlotLayer.isLayer(raw)) {
			g2_begin(l.texpaint, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_image(raw.texpaint, 0, 0);
			g2_set_pipeline(null);
			g2_end();
			///if is_paint
			g2_begin(l.texpaint_nor, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_image(raw.texpaint_nor, 0, 0);
			g2_set_pipeline(null);
			g2_end();
			g2_begin(l.texpaint_pack, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_image(raw.texpaint_pack, 0, 0);
			g2_set_pipeline(null);
			g2_end();
			///end
		}
		else if (SlotLayer.isMask(raw)) {
			g2_begin(l.texpaint, false);
			g2_set_pipeline(Base.pipeCopy8);
			g2_draw_image(raw.texpaint, 0, 0);
			g2_set_pipeline(null);
			g2_end();
		}

		///if is_paint
		g2_begin(l.texpaint_preview, true, 0x00000000);
		g2_set_pipeline(Base.pipeCopy);
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

	static resizeAndSetBits = (raw: SlotLayerRaw) => {
		let resX = Config.getTextureResX();
		let resY = Config.getTextureResY();
		let rts = render_path_render_targets;
		if (Base.pipeMerge == null) Base.makePipe();

		if (SlotLayer.isLayer(raw)) {
			///if is_paint
			let format = Base.bitsHandle.position == TextureBits.Bits8  ? tex_format_t.RGBA32 :
						 Base.bitsHandle.position == TextureBits.Bits16 ? tex_format_t.RGBA64 :
						 									  			  tex_format_t.RGBA128;
			///end

			///if is_sculpt
			let format = tex_format_t.RGBA128;
			///end

			let _texpaint = raw.texpaint;
			raw.texpaint = image_create_render_target(resX, resY, format);
			g2_begin(raw.texpaint, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_scaled_image(_texpaint, 0, 0, resX, resY);
			g2_set_pipeline(null);
			g2_end();

			///if is_paint
			let _texpaint_nor = raw.texpaint_nor;
			let _texpaint_pack = raw.texpaint_pack;
			raw.texpaint_nor = image_create_render_target(resX, resY, format);
			raw.texpaint_pack = image_create_render_target(resX, resY, format);

			g2_begin(raw.texpaint_nor, false);
			g2_set_pipeline(Base.pipeCopy);
			g2_draw_scaled_image(_texpaint_nor, 0, 0, resX, resY);
			g2_set_pipeline(null);
			g2_end();

			g2_begin(raw.texpaint_pack, false);
			g2_set_pipeline(Base.pipeCopy);
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
			Base.notifyOnNextFrame(_next);

			rts.get("texpaint" + raw.ext).image = raw.texpaint;
			///if is_paint
			rts.get("texpaint_nor" + raw.ext).image = raw.texpaint_nor;
			rts.get("texpaint_pack" + raw.ext).image = raw.texpaint_pack;
			///end
		}
		else if (SlotLayer.isMask(raw)) {
			let _texpaint = raw.texpaint;
			raw.texpaint = image_create_render_target(resX, resY, tex_format_t.RGBA32);

			g2_begin(raw.texpaint, false);
			g2_set_pipeline(Base.pipeCopy8);
			g2_draw_scaled_image(_texpaint, 0, 0, resX, resY);
			g2_set_pipeline(null);
			g2_end();

			let _next = () => {
				image_unload(_texpaint);
			}
			Base.notifyOnNextFrame(_next);

			rts.get("texpaint" + raw.ext).image = raw.texpaint;
		}
	}

	static toFillLayer = (raw: SlotLayerRaw) => {
		Context.setLayer(raw);
		raw.fill_layer = Context.raw.material;
		Base.updateFillLayer();
		let _next = () => {
			MakeMaterial.parsePaintMaterial();
			Context.raw.layerPreviewDirty = true;
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
		}
		Base.notifyOnNextFrame(_next);
	}

	static toPaintLayer = (raw: SlotLayerRaw) => {
		Context.setLayer(raw);
		raw.fill_layer = null;
		MakeMaterial.parsePaintMaterial();
		Context.raw.layerPreviewDirty = true;
		UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
	}

	static isVisible = (raw: SlotLayerRaw): bool => {
		return raw.visible && (raw.parent == null || raw.parent.visible);
	}

	static getChildren = (raw: SlotLayerRaw): SlotLayerRaw[] => {
		let children: SlotLayerRaw[] = null; // Child layers of a group
		for (let l of Project.layers) {
			if (l.parent == raw && SlotLayer.isLayer(l)) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	static getRecursiveChildren = (raw: SlotLayerRaw): SlotLayerRaw[] => {
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

	static getMasks = (raw: SlotLayerRaw, includeGroupMasks = true): SlotLayerRaw[] => {
		if (SlotLayer.isMask(raw)) return null;

		let children: SlotLayerRaw[] = null;
		// Child masks of a layer
		for (let l of Project.layers) {
			if (l.parent == raw && SlotLayer.isMask(l)) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		// Child masks of a parent group
		if (includeGroupMasks) {
			if (raw.parent != null && SlotLayer.isGroup(raw.parent)) {
				for (let l of Project.layers) {
					if (l.parent == raw.parent && SlotLayer.isMask(l)) {
						if (children == null) children = [];
						children.push(l);
					}
				}
			}
		}
		return children;
	}

	static hasMasks = (raw: SlotLayerRaw, includeGroupMasks = true): bool => {
		// Layer mask
		for (let l of Project.layers) {
			if (l.parent == raw && SlotLayer.isMask(l)) {
				return true;
			}
		}
		// Group mask
		if (includeGroupMasks && raw.parent != null && SlotLayer.isGroup(raw.parent)) {
			for (let l of Project.layers) {
				if (l.parent == raw.parent && SlotLayer.isMask(l)) {
					return true;
				}
			}
		}
		return false;
	}

	static getOpacity = (raw: SlotLayerRaw): f32 => {
		let f = raw.maskOpacity;
		if (SlotLayer.isLayer(raw) && raw.parent != null) f *= raw.parent.maskOpacity;
		return f;
	}

	static getObjectMask = (raw: SlotLayerRaw): i32 => {
		return SlotLayer.isMask(raw) ? raw.parent.objectMask : raw.objectMask;
	}

	static isLayer = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor != null;
		///end
		///if is_sculpt
		return raw.texpaint != null;
		///end
	}

	static isGroup = (raw: SlotLayerRaw): bool => {
		return raw.texpaint == null;
	}

	static getContainingGroup = (raw: SlotLayerRaw): SlotLayerRaw => {
		if (raw.parent != null && SlotLayer.isGroup(raw.parent))
			return raw.parent;
		else if (raw.parent != null && raw.parent.parent != null && SlotLayer.isGroup(raw.parent.parent))
			return raw.parent.parent;
		else return null;
	}

	static isMask = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor == null;
		///end
		///if is_sculpt
		return false;
		///end
	}

	static isGroupMask = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor == null && SlotLayer.isGroup(raw.parent);
		///end
		///if is_sculpt
		return false;
		///end
	}

	static isLayerMask = (raw: SlotLayerRaw): bool => {
		///if is_paint
		return raw.texpaint != null && raw.texpaint_nor == null && SlotLayer.isLayer(raw.parent);
		///end
		///if is_sculpt
		return false;
		///end
	}

	static isInGroup = (raw: SlotLayerRaw): bool => {
		return raw.parent != null && (SlotLayer.isGroup(raw.parent) || (raw.parent.parent != null && SlotLayer.isGroup(raw.parent.parent)));
	}

	static canMove = (raw: SlotLayerRaw, to: i32): bool => {
		let oldIndex = Project.layers.indexOf(raw);

		let delta = to - oldIndex; // If delta > 0 the layer is moved up, otherwise down
		if (to < 0 || to > Project.layers.length - 1 || delta == 0) return false;

		// If the layer is moved up, all layers between the old position and the new one move one down.
		// The layers above the new position stay where they are.
		// If the new position is on top or on bottom no upper resp. lower layer exists.
		let newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			let children = SlotLayer.getRecursiveChildren(newUpperLayer);
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		let newLowerLayer = delta > 0 ? Project.layers[to] : (to > 0 ? Project.layers[to - 1] : null);

		if (SlotLayer.isMask(raw)) {
			// Masks can not be on top.
			if (newUpperLayer == null) return false;
			// Masks should not be placed below a collapsed group. This condition can be savely removed.
			if (SlotLayer.isInGroup(newUpperLayer) && !SlotLayer.getContainingGroup(newUpperLayer).show_panel) return false;
			// Masks should not be placed below a collapsed layer. This condition can be savely removed.
			if (SlotLayer.isMask(newUpperLayer) && !newUpperLayer.parent.show_panel) return false;
		}

		if (SlotLayer.isLayer(raw)) {
			// Layers can not be moved directly below its own mask(s).
			if (newUpperLayer != null && SlotLayer.isMask(newUpperLayer) && newUpperLayer.parent == raw) return false;
			// Layers can not be placed above a mask as the mask would be reparented.
			if (newLowerLayer != null && SlotLayer.isMask(newLowerLayer)) return false;
		}

		// Currently groups can not be nested. Thus valid positions for groups are:
		if (SlotLayer.isGroup(raw)) {
			// At the top.
			if (newUpperLayer == null) return true;
			// NOT below its own children.
			if (SlotLayer.getContainingGroup(newUpperLayer) == raw) return false;
			// At the bottom.
			if (newLowerLayer == null) return true;
			// Above a group.
			if (SlotLayer.isGroup(newLowerLayer)) return true;
			// Above a non-grouped layer.
			if (SlotLayer.isLayer(newLowerLayer) && !SlotLayer.isInGroup(newLowerLayer)) return true;
			else return false;
		}

		return true;
	}

	static move = (raw: SlotLayerRaw, to: i32) => {
		if (!SlotLayer.canMove(raw, to)) {
			return;
		}

		let pointers = TabLayers.initLayerMap();
		let oldIndex = Project.layers.indexOf(raw);
		let delta = to - oldIndex;
		let newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			let children = SlotLayer.getRecursiveChildren(newUpperLayer);
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		Context.setLayer(raw);
		History.orderLayers(to);
		UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;

		array_remove(Project.layers, raw);
		Project.layers.splice(to, 0, raw);

		if (SlotLayer.isLayer(raw)) {
			let oldParent = raw.parent;

			if (newUpperLayer == null)
				raw.parent = null; // Placed on top.
			else if (SlotLayer.isInGroup(newUpperLayer) && !SlotLayer.getContainingGroup(newUpperLayer).show_panel)
				raw.parent = null; // Placed below a collapsed group.
			else if (SlotLayer.isLayer(newUpperLayer))
				raw.parent = newUpperLayer.parent; // Placed below a layer, use the same parent.
			else if (SlotLayer.isGroup(newUpperLayer))
				raw.parent = newUpperLayer; // Placed as top layer in a group.
			else if (SlotLayer.isGroupMask(newUpperLayer))
				raw.parent = newUpperLayer.parent; // Placed in a group below the lowest group mask.
			else if (SlotLayer.isLayerMask(newUpperLayer))
				raw.parent = SlotLayer.getContainingGroup(newUpperLayer); // Either the group the mask belongs to or null.

			// Layers can have masks as children. These have to be moved, too.
			let layerMasks = SlotLayer.getMasks(raw, false);
			if (layerMasks != null) {
				for (let idx = 0; idx < layerMasks.length; ++idx) {
					let mask = layerMasks[idx];
					array_remove(Project.layers, mask);
					// If the masks are moved down each step increases the index below the layer by one.
					Project.layers.splice(delta > 0 ? oldIndex + delta - 1 : oldIndex + delta + idx, 0, mask);
				}
			}

			// The layer is the last layer in the group, remove it. Notice that this might remove group masks.
			if (oldParent != null && SlotLayer.getChildren(oldParent) == null)
				SlotLayer.delete(oldParent);
		}
		else if (SlotLayer.isMask(raw)) {
			// Precondition newUpperLayer != null, ensured in canMove.
			if (SlotLayer.isLayer(newUpperLayer) || SlotLayer.isGroup(newUpperLayer))
				raw.parent = newUpperLayer;
			else if (SlotLayer.isMask(newUpperLayer)) { // Group mask or layer mask.
				raw.parent = newUpperLayer.parent;
			}
		}
		else if (SlotLayer.isGroup(raw)) {
			let children = SlotLayer.getRecursiveChildren(raw);
			if (children != null) {
				for (let idx = 0; idx < children.length; ++idx) {
					let child = children[idx];
					array_remove(Project.layers, child);
					// If the children are moved down each step increases the index below the layer by one.
					Project.layers.splice(delta > 0 ? oldIndex + delta - 1 : oldIndex + delta + idx, 0, child);
				}
			}
		}

		for (let m of Project.materials) TabLayers.remapLayerPointers(m.canvas.nodes, TabLayers.fillLayerMap(pointers));
	}
}
