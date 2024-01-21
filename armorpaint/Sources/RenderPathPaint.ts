
class RenderPathPaint {

	static liveLayer: SlotLayer = null;
	static liveLayerDrawn = 0;
	static liveLayerLocked = false;
	static path: RenderPath;
	static dilated = true;
	static initVoxels = true; // Bake AO
	static pushUndoLast: bool;
	static painto: MeshObject = null;
	static planeo: MeshObject = null;
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

	static init = (_path: RenderPath) => {
		RenderPathPaint.path = _path;

		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_colorid";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_posnortex_picker0";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			RenderPathPaint.path.createRenderTarget(t);
		}
		{
			let t = new RenderTargetRaw();
			t.name = "texpaint_posnortex_picker1";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			RenderPathPaint.path.createRenderTarget(t);
		}

		RenderPathPaint.path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		RenderPathPaint.path.loadShader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
		///if is_paint
		RenderPathPaint.path.loadShader("shader_datas/dilate_pass/dilate_pass");
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
				RenderPathPaint.path.setTarget("texparticle");
				RenderPathPaint.path.clearTarget(0x00000000);
				RenderPathPaint.path.bindTarget("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
					RenderPathPaint.path.bindTarget("gbuffer0", "gbuffer0");
				}

				let mo: MeshObject = Scene.active.getChild(".ParticleEmitter") as MeshObject;
				mo.visible = true;
				mo.render(RenderPathPaint.path.currentG, "mesh", RenderPathPaint.path.bindParams);
				mo.visible = false;

				mo = Scene.active.getChild(".Particle")as MeshObject;
				mo.visible = true;
				mo.render(RenderPathPaint.path.currentG, "mesh", RenderPathPaint.path.bindParams);
				mo.visible = false;
				RenderPathPaint.path.end();
			}

			///if is_paint
			if (Context.raw.tool == WorkspaceTool.ToolColorId) {
				RenderPathPaint.path.setTarget("texpaint_colorid");
				RenderPathPaint.path.clearTarget(0xff000000);
				RenderPathPaint.path.bindTarget("gbuffer2", "gbuffer2");
				RenderPathPaint.path.drawMeshes("paint");
				UIHeader.headerHandle.redraws = 2;
			}
			else if (Context.raw.tool == WorkspaceTool.ToolPicker || Context.raw.tool == WorkspaceTool.ToolMaterial) {
				if (Context.raw.pickPosNorTex) {
					if (Context.raw.paint2d) {
						RenderPathPaint.path.setTarget("gbuffer0", ["gbuffer1", "gbuffer2"]);
						RenderPathPaint.path.drawMeshes("mesh");
					}
					RenderPathPaint.path.setTarget("texpaint_posnortex_picker0", ["texpaint_posnortex_picker1"]);
					RenderPathPaint.path.bindTarget("gbuffer2", "gbuffer2");
					RenderPathPaint.path.bindTarget("_main", "gbufferD");
					RenderPathPaint.path.drawMeshes("paint");
					let texpaint_posnortex_picker0 = RenderPathPaint.path.renderTargets.get("texpaint_posnortex_picker0").image;
					let texpaint_posnortex_picker1 = RenderPathPaint.path.renderTargets.get("texpaint_posnortex_picker1").image;
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
					RenderPathPaint.path.setTarget("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					RenderPathPaint.path.bindTarget("gbuffer2", "gbuffer2");
					tid = Context.raw.layer.id;
					let useLiveLayer = Context.raw.tool == WorkspaceTool.ToolMaterial;
					if (useLiveLayer) RenderPathPaint.useLiveLayer(true);
					RenderPathPaint.path.bindTarget("texpaint" + tid, "texpaint");
					RenderPathPaint.path.bindTarget("texpaint_nor" + tid, "texpaint_nor");
					RenderPathPaint.path.bindTarget("texpaint_pack" + tid, "texpaint_pack");
					RenderPathPaint.path.drawMeshes("paint");
					if (useLiveLayer) RenderPathPaint.useLiveLayer(false);
					UIHeader.headerHandle.redraws = 2;
					UIBase.hwnds[2].redraws = 2;

					let texpaint_picker = RenderPathPaint.path.renderTargets.get("texpaint_picker").image;
					let texpaint_nor_picker = RenderPathPaint.path.renderTargets.get("texpaint_nor_picker").image;
					let texpaint_pack_picker = RenderPathPaint.path.renderTargets.get("texpaint_pack_picker").image;
					let texpaint_uv_picker = RenderPathPaint.path.renderTargets.get("texpaint_uv_picker").image;
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
					RenderPathPaint.path.clearImage("voxels", 0x00000000);
					RenderPathPaint.path.setTarget("");
					RenderPathPaint.path.setViewport(256, 256);
					RenderPathPaint.path.bindTarget("voxels", "voxels");
					RenderPathPaint.path.drawMeshes("voxel");
					RenderPathPaint.path.generateMipmaps("voxels");
				}
				///end

				let texpaint = "texpaint" + tid;
				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.brushTime == Time.delta) {
					// Clear to black on bake start
					RenderPathPaint.path.setTarget(texpaint);
					RenderPathPaint.path.clearTarget(0xff000000);
				}

				RenderPathPaint.path.setTarget("texpaint_blend1");
				RenderPathPaint.path.bindTarget("texpaint_blend0", "tex");
				RenderPathPaint.path.drawShader("shader_datas/copy_pass/copyR8_pass");
				let isMask = Context.raw.layer.isMask();
				if (isMask) {
					let ptid = Context.raw.layer.parent.id;
					if (Context.raw.layer.parent.isGroup()) { // Group mask
						for (let c of Context.raw.layer.parent.getChildren()) {
							ptid = c.id;
							break;
						}
					}
					RenderPathPaint.path.setTarget(texpaint, ["texpaint_nor" + ptid, "texpaint_pack" + ptid, "texpaint_blend0"]);
				}
				else {
					RenderPathPaint.path.setTarget(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);
				}
				RenderPathPaint.path.bindTarget("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
					RenderPathPaint.path.bindTarget("gbuffer0", "gbuffer0");
				}
				RenderPathPaint.path.bindTarget("texpaint_blend1", "paintmask");
				///if arm_voxels
				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.bakeType == BakeType.BakeAO) {
					RenderPathPaint.path.bindTarget("voxels", "voxels");
				}
				///end
				if (Context.raw.colorIdPicked) {
					RenderPathPaint.path.bindTarget("texpaint_colorid", "texpaint_colorid");
				}

				// Read texcoords from gbuffer
				let readTC = (Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillFace) ||
							  Context.raw.tool == WorkspaceTool.ToolClone ||
							  Context.raw.tool == WorkspaceTool.ToolBlur ||
							  Context.raw.tool == WorkspaceTool.ToolSmudge;
				if (readTC) {
					RenderPathPaint.path.bindTarget("gbuffer2", "gbuffer2");
				}

				RenderPathPaint.path.drawMeshes("paint");

				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.bakeType == BakeType.BakeCurvature && Context.raw.bakeCurvSmooth > 0) {
					if (RenderPathPaint.path.renderTargets.get("texpaint_blur") == null) {
						let t = new RenderTargetRaw();
						t.name = "texpaint_blur";
						t.width = Math.floor(Config.getTextureResX() * 0.95);
						t.height = Math.floor(Config.getTextureResY() * 0.95);
						t.format = "RGBA32";
						RenderPathPaint.path.createRenderTarget(t);
					}
					let blurs = Math.round(Context.raw.bakeCurvSmooth);
					for (let i = 0; i < blurs; ++i) {
						RenderPathPaint.path.setTarget("texpaint_blur");
						RenderPathPaint.path.bindTarget(texpaint, "tex");
						RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");
						RenderPathPaint.path.setTarget(texpaint);
						RenderPathPaint.path.bindTarget("texpaint_blur", "tex");
						RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");
					}
				}

				if (dilation && Config.raw.dilate == DilateType.DilateInstant) {
					RenderPathPaint.dilate(true, false);
				}
			}
			///end

			///if is_sculpt
			let texpaint = "texpaint" + tid;
			RenderPathPaint.path.setTarget("texpaint_blend1");
			RenderPathPaint.path.bindTarget("texpaint_blend0", "tex");
			RenderPathPaint.path.drawShader("shader_datas/copy_pass/copyR8_pass");
			RenderPathPaint.path.setTarget(texpaint, ["texpaint_blend0"]);
			RenderPathPaint.path.bindTarget("gbufferD_undo", "gbufferD");
			if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
				RenderPathPaint.path.bindTarget("gbuffer0", "gbuffer0");
			}
			RenderPathPaint.path.bindTarget("texpaint_blend1", "paintmask");

			// Read texcoords from gbuffer
			let readTC = (Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillFace) ||
						  Context.raw.tool == WorkspaceTool.ToolClone ||
						  Context.raw.tool == WorkspaceTool.ToolBlur ||
						  Context.raw.tool == WorkspaceTool.ToolSmudge;
			if (readTC) {
				RenderPathPaint.path.bindTarget("gbuffer2", "gbuffer2");
			}
			RenderPathPaint.path.bindTarget("gbuffer0_undo", "gbuffer0_undo");

			let materialContexts: MaterialContext[] = [];
			let shaderContexts: ShaderContext[] = [];
			let mats = Project.paintObjects[0].materials;
			Project.paintObjects[0].getContexts("paint", mats, materialContexts, shaderContexts);

			let cc_context = shaderContexts[0];
			if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
			RenderPathPaint.path.currentG.setPipeline(cc_context.pipeState);
			Uniforms.setContextConstants(RenderPathPaint.path.currentG, cc_context, RenderPathPaint.path.bindParams);
			Uniforms.setObjectConstants(RenderPathPaint.path.currentG, cc_context, Project.paintObjects[0]);
			Uniforms.setMaterialConstants(RenderPathPaint.path.currentG, cc_context, materialContexts[0]);
			RenderPathPaint.path.currentG.setVertexBuffer(ConstData.screenAlignedVB);
			RenderPathPaint.path.currentG.setIndexBuffer(ConstData.screenAlignedIB);
			RenderPathPaint.path.currentG.drawIndexedVertices();
			RenderPathPaint.path.end();
			///end
		}
	}

	static useLiveLayer = (use: bool) => {
		let tid = Context.raw.layer.id;
		let hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		if (use) {
			RenderPathPaint._texpaint = RenderPathPaint.path.renderTargets.get("texpaint" + tid);
			RenderPathPaint._texpaint_undo = RenderPathPaint.path.renderTargets.get("texpaint_undo" + hid);
			RenderPathPaint._texpaint_nor_undo = RenderPathPaint.path.renderTargets.get("texpaint_nor_undo" + hid);
			RenderPathPaint._texpaint_pack_undo = RenderPathPaint.path.renderTargets.get("texpaint_pack_undo" + hid);
			RenderPathPaint._texpaint_nor = RenderPathPaint.path.renderTargets.get("texpaint_nor" + tid);
			RenderPathPaint._texpaint_pack = RenderPathPaint.path.renderTargets.get("texpaint_pack" + tid);
			RenderPathPaint.path.renderTargets.set("texpaint_undo" + hid, RenderPathPaint.path.renderTargets.get("texpaint" + tid));
			RenderPathPaint.path.renderTargets.set("texpaint" + tid, RenderPathPaint.path.renderTargets.get("texpaint_live"));
			if (Context.raw.layer.isLayer()) {
				RenderPathPaint.path.renderTargets.set("texpaint_nor_undo" + hid, RenderPathPaint.path.renderTargets.get("texpaint_nor" + tid));
				RenderPathPaint.path.renderTargets.set("texpaint_pack_undo" + hid, RenderPathPaint.path.renderTargets.get("texpaint_pack" + tid));
				RenderPathPaint.path.renderTargets.set("texpaint_nor" + tid, RenderPathPaint.path.renderTargets.get("texpaint_nor_live"));
				RenderPathPaint.path.renderTargets.set("texpaint_pack" + tid, RenderPathPaint.path.renderTargets.get("texpaint_pack_live"));
			}
		}
		else {
			RenderPathPaint.path.renderTargets.set("texpaint" + tid, RenderPathPaint._texpaint);
			RenderPathPaint.path.renderTargets.set("texpaint_undo" + hid, RenderPathPaint._texpaint_undo);
			if (Context.raw.layer.isLayer()) {
				RenderPathPaint.path.renderTargets.set("texpaint_nor_undo" + hid, RenderPathPaint._texpaint_nor_undo);
				RenderPathPaint.path.renderTargets.set("texpaint_pack_undo" + hid, RenderPathPaint._texpaint_pack_undo);
				RenderPathPaint.path.renderTargets.set("texpaint_nor" + tid, RenderPathPaint._texpaint_nor);
				RenderPathPaint.path.renderTargets.set("texpaint_pack" + tid, RenderPathPaint._texpaint_pack);
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
			RenderPathPaint.liveLayer = new SlotLayer("_live");
		}

		let tid = Context.raw.layer.id;
		if (Context.raw.layer.isMask()) {
			RenderPathPaint.path.setTarget("texpaint_live");
			RenderPathPaint.path.bindTarget("texpaint" + tid, "tex");
			RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");
		}
		else {
			RenderPathPaint.path.setTarget("texpaint_live", ["texpaint_nor_live", "texpaint_pack_live"]);
			RenderPathPaint.path.bindTarget("texpaint" + tid, "tex0");
			RenderPathPaint.path.bindTarget("texpaint_nor" + tid, "tex1");
			RenderPathPaint.path.bindTarget("texpaint_pack" + tid, "tex2");
			RenderPathPaint.path.drawShader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
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
		let groupLayer = Context.raw.layer.isGroup();
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
		let plane = (Scene.active.getChild(".Plane") as MeshObject);
		let geom = plane.data;

		let g = RenderPathPaint.path.frameG;
		if (Base.pipeCursor == null) Base.makeCursorPipe();

		RenderPathPaint.path.setTarget("");
		g.setPipeline(Base.pipeCursor);
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
		let img = (decal && !decalMask) ? Context.raw.decalImage : Res.get("cursor.k");
		g.setTexture(Base.cursorTex, img);
		let gbuffer0 = RenderPathPaint.path.renderTargets.get("gbuffer0").image;
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
		RenderPathPaint.path.end();
	}

	static commandsSymmetry = () => {
		if (Context.raw.symX || Context.raw.symY || Context.raw.symZ) {
			Context.raw.ddirty = 2;
			let t = Context.raw.paintObject.transform;
			let sx = t.scale.x;
			let sy = t.scale.y;
			let sz = t.scale.z;
			if (Context.raw.symX) {
				t.scale.set(-sx, sy, sz);
				t.buildMatrix();
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symY) {
				t.scale.set(sx, -sy, sz);
				t.buildMatrix();
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symZ) {
				t.scale.set(sx, sy, -sz);
				t.buildMatrix();
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY) {
				t.scale.set(-sx, -sy, sz);
				t.buildMatrix();
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symZ) {
				t.scale.set(-sx, sy, -sz);
				t.buildMatrix();
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symY && Context.raw.symZ) {
				t.scale.set(sx, -sy, -sz);
				t.buildMatrix();
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY && Context.raw.symZ) {
				t.scale.set(-sx, -sy, -sz);
				t.buildMatrix();
				RenderPathPaint.commandsPaint(false);
			}
			t.scale.set(sx, sy, sz);
			t.buildMatrix();
		}
	}

	static paintEnabled = (): bool => {
		///if is_paint
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != WorkspaceTool.ToolPicker && Context.raw.tool != WorkspaceTool.ToolMaterial && Context.raw.tool != WorkspaceTool.ToolColorId;
		///end

		///if is_sculpt
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != WorkspaceTool.ToolPicker && Context.raw.tool != WorkspaceTool.ToolMaterial;
		///end

		let groupLayer = Context.raw.layer.isGroup();
		return !fillLayer && !groupLayer && !Context.raw.foregroundEvent;
	}

	static liveBrushDirty = () => {
		let mouse = Input.getMouse();
		let mx = RenderPathPaint.lastX;
		let my = RenderPathPaint.lastY;
		RenderPathPaint.lastX = mouse.viewX;
		RenderPathPaint.lastY = mouse.viewY;
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
			RenderPathPaint.path.setTarget("gbuffer0_undo");
			RenderPathPaint.path.bindTarget("gbuffer0", "tex");
			RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");

			RenderPathPaint.path.setTarget("gbufferD_undo");
			RenderPathPaint.path.bindTarget("_main", "tex");
			RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");
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
						let _visible = highPoly.visible;
						highPoly.visible = true;
						Context.selectPaintObject(highPoly);
						RenderPathPaint.commandsPaint();
						highPoly.visible = _visible;
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
					let _visible = isMerged && Context.raw.mergedObject.visible;
					Context.raw.layerFilter = 1;
					if (isMerged) Context.raw.mergedObject.visible = false;

					for (let p of Project.paintObjects) {
						Context.selectPaintObject(p);
						RenderPathPaint.commandsPaint();
					}

					Context.raw.layerFilter = _layerFilter;
					Context.selectPaintObject(_paintObject);
					if (isMerged) Context.raw.mergedObject.visible = _visible;
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
			RenderPathPaint.path.setTarget("texpaint_blend0");
			RenderPathPaint.path.clearTarget(0x00000000);
			RenderPathPaint.path.setTarget("texpaint_blend1");
			RenderPathPaint.path.clearTarget(0x00000000);
			///else
			RenderPathPaint.path.setTarget("texpaint_blend0", ["texpaint_blend1"]);
			RenderPathPaint.path.clearTarget(0x00000000);
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
			RenderPathPaint.visibles.push(p.visible);
			p.visible = false;
		}
		if (Context.raw.mergedObject != null) {
			RenderPathPaint.mergedObjectVisible = Context.raw.mergedObject.visible;
			Context.raw.mergedObject.visible = false;
		}

		let cam = Scene.active.camera;
		Context.raw.savedCamera.setFrom(cam.transform.local);
		RenderPathPaint.savedFov = cam.data.raw.fov;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let m = Mat4.identity();
		m.translate(0, 0, 0.5);
		cam.transform.setMatrix(m);
		cam.data.raw.fov = Base.defaultFov;
		cam.buildProjection();
		cam.buildMatrix();

		let tw = 0.95 * UIView2D.panScale;
		let tx = UIView2D.panX / UIView2D.ww;
		let ty = UIView2D.panY / App.h();
		m.setIdentity();
		m.scale(new Vec4(tw, tw, 1));
		m.setLoc(new Vec4(tx, ty, 0));
		let m2 = Mat4.identity();
		m2.getInverse(Scene.active.camera.VP);
		m.multmat(m2);

		let tiled = UIView2D.tiledShow;
		if (tiled && Scene.active.getChild(".PlaneTiled") == null) {
			// 3x3 planes
			let posa = [32767,0,-32767,0,10922,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,10922,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,32767,0,-32767,0,10922,0,10922,0,10922,0,-10922,0,32767,0,-10922,0,10922,0,32767,0,10922,0,10922,0,32767,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,10922,0,-32767,0,-10922,0,32767,0,-10922,0,10922,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,-32767,0,-10922,0,-32767,0,-32767,0,10922,0,-32767,0,-10922,0,-10922,0,-10922,0,-32767,0,32767,0,-32767,0,32767,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,32767,0,-32767,0,32767,0,10922,0,10922,0,10922,0,32767,0,-10922,0,32767,0,32767,0,10922,0,32767,0,32767,0,10922,0,32767,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,10922,0,32767,0,-10922,0,32767,0,10922,0,10922,0,10922,0,-10922,0,-32767,0,-10922,0,-10922,0,-32767,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,-10922,0];
			let nora = [0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767];
			let texa = [32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0];
			let inda = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53];
			let raw: TMeshData = {
				name: ".PlaneTiled",
				vertex_arrays: [
					{ attrib: "pos", values: RenderPathPaint.array_i16(posa), data: "short4norm" },
					{ attrib: "nor", values: RenderPathPaint.array_i16(nora), data: "short2norm" },
					{ attrib: "tex", values: RenderPathPaint.array_i16(texa), data: "short2norm" }
				],
				index_arrays: [
					{ values: RenderPathPaint.array_u32(inda), material: 0 }
				],
				scale_pos: 1.5,
				scale_tex: 1.0
			};
			new MeshData(raw, (md: MeshData) => {
				let materials = (Scene.active.getChild(".Plane") as MeshObject).materials;
				let o = Scene.active.addMeshObject(md, materials);
				o.name = ".PlaneTiled";
			});
		}

		RenderPathPaint.planeo = Scene.active.getChild(tiled ? ".PlaneTiled" : ".Plane") as MeshObject;
		RenderPathPaint.planeo.visible = true;
		Context.raw.paintObject = RenderPathPaint.planeo;

		let v = new Vec4();
		let sx = v.set(m._00, m._01, m._02).length();
		RenderPathPaint.planeo.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		RenderPathPaint.planeo.transform.scale.set(sx, 1.0, sx);
		RenderPathPaint.planeo.transform.scale.z *= Config.getTextureResY() / Config.getTextureResX();
		RenderPathPaint.planeo.transform.loc.set(m._30, -m._31, 0.0);
		RenderPathPaint.planeo.transform.buildMatrix();
	}

	static restorePlaneMesh = () => {
		Context.raw.paint2dView = false;
		RenderPathPaint.planeo.visible = false;
		RenderPathPaint.planeo.transform.loc.set(0.0, 0.0, 0.0);
		for (let i = 0; i < Project.paintObjects.length; ++i) {
			Project.paintObjects[i].visible = RenderPathPaint.visibles[i];
		}
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.visible = RenderPathPaint.mergedObjectVisible;
		}
		Context.raw.paintObject = RenderPathPaint.painto;
		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Scene.active.camera.data.raw.fov = RenderPathPaint.savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

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
			RenderPathPaint.path.bindTarget("texpaint" + l.id, "texpaint" + l.id);

			///if is_paint
			if (l.isLayer()) {
				RenderPathPaint.path.bindTarget("texpaint_nor" + l.id, "texpaint_nor" + l.id);
				RenderPathPaint.path.bindTarget("texpaint_pack" + l.id, "texpaint_pack" + l.id);
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
				RenderPathPaint.path.setTarget("temptex0");
				RenderPathPaint.path.bindTarget(texpaint + tid, "tex");
				RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");
				RenderPathPaint.path.setTarget(texpaint + tid);
				RenderPathPaint.path.bindTarget("temptex0", "tex");
				RenderPathPaint.path.drawShader("shader_datas/dilate_pass/dilate_pass");
			}
			if (nor_pack && !Context.raw.layer.isMask()) {
				RenderPathPaint.path.setTarget("temptex0");
				RenderPathPaint.path.bindTarget("texpaint_nor" + tid, "tex");
				RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");
				RenderPathPaint.path.setTarget("texpaint_nor" + tid);
				RenderPathPaint.path.bindTarget("temptex0", "tex");
				RenderPathPaint.path.drawShader("shader_datas/dilate_pass/dilate_pass");

				RenderPathPaint.path.setTarget("temptex0");
				RenderPathPaint.path.bindTarget("texpaint_pack" + tid, "tex");
				RenderPathPaint.path.drawShader("shader_datas/copy_pass/copy_pass");
				RenderPathPaint.path.setTarget("texpaint_pack" + tid);
				RenderPathPaint.path.bindTarget("temptex0", "tex");
				RenderPathPaint.path.drawShader("shader_datas/dilate_pass/dilate_pass");
			}
		}
		///end
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
