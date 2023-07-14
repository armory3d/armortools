package arm.io;

import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.Scene;
import arm.util.MeshUtil;
import arm.Viewport;
import arm.sys.Path;
import arm.ui.UIBase;
import arm.ui.UIView2D;
import arm.Project;

class ImportMesh {

	static var clearLayers = true;

	public static function run(path: String, _clearLayers = true, replaceExisting = true) {
		if (!Path.isMesh(path)) {
			if (!Context.enableImportPlugin(path)) {
				Console.error(Strings.error1());
				return;
			}
		}

		clearLayers = _clearLayers;
		Context.raw.layerFilter = 0;

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

		#if arm_debug
		var ar = path.split(Path.sep);
		var name = ar[ar.length - 1];
		Console.info(tr("Mesh imported:") + " " + name);
		#end

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

		UIView2D.inst.hwnd.redraws = 2;

		#if arm_debug
		trace("Mesh imported in " + (iron.system.Time.realTime() - timer));
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

				if (clearLayers) {
					while (Project.layers.length > 0) {
						var l = Project.layers.pop();
						l.unload();
					}
					App.newLayer(false);
					iron.App.notifyOnInit(App.initLayers);
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
				iron.App.notifyOnInit(finishImport);

				arm.App.notifyOnNextFrame(function() {
					var f32 = new kha.arrays.Float32Array(Config.getTextureResX() * Config.getTextureResY() * 4);
					for (i in 0...raw.index_arrays[0].values.length) {
						var index = raw.index_arrays[0].values[i];
						f32[i * 4]     = mesh.posa[index * 4]     / 32767;
						f32[i * 4 + 1] = mesh.posa[index * 4 + 1] / 32767;
						f32[i * 4 + 2] = mesh.posa[index * 4 + 2] / 32767;
						f32[i * 4 + 3] = 1.0;
					}
					var bytes = haxe.io.Bytes.ofData(f32.buffer);
					var imgmesh = kha.Image.fromBytes(bytes, Config.getTextureResX(), Config.getTextureResY(), kha.graphics4.TextureFormat.RGBA128);
					var texpaint = Project.layers[0].texpaint;
					texpaint.g2.begin(false);
					texpaint.g2.pipeline = App.pipeCopy128;
					texpaint.g2.drawScaledImage(imgmesh, 0, 0, Config.getTextureResX(), Config.getTextureResY());
					texpaint.g2.pipeline = null;
					texpaint.g2.end();
				});
			});
		}

		_makeMesh();
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
				UIBase.inst.hwnds[TabSidebar0].redraws = 2;
			});
		}

		_addMesh();
	}

	static function rawMesh(mesh: Dynamic): TMeshData {
		var posa = new kha.arrays.Int16Array(Std.int(mesh.inda.length * 4));
		for (i in 0...posa.length) posa[i] = 32767;
		var inda = new kha.arrays.Uint32Array(mesh.inda.length);
		for (i in 0...inda.length) inda[i] = i;
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
