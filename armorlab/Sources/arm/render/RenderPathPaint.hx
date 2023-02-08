package arm.render;

import iron.math.Mat4;
import iron.math.Vec4;
import iron.system.Input;
import iron.object.MeshObject;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.RenderPath;
import iron.Scene;
import arm.Viewport;
import arm.ui.UIHeader;
import arm.ui.UISidebar;
import arm.ui.UINodes;
import arm.node.MakeMaterial;
import arm.Enums;

class RenderPathPaint {

	static var path: RenderPath;
	public static var liveLayerDrawn = 0; ////

	public static function init(_path: RenderPath) {
		path = _path;

		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			var t = new RenderTargetRaw();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}

		path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
	}

	@:access(iron.RenderPath)
	public static function commandsPaint(dilation = true) {
		var tid = "";

		if (Context.pdirty > 0) {

			if (Context.tool == ToolPicker) {

					#if kha_metal
					//path.setTarget("texpaint_picker");
					//path.clearTarget(0xff000000);
					//path.setTarget("texpaint_nor_picker");
					//path.clearTarget(0xff000000);
					//path.setTarget("texpaint_pack_picker");
					//path.clearTarget(0xff000000);
					path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					#else
					path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					//path.clearTarget(0xff000000);
					#end
					path.bindTarget("gbuffer2", "gbuffer2");
					// tid = Context.layer.id;
					path.bindTarget("texpaint" + tid, "texpaint");
					path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
					path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
					path.drawMeshes("paint");
					UIHeader.inst.headerHandle.redraws = 2;

					var texpaint_picker = path.renderTargets.get("texpaint_picker").image;
					var texpaint_nor_picker = path.renderTargets.get("texpaint_nor_picker").image;
					var texpaint_pack_picker = path.renderTargets.get("texpaint_pack_picker").image;
					var texpaint_uv_picker = path.renderTargets.get("texpaint_uv_picker").image;
					var a = texpaint_picker.getPixels();
					var b = texpaint_nor_picker.getPixels();
					var c = texpaint_pack_picker.getPixels();
					var d = texpaint_uv_picker.getPixels();

					if (Context.colorPickerCallback != null) {
						Context.colorPickerCallback(Context.pickedColor);
					}

					// Picked surface values
					// #if (kha_metal || kha_vulkan)
					// Context.pickedColor.base.Rb = a.get(2);
					// Context.pickedColor.base.Gb = a.get(1);
					// Context.pickedColor.base.Bb = a.get(0);
					// Context.pickedColor.normal.Rb = b.get(2);
					// Context.pickedColor.normal.Gb = b.get(1);
					// Context.pickedColor.normal.Bb = b.get(0);
					// Context.pickedColor.occlusion = c.get(2) / 255;
					// Context.pickedColor.roughness = c.get(1) / 255;
					// Context.pickedColor.metallic = c.get(0) / 255;
					// Context.pickedColor.height = c.get(3) / 255;
					// Context.pickedColor.opacity = a.get(3) / 255;
					// Context.uvxPicked = d.get(2) / 255;
					// Context.uvyPicked = d.get(1) / 255;
					// #else
					// Context.pickedColor.base.Rb = a.get(0);
					// Context.pickedColor.base.Gb = a.get(1);
					// Context.pickedColor.base.Bb = a.get(2);
					// Context.pickedColor.normal.Rb = b.get(0);
					// Context.pickedColor.normal.Gb = b.get(1);
					// Context.pickedColor.normal.Bb = b.get(2);
					// Context.pickedColor.occlusion = c.get(0) / 255;
					// Context.pickedColor.roughness = c.get(1) / 255;
					// Context.pickedColor.metallic = c.get(2) / 255;
					// Context.pickedColor.height = c.get(3) / 255;
					// Context.pickedColor.opacity = a.get(3) / 255;
					// Context.uvxPicked = d.get(0) / 255;
					// Context.uvyPicked = d.get(1) / 255;
					// #end
			}
			else {
				var texpaint = "texpaint_node_target";

				path.setTarget("texpaint_blend1");
				path.bindTarget("texpaint_blend0", "tex");
				path.drawShader("shader_datas/copy_pass/copyR8_pass");

				path.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);

				path.bindTarget("_main", "gbufferD");

				path.bindTarget("texpaint_blend1", "paintmask");

				// Read texcoords from gbuffer
				var readTC = Context.tool == ToolClone ||
							 Context.tool == ToolBlur;
				if (readTC) {
					path.bindTarget("gbuffer2", "gbuffer2");
				}

				path.drawMeshes("paint");
			}
		}
	}

	public static function commandsCursor() {
		var tool = Context.tool;
		if (tool != ToolEraser &&
			tool != ToolClone &&
			tool != ToolBlur) {
			return;
		}

		var inpaint = UINodes.inst.getNodes().nodesSelected.length > 0 && UINodes.inst.getNodes().nodesSelected[0].type == "InpaintNode";

		if (!App.uiEnabled || App.isDragging || !inpaint) {
			return;
		}

		var mx = Context.paintVec.x;
		var my = 1.0 - Context.paintVec.y;
		if (Context.brushLocked) {
			mx = (Context.lockStartedX - iron.App.x()) / iron.App.w();
			my = 1.0 - (Context.lockStartedY - iron.App.y()) / iron.App.h();
		}
		var radius = Context.brushRadius;
		drawCursor(mx, my, radius / 3.4);
	}

	@:access(iron.RenderPath)
	static function drawCursor(mx: Float, my: Float, radius: Float, tintR = 1.0, tintG = 1.0, tintB = 1.0) {
		var plane = cast(Scene.active.getChild(".Plane"), MeshObject);
		var geom = plane.data.geom;

		var g = path.frameG;
		if (Layers.pipeCursor == null) Layers.makeCursorPipe();

		path.setTarget("");
		g.setPipeline(Layers.pipeCursor);
		var img = Res.get("cursor.k");
		g.setTexture(Layers.cursorTex, img);
		var gbuffer0 = path.renderTargets.get("gbuffer0").image;
		g.setTextureDepth(Layers.cursorGbufferD, gbuffer0);
		g.setFloat2(Layers.cursorMouse, mx, my);
		g.setFloat2(Layers.cursorTexStep, 1 / gbuffer0.width, 1 / gbuffer0.height);
		g.setFloat(Layers.cursorRadius, radius);
		var right = Scene.active.camera.rightWorld().normalize();
		g.setFloat3(Layers.cursorCameraRight, right.x, right.y, right.z);
		g.setFloat3(Layers.cursorTint, tintR, tintG, tintB);
		g.setMatrix(Layers.cursorVP, Scene.active.camera.VP.self);
		var helpMat = iron.math.Mat4.identity();
		helpMat.getInverse(Scene.active.camera.VP);
		g.setMatrix(Layers.cursorInvVP, helpMat.self);
		#if (kha_metal || kha_vulkan)
		g.setVertexBuffer(geom.get([{name: "tex", data: "short2norm"}]));
		#else
		g.setVertexBuffer(geom.vertexBuffer);
		#end
		g.setIndexBuffer(geom.indexBuffers[0]);
		g.drawIndexedVertices();

		g.disableScissor();
		path.end();
	}

	static function paintEnabled(): Bool {
		return !Context.foregroundEvent;
	}

	public static function begin() {
		if (!paintEnabled()) return;
	}

	public static function end() {
		commandsCursor();
		Context.ddirty--;
		Context.rdirty--;

		if (!paintEnabled()) return;
		Context.pdirty--;
	}

	public static function draw() {
		if (!paintEnabled()) return;

		commandsPaint();

		if (Context.brushBlendDirty) {
			Context.brushBlendDirty = false;
			#if kha_metal
			path.setTarget("texpaint_blend0");
			path.clearTarget(0x00000000);
			path.setTarget("texpaint_blend1");
			path.clearTarget(0x00000000);
			#else
			path.setTarget("texpaint_blend0", ["texpaint_blend1"]);
			path.clearTarget(0x00000000);
			#end
		}
	}

	public static function bindLayers() {
		var image: kha.Image = null;
		if (UINodes.inst.getNodes().nodesSelected.length > 0) {
			var node = UINodes.inst.getNodes().nodesSelected[0];
			var brushNode = arm.node.Brush.getBrushNode(node);
			if (brushNode != null) {
				image = brushNode.getImage();
			}
		}
		if (image != null) {
			if (path.renderTargets.get("texpaint_node") == null) {
				var t = new RenderTargetRaw();
				t.name = "texpaint_node";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				var rt = new RenderTarget(t);
				path.renderTargets.set(t.name, rt);
			}
			if (path.renderTargets.get("texpaint_node_target") == null) {
				var t = new RenderTargetRaw();
				t.name = "texpaint_node_target";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				var rt = new RenderTarget(t);
				path.renderTargets.set(t.name, rt);
			}
			path.renderTargets.get("texpaint_node").image = image;
			path.bindTarget("texpaint_node", "texpaint");
			path.bindTarget("texpaint_nor_empty", "texpaint_nor");
			path.bindTarget("texpaint_pack_empty", "texpaint_pack");

			var node = UINodes.inst.getNodes().nodesSelected[0];
			var inpaint = node.type == "InpaintNode";
			if (inpaint) {
				var brushNode = arm.node.Brush.getBrushNode(node);
				path.renderTargets.get("texpaint_node_target").image = cast(brushNode, arm.node.brush.InpaintNode).getTarget();
			}
		}
		else {
			path.bindTarget("texpaint", "texpaint");
			path.bindTarget("texpaint_nor", "texpaint_nor");
			path.bindTarget("texpaint_pack", "texpaint_pack");
		}
	}

	public static function unbindLayers() {

	}

	static function u32(ar: Array<Int>): kha.arrays.Uint32Array {
		var res = new kha.arrays.Uint32Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}

	static function i16(ar: Array<Int>): kha.arrays.Int16Array {
		var res = new kha.arrays.Int16Array(ar.length);
		for (i in 0...ar.length) res[i] = ar[i];
		return res;
	}
}
