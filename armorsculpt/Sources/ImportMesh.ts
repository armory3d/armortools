
class ImportMesh {

	static clearLayers = true;

	static run = (path: string, _clearLayers = true, replaceExisting = true) => {
		if (!Path.isMesh(path)) {
			if (!Context.enableImportPlugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		clearLayers = _clearLayers;
		Context.raw.layerFilter = 0;

		let p = path.toLowerCase();
		if (p.endsWith(".obj")) ImportObj.run(path, replaceExisting);
		else if (p.endsWith(".fbx")) ImportFbx.run(path, replaceExisting);
		else if (p.endsWith(".blend")) ImportBlendMesh.run(path, replaceExisting);
		else {
			let ext = path.substr(path.lastIndexOf(".") + 1);
			let importer = Path.meshImporters.get(ext);
			importer(path, (mesh: any) => {
				replaceExisting ? makeMesh(mesh, path) : addMesh(mesh);
			});
		}

		Project.meshAssets = [path];

		///if (krom_android || krom_ios)
		System.title = path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf("."));
		///end
	}

	static finishImport = () => {
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.remove();
			Data.deleteMesh(Context.raw.mergedObject.data.handle);
			Context.raw.mergedObject = null;
		}

		Context.selectPaintObject(Context.mainObject());

		if (Project.paintObjects.length > 1) {
			// Sort by name
			Project.paintObjects.sort((a, b): i32 => {
				if (a.name < b.name) return -1;
				else if (a.name > b.name) return 1;
				return 0;
			});

			// No mask by default
			for (let p of Project.paintObjects) p.visible = true;
			if (Context.raw.mergedObject == null) UtilMesh.mergeMesh();
			Context.raw.paintObject.skip_context = "paint";
			Context.raw.mergedObject.visible = true;
		}

		Viewport.scaleToBounds();

		if (Context.raw.paintObject.name == "") Context.raw.paintObject.name = "Object";
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();

		UIView2D.inst.hwnd.redraws = 2;

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
			let raw = rawMesh(mesh);
			if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

			new MeshData(raw, (md: MeshData) => {
				Context.raw.paintObject = Context.mainObject();

				Context.selectPaintObject(Context.mainObject());
				for (let i = 0; i < Project.paintObjects.length; ++i) {
					let p = Project.paintObjects[i];
					if (p == Context.raw.paintObject) continue;
					Data.deleteMesh(p.data.handle);
					p.remove();
				}
				let handle = Context.raw.paintObject.data.handle;
				if (handle != "SceneSphere" && handle != "ScenePlane") {
					Data.deleteMesh(handle);
				}

				if (clearLayers) {
					while (Project.layers.length > 0) {
						let l = Project.layers.pop();
						l.unload();
					}
					Base.newLayer(false);
					App.notifyOnInit(Base.initLayers);
					History.reset();
				}

				Context.raw.paintObject.setData(md);
				Context.raw.paintObject.name = mesh.name;
				Project.paintObjects = [Context.raw.paintObject];

				md.handle = raw.name;
				Data.cachedMeshes.set(md.handle, md);

				Context.raw.ddirty = 4;
				UIBase.inst.hwnds[TabSidebar0].redraws = 2;
				UIBase.inst.hwnds[TabSidebar1].redraws = 2;

				// Wait for addMesh calls to finish
				App.notifyOnInit(finishImport);

				Base.notifyOnNextFrame(() => {
					let f32 = new Float32Array(Config.getTextureResX() * Config.getTextureResY() * 4);
					for (let i = 0; i < Math.floor(mesh.inda.length); ++i) {
						let index = mesh.inda[i];
						f32[i * 4]     = mesh.posa[index * 4]     / 32767;
						f32[i * 4 + 1] = mesh.posa[index * 4 + 1] / 32767;
						f32[i * 4 + 2] = mesh.posa[index * 4 + 2] / 32767;
						f32[i * 4 + 3] = 1.0;
					}
					let imgmesh = Image.fromBytes(f32.buffer, Config.getTextureResX(), Config.getTextureResY(), TextureFormat.RGBA128);
					let texpaint = Project.layers[0].texpaint;
					texpaint.g2.begin(false);
					texpaint.g2.pipeline = Base.pipeCopy128;
					texpaint.g2.drawScaledImage(imgmesh, 0, 0, Config.getTextureResX(), Config.getTextureResY());
					texpaint.g2.pipeline = null;
					texpaint.g2.end();
				});
			});
		}

		_makeMesh();
	}

	static addMesh = (mesh: any) => {

		let _addMesh = () => {
			let raw = rawMesh(mesh);
			if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

			new MeshData(raw, (md: MeshData) => {

				let object = Scene.active.addMeshObject(md, Context.raw.paintObject.materials, Context.raw.paintObject);
				object.name = mesh.name;
				object.skip_context = "paint";

				// Ensure unique names
				for (let p of Project.paintObjects) {
					if (p.name == object.name) {
						p.name += ".001";
						p.data.handle += ".001";
						Data.cachedMeshes.set(p.data.handle, p.data);
					}
				}

				Project.paintObjects.push(object);

				md.handle = raw.name;
				Data.cachedMeshes.set(md.handle, md);

				Context.raw.ddirty = 4;
				UIBase.inst.hwnds[TabSidebar0].redraws = 2;
			});
		}

		_addMesh();
	}

	static rawMesh = (mesh: any): TMeshData => {
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
