package arm;

import kha.Image;
import kha.graphics4.TextureFormat;
import kha.graphics4.TextureUnit;
import kha.graphics4.ConstantLocation;
import kha.graphics4.PipelineState;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.graphics4.BlendingFactor;
import kha.graphics4.CompareMode;
import iron.RenderPath;
import arm.ui.UITrait;
import arm.data.LayerSlot;
import arm.node.MaterialParser;
import arm.render.RenderPathPaint;
import arm.util.MeshUtil;
import arm.Tool;
import arm.Project;

class Layers {

	public static var pipeMerge: PipelineState = null;
	public static var pipeCopy: PipelineState;
	public static var pipeMask: PipelineState;
	public static var tex0: TextureUnit;
	public static var tex1: TextureUnit;
	public static var texmask: TextureUnit;
	public static var texa: TextureUnit;
	public static var opac: ConstantLocation;
	public static var blending: ConstantLocation;
	public static var tex0Mask: TextureUnit;
	public static var texaMask: TextureUnit;
	public static var imga: Image = null;
	public static var expa: Image = null;
	public static var expb: Image = null;
	public static var expc: Image = null;
	public static var pipeCursor: PipelineState;
	public static var cursorVP: ConstantLocation;
	public static var cursorInvVP: ConstantLocation;
	public static var cursorMouse: ConstantLocation;
	public static var cursorStep: ConstantLocation;
	public static var cursorRadius: ConstantLocation;
	public static var cursorTex: TextureUnit;
	public static var cursorGbufferD: TextureUnit;
	public static var cursorGbuffer0: TextureUnit;

	public static inline var defaultBase = 0.5;
	public static inline var defaultRough = 0.4;

	public static function initLayers(g: kha.graphics4.Graphics) {
		g.end();

		var layers = Project.layers;
		layers[0].texpaint.g4.begin();
		layers[0].texpaint.g4.clear(kha.Color.fromFloats(defaultBase, defaultBase, defaultBase, 0.0)); // Base
		layers[0].texpaint.g4.end();

		layers[0].texpaint_nor.g4.begin();
		layers[0].texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		layers[0].texpaint_nor.g4.end();

		layers[0].texpaint_pack.g4.begin();
		layers[0].texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, defaultRough, 0.0, 0.0)); // Occ, rough, met
		layers[0].texpaint_pack.g4.end();

		g.begin();
		iron.App.removeRender(initLayers);

		Context.layerPreviewDirty = true;
		Context.ddirty = 3;
	}

	public static function resizeLayers(g: kha.graphics4.Graphics) {
		var C = Config.raw;
		if (App.resHandle.position >= 4) { // Save memory for >=16k
			C.undo_steps = 1;
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
		if (rts.get("texpaint_blur") != null) {
			rts.get("texpaint_blur").image.unload();
			var size = Std.int(Config.getTextureRes() * 0.95);
			rts.get("texpaint_blur").raw.width = size;
			rts.get("texpaint_blur").raw.height = size;
			rts.get("texpaint_blur").image = Image.createRenderTarget(size, size);
		}
		#if kha_direct3d12
		arm.render.RenderPathRaytrace.ready = false; // Rebuild baketex
		#end
		g.begin();
		Context.ddirty = 2;
		iron.App.removeRender(resizeLayers);
	}

	public static function setLayerBits(g: kha.graphics4.Graphics) {
		g.end();
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
		g.begin();
		iron.App.removeRender(setLayerBits);
	}

	public static function makePipe() {
		pipeMerge = new PipelineState();
		pipeMerge.vertexShader = Reflect.field(kha.Shaders, "layer_merge_vert");
		pipeMerge.fragmentShader = Reflect.field(kha.Shaders, "layer_merge_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeMerge.inputLayout = [vs];
		pipeMerge.compile();
		tex0 = pipeMerge.getTextureUnit("tex0"); // Always binding texpaint.a for blending
		tex1 = pipeMerge.getTextureUnit("tex1");
		texmask = pipeMerge.getTextureUnit("texmask");
		texa = pipeMerge.getTextureUnit("texa");
		opac = pipeMerge.getConstantLocation("opac");
		blending = pipeMerge.getConstantLocation("blending");

		pipeCopy = new PipelineState();
		pipeCopy.vertexShader = Reflect.field(kha.Shaders, "layer_view_vert");
		pipeCopy.fragmentShader = Reflect.field(kha.Shaders, "layer_view_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopy.inputLayout = [vs];
		pipeCopy.compile();

		pipeMask = new PipelineState();
		pipeMask.vertexShader = Reflect.field(kha.Shaders, "layer_merge_vert");
		pipeMask.fragmentShader = Reflect.field(kha.Shaders, "mask_merge_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeMask.inputLayout = [vs];
		pipeMask.compile();
		tex0Mask = pipeMask.getTextureUnit("tex0");
		texaMask = pipeMask.getTextureUnit("texa");
	}

	public static function makeCursorPipe() {
		pipeCursor = new PipelineState();
		pipeCursor.vertexShader = Reflect.field(kha.Shaders, "cursor_vert");
		pipeCursor.fragmentShader = Reflect.field(kha.Shaders, "cursor_frag");
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
			RenderPath.active.renderTargets.get("temptex0").unload();
			RenderPath.active.renderTargets.remove("temptex0");
			imga = null;
		}
		if (imga == null) {
			{
				var t = new RenderTargetRaw();
				t.name = "temptex0";
				t.width = l.texpaint.width;
				t.height = l.texpaint.height;
				t.format = "RGBA32";
				var rt = RenderPath.active.createRenderTarget(t);
				imga = rt.image;
			}
		}
	}

	public static function makeExportImg() {
		var l = Project.layers[0];
		if (expa != null && expa.width != l.texpaint.width) {
			expa.unload();
			expb.unload();
			expc.unload();
			expa = null;
			expb = null;
			expc = null;
		}
		if (expa == null) {
			expa = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expb = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
			expc = Image.createRenderTarget(l.texpaint.width, l.texpaint.height);
		}
	}

	public static function mergeSelectedLayer(g: kha.graphics4.Graphics) {
		if (pipeMerge == null) makePipe();

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

		// Merge into layer below
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

		imga.g2.begin(false); // Copy to temp
		imga.g2.pipeline = pipeCopy;
		imga.g2.drawImage(l0.texpaint, 0, 0);
		imga.g2.end();

		l0.texpaint.g4.begin();
		l0.texpaint.g4.setPipeline(pipeMerge);
		l0.texpaint.g4.setTexture(tex0, l1.texpaint);
		var empty = RenderPath.active.renderTargets.get("empty_white").image;
		l0.texpaint.g4.setTexture(tex1, empty);
		l0.texpaint.g4.setTexture(texmask, empty);
		l0.texpaint.g4.setTexture(texa, imga);
		l0.texpaint.g4.setFloat(opac, l1.maskOpacity);
		l0.texpaint.g4.setInt(blending, l1.blending);
		l0.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		l0.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		l0.texpaint.g4.drawIndexedVertices();
		l0.texpaint.g4.end();

		imga.g2.begin(false);
		imga.g2.pipeline = pipeCopy;
		imga.g2.drawImage(l0.texpaint_nor, 0, 0);
		imga.g2.end();

		l0.texpaint_nor.g4.begin();
		l0.texpaint_nor.g4.setPipeline(pipeMerge);
		l0.texpaint_nor.g4.setTexture(tex0, l1.texpaint);
		l0.texpaint_nor.g4.setTexture(tex1, l1.texpaint_nor);
		l0.texpaint_nor.g4.setTexture(texmask, empty);
		l0.texpaint_nor.g4.setTexture(texa, imga);
		l0.texpaint_nor.g4.setFloat(opac, l1.maskOpacity);
		l0.texpaint.g4.setInt(blending, -1);
		l0.texpaint_nor.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		l0.texpaint_nor.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		l0.texpaint_nor.g4.drawIndexedVertices();
		l0.texpaint_nor.g4.end();

		imga.g2.begin(false);
		imga.g2.pipeline = pipeCopy;
		imga.g2.drawImage(l0.texpaint_pack, 0, 0);
		imga.g2.end();

		l0.texpaint_pack.g4.begin();
		l0.texpaint_pack.g4.setPipeline(pipeMerge);
		l0.texpaint_pack.g4.setTexture(tex0, l1.texpaint);
		l0.texpaint_pack.g4.setTexture(tex1, l1.texpaint_pack);
		l0.texpaint_pack.g4.setTexture(texmask, empty);
		l0.texpaint_pack.g4.setTexture(texa, imga);
		l0.texpaint_pack.g4.setFloat(opac, l1.maskOpacity);
		l0.texpaint.g4.setInt(blending, -1);
		l0.texpaint_pack.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		l0.texpaint_pack.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		l0.texpaint_pack.g4.drawIndexedVertices();
		l0.texpaint_pack.g4.end();

		g.begin();

		Context.layer.delete();
		iron.App.removeRender(mergeSelectedLayer);
		Context.setLayer(l0);
		Context.layerPreviewDirty = true;
	}

	public static function isFillMaterial(): Bool {
		var m = Context.material;
		for (l in Project.layers) if (l.material_mask == m) return true;
		return false;
	}

	public static function updateFillLayers(fills = 1) {
		var layers = Project.layers;
		var selectedLayer = Context.layer;
		var isMask = Context.layerIsMask;
		var selectedTool = Context.tool;
		var current: kha.graphics4.Graphics2 = null;

		var first = true;
		for (l in layers) {
			if (l.material_mask == Context.material) {
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

	public static function newLayer(clear = true): LayerSlot {
		if (Project.layers.length > 255) return null;
		var l = new LayerSlot();
		Project.layers.push(l);
		Context.setLayer(l);
		if (clear) iron.App.notifyOnRender(l.clear);
		Context.layerPreviewDirty = true;
		return l;
	}

	public static function createFillLayer() {
		function makeFill(g: kha.graphics4.Graphics) {
			g.end();
			var l = newLayer(false);
			History.newLayer();
			l.objectMask = UITrait.inst.layerFilter;
			History.toFillLayer();
			l.toFillLayer();
			g.begin();
			iron.App.removeRender(makeFill);
		}
		iron.App.notifyOnRender(makeFill);
	}

	public static function createImageMask(asset: TAsset) {
		var l = Context.layer;
		if (l != Project.layers[0]) {
			History.newMask();
			l.createMask(0x00000000, true, UITrait.inst.getImage(asset));
			Context.setLayer(l, true);
			Context.layerPreviewDirty = true;
		}
	}
}
