package arm.io;

import haxe.io.Bytes;
import kha.Window;
import kha.Blob;
import kha.Image;
import kha.graphics4.TextureFormat;
import kha.graphics4.DepthStencilFormat;
import iron.data.MaterialData;
import iron.data.MeshData;
import iron.data.Data;
import iron.system.ArmPack;
import iron.RenderPath;
import arm.Project;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.ui.UIFiles;
import arm.sys.Path;
import arm.util.Lz4;
import arm.util.RenderUtil;
import arm.util.ViewportUtil;
import arm.util.MeshUtil;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.MaterialSlot;
import arm.nodes.MaterialParser;

class ImportArm {

	public static function runMesh(project:TProjectFormat) {
		new MeshData(project.mesh_datas[0], function(md:MeshData) {
			Context.paintObject.setData(md);
			Context.paintObject.transform.scale.set(1, 1, 1);
			Context.paintObject.transform.buildMatrix();
			Context.paintObject.name = md.name;
			Project.paintObjects = [Context.paintObject];
			iron.App.notifyOnRender(Layers.initLayers);
			History.reset();
		});
	}

	public static function runProject(path:String) {
		Data.getBlob(path, function(b:Blob) {

			Context.layersPreviewDirty = true;
			var resetLayers = false;
			Project.projectNew(resetLayers);
			Project.filepath = path;
			UIFiles.filename = new haxe.io.Path(Project.filepath).file;
			Window.get(0).title = UIFiles.filename + " - ArmorPaint";
			var project:TProjectFormat = ArmPack.decode(b.toBytes());

			// Import as mesh instead
			if (project.version == null) {
				runMesh(project);
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
				#if krom_windows
				var exists = Krom.sysCommand('IF EXIST "' + abs + '" EXIT /b 1');
				#else
				var exists = 1;
				// { test -e file && echo 1 || echo 0 }
				#end
				if (exists == 0) {
					Log.showError(Strings.error2 + abs);
					var b = Bytes.alloc(4);
					b.set(0, 255);
					b.set(1, 0);
					b.set(2, 255);
					b.set(3, 255);
					var pink = Image.fromBytes(b, 1, 1);
					Data.cachedImages.set(abs, pink);
				}
				ImportTexture.run(abs);
			}

			var m0:MaterialData = null;
			Data.getMaterial("Scene", "Material", function(m:MaterialData) {
				m0 = m;
			});

			Project.materials = [];
			for (n in project.material_nodes) {
				for (node in n.nodes) {
					if (node.type == "TEX_IMAGE") { // Convert image path from relative to absolute
						var filepath = node.buttons[0].data;
						var isAbsolute = filepath.charAt(0) == "/" || filepath.charAt(1) == ":";
						if (!isAbsolute) {
							var abs = base + filepath;
							node.buttons[0].data = abs;
						}
					}
					for (inp in node.inputs) { // Round input socket values
						if (inp.type == "VALUE") inp.default_value = Math.round(inp.default_value * 100) / 100;
					}
				}
				var mat = new MaterialSlot(m0);
				UINodes.inst.canvasMap.set(mat, n);
				Project.materials.push(mat);

				Context.material = mat;
				UINodes.inst.updateCanvasMap();
				MaterialParser.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
			}

			Project.brushes = [];
			for (n in project.brush_nodes) {
				var brush = new BrushSlot();
				UINodes.inst.canvasBrushMap.set(brush, n);
				Project.brushes.push(brush);
			}

			// Synchronous for now
			new MeshData(project.mesh_datas[0], function(md:MeshData) {
				Context.paintObject.setData(md);
				Context.paintObject.transform.scale.set(1, 1, 1);
				Context.paintObject.transform.buildMatrix();
				Context.paintObject.name = md.name;
				Project.paintObjects = [Context.paintObject];
			});

			for (i in 1...project.mesh_datas.length) {
				var raw = project.mesh_datas[i];
				new MeshData(raw, function(md:MeshData) {
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
			UITrait.inst.resHandle.position = Config.getTextureResPos(l0.res);
			if (l0.bpp == null) l0.bpp = 8; // TODO: deprecated
			var bitsPos = l0.bpp == 8 ? 0 : l0.bpp == 16 ? 1 : 2;
			UITrait.inst.bitsHandle.position = bitsPos;
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

	public static function runMaterial(path:String) {
		Data.getBlob(path, function(b:Blob) {
			var project:TProjectFormat = ArmPack.decode(b.toBytes());
			if (project.version == null) { Data.deleteBlob(path); return; }
			runMaterialFromProject(project, path);
		});
	}

	public static function runMaterialFromProject(project:TProjectFormat, path:String) {
		var base = Path.baseDir(path);
		for (file in project.assets) {
			// Convert image path from relative to absolute
			var isAbsolute = file.charAt(0) == "/" || file.charAt(1) == ":";
			var abs = isAbsolute ? file : base + file;
			#if krom_windows
			var exists = Krom.sysCommand('IF EXIST "' + abs + '" EXIT /b 1');
			#else
			var exists = 1;
			// { test -e file && echo 1 || echo 0 }
			#end
			if (exists == 0) {
				Log.showError(Strings.error2 + abs);
				var b = Bytes.alloc(4);
				b.set(0, 255);
				b.set(1, 0);
				b.set(2, 255);
				b.set(3, 255);
				var pink = Image.fromBytes(b, 1, 1);
				Data.cachedImages.set(abs, pink);
			}
			arm.io.ImportTexture.run(abs);
		}

		var m0:MaterialData = null;
		Data.getMaterial("Scene", "Material", function(m:MaterialData) {
			m0 = m;
		});

		for (n in project.material_nodes) {
			for (node in n.nodes) {
				if (node.type == "TEX_IMAGE") { // Convert image path from relative to absolute
					var filepath = node.buttons[0].data;
					var isAbsolute = filepath.charAt(0) == "/" || filepath.charAt(1) == ":";
					if (!isAbsolute) {
						var abs = base + filepath;
						node.buttons[0].data = abs;
					}
				}
				for (inp in node.inputs) { // Round input socket values
					if (inp.type == "VALUE") inp.default_value = Math.round(inp.default_value * 100) / 100;
				}
			}
			var mat = new MaterialSlot(m0);
			UINodes.inst.canvasMap.set(mat, n);
			Project.materials.push(mat);

			Context.material = mat;
			UINodes.inst.updateCanvasMap();
			MaterialParser.parsePaintMaterial();
			RenderUtil.makeMaterialPreview();
		}

		UITrait.inst.hwnd1.redraws = 2;
		Data.deleteBlob(path);
	}
}