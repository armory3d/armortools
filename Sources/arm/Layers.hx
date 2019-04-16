package arm;

import kha.graphics4.TextureFormat;
import kha.graphics4.DepthStencilFormat;
import iron.RenderPath;
import arm.ui.UITrait;
import arm.ui.*;

class Layers {

	public static var pipe:kha.graphics4.PipelineState = null;
	public static var pipeCopy:kha.graphics4.PipelineState;
	public static var pipeMask:kha.graphics4.PipelineState;
	static var tex0:kha.graphics4.TextureUnit;
	static var tex1:kha.graphics4.TextureUnit;
	static var tex2:kha.graphics4.TextureUnit;
	static var texa:kha.graphics4.TextureUnit;
	static var texb:kha.graphics4.TextureUnit;
	static var texc:kha.graphics4.TextureUnit;
	static var opac:kha.graphics4.ConstantLocation;
	public static var tex0Mask:kha.graphics4.TextureUnit;
	public static var texaMask:kha.graphics4.TextureUnit;
	public static var imga:kha.Image = null;
	public static var imgb:kha.Image = null;
	public static var imgc:kha.Image = null;
	
	public static function initLayers(g:kha.graphics4.Graphics) {
		g.end();

		var layers = UITrait.inst.layers;
		layers[0].texpaint.g4.begin();
		layers[0].texpaint.g4.clear(kha.Color.fromFloats(0.5, 0.5, 0.5, 0.0)); // Base
		layers[0].texpaint.g4.end();

		layers[0].texpaint_nor.g4.begin();
		layers[0].texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		layers[0].texpaint_nor.g4.end();

		layers[0].texpaint_pack.g4.begin();
		layers[0].texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		layers[0].texpaint_pack.g4.end();

		g.begin();
		iron.App.removeRender(initLayers);

		UITrait.inst.ddirty = 3;
	}

	public static function clearLastLayer(g:kha.graphics4.Graphics) {
		g.end();

		var layers = UITrait.inst.layers;
		var i = layers.length - 1;
		layers[i].texpaint.g4.begin();
		layers[i].texpaint.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Base
		layers[i].texpaint.g4.end();

		layers[i].texpaint_nor.g4.begin();
		layers[i].texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		layers[i].texpaint_nor.g4.end();

		layers[i].texpaint_pack.g4.begin();
		layers[i].texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Occ, rough, met
		layers[i].texpaint_pack.g4.end();

		g.begin();
		iron.App.removeRender(clearLastLayer);
	}

	public static function resizeLayer(l:LayerSlot, hasDepth:Bool) {
		var res = Config.getTextureRes();
		var rts = RenderPath.active.renderTargets;

		var texpaint = l.texpaint;
		var texpaint_nor = l.texpaint_nor;
		var texpaint_pack = l.texpaint_pack;

		var depthFormat = hasDepth ? DepthStencilFormat.Depth16 : DepthStencilFormat.NoDepthAndStencil;
		l.texpaint = kha.Image.createRenderTarget(res, res, TextureFormat.RGBA32, depthFormat);
		l.texpaint_nor = kha.Image.createRenderTarget(res, res, TextureFormat.RGBA32, DepthStencilFormat.NoDepthAndStencil);
		l.texpaint_pack = kha.Image.createRenderTarget(res, res, TextureFormat.RGBA32, DepthStencilFormat.NoDepthAndStencil);

		l.texpaint.g2.begin(false);
		l.texpaint.g2.drawScaledImage(texpaint, 0, 0, res, res);
		l.texpaint.g2.end();

		l.texpaint_nor.g2.begin(false);
		l.texpaint_nor.g2.drawScaledImage(texpaint_nor, 0, 0, res, res);
		l.texpaint_nor.g2.end();

		l.texpaint_pack.g2.begin(false);
		l.texpaint_pack.g2.drawScaledImage(texpaint_pack, 0, 0, res, res);
		l.texpaint_pack.g2.end();

		texpaint.unload();
		texpaint_nor.unload();
		texpaint_pack.unload();

		rts.get("texpaint" + l.ext).image = l.texpaint;
		rts.get("texpaint_nor" + l.ext).image = l.texpaint_nor;
		rts.get("texpaint_pack" + l.ext).image = l.texpaint_pack;

		if (l.texpaint_mask != null) {
			var texpaint_mask = l.texpaint_mask;
			l.texpaint_mask = kha.Image.createRenderTarget(res, res, TextureFormat.L8, DepthStencilFormat.NoDepthAndStencil);

			l.texpaint_mask.g2.begin(false);
			l.texpaint_mask.g2.drawScaledImage(texpaint_mask, 0, 0, res, res);
			l.texpaint_mask.g2.end();

			texpaint_mask.unload();

			rts.get("texpaint_mask" + l.ext).image = l.texpaint_mask;
		}
	}

	public static function resizeLayers(g:kha.graphics4.Graphics) {
		var C = UITrait.inst.C;
		if (UITrait.inst.resHandle.position >= 4) { // Save memory for >=16k
			C.undo_steps = 1;
			if (UITrait.inst.undoHandle != null) UITrait.inst.undoHandle.value = C.undo_steps;
			while (UITrait.inst.undoLayers.length > C.undo_steps) { var l = UITrait.inst.undoLayers.pop(); l.unload(); }
		}
		g.end();
		var i = 0;
		for (l in UITrait.inst.layers) {
			resizeLayer(l, i == 0);
			if (i > 0) l.texpaint.setDepthStencilFrom(UITrait.inst.layers[0].texpaint);
			i++;
		}
		for (l in UITrait.inst.undoLayers) {
			resizeLayer(l, false);
			l.texpaint.setDepthStencilFrom(UITrait.inst.layers[0].texpaint);
		}
		var rts = RenderPath.active.renderTargets;
		rts.get("texpaint_blend0").image.unload();
		rts.get("texpaint_blend0").raw.width = Config.getTextureRes();
		rts.get("texpaint_blend0").raw.height = Config.getTextureRes();
		rts.get("texpaint_blend0").image = kha.Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), kha.graphics4.TextureFormat.L8, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
		rts.get("texpaint_blend1").image.unload();
		rts.get("texpaint_blend1").raw.width = Config.getTextureRes();
		rts.get("texpaint_blend1").raw.height = Config.getTextureRes();
		rts.get("texpaint_blend1").image = kha.Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), kha.graphics4.TextureFormat.L8, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
		UITrait.inst.brushBlendDirty = true;
		g.begin();
		UITrait.inst.ddirty = 2;
		iron.App.removeRender(resizeLayers);
	}

	public static function deleteSelectedLayer() {
		UITrait.inst.selectedLayer.unload();
		UITrait.inst.layers.remove(UITrait.inst.selectedLayer);
		UITrait.inst.setLayer(UITrait.inst.layers[0]);
	}

	public static function makePipe() {
		pipe = new kha.graphics4.PipelineState();
		pipe.vertexShader = kha.graphics4.VertexShader.fromSource(ConstData.layerMergeVert);
		pipe.fragmentShader = kha.graphics4.FragmentShader.fromSource(ConstData.layerMergeFrag);
		var vs = new kha.graphics4.VertexStructure();
		vs.add("pos", kha.graphics4.VertexData.Float2);
		pipe.inputLayout = [vs];
		pipe.compile();
		tex0 = pipe.getTextureUnit("tex0");
		tex1 = pipe.getTextureUnit("tex1");
		tex2 = pipe.getTextureUnit("tex2");
		texa = pipe.getTextureUnit("texa");
		texb = pipe.getTextureUnit("texb");
		texc = pipe.getTextureUnit("texc");
		opac = pipe.getConstantLocation("opac");

		pipeCopy = new kha.graphics4.PipelineState();
		pipeCopy.vertexShader = kha.graphics4.VertexShader.fromSource(ConstData.layerViewVert);
		pipeCopy.fragmentShader = kha.graphics4.FragmentShader.fromSource(ConstData.layerViewFrag);
		var vs = new kha.graphics4.VertexStructure();
		vs.add("pos", kha.graphics4.VertexData.Float3);
		vs.add("tex", kha.graphics4.VertexData.Float2);
		vs.add("col", kha.graphics4.VertexData.Float4);
		pipeCopy.inputLayout = [vs];
		pipeCopy.compile();

		pipeMask = new kha.graphics4.PipelineState();
		pipeMask.vertexShader = kha.graphics4.VertexShader.fromSource(ConstData.layerMergeVert);
		pipeMask.fragmentShader = kha.graphics4.FragmentShader.fromSource(ConstData.maskMergeFrag);
		var vs = new kha.graphics4.VertexStructure();
		vs.add("pos", kha.graphics4.VertexData.Float2);
		pipeMask.inputLayout = [vs];
		pipeMask.compile();
		tex0Mask = pipeMask.getTextureUnit("tex0");
		texaMask = pipeMask.getTextureUnit("texa");
	}

	public static function makeTempImg() {
		var l = UITrait.inst.layers[0];
		if (imga != null && imga.width != l.texpaint.width) {
			imga.unload();
			imgb.unload();
			imgc.unload();
			imga = null;
			imgb = null;
			imgc = null;
		}
		if (imga == null) {
			imga = kha.Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			imgb = kha.Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			imgc = kha.Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
		}
	}

	public static function mergeSelectedLayer(g:kha.graphics4.Graphics) {
		if (pipe == null) makePipe();

		var l0 = UITrait.inst.layers[0];
		var l1 = UITrait.inst.selectedLayer;
		
		for (i in 1...UITrait.inst.layers.length) { // Merge down
			if (UITrait.inst.layers[i] == l1) {
				l0 = UITrait.inst.layers[i - 1];
				break;
			}
		}

		g.end();

		makeTempImg();

		if (l1.texpaint_mask != null) {
			l1.applyMask();
		}

		// Copy layer0 to temp
		imga.g2.begin(false);
		imga.g2.pipeline = pipeCopy;
		imga.g2.drawImage(l0.texpaint, 0, 0);
		imga.g2.end();
		imgb.g2.begin(false);
		imgb.g2.pipeline = pipeCopy;
		imgb.g2.drawImage(l0.texpaint_nor, 0, 0);
		imgb.g2.end();
		imgc.g2.begin(false);
		imgc.g2.pipeline = pipeCopy;
		imgc.g2.drawImage(l0.texpaint_pack, 0, 0);
		imgc.g2.end();

		// Merge into layer0
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		l0.texpaint.g4.begin([l0.texpaint_nor, l0.texpaint_pack]);
		l0.texpaint.g4.setPipeline(pipe);
		l0.texpaint.g4.setTexture(tex0, l1.texpaint);
		l0.texpaint.g4.setTexture(tex1, l1.texpaint_nor);
		l0.texpaint.g4.setTexture(tex2, l1.texpaint_pack);
		l0.texpaint.g4.setTexture(texa, imga);
		l0.texpaint.g4.setTexture(texb, imgb);
		l0.texpaint.g4.setTexture(texc, imgc);
		l0.texpaint.g4.setFloat(opac, l1.maskOpacity);
		l0.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		l0.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		l0.texpaint.g4.drawIndexedVertices();
		l0.texpaint.g4.end();

		g.begin();

		deleteSelectedLayer();
		iron.App.removeRender(mergeSelectedLayer);
		UITrait.inst.setLayer(l0);
		UITrait.inst.layerPreviewDirty = true;
	}

	public static function isFillMaterial():Bool {
		var m = UITrait.inst.selectedMaterial;
		for (l in UITrait.inst.layers) if (l.material_mask == m) return true;
		return false;
	}

	public static function updateFillLayers(fills = 1) {
		var m = UITrait.inst.selectedMaterial;
		var layers = UITrait.inst.layers;
		var selectedLayer = UITrait.inst.selectedLayer;
		var isMask = UITrait.inst.selectedLayerIsMask;
		var selectedTool = UITrait.inst.selectedTool;
		var current:kha.graphics4.Graphics2 = null;

		var first = true;
		for (l in layers) {
			if (l.material_mask == m) {
				if (first) {
					current = @:privateAccess kha.graphics4.Graphics2.current;
					if (current != null) current.end();
					UITrait.inst.pdirty = fills;
					UITrait.inst.selectedLayerIsMask = false;
					UITrait.inst.selectedTool = ToolFill;
				}

				UITrait.inst.selectedLayer = l;
				UITrait.inst.setObjectMask();

				if (first) {
					first = false;
					UINodes.inst.parsePaintMaterial();
				}
				
				for (i in 0...fills) {
					arm.renderpath.RenderPathDeferred.commandsPaint();
				}
			}
		}

		if (!first) {
			UITrait.inst.pdirty = 0;
			UITrait.inst.ddirty = 2;
			UITrait.inst.rdirty = 2;
			if (current != null) current.begin(false);
			UITrait.inst.selectedLayer = selectedLayer;
			UITrait.inst.selectedLayerIsMask = isMask;
			UITrait.inst.setObjectMask();
			UITrait.inst.selectedTool = selectedTool;
		}
	}
}
