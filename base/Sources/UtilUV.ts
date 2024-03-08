
///if (is_paint || is_sculpt)

class UtilUV {

	static uvmap: image_t = null;
	static uvmap_cached: bool = false;
	static trianglemap: image_t = null;
	static trianglemap_cached: bool = false;
	static dilatemap: image_t = null;
	static dilatemap_cached: bool = false;
	static uvislandmap: image_t = null;
	static uvislandmap_cached: bool = false;
	static dilate_bytes: ArrayBuffer = null;
	static pipe_dilate: pipeline_t = null;

	static cache_uv_map = () => {
		if (UtilUV.uvmap != null && (UtilUV.uvmap.width != Config.get_texture_res_x() || UtilUV.uvmap.height != Config.get_texture_res_y())) {
			image_unload(UtilUV.uvmap);
			UtilUV.uvmap = null;
			UtilUV.uvmap_cached = false;
		}

		if (UtilUV.uvmap_cached) return;

		let res_x: i32 = Config.get_texture_res_x();
		let res_y: i32 = Config.get_texture_res_y();
		if (UtilUV.uvmap == null) {
			UtilUV.uvmap = image_create_render_target(res_x, res_y);
		}

		UtilUV.uvmap_cached = true;
		let merged: mesh_object_t = Context.raw.merged_object;
		let mesh: mesh_data_t = (Context.raw.layer_filter == 0 && merged != null) ?
					merged.data : Context.raw.paint_object.data;

		let texa: i16_array_t = mesh.vertex_arrays[2].values;
		let inda: u32_array_t = mesh.index_arrays[0].values;
		g2_begin(UtilUV.uvmap);
		g2_clear(0x00000000);
		g2_set_color(0xffcccccc);
		let strength: f32 = res_x > 2048 ? 2.0 : 1.0;
		let f: f32 = (1 / 32767) * UtilUV.uvmap.width;
		for (let i: i32 = 0; i < Math.floor(inda.length / 3); ++i) {
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

	static cache_triangle_map = () => {
		if (UtilUV.trianglemap != null && (UtilUV.trianglemap.width != Config.get_texture_res_x() || UtilUV.trianglemap.height != Config.get_texture_res_y())) {
			image_unload(UtilUV.trianglemap);
			UtilUV.trianglemap = null;
			UtilUV.trianglemap_cached = false;
		}

		if (UtilUV.trianglemap_cached) return;

		if (UtilUV.trianglemap == null) {
			UtilUV.trianglemap = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y());
		}

		UtilUV.trianglemap_cached = true;
		let merged: mesh_data_t = Context.raw.merged_object != null ? Context.raw.merged_object.data : Context.raw.paint_object.data;
		let mesh: mesh_data_t = merged;
		let texa: i16_array_t = mesh.vertex_arrays[2].values;
		let inda: u32_array_t = mesh.index_arrays[0].values;
		g2_begin(UtilUV.trianglemap);
		g2_clear(0xff000000);
		let f: f32 = (1 / 32767) * UtilUV.trianglemap.width;
		let color: i32 = 0xff000001;
		for (let i: i32 = 0; i < Math.floor(inda.length / 3); ++i) {
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

	static cache_dilate_map = () => {
		if (UtilUV.dilatemap != null && (UtilUV.dilatemap.width != Config.get_texture_res_x() || UtilUV.dilatemap.height != Config.get_texture_res_y())) {
			image_unload(UtilUV.dilatemap);
			UtilUV.dilatemap = null;
			UtilUV.dilatemap_cached = false;
		}

		if (UtilUV.dilatemap_cached) return;

		if (UtilUV.dilatemap == null) {
			UtilUV.dilatemap = image_create_render_target(Config.get_texture_res_x(), Config.get_texture_res_y(), tex_format_t.R8);
		}

		if (UtilUV.pipe_dilate == null) {
			UtilUV.pipe_dilate = g4_pipeline_create();
			UtilUV.pipe_dilate.vertex_shader = sys_get_shader("dilate_map.vert");
			UtilUV.pipe_dilate.fragment_shader = sys_get_shader("dilate_map.frag");
			let vs: vertex_struct_t = g4_vertex_struct_create();
			///if (krom_metal || krom_vulkan)
			g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
			///else
			g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
			g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
			///end
			UtilUV.pipe_dilate.input_layout = [vs];
			UtilUV.pipe_dilate.depth_write = false;
			UtilUV.pipe_dilate.depth_mode = compare_mode_t.ALWAYS;
			UtilUV.pipe_dilate.color_attachments[0] = tex_format_t.R8;
			g4_pipeline_compile(UtilUV.pipe_dilate);
			// dilateTexUnpack = getConstantLocation(UtilUV.pipeDilate, "texUnpack");
		}

		let mask: i32 = Context.object_mask_used() ? SlotLayer.get_object_mask(Context.raw.layer) : 0;
		if (Context.layer_filter_used()) mask = Context.raw.layer_filter;
		let geom: mesh_data_t = mask == 0 && Context.raw.merged_object != null ? Context.raw.merged_object.data : Context.raw.paint_object.data;
		g4_begin(UtilUV.dilatemap);
		g4_clear(0x00000000);
		g4_set_pipeline(UtilUV.pipe_dilate);
		///if (krom_metal || krom_vulkan)
		g4_set_vertex_buffer(mesh_data_get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		g4_set_vertex_buffer(geom._.vertex_buffer);
		///end
		g4_set_index_buffer(geom._.index_buffers[0]);
		g4_draw();
		g4_end();
		UtilUV.dilatemap_cached = true;
		UtilUV.dilate_bytes = null;
	}

	static cache_uv_island_map = () => {
		UtilUV.cache_dilate_map();
		if (UtilUV.dilate_bytes == null) {
			UtilUV.dilate_bytes = image_get_pixels(UtilUV.dilatemap);
		}
		UtilRender.pick_pos_nor_tex();
		let w: i32 = 2048; // Config.getTextureResX()
		let h: i32 = 2048; // Config.getTextureResY()
		let x: i32 = Math.floor(Context.raw.uvx_picked * w);
		let y: i32 = Math.floor(Context.raw.uvy_picked * h);
		let bytes: ArrayBuffer = new ArrayBuffer(w * h);
		let view: DataView = new DataView(bytes);
		let coords: coord_t[] = [{ x: x, y: y }];
		let r: i32 = Math.floor(UtilUV.dilatemap.width / w);

		let check = (c: coord_t) => {
			if (c.x < 0 || c.x >= w || c.y < 0 || c.y >= h) return;
			if (view.getUint8(c.y * w + c.x) == 255) return;
			let dilate_view: DataView = new DataView(UtilUV.dilate_bytes);
			if (dilate_view.getUint8(c.y * r * UtilUV.dilatemap.width + c.x * r) == 0) return;
			view.setUint8(c.y * w + c.x, 255);
			coords.push({ x: c.x + 1, y: c.y });
			coords.push({ x: c.x - 1, y: c.y });
			coords.push({ x: c.x, y: c.y + 1 });
			coords.push({ x: c.x, y: c.y - 1 });
		}

		while (coords.length > 0) {
			check(coords.pop());
		}

		if (UtilUV.uvislandmap != null) {
			image_unload(UtilUV.uvislandmap);
		}
		UtilUV.uvislandmap = image_from_bytes(bytes, w, h, tex_format_t.R8);
		UtilUV.uvislandmap_cached = true;
	}
}

type coord_t = {
	x?: i32;
	y?: i32;
}

///end
