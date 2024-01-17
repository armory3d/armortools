
///if (is_paint || is_lab)

class ExportTexture {

	static gamma = 1.0 / 2.2;

	static run = (path: string, bakeMaterial = false) => {

		///if is_paint
		if (bakeMaterial) {
			ExportTexture.runBakeMaterial(path);
		}
		else if (Context.raw.layersExport == ExportMode.ExportPerUdimTile) {
			let udimTiles: string[] = [];
			for (let l of Project.layers) {
				if (l.getObjectMask() > 0) {
					let name = Project.paintObjects[l.getObjectMask() - 1].name;
					if (name.substr(name.length - 5, 2) == ".1") { // tile.1001
						udimTiles.push(name.substr(name.length - 5));
					}
				}
			}
			if (udimTiles.length > 0) {
				for (let udimTile of udimTiles) ExportTexture.runLayers(path, Project.layers, udimTile);
			}
			else ExportTexture.runLayers(path, Project.layers);
		}
		else if (Context.raw.layersExport == ExportMode.ExportPerObject) {
			let objectNames: string[] = [];
			for (let l of Project.layers) {
				if (l.getObjectMask() > 0) {
					let name = Project.paintObjects[l.getObjectMask() - 1].name;
					if (objectNames.indexOf(name) == -1) {
						objectNames.push(name);
					}
				}
			}
			if (objectNames.length > 0) {
				for (let name of objectNames) ExportTexture.runLayers(path, Project.layers, name);
			}
			else ExportTexture.runLayers(path, Project.layers);
		}
		else { // Visible or selected
			let atlasExport = false;
			if (Project.atlasObjects != null) {
				for (let i = 1; i < Project.atlasObjects.length; ++i) {
					if (Project.atlasObjects[i - 1] != Project.atlasObjects[i]) {
						atlasExport = true;
						break;
					}
				}
			}
			if (atlasExport) {
				for (let atlasIndex = 0; atlasIndex < Project.atlasObjects.length; ++atlasIndex) {
					let layers: SlotLayer[] = [];
					for (let objectIndex = 0; objectIndex < Project.atlasObjects.length; ++objectIndex) {
						if (Project.atlasObjects[objectIndex] == atlasIndex) {
							for (let l of Project.layers) {
								if (l.getObjectMask() == 0 /* shared object */ || l.getObjectMask() - 1 == objectIndex) layers.push(l);
							}
						}
					}
					if (layers.length > 0) {
						ExportTexture.runLayers(path, layers, Project.atlasNames[atlasIndex]);
					}
				}
			}
			else ExportTexture.runLayers(path, Context.raw.layersExport == ExportMode.ExportSelected ? (Context.raw.layer.isGroup() ? Context.raw.layer.getChildren() : [Context.raw.layer]) : Project.layers);
		}
		///end

		///if is_lab
		ExportTexture.runLayers(path, [BrushOutputNode.inst]);
		///end

		///if krom_ios
		Console.info(tr("Textures exported") + " ('Files/On My iPad/" + Manifest.title + "')");
		///elseif krom_android
		Console.info(tr("Textures exported") + " ('Files/Internal storage/Pictures/" + Manifest.title + "')");
		///else
		Console.info(tr("Textures exported"));
		///end
		UIFiles.lastPath = "";
	}

	///if is_paint
	static runBakeMaterial = (path: string) => {
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = new SlotLayer("_live");
		}

		let _tool = Context.raw.tool;
		Context.raw.tool = WorkspaceTool.ToolFill;
		MakeMaterial.parsePaintMaterial();
		let _paintObject = Context.raw.paintObject;
		let planeo: MeshObject = Scene.active.getChild(".Plane") as MeshObject;
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

		ExportTexture.runLayers(path, [RenderPathPaint.liveLayer], "", true);
	}
	///end

	///if is_paint
	static runLayers = (path: string, layers: SlotLayer[], objectName = "", bakeMaterial = false) => {
	///end

	///if is_lab
	static runLayers = (path: string, layers: any[], objectName = "") => {
	///end

		let textureSizeX = Config.getTextureResX();
		let textureSizeY = Config.getTextureResY();
		let formatQuality = Context.raw.formatQuality;
		///if (krom_android || krom_ios)
		let f = System.title;
		///else
		let f = UIFiles.filename;
		///end
		if (f == "") f = tr("untitled");
		let formatType = Context.raw.formatType;
		let bits = Base.bitsHandle.position == TextureBits.Bits8 ? 8 : 16;
		let ext = bits == 16 ? ".exr" : formatType == TextureLdrFormat.FormatPng ? ".png" : ".jpg";
		if (f.endsWith(ext)) f = f.substr(0, f.length - 4);

		///if is_paint
		let isUdim = Context.raw.layersExport == ExportMode.ExportPerUdimTile;
		if (isUdim) ext = objectName + ext;

		Base.makeTempImg();
		Base.makeExportImg();
		if (Base.pipeMerge == null) Base.makePipe();
		if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		let empty = RenderPath.active.renderTargets.get("empty_white").image;

		// Append object mask name
		let exportSelected = Context.raw.layersExport == ExportMode.ExportSelected;
		if (exportSelected && layers[0].getObjectMask() > 0) {
			f += "_" + Project.paintObjects[layers[0].getObjectMask() - 1].name;
		}
		if (!isUdim && !exportSelected && objectName != "") {
			f += "_" + objectName;
		}

		// Clear export layer
		Base.expa.g4.begin();
		Base.expa.g4.clear(color_from_floats(0.0, 0.0, 0.0, 0.0));
		Base.expa.g4.end();
		Base.expb.g4.begin();
		Base.expb.g4.clear(color_from_floats(0.5, 0.5, 1.0, 0.0));
		Base.expb.g4.end();
		Base.expc.g4.begin();
		Base.expc.g4.clear(color_from_floats(1.0, 0.0, 0.0, 0.0));
		Base.expc.g4.end();

		// Flatten layers
		for (let l1 of layers) {
			if (!exportSelected && !l1.isVisible()) continue;
			if (!l1.isLayer()) continue;

			if (objectName != "" && l1.getObjectMask() > 0) {
				if (isUdim && !Project.paintObjects[l1.getObjectMask() - 1].name.endsWith(objectName)) continue;
				let perObject = Context.raw.layersExport == ExportMode.ExportPerObject;
				if (perObject && Project.paintObjects[l1.getObjectMask() - 1].name != objectName) continue;
			}

			let mask = empty;
			let l1masks = l1.getMasks();
			if (l1masks != null && !bakeMaterial) {
				if (l1masks.length > 1) {
					Base.makeTempMaskImg();
					Base.tempMaskImage.g2.begin(true, 0x00000000);
					Base.tempMaskImage.g2.end();
					let l1: any = { texpaint: Base.tempMaskImage };
					for (let i = 0; i < l1masks.length; ++i) {
						Base.mergeLayer(l1, l1masks[i]);
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

		///if krom_metal
		// Flush command list
		Base.expa.g2.begin(false);
		Base.expa.g2.end();
		Base.expb.g2.begin(false);
		Base.expb.g2.end();
		Base.expc.g2.begin(false);
		Base.expc.g2.end();
		///end
		///end

		///if is_paint
		let texpaint = Base.expa;
		let texpaint_nor = Base.expb;
		let texpaint_pack = Base.expc;
		///end

		///if is_lab
		let texpaint = BrushOutputNode.inst.texpaint;
		let texpaint_nor = BrushOutputNode.inst.texpaint_nor;
		let texpaint_pack = BrushOutputNode.inst.texpaint_pack;
		///end

		let pixpaint: ArrayBuffer = null;
		let pixpaint_nor: ArrayBuffer = null;
		let pixpaint_pack: ArrayBuffer = null;
		let preset = BoxExport.preset;
		let pix: ArrayBuffer = null;

		for (let t of preset.textures) {
			for (let c of t.channels) {
				if      ((c == "base_r" || c == "base_g" || c == "base_b" || c == "opac") && pixpaint == null) pixpaint = texpaint.getPixels();
				else if ((c == "nor_r" || c == "nor_g" || c == "nor_g_directx" || c == "nor_b" || c == "emis" || c == "subs") && pixpaint_nor == null) pixpaint_nor = texpaint_nor.getPixels();
				else if ((c == "occ" || c == "rough" || c == "metal" || c == "height" || c == "smooth") && pixpaint_pack == null) pixpaint_pack = texpaint_pack.getPixels();
			}
		}

		for (let t of preset.textures) {
			let c = t.channels;
			let tex_name = t.name != "" ? "_" + t.name : "";
			let singleChannel = c[0] == c[1] && c[1] == c[2] && c[3] == "1.0";
			if (c[0] == "base_r" && c[1] == "base_g" && c[2] == "base_b" && c[3] == "1.0" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint, 1);
			}
			else if (c[0] == "nor_r" && c[1] == "nor_g" && c[2] == "nor_b" && c[3] == "1.0" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_nor, 1);
			}
			else if (c[0] == "occ" && c[1] == "rough" && c[2] == "metal" && c[3] == "1.0" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 1);
			}
			else if (singleChannel && c[0] == "occ" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 0);
			}
			else if (singleChannel && c[0] == "rough" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 1);
			}
			else if (singleChannel && c[0] == "metal" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 2);
			}
			else if (singleChannel && c[0] == "height" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 3);
			}
			else if (singleChannel && c[0] == "opac" && t.color_space == "linear") {
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pixpaint, 2, 3);
			}
			else {
				if (pix == null) pix = new ArrayBuffer(textureSizeX * textureSizeY * 4 * Math.floor(bits / 8));
				for (let i = 0; i < 4; ++i) {
					let c = t.channels[i];
					if      (c == "base_r") ExportTexture.copyChannel(new DataView(pixpaint), 0, new DataView(pix), i, t.color_space == "linear");
					else if (c == "base_g") ExportTexture.copyChannel(new DataView(pixpaint), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "base_b") ExportTexture.copyChannel(new DataView(pixpaint), 2, new DataView(pix), i, t.color_space == "linear");
					else if (c == "height") ExportTexture.copyChannel(new DataView(pixpaint_pack), 3, new DataView(pix), i, t.color_space == "linear");
					else if (c == "metal") ExportTexture.copyChannel(new DataView(pixpaint_pack), 2, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_r") ExportTexture.copyChannel(new DataView(pixpaint_nor), 0, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_g") ExportTexture.copyChannel(new DataView(pixpaint_nor), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_g_directx") ExportTexture.copyChannelInv(new DataView(pixpaint_nor), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_b") ExportTexture.copyChannel(new DataView(pixpaint_nor), 2, new DataView(pix), i, t.color_space == "linear");
					else if (c == "occ") ExportTexture.copyChannel(new DataView(pixpaint_pack), 0, new DataView(pix), i, t.color_space == "linear");
					else if (c == "opac") ExportTexture.copyChannel(new DataView(pixpaint), 3, new DataView(pix), i, t.color_space == "linear");
					else if (c == "rough") ExportTexture.copyChannel(new DataView(pixpaint_pack), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "smooth") ExportTexture.copyChannelInv(new DataView(pixpaint_pack), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "emis") ExportTexture.extractChannel(new DataView(pixpaint_nor), 3, new DataView(pix), i, 3, 1, t.color_space == "linear");
					else if (c == "subs") ExportTexture.extractChannel(new DataView(pixpaint_nor), 3, new DataView(pix), i, 3, 2, t.color_space == "linear");
					else if (c == "0.0") ExportTexture.setChannel(0, new DataView(pix), i);
					else if (c == "1.0") ExportTexture.setChannel(255, new DataView(pix), i);
				}
				ExportTexture.writeTexture(path + Path.sep + f + tex_name + ext, pix, 3);
			}
		}

		// Release staging memory allocated in Image.getPixels()
		texpaint.pixels = null;
		texpaint_nor.pixels = null;
		texpaint_pack.pixels = null;
	}

	static writeTexture = (file: string, pixels: ArrayBuffer, type = 1, off = 0) => {
		let resX = Config.getTextureResX();
		let resY = Config.getTextureResY();
		let bitsHandle = Base.bitsHandle.position;
		let bits = bitsHandle == TextureBits.Bits8 ? 8 : bitsHandle == TextureBits.Bits16 ? 16 : 32;
		let format = 0; // RGBA
		if (type == 1) format = 2; // RGB1
		if (type == 2 && off == 0) format = 3; // RRR1
		if (type == 2 && off == 1) format = 4; // GGG1
		if (type == 2 && off == 2) format = 5; // BBB1
		if (type == 2 && off == 3) format = 6; // AAA1

		if (Context.raw.layersDestination == ExportDestination.DestinationPacked) {
			let image = Image.fromBytes(pixels, resX, resY);
			Data.cachedImages.set(file, image);
			let ar = file.split(Path.sep);
			let name = ar[ar.length - 1];
			let asset: TAsset = {name: name, file: file, id: Project.assetId++};
			Project.assets.push(asset);
			if (Project.raw.assets == null) Project.raw.assets = [];
			Project.raw.assets.push(asset.file);
			Project.assetNames.push(asset.name);
			Project.assetMap.set(asset.id, image);
			ExportArm.packAssets(Project.raw, [asset]);
			return;
		}

		if (bits == 8 && Context.raw.formatType == TextureLdrFormat.FormatPng) {
			Krom.writePng(file, pixels, resX, resY, format);
		}
		else if (bits == 8 && Context.raw.formatType == TextureLdrFormat.FormatJpg) {
			Krom.writeJpg(file, pixels, resX, resY, format, Math.floor(Context.raw.formatQuality));
		}
		else { // Exr
			let b = ParserExr.run(resX, resY, pixels, bits, type, off);
			Krom.fileSaveBytes(file, b, b.byteLength);
		}
	}

	static copyChannel = (from: DataView, fromChannel: i32, to: DataView, toChannel: i32, linear = true) => {
		for (let i = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, from.getUint8(i * 4 + fromChannel));
		}
		if (!linear) ExportTexture.toSrgb(to, toChannel);
	}

	static copyChannelInv = (from: DataView, fromChannel: i32, to: DataView, toChannel: i32, linear = true) => {
		for (let i = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, 255 - from.getUint8(i * 4 + fromChannel));
		}
		if (!linear) ExportTexture.toSrgb(to, toChannel);
	}

	static extractChannel = (from: DataView, fromChannel: i32, to: DataView, toChannel: i32, step: i32, mask: i32, linear = true) => {
		for (let i = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, from.getUint8(i * 4 + fromChannel) % step == mask ? 255 : 0);
		}
		if (!linear) ExportTexture.toSrgb(to, toChannel);
	}

	static setChannel = (value: i32, to: DataView, toChannel: i32, linear = true) => {
		for (let i = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, value);
		}
		if (!linear) ExportTexture.toSrgb(to, toChannel);
	}

	static toSrgb = (to: DataView, toChannel: i32) => {
		for (let i = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, Math.floor(Math.pow(to.getUint8(i * 4 + toChannel) / 255, ExportTexture.gamma) * 255));
		}
	}
}

type TExportPreset = {
	textures: TExportPresetTexture[];
}

type TExportPresetTexture = {
	name: string;
	channels: string[];
	color_space: string;
}

///end
