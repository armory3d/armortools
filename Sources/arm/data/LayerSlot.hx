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
		var masks = getMasks();
		if (masks != null) for (m in masks) m.delete();
		var children = getChildren();
		if (children != null) for (c in children) c.parent = null;
		var lpos = Project.layers.indexOf(this);
		Project.layers.remove(this);
		// Undo can remove base layer and then restore it from undo layers
		if (Project.layers.length > 0) {
			Context.setLayer(Project.layers[lpos > 0 ? lpos - 1 : 0]);
		}
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
		RenderPath.active.renderTargets.get("texpaint" + ext).image = other.texpaint;
		RenderPath.active.renderTargets.get("texpaint" + other.ext).image = texpaint;
		var _texpaint = texpaint;
		texpaint = other.texpaint;
		other.texpaint = _texpaint;
		var _texpaint_preview = texpaint_preview;
		texpaint_preview = other.texpaint_preview;
		other.texpaint_preview = _texpaint_preview;

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

	public function clear(baseColor = 0x00000000, baseImage: kha.Image = null) {
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
			texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, Layers.defaultRough, 0.0, 0.0)); // Occ, rough, met
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
		Layers.applyMask(parent, this);
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
		l.paintBase = paintBase;
		l.paintOpac = paintOpac;
		l.paintOcc = paintOcc;
		l.paintRough = paintRough;
		l.paintMet = paintMet;
		l.paintNor = paintNor;
		l.paintHeight = paintHeight;
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

	public function getMasks(): Array<LayerSlot> {
		var children: Array<LayerSlot> = null; // Child masks of a layer
		for (l in Project.layers) {
			if (l.parent == this && l.isMask()) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	public function hasMasks(): Bool {
		for (l in Project.layers) {
			if (l.parent == this && l.isMask()) {
				return true;
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

	public function isMask(): Bool {
		return texpaint != null && texpaint_nor == null;
	}

	public function canMove(to: Int): Bool {
		var i = Project.layers.indexOf(this);
		var delta = to - i;
		if (i + delta < 0 || i + delta > Project.layers.length - 1 || delta == 0) return false;

		var isGroup = this.isGroup();
		var isMask = this.isMask();
		var isLayer = this.isLayer();
		var j = delta > 0 ? to : (to == 0 ? 0 : to - 1); // One element down
		var k = delta > 0 ? to + 1 : to; // One element up
		var jParent = j >= 0 ? Project.layers[j].parent : null;
		var kIsGroup = k < Project.layers.length ? Project.layers[k].isGroup() : false;
		var kIsMask = k < Project.layers.length ? Project.layers[k].isMask() : false;
		var jIsMask = j < Project.layers.length ? Project.layers[j].isMask() : false;

		// Prevent group nesting for now
		if (isGroup && jParent != null && jParent.show_panel) {
			return false;
		}

		// Prevent moving mask to group
		if (isMask && kIsGroup) {
			return false;
		}

		// Prevent moving mask to top
		if (isMask && i + delta == Project.layers.length - 1) {
			return false;
		}

		// Prevent moving group to mask
		if (isGroup && kIsMask) {
			return false;
		}

		// Prevent moving layer to mask
		if (isLayer && kIsMask) {
			return false;
		}

		// Prevent moving layer between layer and mask
		if (isLayer && jIsMask) {
			return false;
		}

		return true;
	}

	public function move(to: Int) {
		if (!canMove(to)) {
			return;
		}

		var i = Project.layers.indexOf(this);
		var delta = to - i;

		var pointers = TabLayers.initLayerMap();
		var isGroup = this.isGroup();
		var isMask = this.isMask();
		var j = delta > 0 ? to : (to == 0 ? 0 : to - 1); // One element down
		var k = delta > 0 ? to + 1 : to; // One element up
		var jParent = j >= 0 ? Project.layers[j].parent : null;
		var kParent = k < Project.layers.length ? Project.layers[k].parent : null;
		var kIsGroup = k < Project.layers.length ? Project.layers[k].isGroup() : false;
		var kIsMask = k < Project.layers.length ? Project.layers[k].isMask() : false;
		var kIsLayer = k < Project.layers.length ? Project.layers[k].isLayer() : false;
		var kLayer = k < Project.layers.length ? Project.layers[k] : null;

		if (kIsGroup && !kLayer.show_panel) {
			delta -= kLayer.getChildren().length;
		}

		if (kIsLayer && kLayer.getMasks() != null && !kLayer.show_panel) {
			delta -= kLayer.getMasks().length;
		}

		Context.setLayer(this);
		History.orderLayers(i + delta);
		UISidebar.inst.hwnd0.redraws = 2;

		Project.layers.remove(this);
		Project.layers.insert(i + delta, this);

		if (isGroup) {
			var children = this.getChildren();
			for (l in 0...children.length) {
				var c = children[delta > 0 ? l : children.length - 1 - l];
				Project.layers.remove(c);
				Project.layers.insert(delta > 0 ? i + delta - 1 : i + delta, c);

				var lmasks = c.getMasks();
				if (lmasks != null) {
					for (m in 0...lmasks.length) {
						var mc = lmasks[delta > 0 ? m : lmasks.length - 1 - m];
						Project.layers.remove(mc);
						Project.layers.insert(delta > 0 ? i + delta - 2 : i + delta, mc);
					}
				}
			}
		}
		else if (isMask) {
			// Moved to different layer
			if (kIsMask && kParent != this.parent) {
				this.parent = kParent;
			}
			if (kIsLayer && kLayer != this.parent) {
				this.parent = kLayer;
			}
		}
		else { // Layer
			// Moved to group
			if (this.parent == null && jParent != null && jParent.show_panel) {
				this.parent = jParent;
			}

			var oldParent = null;
			// Moved out of group
			if (this.parent != null && kParent == null && !kIsGroup) {
				oldParent = this.parent;
				this.parent = null;
			}
			// Moved to different group
			if (this.parent != null && ((kParent != null && kParent.show_panel) || kIsGroup)) {
				oldParent = this.parent;
				this.parent = kIsGroup ? kLayer : kParent;
			}

			var lmasks = this.getMasks();
			if (lmasks != null) {
				for (m in 0...lmasks.length) {
					var mc = lmasks[delta > 0 ? m : lmasks.length - 1 - m];
					Project.layers.remove(mc);
					Project.layers.insert(delta > 0 ? i + delta - 1 : i + delta, mc);
				}
			}

			// Remove empty group
			if (oldParent != null && oldParent.getChildren() == null) {
				oldParent.delete();
			}
		}

		for (m in Project.materials) TabLayers.remapLayerPointers(m.canvas.nodes, TabLayers.fillLayerMap(pointers));
	}
}
