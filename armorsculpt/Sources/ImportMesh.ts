
class ImportMesh {

	static clearLayers = true;

	static run = (path: string, _clearLayers = true, replaceExisting = true) => {
		if (!Path.isMesh(path)) {
			if (!Context.enableImportPlugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		ImportMesh.clearLayers = _clearLayers;
		Context.raw.layerFilter = 0;

		let p = path.toLowerCase();
		if (p.endsWith(".obj")) ImportObj.run(path, replaceExisting);
		else if (p.endsWith(".blend")) ImportBlendMesh.run(path, replaceExisting);
		else {
			let ext = path.substr(path.lastIndexOf(".") + 1);
			let importer = Path.meshImporters.get(ext);
			importer(path, (mesh: any) => {
				replaceExisting ? ImportMesh.makeMesh(mesh, path) : ImportMesh.addMesh(mesh);
			});
		}

		Project.meshAssets = [path];

		///if (krom_android || krom_ios)
		sys_title_set(path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf(".")));
		///end
	}

	static finishImport = () => {
		if (Context.raw.mergedObject != null) {
			mesh_object_remove(Context.raw.mergedObject);
			data_delete_mesh(Context.raw.mergedObject.data._handle);
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

		UIView2D.hwnd.redraws = 2;

		///if arm_physics
		Context.raw.paintBody = null;
		///end
	}

	static makeMesh = (mesh: any, path: string) => {
		if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null || mesh.posa.length == 0) {
			Console.error(Strings.error3());
			return;
		}

		let _makeMesh = () => {
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

				if (ImportMesh.clearLayers) {
					while (Project.layers.length > 0) {
						let l = Project.layers.pop();
						SlotLayer.unload(l);
					}
					Base.newLayer(false);
					app_notify_on_init(Base.initLayers);
					History.reset();
				}

				mesh_object_set_data(Context.raw.paintObject, md);
				Context.raw.paintObject.base.name = mesh.name;
				Project.paintObjects = [Context.raw.paintObject];

				md._handle = raw.name;
				data_cached_meshes.set(md._handle, md);

				Context.raw.ddirty = 4;
				UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
				UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;

				// Wait for addMesh calls to finish
				app_notify_on_init(ImportMesh.finishImport);

				Base.notifyOnNextFrame(() => {
					let f32 = new Float32Array(Config.getTextureResX() * Config.getTextureResY() * 4);
					for (let i = 0; i < Math.floor(mesh.inda.length); ++i) {
						let index = mesh.inda[i];
						f32[i * 4]     = mesh.posa[index * 4]     / 32767;
						f32[i * 4 + 1] = mesh.posa[index * 4 + 1] / 32767;
						f32[i * 4 + 2] = mesh.posa[index * 4 + 2] / 32767;
						f32[i * 4 + 3] = 1.0;
					}
					let imgmesh = image_from_bytes(f32.buffer, Config.getTextureResX(), Config.getTextureResY(), tex_format_t.RGBA128);
					let texpaint = Project.layers[0].texpaint;
					g2_begin(texpaint, false);
					g2_set_pipeline(Base.pipeCopy128);
					g2_draw_scaled_image(imgmesh, 0, 0, Config.getTextureResX(), Config.getTextureResY());
					g2_set_pipeline(null);
					g2_end();
				});
			});
		}

		_makeMesh();
	}

	static addMesh = (mesh: any) => {

		let _addMesh = () => {
			let raw = ImportMesh.rawMesh(mesh);
			if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

			mesh_data_create(raw, (md: mesh_data_t) => {

				let object = scene_add_mesh_object(md, Context.raw.paintObject.materials, Context.raw.paintObject.base);
				object.base.name = mesh.base.name;
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
				UIBase.hwnds[TabArea.TabSidebar0].redraws = 2;
			});
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
