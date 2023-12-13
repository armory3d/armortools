package arm.io;

#if (is_paint || is_lab)

import iron.System;
import iron.MeshObject;
import iron.ConstData;
import iron.RenderPath;
import iron.Data;
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
		UIFiles.lastPath = "";
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
		var planeo: MeshObject = cast Scene.active.getChild(".Plane");
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
		var f = System.title;
		#else
		var f = UIFiles.filename;
		#end
		if (f == "") f = tr("untitled");
		var formatType = Context.raw.formatType;
		var bits = Base.bitsHandle.position == Bits8 ? 8 : 16;
		var ext = bits == 16 ? ".exr" : formatType == FormatPng ? ".png" : ".jpg";
		if (f.endsWith(ext)) f = f.substr(0, f.length - 4);

		#if is_paint
		var isUdim = Context.raw.layersExport == ExportPerUdimTile;
		if (isUdim) ext = objectName + ext;

		Base.makeTempImg();
		Base.makeExportImg();
		if (Base.pipeMerge == null) Base.makePipe();
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		var empty = RenderPath.active.renderTargets.get("empty_white").image;

		// Append object mask name
		var exportSelected = Context.raw.layersExport == ExportSelected;
		if (exportSelected && layers[0].getObjectMask() > 0) {
			f += "_" + Project.paintObjects[layers[0].getObjectMask() - 1].name;
		}
		if (!isUdim && !exportSelected && objectName != "") {
			f += "_" + objectName;
		}

		// Clear export layer
		Base.expa.g4.begin();
		Base.expa.g4.clear(Color.fromFloats(0.0, 0.0, 0.0, 0.0));
		Base.expa.g4.end();
		Base.expb.g4.begin();
		Base.expb.g4.clear(Color.fromFloats(0.5, 0.5, 1.0, 0.0));
		Base.expb.g4.end();
		Base.expc.g4.begin();
		Base.expc.g4.clear(Color.fromFloats(1.0, 0.0, 0.0, 0.0));
		Base.expc.g4.end();

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
					Base.makeTempMaskImg();
					Base.tempMaskImage.g2.begin(true, 0x00000000);
					Base.tempMaskImage.g2.end();
					var l1 = { texpaint: Base.tempMaskImage };
					for (i in 0...l1masks.length) {
						Base.mergeLayer(untyped l1, l1masks[i]);
					}
					mask = Base.tempMaskImage;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				Base.tempImage.g2.begin(false); // Copy to temp
				Base.tempImage.g2.pipeline = Base.pipeCopy;
				Base.tempImage.g2.drawImage(Base.expa, 0, 0);
				Base.tempImage.g2.pipeline = null;
				Base.tempImage.g2.end();

				Base.expa.g4.begin();
				Base.expa.g4.setPipeline(Base.pipeMerge);
				Base.expa.g4.setTexture(Base.tex0, l1.texpaint);
				Base.expa.g4.setTexture(Base.tex1, empty);
				Base.expa.g4.setTexture(Base.texmask, mask);
				Base.expa.g4.setTexture(Base.texa, Base.tempImage);
				Base.expa.g4.setFloat(Base.opac, l1.getOpacity());
				Base.expa.g4.setInt(Base.blending, layers.length > 1 ? l1.blending : 0);
				Base.expa.g4.setVertexBuffer(ConstData.screenAlignedVB);
				Base.expa.g4.setIndexBuffer(ConstData.screenAlignedIB);
				Base.expa.g4.drawIndexedVertices();
				Base.expa.g4.end();
			}

			if (l1.paintNor) {
				Base.tempImage.g2.begin(false);
				Base.tempImage.g2.pipeline = Base.pipeCopy;
				Base.tempImage.g2.drawImage(Base.expb, 0, 0);
				Base.tempImage.g2.pipeline = null;
				Base.tempImage.g2.end();

				Base.expb.g4.begin();
				Base.expb.g4.setPipeline(Base.pipeMerge);
				Base.expb.g4.setTexture(Base.tex0, l1.texpaint);
				Base.expb.g4.setTexture(Base.tex1, l1.texpaint_nor);
				Base.expb.g4.setTexture(Base.texmask, mask);
				Base.expb.g4.setTexture(Base.texa, Base.tempImage);
				Base.expb.g4.setFloat(Base.opac, l1.getOpacity());
				Base.expb.g4.setInt(Base.blending, l1.paintNorBlend ? -2 : -1);
				Base.expb.g4.setVertexBuffer(ConstData.screenAlignedVB);
				Base.expb.g4.setIndexBuffer(ConstData.screenAlignedIB);
				Base.expb.g4.drawIndexedVertices();
				Base.expb.g4.end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				Base.tempImage.g2.begin(false);
				Base.tempImage.g2.pipeline = Base.pipeCopy;
				Base.tempImage.g2.drawImage(Base.expc, 0, 0);
				Base.tempImage.g2.pipeline = null;
				Base.tempImage.g2.end();

				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					Base.commandsMergePack(Base.pipeMerge, Base.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) Base.commandsMergePack(Base.pipeMergeR, Base.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintRough) Base.commandsMergePack(Base.pipeMergeG, Base.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
					if (l1.paintMet) Base.commandsMergePack(Base.pipeMergeB, Base.expc, l1.texpaint, l1.texpaint_pack, l1.getOpacity(), mask);
				}
			}
		}

		#if krom_metal
		// Flush command list
		Base.expa.g2.begin(false);
		Base.expa.g2.end();
		Base.expb.g2.begin(false);
		Base.expb.g2.end();
		Base.expc.g2.begin(false);
		Base.expc.g2.end();
		#end
		#end

		#if is_paint
		var texpaint = Base.expa;
		var texpaint_nor = Base.expb;
		var texpaint_pack = Base.expc;
		#end

		#if is_lab
		var texpaint = arm.logic.BrushOutputNode.inst.texpaint;
		var texpaint_nor = arm.logic.BrushOutputNode.inst.texpaint_nor;
		var texpaint_pack = arm.logic.BrushOutputNode.inst.texpaint_pack;
		#end

		var pixpaint: js.lib.ArrayBuffer = null;
		var pixpaint_nor: js.lib.ArrayBuffer = null;
		var pixpaint_pack: js.lib.ArrayBuffer = null;
		var preset = BoxExport.preset;
		var pix: js.lib.ArrayBuffer = null;

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
				if (pix == null) pix = new js.lib.ArrayBuffer(textureSizeX * textureSizeY * 4 * Std.int(bits / 8));
				for (i in 0...4) {
					var c = t.channels[i];
					if      (c == "base_r") copyChannel(new js.lib.DataView(pixpaint), 0, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "base_g") copyChannel(new js.lib.DataView(pixpaint), 1, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "base_b") copyChannel(new js.lib.DataView(pixpaint), 2, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "height") copyChannel(new js.lib.DataView(pixpaint_pack), 3, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "metal") copyChannel(new js.lib.DataView(pixpaint_pack), 2, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_r") copyChannel(new js.lib.DataView(pixpaint_nor), 0, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_g") copyChannel(new js.lib.DataView(pixpaint_nor), 1, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_g_directx") copyChannelInv(new js.lib.DataView(pixpaint_nor), 1, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_b") copyChannel(new js.lib.DataView(pixpaint_nor), 2, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "occ") copyChannel(new js.lib.DataView(pixpaint_pack), 0, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "opac") copyChannel(new js.lib.DataView(pixpaint), 3, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "rough") copyChannel(new js.lib.DataView(pixpaint_pack), 1, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "smooth") copyChannelInv(new js.lib.DataView(pixpaint_pack), 1, new js.lib.DataView(pix), i, t.color_space == "linear");
					else if (c == "emis") extractChannel(new js.lib.DataView(pixpaint_nor), 3, new js.lib.DataView(pix), i, 3, 1, t.color_space == "linear");
					else if (c == "subs") extractChannel(new js.lib.DataView(pixpaint_nor), 3, new js.lib.DataView(pix), i, 3, 2, t.color_space == "linear");
					else if (c == "0.0") setChannel(0, new js.lib.DataView(pix), i);
					else if (c == "1.0") setChannel(255, new js.lib.DataView(pix), i);
				}
				writeTexture(path + Path.sep + f + tex_name + ext, pix, 3);
			}
		}

		// Release staging memory allocated in Image.getPixels()
		texpaint.pixels = null;
		texpaint_nor.pixels = null;
		texpaint_pack.pixels = null;
	}

	static function writeTexture(file: String, pixels: js.lib.ArrayBuffer, type = 1, off = 0) {
		var resX = Config.getTextureResX();
		var resY = Config.getTextureResY();
		var bitsHandle = Base.bitsHandle.position;
		var bits = bitsHandle == Bits8 ? 8 : bitsHandle == Bits16 ? 16 : 32;
		var format = 0; // RGBA
		if (type == 1) format = 2; // RGB1
		if (type == 2 && off == 0) format = 3; // RRR1
		if (type == 2 && off == 1) format = 4; // GGG1
		if (type == 2 && off == 2) format = 5; // BBB1
		if (type == 2 && off == 3) format = 6; // AAA1

		if (Context.raw.layersDestination == DestinationPacked) {
			var image = Image.fromBytes(pixels, resX, resY);
			Data.cachedImages.set(file, image);
			var ar = file.split(Path.sep);
			var name = ar[ar.length - 1];
			var asset: TAsset = {name: name, file: file, id: Project.assetId++};
			Project.assets.push(asset);
			if (Project.raw.assets == null) Project.raw.assets = [];
			Project.raw.assets.push(asset.file);
			Project.assetNames.push(asset.name);
			Project.assetMap.set(asset.id, image);
			ExportArm.packAssets(Project.raw, [asset]);
			return;
		}

		if (bits == 8 && Context.raw.formatType == FormatPng) {
			Krom.writePng(file, pixels, resX, resY, format);
		}
		else if (bits == 8 && Context.raw.formatType == FormatJpg) {
			Krom.writeJpg(file, pixels, resX, resY, format, Std.int(Context.raw.formatQuality));
		}
		else { // Exr
			var b = arm.format.ExrWriter.run(resX, resY, pixels, bits, type, off);
			Krom.fileSaveBytes(file, b, b.byteLength);
		}
	}

	static function copyChannel(from: js.lib.DataView, fromChannel: Int, to: js.lib.DataView, toChannel: Int, linear = true) {
		for (i in 0...Std.int(to.byteLength / 4)) {
			to.setUint8(i * 4 + toChannel, from.getUint8(i * 4 + fromChannel));
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function copyChannelInv(from: js.lib.DataView, fromChannel: Int, to: js.lib.DataView, toChannel: Int, linear = true) {
		for (i in 0...Std.int(to.byteLength / 4)) {
			to.setUint8(i * 4 + toChannel, 255 - from.getUint8(i * 4 + fromChannel));
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function extractChannel(from: js.lib.DataView, fromChannel: Int, to: js.lib.DataView, toChannel: Int, step: Int, mask: Int, linear = true) {
		for (i in 0...Std.int(to.byteLength / 4)) {
			to.setUint8(i * 4 + toChannel, from.getUint8(i * 4 + fromChannel) % step == mask ? 255 : 0);
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function setChannel(value: Int, to: js.lib.DataView, toChannel: Int, linear = true) {
		for (i in 0...Std.int(to.byteLength / 4)) {
			to.setUint8(i * 4 + toChannel, value);
		}
		if (!linear) toSrgb(to, toChannel);
	}

	static function toSrgb(to: js.lib.DataView, toChannel: Int) {
		for (i in 0...Std.int(to.byteLength / 4)) {
			to.setUint8(i * 4 + toChannel, Std.int(Math.pow(to.getUint8(i * 4 + toChannel) / 255, gamma) * 255));
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
