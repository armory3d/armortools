package arm;

import iron.data.SceneFormat;
import arm.ui.UITrait;
import arm.ui.UIBox;

class Exporter {

	public static function exportTextures(path:String) {
		
		var textureSize = Config.getTextureRes();
		var formatQuality = UITrait.inst.formatQuality;

		var f = arm.App.filenameHandle.text;
		if (f == "") f = "untitled";

		var formatType = UITrait.inst.formatType;
		var ext = formatType == 0 ? ".jpg" : formatType == 1 ? ".png" : ".tga";
		var bo = new haxe.io.BytesOutput();
		
		var selectedLayer = UITrait.inst.selectedLayer;
		var pixels:haxe.io.Bytes = null;

		// Append object mask name
		if (selectedLayer.objectMask > 0) {
			f += "_" + UITrait.inst.paintObjects[selectedLayer.objectMask].name;
		}

		if (UITrait.inst.isBase || UITrait.inst.isOpac) {
			pixels = selectedLayer.texpaint.getPixels(); // bgra
			if (UITrait.inst.isBase && UITrait.inst.isBaseSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
					pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
					pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
				}
			}
			if (UITrait.inst.isOpac && UITrait.inst.isOpacSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 3, Std.int(Math.pow(pixels.get(i * 4 + 3) / 255, 1.0 / 2.2) * 255));
				}
			}

			if (UITrait.inst.isBase) {
				if (formatType == 0) {
					var jpgdata:iron.format.jpg.Data.Data = {
						width: textureSize,
						height: textureSize,
						quality: formatQuality,
						pixels: pixels
					};
					var jpgwriter = new iron.format.jpg.Writer(bo);
					jpgwriter.write(jpgdata, 1);
				}
				else {
					var pngwriter = new iron.format.png.Writer(bo);
					pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
				}
				#if kha_krom
				Krom.fileSaveBytes(path + "/" + f + "_base" + ext, bo.getBytes().getData());
				#end
			}
			if (UITrait.inst.isOpac) {
				bo = new haxe.io.BytesOutput();
				if (formatType == 0) {
					var jpgdata:iron.format.jpg.Data.Data = {
						width: textureSize,
						height: textureSize,
						quality: formatQuality,
						pixels: pixels
					};
					var jpgwriter = new iron.format.jpg.Writer(bo);
					jpgwriter.write(jpgdata, 2, 3);
				}
				else {
					var pngwriter = new iron.format.png.Writer(bo);
					pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels, 3));
				}
				#if kha_krom
				Krom.fileSaveBytes(path + "/" + f + "_opac" + ext, bo.getBytes().getData());
				#end
			}
		}

		if (UITrait.inst.isNor) {
			pixels = selectedLayer.texpaint_nor.getPixels();
			if (UITrait.inst.isNorSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
					pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
					pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
				}
			}
			bo = new haxe.io.BytesOutput();
			if (formatType == 0) {
				var jpgdata:iron.format.jpg.Data.Data = {
					width: textureSize,
					height: textureSize,
					quality: formatQuality,
					pixels: pixels
				};
				var jpgwriter = new iron.format.jpg.Writer(bo);
				jpgwriter.write(jpgdata, 1);
			}
			else {
				var pngwriter = new iron.format.png.Writer(bo);
				pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
			}
			#if kha_krom
			Krom.fileSaveBytes(path + "/" + f + "_nor" + ext, bo.getBytes().getData());
			#end
		}

		if (UITrait.inst.isOcc || UITrait.inst.isRough || UITrait.inst.isMet || UITrait.inst.isHeight) {

			pixels = selectedLayer.texpaint_pack.getPixels(); // occ, rough, met, height

			if (UITrait.inst.isOcc && UITrait.inst.isOccSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
				}
			}
			if (UITrait.inst.isRough && UITrait.inst.isRoughSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
				}
			}
			if (UITrait.inst.isMet && UITrait.inst.isMetSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
				}
			}
			if (UITrait.inst.isHeight && UITrait.inst.isHeightSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 3, Std.int(Math.pow(pixels.get(i * 4 + 3) / 255, 1.0 / 2.2) * 255));
				}
			}

			if (UITrait.inst.outputType == 0) {

				if (UITrait.inst.isOcc) {
					bo = new haxe.io.BytesOutput();
					if (formatType == 0) {
						var jpgdata:iron.format.jpg.Data.Data = {
							width: textureSize,
							height: textureSize,
							quality: formatQuality,
							pixels: pixels
						};
						var jpgwriter = new iron.format.jpg.Writer(bo);
						jpgwriter.write(jpgdata, 2, 0);
					}
					else {
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.build32RGBA_(textureSize, textureSize, pixels, 0));
					}
					#if kha_krom
					Krom.fileSaveBytes(path + "/" + f + "_occ" + ext, bo.getBytes().getData());
					#end
				}

				if (UITrait.inst.isRough) {
					bo = new haxe.io.BytesOutput();
					if (formatType == 0) {
						var jpgdata:iron.format.jpg.Data.Data = {
							width: textureSize,
							height: textureSize,
							quality: formatQuality,
							pixels: pixels
						};
						var jpgwriter = new iron.format.jpg.Writer(bo);
						jpgwriter.write(jpgdata, 2, 1);
					}
					else {
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.build32RGBA_(textureSize, textureSize, pixels, 1));
					}
					#if kha_krom
					Krom.fileSaveBytes(path + "/" + f + "_rough" + ext, bo.getBytes().getData());
					#end
				}
				
				if (UITrait.inst.isMet) {
					bo = new haxe.io.BytesOutput();
					if (formatType == 0) {
						var jpgdata:iron.format.jpg.Data.Data = {
							width: textureSize,
							height: textureSize,
							quality: formatQuality,
							pixels: pixels
						};
						var jpgwriter = new iron.format.jpg.Writer(bo);
						jpgwriter.write(jpgdata, 2, 2);
					}
					else {
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.build32RGBA_(textureSize, textureSize, pixels, 2));
					}
					#if kha_krom
					Krom.fileSaveBytes(path + "/" + f + "_met" + ext, bo.getBytes().getData());
					#end
				}
			}
			else { // UE4
				if (UITrait.inst.isOcc || UITrait.inst.isRough || UITrait.inst.isMet) {
					bo = new haxe.io.BytesOutput();
					if (formatType == 0) {
						var jpgdata:iron.format.jpg.Data.Data = {
							width: textureSize,
							height: textureSize,
							quality: formatQuality,
							pixels: pixels
						};
						var jpgwriter = new iron.format.jpg.Writer(bo);
						jpgwriter.write(jpgdata, 1);
					}
					else {
						var pngwriter = new iron.format.png.Writer(bo);
						pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
					}
					#if kha_krom
					Krom.fileSaveBytes(path + "/" + f + "_orm" + ext, bo.getBytes().getData());
					#end
				}
			}

			if (UITrait.inst.isHeight) {
				bo = new haxe.io.BytesOutput();
				if (formatType == 0) {
					var jpgdata:iron.format.jpg.Data.Data = {
						width: textureSize,
						height: textureSize,
						quality: formatQuality,
						pixels: pixels
					};
					var jpgwriter = new iron.format.jpg.Writer(bo);
					jpgwriter.write(jpgdata, 2, 3);
				}
				else {
					var pngwriter = new iron.format.png.Writer(bo);
					pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels, 3));
				}
				#if kha_krom
				Krom.fileSaveBytes(path + "/" + f + "_height" + ext, bo.getBytes().getData());
				#end
			}
		}

		// if (UITrait.inst.isEmis) Krom.fileSaveBytes(path + "/tex_emis" + ext, bo.getBytes().getData());
		// if (UITrait.inst.isSubs) Krom.fileSaveBytes(path + "/tex_subs" + ext, bo.getBytes().getData());
	}

	public static function exportMesh(path:String) {
		if (UITrait.inst.exportMeshFormat == 0) { // .obj
			var s = "";
			var off = 0;
			for (p in UITrait.inst.paintObjects) {
				var mesh = p.data.raw;
				var sc = p.data.scalePos;
				var posa = mesh.vertex_arrays[0].values;
				var nora = mesh.vertex_arrays[1].values;
				var texa = mesh.vertex_arrays[2].values;
				var len = Std.int(posa.length / 4);
				s += "o " + p.name + "\n";
				for (i in 0...len) {
					s += "v " + posa[i * 4    ] * sc + " " +
								posa[i * 4 + 2] * sc + " " +
							  (-posa[i * 4 + 1] * sc) + "\n";
				}
				for (i in 0...len) {
					s += "vn " + nora[i * 2    ] + " " +
								 posa[i * 4 + 3] + " " +
							   (-nora[i * 2 + 1]) + "\n";
				}
				for (i in 0...len) {
					s += "vt " + texa[i * 2 + 0] + " " +
								 (1.0 - texa[i * 2 + 1]) + "\n";
				}
				var inda = mesh.index_arrays[0].values;
				for (i in 0...Std.int(inda.length / 3)) {
					var i1 = inda[i * 3    ] + 1 + off;
					var i2 = inda[i * 3 + 1] + 1 + off;
					var i3 = inda[i * 3 + 2] + 1 + off;
					s += "f " + i1 + "/" + i1 + "/" + i1 + " " +
								i2 + "/" + i2 + "/" + i2 + " " +
								i3 + "/" + i3 + "/" + i3 + "\n";
				}
				off += inda.length;
			}
			if (!StringTools.endsWith(path, ".obj")) path += ".obj";
			#if kha_krom
			Krom.fileSaveBytes(path, haxe.io.Bytes.ofString(s).getData());
			#end
		}
		else { // .arm
			var raw:TSceneFormat = { mesh_datas: [ UITrait.inst.paintObject.data.raw ] };
			var b = iron.system.ArmPack.encode(raw);
			if (!StringTools.endsWith(path, ".arm")) path += ".arm";
			#if kha_krom
			Krom.fileSaveBytes(path, b.getData());
			#end
		}
	}
}
