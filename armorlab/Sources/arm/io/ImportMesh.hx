package arm.io;

import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.Scene;
import arm.util.MeshUtil;
import arm.sys.Path;
import arm.Viewport;
import arm.Project;

class ImportMesh {

	public static function run(path: String, replaceExisting = true) {
		if (!Path.isMesh(path)) {
			if (!ContextBase.enableImportPlugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		#if arm_debug
		var timer = iron.system.Time.realTime();
		#end

		var p = path.toLowerCase();
		if (p.endsWith(".obj")) ImportObj.run(path, replaceExisting);
		else if (p.endsWith(".fbx")) ImportFbx.run(path, replaceExisting);
		else if (p.endsWith(".blend")) ImportBlendMesh.run(path, replaceExisting);
		else {
			var ext = path.substr(path.lastIndexOf(".") + 1);
			var importer = Path.meshImporters.get(ext);
			importer(path, function(mesh: Dynamic) {
				replaceExisting ? makeMesh(mesh, path) : addMesh(mesh);
			});
		}

		Project.meshAssets = [path];

		#if (krom_android || krom_ios)
		kha.Window.get(0).title = path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf("."));
		#end
	}

	static function finishImport() {
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.remove();
			Data.deleteMesh(Context.raw.mergedObject.data.handle);
			Context.raw.mergedObject = null;
		}

		Context.selectPaintObject(Context.mainObject());

		if (Project.paintObjects.length > 1) {
			// Sort by name
			Project.paintObjects.sort(function(a, b): Int {
				if (a.name < b.name) return -1;
				else if (a.name > b.name) return 1;
				return 0;
			});

			// No mask by default
			for (p in Project.paintObjects) p.visible = true;
			if (Context.raw.mergedObject == null) MeshUtil.mergeMesh();
			Context.raw.paintObject.skip_context = "paint";
			Context.raw.mergedObject.visible = true;
		}

		Viewport.scaleToBounds();

		if (Context.raw.paintObject.name == "") Context.raw.paintObject.name = "Object";
		arm.shader.MakeMaterial.parsePaintMaterial();
		arm.shader.MakeMaterial.parseMeshMaterial();

		#if arm_debug
		trace("Mesh imported in " + (iron.system.Time.realTime() - timer));
		#end

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end

		#if arm_physics
		Context.raw.paintBody = null;
		#end
	}

	public static function makeMesh(mesh: Dynamic, path: String) {
		if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null || mesh.posa.length == 0) {
			Console.error(Strings.error3());
			return;
		}

		function _makeMesh() {
			var raw = rawMesh(mesh);
			if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

			new MeshData(raw, function(md: MeshData) {
				Context.raw.paintObject = Context.mainObject();

				Context.selectPaintObject(Context.mainObject());
				for (i in 0...Project.paintObjects.length) {
					var p = Project.paintObjects[i];
					if (p == Context.raw.paintObject) continue;
					Data.deleteMesh(p.data.handle);
					p.remove();
				}
				var handle = Context.raw.paintObject.data.handle;
				if (handle != "SceneSphere" && handle != "ScenePlane") {
					Data.deleteMesh(handle);
				}

				Context.raw.paintObject.setData(md);
				Context.raw.paintObject.name = mesh.name;
				Project.paintObjects = [Context.raw.paintObject];

				md.handle = raw.name;
				Data.cachedMeshes.set(md.handle, md);

				Context.raw.ddirty = 4;

				// Wait for addMesh calls to finish
				iron.App.notifyOnInit(finishImport);
			});
		}

		if (mesh.texa == null) {
			Project.unwrapMeshBox(mesh, _makeMesh);
		}
		else {
			_makeMesh();
		}
	}

	public static function addMesh(mesh: Dynamic) {

		function _addMesh() {
			var raw = rawMesh(mesh);
			if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

			new MeshData(raw, function(md: MeshData) {

				var object = Scene.active.addMeshObject(md, Context.raw.paintObject.materials, Context.raw.paintObject);
				object.name = mesh.name;
				object.skip_context = "paint";

				// Ensure unique names
				for (p in Project.paintObjects) {
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
			});
		}

		if (mesh.texa == null) {
			Project.unwrapMeshBox(mesh, _addMesh);
		}
		else {
			_addMesh();
		}
	}

	static function rawMesh(mesh: Dynamic): TMeshData {
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
