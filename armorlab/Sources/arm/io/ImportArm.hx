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
import iron.system.Lz4;
import iron.object.Object;
import iron.Scene;
import iron.RenderPath;
import arm.ProjectFormat;
import arm.ui.UIFiles;
import arm.ui.UIStatus;
import arm.sys.Path;
import arm.sys.File;
import arm.Viewport;
import arm.Enums;

class ImportArm {

	public static function runProject(path: String) {
		Data.getBlob(path, function(b: Blob) {
			var project: TProjectFormat = ArmPack.decode(b.toBytes());

			// if (project.version != null && project.layer_datas == null) {
			// 	// Import as swatches
			// 	else if (project.swatches != null) {
			// 		runSwatchesFromProject(project, path);
			// 	}
			// 	return;
			// }

			Project.projectNew(true);
			Project.filepath = path;
			UIFiles.filename = path.substring(path.lastIndexOf(Path.sep) + 1, path.lastIndexOf("."));
			#if (krom_android || krom_ios)
			Window.get(0).title = UIFiles.filename;
			#else
			Window.get(0).title = UIFiles.filename + " - " + Main.title;
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

			// Synchronous for now
			new MeshData(project.mesh_data, function(md: MeshData) {
				Context.paintObject.setData(md);
				Context.paintObject.transform.scale.set(1, 1, 1);
				Context.paintObject.transform.buildMatrix();
				Context.paintObject.name = md.name;
				Project.paintObjects = [Context.paintObject];
			});

			Context.selectPaintObject(Context.mainObject());
			Viewport.scaleToBounds();
			Context.paintObject.skip_context = "paint";
			Context.mergedObject.visible = true;

			arm.ui.UINodes.inst.hwnd.redraws = 2;
			arm.ui.UINodes.inst.groupStack = [];
			Project.materialGroups = [];
			if (project.material_groups != null) {
				for (g in project.material_groups) Project.materialGroups.push({ canvas: g, nodes: new Nodes() });
			}

			initNodes(project.material.nodes);
			Project.canvas = project.material;
			arm.node.Brush.parse(Project.canvas, false);

			Context.ddirty = 4;

			Data.deleteBlob(path);
		});
	}

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
		UIStatus.inst.statusHandle.redraws = 2;
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
			if (node.type == "ImageTextureNode") {
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
