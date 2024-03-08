
class Project {

	static raw: project_format_t = {};
	static filepath: string = "";
	static assets: asset_t[] = [];
	static asset_names: string[] = [];
	static asset_id: i32 = 0;
	static mesh_assets: string[] = [];
	static material_groups: node_group_t[] = [];
	static paint_objects: mesh_object_t[] = null;
	static asset_map: Map<i32, any> = new Map(); // ImageRaw | FontRaw
	static mesh_list: string[] = null;
	///if (is_paint || is_sculpt)
	static materials: SlotMaterialRaw[] = null;
	static brushes: SlotBrushRaw[] = null;
	static layers: SlotLayerRaw[] = null;
	static fonts: SlotFontRaw[] = null;
	static atlas_objects: i32[] = null;
	static atlas_names: string[] = null;
	///end
	///if is_lab
	static material_data: material_data_t = null; ////
	static materials: any[] = null; ////
	static nodes: zui_nodes_t;
	static canvas: zui_node_canvas_t;
	static default_canvas: ArrayBuffer = null;
	///end

	static project_open = () => {
		UIFiles.show("arm", false, false, (path: string) => {
			if (!path.endsWith(".arm")) {
				Console.error(Strings.error0());
				return;
			}

			let current: image_t = _g2_current;
			if (current != null) g2_end();

			ImportArm.run_project(path);

			if (current != null) g2_begin(current);
		});
	}

	static project_save = (saveAndQuit: bool = false) => {
		if (Project.filepath == "") {
			///if krom_ios
			let document_directory: string = krom_save_dialog("", "");
			document_directory = document_directory.substr(0, document_directory.length - 8); // Strip /'untitled'
			Project.filepath = document_directory + "/" + sys_title() + ".arm";
			///elseif krom_android
			Project.filepath = krom_save_path() + "/" + sys_title() + ".arm";
			///else
			Project.project_save_as(saveAndQuit);
			return;
			///end
		}

		///if (krom_windows || krom_linux || krom_darwin)
		let filename: string = Project.filepath.substring(Project.filepath.lastIndexOf(Path.sep) + 1, Project.filepath.length - 4);
		sys_title_set(filename + " - " + manifest_title);
		///end

		let _init = () => {
			ExportArm.run_project();
			if (saveAndQuit) sys_stop();
		}
		app_notify_on_init(_init);
	}

	static project_save_as = (saveAndQuit: bool = false) => {
		UIFiles.show("arm", true, false, (path: string) => {
			let f: string = UIFiles.filename;
			if (f == "") f = tr("untitled");
			Project.filepath = path + Path.sep + f;
			if (!Project.filepath.endsWith(".arm")) Project.filepath += ".arm";
			Project.project_save(saveAndQuit);
		});
	}

	static project_new_box = () => {
		///if (is_paint || is_sculpt)
		UIBox.show_custom((ui: zui_t) => {
			if (zui_tab(zui_handle("project_0"), tr("New Project"))) {
				if (Project.mesh_list == null) {
					Project.mesh_list = File.read_directory(Path.data() + Path.sep + "meshes");
					for (let i: i32 = 0; i < Project.mesh_list.length; ++i) Project.mesh_list[i] = Project.mesh_list[i].substr(0, Project.mesh_list[i].length - 4); // Trim .arm
					Project.mesh_list.unshift("plane");
					Project.mesh_list.unshift("sphere");
					Project.mesh_list.unshift("rounded_cube");
				}

				zui_row([0.5, 0.5]);
				Context.raw.project_type = zui_combo(zui_handle("project_1", { position: Context.raw.project_type }), Project.mesh_list, tr("Template"), true);
				Context.raw.project_aspect_ratio = zui_combo(zui_handle("project_2", { position: Context.raw.project_aspect_ratio }), ["1:1", "2:1", "1:2"], tr("Aspect Ratio"), true);

				zui_end_element();
				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("OK")) || ui.is_return_down) {
					Project.project_new();
					Viewport.scale_to_bounds();
					UIBox.hide();
				}
			}
		});
		///end

		///if is_lab
		Project.project_new();
		Viewport.scale_to_bounds();
		///end
	}

	static project_new = (resetLayers: bool = true) => {
		///if (krom_windows || krom_linux || krom_darwin)
		sys_title_set(manifest_title);
		///end
		Project.filepath = "";

		///if (is_paint || is_sculpt)
		if (Context.raw.merged_object != null) {
			mesh_object_remove(Context.raw.merged_object);
			data_delete_mesh(Context.raw.merged_object.data._.handle);
			Context.raw.merged_object = null;
		}
		Context.raw.layer_preview_dirty = true;
		Context.raw.layer_filter = 0;
		Project.mesh_assets = [];
		///end

		Viewport.reset();
		Context.raw.paint_object = Context.main_object();

		Context.select_paint_object(Context.main_object());
		for (let i: i32 = 1; i < Project.paint_objects.length; ++i) {
			let p: mesh_object_t = Project.paint_objects[i];
			if (p == Context.raw.paint_object) continue;
			data_delete_mesh(p.data._.handle);
			mesh_object_remove(p);
		}
		let meshes: mesh_object_t[] = scene_meshes;
		let len: i32 = meshes.length;
		for (let i: i32 = 0; i < len; ++i) {
			let m: mesh_object_t = meshes[len - i - 1];
			if (Context.raw.project_objects.indexOf(m) == -1 &&
				m.base.name != ".ParticleEmitter" &&
				m.base.name != ".Particle") {
				data_delete_mesh(m.data._.handle);
				mesh_object_remove(m);
			}
		}
		let handle: string = Context.raw.paint_object.data._.handle;
		if (handle != "SceneSphere" && handle != "ScenePlane") {
			data_delete_mesh(handle);
		}

		if (Context.raw.project_type != project_model_t.ROUNDED_CUBE) {
			let raw: mesh_data_t = null;
			if (Context.raw.project_type == project_model_t.SPHERE || Context.raw.project_type == project_model_t.TESSELLATED_PLANE) {
				let mesh: any = Context.raw.project_type == project_model_t.SPHERE ?
					Geom.make_uv_sphere(1, 512, 256) :
					Geom.make_plane(1, 1, 512, 512);
				mesh.name = "Tessellated";
				raw = ImportMesh.raw_mesh(mesh);

				///if is_sculpt
				base_notify_on_next_frame(() => {
					let f32a: Float32Array = new Float32Array(Config.get_texture_res_x() * Config.get_texture_res_y() * 4);
					for (let i: i32 = 0; i < Math.floor(mesh.inda.length); ++i) {
						let index: i32 = mesh.inda[i];
						f32a[i * 4]     = mesh.posa[index * 4]     / 32767;
						f32a[i * 4 + 1] = mesh.posa[index * 4 + 1] / 32767;
						f32a[i * 4 + 2] = mesh.posa[index * 4 + 2] / 32767;
						f32a[i * 4 + 3] = 1.0;
					}

					let imgmesh: image_t = image_from_bytes(f32a.buffer, Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.RGBA128);
					let texpaint: image_t = Project.layers[0].texpaint;
					g2_begin(texpaint);
					g2_set_pipeline(base_pipe_copy128);
					g2_draw_scaled_image(imgmesh, 0, 0, Config.get_texture_res_x(), Config.get_texture_res_y());
					g2_set_pipeline(null);
					g2_end();
				});
				///end
			}
			else {
				let b: ArrayBuffer = data_get_blob("meshes/" + Project.mesh_list[Context.raw.project_type] + ".arm");
				raw = armpack_decode(b).mesh_datas[0];
			}

			let md: mesh_data_t = mesh_data_create(raw);
			data_cached_meshes.set("SceneTessellated", md);

			if (Context.raw.project_type == project_model_t.TESSELLATED_PLANE) {
				Viewport.set_view(0, 0, 0.75, 0, 0, 0); // Top
			}
		}

		let n: string = Context.raw.project_type == project_model_t.ROUNDED_CUBE ? ".Cube" : "Tessellated";
		let md: mesh_data_t = data_get_mesh("Scene", n);

		let current: image_t = _g2_current;
		if (current != null) g2_end();

		///if is_paint
		Context.raw.picker_mask_handle.position = picker_mask_t.NONE;
		///end

		mesh_object_set_data(Context.raw.paint_object, md);
		vec4_set(Context.raw.paint_object.base.transform.scale, 1, 1, 1);
		transform_build_matrix(Context.raw.paint_object.base.transform);
		Context.raw.paint_object.base.name = n;
		Project.paint_objects = [Context.raw.paint_object];
		///if (is_paint || is_sculpt)
		while (Project.materials.length > 0) SlotMaterial.unload(Project.materials.pop());
		///end
		let m: material_data_t = data_get_material("Scene", "Material");
		///if (is_paint || is_sculpt)
		Project.materials.push(SlotMaterial.create(m));
		///end
		///if is_lab
		Project.material_data = m;
		///end

		///if (is_paint || is_sculpt)
		Context.raw.material = Project.materials[0];
		///end

		UINodes.hwnd.redraws = 2;
		UINodes.group_stack = [];
		Project.material_groups = [];

		///if (is_paint || is_sculpt)
		Project.brushes = [SlotBrush.create()];
		Context.raw.brush = Project.brushes[0];

		Project.fonts = [SlotFont.create("default.ttf", base_font)];
		Context.raw.font = Project.fonts[0];
		///end

		Project.set_default_swatches();
		Context.raw.swatch = Project.raw.swatches[0];

		Context.raw.picked_color = Project.make_swatch();
		Context.raw.color_picker_callback = null;
		History.reset();

		MakeMaterial.parse_paint_material();

		///if (is_paint || is_sculpt)
		UtilRender.make_material_preview();
		///end

		for (let a of Project.assets) data_delete_image(a.file);
		Project.assets = [];
		Project.asset_names = [];
		Project.asset_map = new Map();
		Project.asset_id = 0;
		Project.raw.packed_assets = [];
		Context.raw.ddirty = 4;

		///if (is_paint || is_sculpt)
		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		///end

		if (resetLayers) {

			///if (is_paint || is_sculpt)
			let aspect_ratio_changed: bool = Project.layers[0].texpaint.width != Config.get_texture_res_x() || Project.layers[0].texpaint.height != Config.get_texture_res_y();
			while (Project.layers.length > 0) SlotLayer.unload(Project.layers.pop());
			let layer: SlotLayerRaw = SlotLayer.create();
			Project.layers.push(layer);
			Context.set_layer(layer);
			if (aspect_ratio_changed) {
				app_notify_on_init(base_resize_layers);
			}
			///end

			app_notify_on_init(base_init_layers);
		}

		if (current != null) g2_begin(current);

		Context.raw.saved_envmap = null;
		Context.raw.envmap_loaded = false;
		scene_world._.envmap = Context.raw.empty_envmap;
		scene_world.envmap = "World_radiance.k";
		Context.raw.show_envmap_handle.selected = Context.raw.show_envmap = false;
		scene_world._.radiance = Context.raw.default_radiance;
		scene_world._.radiance_mipmaps = Context.raw.default_radiance_mipmaps;
		scene_world._.irradiance = Context.raw.default_irradiance;
		scene_world.strength = 4.0;

		///if (is_paint || is_sculpt)
		Context.init_tool();
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end
	}

	///if (is_paint || is_sculpt)
	static import_material = () => {
		UIFiles.show("arm,blend", false, true, (path: string) => {
			path.endsWith(".blend") ?
				ImportBlendMaterial.run(path) :
				ImportArm.run_material(path);
		});
	}

	static import_brush = () => {
		UIFiles.show("arm," + Path.texture_formats.join(","), false, true, (path: string) => {
			// Create brush from texture
			if (Path.is_texture(path)) {
				// Import texture
				ImportAsset.run(path);
				let asset_index: i32 = 0;
				for (let i: i32 = 0; i < Project.assets.length; ++i) {
					if (Project.assets[i].file == path) {
						asset_index = i;
						break;
					}
				}

				// Create a new brush
				Context.raw.brush = SlotBrush.create();
				Project.brushes.push(Context.raw.brush);

				// Create and link image node
				let n: zui_node_t = NodesBrush.create_node("TEX_IMAGE");
				n.x = 83;
				n.y = 340;
				n.buttons[0].default_value = asset_index;
				let links: zui_node_link_t[] = Context.raw.brush.canvas.links;
				links.push({
					id: zui_get_link_id(links),
					from_id: n.id,
					from_socket: 0,
					to_id: 0,
					to_socket: 4
				});

				// Parse brush
				MakeMaterial.parse_brush();
				UINodes.hwnd.redraws = 2;
				let _init = () => {
					UtilRender.make_brush_preview();
				}
				app_notify_on_init(_init);
			}
			// Import from project file
			else {
				ImportArm.run_brush(path);
			}
		});
	}
	///end

	static import_mesh = (replaceExisting: bool = true, done: ()=>void = null) => {
		UIFiles.show(Path.mesh_formats.join(","), false, false, (path: string) => {
			Project.import_mesh_box(path, replaceExisting, true, done);
		});
	}

	static import_mesh_box = (path: string, replaceExisting: bool = true, clearLayers: bool = true, done: ()=>void = null) => {

		///if krom_ios
		// Import immediately while access to resource is unlocked
		// data_get_blob(path, (b: Blob) => {});
		///end

		UIBox.show_custom((ui: zui_t) => {
			let tab_vertical: bool = Config.raw.touch_ui;
			if (zui_tab(zui_handle("project_3"), tr("Import Mesh"), tab_vertical)) {

				if (path.toLowerCase().endsWith(".obj")) {
					Context.raw.split_by = zui_combo(zui_handle("project_4"), [
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
					Context.raw.parse_vcols = zui_check(zui_handle("project_6", { selected: Context.raw.parse_vcols }), tr("Parse Vertex Colors"));
					if (ui.is_hovered) zui_tooltip(tr("Import vertex color data"));
				}
				///end

				zui_row([0.45, 0.45, 0.1]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Import")) || ui.is_return_down) {
					UIBox.hide();
					let do_import = () => {
						///if (is_paint || is_sculpt)
						ImportMesh.run(path, clearLayers, replaceExisting);
						///end
						///if is_lab
						ImportMesh.run(path, replaceExisting);
						///end
						if (done != null) done();
					}
					///if (krom_android || krom_ios)
					base_notify_on_next_frame(() => {
						Console.toast(tr("Importing mesh"));
						base_notify_on_next_frame(do_import);
					});
					///else
					do_import();
					///end
				}
				if (zui_button(tr("?"))) {
					File.load_url("https://github.com/armory3d/armorpaint_docs/blob/master/faq.md");
				}
			}
		});
		UIBox.click_to_hide = false; // Prevent closing when going back to window from file browser
	}

	static reimport_mesh = () => {
		if (Project.mesh_assets != null && Project.mesh_assets.length > 0 && File.exists(Project.mesh_assets[0])) {
			Project.import_mesh_box(Project.mesh_assets[0], true, false);
		}
		else Project.import_asset();
	}

	static unwrap_mesh_box = (mesh: any, done: (a: any)=>void, skipUI: bool = false) => {
		UIBox.show_custom((ui: zui_t) => {
			let tab_vertical: bool = Config.raw.touch_ui;
			if (zui_tab(zui_handle("project_7"), tr("Unwrap Mesh"), tab_vertical)) {

				let unwrapPlugins: string[] = [];
				if (BoxPreferences.files_plugin == null) {
					BoxPreferences.fetch_plugins();
				}
				for (let f of BoxPreferences.files_plugin) {
					if (f.indexOf("uv_unwrap") >= 0 && f.endsWith(".js")) {
						unwrapPlugins.push(f);
					}
				}
				unwrapPlugins.push("equirect");

				let unwrap_by: i32 = zui_combo(zui_handle("project_8"), unwrapPlugins, tr("Plugin"), true);

				zui_row([0.5, 0.5]);
				if (zui_button(tr("Cancel"))) {
					UIBox.hide();
				}
				if (zui_button(tr("Unwrap")) || ui.is_return_down || skipUI) {
					UIBox.hide();
					let do_unwrap = () => {
						if (unwrap_by == unwrapPlugins.length - 1) {
							UtilMesh.equirect_unwrap(mesh);
						}
						else {
							let f: string = unwrapPlugins[unwrap_by];
							if (Config.raw.plugins.indexOf(f) == -1) {
								Config.enable_plugin(f);
								Console.info(f + " " + tr("plugin enabled"));
							}
							UtilMesh.unwrappers.get(f)(mesh);
						}
						done(mesh);
					}
					///if (krom_android || krom_ios)
					base_notify_on_next_frame(() => {
						Console.toast(tr("Unwrapping mesh"));
						base_notify_on_next_frame(do_unwrap);
					});
					///else
					do_unwrap();
					///end
				}
			}
		});
	}

	static import_asset = (filters: string = null, hdrAsEnvmap: bool = true) => {
		if (filters == null) filters = Path.texture_formats.join(",") + "," + Path.mesh_formats.join(",");
		UIFiles.show(filters, false, true, (path: string) => {
			ImportAsset.run(path, -1.0, -1.0, true, hdrAsEnvmap);
		});
	}

	static import_swatches = (replaceExisting: bool = false) => {
		UIFiles.show("arm,gpl", false, false, (path: string) => {
			if (Path.is_gimp_color_palette(path)) ImportGpl.run(path, replaceExisting);
			else ImportArm.run_swatches(path, replaceExisting);
		});
	}

	static reimport_textures = () => {
		for (let asset of Project.assets) {
			Project.reimport_texture(asset);
		}
	}

	static reimport_texture = (asset: asset_t) => {
		let load = (path: string) => {
			asset.file = path;
			let i: i32 = Project.assets.indexOf(asset);
			data_delete_image(asset.file);
			Project.asset_map.delete(asset.id);
			let old_asset: asset_t = Project.assets[i];
			Project.assets.splice(i, 1);
			Project.asset_names.splice(i, 1);
			ImportTexture.run(asset.file);
			Project.assets.splice(i, 0, Project.assets.pop());
			Project.asset_names.splice(i, 0, Project.asset_names.pop());

			///if (is_paint || is_sculpt)
			if (Context.raw.texture == old_asset) Context.raw.texture = Project.assets[i];
			///end

			let _next = () => {
				MakeMaterial.parse_paint_material();

				///if (is_paint || is_sculpt)
				UtilRender.make_material_preview();
				UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
				///end
			}
			base_notify_on_next_frame(_next);
		}
		if (!File.exists(asset.file)) {
			let filters: string = Path.texture_formats.join(",");
			UIFiles.show(filters, false, false, (path: string) => {
				load(path);
			});
		}
		else load(asset.file);
	}

	static get_image = (asset: asset_t): image_t => {
		return asset != null ? Project.asset_map.get(asset.id) : null;
	}

	///if (is_paint || is_sculpt)
	static get_used_atlases = (): string[] => {
		if (Project.atlas_objects == null) return null;
		let used: i32[] = [];
		for (let i of Project.atlas_objects) if (used.indexOf(i) == -1) used.push(i);
		if (used.length > 1) {
			let res: string[] = [];
			for (let i of used) res.push(Project.atlas_names[i]);
			return res;
		}
		else return null;
	}

	static is_atlas_object = (p: mesh_object_t): bool => {
		if (Context.raw.layer_filter <= Project.paint_objects.length) return false;
		let atlas_name: string = Project.get_used_atlases()[Context.raw.layer_filter - Project.paint_objects.length - 1];
		let atlas_i: i32 = Project.atlas_names.indexOf(atlas_name);
		return atlas_i == Project.atlas_objects[Project.paint_objects.indexOf(p)];
	}

	static get_atlas_objects = (objectMask: i32): mesh_object_t[] => {
		let atlas_name: string = Project.get_used_atlases()[objectMask - Project.paint_objects.length - 1];
		let atlas_i: i32 = Project.atlas_names.indexOf(atlas_name);
		let visibles: mesh_object_t[] = [];
		for (let i: i32 = 0; i < Project.paint_objects.length; ++i) if (Project.atlas_objects[i] == atlas_i) visibles.push(Project.paint_objects[i]);
		return visibles;
	}
	///end

	static packed_asset_exists = (packed_assets: packed_asset_t[], name: string): bool => {
		for (let pa of packed_assets) if (pa.name == name) return true;
		return false;
	}

	static export_swatches = () => {
		UIFiles.show("arm,gpl", true, false, (path: string) => {
			let f: string = UIFiles.filename;
			if (f == "") f = tr("untitled");
			if (Path.is_gimp_color_palette(f)) ExportGpl.run(path + Path.sep + f, f.substring(0, f.lastIndexOf(".")), Project.raw.swatches);
			else ExportArm.run_swatches(path + Path.sep + f);
		});
	}

	static make_swatch = (base: i32 = 0xffffffff): swatch_color_t => {
		return { base: base, opacity: 1.0, occlusion: 1.0, roughness: 0.0, metallic: 0.0, normal: 0xff8080ff, emission: 0.0, height: 0.0, subsurface: 0.0 };
	}

	static clone_swatch = (swatch: swatch_color_t): swatch_color_t => {
		return { base: swatch.base, opacity: swatch.opacity, occlusion: swatch.occlusion, roughness: swatch.roughness, metallic: swatch.metallic, normal: swatch.normal, emission: swatch.emission, height: swatch.height, subsurface: swatch.subsurface };
	}

	static set_default_swatches = () => {
		// 32-Color Palette by Andrew Kensler
		// http://eastfarthing.com/blog/2016-05-06-palette/
		Project.raw.swatches = [];
		let colors: i32[] = [0xffffffff, 0xff000000, 0xffd6a090, 0xffa12c32, 0xfffa2f7a, 0xfffb9fda, 0xffe61cf7, 0xff992f7c, 0xff47011f, 0xff051155, 0xff4f02ec, 0xff2d69cb, 0xff00a6ee, 0xff6febff, 0xff08a29a, 0xff2a666a, 0xff063619, 0xff4a4957, 0xff8e7ba4, 0xffb7c0ff, 0xffacbe9c, 0xff827c70, 0xff5a3b1c, 0xffae6507, 0xfff7aa30, 0xfff4ea5c, 0xff9b9500, 0xff566204, 0xff11963b, 0xff51e113, 0xff08fdcc];
		for (let c of colors) Project.raw.swatches.push(Project.make_swatch(c));
	}

	static get_material_group_by_name = (groupName: string): node_group_t => {
		for (let g of Project.material_groups) if (g.canvas.name == groupName) return g;
		return null;
	}

	///if (is_paint || is_sculpt)
	static is_material_group_in_use = (group: node_group_t): bool => {
		let canvases: zui_node_canvas_t[] = [];
		for (let m of Project.materials) canvases.push(m.canvas);
		for (let m of Project.material_groups) canvases.push(m.canvas);
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

type node_group_t = {
	nodes?: zui_nodes_t;
	canvas?: zui_node_canvas_t;
};
