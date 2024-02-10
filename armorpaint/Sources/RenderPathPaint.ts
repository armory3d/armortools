
class RenderPathPaint {

	static liveLayer: SlotLayerRaw = null;
	static liveLayerDrawn = 0;
	static liveLayerLocked = false;
	static dilated = true;
	static initVoxels = true; // Bake AO
	static pushUndoLast: bool;
	static painto: mesh_object_t = null;
	static planeo: mesh_object_t = null;
	static visibles: bool[] = null;
	static mergedObjectVisible = false;
	static savedFov = 0.0;
	static baking = false;
	static _texpaint: render_target_t;
	static _texpaint_nor: render_target_t;
	static _texpaint_pack: render_target_t;
	static _texpaint_undo: render_target_t;
	static _texpaint_nor_undo: render_target_t;
	static _texpaint_pack_undo: render_target_t;
	static lastX = -1.0;
	static lastY = -1.0;

	static init = () => {

		{
			let t = render_target_create();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_colorid";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_posnortex_picker0";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_posnortex_picker1";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA128";
			render_path_create_render_target(t);
		}

		render_path_load_shader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		render_path_load_shader("shader_datas/copy_mrt3_pass/copy_mrt3RGBA64_pass");
		///if is_paint
		render_path_load_shader("shader_datas/dilate_pass/dilate_pass");
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
				render_path_set_target("texparticle");
				render_path_clear_target(0x00000000);
				render_path_bind_target("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
					render_path_bind_target("gbuffer0", "gbuffer0");
				}

				let mo: mesh_object_t = scene_get_child(".ParticleEmitter").ext;
				mo.base.visible = true;
				mesh_object_render(mo, "mesh",_render_path_bind_params);
				mo.base.visible = false;

				mo = scene_get_child(".Particle").ext;
				mo.base.visible = true;
				mesh_object_render(mo, "mesh",_render_path_bind_params);
				mo.base.visible = false;
				render_path_end();
			}

			///if is_paint
			if (Context.raw.tool == WorkspaceTool.ToolColorId) {
				render_path_set_target("texpaint_colorid");
				render_path_clear_target(0xff000000);
				render_path_bind_target("gbuffer2", "gbuffer2");
				render_path_draw_meshes("paint");
				UIHeader.headerHandle.redraws = 2;
			}
			else if (Context.raw.tool == WorkspaceTool.ToolPicker || Context.raw.tool == WorkspaceTool.ToolMaterial) {
				if (Context.raw.pickPosNorTex) {
					if (Context.raw.paint2d) {
						render_path_set_target("gbuffer0", ["gbuffer1", "gbuffer2"]);
						render_path_draw_meshes("mesh");
					}
					render_path_set_target("texpaint_posnortex_picker0", ["texpaint_posnortex_picker1"]);
					render_path_bind_target("gbuffer2", "gbuffer2");
					render_path_bind_target("_main", "gbufferD");
					render_path_draw_meshes("paint");
					let texpaint_posnortex_picker0 = render_path_render_targets.get("texpaint_posnortex_picker0").image;
					let texpaint_posnortex_picker1 = render_path_render_targets.get("texpaint_posnortex_picker1").image;
					let a = new DataView(image_get_pixels(texpaint_posnortex_picker0));
					let b = new DataView(image_get_pixels(texpaint_posnortex_picker1));
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
					render_path_set_target("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					render_path_bind_target("gbuffer2", "gbuffer2");
					tid = Context.raw.layer.id;
					let useLiveLayer = Context.raw.tool == WorkspaceTool.ToolMaterial;
					if (useLiveLayer) RenderPathPaint.useLiveLayer(true);
					render_path_bind_target("texpaint" + tid, "texpaint");
					render_path_bind_target("texpaint_nor" + tid, "texpaint_nor");
					render_path_bind_target("texpaint_pack" + tid, "texpaint_pack");
					render_path_draw_meshes("paint");
					if (useLiveLayer) RenderPathPaint.useLiveLayer(false);
					UIHeader.headerHandle.redraws = 2;
					UIBase.hwnds[2].redraws = 2;

					let texpaint_picker = render_path_render_targets.get("texpaint_picker").image;
					let texpaint_nor_picker = render_path_render_targets.get("texpaint_nor_picker").image;
					let texpaint_pack_picker = render_path_render_targets.get("texpaint_pack_picker").image;
					let texpaint_uv_picker = render_path_render_targets.get("texpaint_uv_picker").image;
					let a = new DataView(image_get_pixels(texpaint_picker));
					let b = new DataView(image_get_pixels(texpaint_nor_picker));
					let c = new DataView(image_get_pixels(texpaint_pack_picker));
					let d = new DataView(image_get_pixels(texpaint_uv_picker));

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
					render_path_clear_image("voxels", 0x00000000);
					render_path_set_target("");
					render_path_set_viewport(256, 256);
					render_path_bind_target("voxels", "voxels");
					render_path_draw_meshes("voxel");
					render_path_gen_mipmaps("voxels");
				}
				///end

				let texpaint = "texpaint" + tid;
				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.brushTime == time_delta()) {
					// Clear to black on bake start
					render_path_set_target(texpaint);
					render_path_clear_target(0xff000000);
				}

				render_path_set_target("texpaint_blend1");
				render_path_bind_target("texpaint_blend0", "tex");
				render_path_draw_shader("shader_datas/copy_pass/copyR8_pass");
				let isMask = SlotLayer.isMask(Context.raw.layer);
				if (isMask) {
					let ptid = Context.raw.layer.parent.id;
					if (SlotLayer.isGroup(Context.raw.layer.parent)) { // Group mask
						for (let c of SlotLayer.getChildren(Context.raw.layer.parent)) {
							ptid = c.id;
							break;
						}
					}
					render_path_set_target(texpaint, ["texpaint_nor" + ptid, "texpaint_pack" + ptid, "texpaint_blend0"]);
				}
				else {
					render_path_set_target(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);
				}
				render_path_bind_target("_main", "gbufferD");
				if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
					render_path_bind_target("gbuffer0", "gbuffer0");
				}
				render_path_bind_target("texpaint_blend1", "paintmask");
				///if arm_voxels
				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.bakeType == BakeType.BakeAO) {
					render_path_bind_target("voxels", "voxels");
				}
				///end
				if (Context.raw.colorIdPicked) {
					render_path_bind_target("texpaint_colorid", "texpaint_colorid");
				}

				// Read texcoords from gbuffer
				let readTC = (Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillFace) ||
							  Context.raw.tool == WorkspaceTool.ToolClone ||
							  Context.raw.tool == WorkspaceTool.ToolBlur ||
							  Context.raw.tool == WorkspaceTool.ToolSmudge;
				if (readTC) {
					render_path_bind_target("gbuffer2", "gbuffer2");
				}

				render_path_draw_meshes("paint");

				if (Context.raw.tool == WorkspaceTool.ToolBake && Context.raw.bakeType == BakeType.BakeCurvature && Context.raw.bakeCurvSmooth > 0) {
					if (render_path_render_targets.get("texpaint_blur") == null) {
						let t = render_target_create();
						t.name = "texpaint_blur";
						t.width = Math.floor(Config.getTextureResX() * 0.95);
						t.height = Math.floor(Config.getTextureResY() * 0.95);
						t.format = "RGBA32";
						render_path_create_render_target(t);
					}
					let blurs = Math.round(Context.raw.bakeCurvSmooth);
					for (let i = 0; i < blurs; ++i) {
						render_path_set_target("texpaint_blur");
						render_path_bind_target(texpaint, "tex");
						render_path_draw_shader("shader_datas/copy_pass/copy_pass");
						render_path_set_target(texpaint);
						render_path_bind_target("texpaint_blur", "tex");
						render_path_draw_shader("shader_datas/copy_pass/copy_pass");
					}
				}

				if (dilation && Config.raw.dilate == DilateType.DilateInstant) {
					RenderPathPaint.dilate(true, false);
				}
			}
			///end

			///if is_sculpt
			let texpaint = "texpaint" + tid;
			render_path_set_target("texpaint_blend1");
			render_path_bind_target("texpaint_blend0", "tex");
			render_path_draw_shader("shader_datas/copy_pass/copyR8_pass");
			render_path_set_target(texpaint, ["texpaint_blend0"]);
			render_path_bind_target("gbufferD_undo", "gbufferD");
			if ((Context.raw.xray || Config.raw.brush_angle_reject) && Config.raw.brush_3d) {
				render_path_bind_target("gbuffer0", "gbuffer0");
			}
			render_path_bind_target("texpaint_blend1", "paintmask");

			// Read texcoords from gbuffer
			let readTC = (Context.raw.tool == WorkspaceTool.ToolFill && Context.raw.fillTypeHandle.position == FillType.FillFace) ||
						  Context.raw.tool == WorkspaceTool.ToolClone ||
						  Context.raw.tool == WorkspaceTool.ToolBlur ||
						  Context.raw.tool == WorkspaceTool.ToolSmudge;
			if (readTC) {
				render_path_bind_target("gbuffer2", "gbuffer2");
			}
			render_path_bind_target("gbuffer0_undo", "gbuffer0_undo");

			let materialContexts: material_context_t[] = [];
			let shaderContexts: shader_context_t[] = [];
			let mats = Project.paintObjects[0].materials;
			mesh_object_get_contexts(Project.paintObjects[0], "paint", mats, materialContexts, shaderContexts);

			let cc_context = shaderContexts[0];
			if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
			g4_set_pipeline(cc_context._pipe_state);
			uniforms_set_context_consts(cc_context,_render_path_bind_params);
			uniforms_set_obj_consts(cc_context, Project.paintObjects[0].base);
			uniforms_set_material_consts(cc_context, materialContexts[0]);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			render_path_end();
			///end
		}
	}

	static useLiveLayer = (use: bool) => {
		let tid = Context.raw.layer.id;
		let hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		if (use) {
			RenderPathPaint._texpaint = render_path_render_targets.get("texpaint" + tid);
			RenderPathPaint._texpaint_undo = render_path_render_targets.get("texpaint_undo" + hid);
			RenderPathPaint._texpaint_nor_undo = render_path_render_targets.get("texpaint_nor_undo" + hid);
			RenderPathPaint._texpaint_pack_undo = render_path_render_targets.get("texpaint_pack_undo" + hid);
			RenderPathPaint._texpaint_nor = render_path_render_targets.get("texpaint_nor" + tid);
			RenderPathPaint._texpaint_pack = render_path_render_targets.get("texpaint_pack" + tid);
			render_path_render_targets.set("texpaint_undo" + hid,render_path_render_targets.get("texpaint" + tid));
			render_path_render_targets.set("texpaint" + tid,render_path_render_targets.get("texpaint_live"));
			if (SlotLayer.isLayer(Context.raw.layer)) {
				render_path_render_targets.set("texpaint_nor_undo" + hid,render_path_render_targets.get("texpaint_nor" + tid));
				render_path_render_targets.set("texpaint_pack_undo" + hid,render_path_render_targets.get("texpaint_pack" + tid));
				render_path_render_targets.set("texpaint_nor" + tid,render_path_render_targets.get("texpaint_nor_live"));
				render_path_render_targets.set("texpaint_pack" + tid,render_path_render_targets.get("texpaint_pack_live"));
			}
		}
		else {
			render_path_render_targets.set("texpaint" + tid, RenderPathPaint._texpaint);
			render_path_render_targets.set("texpaint_undo" + hid, RenderPathPaint._texpaint_undo);
			if (SlotLayer.isLayer(Context.raw.layer)) {
				render_path_render_targets.set("texpaint_nor_undo" + hid, RenderPathPaint._texpaint_nor_undo);
				render_path_render_targets.set("texpaint_pack_undo" + hid, RenderPathPaint._texpaint_pack_undo);
				render_path_render_targets.set("texpaint_nor" + tid, RenderPathPaint._texpaint_nor);
				render_path_render_targets.set("texpaint_pack" + tid, RenderPathPaint._texpaint_pack);
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
			render_path_set_target("texpaint_live");
			render_path_bind_target("texpaint" + tid, "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");
		}
		else {
			render_path_set_target("texpaint_live", ["texpaint_nor_live", "texpaint_pack_live"]);
			render_path_bind_target("texpaint" + tid, "tex0");
			render_path_bind_target("texpaint_nor" + tid, "tex1");
			render_path_bind_target("texpaint_pack" + tid, "tex2");
			render_path_draw_shader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
		}

		RenderPathPaint.useLiveLayer(true);

		RenderPathPaint.liveLayerDrawn = 2;

		UIView2D.hwnd.redraws = 2;
		let _x = Context.raw.paintVec.x;
		let _y = Context.raw.paintVec.y;
		if (Context.raw.brushLocked) {
			Context.raw.paintVec.x = (Context.raw.lockStartedX - app_x()) / app_w();
			Context.raw.paintVec.y = (Context.raw.lockStartedY - app_y()) / app_h();
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
			mx = (Context.raw.lockStartedX - app_x()) / app_w();
			my = 1.0 - (Context.raw.lockStartedY - app_y()) / app_h();
		}
		let radius = decalMask ? Context.raw.brushDecalMaskRadius : Context.raw.brushRadius;
		RenderPathPaint.drawCursor(mx, my, Context.raw.brushNodesRadius * radius / 3.4);
	}

	static drawCursor = (mx: f32, my: f32, radius: f32, tintR = 1.0, tintG = 1.0, tintB = 1.0) => {
		let plane: mesh_object_t = scene_get_child(".Plane").ext;
		let geom = plane.data;

		if (Base.pipeCursor == null) Base.makeCursorPipe();

		render_path_set_target("");
		g4_set_pipeline(Base.pipeCursor);
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
		let img = (decal && !decalMask) ? Context.raw.decalImage : Res.get("cursor.k");
		g4_set_tex(Base.cursorTex, img);
		let gbuffer0 = render_path_render_targets.get("gbuffer0").image;
		g4_set_tex_depth(Base.cursorGbufferD, gbuffer0);
		g4_set_float2(Base.cursorMouse, mx, my);
		g4_set_float2(Base.cursorTexStep, 1 / gbuffer0.width, 1 / gbuffer0.height);
		g4_set_float(Base.cursorRadius, radius);
		let right = vec4_normalize(camera_object_right_world(scene_camera));
		g4_set_float3(Base.cursorCameraRight, right.x, right.y, right.z);
		g4_set_float3(Base.cursorTint, tintR, tintG, tintB);
		g4_set_mat(Base.cursorVP, scene_camera.vp);
		let helpMat = mat4_identity();
		mat4_get_inv(helpMat, scene_camera.vp);
		g4_set_mat(Base.cursorInvVP, helpMat);
		///if (krom_metal || krom_vulkan)
		g4_set_vertex_buffer(mesh_data_get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		g4_set_vertex_buffer(geom._vertex_buffer);
		///end
		g4_set_index_buffer(geom._index_buffers[0]);
		g4_draw();

		g4_disable_scissor();
		render_path_end();
	}

	static commandsSymmetry = () => {
		if (Context.raw.symX || Context.raw.symY || Context.raw.symZ) {
			Context.raw.ddirty = 2;
			let t = Context.raw.paintObject.base.transform;
			let sx = t.scale.x;
			let sy = t.scale.y;
			let sz = t.scale.z;
			if (Context.raw.symX) {
				vec4_set(t.scale, -sx, sy, sz);
				transform_build_matrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symY) {
				vec4_set(t.scale, sx, -sy, sz);
				transform_build_matrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symZ) {
				vec4_set(t.scale, sx, sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY) {
				vec4_set(t.scale, -sx, -sy, sz);
				transform_build_matrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symZ) {
				vec4_set(t.scale, -sx, sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symY && Context.raw.symZ) {
				vec4_set(t.scale, sx, -sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			if (Context.raw.symX && Context.raw.symY && Context.raw.symZ) {
				vec4_set(t.scale, -sx, -sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commandsPaint(false);
			}
			vec4_set(t.scale, sx, sy, sz);
			transform_build_matrix(t);
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
		RenderPathPaint.lastX = mouse_view_x();
		RenderPathPaint.lastY = mouse_view_y();
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
			render_path_set_target("gbuffer0_undo");
			render_path_bind_target("gbuffer0", "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");

			render_path_set_target("gbufferD_undo");
			render_path_bind_target("_main", "tex");
			render_path_draw_shader("shader_datas/copy_pass/copy_pass");
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
							app_notify_on_init(_renderFinal);
						}
						let bakeType = Context.raw.bakeType as BakeType;
						app_notify_on_init(bakeType == BakeType.BakeDerivative ? _renderDeriv : _renderFinal);
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
			render_path_set_target("texpaint_blend0");
			render_path_clear_target(0x00000000);
			render_path_set_target("texpaint_blend1");
			render_path_clear_target(0x00000000);
			///else
			render_path_set_target("texpaint_blend0", ["texpaint_blend1"]);
			render_path_clear_target(0x00000000);
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

		let cam = scene_camera;
		mat4_set_from(Context.raw.savedCamera, cam.base.transform.local);
		RenderPathPaint.savedFov = cam.data.fov;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let m = mat4_identity();
		mat4_translate(m, 0, 0, 0.5);
		transform_set_matrix(cam.base.transform, m);
		cam.data.fov = Base.defaultFov;
		camera_object_build_proj(cam);
		camera_object_build_mat(cam);

		let tw = 0.95 * UIView2D.panScale;
		let tx = UIView2D.panX / UIView2D.ww;
		let ty = UIView2D.panY / app_h();
		mat4_set_identity(m);
		mat4_scale(m, vec4_create(tw, tw, 1));
		mat4_set_loc(m, vec4_create(tx, ty, 0));
		let m2 = mat4_identity();
		mat4_get_inv(m2, scene_camera.vp);
		mat4_mult_mat(m, m2);

		let tiled = UIView2D.tiledShow;
		if (tiled && scene_get_child(".PlaneTiled") == null) {
			// 3x3 planes
			let posa = [32767,0,-32767,0,10922,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,10922,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,32767,0,-32767,0,10922,0,10922,0,10922,0,-10922,0,32767,0,-10922,0,10922,0,32767,0,10922,0,10922,0,32767,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,10922,0,-32767,0,-10922,0,32767,0,-10922,0,10922,0,10922,0,10922,0,-10922,0,-10922,0,-32767,0,-32767,0,-10922,0,-32767,0,-32767,0,10922,0,-32767,0,-10922,0,-10922,0,-10922,0,-32767,0,32767,0,-32767,0,32767,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,10922,0,-10922,0,10922,0,-10922,0,10922,0,-10922,0,32767,0,-32767,0,32767,0,10922,0,10922,0,10922,0,32767,0,-10922,0,32767,0,32767,0,10922,0,32767,0,32767,0,10922,0,32767,0,-10922,0,-10922,0,-10922,0,10922,0,-32767,0,10922,0,32767,0,-10922,0,32767,0,10922,0,10922,0,10922,0,-10922,0,-32767,0,-10922,0,-10922,0,-32767,0,-10922,0,10922,0,-32767,0,10922,0,-10922,0,-10922,0,-10922,0];
			let nora = [0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767,0,-32767];
			let texa = [32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0,32767,32767,32767,0,0,0];
			let inda = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53];
			let raw: mesh_data_t = {
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
			mesh_data_create(raw, (md: mesh_data_t) => {
				let materials: material_data_t[] = scene_get_child(".Plane").ext.materials;
				let o = scene_add_mesh_object(md, materials);
				o.base.name = ".PlaneTiled";
			});
		}

		RenderPathPaint.planeo = scene_get_child(tiled ? ".PlaneTiled" : ".Plane").ext;
		RenderPathPaint.planeo.base.visible = true;
		Context.raw.paintObject = RenderPathPaint.planeo;

		let v = vec4_create();
		let sx = vec4_len(vec4_set(v, m.m[0], m.m[1], m.m[2]));
		quat_from_euler(RenderPathPaint.planeo.base.transform.rot, -Math.PI / 2, 0, 0);
		vec4_set(RenderPathPaint.planeo.base.transform.scale, sx, 1.0, sx);
		RenderPathPaint.planeo.base.transform.scale.z *= Config.getTextureResY() / Config.getTextureResX();
		vec4_set(RenderPathPaint.planeo.base.transform.loc, m.m[12], -m.m[13], 0.0);
		transform_build_matrix(RenderPathPaint.planeo.base.transform);
	}

	static restorePlaneMesh = () => {
		Context.raw.paint2dView = false;
		RenderPathPaint.planeo.base.visible = false;
		vec4_set(RenderPathPaint.planeo.base.transform.loc, 0.0, 0.0, 0.0);
		for (let i = 0; i < Project.paintObjects.length; ++i) {
			Project.paintObjects[i].base.visible = RenderPathPaint.visibles[i];
		}
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.base.visible = RenderPathPaint.mergedObjectVisible;
		}
		Context.raw.paintObject = RenderPathPaint.painto;
		transform_set_matrix(scene_camera.base.transform, Context.raw.savedCamera);
		scene_camera.data.fov = RenderPathPaint.savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

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
			render_path_bind_target("texpaint" + l.id, "texpaint" + l.id);

			///if is_paint
			if (SlotLayer.isLayer(l)) {
				render_path_bind_target("texpaint_nor" + l.id, "texpaint_nor" + l.id);
				render_path_bind_target("texpaint_pack" + l.id, "texpaint_pack" + l.id);
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
				render_path_set_target("temptex0");
				render_path_bind_target(texpaint + tid, "tex");
				render_path_draw_shader("shader_datas/copy_pass/copy_pass");
				render_path_set_target(texpaint + tid);
				render_path_bind_target("temptex0", "tex");
				render_path_draw_shader("shader_datas/dilate_pass/dilate_pass");
			}
			if (nor_pack && !SlotLayer.isMask(Context.raw.layer)) {
				render_path_set_target("temptex0");
				render_path_bind_target("texpaint_nor" + tid, "tex");
				render_path_draw_shader("shader_datas/copy_pass/copy_pass");
				render_path_set_target("texpaint_nor" + tid);
				render_path_bind_target("temptex0", "tex");
				render_path_draw_shader("shader_datas/dilate_pass/dilate_pass");

				render_path_set_target("temptex0");
				render_path_bind_target("texpaint_pack" + tid, "tex");
				render_path_draw_shader("shader_datas/copy_pass/copy_pass");
				render_path_set_target("texpaint_pack" + tid);
				render_path_bind_target("temptex0", "tex");
				render_path_draw_shader("shader_datas/dilate_pass/dilate_pass");
			}
		}
		///end
	}
}
