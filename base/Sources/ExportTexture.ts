
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
				if (SlotLayer.getObjectMask(l) > 0) {
					let name = Project.paintObjects[SlotLayer.getObjectMask(l) - 1].base.name;
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
				if (SlotLayer.getObjectMask(l) > 0) {
					let name = Project.paintObjects[SlotLayer.getObjectMask(l) - 1].base.name;
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
					let layers: SlotLayerRaw[] = [];
					for (let objectIndex = 0; objectIndex < Project.atlasObjects.length; ++objectIndex) {
						if (Project.atlasObjects[objectIndex] == atlasIndex) {
							for (let l of Project.layers) {
								if (SlotLayer.getObjectMask(l) == 0 /* shared object */ || SlotLayer.getObjectMask(l) - 1 == objectIndex) layers.push(l);
							}
						}
					}
					if (layers.length > 0) {
						ExportTexture.runLayers(path, layers, Project.atlasNames[atlasIndex]);
					}
				}
			}
			else ExportTexture.runLayers(path, Context.raw.layersExport == ExportMode.ExportSelected ? (SlotLayer.isGroup(Context.raw.layer) ? SlotLayer.getChildren(Context.raw.layer) : [Context.raw.layer]) : Project.layers);
		}
		///end

		///if is_lab
		ExportTexture.runLayers(path, [BrushOutputNode.inst]);
		///end

		///if krom_ios
		Console.info(tr("Textures exported") + " ('Files/On My iPad/" + manifest_title + "')");
		///elseif krom_android
		Console.info(tr("Textures exported") + " ('Files/Internal storage/Pictures/" + manifest_title + "')");
		///else
		Console.info(tr("Textures exported"));
		///end
		UIFiles.lastPath = "";
	}

	///if is_paint
	static runBakeMaterial = (path: string) => {
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = SlotLayer.create("_live");
		}

		let _tool = Context.raw.tool;
		Context.raw.tool = WorkspaceTool.ToolFill;
		MakeMaterial.parsePaintMaterial();
		let _paintObject = Context.raw.paintObject;
		let planeo: mesh_object_t = scene_get_child(".Plane").ext;
		planeo.base.visible = true;
		Context.raw.paintObject = planeo;
		Context.raw.pdirty = 1;
		RenderPathPaint.useLiveLayer(true);
		RenderPathPaint.commandsPaint(false);
		RenderPathPaint.useLiveLayer(false);
		Context.raw.tool = _tool;
		MakeMaterial.parsePaintMaterial();
		Context.raw.pdirty = 0;
		planeo.base.visible = false;
		Context.raw.paintObject = _paintObject;

		ExportTexture.runLayers(path, [RenderPathPaint.liveLayer], "", true);
	}
	///end

	///if is_paint
	static runLayers = (path: string, layers: SlotLayerRaw[], objectName = "", bakeMaterial = false) => {
	///end

	///if is_lab
	static runLayers = (path: string, layers: any[], objectName = "") => {
	///end

		let textureSizeX = Config.getTextureResX();
		let textureSizeY = Config.getTextureResY();
		let formatQuality = Context.raw.formatQuality;
		///if (krom_android || krom_ios)
		let f = sys_title();
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
		if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
		let empty = render_path_render_targets.get("empty_white").image;

		// Append object mask name
		let exportSelected = Context.raw.layersExport == ExportMode.ExportSelected;
		if (exportSelected && SlotLayer.getObjectMask(layers[0]) > 0) {
			f += "_" + Project.paintObjects[SlotLayer.getObjectMask(layers[0]) - 1].base.name;
		}
		if (!isUdim && !exportSelected && objectName != "") {
			f += "_" + objectName;
		}

		// Clear export layer
		g4_begin(Base.expa);
		g4_clear(color_from_floats(0.0, 0.0, 0.0, 0.0));
		g4_end();
		g4_begin(Base.expb);
		g4_clear(color_from_floats(0.5, 0.5, 1.0, 0.0));
		g4_end();
		g4_begin(Base.expc);
		g4_clear(color_from_floats(1.0, 0.0, 0.0, 0.0));
		g4_end();

		// Flatten layers
		for (let l1 of layers) {
			if (!exportSelected && !SlotLayer.isVisible(l1)) continue;
			if (!SlotLayer.isLayer(l1)) continue;

			if (objectName != "" && SlotLayer.getObjectMask(l1) > 0) {
				if (isUdim && !Project.paintObjects[SlotLayer.getObjectMask(l1) - 1].base.name.endsWith(objectName)) continue;
				let perObject = Context.raw.layersExport == ExportMode.ExportPerObject;
				if (perObject && Project.paintObjects[SlotLayer.getObjectMask(l1) - 1].base.name != objectName) continue;
			}

			let mask = empty;
			let l1masks = SlotLayer.getMasks(l1);
			if (l1masks != null && !bakeMaterial) {
				if (l1masks.length > 1) {
					Base.makeTempMaskImg();
					g2_begin(Base.tempMaskImage, true, 0x00000000);
					g2_end();
					let l1: any = { texpaint: Base.tempMaskImage };
					for (let i = 0; i < l1masks.length; ++i) {
						Base.mergeLayer(l1, l1masks[i]);
					}
					mask = Base.tempMaskImage;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				g2_begin(Base.tempImage, false); // Copy to temp
				g2_set_pipeline(Base.pipeCopy);
				g2_draw_image(Base.expa, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				g4_begin(Base.expa);
				g4_set_pipeline(Base.pipeMerge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, empty);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.tempImage);
				g4_set_float(Base.opac, SlotLayer.getOpacity(l1));
				g4_set_int(Base.blending, layers.length > 1 ? l1.blending : 0);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			if (l1.paintNor) {
				g2_begin(Base.tempImage, false);
				g2_set_pipeline(Base.pipeCopy);
				g2_draw_image(Base.expb, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				g4_begin(Base.expb);
				g4_set_pipeline(Base.pipeMerge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, l1.texpaint_nor);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.tempImage);
				g4_set_float(Base.opac, SlotLayer.getOpacity(l1));
				g4_set_int(Base.blending, l1.paintNorBlend ? -2 : -1);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				g2_begin(Base.tempImage, false);
				g2_set_pipeline(Base.pipeCopy);
				g2_draw_image(Base.expc, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					Base.commandsMergePack(Base.pipeMerge, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) Base.commandsMergePack(Base.pipeMergeR, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
					if (l1.paintRough) Base.commandsMergePack(Base.pipeMergeG, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
					if (l1.paintMet) Base.commandsMergePack(Base.pipeMergeB, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.getOpacity(l1), mask);
				}
			}
		}

		///if krom_metal
		// Flush command list
		g2_begin(Base.expa, false);
		g2_end();
		g2_begin(Base.expb, false);
		g2_end();
		g2_begin(Base.expc, false);
		g2_end();
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
				if      ((c == "base_r" || c == "base_g" || c == "base_b" || c == "opac") && pixpaint == null) pixpaint = image_get_pixels(texpaint);
				else if ((c == "nor_r" || c == "nor_g" || c == "nor_g_directx" || c == "nor_b" || c == "emis" || c == "subs") && pixpaint_nor == null) pixpaint_nor = image_get_pixels(texpaint_nor);
				else if ((c == "occ" || c == "rough" || c == "metal" || c == "height" || c == "smooth") && pixpaint_pack == null) pixpaint_pack = image_get_pixels(texpaint_pack);
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

		// Release staging memory allocated in image_get_pixels()
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
			let image = image_from_bytes(pixels, resX, resY);
			data_cached_images.set(file, image);
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
			krom_write_png(file, pixels, resX, resY, format);
		}
		else if (bits == 8 && Context.raw.formatType == TextureLdrFormat.FormatJpg) {
			krom_write_jpg(file, pixels, resX, resY, format, Math.floor(Context.raw.formatQuality));
		}
		else { // Exr
			let b = ParserExr.run(resX, resY, pixels, bits, type, off);
			krom_file_save_bytes(file, b, b.byteLength);
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
