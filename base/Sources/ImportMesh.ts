
class ImportMesh {

	///if (is_paint || is_sculpt)
	static clearLayers = true;
	///end

	static meshesToUnwrap: any[] = null;

	///if (is_paint || is_sculpt)
	static run = (path: string, _clearLayers = true, replaceExisting = true) => {
	///end

	///if is_lab
	static run = (path: string, replaceExisting = true) => {
	///end

		if (!Path.isMesh(path)) {
			if (!Context.enableImportPlugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		///if (is_paint || is_sculpt)
		ImportMesh.clearLayers = _clearLayers;
		Context.raw.layerFilter = 0;
		///end

		ImportMesh.meshesToUnwrap = null;

		let p = path.toLowerCase();
		if (p.endsWith(".obj")) ImportObj.run(path, replaceExisting);
		else if (p.endsWith(".blend")) ImportBlendMesh.run(path, replaceExisting);
		else {
			let ext = path.substr(path.lastIndexOf(".") + 1);
			let importer = Path.meshImporters.get(ext);
			importer(path, (mesh: any) => {
				replaceExisting ? ImportMesh.makeMesh(mesh, path) : ImportMesh.addMesh(mesh);

				let has_next = mesh.has_next;
				while (has_next) {
					importer(path, (mesh: any) => {
						has_next = mesh.has_next;
						ImportMesh.addMesh(mesh);

						// let m = fromFloat32Array(mesh.transform);
						// Project.paintObjects[Project.paintObjects.length - 1].transform.localOnly = true;
						// Project.paintObjects[Project.paintObjects.length - 1].transform.setMatrix(m);
					});
				}
			});
		}

		Project.meshAssets = [path];

		///if (krom_android || krom_ios)
		sys_title_set(path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf(".")));
		///end
	}

	static finishImport = () => {
		if (Context.raw.mergedObject != null) {
			mesh_data_delete(Context.raw.mergedObject.data);
			mesh_object_remove(Context.raw.mergedObject);
			Context.raw.mergedObject = null;
		}

		Context.selectPaintObject(Context.mainObject());

		if (Project.paintObjects.length > 1) {
			// Sort by name
			Project.paintObjects.sort((a, b): i32 => {
				if (a.base.name < b.base.name) return -1;
				else if (a.base.name > b.base.name) return 1;
				return 0;
			});

			// No mask by default
			for (let p of Project.paintObjects) p.base.visible = true;
			if (Context.raw.mergedObject == null) UtilMesh.mergeMesh();
			Context.raw.paintObject.skip_context = "paint";
			Context.raw.mergedObject.base.visible = true;
		}

		Viewport.scaleToBounds();

		if (Context.raw.paintObject.base.name == "") Context.raw.paintObject.base.name = "Object";
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();

		///if (is_paint || is_sculpt)
		UIView2D.hwnd.redraws = 2;
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.ready = false;
		///end

		///if arm_physics
		Context.raw.paintBody = null;
		///end
	}

	static _makeMesh = (mesh: any) => {
		let raw = ImportMesh.rawMesh(mesh);
		if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

		mesh_data_create(raw, (md: mesh_data_t) => {
			Context.raw.paintObject = Context.mainObject();

			Context.selectPaintObject(Context.mainObject());
			for (let i = 0; i < Project.paintObjects.length; ++i) {
				let p = Project.paintObjects[i];
				if (p == Context.raw.paintObject) continue;
				data_delete_mesh(p.data._handle);
				mesh_object_remove(p);
			}
			let handle = Context.raw.paintObject.data._handle;
			if (handle != "SceneSphere" && handle != "ScenePlane") {
				data_delete_mesh(handle);
			}

			mesh_object_set_data(Context.raw.paintObject, md);
			Context.raw.paintObject.base.name = mesh.name;
			Project.paintObjects = [Context.raw.paintObject];

			md._handle = raw.name;
			data_cached_meshes.set(md._handle, md);

			Context.raw.ddirty = 4;

			///if (is_paint || is_sculpt)
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			UtilUV.uvmapCached = false;
			UtilUV.trianglemapCached = false;
			UtilUV.dilatemapCached = false;
			///end

			///if (is_paint || is_sculpt)
			if (ImportMesh.clearLayers) {
				while (Project.layers.length > 0) {
					let l = Project.layers.pop();
					SlotLayer.unload(l);
				}
				Base.newLayer(false);
				app_notify_on_init(Base.initLayers);
				History.reset();
			}
			///end

			// Wait for addMesh calls to finish
			if (ImportMesh.meshesToUnwrap != null) {
				Base.notifyOnNextFrame(ImportMesh.finishImport);
			}
			else {
				app_notify_on_init(ImportMesh.finishImport);
			}
		});
	}

	static makeMesh = (mesh: any, path: string) => {
		if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null || mesh.posa.length == 0) {
			Console.error(Strings.error3());
			return;
		}

		if (mesh.texa == null) {
			if (ImportMesh.meshesToUnwrap == null) {
				ImportMesh.meshesToUnwrap = [];
			}
			let firstUnwrapDone = (mesh: any) => {
				ImportMesh._makeMesh(mesh);
				for (let mesh of ImportMesh.meshesToUnwrap) Project.unwrapMeshBox(mesh, ImportMesh._addMesh, true);
			}
			Project.unwrapMeshBox(mesh, firstUnwrapDone);
		}
		else {
			ImportMesh._makeMesh(mesh);
		}
	}

	static _addMesh = (mesh: any) => {
		let raw = ImportMesh.rawMesh(mesh);
		if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

		mesh_data_create(raw, (md: mesh_data_t) => {

			let object = scene_add_mesh_object(md, Context.raw.paintObject.materials, Context.raw.paintObject.base);
			object.base.name = mesh.name;
			object.skip_context = "paint";

			// Ensure unique names
			for (let p of Project.paintObjects) {
				if (p.base.name == object.base.name) {
					p.base.name += ".001";
					p.data._handle += ".001";
					data_cached_meshes.set(p.data._handle, p.data);
				}
			}

			Project.paintObjects.push(object);

			md._handle = raw.name;
			data_cached_meshes.set(md._handle, md);

			Context.raw.ddirty = 4;

			///if (is_paint || is_sculpt)
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
			UtilUV.uvmapCached = false;
			UtilUV.trianglemapCached = false;
			UtilUV.dilatemapCached = false;
			///end
		});
	}

	static addMesh = (mesh: any) => {
		if (mesh.texa == null) {
			if (ImportMesh.meshesToUnwrap != null) ImportMesh.meshesToUnwrap.push(mesh);
			else Project.unwrapMeshBox(mesh, ImportMesh._addMesh);
		}
		else {
			ImportMesh._addMesh(mesh);
		}
	}

	static rawMesh = (mesh: any): mesh_data_t => {
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
