package arm;

import kha.Window;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MeshData;
import iron.data.Data;
import iron.Scene;
import arm.util.MeshUtil;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.util.Path;
import arm.util.Lz4;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.ui.UIFiles;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.MaterialSlot;
import arm.nodes.MaterialParser;
import arm.io.ImportArm;
using StringTools;

class Project {
	public static function projectOpen() {
		App.showFiles = true;
		App.whandle.redraws = 2;
		App.foldersOnly = false;
		App.showFilename = false;
		UIFiles.filters = "arm";
		App.filesDone = function(path:String) {
			if (!path.endsWith(".arm")) {
				UITrait.inst.showError(Strings.error5);
				return;
			}

			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			ImportArm.runProject(path);

			if (current != null) current.begin(false);
		};
	}

	public static function projectSave() {
		if (UITrait.inst.projectPath == "") {
			projectSaveAs();
			return;
		}
		Window.get(0).title = App.filenameHandle.text + " - ArmorPaint";
		UITrait.inst.projectExport = true;
	}

	public static function projectSaveAs() {
		App.showFiles = true;
		App.whandle.redraws = 2;
		App.foldersOnly = true;
		App.showFilename = true;
		UIFiles.filters = "arm";
		App.filesDone = function(path:String) {
			var f = App.filenameHandle.text;
			if (f == "") f = "untitled";
			UITrait.inst.projectPath = path + "/" + f;
			if (!UITrait.inst.projectPath.endsWith(".arm")) UITrait.inst.projectPath += ".arm";
			projectSave();
		};
	}

	public static function projectNew(resetLayers = true) {
		Window.get(0).title = "ArmorPaint";
		UITrait.inst.projectPath = "";
		if (UITrait.inst.mergedObject != null) {
			UITrait.inst.mergedObject.remove();
			Data.deleteMesh(UITrait.inst.mergedObject.data.handle);
			UITrait.inst.mergedObject = null;
		}

		ViewportUtil.resetViewport();
		UITrait.inst.layerPreviewDirty = true;
		LayerSlot.counter = 0;

		UITrait.inst.paintObject = UITrait.inst.mainObject();

		UITrait.inst.selectPaintObject(UITrait.inst.mainObject());
		for (i in 1...UITrait.inst.paintObjects.length) {
			var p = UITrait.inst.paintObjects[i];
			if (p == UITrait.inst.paintObject) continue;
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
		var handle = UITrait.inst.paintObject.data.handle;
		if (handle != "SceneSphere" && handle != "ScenePlane") {
			Data.deleteMesh(handle);
		}

		if (UITrait.inst.projectType > 0) {
			var mesh:Dynamic = UITrait.inst.projectType == 1 ?
				new iron.format.proc.Sphere(1, 512, 256) :
				new iron.format.proc.Plane(1, 1, 512, 512);
			var raw = {
				name: "Tesselated",
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
			var md = new MeshData(raw, function(md:MeshData) {});
			Data.cachedMeshes.set("SceneTesselated", md);

			if (UITrait.inst.projectType == 2) {
				ViewportUtil.setView(0, 0, 1, 0, 0, 0); // Top
				ViewportUtil.orbit(0, Math.PI / 6); // Orbit down
			}
		}

		var n = UITrait.inst.projectType == 0 ? "Cube" : "Tesselated";
		Data.getMesh("Scene", n, function(md:MeshData) {
			
			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			UITrait.inst.pickerMaskHandle.position = 0;
			UITrait.inst.paintObject.setData(md);
			UITrait.inst.paintObject.transform.scale.set(1, 1, 1);
			UITrait.inst.paintObject.transform.buildMatrix();
			UITrait.inst.paintObject.name = n;
			UITrait.inst.paintObjects = [UITrait.inst.paintObject];
			Data.getMaterial("Scene", "Material", function(m:iron.data.MaterialData) {
				UITrait.inst.materials = [new MaterialSlot(m)];
			});
			UITrait.inst.selectedMaterial = UITrait.inst.materials[0];
			UINodes.inst.canvasMap = new Map();
			UINodes.inst.canvasBrushMap = new Map();
			UITrait.inst.brushes = [new BrushSlot()];
			UITrait.inst.selectedBrush = UITrait.inst.brushes[0];

			History.reset();
			
			UINodes.inst.updateCanvasMap();
			MaterialParser.parsePaintMaterial();
			RenderUtil.makeMaterialPreview();
			for (a in UITrait.inst.assets) Data.deleteImage(a.file);
			UITrait.inst.assets = [];
			UITrait.inst.assetNames = [];
			UITrait.inst.assetId = 0;
			UITrait.inst.ddirty = 4;
			UITrait.inst.hwnd.redraws = 2;
			UITrait.inst.hwnd1.redraws = 2;
			UITrait.inst.hwnd2.redraws = 2;

			if (resetLayers) {
				// for (l in layers) l.unload();
				var layer = new LayerSlot();
				UITrait.inst.layers = [layer];
				UITrait.inst.setLayer(layer);
				if (UITrait.inst.projectType == 1) {
					layer.material_mask = UITrait.inst.materials[0];
					Layers.updateFillLayers(4);
				}
				else {
					iron.App.notifyOnRender(Layers.initLayers);
				}
			}

			if (current != null) current.begin(false);
		});
	}
}

typedef TProjectFormat = {
	public var version:String;
	public var brush_nodes:Array<TNodeCanvas>;
	public var material_nodes:Array<TNodeCanvas>;
	public var assets:Array<String>;
	public var layer_datas:Array<TLayerData>;
	public var mesh_datas:Array<TMeshData>;
}

typedef TLayerData = {
	public var res:Int;
	public var texpaint:haxe.io.Bytes;
	public var texpaint_nor:haxe.io.Bytes;
	public var texpaint_pack:haxe.io.Bytes;
	public var texpaint_mask:haxe.io.Bytes;
	public var opacity_mask:Float;
	public var material_mask:Int;
	public var object_mask:Int;
}
