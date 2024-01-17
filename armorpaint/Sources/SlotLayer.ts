
class SlotLayer {
	id = 0;
	name: string;
	ext = "";
	visible = true;
	parent: SlotLayer = null; // Group (for layers) or layer (for masks)

	texpaint: Image = null; // Base or mask
	///if is_paint
	texpaint_nor: Image = null;
	texpaint_pack: Image = null;
	texpaint_preview: Image = null; // Layer preview
	///end

	maskOpacity = 1.0; // Opacity mask
	fill_layer: SlotMaterial = null;
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
	decalMat = Mat4.identity(); // Decal layer

	constructor(ext = "", type = LayerSlotType.SlotLayer, parent: SlotLayer = null) {
		if (ext == "") {
			this.id = 0;
			for (let l of Project.layers) if (l.id >= this.id) this.id = l.id + 1;
			ext = this.id + "";
		}
		this.ext = ext;
		this.parent = parent;

		if (type == LayerSlotType.SlotGroup) {
			this.name = "Group " + (this.id + 1);
		}
		else if (type == LayerSlotType.SlotLayer) {
			this.name = "Layer " + (this.id + 1);
			///if is_paint
			let format = Base.bitsHandle.position == TextureBits.Bits8  ? "RGBA32" :
						 Base.bitsHandle.position == TextureBits.Bits16 ? "RGBA64" :
						 									  			  "RGBA128";
			///end

			///if is_sculpt
			let format = "RGBA128";
			///end

			{
				let t = new RenderTargetRaw();
				t.name = "texpaint" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				this.texpaint = RenderPath.active.createRenderTarget(t).image;
			}

			///if is_paint
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_nor" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				this.texpaint_nor = RenderPath.active.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_pack" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				this.texpaint_pack = RenderPath.active.createRenderTarget(t).image;
			}

			this.texpaint_preview = Image.createRenderTarget(UtilRender.layerPreviewSize, UtilRender.layerPreviewSize, TextureFormat.RGBA32);
			///end
		}

		///if is_paint
		else { // Mask
			this.name = "Mask " + (this.id + 1);
			let format = "RGBA32"; // Full bits for undo support, R8 is used
			this.blending = BlendType.BlendAdd;

			{
				let t = new RenderTargetRaw();
				t.name = "texpaint" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				this.texpaint = RenderPath.active.createRenderTarget(t).image;
			}

			this.texpaint_preview = Image.createRenderTarget(UtilRender.layerPreviewSize, UtilRender.layerPreviewSize, TextureFormat.RGBA32);
		}
		///end
	}

	delete = () => {
		this.unload();

		if (this.isLayer()) {
			let masks = this.getMasks(false); // Prevents deleting group masks
			if (masks != null) for (let m of masks) m.delete();
		}
		else if (this.isGroup()) {
			let children = this.getChildren();
			if (children != null) for (let c of children) c.delete();
			let masks = this.getMasks();
			if (masks != null) for (let m of masks) m.delete();
		}

		let lpos = Project.layers.indexOf(this);
		array_remove(Project.layers, this);
		// Undo can remove base layer and then restore it from undo layers
		if (Project.layers.length > 0) {
			Context.setLayer(Project.layers[lpos > 0 ? lpos - 1 : 0]);
		}

		// Do not remove empty groups if the last layer is deleted as this prevents redo from working properly
	}

	unload = () => {
		if (this.isGroup()) return;

		let _texpaint = this.texpaint;
		///if is_paint
		let _texpaint_nor = this.texpaint_nor;
		let _texpaint_pack = this.texpaint_pack;
		let _texpaint_preview = this.texpaint_preview;
		///end

		let _next = () => {
			_texpaint.unload();
			///if is_paint
			if (_texpaint_nor != null) _texpaint_nor.unload();
			if (_texpaint_pack != null) _texpaint_pack.unload();
			_texpaint_preview.unload();
			///end
		}
		Base.notifyOnNextFrame(_next);

		RenderPath.active.renderTargets.delete("texpaint" + this.ext);
		///if is_paint
		if (this.isLayer()) {
			RenderPath.active.renderTargets.delete("texpaint_nor" + this.ext);
			RenderPath.active.renderTargets.delete("texpaint_pack" + this.ext);
		}
		///end
	}

	swap = (other: SlotLayer) => {
		if ((this.isLayer() || this.isMask()) && (other.isLayer() || other.isMask())) {
			RenderPath.active.renderTargets.get("texpaint" + this.ext).image = other.texpaint;
			RenderPath.active.renderTargets.get("texpaint" + other.ext).image = this.texpaint;
			let _texpaint = this.texpaint;
			this.texpaint = other.texpaint;
			other.texpaint = _texpaint;

			///if is_paint
			let _texpaint_preview = this.texpaint_preview;
			this.texpaint_preview = other.texpaint_preview;
			other.texpaint_preview = _texpaint_preview;
			///end
		}

		///if is_paint
		if (this.isLayer() && other.isLayer()) {
			RenderPath.active.renderTargets.get("texpaint_nor" + this.ext).image = other.texpaint_nor;
			RenderPath.active.renderTargets.get("texpaint_pack" + this.ext).image = other.texpaint_pack;
			RenderPath.active.renderTargets.get("texpaint_nor" + other.ext).image = this.texpaint_nor;
			RenderPath.active.renderTargets.get("texpaint_pack" + other.ext).image = this.texpaint_pack;
			let _texpaint_nor = this.texpaint_nor;
			let _texpaint_pack = this.texpaint_pack;
			this.texpaint_nor = other.texpaint_nor;
			this.texpaint_pack = other.texpaint_pack;
			other.texpaint_nor = _texpaint_nor;
			other.texpaint_pack = _texpaint_pack;
		}
		///end
	}

	clear = (baseColor = 0x00000000, baseImage: Image = null, occlusion = 1.0, roughness = Base.defaultRough, metallic = 0.0) => {
		this.texpaint.g4.begin();
		this.texpaint.g4.clear(baseColor); // Base
		this.texpaint.g4.end();
		if (baseImage != null) {
			this.texpaint.g2.begin(false);
			this.texpaint.g2.drawScaledImage(baseImage, 0, 0, this.texpaint.width, this.texpaint.height);
			this.texpaint.g2.end();
		}

		///if is_paint
		if (this.isLayer()) {
			this.texpaint_nor.g4.begin();
			this.texpaint_nor.g4.clear(color_from_floats(0.5, 0.5, 1.0, 0.0)); // Nor
			this.texpaint_nor.g4.end();
			this.texpaint_pack.g4.begin();
			this.texpaint_pack.g4.clear(color_from_floats(occlusion, roughness, metallic, 0.0)); // Occ, rough, met
			this.texpaint_pack.g4.end();
		}
		///end

		Context.raw.layerPreviewDirty = true;
		Context.raw.ddirty = 3;
	}

	invertMask = () => {
		if (Base.pipeInvert8 == null) Base.makePipe();
		let inverted = Image.createRenderTarget(this.texpaint.width, this.texpaint.height, TextureFormat.RGBA32);
		inverted.g2.begin(false);
		inverted.g2.pipeline = Base.pipeInvert8;
		inverted.g2.drawImage(this.texpaint, 0, 0);
		inverted.g2.pipeline = null;
		inverted.g2.end();
		let _texpaint = this.texpaint;
		let _next = () => {
			_texpaint.unload();
		}
		Base.notifyOnNextFrame(_next);
		this.texpaint = RenderPath.active.renderTargets.get("texpaint" + this.id).image = inverted;
		Context.raw.layerPreviewDirty = true;
		Context.raw.ddirty = 3;
	}

	applyMask = () => {
		if (this.parent.fill_layer != null) {
			this.parent.toPaintLayer();
		}
		if (this.parent.isGroup()) {
			for (let c of this.parent.getChildren()) {
				Base.applyMask(c, this);
			}
		}
		else {
			Base.applyMask(this.parent, this);
		}
		this.delete();
	}

	duplicate = (): SlotLayer => {
		let layers = Project.layers;
		let i = layers.indexOf(this) + 1;
		let l = new SlotLayer("", this.isLayer() ? LayerSlotType.SlotLayer : this.isMask() ? LayerSlotType.SlotMask : LayerSlotType.SlotGroup, this.parent);
		layers.splice(i, 0, l);

		if (Base.pipeMerge == null) Base.makePipe();
		if (this.isLayer()) {
			l.texpaint.g2.begin(false);
			l.texpaint.g2.pipeline = Base.pipeCopy;
			l.texpaint.g2.drawImage(this.texpaint, 0, 0);
			l.texpaint.g2.pipeline = null;
			l.texpaint.g2.end();
			///if is_paint
			l.texpaint_nor.g2.begin(false);
			l.texpaint_nor.g2.pipeline = Base.pipeCopy;
			l.texpaint_nor.g2.drawImage(this.texpaint_nor, 0, 0);
			l.texpaint_nor.g2.pipeline = null;
			l.texpaint_nor.g2.end();
			l.texpaint_pack.g2.begin(false);
			l.texpaint_pack.g2.pipeline = Base.pipeCopy;
			l.texpaint_pack.g2.drawImage(this.texpaint_pack, 0, 0);
			l.texpaint_pack.g2.pipeline = null;
			l.texpaint_pack.g2.end();
			///end
		}
		else if (this.isMask()) {
			l.texpaint.g2.begin(false);
			l.texpaint.g2.pipeline = Base.pipeCopy8;
			l.texpaint.g2.drawImage(this.texpaint, 0, 0);
			l.texpaint.g2.pipeline = null;
			l.texpaint.g2.end();
		}

		///if is_paint
		l.texpaint_preview.g2.begin(true, 0x00000000);
		l.texpaint_preview.g2.pipeline = Base.pipeCopy;
		l.texpaint_preview.g2.drawScaledImage(this.texpaint_preview, 0, 0, this.texpaint_preview.width, this.texpaint_preview.height);
		l.texpaint_preview.g2.pipeline = null;
		l.texpaint_preview.g2.end();
		///end

		l.visible = this.visible;
		l.maskOpacity = this.maskOpacity;
		l.fill_layer = this.fill_layer;
		l.objectMask = this.objectMask;
		l.blending = this.blending;
		l.uvType = this.uvType;
		l.scale = this.scale;
		l.angle = this.angle;
		l.paintBase = this.paintBase;
		l.paintOpac = this.paintOpac;
		l.paintOcc = this.paintOcc;
		l.paintRough = this.paintRough;
		l.paintMet = this.paintMet;
		l.paintNor = this.paintNor;
		l.paintNorBlend = this.paintNorBlend;
		l.paintHeight = this.paintHeight;
		l.paintHeightBlend = this.paintHeightBlend;
		l.paintEmis = this.paintEmis;
		l.paintSubs = this.paintSubs;

		return l;
	}

	resizeAndSetBits = () => {
		let resX = Config.getTextureResX();
		let resY = Config.getTextureResY();
		let rts = RenderPath.active.renderTargets;
		if (Base.pipeMerge == null) Base.makePipe();

		if (this.isLayer()) {
			///if is_paint
			let format = Base.bitsHandle.position == TextureBits.Bits8  ? TextureFormat.RGBA32 :
						 Base.bitsHandle.position == TextureBits.Bits16 ? TextureFormat.RGBA64 :
						 									  			  TextureFormat.RGBA128;
			///end

			///if is_sculpt
			let format = TextureFormat.RGBA128;
			///end

			let _texpaint = this.texpaint;
			this.texpaint = Image.createRenderTarget(resX, resY, format);
			this.texpaint.g2.begin(false);
			this.texpaint.g2.pipeline = Base.pipeCopy;
			this.texpaint.g2.drawScaledImage(_texpaint, 0, 0, resX, resY);
			this.texpaint.g2.pipeline = null;
			this.texpaint.g2.end();

			///if is_paint
			let _texpaint_nor = this.texpaint_nor;
			let _texpaint_pack = this.texpaint_pack;
			this.texpaint_nor = Image.createRenderTarget(resX, resY, format);
			this.texpaint_pack = Image.createRenderTarget(resX, resY, format);

			this.texpaint_nor.g2.begin(false);
			this.texpaint_nor.g2.pipeline = Base.pipeCopy;
			this.texpaint_nor.g2.drawScaledImage(_texpaint_nor, 0, 0, resX, resY);
			this.texpaint_nor.g2.pipeline = null;
			this.texpaint_nor.g2.end();

			this.texpaint_pack.g2.begin(false);
			this.texpaint_pack.g2.pipeline = Base.pipeCopy;
			this.texpaint_pack.g2.drawScaledImage(_texpaint_pack, 0, 0, resX, resY);
			this.texpaint_pack.g2.pipeline = null;
			this.texpaint_pack.g2.end();
			///end

			let _next = () => {
				_texpaint.unload();
				///if is_paint
				_texpaint_nor.unload();
				_texpaint_pack.unload();
				///end
			}
			Base.notifyOnNextFrame(_next);

			rts.get("texpaint" + this.ext).image = this.texpaint;
			///if is_paint
			rts.get("texpaint_nor" + this.ext).image = this.texpaint_nor;
			rts.get("texpaint_pack" + this.ext).image = this.texpaint_pack;
			///end
		}
		else if (this.isMask()) {
			let _texpaint = this.texpaint;
			this.texpaint = Image.createRenderTarget(resX, resY, TextureFormat.RGBA32);

			this.texpaint.g2.begin(false);
			this.texpaint.g2.pipeline = Base.pipeCopy8;
			this.texpaint.g2.drawScaledImage(_texpaint, 0, 0, resX, resY);
			this.texpaint.g2.pipeline = null;
			this.texpaint.g2.end();

			let _next = () => {
				_texpaint.unload();
			}
			Base.notifyOnNextFrame(_next);

			rts.get("texpaint" + this.ext).image = this.texpaint;
		}
	}

	toFillLayer = () => {
		Context.setLayer(this);
		this.fill_layer = Context.raw.material;
		Base.updateFillLayer();
		let _next = () => {
			MakeMaterial.parsePaintMaterial();
			Context.raw.layerPreviewDirty = true;
			UIBase.inst.hwnds[TabArea.TabSidebar0].redraws = 2;
		}
		Base.notifyOnNextFrame(_next);
	}

	toPaintLayer = () => {
		Context.setLayer(this);
		this.fill_layer = null;
		MakeMaterial.parsePaintMaterial();
		Context.raw.layerPreviewDirty = true;
		UIBase.inst.hwnds[TabArea.TabSidebar0].redraws = 2;
	}

	isVisible = (): bool => {
		return this.visible && (this.parent == null || this.parent.visible);
	}

	getChildren = (): SlotLayer[] => {
		let children: SlotLayer[] = null; // Child layers of a group
		for (let l of Project.layers) {
			if (l.parent == this && l.isLayer()) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	getRecursiveChildren = (): SlotLayer[] => {
		let children: SlotLayer[] = null;
		for (let l of Project.layers) {
			if (l.parent == this) { // Child layers and group masks
				if (children == null) children = [];
				children.push(l);
			}
			if (l.parent != null && l.parent.parent == this) { // Layer masks
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	getMasks = (includeGroupMasks = true): SlotLayer[] => {
		if (this.isMask()) return null;

		let children: SlotLayer[] = null;
		// Child masks of a layer
		for (let l of Project.layers) {
			if (l.parent == this && l.isMask()) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		// Child masks of a parent group
		if (includeGroupMasks) {
			if (this.parent != null && this.parent.isGroup()) {
				for (let l of Project.layers) {
					if (l.parent == this.parent && l.isMask()) {
						if (children == null) children = [];
						children.push(l);
					}
				}
			}
		}
		return children;
	}

	hasMasks = (includeGroupMasks = true): bool => {
		// Layer mask
		for (let l of Project.layers) {
			if (l.parent == this && l.isMask()) {
				return true;
			}
		}
		// Group mask
		if (includeGroupMasks && this.parent != null && this.parent.isGroup()) {
			for (let l of Project.layers) {
				if (l.parent == this.parent && l.isMask()) {
					return true;
				}
			}
		}
		return false;
	}

	getOpacity = (): f32 => {
		let f = this.maskOpacity;
		if (this.isLayer() && this.parent != null) f *= this.parent.maskOpacity;
		return f;
	}

	getObjectMask = (): i32 => {
		return this.isMask() ? this.parent.objectMask : this.objectMask;
	}

	isLayer = (): bool => {
		///if is_paint
		return this.texpaint != null && this.texpaint_nor != null;
		///end
		///if is_sculpt
		return this.texpaint != null;
		///end
	}

	isGroup = (): bool => {
		return this.texpaint == null;
	}

	getContainingGroup = (): SlotLayer => {
		if (this.parent != null && this.parent.isGroup())
			return this.parent;
		else if (this.parent != null && this.parent.parent != null && this.parent.parent.isGroup())
			return this.parent.parent;
		else return null;
	}

	isMask = (): bool => {
		///if is_paint
		return this.texpaint != null && this.texpaint_nor == null;
		///end
		///if is_sculpt
		return false;
		///end
	}

	isGroupMask = (): bool => {
		///if is_paint
		return this.texpaint != null && this.texpaint_nor == null && this.parent.isGroup();
		///end
		///if is_sculpt
		return false;
		///end
	}

	isLayerMask = (): bool => {
		///if is_paint
		return this.texpaint != null && this.texpaint_nor == null && this.parent.isLayer();
		///end
		///if is_sculpt
		return false;
		///end
	}

	isInGroup = (): bool => {
		return this.parent != null && (this.parent.isGroup() || (this.parent.parent != null && this.parent.parent.isGroup()));
	}

	canMove = (to: i32): bool => {
		let oldIndex = Project.layers.indexOf(this);

		let delta = to - oldIndex; // If delta > 0 the layer is moved up, otherwise down
		if (to < 0 || to > Project.layers.length - 1 || delta == 0) return false;

		// If the layer is moved up, all layers between the old position and the new one move one down.
		// The layers above the new position stay where they are.
		// If the new position is on top or on bottom no upper resp. lower layer exists.
		let newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			let children = newUpperLayer.getRecursiveChildren();
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		let newLowerLayer = delta > 0 ? Project.layers[to] : (to > 0 ? Project.layers[to - 1] : null);

		if (this.isMask()) {
			// Masks can not be on top.
			if (newUpperLayer == null) return false;
			// Masks should not be placed below a collapsed group. This condition can be savely removed.
			if (newUpperLayer.isInGroup() && !newUpperLayer.getContainingGroup().show_panel) return false;
			// Masks should not be placed below a collapsed layer. This condition can be savely removed.
			if (newUpperLayer.isMask() && !newUpperLayer.parent.show_panel) return false;
		}

		if (this.isLayer()) {
			// Layers can not be moved directly below its own mask(s).
			if (newUpperLayer != null && newUpperLayer.isMask() && newUpperLayer.parent == this) return false;
			// Layers can not be placed above a mask as the mask would be reparented.
			if (newLowerLayer != null && newLowerLayer.isMask()) return false;
		}

		// Currently groups can not be nested. Thus valid positions for groups are:
		if (this.isGroup()) {
			// At the top.
			if (newUpperLayer == null) return true;
			// NOT below its own children.
			if (newUpperLayer.getContainingGroup() == this) return false;
			// At the bottom.
			if (newLowerLayer == null) return true;
			// Above a group.
			if (newLowerLayer.isGroup()) return true;
			// Above a non-grouped layer.
			if (newLowerLayer.isLayer() && !newLowerLayer.isInGroup()) return true;
			else return false;
		}

		return true;
	}

	move = (to: i32) => {
		if (!this.canMove(to)) {
			return;
		}

		let pointers = TabLayers.initLayerMap();
		let oldIndex = Project.layers.indexOf(this);
		let delta = to - oldIndex;
		let newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			let children = newUpperLayer.getRecursiveChildren();
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		Context.setLayer(this);
		History.orderLayers(to);
		UIBase.inst.hwnds[TabArea.TabSidebar0].redraws = 2;

		array_remove(Project.layers, this);
		Project.layers.splice(to, 0, this);

		if (this.isLayer()) {
			let oldParent = this.parent;

			if (newUpperLayer == null)
				this.parent = null; // Placed on top.
			else if (newUpperLayer.isInGroup() && !newUpperLayer.getContainingGroup().show_panel)
				this.parent = null; // Placed below a collapsed group.
			else if (newUpperLayer.isLayer())
				this.parent = newUpperLayer.parent; // Placed below a layer, use the same parent.
			else if (newUpperLayer.isGroup())
				this.parent = newUpperLayer; // Placed as top layer in a group.
			else if (newUpperLayer.isGroupMask())
				this.parent = newUpperLayer.parent; // Placed in a group below the lowest group mask.
			else if (newUpperLayer.isLayerMask())
				this.parent = newUpperLayer.getContainingGroup(); // Either the group the mask belongs to or null.

			// Layers can have masks as children. These have to be moved, too.
			let layerMasks = this.getMasks(false);
			if (layerMasks != null) {
				for (let idx = 0; idx < layerMasks.length; ++idx) {
					let mask = layerMasks[idx];
					array_remove(Project.layers, mask);
					// If the masks are moved down each step increases the index below the layer by one.
					Project.layers.splice(delta > 0 ? oldIndex + delta - 1 : oldIndex + delta + idx, 0, mask);
				}
			}

			// The layer is the last layer in the group, remove it. Notice that this might remove group masks.
			if (oldParent != null && oldParent.getChildren() == null)
				oldParent.delete();
		}
		else if (this.isMask()) {
			// Precondition newUpperLayer != null, ensured in canMove.
			if (newUpperLayer.isLayer() || newUpperLayer.isGroup())
				this.parent = newUpperLayer;
			else if (newUpperLayer.isMask()) { // Group mask or layer mask.
				this.parent = newUpperLayer.parent;
			}
		}
		else if (this.isGroup()) {
			let children = this.getRecursiveChildren();
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
