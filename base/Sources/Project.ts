
class Project {

	static raw: TProjectFormat = {};
	static filepath = "";
	static assets: TAsset[] = [];
	static assetNames: string[] = [];
	static assetId = 0;
	static meshAssets: string[] = [];
	static materialGroups: TNodeGroup[] = [];
	static paintObjects: mesh_object_t[] = null;
	static assetMap = new Map<i32, any>(); // ImageRaw | FontRaw
	static meshList: string[] = null;
	///if (is_paint || is_sculpt)
	static materials: SlotMaterialRaw[] = null;
	static brushes: SlotBrushRaw[] = null;
	static layers: SlotLayerRaw[] = null;
	static fonts: SlotFontRaw[] = null;
	static atlasObjects: i32[] = null;
	static atlasNames: string[] = null;
	///end
	///if is_lab
	static materialData: material_data_t = null; ////
	static materials: any[] = null; ////
	static nodes: zui_nodes_t;
	static canvas: zui_node_canvas_t;
	static defaultCanvas: ArrayBuffer = null;
	///end

	static projectOpen = () => {
		UIFiles.show("arm", false, false, (path: string) => {
			if (!path.endsWith(".arm")) {
				Console.error(Strings.error0());
				return;
			}

			let current = _g2_current;
			if (current != null) g2_end();

			ImportArm.runProject(path);

			if (current != null) g2_begin(current, false);
		});
	}

	static projectSave = (saveAndQuit = false) => {
		if (Project.filepath == "") {
			///if krom_ios
			let documentDirectory = krom_save_dialog("", "");
			documentDirectory = documentDirectory.substr(0, documentDirectory.length - 8); // Strip /'untitled'
			Project.filepath = documentDirectory + "/" + sys_title() + ".arm";
			///elseif krom_android
			Project.filepath = krom_save_path() + "/" + sys_title() + ".arm";
			///else
			Project.projectSaveAs(saveAndQuit);
			return;
			///end
		}

		///if (krom_windows || krom_linux || krom_darwin)
		let filename = Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		sys_title_set(filename + " - " + manifest_title);
		///end

		let _init = () => {
			ExportArm.runProject();
			if (saveAndQuit) sys_stop();
		}
		app_notify_on_init(_init);
	}

	static projectSaveAs = (saveAndQuit = false) => {
		UIFiles.show("arm", true, false, (path: string) => {
			let f = UIFiles.filename;
			if (f == "") f = tr("untitled");
			Project.filepath = path + Path.sep + f;
			if (!Project.filepath.endsWith(".arm")) Project.filepath += ".arm";
			Project.projectSave(saveAndQuit);
		});
	}

	static projectNewBox = () => {
		///if (is_paint || is_sculpt)
		UIBox.showCustom((ui: zui_t) => {
			if (zui_tab(zui_handle("project_0"), tr("New Project"))) {
				if (Project.meshList == null) {
					Project.meshList = File.readDirectory(Path.data() + Path.sep + "meshes");
					for (let i = 0; i < Project.meshList.length; ++i) Project.meshList[i] = Project.meshList[i].substr(0, Project.meshList[i].length - 4); // Trim .arm
					Project.meshList.unshift("plane");
					Project.meshList.unshift("sphere");
					Project.meshList.unshift("rounded_cube");
				}

				zui_row([0.5, 0.5]);
				Context.raw.projectType = zui_combo(zui_handle("project_1", { position: Context.raw.projectType }), Project.meshList, tr("Template"), true);
				Context.raw.projectAspectRatio = zui_combo(zui_handle("project_2", { position: Context.raw.projectAspectRatio }), ["1:1", "2:1", "1:2"], tr("Aspect Ratio"), true);

				zui_end_element();
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("OK")) || ui.is_return_down) {
					Project.projectNew();
					Viewport.scaleToBounds();
					UIBox.hide();
				}
			}
		});
		///end

		///if is_lab
		Project.projectNew();
		Viewport.scaleToBounds();
		///end
	}

	static projectNew = (resetLayers = true) => {
		///if (krom_windows || krom_linux || krom_darwin)
		sys_title_set(manifest_title);
		///end
		Project.filepath = "";

		///if (is_paint || is_sculpt)
		if (Context.raw.mergedObject != null) {
			mesh_object_remove(Context.raw.mergedObject);
			data_delete_mesh(Context.raw.mergedObject.data._handle);
			Context.raw.mergedObject = null;
		}
		Context.raw.layerPreviewDirty = true;
		Context.raw.layerFilter = 0;
		Project.meshAssets = [];
		///end

		Viewport.reset();
		Context.raw.paintObject = Context.mainObject();

		Context.selectPaintObject(Context.mainObject());
		for (let i = 1; i < Project.paintObjects.length; ++i) {
			let p = Project.paintObjects[i];
			if (p == Context.raw.paintObject) continue;
			data_delete_mesh(p.data._handle);
			mesh_object_remove(p);
		}
		let meshes = scene_meshes;
		let len = meshes.length;
		for (let i = 0; i < len; ++i) {
			let m = meshes[len - i - 1];
			if (Context.raw.projectObjects.indexOf(m) == -1 &&
				m.base.name != ".ParticleEmitter" &&
				m.base.name != ".Particle") {
				data_delete_mesh(m.data._handle);
				mesh_object_remove(m);
			}
		}
		let handle = Context.raw.paintObject.data._handle;
		if (handle != "SceneSphere" && handle != "ScenePlane") {
			data_delete_mesh(handle);
		}

		if (Context.raw.projectType != ProjectModel.ModelRoundedCube) {
			let raw: mesh_data_t = null;
			if (Context.raw.projectType == ProjectModel.ModelSphere || Context.raw.projectType == ProjectModel.ModelTessellatedPlane) {
				let mesh: any = Context.raw.projectType == ProjectModel.ModelSphere ?
					Geom.make_uv_sphere(1, 512, 256) :
					Geom.make_plane(1, 1, 512, 512);
				mesh.name = "Tessellated";
				raw = ImportMesh.rawMesh(mesh);

				///if is_sculpt
				Base.notifyOnNextFrame(() => {
					let f32a = new Float32Array(Config.getTextureResX() * Config.getTextureResY() * 4);
					for (let i = 0; i < Math.floor(mesh.inda.length); ++i) {
						let index = mesh.inda[i];
						f32a[i * 4]     = mesh.posa[index * 4]     / 32767;
						f32a[i * 4 + 1] = mesh.posa[index * 4 + 1] / 32767;
						f32a[i * 4 + 2] = mesh.posa[index * 4 + 2] / 32767;
						f32a[i * 4 + 3] = 1.0;
					}

					let imgmesh = image_from_bytes(f32a.buffer, Config.getTextureResX(), Config.getTextureResY(), tex_format_t.RGBA128);
					let texpaint = Project.layers[0].texpaint;
					g2_begin(texpaint, false);
					g2_set_pipeline(Base.pipeCopy128);
					g2_draw_scaled_image(imgmesh, 0, 0, Config.getTextureResX(), Config.getTextureResY());
					g2_set_pipeline(null);
					g2_end();
				});
				///end
			}
			else {
				data_get_blob("meshes/" + Project.meshList[Context.raw.projectType] + ".arm", (b: ArrayBuffer) => {
					raw = armpack_decode(b).mesh_datas[0];
				});
			}

			let md: mesh_data_t;
			mesh_data_create(raw, (mdata: mesh_data_t) => { md = mdata; });
			data_cached_meshes.set("SceneTessellated", md);

			if (Context.raw.projectType == ProjectModel.ModelTessellatedPlane) {
				Viewport.setView(0, 0, 0.75, 0, 0, 0); // Top
			}
		}

		let n = Context.raw.projectType == ProjectModel.ModelRoundedCube ? ".Cube" : "Tessellated";
		data_get_mesh("Scene", n, (md: mesh_data_t) => {

			let current = _g2_current;
			if (current != null) g2_end();

			///if is_paint
			Context.raw.pickerMaskHandle.position = PickerMask.MaskNone;
			///end

			mesh_object_set_data(Context.raw.paintObject, md);
			vec4_set(Context.raw.paintObject.base.transform.scale, 1, 1, 1);
			transform_build_matrix(Context.raw.paintObject.base.transform);
			Context.raw.paintObject.base.name = n;
			Project.paintObjects = [Context.raw.paintObject];
			///if (is_paint || is_sculpt)
			while (Project.materials.length > 0) SlotMaterial.unload(Project.materials.pop());
			///end
			data_get_material("Scene", "Material", (m: material_data_t) => {
				///if (is_paint || is_sculpt)
				Project.materials.push(SlotMaterial.create(m));
				///end
				///if is_lab
				Project.materialData = m;
				///end
			});

			///if (is_paint || is_sculpt)
			Context.raw.material = Project.materials[0];
			///end

			UINodes.hwnd.redraws = 2;
			UINodes.groupStack = [];
			Project.materialGroups = [];

			///if (is_paint || is_sculpt)
			Project.brushes = [SlotBrush.create()];
			Context.raw.brush = Project.brushes[0];

			Project.fonts = [SlotFont.create("default.ttf", Base.font)];
			Context.raw.font = Project.fonts[0];
			///end

			Project.setDefaultSwatches();
			Context.raw.swatch = Project.raw.swatches[0];

			Context.raw.pickedColor = Project.makeSwatch();
			Context.raw.colorPickerCallback = null;
			History.reset();

			MakeMaterial.parsePaintMaterial();

			///if (is_paint || is_sculpt)
			UtilRender.makeMaterialPreview();
			///end

			for (let a of Project.assets) data_delete_image(a.file);
			Project.assets = [];
			Project.assetNames = [];
			Project.assetMap = new Map();
			Project.assetId = 0;
			Project.raw.packed_assets = [];
			Context.raw.ddirty = 4;

			///if (is_paint || is_sculpt)
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			///end

			if (resetLayers) {

				///if (is_paint || is_sculpt)
				let aspectRatioChanged = Project.layers[0].texpaint.width != Config.getTextureResX() || Project.layers[0].texpaint.height != Config.getTextureResY();
				while (Project.layers.length > 0) SlotLayer.unload(Project.layers.pop());
				let layer = SlotLayer.create();
				Project.layers.push(layer);
				Context.setLayer(layer);
				if (aspectRatioChanged) {
					app_notify_on_init(Base.resizeLayers);
				}
				///end

				app_notify_on_init(Base.initLayers);
			}

			if (current != null) g2_begin(current, false);

			Context.raw.savedEnvmap = null;
			Context.raw.envmapLoaded = false;
			scene_world._envmap = Context.raw.emptyEnvmap;
			scene_world.envmap = "World_radiance.k";
			Context.raw.showEnvmapHandle.selected = Context.raw.showEnvmap = false;
			scene_world._radiance = Context.raw.defaultRadiance;
			scene_world._radiance_mipmaps = Context.raw.defaultRadianceMipmaps;
			scene_world._irradiance = Context.raw.defaultIrradiance;
			scene_world.strength = 4.0;

			///if (is_paint || is_sculpt)
			Context.initTool();
			///end
		});

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end
	}

	///if (is_paint || is_sculpt)
	static importMaterial = () => {
		UIFiles.show("arm,blend", false, true, (path: string) => {
			path.endsWith(".blend") ?
				ImportBlendMaterial.run(path) :
				ImportArm.runMaterial(path);
		});
	}

	static importBrush = () => {
		UIFiles.show("arm," + Path.textureFormats.join(","), false, true, (path: string) => {
			// Create brush from texture
			if (Path.isTexture(path)) {
				// Import texture
				ImportAsset.run(path);
				let assetIndex = 0;
				for (let i = 0; i < Project.assets.length; ++i) {
					if (Project.assets[i].file == path) {
						assetIndex = i;
						break;
					}
				}

				// Create a new brush
				Context.raw.brush = SlotBrush.create();
				Project.brushes.push(Context.raw.brush);

				// Create and link image node
				let n = NodesBrush.createNode("TEX_IMAGE");
				n.x = 83;
				n.y = 340;
				n.buttons[0].default_value = assetIndex;
				let links = Context.raw.brush.canvas.links;
				links.push({
					id: zui_get_link_id(links),
					from_id: n.id,
					from_socket: 0,
					to_id: 0,
					to_socket: 4
				});

				// Parse brush
				MakeMaterial.parseBrush();
				UINodes.hwnd.redraws = 2;
				let _init = () => {
					UtilRender.makeBrushPreview();
				}
				app_notify_on_init(_init);
			}
			// Import from project file
			else {
				ImportArm.runBrush(path);
			}
		});
	}
	///end

	static importMesh = (replaceExisting = true, done: ()=>void = null) => {
		UIFiles.show(Path.meshFormats.join(","), false, false, (path: string) => {
			Project.importMeshBox(path, replaceExisting, true, done);
		});
	}

	static importMeshBox = (path: string, replaceExisting = true, clearLayers = true, done: ()=>void = null) => {

		///if krom_ios
		// Import immediately while access to resource is unlocked
		// data_get_blob(path, (b: Blob) => {});
		///end

		UIBox.showCustom((ui: zui_t) => {
			let tabVertical = Config.raw.touch_ui;
			if (zui_tab(zui_handle("project_3"), tr("Import Mesh"), tabVertical)) {

				if (path.toLowerCase().endsWith(".obj")) {
					Context.raw.splitBy = zui_combo(zui_handle("project_4"), [
						tr("Object"),
						tr("Group"),
						tr("Material"),
						tr("UDIM Tile"),
					], tr("Split By"), true);
					if (ui.is_hovered) zui_tooltip(tr("Split .obj mesh into objects"));
				}

				// if (path.toLowerCase().endsWith(".fbx")) {
				// 	Context.raw.parseTransform = Zui.check(Zui.handle("project_5", { selected: Context.raw.parseTransform }), tr("Parse Transforms"));
				// 	if (ui.isHovered) Zui.tooltip(tr("Load per-object transforms from .fbx"));
				// }

				///if (is_paint || is_sculpt)
				// if (path.toLowerCase().endsWith(".fbx") || path.toLowerCase().endsWith(".blend")) {
				if (path.toLowerCase().endsWith(".blend")) {
					Context.raw.parseVCols = zui_check(zui_handle("project_6", { selected: Context.raw.parseVCols }), tr("Parse Vertex Colors"));
					if (ui.is_hovered) zui_tooltip(tr("Import vertex color data"));
				}
				///end

				zui_row([0.45, 0.45, 0.1]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Import")) || ui.is_return_down) {
					UIBox.hide();
					let doImport = () => {
						///if (is_paint || is_sculpt)
						ImportMesh.run(path, clearLayers, replaceExisting);
						///end
						///if is_lab
						ImportMesh.run(path, replaceExisting);
						///end
						if (done != null) done();
					}
					///if (krom_android || krom_ios)
					Base.notifyOnNextFrame(() => {
						Console.toast(tr("Importing mesh"));
						Base.notifyOnNextFrame(doImport);
					});
					///else
					doImport();
					///end
				}
				if (zui_button(tr("?"))) {
					File.loadUrl("https://github.com/armory3d/armorpaint_docs/blob/master/faq.md");
				}
			}
		});
		UIBox.clickToHide = false; // Prevent closing when going back to window from file browser
	}

	static reimportMesh = () => {
		if (Project.meshAssets != null && Project.meshAssets.length > 0 && File.exists(Project.meshAssets[0])) {
			Project.importMeshBox(Project.meshAssets[0], true, false);
		}
		else Project.importAsset();
	}

	static unwrapMeshBox = (mesh: any, done: (a: any)=>void, skipUI = false) => {
		UIBox.showCustom((ui: zui_t) => {
			let tabVertical = Config.raw.touch_ui;
			if (zui_tab(zui_handle("project_7"), tr("Unwrap Mesh"), tabVertical)) {

				let unwrapPlugins: string[] = [];
				if (BoxPreferences.filesPlugin == null) {
					BoxPreferences.fetchPlugins();
				}
				for (let f of BoxPreferences.filesPlugin) {
					if (f.indexOf("uv_unwrap") >= 0 && f.endsWith(".js")) {
						unwrapPlugins.push(f);
					}
				}
				unwrapPlugins.push("equirect");

				let unwrapBy = zui_combo(zui_handle("project_8"), unwrapPlugins, tr("Plugin"), true);

				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Unwrap")) || ui.is_return_down || skipUI) {
					UIBox.hide();
					let doUnwrap = () => {
						if (unwrapBy == unwrapPlugins.length - 1) {
							UtilMesh.equirectUnwrap(mesh);
						}
						else {
							let f = unwrapPlugins[unwrapBy];
							if (Config.raw.plugins.indexOf(f) == -1) {
								Config.enablePlugin(f);
								Console.info(f + " " + tr("plugin enabled"));
							}
							UtilMesh.unwrappers.get(f)(mesh);
						}
						done(mesh);
					}
					///if (krom_android || krom_ios)
					Base.notifyOnNextFrame(() => {
						Console.toast(tr("Unwrapping mesh"));
						Base.notifyOnNextFrame(doUnwrap);
					});
					///else
					doUnwrap();
					///end
				}
			}
		});
	}

	static importAsset = (filters: string = null, hdrAsEnvmap = true) => {
		if (filters == null) filters = Path.textureFormats.join(",") + "," + Path.meshFormats.join(",");
		UIFiles.show(filters, false, true, (path: string) => {
			ImportAsset.run(path, -1.0, -1.0, true, hdrAsEnvmap);
		});
	}

	static importSwatches = (replaceExisting = false) => {
		UIFiles.show("arm,gpl", false, false, (path: string) => {
			if (Path.isGimpColorPalette(path)) ImportGpl.run(path, replaceExisting);
			else ImportArm.runSwatches(path, replaceExisting);
		});
	}

	static reimportTextures = () => {
		for (let asset of Project.assets) {
			Project.reimportTexture(asset);
		}
	}

	static reimportTexture = (asset: TAsset) => {
		let load = (path: string) => {
			asset.file = path;
			let i = Project.assets.indexOf(asset);
			data_delete_image(asset.file);
			Project.assetMap.delete(asset.id);
			let oldAsset = Project.assets[i];
			Project.assets.splice(i, 1);
			Project.assetNames.splice(i, 1);
			ImportTexture.run(asset.file);
			Project.assets.splice(i, 0, Project.assets.pop());
			Project.assetNames.splice(i, 0, Project.assetNames.pop());

			///if (is_paint || is_sculpt)
			if (Context.raw.texture == oldAsset) Context.raw.texture = Project.assets[i];
			///end

			let _next = () => {
				MakeMaterial.parsePaintMaterial();

				///if (is_paint || is_sculpt)
				UtilRender.makeMaterialPreview();
				UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
				///end
			}
			Base.notifyOnNextFrame(_next);
		}
		if (!File.exists(asset.file)) {
			let filters = Path.textureFormats.join(",");
			UIFiles.show(filters, false, false, (path: string) => {
				load(path);
			});
		}
		else load(asset.file);
	}

	static getImage = (asset: TAsset): image_t => {
		return asset != null ? Project.assetMap.get(asset.id) : null;
	}

	///if (is_paint || is_sculpt)
	static getUsedAtlases = (): string[] => {
		if (Project.atlasObjects == null) return null;
		let used: i32[] = [];
		for (let i of Project.atlasObjects) if (used.indexOf(i) == -1) used.push(i);
		if (used.length > 1) {
			let res: string[] = [];
			for (let i of used) res.push(Project.atlasNames[i]);
			return res;
		}
		else return null;
	}

	static isAtlasObject = (p: mesh_object_t): bool => {
		if (Context.raw.layerFilter <= Project.paintObjects.length) return false;
		let atlasName = Project.getUsedAtlases()[Context.raw.layerFilter - Project.paintObjects.length - 1];
		let atlasI = Project.atlasNames.indexOf(atlasName);
		return atlasI == Project.atlasObjects[Project.paintObjects.indexOf(p)];
	}

	static getAtlasObjects = (objectMask: i32): mesh_object_t[] => {
		let atlasName = Project.getUsedAtlases()[objectMask - Project.paintObjects.length - 1];
		let atlasI = Project.atlasNames.indexOf(atlasName);
		let visibles: mesh_object_t[] = [];
		for (let i = 0; i < Project.paintObjects.length; ++i) if (Project.atlasObjects[i] == atlasI) visibles.push(Project.paintObjects[i]);
		return visibles;
	}
	///end

	static packedAssetExists = (packed_assets: TPackedAsset[], name: string): bool => {
		for (let pa of packed_assets) if (pa.name == name) return true;
		return false;
	}

	static exportSwatches = () => {
		UIFiles.show("arm,gpl", true, false, (path: string) => {
			let f = UIFiles.filename;
			if (f == "") f = tr("untitled");
			if (Path.isGimpColorPalette(f)) ExportGpl.run(path + Path.sep + f, f.substring(0, f.lastIndexOf(".")), Project.raw.swatches);
			else ExportArm.runSwatches(path + Path.sep + f);
		});
	}

	static makeSwatch = (base = 0xffffffff): TSwatchColor => {
		return { base: base, opacity: 1.0, occlusion: 1.0, roughness: 0.0, metallic: 0.0, normal: 0xff8080ff, emission: 0.0, height: 0.0, subsurface: 0.0 };
	}

	static cloneSwatch = (swatch: TSwatchColor): TSwatchColor => {
		return { base: swatch.base, opacity: swatch.opacity, occlusion: swatch.occlusion, roughness: swatch.roughness, metallic: swatch.metallic, normal: swatch.normal, emission: swatch.emission, height: swatch.height, subsurface: swatch.subsurface };
	}

	static setDefaultSwatches = () => {
		// 32-Color Palette by Andrew Kensler
		// http://eastfarthing.com/blog/2016-05-06-palette/
		Project.raw.swatches = [];
		let colors = [0xffffffff, 0xff000000, 0xffd6a090, 0xffa12c32, 0xfffa2f7a, 0xfffb9fda, 0xffe61cf7, 0xff992f7c, 0xff47011f, 0xff051155, 0xff4f02ec, 0xff2d69cb, 0xff00a6ee, 0xff6febff, 0xff08a29a, 0xff2a666a, 0xff063619, 0xff4a4957, 0xff8e7ba4, 0xffb7c0ff, 0xffacbe9c, 0xff827c70, 0xff5a3b1c, 0xffae6507, 0xfff7aa30, 0xfff4ea5c, 0xff9b9500, 0xff566204, 0xff11963b, 0xff51e113, 0xff08fdcc];
		for (let c of colors) Project.raw.swatches.push(Project.makeSwatch(c));
	}

	static getMaterialGroupByName = (groupName: string): TNodeGroup => {
		for (let g of Project.materialGroups) if (g.canvas.name == groupName) return g;
		return null;
	}

	///if (is_paint || is_sculpt)
	static isMaterialGroupInUse = (group: TNodeGroup): bool => {
		let canvases: zui_node_canvas_t[] = [];
		for (let m of Project.materials) canvases.push(m.canvas);
		for (let m of Project.materialGroups) canvases.push(m.canvas);
		for (let canvas of canvases) {
			for (let n of canvas.nodes) {
				if (n.type == "GROUP" && n.name == group.canvas.name) {
					return true;
				}
			}
		}
		return false;
	}
	///end
}

type TNodeGroup = {
	nodes: zui_nodes_t;
	canvas: zui_node_canvas_t;
}
