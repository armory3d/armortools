
let project_raw: project_format_t = {};
let project_filepath: string = "";
let project_assets: asset_t[] = [];
let project_asset_names: string[] = [];
let project_asset_id: i32 = 0;
let project_mesh_assets: string[] = [];
let project_material_groups: node_group_t[] = [];
let project_paint_objects: mesh_object_t[] = null;
let project_asset_map: Map<i32, any> = new Map(); // image_t | font_t
let project_mesh_list: string[] = null;
///if (is_paint || is_sculpt)
let project_materials: SlotMaterialRaw[] = null;
let project_brushes: SlotBrushRaw[] = null;
let project_layers: SlotLayerRaw[] = null;
let project_fonts: SlotFontRaw[] = null;
let project_atlas_objects: i32[] = null;
let project_atlas_names: string[] = null;
///end
///if is_lab
let project_material_data: material_data_t = null; ////
let project_materials: any[] = null; ////
let project_nodes: zui_nodes_t;
let project_canvas: zui_node_canvas_t;
let project_default_canvas: ArrayBuffer = null;
///end

function project_open() {
	UIFiles.show("arm", false, false, function (path: string) {
		if (!path.endsWith(".arm")) {
			console_error(strings_error0());
			return;
		}

		let current: image_t = _g2_current;
		if (current != null) g2_end();

		ImportArm.run_project(path);

		if (current != null) g2_begin(current);
	});
}

function project_save(saveAndQuit: bool = false) {
	if (project_filepath == "") {
		///if krom_ios
		let document_directory: string = krom_save_dialog("", "");
		document_directory = document_directory.substr(0, document_directory.length - 8); // Strip /'untitled'
		project_filepath = document_directory + "/" + sys_title() + ".arm";
		///elseif krom_android
		project_filepath = krom_save_path() + "/" + sys_title() + ".arm";
		///else
		project_save_as(saveAndQuit);
		return;
		///end
	}

	///if (krom_windows || krom_linux || krom_darwin)
	let filename: string = project_filepath.substring(project_filepath.lastIndexOf(path_sep) + 1, project_filepath.length - 4);
	sys_title_set(filename + " - " + manifest_title);
	///end

	let _init = function () {
		ExportArm.run_project();
		if (saveAndQuit) sys_stop();
	}
	app_notify_on_init(_init);
}

function project_save_as(saveAndQuit: bool = false) {
	UIFiles.show("arm", true, false, function (path: string) {
		let f: string = UIFiles.filename;
		if (f == "") f = tr("untitled");
		project_filepath = path + path_sep + f;
		if (!project_filepath.endsWith(".arm")) project_filepath += ".arm";
		project_save(saveAndQuit);
	});
}

function project_new_box() {
	///if (is_paint || is_sculpt)
	UIBox.show_custom(function (ui: zui_t) {
		if (zui_tab(zui_handle("project_0"), tr("New Project"))) {
			if (project_mesh_list == null) {
				project_mesh_list = file_read_directory(path_data() + path_sep + "meshes");
				for (let i: i32 = 0; i < project_mesh_list.length; ++i) project_mesh_list[i] = project_mesh_list[i].substr(0, project_mesh_list[i].length - 4); // Trim .arm
				project_mesh_list.unshift("plane");
				project_mesh_list.unshift("sphere");
				project_mesh_list.unshift("rounded_cube");
			}

			zui_row([0.5, 0.5]);
			context_raw.project_type = zui_combo(zui_handle("project_1", { position: context_raw.project_type }), project_mesh_list, tr("Template"), true);
			context_raw.project_aspect_ratio = zui_combo(zui_handle("project_2", { position: context_raw.project_aspect_ratio }), ["1:1", "2:1", "1:2"], tr("Aspect Ratio"), true);

			zui_end_element();
			zui_row([0.5, 0.5]);
			if (zui_button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (zui_button(tr("OK")) || ui.is_return_down) {
				project_new();
				viewport_scale_to_bounds();
				UIBox.hide();
			}
		}
	});
	///end

	///if is_lab
	project_new();
	viewport_scale_to_bounds();
	///end
}

function project_new(resetLayers: bool = true) {
	///if (krom_windows || krom_linux || krom_darwin)
	sys_title_set(manifest_title);
	///end
	project_filepath = "";

	///if (is_paint || is_sculpt)
	if (context_raw.merged_object != null) {
		mesh_object_remove(context_raw.merged_object);
		data_delete_mesh(context_raw.merged_object.data._.handle);
		context_raw.merged_object = null;
	}
	context_raw.layer_preview_dirty = true;
	context_raw.layer_filter = 0;
	project_mesh_assets = [];
	///end

	viewport_reset();
	context_raw.paint_object = context_main_object();

	context_select_paint_object(context_main_object());
	for (let i: i32 = 1; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		if (p == context_raw.paint_object) continue;
		data_delete_mesh(p.data._.handle);
		mesh_object_remove(p);
	}
	let meshes: mesh_object_t[] = scene_meshes;
	let len: i32 = meshes.length;
	for (let i: i32 = 0; i < len; ++i) {
		let m: mesh_object_t = meshes[len - i - 1];
		if (context_raw.project_objects.indexOf(m) == -1 &&
			m.base.name != ".ParticleEmitter" &&
			m.base.name != ".Particle") {
			data_delete_mesh(m.data._.handle);
			mesh_object_remove(m);
		}
	}
	let handle: string = context_raw.paint_object.data._.handle;
	if (handle != "SceneSphere" && handle != "ScenePlane") {
		data_delete_mesh(handle);
	}

	if (context_raw.project_type != project_model_t.ROUNDED_CUBE) {
		let raw: mesh_data_t = null;
		if (context_raw.project_type == project_model_t.SPHERE || context_raw.project_type == project_model_t.TESSELLATED_PLANE) {
			let mesh: any = context_raw.project_type == project_model_t.SPHERE ?
				geom_make_uv_sphere(1, 512, 256) :
				geom_make_plane(1, 1, 512, 512);
			mesh.name = "Tessellated";
			raw = ImportMesh.raw_mesh(mesh);

			///if is_sculpt
			base_notify_on_next_frame(function () {
				let f32a: Float32Array = new Float32Array(config_get_texture_res_x() * config_get_texture_res_y() * 4);
				for (let i: i32 = 0; i < Math.floor(mesh.inda.length); ++i) {
					let index: i32 = mesh.inda[i];
					f32a[i * 4]     = mesh.posa[index * 4]     / 32767;
					f32a[i * 4 + 1] = mesh.posa[index * 4 + 1] / 32767;
					f32a[i * 4 + 2] = mesh.posa[index * 4 + 2] / 32767;
					f32a[i * 4 + 3] = 1.0;
				}

				let imgmesh: image_t = image_from_bytes(f32a.buffer, config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.RGBA128);
				let texpaint: image_t = project_layers[0].texpaint;
				g2_begin(texpaint);
				g2_set_pipeline(base_pipe_copy128);
				g2_draw_scaled_image(imgmesh, 0, 0, config_get_texture_res_x(), config_get_texture_res_y());
				g2_set_pipeline(null);
				g2_end();
			});
			///end
		}
		else {
			let b: ArrayBuffer = data_get_blob("meshes/" + project_mesh_list[context_raw.project_type] + ".arm");
			raw = armpack_decode(b).mesh_datas[0];
		}

		let md: mesh_data_t = mesh_data_create(raw);
		data_cached_meshes.set("SceneTessellated", md);

		if (context_raw.project_type == project_model_t.TESSELLATED_PLANE) {
			viewport_set_view(0, 0, 0.75, 0, 0, 0); // Top
		}
	}

	let n: string = context_raw.project_type == project_model_t.ROUNDED_CUBE ? ".Cube" : "Tessellated";
	let md: mesh_data_t = data_get_mesh("Scene", n);

	let current: image_t = _g2_current;
	if (current != null) g2_end();

	///if is_paint
	context_raw.picker_mask_handle.position = picker_mask_t.NONE;
	///end

	mesh_object_set_data(context_raw.paint_object, md);
	vec4_set(context_raw.paint_object.base.transform.scale, 1, 1, 1);
	transform_build_matrix(context_raw.paint_object.base.transform);
	context_raw.paint_object.base.name = n;
	project_paint_objects = [context_raw.paint_object];
	///if (is_paint || is_sculpt)
	while (project_materials.length > 0) SlotMaterial.unload(project_materials.pop());
	///end
	let m: material_data_t = data_get_material("Scene", "Material");
	///if (is_paint || is_sculpt)
	project_materials.push(SlotMaterial.create(m));
	///end
	///if is_lab
	project_material_data = m;
	///end

	///if (is_paint || is_sculpt)
	context_raw.material = project_materials[0];
	///end

	UINodes.hwnd.redraws = 2;
	UINodes.group_stack = [];
	project_material_groups = [];

	///if (is_paint || is_sculpt)
	project_brushes = [SlotBrush.create()];
	context_raw.brush = project_brushes[0];

	project_fonts = [SlotFont.create("default.ttf", base_font)];
	context_raw.font = project_fonts[0];
	///end

	project_set_default_swatches();
	context_raw.swatch = project_raw.swatches[0];

	context_raw.picked_color = make_swatch();
	context_raw.color_picker_callback = null;
	history_reset();

	MakeMaterial.parse_paint_material();

	///if (is_paint || is_sculpt)
	UtilRender.make_material_preview();
	///end

	for (let a of project_assets) data_delete_image(a.file);
	project_assets = [];
	project_asset_names = [];
	project_asset_map = new Map();
	project_asset_id = 0;
	project_raw.packed_assets = [];
	context_raw.ddirty = 4;

	///if (is_paint || is_sculpt)
	UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	///end

	if (resetLayers) {

		///if (is_paint || is_sculpt)
		let aspect_ratio_changed: bool = project_layers[0].texpaint.width != config_get_texture_res_x() || project_layers[0].texpaint.height != config_get_texture_res_y();
		while (project_layers.length > 0) SlotLayer.unload(project_layers.pop());
		let layer: SlotLayerRaw = SlotLayer.create();
		project_layers.push(layer);
		context_set_layer(layer);
		if (aspect_ratio_changed) {
			app_notify_on_init(base_resize_layers);
		}
		///end

		app_notify_on_init(base_init_layers);
	}

	if (current != null) g2_begin(current);

	context_raw.saved_envmap = null;
	context_raw.envmap_loaded = false;
	scene_world._.envmap = context_raw.empty_envmap;
	scene_world.envmap = "World_radiance.k";
	context_raw.show_envmap_handle.selected = context_raw.show_envmap = false;
	scene_world._.radiance = context_raw.default_radiance;
	scene_world._.radiance_mipmaps = context_raw.default_radiance_mipmaps;
	scene_world._.irradiance = context_raw.default_irradiance;
	scene_world.strength = 4.0;

	///if (is_paint || is_sculpt)
	context_init_tool();
	///end

	///if (krom_direct3d12 || krom_vulkan || krom_metal)
	RenderPathRaytrace.ready = false;
	///end
}

///if (is_paint || is_sculpt)
function project_import_material() {
	UIFiles.show("arm,blend", false, true, function (path: string) {
		path.endsWith(".blend") ?
			ImportBlendMaterial.run(path) :
			ImportArm.run_material(path);
	});
}

function project_import_brush() {
	UIFiles.show("arm," + path_texture_formats.join(","), false, true, function (path: string) {
		// Create brush from texture
		if (path_is_texture(path)) {
			// Import texture
			ImportAsset.run(path);
			let asset_index: i32 = 0;
			for (let i: i32 = 0; i < project_assets.length; ++i) {
				if (project_assets[i].file == path) {
					asset_index = i;
					break;
				}
			}

			// Create a new brush
			context_raw.brush = SlotBrush.create();
			project_brushes.push(context_raw.brush);

			// Create and link image node
			let n: zui_node_t = NodesBrush.create_node("TEX_IMAGE");
			n.x = 83;
			n.y = 340;
			n.buttons[0].default_value = asset_index;
			let links: zui_node_link_t[] = context_raw.brush.canvas.links;
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
			let _init = function () {
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

function project_import_mesh(replaceExisting: bool = true, done: ()=>void = null) {
	UIFiles.show(path_mesh_formats.join(","), false, false, function (path: string) {
		project_import_mesh_box(path, replaceExisting, true, done);
	});
}

function project_import_mesh_box(path: string, replaceExisting: bool = true, clearLayers: bool = true, done: ()=>void = null) {

	///if krom_ios
	// Import immediately while access to resource is unlocked
	// data_get_blob(path);
	///end

	UIBox.show_custom(function (ui: zui_t) {
		let tab_vertical: bool = config_raw.touch_ui;
		if (zui_tab(zui_handle("project_3"), tr("Import Mesh"), tab_vertical)) {

			if (path.toLowerCase().endsWith(".obj")) {
				context_raw.split_by = zui_combo(zui_handle("project_4"), [
					tr("Object"),
					tr("Group"),
					tr("Material"),
					tr("UDIM Tile"),
				], tr("Split By"), true);
				if (ui.is_hovered) zui_tooltip(tr("Split .obj mesh into objects"));
			}

			// if (path.toLowerCase().endsWith(".fbx")) {
			// 	raw.parseTransform = Zui.check(Zui.handle("project_5", { selected: raw.parseTransform }), tr("Parse Transforms"));
			// 	if (ui.isHovered) Zui.tooltip(tr("Load per-object transforms from .fbx"));
			// }

			///if (is_paint || is_sculpt)
			// if (path.toLowerCase().endsWith(".fbx") || path.toLowerCase().endsWith(".blend")) {
			if (path.toLowerCase().endsWith(".blend")) {
				context_raw.parse_vcols = zui_check(zui_handle("project_6", { selected: context_raw.parse_vcols }), tr("Parse Vertex Colors"));
				if (ui.is_hovered) zui_tooltip(tr("Import vertex color data"));
			}
			///end

			zui_row([0.45, 0.45, 0.1]);
			if (zui_button(tr("Cancel"))) {
				UIBox.hide();
			}
			if (zui_button(tr("Import")) || ui.is_return_down) {
				UIBox.hide();
				let do_import = function () {
					///if (is_paint || is_sculpt)
					ImportMesh.run(path, clearLayers, replaceExisting);
					///end
					///if is_lab
					ImportMesh.run(path, replaceExisting);
					///end
					if (done != null) done();
				}
				///if (krom_android || krom_ios)
				base_notify_on_next_frame(function () {
					console_toast(tr("Importing mesh"));
					base_notify_on_next_frame(do_import);
				});
				///else
				do_import();
				///end
			}
			if (zui_button(tr("?"))) {
				file_load_url("https://github.com/armory3d/armorpaint_docs/blob/master/faq.md");
			}
		}
	});
	UIBox.click_to_hide = false; // Prevent closing when going back to window from file browser
}

function project_reimport_mesh() {
	if (project_mesh_assets != null && project_mesh_assets.length > 0 && file_exists(project_mesh_assets[0])) {
		project_import_mesh_box(project_mesh_assets[0], true, false);
	}
	else project_import_asset();
}

function project_unwrap_mesh_box(mesh: any, done: (a: any)=>void, skipUI: bool = false) {
	UIBox.show_custom(function (ui: zui_t) {
		let tab_vertical: bool = config_raw.touch_ui;
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
				let do_unwrap = function () {
					if (unwrap_by == unwrapPlugins.length - 1) {
						UtilMesh.equirect_unwrap(mesh);
					}
					else {
						let f: string = unwrapPlugins[unwrap_by];
						if (config_raw.plugins.indexOf(f) == -1) {
							config_enable_plugin(f);
							console_info(f + " " + tr("plugin enabled"));
						}
						UtilMesh.unwrappers.get(f)(mesh);
					}
					done(mesh);
				}
				///if (krom_android || krom_ios)
				base_notify_on_next_frame(function () {
					console_toast(tr("Unwrapping mesh"));
					base_notify_on_next_frame(do_unwrap);
				});
				///else
				do_unwrap();
				///end
			}
		}
	});
}

function project_import_asset(filters: string = null, hdrAsEnvmap: bool = true) {
	if (filters == null) filters = path_texture_formats.join(",") + "," + path_mesh_formats.join(",");
	UIFiles.show(filters, false, true, function (path: string) {
		ImportAsset.run(path, -1.0, -1.0, true, hdrAsEnvmap);
	});
}

function project_import_swatches(replaceExisting: bool = false) {
	UIFiles.show("arm,gpl", false, false, function (path: string) {
		if (path_is_gimp_color_palette(path)) ImportGpl.run(path, replaceExisting);
		else ImportArm.run_swatches(path, replaceExisting);
	});
}

function project_reimport_textures() {
	for (let asset of project_assets) {
		project_reimport_texture(asset);
	}
}

function project_reimport_texture(asset: asset_t) {
	let load = function (path: string) {
		asset.file = path;
		let i: i32 = project_assets.indexOf(asset);
		data_delete_image(asset.file);
		project_asset_map.delete(asset.id);
		let old_asset: asset_t = project_assets[i];
		project_assets.splice(i, 1);
		project_asset_names.splice(i, 1);
		ImportTexture.run(asset.file);
		project_assets.splice(i, 0, project_assets.pop());
		project_asset_names.splice(i, 0, project_asset_names.pop());

		///if (is_paint || is_sculpt)
		if (context_raw.texture == old_asset) context_raw.texture = project_assets[i];
		///end

		let _next = function () {
			MakeMaterial.parse_paint_material();

			///if (is_paint || is_sculpt)
			UtilRender.make_material_preview();
			UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
			///end
		}
		base_notify_on_next_frame(_next);
	}
	if (!file_exists(asset.file)) {
		let filters: string = path_texture_formats.join(",");
		UIFiles.show(filters, false, false, function (path: string) {
			load(path);
		});
	}
	else load(asset.file);
}

function project_get_image(asset: asset_t): image_t {
	return asset != null ? project_asset_map.get(asset.id) : null;
}

///if (is_paint || is_sculpt)
function project_get_used_atlases(): string[] {
	if (project_atlas_objects == null) return null;
	let used: i32[] = [];
	for (let i of project_atlas_objects) if (used.indexOf(i) == -1) used.push(i);
	if (used.length > 1) {
		let res: string[] = [];
		for (let i of used) res.push(project_atlas_names[i]);
		return res;
	}
	else return null;
}

function project_is_atlas_object(p: mesh_object_t): bool {
	if (context_raw.layer_filter <= project_paint_objects.length) return false;
	let atlas_name: string = project_get_used_atlases()[context_raw.layer_filter - project_paint_objects.length - 1];
	let atlas_i: i32 = project_atlas_names.indexOf(atlas_name);
	return atlas_i == project_atlas_objects[project_paint_objects.indexOf(p)];
}

function project_get_atlas_objects(objectMask: i32): mesh_object_t[] {
	let atlas_name: string = project_get_used_atlases()[objectMask - project_paint_objects.length - 1];
	let atlas_i: i32 = project_atlas_names.indexOf(atlas_name);
	let visibles: mesh_object_t[] = [];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) if (project_atlas_objects[i] == atlas_i) visibles.push(project_paint_objects[i]);
	return visibles;
}
///end

function project_packed_asset_exists(packed_assets: packed_asset_t[], name: string): bool {
	for (let pa of packed_assets) if (pa.name == name) return true;
	return false;
}

function project_export_swatches() {
	UIFiles.show("arm,gpl", true, false, function (path: string) {
		let f: string = UIFiles.filename;
		if (f == "") f = tr("untitled");
		if (path_is_gimp_color_palette(f)) ExportGpl.run(path + path_sep + f, f.substring(0, f.lastIndexOf(".")), project_raw.swatches);
		else ExportArm.run_swatches(path + path_sep + f);
	});
}

function make_swatch(base: i32 = 0xffffffff): swatch_color_t {
	return { base: base, opacity: 1.0, occlusion: 1.0, roughness: 0.0, metallic: 0.0, normal: 0xff8080ff, emission: 0.0, height: 0.0, subsurface: 0.0 };
}

function project_clone_swatch(swatch: swatch_color_t): swatch_color_t {
	return { base: swatch.base, opacity: swatch.opacity, occlusion: swatch.occlusion, roughness: swatch.roughness, metallic: swatch.metallic, normal: swatch.normal, emission: swatch.emission, height: swatch.height, subsurface: swatch.subsurface };
}

function project_set_default_swatches() {
	// 32-Color Palette by Andrew Kensler
	// http://eastfarthing.com/blog/2016-05-06-palette/
	project_raw.swatches = [];
	let colors: i32[] = [0xffffffff, 0xff000000, 0xffd6a090, 0xffa12c32, 0xfffa2f7a, 0xfffb9fda, 0xffe61cf7, 0xff992f7c, 0xff47011f, 0xff051155, 0xff4f02ec, 0xff2d69cb, 0xff00a6ee, 0xff6febff, 0xff08a29a, 0xff2a666a, 0xff063619, 0xff4a4957, 0xff8e7ba4, 0xffb7c0ff, 0xffacbe9c, 0xff827c70, 0xff5a3b1c, 0xffae6507, 0xfff7aa30, 0xfff4ea5c, 0xff9b9500, 0xff566204, 0xff11963b, 0xff51e113, 0xff08fdcc];
	for (let c of colors) project_raw.swatches.push(make_swatch(c));
}

function project_get_material_group_by_name(groupName: string): node_group_t {
	for (let g of project_material_groups) if (g.canvas.name == groupName) return g;
	return null;
}

///if (is_paint || is_sculpt)
function project_is_material_group_in_use(group: node_group_t): bool {
	let canvases: zui_node_canvas_t[] = [];
	for (let m of project_materials) canvases.push(m.canvas);
	for (let m of project_material_groups) canvases.push(m.canvas);
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

type node_group_t = {
	nodes?: zui_nodes_t;
	canvas?: zui_node_canvas_t;
};
