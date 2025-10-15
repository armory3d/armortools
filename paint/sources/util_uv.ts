
let util_uv_uvmap: gpu_texture_t        = null;
let util_uv_uvmap_cached: bool          = false;
let util_uv_trianglemap: gpu_texture_t  = null;
let util_uv_trianglemap_cached: bool    = false;
let util_uv_dilatemap: gpu_texture_t    = null;
let util_uv_dilatemap_cached: bool      = false;
let util_uv_uvislandmap: gpu_texture_t  = null;
let util_uv_uvislandmap_cached: bool    = false;
let util_uv_dilate_bytes: buffer_t      = null;
let util_uv_pipe_dilate: gpu_pipeline_t = null;

function util_uv_cache_uv_map() {
	if (util_uv_uvmap != null && (util_uv_uvmap.width != config_get_texture_res_x() || util_uv_uvmap.height != config_get_texture_res_y())) {
		gpu_delete_texture(util_uv_uvmap);
		util_uv_uvmap        = null;
		util_uv_uvmap_cached = false;
	}

	if (util_uv_uvmap_cached) {
		return;
	}

	let res_x: i32 = config_get_texture_res_x();
	let res_y: i32 = config_get_texture_res_y();
	if (util_uv_uvmap == null) {
		util_uv_uvmap = gpu_create_render_target(res_x, res_y);
	}

	util_uv_uvmap_cached      = true;
	let merged: mesh_object_t = context_raw.merged_object;
	let mesh: mesh_data_t     = (context_raw.layer_filter == 0 && merged != null) ? merged.data : context_raw.paint_object.data;

	let texa: i16_array_t = mesh.vertex_arrays[2].values;
	let inda: u32_array_t = mesh.index_array;
	draw_begin(util_uv_uvmap, true, 0x00000000);
	draw_set_color(0xffffffff);
	let strength: f32 = res_x > 2048 ? 2.0 : 1.0;
	let f: f32        = (1 / 32767) * util_uv_uvmap.width;
	for (let i: i32 = 0; i < math_floor(inda.length / 3); ++i) {
		let x1: f32 = (texa[inda[i * 3] * 2]) * f;
		let x2: f32 = (texa[inda[i * 3 + 1] * 2]) * f;
		let x3: f32 = (texa[inda[i * 3 + 2] * 2]) * f;
		let y1: f32 = (texa[inda[i * 3] * 2 + 1]) * f;
		let y2: f32 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
		let y3: f32 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
		draw_line_aa(x1, y1, x2, y2, strength);
		draw_line_aa(x2, y2, x3, y3, strength);
		draw_line_aa(x3, y3, x1, y1, strength);
	}
	draw_end();
}

function util_uv_cache_triangle_map() {
	if (util_uv_trianglemap != null && (util_uv_trianglemap.width != config_get_texture_res_x() || util_uv_trianglemap.height != config_get_texture_res_y())) {
		gpu_delete_texture(util_uv_trianglemap);
		util_uv_trianglemap        = null;
		util_uv_trianglemap_cached = false;
	}

	if (util_uv_trianglemap_cached) {
		return;
	}

	if (util_uv_trianglemap == null) {
		util_uv_trianglemap = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());
	}

	util_uv_trianglemap_cached = true;
	let merged: mesh_data_t    = context_raw.merged_object != null ? context_raw.merged_object.data : context_raw.paint_object.data;
	let mesh: mesh_data_t      = merged;
	let texa: i16_array_t      = mesh.vertex_arrays[2].values;
	let inda: u32_array_t      = mesh.index_array;
	draw_begin(util_uv_trianglemap, true, 0xff000000);
	let f: f32     = (1 / 32767) * util_uv_trianglemap.width;
	let color: i32 = 0xff000001;
	for (let i: i32 = 0; i < math_floor(inda.length / 3); ++i) {
		if (color == 0xffffffff)
			color = 0xff000001;
		color++;
		draw_set_color(color);
		let x1: f32 = (texa[inda[i * 3] * 2]) * f;
		let x2: f32 = (texa[inda[i * 3 + 1] * 2]) * f;
		let x3: f32 = (texa[inda[i * 3 + 2] * 2]) * f;
		let y1: f32 = (texa[inda[i * 3] * 2 + 1]) * f;
		let y2: f32 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
		let y3: f32 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
		draw_filled_triangle(x1, y1, x2, y2, x3, y3);
	}
	draw_end();
}

function util_uv_cache_dilate_map() {
	if (util_uv_dilatemap != null && (util_uv_dilatemap.width != config_get_texture_res_x() || util_uv_dilatemap.height != config_get_texture_res_y())) {
		gpu_delete_texture(util_uv_dilatemap);
		util_uv_dilatemap        = null;
		util_uv_dilatemap_cached = false;
	}

	if (util_uv_dilatemap_cached)
		return;

	if (util_uv_dilatemap == null) {
		util_uv_dilatemap = gpu_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);
	}

	if (util_uv_pipe_dilate == null) {
		util_uv_pipe_dilate                 = gpu_create_pipeline();
		util_uv_pipe_dilate.vertex_shader   = sys_get_shader("dilate_map.vert");
		util_uv_pipe_dilate.fragment_shader = sys_get_shader("dilate_map.frag");
		let vs: gpu_vertex_structure_t      = {};
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
		gpu_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
		gpu_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		util_uv_pipe_dilate.input_layout                      = vs;
		util_uv_pipe_dilate.depth_write                       = false;
		util_uv_pipe_dilate.depth_mode                        = compare_mode_t.ALWAYS;
		ARRAY_ACCESS(util_uv_pipe_dilate.color_attachment, 0) = tex_format_t.R8;
		gpu_pipeline_compile(util_uv_pipe_dilate);
		// dilate_tex_unpack = getConstantLocation(pipeDilate, "tex_unpack");
	}

	let mask: i32 = context_object_mask_used() ? slot_layer_get_object_mask(context_raw.layer) : 0;
	if (context_layer_filter_used()) {
		mask = context_raw.layer_filter;
	}
	let geom: mesh_data_t = mask == 0 && context_raw.merged_object != null ? context_raw.merged_object.data : context_raw.paint_object.data;
	_gpu_begin(util_uv_dilatemap, null, null, clear_flag_t.COLOR, 0x00000000);
	gpu_set_pipeline(util_uv_pipe_dilate);
	gpu_set_vertex_buffer(geom._.vertex_buffer);
	gpu_set_index_buffer(geom._.index_buffer);
	gpu_draw();
	gpu_end();
	util_uv_dilatemap_cached = true;
	util_uv_dilate_bytes     = null;
}

function _util_uv_check(cx: i32, cy: i32, w: i32, h: i32, r: i32, view: buffer_t, coords_x: i32[], coords_y: i32[]) {
	if (cx < 0 || cx >= w || cy < 0 || cy >= h) {
		return;
	}
	if (buffer_get_u8(view, cy * w + cx) == 255) {
		return;
	}
	if (buffer_get_u8(util_uv_dilate_bytes, cy * r * util_uv_dilatemap.width + cx * r) == 0) {
		return;
	}
	buffer_set_u8(view, cy * w + cx, 255);

	array_push(coords_x, cx + 1);
	array_push(coords_y, cy);

	array_push(coords_x, cx - 1);
	array_push(coords_y, cy);

	array_push(coords_x, cx);
	array_push(coords_y, cy + 1);

	array_push(coords_x, cx);
	array_push(coords_y, cy - 1);
}

function util_uv_cache_uv_island_map() {
	util_uv_cache_dilate_map();
	if (util_uv_dilate_bytes == null) {
		util_uv_dilate_bytes = gpu_get_texture_pixels(util_uv_dilatemap);
	}
	util_render_pick_pos_nor_tex();
	let w: i32          = 2048; // config_get_texture_res_x()
	let h: i32          = 2048; // config_get_texture_res_y()
	let x: i32          = math_floor(context_raw.uvx_picked * w);
	let y: i32          = math_floor(context_raw.uvy_picked * h);
	let bytes: buffer_t = buffer_create(w * h);
	let coords_x: i32[] = [ x ];
	let coords_y: i32[] = [ y ];
	let r: i32          = math_floor(util_uv_dilatemap.width / w);

	while (coords_x.length > 0) {
		let cx: i32 = i32_array_pop(coords_x);
		let cy: i32 = i32_array_pop(coords_y);
		_util_uv_check(cx, cy, w, h, r, bytes, coords_x, coords_y);
	}

	if (util_uv_uvislandmap != null) {
		gpu_delete_texture(util_uv_uvislandmap);
	}
	util_uv_uvislandmap        = gpu_create_texture_from_bytes(bytes, w, h, tex_format_t.R8);
	util_uv_uvislandmap_cached = true;
}
