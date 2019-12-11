package arm.io;

import haxe.io.Bytes;
import kha.Window;
import kha.Blob;
import kha.Image;
import kha.graphics4.TextureFormat;
import kha.graphics4.DepthStencilFormat;
import zui.Nodes;
import iron.data.MaterialData;
import iron.data.MeshData;
import iron.data.Data;
import iron.data.SceneFormat;
import iron.system.ArmPack;
import iron.object.Object;
import iron.object.MeshObject;
import iron.Scene;
import iron.RenderPath;
import arm.Project;
import arm.ui.UITrait;
import arm.ui.UIFiles;
import arm.sys.Path;
import arm.sys.File;
import arm.format.Lz4;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.util.MeshUtil;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.MaterialSlot;
import arm.node.MaterialParser;
import arm.Tool;
using StringTools;

class ImportArm {

	public static function runMesh(raw: TSceneFormat) {
		Project.paintObjects = [];
		for (i in 0...raw.mesh_datas.length) {
			new MeshData(raw.mesh_datas[i], function(md: MeshData) {
				var object: MeshObject = null;
				if (i == 0) {
					Context.paintObject.setData(md);
					object = Context.paintObject;
				}
				else {
					object = Scene.active.addMeshObject(md, Context.paintObject.materials, Context.paintObject);
					object.name = md.name;
					object.skip_context = "paint";
					md.handle = md.name;
					Data.cachedMeshes.set(md.handle, md);
				}
				object.transform.scale.set(1, 1, 1);
				object.transform.buildMatrix();
				object.name = md.name;
				Project.paintObjects.push(object);
			});
		}
		iron.App.notifyOnRender(Layers.initLayers);
		History.reset();
	}

	public static function runScene(raw: TSceneFormat, path: String) {
		#if krom_windows
		path = path.replace("\\", "/");
		#end
		var _dataPath = Data.dataPath;
		Data.dataPath = path.substring(0, path.lastIndexOf("/") + 1);
		#if krom_windows
		Data.dataPath = Data.dataPath.replace("/", "\\");
		#end
		raw.name += "_imported";
		Data.cachedSceneRaws.set(raw.name, raw);
		Scene.active.addScene(raw.name, null, function(sceneObject: Object) {
			traverseObjects(sceneObject.children);
		});
		Data.dataPath = _dataPath;
	}

	static function traverseObjects(objects: Array<Object>) {
		if (objects == null) return;
		for (o in objects) {
			if (Std.is(o, MeshObject)) {
				var mo = cast(o, MeshObject);
				var count = mo.data.geom.indices.length;
				mo.materials = new haxe.ds.Vector(count);
				for (i in 0...count) {
					mo.materials[i] = Context.materialScene.data;
				}
			}
			traverseObjects(o.children);
		}
	}

	public static function runProject(path: String) {
		Data.getBlob(path, function(b: Blob) {

			Context.layersPreviewDirty = true;
			var resetLayers = false;
			Project.projectNew(resetLayers);
			path = path.replace("\\", "/");
			Project.filepath = path;
			UIFiles.filename = path.substring(path.lastIndexOf("/") + 1, path.lastIndexOf("."));
			Window.get(0).title = UIFiles.filename + " - ArmorPaint";
			var project: TProjectFormat = ArmPack.decode(b.toBytes());

			// Import as mesh instead
			if (project.version == null) {
				untyped project.objects == null ? runMesh(untyped project) : runScene(untyped project, path);
				return;
			}

			if (project.layer_datas == null) {
				runMaterialFromProject(project, Project.filepath);
				return;
			}

			Project.raw = project;
			var base = Path.baseDir(path);

			for (file in project.assets) {
				// Convert image path from relative to absolute
				var isAbsolute = file.charAt(0) == "/" || file.charAt(1) == ":";
				var abs = isAbsolute ? file : base + file;
				if (!File.exists(abs)) {
					makePink(abs);
				}
				ImportTexture.run(abs);
			}

			var m0: MaterialData = null;
			Data.getMaterial("Scene", "Material", function(m: MaterialData) {
				m0 = m;
			});

			Project.materials = [];
			for (n in project.material_nodes) {
				initNodes(n.nodes);
				Context.material = new MaterialSlot(m0, n);
				Project.materials.push(Context.material);
				MaterialParser.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
			}

			Project.brushes = [];
			for (n in project.brush_nodes) {
				Project.brushes.push(new BrushSlot(n));
			}

			// Synchronous for now
			new MeshData(project.mesh_datas[0], function(md: MeshData) {
				Context.paintObject.setData(md);
				Context.paintObject.transform.scale.set(1, 1, 1);
				Context.paintObject.transform.buildMatrix();
				Context.paintObject.name = md.name;
				Project.paintObjects = [Context.paintObject];
			});

			for (i in 1...project.mesh_datas.length) {
				var raw = project.mesh_datas[i];
				new MeshData(raw, function(md: MeshData) {
					var object = iron.Scene.active.addMeshObject(md, Context.paintObject.materials, Context.paintObject);
					object.name = md.name;
					object.skip_context = "paint";
					Project.paintObjects.push(object);
				});
			}

			if (project.mesh_assets != null && project.mesh_assets.length > 0) {
				var file = project.mesh_assets[0];
				var isAbsolute = file.charAt(0) == "/" || file.charAt(1) == ":";
				var abs = isAbsolute ? file : base + file;
				Project.meshAssets = [abs];
			}

			// No mask by default
			if (Context.mergedObject == null) MeshUtil.mergeMesh();
			Context.selectPaintObject(Context.mainObject());
			ViewportUtil.scaleToBounds();
			Context.paintObject.skip_context = "paint";
			Context.mergedObject.visible = true;

			var l0 = project.layer_datas[0];
			App.resHandle.position = Config.getTextureResPos(l0.res);
			if (l0.bpp == null) l0.bpp = 8; // TODO: deprecated
			var bitsPos = l0.bpp == 8 ? Bits8 : l0.bpp == 16 ? Bits16 : Bits32;
			App.bitsHandle.position = bitsPos;
			var bytesPerPixel = Std.int(l0.bpp / 8);
			var format = l0.bpp == 8 ? TextureFormat.RGBA32 : l0.bpp == 16 ? TextureFormat.RGBA64 : TextureFormat.RGBA128;

			if (Project.layers[0].texpaint.width != Config.getTextureRes()) {
				for (l in Project.layers) l.resizeAndSetBits();
				if (History.undoLayers != null) for (l in History.undoLayers) l.resizeAndSetBits();
				var rts = RenderPath.active.renderTargets;
				rts.get("texpaint_blend0").image.unload();
				rts.get("texpaint_blend0").raw.width = Config.getTextureRes();
				rts.get("texpaint_blend0").raw.height = Config.getTextureRes();
				rts.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), TextureFormat.L8, DepthStencilFormat.NoDepthAndStencil);
				rts.get("texpaint_blend1").image.unload();
				rts.get("texpaint_blend1").raw.width = Config.getTextureRes();
				rts.get("texpaint_blend1").raw.height = Config.getTextureRes();
				rts.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureRes(), Config.getTextureRes(), TextureFormat.L8, DepthStencilFormat.NoDepthAndStencil);
				Context.brushBlendDirty = true;
			}

			// for (l in Project.layers) l.unload();
			Project.layers = [];
			for (i in 0...project.layer_datas.length) {
				var ld = project.layer_datas[i];
				var l = new LayerSlot();
				Project.layers.push(l);

				// TODO: create render target from bytes
				var texpaint = Image.fromBytes(Lz4.decode(ld.texpaint, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
				l.texpaint.g2.begin(false);
				l.texpaint.g2.drawImage(texpaint, 0, 0);
				l.texpaint.g2.end();
				// texpaint.unload();

				var texpaint_nor = Image.fromBytes(Lz4.decode(ld.texpaint_nor, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
				l.texpaint_nor.g2.begin(false);
				l.texpaint_nor.g2.drawImage(texpaint_nor, 0, 0);
				l.texpaint_nor.g2.end();
				// texpaint_nor.unload();

				var texpaint_pack = Image.fromBytes(Lz4.decode(ld.texpaint_pack, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
				l.texpaint_pack.g2.begin(false);
				l.texpaint_pack.g2.drawImage(texpaint_pack, 0, 0);
				l.texpaint_pack.g2.end();
				// texpaint_pack.unload();

				if (ld.texpaint_mask != null) {
					l.createMask(0, false);
					var texpaint_mask = Image.fromBytes(Lz4.decode(ld.texpaint_mask, ld.res * ld.res), ld.res, ld.res, TextureFormat.L8);
					l.texpaint_mask.g2.begin(false);
					l.texpaint_mask.g2.drawImage(texpaint_mask, 0, 0);
					l.texpaint_mask.g2.end();
					// texpaint_mask.unload();
				}

				l.uvScale = ld.uv_scale;
				if (l.uvScale == null) l.uvScale = 1.0; // TODO: deprecated
				l.uvRot = ld.uv_rot;
				if (l.uvRot == null) l.uvRot = 0.0; // TODO: deprecated
				l.uvType = ld.uv_type;
				if (l.uvType == null) l.uvType = 0; // TODO: deprecated
				l.maskOpacity = ld.opacity_mask;
				l.material_mask = ld.material_mask > -1 ? Project.materials[ld.material_mask] : null;
				l.objectMask = ld.object_mask;
				l.blending = ld.blending;
				if (l.blending == null) l.blending = 0; // TODO: deprecated
			}
			Context.setLayer(Project.layers[0]);

			Context.ddirty = 4;
			UITrait.inst.hwnd.redraws = 2;
			UITrait.inst.hwnd1.redraws = 2;
			UITrait.inst.hwnd2.redraws = 2;

			Data.deleteBlob(path);
		});
	}

	public static function runMaterial(path: String) {
		Data.getBlob(path, function(b: Blob) {
			var project: TProjectFormat = ArmPack.decode(b.toBytes());
			if (project.version == null) { Data.deleteBlob(path); return; }
			runMaterialFromProject(project, path);
		});
	}

	public static function runMaterialFromProject(project: TProjectFormat, path: String) {
		var base = Path.baseDir(path);
		for (file in project.assets) {
			// Convert image path from relative to absolute
			var isAbsolute = file.charAt(0) == "/" || file.charAt(1) == ":";
			var abs = isAbsolute ? file : base + file;
			if (!File.exists(abs)) {
				makePink(abs);
			}
			arm.io.ImportTexture.run(abs);
		}

		var m0: MaterialData = null;
		Data.getMaterial("Scene", "Material", function(m: MaterialData) {
			m0 = m;
		});

		var imported: Array<MaterialSlot> = [];

		for (n in project.material_nodes) {
			initNodes(n.nodes);
			Context.material = new MaterialSlot(m0, n);
			Project.materials.push(Context.material);
			imported.push(Context.material);
		}

		function makeMaterialPreview(_) {
			for (m in imported) {
				Context.setMaterial(m);
				MaterialParser.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
			}
			iron.App.removeRender(makeMaterialPreview);
		}
		iron.App.notifyOnRender(makeMaterialPreview);

		UITrait.inst.hwnd1.redraws = 2;
		Data.deleteBlob(path);
	}

	static function makePink(abs: String) {
		Log.error(Strings.error2 + abs);
		var b = Bytes.alloc(4);
		b.set(0, 255);
		b.set(1, 0);
		b.set(2, 255);
		b.set(3, 255);
		var pink = Image.fromBytes(b, 1, 1);
		Data.cachedImages.set(abs, pink);
	}

	static function initNodes(nodes: Array<TNode>) {
		for (node in nodes) {
			if (node.type == "TEX_IMAGE") {
				// TODO: deprecated, stores filename now
				var s = node.buttons[0].data + "";
				node.buttons[0].data = s.substr(s.lastIndexOf("/") + 1);
				//
				node.buttons[0].default_value = App.getAssetIndex(node.buttons[0].data);
			}
			for (inp in node.inputs) { // Round input socket values
				if (inp.type == "VALUE") inp.default_value = Math.round(inp.default_value * 100) / 100;
			}
		}
	}
}