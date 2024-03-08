
class ImportMesh {

	///if (is_paint || is_sculpt)
	static clear_layers: bool = true;
	///end

	static meshes_to_unwrap: any[] = null;

	///if (is_paint || is_sculpt)
	static run = (path: string, _clear_layers: bool = true, replace_existing: bool = true) => {
	///end

	///if is_lab
	static run = (path: string, replace_existing: bool = true) => {
	///end

		if (!Path.is_mesh(path)) {
			if (!Context.enable_import_plugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		///if (is_paint || is_sculpt)
		ImportMesh.clear_layers = _clear_layers;
		Context.raw.layer_filter = 0;
		///end

		ImportMesh.meshes_to_unwrap = null;

		let p: string = path.toLowerCase();
		if (p.endsWith(".obj")) ImportObj.run(path, replace_existing);
		else if (p.endsWith(".blend")) ImportBlendMesh.run(path, replace_existing);
		else {
			let ext: string = path.substr(path.lastIndexOf(".") + 1);
			let importer: (s: string, f: (a: any)=>void)=>void = Path.mesh_importers.get(ext);
			importer(path, (mesh: any) => {
				replace_existing ? ImportMesh.make_mesh(mesh, path) : ImportMesh.add_mesh(mesh);

				let has_next: bool = mesh.has_next;
				while (has_next) {
					importer(path, (mesh: any) => {
						has_next = mesh.has_next;
						ImportMesh.add_mesh(mesh);

						// let m: mat4_t = fromFloat32Array(mesh.transform);
						// Project.paintObjects[Project.paintObjects.length - 1].transform.localOnly = true;
						// Project.paintObjects[Project.paintObjects.length - 1].transform.setMatrix(m);
					});
				}
			});
		}

		Project.mesh_assets = [path];

		///if (krom_android || krom_ios)
		sys_title_set(path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf(".")));
		///end
	}

	static finish_import = () => {
		if (Context.raw.merged_object != null) {
			mesh_data_delete(Context.raw.merged_object.data);
			mesh_object_remove(Context.raw.merged_object);
			Context.raw.merged_object = null;
		}

		Context.select_paint_object(Context.main_object());

		if (Project.paint_objects.length > 1) {
			// Sort by name
			Project.paint_objects.sort((a, b): i32 => {
				if (a.base.name < b.base.name) return -1;
				else if (a.base.name > b.base.name) return 1;
				return 0;
			});

			// No mask by default
			for (let p of Project.paint_objects) p.base.visible = true;
			if (Context.raw.merged_object == null) UtilMesh.merge_mesh();
			Context.raw.paint_object.skip_context = "paint";
			Context.raw.merged_object.base.visible = true;
		}

		Viewport.scale_to_bounds();

		if (Context.raw.paint_object.base.name == "") Context.raw.paint_object.base.name = "Object";
		MakeMaterial.parse_paint_material();
		MakeMaterial.parse_mesh_material();

		///if (is_paint || is_sculpt)
		UIView2D.hwnd.redraws = 2;
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end

		///if arm_physics
		Context.raw.paint_body = null;
		///end
	}

	static _make_mesh = (mesh: any) => {
		let raw: mesh_data_t = ImportMesh.raw_mesh(mesh);
		if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm" });

		let md: mesh_data_t = mesh_data_create(raw);
		Context.raw.paint_object = Context.main_object();

		Context.select_paint_object(Context.main_object());
		for (let i: i32 = 0; i < Project.paint_objects.length; ++i) {
			let p: mesh_object_t = Project.paint_objects[i];
			if (p == Context.raw.paint_object) continue;
			data_delete_mesh(p.data._.handle);
			mesh_object_remove(p);
		}
		let handle: string = Context.raw.paint_object.data._.handle;
		if (handle != "SceneSphere" && handle != "ScenePlane") {
			data_delete_mesh(handle);
		}

		mesh_object_set_data(Context.raw.paint_object, md);
		Context.raw.paint_object.base.name = mesh.name;
		Project.paint_objects = [Context.raw.paint_object];

		md._.handle = raw.name;
		data_cached_meshes.set(md._.handle, md);

		Context.raw.ddirty = 4;

		///if (is_paint || is_sculpt)
		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		UIBase.hwnds[tab_area_t.SIDEBAR1].redraws = 2;
		UtilUV.uvmap_cached = false;
		UtilUV.trianglemap_cached = false;
		UtilUV.dilatemap_cached = false;
		///end

		///if (is_paint || is_sculpt)
		if (ImportMesh.clear_layers) {
			while (Project.layers.length > 0) {
				let l: SlotLayerRaw = Project.layers.pop();
				SlotLayer.unload(l);
			}
			base_new_layer(false);
			app_notify_on_init(base_init_layers);
			History.reset();
		}
		///end

		// Wait for addMesh calls to finish
		if (ImportMesh.meshes_to_unwrap != null) {
			base_notify_on_next_frame(ImportMesh.finish_import);
		}
		else {
			app_notify_on_init(ImportMesh.finish_import);
		}
	}

	static make_mesh = (mesh: any, path: string) => {
		if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null || mesh.posa.length == 0) {
			Console.error(Strings.error3());
			return;
		}

		if (mesh.texa == null) {
			if (ImportMesh.meshes_to_unwrap == null) {
				ImportMesh.meshes_to_unwrap = [];
			}
			let first_unwrap_done = (mesh: any) => {
				ImportMesh._make_mesh(mesh);
				for (let mesh of ImportMesh.meshes_to_unwrap) Project.unwrap_mesh_box(mesh, ImportMesh._add_mesh, true);
			}
			Project.unwrap_mesh_box(mesh, first_unwrap_done);
		}
		else {
			ImportMesh._make_mesh(mesh);
		}
	}

	static _add_mesh = (mesh: any) => {
		let raw: mesh_data_t = ImportMesh.raw_mesh(mesh);
		if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm" });

		let md: mesh_data_t = mesh_data_create(raw);

		let object: mesh_object_t = scene_add_mesh_object(md, Context.raw.paint_object.materials, Context.raw.paint_object.base);
		object.base.name = mesh.name;
		object.skip_context = "paint";

		// Ensure unique names
		for (let p of Project.paint_objects) {
			if (p.base.name == object.base.name) {
				p.base.name += ".001";
				p.data._.handle += ".001";
				data_cached_meshes.set(p.data._.handle, p.data);
			}
		}

		Project.paint_objects.push(object);

		md._.handle = raw.name;
		data_cached_meshes.set(md._.handle, md);

		Context.raw.ddirty = 4;

		///if (is_paint || is_sculpt)
		UIBase.hwnds[tab_area_t.SIDEBAR0].redraws = 2;
		UtilUV.uvmap_cached = false;
		UtilUV.trianglemap_cached = false;
		UtilUV.dilatemap_cached = false;
		///end
	}

	static add_mesh = (mesh: any) => {
		if (mesh.texa == null) {
			if (ImportMesh.meshes_to_unwrap != null) ImportMesh.meshes_to_unwrap.push(mesh);
			else Project.unwrap_mesh_box(mesh, ImportMesh._add_mesh);
		}
		else {
			ImportMesh._add_mesh(mesh);
		}
	}

	static raw_mesh = (mesh: any): mesh_data_t => {
		return {
			name: mesh.name,
			vertex_arrays: [
				{ values: mesh.posa, attrib: "pos", data: "short4norm" },
				{ values: mesh.nora, attrib: "nor", data: "short2norm" },
				{ values: mesh.texa, attrib: "tex", data: "short2norm" }
			],
			index_arrays: [
				{ values: mesh.inda, material: 0 }
			],
			scale_pos: mesh.scalePos,
			scale_tex: mesh.scaleTex
		};
	}
}
