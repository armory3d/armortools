
///if (is_paint || is_sculpt)

class UtilUV {

	static uvmap: ImageRaw = null;
	static uvmapCached = false;
	static trianglemap: ImageRaw = null;
	static trianglemapCached = false;
	static dilatemap: ImageRaw = null;
	static dilatemapCached = false;
	static uvislandmap: ImageRaw = null;
	static uvislandmapCached = false;
	static dilateBytes: ArrayBuffer = null;
	static pipeDilate: PipelineStateRaw = null;

	static cacheUVMap = () => {
		if (UtilUV.uvmap != null && (UtilUV.uvmap.width != Config.getTextureResX() || UtilUV.uvmap.height != Config.getTextureResY())) {
			Image.unload(UtilUV.uvmap);
			UtilUV.uvmap = null;
			UtilUV.uvmapCached = false;
		}

		if (UtilUV.uvmapCached) return;

		let resX = Config.getTextureResX();
		let resY = Config.getTextureResY();
		if (UtilUV.uvmap == null) {
			UtilUV.uvmap = Image.createRenderTarget(resX, resY);
		}

		UtilUV.uvmapCached = true;
		let merged = Context.raw.mergedObject;
		let mesh = (Context.raw.layerFilter == 0 && merged != null) ?
					merged.data : Context.raw.paintObject.data;

		let texa = mesh.vertex_arrays[2].values;
		let inda = mesh.index_arrays[0].values;
		Graphics2.begin(UtilUV.uvmap.g2, true, 0x00000000);
		UtilUV.uvmap.g2.color = 0xffcccccc;
		let strength = resX > 2048 ? 2.0 : 1.0;
		let f = (1 / 32767) * UtilUV.uvmap.width;
		for (let i = 0; i < Math.floor(inda.length / 3); ++i) {
			let x1 = (texa[inda[i * 3    ] * 2    ]) * f;
			let x2 = (texa[inda[i * 3 + 1] * 2    ]) * f;
			let x3 = (texa[inda[i * 3 + 2] * 2    ]) * f;
			let y1 = (texa[inda[i * 3    ] * 2 + 1]) * f;
			let y2 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
			let y3 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
			Graphics2.drawLine(x1, y1, x2, y2, strength);
			Graphics2.drawLine(x2, y2, x3, y3, strength);
			Graphics2.drawLine(x3, y3, x1, y1, strength);
		}
		Graphics2.end(UtilUV.uvmap.g2);
	}

	static cacheTriangleMap = () => {
		if (UtilUV.trianglemap != null && (UtilUV.trianglemap.width != Config.getTextureResX() || UtilUV.trianglemap.height != Config.getTextureResY())) {
			Image.unload(UtilUV.trianglemap);
			UtilUV.trianglemap = null;
			UtilUV.trianglemapCached = false;
		}

		if (UtilUV.trianglemapCached) return;

		if (UtilUV.trianglemap == null) {
			UtilUV.trianglemap = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY());
		}

		UtilUV.trianglemapCached = true;
		let merged = Context.raw.mergedObject != null ? Context.raw.mergedObject.data : Context.raw.paintObject.data;
		let mesh = merged;
		let texa = mesh.vertex_arrays[2].values;
		let inda = mesh.index_arrays[0].values;
		Graphics2.begin(UtilUV.trianglemap.g2, true, 0xff000000);
		let f = (1 / 32767) * UtilUV.trianglemap.width;
		let color = 0xff000001;
		for (let i = 0; i < Math.floor(inda.length / 3); ++i) {
			if (color == 0xffffffff) color = 0xff000001;
			color++;
			UtilUV.trianglemap.g2.color = color;
			let x1 = (texa[inda[i * 3    ] * 2    ]) * f;
			let x2 = (texa[inda[i * 3 + 1] * 2    ]) * f;
			let x3 = (texa[inda[i * 3 + 2] * 2    ]) * f;
			let y1 = (texa[inda[i * 3    ] * 2 + 1]) * f;
			let y2 = (texa[inda[i * 3 + 1] * 2 + 1]) * f;
			let y3 = (texa[inda[i * 3 + 2] * 2 + 1]) * f;
			Graphics2.fillTriangle(x1, y1, x2, y2, x3, y3);
		}
		Graphics2.end(UtilUV.trianglemap.g2);
	}

	static cacheDilateMap = () => {
		if (UtilUV.dilatemap != null && (UtilUV.dilatemap.width != Config.getTextureResX() || UtilUV.dilatemap.height != Config.getTextureResY())) {
			Image.unload(UtilUV.dilatemap);
			UtilUV.dilatemap = null;
			UtilUV.dilatemapCached = false;
		}

		if (UtilUV.dilatemapCached) return;

		if (UtilUV.dilatemap == null) {
			UtilUV.dilatemap = Image.createRenderTarget(Config.getTextureResX(), Config.getTextureResY(), TextureFormat.R8);
		}

		if (UtilUV.pipeDilate == null) {
			UtilUV.pipeDilate = PipelineState.create();
			UtilUV.pipeDilate.vertexShader = System.getShader("dilate_map.vert");
			UtilUV.pipeDilate.fragmentShader = System.getShader("dilate_map.frag");
			let vs = VertexStructure.create();
			///if (krom_metal || krom_vulkan)
			VertexStructure.add(vs, "tex", VertexData.I16_2X_Normalized);
			///else
			VertexStructure.add(vs, "pos", VertexData.I16_4X_Normalized);
			VertexStructure.add(vs, "nor", VertexData.I16_2X_Normalized);
			VertexStructure.add(vs, "tex", VertexData.I16_2X_Normalized);
			///end
			UtilUV.pipeDilate.inputLayout = [vs];
			UtilUV.pipeDilate.depthWrite = false;
			UtilUV.pipeDilate.depthMode = CompareMode.Always;
			UtilUV.pipeDilate.colorAttachments[0] = TextureFormat.R8;
			PipelineState.compile(UtilUV.pipeDilate);
			// dilateTexUnpack = PipelineState.getConstantLocation(UtilUV.pipeDilate, "texUnpack");
		}

		let mask = Context.objectMaskUsed() ? SlotLayer.getObjectMask(Context.raw.layer) : 0;
		if (Context.layerFilterUsed()) mask = Context.raw.layerFilter;
		let geom = mask == 0 && Context.raw.mergedObject != null ? Context.raw.mergedObject.data : Context.raw.paintObject.data;
		let g4 = UtilUV.dilatemap.g4;
		Graphics4.begin(g4);
		Graphics4.clear(0x00000000);
		Graphics4.setPipeline(UtilUV.pipeDilate);
		///if (krom_metal || krom_vulkan)
		Graphics4.setVertexBuffer(MeshData.get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		Graphics4.setVertexBuffer(geom._vertexBuffer);
		///end
		Graphics4.setIndexBuffer(geom._indexBuffers[0]);
		Graphics4.drawIndexedVertices();
		Graphics4.end();
		UtilUV.dilatemapCached = true;
		UtilUV.dilateBytes = null;
	}

	static cacheUVIslandMap = () => {
		UtilUV.cacheDilateMap();
		if (UtilUV.dilateBytes == null) {
			UtilUV.dilateBytes = Image.getPixels(UtilUV.dilatemap);
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
			Image.unload(UtilUV.uvislandmap);
		}
		UtilUV.uvislandmap = Image.fromBytes(bytes, w, h, TextureFormat.R8);
		UtilUV.uvislandmapCached = true;
	}
}

type TCoord = {
	x: i32;
	y: i32;
}

///end
