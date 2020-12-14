package arm.data;

import kha.graphics4.TextureFormat;
import kha.Image;
import iron.RenderPath;
import arm.ui.UISidebar;
import arm.ui.TabLayers;
import arm.node.MakeMaterial;
import arm.Enums;

class LayerSlot {
	public var id = 0;
	public var visible = true;
	public var ext = "";

	public var parent: LayerSlot = null; // Layer inside group

	public var name: String;

	public var texpaint: Image = null;
	public var texpaint_nor: Image = null;
	public var texpaint_pack: Image = null;

	public var texpaint_preview: Image = null; // Layer preview

	public var texpaint_mask: Image = null; // Texture mask
	public var texpaint_mask_preview: Image;
	public var maskOpacity = 1.0; // Opacity mask
	public var fill_layer: MaterialSlot = null;
	public var fill_mask: MaterialSlot = null;
	public var show_panel = false;

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
	public var paintHeight = true;
	public var paintEmis = true;
	public var paintSubs = true;
	public var decalMat = iron.math.Mat4.identity(); // Decal layer

	var createMaskColor: Int;
	var createMaskImage: Image;

	public function new(ext = "", isGroup = false) {
		if (ext == "") {
			id = 0;
			for (l in Project.layers) if (l.id >= id) id = l.id + 1;
			ext = id + "";
		}

		this.ext = ext;

		if (isGroup) {
			name = "Group " + (id + 1);
			return;
		}

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

		texpaint_preview = Image.createRenderTarget(200, 200, TextureFormat.RGBA32);
	}

	public function delete() {
		unload();
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
		if (texpaint == null) return; // Layer is group

		var _texpaint = texpaint;
		var _texpaint_nor = texpaint_nor;
		var _texpaint_pack = texpaint_pack;
		var _texpaint_preview = texpaint_preview;
		function _next() {
			_texpaint.unload();
			_texpaint_nor.unload();
			_texpaint_pack.unload();
			_texpaint_preview.unload();
		}
		App.notifyOnNextFrame(_next);

		RenderPath.active.renderTargets.remove("texpaint" + ext);
		RenderPath.active.renderTargets.remove("texpaint_nor" + ext);
		RenderPath.active.renderTargets.remove("texpaint_pack" + ext);

		deleteMask();
	}

	public function swap(other: LayerSlot) {
		RenderPath.active.renderTargets.get("texpaint" + ext).image = other.texpaint;
		RenderPath.active.renderTargets.get("texpaint_nor" + ext).image = other.texpaint_nor;
		RenderPath.active.renderTargets.get("texpaint_pack" + ext).image = other.texpaint_pack;

		RenderPath.active.renderTargets.get("texpaint" + other.ext).image = texpaint;
		RenderPath.active.renderTargets.get("texpaint_nor" + other.ext).image = texpaint_nor;
		RenderPath.active.renderTargets.get("texpaint_pack" + other.ext).image = texpaint_pack;

		var _texpaint = texpaint;
		var _texpaint_nor = texpaint_nor;
		var _texpaint_pack = texpaint_pack;
		texpaint = other.texpaint;
		texpaint_nor = other.texpaint_nor;
		texpaint_pack = other.texpaint_pack;
		other.texpaint = _texpaint;
		other.texpaint_nor = _texpaint_nor;
		other.texpaint_pack = _texpaint_pack;
	}

	public function clearLayer(baseColor = 0x00000000) {
		texpaint.g4.begin();
		texpaint.g4.clear(baseColor); // Base
		texpaint.g4.end();

		texpaint_nor.g4.begin();
		texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor.g4.end();

		texpaint_pack.g4.begin();
		texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, Layers.defaultRough, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack.g4.end();

		Context.layerPreviewDirty = true;
		Context.ddirty = 3;
	}

	public function swapMask(other: LayerSlot) {
		RenderPath.active.renderTargets.get("texpaint_mask" + ext).image = other.texpaint_mask;
		RenderPath.active.renderTargets.get("texpaint_mask" + other.ext).image = texpaint_mask;
		var _texpaint_mask = texpaint_mask;
		texpaint_mask = other.texpaint_mask;
		other.texpaint_mask = _texpaint_mask;
		var _texpaint_mask_preview = texpaint_mask_preview;
		texpaint_mask_preview = other.texpaint_mask_preview;
		other.texpaint_mask_preview = _texpaint_mask_preview;
	}

	public function createMask(color: Int, clear = true, image: Image = null) {
		if (texpaint_mask != null) return;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_mask" + ext;
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			texpaint_mask = RenderPath.active.createRenderTarget(t).image;
		}

		texpaint_mask_preview = Image.createRenderTarget(200, 200, TextureFormat.L8);

		if (clear) {
			function _next() {
				clearMask(createMaskColor);
				createMaskColor = 0;
				createMaskImage = null;
			}
			createMaskColor = color;
			createMaskImage = image;
			App.notifyOnNextFrame(_next);
		}
	}

	public function clearMask(color = 0x00000000) {
		texpaint_mask.g2.begin(false);
		if (createMaskImage != null) {
			texpaint_mask.g2.drawScaledImage(createMaskImage, 0, 0, texpaint_mask.width, texpaint_mask.height);
		}
		else {
			texpaint_mask.g2.clear(color);
		}
		texpaint_mask.g2.end();
		Context.layerPreviewDirty = true;
		Context.ddirty = 3;
	}

	public function invertMask() {
		if (Layers.pipeInvert8 == null) Layers.makePipe();
		var inverted = Image.createRenderTarget(texpaint_mask.width, texpaint_mask.height, TextureFormat.L8);
		inverted.g2.begin(false);
		inverted.g2.pipeline = Layers.pipeInvert8;
		inverted.g2.drawImage(texpaint_mask, 0, 0);
		inverted.g2.pipeline = null;
		inverted.g2.end();
		var _texpaint_mask = texpaint_mask;
		function _next() {
			_texpaint_mask.unload();
		}
		App.notifyOnNextFrame(_next);
		texpaint_mask = RenderPath.active.renderTargets.get("texpaint_mask" + id).image = inverted;
		Context.layerPreviewDirty = true;
		Context.ddirty = 3;
	}

	public function deleteMask() {
		if (texpaint_mask == null) return;

		var _texpaint_mask = texpaint_mask;
		var _texpaint_mask_preview = texpaint_mask_preview;
		function _next() {
			_texpaint_mask.unload();
			_texpaint_mask_preview.unload();
		}
		App.notifyOnNextFrame(_next);

		RenderPath.active.renderTargets.remove("texpaint_mask" + ext);
		texpaint_mask = null;
		fill_mask = null;
	}

	public function applyMask() {
		Layers.applyMask(this);
		deleteMask();
	}

	public function duplicate(): LayerSlot {
		var layers = Project.layers;
		var i = 0;
		while (i++ < layers.length) if (layers[i] == this) break;
		i++;

		var l = new LayerSlot();
		layers.insert(i, l);

		if (Layers.pipeMerge == null) Layers.makePipe();
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

		l.texpaint_preview.g2.begin(true, 0x00000000);
		l.texpaint_preview.g2.pipeline = Layers.pipeCopy;
		l.texpaint_preview.g2.drawScaledImage(texpaint_preview, 0, 0, texpaint_preview.width, texpaint_preview.height);
		l.texpaint_preview.g2.pipeline = null;
		l.texpaint_preview.g2.end();

		if (texpaint_mask != null) {
			l.createMask(0, false);
			l.texpaint_mask.g2.begin(false);
			l.texpaint_mask.g2.pipeline = Layers.pipeCopy8;
			l.texpaint_mask.g2.drawImage(texpaint_mask, 0, 0);
			l.texpaint_mask.g2.pipeline = null;
			l.texpaint_mask.g2.end();

			l.texpaint_mask_preview.g2.begin(true, 0x00000000);
			l.texpaint_mask_preview.g2.pipeline = Layers.pipeCopy8;
			l.texpaint_mask_preview.g2.drawScaledImage(texpaint_mask_preview, 0, 0, texpaint_mask_preview.width, texpaint_mask_preview.height);
			l.texpaint_mask_preview.g2.pipeline = null;
			l.texpaint_mask_preview.g2.end();
		}

		l.parent = parent;
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
		var format = App.bitsHandle.position == Bits8  ? TextureFormat.RGBA32 :
					 App.bitsHandle.position == Bits16 ? TextureFormat.RGBA64 :
					 									 TextureFormat.RGBA128;

		var resX = Config.getTextureResX();
		var resY = Config.getTextureResY();
		var rts = RenderPath.active.renderTargets;

		var _texpaint = this.texpaint;
		var _texpaint_nor = this.texpaint_nor;
		var _texpaint_pack = this.texpaint_pack;

		this.texpaint = Image.createRenderTarget(resX, resY, format);
		this.texpaint_nor = Image.createRenderTarget(resX, resY, format);
		this.texpaint_pack = Image.createRenderTarget(resX, resY, format);

		if (Layers.pipeMerge == null) Layers.makePipe();

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

		function _next() { // Out of command list execution
			_texpaint.unload();
			_texpaint_nor.unload();
			_texpaint_pack.unload();
		}
		App.notifyOnNextFrame(_next);

		rts.get("texpaint" + this.ext).image = this.texpaint;
		rts.get("texpaint_nor" + this.ext).image = this.texpaint_nor;
		rts.get("texpaint_pack" + this.ext).image = this.texpaint_pack;

		if (this.texpaint_mask != null && (this.texpaint_mask.width != resX || this.texpaint_mask.height != resY)) {
			var _texpaint_mask = this.texpaint_mask;
			this.texpaint_mask = Image.createRenderTarget(resX, resY, TextureFormat.L8);

			this.texpaint_mask.g2.begin(false);
			this.texpaint_mask.g2.pipeline = Layers.pipeCopy8;
			this.texpaint_mask.g2.drawScaledImage(_texpaint_mask, 0, 0, resX, resY);
			this.texpaint_mask.g2.pipeline = null;
			this.texpaint_mask.g2.end();

			function _next() { // Out of command list execution
				_texpaint_mask.unload();
			}
			App.notifyOnNextFrame(_next);

			rts.get("texpaint_mask" + this.ext).image = this.texpaint_mask;
		}
	}

	public function clear() {
		texpaint.g4.begin();
		texpaint.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Base
		texpaint.g4.end();

		texpaint_nor.g4.begin();
		texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor.g4.end();

		texpaint_pack.g4.begin();
		texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack.g4.end();

		#if krom_linux
		Context.layerPreviewDirty = true;
		#end
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

	public function toFillMask() {
		Context.setLayer(this, true);
		fill_mask = Context.material;
		Layers.updateFillLayers();
		function _next() {
			MakeMaterial.parsePaintMaterial();
			Context.layerPreviewDirty = true;
			UISidebar.inst.hwnd0.redraws = 2;
		}
		App.notifyOnNextFrame(_next);
	}

	public function toPaintMask() {
		Context.setLayer(this, true);
		fill_mask = null;
		MakeMaterial.parsePaintMaterial();
		Context.layerPreviewDirty = true;
		UISidebar.inst.hwnd0.redraws = 2;
	}

	public function isVisible(): Bool {
		return visible && (parent == null || parent.visible);
	}

	public function getChildren(): Array<LayerSlot> {
		var children: Array<LayerSlot> = null; // Layer with children is a group
		for (l in Project.layers) {
			if (l.parent == this) {
				if (children == null) children = [];
				children.push(l);
			}
		}
		return children;
	}

	public function move(to: Int) {
		var i = Project.layers.indexOf(this);
		var delta = to - i;
		if (i + delta < 0 || i + delta > Project.layers.length - 1) return;

		var pointers = TabLayers.initLayerMap();
		var isGroup = this.getChildren() != null;
		var j = delta > 0 ? to : to - 1; // One element down
		var k = delta > 0 ? to + 1 : to; // One element up
		var jParent = j >= 0 ? Project.layers[j].parent : null;
		var kParent = k < Project.layers.length ? Project.layers[k].parent : null;
		var kGroup = k < Project.layers.length ? Project.layers[k].getChildren() != null : false;
		var kLayer = k < Project.layers.length ? Project.layers[k] : null;

		// Prevent group nesting for now
		if (isGroup && jParent != null) {
			return;
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
			}
		}
		else {
			// Moved to group
			if (this.parent == null && jParent != null) {
				this.parent = jParent;
			}
			// Moved out of group
			if (this.parent != null && kParent == null && !kGroup) {
				var parent = this.parent;
				this.parent = null;
				// Remove empty group
				if (parent.getChildren() == null) {
					parent.delete();
				}
			}
			// Moved to different group
			if (this.parent != null && (kParent != null || kGroup)) {
				this.parent = kGroup ? kLayer : kParent;
			}
		}

		for (m in Project.materials) TabLayers.remapLayerPointers(m.canvas.nodes, TabLayers.fillLayerMap(pointers));
	}
}
