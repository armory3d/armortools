package arm;

import zui.Canvas;
import zui.Nodes;
import iron.data.SceneFormat;
import iron.data.MeshData;
import arm.creator.NodeCreator;
import arm.util.RenderUtil;
import arm.util.MeshUtil;
import arm.util.UVUtil;
import arm.util.ViewportUtil;
import arm.ui.UITrait;
import arm.ui.UIBox;
import arm.ui.UIView2D;
import arm.ui.UINodes;

class Importer {

	public static var fontList = ["default.ttf"];
	public static var fontMap = new Map<String, kha.Font>();

	public static function importFile(path:String, dropX = -1.0, dropY = -1.0) {
		// Mesh
		if (Format.checkMeshFormat(path)) {
			importMesh(path);
		}
		// Image
		else if (Format.checkTextureFormat(path)) {
			importTexture(path);
			// Place image node
			var x0 = UINodes.inst.wx;
			var x1 = UINodes.inst.wx + UINodes.inst.ww;
			if (UINodes.inst.show && dropX > x0 && dropX < x1) {
				UINodes.inst.acceptDrag(UITrait.inst.assets.length - 1);
				UINodes.inst.nodes.nodesDrag = false;
				UINodes.inst.hwnd.redraws = 2;
			}
		}
		// Font
		else if (Format.checkFontFormat(path)) {
			importFont(path);
		}
		// Project
		else if (Format.checkProjectFormat(path)) {
			Project.importProject(path);
		}
		// Folder
		else if (path.indexOf(".") == -1) {
			importFolder(path);
		}
		else {
			UITrait.inst.showError("Error: Unknown asset format");
		}
	}

	static function importFolder(path:String) {
		#if kha_krom
		var systemId = kha.System.systemId;
		var cmd = systemId == "Windows" ? "dir /b " : "ls ";
		var sep = systemId == "Windows" ? "\\" : "/";
		var save = systemId == "Linux" ? "/tmp" : Krom.savePath();
		save += sep + "dir.txt";
		Krom.sysCommand(cmd + '"' + path + '"' + ' > ' + '"' + save + '"');
		var str = haxe.io.Bytes.ofData(Krom.loadBlob(save)).toString();
		var files = str.split("\n");
		var mapbase = "";
		var mapopac = "";
		var mapnor = "";
		var mapocc = "";
		var maprough = "";
		var mapmet = "";
		var mapheight = "";
		// Import maps
		for (f in files) {
			if (f.length == 0) continue;
			f = StringTools.rtrim(f);
			if (!Format.checkTextureFormat(f)) continue;
			
			f = path + sep + f;
			if (systemId == "Windows") f = StringTools.replace(f, "/", "\\");

			// TODO: handle -albedo
			
			var base = f.substr(0, f.lastIndexOf(".")).toLowerCase();
			var valid = false;
			if (mapbase == "" && Format.checkBaseTex(base)) {
				mapbase = f;
				valid = true;
			}
			if (mapopac == "" && Format.checkOpacTex(base)) {
				mapopac = f;
				valid = true;
			}
			if (mapnor == "" && Format.checkNorTex(base)) {
				mapnor = f;
				valid = true;
			}
			if (mapocc == "" && Format.checkOccTex(base)) {
				mapocc = f;
				valid = true;
			}
			if (maprough == "" && Format.checkRoughTex(base)) {
				maprough = f;
				valid = true;
			}
			if (mapmet == "" && Format.checkMetTex(base)) {
				mapmet = f;
				valid = true;
			}
			if (mapheight == "" && Format.checkDispTex(base)) {
				mapheight = f;
				valid = true;
			}

			if (valid) importTexture(f);
		}
		// Create material
		UITrait.inst.selectedMaterial = new MaterialSlot();
		UITrait.inst.materials.push(UITrait.inst.selectedMaterial);
		UINodes.inst.updateCanvasMap();
		var nodes = UINodes.inst.nodes;
		var canvas = UINodes.inst.canvas;
		var nout:TNode = null;
		for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") { nout = n; break; }
		for (n in canvas.nodes) if (n.name == "RGB") { nodes.removeNode(n, canvas); break; }
		
		var pos = 0;
		if (mapbase != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapbase);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 0 };
			canvas.links.push(l);
		}
		if (mapopac != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapopac);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 1 };
			canvas.links.push(l);
		}
		if (mapocc != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapocc);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 2 };
			canvas.links.push(l);
		}
		if (maprough != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(maprough);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 3 };
			canvas.links.push(l);
		}
		if (mapmet != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapmet);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 4 };
			canvas.links.push(l);
		}
		if (mapnor != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapnor);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 5 };
			canvas.links.push(l);
		}
		if (mapheight != "") {
			var n = NodeCreator.createImageTexture();
			n.buttons[0].default_value = arm.App.getAssetIndex(mapheight);
			n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
			n.x = 72;
			n.y = 192 + 160 * pos;
			pos++;
			var l = { id: nodes.getLinkId(canvas.links), from_id: n.id, from_socket: 0, to_id: nout.id, to_socket: 7 };
			canvas.links.push(l);
		}
		iron.system.Tween.timer(0.01, function() {
			arm.MaterialParser.parsePaintMaterial();
			RenderUtil.makeMaterialPreview();
			UITrait.inst.hwnd1.redraws = 2;
		});
		#end
	}

	public static function importFont(path:String) {
		iron.data.Data.getFont(path, function(font:kha.Font) {
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			fontList.push(name);
			fontMap.set(name, font);
		});
	}

	public static function importTexture(path:String) {
		if (!Format.checkTextureFormat(path)) {
			UITrait.inst.showError("Error: Unknown asset format");
			return;
		}

		for (a in UITrait.inst.assets) if (a.file == path) { UITrait.inst.showMessage("Info: Asset already imported"); return; }
		
		iron.data.Data.getImage(path, function(image:kha.Image) {
			var ar = path.split("/");
			ar = ar[ar.length - 1].split("\\");
			var name = ar[ar.length - 1];
			var asset:TAsset = {name: name, file: path, id: UITrait.inst.assetId++};
			UITrait.inst.assets.push(asset);
			UITrait.inst.assetNames.push(name);
			Canvas.assetMap.set(asset.id, image);
			UITrait.inst.hwnd2.redraws = 2;

			// Set envmap, has to be 2K res for now
			if (StringTools.endsWith(path.toLowerCase(), ".hdr") &&
				(image.width == 1024 || image.width == 2048 || image.width == 4096)) {
				
				#if kha_krom
				var sys = kha.System.systemId;
				var dataPath = iron.data.Data.dataPath;
				var p = Krom.getFilesLocation() + '/' + dataPath;
				var cmft = p + "/cmft" + (sys == "Windows" ? ".exe" : sys == "Linux" ? "-linux64" : "-osx");

				var cmd = '';
				var tmp = Krom.getFilesLocation() + '/' + dataPath;

				// Irr
				cmd = cmft;
				cmd += ' --input "' + path + '"';
				cmd += ' --filter shcoeffs';
				cmd += ' --outputNum 1';
				cmd += ' --output0 "' + tmp + 'tmp_irr"';
				Krom.sysCommand(cmd);

				// Rad
				var faceSize = Std.int(image.width / 8);
				cmd = cmft;
				cmd += ' --input "' + path + '"';
				cmd += ' --filter radiance';
				cmd += ' --dstFaceSize ' + faceSize;
				cmd += ' --srcFaceSize ' + faceSize;
				cmd += ' --excludeBase false';
				cmd += ' --glossScale 8';
				cmd += ' --glossBias 3';
				cmd += ' --lightingModel blinnbrdf';
				cmd += ' --edgeFixup none';
				cmd += ' --numCpuProcessingThreads 4';
				cmd += ' --useOpenCL true';
				cmd += ' --clVendor anyGpuVendor';
				cmd += ' --deviceType gpu';
				cmd += ' --deviceIndex 0';
				cmd += ' --generateMipChain true';
				cmd += ' --inputGammaNumerator 1.0';
				cmd += ' --inputGammaDenominator 2.2';
				cmd += ' --outputGammaNumerator 1.0';
				cmd += ' --outputGammaDenominator 1.0';
				cmd += ' --outputNum 1';
				cmd += ' --output0 "' + tmp + 'tmp_rad"';
				cmd += ' --output0params hdr,rgbe,latlong';
				Krom.sysCommand(cmd);
				#else
				var tmp = "";
				#end

				// Load irr
				iron.data.Data.getBlob(tmp + "tmp_irr.c", function(blob:kha.Blob) {
					var lines = blob.toString().split("\n");
					var band0 = lines[5];
					var band1 = lines[6];
					var band2 = lines[7];
					band0 = band0.substring(band0.indexOf("{"), band0.length);
					band1 = band1.substring(band1.indexOf("{"), band1.length);
					band2 = band2.substring(band2.indexOf("{"), band2.length);
					var band = band0 + band1 + band2;
					band = StringTools.replace(band, "{", "");
					band = StringTools.replace(band, "}", "");
					var ar = band.split(",");
					var buf = new kha.arrays.Float32Array(27);
					for (i in 0...ar.length) buf[i] = Std.parseFloat(ar[i]);
					iron.Scene.active.world.probe.irradiance = buf;
					UITrait.inst.ddirty = 2;
				});

				// World envmap
				iron.Scene.active.world.envmap = image;
				UITrait.inst.savedEnvmap = image;
				UITrait.inst.showEnvmapHandle.selected = UITrait.inst.showEnvmap = true;

				// Load mips
				var mips = Std.int(image.width / 1024);
				var mipsCount = 6 + mips;
				var mipsLoaded = 0;
				var mips:Array<kha.Image> = [];
				while (mips.length < mipsCount + 2) mips.push(null);
				var mw = Std.int(image.width / 2);
				var mh = Std.int(image.width / 4);
				for (i in 0...mipsCount) {
					iron.data.Data.getImage(tmp + "tmp_rad_" + i + "_" + mw + "x" + mh + ".hdr", function(mip:kha.Image) {
						mips[i] = mip;
						mipsLoaded++;
						if (mipsLoaded == mipsCount) {
							// 2x1 and 1x1 mips
							mips[mipsCount] = kha.Image.create(2, 1, kha.graphics4.TextureFormat.RGBA128);
							mips[mipsCount + 1] = kha.Image.create(1, 1, kha.graphics4.TextureFormat.RGBA128);
							// Set radiance
							image.setMipmaps(mips);
							iron.Scene.active.world.probe.radiance = image;
							iron.Scene.active.world.probe.radianceMipmaps = mips;
							UITrait.inst.ddirty = 2;
						}
					}, true); // Readable
					mw = Std.int(mw / 2);
					mh = Std.int(mh / 2);
				}
			}
		});
	}

	public static function importMesh(path:String) {
		if (!Format.checkMeshFormat(path)) {
			UITrait.inst.showError("Error: Unknown mesh format");
			return;
		}

		#if arm_debug
		var timer = iron.system.Time.realTime();
		#end

		var p = path.toLowerCase();
		if (StringTools.endsWith(p, ".obj")) importObj(path);
		else if (StringTools.endsWith(p, ".gltf")) importGltf(path);
		else if (StringTools.endsWith(p, ".fbx")) importFbx(path);
		else if (StringTools.endsWith(p, ".blend")) importBlend(path);

		if (UITrait.inst.mergedObject != null) {
			UITrait.inst.mergedObject.remove();
			iron.data.Data.deleteMesh(UITrait.inst.mergedObject.data.handle);
			UITrait.inst.mergedObject = null;
		}

		UITrait.inst.selectPaintObject(UITrait.inst.mainObject());

		if (UITrait.inst.paintObjects.length > 1) {
			// Sort by name
			UITrait.inst.paintObjects.sort(function(a, b):Int {
				if (a.name < b.name) return -1;
				else if (a.name > b.name) return 1;
				return 0;
			});

			// No mask by default
			if (UITrait.inst.mergedObject == null) MeshUtil.mergeMesh();
			UITrait.inst.paintObject.skip_context = "paint";
			UITrait.inst.mergedObject.visible = true;
		}

		ViewportUtil.scaleToBounds();

		if (UITrait.inst.paintObject.name == "") UITrait.inst.paintObject.name = "Object";

		UIView2D.inst.hwnd.redraws = 2;

		#if arm_debug
		trace("Mesh imported in " + (iron.system.Time.realTime() - timer));
		#end
	}

	static function importObj(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			if (UITrait.inst.isUdim) {
				var obj = new iron.format.obj.ObjParser(b, 0, UITrait.inst.isUdim);
				var name = obj.name;
				for (i in 0...obj.udims.length) {
					var u = i % obj.udimsU;
					var v = Std.int(i / obj.udimsU);
					obj.name = name + "." + (1000 + v * 10 + u + 1);
					obj.inda = obj.udims[i];
					i == 0 ? makeMesh(obj, path) : addMesh(obj);
				}
			}
			else {
				var obj = new iron.format.obj.ObjParser(b);
				makeMesh(obj, path);
				while (obj.hasNext) {
					obj = new iron.format.obj.ObjParser(b, obj.pos);
					addMesh(obj);
				}
			}
			iron.data.Data.deleteBlob(path);
		});
	}

	static function importGltf(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.gltf.GltfParser(b);
			makeMesh(obj, path);
			iron.data.Data.deleteBlob(path);
		});
	}

	static function importFbx(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var obj = new iron.format.fbx.FbxParser(b);
			makeMesh(obj, path);
			while (obj.next()) {
				addMesh(obj);
			}
			iron.data.Data.deleteBlob(path);
		});
	}

	static function importBlend(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var bl = new iron.format.blend.Blend(b);
			if (bl.dna == null) { makeMesh(null, path); return; }

			// var obs = bl.get("Object");
			// var ob = obs[0];
			// var name:String = ob.get("id").get("name");
			// name = name.substring(2, name.length);
			// trace(ob.get("type")); // 1

			var m = bl.get("Mesh")[0];
			if (m == null) { makeMesh(null, path); return; }

			var totpoly = m.get("totpoly");
			var numtri = 0;
			for (i in 0...totpoly) {
				var poly = m.get("mpoly", i);
				var totloop = poly.get("totloop");
				numtri += totloop == 3 ? 1 : 2;
			}
			var inda = new kha.arrays.Uint32Array(numtri * 3);
			for (i in 0...inda.length) inda[i] = i;

			var posa32 = new kha.arrays.Float32Array(numtri * 3 * 4);
			var posa = new kha.arrays.Int16Array(numtri * 3 * 4);
			var nora = new kha.arrays.Int16Array(numtri * 3 * 2);
			var hasuv = m.get("mloopuv") != null;
			var texa = hasuv ? new kha.arrays.Int16Array(numtri * 3 * 2) : null;
			
			var tri = 0;
			var vec0 = new iron.math.Vec4();
			var vec1 = new iron.math.Vec4();
			var vec2 = new iron.math.Vec4();
			var vec3 = new iron.math.Vec4();
			for (i in 0...totpoly) {
				var poly = m.get("mpoly", i);
				var loopstart = poly.get("loopstart");
				var totloop = poly.get("totloop");
				if (totloop >= 3) {
					var v0 = m.get("mvert", m.get("mloop", loopstart + 0).get("v"));
					var v1 = m.get("mvert", m.get("mloop", loopstart + 1).get("v"));
					var v2 = m.get("mvert", m.get("mloop", loopstart + 2).get("v"));
					var co0 = v0.get("co");
					var co1 = v1.get("co");
					var co2 = v2.get("co");
					var no0 = v0.get("no");
					var no1 = v1.get("no");
					var no2 = v2.get("no");
					vec0.set(no0[0] / 32767, no0[1] / 32767, no0[2] / 32767).normalize(); // shortmax
					vec1.set(no1[0] / 32767, no1[1] / 32767, no1[2] / 32767).normalize();
					vec2.set(no2[0] / 32767, no2[1] / 32767, no2[2] / 32767).normalize();
					posa32[tri * 9    ] = co0[0];
					posa32[tri * 9 + 1] = co0[1];
					posa32[tri * 9 + 2] = co0[2];
					posa32[tri * 9 + 3] = co1[0];
					posa32[tri * 9 + 4] = co1[1];
					posa32[tri * 9 + 5] = co1[2];
					posa32[tri * 9 + 6] = co2[0];
					posa32[tri * 9 + 7] = co2[1];
					posa32[tri * 9 + 8] = co2[2];
					posa[tri * 12 + 3] = Std.int(vec0.z * 32767);
					posa[tri * 12 + 7] = Std.int(vec1.z * 32767);
					posa[tri * 12 + 11] = Std.int(vec2.z * 32767);
					nora[tri * 6    ] = Std.int(vec0.x * 32767);
					nora[tri * 6 + 1] = Std.int(vec0.y * 32767);
					nora[tri * 6 + 2] = Std.int(vec1.x * 32767);
					nora[tri * 6 + 3] = Std.int(vec1.y * 32767);
					nora[tri * 6 + 4] = Std.int(vec2.x * 32767);
					nora[tri * 6 + 5] = Std.int(vec2.y * 32767);
					
					var uv0:kha.arrays.Float32Array = null;
					var uv1:kha.arrays.Float32Array = null;
					var uv2:kha.arrays.Float32Array = null;
					if (hasuv) {
						uv0 = m.get("mloopuv", loopstart + 0).get("uv");
						uv1 = m.get("mloopuv", loopstart + 1).get("uv");
						uv2 = m.get("mloopuv", loopstart + 2).get("uv");
						texa[tri * 6    ] = Std.int(uv0[0] * 32767);
						texa[tri * 6 + 1] = Std.int((1.0 - uv0[1]) * 32767);
						texa[tri * 6 + 2] = Std.int(uv1[0] * 32767);
						texa[tri * 6 + 3] = Std.int((1.0 - uv1[1]) * 32767);
						texa[tri * 6 + 4] = Std.int(uv2[0] * 32767);
						texa[tri * 6 + 5] = Std.int((1.0 - uv2[1]) * 32767);
					}
					tri++;

					if (totloop >= 4) {
						var v3 = m.get("mvert", m.get("mloop", loopstart + 3).get("v"));
						var co3 = v3.get("co");
						var no3 = v3.get("no");
						vec3.set(no3[0] / 32767, no3[1] / 32767, no3[2] / 32767).normalize();
						posa32[tri * 9    ] = co2[0];
						posa32[tri * 9 + 1] = co2[1];
						posa32[tri * 9 + 2] = co2[2];
						posa32[tri * 9 + 3] = co3[0];
						posa32[tri * 9 + 4] = co3[1];
						posa32[tri * 9 + 5] = co3[2];
						posa32[tri * 9 + 6] = co0[0];
						posa32[tri * 9 + 7] = co0[1];
						posa32[tri * 9 + 8] = co0[2];
						posa[tri * 12 + 3] = Std.int(vec2.z * 32767);
						posa[tri * 12 + 7] = Std.int(vec3.z * 32767);
						posa[tri * 12 + 11] = Std.int(vec0.z * 32767);
						nora[tri * 6    ] = Std.int(vec2.x * 32767);
						nora[tri * 6 + 1] = Std.int(vec2.y * 32767);
						nora[tri * 6 + 2] = Std.int(vec3.x * 32767);
						nora[tri * 6 + 3] = Std.int(vec3.y * 32767);
						nora[tri * 6 + 4] = Std.int(vec0.x * 32767);
						nora[tri * 6 + 5] = Std.int(vec0.y * 32767);
						
						if (hasuv) {
							var uv3 = m.get("mloopuv", loopstart + 3).get("uv");
							texa[tri * 6    ] = Std.int(uv2[0] * 32767);
							texa[tri * 6 + 1] = Std.int((1.0 - uv2[1]) * 32767);
							texa[tri * 6 + 2] = Std.int(uv3[0] * 32767);
							texa[tri * 6 + 3] = Std.int((1.0 - uv3[1]) * 32767);
							texa[tri * 6 + 4] = Std.int(uv0[0] * 32767);
							texa[tri * 6 + 5] = Std.int((1.0 - uv0[1]) * 32767);
						}
						tri++;
					}
				}
			}

			// Pack positions to (-1, 1) range
			var hx = 0.0;
			var hy = 0.0;
			var hz = 0.0;
			for (i in 0...Std.int(posa32.length / 3)) {
				var f = Math.abs(posa32[i * 3]);
				if (hx < f) hx = f;
				f = Math.abs(posa32[i * 3 + 1]);
				if (hy < f) hy = f;
				f = Math.abs(posa32[i * 3 + 2]);
				if (hz < f) hz = f;
			}
			var scalePos = Math.max(hx, Math.max(hy, hz));
			var inv = 1 / scalePos;
			for (i in 0...Std.int(posa32.length / 3)) {
				posa[i * 4    ] = Std.int(posa32[i * 3    ] * 32767 * inv);
				posa[i * 4 + 1] = Std.int(posa32[i * 3 + 1] * 32767 * inv);
				posa[i * 4 + 2] = Std.int(posa32[i * 3 + 2] * 32767 * inv);
			}

			var name:String = m.get("id").get("name");
			name = name.substring(2, name.length);
			var obj = {posa: posa, nora: nora, texa: texa, inda: inda, name: name, scalePos: scalePos, scaleTes: 1.0};
			makeMesh(obj, path);
			iron.data.Data.deleteBlob(path);
		});
	}

	static function makeMesh(mesh:Dynamic, path:String) {
		if (mesh == null || mesh.posa == null || mesh.nora == null || mesh.inda == null) {
			UITrait.inst.showError("Error: Failed to read mesh data");
			return;
		}

		var raw:TMeshData = null;
		if (UITrait.inst.worktab.position == 1) {
			raw = {
				name: mesh.name,
				vertex_arrays: [
					{ values: mesh.posa, attrib: "pos" },
					{ values: mesh.nora, attrib: "nor" }
				],
				index_arrays: [
					{ values: mesh.inda, material: 0 }
				],
				scale_pos: mesh.scalePos,
				scale_tex: mesh.scaleTex
			};
			if (mesh.texa != null) raw.vertex_arrays.push({ values: mesh.texa, attrib: "tex" });
		}
		else {

			if (mesh.texa == null) {
				UITrait.inst.showError("Error: Mesh has no UVs, generating defaults");
				var verts = Std.int(mesh.posa.length / 4);
				mesh.texa = new kha.arrays.Int16Array(verts * 2);
				var n = new iron.math.Vec4();
				for (i in 0...verts) {
					n.set(mesh.posa[i * 4] / 32767, mesh.posa[i * 4 + 1] / 32767, mesh.posa[i * 4 + 2] / 32767).normalize();
					// Sphere projection
					// mesh.texa[i * 2 + 0] = Math.atan2(n.x, n.y) / (Math.PI * 2) + 0.5;
					// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
					// Equirect
					mesh.texa[i * 2    ] = Std.int(((Math.atan2(-n.z, n.x) + Math.PI) / (Math.PI * 2)) * 32767);
					mesh.texa[i * 2 + 1] = Std.int((Math.acos(n.y) / Math.PI) * 32767);
				}
			}
			raw = {
				name: mesh.name,
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
		}

		new MeshData(raw, function(md:MeshData) {
			
			// Append
			if (UITrait.inst.worktab.position == 1) {
				var mats = new haxe.ds.Vector(1);
				mats[0] = UITrait.inst.selectedMaterial2.data;
				var object = iron.Scene.active.addMeshObject(md, mats, iron.Scene.active.getChild("Scene"));
				path = StringTools.replace(path, "\\", "/");
				var ar = path.split("/");
				var s = ar[ar.length - 1];
				object.name = s.substring(0, s.length - 4);

				// md.geom.calculateAABB();
				// var aabb = md.geom.aabb;
				// var dim = new TFloat32Array(3);
				// dim[0] = aabb.x;
				// dim[1] = aabb.y;
				// dim[2] = aabb.z;
				// object.raw.dimensions = dim;
				#if arm_physics
				object.addTrait(new armory.trait.physics.RigidBody(0.0));
				#end
				
				UITrait.inst.selectObject(object);
			}
			// Replace
			else {
				UITrait.inst.paintObject = UITrait.inst.mainObject();

				UITrait.inst.selectPaintObject(UITrait.inst.mainObject());
				for (i in 0...UITrait.inst.paintObjects.length) {
					var p = UITrait.inst.paintObjects[i];
					if (p == UITrait.inst.paintObject) continue;
					iron.data.Data.deleteMesh(p.data.handle);
					p.remove();
				}
				var handle = UITrait.inst.paintObject.data.handle;
				if (handle != "SceneSphere" && handle != "ScenePlane") {
					iron.data.Data.deleteMesh(handle);
				}

				while (UITrait.inst.layers.length > 1) { var l = UITrait.inst.layers.pop(); l.unload(); }
				UITrait.inst.setLayer(UITrait.inst.layers[0]);
				iron.App.notifyOnRender(Layers.initLayers);
				
				UITrait.inst.paintObject.setData(md);
				UITrait.inst.paintObject.name = mesh.name;

				// Face camera
				// UITrait.inst.paintObject.transform.setRotation(Math.PI / 2, 0, 0);

				UITrait.inst.paintObjects = [UITrait.inst.paintObject];
			}

			UITrait.inst.ddirty = 4;
			UITrait.inst.hwnd.redraws = 2;
			UITrait.inst.hwnd1.redraws = 2;
			UITrait.inst.hwnd2.redraws = 2;
			UVUtil.uvmapCached = false;
			UVUtil.trianglemapCached = false;
		});
	}

	static function addMesh(mesh:Dynamic) {

		if (mesh.texa == null) {
			UITrait.inst.showError("Error: Mesh has no UVs, generating defaults");
			var verts = Std.int(mesh.posa.length / 4);
			mesh.texa = new kha.arrays.Int16Array(verts * 2);
			var n = new iron.math.Vec4();
			for (i in 0...verts) {
				n.set(mesh.posa[i * 4] / 32767, mesh.posa[i * 4 + 1] / 32767, mesh.posa[i * 4 + 2] / 32767).normalize();
				// Sphere projection
				// mesh.texa[i * 2 + 0] = Math.atan2(n.x, n.y) / (Math.PI * 2) + 0.5;
				// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
				// Equirect
				mesh.texa[i * 2    ] = Std.int(((Math.atan2(-n.z, n.x) + Math.PI) / (Math.PI * 2)) * 32767);
				mesh.texa[i * 2 + 1] = Std.int((Math.acos(n.y) / Math.PI) * 32767);
			}
		}
		var raw:TMeshData = {
			name: mesh.name,
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

		new MeshData(raw, function(md:MeshData) {
			
			var object = iron.Scene.active.addMeshObject(md, UITrait.inst.paintObject.materials, UITrait.inst.paintObject);
			object.name = mesh.name;
			object.skip_context = "paint";

			UITrait.inst.paintObjects.push(object);

			UITrait.inst.ddirty = 4;
			UITrait.inst.hwnd.redraws = 2;
			UVUtil.uvmapCached = false;
			UVUtil.trianglemapCached = false;
		});
	}
}
