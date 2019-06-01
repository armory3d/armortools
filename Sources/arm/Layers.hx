package arm;

import kha.graphics4.TextureFormat;
import kha.graphics4.TextureUnit;
import kha.graphics4.ConstantLocation;
import iron.RenderPath;
import arm.ui.UITrait;
import arm.Tool;

class Layers {

	public static var pipe:kha.graphics4.PipelineState = null;
	public static var pipeCopy:kha.graphics4.PipelineState;
	public static var pipeMask:kha.graphics4.PipelineState;
	public static var tex0:TextureUnit;
	public static var tex1:TextureUnit;
	public static var tex2:TextureUnit;
	public static var texa:TextureUnit;
	public static var texb:TextureUnit;
	public static var texc:TextureUnit;
	public static var opac:ConstantLocation;
	public static var tex0Mask:TextureUnit;
	public static var texaMask:TextureUnit;
	public static var imga:kha.Image = null;
	public static var imgb:kha.Image = null;
	public static var imgc:kha.Image = null;
	public static var expa:kha.Image = null;
	public static var expb:kha.Image = null;
	public static var expc:kha.Image = null;
	public static var expd:kha.Image = null;
	public static var pipeCursor:kha.graphics4.PipelineState;
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

		UITrait.inst.layerPreviewDirty = true;
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

	public static function resizeLayers(g:kha.graphics4.Graphics) {
		var C = App.C;
		if (UITrait.inst.resHandle.position >= 4) { // Save memory for >=16k
			C.undo_steps = 1;
			if (UITrait.inst.undoHandle != null) UITrait.inst.undoHandle.value = C.undo_steps;
			while (UITrait.inst.undoLayers.length > C.undo_steps) { var l = UITrait.inst.undoLayers.pop(); l.unload(); }
		}
		g.end();
		for (l in UITrait.inst.layers) l.resize();
		for (l in UITrait.inst.undoLayers) l.resize();
		var rts = RenderPath.active.renderTargets;
		rts.get("texpaint_blend0").image.unload();
		rts.get("texpaint_blend0").raw.width = Config.getTextureRes();
		rts.get("texpaint_blend0").raw.height = Config.getTextureRes();
		rts.get("texpaint_blend0").image = kha.Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), TextureFormat.L8);
		rts.get("texpaint_blend1").image.unload();
		rts.get("texpaint_blend1").raw.width = Config.getTextureRes();
		rts.get("texpaint_blend1").raw.height = Config.getTextureRes();
		rts.get("texpaint_blend1").image = kha.Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), TextureFormat.L8);
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

	public static function makeCursorPipe() {
		pipeCursor = new kha.graphics4.PipelineState();
		pipeCursor.vertexShader = kha.graphics4.VertexShader.fromSource(ConstData.cursorVert);
		pipeCursor.fragmentShader = kha.graphics4.FragmentShader.fromSource(ConstData.cursorFrag);
		var vs = new kha.graphics4.VertexStructure();
		vs.add("pos", kha.graphics4.VertexData.Short4Norm);
		vs.add("nor", kha.graphics4.VertexData.Short2Norm);
		vs.add("tex", kha.graphics4.VertexData.Short2Norm);
		pipeCursor.inputLayout = [vs];
		pipeCursor.blendSource = kha.graphics4.BlendingFactor.SourceAlpha;
		pipeCursor.blendDestination = kha.graphics4.BlendingFactor.InverseSourceAlpha;
		pipeCursor.depthWrite = false;
		pipeCursor.depthMode = kha.graphics4.CompareMode.Always;
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

	public static function makeExportImg() {
		var l = UITrait.inst.layers[0];
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
			expa = kha.Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expb = kha.Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expc = kha.Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expd = kha.Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
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
					arm.MaterialParser.parsePaintMaterial();
				}
				
				for (i in 0...fills) {
					arm.renderpath.RenderPathPaint.commandsPaint();
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
