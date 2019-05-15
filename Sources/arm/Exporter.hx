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
		var texpaint:kha.Image = null;
		var texpaint_nor:kha.Image = null;
		var texpaint_pack:kha.Image = null;
		var layers = UITrait.inst.layers;

		// Export visible layers
		if (UITrait.inst.layersExport == 0 && layers.length > 1) {
			Layers.makeTempImg();
			Layers.makeExportImg();
			if (Layers.pipe == null) Layers.makePipe();
			if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

			// Duplicate base layer
			if (layers[0].visible) {
				Layers.expa.g2.begin(false);
				Layers.expa.g2.pipeline = Layers.pipeCopy;
				Layers.expa.g2.drawImage(layers[0].texpaint, 0, 0);
				Layers.expa.g2.end();
				Layers.expb.g2.begin(false);
				Layers.expb.g2.pipeline = Layers.pipeCopy;
				Layers.expb.g2.drawImage(layers[0].texpaint_nor, 0, 0);
				Layers.expb.g2.end();
				Layers.expc.g2.begin(false);
				Layers.expc.g2.pipeline = Layers.pipeCopy;
				Layers.expc.g2.drawImage(layers[0].texpaint_pack, 0, 0);
				Layers.expc.g2.end();
			}
			else {
				Layers.expa.g4.begin();
				Layers.expa.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0));
				Layers.expa.g4.end();
				Layers.expb.g4.begin();
				Layers.expb.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0));
				Layers.expb.g4.end();
				Layers.expc.g4.begin();
				Layers.expc.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0));
				Layers.expc.g4.end();
			}

			for (i in 1...layers.length) {
				var l1 = layers[i];
				if (!l1.visible) continue;

				// Apply mask
				var hasMask = l1.texpaint_mask != null;
				if (hasMask) {
					Layers.expd.g4.begin();
					Layers.expd.g4.setPipeline(Layers.pipeMask);
					Layers.expd.g4.setTexture(Layers.tex0Mask, l1.texpaint);
					Layers.expd.g4.setTexture(Layers.texaMask, l1.texpaint_mask);
					Layers.expd.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
					Layers.expd.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
					Layers.expd.g4.drawIndexedVertices();
					Layers.expd.g4.end();
				}
				
				// Copy layer0 to temp
				Layers.imga.g2.begin(false);
				Layers.imga.g2.pipeline = Layers.pipeCopy;
				Layers.imga.g2.drawImage(Layers.expa, 0, 0);
				Layers.imga.g2.end();
				Layers.imgb.g2.begin(false);
				Layers.imgb.g2.pipeline = Layers.pipeCopy;
				Layers.imgb.g2.drawImage(Layers.expb, 0, 0);
				Layers.imgb.g2.end();
				Layers.imgc.g2.begin(false);
				Layers.imgc.g2.pipeline = Layers.pipeCopy;
				Layers.imgc.g2.drawImage(Layers.expc, 0, 0);
				Layers.imgc.g2.end();
				
				// Merge into layer0
				Layers.expa.g4.begin([Layers.expb, Layers.expc]);
				Layers.expa.g4.setPipeline(Layers.pipe);
				Layers.expa.g4.setTexture(Layers.tex0, hasMask ? Layers.expd : l1.texpaint);
				Layers.expa.g4.setTexture(Layers.tex1, l1.texpaint_nor);
				Layers.expa.g4.setTexture(Layers.tex2, l1.texpaint_pack);
				Layers.expa.g4.setTexture(Layers.texa, Layers.imga);
				Layers.expa.g4.setTexture(Layers.texb, Layers.imgb);
				Layers.expa.g4.setTexture(Layers.texc, Layers.imgc);
				Layers.expa.g4.setFloat(Layers.opac, l1.maskOpacity);
				Layers.expa.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				Layers.expa.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				Layers.expa.g4.drawIndexedVertices();
				Layers.expa.g4.end();
			}

			texpaint = Layers.expa;
			texpaint_nor = Layers.expb;
			texpaint_pack = Layers.expc;
		}
		// Export selected layer
		else {
			var selectedLayer = UITrait.inst.selectedLayer;
			texpaint = selectedLayer.texpaint;
			texpaint_nor = selectedLayer.texpaint_nor;
			texpaint_pack = selectedLayer.texpaint_pack;
			if (selectedLayer.objectMask > 0) { // Append object mask name
				f += "_" + UITrait.inst.paintObjects[selectedLayer.objectMask].name;
			}
		}

		var bo = new haxe.io.BytesOutput();
		var pixels:haxe.io.Bytes = null;

		if (UITrait.inst.isBase || UITrait.inst.isOpac) {
			pixels = texpaint.getPixels(); // bgra
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
			pixels = texpaint_nor.getPixels();
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

			pixels = texpaint_pack.getPixels(); // occ, rough, met, height

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
