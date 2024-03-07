
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
			t.width = Config.get_texture_res_x();
			t.height = Config.get_texture_res_y();
			t.format = "R8";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_blend1";
			t.width = Config.get_texture_res_x();
			t.height = Config.get_texture_res_y();
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

	static commands_paint = (dilation = true) => {
		let tid = Context.raw.layer.id;

		if (Context.raw.pdirty > 0) {
			///if arm_physics
			let particlePhysics = Context.raw.particle_physics;
			///else
			let particlePhysics = false;
			///end
			if (Context.raw.tool == workspace_tool_t.PARTICLE && !particlePhysics) {
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
			if (Context.raw.tool == workspace_tool_t.COLORID) {
				render_path_set_target("texpaint_colorid");
				render_path_clear_target(0xff000000);
				render_path_bind_target("gbuffer2", "gbuffer2");
				render_path_draw_meshes("paint");
				UIHeader.header_handle.redraws = 2;
			}
			else if (Context.raw.tool == workspace_tool_t.PICKER || Context.raw.tool == workspace_tool_t.MATERIAL) {
				if (Context.raw.pick_pos_nor_tex) {
					if (Context.raw.paint2d) {
						render_path_set_target("gbuffer0", ["gbuffer1", "gbuffer2"]);
						render_path_draw_meshes("mesh");
					}
					render_path_set_target("texpaint_posnortex_picker0", ["texpaint_posnortex_picker1"]);
					render_path_bind_target("gbuffer2", "gbuffer2");
					render_path_bind_target("_main", "gbufferD");
					render_path_draw_meshes("paint");
					let texpaint_posnortex_picker0 = render_path_render_targets.get("texpaint_posnortex_picker0")._image;
					let texpaint_posnortex_picker1 = render_path_render_targets.get("texpaint_posnortex_picker1")._image;
					let a = new DataView(image_get_pixels(texpaint_posnortex_picker0));
					let b = new DataView(image_get_pixels(texpaint_posnortex_picker1));
					Context.raw.posx_picked = a.getFloat32(0, true);
					Context.raw.posy_picked = a.getFloat32(4, true);
					Context.raw.posz_picked = a.getFloat32(8, true);
					Context.raw.uvx_picked = a.getFloat32(12, true);
					Context.raw.norx_picked = b.getFloat32(0, true);
					Context.raw.nory_picked = b.getFloat32(4, true);
					Context.raw.norz_picked = b.getFloat32(8, true);
					Context.raw.uvy_picked = b.getFloat32(12, true);
				}
				else {
					render_path_set_target("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					render_path_bind_target("gbuffer2", "gbuffer2");
					tid = Context.raw.layer.id;
					let useLiveLayer = Context.raw.tool == workspace_tool_t.MATERIAL;
					if (useLiveLayer) RenderPathPaint.use_live_layer(true);
					render_path_bind_target("texpaint" + tid, "texpaint");
					render_path_bind_target("texpaint_nor" + tid, "texpaint_nor");
					render_path_bind_target("texpaint_pack" + tid, "texpaint_pack");
					render_path_draw_meshes("paint");
					if (useLiveLayer) RenderPathPaint.use_live_layer(false);
					UIHeader.header_handle.redraws = 2;
					UIBase.hwnds[2].redraws = 2;

					let texpaint_picker = render_path_render_targets.get("texpaint_picker")._image;
					let texpaint_nor_picker = render_path_render_targets.get("texpaint_nor_picker")._image;
					let texpaint_pack_picker = render_path_render_targets.get("texpaint_pack_picker")._image;
					let texpaint_uv_picker = render_path_render_targets.get("texpaint_uv_picker")._image;
					let a = new DataView(image_get_pixels(texpaint_picker));
					let b = new DataView(image_get_pixels(texpaint_nor_picker));
					let c = new DataView(image_get_pixels(texpaint_pack_picker));
					let d = new DataView(image_get_pixels(texpaint_uv_picker));

					if (Context.raw.color_picker_callback != null) {
						Context.raw.color_picker_callback(Context.raw.picked_color);
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
					Context.raw.picked_color.base = color_set_rb(Context.raw.picked_color.base, a.getUint8(i0));
					Context.raw.picked_color.base = color_set_gb(Context.raw.picked_color.base, a.getUint8(i1));
					Context.raw.picked_color.base = color_set_bb(Context.raw.picked_color.base, a.getUint8(i2));
					Context.raw.picked_color.normal = color_set_rb(Context.raw.picked_color.normal, b.getUint8(i0));
					Context.raw.picked_color.normal = color_set_gb(Context.raw.picked_color.normal, b.getUint8(i1));
					Context.raw.picked_color.normal = color_set_bb(Context.raw.picked_color.normal, b.getUint8(i2));
					Context.raw.picked_color.occlusion = c.getUint8(i0) / 255;
					Context.raw.picked_color.roughness = c.getUint8(i1) / 255;
					Context.raw.picked_color.metallic = c.getUint8(i2) / 255;
					Context.raw.picked_color.height = c.getUint8(i3) / 255;
					Context.raw.picked_color.opacity = a.getUint8(i3) / 255;
					Context.raw.uvx_picked = d.getUint8(i0) / 255;
					Context.raw.uvy_picked = d.getUint8(i1) / 255;
					// Pick material
					if (Context.raw.picker_select_material && Context.raw.color_picker_callback == null) {
						// matid % 3 == 0 - normal, 1 - emission, 2 - subsurface
						let matid = Math.floor((b.getUint8(3) - (b.getUint8(3) % 3)) / 3);
						for (let m of Project.materials) {
							if (m.id == matid) {
								Context.set_material(m);
								Context.raw.materialid_picked = matid;
								break;
							}
						}
					}
				}
			}
			else {
				///if arm_voxels
				if (Context.raw.tool == workspace_tool_t.BAKE && Context.raw.bake_type == bake_type_t.AO) {
					if (RenderPathPaint.initVoxels) {
						RenderPathPaint.initVoxels = false;
						let _rp_gi = Config.raw.rp_gi;
						Config.raw.rp_gi = true;
						RenderPathBase.init_voxels();
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
				if (Context.raw.tool == workspace_tool_t.BAKE && Context.raw.brush_time == time_delta()) {
					// Clear to black on bake start
					render_path_set_target(texpaint);
					render_path_clear_target(0xff000000);
				}

				render_path_set_target("texpaint_blend1");
				render_path_bind_target("texpaint_blend0", "tex");
				render_path_draw_shader("shader_datas/copy_pass/copyR8_pass");
				let isMask = SlotLayer.is_mask(Context.raw.layer);
				if (isMask) {
					let ptid = Context.raw.layer.parent.id;
					if (SlotLayer.is_group(Context.raw.layer.parent)) { // Group mask
						for (let c of SlotLayer.get_children(Context.raw.layer.parent)) {
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
				if (Context.raw.tool == workspace_tool_t.BAKE && Context.raw.bake_type == bake_type_t.AO) {
					render_path_bind_target("voxels", "voxels");
				}
				///end
				if (Context.raw.colorid_picked) {
					render_path_bind_target("texpaint_colorid", "texpaint_colorid");
				}

				// Read texcoords from gbuffer
				let readTC = (Context.raw.tool == workspace_tool_t.FILL && Context.raw.fill_type_handle.position == fill_type_t.FACE) ||
							  Context.raw.tool == workspace_tool_t.CLONE ||
							  Context.raw.tool == workspace_tool_t.BLUR ||
							  Context.raw.tool == workspace_tool_t.SMUDGE;
				if (readTC) {
					render_path_bind_target("gbuffer2", "gbuffer2");
				}

				render_path_draw_meshes("paint");

				if (Context.raw.tool == workspace_tool_t.BAKE && Context.raw.bake_type == bake_type_t.CURVATURE && Context.raw.bake_curv_smooth > 0) {
					if (render_path_render_targets.get("texpaint_blur") == null) {
						let t = render_target_create();
						t.name = "texpaint_blur";
						t.width = Math.floor(Config.get_texture_res_x() * 0.95);
						t.height = Math.floor(Config.get_texture_res_y() * 0.95);
						t.format = "RGBA32";
						render_path_create_render_target(t);
					}
					let blurs = Math.round(Context.raw.bake_curv_smooth);
					for (let i = 0; i < blurs; ++i) {
						render_path_set_target("texpaint_blur");
						render_path_bind_target(texpaint, "tex");
						render_path_draw_shader("shader_datas/copy_pass/copy_pass");
						render_path_set_target(texpaint);
						render_path_bind_target("texpaint_blur", "tex");
						render_path_draw_shader("shader_datas/copy_pass/copy_pass");
					}
				}

				if (dilation && Config.raw.dilate == dilate_type_t.INSTANT) {
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
			let readTC = (Context.raw.tool == workspace_tool_t.FILL && Context.raw.fill_type_handle.position == fill_type_t.FACE) ||
						  Context.raw.tool == workspace_tool_t.CLONE ||
						  Context.raw.tool == workspace_tool_t.BLUR ||
						  Context.raw.tool == workspace_tool_t.SMUDGE;
			if (readTC) {
				render_path_bind_target("gbuffer2", "gbuffer2");
			}
			render_path_bind_target("gbuffer0_undo", "gbuffer0_undo");

			let materialContexts: material_context_t[] = [];
			let shaderContexts: shader_context_t[] = [];
			let mats = Project.paint_objects[0].materials;
			mesh_object_get_contexts(Project.paint_objects[0], "paint", mats, materialContexts, shaderContexts);

			let cc_context = shaderContexts[0];
			if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
			g4_set_pipeline(cc_context._.pipe_state);
			uniforms_set_context_consts(cc_context,_render_path_bind_params);
			uniforms_set_obj_consts(cc_context, Project.paint_objects[0].base);
			uniforms_set_material_consts(cc_context, materialContexts[0]);
			g4_set_vertex_buffer(const_data_screen_aligned_vb);
			g4_set_index_buffer(const_data_screen_aligned_ib);
			g4_draw();
			render_path_end();
			///end
		}
	}

	static use_live_layer = (use: bool) => {
		let tid = Context.raw.layer.id;
		let hid = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
		if (use) {
			RenderPathPaint._texpaint = render_path_render_targets.get("texpaint" + tid);
			RenderPathPaint._texpaint_undo = render_path_render_targets.get("texpaint_undo" + hid);
			RenderPathPaint._texpaint_nor_undo = render_path_render_targets.get("texpaint_nor_undo" + hid);
			RenderPathPaint._texpaint_pack_undo = render_path_render_targets.get("texpaint_pack_undo" + hid);
			RenderPathPaint._texpaint_nor = render_path_render_targets.get("texpaint_nor" + tid);
			RenderPathPaint._texpaint_pack = render_path_render_targets.get("texpaint_pack" + tid);
			render_path_render_targets.set("texpaint_undo" + hid,render_path_render_targets.get("texpaint" + tid));
			render_path_render_targets.set("texpaint" + tid,render_path_render_targets.get("texpaint_live"));
			if (SlotLayer.is_layer(Context.raw.layer)) {
				render_path_render_targets.set("texpaint_nor_undo" + hid,render_path_render_targets.get("texpaint_nor" + tid));
				render_path_render_targets.set("texpaint_pack_undo" + hid,render_path_render_targets.get("texpaint_pack" + tid));
				render_path_render_targets.set("texpaint_nor" + tid,render_path_render_targets.get("texpaint_nor_live"));
				render_path_render_targets.set("texpaint_pack" + tid,render_path_render_targets.get("texpaint_pack_live"));
			}
		}
		else {
			render_path_render_targets.set("texpaint" + tid, RenderPathPaint._texpaint);
			render_path_render_targets.set("texpaint_undo" + hid, RenderPathPaint._texpaint_undo);
			if (SlotLayer.is_layer(Context.raw.layer)) {
				render_path_render_targets.set("texpaint_nor_undo" + hid, RenderPathPaint._texpaint_nor_undo);
				render_path_render_targets.set("texpaint_pack_undo" + hid, RenderPathPaint._texpaint_pack_undo);
				render_path_render_targets.set("texpaint_nor" + tid, RenderPathPaint._texpaint_nor);
				render_path_render_targets.set("texpaint_pack" + tid, RenderPathPaint._texpaint_pack);
			}
		}
		RenderPathPaint.liveLayerLocked = use;
	}

	static commands_live_brush = () => {
		let tool = Context.raw.tool;
		if (tool != workspace_tool_t.BRUSH &&
			tool != workspace_tool_t.ERASER &&
			tool != workspace_tool_t.CLONE &&
			tool != workspace_tool_t.DECAL &&
			tool != workspace_tool_t.TEXT &&
			tool != workspace_tool_t.BLUR &&
			tool != workspace_tool_t.SMUDGE) {
				return;
		}

		if (RenderPathPaint.liveLayerLocked) return;

		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = SlotLayer.create("_live");
		}

		let tid = Context.raw.layer.id;
		if (SlotLayer.is_mask(Context.raw.layer)) {
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

		RenderPathPaint.use_live_layer(true);

		RenderPathPaint.liveLayerDrawn = 2;

		UIView2D.hwnd.redraws = 2;
		let _x = Context.raw.paint_vec.x;
		let _y = Context.raw.paint_vec.y;
		if (Context.raw.brush_locked) {
			Context.raw.paint_vec.x = (Context.raw.lock_started_x - app_x()) / app_w();
			Context.raw.paint_vec.y = (Context.raw.lock_started_y - app_y()) / app_h();
		}
		let _lastX = Context.raw.last_paint_vec_x;
		let _lastY = Context.raw.last_paint_vec_y;
		let _pdirty = Context.raw.pdirty;
		Context.raw.last_paint_vec_x = Context.raw.paint_vec.x;
		Context.raw.last_paint_vec_y = Context.raw.paint_vec.y;
		if (Operator.shortcut(Config.keymap.brush_ruler)) {
			Context.raw.last_paint_vec_x = Context.raw.last_paint_x;
			Context.raw.last_paint_vec_y = Context.raw.last_paint_y;
		}
		Context.raw.pdirty = 2;

		RenderPathPaint.commands_symmetry();
		RenderPathPaint.commands_paint();

		RenderPathPaint.use_live_layer(false);

		Context.raw.paint_vec.x = _x;
		Context.raw.paint_vec.y = _y;
		Context.raw.last_paint_vec_x = _lastX;
		Context.raw.last_paint_vec_y = _lastY;
		Context.raw.pdirty = _pdirty;
		Context.raw.brush_blend_dirty = true;
	}

	static commands_cursor = () => {
		if (!Config.raw.brush_3d) return;
		let decal = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
		let tool = Context.raw.tool;
		if (tool != workspace_tool_t.BRUSH &&
			tool != workspace_tool_t.ERASER &&
			tool != workspace_tool_t.CLONE &&
			tool != workspace_tool_t.BLUR &&
			tool != workspace_tool_t.SMUDGE &&
			tool != workspace_tool_t.PARTICLE &&
			!decalMask) {
				return;
		}

		let fillLayer = Context.raw.layer.fill_layer != null;
		let groupLayer = SlotLayer.is_group(Context.raw.layer);
		if (!Base.ui_enabled || Base.is_dragging || fillLayer || groupLayer) {
			return;
		}

		let mx = Context.raw.paint_vec.x;
		let my = 1.0 - Context.raw.paint_vec.y;
		if (Context.raw.brush_locked) {
			mx = (Context.raw.lock_started_x - app_x()) / app_w();
			my = 1.0 - (Context.raw.lock_started_y - app_y()) / app_h();
		}
		let radius = decalMask ? Context.raw.brush_decal_mask_radius : Context.raw.brush_radius;
		RenderPathPaint.draw_cursor(mx, my, Context.raw.brush_nodes_radius * radius / 3.4);
	}

	static draw_cursor = (mx: f32, my: f32, radius: f32, tintR = 1.0, tintG = 1.0, tintB = 1.0) => {
		let plane: mesh_object_t = scene_get_child(".Plane").ext;
		let geom = plane.data;

		if (Base.pipe_cursor == null) Base.make_cursor_pipe();

		render_path_set_target("");
		g4_set_pipeline(Base.pipe_cursor);
		let decal = Context.raw.tool == workspace_tool_t.DECAL || Context.raw.tool == workspace_tool_t.TEXT;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask, ShortcutType.ShortcutDown);
		let img = (decal && !decalMask) ? Context.raw.decal_image : Res.get("cursor.k");
		g4_set_tex(Base.cursor_tex, img);
		let gbuffer0 = render_path_render_targets.get("gbuffer0")._image;
		g4_set_tex_depth(Base.cursor_gbufferd, gbuffer0);
		g4_set_float2(Base.cursor_mouse, mx, my);
		g4_set_float2(Base.cursor_tex_step, 1 / gbuffer0.width, 1 / gbuffer0.height);
		g4_set_float(Base.cursor_radius, radius);
		let right = vec4_normalize(camera_object_right_world(scene_camera));
		g4_set_float3(Base.cursor_camera_right, right.x, right.y, right.z);
		g4_set_float3(Base.cursor_tint, tintR, tintG, tintB);
		g4_set_mat(Base.cursor_vp, scene_camera.vp);
		let helpMat = mat4_identity();
		mat4_get_inv(helpMat, scene_camera.vp);
		g4_set_mat(Base.cursor_inv_vp, helpMat);
		///if (krom_metal || krom_vulkan)
		g4_set_vertex_buffer(mesh_data_get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		g4_set_vertex_buffer(geom._.vertex_buffer);
		///end
		g4_set_index_buffer(geom._.index_buffers[0]);
		g4_draw();

		g4_disable_scissor();
		render_path_end();
	}

	static commands_symmetry = () => {
		if (Context.raw.sym_x || Context.raw.sym_y || Context.raw.sym_z) {
			Context.raw.ddirty = 2;
			let t = Context.raw.paint_object.base.transform;
			let sx = t.scale.x;
			let sy = t.scale.y;
			let sz = t.scale.z;
			if (Context.raw.sym_x) {
				vec4_set(t.scale, -sx, sy, sz);
				transform_build_matrix(t);
				RenderPathPaint.commands_paint(false);
			}
			if (Context.raw.sym_y) {
				vec4_set(t.scale, sx, -sy, sz);
				transform_build_matrix(t);
				RenderPathPaint.commands_paint(false);
			}
			if (Context.raw.sym_z) {
				vec4_set(t.scale, sx, sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commands_paint(false);
			}
			if (Context.raw.sym_x && Context.raw.sym_y) {
				vec4_set(t.scale, -sx, -sy, sz);
				transform_build_matrix(t);
				RenderPathPaint.commands_paint(false);
			}
			if (Context.raw.sym_x && Context.raw.sym_z) {
				vec4_set(t.scale, -sx, sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commands_paint(false);
			}
			if (Context.raw.sym_y && Context.raw.sym_z) {
				vec4_set(t.scale, sx, -sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commands_paint(false);
			}
			if (Context.raw.sym_x && Context.raw.sym_y && Context.raw.sym_z) {
				vec4_set(t.scale, -sx, -sy, -sz);
				transform_build_matrix(t);
				RenderPathPaint.commands_paint(false);
			}
			vec4_set(t.scale, sx, sy, sz);
			transform_build_matrix(t);
		}
	}

	static paint_enabled = (): bool => {
		///if is_paint
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != workspace_tool_t.PICKER && Context.raw.tool != workspace_tool_t.MATERIAL && Context.raw.tool != workspace_tool_t.COLORID;
		///end

		///if is_sculpt
		let fillLayer = Context.raw.layer.fill_layer != null && Context.raw.tool != workspace_tool_t.PICKER && Context.raw.tool != workspace_tool_t.MATERIAL;
		///end

		let groupLayer = SlotLayer.is_group(Context.raw.layer);
		return !fillLayer && !groupLayer && !Context.raw.foreground_event;
	}

	static live_brush_dirty = () => {
		let mx = RenderPathPaint.lastX;
		let my = RenderPathPaint.lastY;
		RenderPathPaint.lastX = mouse_view_x();
		RenderPathPaint.lastY = mouse_view_y();
		if (Config.raw.brush_live && Context.raw.pdirty <= 0) {
			let moved = (mx != RenderPathPaint.lastX || my != RenderPathPaint.lastY) && (Context.in_viewport() || Context.in_2d_view());
			if (moved || Context.raw.brush_locked) {
				Context.raw.rdirty = 2;
			}
		}
	}

	static begin = () => {

		///if is_paint
		if (!RenderPathPaint.dilated) {
			RenderPathPaint.dilate(Config.raw.dilate == dilate_type_t.DELAYED, true);
			RenderPathPaint.dilated = true;
		}
		///end

		if (!RenderPathPaint.paint_enabled()) return;

		///if is_paint
		RenderPathPaint.pushUndoLast = History.push_undo;
		///end

		if (History.push_undo && History.undo_layers != null) {
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
		if (History.push_undo2 && History.undo_layers != null) {
			History.paint();
		}
		///end

		if (Context.raw.paint2d) {
			RenderPathPaint.set_plane_mesh();
		}

		if (RenderPathPaint.liveLayerDrawn > 0) RenderPathPaint.liveLayerDrawn--;

		if (Config.raw.brush_live && Context.raw.pdirty <= 0 && Context.raw.ddirty <= 0 && Context.raw.brush_time == 0) {
			// Depth is unchanged, draw before gbuffer gets updated
			RenderPathPaint.commands_live_brush();
		}
	}

	static end = () => {
		RenderPathPaint.commands_cursor();
		Context.raw.ddirty--;
		Context.raw.rdirty--;

		if (!RenderPathPaint.paint_enabled()) return;
		Context.raw.pdirty--;
	}

	static draw = () => {
		if (!RenderPathPaint.paint_enabled()) return;

		///if (!krom_ios) // No hover on iPad, decals are painted by pen release
		if (Config.raw.brush_live && Context.raw.pdirty <= 0 && Context.raw.ddirty > 0 && Context.raw.brush_time == 0) {
			// gbuffer has been updated now but brush will lag 1 frame
			RenderPathPaint.commands_live_brush();
		}
		///end

		if (History.undo_layers != null) {
			RenderPathPaint.commands_symmetry();

			if (Context.raw.pdirty > 0) RenderPathPaint.dilated = false;

			///if is_paint
			if (Context.raw.tool == workspace_tool_t.BAKE) {

				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				let isRaytracedBake = (Context.raw.bake_type == bake_type_t.AO  ||
					Context.raw.bake_type == bake_type_t.LIGHTMAP ||
					Context.raw.bake_type == bake_type_t.BENT_NORMAL ||
					Context.raw.bake_type == bake_type_t.THICKNESS);
				///end

				if (Context.raw.bake_type == bake_type_t.NORMAL || Context.raw.bake_type == bake_type_t.HEIGHT || Context.raw.bake_type == bake_type_t.DERIVATIVE) {
					if (!RenderPathPaint.baking && Context.raw.pdirty > 0) {
						RenderPathPaint.baking = true;
						let _bakeType = Context.raw.bake_type;
						Context.raw.bake_type = Context.raw.bake_type == bake_type_t.NORMAL ? bake_type_t.NORMAL_OBJECT : bake_type_t.POSITION; // Bake high poly data
						MakeMaterial.parse_paint_material();
						let _paintObject = Context.raw.paint_object;
						let highPoly = Project.paint_objects[Context.raw.bake_high_poly];
						let _visible = highPoly.base.visible;
						highPoly.base.visible = true;
						Context.select_paint_object(highPoly);
						RenderPathPaint.commands_paint();
						highPoly.base.visible = _visible;
						if (RenderPathPaint.pushUndoLast) History.paint();
						Context.select_paint_object(_paintObject);

						let _renderFinal = () => {
							Context.raw.bake_type = _bakeType;
							MakeMaterial.parse_paint_material();
							Context.raw.pdirty = 1;
							RenderPathPaint.commands_paint();
							Context.raw.pdirty = 0;
							RenderPathPaint.baking = false;
						}
						let _renderDeriv = () => {
							Context.raw.bake_type = bake_type_t.HEIGHT;
							MakeMaterial.parse_paint_material();
							Context.raw.pdirty = 1;
							RenderPathPaint.commands_paint();
							Context.raw.pdirty = 0;
							if (RenderPathPaint.pushUndoLast) History.paint();
							app_notify_on_init(_renderFinal);
						}
						let bakeType = Context.raw.bake_type as bake_type_t;
						app_notify_on_init(bakeType == bake_type_t.DERIVATIVE ? _renderDeriv : _renderFinal);
					}
				}
				else if (Context.raw.bake_type == bake_type_t.OBJECTID) {
					let _layerFilter = Context.raw.layer_filter;
					let _paintObject = Context.raw.paint_object;
					let isMerged = Context.raw.merged_object != null;
					let _visible = isMerged && Context.raw.merged_object.base.visible;
					Context.raw.layer_filter = 1;
					if (isMerged) Context.raw.merged_object.base.visible = false;

					for (let p of Project.paint_objects) {
						Context.select_paint_object(p);
						RenderPathPaint.commands_paint();
					}

					Context.raw.layer_filter = _layerFilter;
					Context.select_paint_object(_paintObject);
					if (isMerged) Context.raw.merged_object.base.visible = _visible;
				}
				///if (krom_direct3d12 || krom_vulkan || krom_metal)
				else if (isRaytracedBake) {
					let dirty = RenderPathRaytraceBake.commands(MakeMaterial.parse_paint_material);
					if (dirty) UIHeader.header_handle.redraws = 2;
					if (Config.raw.dilate == dilate_type_t.INSTANT) { // && Context.raw.pdirty == 1
						RenderPathPaint.dilate(true, false);
					}
				}
				///end
				else {
					RenderPathPaint.commands_paint();
				}
			}
			else { // Paint
				RenderPathPaint.commands_paint();
			}
			///end

			///if is_sculpt
			RenderPathPaint.commands_paint();
			///end
		}

		if (Context.raw.brush_blend_dirty) {
			Context.raw.brush_blend_dirty = false;
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
			RenderPathPaint.restore_plane_mesh();
		}
	}

	static set_plane_mesh = () => {
		Context.raw.paint2d_view = true;
		RenderPathPaint.painto = Context.raw.paint_object;
		RenderPathPaint.visibles = [];
		for (let p of Project.paint_objects) {
			RenderPathPaint.visibles.push(p.base.visible);
			p.base.visible = false;
		}
		if (Context.raw.merged_object != null) {
			RenderPathPaint.mergedObjectVisible = Context.raw.merged_object.base.visible;
			Context.raw.merged_object.base.visible = false;
		}

		let cam = scene_camera;
		mat4_set_from(Context.raw.saved_camera, cam.base.transform.local);
		RenderPathPaint.savedFov = cam.data.fov;
		Viewport.update_camera_type(camera_type_t.PERSPECTIVE);
		let m = mat4_identity();
		mat4_translate(m, 0, 0, 0.5);
		transform_set_matrix(cam.base.transform, m);
		cam.data.fov = Base.default_fov;
		camera_object_build_proj(cam);
		camera_object_build_mat(cam);

		let tw = 0.95 * UIView2D.pan_scale;
		let tx = UIView2D.pan_x / UIView2D.ww;
		let ty = UIView2D.pan_y / app_h();
		mat4_set_identity(m);
		mat4_scale(m, vec4_create(tw, tw, 1));
		mat4_set_loc(m, vec4_create(tx, ty, 0));
		let m2 = mat4_identity();
		mat4_get_inv(m2, scene_camera.vp);
		mat4_mult_mat(m, m2);

		let tiled = UIView2D.tiled_show;
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
			let md: mesh_data_t = mesh_data_create(raw);
			let materials: material_data_t[] = scene_get_child(".Plane").ext.materials;
			let o = scene_add_mesh_object(md, materials);
			o.base.name = ".PlaneTiled";
		}

		RenderPathPaint.planeo = scene_get_child(tiled ? ".PlaneTiled" : ".Plane").ext;
		RenderPathPaint.planeo.base.visible = true;
		Context.raw.paint_object = RenderPathPaint.planeo;

		let v = vec4_create();
		let sx = vec4_len(vec4_set(v, m.m[0], m.m[1], m.m[2]));
		quat_from_euler(RenderPathPaint.planeo.base.transform.rot, -Math.PI / 2, 0, 0);
		vec4_set(RenderPathPaint.planeo.base.transform.scale, sx, 1.0, sx);
		RenderPathPaint.planeo.base.transform.scale.z *= Config.get_texture_res_y() / Config.get_texture_res_x();
		vec4_set(RenderPathPaint.planeo.base.transform.loc, m.m[12], -m.m[13], 0.0);
		transform_build_matrix(RenderPathPaint.planeo.base.transform);
	}

	static restore_plane_mesh = () => {
		Context.raw.paint2d_view = false;
		RenderPathPaint.planeo.base.visible = false;
		vec4_set(RenderPathPaint.planeo.base.transform.loc, 0.0, 0.0, 0.0);
		for (let i = 0; i < Project.paint_objects.length; ++i) {
			Project.paint_objects[i].base.visible = RenderPathPaint.visibles[i];
		}
		if (Context.raw.merged_object != null) {
			Context.raw.merged_object.base.visible = RenderPathPaint.mergedObjectVisible;
		}
		Context.raw.paint_object = RenderPathPaint.painto;
		transform_set_matrix(scene_camera.base.transform, Context.raw.saved_camera);
		scene_camera.data.fov = RenderPathPaint.savedFov;
		Viewport.update_camera_type(Context.raw.camera_type);
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

		RenderPathBase.draw_gbuffer();
	}

	static bind_layers = () => {
		///if is_paint
		let isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		let isMaterialTool = Context.raw.tool == workspace_tool_t.MATERIAL;
		if (isLive || isMaterialTool) RenderPathPaint.use_live_layer(true);
		///end

		for (let i = 0; i < Project.layers.length; ++i) {
			let l = Project.layers[i];
			render_path_bind_target("texpaint" + l.id, "texpaint" + l.id);

			///if is_paint
			if (SlotLayer.is_layer(l)) {
				render_path_bind_target("texpaint_nor" + l.id, "texpaint_nor" + l.id);
				render_path_bind_target("texpaint_pack" + l.id, "texpaint_pack" + l.id);
			}
			///end
		}
	}

	static unbind_layers = () => {
		///if is_paint
		let isLive = Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0;
		let isMaterialTool = Context.raw.tool == workspace_tool_t.MATERIAL;
		if (isLive || isMaterialTool) RenderPathPaint.use_live_layer(false);
		///end
	}

	static dilate = (base: bool, nor_pack: bool) => {
		///if is_paint
		if (Config.raw.dilate_radius > 0 && !Context.raw.paint2d) {
			UtilUV.cache_dilate_map();
			Base.make_temp_img();
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
			if (nor_pack && !SlotLayer.is_mask(Context.raw.layer)) {
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
