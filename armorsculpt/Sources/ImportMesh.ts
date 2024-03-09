
class ImportMesh {

	static clearLayers = true;

	static run = (path: string, _clearLayers = true, replaceExisting = true) => {
		if (!path_isMesh(path)) {
			if (!context_enableImportPlugin(path)) {
				console_error(Strings.error1());
				return;
			}
		}

		ImportMesh.clearLayers = _clearLayers;
		context_raw.layerFilter = 0;

		let p = path.toLowerCase();
		if (p.endsWith(".obj")) ImportObj.run(path, replaceExisting);
		else if (p.endsWith(".blend")) ImportBlendMesh.run(path, replaceExisting);
		else {
			let ext = path.substr(path.lastIndexOf(".") + 1);
			let importer = path_meshImporters.get(ext);
			importer(path, (mesh: any) => {
				replaceExisting ? ImportMesh.makeMesh(mesh, path) : ImportMesh.addMesh(mesh);
			});
		}

		project_meshAssets = [path];

		///if (krom_android || krom_ios)
		sys_title_set(path.substring(path.lastIndexOf(path_sep) + 1, path.lastIndexOf(".")));
		///end
	}

	static finishImport = () => {
		if (context_raw.mergedObject != null) {
			mesh_object_remove(context_raw.mergedObject);
			data_delete_mesh(context_raw.mergedObject.data._.handle);
			context_raw.mergedObject = null;
		}

		context_selectPaintObject(context_mainObject());

		if (project_paintObjects.length > 1) {
			// Sort by name
			project_paintObjects.sort((a, b): i32 => {
				if (a.base.name < b.base.name) return -1;
				else if (a.base.name > b.base.name) return 1;
				return 0;
			});

			// No mask by default
			for (let p of project_paintObjects) p.base.visible = true;
			if (context_raw.mergedObject == null) UtilMesh.mergeMesh();
			context_raw.paintObject.skip_context = "paint";
			context_raw.mergedObject.base.visible = true;
		}

		Viewport.scaleToBounds();

		if (context_raw.paintObject.base.name == "") context_raw.paintObject.base.name = "Object";
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();

		UIView2D.hwnd.redraws = 2;

		///if arm_physics
		context_raw.paintBody = null;
		///end
	}

	static makeMesh = (mesh: any, path: string) => {
		if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null || mesh.posa.length == 0) {
			console_error(Strings.error3());
			return;
		}

		let _makeMesh = () => {
			let raw = ImportMesh.rawMesh(mesh);
			if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm" });

			let md: mesh_data_t = mesh_data_create(raw);
			context_raw.paintObject = context_mainObject();

			context_selectPaintObject(context_mainObject());
			for (let i = 0; i < project_paintObjects.length; ++i) {
				let p = project_paintObjects[i];
				if (p == context_raw.paintObject) continue;
				data_delete_mesh(p.data._.handle);
				mesh_object_remove(p);
			}
			let handle = context_raw.paintObject.data._.handle;
			if (handle != "SceneSphere" && handle != "ScenePlane") {
				data_delete_mesh(handle);
			}

			if (ImportMesh.clearLayers) {
				while (project_layers.length > 0) {
					let l = project_layers.pop();
					SlotLayer.unload(l);
				}
				base_newLayer(false);
				app_notify_on_init(base_initLayers);
				history_reset();
			}

			mesh_object_set_data(context_raw.paintObject, md);
			context_raw.paintObject.base.name = mesh.name;
			project_paintObjects = [context_raw.paintObject];

			md._.handle = raw.name;
			data_cached_meshes.set(md._.handle, md);

			context_raw.ddirty = 4;
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;

			// Wait for addMesh calls to finish
			app_notify_on_init(ImportMesh.finishImport);

			base_notifyOnNextFrame(() => {
				let f32 = new Float32Array(config_getTextureResX() * config_getTextureResY() * 4);
				for (let i = 0; i < Math.floor(mesh.inda.length); ++i) {
					let index = mesh.inda[i];
					f32[i * 4]     = mesh.posa[index * 4]     / 32767;
					f32[i * 4 + 1] = mesh.posa[index * 4 + 1] / 32767;
					f32[i * 4 + 2] = mesh.posa[index * 4 + 2] / 32767;
					f32[i * 4 + 3] = 1.0;
				}
				let imgmesh = image_from_bytes(f32.buffer, config_getTextureResX(), config_getTextureResY(), tex_format_t.RGBA128);
				let texpaint = project_layers[0].texpaint;
				g2_begin(texpaint);
				g2_set_pipeline(base_pipeCopy128);
				g2_draw_scaled_image(imgmesh, 0, 0, config_getTextureResX(), config_getTextureResY());
				g2_set_pipeline(null);
				g2_end();
			});
		}

		_makeMesh();
	}

	static addMesh = (mesh: any) => {

		let _addMesh = () => {
			let raw = ImportMesh.rawMesh(mesh);
			if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm" });

			let md: mesh_data_t = mesh_data_create(raw);

			let object = scene_add_mesh_object(md, context_raw.paintObject.materials, context_raw.paintObject.base);
			object.base.name = mesh.base.name;
			object.skip_context = "paint";

			// Ensure unique names
			for (let p of project_paintObjects) {
				if (p.base.name == object.base.name) {
					p.base.name += ".001";
					p.data._.handle += ".001";
					data_cached_meshes.set(p.data._.handle, p.data);
				}
			}

			project_paintObjects.push(object);

			md._.handle = raw.name;
			data_cached_meshes.set(md._.handle, md);

			context_raw.ddirty = 4;
			UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
		}

		_addMesh();
	}

	static rawMesh = (mesh: any): mesh_data_t => {
		let posa = new Int16Array(Math.floor(mesh.inda.length * 4));
		for (let i = 0; i < posa.length; ++i) posa[i] = 32767;
		let inda = new Uint32Array(mesh.inda.length);
		for (let i = 0; i < inda.length; ++i) inda[i] = i;
		return {
			name: mesh.name,
			vertex_arrays: [
				{ values: posa, attrib: "pos", data: "short4norm" }
			],
			index_arrays: [
				{ values: inda, material: 0 }
			],
			scale_pos: 1.0
		};
	}
}
