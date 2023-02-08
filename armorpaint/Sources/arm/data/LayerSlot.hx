package arm.data;

import kha.graphics4.TextureFormat;
import kha.Image;
import iron.RenderPath;
import arm.ui.UISidebar;
import arm.ui.TabLayers;
import arm.util.RenderUtil;
import arm.node.MakeMaterial;
import arm.Enums;

class LayerSlot {
	public var id = 0;
	public var name: String;
	public var ext = "";
	public var visible = true;
	public var parent: LayerSlot = null; // Group (for layers) or layer (for masks)

	public var texpaint: Image = null; // Base or mask
	public var texpaint_nor: Image = null;
	public var texpaint_pack: Image = null;
	public var texpaint_preview: Image = null; // Layer preview

	public var maskOpacity = 1.0; // Opacity mask
	public var fill_layer: MaterialSlot = null;
	public var show_panel = true;
	public var blending = BlendMix;
	public var objectMask = 0;
	public var scale = 1.0;
	public var angle = 0.0;
	public var uvType = UVMap;
	public var paintBase = true;
	public var paintOpac = true;
	public var paintOcc = true;
	public var paintRough = true;
	public var paintMet = true;
	public var paintNor = true;
	public var paintNorBlend = true;
	public var paintHeight = true;
	public var paintHeightBlend = true;
	public var paintEmis = true;
	public var paintSubs = true;
	public var decalMat = iron.math.Mat4.identity(); // Decal layer

	public function new(ext = "", type = SlotLayer, parent: LayerSlot = null) {
		if (ext == "") {
			id = 0;
			for (l in Project.layers) if (l.id >= id) id = l.id + 1;
			ext = id + "";
		}
		this.ext = ext;
		this.parent = parent;

		if (type == SlotGroup) {
			name = "Group " + (id + 1);
		}
		else if (type == SlotLayer) {
			name = "Layer " + (id + 1);
			var format = App.bitsHandle.position == Bits8  ? "RGBA32" :
						 App.bitsHandle.position == Bits16 ? "RGBA64" :
						 									 "RGBA128";

			{
				var t = new RenderTargetRaw();
				t.name = "texpaint" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				texpaint = RenderPath.active.createRenderTarget(t).image;
			}
			{
				var t = new RenderTargetRaw();
				t.name = "texpaint_nor" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				texpaint_nor = RenderPath.active.createRenderTarget(t).image;
			}
			{
				var t = new RenderTargetRaw();
				t.name = "texpaint_pack" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				texpaint_pack = RenderPath.active.createRenderTarget(t).image;
			}

			texpaint_preview = Image.createRenderTarget(RenderUtil.layerPreviewSize, RenderUtil.layerPreviewSize, TextureFormat.RGBA32);
		}
		else { // Mask
			name = "Mask " + (id + 1);
			var format = "RGBA32"; // Full bits for undo support, R8 is used
			blending = BlendAdd;

			{
				var t = new RenderTargetRaw();
				t.name = "texpaint" + ext;
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = format;
				texpaint = RenderPath.active.createRenderTarget(t).image;
			}

			texpaint_preview = Image.createRenderTarget(RenderUtil.layerPreviewSize, RenderUtil.layerPreviewSize, TextureFormat.RGBA32);
		}
	}

	public function delete() {
		unload();

		if (isLayer()) {
			var masks = getMasks(false); // Prevents deleting group masks
			if (masks != null) for (m in masks) m.delete();
		}
		else if (isGroup()) {
			var children = getChildren();
			if (children != null) for (c in children) c.delete();
			var masks = getMasks();
			if (masks != null) for (m in masks) m.delete();
		}

		var lpos = Project.layers.indexOf(this);
		Project.layers.remove(this);
		// Undo can remove base layer and then restore it from undo layers
		if (Project.layers.length > 0) {
			Context.setLayer(Project.layers[lpos > 0 ? lpos - 1 : 0]);
		}

		// Do not remove empty groups if the last layer is deleted as this prevents redo from working properly
	}

	public function unload() {
		if (isGroup()) return;

		var _texpaint = texpaint;
		var _texpaint_nor = texpaint_nor;
		var _texpaint_pack = texpaint_pack;
		var _texpaint_preview = texpaint_preview;
		function _next() {
			_texpaint.unload();
			if (_texpaint_nor != null) _texpaint_nor.unload();
			if (_texpaint_pack != null) _texpaint_pack.unload();
			_texpaint_preview.unload();
		}
		App.notifyOnNextFrame(_next);

		RenderPath.active.renderTargets.remove("texpaint" + ext);
		if (isLayer()) {
			RenderPath.active.renderTargets.remove("texpaint_nor" + ext);
			RenderPath.active.renderTargets.remove("texpaint_pack" + ext);
		}
	}

	public function swap(other: LayerSlot) {
		if ((isLayer() || isMask()) && (other.isLayer() || other.isMask())) {
			RenderPath.active.renderTargets.get("texpaint" + ext).image = other.texpaint;
			RenderPath.active.renderTargets.get("texpaint" + other.ext).image = texpaint;
			var _texpaint = texpaint;
			texpaint = other.texpaint;
			other.texpaint = _texpaint;
			var _texpaint_preview = texpaint_preview;
			texpaint_preview = other.texpaint_preview;
			other.texpaint_preview = _texpaint_preview;
		}

		if (isLayer() && other.isLayer()) {
			RenderPath.active.renderTargets.get("texpaint_nor" + ext).image = other.texpaint_nor;
			RenderPath.active.renderTargets.get("texpaint_pack" + ext).image = other.texpaint_pack;
			RenderPath.active.renderTargets.get("texpaint_nor" + other.ext).image = texpaint_nor;
			RenderPath.active.renderTargets.get("texpaint_pack" + other.ext).image = texpaint_pack;
			var _texpaint_nor = texpaint_nor;
			var _texpaint_pack = texpaint_pack;
			texpaint_nor = other.texpaint_nor;
			texpaint_pack = other.texpaint_pack;
			other.texpaint_nor = _texpaint_nor;
			other.texpaint_pack = _texpaint_pack;
		}
	}

	public function clear(baseColor = 0x00000000, baseImage: kha.Image = null, occlusion = 1.0, roughness = Layers.defaultRough, metallic = 0.0) {
		texpaint.g4.begin();
		texpaint.g4.clear(baseColor); // Base
		texpaint.g4.end();
		if (baseImage != null) {
			texpaint.g2.begin(false);
			texpaint.g2.drawScaledImage(baseImage, 0, 0, texpaint.width, texpaint.height);
			texpaint.g2.end();
		}

		if (isLayer()) {
			texpaint_nor.g4.begin();
			texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
			texpaint_nor.g4.end();
			texpaint_pack.g4.begin();
			texpaint_pack.g4.clear(kha.Color.fromFloats(occlusion, roughness, metallic, 0.0)); // Occ, rough, met
			texpaint_pack.g4.end();
		}

		Context.layerPreviewDirty = true;
		Context.ddirty = 3;
	}

	public function invertMask() {
		if (Layers.pipeInvert8 == null) Layers.makePipe();
		var inverted = Image.createRenderTarget(texpaint.width, texpaint.height, TextureFormat.RGBA32);
		inverted.g2.begin(false);
		inverted.g2.pipeline = Layers.pipeInvert8;
		inverted.g2.drawImage(texpaint, 0, 0);
		inverted.g2.pipeline = null;
		inverted.g2.end();
		var _texpaint = texpaint;
		function _next() {
			_texpaint.unload();
		}
		App.notifyOnNextFrame(_next);
		texpaint = RenderPath.active.renderTargets.get("texpaint" + id).image = inverted;
		Context.layerPreviewDirty = true;
		Context.ddirty = 3;
	}

	public function applyMask() {
		if (parent.fill_layer != null) {
			parent.toPaintLayer();
		}
		if (parent.isGroup()) {
			for (c in parent.getChildren()) {
				Layers.applyMask(c, this);
			}
		}
		else {
			Layers.applyMask(parent, this);
		}
		delete();
	}

	public function duplicate(): LayerSlot {
		var layers = Project.layers;
		var i = layers.indexOf(this) + 1;
		var l = new LayerSlot("", isLayer() ? SlotLayer : isMask() ? SlotMask : SlotGroup, parent);
		layers.insert(i, l);

		if (Layers.pipeMerge == null) Layers.makePipe();
		if (isLayer()) {
			l.texpaint.g2.begin(false);
			l.texpaint.g2.pipeline = Layers.pipeCopy;
			l.texpaint.g2.drawImage(texpaint, 0, 0);
			l.texpaint.g2.pipeline = null;
			l.texpaint.g2.end();
			l.texpaint_nor.g2.begin(false);
			l.texpaint_nor.g2.pipeline = Layers.pipeCopy;
			l.texpaint_nor.g2.drawImage(texpaint_nor, 0, 0);
			l.texpaint_nor.g2.pipeline = null;
			l.texpaint_nor.g2.end();
			l.texpaint_pack.g2.begin(false);
			l.texpaint_pack.g2.pipeline = Layers.pipeCopy;
			l.texpaint_pack.g2.drawImage(texpaint_pack, 0, 0);
			l.texpaint_pack.g2.pipeline = null;
			l.texpaint_pack.g2.end();
		}
		else if (isMask()) {
			l.texpaint.g2.begin(false);
			l.texpaint.g2.pipeline = Layers.pipeCopy8;
			l.texpaint.g2.drawImage(texpaint, 0, 0);
			l.texpaint.g2.pipeline = null;
			l.texpaint.g2.end();
		}

		l.texpaint_preview.g2.begin(true, 0x00000000);
		l.texpaint_preview.g2.pipeline = Layers.pipeCopy;
		l.texpaint_preview.g2.drawScaledImage(texpaint_preview, 0, 0, texpaint_preview.width, texpaint_preview.height);
		l.texpaint_preview.g2.pipeline = null;
		l.texpaint_preview.g2.end();

		l.visible = visible;
		l.maskOpacity = maskOpacity;
		l.fill_layer = fill_layer;
		l.objectMask = objectMask;
		l.blending = blending;
		l.uvType = uvType;
		l.scale = scale;
		l.angle = angle;
		l.paintBase = paintBase;
		l.paintOpac = paintOpac;
		l.paintOcc = paintOcc;
		l.paintRough = paintRough;
		l.paintMet = paintMet;
		l.paintNor = paintNor;
		l.paintNorBlend = paintNorBlend;
		l.paintHeight = paintHeight;
		l.paintHeightBlend = paintHeightBlend;
		l.paintEmis = paintEmis;
		l.paintSubs = paintSubs;

		return l;
	}

	public function resizeAndSetBits() {
		var resX = Config.getTextureResX();
		var resY = Config.getTextureResY();
		var rts = RenderPath.active.renderTargets;
		if (Layers.pipeMerge == null) Layers.makePipe();

		if (isLayer()) {
			var format = App.bitsHandle.position == Bits8  ? TextureFormat.RGBA32 :
						 App.bitsHandle.position == Bits16 ? TextureFormat.RGBA64 :
						 									 TextureFormat.RGBA128;

			var _texpaint = this.texpaint;
			var _texpaint_nor = this.texpaint_nor;
			var _texpaint_pack = this.texpaint_pack;

			this.texpaint = Image.createRenderTarget(resX, resY, format);
			this.texpaint_nor = Image.createRenderTarget(resX, resY, format);
			this.texpaint_pack = Image.createRenderTarget(resX, resY, format);

			this.texpaint.g2.begin(false);
			this.texpaint.g2.pipeline = Layers.pipeCopy;
			this.texpaint.g2.drawScaledImage(_texpaint, 0, 0, resX, resY);
			this.texpaint.g2.pipeline = null;
			this.texpaint.g2.end();

			this.texpaint_nor.g2.begin(false);
			this.texpaint_nor.g2.pipeline = Layers.pipeCopy;
			this.texpaint_nor.g2.drawScaledImage(_texpaint_nor, 0, 0, resX, resY);
			this.texpaint_nor.g2.pipeline = null;
			this.texpaint_nor.g2.end();

			this.texpaint_pack.g2.begin(false);
			this.texpaint_pack.g2.pipeline = Layers.pipeCopy;
			this.texpaint_pack.g2.drawScaledImage(_texpaint_pack, 0, 0, resX, resY);
			this.texpaint_pack.g2.pipeline = null;
			this.texpaint_pack.g2.end();

			function _next() {
				_texpaint.unload();
				_texpaint_nor.unload();
				_texpaint_pack.unload();
			}
			App.notifyOnNextFrame(_next);

			rts.get("texpaint" + this.ext).image = this.texpaint;
			rts.get("texpaint_nor" + this.ext).image = this.texpaint_nor;
			rts.get("texpaint_pack" + this.ext).image = this.texpaint_pack;
		}
		else if (isMask()) {
			var _texpaint = this.texpaint;
			this.texpaint = Image.createRenderTarget(resX, resY, TextureFormat.RGBA32);

			this.texpaint.g2.begin(false);
			this.texpaint.g2.pipeline = Layers.pipeCopy8;
			this.texpaint.g2.drawScaledImage(_texpaint, 0, 0, resX, resY);
			this.texpaint.g2.pipeline = null;
			this.texpaint.g2.end();

			function _next() {
				_texpaint.unload();
			}
			App.notifyOnNextFrame(_next);

			rts.get("texpaint" + this.ext).image = this.texpaint;
		}
	}

	public function toFillLayer() {
		Context.setLayer(this);
		fill_layer = Context.material;
		Layers.updateFillLayer();
		function _next() {
			MakeMaterial.parsePaintMaterial();
			Context.layerPreviewDirty = true;
			UISidebar.inst.hwnd0.redraws = 2;
		}
		App.notifyOnNextFrame(_next);
	}

	public function toPaintLayer() {
		Context.setLayer(this);
		fill_layer = null;
		MakeMaterial.parsePaintMaterial();
		Context.layerPreviewDirty = true;
		UISidebar.inst.hwnd0.redraws = 2;
	}

	public function isVisible(): Bool {
		return visible && (parent == null || parent.visible);
	}

	public function getChildren(): Array<LayerSlot> {
		var children: Array<LayerSlot> = null; // Child layers of a group
		for (l in Project.layers) {
			if (l.parent == this && l.isLayer()) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	public function getRecursiveChildren(): Array<LayerSlot> {
		var children: Array<LayerSlot> = null;
		for (l in Project.layers) {
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

	public function getMasks(includeGroupMasks = true): Array<LayerSlot> {
		if (this.isMask()) return null;

		var children: Array<LayerSlot> = null;
		// Child masks of a layer
		for (l in Project.layers) {
			if (l.parent == this && l.isMask()) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		// Child masks of a parent group
		if (includeGroupMasks) {
			if (this.parent != null && this.parent.isGroup()) {
				for (l in Project.layers) {
					if (l.parent == this.parent && l.isMask()) {
						if (children == null) children = [];
						children.push(l);
					}
				}
			}
		}
		return children;
	}

	public function hasMasks(includeGroupMasks = true): Bool {
		// Layer mask
		for (l in Project.layers) {
			if (l.parent == this && l.isMask()) {
				return true;
			}
		}
		// Group mask
		if (includeGroupMasks && this.parent != null && this.parent.isGroup()) {
			for (l in Project.layers) {
				if (l.parent == this.parent && l.isMask()) {
					return true;
				}
			}
		}
		return false;
	}

	public function getOpacity(): Float {
		var f = maskOpacity;
		if (isLayer() && parent != null) f *= parent.maskOpacity;
		return f;
	}

	public function getObjectMask(): Int {
		return isMask() ? parent.objectMask : objectMask;
	}

	public function isLayer(): Bool {
		return texpaint != null && texpaint_nor != null;
	}

	public function isGroup(): Bool {
		return texpaint == null;
	}

	public function getContainingGroup(): LayerSlot {
		if (parent != null && parent.isGroup())
			return parent;
		else if (parent != null && parent.parent != null && parent.parent.isGroup())
			return parent.parent;
		else return null;
	}

	public function isMask(): Bool {
		return texpaint != null && texpaint_nor == null;
	}

	public function isGroupMask(): Bool {
		return texpaint != null && texpaint_nor == null && parent.isGroup();
	}

	public function isLayerMask(): Bool {
		return texpaint != null && texpaint_nor == null && parent.isLayer();
	}

	public function isInGroup(): Bool {
		return parent != null && (parent.isGroup() || (parent.parent != null && parent.parent.isGroup()));
	}

	public function canMove(to: Int): Bool {
		var oldIndex = Project.layers.indexOf(this);

		var delta = to - oldIndex; // If delta > 0 the layer is moved up, otherwise down
		if (to < 0 || to > Project.layers.length - 1 || delta == 0) return false;

		// If the layer is moved up, all layers between the old position and the new one move one down.
		// The layers above the new position stay where they are.
		// If the new position is on top or on bottom no upper resp. lower layer exists.
		var newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			var children = newUpperLayer.getRecursiveChildren();
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		var newLowerLayer = delta > 0 ? Project.layers[to] : (to > 0 ? Project.layers[to - 1] : null);

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

	public function move(to: Int) {
		if (!canMove(to)) {
			return;
		}

		var pointers = TabLayers.initLayerMap();
		var oldIndex = Project.layers.indexOf(this);
		var delta = to - oldIndex;
		var newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];

		// Group or layer is collapsed so we check below and update the upper layer.
		if (newUpperLayer != null && !newUpperLayer.show_panel) {
			var children = newUpperLayer.getRecursiveChildren();
			to -= children != null ? children.length : 0;
			delta = to - oldIndex;
			newUpperLayer = delta > 0 ? (to < Project.layers.length - 1 ? Project.layers[to + 1] : null) : Project.layers[to];
		}

		Context.setLayer(this);
		History.orderLayers(to);
		UISidebar.inst.hwnd0.redraws = 2;

		Project.layers.remove(this);
		Project.layers.insert(to, this);

		if (this.isLayer()) {
			var oldParent = this.parent;

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
			var layerMasks = this.getMasks(false);
			if (layerMasks != null) {
				for (idx in 0...layerMasks.length) {
					var mask = layerMasks[idx];
					Project.layers.remove(mask);
					// If the masks are moved down each step increases the index below the layer by one.
					Project.layers.insert(delta > 0 ? oldIndex + delta - 1 : oldIndex + delta + idx, mask);
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
			var children = this.getRecursiveChildren();
			if (children != null) {
				for (idx in 0...children.length) {
					var child = children[idx];
					Project.layers.remove(child);
					// If the children are moved down each step increases the index below the layer by one.
					Project.layers.insert(delta > 0 ? oldIndex + delta - 1 : oldIndex + delta + idx, child);
				}
			}
		}

		for (m in Project.materials) TabLayers.remapLayerPointers(m.canvas.nodes, TabLayers.fillLayerMap(pointers));
	}
}
