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
import arm.sys.File;
import arm.sys.Path;
import arm.ui.UISidebar;
import arm.ui.UIFiles;
import arm.ui.UIBox;
import arm.ui.UINodes;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
import arm.data.MaterialSlot;
import arm.node.MaterialParser;
import arm.io.ImportAsset;
import arm.io.ImportArm;
import arm.io.ImportBlend;
import arm.io.ImportMesh;
import arm.io.ExportArm;
import arm.node.NodesBrush;
import arm.ProjectFormat;
import arm.Enums;

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
	public static var fonts: Array<FontSlot> = null;
	public static var paintObjects: Array<MeshObject> = null;
	public static var assetMap = new Map<Int, Dynamic>(); // kha.Image | kha.Font
	#if arm_world
	public static var waterPass = true;
	#end
	static var meshList: Array<String> = null;

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

	public static function projectOpenRecentBox() {
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), tr("Recent Projects"))) {
				for (path in Config.raw.recent_projects) {
					if (ui.button(path, Left)) {
						var current = @:privateAccess kha.graphics4.Graphics2.current;
						if (current != null) current.end();

						ImportArm.runProject(path);

						if (current != null) current.begin(false);
						UIBox.show = false;
					}
				}
				if (ui.button("Clear", Left)) {
					Config.raw.recent_projects = [];
					Config.save();
				}
			}
		}, 400, 320);
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
			if (f == "") f = tr("untitled");
			filepath = path + Path.sep + f;
			if (!filepath.endsWith(".arm")) filepath += ".arm";
			projectSave();
		});
	}

	public static function projectNewBox() {
		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), tr("New Project"))) {
				if (meshList == null) {
					meshList = File.readDirectory(Path.data() + Path.sep + "meshes");
					for (i in 0...meshList.length) meshList[i] = meshList[i].substr(0, meshList[i].length - 4); // Trim .arm
					meshList.unshift("plane");
					meshList.unshift("sphere");
					meshList.unshift("rounded_cube");

				}

				ui.row([0.5, 0.5]);
				Context.projectType = ui.combo(Id.handle({position: Context.projectType}), meshList, tr("Template"), true);
				Context.projectAspectRatio = ui.combo(Id.handle({position: Context.projectAspectRatio}), ["1:1", "2:1", "1:2"], tr("Aspect Ratio"), true);

				@:privateAccess ui.endElement();
				ui.row([0.5, 0.5]);
				if (ui.button(tr("Cancel"))) {
					UIBox.show = false;
				}
				if (ui.button(tr("OK")) || ui.isReturnDown) {
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
			if (Context.projectObjects.indexOf(m) == -1) {
				Data.deleteMesh(m.data.handle);
				m.remove();
			}
		}
		var handle = Context.paintObject.data.handle;
		if (handle != "SceneSphere" && handle != "ScenePlane") {
			Data.deleteMesh(handle);
		}

		if (Context.projectType != ModelRoundedCube) {
			var raw: TMeshData = null;
			if (Context.projectType == ModelSphere || Context.projectType == ModelTessellatedPlane) {
				var mesh: Dynamic = Context.projectType == ModelSphere ?
					new arm.format.proc.Sphere(1, 512, 256) :
					new arm.format.proc.Plane(1, 1, 512, 512);
				raw = {
					name: "Tessellated",
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
			else {
				Data.getBlob("meshes/" + meshList[Context.projectType] + ".arm", function(b: kha.Blob) {
					raw = iron.system.ArmPack.decode(b.toBytes()).mesh_datas[0];
				});
			}

			var md = new MeshData(raw, function(md: MeshData) {});
			Data.cachedMeshes.set("SceneTessellated", md);

			if (Context.projectType == ModelTessellatedPlane) {
				ViewportUtil.setView(0, 0, 5, 0, 0, 0); // Top
				ViewportUtil.orbit(0, Math.PI / 6); // Orbit down
			}
		}

		var n = Context.projectType == ModelRoundedCube ? "Cube" : "Tessellated";
		Data.getMesh("Scene", n, function(md: MeshData) {

			var current = @:privateAccess kha.graphics4.Graphics2.current;
			if (current != null) current.end();

			Context.pickerMaskHandle.position = MaskNone;
			Context.paintObject.setData(md);
			Context.paintObject.transform.scale.set(1, 1, 1);
			#if arm_creator
			if (Context.projectType == ModelTessellatedPlane) {
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
			var fontNames = App.font.getFontNames();
			fonts = [new FontSlot(fontNames.length > 0 ? fontNames[0] : "default.ttf", App.font)];
			Context.font = fonts[0];

			History.reset();

			MaterialParser.parsePaintMaterial();
			RenderUtil.makeMaterialPreview();
			for (a in assets) Data.deleteImage(a.file);
			assets = [];
			assetNames = [];
			assetId = 0;
			Context.ddirty = 4;
			UISidebar.inst.hwnd.redraws = 2;
			UISidebar.inst.hwnd1.redraws = 2;
			UISidebar.inst.hwnd2.redraws = 2;

			if (resetLayers) {
				var aspectRatioChanged = layers[0].texpaint.width != Config.getTextureResX() || layers[0].texpaint.height != Config.getTextureResY();
				while (layers.length > 0) layers.pop().unload();
				var layer = new LayerSlot();
				layers.push(layer);
				Context.setLayer(layer);
				if (aspectRatioChanged) {
					iron.App.notifyOnRender(Layers.resizeLayers);
				}
				iron.App.notifyOnRender(Layers.initLayers);
			}

			if (current != null) current.begin(false);

			Context.savedEnvmap = Context.defaultEnvmap;
			Scene.active.world.envmap = Context.emptyEnvmap;
			Scene.active.world.raw.envmap = "World_radiance.k";
			Context.showEnvmapHandle.selected = Context.showEnvmap = false;
			Scene.active.world.probe.radiance = Context.defaultRadiance;
			Scene.active.world.probe.radianceMipmaps = Context.defaultRadianceMipmaps;
			Scene.active.world.probe.irradiance = Context.defaultIrradiance;
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

	public static function importBrush() {
		UIFiles.show("arm," + Path.textureFormats.join(","), false, function(path: String) {
			// Create brush from texture
			if (Path.isTexture(path)) {
				// Import texture
				ImportAsset.run(path);
				var assetIndex = 0;
				for (i in 0...Project.assets.length) {
					if (Project.assets[i].file == path) {
						assetIndex = i;
						break;
					}
				}

				// Create a new brush
				Context.brush = new BrushSlot();
				Project.brushes.push(Context.brush);

				// Create and link image node
				var n = NodesBrush.createNode("TEX_IMAGE");
				n.x = 83;
				n.y = 340;
				n.buttons[0].default_value = assetIndex;
				var links = Context.brush.canvas.links;
				links.push({
					id: Context.brush.nodes.getLinkId(links),
					from_id: n.id,
					from_socket: 0,
					to_id: 0,
					to_socket: 4
				});

				// Parse brush
				MaterialParser.parseBrush();
				Context.parseBrushInputs();
				UINodes.inst.hwnd.redraws = 2;
				function makeBrushPreview(_) {
					RenderUtil.makeBrushPreview();
					iron.App.removeRender(makeBrushPreview);
				}
				iron.App.notifyOnRender(makeBrushPreview);
			}
			// Import from project file
			else {
				ImportArm.runBrush(path);
			}
		});
	}

	public static function importMesh() {
		UIFiles.show(Path.meshFormats.join(","), false, function(path: String) {
			importMeshBox(path);
		});
	}

	public static function importMeshBox(path: String) {

		#if krom_ios
		// Import immediately while access to resource is unlocked
		Data.getBlob(path, function(b: kha.Blob) {});
		#end

		UIBox.showCustom(function(ui: Zui) {
			if (ui.tab(Id.handle(), tr("Import Mesh"))) {

				if (path.toLowerCase().endsWith(".obj")) {
					Context.splitBy = ui.combo(Id.handle(), [
						tr("Object"),
						tr("Group"),
						tr("Material"),
						tr("UDIM Tile"),
					], tr("Split By"), true);
					if (ui.isHovered) ui.tooltip(tr("Split .obj mesh into objects"));
				}

				if (path.toLowerCase().endsWith(".fbx")) {
					Context.parseTransform = ui.check(Id.handle({selected: Context.parseTransform}), tr("Parse Transforms"));
					if (ui.isHovered) ui.tooltip(tr("Load per-object transforms from .fbx"));
				}

				if (path.toLowerCase().endsWith(".fbx") || path.toLowerCase().endsWith(".blend")) {
					Context.parseVCols = ui.check(Id.handle({selected: Context.parseVCols}), tr("Parse Vertex Colors"));
					if (ui.isHovered) ui.tooltip(tr("Import vertex color data"));
				}

				ui.row([0.5, 0.5]);
				if (ui.button(tr("Cancel"))) {
					UIBox.show = false;
				}
				if (ui.button(tr("Import")) || ui.isReturnDown) {
					UIBox.show = false;
					App.redrawUI();
					ImportMesh.run(path);
				}
			}
		});
		UIBox.clickToHide = false; // Prevent closing when going back to window from file browser
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

	public static function reimportTextures() {
		for (asset in Project.assets) {
			Data.deleteImage(asset.file);
			Data.getImage(asset.file, function(image: kha.Image) {
				Project.assetMap.set(asset.id, image);
			});
		}
	}
}
