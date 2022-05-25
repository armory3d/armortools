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
import arm.ui.UIHeader;
import arm.data.LayerSlot;
import arm.node.MakeMaterial;
import arm.render.RenderPathPaint;
import arm.util.MeshUtil;
import arm.util.UVUtil;
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
	public static var pipeCopyRGB: PipelineState = null;
	public static var pipeInvert8: PipelineState;
	public static var pipeApplyMask: PipelineState;
	public static var pipeMergeMask: PipelineState;
	public static var tex0: TextureUnit;
	public static var tex1: TextureUnit;
	public static var texmask: TextureUnit;
	public static var texa: TextureUnit;
	public static var opac: ConstantLocation;
	public static var blending: ConstantLocation;
	public static var tex0Mask: TextureUnit;
	public static var texaMask: TextureUnit;
	public static var tex0MergeMask: TextureUnit;
	public static var texaMergeMask: TextureUnit;
	public static var opacMergeMask: ConstantLocation;
	public static var blendingMergeMask: ConstantLocation;
	public static var tempImage: Image = null;
	public static var tempMaskImage: Image = null;
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
	#if (krom_android || krom_ios)
	public static inline var maxLayers = 18;
	#else
	public static inline var maxLayers = 255;
	#end

	public static function initLayers() {
		Project.layers[0].clear(kha.Color.fromFloats(defaultBase, defaultBase, defaultBase, 1.0));
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
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopy.inputLayout = [vs];
		pipeCopy.compile();

		pipeCopyBGRA = new PipelineState();
		pipeCopyBGRA.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyBGRA.fragmentShader = kha.Shaders.getFragment("layer_copy_bgra.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyBGRA.inputLayout = [vs];
		pipeCopyBGRA.compile();

		#if (kha_metal || kha_vulkan || kha_direct3d12)
		pipeCopy8 = new PipelineState();
		pipeCopy8.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopy8.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
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
		vs.add("col", VertexData.UInt8_4X_Normalized);
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
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeInvert8.inputLayout = [vs];
		pipeInvert8.colorAttachmentCount = 1;
		pipeInvert8.colorAttachments[0] = TextureFormat.L8;
		pipeInvert8.compile();

		pipeApplyMask = new PipelineState();
		pipeApplyMask.vertexShader = kha.Shaders.getVertex("layer_merge.vert");
		pipeApplyMask.fragmentShader = kha.Shaders.getFragment("mask_apply.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeApplyMask.inputLayout = [vs];
		pipeApplyMask.compile();
		tex0Mask = pipeApplyMask.getTextureUnit("tex0");
		texaMask = pipeApplyMask.getTextureUnit("texa");

		pipeMergeMask = new PipelineState();
		pipeMergeMask.vertexShader = kha.Shaders.getVertex("layer_merge.vert");
		pipeMergeMask.fragmentShader = kha.Shaders.getFragment("mask_merge.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeMergeMask.inputLayout = [vs];
		pipeMergeMask.compile();
		tex0MergeMask = pipeMergeMask.getTextureUnit("tex0");
		texaMergeMask = pipeMergeMask.getTextureUnit("texa");
		opacMergeMask = pipeMergeMask.getConstantLocation("opac");
		blendingMergeMask = pipeMergeMask.getConstantLocation("blending");
	}

	public static function makePipeCopyRGB() {
		pipeCopyRGB = new PipelineState();
		pipeCopyRGB.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyRGB.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyRGB.inputLayout = [vs];
		pipeCopyRGB.colorWriteMasksAlpha = [false];
		pipeCopyRGB.compile();
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
		if (tempImage != null && (tempImage.width != l.texpaint.width || tempImage.height != l.texpaint.height || tempImage.format != l.texpaint.format)) {
			var _temptex0 = RenderPath.active.renderTargets.get("temptex0");
			App.notifyOnNextFrame(function() {
				_temptex0.unload();
			});
			RenderPath.active.renderTargets.remove("temptex0");
			tempImage = null;
		}
		if (tempImage == null) {
			var format = App.bitsHandle.position == Bits8  ? "RGBA32" :
					 	 App.bitsHandle.position == Bits16 ? "RGBA64" :
					 										 "RGBA128";
			var t = new RenderTargetRaw();
			t.name = "temptex0";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = format;
			var rt = RenderPath.active.createRenderTarget(t);
			tempImage = rt.image;
		}
	}

	public static function makeTempMaskImg() {
		if (tempMaskImage != null && (tempMaskImage.width != Config.getTextureResX() || tempMaskImage.height != Config.getTextureResY())) {
			var _tempMaskImage = tempMaskImage;
			App.notifyOnNextFrame(function() {
				_tempMaskImage.unload();
			});
			tempMaskImage = null;
		}
		if (tempMaskImage == null) {
			tempMaskImage = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
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

	public static function duplicateLayer(l: LayerSlot) {
		if (!l.isGroup()) {
			var newLayer = l.duplicate();
			Context.setLayer(newLayer);
			var masks = l.getMasks(false);
			if (masks != null) {
				for (m in masks) {
					m = m.duplicate();
					m.parent = newLayer;
					Project.layers.remove(m);
					Project.layers.insert(Project.layers.indexOf(newLayer), m);
				}
			}
			Context.setLayer(newLayer);
		}
		else {
			var newGroup = Layers.newGroup();
			Project.layers.remove(newGroup);
			Project.layers.insert(Project.layers.indexOf(l) + 1, newGroup);
			// group.show_panel = true;
			for (c in l.getChildren()) {
				var masks = c.getMasks(false);
				var newLayer = c.duplicate();
				newLayer.parent = newGroup;
				Project.layers.remove(newLayer);
				Project.layers.insert(Project.layers.indexOf(newGroup), newLayer);
				if (masks != null) {
					for (m in masks) {
						var newMask = m.duplicate();
						newMask.parent = newLayer;
						Project.layers.remove(newMask);
						Project.layers.insert(Project.layers.indexOf(newLayer), newMask);
					}
				}
			}
			var groupMasks = l.getMasks();
			if (groupMasks != null) {
				for (m in groupMasks) {
					var newMask = m.duplicate();
					newMask.parent = newGroup;
					Project.layers.remove(newMask);
					Project.layers.insert(Project.layers.indexOf(newGroup), newMask);
				}
			}
			Context.setLayer(newGroup);
		}
	}

	public static function applyMasks(l: LayerSlot) {
		var masks = l.getMasks();

		if (masks != null) {
			for (i in 0...masks.length - 1) {
				mergeLayer(masks[i + 1], masks[i]);
				masks[i].delete();
			}
			masks[masks.length - 1].applyMask();
			Context.layerPreviewDirty = true;
		}
	}

	public static function mergeDown() {
		var l1 = Context.layer;

		if (l1.isGroup()) {
			l1 = mergeGroup(l1);
		}
		else if (l1.hasMasks()) { // It is a layer
			applyMasks(l1);
			Context.setLayer(l1);
		}

		var l0 = Project.layers[Project.layers.indexOf(l1) - 1];

		if (l0.isGroup()) {
			l0 = mergeGroup(l0);
		}
		else if (l0.hasMasks()) { // It is a layer
			applyMasks(l0);
			Context.setLayer(l0);
		}

		mergeLayer(l0, l1);
		l1.delete();
		Context.setLayer(l0);
		Context.layerPreviewDirty = true;
	}

	public static function mergeGroup(l: LayerSlot) {
		if (!l.isGroup()) return null;

		var children = l.getChildren();

		if (children.length == 1 && children[0].hasMasks(false)) {
			Layers.applyMasks(children[0]);
		}

		for (i in 0...children.length - 1) {
			Context.setLayer(children[children.length - 1 - i]);
			History.mergeLayers();
			Layers.mergeDown();
		}

		// Now apply the group masks
		var masks = l.getMasks();
		if (masks != null) {
			for (i in 0...masks.length - 1) {
				mergeLayer(masks[i + 1], masks[i]);
				masks[i].delete();
			}
			Layers.applyMask(children[0], masks[masks.length - 1]);
		}

		children[0].parent = null;
		children[0].name = l.name;
		if (children[0].fill_layer != null) children[0].toPaintLayer();
		l.delete();
		return children[0];
	}

	public static function mergeLayer(l0 : LayerSlot, l1: LayerSlot, use_mask = false) {
		if (!l1.visible || l1.isGroup()) return;

		if (pipeMerge == null) makePipe();
		makeTempImg();
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

		tempImage.g2.begin(false); // Copy to temp
		tempImage.g2.pipeline = pipeCopy;
		tempImage.g2.drawImage(l0.texpaint, 0, 0);
		tempImage.g2.pipeline = null;
		tempImage.g2.end();

		var empty = RenderPath.active.renderTargets.get("empty_white").image;
		var mask = empty;
		var l1masks =  use_mask ? l1.getMasks() : null;
		if (l1masks != null) {
			// for (i in 1...l1masks.length - 1) {
			// 	mergeLayer(l1masks[i + 1], l1masks[i]);
			// }
			mask = l1masks[0].texpaint;
		}

		if (l1.isMask()) {
			l0.texpaint.g4.begin();
			l0.texpaint.g4.setPipeline(pipeMergeMask);
			l0.texpaint.g4.setTexture(Layers.tex0MergeMask, l1.texpaint);
			l0.texpaint.g4.setTexture(Layers.texaMergeMask, tempImage);
			l0.texpaint.g4.setFloat(opacMergeMask, l1.getOpacity());
			l0.texpaint.g4.setInt(blendingMergeMask, l1.blending);
			l0.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
			l0.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
			l0.texpaint.g4.drawIndexedVertices();
			l0.texpaint.g4.end();
		}

		if (l1.isLayer()) {
			if (l1.paintBase) {
				l0.texpaint.g4.begin();
				l0.texpaint.g4.setPipeline(pipeMerge);
				l0.texpaint.g4.setTexture(tex0, l1.texpaint);
				l0.texpaint.g4.setTexture(tex1, empty);
				l0.texpaint.g4.setTexture(texmask, mask);
				l0.texpaint.g4.setTexture(texa, tempImage);
				l0.texpaint.g4.setFloat(opac, l1.getOpacity());
				l0.texpaint.g4.setInt(blending, l1.blending);
				l0.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				l0.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				l0.texpaint.g4.drawIndexedVertices();
				l0.texpaint.g4.end();
			}

			tempImage.g2.begin(false);
			tempImage.g2.pipeline = pipeCopy;
			tempImage.g2.drawImage(l0.texpaint_nor, 0, 0);
			tempImage.g2.pipeline = null;
			tempImage.g2.end();

			if (l1.paintNor) {
				l0.texpaint_nor.g4.begin();
				l0.texpaint_nor.g4.setPipeline(pipeMerge);
				l0.texpaint_nor.g4.setTexture(tex0, l1.texpaint);
				l0.texpaint_nor.g4.setTexture(tex1, l1.texpaint_nor);
				l0.texpaint_nor.g4.setTexture(texmask, mask);
				l0.texpaint_nor.g4.setTexture(texa, tempImage);
				l0.texpaint_nor.g4.setFloat(opac, l1.getOpacity());
				l0.texpaint_nor.g4.setInt(blending, l1.paintNorBlend ? -2 : -1);
				l0.texpaint_nor.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				l0.texpaint_nor.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				l0.texpaint_nor.g4.drawIndexedVertices();
				l0.texpaint_nor.g4.end();
			}

			tempImage.g2.begin(false);
			tempImage.g2.pipeline = pipeCopy;
			tempImage.g2.drawImage(l0.texpaint_pack, 0, 0);
			tempImage.g2.pipeline = null;
			tempImage.g2.end();

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					commandsMergePack(pipeMerge, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) commandsMergePack(pipeMergeR, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintRough) commandsMergePack(pipeMergeG, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintMet) commandsMergePack(pipeMergeB, l0.texpaint_pack, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
				}
			}
		}
	}

	public static function flatten(heightToNormal = false, layers: Array<LayerSlot> = null): Dynamic {
		if (layers == null) layers = Project.layers;
		Layers.makeTempImg();
		Layers.makeExportImg();
		if (Layers.pipeMerge == null) Layers.makePipe();
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		var empty = iron.RenderPath.active.renderTargets.get("empty_white").image;

		// Clear export layer
		Layers.expa.g4.begin();
		Layers.expa.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0));
		Layers.expa.g4.end();
		Layers.expb.g4.begin();
		Layers.expb.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0));
		Layers.expb.g4.end();
		Layers.expc.g4.begin();
		Layers.expc.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0));
		Layers.expc.g4.end();

		// Flatten layers
		for (l1 in layers) {
			if (!l1.isVisible()) continue;
			if (!l1.isLayer()) continue;

			var mask = empty;
			var l1masks = l1.getMasks();
			if (l1masks != null) {
				if (l1masks.length > 1) {
					Layers.makeTempMaskImg();
					Layers.tempMaskImage.g2.begin(true, 0x00000000);
					Layers.tempMaskImage.g2.end();
					var l1 = { texpaint: Layers.tempMaskImage };
					for (i in 0...l1masks.length) {
						Layers.mergeLayer(untyped l1, l1masks[i]);
					}
					mask = Layers.tempMaskImage;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				Layers.tempImage.g2.begin(false); // Copy to temp
				Layers.tempImage.g2.pipeline = Layers.pipeCopy;
				Layers.tempImage.g2.drawImage(Layers.expa, 0, 0);
				Layers.tempImage.g2.pipeline = null;
				Layers.tempImage.g2.end();

				Layers.expa.g4.begin();
				Layers.expa.g4.setPipeline(Layers.pipeMerge);
				Layers.expa.g4.setTexture(Layers.tex0, l1.texpaint);
				Layers.expa.g4.setTexture(Layers.tex1, empty);
				Layers.expa.g4.setTexture(Layers.texmask, mask);
				Layers.expa.g4.setTexture(Layers.texa, Layers.tempImage);
				Layers.expa.g4.setFloat(Layers.opac, l1.getOpacity());
				Layers.expa.g4.setInt(Layers.blending, layers.length > 1 ? l1.blending : 0);
				Layers.expa.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				Layers.expa.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				Layers.expa.g4.drawIndexedVertices();
				Layers.expa.g4.end();
			}

			if (l1.paintNor) {
				Layers.tempImage.g2.begin(false);
				Layers.tempImage.g2.pipeline = Layers.pipeCopy;
				Layers.tempImage.g2.drawImage(Layers.expb, 0, 0);
				Layers.tempImage.g2.pipeline = null;
				Layers.tempImage.g2.end();

				Layers.expb.g4.begin();
				Layers.expb.g4.setPipeline(Layers.pipeMerge);
				Layers.expb.g4.setTexture(Layers.tex0, l1.texpaint);
				Layers.expb.g4.setTexture(Layers.tex1, l1.texpaint_nor);
				Layers.expb.g4.setTexture(Layers.texmask, mask);
				Layers.expb.g4.setTexture(Layers.texa, Layers.tempImage);
				Layers.expb.g4.setFloat(Layers.opac, l1.getOpacity());
				Layers.expb.g4.setInt(Layers.blending, l1.paintNorBlend ? -2 : -1);
				Layers.expb.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				Layers.expb.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				Layers.expb.g4.drawIndexedVertices();
				Layers.expb.g4.end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				Layers.tempImage.g2.begin(false);
				Layers.tempImage.g2.pipeline = Layers.pipeCopy;
				Layers.tempImage.g2.drawImage(Layers.expc, 0, 0);
				Layers.tempImage.g2.pipeline = null;
				Layers.tempImage.g2.end();

				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					Layers.commandsMergePack(Layers.pipeMerge, Layers.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) Layers.commandsMergePack(Layers.pipeMergeR, Layers.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintRough) Layers.commandsMergePack(Layers.pipeMergeG, Layers.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintMet) Layers.commandsMergePack(Layers.pipeMergeB, Layers.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
				}
			}
		}

		var l0 = { texpaint: Layers.expa, texpaint_nor: Layers.expb, texpaint_pack: Layers.expc };

		// Merge height map into normal map
		if (heightToNormal && MakeMaterial.heightUsed) {

			tempImage.g2.begin(false);
			tempImage.g2.pipeline = Layers.pipeCopy;
			tempImage.g2.drawImage(l0.texpaint_nor, 0, 0);
			tempImage.g2.pipeline = null;
			tempImage.g2.end();

			l0.texpaint_nor.g4.begin();
			l0.texpaint_nor.g4.setPipeline(Layers.pipeMerge);
			l0.texpaint_nor.g4.setTexture(Layers.tex0, tempImage);
			l0.texpaint_nor.g4.setTexture(Layers.tex1, l0.texpaint_pack);
			l0.texpaint_nor.g4.setTexture(Layers.texmask, empty);
			l0.texpaint_nor.g4.setTexture(Layers.texa, empty);
			l0.texpaint_nor.g4.setFloat(Layers.opac, 1.0);
			l0.texpaint_nor.g4.setInt(Layers.blending, -4);
			l0.texpaint_nor.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
			l0.texpaint_nor.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
			l0.texpaint_nor.g4.drawIndexedVertices();
			l0.texpaint_nor.g4.end();
		}

		return untyped l0;
	}

	public static function applyMask(l: LayerSlot, m: LayerSlot) {
		if (!l.isLayer() || !m.isMask()) return;

		if (Layers.pipeMerge == null) Layers.makePipe();
		Layers.makeTempImg();

		// Copy layer to temp
		tempImage.g2.begin(false);
		tempImage.g2.pipeline = Layers.pipeCopy;
		tempImage.g2.drawImage(l.texpaint, 0, 0);
		tempImage.g2.pipeline = null;
		tempImage.g2.end();

		// Apply mask
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		l.texpaint.g4.begin();
		l.texpaint.g4.setPipeline(Layers.pipeApplyMask);
		l.texpaint.g4.setTexture(Layers.tex0Mask, tempImage);
		l.texpaint.g4.setTexture(Layers.texaMask, m.texpaint);
		l.texpaint.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		l.texpaint.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		l.texpaint.g4.drawIndexedVertices();
		l.texpaint.g4.end();
	}

	public static function commandsMergePack(pipe: PipelineState, i0: kha.Image, i1: kha.Image, i1pack: kha.Image, i1maskOpacity: Float, i1texmask: kha.Image, i1blending = -1) {
		i0.g4.begin();
		i0.g4.setPipeline(pipe);
		i0.g4.setTexture(tex0, i1);
		i0.g4.setTexture(tex1, i1pack);
		i0.g4.setTexture(texmask, i1texmask);
		i0.g4.setTexture(texa, tempImage);
		i0.g4.setFloat(opac, i1maskOpacity);
		i0.g4.setInt(blending, i1blending);
		i0.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
		i0.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
		i0.g4.drawIndexedVertices();
		i0.g4.end();
	}

	public static function isFillMaterial(): Bool {
		if (UIHeader.inst.worktab.position == SpaceMaterial) return true;
		var m = Context.material;
		for (l in Project.layers) if (l.fill_layer == m) return true;
		return false;
	}

	public static function updateFillLayers() {
		var _layer = Context.layer;
		var _tool = Context.tool;
		var _fillType = Context.fillTypeHandle.position;
		var current: kha.graphics2.Graphics = null;

		if (UIHeader.inst.worktab.position == SpaceMaterial) {
			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
			}

			current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();

			UIHeader.inst.worktab.position = SpacePaint;
			Context.tool = ToolFill;
			Context.fillTypeHandle.position = FillObject;
			MakeMaterial.parsePaintMaterial(false);
			Context.pdirty = 1;
			RenderPathPaint.useLiveLayer(true);
			RenderPathPaint.commandsPaint(false);
			RenderPathPaint.dilate(true, true);
			RenderPathPaint.useLiveLayer(false);
			Context.tool = _tool;
			Context.fillTypeHandle.position = _fillType;
			Context.pdirty = 0;
			Context.rdirty = 2;
			UIHeader.inst.worktab.position = SpaceMaterial;

			if (current != null) current.begin(false);
			return;
		}

		var hasFillLayer = false;
		var hasFillMask = false;
		for (l in Project.layers) if (l.isLayer() && l.fill_layer == Context.material) hasFillLayer = true;
		for (l in Project.layers) if (l.isMask() && l.fill_layer == Context.material) hasFillMask = true;

		if (hasFillLayer || hasFillMask) {
			current = @:privateAccess kha.graphics2.Graphics.current;
			if (current != null) current.end();
			Context.pdirty = 1;
			Context.tool = ToolFill;
			Context.fillTypeHandle.position = FillObject;

			if (hasFillLayer) {
				var first = true;
				for (l in Project.layers) {
					if (l.isLayer() && l.fill_layer == Context.material) {
						Context.layer = l;
						if (first) {
							first = false;
							MakeMaterial.parsePaintMaterial(false);
						}
						setObjectMask();
						l.clear();
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}
			if (hasFillMask) {
				var first = true;
				for (l in Project.layers) {
					if (l.isMask() && l.fill_layer == Context.material) {
						Context.layer = l;
						if (first) {
							first = false;
							MakeMaterial.parsePaintMaterial(false);
						}
						setObjectMask();
						l.clear();
						RenderPathPaint.commandsPaint(false);
						RenderPathPaint.dilate(true, true);
					}
				}
			}

			Context.pdirty = 0;
			Context.ddirty = 2;
			Context.rdirty = 2;
			Context.layersPreviewDirty = true; // Repaint all layer previews as multiple layers might have changed.
			if (current != null) current.begin(false);
			Context.layer = _layer;
			setObjectMask();
			Context.tool = _tool;
			Context.fillTypeHandle.position = _fillType;
			MakeMaterial.parsePaintMaterial(false);
		}
	}

	public static function updateFillLayer(parsePaint = true) {
		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		var _tool = Context.tool;
		var _fillType = Context.fillTypeHandle.position;
		Context.tool = ToolFill;
		Context.fillTypeHandle.position = FillObject;
		Context.pdirty = 1;
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = SpacePaint;
		Context.layer.clear();

		if (parsePaint) MakeMaterial.parsePaintMaterial(false);
		RenderPathPaint.commandsPaint(false);
		RenderPathPaint.dilate(true, true);

		Context.rdirty = 2;
		Context.tool = _tool;
		Context.fillTypeHandle.position = _fillType;
		UIHeader.inst.worktab.position = _workspace;
		if (current != null) current.begin(false);
	}

	public static function setObjectMask() {
		var ar = [tr("None")];
		for (p in Project.paintObjects) ar.push(p.name);

		var mask = Context.objectMaskUsed() ? Context.layer.getObjectMask() : 0;
		if (Context.layerFilterUsed()) mask = Context.layerFilter;
		if (mask > 0) {
			if (Context.mergedObject != null) {
				Context.mergedObject.visible = false;
			}
			var o = Project.paintObjects[0];
			for (p in Project.paintObjects) {
				if (p.name == ar[mask]) {
					o = p;
					break;
				}
			}
			Context.selectPaintObject(o);
		}
		else {
			var isAtlas = Context.layer.getObjectMask() > 0 && Context.layer.getObjectMask() <= Project.paintObjects.length;
			if (Context.mergedObject == null || isAtlas || Context.mergedObjectIsAtlas) {
				var visibles = isAtlas ? Project.getAtlasObjects(Context.layer.getObjectMask()) : null;
				MeshUtil.mergeMesh(visibles);
			}
			Context.selectPaintObject(Context.mainObject());
			Context.paintObject.skip_context = "paint";
			Context.mergedObject.visible = true;
		}
		UVUtil.dilatemapCached = false;
	}

	public static function newLayer(clear = true): LayerSlot {
		if (Project.layers.length > maxLayers) return null;
		var l = new LayerSlot();
		l.objectMask = Context.layerFilter;
		if (Context.layer.isMask()) Context.setLayer(Context.layer.parent);
		Project.layers.insert(Project.layers.indexOf(Context.layer) + 1, l);
		Context.setLayer(l);
		var li = Project.layers.indexOf(Context.layer);
		if (li > 0) {
			var below = Project.layers[li - 1];
			if (below.isLayer()) {
				Context.layer.parent = below.parent;
			}
		}
		if (clear) iron.App.notifyOnInit(function() { l.clear(); });
		Context.layerPreviewDirty = true;
		return l;
	}

	public static function newMask(clear = true, parent: LayerSlot, position = -1): LayerSlot {
		if (Project.layers.length > maxLayers) return null;
		var l = new LayerSlot("", SlotMask, parent);
		if (position == -1) position = Project.layers.indexOf(parent);
		Project.layers.insert(position, l);
		Context.setLayer(l);
		if (clear) iron.App.notifyOnInit(function() { l.clear(); });
		Context.layerPreviewDirty = true;
		return l;
	}

	public static function newGroup(): LayerSlot {
		if (Project.layers.length > maxLayers) return null;
		var l = new LayerSlot("", SlotGroup);
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
		if (l.isMask() || l.isGroup()) {
			return;
		}

		History.newLayer();
		var m = Layers.newMask(false, l);
		m.clear(0x00000000, Project.getImage(asset));
		Context.layerPreviewDirty = true;
	}

	public static function createColorLayer(baseColor: Int, occlusion = 1.0, roughness = Layers.defaultRough, metallic = 0.0 ) {
		function _init() {
			var l = newLayer(false);
			History.newLayer();
			l.uvType = UVMap;
			l.objectMask = Context.layerFilter;
			l.clear(baseColor, occlusion, roughness, metallic);
		}
		iron.App.notifyOnInit(_init);
	}

	public static function onLayersResized() {
		iron.App.notifyOnInit(function() {
			Layers.resizeLayers();
			var _layer = Context.layer;
			var _material = Context.material;
			for (l in arm.Project.layers) {
				if (l.fill_layer != null) {
					Context.layer = l;
					Context.material = l.fill_layer;
					Layers.updateFillLayer();
				}
			}
			Context.layer = _layer;
			Context.material = _material;
			MakeMaterial.parsePaintMaterial();
		});
		UVUtil.uvmap = null;
		UVUtil.uvmapCached = false;
		UVUtil.trianglemap = null;
		UVUtil.trianglemapCached = false;
		UVUtil.dilatemapCached = false;
		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}
}
