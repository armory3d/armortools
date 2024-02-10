
class ImportArm {

	static runProject = (path: string) => {
		data_get_blob(path, (b: ArrayBuffer) => {
			let project: TProjectFormat = armpack_decode(b);

			///if (is_paint || is_sculpt)
			if (project.version != null && project.layer_datas == null) {
				// Import as material
				if (project.material_nodes != null) {
					ImportArm.runMaterialFromProject(project, path);
				}
				// Import as brush
				else if (project.brush_nodes != null) {
					ImportArm.runBrushFromProject(project, path);
				}
				// Import as swatches
				else if (project.swatches != null) {
					ImportArm.runSwatchesFromProject(project, path);
				}
				return;
			}

			let importAsMesh = project.version == null;
			Context.raw.layersPreviewDirty = true;
			Context.raw.layerFilter = 0;
			///end

			///if is_lab
			let importAsMesh = true;
			///end

			Project.projectNew(importAsMesh);
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
				ImportArm.runMesh(project);
				return;
			}
			///end

			// Save to recent
			///if krom_ios
			let recent_path = path.substr(path.lastIndexOf("/") + 1);
			///else
			let recent_path = path;
			///end
			let recent = Config.raw.recent_projects;
			array_remove(recent, recent_path);
			recent.unshift(recent_path);
			Config.save();

			Project.raw = project;

			///if (is_paint || is_sculpt)
			let l0 = project.layer_datas[0];
			Base.resHandle.position = Config.getTextureResPos(l0.res);
			let bitsPos = l0.bpp == 8 ? TextureBits.Bits8 : l0.bpp == 16 ? TextureBits.Bits16 : TextureBits.Bits32;
			Base.bitsHandle.position = bitsPos;
			let bytesPerPixel = Math.floor(l0.bpp / 8);
			let format = l0.bpp == 8 ? tex_format_t.RGBA32 : l0.bpp == 16 ? tex_format_t.RGBA64 : tex_format_t.RGBA128;
			///end

			let base = Path.baseDir(path);
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
				let origin = Project.raw.camera_origin;
				Camera.origins[0].x = origin[0];
				Camera.origins[0].y = origin[1];
				Camera.origins[0].z = origin[2];
			}

			for (let file of project.assets) {
				///if krom_windows
				file = file.replaceAll("/", "\\");
				///else
				file = file.replaceAll("\\", "/");
				///end
				// Convert image path from relative to absolute
				let abs = data_is_abs(file) ? file : base + file;
				if (project.packed_assets != null) {
					abs = Path.normalize(abs);
					ImportArm.unpackAsset(project, abs, file);
				}
				if (data_cached_images.get(abs) == null && !File.exists(abs)) {
					ImportArm.makePink(abs);
				}
				let hdrAsEnvmap = abs.endsWith(".hdr") && Project.raw.envmap == abs;
				ImportTexture.run(abs, hdrAsEnvmap);
			}

			///if (is_paint || is_sculpt)
			if (project.font_assets != null) {
				for (let file of project.font_assets) {
					///if krom_windows
					file = file.replaceAll("/", "\\");
					///else
					file = file.replaceAll("\\", "/");
					///end
					// Convert font path from relative to absolute
					let abs = data_is_abs(file) ? file : base + file;
					if (File.exists(abs)) {
						ImportFont.run(abs);
					}
				}
			}
			///end

			// Synchronous for now
			///if (is_paint || is_sculpt)
			mesh_data_create(project.mesh_datas[0], (md: mesh_data_t) => {
			///end

			///if is_lab
			mesh_data_create(project.mesh_data, (md: mesh_data_t) => {
			///end

				mesh_object_set_data(Context.raw.paintObject, md);
				vec4_set(Context.raw.paintObject.base.transform.scale, 1, 1, 1);
				transform_build_matrix(Context.raw.paintObject.base.transform);
				Context.raw.paintObject.base.name = md.name;
				Project.paintObjects = [Context.raw.paintObject];
			});

			///if (is_paint || is_sculpt)
			for (let i = 1; i < project.mesh_datas.length; ++i) {
				let raw = project.mesh_datas[i];
				mesh_data_create(raw, (md: mesh_data_t) => {
					let object = scene_add_mesh_object(md, Context.raw.paintObject.materials, Context.raw.paintObject.base);
					object.base.name = md.name;
					object.skip_context = "paint";
					Project.paintObjects.push(object);
				});
			}

			if (project.mesh_assets != null && project.mesh_assets.length > 0) {
				let file = project.mesh_assets[0];
				let abs = data_is_abs(file) ? file : base + file;
				Project.meshAssets = [abs];
			}

			///if is_paint
			if (project.atlas_objects != null) Project.atlasObjects = project.atlas_objects;
			if (project.atlas_names != null) Project.atlasNames = project.atlas_names;
			///end

			// No mask by default
			if (Context.raw.mergedObject == null) UtilMesh.mergeMesh();
			///end

			Context.selectPaintObject(Context.mainObject());
			Viewport.scaleToBounds();
			Context.raw.paintObject.skip_context = "paint";
			Context.raw.mergedObject.base.visible = true;

			///if (is_paint || is_sculpt)
			let tex = Project.layers[0].texpaint;
			if (tex.width != Config.getTextureResX() || tex.height != Config.getTextureResY()) {
				if (History.undoLayers != null) for (let l of History.undoLayers) SlotLayer.resizeAndSetBits(l);
				let rts = render_path_render_targets;
				let _texpaint_blend0 = rts.get("texpaint_blend0").image;
				Base.notifyOnNextFrame(() => {
					image_unload(_texpaint_blend0);
				});
				rts.get("texpaint_blend0").width = Config.getTextureResX();
				rts.get("texpaint_blend0").height = Config.getTextureResY();
				rts.get("texpaint_blend0").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8, depth_format_t.NO_DEPTH);
				let _texpaint_blend1 = rts.get("texpaint_blend1").image;
				Base.notifyOnNextFrame(() => {
					image_unload(_texpaint_blend1);
				});
				rts.get("texpaint_blend1").width = Config.getTextureResX();
				rts.get("texpaint_blend1").height = Config.getTextureResY();
				rts.get("texpaint_blend1").image = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8, depth_format_t.NO_DEPTH);
				Context.raw.brushBlendDirty = true;
			}

			for (let l of Project.layers) SlotLayer.unload(l);
			Project.layers = [];
			for (let i = 0; i < project.layer_datas.length; ++i) {
				let ld = project.layer_datas[i];
				let isGroup = ld.texpaint == null;

				///if is_paint
				let isMask = ld.texpaint != null && ld.texpaint_nor == null;
				///end
				///if is_sculpt
				let isMask = false;
				///end

				let l = SlotLayer.create("", isGroup ? LayerSlotType.SlotGroup : isMask ? LayerSlotType.SlotMask : LayerSlotType.SlotLayer);
				if (ld.name != null) l.name = ld.name;
				l.visible = ld.visible;
				Project.layers.push(l);

				if (!isGroup) {
					if (Base.pipeMerge == null) Base.makePipe();

					let _texpaint: image_t = null;

					///if is_paint
					let _texpaint_nor: image_t = null;
					let _texpaint_pack: image_t = null;
					///end

					if (isMask) {
						_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4), ld.res, ld.res, tex_format_t.RGBA32);
						g2_begin(l.texpaint, false);
						// g2_set_pipeline(Base.pipeCopy8);
						g2_set_pipeline(project.is_bgra ? Base.pipeCopyBGRA : Base.pipeCopy); // Full bits for undo support, R8 is used
						g2_draw_image(_texpaint, 0, 0);
						g2_set_pipeline(null);
						g2_end();
					}
					else { // Layer
						// TODO: create render target from bytes
						_texpaint = image_from_bytes(lz4_decode(ld.texpaint, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
						g2_begin(l.texpaint, false);
						g2_set_pipeline(project.is_bgra ? Base.pipeCopyBGRA : Base.pipeCopy);
						g2_draw_image(_texpaint, 0, 0);
						g2_set_pipeline(null);
						g2_end();

						///if is_paint
						_texpaint_nor = image_from_bytes(lz4_decode(ld.texpaint_nor, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
						g2_begin(l.texpaint_nor, false);
						g2_set_pipeline(project.is_bgra ? Base.pipeCopyBGRA : Base.pipeCopy);
						g2_draw_image(_texpaint_nor, 0, 0);
						g2_set_pipeline(null);
						g2_end();

						_texpaint_pack = image_from_bytes(lz4_decode(ld.texpaint_pack, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
						g2_begin(l.texpaint_pack, false);
						g2_set_pipeline(project.is_bgra ? Base.pipeCopyBGRA : Base.pipeCopy);
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

					Base.notifyOnNextFrame(() => {
						image_unload(_texpaint);
						///if is_paint
						if (_texpaint_nor != null) image_unload(_texpaint_nor);
						if (_texpaint_pack != null) image_unload(_texpaint_pack);
						///end
					});
				}
			}

			// Assign parents to groups and masks
			for (let i = 0; i < project.layer_datas.length; ++i) {
				let ld = project.layer_datas[i];
				if (ld.parent >= 0) {
					Project.layers[i].parent = Project.layers[ld.parent];
				}
			}

			Context.setLayer(Project.layers[0]);

			// Materials
			let m0: material_data_t = null;
			data_get_material("Scene", "Material", (m: material_data_t) => {
				m0 = m;
			});

			Project.materials = [];
			for (let n of project.material_nodes) {
				ImportArm.initNodes(n.nodes);
				Context.raw.material = SlotMaterial.create(m0, n);
				Project.materials.push(Context.raw.material);
			}
			///end

			UINodes.hwnd.redraws = 2;
			UINodes.groupStack = [];
			Project.materialGroups = [];
			if (project.material_groups != null) {
				for (let g of project.material_groups) Project.materialGroups.push({ canvas: g, nodes: zui_nodes_create() });
			}

			///if (is_paint || is_sculpt)
			for (let m of Project.materials) {
				Context.raw.material = m;
				MakeMaterial.parsePaintMaterial();
				UtilRender.makeMaterialPreview();
			}

			Project.brushes = [];
			for (let n of project.brush_nodes) {
				ImportArm.initNodes(n.nodes);
				Context.raw.brush = SlotBrush.create(n);
				Project.brushes.push(Context.raw.brush);
				MakeMaterial.parseBrush();
				UtilRender.makeBrushPreview();
			}

			// Fill layers
			for (let i = 0; i < project.layer_datas.length; ++i) {
				let ld = project.layer_datas[i];
				let l = Project.layers[i];
				let isGroup = ld.texpaint == null;
				if (!isGroup) {
					l.fill_layer = ld.fill_layer > -1 ? Project.materials[ld.fill_layer] : null;
				}
			}

			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			///end

			///if is_lab
			ImportArm.initNodes(project.material.nodes);
			Project.canvas = project.material;
			ParserLogic.parse(Project.canvas);
			///end

			Context.raw.ddirty = 4;
			data_delete_blob(path);
		});
	}

	///if (is_paint || is_sculpt)
	static runMesh = (raw: scene_t) => {
		Project.paintObjects = [];
		for (let i = 0; i < raw.mesh_datas.length; ++i) {
			mesh_data_create(raw.mesh_datas[i], (md: mesh_data_t) => {
				let object: mesh_object_t = null;
				if (i == 0) {
					mesh_object_set_data(Context.raw.paintObject, md);
					object = Context.raw.paintObject;
				}
				else {
					object = scene_add_mesh_object(md, Context.raw.paintObject.materials, Context.raw.paintObject.base);
					object.base.name = md.name;
					object.skip_context = "paint";
					md._handle = md.name;
					data_cached_meshes.set(md._handle, md);
				}
				vec4_set(object.base.transform.scale, 1, 1, 1);
				transform_build_matrix(object.base.transform);
				object.base.name = md.name;
				Project.paintObjects.push(object);
				UtilMesh.mergeMesh();
				Viewport.scaleToBounds();
			});
		}
		app_notify_on_init(Base.initLayers);
		History.reset();
	}

	static runMaterial = (path: string) => {
		data_get_blob(path, (b: ArrayBuffer) => {
			let project: TProjectFormat = armpack_decode(b);
			if (project.version == null) { data_delete_blob(path); return; }
			ImportArm.runMaterialFromProject(project, path);
		});
	}

	static runMaterialFromProject = (project: TProjectFormat, path: string) => {
		let base = Path.baseDir(path);
		for (let file of project.assets) {
			///if krom_windows
			file = file.replaceAll("/", "\\");
			///else
			file = file.replaceAll("\\", "/");
			///end
			// Convert image path from relative to absolute
			let abs = data_is_abs(file) ? file : base + file;
			if (project.packed_assets != null) {
				abs = Path.normalize(abs);
				ImportArm.unpackAsset(project, abs, file);
			}
			if (data_cached_images.get(abs) == null && !File.exists(abs)) {
				ImportArm.makePink(abs);
			}
			ImportTexture.run(abs);
		}

		let m0: material_data_t = null;
		data_get_material("Scene", "Material", (m: material_data_t) => {
			m0 = m;
		});

		let imported: SlotMaterialRaw[] = [];

		for (let c of project.material_nodes) {
			ImportArm.initNodes(c.nodes);
			Context.raw.material = SlotMaterial.create(m0, c);
			Project.materials.push(Context.raw.material);
			imported.push(Context.raw.material);
			History.newMaterial();
		}

		if (project.material_groups != null) {
			for (let c of project.material_groups) {
				while (ImportArm.groupExists(c)) ImportArm.renameGroup(c.name, imported, project.material_groups); // Ensure unique group name
				ImportArm.initNodes(c.nodes);
				Project.materialGroups.push({ canvas: c, nodes: zui_nodes_create() });
			}
		}

		let _init = () => {
			for (let m of imported) {
				Context.setMaterial(m);
				MakeMaterial.parsePaintMaterial();
				UtilRender.makeMaterialPreview();
			}
		}
		app_notify_on_init(_init);

		UINodes.groupStack = [];
		UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
		data_delete_blob(path);
	}

	static groupExists = (c: zui_node_canvas_t): bool => {
		for (let g of Project.materialGroups) {
			if (g.canvas.name == c.name) return true;
		}
		return false;
	}

	static renameGroup = (name: string, materials: SlotMaterialRaw[], groups: zui_node_canvas_t[]) => {
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

	static runBrush = (path: string) => {
		data_get_blob(path, (b: ArrayBuffer) => {
			let project: TProjectFormat = armpack_decode(b);
			if (project.version == null) { data_delete_blob(path); return; }
			ImportArm.runBrushFromProject(project, path);
		});
	}

	static runBrushFromProject = (project: TProjectFormat, path: string) => {
		let base = Path.baseDir(path);
		for (let file of project.assets) {
			///if krom_windows
			file = file.replaceAll("/", "\\");
			///else
			file = file.replaceAll("\\", "/");
			///end
			// Convert image path from relative to absolute
			let abs = data_is_abs(file) ? file : base + file;
			if (project.packed_assets != null) {
				abs = Path.normalize(abs);
				ImportArm.unpackAsset(project, abs, file);
			}
			if (data_cached_images.get(abs) == null && !File.exists(abs)) {
				ImportArm.makePink(abs);
			}
			ImportTexture.run(abs);
		}

		let imported: SlotBrushRaw[] = [];

		for (let n of project.brush_nodes) {
			ImportArm.initNodes(n.nodes);
			Context.raw.brush = SlotBrush.create(n);
			Project.brushes.push(Context.raw.brush);
			imported.push(Context.raw.brush);
		}

		let _init = () => {
			for (let b of imported) {
				Context.setBrush(b);
				UtilRender.makeBrushPreview();
			}
		}
		app_notify_on_init(_init);

		UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
		data_delete_blob(path);
	}
	///end

	static runSwatches = (path: string, replaceExisting = false) => {
		data_get_blob(path, (b: ArrayBuffer) => {
			let project: TProjectFormat = armpack_decode(b);
			if (project.version == null) { data_delete_blob(path); return; }
			ImportArm.runSwatchesFromProject(project, path, replaceExisting);
		});
	}

	static runSwatchesFromProject = (project: TProjectFormat, path: string, replaceExisting = false) => {
		if (replaceExisting) {
			Project.raw.swatches = [];

			if (project.swatches == null) { // No swatches contained
				Project.raw.swatches.push(Project.makeSwatch());
			}
		}

		if (project.swatches != null) {
			for (let s of project.swatches) {
				Project.raw.swatches.push(s);
			}
		}
		UIBase.hwnds[TabArea.TabStatus].redraws = 2;
		data_delete_blob(path);
	}

	static makePink = (abs: string) => {
		Console.error(Strings.error2() + " " + abs);
		let b = new Uint8Array(4);
		b[0] = 255;
		b[1] = 0;
		b[2] = 255;
		b[3] = 255;
		let pink = image_from_bytes(b.buffer, 1, 1);
		data_cached_images.set(abs, pink);
	}

	static textureNodeName = (): string => {
		///if (is_paint || is_sculpt)
		return "TEX_IMAGE";
		///else
		return "ImageTextureNode";
		///end
	}

	static initNodes = (nodes: zui_node_t[]) => {
		for (let node of nodes) {
			if (node.type == ImportArm.textureNodeName()) {
				node.buttons[0].default_value = Base.getAssetIndex(node.buttons[0].data);
				node.buttons[0].data = "";
			}
		}
	}

	static unpackAsset = (project: TProjectFormat, abs: string, file: string) => {
		if (Project.raw.packed_assets == null) {
			Project.raw.packed_assets = [];
		}
		for (let pa of project.packed_assets) {
			///if krom_windows
			pa.name = pa.name.replaceAll("/", "\\");
			///else
			pa.name = pa.name.replaceAll("\\", "/");
			///end
			pa.name = Path.normalize(pa.name);
			if (pa.name == file) pa.name = abs; // From relative to absolute
			if (pa.name == abs) {
				if (!Project.packedAssetExists(Project.raw.packed_assets, pa.name)) {
					Project.raw.packed_assets.push(pa);
				}
				image_from_encoded_bytes(pa.bytes, pa.name.endsWith(".jpg") ? ".jpg" : ".png", (image: image_t) => {
					data_cached_images.set(abs, image);
				}, false);
				break;
			}
		}
	}
}
