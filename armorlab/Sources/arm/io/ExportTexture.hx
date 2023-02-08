package arm.io;

import haxe.io.Bytes;
import haxe.io.BytesOutput;
import kha.Image;
import iron.Scene;
import arm.render.RenderPathPaint;
import arm.node.MakeMaterial;
import arm.ui.UIHeader;
import arm.ui.UISidebar;
import arm.ui.UIFiles;
import arm.ui.BoxExport;
import arm.sys.Path;
import arm.ProjectFormat;
import arm.Enums;

class ExportTexture {

	static inline var gamma = 1.0 / 2.2;

	public static function run(path: String) {
		runLayers(path, [arm.node.brush.BrushOutputNode.inst]);
		#if krom_ios
		Console.info(tr("Textures exported") + " ('Files/On My iPad/" + Main.title + "')");
		#elseif krom_android
		Console.info(tr("Textures exported") + " ('Files/Internal storage/Pictures/" + Main.title + "')");
		#else
		Console.info(tr("Textures exported"));
		#end
		@:privateAccess UIFiles.lastPath = "";
	}

	static function runLayers(path: String, layers: Array<Dynamic>, objectName = "") {
		var textureSizeX = Config.getTextureResX();
		var textureSizeY = Config.getTextureResY();
		var formatQuality = Context.formatQuality;
		#if (krom_android || krom_ios)
		var f = kha.Window.get(0).title;
		#else
		var f = UIFiles.filename;
		#end
		if (f == "") f = tr("untitled");
		var formatType = Context.formatType;
		var ext = formatType == FormatPng ? ".png" : ".jpg";
		if (f.endsWith(ext)) f = f.substr(0, f.length - 4);

		var texpaint = arm.node.brush.BrushOutputNode.inst.texpaint;
		var texpaint_nor = arm.node.brush.BrushOutputNode.inst.texpaint_nor;
		var texpaint_pack = arm.node.brush.BrushOutputNode.inst.texpaint_pack;
		var pixpaint: Bytes = null;
		var pixpaint_nor: Bytes = null;
		var pixpaint_pack: Bytes = null;
		var preset = BoxExport.preset;
		var pix: Bytes = null;

		for (t in preset.textures) {
			for (c in t.channels) {
				if      ((c == "base_r" || c == "base_g" || c == "base_b" || c == "opac") && pixpaint == null) pixpaint = texpaint.getPixels();
				else if ((c == "nor_r" || c == "nor_g" || c == "nor_g_directx" || c == "nor_b") && pixpaint_nor == null) pixpaint_nor = texpaint_nor.getPixels();
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
				if (pix == null) pix = Bytes.alloc(textureSizeX * textureSizeY * 4);
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
		var format = 0; // RGBA
		if (type == 1) format = 2; // RGB1
		if (type == 2 && off == 0) format = 3; // RRR1
		if (type == 2 && off == 1) format = 4; // GGG1
		if (type == 2 && off == 2) format = 5; // BBB1
		if (type == 2 && off == 3) format = 6; // AAA1

		if (Context.layersDestination == DestinationPacked) {
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

		if (Context.formatType == FormatPng) {
			Krom.writePng(file, pixels.getData(), resX, resY, format);
		}
		else if (Context.formatType == FormatJpg) {
			Krom.writeJpg(file, pixels.getData(), resX, resY, format, Std.int(Context.formatQuality));
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

	static function extractChannel(from: Bytes, fromChannel: Int, to: Bytes, toChannel: Int, mask: Int, linear = true) {
		for (i in 0...Std.int(to.length / 4)) {
			to.set(i * 4 + toChannel, from.get(i * 4 + fromChannel) == mask ? 255 : 0);
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
