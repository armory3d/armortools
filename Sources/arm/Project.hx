package arm;

import kha.System;
import kha.Window;
import zui.Zui;
import zui.Id;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.object.MeshObject;
import iron.Scene;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.sys.Path;
import arm.ui.UITrait;
import arm.ui.UIFiles;
import arm.ui.UIBox;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.MaterialSlot;
import arm.node.MaterialParser;
import arm.io.ImportAsset;
import arm.io.ImportArm;
import arm.io.ImportBlend;
import arm.io.ImportMesh;
import arm.io.ExportArm;
import arm.Tool;
using StringTools;

class Project {

	public static var raw: TProjectFormat;
	public static var filepath = "";
	public static var assets: Array<TAsset> = [];
	public static var assetNames: Array<String> = [];
	public static var assetId = 0;
	public static var meshAssets: Array<String> = [];
	public static var materials: Array<MaterialSlot> = null;
	public static var materialsScene: Array<MaterialSlot> = null;
	public static var brushes: Array<BrushSlot> = null;
	public static var layers: Array<LayerSlot> = null;
	public static var paintObjects: Array<MeshObject> = null;
	public static var assetMap = new Map<Int, Dynamic>(); // kha.Image | kha.Font
	#if arm_world
	public static var waterPass = true;
	#end

	public static function projectOpen() {
		UIFiles.show("arm", false, function(path: String) {
			if (!path.endsWith(".arm")) {
				Log.error(Strings.error5);
				return;
			}

			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			ImportArm.runProject(path);

			if (current != null) current.begin(false);
		});
	}

	public static function projectSave(saveAndQuit = false) {
		if (filepath == "") {
			projectSaveAs();
			return;
		}
		Window.get(0).title = UIFiles.filename + " - ArmorPaint";

		function export(_) {
			ExportArm.runProject();
			iron.App.removeRender(export);
			if (saveAndQuit) System.stop();
		}
		iron.App.notifyOnRender(export);
	}

	public static function projectSaveAs() {
		UIFiles.show("arm", true, function(path: String) {
			var f = UIFiles.filename;
			if (f == "") f = "untitled";
			filepath = path + "/" + f;
			if (!filepath.endsWith(".arm")) filepath += ".arm";
			projectSave();
		});
	}

	public static function projectNewBox() {
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), "New Project")) {
				ui.row([0.5, 0.5]);
				UITrait.inst.projectType = ui.combo(Id.handle({position: UITrait.inst.projectType}), ["Cube", "Sphere", "Tessellated Plane"], "Template");
				if (ui.button("OK") || ui.isReturnDown) {
					Project.projectNew();
					ViewportUtil.scaleToBounds();
					UIBox.show = false;
					App.redrawUI();
				}
			}
		});
	}

	public static function projectNew(resetLayers = true) {
		Window.get(0).title = "ArmorPaint";
		filepath = "";
		if (Context.mergedObject != null) {
			Context.mergedObject.remove();
			Data.deleteMesh(Context.mergedObject.data.handle);
			Context.mergedObject = null;
		}

		ViewportUtil.resetViewport();
		Context.layerPreviewDirty = true;

		Context.paintObject = Context.mainObject();

		Context.selectPaintObject(Context.mainObject());
		for (i in 1...paintObjects.length) {
			var p = paintObjects[i];
			if (p == Context.paintObject) continue;
			Data.deleteMesh(p.data.handle);
			p.remove();
		}
		var meshes = Scene.active.meshes;
		var len = meshes.length;
		for (i in 0...len) {
			var m = meshes[len - i - 1];
			if (UITrait.inst.projectObjects.indexOf(m) == -1) {
				Data.deleteMesh(m.data.handle);
				m.remove();
			}
		}
		var handle = Context.paintObject.data.handle;
		if (handle != "SceneSphere" && handle != "ScenePlane") {
			Data.deleteMesh(handle);
		}

		if (UITrait.inst.projectType != ModelCube) {
			var mesh: Dynamic = UITrait.inst.projectType == ModelSphere ?
				new arm.format.proc.Sphere(1, 512, 256) :
				new arm.format.proc.Plane(1, 1, 512, 512);
			var raw = {
				name: "Tessellated",
				vertex_arrays: [
					{ values: mesh.posa, attrib: "pos" },
					{ values: mesh.nora, attrib: "nor" },
					{ values: mesh.texa, attrib: "tex" }
				],
				index_arrays: [
					{ values: mesh.inda, material: 0 }
				],
				scale_pos: mesh.scalePos,
				scale_tex: mesh.scaleTex
			};
			var md = new MeshData(raw, function(md: MeshData) {});
			Data.cachedMeshes.set("SceneTessellated", md);

			if (UITrait.inst.projectType == ModelSphere) {
				ViewportUtil.setView(0, 0, 1, 0, 0, 0); // Top
				ViewportUtil.orbit(0, Math.PI / 6); // Orbit down
			}
			else if (UITrait.inst.projectType == ModelTessellatedPlane) {
				ViewportUtil.setView(0, 0, 5, 0, 0, 0); // Top
				ViewportUtil.orbit(0, Math.PI / 6); // Orbit down
			}
		}

		var n = UITrait.inst.projectType == ModelCube ? "Cube" : "Tessellated";
		Data.getMesh("Scene", n, function(md: MeshData) {

			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			UITrait.inst.pickerMaskHandle.position = MaskNone;
			Context.paintObject.setData(md);
			Context.paintObject.transform.scale.set(1, 1, 1);
			#if arm_creator
			if (UITrait.inst.projectType == ModelTessellatedPlane) {
				Context.paintObject.transform.loc.set(0, 0, -0.15);
				Context.paintObject.transform.scale.set(10, 10, 1);
			}
			#end
			Context.paintObject.transform.buildMatrix();
			Context.paintObject.name = n;
			paintObjects = [Context.paintObject];
			while (materials.length > 0) materials.pop().unload();
			Data.getMaterial("Scene", "Material", function(m: iron.data.MaterialData) {
				materials.push(new MaterialSlot(m));
			});
			Context.material = materials[0];
			brushes = [new BrushSlot()];
			Context.brush = brushes[0];

			History.reset();

			MaterialParser.parsePaintMaterial();
			RenderUtil.makeMaterialPreview();
			for (a in assets) Data.deleteImage(a.file);
			assets = [];
			assetNames = [];
			assetId = 0;
			Context.ddirty = 4;
			UITrait.inst.hwnd.redraws = 2;
			UITrait.inst.hwnd1.redraws = 2;
			UITrait.inst.hwnd2.redraws = 2;

			if (resetLayers) {
				while (layers.length > 0) layers.pop().unload();
				var layer = new LayerSlot();
				layers.push(layer);
				Context.setLayer(layer);
				iron.App.notifyOnRender(Layers.initLayers);
			}

			if (current != null) current.begin(false);

			UITrait.inst.savedEnvmap = UITrait.inst.defaultEnvmap;
			Scene.active.world.envmap = UITrait.inst.emptyEnvmap;
			Scene.active.world.raw.envmap = "World_radiance.k";
			UITrait.inst.showEnvmapHandle.selected = UITrait.inst.showEnvmap = false;
			Scene.active.world.probe.radiance = UITrait.inst.defaultRadiance;
			Scene.active.world.probe.radianceMipmaps = UITrait.inst.defaultRadianceMipmaps;
			Scene.active.world.probe.irradiance = UITrait.inst.defaultIrradiance;
			Scene.active.world.probe.raw.strength = 4.0;
		});
	}

	public static function importMaterial() {
		UIFiles.show("arm,blend", false, function(path: String) {
			path.endsWith(".blend") ?
				ImportBlend.runMaterial(path) :
				ImportArm.runMaterial(path);
		});
	}

	public static function importMesh() {
		UIFiles.show(Path.meshFormats.join(","), false, function(path: String) {
			importMeshBox(path);
		});
	}

	public static function importMeshBox(path: String) {
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), "Import Mesh")) {

				if (path.toLowerCase().endsWith(".obj")) {
					UITrait.inst.splitBy = ui.combo(Id.handle(), ["Object", "Group", "Material", "UDIM Tile"], "Split By", true);
					if (ui.isHovered) ui.tooltip("Split .obj mesh into objects");
				}

				if (path.toLowerCase().endsWith(".fbx")) {
					UITrait.inst.parseTransform = ui.check(Id.handle({selected: UITrait.inst.parseTransform}), "Parse Transforms");
					if (ui.isHovered) ui.tooltip("Load per-object transforms from .fbx");
				}

				ui.row([0.5, 0.5]);
				if (ui.button("Cancel")) {
					UIBox.show = false;
				}
				if (ui.button("Import") || ui.isReturnDown) {
					UIBox.show = false;
					App.redrawUI();
					ImportMesh.run(path);
				}
			}
		});
	}

	public static function reimportMesh() {
		if (Project.meshAssets != null && Project.meshAssets.length > 0) {
			ImportMesh.run(Project.meshAssets[0], false);
			Log.info("Mesh reimported.");
		}
		else importAsset();
	}

	public static function importAsset(filters: String = null) {
		if (filters == null) filters = Path.textureFormats.join(",") + "," + Path.meshFormats.join(",");
		UIFiles.show(filters, false, function(path: String) {
			ImportAsset.run(path);
		});
	}
}

typedef TProjectFormat = {
	public var version: String;
	@:optional public var brush_nodes: Array<TNodeCanvas>;
	@:optional public var material_nodes: Array<TNodeCanvas>;
	@:optional public var assets: Array<String>; // texture_assets
	@:optional public var layer_datas: Array<TLayerData>;
	@:optional public var mesh_datas: Array<TMeshData>;
	@:optional public var mesh_assets: Array<String>;
}

typedef TLayerData = {
	public var res: Int; // Width pixels
	public var bpp: Int; // Bits per pixel
	public var texpaint: haxe.io.Bytes;
	public var texpaint_nor: haxe.io.Bytes;
	public var texpaint_pack: haxe.io.Bytes;
	public var texpaint_mask: haxe.io.Bytes;
	public var uv_scale: Float;
	public var uv_rot: Float;
	public var uv_type: Int;
	public var opacity_mask: Float;
	public var material_mask: Int;
	public var object_mask: Int;
	public var blending: Int;
}

typedef TAsset = {
	public var id: Int;
	public var name: String;
	public var file: String;
}
