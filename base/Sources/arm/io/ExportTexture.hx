package arm.io;

#if (is_paint || is_lab)

import haxe.io.Bytes;
import haxe.io.BytesOutput;
import kha.Image;
import arm.ui.UIFiles;
import arm.ui.BoxExport;
import arm.sys.Path;
import arm.ProjectFormat;
#if is_paint
import iron.Scene;
import arm.render.RenderPathPaint;
import arm.shader.MakeMaterial;
import arm.ui.UIHeader;
#end
#if is_paint
import arm.data.LayerSlot;
#end

class ExportTexture {

	static inline var gamma = 1.0 / 2.2;

	public static function run(path: String, bakeMaterial = false) {

		#if is_paint
		if (bakeMaterial) {
			runBakeMaterial(path);
		}
		else if (Context.raw.layersExport == ExportPerUdimTile) {
			var udimTiles: Array<String> = [];
			for (l in Project.layers) {
				if (l.getObjectMask() > 0) {
					var name = Project.paintObjects[l.getObjectMask() - 1].name;
					if (name.substr(name.length - 5, 2) == ".1") { // tile.1001
						udimTiles.push(name.substr(name.length - 5));
					}
				}
			}
			if (udimTiles.length > 0) {
				for (udimTile in udimTiles) runLayers(path, Project.layers, udimTile);
			}
			else runLayers(path, Project.layers);
		}
		else if (Context.raw.layersExport == ExportPerObject) {
			var objectNames: Array<String> = [];
			for (l in Project.layers) {
				if (l.getObjectMask() > 0) {
					var name = Project.paintObjects[l.getObjectMask() - 1].name;
					if (objectNames.indexOf(name) == -1) {
						objectNames.push(name);
					}
				}
			}
			if (objectNames.length > 0) {
				for (name in objectNames) runLayers(path, Project.layers, name);
			}
			else runLayers(path, Project.layers);
		}
		else { // Visible or selected
			var atlasExport = false;
			if (Project.atlasObjects != null) {
				for (i in 1...Project.atlasObjects.length) {
					if (Project.atlasObjects[i - 1] != Project.atlasObjects[i]) {
						atlasExport = true;
						break;
					}
				}
			}
			if (atlasExport) {
				for (atlasIndex in 0...Project.atlasObjects.length) {
					var layers: Array<LayerSlot> = [];
					for (objectIndex in 0...Project.atlasObjects.length) {
						if (Project.atlasObjects[objectIndex] == atlasIndex) {
							for (l in Project.layers) {
								if (l.getObjectMask() == 0 /* shared object */ || l.getObjectMask() - 1 == objectIndex) layers.push(l);
							}
						}
					}
					if (layers.length > 0) {
						runLayers(path, layers, Project.atlasNames[atlasIndex]);
					}
				}
			}
			else runLayers(path, Context.raw.layersExport == ExportSelected ? (Context.raw.layer.isGroup() ? Context.raw.layer.getChildren() : [Context.raw.layer]) : Project.layers);
		}
		#end

		#if is_lab
		runLayers(path, [arm.logic.BrushOutputNode.inst]);
		#end

		#if krom_ios
		Console.info(tr("Textures exported") + " ('Files/On My iPad/" + Manifest.title + "')");
		#elseif krom_android
		Console.info(tr("Textures exported") + " ('Files/Internal storage/Pictures/" + Manifest.title + "')");
		#else
		Console.info(tr("Textures exported"));
		#end
		@:privateAccess UIFiles.lastPath = "";
	}

	#if is_paint
	static function runBakeMaterial(path: String) {
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = new arm.data.LayerSlot("_live");
		}

		var _tool = Context.raw.tool;
		Context.raw.tool = ToolFill;
		MakeMaterial.parsePaintMaterial();
		var _paintObject = Context.raw.paintObject;
		var planeo: iron.object.MeshObject = cast Scene.active.getChild(".Plane");
		planeo.visible = true;
		Context.raw.paintObject = planeo;
		Context.raw.pdirty = 1;
		RenderPathPaint.useLiveLayer(true);
		RenderPathPaint.commandsPaint(false);
		RenderPathPaint.useLiveLayer(false);
		Context.raw.tool = _tool;
		MakeMaterial.parsePaintMaterial();
		Context.raw.pdirty = 0;
		planeo.visible = false;
		Context.raw.paintObject = _paintObject;

		runLayers(path, [RenderPathPaint.liveLayer], "", true);
	}
	#end

	#if is_paint
	static function runLayers(path: String, layers: Array<LayerSlot>, objectName = "", bakeMaterial = false) {
	#end

	#if is_lab
	static function runLayers(path: String, layers: Array<Dynamic>, objectName = "") {
	#end

		var textureSizeX = Config.getTextureResX();
		var textureSizeY = Config.getTextureResY();
		var formatQuality = Context.raw.formatQuality;
		#if (krom_android || krom_ios)
		var f = kha.Window.get(0).title;
		#else
		var f = UIFiles.filename;
		#end
		if (f == "") f = tr("untitled");
		var formatType = Context.raw.formatType;
		var bits = App.bitsHandle.position == Bits8 ? 8 : 16;
		var ext = bits == 16 ? ".exr" : formatType == FormatPng ? ".png" : ".jpg";
		if (f.endsWith(ext)) f = f.substr(0, f.length - 4);

		#if is_paint
		var isUdim = Context.raw.layersExport == ExportPerUdimTile;
		if (isUdim) ext = objectName + ext;

		App.makeTempImg();
		App.makeExportImg();
		if (App.pipeMerge == null) App.makePipe();
		if (iron.data.ConstData.screenAlignedVB == null) iron.data.ConstData.createScreenAlignedData();
		var empty = iron.RenderPath.active.renderTargets.get("empty_white").image;

		// Append object mask name
		var exportSelected = Context.raw.layersExport == ExportSelected;
		if (exportSelected && layers[0].getObjectMask() > 0) {
			f += "_" + Project.paintObjects[layers[0].getObjectMask() - 1].name;
		}
		if (!isUdim && !exportSelected && objectName != "") {
			f += "_" + objectName;
		}

		// Clear export layer
		App.expa.g4.begin();
		App.expa.g4.clear(kha.Color.fromFloats(0.0, 0.0, 0.0, 0.0));
		App.expa.g4.end();
		App.expb.g4.begin();
		App.expb.g4.clear(kha.Color.fromFloats(0.5, 0.5, 1.0, 0.0));
		App.expb.g4.end();
		App.expc.g4.begin();
		App.expc.g4.clear(kha.Color.fromFloats(1.0, 0.0, 0.0, 0.0));
		App.expc.g4.end();

		// Flatten layers
		for (l1 in layers) {
			if (!exportSelected && !l1.isVisible()) continue;
			if (!l1.isLayer()) continue;

			if (objectName != "" && l1.getObjectMask() > 0) {
				if (isUdim && !Project.paintObjects[l1.getObjectMask() - 1].name.endsWith(objectName)) continue;
				var perObject = Context.raw.layersExport == ExportPerObject;
				if (perObject && Project.paintObjects[l1.getObjectMask() - 1].name != objectName) continue;
			}

			var mask = empty;
			var l1masks = l1.getMasks();
			if (l1masks != null && !bakeMaterial) {
				if (l1masks.length > 1) {
					App.makeTempMaskImg();
					App.tempMaskImage.g2.begin(true, 0x00000000);
					App.tempMaskImage.g2.end();
					var l1 = { texpaint: App.tempMaskImage };
					for (i in 0...l1masks.length) {
						App.mergeLayer(untyped l1, l1masks[i]);
					}
					mask = App.tempMaskImage;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				App.tempImage.g2.begin(false); // Copy to temp
				App.tempImage.g2.pipeline = App.pipeCopy;
				App.tempImage.g2.drawImage(App.expa, 0, 0);
				App.tempImage.g2.pipeline = null;
				App.tempImage.g2.end();

				App.expa.g4.begin();
				App.expa.g4.setPipeline(App.pipeMerge);
				App.expa.g4.setTexture(App.tex0, l1.texpaint);
				App.expa.g4.setTexture(App.tex1, empty);
				App.expa.g4.setTexture(App.texmask, mask);
				App.expa.g4.setTexture(App.texa, App.tempImage);
				App.expa.g4.setFloat(App.opac, l1.getOpacity());
				App.expa.g4.setInt(App.blending, layers.length > 1 ? l1.blending : 0);
				App.expa.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				App.expa.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				App.expa.g4.drawIndexedVertices();
				App.expa.g4.end();
			}

			if (l1.paintNor) {
				App.tempImage.g2.begin(false);
				App.tempImage.g2.pipeline = App.pipeCopy;
				App.tempImage.g2.drawImage(App.expb, 0, 0);
				App.tempImage.g2.pipeline = null;
				App.tempImage.g2.end();

				App.expb.g4.begin();
				App.expb.g4.setPipeline(App.pipeMerge);
				App.expb.g4.setTexture(App.tex0, l1.texpaint);
				App.expb.g4.setTexture(App.tex1, l1.texpaint_nor);
				App.expb.g4.setTexture(App.texmask, mask);
				App.expb.g4.setTexture(App.texa, App.tempImage);
				App.expb.g4.setFloat(App.opac, l1.getOpacity());
				App.expb.g4.setInt(App.blending, l1.paintNorBlend ? -2 : -1);
				App.expb.g4.setVertexBuffer(iron.data.ConstData.screenAlignedVB);
				App.expb.g4.setIndexBuffer(iron.data.ConstData.screenAlignedIB);
				App.expb.g4.drawIndexedVertices();
				App.expb.g4.end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				App.tempImage.g2.begin(false);
				App.tempImage.g2.pipeline = App.pipeCopy;
				App.tempImage.g2.drawImage(App.expc, 0, 0);
				App.tempImage.g2.pipeline = null;
				App.tempImage.g2.end();

				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					App.commandsMergePack(App.pipeMerge, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) App.commandsMergePack(App.pipeMergeR, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintRough) App.commandsMergePack(App.pipeMergeG, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintMet) App.commandsMergePack(App.pipeMergeB, App.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
				}
			}
		}

		#if kha_metal
		// Flush command list
		App.expa.g2.begin(false);
		App.expa.g2.end();
		App.expb.g2.begin(false);
		App.expb.g2.end();
		App.expc.g2.begin(false);
		App.expc.g2.end();
		#end
		#end

		#if is_paint
		var texpaint = App.expa;
		var texpaint_nor = App.expb;
		var texpaint_pack = App.expc;
		#end

		#if is_lab
		var texpaint = arm.logic.BrushOutputNode.inst.texpaint;
		var texpaint_nor = arm.logic.BrushOutputNode.inst.texpaint_nor;
		var texpaint_pack = arm.logic.BrushOutputNode.inst.texpaint_pack;
		#end

		var pixpaint: Bytes = null;
		var pixpaint_nor: Bytes = null;
		var pixpaint_pack: Bytes = null;
		var preset = BoxExport.preset;
		var pix: Bytes = null;

		for (t in preset.textures) {
			for (c in t.channels) {
				if      ((c == "base_r" || c == "base_g" || c == "base_b" || c == "opac") && pixpaint == null) pixpaint = texpaint.getPixels();
				else if ((c == "nor_r" || c == "nor_g" || c == "nor_g_directx" || c == "nor_b" || c == "emis" || c == "subs") && pixpaint_nor == null) pixpaint_nor = texpaint_nor.getPixels();
				else if ((c == "occ" || c == "rough" || c == "metal" || c == "height" || c == "smooth") && pixpaint_pack == null) pixpaint_pack = texpaint_pack.getPixels();
			}
		}

		for (t in preset.textures) {
			var c = t.channels;
			var tex_name = t.name != "" ? "_" + t.name : "";
			var singleChannel = c[0] == c[1] && c[1] == c[2] && c[3] == "1.0";
			if (c[0] == "base_r" && c[1] == "base_g" && c[2] == "base_b" && c[3] == "1.0" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint, 1);
			}
			else if (c[0] == "nor_r" && c[1] == "nor_g" && c[2] == "nor_b" && c[3] == "1.0" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_nor, 1);
			}
			else if (c[0] == "occ" && c[1] == "rough" && c[2] == "metal" && c[3] == "1.0" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 1);
			}
			else if (singleChannel && c[0] == "occ" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 0);
			}
			else if (singleChannel && c[0] == "rough" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 1);
			}
			else if (singleChannel && c[0] == "metal" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 2);
			}
			else if (singleChannel && c[0] == "height" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 3);
			}
			else if (singleChannel && c[0] == "opac" && t.color_space == "linear") {
				writeTexture(path + Path.sep + f + tex_name + ext, pixpaint, 2, 3);
			}
			else {
				if (pix == null) pix = Bytes.alloc(textureSizeX * textureSizeY * 4 * Std.int(bits / 8));
				for (i in 0...4) {
					var c = t.channels[i];
					if      (c == "base_r") copyChannel(pixpaint, 0, pix, i, t.color_space == "linear");
					else if (c == "base_g") copyChannel(pixpaint, 1, pix, i, t.color_space == "linear");
					else if (c == "base_b") copyChannel(pixpaint, 2, pix, i, t.color_space == "linear");
					else if (c == "height") copyChannel(pixpaint_pack, 3, pix, i, t.color_space == "linear");
					else if (c == "metal") copyChannel(pixpaint_pack, 2, pix, i, t.color_space == "linear");
					else if (c == "nor_r") copyChannel(pixpaint_nor, 0, pix, i, t.color_space == "linear");
					else if (c == "nor_g") copyChannel(pixpaint_nor, 1, pix, i, t.color_space == "linear");
					else if (c == "nor_g_directx") copyChannelInv(pixpaint_nor, 1, pix, i, t.color_space == "linear");
					else if (c == "nor_b") copyChannel(pixpaint_nor, 2, pix, i, t.color_space == "linear");
					else if (c == "occ") copyChannel(pixpaint_pack, 0, pix, i, t.color_space == "linear");
					else if (c == "opac") copyChannel(pixpaint, 3, pix, i, t.color_space == "linear");
					else if (c == "rough") copyChannel(pixpaint_pack, 1, pix, i, t.color_space == "linear");
					else if (c == "smooth") copyChannelInv(pixpaint_pack, 1, pix, i, t.color_space == "linear");
					else if (c == "emis") extractChannel(pixpaint_nor, 3, pix, i, 3, 1, t.color_space == "linear");
					else if (c == "subs") extractChannel(pixpaint_nor, 3, pix, i, 3, 2, t.color_space == "linear");
					else if (c == "0.0") setChannel(0, pix, i);
					else if (c == "1.0") setChannel(255, pix, i);
				}
				writeTexture(path + Path.sep + f + tex_name + ext, pix, 3);
			}
		}
	}

	static function writeTexture(file: String, pixels: Bytes, type = 1, off = 0) {
		var resX = Config.getTextureResX();
		var resY = Config.getTextureResY();
		var bitsHandle = App.bitsHandle.position;
		var bits = bitsHandle == Bits8 ? 8 : bitsHandle == Bits16 ? 16 : 32;
		var format = 0; // RGBA
		if (type == 1) format = 2; // RGB1
		if (type == 2 && off == 0) format = 3; // RRR1
		if (type == 2 && off == 1) format = 4; // GGG1
		if (type == 2 && off == 2) format = 5; // BBB1
		if (type == 2 && off == 3) format = 6; // AAA1

		if (Context.raw.layersDestination == DestinationPacked) {
			var image = kha.Image.fromBytes(pixels, resX, resY);
			iron.data.Data.cachedImages.set(file, image);
			var ar = file.split(Path.sep);
			var name = ar[ar.length - 1];
			var asset: TAsset = {name: name, file: file, id: Project.assetId++};
			Project.assets.push(asset);
			if (Project.raw.assets == null) Project.raw.assets = [];
			Project.raw.assets.push(asset.file);
			Project.assetNames.push(asset.name);
			Project.assetMap.set(asset.id, image);
			@:privateAccess ExportArm.packAssets(Project.raw, [asset]);
			return;
		}

		if (bits == 8 && Context.raw.formatType == FormatPng) {
			Krom.writePng(file, pixels.getData(), resX, resY, format);
		}
		else if (bits == 8 && Context.raw.formatType == FormatJpg) {
			Krom.writeJpg(file, pixels.getData(), resX, resY, format, Std.int(Context.raw.formatQuality));
		}
		else { // Exr
			var out = new BytesOutput();
			var writer = new arm.format.ExrWriter(out, resX, resY, pixels, bits, type, off);
			Krom.fileSaveBytes(file, out.getBytes().getData(), out.getBytes().length);
		}
	}

	static function copyChannel(from: Bytes, fromChannel: Int, to: Bytes, toChannel: Int, linear = true) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, from.get(i * 4 + fromChannel));
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function copyChannelInv(from: Bytes, fromChannel: Int, to: Bytes, toChannel: Int, linear = true) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, 255 - from.get(i * 4 + fromChannel));
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function extractChannel(from: Bytes, fromChannel: Int, to: Bytes, toChannel: Int, step: Int, mask: Int, linear = true) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, from.get(i * 4 + fromChannel) % step == mask ? 255 : 0);
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function setChannel(value: Int, to: Bytes, toChannel: Int, linear = true) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, value);
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function toSrgb(to: Bytes, toChannel: Int) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, Std.int(Math.pow(to.get(i * 4 + toChannel) / 255, gamma) * 255));
		}
	}
}

typedef TExportPreset = {
	public var textures: Array<TExportPresetTexture>;
}

typedef TExportPresetTexture = {
	public var name: String;
	public var channels: Array<String>;
	public var color_space: String;
}

#end
