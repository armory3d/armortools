package arm.io;

import haxe.io.Bytes;
import kha.Window;
import kha.Blob;
import kha.Image;
import zui.Nodes;
import iron.data.MeshData;
import iron.data.Data;
import iron.data.SceneFormat;
import iron.math.Mat4;
import iron.system.ArmPack;
import iron.Scene;
import arm.ProjectFormat;
import arm.ui.UIFiles;
import arm.ui.UIBase;
import arm.sys.Path;
import arm.sys.File;
import arm.Viewport;
#if (is_paint || is_sculpt)
import kha.graphics4.TextureFormat;
import kha.graphics4.DepthStencilFormat;
import iron.data.MaterialData;
import iron.system.Lz4;
import iron.object.MeshObject;
import iron.RenderPath;
import arm.util.RenderUtil;
import arm.ui.UIStatus;
import arm.util.MeshUtil;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.MaterialSlot;
import arm.shader.MakeMaterial;
#end

class ImportArm {

	public static function runProject(path: String) {
		Data.getBlob(path, function(b: Blob) {
			var project: TProjectFormat = ArmPack.decode(b.toBytes());

			#if (is_paint || is_sculpt)
			if (project.version != null && project.layer_datas == null) {
				// Import as material
				if (project.material_nodes != null) {
					runMaterialFromProject(project, path);
				}
				// Import as brush
				else if (project.brush_nodes != null) {
					runBrushFromProject(project, path);
				}
				// Import as swatches
				else if (project.swatches != null) {
					runSwatchesFromProject(project, path);
				}
				return;
			}

			var importAsMesh = project.version == null;
			Context.raw.layersPreviewDirty = true;
			Context.raw.layerFilter = 0;
			#end

			#if is_lab
			var importAsMesh = true;
			#end

			Project.projectNew(importAsMesh);
			Project.filepath = path;
			UIFiles.filename = path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf("."));
			#if (krom_android || krom_ios)
			Window.get(0).title = UIFiles.filename;
			#else
			Window.get(0).title = UIFiles.filename + " - " + Manifest.title;
			#end

			#if (is_paint || is_sculpt)
			// Import as mesh instead
			if (importAsMesh) {
				runMesh(untyped project);
				return;
			}
			#end

			// Save to recent
			#if krom_ios
			var recent_path = path.substr(path.lastIndexOf("/") + 1);
			#else
			var recent_path = path;
			#end
			var recent = Config.raw.recent_projects;
			recent.remove(recent_path);
			recent.unshift(recent_path);
			Config.save();

			Project.raw = project;

			#if (is_paint || is_sculpt)
			var l0 = project.layer_datas[0];
			App.resHandle.position = Config.getTextureResPos(l0.res);
			var bitsPos = l0.bpp == 8 ? Bits8 : l0.bpp == 16 ? Bits16 : Bits32;
			App.bitsHandle.position = bitsPos;
			var bytesPerPixel = Std.int(l0.bpp / 8);
			var format = l0.bpp == 8 ? TextureFormat.RGBA32 : l0.bpp == 16 ? TextureFormat.RGBA64 : TextureFormat.RGBA128;
			#end

			var base = Path.baseDir(path);
			if (Project.raw.envmap != null) {
				Project.raw.envmap = Data.isAbsolute(Project.raw.envmap) ? Project.raw.envmap : base + Project.raw.envmap;
			}
			if (Project.raw.envmap_strength != null) {
				iron.Scene.active.world.probe.raw.strength = Project.raw.envmap_strength;
			}
			if (Project.raw.camera_world != null) {
				iron.Scene.active.camera.transform.local = Mat4.fromFloat32Array(Project.raw.camera_world);
				iron.Scene.active.camera.transform.decompose();
				iron.Scene.active.camera.data.raw.fov = Project.raw.camera_fov;
				iron.Scene.active.camera.buildProjection();
				var origin = Project.raw.camera_origin;
				arm.Camera.inst.origins[0].x = origin[0];
				arm.Camera.inst.origins[0].y = origin[1];
				arm.Camera.inst.origins[0].z = origin[2];
			}

			for (file in project.assets) {
				#if krom_windows
				file = file.replace("/", "\\");
				#else
				file = file.replace("\\", "/");
				#end
				// Convert image path from relative to absolute
				var abs = Data.isAbsolute(file) ? file : base + file;
				if (project.packed_assets != null) {
					abs = Path.normalize(abs);
					unpackAsset(project, abs, file);
				}
				if (Data.cachedImages.get(abs) == null && !File.exists(abs)) {
					makePink(abs);
				}
				var hdrAsEnvmap = abs.endsWith(".hdr") && Project.raw.envmap == abs;
				ImportTexture.run(abs, hdrAsEnvmap);
			}

			#if (is_paint || is_sculpt)
			if (project.font_assets != null) {
				for (file in project.font_assets) {
					#if krom_windows
					file = file.replace("/", "\\");
					#else
					file = file.replace("\\", "/");
					#end
					// Convert font path from relative to absolute
					var abs = Data.isAbsolute(file) ? file : base + file;
					if (File.exists(abs)) {
						ImportFont.run(abs);
					}
				}
			}
			#end

			// Synchronous for now
			#if (is_paint || is_sculpt)
			new MeshData(project.mesh_datas[0], function(md: MeshData) {
			#end

			#if is_lab
			new MeshData(project.mesh_data, function(md: MeshData) {
			#end

				Context.raw.paintObject.setData(md);
				Context.raw.paintObject.transform.scale.set(1, 1, 1);
				Context.raw.paintObject.transform.buildMatrix();
				Context.raw.paintObject.name = md.name;
				Project.paintObjects = [Context.raw.paintObject];
			});

			#if (is_paint || is_sculpt)
			for (i in 1...project.mesh_datas.length) {
				var raw = project.mesh_datas[i];
				new MeshData(raw, function(md: MeshData) {
					var object = iron.Scene.active.addMeshObject(md, Context.raw.paintObject.materials, Context.raw.paintObject);
					object.name = md.name;
					object.skip_context = "paint";
					Project.paintObjects.push(object);
				});
			}

			if (project.mesh_assets != null && project.mesh_assets.length > 0) {
				var file = project.mesh_assets[0];
				var abs = Data.isAbsolute(file) ? file : base + file;
				Project.meshAssets = [abs];
			}

			#if is_paint
			if (project.atlas_objects != null) Project.atlasObjects = project.atlas_objects;
			if (project.atlas_names != null) Project.atlasNames = project.atlas_names;
			#end

			// No mask by default
			if (Context.raw.mergedObject == null) MeshUtil.mergeMesh();
			#end

			Context.selectPaintObject(Context.mainObject());
			Viewport.scaleToBounds();
			Context.raw.paintObject.skip_context = "paint";
			Context.raw.mergedObject.visible = true;

			#if (is_paint || is_sculpt)
			var tex = Project.layers[0].texpaint;
			if (tex.width != Config.getTextureResX() || tex.height != Config.getTextureResY()) {
				if (History.undoLayers != null) for (l in History.undoLayers) l.resizeAndSetBits();
				var rts = RenderPath.active.renderTargets;
				var _texpaint_blend0 = rts.get("texpaint_blend0").image;
				App.notifyOnNextFrame(function() {
					_texpaint_blend0.unload();
				});
				rts.get("texpaint_blend0").raw.width = Config.getTextureResX();
				rts.get("texpaint_blend0").raw.height = Config.getTextureResY();
				rts.get("texpaint_blend0").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8, DepthStencilFormat.NoDepthAndStencil);
				var _texpaint_blend1 = rts.get("texpaint_blend1").image;
				App.notifyOnNextFrame(function() {
					_texpaint_blend1.unload();
				});
				rts.get("texpaint_blend1").raw.width = Config.getTextureResX();
				rts.get("texpaint_blend1").raw.height = Config.getTextureResY();
				rts.get("texpaint_blend1").image = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.L8, DepthStencilFormat.NoDepthAndStencil);
				Context.raw.brushBlendDirty = true;
			}

			for (l in Project.layers) l.unload();
			Project.layers = [];
			for (i in 0...project.layer_datas.length) {
				var ld = project.layer_datas[i];
				var isGroup = ld.texpaint == null;

				#if is_paint
				var isMask = ld.texpaint != null && ld.texpaint_nor == null;
				#end
				#if is_sculpt
				var isMask = false;
				#end

				var l = new LayerSlot("", isGroup ? SlotGroup : isMask ? SlotMask : SlotLayer);
				if (ld.name != null) l.name = ld.name;
				l.visible = ld.visible;
				Project.layers.push(l);

				if (!isGroup) {
					if (App.pipeMerge == null) App.makePipe();

					var _texpaint: kha.Image = null;

					#if is_paint
					var _texpaint_nor: kha.Image = null;
					var _texpaint_pack: kha.Image = null;
					#end

					if (isMask) {
						_texpaint = Image.fromBytes(Lz4.decode(ld.texpaint, ld.res * ld.res * 4), ld.res, ld.res, TextureFormat.RGBA32);
						l.texpaint.g2.begin(false);
						// l.texpaint.g2.pipeline = App.pipeCopy8;
						l.texpaint.g2.pipeline = project.is_bgra ? App.pipeCopyBGRA : App.pipeCopy; // Full bits for undo support, R8 is used
						l.texpaint.g2.drawImage(_texpaint, 0, 0);
						l.texpaint.g2.pipeline = null;
						l.texpaint.g2.end();
					}
					else { // Layer
						// TODO: create render target from bytes
						_texpaint = Image.fromBytes(Lz4.decode(ld.texpaint, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
						l.texpaint.g2.begin(false);
						l.texpaint.g2.pipeline = project.is_bgra ? App.pipeCopyBGRA : App.pipeCopy;
						l.texpaint.g2.drawImage(_texpaint, 0, 0);
						l.texpaint.g2.pipeline = null;
						l.texpaint.g2.end();

						#if is_paint
						_texpaint_nor = Image.fromBytes(Lz4.decode(ld.texpaint_nor, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
						l.texpaint_nor.g2.begin(false);
						l.texpaint_nor.g2.pipeline = project.is_bgra ? App.pipeCopyBGRA : App.pipeCopy;
						l.texpaint_nor.g2.drawImage(_texpaint_nor, 0, 0);
						l.texpaint_nor.g2.pipeline = null;
						l.texpaint_nor.g2.end();

						_texpaint_pack = Image.fromBytes(Lz4.decode(ld.texpaint_pack, ld.res * ld.res * 4 * bytesPerPixel), ld.res, ld.res, format);
						l.texpaint_pack.g2.begin(false);
						l.texpaint_pack.g2.pipeline = project.is_bgra ? App.pipeCopyBGRA : App.pipeCopy;
						l.texpaint_pack.g2.drawImage(_texpaint_pack, 0, 0);
						l.texpaint_pack.g2.pipeline = null;
						l.texpaint_pack.g2.end();
						#end
					}

					l.scale = ld.uv_scale;
					l.angle = ld.uv_rot;
					l.uvType = ld.uv_type;
					if (ld.decal_mat != null) l.decalMat = Mat4.fromFloat32Array(ld.decal_mat);
					l.maskOpacity = ld.opacity_mask;
					l.objectMask = ld.object_mask;
					l.blending = ld.blending;

					#if is_paint
					l.paintBase = ld.paint_base;
					l.paintOpac = ld.paint_opac;
					l.paintOcc = ld.paint_occ;
					l.paintRough = ld.paint_rough;
					l.paintMet = ld.paint_met;
					l.paintNor = ld.paint_nor;
					l.paintNorBlend = ld.paint_nor_blend != null ? ld.paint_nor_blend : true; // TODO: deprecated
					l.paintHeight = ld.paint_height;
					l.paintHeightBlend = ld.paint_height_blend != null ? ld.paint_height_blend : true; // TODO: deprecated
					l.paintEmis = ld.paint_emis;
					l.paintSubs = ld.paint_subs;
					#end

					App.notifyOnNextFrame(function() {
						_texpaint.unload();
						#if is_paint
						if (_texpaint_nor != null) _texpaint_nor.unload();
						if (_texpaint_pack != null) _texpaint_pack.unload();
						#end
					});
				}
			}

			// Assign parents to groups and masks
			for (i in 0...project.layer_datas.length) {
				var ld = project.layer_datas[i];
				if (ld.parent >= 0) {
					Project.layers[i].parent = Project.layers[ld.parent];
				}
			}

			Context.setLayer(Project.layers[0]);

			// Materials
			var m0: MaterialData = null;
			Data.getMaterial("Scene", "Material", function(m: MaterialData) {
				m0 = m;
			});

			Project.materials = [];
			for (n in project.material_nodes) {
				initNodes(n.nodes);
				Context.raw.material = new MaterialSlot(m0, n);
				Project.materials.push(Context.raw.material);
			}
			#end

			arm.ui.UINodes.inst.hwnd.redraws = 2;
			arm.ui.UINodes.inst.groupStack = [];
			Project.materialGroups = [];
			if (project.material_groups != null) {
				for (g in project.material_groups) Project.materialGroups.push({ canvas: g, nodes: new Nodes() });
			}

			#if (is_paint || is_sculpt)
			for (m in Project.materials) {
				Context.raw.material = m;
				MakeMaterial.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
			}

			Project.brushes = [];
			for (n in project.brush_nodes) {
				initNodes(n.nodes);
				Context.raw.brush = new BrushSlot(n);
				Project.brushes.push(Context.raw.brush);
				MakeMaterial.parseBrush();
				RenderUtil.makeBrushPreview();
			}

			// Fill layers
			for (i in 0...project.layer_datas.length) {
				var ld = project.layer_datas[i];
				var l = Project.layers[i];
				var isGroup = ld.texpaint == null;
				if (!isGroup) {
					l.fill_layer = ld.fill_layer > -1 ? Project.materials[ld.fill_layer] : null;
				}
			}

			UIBase.inst.hwnds[TabSidebar0].redraws = 2;
			UIBase.inst.hwnds[TabSidebar1].redraws = 2;
			#end

			#if is_lab
			initNodes(project.material.nodes);
			Project.canvas = project.material;
			arm.logic.LogicParser.parse(Project.canvas, false);
			#end

			Context.raw.ddirty = 4;
			Data.deleteBlob(path);
		});
	}

	#if (is_paint || is_sculpt)
	public static function runMesh(raw: TSceneFormat) {
		Project.paintObjects = [];
		for (i in 0...raw.mesh_datas.length) {
			new MeshData(raw.mesh_datas[i], function(md: MeshData) {
				var object: MeshObject = null;
				if (i == 0) {
					Context.raw.paintObject.setData(md);
					object = Context.raw.paintObject;
				}
				else {
					object = Scene.active.addMeshObject(md, Context.raw.paintObject.materials, Context.raw.paintObject);
					object.name = md.name;
					object.skip_context = "paint";
					md.handle = md.name;
					Data.cachedMeshes.set(md.handle, md);
				}
				object.transform.scale.set(1, 1, 1);
				object.transform.buildMatrix();
				object.name = md.name;
				Project.paintObjects.push(object);
				MeshUtil.mergeMesh();
				Viewport.scaleToBounds();
			});
		}
		iron.App.notifyOnInit(App.initLayers);
		History.reset();
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
			#if krom_windows
			file = file.replace("/", "\\");
			#else
			file = file.replace("\\", "/");
			#end
			// Convert image path from relative to absolute
			var abs = Data.isAbsolute(file) ? file : base + file;
			if (project.packed_assets != null) {
				unpackAsset(project, abs, file);
			}
			if (Data.cachedImages.get(abs) == null && !File.exists(abs)) {
				makePink(abs);
			}
			arm.io.ImportTexture.run(abs);
		}

		var m0: MaterialData = null;
		Data.getMaterial("Scene", "Material", function(m: MaterialData) {
			m0 = m;
		});

		var imported: Array<MaterialSlot> = [];

		for (c in project.material_nodes) {
			initNodes(c.nodes);
			Context.raw.material = new MaterialSlot(m0, c);
			Project.materials.push(Context.raw.material);
			imported.push(Context.raw.material);
			History.newMaterial();
		}

		if (project.material_groups != null) {
			for (c in project.material_groups) {
				while (groupExists(c)) renameGroup(c.name, imported, project.material_groups); // Ensure unique group name
				initNodes(c.nodes);
				Project.materialGroups.push({ canvas: c, nodes: new Nodes() });
			}
		}

		function _init() {
			for (m in imported) {
				Context.setMaterial(m);
				MakeMaterial.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
			}
		}
		iron.App.notifyOnInit(_init);

		arm.ui.UINodes.inst.groupStack = [];
		UIBase.inst.hwnds[TabSidebar1].redraws = 2;
		Data.deleteBlob(path);
	}

	static function groupExists(c: TNodeCanvas): Bool {
		for (g in Project.materialGroups) {
			if (g.canvas.name == c.name) return true;
		}
		return false;
	}

	static function renameGroup(name: String, materials: Array<MaterialSlot>, groups: Array<TNodeCanvas>) {
		for (m in materials) {
			for (n in m.canvas.nodes) {
				if (n.type == "GROUP" && n.name == name) n.name += ".1";
			}
		}
		for (c in groups) {
			if (c.name == name) c.name += ".1";
			for (n in c.nodes) {
				if (n.type == "GROUP" && n.name == name) n.name += ".1";
			}
		}
	}

	public static function runBrush(path: String) {
		Data.getBlob(path, function(b: Blob) {
			var project: TProjectFormat = ArmPack.decode(b.toBytes());
			if (project.version == null) { Data.deleteBlob(path); return; }
			runBrushFromProject(project, path);
		});
	}

	public static function runBrushFromProject(project: TProjectFormat, path: String) {
		var base = Path.baseDir(path);
		for (file in project.assets) {
			#if krom_windows
			file = file.replace("/", "\\");
			#else
			file = file.replace("\\", "/");
			#end
			// Convert image path from relative to absolute
			var abs = Data.isAbsolute(file) ? file : base + file;
			if (project.packed_assets != null) {
				unpackAsset(project, abs, file);
			}
			if (Data.cachedImages.get(abs) == null && !File.exists(abs)) {
				makePink(abs);
			}
			arm.io.ImportTexture.run(abs);
		}

		var imported: Array<BrushSlot> = [];

		for (n in project.brush_nodes) {
			initNodes(n.nodes);
			Context.raw.brush = new BrushSlot(n);
			Project.brushes.push(Context.raw.brush);
			imported.push(Context.raw.brush);
		}

		function _init() {
			for (b in imported) {
				Context.setBrush(b);
				RenderUtil.makeBrushPreview();
			}
		}
		iron.App.notifyOnInit(_init);

		UIBase.inst.hwnds[TabSidebar1].redraws = 2;
		Data.deleteBlob(path);
	}
	#end

	public static function runSwatches(path: String, replaceExisting = false) {
		Data.getBlob(path, function(b: Blob) {
			var project: TProjectFormat = ArmPack.decode(b.toBytes());
			if (project.version == null) { Data.deleteBlob(path); return; }
			runSwatchesFromProject(project, path, replaceExisting);
		});
	}

	public static function runSwatchesFromProject(project: TProjectFormat, path: String, replaceExisting = false) {
		if (replaceExisting) {
			Project.raw.swatches = [];

			if (project.swatches == null) { // No swatches contained
				Project.raw.swatches.push(Project.makeSwatch());
			}
		}

		if (project.swatches != null) {
			for (s in project.swatches) {
				Project.raw.swatches.push(s);
			}
		}
		UIBase.inst.hwnds[TabStatus].redraws = 2;
		Data.deleteBlob(path);
	}

	static function makePink(abs: String) {
		Console.error(Strings.error2() + " " + abs);
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

			#if (is_paint || is_sculpt)
			if (node.type == "TEX_IMAGE") {
			#end

			#if is_lab
			if (node.type == "ImageTextureNode") {
			#end

				node.buttons[0].default_value = App.getAssetIndex(node.buttons[0].data);
			}
		}
	}

	static function unpackAsset(project: TProjectFormat, abs: String, file: String) {
		if (Project.raw.packed_assets == null) {
			Project.raw.packed_assets = [];
		}
		for (pa in project.packed_assets) {
			#if krom_windows
			pa.name = pa.name.replace("/", "\\");
			#else
			pa.name = pa.name.replace("\\", "/");
			#end
			pa.name = Path.normalize(pa.name);
			if (pa.name == file) pa.name = abs; // From relative to absolute
			if (pa.name == abs) {
				if (!Project.packedAssetExists(Project.raw.packed_assets, pa.name)) {
					Project.raw.packed_assets.push(pa);
				}
				kha.Image.fromEncodedBytes(pa.bytes, pa.name.endsWith(".jpg") ? ".jpg" : ".png", function(image: kha.Image) {
					Data.cachedImages.set(abs, image);
				}, null, false);
				break;
			}
		}
	}
}
