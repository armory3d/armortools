
class RenderPathPaint {

	static liveLayerDrawn = 0; ////

	static init = () => {

		{
			let t = RenderTarget.create();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = RenderTarget.create();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}

		RenderPath.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
	}

	static commandsPaint = (dilation = true) => {
		let tid = "";

		if (Context.raw.pdirty > 0) {

			if (Context.raw.tool == WorkspaceTool.ToolPicker) {

					///if krom_metal
					//RenderPath.setTarget("texpaint_picker");
					//RenderPath.clearTarget(0xff000000);
					//RenderPath.setTarget("texpaint_nor_picker");
					//RenderPath.clearTarget(0xff000000);
					//RenderPath.setTarget("texpaint_pack_picker");
					//RenderPath.clearTarget(0xff000000);
					RenderPath.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					///else
					RenderPath.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					//RenderPath.clearTarget(0xff000000);
					///end
					RenderPath.bindTarget("gbuffer2", "gbuffer2");
					// tid = Context.raw.layer.id;
					RenderPath.bindTarget("texpaint" + tid, "texpaint");
					RenderPath.bindTarget("texpaint_nor" + tid, "texpaint_nor");
					RenderPath.bindTarget("texpaint_pack" + tid, "texpaint_pack");
					RenderPath.drawMeshes("paint");
					UIHeader.headerHandle.redraws = 2;

					let texpaint_picker = RenderPath.renderTargets.get("texpaint_picker").image;
					let texpaint_nor_picker = RenderPath.renderTargets.get("texpaint_nor_picker").image;
					let texpaint_pack_picker = RenderPath.renderTargets.get("texpaint_pack_picker").image;
					let texpaint_uv_picker = RenderPath.renderTargets.get("texpaint_uv_picker").image;
					let a = Image.getPixels(texpaint_picker);
					let b = Image.getPixels(texpaint_nor_picker);
					let c = Image.getPixels(texpaint_pack_picker);
					let d = Image.getPixels(texpaint_uv_picker);

					if (Context.raw.colorPickerCallback != null) {
						Context.raw.colorPickerCallback(Context.raw.pickedColor);
					}

					// Picked surface values
					// ///if (krom_metal || krom_vulkan)
					// Context.raw.pickedColor.base.Rb = a.get(2);
					// Context.raw.pickedColor.base.Gb = a.get(1);
					// Context.raw.pickedColor.base.Bb = a.get(0);
					// Context.raw.pickedColor.normal.Rb = b.get(2);
					// Context.raw.pickedColor.normal.Gb = b.get(1);
					// Context.raw.pickedColor.normal.Bb = b.get(0);
					// Context.raw.pickedColor.occlusion = c.get(2) / 255;
					// Context.raw.pickedColor.roughness = c.get(1) / 255;
					// Context.raw.pickedColor.metallic = c.get(0) / 255;
					// Context.raw.pickedColor.height = c.get(3) / 255;
					// Context.raw.pickedColor.opacity = a.get(3) / 255;
					// Context.raw.uvxPicked = d.get(2) / 255;
					// Context.raw.uvyPicked = d.get(1) / 255;
					// ///else
					// Context.raw.pickedColor.base.Rb = a.get(0);
					// Context.raw.pickedColor.base.Gb = a.get(1);
					// Context.raw.pickedColor.base.Bb = a.get(2);
					// Context.raw.pickedColor.normal.Rb = b.get(0);
					// Context.raw.pickedColor.normal.Gb = b.get(1);
					// Context.raw.pickedColor.normal.Bb = b.get(2);
					// Context.raw.pickedColor.occlusion = c.get(0) / 255;
					// Context.raw.pickedColor.roughness = c.get(1) / 255;
					// Context.raw.pickedColor.metallic = c.get(2) / 255;
					// Context.raw.pickedColor.height = c.get(3) / 255;
					// Context.raw.pickedColor.opacity = a.get(3) / 255;
					// Context.raw.uvxPicked = d.get(0) / 255;
					// Context.raw.uvyPicked = d.get(1) / 255;
					// ///end
			}
			else {
				let texpaint = "texpaint_node_target";

				RenderPath.setTarget("texpaint_blend1");
				RenderPath.bindTarget("texpaint_blend0", "tex");
				RenderPath.drawShader("shader_datas/copy_pass/copyR8_pass");

				RenderPath.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);

				RenderPath.bindTarget("_main", "gbufferD");

				RenderPath.bindTarget("texpaint_blend1", "paintmask");

				// Read texcoords from gbuffer
				let readTC = Context.raw.tool == WorkspaceTool.ToolClone ||
							 Context.raw.tool == WorkspaceTool.ToolBlur ||
							 Context.raw.tool == WorkspaceTool.ToolSmudge;
				if (readTC) {
					RenderPath.bindTarget("gbuffer2", "gbuffer2");
				}

				RenderPath.drawMeshes("paint");
			}
		}
	}

	static commandsCursor = () => {
		let tool = Context.raw.tool;
		if (tool != WorkspaceTool.ToolEraser &&
			tool != WorkspaceTool.ToolClone &&
			tool != WorkspaceTool.ToolBlur &&
			tool != WorkspaceTool.ToolSmudge) {
			return;
		}

		let nodes = UINodes.getNodes();
		let canvas = UINodes.getCanvas(true);
		let inpaint = nodes.nodesSelectedId.length > 0 && nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]).type == "InpaintNode";

		if (!Base.uiEnabled || Base.isDragging || !inpaint) {
			return;
		}

		let mx = Context.raw.paintVec.x;
		let my = 1.0 - Context.raw.paintVec.y;
		if (Context.raw.brushLocked) {
			mx = (Context.raw.lockStartedX - App.x()) / App.w();
			my = 1.0 - (Context.raw.lockStartedY - App.y()) / App.h();
		}
		let radius = Context.raw.brushRadius;
		RenderPathPaint.drawCursor(mx, my, radius / 3.4);
	}

	static drawCursor = (mx: f32, my: f32, radius: f32, tintR = 1.0, tintG = 1.0, tintB = 1.0) => {
		let plane = Scene.getChild(".Plane").ext;
		let geom = plane.data;
		if (Base.pipeCursor == null) Base.makeCursorPipe();

		RenderPath.setTarget("");
		Graphics4.setPipeline(Base.pipeCursor);
		let img = Res.get("cursor.k");
		Graphics4.setTexture(Base.cursorTex, img);
		let gbuffer0 = RenderPath.renderTargets.get("gbuffer0").image;
		Graphics4.setTextureDepth(Base.cursorGbufferD, gbuffer0);
		Graphics4.setFloat2(Base.cursorMouse, mx, my);
		Graphics4.setFloat2(Base.cursorTexStep, 1 / gbuffer0.width, 1 / gbuffer0.height);
		Graphics4.setFloat(Base.cursorRadius, radius);
		let right = Vec4.normalize(CameraObject.rightWorld(Scene.camera));
		Graphics4.setFloat3(Base.cursorCameraRight, right.x, right.y, right.z);
		Graphics4.setFloat3(Base.cursorTint, tintR, tintG, tintB);
		Graphics4.setMatrix(Base.cursorVP, Scene.camera.VP);
		let helpMat = Mat4.identity();
		Mat4.getInverse(helpMat, Scene.camera.VP);
		Graphics4.setMatrix(Base.cursorInvVP, helpMat);
		///if (krom_metal || krom_vulkan)
		Graphics4.setVertexBuffer(MeshData.get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		Graphics4.setVertexBuffer(geom._vertexBuffer);
		///end
		Graphics4.setIndexBuffer(geom._indexBuffers[0]);
		Graphics4.drawIndexedVertices();

		Graphics4.disableScissor();
		RenderPath.end();
	}

	static paintEnabled = (): bool => {
		return !Context.raw.foregroundEvent;
	}

	static begin = () => {
		if (!RenderPathPaint.paintEnabled()) return;
	}

	static end = () => {
		RenderPathPaint.commandsCursor();
		Context.raw.ddirty--;
		Context.raw.rdirty--;

		if (!RenderPathPaint.paintEnabled()) return;
		Context.raw.pdirty--;
	}

	static draw = () => {
		if (!RenderPathPaint.paintEnabled()) return;

		RenderPathPaint.commandsPaint();

		if (Context.raw.brushBlendDirty) {
			Context.raw.brushBlendDirty = false;
			///if krom_metal
			RenderPath.setTarget("texpaint_blend0");
			RenderPath.clearTarget(0x00000000);
			RenderPath.setTarget("texpaint_blend1");
			RenderPath.clearTarget(0x00000000);
			///else
			RenderPath.setTarget("texpaint_blend0", ["texpaint_blend1"]);
			RenderPath.clearTarget(0x00000000);
			///end
		}
	}

	static bindLayers = () => {
		let image: ImageRaw = null;
		let nodes = UINodes.getNodes();
		let canvas = UINodes.getCanvas(true);
		if (nodes.nodesSelectedId.length > 0) {
			let node = nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]);
			let brushNode = ParserLogic.getLogicNode(node);
			if (brushNode != null) {
				image = brushNode.getCachedImage();
			}
		}
		if (image != null) {
			if (RenderPath.renderTargets.get("texpaint_node") == null) {
				let t = RenderTarget.create();
				t.name = "texpaint_node";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				RenderPath.renderTargets.set(t.name, t);
			}
			if (RenderPath.renderTargets.get("texpaint_node_target") == null) {
				let t = RenderTarget.create();
				t.name = "texpaint_node_target";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				RenderPath.renderTargets.set(t.name, t);
			}
			RenderPath.renderTargets.get("texpaint_node").image = image;
			RenderPath.bindTarget("texpaint_node", "texpaint");
			RenderPath.bindTarget("texpaint_nor_empty", "texpaint_nor");
			RenderPath.bindTarget("texpaint_pack_empty", "texpaint_pack");

			let nodes = UINodes.getNodes();
			let canvas = UINodes.getCanvas(true);
			let node = nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]);
			let inpaint = node.type == "InpaintNode";
			if (inpaint) {
				let brushNode = ParserLogic.getLogicNode(node);
				RenderPath.renderTargets.get("texpaint_node_target").image = (brushNode as InpaintNode).getTarget();
			}
		}
		else {
			RenderPath.bindTarget("texpaint", "texpaint");
			RenderPath.bindTarget("texpaint_nor", "texpaint_nor");
			RenderPath.bindTarget("texpaint_pack", "texpaint_pack");
		}
	}

	static unbindLayers = () => {

	}
}
