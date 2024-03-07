
///if (is_paint || is_lab)

class ExportTexture {

	static gamma: f32 = 1.0 / 2.2;

	static run = (path: string, bakeMaterial: bool = false) => {

		///if is_paint
		if (bakeMaterial) {
			ExportTexture.run_bake_material(path);
		}
		else if (Context.raw.layers_export == export_mode_t.PER_UDIM_TILE) {
			let udimTiles: string[] = [];
			for (let l of Project.layers) {
				if (SlotLayer.get_object_mask(l) > 0) {
					let name: string = Project.paint_objects[SlotLayer.get_object_mask(l) - 1].base.name;
					if (name.substr(name.length - 5, 2) == ".1") { // tile.1001
						udimTiles.push(name.substr(name.length - 5));
					}
				}
			}
			if (udimTiles.length > 0) {
				for (let udimTile of udimTiles) ExportTexture.run_layers(path, Project.layers, udimTile);
			}
			else ExportTexture.run_layers(path, Project.layers);
		}
		else if (Context.raw.layers_export == export_mode_t.PER_OBJECT) {
			let objectNames: string[] = [];
			for (let l of Project.layers) {
				if (SlotLayer.get_object_mask(l) > 0) {
					let name: string = Project.paint_objects[SlotLayer.get_object_mask(l) - 1].base.name;
					if (objectNames.indexOf(name) == -1) {
						objectNames.push(name);
					}
				}
			}
			if (objectNames.length > 0) {
				for (let name of objectNames) ExportTexture.run_layers(path, Project.layers, name);
			}
			else ExportTexture.run_layers(path, Project.layers);
		}
		else { // Visible or selected
			let atlasExport: bool = false;
			if (Project.atlas_objects != null) {
				for (let i: i32 = 1; i < Project.atlas_objects.length; ++i) {
					if (Project.atlas_objects[i - 1] != Project.atlas_objects[i]) {
						atlasExport = true;
						break;
					}
				}
			}
			if (atlasExport) {
				for (let atlasIndex: i32 = 0; atlasIndex < Project.atlas_objects.length; ++atlasIndex) {
					let layers: SlotLayerRaw[] = [];
					for (let objectIndex: i32 = 0; objectIndex < Project.atlas_objects.length; ++objectIndex) {
						if (Project.atlas_objects[objectIndex] == atlasIndex) {
							for (let l of Project.layers) {
								if (SlotLayer.get_object_mask(l) == 0 /* shared object */ || SlotLayer.get_object_mask(l) - 1 == objectIndex) layers.push(l);
							}
						}
					}
					if (layers.length > 0) {
						ExportTexture.run_layers(path, layers, Project.atlas_names[atlasIndex]);
					}
				}
			}
			else ExportTexture.run_layers(path, Context.raw.layers_export == export_mode_t.SELECTED ? (SlotLayer.is_group(Context.raw.layer) ? SlotLayer.get_children(Context.raw.layer) : [Context.raw.layer]) : Project.layers);
		}
		///end

		///if is_lab
		ExportTexture.run_layers(path, [BrushOutputNode.inst]);
		///end

		///if krom_ios
		Console.info(tr("Textures exported") + " ('Files/On My iPad/" + manifest_title + "')");
		///elseif krom_android
		Console.info(tr("Textures exported") + " ('Files/Internal storage/Pictures/" + manifest_title + "')");
		///else
		Console.info(tr("Textures exported"));
		///end
		UIFiles.last_path = "";
	}

	///if is_paint
	static run_bake_material = (path: string) => {
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = SlotLayer.create("_live");
		}

		let _tool: workspace_tool_t = Context.raw.tool;
		Context.raw.tool = workspace_tool_t.FILL;
		MakeMaterial.parse_paint_material();
		let _paintObject: mesh_object_t = Context.raw.paint_object;
		let planeo: mesh_object_t = scene_get_child(".Plane").ext;
		planeo.base.visible = true;
		Context.raw.paint_object = planeo;
		Context.raw.pdirty = 1;
		RenderPathPaint.use_live_layer(true);
		RenderPathPaint.commands_paint(false);
		RenderPathPaint.use_live_layer(false);
		Context.raw.tool = _tool;
		MakeMaterial.parse_paint_material();
		Context.raw.pdirty = 0;
		planeo.base.visible = false;
		Context.raw.paint_object = _paintObject;

		ExportTexture.run_layers(path, [RenderPathPaint.liveLayer], "", true);
	}
	///end

	///if is_paint
	static run_layers = (path: string, layers: SlotLayerRaw[], objectName: string = "", bakeMaterial: bool = false) => {
	///end

	///if is_lab
	static run_layers = (path: string, layers: any[], objectName: string = "") => {
	///end

		let textureSizeX: i32 = Config.get_texture_res_x();
		let textureSizeY: i32 = Config.get_texture_res_y();
		///if (krom_android || krom_ios)
		let f: string = sys_title();
		///else
		let f: string = UIFiles.filename;
		///end
		if (f == "") f = tr("untitled");
		let formatType: texture_ldr_format_t = Context.raw.format_type;
		let bits: i32 = Base.bits_handle.position == texture_bits_t.BITS8 ? 8 : 16;
		let ext: string = bits == 16 ? ".exr" : formatType == texture_ldr_format_t.PNG ? ".png" : ".jpg";
		if (f.endsWith(ext)) f = f.substr(0, f.length - 4);

		///if is_paint
		let isUdim: bool = Context.raw.layers_export == export_mode_t.PER_UDIM_TILE;
		if (isUdim) ext = objectName + ext;

		Base.make_temp_img();
		Base.make_export_img();
		if (Base.pipe_merge == null) Base.make_pipe();
		if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
		let empty: image_t = render_path_render_targets.get("empty_white")._image;

		// Append object mask name
		let exportSelected: bool = Context.raw.layers_export == export_mode_t.SELECTED;
		if (exportSelected && SlotLayer.get_object_mask(layers[0]) > 0) {
			f += "_" + Project.paint_objects[SlotLayer.get_object_mask(layers[0]) - 1].base.name;
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
			if (!exportSelected && !SlotLayer.is_visible(l1)) continue;
			if (!SlotLayer.is_layer(l1)) continue;

			if (objectName != "" && SlotLayer.get_object_mask(l1) > 0) {
				if (isUdim && !Project.paint_objects[SlotLayer.get_object_mask(l1) - 1].base.name.endsWith(objectName)) continue;
				let perObject: bool = Context.raw.layers_export == export_mode_t.PER_OBJECT;
				if (perObject && Project.paint_objects[SlotLayer.get_object_mask(l1) - 1].base.name != objectName) continue;
			}

			let mask: image_t = empty;
			let l1masks: SlotLayerRaw[] = SlotLayer.get_masks(l1);
			if (l1masks != null && !bakeMaterial) {
				if (l1masks.length > 1) {
					Base.make_temp_mask_img();
					g2_begin(Base.temp_mask_image);
					g2_clear(0x00000000);
					g2_end();
					let l1: any = { texpaint: Base.temp_mask_image };
					for (let i: i32 = 0; i < l1masks.length; ++i) {
						Base.merge_layer(l1, l1masks[i]);
					}
					mask = Base.temp_mask_image;
				}
				else mask = l1masks[0].texpaint;
			}

			if (l1.paintBase) {
				g2_begin(Base.temp_image); // Copy to temp
				g2_set_pipeline(Base.pipe_copy);
				g2_draw_image(Base.expa, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				g4_begin(Base.expa);
				g4_set_pipeline(Base.pipe_merge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, empty);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.temp_image);
				g4_set_float(Base.opac, SlotLayer.get_opacity(l1));
				g4_set_int(Base.blending, layers.length > 1 ? l1.blending : 0);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			if (l1.paintNor) {
				g2_begin(Base.temp_image);
				g2_set_pipeline(Base.pipe_copy);
				g2_draw_image(Base.expb, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				g4_begin(Base.expb);
				g4_set_pipeline(Base.pipe_merge);
				g4_set_tex(Base.tex0, l1.texpaint);
				g4_set_tex(Base.tex1, l1.texpaint_nor);
				g4_set_tex(Base.texmask, mask);
				g4_set_tex(Base.texa, Base.temp_image);
				g4_set_float(Base.opac, SlotLayer.get_opacity(l1));
				g4_set_int(Base.blending, l1.paintNorBlend ? -2 : -1);
				g4_set_vertex_buffer(const_data_screen_aligned_vb);
				g4_set_index_buffer(const_data_screen_aligned_ib);
				g4_draw();
				g4_end();
			}

			if (l1.paintOcc || l1.paintRough || l1.paintMet || l1.paintHeight) {
				g2_begin(Base.temp_image);
				g2_set_pipeline(Base.pipe_copy);
				g2_draw_image(Base.expc, 0, 0);
				g2_set_pipeline(null);
				g2_end();

				if (l1.paintOcc && l1.paintRough && l1.paintMet && l1.paintHeight) {
					Base.commands_merge_pack(Base.pipe_merge, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask, l1.paintHeightBlend ? -3 : -1);
				}
				else {
					if (l1.paintOcc) Base.commands_merge_pack(Base.pipe_merge_r, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
					if (l1.paintRough) Base.commands_merge_pack(Base.pipe_merge_g, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
					if (l1.paintMet) Base.commands_merge_pack(Base.pipe_merge_b, Base.expc, l1.texpaint, l1.texpaint_pack, SlotLayer.get_opacity(l1), mask);
				}
			}
		}

		///if krom_metal
		// Flush command list
		g2_begin(Base.expa);
		g2_end();
		g2_begin(Base.expb);
		g2_end();
		g2_begin(Base.expc);
		g2_end();
		///end
		///end

		///if is_paint
		let texpaint: image_t = Base.expa;
		let texpaint_nor: image_t = Base.expb;
		let texpaint_pack: image_t = Base.expc;
		///end

		///if is_lab
		let texpaint: image_t = BrushOutputNode.inst.texpaint;
		let texpaint_nor: image_t = BrushOutputNode.inst.texpaint_nor;
		let texpaint_pack: image_t = BrushOutputNode.inst.texpaint_pack;
		///end

		let pixpaint: ArrayBuffer = null;
		let pixpaint_nor: ArrayBuffer = null;
		let pixpaint_pack: ArrayBuffer = null;
		let preset: export_preset_t = BoxExport.preset;
		let pix: ArrayBuffer = null;

		for (let t of preset.textures) {
			for (let c of t.channels) {
				if      ((c == "base_r" || c == "base_g" || c == "base_b" || c == "opac") && pixpaint == null) pixpaint = image_get_pixels(texpaint);
				else if ((c == "nor_r" || c == "nor_g" || c == "nor_g_directx" || c == "nor_b" || c == "emis" || c == "subs") && pixpaint_nor == null) pixpaint_nor = image_get_pixels(texpaint_nor);
				else if ((c == "occ" || c == "rough" || c == "metal" || c == "height" || c == "smooth") && pixpaint_pack == null) pixpaint_pack = image_get_pixels(texpaint_pack);
			}
		}

		for (let t of preset.textures) {
			let c: string[] = t.channels;
			let tex_name = t.name != "" ? "_" + t.name : "";
			let singleChannel: bool = c[0] == c[1] && c[1] == c[2] && c[3] == "1.0";
			if (c[0] == "base_r" && c[1] == "base_g" && c[2] == "base_b" && c[3] == "1.0" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint, 1);
			}
			else if (c[0] == "nor_r" && c[1] == "nor_g" && c[2] == "nor_b" && c[3] == "1.0" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint_nor, 1);
			}
			else if (c[0] == "occ" && c[1] == "rough" && c[2] == "metal" && c[3] == "1.0" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 1);
			}
			else if (singleChannel && c[0] == "occ" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 0);
			}
			else if (singleChannel && c[0] == "rough" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 1);
			}
			else if (singleChannel && c[0] == "metal" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 2);
			}
			else if (singleChannel && c[0] == "height" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint_pack, 2, 3);
			}
			else if (singleChannel && c[0] == "opac" && t.color_space == "linear") {
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pixpaint, 2, 3);
			}
			else {
				if (pix == null) pix = new ArrayBuffer(textureSizeX * textureSizeY * 4 * Math.floor(bits / 8));
				for (let i: i32 = 0; i < 4; ++i) {
					let c: string = t.channels[i];
					if      (c == "base_r") ExportTexture.copy_channel(new DataView(pixpaint), 0, new DataView(pix), i, t.color_space == "linear");
					else if (c == "base_g") ExportTexture.copy_channel(new DataView(pixpaint), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "base_b") ExportTexture.copy_channel(new DataView(pixpaint), 2, new DataView(pix), i, t.color_space == "linear");
					else if (c == "height") ExportTexture.copy_channel(new DataView(pixpaint_pack), 3, new DataView(pix), i, t.color_space == "linear");
					else if (c == "metal") ExportTexture.copy_channel(new DataView(pixpaint_pack), 2, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_r") ExportTexture.copy_channel(new DataView(pixpaint_nor), 0, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_g") ExportTexture.copy_channel(new DataView(pixpaint_nor), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_g_directx") ExportTexture.copy_channel_inv(new DataView(pixpaint_nor), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "nor_b") ExportTexture.copy_channel(new DataView(pixpaint_nor), 2, new DataView(pix), i, t.color_space == "linear");
					else if (c == "occ") ExportTexture.copy_channel(new DataView(pixpaint_pack), 0, new DataView(pix), i, t.color_space == "linear");
					else if (c == "opac") ExportTexture.copy_channel(new DataView(pixpaint), 3, new DataView(pix), i, t.color_space == "linear");
					else if (c == "rough") ExportTexture.copy_channel(new DataView(pixpaint_pack), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "smooth") ExportTexture.copy_channel_inv(new DataView(pixpaint_pack), 1, new DataView(pix), i, t.color_space == "linear");
					else if (c == "emis") ExportTexture.extract_channel(new DataView(pixpaint_nor), 3, new DataView(pix), i, 3, 1, t.color_space == "linear");
					else if (c == "subs") ExportTexture.extract_channel(new DataView(pixpaint_nor), 3, new DataView(pix), i, 3, 2, t.color_space == "linear");
					else if (c == "0.0") ExportTexture.set_channel(0, new DataView(pix), i);
					else if (c == "1.0") ExportTexture.set_channel(255, new DataView(pix), i);
				}
				ExportTexture.write_texture(path + Path.sep + f + tex_name + ext, pix, 3);
			}
		}

		// Release staging memory allocated in image_get_pixels()
		texpaint.pixels = null;
		texpaint_nor.pixels = null;
		texpaint_pack.pixels = null;
	}

	static write_texture = (file: string, pixels: ArrayBuffer, type: i32 = 1, off: i32 = 0) => {
		let resX: i32 = Config.get_texture_res_x();
		let resY: i32 = Config.get_texture_res_y();
		let bitsHandle: i32 = Base.bits_handle.position;
		let bits: i32 = bitsHandle == texture_bits_t.BITS8 ? 8 : bitsHandle == texture_bits_t.BITS16 ? 16 : 32;
		let format: i32 = 0; // RGBA
		if (type == 1) format = 2; // RGB1
		if (type == 2 && off == 0) format = 3; // RRR1
		if (type == 2 && off == 1) format = 4; // GGG1
		if (type == 2 && off == 2) format = 5; // BBB1
		if (type == 2 && off == 3) format = 6; // AAA1

		if (Context.raw.layers_destination == export_destination_t.PACKED) {
			let image: image_t = image_from_bytes(pixels, resX, resY);
			data_cached_images.set(file, image);
			let ar: string[] = file.split(Path.sep);
			let name: string = ar[ar.length - 1];
			let asset: asset_t = {name: name, file: file, id: Project.asset_id++};
			Project.assets.push(asset);
			if (Project.raw.assets == null) Project.raw.assets = [];
			Project.raw.assets.push(asset.file);
			Project.asset_names.push(asset.name);
			Project.asset_map.set(asset.id, image);
			ExportArm.pack_assets(Project.raw, [asset]);
			return;
		}

		if (bits == 8 && Context.raw.format_type == texture_ldr_format_t.PNG) {
			krom_write_png(file, pixels, resX, resY, format);
		}
		else if (bits == 8 && Context.raw.format_type == texture_ldr_format_t.JPG) {
			krom_write_jpg(file, pixels, resX, resY, format, Math.floor(Context.raw.format_quality));
		}
		else { // Exr
			let b: ArrayBuffer = ParserExr.run(resX, resY, pixels, bits, type, off);
			krom_file_save_bytes(file, b, b.byteLength);
		}
	}

	static copy_channel = (from: DataView, fromChannel: i32, to: DataView, toChannel: i32, linear: bool = true) => {
		for (let i: i32 = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, from.getUint8(i * 4 + fromChannel));
		}
		if (!linear) ExportTexture.to_srgb(to, toChannel);
	}

	static copy_channel_inv = (from: DataView, fromChannel: i32, to: DataView, toChannel: i32, linear: bool = true) => {
		for (let i: i32 = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, 255 - from.getUint8(i * 4 + fromChannel));
		}
		if (!linear) ExportTexture.to_srgb(to, toChannel);
	}

	static extract_channel = (from: DataView, fromChannel: i32, to: DataView, toChannel: i32, step: i32, mask: i32, linear: bool = true) => {
		for (let i: i32 = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, from.getUint8(i * 4 + fromChannel) % step == mask ? 255 : 0);
		}
		if (!linear) ExportTexture.to_srgb(to, toChannel);
	}

	static set_channel = (value: i32, to: DataView, toChannel: i32, linear: bool = true) => {
		for (let i: i32 = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, value);
		}
		if (!linear) ExportTexture.to_srgb(to, toChannel);
	}

	static to_srgb = (to: DataView, toChannel: i32) => {
		for (let i: i32 = 0; i < Math.floor(to.byteLength / 4); ++i) {
			to.setUint8(i * 4 + toChannel, Math.floor(Math.pow(to.getUint8(i * 4 + toChannel) / 255, ExportTexture.gamma) * 255));
		}
	}
}

type export_preset_t = {
	textures?: export_preset_texture_t[];
};

type export_preset_texture_t = {
	name?: string;
	channels?: string[];
	color_space?: string;
};

///end
