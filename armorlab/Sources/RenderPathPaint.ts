
class RenderPathPaint {

	static path: RenderPath;
	static liveLayerDrawn = 0; ////

	static init = (_path: RenderPath) => {
		path = _path;

		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			path.createRenderTarget(t);
		}

		path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
	}

	static commandsPaint = (dilation = true) => {
		let tid = "";

		if (Context.raw.pdirty > 0) {

			if (Context.raw.tool == ToolPicker) {

					///if krom_metal
					//path.setTarget("texpaint_picker");
					//path.clearTarget(0xff000000);
					//path.setTarget("texpaint_nor_picker");
					//path.clearTarget(0xff000000);
					//path.setTarget("texpaint_pack_picker");
					//path.clearTarget(0xff000000);
					path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					///else
					path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					//path.clearTarget(0xff000000);
					///end
					path.bindTarget("gbuffer2", "gbuffer2");
					// tid = Context.raw.layer.id;
					path.bindTarget("texpaint" + tid, "texpaint");
					path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
					path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
					path.drawMeshes("paint");
					UIHeader.inst.headerHandle.redraws = 2;

					let texpaint_picker = path.renderTargets.get("texpaint_picker").image;
					let texpaint_nor_picker = path.renderTargets.get("texpaint_nor_picker").image;
					let texpaint_pack_picker = path.renderTargets.get("texpaint_pack_picker").image;
					let texpaint_uv_picker = path.renderTargets.get("texpaint_uv_picker").image;
					let a = texpaint_picker.getPixels();
					let b = texpaint_nor_picker.getPixels();
					let c = texpaint_pack_picker.getPixels();
					let d = texpaint_uv_picker.getPixels();

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

				path.setTarget("texpaint_blend1");
				path.bindTarget("texpaint_blend0", "tex");
				path.drawShader("shader_datas/copy_pass/copyR8_pass");

				path.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);

				path.bindTarget("_main", "gbufferD");

				path.bindTarget("texpaint_blend1", "paintmask");

				// Read texcoords from gbuffer
				let readTC = Context.raw.tool == ToolClone ||
							 Context.raw.tool == ToolBlur ||
							 Context.raw.tool == ToolSmudge;
				if (readTC) {
					path.bindTarget("gbuffer2", "gbuffer2");
				}

				path.drawMeshes("paint");
			}
		}
	}

	static commandsCursor = () => {
		let tool = Context.raw.tool;
		if (tool != ToolEraser &&
			tool != ToolClone &&
			tool != ToolBlur &&
			tool != ToolSmudge) {
			return;
		}

		let nodes = UINodes.inst.getNodes();
		let canvas = UINodes.inst.getCanvas(true);
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
		drawCursor(mx, my, radius / 3.4);
	}

	static drawCursor = (mx: f32, my: f32, radius: f32, tintR = 1.0, tintG = 1.0, tintB = 1.0) => {
		let plane = cast(Scene.active.getChild(".Plane"), MeshObject);
		let geom = plane.data;

		let g = path.frameG;
		if (Base.pipeCursor == null) Base.makeCursorPipe();

		path.setTarget("");
		g.setPipeline(Base.pipeCursor);
		let img = Res.get("cursor.k");
		g.setTexture(Base.cursorTex, img);
		let gbuffer0 = path.renderTargets.get("gbuffer0").image;
		g.setTextureDepth(Base.cursorGbufferD, gbuffer0);
		g.setFloat2(Base.cursorMouse, mx, my);
		g.setFloat2(Base.cursorTexStep, 1 / gbuffer0.width, 1 / gbuffer0.height);
		g.setFloat(Base.cursorRadius, radius);
		let right = Scene.active.camera.rightWorld().normalize();
		g.setFloat3(Base.cursorCameraRight, right.x, right.y, right.z);
		g.setFloat3(Base.cursorTint, tintR, tintG, tintB);
		g.setMatrix(Base.cursorVP, Scene.active.camera.VP);
		let helpMat = Mat4.identity();
		helpMat.getInverse(Scene.active.camera.VP);
		g.setMatrix(Base.cursorInvVP, helpMat);
		///if (krom_metal || krom_vulkan)
		g.setVertexBuffer(geom.get([{name: "tex", data: "short2norm"}]));
		///else
		g.setVertexBuffer(geom.vertexBuffer);
		///end
		g.setIndexBuffer(geom.indexBuffers[0]);
		g.drawIndexedVertices();

		g.disableScissor();
		path.end();
	}

	static paintEnabled = (): bool => {
		return !Context.raw.foregroundEvent;
	}

	static begin = () => {
		if (!paintEnabled()) return;
	}

	static end = () => {
		commandsCursor();
		Context.raw.ddirty--;
		Context.raw.rdirty--;

		if (!paintEnabled()) return;
		Context.raw.pdirty--;
	}

	static draw = () => {
		if (!paintEnabled()) return;

		commandsPaint();

		if (Context.raw.brushBlendDirty) {
			Context.raw.brushBlendDirty = false;
			///if krom_metal
			path.setTarget("texpaint_blend0");
			path.clearTarget(0x00000000);
			path.setTarget("texpaint_blend1");
			path.clearTarget(0x00000000);
			///else
			path.setTarget("texpaint_blend0", ["texpaint_blend1"]);
			path.clearTarget(0x00000000);
			///end
		}
	}

	static bindLayers = () => {
		let image: Image = null;
		let nodes = UINodes.inst.getNodes();
		let canvas = UINodes.inst.getCanvas(true);
		if (nodes.nodesSelectedId.length > 0) {
			let node = nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]);
			let brushNode = ParserLogic.getLogicNode(node);
			if (brushNode != null) {
				image = brushNode.getCachedImage();
			}
		}
		if (image != null) {
			if (path.renderTargets.get("texpaint_node") == null) {
				let t = new RenderTargetRaw();
				t.name = "texpaint_node";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				let rt = new RenderTarget(t);
				path.renderTargets.set(t.name, rt);
			}
			if (path.renderTargets.get("texpaint_node_target") == null) {
				let t = new RenderTargetRaw();
				t.name = "texpaint_node_target";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				let rt = new RenderTarget(t);
				path.renderTargets.set(t.name, rt);
			}
			path.renderTargets.get("texpaint_node").image = image;
			path.bindTarget("texpaint_node", "texpaint");
			path.bindTarget("texpaint_nor_empty", "texpaint_nor");
			path.bindTarget("texpaint_pack_empty", "texpaint_pack");

			let nodes = UINodes.inst.getNodes();
			let canvas = UINodes.inst.getCanvas(true);
			let node = nodes.getNode(canvas.nodes, nodes.nodesSelectedId[0]);
			let inpaint = node.type == "InpaintNode";
			if (inpaint) {
				let brushNode = ParserLogic.getLogicNode(node);
				path.renderTargets.get("texpaint_node_target").image = cast(brushNode, arm.nodes.InpaintNode).getTarget();
			}
		}
		else {
			path.bindTarget("texpaint", "texpaint");
			path.bindTarget("texpaint_nor", "texpaint_nor");
			path.bindTarget("texpaint_pack", "texpaint_pack");
		}
	}

	static unbindLayers = () => {

	}

	static array_u32 = (ar: i32[]): Uint32Array => {
		let res = new Uint32Array(ar.length);
		for (let i = 0; i < ar.length; ++i) res[i] = ar[i];
		return res;
	}

	static array_i16 = (ar: i32[]): Int16Array => {
		let res = new Int16Array(ar.length);
		for (let i = 0; i < ar.length; ++i) res[i] = ar[i];
		return res;
	}
}
