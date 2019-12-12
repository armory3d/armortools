package arm.data;

import kha.graphics4.TextureFormat;
import kha.Image;
import iron.RenderPath;
import arm.ui.UITrait;
import arm.node.MaterialParser;
import arm.Tool;

class LayerSlot {
	public var id = 0;
	public var visible = true;
	public var ext = "";

	public var name: String;

	public var texpaint: Image;
	public var texpaint_nor: Image;
	public var texpaint_pack: Image;

	public var texpaint_preview: Image; // Layer preview

	public var texpaint_mask: Image = null; // Texture mask
	public var texpaint_mask_preview: Image;
	public var maskOpacity = 1.0; // Opacity mask
	public var material_mask: MaterialSlot = null; // Fill layer

	public var blending = BlendMix;
	public var objectMask = 0;
	public var uvScale = 1.0;
	public var uvRot = 0.0;
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

	var createMaskColor: Int;
	var createMaskImage: Image;

	public function new(ext = "") {
		if (ext == "") {
			id = 0;
			for (l in Project.layers) if (l.id >= id) id = l.id + 1;
			ext = id + "";
		}

		this.ext = ext;
		name = "Layer " + (id + 1);
		var format = App.bitsHandle.position == Bits8 ?  "RGBA32" :
					 App.bitsHandle.position == Bits16 ? "RGBA64" :
					 									 "RGBA128";

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = format;
			texpaint = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = format;
			texpaint_nor = RenderPath.active.createRenderTarget(t).image;
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = format;
			texpaint_pack = RenderPath.active.createRenderTarget(t).image;
		}

		texpaint_preview = Image.createRenderTarget(200, 200, TextureFormat.RGBA32);
	}

	public function delete() {
		unload();
		var lpos = Project.layers.indexOf(this);
		Project.layers.remove(this);
		// Undo can remove base layer and then restore it from undo layers
		if (lpos > 0) Context.setLayer(Project.layers[lpos - 1]);
	}

	public function unload() {
		texpaint.unload();
		texpaint_nor.unload();
		texpaint_pack.unload();

		RenderPath.active.renderTargets.remove("texpaint" + ext);
		RenderPath.active.renderTargets.remove("texpaint_nor" + ext);
		RenderPath.active.renderTargets.remove("texpaint_pack" + ext);

		texpaint_preview.unload();

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

	public function swapMask(other: LayerSlot) {
		RenderPath.active.renderTargets.get("texpaint_mask" + ext).image = other.texpaint_mask;
		RenderPath.active.renderTargets.get("texpaint_mask" + other.ext).image = texpaint_mask;
		var _texpaint_mask = texpaint_mask;
		texpaint_mask = other.texpaint_mask;
		other.texpaint_mask = _texpaint_mask;
	}

	public function createMask(color: Int, clear = true, image: Image = null) {
		if (texpaint_mask != null) return;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_mask" + ext;
			t.width = Config.getTextureRes();
			t.height = Config.getTextureRes();
			t.format = "R8";
			texpaint_mask = RenderPath.active.createRenderTarget(t).image;
		}

		texpaint_mask_preview = Image.createRenderTarget(200, 200, TextureFormat.L8);

		if (clear) {
			createMaskColor = color;
			createMaskImage = image;
			iron.App.notifyOnRender(clearMask);
		}
	}

	function clearMask(g: kha.graphics4.Graphics) {
		g.end();

		texpaint_mask.g2.begin();

		if (createMaskImage != null) texpaint_mask.g2.drawScaledImage(createMaskImage, 0, 0, texpaint_mask.width, texpaint_mask.height);
		else texpaint_mask.g2.clear(createMaskColor);
		texpaint_mask.g2.end();

		g.begin();
		iron.App.removeRender(clearMask);

		Context.layerPreviewDirty = true;
		createMaskColor = 0;
		createMaskImage = null;
	}

	public function deleteMask() {
		if (texpaint_mask == null) return;

		texpaint_mask.unload();
		RenderPath.active.renderTargets.remove("texpaint_mask" + ext);
		texpaint_mask = null;

		texpaint_mask_preview.unload();
	}

	public function applyMask() {
		if (texpaint_mask == null) return;

		if (Layers.pipeMerge == null) Layers.makePipe();
		Layers.makeTempImg();

		// Copy layer to temp
		Layers.imga.g2.begin(false);
		Layers.imga.g2.pipeline = Layers.pipeCopy;
		Layers.imga.g2.drawImage(texpaint, 0, 0);
		Layers.imga.g2.end();

		// Merge mask
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		texpaint.g4.begin();
		texpaint.g4.setPipeline(Layers.pipeMask);
		texpaint.g4.setTexture(Layers.tex0Mask, Layers.imga);
		texpaint.g4.setTexture(Layers.texaMask, texpaint_mask);
		texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		texpaint.g4.drawIndexedVertices();
		texpaint.g4.end();

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
		l.texpaint.g2.end();
		l.texpaint_nor.g2.begin(false);
		l.texpaint_nor.g2.pipeline = Layers.pipeCopy;
		l.texpaint_nor.g2.drawImage(texpaint_nor, 0, 0);
		l.texpaint_nor.g2.end();
		l.texpaint_pack.g2.begin(false);
		l.texpaint_pack.g2.pipeline = Layers.pipeCopy;
		l.texpaint_pack.g2.drawImage(texpaint_pack, 0, 0);
		l.texpaint_pack.g2.end();

		l.texpaint_preview.g2.begin(true, 0xff000000);
		l.texpaint_preview.g2.drawScaledImage(texpaint_preview, 0, 0, texpaint_preview.width, texpaint_preview.height);
		l.texpaint_preview.g2.end();

		if (texpaint_mask != null) {
			l.createMask(0, false);
			l.texpaint_mask.g2.begin(false);
			l.texpaint_mask.g2.pipeline = Layers.pipeCopy;
			l.texpaint_mask.g2.drawImage(texpaint_mask, 0, 0);
			l.texpaint_mask.g2.end();

			l.texpaint_mask_preview.g2.begin(true, 0xff000000);
			l.texpaint_mask_preview.g2.drawScaledImage(texpaint_mask_preview, 0, 0, texpaint_mask_preview.width, texpaint_mask_preview.height);
			l.texpaint_mask_preview.g2.end();
		}

		l.visible = visible;
		l.maskOpacity = maskOpacity;
		l.material_mask = material_mask;
		l.objectMask = objectMask;
		l.blending = blending;
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

		var res = Config.getTextureRes();
		var rts = RenderPath.active.renderTargets;

		var texpaint = this.texpaint;
		var texpaint_nor = this.texpaint_nor;
		var texpaint_pack = this.texpaint_pack;

		this.texpaint = Image.createRenderTarget(res, res, format);
		this.texpaint_nor = Image.createRenderTarget(res, res, format);
		this.texpaint_pack = Image.createRenderTarget(res, res, format);

		this.texpaint.g2.begin(false);
		this.texpaint.g2.drawScaledImage(texpaint, 0, 0, res, res);
		this.texpaint.g2.end();

		this.texpaint_nor.g2.begin(false);
		this.texpaint_nor.g2.drawScaledImage(texpaint_nor, 0, 0, res, res);
		this.texpaint_nor.g2.end();

		this.texpaint_pack.g2.begin(false);
		this.texpaint_pack.g2.drawScaledImage(texpaint_pack, 0, 0, res, res);
		this.texpaint_pack.g2.end();

		iron.App.notifyOnInit(function() { // Out of command list execution
			texpaint.unload();
			texpaint_nor.unload();
			texpaint_pack.unload();
		});

		rts.get("texpaint" + this.ext).image = this.texpaint;
		rts.get("texpaint_nor" + this.ext).image = this.texpaint_nor;
		rts.get("texpaint_pack" + this.ext).image = this.texpaint_pack;

		if (this.texpaint_mask != null && this.texpaint_mask.width != res) {
			var texpaint_mask = this.texpaint_mask;
			this.texpaint_mask = Image.createRenderTarget(res, res, TextureFormat.L8);

			this.texpaint_mask.g2.begin(false);
			this.texpaint_mask.g2.drawScaledImage(texpaint_mask, 0, 0, res, res);
			this.texpaint_mask.g2.end();

			iron.App.notifyOnInit(function() { // Out of command list execution
				texpaint_mask.unload();
			});

			rts.get("texpaint_mask" + this.ext).image = this.texpaint_mask;
		}
	}

	public function clear(g: kha.graphics4.Graphics) {
		g.end();

		texpaint.g4.begin();
		texpaint.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Base
		texpaint.g4.end();

		texpaint_nor.g4.begin();
		texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor.g4.end();

		texpaint_pack.g4.begin();
		texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack.g4.end();

		g.begin();
		iron.App.removeRender(clear);

		#if krom_linux
		Context.layerPreviewDirty = true;
		#end
	}

	public function toFillLayer() {
		Context.setLayer(this);
		material_mask = Context.material;
		Layers.updateFillLayers(4);
		function _parse(_) {
			MaterialParser.parsePaintMaterial();
			Context.layerPreviewDirty = true;
			UITrait.inst.hwnd.redraws = 2;
			iron.App.removeRender(_parse);
		}
		iron.App.notifyOnRender(_parse);
	}

	public function toPaintLayer() {
		Context.setLayer(this);
		material_mask = null;
		MaterialParser.parsePaintMaterial();
		Context.layerPreviewDirty = true;
		UITrait.inst.hwnd.redraws = 2;
	}
}
