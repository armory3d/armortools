
let export_texture_gamma: f32 = 1.0 / 2.2;

function export_texture_run(path: string, bake_material: bool = false) {

	if (bake_material) {
		export_texture_run_bake_material(path);
	}
	else if (context_raw.layers_export == export_mode_t.PER_UDIM_TILE) {
		let udim_tiles: string[] = [];
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (slot_layer_get_object_mask(l) > 0) {
				let name: string = project_paint_objects[slot_layer_get_object_mask(l) - 1].base.name;
				if (substring(name, name.length - 5, 2) == ".1") { // tile.1001
					array_push(udim_tiles, substring(name, name.length - 5, name.length));
				}
			}
		}
		if (udim_tiles.length > 0) {
			for (let i: i32 = 0; i < udim_tiles.length; ++i) {
				let udim_tile: string = udim_tiles[i];
				export_texture_run_layers(path, project_layers, udim_tile);
			}
		}
		else {
			export_texture_run_layers(path, project_layers);
		}
	}
	else if (context_raw.layers_export == export_mode_t.PER_OBJECT) {
		let object_names: string[] = [];
		for (let i: i32 = 0; i < project_layers.length; ++i) {
			let l: slot_layer_t = project_layers[i];
			if (slot_layer_get_object_mask(l) > 0) {
				let name: string = project_paint_objects[slot_layer_get_object_mask(l) - 1].base.name;
				if (array_index_of(object_names, name) == -1) {
					array_push(object_names, name);
				}
			}
		}
		if (object_names.length > 0) {
			for (let i: i32 = 0; i < object_names.length; ++i) {
				let name: string = object_names[i];
				export_texture_run_layers(path, project_layers, name);
			}
		}
		else {
			export_texture_run_layers(path, project_layers);
		}
	}
	else { // Visible or selected
		let atlas_export: bool = false;
		if (project_atlas_objects != null) {
			for (let i: i32 = 1; i < project_atlas_objects.length; ++i) {
				if (project_atlas_objects[i - 1] != project_atlas_objects[i]) {
					atlas_export = true;
					break;
				}
			}
		}
		if (atlas_export) {
			for (let atlas_index: i32 = 0; atlas_index < project_atlas_objects.length; ++atlas_index) {
				let layers: slot_layer_t[] = [];
				for (let object_index: i32 = 0; object_index < project_atlas_objects.length; ++object_index) {
					if (project_atlas_objects[object_index] == atlas_index) {
						for (let i: i32 = 0; i < project_layers.length; ++i) {
							let l: slot_layer_t = project_layers[i];
							if (slot_layer_get_object_mask(l) == 0 || // shared object
							    slot_layer_get_object_mask(l) - 1 == object_index) {
								array_push(layers, l);
							}
						}
					}
				}
				if (layers.length > 0) {
					export_texture_run_layers(path, layers, project_atlas_names[atlas_index]);
				}
			}
		}
		else {
			let layers: slot_layer_t[];
			if (context_raw.layers_export == export_mode_t.SELECTED) {
				if (slot_layer_is_group(context_raw.layer)) {
					layers = slot_layer_get_children(context_raw.layer);
				}
				else {
					layers = [ context_raw.layer ];
				}
			}
			else {
				layers = project_layers;
			}
			export_texture_run_layers(path, layers);
		}
	}

	/// if arm_ios
	console_info(tr("Textures exported") + " (\"Files/On My iPad/" + manifest_title + "\")");
	/// elseif arm_android
	console_info(tr("Textures exported") + " (\"Files/Internal storage/Pictures/" + manifest_title + "\")");
	/// else
	console_info(tr("Textures exported"));
	/// end
	ui_files_last_path = "";
}

function export_texture_run_bake_material(path: string) {
	if (render_path_paint_live_layer == null) {
		render_path_paint_live_layer = slot_layer_create("_live");
	}

	let _tool: tool_type_t = context_raw.tool;
	context_raw.tool       = tool_type_t.FILL;
	make_material_parse_paint_material();
	let _paint_object: mesh_object_t = context_raw.paint_object;
	let planeo: mesh_object_t        = scene_get_child(".Plane").ext;
	planeo.base.visible              = true;
	context_raw.paint_object         = planeo;
	context_raw.pdirty               = 1;

	let _visibles: bool[] = [];
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let p: mesh_object_t = project_paint_objects[i];
		array_push(_visibles, p.base.visible);
		p.base.visible = false;
	}

	render_path_paint_use_live_layer(true);
	render_path_paint_commands_paint(false);
	render_path_paint_use_live_layer(false);

	context_raw.tool = _tool;
	make_material_parse_paint_material();
	context_raw.pdirty       = 0;
	planeo.base.visible      = false;
	context_raw.paint_object = _paint_object;

	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		project_paint_objects[i].base.visible = _visibles[i];
	}

	let layers: slot_layer_t[] = [ render_path_paint_live_layer ];
	export_texture_run_layers(path, layers, "", true);
}

function export_texture_run_layers(path: string, layers: slot_layer_t[], object_name: string = "", bake_material: bool = false) {

	let texture_size_x: i32 = config_get_texture_res_x();
	let texture_size_y: i32 = config_get_texture_res_y();
	/// if (arm_android || arm_ios)
	let f: string = sys_title();
	/// else
	let f: string = ui_files_filename;
	/// end
	if (f == "") {
		f = tr("untitled");
	}
	let format_type: texture_ldr_format_t = context_raw.format_type;
	let bits: i32                         = base_bits_handle.i == texture_bits_t.BITS8 ? 8 : 16;
	let ext: string                       = bits == 16 ? ".exr" : format_type == texture_ldr_format_t.PNG ? ".png" : ".jpg";
	if (ends_with(f, ext)) {
		f = substring(f, 0, f.length - 4);
	}

	let is_udim: bool = context_raw.layers_export == export_mode_t.PER_UDIM_TILE;
	if (is_udim) {
		ext = object_name + ext;
	}

	layers_make_temp_img();
	layers_make_export_img();
	let rt: render_target_t  = map_get(render_path_render_targets, "empty_white");
	let empty: gpu_texture_t = rt._image;

	// Append object mask name
	let export_selected: bool = context_raw.layers_export == export_mode_t.SELECTED;
	if (export_selected && slot_layer_get_object_mask(layers[0]) > 0) {
		f += "_" + project_paint_objects[slot_layer_get_object_mask(layers[0]) - 1].base.name;
	}
	if (!is_udim && !export_selected && object_name != "") {
		f += "_" + object_name;
	}

	// Clear export layer
	_gpu_begin(layers_expa, null, null, clear_flag_t.COLOR, color_from_floats(0.0, 0.0, 0.0, 0.0));
	gpu_end();
	_gpu_begin(layers_expb, null, null, clear_flag_t.COLOR, color_from_floats(0.5, 0.5, 1.0, 0.0));
	gpu_end();
	_gpu_begin(layers_expc, null, null, clear_flag_t.COLOR, color_from_floats(1.0, 0.0, 0.0, 0.0));
	gpu_end();

	// Flatten layers
	for (let i: i32 = 0; i < layers.length; ++i) {
		let l1: slot_layer_t = layers[i];
		if (!export_selected && !slot_layer_is_visible(l1)) {
			continue;
		}
		if (!slot_layer_is_layer(l1)) {
			continue;
		}

		if (object_name != "" && slot_layer_get_object_mask(l1) > 0) {
			if (is_udim && !ends_with(project_paint_objects[slot_layer_get_object_mask(l1) - 1].base.name, object_name)) {
				continue;
			}
			let per_object: bool = context_raw.layers_export == export_mode_t.PER_OBJECT;
			if (per_object && project_paint_objects[slot_layer_get_object_mask(l1) - 1].base.name != object_name) {
				continue;
			}
		}

		let mask: gpu_texture_t     = empty;
		let l1masks: slot_layer_t[] = slot_layer_get_masks(l1);
		if (l1masks != null && !bake_material) {
			if (l1masks.length > 1) {
				layers_make_temp_mask_img();
				draw_begin(pipes_temp_mask_image, true, 0x00000000);
				draw_end();
				let l1: slot_layer_t = {texpaint : pipes_temp_mask_image};
				for (let i: i32 = 0; i < l1masks.length; ++i) {
					layers_merge_layer(l1, l1masks[i]);
				}
				mask = pipes_temp_mask_image;
			}
			else {
				mask = l1masks[0].texpaint;
			}
		}

		if (l1.paint_base) {
			draw_begin(layers_temp_image); // Copy to temp
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expa, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			_gpu_begin(layers_expa);
			gpu_set_pipeline(pipes_merge);
			gpu_set_texture(pipes_tex0, l1.texpaint);
			gpu_set_texture(pipes_tex1, empty);
			gpu_set_texture(pipes_texmask, mask);
			gpu_set_texture(pipes_texa, layers_temp_image);
			gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
			gpu_set_float(pipes_tex1w, empty.width);
			gpu_set_int(pipes_blending, layers.length > 1 ? l1.blending : 0);
			gpu_set_vertex_buffer(const_data_screen_aligned_vb);
			gpu_set_index_buffer(const_data_screen_aligned_ib);
			gpu_draw();
			gpu_end();
		}

		if (l1.paint_nor) {
			draw_begin(layers_temp_image);
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expb, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			_gpu_begin(layers_expb);
			gpu_set_pipeline(pipes_merge);
			gpu_set_texture(pipes_tex0, l1.texpaint);
			gpu_set_texture(pipes_tex1, l1.texpaint_nor);
			gpu_set_texture(pipes_texmask, mask);
			gpu_set_texture(pipes_texa, layers_temp_image);
			gpu_set_float(pipes_opac, slot_layer_get_opacity(l1));
			gpu_set_float(pipes_tex1w, l1.texpaint_nor.width);
			gpu_set_int(pipes_blending, l1.paint_nor_blend ? 102 : 101);
			gpu_set_vertex_buffer(const_data_screen_aligned_vb);
			gpu_set_index_buffer(const_data_screen_aligned_ib);
			gpu_draw();
			gpu_end();
		}

		if (l1.paint_occ || l1.paint_rough || l1.paint_met || l1.paint_height) {
			draw_begin(layers_temp_image);
			draw_set_pipeline(pipes_copy);
			draw_image(layers_expc, 0, 0);
			draw_set_pipeline(null);
			draw_end();

			if (l1.paint_occ && l1.paint_rough && l1.paint_met && l1.paint_height) {
				layers_commands_merge_pack(pipes_merge, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask,
				                           l1.paint_height_blend ? 103 : 101);
			}
			else {
				if (l1.paint_occ)
					layers_commands_merge_pack(pipes_merge_r, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				if (l1.paint_rough)
					layers_commands_merge_pack(pipes_merge_g, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
				if (l1.paint_met)
					layers_commands_merge_pack(pipes_merge_b, layers_expc, l1.texpaint, l1.texpaint_pack, slot_layer_get_opacity(l1), mask);
			}
		}
	}

	let texpaint: gpu_texture_t      = layers_expa;
	let texpaint_nor: gpu_texture_t  = layers_expb;
	let texpaint_pack: gpu_texture_t = layers_expc;

	let pixpaint: buffer_t      = null;
	let pixpaint_nor: buffer_t  = null;
	let pixpaint_pack: buffer_t = null;
	let preset: export_preset_t = box_export_preset;
	let pix: buffer_t           = null;

	for (let i: i32 = 0; i < preset.textures.length; ++i) {
		let t: export_preset_texture_t = preset.textures[i];
		for (let i: i32 = 0; i < t.channels.length; ++i) {
			let c: string = t.channels[i];
			if ((c == "base_r" || c == "base_g" || c == "base_b" || c == "opac") && pixpaint == null) {
				pixpaint = gpu_get_texture_pixels(texpaint);
			}
			else if ((c == "nor_r" || c == "nor_g" || c == "nor_g_directx" || c == "nor_b" || c == "emis" || c == "subs") && pixpaint_nor == null) {
				pixpaint_nor = gpu_get_texture_pixels(texpaint_nor);
			}
			else if ((c == "occ" || c == "rough" || c == "metal" || c == "height" || c == "smooth") && pixpaint_pack == null) {
				pixpaint_pack = gpu_get_texture_pixels(texpaint_pack);
			}
		}
	}

	for (let i: i32 = 0; i < preset.textures.length; ++i) {
		let t: export_preset_texture_t = preset.textures[i];
		let c: string[]                = t.channels;
		let tex_name: string           = t.name != "" ? "_" + t.name : "";
		let single_channel: bool       = c[0] == c[1] && c[1] == c[2] && c[3] == "1.0";
		if (c[0] == "base_r" && c[1] == "base_g" && c[2] == "base_b" && c[3] == "1.0" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint, 1);
		}
		else if (c[0] == "nor_r" && c[1] == "nor_g" && c[2] == "nor_b" && c[3] == "1.0" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint_nor, 1);
		}
		else if (c[0] == "occ" && c[1] == "rough" && c[2] == "metal" && c[3] == "1.0" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint_pack, 1);
		}
		else if (single_channel && c[0] == "occ" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint_pack, 2, 0);
		}
		else if (single_channel && c[0] == "rough" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint_pack, 2, 1);
		}
		else if (single_channel && c[0] == "metal" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint_pack, 2, 2);
		}
		else if (single_channel && c[0] == "height" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint_pack, 2, 3);
		}
		else if (single_channel && c[0] == "opac" && t.color_space == "linear") {
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pixpaint, 2, 3);
		}
		else {
			if (pix == null) {
				pix = buffer_create(texture_size_x * texture_size_y * 4 * math_floor(bits / 8));
			}
			for (let i: i32 = 0; i < 4; ++i) {
				let c: string = t.channels[i];
				if (c == "base_r") {
					export_texture_copy_channel(pixpaint, 0, pix, i, t.color_space == "linear");
				}
				else if (c == "base_g") {
					export_texture_copy_channel(pixpaint, 1, pix, i, t.color_space == "linear");
				}
				else if (c == "base_b") {
					export_texture_copy_channel(pixpaint, 2, pix, i, t.color_space == "linear");
				}
				else if (c == "height") {
					export_texture_copy_channel(pixpaint_pack, 3, pix, i, t.color_space == "linear");
				}
				else if (c == "metal") {
					export_texture_copy_channel(pixpaint_pack, 2, pix, i, t.color_space == "linear");
				}
				else if (c == "nor_r") {
					export_texture_copy_channel(pixpaint_nor, 0, pix, i, t.color_space == "linear");
				}
				else if (c == "nor_g") {
					export_texture_copy_channel(pixpaint_nor, 1, pix, i, t.color_space == "linear");
				}
				else if (c == "nor_g_directx") {
					export_texture_copy_channel_inv(pixpaint_nor, 1, pix, i, t.color_space == "linear");
				}
				else if (c == "nor_b") {
					export_texture_copy_channel(pixpaint_nor, 2, pix, i, t.color_space == "linear");
				}
				else if (c == "occ") {
					export_texture_copy_channel(pixpaint_pack, 0, pix, i, t.color_space == "linear");
				}
				else if (c == "opac") {
					export_texture_copy_channel(pixpaint, 3, pix, i, t.color_space == "linear");
				}
				else if (c == "rough") {
					export_texture_copy_channel(pixpaint_pack, 1, pix, i, t.color_space == "linear");
				}
				else if (c == "smooth") {
					export_texture_copy_channel_inv(pixpaint_pack, 1, pix, i, t.color_space == "linear");
				}
				else if (c == "emis") {
					export_texture_extract_channel(pixpaint_nor, 3, pix, i, 3, 1, t.color_space == "linear");
				}
				else if (c == "subs") {
					export_texture_extract_channel(pixpaint_nor, 3, pix, i, 3, 2, t.color_space == "linear");
				}
				else if (c == "0.0") {
					export_texture_set_channel(0, pix, i);
				}
				else if (c == "1.0") {
					export_texture_set_channel(255, pix, i);
				}
			}
			export_texture_write_texture(path + path_sep + f + tex_name + ext, pix, 3);
		}
	}

	// Release staging memory allocated in gpu_get_texture_pixels()
	// texpaint.pixels = null;
	// texpaint_nor.pixels = null;
	// texpaint_pack.pixels = null;
}

function export_texture_write_texture(file: string, pixels: buffer_t, type: i32 = 1, off: i32 = 0) {
	let res_x: i32       = config_get_texture_res_x();
	let res_y: i32       = config_get_texture_res_y();
	let bits_handle: i32 = base_bits_handle.i;
	let bits: i32        = bits_handle == texture_bits_t.BITS8 ? 8 : bits_handle == texture_bits_t.BITS16 ? 16 : 32;
	let format: i32      = 0; // RGBA
	if (type == 1) {
		format = 2; // RGB1
	}
	if (type == 2 && off == 0) {
		format = 3; // RRR1
	}
	if (type == 2 && off == 1) {
		format = 4; // GGG1
	}
	if (type == 2 && off == 2) {
		format = 5; // BBB1
	}
	if (type == 2 && off == 3) {
		format = 6; // AAA1
	}

	if (context_raw.layers_destination == export_destination_t.PACK_INTO_PROJECT) {
		/// if IRON_BGRA
		if (format == 2) { // RGB1
			export_arm_bgra_swap(pixels);
		}
		/// end
		let image: gpu_texture_t = gpu_create_texture_from_bytes(pixels, res_x, res_y);
		map_set(data_cached_images, file, image);
		let ar: string[]   = string_split(file, path_sep);
		let name: string   = ar[ar.length - 1];
		let asset: asset_t = {name : name, file : file, id : project_asset_id++};
		array_push(project_assets, asset);
		if (project_raw.assets == null) {
			project_raw.assets = [];
		}
		array_push(project_raw.assets, asset.file);
		array_push(project_asset_names, asset.name);
		map_set(project_asset_map, asset.id, image);
		let assets: asset_t[] = [ asset ];
		export_arm_pack_assets(project_raw, assets);
		return;
	}

	if (bits == 8 && context_raw.format_type == texture_ldr_format_t.PNG) {
		iron_write_png(file, pixels, res_x, res_y, format);
	}
	else if (bits == 8 && context_raw.format_type == texture_ldr_format_t.JPG) {
		iron_write_jpg(file, pixels, res_x, res_y, format, math_floor(context_raw.format_quality));
	}
	else { // Exr
		let b: buffer_t = export_exr_run(res_x, res_y, pixels, bits, type, off);
		iron_file_save_bytes(file, b, b.length);
	}
}

/// if IRON_BGRA
function _export_texture_channel_bgra_swap(c: i32): i32 {
	return c == 0 ? 2 : c == 2 ? 0 : c;
}
/// end

function export_texture_copy_channel(from: buffer_t, from_channel: i32, to: buffer_t, to_channel: i32, linear: bool = true) {
	/// if IRON_BGRA
	from_channel = _export_texture_channel_bgra_swap(from_channel);
	/// end
	for (let i: i32 = 0; i < math_floor((to.length) / 4); ++i) {
		buffer_set_u8(to, i * 4 + to_channel, buffer_get_u8(from, i * 4 + from_channel));
	}
	if (!linear) {
		export_texture_to_srgb(to, to_channel);
	}
}

function export_texture_copy_channel_inv(from: buffer_t, from_channel: i32, to: buffer_t, to_channel: i32, linear: bool = true) {
	/// if IRON_BGRA
	from_channel = _export_texture_channel_bgra_swap(from_channel);
	/// end
	for (let i: i32 = 0; i < math_floor((to.length) / 4); ++i) {
		buffer_set_u8(to, i * 4 + to_channel, 255 - buffer_get_u8(from, i * 4 + from_channel));
	}
	if (!linear) {
		export_texture_to_srgb(to, to_channel);
	}
}

function export_texture_extract_channel(from: buffer_t, from_channel: i32, to: buffer_t, to_channel: i32, step: i32, mask: i32, linear: bool = true) {
	/// if IRON_BGRA
	from_channel = _export_texture_channel_bgra_swap(from_channel);
	/// end
	for (let i: i32 = 0; i < math_floor((to.length) / 4); ++i) {
		buffer_set_u8(to, i * 4 + to_channel, buffer_get_u8(from, i * 4 + from_channel) % step == mask ? 255 : 0);
	}
	if (!linear) {
		export_texture_to_srgb(to, to_channel);
	}
}

function export_texture_set_channel(value: i32, to: buffer_t, to_channel: i32, linear: bool = true) {
	for (let i: i32 = 0; i < math_floor((to.length) / 4); ++i) {
		buffer_set_u8(to, i * 4 + to_channel, value);
	}
	if (!linear) {
		export_texture_to_srgb(to, to_channel);
	}
}

function export_texture_to_srgb(to: buffer_t, to_channel: i32) {
	for (let i: i32 = 0; i < math_floor((to.length) / 4); ++i) {
		buffer_set_u8(to, i * 4 + to_channel, math_floor(math_pow(buffer_get_u8(to, i * 4 + to_channel) / 255, export_texture_gamma) * 255));
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
