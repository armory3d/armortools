
class ImportArm {

	static run_project = (path: string) => {
		let b: ArrayBuffer = data_get_blob(path);
		let project: project_format_t = armpack_decode(b);

		///if (is_paint || is_sculpt)
		if (project.version != null && project.layer_datas == null) {
			// Import as material
			if (project.material_nodes != null) {
				ImportArm.run_material_from_project(project, path);
			}
			// Import as brush
			else if (project.brush_nodes != null) {
				ImportArm.run_brush_from_project(project, path);
			}
			// Import as swatches
			else if (project.swatches != null) {
				ImportArm.run_swatches_from_project(project, path);
			}
			return;
		}

		let importAsMesh: bool = project.version == null;
		Context.raw.layers_preview_dirty = true;
		Context.raw.layer_filter = 0;
		///end

		///if is_lab
		let importAsMesh: bool = true;
		///end

		Project.project_new(importAsMesh);
		Project.filepath = path;
		UIFiles.filename = path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf("."));
		///if (krom_android || krom_ios)
		sys_title_set(UIFiles.filename);
		///else
		sys_title_set(UIFiles.filename + " - " + manifest_title);
		///end

		///if (is_paint || is_sculpt)
		// Import as mesh instead
		if (importAsMesh) {
			ImportArm.run_mesh(project);
			return;
		}
		///end

		// Save to recent
		///if krom_ios
		let recent_path: string = path.substr(path.lastIndexOf("/") + 1);
		///else
		let recent_path: string = path;
		///end
		let recent: string[] = Config.raw.recent_projects;
		array_remove(recent, recent_path);
		recent.unshift(recent_path);
		Config.save();

		Project.raw = project;

		///if (is_paint || is_sculpt)
		let l0: layer_data_t = project.layer_datas[0];
		Base.res_handle.position = Config.get_texture_res_pos(l0.res);
		let bitsPos: texture_bits_t = l0.bpp == 8 ? texture_bits_t.BITS8 : l0.bpp == 16 ? texture_bits_t.BITS16 : texture_bits_t.BITS32;
		Base.bits_handle.position = bitsPos;
		let bytesPerPixel: i32 = Math.floor(l0.bpp / 8);
		let format: tex_format_t = l0.bpp == 8 ? tex_format_t.RGBA32 : l0.bpp == 16 ? tex_format_t.RGBA64 : tex_format_t.RGBA128;
		///end

		let base: string = Path.base_dir(path);
		if (Project.raw.envmap != null) {
			Project.raw.envmap = data_is_abs(Project.raw.envmap) ? Project.raw.envmap : base + Project.raw.envmap;
		}
		if (Project.raw.envmap_strength != null) {
			scene_world.strength = Project.raw.envmap_strength;
		}
		if (Project.raw.camera_world != null) {
			scene_camera.base.transform.local = mat4_from_f32_array(Project.raw.camera_world);
			transform_decompose(scene_camera.base.transform);
			scene_camera.data.fov = Project.raw.camera_fov;
			camera_object_build_proj(scene_camera);
			let origin: Float32Array = Project.raw.camera_origin;
			Camera.origins[0].x = origin[0];
			Camera.origins[0].y = origin[1];
			Camera.origins[0].z = origin[2];
		}

		for (let file of project.assets) {
			///if krom_windows
			file = string_replace_all(file, "/", "\\");
			///else
			file = string_replace_all(file, "\\", "/");
			///end
			// Convert image path from relative to absolute
			let abs: string = data_is_abs(file) ? file : base + file;
			if (project.packed_assets != null) {
				abs = Path.normalize(abs);
				ImportArm.unpack_asset(project, abs, file);
			}
			if (data_cached_images.get(abs) == null && !File.exists(abs)) {
				ImportArm.make_pink(abs);
			}
			let hdrAsEnvmap: bool = abs.endsWith(".hdr") && Project.raw.envmap == abs;
			ImportTexture.run(abs, hdrAsEnvmap);
		}

		///if (is_paint || is_sculpt)
		if (project.font_assets != null) {
			for (let file of project.font_assets) {
				///if krom_windows
				file = string_replace_all(file, "/", "\\");
				///else
				file = string_replace_all(file, "\\", "/");
				///end
				// Convert font path from relative to absolute
				let abs: string = data_is_abs(file) ? file : base + file;
				if (File.exists(abs)) {
					ImportFont.run(abs);
				}
			}
		}
		///end

		///if (is_paint || is_sculpt)
		let md: mesh_data_t = mesh_data_create(project.mesh_datas[0]);
		///end

		///if is_lab
		let md: mesh_data_t = mesh_data_create(project.mesh_data);
		///end

		mesh_object_set_data(Context.raw.paint_object, md);
		vec4_set(Context.raw.paint_object.base.transform.scale, 1, 1, 1);
		transform_build_matrix(Context.raw.paint_object.base.transform);
		Context.raw.paint_object.base.name = md.name;
		Project.paint_objects = [Context.raw.paint_object];

		///if (is_paint || is_sculpt)
		for (let i: i32 = 1; i < project.mesh_datas.length; ++i) {
			let raw: mesh_data_t = project.mesh_datas[i];
			let md: mesh_data_t = mesh_data_create(raw);
			let object: mesh_object_t = scene_add_mesh_object(md, Context.raw.paint_object.materials, Context.raw.paint_object.base);
			object.base.name = md.name;
			object.skip_context = "paint";
			Project.paint_objects.push(object);
		}

		if (project.mesh_assets != null && project.mesh_assets.length > 0) {
			let file: string = project.mesh_assets[0];
			let abs: string = data_is_abs(file) ? file : base + file;
			Project.mesh_assets = [abs];
		}

		///if is_paint
		if (project.atlas_objects != null) Project.atlas_objects = project.atlas_objects;
		if (project.atlas_names != null) Project.atlas_names = project.atlas_names;
		///end

		// No mask by default
		if (Context.raw.merged_object == null) UtilMesh.merge_mesh();
		///end

		Context.select_paint_object(Context.main_object());
		Viewport.scale_to_bounds();
		Context.raw.paint_object.skip_context = "paint";
		Context.raw.merged_object.base.visible = true;

		///if (is_paint || is_sculpt)
		let tex: image_t = Project.layers[0].texpaint;
		if (tex.width != Config.get_texture_res_x() || tex.height != Config.get_texture_res_y()) {
			if (History.undo_layers != null) for (let l of History.undo_layers) SlotLayer.resize_and_set_bits(l);
			let rts: map_t<string, render_target_t> = render_path_render_targets;
			let _texpaint_blend0: image_t = rts.get("texpaint_blend0")._image;
			Base.notify_on_next_frame(() => {
				image_unload(_texpaint_blend0);
			});
			rts.get("texpaint_blend0").width = Config.get_texture_res_x();
			rts.get("texpaint_blend0").height = Config.get_texture_res_y();
			rts.get("texpaint_blend0")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8, depth_format_t.NO_DEPTH);
			let _texpaint_blend1: image_t = rts.get("texpaint_blend1")._image;
			Base.notify_on_next_frame(() => {
				image_unload(_texpaint_blend1);
			});
			rts.get("texpaint_blend1").width = Config.get_texture_res_x();
			rts.get("texpaint_blend1").height = Config.get_texture_res_y();
			rts.get("texpaint_blend1")._image = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8, depth_format_t.NO_DEPTH);
			Context.raw.brush_blend_dirty = true;
		}

		for (let l of Project.layers) SlotLayer.unload(l);
		Project.layers = [];
		for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
			let ld: layer_data_t = project.layer_datas[i];
			let isGroup: bool = ld.texpaint == null;

			///if is_paint
			let isMask: bool = ld.texpaint != null && ld.texpaint_nor == null;
			///end
			///if is_sculpt
			let isMask: bool = false;
			///end

			let l: SlotLayerRaw = SlotLayer.create("", isGroup ? layer_slot_type_t.GROUP : isMask ? layer_slot_type_t.MASK : layer_slot_type_t.LAYER);
			if (ld.name != null) l.name = ld.name;
			l.visible = ld.visible;
			Project.layers.push(l);

			if (!isGroup) {
				if (Base.pipe_merge == null) Base.make_pipe();

				let _texpaint: image_t = null;

				///if is_paint
				let _texpaint_nor: image_t = null;
				let _texpaint_pack: image_t = null;
				///end

				if (isMask) {
					_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4), ld.res, ld.res, tex_format_t.RGBA32);
					g2_begin(l.texpaint);
					// g2_set_pipeline(Base.pipeCopy8);
					g2_set_pipeline(project.is_bgra ? Base.pipe_copyBGRA : Base.pipe_copy); // Full bits for undo support, R8 is used
					g2_draw_image(_texpaint, 0, 0);
					g2_set_pipeline(null);
					g2_end();
				}
				else { // Layer
					// TODO: create render target from bytes
					_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
					g2_begin(l.texpaint);
					g2_set_pipeline(project.is_bgra ? Base.pipe_copyBGRA : Base.pipe_copy);
					g2_draw_image(_texpaint, 0, 0);
					g2_set_pipeline(null);
					g2_end();

					///if is_paint
					_texpaint_nor = image_from_bytes(lz4_decode(ld.texpaint_nor, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
					g2_begin(l.texpaint_nor);
					g2_set_pipeline(project.is_bgra ? Base.pipe_copyBGRA : Base.pipe_copy);
					g2_draw_image(_texpaint_nor, 0, 0);
					g2_set_pipeline(null);
					g2_end();

					_texpaint_pack = image_from_bytes(lz4_decode(ld.texpaint_pack, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
					g2_begin(l.texpaint_pack);
					g2_set_pipeline(project.is_bgra ? Base.pipe_copyBGRA : Base.pipe_copy);
					g2_draw_image(_texpaint_pack, 0, 0);
					g2_set_pipeline(null);
					g2_end();
					///end
				}

				l.scale = ld.uv_scale;
				l.angle = ld.uv_rot;
				l.uvType = ld.uv_type;
				if (ld.decal_mat != null) l.decalMat = mat4_from_f32_array(ld.decal_mat);
				l.maskOpacity = ld.opacity_mask;
				l.objectMask = ld.object_mask;
				l.blending = ld.blending;

				///if is_paint
				l.paintBase = ld.paint_base;
				l.paintOpac = ld.paint_opac;
				l.paintOcc = ld.paint_occ;
				l.paintRough = ld.paint_rough;
				l.paintMet = ld.paint_met;
				l.paintNor = ld.paint_nor;
				l.paintNorBlend = ld.paint_nor_blend != null ? ld.paint_nor_blend : true; // TODO: deprecated
				l.paintHeight = ld.paint_height;
				l.paintHeightBlend = ld.paint_height_blend != null ? ld.paint_height_blend : true; // TODO: deprecated
				l.paintEmis = ld.paint_emis;
				l.paintSubs = ld.paint_subs;
				///end

				Base.notify_on_next_frame(() => {
					image_unload(_texpaint);
					///if is_paint
					if (_texpaint_nor != null) image_unload(_texpaint_nor);
					if (_texpaint_pack != null) image_unload(_texpaint_pack);
					///end
				});
			}
		}

		// Assign parents to groups and masks
		for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
			let ld: layer_data_t = project.layer_datas[i];
			if (ld.parent >= 0) {
				Project.layers[i].parent = Project.layers[ld.parent];
			}
		}

		Context.set_layer(Project.layers[0]);

		// Materials
		let m0: material_data_t = data_get_material("Scene", "Material");

		Project.materials = [];
		for (let n of project.material_nodes) {
			ImportArm.init_nodes(n.nodes);
			Context.raw.material = SlotMaterial.create(m0, n);
			Project.materials.push(Context.raw.material);
		}
		///end

		UINodes.hwnd.redraws = 2;
		UINodes.group_stack = [];
		Project.material_groups = [];
		if (project.material_groups != null) {
			for (let g of project.material_groups) Project.material_groups.push({ canvas: g, nodes: zui_nodes_create() });
		}

		///if (is_paint || is_sculpt)
		for (let m of Project.materials) {
			Context.raw.material = m;
			MakeMaterial.parse_paint_material();
			UtilRender.make_material_preview();
		}

		Project.brushes = [];
		for (let n of project.brush_nodes) {
			ImportArm.init_nodes(n.nodes);
			Context.raw.brush = SlotBrush.create(n);
			Project.brushes.push(Context.raw.brush);
			MakeMaterial.parse_brush();
			UtilRender.make_brush_preview();
		}

		// Fill layers
		for (let i: i32 = 0; i < project.layer_datas.length; ++i) {
			let ld: layer_data_t = project.layer_datas[i];
			let l: SlotLayerRaw = Project.layers[i];
			let isGroup: bool = ld.texpaint == null;
			if (!isGroup) {
				l.fill_layer = ld.fill_layer > -1 ? Project.materials[ld.fill_layer] : null;
			}
		}

		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		///end

		///if is_lab
		ImportArm.init_nodes(project.material.nodes);
		Project.canvas = project.material;
		ParserLogic.parse(Project.canvas);
		///end

		Context.raw.ddirty = 4;
		data_delete_blob(path);
	}

	///if (is_paint || is_sculpt)
	static run_mesh = (raw: scene_t) => {
		Project.paint_objects = [];
		for (let i: i32 = 0; i < raw.mesh_datas.length; ++i) {
			let md: mesh_data_t = mesh_data_create(raw.mesh_datas[i]);
			let object: mesh_object_t = null;
			if (i == 0) {
				mesh_object_set_data(Context.raw.paint_object, md);
				object = Context.raw.paint_object;
			}
			else {
				object = scene_add_mesh_object(md, Context.raw.paint_object.materials, Context.raw.paint_object.base);
				object.base.name = md.name;
				object.skip_context = "paint";
				md._.handle = md.name;
				data_cached_meshes.set(md._.handle, md);
			}
			vec4_set(object.base.transform.scale, 1, 1, 1);
			transform_build_matrix(object.base.transform);
			object.base.name = md.name;
			Project.paint_objects.push(object);
			UtilMesh.merge_mesh();
			Viewport.scale_to_bounds();
		}
		app_notify_on_init(Base.init_layers);
		History.reset();
	}

	static run_material = (path: string) => {
		let b: ArrayBuffer = data_get_blob(path);
		let project: project_format_t = armpack_decode(b);
		if (project.version == null) { data_delete_blob(path); return; }
		ImportArm.run_material_from_project(project, path);
	}

	static run_material_from_project = (project: project_format_t, path: string) => {
		let base: string = Path.base_dir(path);
		for (let file of project.assets) {
			///if krom_windows
			file = string_replace_all(file, "/", "\\");
			///else
			file = string_replace_all(file, "\\", "/");
			///end
			// Convert image path from relative to absolute
			let abs: string = data_is_abs(file) ? file : base + file;
			if (project.packed_assets != null) {
				abs = Path.normalize(abs);
				ImportArm.unpack_asset(project, abs, file);
			}
			if (data_cached_images.get(abs) == null && !File.exists(abs)) {
				ImportArm.make_pink(abs);
			}
			ImportTexture.run(abs);
		}

		let m0: material_data_t = data_get_material("Scene", "Material");

		let imported: SlotMaterialRaw[] = [];

		for (let c of project.material_nodes) {
			ImportArm.init_nodes(c.nodes);
			Context.raw.material = SlotMaterial.create(m0, c);
			Project.materials.push(Context.raw.material);
			imported.push(Context.raw.material);
			History.new_material();
		}

		if (project.material_groups != null) {
			for (let c of project.material_groups) {
				while (ImportArm.group_exists(c)) ImportArm.rename_group(c.name, imported, project.material_groups); // Ensure unique group name
				ImportArm.init_nodes(c.nodes);
				Project.material_groups.push({ canvas: c, nodes: zui_nodes_create() });
			}
		}

		let _init = () => {
			for (let m of imported) {
				Context.set_material(m);
				MakeMaterial.parse_paint_material();
				UtilRender.make_material_preview();
			}
		}
		app_notify_on_init(_init);

		UINodes.group_stack = [];
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		data_delete_blob(path);
	}

	static group_exists = (c: zui_node_canvas_t): bool => {
		for (let g of Project.material_groups) {
			if (g.canvas.name == c.name) return true;
		}
		return false;
	}

	static rename_group = (name: string, materials: SlotMaterialRaw[], groups: zui_node_canvas_t[]) => {
		for (let m of materials) {
			for (let n of m.canvas.nodes) {
				if (n.type == "GROUP" && n.name == name) n.name += ".1";
			}
		}
		for (let c of groups) {
			if (c.name == name) c.name += ".1";
			for (let n of c.nodes) {
				if (n.type == "GROUP" && n.name == name) n.name += ".1";
			}
		}
	}

	static run_brush = (path: string) => {
		let b: ArrayBuffer = data_get_blob(path);
		let project: project_format_t = armpack_decode(b);
		if (project.version == null) { data_delete_blob(path); return; }
		ImportArm.run_brush_from_project(project, path);
	}

	static run_brush_from_project = (project: project_format_t, path: string) => {
		let base: string = Path.base_dir(path);
		for (let file of project.assets) {
			///if krom_windows
			file = string_replace_all(file, "/", "\\");
			///else
			file = string_replace_all(file, "\\", "/");
			///end
			// Convert image path from relative to absolute
			let abs: string = data_is_abs(file) ? file : base + file;
			if (project.packed_assets != null) {
				abs = Path.normalize(abs);
				ImportArm.unpack_asset(project, abs, file);
			}
			if (data_cached_images.get(abs) == null && !File.exists(abs)) {
				ImportArm.make_pink(abs);
			}
			ImportTexture.run(abs);
		}

		let imported: SlotBrushRaw[] = [];

		for (let n of project.brush_nodes) {
			ImportArm.init_nodes(n.nodes);
			Context.raw.brush = SlotBrush.create(n);
			Project.brushes.push(Context.raw.brush);
			imported.push(Context.raw.brush);
		}

		let _init = () => {
			for (let b of imported) {
				Context.set_brush(b);
				UtilRender.make_brush_preview();
			}
		}
		app_notify_on_init(_init);

		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		data_delete_blob(path);
	}
	///end

	static run_swatches = (path: string, replaceExisting: bool = false) => {
		let b: ArrayBuffer = data_get_blob(path);
		let project: project_format_t = armpack_decode(b);
		if (project.version == null) { data_delete_blob(path); return; }
		ImportArm.run_swatches_from_project(project, path, replaceExisting);
	}

	static run_swatches_from_project = (project: project_format_t, path: string, replaceExisting: bool = false) => {
		if (replaceExisting) {
			Project.raw.swatches = [];

			if (project.swatches == null) { // No swatches contained
				Project.raw.swatches.push(Project.make_swatch());
			}
		}

		if (project.swatches != null) {
			for (let s of project.swatches) {
				Project.raw.swatches.push(s);
			}
		}
		UIBase.hwnds[tab_area_t.STATUS].redraws = 2;
		data_delete_blob(path);
	}

	static make_pink = (abs: string) => {
		Console.error(Strings.error2() + " " + abs);
		let b: Uint8Array = new Uint8Array(4);
		b[0] = 255;
		b[1] = 0;
		b[2] = 255;
		b[3] = 255;
		let pink: image_t = image_from_bytes(b.buffer, 1, 1);
		data_cached_images.set(abs, pink);
	}

	static texture_node_name = (): string => {
		///if (is_paint || is_sculpt)
		return "TEX_IMAGE";
		///else
		return "ImageTextureNode";
		///end
	}

	static init_nodes = (nodes: zui_node_t[]) => {
		for (let node of nodes) {
			if (node.type == ImportArm.texture_node_name()) {
				node.buttons[0].default_value = Base.get_asset_index(node.buttons[0].data);
				node.buttons[0].data = "";
			}
		}
	}

	static unpack_asset = (project: project_format_t, abs: string, file: string) => {
		if (Project.raw.packed_assets == null) {
			Project.raw.packed_assets = [];
		}
		for (let pa of project.packed_assets) {
			///if krom_windows
			pa.name = string_replace_all(pa.name, "/", "\\");
			///else
			pa.name = string_replace_all(pa.name, "\\", "/");
			///end
			pa.name = Path.normalize(pa.name);
			if (pa.name == file) pa.name = abs; // From relative to absolute
			if (pa.name == abs) {
				if (!Project.packed_asset_exists(Project.raw.packed_assets, pa.name)) {
					Project.raw.packed_assets.push(pa);
				}
				let image: image_t = image_from_encoded_bytes(pa.bytes, pa.name.endsWith(".jpg") ? ".jpg" : ".png");
				data_cached_images.set(abs, image);
				break;
			}
		}
	}
}
