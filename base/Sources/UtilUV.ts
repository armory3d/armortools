
///if (is_paint || is_sculpt)

class UtilUV {

	static uvmap: image_t = null;
	static uvmapCached = false;
	static trianglemap: image_t = null;
	static trianglemapCached = false;
	static dilatemap: image_t = null;
	static dilatemapCached = false;
	static uvislandmap: image_t = null;
	static uvislandmapCached = false;
	static dilateBytes: ArrayBuffer = null;
	static pipeDilate: pipeline_t = null;

	static cacheUVMap = () => {
		if (UtilUV.uvmap != null && (UtilUV.uvmap.width != Config.getTextureResX() || UtilUV.uvmap.height != Config.getTextureResY())) {
			image_unload(UtilUV.uvmap);
			UtilUV.uvmap = null;
			UtilUV.uvmapCached = false;
		}

		if (UtilUV.uvmapCached) return;

		let resX = Config.getTextureResX();
		let resY = Config.getTextureResY();
		if (UtilUV.uvmap == null) {
			UtilUV.uvmap = image_create_render_target(resX, resY);
		}

		UtilUV.uvmapCached = true;
		let merged = Context.raw.mergedObject;
		let mesh = (Context.raw.layerFilter == 0 && merged != null) ?
					merged.data : Context.raw.paintObject.data;

		let texa = mesh.vertex_arrays[2].values;
		let inda = mesh.index_arrays[0].values;
		g2_begin(UtilUV.uvmap, true, 0x00000000);
		g2_set_color(0xffcccccc);
		let strength = resX > 2048 ? 2.0 : 1.0;
		let f = (1 / 32767) * UtilUV.uvmap.width;
		for (let i = 0; i < Math.floor(inda.length / 3); ++i) {
			let x1 = (texa[inda[i * 3    ] * 2    ]) * f;
			let x2 = (texa[inda[i * 3 + 1] * 2    ]) * f;
			let x3 = (texa[inda[i * 3 + 2] * 2    ]) * f;
			let y1 = (texa[inda[i * 3    ] * 2 + 1]) * f;
			let y2 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
			let y3 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
			g2_draw_line(x1, y1, x2, y2, strength);
			g2_draw_line(x2, y2, x3, y3, strength);
			g2_draw_line(x3, y3, x1, y1, strength);
		}
		g2_end();
	}

	static cacheTriangleMap = () => {
		if (UtilUV.trianglemap != null && (UtilUV.trianglemap.width != Config.getTextureResX() || UtilUV.trianglemap.height != Config.getTextureResY())) {
			image_unload(UtilUV.trianglemap);
			UtilUV.trianglemap = null;
			UtilUV.trianglemapCached = false;
		}

		if (UtilUV.trianglemapCached) return;

		if (UtilUV.trianglemap == null) {
			UtilUV.trianglemap = image_create_render_target(Config.getTextureResX(), Config.getTextureResY());
		}

		UtilUV.trianglemapCached = true;
		let merged = Context.raw.mergedObject != null ? Context.raw.mergedObject.data : Context.raw.paintObject.data;
		let mesh = merged;
		let texa = mesh.vertex_arrays[2].values;
		let inda = mesh.index_arrays[0].values;
		g2_begin(UtilUV.trianglemap, true, 0xff000000);
		let f = (1 / 32767) * UtilUV.trianglemap.width;
		let color = 0xff000001;
		for (let i = 0; i < Math.floor(inda.length / 3); ++i) {
			if (color == 0xffffffff) color = 0xff000001;
			color++;
			g2_set_color(color);
			let x1 = (texa[inda[i * 3    ] * 2    ]) * f;
			let x2 = (texa[inda[i * 3 + 1] * 2    ]) * f;
			let x3 = (texa[inda[i * 3 + 2] * 2    ]) * f;
			let y1 = (texa[inda[i * 3    ] * 2 + 1]) * f;
			let y2 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
			let y3 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
			g2_fill_triangle(x1, y1, x2, y2, x3, y3);
		}
		g2_end();
	}

	static cacheDilateMap = () => {
		if (UtilUV.dilatemap != null && (UtilUV.dilatemap.width != Config.getTextureResX() || UtilUV.dilatemap.height != Config.getTextureResY())) {
			image_unload(UtilUV.dilatemap);
			UtilUV.dilatemap = null;
			UtilUV.dilatemapCached = false;
		}

		if (UtilUV.dilatemapCached) return;

		if (UtilUV.dilatemap == null) {
			UtilUV.dilatemap = image_create_render_target(Config.getTextureResX(), Config.getTextureResY(), tex_format_t.R8);
		}

		if (UtilUV.pipeDilate == null) {
			UtilUV.pipeDilate = g4_pipeline_create();
			UtilUV.pipeDilate.vertex_shader = sys_get_shader("dilate_map.vert");
			UtilUV.pipeDilate.fragment_shader = sys_get_shader("dilate_map.frag");
			let vs = g4_vertex_struct_create();
			///if (krom_metal || krom_vulkan)
			g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
			///else
			g4_vertex_struct_add(vs, "pos", vertex_data_t.I16_4X_NORM);
			g4_vertex_struct_add(vs, "nor", vertex_data_t.I16_2X_NORM);
			g4_vertex_struct_add(vs, "tex", vertex_data_t.I16_2X_NORM);
			///end
			UtilUV.pipeDilate.input_layout = [vs];
			UtilUV.pipeDilate.depth_write = false;
			UtilUV.pipeDilate.depth_mode = compare_mode_t.ALWAYS;
			UtilUV.pipeDilate.color_attachments[0] = tex_format_t.R8;
			g4_pipeline_compile(UtilUV.pipeDilate);
			// dilateTexUnpack = getConstantLocation(UtilUV.pipeDilate, "texUnpack");
		}

		let mask = Context.objectMaskUsed() ? SlotLayer.getObjectMask(Context.raw.layer) : 0;
		if (Context.layerFilterUsed()) mask = Context.raw.layerFilter;
		let geom = mask == 0 && Context.raw.mergedObject != null ? Context.raw.mergedObject.data : Context.raw.paintObject.data;
		g4_begin(UtilUV.dilatemap);
		g4_clear(0x00000000);
		g4_set_pipeline(UtilUV.pipeDilate);
		///if (krom_metal || krom_vulkan)
		g4_set_vertex_buffer(mesh_data_get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		g4_set_vertex_buffer(geom._vertex_buffer);
		///end
		g4_set_index_buffer(geom._index_buffers[0]);
		g4_draw();
		g4_end();
		UtilUV.dilatemapCached = true;
		UtilUV.dilateBytes = null;
	}

	static cacheUVIslandMap = () => {
		UtilUV.cacheDilateMap();
		if (UtilUV.dilateBytes == null) {
			UtilUV.dilateBytes = image_get_pixels(UtilUV.dilatemap);
		}
		UtilRender.pickPosNorTex();
		let w = 2048; // Config.getTextureResX()
		let h = 2048; // Config.getTextureResY()
		let x = Math.floor(Context.raw.uvxPicked * w);
		let y = Math.floor(Context.raw.uvyPicked * h);
		let bytes = new ArrayBuffer(w * h);
		let view = new DataView(bytes);
		let coords: TCoord[] = [{ x: x, y: y }];
		let r = Math.floor(UtilUV.dilatemap.width / w);

		let check = (c: TCoord) => {
			if (c.x < 0 || c.x >= w || c.y < 0 || c.y >= h) return;
			if (view.getUint8(c.y * w + c.x) == 255) return;
			let dilateView = new DataView(UtilUV.dilateBytes);
			if (dilateView.getUint8(c.y * r * UtilUV.dilatemap.width + c.x * r) == 0) return;
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
		UtilUV.uvislandmapCached = true;
	}
}

type TCoord = {
	x: i32;
	y: i32;
}

///end
