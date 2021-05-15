package arm.io;

import kha.arrays.Int16Array;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.math.Vec4;
import iron.Scene;
import arm.util.MeshUtil;
import arm.util.UVUtil;
import arm.util.ViewportUtil;
import arm.sys.Path;
import arm.ui.UIHeader;
import arm.ui.UISidebar;
import arm.ui.UIView2D;
import arm.Project;
import arm.Enums;

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
		Context.layerFilter = 0;

		#if arm_debug
		var timer = iron.system.Time.realTime();
		#end

		var p = path.toLowerCase();
		if (p.endsWith(".obj")) ImportObj.run(path, replaceExisting);
		else if (p.endsWith(".fbx")) ImportFbx.run(path, replaceExisting);
		else if (p.endsWith(".blend")) ImportBlend.run(path, replaceExisting);
		else {
			var ext = path.substr(path.lastIndexOf(".") + 1);
			var importer = Path.meshImporters.get(ext);
			importer(path, function(mesh: Dynamic) {
				replaceExisting ? ImportMesh.makeMesh(mesh, path) : ImportMesh.addMesh(mesh);
			});
		}

		if (Context.mergedObject != null) {
			Context.mergedObject.remove();
			Data.deleteMesh(Context.mergedObject.data.handle);
			Context.mergedObject = null;
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
			if (Context.mergedObject == null) MeshUtil.mergeMesh();
			Context.paintObject.skip_context = "paint";
			Context.mergedObject.visible = true;
		}
		Project.meshAssets = [path];

		ViewportUtil.scaleToBounds();

		if (Context.paintObject.name == "") Context.paintObject.name = "Object";
		arm.node.MakeMaterial.parsePaintMaterial();
		arm.node.MakeMaterial.parseMeshMaterial();

		UIView2D.inst.hwnd.redraws = 2;

		#if arm_debug
		trace("Mesh imported in " + (iron.system.Time.realTime() - timer));
		#end

		#if (kha_direct3d12 || kha_vulkan)
		arm.render.RenderPathRaytrace.ready = false;
		#end
	}

	public static function makeMesh(mesh: Dynamic, path: String) {
		if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null || mesh.posa.length == 0) {
			Console.error(Strings.error3());
			return;
		}

		var raw = rawMesh(mesh);
		if (mesh.texa == null) equirectUnwrap(mesh);
		raw.vertex_arrays.push({ values: mesh.texa, attrib: "tex", data: "short2norm" });
		if (mesh.cola != null) raw.vertex_arrays.push({ values: mesh.cola, attrib: "col", data: "short4norm", padding: 1 });

		new MeshData(raw, function(md: MeshData) {
			Context.paintObject = Context.mainObject();

			Context.selectPaintObject(Context.mainObject());
			for (i in 0...Project.paintObjects.length) {
				var p = Project.paintObjects[i];
				if (p == Context.paintObject) continue;
				Data.deleteMesh(p.data.handle);
				p.remove();
			}
			var handle = Context.paintObject.data.handle;
			if (handle != "SceneSphere" && handle != "ScenePlane") {
				Data.deleteMesh(handle);
			}

			if (clearLayers) {
				while (Project.layers.length > 0) {
					var l = Project.layers.pop();
					l.unload();
				}
				Layers.newLayer(false);
				iron.App.notifyOnInit(Layers.initLayers);
				History.reset();
			}

			Context.paintObject.setData(md);
			Context.paintObject.name = mesh.name;
			Project.paintObjects = [Context.paintObject];

			md.handle = raw.name;
			Data.cachedMeshes.set(md.handle, md);

			Context.ddirty = 4;
			UISidebar.inst.hwnd0.redraws = 2;
			UISidebar.inst.hwnd1.redraws = 2;
			UISidebar.inst.hwnd2.redraws = 2;
			UVUtil.uvmapCached = false;
			UVUtil.trianglemapCached = false;
			UVUtil.dilatemapCached = false;
		});
	}

	public static function addMesh(mesh: Dynamic) {

		if (mesh.texa == null) {
			equirectUnwrap(mesh);
		}
		var raw = rawMesh(mesh);
		raw.vertex_arrays.push({ values: mesh.texa, attrib: "tex", data: "short2norm" });

		new MeshData(raw, function(md: MeshData) {

			var object = Scene.active.addMeshObject(md, Context.paintObject.materials, Context.paintObject);
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

			Context.ddirty = 4;
			UISidebar.inst.hwnd0.redraws = 2;
			UVUtil.uvmapCached = false;
			UVUtil.trianglemapCached = false;
			UVUtil.dilatemapCached = false;
		});
	}

	static function equirectUnwrap(mesh: Dynamic) {
		Console.error(Strings.error4());
		var verts = Std.int(mesh.posa.length / 4);
		mesh.texa = new Int16Array(verts * 2);
		var n = new Vec4();
		for (i in 0...verts) {
			n.set(mesh.posa[i * 4] / 32767, mesh.posa[i * 4 + 1] / 32767, mesh.posa[i * 4 + 2] / 32767).normalize();
			// Sphere projection
			// mesh.texa[i * 2    ] = Math.atan2(n.x, n.y) / (Math.PI * 2) + 0.5;
			// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
			// Equirect
			mesh.texa[i * 2    ] = Std.int(((Math.atan2(-n.z, n.x) + Math.PI) / (Math.PI * 2)) * 32767);
			mesh.texa[i * 2 + 1] = Std.int((Math.acos(n.y) / Math.PI) * 32767);
		}
	}

	static function rawMesh(mesh: Dynamic): TMeshData {
		return {
			name: mesh.name,
			vertex_arrays: [
				{ values: mesh.posa, attrib: "pos", data: "short4norm" },
				{ values: mesh.nora, attrib: "nor", data: "short2norm" }
			],
			index_arrays: [
				{ values: mesh.inda, material: 0 }
			],
			scale_pos: mesh.scalePos,
			scale_tex: mesh.scaleTex
		};
	}
}
