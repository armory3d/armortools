package arm.io;

import haxe.io.Bytes;
import haxe.io.BytesOutput;
import kha.Image;
import iron.data.SceneFormat;
import iron.format.ExrWriter;
import iron.format.JpgWriter;
import iron.format.PngWriter;
import iron.format.PngTools;
import arm.ui.UITrait;
import arm.ui.UIBox;
import arm.ui.UIFiles;
using StringTools;

class Exporter {

	static inline var gamma = 1.0 / 2.2;

	static function writeTexture(file:String, pixels:Bytes, type = 1, off = 0) {
		var out = new BytesOutput();
		var res = Config.getTextureRes();
		var bitsHandle = UITrait.inst.bitsHandle.position;
		var bits = bitsHandle == 0 ? 8 : bitsHandle == 1 ? 16 : 32;
		if (bits > 8) { // 16/32bit
			var writer = new ExrWriter(out, res, res, pixels, bits, type, off);
		}
		else if (UITrait.inst.formatType == 0) {
			var writer = new PngWriter(out);
			var data =
				type == 1 ?
					PngTools.build32RGB1(res, res, pixels) :
				type == 2 ?
					PngTools.build32RRR1(res, res, pixels, off) :
					PngTools.build32RGBA(res, res, pixels);
			writer.write(data);
		}
		else {
			var writer = new JpgWriter(out);
			writer.write({
				width: res,
				height: res,
				quality: UITrait.inst.formatQuality,
				pixels: pixels
			}, type, off);
		}
		#if kha_krom
		Krom.fileSaveBytes(file, out.getBytes().getData());
		#end
	}

	public static function exportTextures(path:String) {
		#if arm_debug
		var timer = iron.system.Time.realTime();
		#end

		var textureSize = Config.getTextureRes();
		var formatQuality = UITrait.inst.formatQuality;
		var f = UIFiles.filename;
		if (f == "") f = "untitled";
		var formatType = UITrait.inst.formatType;
		var bits = UITrait.inst.bitsHandle.position == 0 ? 8 : 16;
		var ext = bits == 16 ? ".exr" : formatType == 0 ? ".png" : ".jpg";
		if (f.endsWith(ext)) f = f.substr(0, f.length - 4);
		var texpaint:Image = null;
		var texpaint_nor:Image = null;
		var texpaint_pack:Image = null;
		var layers = Project.layers;

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
			var selectedLayer = Context.layer;
			texpaint = selectedLayer.texpaint;
			texpaint_nor = selectedLayer.texpaint_nor;
			texpaint_pack = selectedLayer.texpaint_pack;
			if (selectedLayer.objectMask > 0) { // Append object mask name
				f += "_" + Project.paintObjects[selectedLayer.objectMask].name;
			}
		}

		var pixels:Bytes = null;

		if (UITrait.inst.isBase || UITrait.inst.isOpac) {
			pixels = texpaint.getPixels(); // bgra
			if (UITrait.inst.isBase && UITrait.inst.isBaseSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4    , Std.int(Math.pow(pixels.get(i * 4    ) / 255, gamma) * 255));
					pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, gamma) * 255));
					pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, gamma) * 255));
				}
			}
			if (UITrait.inst.isOpac && UITrait.inst.isOpacSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 3, Std.int(Math.pow(pixels.get(i * 4 + 3) / 255, gamma) * 255));
				}
			}

			if (UITrait.inst.isBase) writeTexture(path + "/" + f + "_base" + ext, pixels);
			if (UITrait.inst.isOpac) writeTexture(path + "/" + f + "_opac" + ext, pixels, 2, 3);
		}

		if (UITrait.inst.isNor) {
			pixels = texpaint_nor.getPixels();
			if (UITrait.inst.isNorSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4    , Std.int(Math.pow(pixels.get(i * 4    ) / 255, gamma) * 255));
					pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, gamma) * 255));
					pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, gamma) * 255));
				}
			}
			writeTexture(path + "/" + f + "_nor" + ext, pixels);
		}

		if (UITrait.inst.isOcc || UITrait.inst.isRough || UITrait.inst.isMet || UITrait.inst.isHeight) {
			pixels = texpaint_pack.getPixels(); // occ, rough, met, height
			if (UITrait.inst.isOcc && UITrait.inst.isOccSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 2, Std.int(Math.pow(pixels.get(i * 4 + 2) / 255, gamma) * 255));
				}
			}
			if (UITrait.inst.isRough && UITrait.inst.isRoughSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 1, Std.int(Math.pow(pixels.get(i * 4 + 1) / 255, gamma) * 255));
				}
			}
			if (UITrait.inst.isMet && UITrait.inst.isMetSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4    , Std.int(Math.pow(pixels.get(i * 4    ) / 255, gamma) * 255));
				}
			}
			if (UITrait.inst.isHeight && UITrait.inst.isHeightSpace == 1) {
				for (i in 0...Std.int(pixels.length / 4)) {
					pixels.set(i * 4 + 3, Std.int(Math.pow(pixels.get(i * 4 + 3) / 255, gamma) * 255));
				}
			}

			if (UITrait.inst.outputType == 0) {
				if (UITrait.inst.isOcc) writeTexture(path + "/" + f + "_occ" + ext, pixels, 2, 0);
				if (UITrait.inst.isRough) writeTexture(path + "/" + f + "_rough" + ext, pixels, 2, 1);
				if (UITrait.inst.isMet) writeTexture(path + "/" + f + "_met" + ext, pixels, 2, 2);
			}
			else {
				if (UITrait.inst.isOcc || UITrait.inst.isRough || UITrait.inst.isMet) {
					if (UITrait.inst.outputType == 1) { // Unreal 4
						writeTexture(path + "/" + f + "_orm" + ext, pixels);
					}
					else { // Unity 5
						var pixels2 = Bytes.alloc(pixels.length);
						// 8bit only
						for (i in 0...Std.int(pixels.length / 4)) {
							pixels2.set(i * 4    , pixels.get(i * 4 + 2));
							pixels2.set(i * 4 + 1, pixels.get(i * 4    ));
							// pixels2.set(i * 4 + 2, 0);
							pixels2.set(i * 4 + 3, 255 - pixels.get(i * 4 + 1));
						}
						writeTexture(path + "/" + f + "_mos" + ext, pixels2, 3);
					}
				}
			}

			if (UITrait.inst.isHeight) writeTexture(path + "/" + f + "_height" + ext, pixels, 2, 3);
		}

		// if (UITrait.inst.isEmis) Krom.fileSaveBytes(path + "/tex_emis" + ext, bo.getBytes().getData());
		// if (UITrait.inst.isSubs) Krom.fileSaveBytes(path + "/tex_subs" + ext, bo.getBytes().getData());

		#if arm_debug
		trace("Textures exported in " + (iron.system.Time.realTime() - timer));
		#end
	}

	public static function exportMesh(path:String) {
		if (UITrait.inst.exportMeshFormat == 0) ExportObj.run(path);
		else ExportArm.run(path);
	}
}
