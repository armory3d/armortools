
function util_encode_scene(raw: scene_t): buffer_t {
	return null;

	// armpack_encode_start(encoded.buffer);
	// armpack_encode_map(3);
	// armpack_encode_string("name");
	// armpack_encode_string(canvas->name);
}

function util_encode_node_canvas(c: ui_node_canvas_t) {
	ui_node_canvas_encode(c);
}

function util_encode_project(raw: project_format_t): buffer_t {

    // let size: i32 = util_encode_project_size(raw);
    let size: i32 = 128 * 1024 * 1024;
    let encoded: buffer_t = buffer_create(size);

    armpack_encode_start(encoded.buffer);
    armpack_encode_map(22);
    armpack_encode_string("version");
    armpack_encode_string(raw.version);
    armpack_encode_string("assets");
    armpack_encode_array_string(raw.assets);
    armpack_encode_string("is_bgra");
    armpack_encode_bool(raw.is_bgra);
	armpack_encode_string("packed_assets");
	if (raw.packed_assets != null) {
		armpack_encode_array(raw.packed_assets.length);
		for (let i: i32 = 0; i < raw.packed_assets.length; ++i) {
			armpack_encode_map(2);
			armpack_encode_string("name");
			armpack_encode_string(raw.packed_assets[i].name);
			armpack_encode_string("bytes");
			armpack_encode_array_u8(raw.packed_assets[i].bytes);
		}
	}
	else {
		armpack_encode_null();
	}
	armpack_encode_string("envmap");
	armpack_encode_string(raw.envmap);
	armpack_encode_string("envmap_strength");
	armpack_encode_f32(raw.envmap_strength);
	armpack_encode_string("camera_world");
	armpack_encode_array_f32(raw.camera_world);
	armpack_encode_string("camera_origin");
	armpack_encode_array_f32(raw.camera_origin);
	armpack_encode_string("camera_fov");
	armpack_encode_f32(raw.camera_fov);
	armpack_encode_string("swatches");
	if (raw.swatches != null) {
		armpack_encode_array(raw.swatches.length);
		for (let i: i32 = 0; i < raw.swatches.length; ++i) {
			armpack_encode_map(9);
			armpack_encode_string("base");
			armpack_encode_i32(raw.swatches[i].base);
			armpack_encode_string("opacity");
			armpack_encode_f32(raw.swatches[i].opacity);
			armpack_encode_string("occlusion");
			armpack_encode_f32(raw.swatches[i].occlusion);
			armpack_encode_string("roughness");
			armpack_encode_f32(raw.swatches[i].roughness);
			armpack_encode_string("metallic");
			armpack_encode_f32(raw.swatches[i].metallic);
			armpack_encode_string("normal");
			armpack_encode_i32(raw.swatches[i].normal);
			armpack_encode_string("emission");
			armpack_encode_f32(raw.swatches[i].emission);
			armpack_encode_string("height");
			armpack_encode_f32(raw.swatches[i].height);
			armpack_encode_string("subsurface");
			armpack_encode_f32(raw.swatches[i].subsurface);
		}
	}
	else {
		armpack_encode_null();
	}

	///if (is_paint || is_sculpt)
	armpack_encode_string("brush_nodes");
	armpack_encode_array(raw.brush_nodes.length);
	for (let i: i32 = 0; i < raw.brush_nodes.length; ++i) {
		util_encode_node_canvas(raw.brush_nodes[i]);
	}
	armpack_encode_string("brush_icons");
	if (raw.brush_icons != null) {
		armpack_encode_array(raw.brush_icons.length);
		for (let i: i32 = 0; i < raw.brush_icons.length; ++i) {
			armpack_encode_array_u8(raw.brush_icons[i]);
		}
	}
	else {
		armpack_encode_null();
	}
	armpack_encode_string("material_nodes");
	armpack_encode_array(raw.material_nodes.length);
	for (let i: i32 = 0; i < raw.material_nodes.length; ++i) {
		util_encode_node_canvas(raw.material_nodes[i]);
	}
	armpack_encode_string("material_groups");
	if (raw.material_groups != null) {
		armpack_encode_array(raw.material_groups.length);
		for (let i: i32 = 0; i < raw.material_groups.length; ++i) {
			util_encode_node_canvas(raw.material_groups[i]);
		}
	}
	else {
		armpack_encode_null();
	}
	armpack_encode_string("material_icons");
	if (raw.material_icons != null) {
		armpack_encode_array(raw.material_icons.length);
		for (let i: i32 = 0; i < raw.material_icons.length; ++i) {
			armpack_encode_array_u8(raw.material_icons[i]);
		}
	}
	else {
		armpack_encode_null();
	}
	armpack_encode_string("font_assets");
	armpack_encode_array_string(raw.font_assets);
	armpack_encode_string("layer_datas");
	armpack_encode_array(raw.layer_datas.length);
	for (let i: i32 = 0; i < raw.layer_datas.length; ++i) {
		armpack_encode_map(27);
		armpack_encode_string("name");
		armpack_encode_string(raw.layer_datas[i].name);
		armpack_encode_string("res");
		armpack_encode_i32(raw.layer_datas[i].res);
		armpack_encode_string("bpp");
		armpack_encode_i32(raw.layer_datas[i].bpp);
		armpack_encode_string("texpaint");
		armpack_encode_array_u8(raw.layer_datas[i].texpaint);
		armpack_encode_string("uv_scale");
		armpack_encode_f32(raw.layer_datas[i].uv_scale);
		armpack_encode_string("uv_rot");
		armpack_encode_f32(raw.layer_datas[i].uv_rot);
		armpack_encode_string("uv_type");
		armpack_encode_i32(raw.layer_datas[i].uv_type);
		armpack_encode_string("decal_mat");
		armpack_encode_array_f32(raw.layer_datas[i].decal_mat);
		armpack_encode_string("opacity_mask");
		armpack_encode_f32(raw.layer_datas[i].opacity_mask);
		armpack_encode_string("fill_layer");
		armpack_encode_i32(raw.layer_datas[i].fill_layer);
		armpack_encode_string("object_mask");
		armpack_encode_i32(raw.layer_datas[i].object_mask);
		armpack_encode_string("blending");
		armpack_encode_i32(raw.layer_datas[i].blending);
		armpack_encode_string("parent");
		armpack_encode_i32(raw.layer_datas[i].parent);
		armpack_encode_string("visible");
		armpack_encode_bool(raw.layer_datas[i].visible);
		///if is_paint
		armpack_encode_string("texpaint_nor");
		armpack_encode_array_u8(raw.layer_datas[i].texpaint_nor);
		armpack_encode_string("texpaint_pack");
		armpack_encode_array_u8(raw.layer_datas[i].texpaint_pack);
		armpack_encode_string("paint_base");
		armpack_encode_bool(raw.layer_datas[i].paint_base);
		armpack_encode_string("paint_opac");
		armpack_encode_bool(raw.layer_datas[i].paint_opac);
		armpack_encode_string("paint_occ");
		armpack_encode_bool(raw.layer_datas[i].paint_occ);
		armpack_encode_string("paint_rough");
		armpack_encode_bool(raw.layer_datas[i].paint_rough);
		armpack_encode_string("paint_met");
		armpack_encode_bool(raw.layer_datas[i].paint_met);
		armpack_encode_string("paint_nor");
		armpack_encode_bool(raw.layer_datas[i].paint_nor);
		armpack_encode_string("paint_nor_blend");
		armpack_encode_bool(raw.layer_datas[i].paint_nor_blend);
		armpack_encode_string("paint_height");
		armpack_encode_bool(raw.layer_datas[i].paint_height);
		armpack_encode_string("paint_height_blend");
		armpack_encode_bool(raw.layer_datas[i].paint_height_blend);
		armpack_encode_string("paint_emis");
		armpack_encode_bool(raw.layer_datas[i].paint_emis);
		armpack_encode_string("paint_subs");
		armpack_encode_bool(raw.layer_datas[i].paint_subs);
		///end
	}
	armpack_encode_string("mesh_datas");
	armpack_encode_array(raw.mesh_datas.length);
	for (let i: i32 = 0; i < raw.mesh_datas.length; ++i) {
		armpack_encode_map(7);
		armpack_encode_string("name");
		armpack_encode_string(raw.mesh_datas[i].name);
		armpack_encode_string("scale_pos");
		armpack_encode_f32(raw.mesh_datas[i].scale_pos);
		armpack_encode_string("scale_tex");
		armpack_encode_f32(raw.mesh_datas[i].scale_tex);
		armpack_encode_string("instancing");
		armpack_encode_null(); // mesh_data_instancing_t
		armpack_encode_string("skin");
		armpack_encode_null(); // skin_t
		armpack_encode_string("vertex_arrays");
		armpack_encode_array(raw.mesh_datas[i].vertex_arrays.length);
		for (let j: i32 = 0; j < raw.mesh_datas[i].vertex_arrays.length; ++j) {
			armpack_encode_map(3);
			armpack_encode_string("attrib");
			armpack_encode_string(raw.mesh_datas[i].vertex_arrays[j].attrib);
			armpack_encode_string("data");
			armpack_encode_string(raw.mesh_datas[i].vertex_arrays[j].data);
			armpack_encode_string("values");
			armpack_encode_array_i16(raw.mesh_datas[i].vertex_arrays[j].values);
		}
		armpack_encode_string("index_arrays");
		armpack_encode_array(raw.mesh_datas[i].index_arrays.length);
		for (let j: i32 = 0; j < raw.mesh_datas[i].index_arrays.length; ++j) {
			armpack_encode_map(2);
			armpack_encode_string("material");
			armpack_encode_i32(raw.mesh_datas[i].index_arrays[j].material);
			armpack_encode_string("values");
			armpack_encode_array_i32(raw.mesh_datas[i].index_arrays[j].values);
		}
	}
	armpack_encode_string("mesh_assets");
	armpack_encode_array_string(raw.mesh_assets);
	armpack_encode_string("mesh_icons");
	if (raw.mesh_icons != null) {
		armpack_encode_array(raw.mesh_icons.length);
		for (let i: i32 = 0; i < raw.mesh_icons.length; ++i) {
			armpack_encode_array_u8(raw.mesh_icons[i]);
		}
	}
	else {
		armpack_encode_null();
	}
	///end

	///if is_paint
	armpack_encode_string("atlas_objects");
	armpack_encode_array_i32(raw.atlas_objects);
	armpack_encode_string("atlas_names");
	armpack_encode_array_string(raw.atlas_names);
	///end

	///if is_lab
	// material?: ui_node_canvas_t;
	// material_groups?: ui_node_canvas_t[];
	// mesh_data?: mesh_data_t;
	// mesh_icon?: buffer_t;
	///end

	let ei: i32 = armpack_encode_end();
	encoded.length = ei;

    return encoded;
}
