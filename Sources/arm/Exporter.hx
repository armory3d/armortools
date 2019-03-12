package arm;

import arm.ui.*;

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
		var pixels = selectedLayer.texpaint.getPixels(); // bgra
		if (UITrait.inst.isBaseSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
				// pixels.set(i * 4 + 3, 255);
			}
		}
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
		if (UITrait.inst.isBase) Krom.fileSaveBytes(path + "/" + f + "_base" + ext, bo.getBytes().getData());
		#end

		pixels = selectedLayer.texpaint_nor.getPixels();
		if (UITrait.inst.isNorSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
				pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
				// pixels.set(i * 4 + 3, 255);
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
		if (UITrait.inst.isNor) Krom.fileSaveBytes(path + "/" + f + "_nor" + ext, bo.getBytes().getData());
		#end

		pixels = selectedLayer.texpaint_pack.getPixels(); // occ, rough, met

		if (UITrait.inst.isOccSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
			}
		}
		if (UITrait.inst.isRoughSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
			}
		}
		if (UITrait.inst.isMetSpace == 1) {
			for (i in 0...Std.int(pixels.length / 4)) {
				pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
			}
		}

		if (UITrait.inst.outputType == 0) {
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
			if (UITrait.inst.isOcc) Krom.fileSaveBytes(path + "/" + f + "_occ" + ext, bo.getBytes().getData());
			#end

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
			if (UITrait.inst.isRough) Krom.fileSaveBytes(path + "/" + f + "_rough" + ext, bo.getBytes().getData());
			#end
			
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
			if (UITrait.inst.isMet) Krom.fileSaveBytes(path + "/" + f + "_met" + ext, bo.getBytes().getData());
			#end
		}
		else { // UE4
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
			if (UITrait.inst.isOcc) Krom.fileSaveBytes(path + "/" + f + "_orm" + ext, bo.getBytes().getData());
			#end
		}

		if (UITrait.inst.isHeight) {
			// pixels = selectedLayer.texpaint_opt.getPixels();
			// if (UITrait.inst.isHeightSpace == 1) {
			// 	for (i in 0...Std.int(pixels.length / 4)) {
			// 		pixels.set(i * 4 + 0, Std.int(Math.pow(pixels.get(i * 4 + 0) / 255, 1.0 / 2.2) * 255));
			// 		pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, 1.0 / 2.2) * 255));
			// 		pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, 1.0 / 2.2) * 255));
			// 		// pixels.set(i * 4 + 3, 255);
			// 	}
			// }
			// bo = new haxe.io.BytesOutput();
			// if (formatType == 0) {
			// 	var jpgdata:iron.format.jpg.Data.Data = {
			// 		width: textureSize,
			// 		height: textureSize,
			// 		quality: formatQuality,
			// 		pixels: pixels
			// 	};
			// 	var jpgwriter = new iron.format.jpg.Writer(bo);
			// 	jpgwriter.write(jpgdata, 1);
			// }
			// else {
			// 	var pngwriter = new iron.format.png.Writer(bo);
			// 	pngwriter.write(iron.format.png.Tools.build32RGBA(textureSize, textureSize, pixels));
			// }
			// #if kha_krom
			// Krom.fileSaveBytes(path + "/" + f + "_height" + ext, bo.getBytes().getData());
			// #end
		}

		// if (UITrait.inst.isOpac) Krom.fileSaveBytes(path + "/tex_opac" + ext, bo.getBytes().getData());
		// if (UITrait.inst.isEmis) Krom.fileSaveBytes(path + "/tex_emis" + ext, bo.getBytes().getData());
		// if (UITrait.inst.isSubs) Krom.fileSaveBytes(path + "/tex_subs" + ext, bo.getBytes().getData());
	}

	public static function exportMesh(path:String) {
		var s = "";
		var off = 0;
		for (p in UITrait.inst.paintObjects) {
			var mesh = p.data.raw;
			var posa = mesh.vertex_arrays[0].values;
			var nora = mesh.vertex_arrays[1].values;
			var texa = mesh.vertex_arrays[2].values;
			var len = Std.int(posa.length / 3);
			s += "o " + p.name + "\n";
			for (i in 0...len) {
				s += "v " + posa[i * 4 + 0] + " " +
							posa[i * 4 + 2] + " " +
							(-posa[i * 4 + 1]) + "\n";
			}
			for (i in 0...len) {
				s += "vn " + nora[i * 2 + 0] + " " +
							 posa[i * 4 + 3] + " " +
							 (-nora[i * 2 + 1]) + "\n";
			}
			for (i in 0...len) {
				s += "vt " + texa[i * 2 + 0] + " " +
							 (1.0 - texa[i * 2 + 1]) + "\n";
			}
			var inda = mesh.index_arrays[0].values;
			for (i in 0...Std.int(inda.length / 3)) {

				var i1 = inda[i * 3 + 0] + 1 + off;
				var i2 = inda[i * 3 + 1] + 1 + off;
				var i3 = inda[i * 3 + 2] + 1 + off;
				s += "f " + i1 + "/" + i1 + "/" + i1 + " " +
							i2 + "/" + i2 + "/" + i2 + " " +
							i3 + "/" + i3 + "/" + i3 + "\n";
			}
			off += inda.length;
		}
		#if kha_krom
		if (!StringTools.endsWith(path, ".obj")) path += ".obj";
		Krom.fileSaveBytes(path, haxe.io.Bytes.ofString(s).getData());
		#end
	}
}
