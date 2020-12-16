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
import iron.math.Mat4;
import arm.ui.UISidebar;
import arm.ui.UIHeader;
import arm.data.LayerSlot;
import arm.node.MakeMaterial;
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

	public static function initLayers() {
		Project.layers[0].clearLayer(kha.Color.fromFloats(defaultBase, defaultBase, defaultBase, 1.0));
	}

	public static function resizeLayers() {
		var C = Config.raw;
		if (App.resHandle.position >= Std.int(Res16384)) { // Save memory for >=16k
			C.undo_steps = 1;
			if (Context.undoHandle != null) {
				Context.undoHandle.value = C.undo_steps;
			}
			while (History.undoLayers.length > C.undo_steps) {
				var l = History.undoLayers.pop();
				App.notifyOnNextFrame(function() {
					l.unload();
				});
			}
		}
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
		var rts = RenderPath.active.renderTargets;
		var _texpaint_blend0 = rts.get("texpaint_blend0").image;
		App.notifyOnNextFrame(function() {
			_texpaint_blend0.unload();
		});
		rts.get("texpaint_blend0").raw.width = Config.getTextureResX();
		rts.get("texpaint_blend0").raw.height = Config.getTextureResY();
		rts.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		var _texpaint_blend1 = rts.get("texpaint_blend1").image;
		App.notifyOnNextFrame(function() {
			_texpaint_blend1.unload();
		});
		rts.get("texpaint_blend1").raw.width = Config.getTextureResX();
		rts.get("texpaint_blend1").raw.height = Config.getTextureResY();
		rts.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		Context.brushBlendDirty = true;
		if (rts.get("texpaint_blur") != null) {
			var _texpaint_blur = rts.get("texpaint_blur").image;
			App.notifyOnNextFrame(function() {
				_texpaint_blur.unload();
			});
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
		Context.ddirty = 2;
	}

	public static function setLayerBits() {
		for (l in Project.layers) l.resizeAndSetBits();
		for (l in History.undoLayers) l.resizeAndSetBits();
	}

	static function makeMergePipe(red: Bool, green: Bool, blue: Bool, alpha: Bool): PipelineState {
		var pipe = new PipelineState();
		pipe.vertexShader = kha.Shaders.getVertex("layer_merge.vert");
		pipe.fragmentShader = kha.Shaders.getFragment("layer_merge.frag");
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
		pipeCopy.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopy.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopy.inputLayout = [vs];
		pipeCopy.compile();

		pipeCopyBGRA = new PipelineState();
		pipeCopyBGRA.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyBGRA.fragmentShader = kha.Shaders.getFragment("layer_copy_bgra.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopyBGRA.inputLayout = [vs];
		pipeCopyBGRA.compile();

		#if (kha_metal || kha_vulkan || kha_direct3d12)
		pipeCopy8 = new PipelineState();
		pipeCopy8.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopy8.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeCopy8.inputLayout = [vs];
		pipeCopy8.colorAttachmentCount = 1;
		pipeCopy8.colorAttachments[0] = TextureFormat.L8;
		pipeCopy8.compile();

		pipeCopy128 = new PipelineState();
		pipeCopy128.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopy128.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
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
		pipeInvert8.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeInvert8.fragmentShader = kha.Shaders.getFragment("layer_invert.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.Float4);
		pipeInvert8.inputLayout = [vs];
		pipeCopy8.colorAttachmentCount = 1;
		pipeCopy8.colorAttachments[0] = TextureFormat.L8;
		pipeInvert8.compile();

		pipeMask = new PipelineState();
		pipeMask.vertexShader = kha.Shaders.getVertex("layer_merge.vert");
		pipeMask.fragmentShader = kha.Shaders.getFragment("mask_merge.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeMask.inputLayout = [vs];
		pipeMask.compile();
		tex0Mask = pipeMask.getTextureUnit("tex0");
		texaMask = pipeMask.getTextureUnit("texa");
	}

	public static function makeCursorPipe() {
		pipeCursor = new PipelineState();
		pipeCursor.vertexShader = kha.Shaders.getVertex("cursor.vert");
		pipeCursor.fragmentShader = kha.Shaders.getFragment("cursor.frag");
		var vs = new VertexStructure();
		#if (kha_metal || kha_vulkan)
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
		if (imga != null && (imga.width != l.texpaint.width || imga.height != l.texpaint.height || imga.format != l.texpaint.format)) {
			var _temptex0 = RenderPath.active.renderTargets.get("temptex0");
			App.notifyOnNextFrame(function() {
				_temptex0.unload();
			});
			RenderPath.active.renderTargets.remove("temptex0");
			imga = null;
		}
		if (imga == null) {
			var format = App.bitsHandle.position == Bits8  ? "RGBA32" :
					 	 App.bitsHandle.position == Bits16 ? "RGBA64" :
					 										 "RGBA128";
			var t = new RenderTargetRaw();
			t.name = "temptex0";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			imga = rt.image;
		}
	}

	public static function makeExportImg() {
		var l = Project.layers[0];
		if (expa != null && (expa.width != l.texpaint.width || expa.height != l.texpaint.height || expa.format != l.texpaint.format)) {
			var _expa = expa;
			var _expb = expb;
			var _expc = expc;
			App.notifyOnNextFrame(function() {
				_expa.unload();
				_expb.unload();
				_expc.unload();
			});
			expa = null;
			expb = null;
			expc = null;
			RenderPath.active.renderTargets.remove("expa");
			RenderPath.active.renderTargets.remove("expb");
			RenderPath.active.renderTargets.remove("expc");
		}
		if (expa == null) {
			var format = App.bitsHandle.position == Bits8  ? "RGBA32" :
					 	 App.bitsHandle.position == Bits16 ? "RGBA64" :
					 										 "RGBA128";
			var t = new RenderTargetRaw();
			t.name = "expa";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			expa = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expb";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			expb = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expc";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			expc = rt.image;
		}
	}

	public static function mergeDown() {
		var l0 = Project.layers[0];
		var l1 = Context.layer;
		for (i in 1...Project.layers.length) {
			if (Project.layers[i] == l1) {
				l0 = Project.layers[i - 1];
				break;
			}
		}

		if (l1.texpaint_mask != null) {
			l1.applyMask();
		}

		mergeLayer(l0, l1);

		Context.layer.delete();
		Context.setLayer(l0);
		Context.layerPreviewDirty = true;
	}

	public static function mergeLayer(l0 : LayerSlot, l1: LayerSlot, use_mask = false) {
		if (!l1.visible) return;

		if (pipeMerge == null) makePipe();
		makeTempImg();

		// Merge into layer below
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

		imga.g2.begin(false); // Copy to temp
		imga.g2.pipeline = pipeCopy;
		imga.g2.drawImage(l0.texpaint, 0, 0);
		imga.g2.pipeline = null;
		imga.g2.end();

		var empty = RenderPath.active.renderTargets.get("empty_white").image;
		var mask = (use_mask && l1.texpaint_mask != null) ? l1.texpaint_mask : empty;

		if (l1.paintBase) {
			l0.texpaint.g4.begin();
			l0.texpaint.g4.setPipeline(pipeMerge);
			l0.texpaint.g4.setTexture(tex0, l1.texpaint);
			l0.texpaint.g4.setTexture(tex1, empty);
			l0.texpaint.g4.setTexture(texmask, mask);
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
			l0.texpaint_nor.g4.setTexture(texmask, mask);
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
				commandsMergePack(pipeMerge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, mask);
			}
			else {
				if (l1.paintOcc) commandsMergePack(pipeMergeR, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, mask);
				if (l1.paintRough) commandsMergePack(pipeMergeG, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, mask);
				if (l1.paintMet) commandsMergePack(pipeMergeB, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.maskOpacity, mask);
			}
		}
	}

	public static function applyMask(l: LayerSlot) {
		if (l.texpaint_mask == null) return;

		if (Layers.pipeMerge == null) Layers.makePipe();
		Layers.makeTempImg();

		// Copy layer to temp
		Layers.imga.g2.begin(false);
		Layers.imga.g2.pipeline = Layers.pipeCopy;
		Layers.imga.g2.drawImage(l.texpaint, 0, 0);
		Layers.imga.g2.pipeline = null;
		Layers.imga.g2.end();

		// Merge mask
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		l.texpaint.g4.begin();
		l.texpaint.g4.setPipeline(Layers.pipeMask);
		l.texpaint.g4.setTexture(Layers.tex0Mask, Layers.imga);
		l.texpaint.g4.setTexture(Layers.texaMask, l.texpaint_mask);
		l.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		l.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		l.texpaint.g4.drawIndexedVertices();
		l.texpaint.g4.end();
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
		for (l in Project.layers) if (l.fill_layer == m) return true;
		for (l in Project.layers) if (l.fill_mask == m) return true;
		return false;
	}

	public static function updateFillLayers() {
		var layers = Project.layers;
		var selectedLayer = Context.layer;
		var isMask = Context.layerIsMask;
		var selectedTool = Context.tool;
		var current: kha.graphics2.Graphics = null;

		if (UIHeader.inst.worktab.position == SpaceMaterial) {
			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
				RenderPathPaint.liveLayer.createMask(0x00000000);
			}

			current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();

			UIHeader.inst.worktab.position = SpacePaint;
			Context.tool = ToolFill;
			MakeMaterial.parsePaintMaterial();
			Context.pdirty = 1;
			RenderPathPaint.useLiveLayer(true);
			RenderPathPaint.commandsPaint(false);
			RenderPathPaint.dilate(true, true);
			RenderPathPaint.useLiveLayer(false);
			Context.tool = selectedTool;
			Context.pdirty = 0;
			Context.rdirty = 2;
			UIHeader.inst.worktab.position = SpaceMaterial;

			if (current != null) current.begin(false);
			return;
		}

		var hasFillLayer = false;
		var hasFillMask = false;
		for (l in layers) if (l.fill_layer == Context.material) hasFillLayer = true;
		for (l in layers) if (l.fill_mask == Context.material) hasFillMask = true;

		if (hasFillLayer || hasFillMask) {

			current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();
			Context.pdirty = 1;
			Context.tool = ToolFill;

			if (hasFillLayer) {
				Context.layerIsMask = false;
				MakeMaterial.parsePaintMaterial();

				for (l in layers) {
					if (l.fill_layer == Context.material) {
						Context.layer = l;
						setObjectMask();

						// Decal layer
						if (l.uvType == UVProject && !Context.layerIsMask) {
							l.clearLayer();
						}
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}

			if (hasFillMask) {
				Context.layerIsMask = true;
				MakeMaterial.parsePaintMaterial();

				for (l in layers) {
					if (l.fill_mask == Context.material) {
						Context.layer = l;
						setObjectMask();
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}

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

	public static function updateFillLayer(parsePaint = true) {
		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		var _tool = Context.tool;
		Context.tool = ToolFill;
		Context.pdirty = 1;
		Context.layerIsMask = false;
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = SpacePaint;

		// Decal layer
		if (Context.layer.uvType == UVProject && !Context.layerIsMask) {
			Context.layer.clearLayer();
		}

		if (parsePaint) MakeMaterial.parsePaintMaterial();
		RenderPathPaint.commandsPaint(false);
		RenderPathPaint.dilate(true, true);

		Context.rdirty = 2;
		Context.tool = _tool;
		UIHeader.inst.worktab.position = _workspace;
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
		if (clear) iron.App.notifyOnInit(l.clear);
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

	public static function createFillLayer(uvType = UVMap, decalMat: Mat4 = null) {
		function _init() {
			var l = newLayer(false);
			History.newLayer();
			l.uvType = uvType;
			if (decalMat != null) l.decalMat = decalMat;
			l.objectMask = Context.layerFilter;
			History.toFillLayer();
			l.toFillLayer();
		}
		iron.App.notifyOnInit(_init);
	}

	public static function createImageMask(asset: TAsset) {
		var l = Context.layer;
		if (l != Project.layers[0]) {
			History.newMask();
			l.createMask(0x00000000, true, Project.getImage(asset));
			Context.setLayer(l, true);
			Context.layerPreviewDirty = true;
		}
	}
}
