
let import_mesh_clear_layers: bool = true;
let import_mesh_meshes_to_unwrap: any[] = null;

///if (is_paint || is_sculpt)
function import_mesh_run(path: string, _clear_layers: bool = true, replace_existing: bool = true) {
///end

///if is_lab
function import_mesh_run(path: string, replace_existing: bool = true) {
///end

	if (!path_is_mesh(path)) {
		if (!context_enable_import_plugin(path)) {
			console_error(strings_error1());
			return;
		}
	}

	///if (is_paint || is_sculpt)
	import_mesh_clear_layers = _clear_layers;
	context_raw.layer_filter = 0;
	///end

	import_mesh_meshes_to_unwrap = null;

	let p: string = to_lower_case(path);
	if (ends_with(p, ".obj")) {
		import_obj_run(path, replace_existing);
	}
	else if (ends_with(p, ".blend")) {
		import_blend_mesh_run(path, replace_existing);
	}
	else {
		let ext: string = substring(path, string_last_index_of(path, ".") + 1, path.length);
		let importer: any = map_get(path_mesh_importers, ext); // JSValue -> (s: string)=>raw_mesh_t
		let mesh: raw_mesh_t = js_pcall_str(importer, path);
		if (mesh.name == "") {
			mesh.name = path_base_name(path);
		}

		replace_existing ? import_mesh_make_mesh(mesh) : import_mesh_add_mesh(mesh);

		let has_next: bool = mesh.has_next;
		while (has_next) {
			let mesh: raw_mesh_t = js_pcall_str(importer, path);
			if (mesh.name == "") {
				mesh.name = path_base_name(path);
			}
			has_next = mesh.has_next;
			import_mesh_add_mesh(mesh);
		}
	}

	project_mesh_assets = [path];

	///if (arm_android || arm_ios)
	sys_title_set(substring(path, string_last_index_of(path, path_sep) + 1, string_last_index_of(path, ".")));
	///end
}

function import_mesh_finish_import() {
	if (context_raw.merged_object != null) {
		mesh_data_delete(context_raw.merged_object.data);
		mesh_object_remove(context_raw.merged_object);
		context_raw.merged_object = null;
	}

	context_select_paint_object(context_main_object());

	if (project_paint_objects.length > 1) {
		// Sort by name
		array_sort(project_paint_objects, function (pa: any_ptr, pb: any_ptr): i32 {
			let a: mesh_object_t = DEREFERENCE(pa);
			let b: mesh_object_t = DEREFERENCE(pb);
			return strcmp(a.base.name, b.base.name);
		});

		// No mask by default
		for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
			let p: mesh_object_t = project_paint_objects[i];
			p.base.visible = true;
		}
		if (context_raw.merged_object == null) {
			util_mesh_merge();
		}
		context_raw.paint_object.skip_context = "paint";
		context_raw.merged_object.base.visible = true;
	}

	viewport_scale_to_bounds();

	if (context_raw.paint_object.base.name == "") {
		context_raw.paint_object.base.name = "Object";
	}
	make_material_parse_paint_material();
	make_material_parse_mesh_material();

	///if (is_paint || is_sculpt)
	ui_view2d_hwnd.redraws = 2;
	///end

	///if (arm_direct3d12 || arm_vulkan || arm_metal)
	render_path_raytrace_ready = false;
	///end

	///if arm_physics
	context_raw.paint_body = null;
	///end
}

function _import_mesh_make_mesh(mesh: raw_mesh_t) {
	let raw: mesh_data_t = import_mesh_raw_mesh(mesh);
	if (mesh.cola != null) {
		let va: vertex_array_t = {
			values: mesh.cola,
			attrib: "col",
			data: "short4norm"
		};
		array_push(raw.vertex_arrays, va);
	}

	let md: mesh_data_t = mesh_data_create(raw);
	context_raw.paint_object = context_main_object();

	context_select_paint_object(context_main_object());
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		if (p == context_raw.paint_object) {
			continue;
		}
		data_delete_mesh(p.data._.handle);
		mesh_object_remove(p);
	}
	let handle: string = context_raw.paint_object.data._.handle;
	if (handle != "SceneSphere" && handle != "ScenePlane") {
		data_delete_mesh(handle);
	}

	mesh_object_set_data(context_raw.paint_object, md);
	context_raw.paint_object.base.name = mesh.name;
	project_paint_objects = [context_raw.paint_object];

	md._.handle = raw.name;
	map_set(data_cached_meshes, md._.handle, md);

	context_raw.ddirty = 4;

	///if (is_paint || is_sculpt)
	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	ui_base_hwnds[tab_area_t.SIDEBAR1].redraws = 2;
	util_uv_uvmap_cached = false;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached = false;
	///end

	///if (is_paint || is_sculpt)
	if (import_mesh_clear_layers) {
		while (project_layers.length > 0) {
			let l: slot_layer_t = array_pop(project_layers);
			slot_layer_unload(l);
		}
		layers_new_layer(false);
		app_notify_on_init(layers_init);
		history_reset();
	}
	///end

	// Wait for addMesh calls to finish
	if (import_mesh_meshes_to_unwrap != null) {
		app_notify_on_next_frame(import_mesh_finish_import);
	}
	else {
		app_notify_on_init(import_mesh_finish_import);
	}
}

function import_mesh_first_unwrap_done(mesh: raw_mesh_t) {
	_import_mesh_make_mesh(mesh);
	for (let i: i32 = 0; i < import_mesh_meshes_to_unwrap.length; ++i) {
		let mesh: raw_mesh_t = import_mesh_meshes_to_unwrap[i];
		project_unwrap_mesh_box(mesh, _import_mesh_add_mesh, true);
	}
}

function import_mesh_make_mesh(mesh: raw_mesh_t) {
	if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null || mesh.posa.length == 0) {
		console_error(strings_error3());
		return;
	}

	if (mesh.texa == null) {
		if (import_mesh_meshes_to_unwrap == null) {
			import_mesh_meshes_to_unwrap = [];
		}
		project_unwrap_mesh_box(mesh, import_mesh_first_unwrap_done);
	}
	else {
		_import_mesh_make_mesh(mesh);
	}
}

function _import_mesh_add_mesh(mesh: raw_mesh_t) {
	let raw: mesh_data_t = import_mesh_raw_mesh(mesh);
	if (mesh.cola != null) {
		let va: vertex_array_t = {
			values: mesh.cola,
			attrib: "col",
			data: "short4norm"
		};
		array_push(raw.vertex_arrays, va);
	}

	let md: mesh_data_t = mesh_data_create(raw);

	let object: mesh_object_t = scene_add_mesh_object(md, context_raw.paint_object.materials, context_raw.paint_object.base);
	object.base.name = mesh.name;
	object.skip_context = "paint";

	// Ensure unique names
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		let oname: string = object.base.name;
		if (p.base.name == oname) {
			p.base.name += ".001";
			p.data._.handle += ".001";
			map_set(data_cached_meshes, p.data._.handle, p.data);
		}
	}

	array_push(project_paint_objects, object);

	md._.handle = raw.name;
	map_set(data_cached_meshes, md._.handle, md);

	context_raw.ddirty = 4;

	///if (is_paint || is_sculpt)
	ui_base_hwnds[tab_area_t.SIDEBAR0].redraws = 2;
	util_uv_uvmap_cached = false;
	util_uv_trianglemap_cached = false;
	util_uv_dilatemap_cached = false;
	///end
}

function import_mesh_add_mesh(mesh: raw_mesh_t) {
	if (mesh.texa == null) {
		if (import_mesh_meshes_to_unwrap != null) {
			array_push(import_mesh_meshes_to_unwrap, mesh);
		}
		else {
			project_unwrap_mesh_box(mesh, _import_mesh_add_mesh);
		}
	}
	else {
		_import_mesh_add_mesh(mesh);
	}
}

function import_mesh_raw_mesh(mesh: raw_mesh_t): mesh_data_t {
	let raw: mesh_data_t = {
		name: mesh.name,
		vertex_arrays: [
			{
				values: mesh.posa,
				attrib: "pos",
				data: "short4norm"
			},
			{
				values: mesh.nora,
				attrib: "nor",
				data: "short2norm"
			},
			{
				values: mesh.texa,
				attrib: "tex",
				data: "short2norm"
			}
		],
		index_arrays: [
			{
				values: mesh.inda,
				material: 0
			}
		],
		scale_pos: mesh.scale_pos,
		scale_tex: mesh.scale_tex
	};
	return raw;
}
