
class RenderPathPaint {

	static liveLayer: SlotLayerRaw = null;
	static liveLayerDrawn = 0;
	static liveLayerLocked = false;
	static dilated = true;
	static initVoxels = true; // Bake AO
	static pushUndoLast: bool;
	static painto: TMeshObject = null;
	static planeo: TMeshObject = null;
	static visibles: bool[] = null;
	static mergedObjectVisible = false;
	static savedFov = 0.0;
	static baking = false;
	static _texpaint: RenderTarget;
	static _texpaint_nor: RenderTarget;
	static _texpaint_pack: RenderTarget;
	static _texpaint_undo: RenderTarget;
	static _texpaint_nor_undo: RenderTarget;
	static _texpaint_pack_undo: RenderTarget;
	static lastX = -1.0;
	static lastY = -1.0;

	static init = () => {

		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_colorid";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_posnortex_picker0";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			RenderPath.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_posnortex_picker1";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			RenderPath.createRenderTarget(t);
		}

		RenderPath.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		RenderPath.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
		///if is_paint
		RenderPath.loadShader("shader_datas/dilate_pass/dilate_pass");
		///end
	}

	static commandsPaint = (dilation = true) => {
		let tid = Context.raw.layer.id;

		if (Context.raw.pdirty > 0) {
			///if arm_physics
			let particlePhysics = Context.raw.particlePhysics;
			///else
			let particlePhysics = false;
			///end
			if (Context.raw.tool == WorkspaceTool.ToolParticle && !particlePhysics) {
				RenderPath.setTarget("texparticle");
				RenderPath.clearTarget(0x00000000);
				RenderPath.bindTarget("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
					RenderPath.bindTarget("gbuffer0", "gbuffer0");
				}

				let mo: TMeshObject = Scene.getChild(".ParticleEmitter").ext;
				mo.base.visible = true;
				MeshObject.render(mo, RenderPath.currentG, "mesh", RenderPath.bindParams);
				mo.base.visible = false;

				mo = Scene.getChild(".Particle").ext;
				mo.base.visible = true;
				MeshObject.render(mo, RenderPath.currentG, "mesh", RenderPath.bindParams);
				mo.base.visible = false;
				RenderPath.end();
			}

			///if is_paint
			if (Context.raw.tool == WorkspaceTool.ToolColorId) {
				RenderPath.setTarget("texpaint_colorid");
				RenderPath.clearTarget(0xff000000);
				RenderPath.bindTarget("gbuffer2", "gbuffer2");
				RenderPath.drawMeshes("paint");
				UIHeader.headerHandle.redraws = 2;
			}
			else if (Context.raw.tool == WorkspaceTool.ToolPicker || Context.raw.tool == WorkspaceTool.ToolMaterial) {
				if (Context.raw.pickPosNorTex) {
					if (Context.raw.paint2d) {
						RenderPath.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
						RenderPath.drawMeshes("mesh");
					}
					RenderPath.setTarget("texpaint_posnortex_picker0", ["texpaint_posnortex_picker1"]);
					RenderPath.bindTarget("gbuffer2", "gbuffer2");
					RenderPath.bindTarget("_main", "gbufferD");
					RenderPath.drawMeshes("paint");
					let texpaint_posnortex_picker0 = RenderPath.renderTargets.get("texpaint_posnortex_picker0").image;
					let texpaint_posnortex_picker1 = RenderPath.renderTargets.get("texpaint_posnortex_picker1").image;
					let a = new DataView(texpaint_posnortex_picker0.getPixels());
					let b = new DataView(texpaint_posnortex_picker1.getPixels());
					Context.raw.posXPicked = a.getFloat32(0, true);
					Context.raw.posYPicked = a.getFloat32(4, true);
					Context.raw.posZPicked = a.getFloat32(8, true);
					Context.raw.uvxPicked = a.getFloat32(12, true);
					Context.raw.norXPicked = b.getFloat32(0, true);
					Context.raw.norYPicked = b.getFloat32(4, true);
					Context.raw.norZPicked = b.getFloat32(8, true);
					Context.raw.uvyPicked = b.getFloat32(12, true);
				}
				else {
					RenderPath.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					RenderPath.bindTarget("gbuffer2", "gbuffer2");
					tid = Context.raw.layer.id;
					let useLiveLayer = Context.raw.tool == WorkspaceTool.ToolMaterial;
					if (useLiveLayer) RenderPathPaint.useLiveLayer(true);
					RenderPath.bindTarget("texpaint" + tid, "texpaint");
					RenderPath.bindTarget("texpaint_nor" + tid, "texpaint_nor");
					RenderPath.bindTarget("texpaint_pack" + tid, "texpaint_pack");
					RenderPath.drawMeshes("paint");
					if (useLiveLayer) RenderPathPaint.useLiveLayer(false);
					UIHeader.headerHandle.redraws = 2;
					UIBase.hwnds[2].redraws = 2;

					let texpaint_picker = RenderPath.renderTargets.get("texpaint_picker").image;
					let texpaint_nor_picker = RenderPath.renderTargets.get("texpaint_nor_picker").image;
					let texpaint_pack_picker = RenderPath.renderTargets.get("texpaint_pack_picker").image;
					let texpaint_uv_picker = RenderPath.renderTargets.get("texpaint_uv_picker").image;
					let a = new DataView(texpaint_picker.getPixels());
					let b = new DataView(texpaint_nor_picker.getPixels());
					let c = new DataView(texpaint_pack_picker.getPixels());
					let d = new DataView(texpaint_uv_picker.getPixels());

					if (Context.raw.colorPickerCallback != null) {
						Context.raw.colorPickerCallback(Context.raw.pickedColor);
					}

					// Picked surface values
					///if (krom_metal || krom_vulkan)
					let i0 = 2;
					let i1 = 1;
					let i2 = 0;
					///else
					let i0 = 0;
					let i1 = 1;
					let i2 = 2;
					///end
					let i3 = 3;
					Context.raw.pickedColor.base = color_set_rb(Context.raw.pickedColor.base, a.getUint8(i0));
					Context.raw.pickedColor.base = color_set_gb(Context.raw.pickedColor.base, a.getUint8(i1));
					Context.raw.pickedColor.base = color_set_bb(Context.raw.pickedColor.base, a.getUint8(i2));
					Context.raw.pickedColor.normal = color_set_rb(Context.raw.pickedColor.normal, b.getUint8(i0));
					Context.raw.pickedColor.normal = color_set_gb(Context.raw.pickedColor.normal, b.getUint8(i1));
					Context.raw.pickedColor.normal = color_set_bb(Context.raw.pickedColor.normal, b.getUint8(i2));
					Context.raw.pickedColor.occlusion = c.getUint8(i0) / 255;
					Context.raw.pickedColor.roughness = c.getUint8(i1) / 255;
					Context.raw.pickedColor.metallic = c.getUint8(i2) / 255;
					Context.raw.pickedColor.height = c.getUint8(i3) / 255;
					Context.raw.pickedColor.opacity = a.getUint8(i3) / 255;
					Context.raw.uvxPicked = d.getUint8(i0) / 255;
					Context.raw.uvyPicked = d.getUint8(i1) / 255;
					// Pick material
					if (Context.raw.pickerSelectMaterial && Context.raw.colorPickerCallback == null) {
						// matid % 3 == 0 - normal, 1 - emission, 2 - subsurface
						let matid = Math.floor((b.getUint8(3) - (b.getUint8(3) % 3)) / 3);
						for (let m of Project.materials) {
							if (m.id == matid) {
								Context.setMaterial(m);
								Context.raw.materialIdPicked = matid;
								break;
							}
						}
					}
				}
			}
			else {
				///if arm_voxels
				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.bakeType == BakeType.BakeAO) {
					if (RenderPathPaint.initVoxels) {
						RenderPathPaint.initVoxels = false;
						let _rp_gi = Config.raw.rp_gi;
						Config.raw.rp_gi = true;
						RenderPathBase.initVoxels();
						Config.raw.rp_gi = _rp_gi;
					}
					RenderPath.clearImage("voxels", 0x00000000);
					RenderPath.setTarget("");
					RenderPath.setViewport(256, 256);
					RenderPath.bindTarget("voxels", "voxels");
					RenderPath.drawMeshes("voxel");
					RenderPath.generateMipmaps("voxels");
				}
				///end

				let texpaint = "texpaint" + tid;
				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.brushTime == Time.delta) {
					// Clear to black on bake start
					RenderPath.setTarget(texpaint);
					RenderPath.clearTarget(0xff000000);
				}

				RenderPath.setTarget("texpaint_blend1");
				RenderPath.bindTarget("texpaint_blend0", "tex");
				RenderPath.drawShader("shader_datas/copy_pass/copyR8_pass");
				let isMask = SlotLayer.isMask(Context.raw.layer);
				if (isMask) {
					let ptid = Context.raw.layer.parent.id;
					if (SlotLayer.isGroup(Context.raw.layer.parent)) { // Group mask
						for (let c of SlotLayer.getChildren(Context.raw.layer.parent)) {
							ptid = c.id;
							break;
						}
					}
					RenderPath.setTarget(texpaint, ["texpaint_nor" + ptid, "texpaint_pack" + ptid, "texpaint_blend0"]);
				}
				else {
					RenderPath.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);
				}
				RenderPath.bindTarget("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
					RenderPath.bindTarget("gbuffer0", "gbuffer0");
				}
				RenderPath.bindTarget("texpaint_blend1", "paintmask");
				///if arm_voxels
				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.bakeType == BakeType.BakeAO) {
					RenderPath.bindTarget("voxels", "voxels");
				}
				///end
				if (Context.raw.colorIdPicked) {
					RenderPath.bindTarget("texpaint_colorid", "texpaint_colorid");
				}

				// Read texcoords from gbuffer
				let readTC = (Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillFace) ||
							  Context.raw.tool == WorkspaceTool.ToolClone ||
							  Context.raw.tool == WorkspaceTool.ToolBlur ||
							  Context.raw.tool == WorkspaceTool.ToolSmudge;
				if (readTC) {
					RenderPath.bindTarget("gbuffer2", "gbuffer2");
				}

				RenderPath.drawMeshes("paint");

				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.bakeType == BakeType.BakeCurvature && Context.raw.bakeCurvSmooth > 0) {
					if (RenderPath.renderTargets.get("texpaint_blur") == null) {
						let t = new RenderTargetRaw();
						t.name = "texpaint_blur";
						t.width = Math.floor(Config.getTextureResX() * 0.95);
						t.height = Math.floor(Config.getTextureResY() * 0.95);
						t.format = "RGBA32";
						RenderPath.createRenderTarget(t);
					}
					let blurs = Math.round(Context.raw.bakeCurvSmooth);
					for (let i = 0; i < blurs; ++i) {
						RenderPath.setTarget("texpaint_blur");
						RenderPath.bindTarget(texpaint, "tex");
						RenderPath.drawShader("shader_datas/copy_pass/copy_pass");
						RenderPath.setTarget(texpaint);
						RenderPath.bindTarget("texpaint_blur", "tex");
						RenderPath.drawShader("shader_datas/copy_pass/copy_pass");
					}
				}

				if (dilation && Config.raw.dilate == DilateType.DilateInstant) {
					RenderPathPaint.dilate(true, false);
				}
			}
			///end

			///if is_sculpt
			let texpaint = "texpaint" + tid;
			RenderPath.setTarget("texpaint_blend1");
			RenderPath.bindTarget("texpaint_blend0", "tex");
			RenderPath.drawShader("shader_datas/copy_pass/copyR8_pass");
			RenderPath.setTarget(texpaint, ["texpaint_blend0"]);
			RenderPath.bindTarget("gbufferD_undo", "gbufferD");
			if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
				RenderPath.bindTarget("gbuffer0", "gbuffer0");
			}
			RenderPath.bindTarget("texpaint_blend1", "paintmask");

			// Read texcoords from gbuffer
			let readTC = (Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillFace) ||
						  Context.raw.tool == WorkspaceTool.ToolClone ||
						  Context.raw.tool == WorkspaceTool.ToolBlur ||
						  Context.raw.tool == WorkspaceTool.ToolSmudge;
			if (readTC) {
				RenderPath.bindTarget("gbuffer2", "gbuffer2");
			}
			RenderPath.bindTarget("gbuffer0_undo", "gbuffer0_undo");

			let materialContexts: TMaterialContext[] = [];
			let shaderContexts: TShaderContext[] = [];
			let mats = Project.paintObjects[0].materials;
			MeshObject.getContexts(Project.paintObjects[0], "paint", mats, materialContexts, shaderContexts);

			let cc_context = shaderContexts[0];
			if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
			RenderPath.currentG.setPipeline(cc_context._pipeState);
			Uniforms.setContextConstants(RenderPath.currentG, cc_context, RenderPath.bindParams);
			Uniforms.setObjectConstants(RenderPath.currentG, cc_context, Project.paintObjects[0].base);
			Uniforms.setMaterialConstants(RenderPath.currentG, cc_context, materialContexts[0]);
			RenderPath.currentG.setVertexBuffer(ConstData.screenAlignedVB);
			RenderPath.currentG.setIndexBuffer(ConstData.screenAlignedIB);
			RenderPath.currentG.drawIndexedVertices();
			RenderPath.end();
			///end
		}
	}

	static useLiveLayer = (use: bool) => {
		let tid = Context.raw.layer.id;
		let hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		if (use) {
			RenderPathPaint._texpaint = RenderPath.renderTargets.get("texpaint" + tid);
			RenderPathPaint._texpaint_undo = RenderPath.renderTargets.get("texpaint_undo" + hid);
			RenderPathPaint._texpaint_nor_undo = RenderPath.renderTargets.get("texpaint_nor_undo" + hid);
			RenderPathPaint._texpaint_pack_undo = RenderPath.renderTargets.get("texpaint_pack_undo" + hid);
			RenderPathPaint._texpaint_nor = RenderPath.renderTargets.get("texpaint_nor" + tid);
			RenderPathPaint._texpaint_pack = RenderPath.renderTargets.get("texpaint_pack" + tid);
			RenderPath.renderTargets.set("texpaint_undo" + hid, RenderPath.renderTargets.get("texpaint" + tid));
			RenderPath.renderTargets.set("texpaint" + tid, RenderPath.renderTargets.get("texpaint_live"));
			if (SlotLayer.isLayer(Context.raw.layer)) {
				RenderPath.renderTargets.set("texpaint_nor_undo" + hid, RenderPath.renderTargets.get("texpaint_nor" + tid));
				RenderPath.renderTargets.set("texpaint_pack_undo" + hid, RenderPath.renderTargets.get("texpaint_pack" + tid));
				RenderPath.renderTargets.set("texpaint_nor" + tid, RenderPath.renderTargets.get("texpaint_nor_live"));
				RenderPath.renderTargets.set("texpaint_pack" + tid, RenderPath.renderTargets.get("texpaint_pack_live"));
			}
		}
		else {
			RenderPath.renderTargets.set("texpaint" + tid, RenderPathPaint._texpaint);
			RenderPath.renderTargets.set("texpaint_undo" + hid, RenderPathPaint._texpaint_undo);
			if (SlotLayer.isLayer(Context.raw.layer)) {
				RenderPath.renderTargets.set("texpaint_nor_undo" + hid, RenderPathPaint._texpaint_nor_undo);
				RenderPath.renderTargets.set("texpaint_pack_undo" + hid, RenderPathPaint._texpaint_pack_undo);
				RenderPath.renderTargets.set("texpaint_nor" + tid, RenderPathPaint._texpaint_nor);
				RenderPath.renderTargets.set("texpaint_pack" + tid, RenderPathPaint._texpaint_pack);
			}
		}
		RenderPathPaint.liveLayerLocked = use;
	}

	static commandsLiveBrush = () => {
		let tool = Context.raw.tool;
		if (tool != WorkspaceTool.ToolBrush &&
			tool != WorkspaceTool.ToolEraser &&
			tool != WorkspaceTool.ToolClone &&
			tool != WorkspaceTool.ToolDecal &&
			tool != WorkspaceTool.ToolText &&
			tool != WorkspaceTool.ToolBlur &&
			tool != WorkspaceTool.ToolSmudge) {
				return;
		}

		if (RenderPathPaint.liveLayerLocked) return;

		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = SlotLayer.create("_live");
		}

		let tid = Context.raw.layer.id;
		if (SlotLayer.isMask(Context.raw.layer)) {
			RenderPath.setTarget("texpaint_live");
			RenderPath.bindTarget("texpaint" + tid, "tex");
			RenderPath.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			RenderPath.setTarget("texpaint_live", ["texpaint_nor_live", "texpaint_pack_live"]);
			RenderPath.bindTarget("texpaint" + tid, "tex0");
			RenderPath.bindTarget("texpaint_nor" + tid, "tex1");
			RenderPath.bindTarget("texpaint_pack" + tid, "tex2");
			RenderPath.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		}

		RenderPathPaint.useLiveLayer(true);

		RenderPathPaint.liveLayerDrawn = 2;

		UIView2D.hwnd.redraws = 2;
		let _x = Context.raw.paintVec.x;
		let _y = Context.raw.paintVec.y;
		if (Context.raw.brushLocked) {
			Context.raw.paintVec.x = (Context.raw.lockStartedX - App.x()) / App.w();
			Context.raw.paintVec.y = (Context.raw.lockStartedY - App.y()) / App.h();
		}
		let _lastX = Context.raw.lastPaintVecX;
		let _lastY = Context.raw.lastPaintVecY;
		let _pdirty = Context.raw.pdirty;
		Context.raw.lastPaintVecX = Context.raw.paintVec.x;
		Context.raw.lastPaintVecY = Context.raw.paintVec.y;
		if (Operator.shortcut(Config.keymap.brush_ruler)) {
			Context.raw.lastPaintVecX = Context.raw.lastPaintX;
			Context.raw.lastPaintVecY = Context.raw.lastPaintY;
		}
		Context.raw.pdirty = 2;

		RenderPathPaint.commandsSymmetry();
		RenderPathPaint.commandsPaint();

		RenderPathPaint.useLiveLayer(false);

		Context.raw.paintVec.x = _x;
		Context.raw.paintVec.y = _y;
		Context.raw.lastPaintVecX = _lastX;
		Context.raw.lastPaintVecY = _lastY;
		Context.raw.pdirty = _pdirty;
		Context.raw.brushBlendDirty = true;
	}

	static commandsCursor = () => {
		if (!Config.raw.brush_3d) return;
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
		let tool = Context.raw.tool;
		if (tool != WorkspaceTool.ToolBrush &&
			tool != WorkspaceTool.ToolEraser &&
			tool != WorkspaceTool.ToolClone &&
			tool != WorkspaceTool.ToolBlur &&
			tool != WorkspaceTool.ToolSmudge &&
			tool != WorkspaceTool.ToolParticle &&
			!decalMask) {
				return;
		}

		let fillLayer = Context.raw.layer.fill_layer != null;
		let groupLayer = SlotLayer.isGroup(Context.raw.layer);
		if (!Base.uiEnabled || Base.isDragging || fillLayer || groupLayer) {
			return;
		}

		let mx = Context.raw.paintVec.x;
		let my = 1.0 - Context.raw.paintVec.y;
		if (Context.raw.brushLocked) {
			mx = (Context.raw.lockStartedX - App.x()) / App.w();
			my = 1.0 - (Context.raw.lockStartedY - App.y()) / App.h();
		}
		let radius = decalMask ? Context.raw.brushDecalMaskRadius : Context.raw.brushRadius;
		RenderPathPaint.drawCursor(mx, my, Context.raw.brushNodesRadius * radius / 3.4);
	}

	static drawCursor = (mx: f32, my: f32, radius: f32, tintR = 1.0, tintG = 1.0, tintB = 1.0) => {
		let plane: TMeshObject = Scene.getChild(".Plane").ext;
		let geom = plane.data;

		let g = RenderPath.frameG;
		if (Base.pipeCursor == null) Base.makeCursorPipe();

		RenderPath.setTarget("");
		g.setPipeline(Base.pipeCursor);
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
		let img = (decal && !decalMask) ? Context.raw.decalImage : Res.get("cursor.k");
		g.setTexture(Base.cursorTex, img);
		let gbuffer0 = RenderPath.renderTargets.get("gbuffer0").image;
		g.setTextureDepth(Base.cursorGbufferD, gbuffer0);
		g.setFloat2(Base.cursorMouse, mx, my);
		g.setFloat2(Base.cursorTexStep, 1 / gbuffer0.width, 1 / gbuffer0.height);
		g.setFloat(Base.cursorRadius, radius);
		let right = Vec4.normalize(CameraObject.rightWorld(Scene.camera));
		g.setFloat3(Base.cursorCameraRight, right.x, right.y, right.z);
		g.setFloat3(Base.cursorTint, tintR, tintG, tintB);
		g.setMatrix(Base.cursorVP, Scene.camera.VP);
		let helpMat = Mat4.identity();
		Mat4.getInverse(helpMat, Scene.camera.VP);
		g.setMatrix(Base.cursorInvVP, helpMat);
		///if (krom_metal || krom_vulkan)
		g.setVertexBuffer(MeshData.get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		g.setVertexBuffer(geom._vertexBuffer);
		///end
		g.setIndexBuffer(geom._indexBuffers[0]);
		g.drawIndexedVertices();

		g.disableScissor();
		RenderPath.end();
	}

	static commandsSymmetry = () => {
		if (Context.raw.symX || Context.raw.symY || Context.raw.symZ) {
			Context.raw.ddirty = 2;
			let t = Context.raw.paintObject.base.transform;
			let sx = t.scale.x;
			let sy = t.scale.y;
			let sz = t.scale.z;
			if (Context.raw.symX) {
				Vec4.set(t.scale, -sx, sy, sz);
				Transform.buildMatrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symY) {
				Vec4.set(t.scale, sx, -sy, sz);
				Transform.buildMatrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symZ) {
				Vec4.set(t.scale, sx, sy, -sz);
				Transform.buildMatrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY) {
				Vec4.set(t.scale, -sx, -sy, sz);
				Transform.buildMatrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symZ) {
				Vec4.set(t.scale, -sx, sy, -sz);
				Transform.buildMatrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symY && Context.raw.symZ) {
				Vec4.set(t.scale, sx, -sy, -sz);
				Transform.buildMatrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY && Context.raw.symZ) {
				Vec4.set(t.scale, -sx, -sy, -sz);
				Transform.buildMatrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			Vec4.set(t.scale, sx, sy, sz);
			Transform.buildMatrix(t);
		}
	}

	static paintEnabled = (): bool => {
		///if is_paint
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != WorkspaceTool.ToolPicker && Context.raw.tool != WorkspaceTool.ToolMaterial && Context.raw.tool != WorkspaceTool.ToolColorId;
		///end

		///if is_sculpt
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != WorkspaceTool.ToolPicker && Context.raw.tool != WorkspaceTool.ToolMaterial;
		///end

		let groupLayer = SlotLayer.isGroup(Context.raw.layer);
		return !fillLayer && !groupLayer && !Context.raw.foregroundEvent;
	}

	static liveBrushDirty = () => {
		let mx = RenderPathPaint.lastX;
		let my = RenderPathPaint.lastY;
		RenderPathPaint.lastX = Mouse.viewX;
		RenderPathPaint.lastY = Mouse.viewY;
		if (Config.raw.brush_live && Context.raw.pdirty <= 0) {
			let moved = (mx != RenderPathPaint.lastX || my != RenderPathPaint.lastY) && (Context.inViewport() || Context.in2dView());
			if (moved || Context.raw.brushLocked) {
				Context.raw.rdirty = 2;
			}
		}
	}

	static begin = () => {

		///if is_paint
		if (!RenderPathPaint.dilated) {
			RenderPathPaint.dilate(Config.raw.dilate == DilateType.DilateDelayed, true);
			RenderPathPaint.dilated = true;
		}
		///end

		if (!RenderPathPaint.paintEnabled()) return;

		///if is_paint
		RenderPathPaint.pushUndoLast = History.pushUndo;
		///end

		if (History.pushUndo && History.undoLayers != null) {
			History.paint();

			///if is_sculpt
			RenderPath.setTarget("gbuffer0_undo");
			RenderPath.bindTarget("gbuffer0", "tex");
			RenderPath.drawShader("shader_datas/copy_pass/copy_pass");

			RenderPath.setTarget("gbufferD_undo");
			RenderPath.bindTarget("_main", "tex");
			RenderPath.drawShader("shader_datas/copy_pass/copy_pass");
			///end
		}

		///if is_sculpt
		if (History.pushUndo2 && History.undoLayers != null) {
			History.paint();
		}
		///end

		if (Context.raw.paint2d) {
			RenderPathPaint.setPlaneMesh();
		}

		if (RenderPathPaint.liveLayerDrawn > 0) RenderPathPaint.liveLayerDrawn--;

		if (Config.raw.brush_live && Context.raw.pdirty <= 0 && Context.raw.ddirty <= 0 && Context.raw.brushTime == 0) {
			// Depth is unchanged, draw before gbuffer gets updated
			RenderPathPaint.commandsLiveBrush();
		}
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

		///if (!krom_ios) // No hover on iPad, decals are painted by pen release
		if (Config.raw.brush_live && Context.raw.pdirty <= 0 && Context.raw.ddirty > 0 && Context.raw.brushTime == 0) {
			// gbuffer has been updated now but brush will lag 1 frame
			RenderPathPaint.commandsLiveBrush();
		}
		///end

		if (History.undoLayers != null) {
			RenderPathPaint.commandsSymmetry();

			if (Context.raw.pdirty > 0) RenderPathPaint.dilated = false;

			///if is_paint
			if (Context.raw.tool == WorkspaceTool.ToolBake) {

				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				let isRaytracedBake = (Context.raw.bakeType == BakeType.BakeAO  ||
					Context.raw.bakeType == BakeType.BakeLightmap ||
					Context.raw.bakeType == BakeType.BakeBentNormal ||
					Context.raw.bakeType == BakeType.BakeThickness);
				///end

				if (Context.raw.bakeType == BakeType.BakeNormal || Context.raw.bakeType == BakeType.BakeHeight || Context.raw.bakeType == BakeType.BakeDerivative) {
					if (!RenderPathPaint.baking && Context.raw.pdirty > 0) {
						RenderPathPaint.baking = true;
						let _bakeType = Context.raw.bakeType;
						Context.raw.bakeType = Context.raw.bakeType == BakeType.BakeNormal ? BakeType.BakeNormalObject : BakeType.BakePosition; // Bake high poly data
						MakeMaterial.parsePaintMaterial();
						let _paintObject = Context.raw.paintObject;
						let highPoly = Project.paintObjects[Context.raw.bakeHighPoly];
						let _visible = highPoly.base.visible;
						highPoly.base.visible = true;
						Context.selectPaintObject(highPoly);
						RenderPathPaint.commandsPaint();
						highPoly.base.visible = _visible;
						if (RenderPathPaint.pushUndoLast) History.paint();
						Context.selectPaintObject(_paintObject);

						let _renderFinal = () => {
							Context.raw.bakeType = _bakeType;
							MakeMaterial.parsePaintMaterial();
							Context.raw.pdirty = 1;
							RenderPathPaint.commandsPaint();
							Context.raw.pdirty = 0;
							RenderPathPaint.baking = false;
						}
						let _renderDeriv = () => {
							Context.raw.bakeType = BakeType.BakeHeight;
							MakeMaterial.parsePaintMaterial();
							Context.raw.pdirty = 1;
							RenderPathPaint.commandsPaint();
							Context.raw.pdirty = 0;
							if (RenderPathPaint.pushUndoLast) History.paint();
							App.notifyOnInit(_renderFinal);
						}
						let bakeType = Context.raw.bakeType as BakeType;
						App.notifyOnInit(bakeType == BakeType.BakeDerivative ? _renderDeriv : _renderFinal);
					}
				}
				else if (Context.raw.bakeType == BakeType.BakeObjectID) {
					let _layerFilter = Context.raw.layerFilter;
					let _paintObject = Context.raw.paintObject;
					let isMerged = Context.raw.mergedObject != null;
					let _visible = isMerged && Context.raw.mergedObject.base.visible;
					Context.raw.layerFilter = 1;
					if (isMerged) Context.raw.mergedObject.base.visible = false;

					for (let p of Project.paintObjects) {
						Context.selectPaintObject(p);
						RenderPathPaint.commandsPaint();
					}

					Context.raw.layerFilter = _layerFilter;
					Context.selectPaintObject(_paintObject);
					if (isMerged) Context.raw.mergedObject.base.visible = _visible;
				}
				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				else if (isRaytracedBake) {
					let dirty = RenderPathRaytraceBake.commands(MakeMaterial.parsePaintMaterial);
					if (dirty) UIHeader.headerHandle.redraws = 2;
					if (Config.raw.dilate == DilateType.DilateInstant) { // && Context.raw.pdirty == 1
						RenderPathPaint.dilate(true, false);
					}
				}
				///end
				else {
					RenderPathPaint.commandsPaint();
				}
			}
			else { // Paint
				RenderPathPaint.commandsPaint();
			}
			///end

			///if is_sculpt
			RenderPathPaint.commandsPaint();
			///end
		}

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

		if (Context.raw.paint2d) {
			RenderPathPaint.restorePlaneMesh();
		}
	}

	static setPlaneMesh = () => {
		Context.raw.paint2dView = true;
		RenderPathPaint.painto = Context.raw.paintObject;
		RenderPathPaint.visibles = [];
		for (let p of Project.paintObjects) {
			RenderPathPaint.visibles.push(p.base.visible);
			p.base.visible = false;
		}
		if (Context.raw.mergedObject != null) {
			RenderPathPaint.mergedObjectVisible = Context.raw.mergedObject.base.visible;
			Context.raw.mergedObject.base.visible = false;
		}

		let cam = Scene.camera;
		Mat4.setFrom(Context.raw.savedCamera, cam.base.transform.local);
		RenderPathPaint.savedFov = cam.data.fov;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let m = Mat4.identity();
		Mat4.translate(m, 0, 0, 0.5);
		Transform.setMatrix(cam.base.transform, m);
		cam.data.fov = Base.defaultFov;
		CameraObject.buildProjection(cam);
		CameraObject.buildMatrix(cam);

		let tw = 0.95 * UIView2D.panScale;
		let tx = UIView2D.panX / UIView2D.ww;
		let ty = UIView2D.panY / App.h();
		Mat4.setIdentity(m);
		Mat4.scale(m, Vec4.create(tw, tw, 1));
		Mat4.setLoc(m, Vec4.create(tx, ty, 0));
		let m2 = Mat4.identity();
		Mat4.getInverse(m2, Scene.camera.VP);
		Mat4.multmat(m, m2);

		let tiled = UIView2D.tiledShow;
		if (tiled && Scene.getChild(".PlaneTiled") == null) {
			// 3x3 planes
			let posa = [32767,0,-32767,0,10922,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,10922,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,32767,0,-32767,0,10922,0,10922,0,10922,0,-10922,0,32767,0,-10922,0,10922,0,32767,0,10922,0,10922,0,32767,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,10922,0,-32767,0,-10922,0,32767,0,-10922,0,10922,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,-32767,0,-10922,0,-32767,0,-32767,0,10922,0,-32767,0,-10922,0,-10922,0,-10922,0,-32767,0,32767,0,-32767,0,32767,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,32767,0,-32767,0,32767,0,10922,0,10922,0,10922,0,32767,0,-10922,0,32767,0,32767,0,10922,0,32767,0,32767,0,10922,0,32767,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,10922,0,32767,0,-10922,0,32767,0,10922,0,10922,0,10922,0,-10922,0,-32767,0,-10922,0,-10922,0,-32767,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,-10922,0];
			let nora = [0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767];
			let texa = [32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0];
			let inda = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53];
			let raw: TMeshData = {
				name: ".PlaneTiled",
				vertex_arrays: [
					{ attrib: "pos", values: new Int16Array(posa), data: "short4norm" },
					{ attrib: "nor", values: new Int16Array(nora), data: "short2norm" },
					{ attrib: "tex", values: new Int16Array(texa), data: "short2norm" }
				],
				index_arrays: [
					{ values: new Uint32Array(inda), material: 0 }
				],
				scale_pos: 1.5,
				scale_tex: 1.0
			};
			MeshData.create(raw, (md: TMeshData) => {
				let materials: TMaterialData[] = Scene.getChild(".Plane").ext.materials;
				let o = Scene.addMeshObject(md, materials);
				o.base.name = ".PlaneTiled";
			});
		}

		RenderPathPaint.planeo = Scene.getChild(tiled ? ".PlaneTiled" : ".Plane").ext;
		RenderPathPaint.planeo.base.visible = true;
		Context.raw.paintObject = RenderPathPaint.planeo;

		let v = Vec4.create();
		let sx = Vec4.vec4_length(Vec4.set(v, m._00, m._01, m._02));
		Quat.fromEuler(RenderPathPaint.planeo.base.transform.rot, -Math.PI / 2, 0, 0);
		Vec4.set(RenderPathPaint.planeo.base.transform.scale, sx, 1.0, sx);
		RenderPathPaint.planeo.base.transform.scale.z *= Config.getTextureResY() / Config.getTextureResX();
		Vec4.set(RenderPathPaint.planeo.base.transform.loc, m._30, -m._31, 0.0);
		Transform.buildMatrix(RenderPathPaint.planeo.base.transform);
	}

	static restorePlaneMesh = () => {
		Context.raw.paint2dView = false;
		RenderPathPaint.planeo.base.visible = false;
		Vec4.set(RenderPathPaint.planeo.base.transform.loc, 0.0, 0.0, 0.0);
		for (let i = 0; i < Project.paintObjects.length; ++i) {
			Project.paintObjects[i].base.visible = RenderPathPaint.visibles[i];
		}
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.base.visible = RenderPathPaint.mergedObjectVisible;
		}
		Context.raw.paintObject = RenderPathPaint.painto;
		Transform.setMatrix(Scene.camera.base.transform, Context.raw.savedCamera);
		Scene.camera.data.fov = RenderPathPaint.savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		CameraObject.buildProjection(Scene.camera);
		CameraObject.buildMatrix(Scene.camera);

		RenderPathBase.drawGbuffer();
	}

	static bindLayers = () => {
		///if is_paint
		let isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		let isMaterialTool = Context.raw.tool == WorkspaceTool.ToolMaterial;
		if (isLive || isMaterialTool) RenderPathPaint.useLiveLayer(true);
		///end

		for (let i = 0; i < Project.layers.length; ++i) {
			let l = Project.layers[i];
			RenderPath.bindTarget("texpaint" + l.id, "texpaint" + l.id);

			///if is_paint
			if (SlotLayer.isLayer(l)) {
				RenderPath.bindTarget("texpaint_nor" + l.id, "texpaint_nor" + l.id);
				RenderPath.bindTarget("texpaint_pack" + l.id, "texpaint_pack" + l.id);
			}
			///end
		}
	}

	static unbindLayers = () => {
		///if is_paint
		let isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		let isMaterialTool = Context.raw.tool == WorkspaceTool.ToolMaterial;
		if (isLive || isMaterialTool) RenderPathPaint.useLiveLayer(false);
		///end
	}

	static dilate = (base: bool, nor_pack: bool) => {
		///if is_paint
		if (Config.raw.dilate_radius > 0 && !Context.raw.paint2d) {
			UtilUV.cacheDilateMap();
			Base.makeTempImg();
			let tid = Context.raw.layer.id;
			if (base) {
				let texpaint = "texpaint";
				RenderPath.setTarget("temptex0");
				RenderPath.bindTarget(texpaint + tid, "tex");
				RenderPath.drawShader("shader_datas/copy_pass/copy_pass");
				RenderPath.setTarget(texpaint + tid);
				RenderPath.bindTarget("temptex0", "tex");
				RenderPath.drawShader("shader_datas/dilate_pass/dilate_pass");
			}
			if (nor_pack && !SlotLayer.isMask(Context.raw.layer)) {
				RenderPath.setTarget("temptex0");
				RenderPath.bindTarget("texpaint_nor" + tid, "tex");
				RenderPath.drawShader("shader_datas/copy_pass/copy_pass");
				RenderPath.setTarget("texpaint_nor" + tid);
				RenderPath.bindTarget("temptex0", "tex");
				RenderPath.drawShader("shader_datas/dilate_pass/dilate_pass");

				RenderPath.setTarget("temptex0");
				RenderPath.bindTarget("texpaint_pack" + tid, "tex");
				RenderPath.drawShader("shader_datas/copy_pass/copy_pass");
				RenderPath.setTarget("texpaint_pack" + tid);
				RenderPath.bindTarget("temptex0", "tex");
				RenderPath.drawShader("shader_datas/dilate_pass/dilate_pass");
			}
		}
		///end
	}
}
