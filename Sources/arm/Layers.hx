package arm;

import kha.Image;
import kha.graphics4.TextureFormat;
import kha.graphics4.TextureUnit;
import kha.graphics4.ConstantLocation;
import kha.graphics4.PipelineState;
import kha.graphics4.VertexShader;
import kha.graphics4.FragmentShader;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.graphics4.BlendingFactor;
import kha.graphics4.CompareMode;
import iron.RenderPath;
import arm.ui.UITrait;
import arm.data.ConstData;
import arm.data.LayerSlot;
import arm.nodes.MaterialParser;
import arm.render.RenderPathPaint;
import arm.util.MeshUtil;
import arm.Tool;

class Layers {

	public static var pipe:PipelineState = null;
	public static var pipeCopy:PipelineState;
	public static var pipeMask:PipelineState;
	public static var tex0:TextureUnit;
	public static var tex1:TextureUnit;
	public static var tex2:TextureUnit;
	public static var texa:TextureUnit;
	public static var texb:TextureUnit;
	public static var texc:TextureUnit;
	public static var opac:ConstantLocation;
	public static var tex0Mask:TextureUnit;
	public static var texaMask:TextureUnit;
	public static var imga:Image = null;
	public static var imgb:Image = null;
	public static var imgc:Image = null;
	public static var expa:Image = null;
	public static var expb:Image = null;
	public static var expc:Image = null;
	public static var expd:Image = null;
	public static var pipeCursor:PipelineState;
	public static var cursorVP:ConstantLocation;
	public static var cursorInvVP:ConstantLocation;
	public static var cursorMouse:ConstantLocation;
	public static var cursorStep:ConstantLocation;
	public static var cursorRadius:ConstantLocation;
	public static var cursorTex:TextureUnit;
	public static var cursorGbufferD:TextureUnit;
	public static var cursorGbuffer0:TextureUnit;
	
	public static function initLayers(g:kha.graphics4.Graphics) {
		g.end();

		var layers = Project.layers;
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

		Context.layerPreviewDirty = true;
		Context.ddirty = 3;
	}

	public static function clearLastLayer(g:kha.graphics4.Graphics) {
		g.end();

		var layers = Project.layers;
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

	public static function resizeLayers(g:kha.graphics4.Graphics) {
		var C = Config.raw;
		if (UITrait.inst.resHandle.position >= 4) { // Save memory for >=16k
			C.undo_steps = 2;
			if (UITrait.inst.undoHandle != null) UITrait.inst.undoHandle.value = C.undo_steps;
			while (History.undoLayers.length > C.undo_steps) { var l = History.undoLayers.pop(); l.unload(); }
		}
		g.end();
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
		var rts = RenderPath.active.renderTargets;
		rts.get("texpaint_blend0").image.unload();
		rts.get("texpaint_blend0").raw.width = Config.getTextureRes();
		rts.get("texpaint_blend0").raw.height = Config.getTextureRes();
		rts.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), TextureFormat.L8);
		rts.get("texpaint_blend1").image.unload();
		rts.get("texpaint_blend1").raw.width = Config.getTextureRes();
		rts.get("texpaint_blend1").raw.height = Config.getTextureRes();
		rts.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), TextureFormat.L8);
		Context.brushBlendDirty = true;
		g.begin();
		Context.ddirty = 2;
		iron.App.removeRender(resizeLayers);
	}

	public static function setLayerBits(g:kha.graphics4.Graphics) {
		g.end();
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
		g.begin();
		iron.App.removeRender(setLayerBits);
	}

	public static function deleteSelectedLayer() {
		Context.layer.unload();
		var lpos = Project.layers.indexOf(Context.layer);
		Project.layers.remove(Context.layer);
		// Undo can remove base layer and then restore it from undo layers
		if (lpos > 0) Context.setLayer(Project.layers[lpos - 1]);
	}

	public static function makePipe() {
		pipe = new PipelineState();
		pipe.vertexShader = VertexShader.fromSource(ConstData.layerMergeVert);
		pipe.fragmentShader = FragmentShader.fromSource(ConstData.layerMergeFrag);
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipe.inputLayout = [vs];
		pipe.compile();
		tex0 = pipe.getTextureUnit("tex0");
		tex1 = pipe.getTextureUnit("tex1");
		tex2 = pipe.getTextureUnit("tex2");
		texa = pipe.getTextureUnit("texa");
		texb = pipe.getTextureUnit("texb");
		texc = pipe.getTextureUnit("texc");
		opac = pipe.getConstantLocation("opac");

		pipeCopy = new PipelineState();
		pipeCopy.vertexShader = VertexShader.fromSource(ConstData.layerViewVert);
		pipeCopy.fragmentShader = FragmentShader.fromSource(ConstData.layerViewFrag);
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopy.inputLayout = [vs];
		pipeCopy.compile();

		pipeMask = new PipelineState();
		pipeMask.vertexShader = VertexShader.fromSource(ConstData.layerMergeVert);
		pipeMask.fragmentShader = FragmentShader.fromSource(ConstData.maskMergeFrag);
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeMask.inputLayout = [vs];
		pipeMask.compile();
		tex0Mask = pipeMask.getTextureUnit("tex0");
		texaMask = pipeMask.getTextureUnit("texa");
	}

	public static function makeCursorPipe() {
		pipeCursor = new PipelineState();
		pipeCursor.vertexShader = VertexShader.fromSource(ConstData.cursorVert);
		pipeCursor.fragmentShader = FragmentShader.fromSource(ConstData.cursorFrag);
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Short4Norm);
		vs.add("nor", VertexData.Short2Norm);
		vs.add("tex", VertexData.Short2Norm);
		pipeCursor.inputLayout = [vs];
		pipeCursor.blendSource = BlendingFactor.SourceAlpha;
		pipeCursor.blendDestination = BlendingFactor.InverseSourceAlpha;
		pipeCursor.depthWrite = false;
		pipeCursor.depthMode = CompareMode.Always;
		pipeCursor.compile();
		cursorVP = pipeCursor.getConstantLocation("VP");
		cursorInvVP = pipeCursor.getConstantLocation("invVP");
		cursorMouse = pipeCursor.getConstantLocation("mouse");
		cursorStep = pipeCursor.getConstantLocation("step");
		cursorRadius = pipeCursor.getConstantLocation("radius");
		cursorGbufferD = pipeCursor.getTextureUnit("gbufferD");
		cursorGbuffer0 = pipeCursor.getTextureUnit("gbuffer0");
		cursorTex = pipeCursor.getTextureUnit("tex");
	}

	public static function makeTempImg() {
		var l = Project.layers[0];
		if (imga != null && imga.width != l.texpaint.width) {
			imga.unload();
			imgb.unload();
			imgc.unload();
			imga = null;
			imgb = null;
			imgc = null;
		}
		if (imga == null) {
			imga = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			imgb = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			imgc = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
		}
	}

	public static function makeExportImg() {
		var l = Project.layers[0];
		if (expa != null && expa.width != l.texpaint.width) {
			expa.unload();
			expb.unload();
			expc.unload();
			expd.unload();
			expa = null;
			expb = null;
			expc = null;
			expd = null;
		}
		if (expa == null) {
			expa = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expb = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expc = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expd = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
		}
	}

	public static function mergeSelectedLayer(g:kha.graphics4.Graphics) {
		if (pipe == null) makePipe();

		var l0 = Project.layers[0];
		var l1 = Context.layer;
		
		for (i in 1...Project.layers.length) { // Merge down
			if (Project.layers[i] == l1) {
				l0 = Project.layers[i - 1];
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
		Context.setLayer(l0);
		Context.layerPreviewDirty = true;
	}

	public static function isFillMaterial():Bool {
		var m = Context.material;
		for (l in Project.layers) if (l.material_mask == m) return true;
		return false;
	}

	public static function updateFillLayers(fills = 1) {
		var m = Context.material;
		var layers = Project.layers;
		var selectedLayer = Context.layer;
		var isMask = Context.layerIsMask;
		var selectedTool = Context.tool;
		var current:kha.graphics4.Graphics2 = null;

		var first = true;
		for (l in layers) {
			if (l.material_mask == m) {
				if (first) {
					current = @:privateAccess kha.graphics4.Graphics2.current;
					if (current != null) current.end();
					Context.pdirty = fills;
					Context.layerIsMask = false;
					Context.tool = ToolFill;
				}

				Context.layer = l;
				setObjectMask();

				if (first) {
					first = false;
					MaterialParser.parsePaintMaterial();
				}
				
				for (i in 0...fills) {
					RenderPathPaint.commandsPaint();
				}
			}
		}

		if (!first) {
			Context.pdirty = 0;
			Context.ddirty = 2;
			Context.rdirty = 2;
			if (current != null) current.begin(false);
			Context.layer = selectedLayer;
			Context.layerIsMask = isMask;
			setObjectMask();
			Context.tool = selectedTool;
		}
	}

	public static function setObjectMask() {
		var ar = ["None"];
		for (p in Project.paintObjects) ar.push(p.name);

		var mask = Context.layer.objectMask;
		if (UITrait.inst.layerFilter > 0) mask = UITrait.inst.layerFilter;
		if (mask > 0) {
			if (Context.mergedObject != null) Context.mergedObject.visible = false;
			var o = Project.paintObjects[0];
			for (p in Project.paintObjects) if (p.name == ar[mask]) { o = p; break; }
			Context.selectPaintObject(o);
		}
		else {
			if (Context.mergedObject == null) {
				MeshUtil.mergeMesh();
			}
			Context.selectPaintObject(Context.mainObject());
			Context.paintObject.skip_context = "paint";
			Context.mergedObject.visible = true;
		}
	}

	public static function newLayer(clear = true):LayerSlot {
		if (Project.layers.length > 255) return null;
		var l = new LayerSlot();
		Project.layers.push(l);
		Context.setLayer(l);
		if (clear) iron.App.notifyOnRender(Layers.clearLastLayer);
		Context.layerPreviewDirty = true;
		return l;
	}

	public static function toFillLayer(l:LayerSlot) {
		Context.setLayer(l);
		l.material_mask = Context.material;
		Layers.updateFillLayers(4);
		MaterialParser.parsePaintMaterial();
		Context.layerPreviewDirty = true;
		UITrait.inst.hwnd.redraws = 2;
	}

	public static function toPaintLayer(l:LayerSlot) {
		Context.setLayer(l);
		l.material_mask = null;
		MaterialParser.parsePaintMaterial();
		Context.layerPreviewDirty = true;
		UITrait.inst.hwnd.redraws = 2;
	}

	public static function createFillLayer() {
		function makeFill(g:kha.graphics4.Graphics) {
			g.end();
			var l = newLayer();
			History.newLayer();
			l.objectMask = UITrait.inst.layerFilter;
			History.toFillLayer();
			toFillLayer(l);
			g.begin();
			iron.App.removeRender(makeFill);
		}
		iron.App.notifyOnRender(makeFill);
	}

	public static function createImageMask(asset:zui.Canvas.TAsset) {
		var l = Context.layer;
		if (l != Project.layers[0]) {
			History.newMask();
			l.createMask(0x00000000, true, UITrait.inst.getImage(asset));
			Context.setLayer(l, true);
			Context.layerPreviewDirty = true;
		}
	}
}
