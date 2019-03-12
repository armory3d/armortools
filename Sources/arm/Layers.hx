package arm;

import iron.RenderPath;
import arm.ui.*;

class Layers {
	
	public static function initLayers(g:kha.graphics4.Graphics) {
		g.end();

		var layers = UITrait.inst.layers;
		layers[0].texpaint.g4.begin();
		layers[0].texpaint.g4.clear(kha.Color.fromFloats(0.5, 0.5, 0.5, 1.0)); // Base
		layers[0].texpaint.g4.end();

		layers[0].texpaint_nor.g4.begin();
		layers[0].texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 1.0)); // Nor
		layers[0].texpaint_nor.g4.end();

		layers[0].texpaint_pack.g4.begin();
		layers[0].texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.4, 0.0, 1.0)); // Occ, rough, met
		layers[0].texpaint_pack.g4.end();

		g.begin();
		iron.App.removeRender(initLayers);

		UITrait.inst.ddirty = 3;
	}

	public static function initHeightLayer(g:kha.graphics4.Graphics) {
		g.end();

		var layers = UITrait.inst.layers;
		layers[0].texpaint_opt.g4.begin();
		layers[0].texpaint_opt.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0)); // Opac, emis, height
		layers[0].texpaint_opt.g4.end();

		g.begin();
		iron.App.removeRender(initHeightLayer);
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

		if (layers[i].texpaint_opt != null) {
			layers[i].texpaint_opt.g4.begin();
			layers[i].texpaint_opt.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0)); // Opac, emis, height
			layers[i].texpaint_opt.g4.end();
		}

		g.begin();
		iron.App.removeRender(clearLastLayer);
	}

	public static function resizeLayer(l:LayerSlot) {
		var res = Config.getTextureRes();
		var rts = RenderPath.active.renderTargets;

		var texpaint = l.texpaint;
		var texpaint_nor = l.texpaint_nor;
		var texpaint_pack = l.texpaint_pack;
		var texpaint_opt = l.texpaint_opt;

		l.texpaint = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.Depth16);
		l.texpaint_nor = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
		l.texpaint_pack = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);
		if (l.texpaint_opt != null) l.texpaint_opt = kha.Image.createRenderTarget(res, res, kha.graphics4.TextureFormat.RGBA32, kha.graphics4.DepthStencilFormat.NoDepthAndStencil);

		l.texpaint.g2.begin(false);
		l.texpaint.g2.drawScaledImage(texpaint, 0, 0, res, res);
		l.texpaint.g2.end();

		l.texpaint_nor.g2.begin(false);
		l.texpaint_nor.g2.drawScaledImage(texpaint_nor, 0, 0, res, res);
		l.texpaint_nor.g2.end();

		l.texpaint_pack.g2.begin(false);
		l.texpaint_pack.g2.drawScaledImage(texpaint_pack, 0, 0, res, res);
		l.texpaint_pack.g2.end();

		if (texpaint_opt != null) {
			l.texpaint_opt.g2.begin(false);
			l.texpaint_opt.g2.drawScaledImage(texpaint_opt, 0, 0, res, res);
			l.texpaint_opt.g2.end();
			rts.get("texpaint_opt" + l.ext).image = l.texpaint_opt;
		}

		texpaint.unload();
		texpaint_nor.unload();
		texpaint_pack.unload();
		if (texpaint_opt != null) texpaint_opt.unload();

		rts.get("texpaint" + l.ext).image = l.texpaint;
		rts.get("texpaint_nor" + l.ext).image = l.texpaint_nor;
		rts.get("texpaint_pack" + l.ext).image = l.texpaint_pack;
	}

	public static function resizeLayers(g:kha.graphics4.Graphics) {
		var C = UITrait.inst.C;
		if (UITrait.inst.resHandle.position >= 4) { // Save memory for >=16k
			C.undo_steps = 0;// No undo for 16k+
			if (UITrait.inst.undoHandle != null) UITrait.inst.undoHandle.value = C.undo_steps;
			while (UITrait.inst.undoLayers.length > C.undo_steps) { var l = UITrait.inst.undoLayers.pop(); l.unload(); }
		}
		g.end();
		for (l in UITrait.inst.layers) resizeLayer(l);
		for (l in UITrait.inst.undoLayers) resizeLayer(l);
		g.begin();
		UITrait.inst.ddirty = 2;
		iron.App.removeRender(resizeLayers);
	}

	public static function deleteSelectedLayer() {
		UITrait.inst.selectedLayer.unload();
		UITrait.inst.layers.remove(UITrait.inst.selectedLayer);
		UITrait.inst.selectedLayer = UITrait.inst.layers[0];
		UINodes.inst.parseMeshMaterial();
		UINodes.inst.parsePaintMaterial();
		UITrait.inst.ddirty = 2;
	}

	static function makePipe() {
		UITrait.inst.pipe = new kha.graphics4.PipelineState();
		UITrait.inst.pipe.vertexShader = kha.graphics4.VertexShader.fromSource(ConstData.painterVert);
		UITrait.inst.pipe.fragmentShader = kha.graphics4.FragmentShader.fromSource(ConstData.painterFrag);
		var vs = new kha.graphics4.VertexStructure();
		vs.add("pos", kha.graphics4.VertexData.Float3);
		vs.add("tex", kha.graphics4.VertexData.Float2);
		vs.add("col", kha.graphics4.VertexData.Float4);
		UITrait.inst.pipe.inputLayout = [vs];
		UITrait.inst.pipe.blendSource = kha.graphics4.BlendingFactor.SourceAlpha;
		UITrait.inst.pipe.blendDestination = kha.graphics4.BlendingFactor.InverseSourceAlpha;
		UITrait.inst.pipe.colorWriteMaskAlpha = false;
		UITrait.inst.pipe.compile();
	}

	public static function applySelectedLayer(g:kha.graphics4.Graphics) {
		if (UITrait.inst.pipe == null) makePipe();

		var l0 = UITrait.inst.layers[0];
		var l1 = UITrait.inst.selectedLayer;

		g.end();

		l0.texpaint.g2.begin(false);
		l0.texpaint.g2.pipeline = UITrait.inst.pipe;
		l0.texpaint.g2.drawImage(l1.texpaint, 0, 0);
		l0.texpaint.g2.end();

		l0.texpaint_nor.g2.begin(false);
		l0.texpaint_nor.g2.pipeline = UITrait.inst.pipe;
		l0.texpaint_nor.g2.drawImage(l1.texpaint_nor, 0, 0);
		l0.texpaint_nor.g2.end();

		l0.texpaint_pack.g2.begin(false);
		l0.texpaint_pack.g2.pipeline = UITrait.inst.pipe;
		l0.texpaint_pack.g2.drawImage(l1.texpaint_pack, 0, 0);
		l0.texpaint_pack.g2.end();

		if (l0.texpaint_opt != null) {
			l0.texpaint_opt.g2.begin(false);
			l0.texpaint_opt.g2.drawImage(l1.texpaint_opt, 0, 0);
			l0.texpaint_opt.g2.end();
		}

		g.begin();

		deleteSelectedLayer();
		iron.App.removeRender(applySelectedLayer);
	}
}
