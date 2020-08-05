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
import arm.ui.UISidebar;
import arm.ui.UIHeader;
import arm.data.LayerSlot;
import arm.node.MaterialParser;
import arm.render.RenderPathPaint;
import arm.util.MeshUtil;
import arm.Enums;
import arm.ProjectFormat;

class Layers {

	public static var pipeMerge: PipelineState = null;
	public static var pipeMergeR: PipelineState = null;
	public static var pipeMergeG: PipelineState = null;
	public static var pipeMergeB: PipelineState = null;
	public static var pipeMergeA: PipelineState = null;
	public static var pipeCopy: PipelineState;
	public static var pipeCopy8: PipelineState;
	public static var pipeCopy128: PipelineState;
	public static var pipeCopyBGRA: PipelineState;
	public static var pipeInvert8: PipelineState;
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
	public static var cursorTexStep: ConstantLocation;
	public static var cursorRadius: ConstantLocation;
	public static var cursorCameraRight: ConstantLocation;
	public static var cursorTint: ConstantLocation;
	public static var cursorTex: TextureUnit;
	public static var cursorGbufferD: TextureUnit;

	public static inline var defaultBase = 0.5;
	public static inline var defaultRough = 0.4;

	public static function initLayers(g: kha.graphics4.Graphics) {
		g.end();
		Project.layers[0].clearLayer(kha.Color.fromFloats(defaultBase, defaultBase, defaultBase, 1.0));
		g.begin();
		iron.App.removeRender(initLayers);
	}

	public static function resizeLayers(g: kha.graphics4.Graphics) {
		var C = Config.raw;
		if (App.resHandle.position >= Std.int(Res16384)) { // Save memory for >=16k
			C.undo_steps = 1;
			if (Context.undoHandle != null) Context.undoHandle.value = C.undo_steps;
			while (History.undoLayers.length > C.undo_steps) { var l = History.undoLayers.pop(); l.unload(); }
		}
		g.end();
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
		var rts = RenderPath.active.renderTargets;
		rts.get("texpaint_blend0").image.unload();
		rts.get("texpaint_blend0").raw.width = Config.getTextureResX();
		rts.get("texpaint_blend0").raw.height = Config.getTextureResY();
		rts.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		rts.get("texpaint_blend1").image.unload();
		rts.get("texpaint_blend1").raw.width = Config.getTextureResX();
		rts.get("texpaint_blend1").raw.height = Config.getTextureResY();
		rts.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		Context.brushBlendDirty = true;
		if (rts.get("texpaint_blur") != null) {
			rts.get("texpaint_blur").image.unload();
			var sizeX = Std.int(Config.getTextureResX() * 0.95);
			var sizeY = Std.int(Config.getTextureResY() * 0.95);
			rts.get("texpaint_blur").raw.width = sizeX;
			rts.get("texpaint_blur").raw.height = sizeY;
			rts.get("texpaint_blur").image = Image.createRenderTarget(sizeX, sizeY);
		}
		if (RenderPathPaint.liveLayer != null) RenderPathPaint.liveLayer.resizeAndSetBits();
		#if (kha_direct3d12 || kha_vulkan)
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

	static function makeMergePipe(red: Bool, green: Bool, blue: Bool, alpha: Bool): PipelineState {
		var pipe = new PipelineState();
		pipe.vertexShader = Reflect.field(kha.Shaders, "layer_merge_vert");
		pipe.fragmentShader = Reflect.field(kha.Shaders, "layer_merge_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipe.inputLayout = [vs];
		pipe.colorWriteMasksRed = [red];
		pipe.colorWriteMasksGreen = [green];
		pipe.colorWriteMasksBlue = [blue];
		pipe.colorWriteMasksAlpha = [alpha];
		pipe.compile();
		return pipe;
	}

	public static function makePipe() {
		pipeMerge = makeMergePipe(true, true, true, true);
		pipeMergeR = makeMergePipe(true, false, false, false);
		pipeMergeG = makeMergePipe(false, true, false, false);
		pipeMergeB = makeMergePipe(false, false, true, false);
		pipeMergeA = makeMergePipe(false, false, false, true);
		tex0 = pipeMerge.getTextureUnit("tex0"); // Always binding texpaint.a for blending
		tex1 = pipeMerge.getTextureUnit("tex1");
		texmask = pipeMerge.getTextureUnit("texmask");
		texa = pipeMerge.getTextureUnit("texa");
		opac = pipeMerge.getConstantLocation("opac");
		blending = pipeMerge.getConstantLocation("blending");

		pipeCopy = new PipelineState();
		pipeCopy.vertexShader = Reflect.field(kha.Shaders, "layer_view_vert");
		pipeCopy.fragmentShader = Reflect.field(kha.Shaders, "layer_copy_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopy.inputLayout = [vs];
		pipeCopy.compile();

		pipeCopyBGRA = new PipelineState();
		pipeCopyBGRA.vertexShader = Reflect.field(kha.Shaders, "layer_view_vert");
		pipeCopyBGRA.fragmentShader = Reflect.field(kha.Shaders, "layer_copy_bgra_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopyBGRA.inputLayout = [vs];
		pipeCopyBGRA.compile();

		#if (kha_metal || kha_vulkan)
		pipeCopy8 = new PipelineState();
		pipeCopy8.vertexShader = Reflect.field(kha.Shaders, "layer_view_vert");
		pipeCopy8.fragmentShader = Reflect.field(kha.Shaders, "layer_copy_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopy8.inputLayout = [vs];
		pipeCopy8.colorAttachmentCount = 1;
		pipeCopy8.colorAttachments[0] = TextureFormat.L8;
		pipeCopy8.compile();

		pipeCopy128 = new PipelineState();
		pipeCopy128.vertexShader = Reflect.field(kha.Shaders, "layer_view_vert");
		pipeCopy128.fragmentShader = Reflect.field(kha.Shaders, "layer_copy_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopy128.inputLayout = [vs];
		pipeCopy128.colorAttachmentCount = 1;
		pipeCopy128.colorAttachments[0] = TextureFormat.RGBA128;
		pipeCopy128.compile();
		#else
		pipeCopy8 = pipeCopy;
		pipeCopy128 = pipeCopy;
		#end

		pipeInvert8 = new PipelineState();
		pipeInvert8.vertexShader = Reflect.field(kha.Shaders, "layer_view_vert");
		pipeInvert8.fragmentShader = Reflect.field(kha.Shaders, "layer_invert_frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeInvert8.inputLayout = [vs];
		pipeCopy8.colorAttachmentCount = 1;
		pipeCopy8.colorAttachments[0] = TextureFormat.L8;
		pipeInvert8.compile();

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
		#if kha_metal
		vs.add("tex", VertexData.Short2Norm);
		#else
		vs.add("pos", VertexData.Short4Norm);
		vs.add("nor", VertexData.Short2Norm);
		vs.add("tex", VertexData.Short2Norm);
		#end
		pipeCursor.inputLayout = [vs];
		pipeCursor.blendSource = BlendingFactor.SourceAlpha;
		pipeCursor.blendDestination = BlendingFactor.InverseSourceAlpha;
		pipeCursor.depthWrite = false;
		pipeCursor.depthMode = CompareMode.Always;
		pipeCursor.compile();
		cursorVP = pipeCursor.getConstantLocation("VP");
		cursorInvVP = pipeCursor.getConstantLocation("invVP");
		cursorMouse = pipeCursor.getConstantLocation("mouse");
		cursorTexStep = pipeCursor.getConstantLocation("texStep");
		cursorRadius = pipeCursor.getConstantLocation("radius");
		cursorCameraRight = pipeCursor.getConstantLocation("cameraRight");
		cursorTint = pipeCursor.getConstantLocation("tint");
		cursorGbufferD = pipeCursor.getTextureUnit("gbufferD");
		cursorTex = pipeCursor.getTextureUnit("tex");
	}

	public static function makeTempImg() {
		var l = Project.layers[0];
		if (imga != null && (imga.width != l.texpaint.width || imga.height != l.texpaint.height)) {
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
		if (expa != null && (expa.width != l.texpaint.width || expa.height != l.texpaint.height)) {
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
		imga.g2.pipeline = null;
		imga.g2.end();

		var empty = RenderPath.active.renderTargets.get("empty_white").image;

		if (l1.paintBase) {
			l0.texpaint.g4.begin();
			l0.texpaint.g4.setPipeline(pipeMerge);
			l0.texpaint.g4.setTexture(tex0, l1.texpaint);
			l0.texpaint.g4.setTexture(tex1, empty);
			l0.texpaint.g4.setTexture(texmask, empty);
			l0.texpaint.g4.setTexture(texa, imga);
			l0.texpaint.g4.setFloat(opac, l1.maskOpacity);
			l0.texpaint.g4.setInt(blending, l1.blending);
			l0.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
			l0.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
			l0.texpaint.g4.drawIndexedVertices();
			l0.texpaint.g4.end();
		}

		imga.g2.begin(false);
		imga.g2.pipeline = pipeCopy;
		imga.g2.drawImage(l0.texpaint_nor, 0, 0);
		imga.g2.pipeline = null;
		imga.g2.end();

		if (l1.paintNor) {
			l0.texpaint_nor.g4.begin();
			l0.texpaint_nor.g4.setPipeline(pipeMerge);
			l0.texpaint_nor.g4.setTexture(tex0, l1.texpaint);
			l0.texpaint_nor.g4.setTexture(tex1, l1.texpaint_nor);
			l0.texpaint_nor.g4.setTexture(texmask, empty);
			l0.texpaint_nor.g4.setTexture(texa, imga);
			l0.texpaint_nor.g4.setFloat(opac, l1.maskOpacity);
			l0.texpaint_nor.g4.setInt(blending, -1);
			l0.texpaint_nor.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
			l0.texpaint_nor.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
			l0.texpaint_nor.g4.drawIndexedVertices();
			l0.texpaint_nor.g4.end();
		}

		imga.g2.begin(false);
		imga.g2.pipeline = pipeCopy;
		imga.g2.drawImage(l0.texpaint_pack, 0, 0);
		imga.g2.pipeline = null;
		imga.g2.end();

		if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
			if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
				commandsMergePack(pipeMerge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, empty);
			}
			else {
				if (l1.paintOcc) commandsMergePack(pipeMergeR, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, empty);
				if (l1.paintRough) commandsMergePack(pipeMergeG, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, empty);
				if (l1.paintMet) commandsMergePack(pipeMergeB, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, empty);
			}
		}

		g.begin();

		Context.layer.delete();
		iron.App.removeRender(mergeSelectedLayer);
		Context.setLayer(l0);
		Context.layerPreviewDirty = true;
	}

	public static function commandsMergePack(pipe: PipelineState, i0: kha.Image, i1: kha.Image, i1pack: kha.Image, i1maskOpacity: Float, i1texmask: kha.Image) {
		i0.g4.begin();
		i0.g4.setPipeline(pipe);
		i0.g4.setTexture(tex0, i1);
		i0.g4.setTexture(tex1, i1pack);
		i0.g4.setTexture(texmask, i1texmask);
		i0.g4.setTexture(texa, imga);
		i0.g4.setFloat(opac, i1maskOpacity);
		i0.g4.setInt(blending, -1);
		i0.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		i0.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		i0.g4.drawIndexedVertices();
		i0.g4.end();
	}

	public static function isFillMaterial(): Bool {
		if (UIHeader.inst.worktab.position == SpaceMaterial) return true;
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

		if (UIHeader.inst.worktab.position == SpaceMaterial) {
			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
				RenderPathPaint.liveLayer.createMask(0x00000000);
			}

			current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			UIHeader.inst.worktab.position = SpacePaint;
			Context.tool = ToolFill;
			MaterialParser.parsePaintMaterial();
			Context.pdirty = 1;
			RenderPathPaint.useLiveLayer(true);
			RenderPathPaint.commandsPaint();
			RenderPathPaint.useLiveLayer(false);
			Context.tool = selectedTool;
			Context.pdirty = 0;
			Context.rdirty = 2;
			UIHeader.inst.worktab.position = SpaceMaterial;

			if (current != null) current.begin(false);
			return;
		}

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
					MaterialParser.parsePaintMaterial();
					first = false;
				}

				// Decal layer
				if (l.uvType == UVProject) {
					l.clearLayer();
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

	public static function updateFillLayer(fills = 1) {
		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();

		var selectedTool = Context.tool;
		Context.pdirty = fills;
		Context.layerIsMask = false;
		Context.tool = ToolFill;

		// Decal layer
		if (Context.layer.uvType == UVProject) {
			Context.layer.clearLayer();
		}

		MaterialParser.parsePaintMaterial();

		for (i in 0...fills) {
			RenderPathPaint.commandsPaint();
		}

		Context.rdirty = 2;
		Context.tool = selectedTool;
		if (current != null) current.begin(false);
	}

	public static function setObjectMask() {
		var ar = ["None"];
		for (p in Project.paintObjects) ar.push(p.name);

		var mask = Context.layer.objectMask;
		if (Context.layerFilter > 0) mask = Context.layerFilter;
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
		l.objectMask = Context.layerFilter;
		Project.layers.push(l);
		Context.setLayer(l);
		if (clear) iron.App.notifyOnRender(l.clear);
		Context.layerPreviewDirty = true;
		return l;
	}

	public static function newGroup(): LayerSlot {
		if (Project.layers.length > 255) return null;
		var l = new LayerSlot("", true);
		Project.layers.push(l);
		Context.setLayer(l);
		return l;
	}

	public static function createFillLayer() {
		function makeFill(g: kha.graphics4.Graphics) {
			g.end();
			var l = newLayer(false);
			History.newLayer();
			l.objectMask = Context.layerFilter;
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
			l.createMask(0x00000000, true, UISidebar.inst.getImage(asset));
			Context.setLayer(l, true);
			Context.layerPreviewDirty = true;
		}
	}
}
