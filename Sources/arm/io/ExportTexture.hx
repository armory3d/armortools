package arm.io;

import haxe.io.Bytes;
import haxe.io.BytesOutput;
import kha.Image;
import arm.format.ExrWriter;
import arm.format.JpgWriter;
import arm.format.PngWriter;
import arm.format.PngTools;
import arm.ui.UITrait;
import arm.ui.UIFiles;
import arm.ui.BoxExport;
import arm.Tool;
using StringTools;

class ExportTexture {

	public static function run(path: String) {
		#if arm_debug
		var timer = iron.system.Time.realTime();
		#end

		var udimTiles: Array<String> = [];
		for (l in Project.layers) {
			if (l.objectMask > 0) {
				var name = Project.paintObjects[l.objectMask - 1].name;
				if (name.substr(name.length - 5, 2) == ".1") { // tile.1001
					udimTiles.push(name.substr(name.length - 5));
				}
			}
		}

		if (udimTiles.length > 0 && UITrait.inst.layersExport == 0) {
			for (udimTile in udimTiles) runLayers(path, udimTile);
		}
		else {
			runLayers(path);
		}

		#if arm_debug
		trace("Textures exported in " + (iron.system.Time.realTime() - timer));
		#end

		Log.info("Textures exported.");
	}

	static function runLayers(path: String, udimTile = "") {
		var textureSize = Config.getTextureRes();
		var formatQuality = UITrait.inst.formatQuality;
		var f = UIFiles.filename;
		if (f == "") f = "untitled";
		var formatType = UITrait.inst.formatType;
		var bits = App.bitsHandle.position == Bits8 ? 8 : 16;
		var ext = bits == 16 ? ".exr" : formatType == FormatPng ? ".png" : ".jpg";
		if (f.endsWith(ext)) f = f.substr(0, f.length - 4);
		ext = udimTile + ext;
		var texpaint: Image = null;
		var texpaint_nor: Image = null;
		var texpaint_pack: Image = null;
		var layers = Project.layers;

		// Export visible layers
		if (UITrait.inst.layersExport == 0 && layers.length > 1) {
			Layers.makeTempImg();
			Layers.makeExportImg();
			if (Layers.pipeMerge == null) Layers.makePipe();
			if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();

			// Duplicate base layer
			if (layers[0].visible && udimTile == "") {
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

				if (udimTile != "" && l1.objectMask > 0) {
					if (!Project.paintObjects[l1.objectMask - 1].name.endsWith(udimTile)) continue;
				}

				// Merge into layer0
				Layers.imga.g2.begin(false); // Copy to temp
				Layers.imga.g2.pipeline = Layers.pipeCopy;
				Layers.imga.g2.drawImage(Layers.expa, 0, 0);
				Layers.imga.g2.end();
				Layers.expa.g4.begin();
				Layers.expa.g4.setPipeline(Layers.pipeMerge);
				Layers.expa.g4.setTexture(Layers.tex0, l1.texpaint);
				var empty = iron.RenderPath.active.renderTargets.get("empty_white").image;
				Layers.expa.g4.setTexture(Layers.tex1, empty);
				var hasMask = l1.texpaint_mask != null;
				Layers.expa.g4.setTexture(Layers.texmask, hasMask ? l1.texpaint_mask : empty);
				Layers.expa.g4.setTexture(Layers.texa, Layers.imga);
				Layers.expa.g4.setFloat(Layers.opac, l1.maskOpacity);
				Layers.expa.g4.setInt(Layers.blending, l1.blending);
				Layers.expa.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				Layers.expa.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				Layers.expa.g4.drawIndexedVertices();
				Layers.expa.g4.end();

				Layers.imga.g2.begin(false);
				Layers.imga.g2.pipeline = Layers.pipeCopy;
				Layers.imga.g2.drawImage(Layers.expb, 0, 0);
				Layers.imga.g2.end();
				Layers.expb.g4.begin();
				Layers.expb.g4.setPipeline(Layers.pipeMerge);
				Layers.expb.g4.setTexture(Layers.tex0, l1.texpaint);
				Layers.expb.g4.setTexture(Layers.tex1, l1.texpaint_nor);
				Layers.expb.g4.setTexture(Layers.texmask, empty);
				Layers.expb.g4.setTexture(Layers.texa, Layers.imga);
				Layers.expb.g4.setFloat(Layers.opac, l1.maskOpacity);
				Layers.expa.g4.setInt(Layers.blending, -1);
				Layers.expb.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				Layers.expb.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				Layers.expb.g4.drawIndexedVertices();
				Layers.expb.g4.end();

				Layers.imga.g2.begin(false);
				Layers.imga.g2.pipeline = Layers.pipeCopy;
				Layers.imga.g2.drawImage(Layers.expc, 0, 0);
				Layers.imga.g2.end();
				Layers.expc.g4.begin();
				Layers.expc.g4.setPipeline(Layers.pipeMerge);
				Layers.expc.g4.setTexture(Layers.tex0, l1.texpaint);
				Layers.expc.g4.setTexture(Layers.tex1, l1.texpaint_pack);
				Layers.expc.g4.setTexture(Layers.texmask, empty);
				Layers.expc.g4.setTexture(Layers.texa, Layers.imga);
				Layers.expc.g4.setFloat(Layers.opac, l1.maskOpacity);
				Layers.expa.g4.setInt(Layers.blending, -1);
				Layers.expc.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				Layers.expc.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				Layers.expc.g4.drawIndexedVertices();
				Layers.expc.g4.end();
			}

			texpaint = Layers.expa;
			texpaint_nor = Layers.expb;
			texpaint_pack = Layers.expc;
		}
		else { // Export selected layer
			var selectedLayer = Context.layer;
			texpaint = selectedLayer.texpaint;
			texpaint_nor = selectedLayer.texpaint_nor;
			texpaint_pack = selectedLayer.texpaint_pack;
			if (selectedLayer.objectMask > 0) { // Append object mask name
				f += "_" + Project.paintObjects[selectedLayer.objectMask].name;
			}
		}

		var pixpaint: Bytes = null;
		var pixpaint_nor: Bytes = null;
		var pixpaint_pack: Bytes = null;
		var preset = BoxExport.preset;
		var pix: Bytes = null;

		for (t in preset.textures) {
			for (c in t.channels) {
				if      ((c == "base_r" || c == "base_g" || c == "base_b" || c == "opac") && pixpaint == null) pixpaint = texpaint.getPixels();
				else if ((c == "nor_r" || c == "nor_g" || c == "nor_b") && pixpaint_nor == null) pixpaint_nor = texpaint_nor.getPixels();
				else if ((c == "occ" || c == "rough" || c == "metal" || c == "height" || c == "smooth") && pixpaint_pack == null) pixpaint_pack = texpaint_pack.getPixels();
			}
		}

		for (t in preset.textures) {
			var c = t.channels;
			var singleChannel = c[0] == c[1] && c[1] == c[2] && c[3] == "1.0";
			if (c[0] == "base_r" && c[1] == "base_g" && c[2] == "base_b" && c[3] == "1.0") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint, 1);
			}
			else if (c[0] == "nor_r" && c[1] == "nor_g" && c[2] == "nor_b" && c[3] == "1.0") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint_nor, 1);
			}
			else if (c[0] == "occ" && c[1] == "rough" && c[2] == "metal" && c[3] == "1.0") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint_pack, 1);
			}
			else if (singleChannel && c[0] == "occ") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint_pack, 2, 0);
			}
			else if (singleChannel && c[0] == "rough") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint_pack, 2, 1);
			}
			else if (singleChannel && c[0] == "metal") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint_pack, 2, 2);
			}
			else if (singleChannel && c[0] == "height") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint_pack, 2, 3);
			}
			else if (singleChannel && c[0] == "opac") {
				writeTexture(path + "/" + f + "_" + t.name + ext, pixpaint, 2, 3);
			}
			else {
				if (pix == null) pix = Bytes.alloc(textureSize * textureSize * 4 * Std.int(bits / 8));
				for (i in 0...4) {
					var c = t.channels[i];
					if      (c == "base_r") copyChannel(pixpaint, 0, pix, i); // copyChannelGamma
					else if (c == "base_g") copyChannel(pixpaint, 1, pix, i); // copyChannelGamma
					else if (c == "base_b") copyChannel(pixpaint, 2, pix, i); // copyChannelGamma
					else if (c == "height") copyChannel(pixpaint_pack, 4, pix, i);
					else if (c == "metal") copyChannel(pixpaint_pack, 3, pix, i);
					else if (c == "nor_r") copyChannel(pixpaint_nor, 0, pix, i);
					else if (c == "nor_g") copyChannel(pixpaint_nor, 1, pix, i);
					else if (c == "nor_b") copyChannel(pixpaint_nor, 2, pix, i);
					else if (c == "occ") copyChannel(pixpaint_pack, 0, pix, i);
					else if (c == "opac") copyChannel(pixpaint, 3, pix, i);
					else if (c == "rough") copyChannel(pixpaint_pack, 1, pix, i);
					else if (c == "smooth") copyChannelInv(pixpaint_pack, 1, pix, i);
					else if (c == "0.0") setChannel(0, pix, i);
					else if (c == "1.0") setChannel(255, pix, i);
				}
				writeTexture(path + "/" + f + "_" + t.name + ext, pix);
			}
		}
	}

	static function writeTexture(file: String, pixels: Bytes, type = 1, off = 0) {
		var out = new BytesOutput();
		var res = Config.getTextureRes();
		var bitsHandle = App.bitsHandle.position;
		var bits = bitsHandle == Bits8 ? 8 : bitsHandle == Bits16 ? 16 : 32;
		if (bits > 8) { // 16/32bit
			var writer = new ExrWriter(out, res, res, pixels, bits, type, off);
		}
		else if (UITrait.inst.formatType == FormatPng) {
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
		Krom.fileSaveBytes(file, out.getBytes().getData());
	}

	static function copyChannel(from: Bytes, fromChannel: Int, to: Bytes, toChannel: Int) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, from.get(i * 4 + fromChannel));
		}
	}

	static inline var gamma = 1.0 / 2.2;
	static function copyChannelGamma(from: Bytes, fromChannel: Int, to: Bytes, toChannel: Int) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, Std.int(Math.pow(from.get(i * 4 + fromChannel) / 255, gamma) * 255));
		}
	}

	static function copyChannelInv(from: Bytes, fromChannel: Int, to: Bytes, toChannel: Int) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, 255 - from.get(i * 4 + fromChannel));
		}
	}

	static function setChannel(value: Int, to: Bytes, toChannel: Int) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, value);
		}
	}
}

typedef TExportPreset = {
	public var textures: Array<TExportPresetTexture>;
}

typedef TExportPresetTexture = {
	public var name: String;
	public var channels: Array<String>;
}
