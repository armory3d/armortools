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
import arm.node.MakeMaterial;
import arm.render.RenderPathPaint;
import arm.Enums;
import arm.ProjectFormat;

class Layers {

	public static var pipeCopy: PipelineState;
	public static var pipeCopyR: PipelineState;
	public static var pipeCopyG: PipelineState;
	public static var pipeCopyB: PipelineState;
	public static var pipeCopyA: PipelineState;
	public static var pipeCopyATex: TextureUnit;
	public static var pipeCopy8: PipelineState;
	public static var pipeCopy128: PipelineState;
	public static var pipeCopyBGRA: PipelineState;
	public static var pipeCopyRGB: PipelineState = null;
	public static var pipeApplyMask: PipelineState;
	public static var tex0Mask: TextureUnit;
	public static var texaMask: TextureUnit;
	public static var tempImage: Image = null;
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

	public static function initLayers() {
		var texpaint = iron.RenderPath.active.renderTargets.get("texpaint").image;
		var texpaint_nor = iron.RenderPath.active.renderTargets.get("texpaint_nor").image;
		var texpaint_pack = iron.RenderPath.active.renderTargets.get("texpaint_pack").image;
		// texpaint.g4.begin();
		// texpaint.g4.clear(kha.Color.fromFloats(0.5, 0.5, 0.5, 0.0)); // Base
		// texpaint.g4.end();
		texpaint.g2.begin(false);
		texpaint.g2.drawScaledImage(Res.get("placeholder.k"), 0, 0, Config.getTextureResX(), Config.getTextureResY()); // Base
		texpaint.g2.end();
		texpaint_nor.g4.begin();
		texpaint_nor.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor.g4.end();
		texpaint_pack.g4.begin();
		texpaint_pack.g4.clear(kha.Color.fromFloats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack.g4.end();
		var texpaint_nor_empty = iron.RenderPath.active.renderTargets.get("texpaint_nor_empty").image;
		var texpaint_pack_empty = iron.RenderPath.active.renderTargets.get("texpaint_pack_empty").image;
		texpaint_nor_empty.g4.begin();
		texpaint_nor_empty.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0)); // Nor
		texpaint_nor_empty.g4.end();
		texpaint_pack_empty.g4.begin();
		texpaint_pack_empty.g4.clear(kha.Color.fromFloats(1.0, 0.4, 0.0, 0.0)); // Occ, rough, met
		texpaint_pack_empty.g4.end();
	}

	public static function makePipe() {
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

		pipeCopyR = new PipelineState();
		pipeCopyR.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyR.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyR.inputLayout = [vs];
		pipeCopyR.colorWriteMasksGreen = [false];
		pipeCopyR.colorWriteMasksBlue = [false];
		pipeCopyR.colorWriteMasksAlpha = [false];
		pipeCopyR.compile();

		pipeCopyG = new PipelineState();
		pipeCopyG.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyG.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyG.inputLayout = [vs];
		pipeCopyG.colorWriteMasksRed = [false];
		pipeCopyG.colorWriteMasksBlue = [false];
		pipeCopyG.colorWriteMasksAlpha = [false];
		pipeCopyG.compile();

		pipeCopyB = new PipelineState();
		pipeCopyB.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipeCopyB.fragmentShader = kha.Shaders.getFragment("layer_copy.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipeCopyB.inputLayout = [vs];
		pipeCopyB.colorWriteMasksRed = [false];
		pipeCopyB.colorWriteMasksGreen = [false];
		pipeCopyB.colorWriteMasksAlpha = [false];
		pipeCopyB.compile();

		pipeApplyMask = new PipelineState();
		pipeApplyMask.vertexShader = kha.Shaders.getVertex("layer_copy_rrrr.vert");
		pipeApplyMask.fragmentShader = kha.Shaders.getFragment("mask_apply.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeApplyMask.inputLayout = [vs];
		pipeApplyMask.compile();
		tex0Mask = pipeApplyMask.getTextureUnit("tex0");
		texaMask = pipeApplyMask.getTextureUnit("texa");
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

	public static function makePipeCopyA() {
		pipeCopyA = new PipelineState();
		pipeCopyA.vertexShader = kha.Shaders.getVertex("layer_copy_rrrr.vert");
		pipeCopyA.fragmentShader = kha.Shaders.getFragment("layer_copy_rrrr.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float2);
		pipeCopyA.inputLayout = [vs];
		pipeCopyA.colorWriteMasksRed = [false];
		pipeCopyA.colorWriteMasksGreen = [false];
		pipeCopyA.colorWriteMasksBlue = [false];
		pipeCopyA.compile();
		pipeCopyATex = pipeCopyA.getTextureUnit("tex");
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
		var l = arm.node.brush.BrushOutputNode.inst;
		if (tempImage != null && (tempImage.width != l.texpaint.width || tempImage.height != l.texpaint.height || tempImage.format != l.texpaint.format)) {
			var _temptex0 = RenderPath.active.renderTargets.get("temptex0");
			App.notifyOnNextFrame(function() {
				_temptex0.unload();
			});
			RenderPath.active.renderTargets.remove("temptex0");
			tempImage = null;
		}
		if (tempImage == null) {
			var t = new RenderTargetRaw();
			t.name = "temptex0";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			tempImage = rt.image;
		}
	}

	public static function makeExportImg() {
		var l = arm.node.brush.BrushOutputNode.inst;
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
			var t = new RenderTargetRaw();
			t.name = "expa";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			expa = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expb";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			expb = rt.image;

			var t = new RenderTargetRaw();
			t.name = "expc";
			t.width = l.texpaint.width;
			t.height = l.texpaint.height;
			t.format = "RGBA32";
			var rt = RenderPath.active.createRenderTarget(t);
			expc = rt.image;
		}
	}

	public static function flatten(heightToNormal = false): Dynamic {
		var texpaint = arm.node.brush.BrushOutputNode.inst.texpaint;
		var texpaint_nor = arm.node.brush.BrushOutputNode.inst.texpaint_nor;
		var texpaint_pack = arm.node.brush.BrushOutputNode.inst.texpaint_pack;
		return { texpaint: texpaint, texpaint_nor: texpaint_nor, texpaint_pack: texpaint_pack };
	}

	public static function onLayersResized() {
		arm.node.brush.BrushOutputNode.inst.texpaint.unload();
		arm.node.brush.BrushOutputNode.inst.texpaint = iron.RenderPath.active.renderTargets.get("texpaint").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		arm.node.brush.BrushOutputNode.inst.texpaint_nor.unload();
		arm.node.brush.BrushOutputNode.inst.texpaint_nor = iron.RenderPath.active.renderTargets.get("texpaint_nor").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		arm.node.brush.BrushOutputNode.inst.texpaint_pack.unload();
		arm.node.brush.BrushOutputNode.inst.texpaint_pack = iron.RenderPath.active.renderTargets.get("texpaint_pack").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());

		if (@:privateAccess arm.node.brush.InpaintNode.image != null) {
			@:privateAccess arm.node.brush.InpaintNode.image.unload();
			@:privateAccess arm.node.brush.InpaintNode.image = null;
			@:privateAccess arm.node.brush.InpaintNode.mask.unload();
			@:privateAccess arm.node.brush.InpaintNode.mask = null;
			arm.node.brush.InpaintNode.init();
		}

		if (@:privateAccess arm.node.brush.PhotoToPBRNode.images != null) {
			for (image in @:privateAccess arm.node.brush.PhotoToPBRNode.images) image.unload();
			@:privateAccess arm.node.brush.PhotoToPBRNode.images = null;
			arm.node.brush.PhotoToPBRNode.init();
		}

		if (@:privateAccess arm.node.brush.TilingNode.image != null) {
			@:privateAccess arm.node.brush.TilingNode.image.unload();
			@:privateAccess arm.node.brush.TilingNode.image = null;
			arm.node.brush.TilingNode.init();
		}

		iron.RenderPath.active.renderTargets.get("texpaint_blend0").image.unload();
		iron.RenderPath.active.renderTargets.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);
		iron.RenderPath.active.renderTargets.get("texpaint_blend1").image.unload();
		iron.RenderPath.active.renderTargets.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8);

		if (iron.RenderPath.active.renderTargets.get("texpaint_node") != null) {
			iron.RenderPath.active.renderTargets.remove("texpaint_node");
		}
		if (iron.RenderPath.active.renderTargets.get("texpaint_node_target") != null) {
			iron.RenderPath.active.renderTargets.remove("texpaint_node_target");
		}

		App.notifyOnNextFrame(function() {
			initLayers();
		});

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}
}
