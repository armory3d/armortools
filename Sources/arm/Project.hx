package arm;

import iron.data.SceneFormat;
import iron.data.MeshData;
import arm.ProjectFormat;
import arm.util.*;
import arm.ui.*;

class Project {
	public static function projectOpen() {
		arm.App.showFiles = true;
		@:privateAccess zui.Ext.lastPath = ""; // Refresh
		arm.App.whandle.redraws = 2;
		arm.App.foldersOnly = false;
		arm.App.showFilename = false;
		arm.App.filesDone = function(path:String) {
			if (!StringTools.endsWith(path, ".arm")) {
				UITrait.inst.showError("Error: .arm file expected");
				return;
			}

			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			importProject(path);

			if (current != null) current.begin(false);
		};
	}

	public static function exportProject() {
		var mnodes:Array<zui.Nodes.TNodeCanvas> = [];
		var bnodes:Array<zui.Nodes.TNodeCanvas> = [];

		for (m in UITrait.inst.materials) mnodes.push(UINodes.inst.canvasMap.get(m));
		for (b in UITrait.inst.brushes) bnodes.push(UINodes.inst.canvasBrushMap.get(b));

		var md:Array<TMeshData> = [];
		for (p in UITrait.inst.paintObjects) md.push(p.data.raw);

		var asset_files:Array<String> = [];
		for (a in UITrait.inst.assets) asset_files.push(a.file);

		var ld:Array<TLayerData> = [];
		for (l in UITrait.inst.layers) {
			ld.push({
				res: l.texpaint.width,
				texpaint: l.texpaint.getPixels(),
				texpaint_nor: l.texpaint_nor.getPixels(),
				texpaint_pack: l.texpaint_pack.getPixels(),
				texpaint_opt: l.texpaint_opt != null ? l.texpaint_opt.getPixels() : null,
			});
		}

		UITrait.inst.project = {
			version: UITrait.inst.version,
			material_nodes: mnodes,
			brush_nodes: bnodes,
			mesh_datas: md,
			layer_datas: ld,
			assets: asset_files
		};
		
		var bytes = iron.system.ArmPack.encode(UITrait.inst.project);

		#if kha_krom
		Krom.fileSaveBytes(UITrait.inst.projectPath, bytes.getData());
		#elseif kha_kore
		sys.io.File.saveBytes(UITrait.inst.projectPath, bytes);
		#end
	}

	public static function projectSave() {
		if (UITrait.inst.projectPath == "") {
			projectSaveAs();
			return;
		}
		kha.Window.get(0).title = arm.App.filenameHandle.text + " - ArmorPaint";
		UITrait.inst.projectExport = true;
	}

	public static function projectSaveAs() {
		arm.App.showFiles = true;
		@:privateAccess zui.Ext.lastPath = ""; // Refresh
		arm.App.whandle.redraws = 2;
		arm.App.foldersOnly = true;
		arm.App.showFilename = true;
		arm.App.filesDone = function(path:String) {
			var f = arm.App.filenameHandle.text;
			if (f == "") f = "untitled";
			UITrait.inst.projectPath = path + "/" + f;
			if (!StringTools.endsWith(UITrait.inst.projectPath, ".arm")) UITrait.inst.projectPath += ".arm";
			projectSave();
		};
	}

	public static function projectNew(resetLayers = true) {
		kha.Window.get(0).title = "ArmorPaint";
		UITrait.inst.projectPath = "";
		if (UITrait.inst.mergedObject != null) {
			UITrait.inst.mergedObject.remove();
			iron.data.Data.deleteMesh(UITrait.inst.mergedObject.data.handle);
			UITrait.inst.mergedObject = null;
		}

		UITrait.inst.paintObject = UITrait.inst.mainObject();

		UITrait.inst.selectPaintObject(UITrait.inst.mainObject());
		for (i in 1...UITrait.inst.paintObjects.length) {
			var p = UITrait.inst.paintObjects[i];
			if (p == UITrait.inst.paintObject) continue;
			iron.data.Data.deleteMesh(p.data.handle);
			p.remove();
		}
		var n = UITrait.inst.newObjectNames[UITrait.inst.newObject];
		iron.data.Data.deleteMesh(UITrait.inst.paintObject.data.handle);
		iron.data.Data.getMesh("mesh_" + n, n, function(md:MeshData) {
			
			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			UITrait.inst.autoFillHandle.selected = false;
			UITrait.inst.paintObject.setData(md);
			UITrait.inst.paintObject.transform.scale.set(1, 1, 1);
			UITrait.inst.paintObject.transform.buildMatrix();
			UITrait.inst.paintObject.name = n;
			UITrait.inst.paintObjects = [UITrait.inst.paintObject];
			UITrait.inst.maskHandle.position = 0;
			UITrait.inst.materials = [new MaterialSlot()];
			UITrait.inst.selectedMaterial = UITrait.inst.materials[0];
			UINodes.inst.canvasMap = new Map();
			UINodes.inst.canvasBrushMap = new Map();
			UITrait.inst.brushes = [new BrushSlot()];
			UITrait.inst.selectedBrush = UITrait.inst.brushes[0];
			
			if (resetLayers) {
				// for (l in layers) l.unload();
				UITrait.inst.layers = [new LayerSlot()];
				UITrait.inst.selectedLayer = UITrait.inst.layers[0];
				iron.App.notifyOnRender(Layers.initLayers);
				if (UITrait.inst.paintHeight) iron.App.notifyOnRender(Layers.initHeightLayer);
			}
			
			UINodes.inst.updateCanvasMap();
			UINodes.inst.parsePaintMaterial();
			RenderUtil.makeMaterialPreview();
			UITrait.inst.assets = [];
			UITrait.inst.assetNames = [];
			UITrait.inst.assetId = 0;
			ViewportUtil.resetViewport();

			if (current != null) current.begin(false);
		});
	}

	public static function importProject(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var resetLayers = false;
			projectNew(resetLayers);
			UITrait.inst.projectPath = path;
			arm.App.filenameHandle.text = new haxe.io.Path(UITrait.inst.projectPath).file;

			kha.Window.get(0).title = arm.App.filenameHandle.text + " - ArmorPaint";

			UITrait.inst.project = iron.system.ArmPack.decode(b.toBytes());
			var project = UITrait.inst.project;

			for (file in project.assets) Importer.importTexture(file);

			UITrait.inst.materials = [];
			for (n in project.material_nodes) {
				var mat = new MaterialSlot();
				UINodes.inst.canvasMap.set(mat, n);
				UITrait.inst.materials.push(mat);

				UITrait.inst.selectedMaterial = mat;
				UINodes.inst.updateCanvasMap();
				UINodes.inst.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
			}

			UITrait.inst.brushes = [];
			for (n in project.brush_nodes) {
				var brush = new BrushSlot();
				UINodes.inst.canvasBrushMap.set(brush, n);
				UITrait.inst.brushes.push(brush);
			}

			// Synchronous for now
			new MeshData(project.mesh_datas[0], function(md:MeshData) {
				UITrait.inst.paintObject.setData(md);
				UITrait.inst.paintObject.transform.scale.set(1, 1, 1);
				UITrait.inst.paintObject.transform.buildMatrix();
				UITrait.inst.paintObject.name = md.name;
				UITrait.inst.paintObjects = [UITrait.inst.paintObject];
			});

			for (i in 1...project.mesh_datas.length) {
				var raw = project.mesh_datas[i];  
				new MeshData(raw, function(md:MeshData) {
					var object = iron.Scene.active.addMeshObject(md, UITrait.inst.paintObject.materials, UITrait.inst.paintObject);
					object.name = md.name;
					object.skip_context = "paint";
					UITrait.inst.paintObjects.push(object);					
				});
			}

			// No mask by default
			if (UITrait.inst.mergedObject == null) MeshUtil.mergeMesh();
			UITrait.inst.selectPaintObject(UITrait.inst.mainObject());
			ViewportUtil.scaleToBounds();
			UITrait.inst.paintObject.skip_context = "paint";
			UITrait.inst.mergedObject.visible = true;

			UITrait.inst.resHandle.position = Config.getTextureResPos(project.layer_datas[0].res);

			if (UITrait.inst.undoLayers[0].texpaint.width != Config.getTextureRes()) {
				for (l in UITrait.inst.undoLayers) Layers.resizeLayer(l); // TODO
				for (l in UITrait.inst.layers) Layers.resizeLayer(l);
			}

			// for (l in UITrait.inst.layers) l.unload();
			UITrait.inst.layers = [];
			for (i in 0...project.layer_datas.length) {
				var ld = project.layer_datas[i];
				var l = new LayerSlot();
				UITrait.inst.layers.push(l);

				// TODO: create render target from bytes
				var texpaint = kha.Image.fromBytes(ld.texpaint, ld.res, ld.res);
				l.texpaint.g2.begin(false);
				l.texpaint.g2.drawImage(texpaint, 0, 0);
				l.texpaint.g2.end();
				// texpaint.unload();

				var texpaint_nor = kha.Image.fromBytes(ld.texpaint_nor, ld.res, ld.res);
				l.texpaint_nor.g2.begin(false);
				l.texpaint_nor.g2.drawImage(texpaint_nor, 0, 0);
				l.texpaint_nor.g2.end();
				// texpaint_nor.unload();

				var texpaint_pack = kha.Image.fromBytes(ld.texpaint_pack, ld.res, ld.res);
				l.texpaint_pack.g2.begin(false);
				l.texpaint_pack.g2.drawImage(texpaint_pack, 0, 0);
				l.texpaint_pack.g2.end();
				// texpaint_pack.unload();

				if (ld.texpaint_opt != null) {
					var texpaint_opt = kha.Image.fromBytes(ld.texpaint_opt, ld.res, ld.res);
					l.texpaint_opt.g2.begin(false);
					l.texpaint_opt.g2.drawImage(texpaint_opt, 0, 0);
					l.texpaint_opt.g2.end();
					// texpaint_opt.unload();
				}
			}
			UITrait.inst.selectedLayer = UITrait.inst.layers[0];

			if (UITrait.inst.layers.length > 0) UINodes.inst.parseMeshMaterial();

			UITrait.inst.ddirty = 4;
			UITrait.inst.hwnd.redraws = 2;
		});
	}
}
