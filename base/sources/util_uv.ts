
///if (is_paint || is_sculpt)

let util_uv_uvmap: image_t = null;
let util_uv_uvmap_cached: bool = false;
let util_uv_trianglemap: image_t = null;
let util_uv_trianglemap_cached: bool = false;
let util_uv_dilatemap: image_t = null;
let util_uv_dilatemap_cached: bool = false;
let util_uv_uvislandmap: image_t = null;
let util_uv_uvislandmap_cached: bool = false;
let util_uv_dilate_bytes: buffer_t = null;
let util_uv_pipe_dilate: pipeline_t = null;

function util_uv_cache_uv_map() {
	if (util_uv_uvmap != null && (util_uv_uvmap.width != config_get_texture_res_x() || util_uv_uvmap.height != config_get_texture_res_y())) {
		image_unload(util_uv_uvmap);
		util_uv_uvmap = null;
		util_uv_uvmap_cached = false;
	}

	if (util_uv_uvmap_cached) {
		return;
	}

	let res_x: i32 = config_get_texture_res_x();
	let res_y: i32 = config_get_texture_res_y();
	if (util_uv_uvmap == null) {
		util_uv_uvmap = image_create_render_target(res_x, res_y);
	}

	util_uv_uvmap_cached = true;
	let merged: mesh_object_t = context_raw.merged_object;
	let mesh: mesh_data_t = (context_raw.layer_filter == 0 && merged != null) ?
				merged.data : context_raw.paint_object.data;

	let texa: i16_array_t = mesh.vertex_arrays[2].values;
	let inda: u32_array_t = mesh.index_arrays[0].values;
	g2_begin(util_uv_uvmap);
	g2_clear(0x00000000);
	g2_set_color(0xffcccccc);
	let strength: f32 = res_x > 2048 ? 2.0 : 1.0;
	let f: f32 = (1 / 32767) * util_uv_uvmap.width;
	for (let i: i32 = 0; i < math_floor(inda.length / 3); ++i) {
		let x1: f32 = (texa[inda[i * 3    ] * 2    ]) * f;
		let x2: f32 = (texa[inda[i * 3 + 1] * 2    ]) * f;
		let x3: f32 = (texa[inda[i * 3 + 2] * 2    ]) * f;
		let y1: f32 = (texa[inda[i * 3    ] * 2 + 1]) * f;
		let y2: f32 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
		let y3: f32 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
		g2_draw_line(x1, y1, x2, y2, strength);
		g2_draw_line(x2, y2, x3, y3, strength);
		g2_draw_line(x3, y3, x1, y1, strength);
	}
	g2_end();
}

function util_uv_cache_triangle_map() {
	if (util_uv_trianglemap != null && (util_uv_trianglemap.width != config_get_texture_res_x() || util_uv_trianglemap.height != config_get_texture_res_y())) {
		image_unload(util_uv_trianglemap);
		util_uv_trianglemap = null;
		util_uv_trianglemap_cached = false;
	}

	if (util_uv_trianglemap_cached) {
		return;
	}

	if (util_uv_trianglemap == null) {
		util_uv_trianglemap = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y());
	}

	util_uv_trianglemap_cached = true;
	let merged: mesh_data_t = context_raw.merged_object != null ? context_raw.merged_object.data : context_raw.paint_object.data;
	let mesh: mesh_data_t = merged;
	let texa: i16_array_t = mesh.vertex_arrays[2].values;
	let inda: u32_array_t = mesh.index_arrays[0].values;
	g2_begin(util_uv_trianglemap);
	g2_clear(0xff000000);
	let f: f32 = (1 / 32767) * util_uv_trianglemap.width;
	let color: i32 = 0xff000001;
	for (let i: i32 = 0; i < math_floor(inda.length / 3); ++i) {
		if (color == 0xffffffff) color = 0xff000001;
		color++;
		g2_set_color(color);
		let x1: f32 = (texa[inda[i * 3    ] * 2    ]) * f;
		let x2: f32 = (texa[inda[i * 3 + 1] * 2    ]) * f;
		let x3: f32 = (texa[inda[i * 3 + 2] * 2    ]) * f;
		let y1: f32 = (texa[inda[i * 3    ] * 2 + 1]) * f;
		let y2: f32 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
		let y3: f32 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
		g2_fill_triangle(x1, y1, x2, y2, x3, y3);
	}
	g2_end();
}

function util_uv_cache_dilate_map() {
	if (util_uv_dilatemap != null && (util_uv_dilatemap.width != config_get_texture_res_x() || util_uv_dilatemap.height != config_get_texture_res_y())) {
		image_unload(util_uv_dilatemap);
		util_uv_dilatemap = null;
		util_uv_dilatemap_cached = false;
	}

	if (util_uv_dilatemap_cached) return;

	if (util_uv_dilatemap == null) {
		util_uv_dilatemap = image_create_render_target(config_get_texture_res_x(), config_get_texture_res_y(), tex_format_t.R8);
	}

	if (util_uv_pipe_dilate == null) {
		util_uv_pipe_dilate = g4_pipeline_create();
		util_uv_pipe_dilate.vertex_shader = sys_get_shader("dilate_map.vert");
		util_uv_pipe_dilate.fragment_shader = sys_get_shader("dilate_map.frag");
		let vs: vertex_struct_t = g4_vertex_struct_create();
		///if (arm_metal || arm_vulkan)
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///else
		g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
		g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
		///end
		util_uv_pipe_dilate.input_layout = [vs];
		util_uv_pipe_dilate.depth_write = false;
		util_uv_pipe_dilate.depth_mode = compare_mode_t.ALWAYS;
		util_uv_pipe_dilate.color_attachments[0] = tex_format_t.R8;
		g4_pipeline_compile(util_uv_pipe_dilate);
		// dilate_tex_unpack = getConstantLocation(pipeDilate, "tex_unpack");
	}

	let mask: i32 = context_object_mask_used() ? slot_layer_get_object_mask(context_raw.layer) : 0;
	if (context_layer_filter_used()) {
		mask = context_raw.layer_filter;
	}
	let geom: mesh_data_t = mask == 0 && context_raw.merged_object != null ? context_raw.merged_object.data : context_raw.paint_object.data;
	g4_begin(util_uv_dilatemap);
	g4_clear(0x00000000);
	g4_set_pipeline(util_uv_pipe_dilate);
	///if (arm_metal || arm_vulkan)
	let vs: vertex_element_t[] = [
		{
			name: "tex",
			data: "short2norm"
		}
	];
	g4_set_vertex_buffer(mesh_data_get(geom, vs));
	///else
	g4_set_vertex_buffer(geom._.vertex_buffer);
	///end
	g4_set_index_buffer(geom._.index_buffers[0]);
	g4_draw();
	g4_end();
	util_uv_dilatemap_cached = true;
	util_uv_dilate_bytes = null;
}

function _util_uv_check(c: coord_t, w: i32, h: i32, r: i32, view: buffer_t, coords: coord_t[]) {
	if (c.x < 0 || c.x >= w || c.y < 0 || c.y >= h) {
		return;
	}
	if (buffer_get_u8(view, c.y * w + c.x) == 255) {
		return;
	}
	let dilate_view: buffer_t = util_uv_dilate_bytes;
	if (buffer_get_u8(dilate_view, c.y * r * util_uv_dilatemap.width + c.x * r) == 0) {
		return;
	}
	buffer_set_u8(view, c.y * w + c.x, 255);
	let co: coord_t = { x: c.x + 1, y: c.y };
	array_push(coords, co);
	co = { x: c.x - 1, y: c.y };
	array_push(coords, co);
	co = { x: c.x, y: c.y + 1 };
	array_push(coords, co);
	co = { x: c.x, y: c.y - 1 };
	array_push(coords, co);
}

function util_uv_cache_uv_island_map() {
	util_uv_cache_dilate_map();
	if (util_uv_dilate_bytes == null) {
		util_uv_dilate_bytes = image_get_pixels(util_uv_dilatemap);
	}
	util_render_pick_pos_nor_tex();
	let w: i32 = 2048; // config_get_texture_res_x()
	let h: i32 = 2048; // config_get_texture_res_y()
	let x: i32 = math_floor(context_raw.uvx_picked * w);
	let y: i32 = math_floor(context_raw.uvy_picked * h);
	let bytes: buffer_t = buffer_create(w * h);
	let coords: coord_t[] = [{ x: x, y: y }];
	let r: i32 = math_floor(util_uv_dilatemap.width / w);

	while (coords.length > 0) {
		_util_uv_check(array_pop(coords), w, h, r, bytes, coords);
	}

	if (util_uv_uvislandmap != null) {
		image_unload(util_uv_uvislandmap);
	}
	util_uv_uvislandmap = image_from_bytes(bytes, w, h, tex_format_t.R8);
	util_uv_uvislandmap_cached = true;
}

type coord_t = {
	x?: i32;
	y?: i32;
}

///end
