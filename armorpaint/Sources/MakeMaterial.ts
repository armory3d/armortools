
class MakeMaterial {

	static defaultScon: shader_context_t = null;
	static defaultMcon: material_context_t = null;

	static heightUsed = false;
	static emisUsed = false;
	static subsUsed = false;

	static getMOut = (): bool => {
		for (let n of UINodes.getCanvasMaterial().nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
		return false;
	}

	static parseMeshMaterial = () => {
		let m = Project.materials[0].data;

		for (let c of m._shader._contexts) {
			if (c.name == "mesh") {
				array_remove(m._shader.contexts, c);
				array_remove(m._shader._contexts, c);
				MakeMaterial.deleteContext(c);
				break;
			}
		}

		if (MakeMesh.layerPassCount > 1) {
			let i = 0;
			while (i < m._shader._contexts.length) {
				let c = m._shader._contexts[i];
				for (let j = 1; j < MakeMesh.layerPassCount; ++j) {
					if (c.name == "mesh" + j) {
						array_remove(m._shader.contexts, c);
						array_remove(m._shader._contexts, c);
						MakeMaterial.deleteContext(c);
						i--;
						break;
					}
				}
				i++;
			}

			i = 0;
			while (i < m._contexts.length) {
				let c = m._contexts[i];
				for (let j = 1; j < MakeMesh.layerPassCount; ++j) {
					if (c.name == "mesh" + j) {
						array_remove(m.contexts, c);
						array_remove(m._contexts, c);
						i--;
						break;
					}
				}
				i++;
			}
		}

		let con = MakeMesh.run({ name: "Material", canvas: null });
		let scon: shader_context_t;
		shader_context_create(con.data, (_scon: shader_context_t) => { scon = _scon; });
		scon._override_context = {};
		if (con.frag.sharedSamplers.length > 0) {
			let sampler = con.frag.sharedSamplers[0];
			scon._override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!Context.raw.textureFilter) {
			scon._override_context.filter = "point";
		}
		m._shader.contexts.push(scon);
		m._shader._contexts.push(scon);

		for (let i = 1; i < MakeMesh.layerPassCount; ++i) {
			let con = MakeMesh.run({ name: "Material", canvas: null }, i);
			let scon: shader_context_t;
			shader_context_create(con.data, (_scon: shader_context_t) => { scon = _scon; });
			scon._override_context = {};
			if (con.frag.sharedSamplers.length > 0) {
				let sampler = con.frag.sharedSamplers[0];
				scon._override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
			}
			if (!Context.raw.textureFilter) {
				scon._override_context.filter = "point";
			}
			m._shader.contexts.push(scon);
			m._shader._contexts.push(scon);

			let mcon: material_context_t;
			material_context_create({ name: "mesh" + i, bind_textures: [] }, (self: material_context_t) => { mcon = self; });
			m.contexts.push(mcon);
			m._contexts.push(mcon);
		}

		Context.raw.ddirty = 2;

		///if arm_voxels
		MakeMaterial.makeVoxel(m);
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.dirty = 1;
		///end
	}

	static parseParticleMaterial = () => {
		let m = Context.raw.particleMaterial;
		let sc: shader_context_t = null;
		for (let c of m._shader._contexts) {
			if (c.name == "mesh") {
				sc = c;
				break;
			}
		}
		if (sc != null) {
			array_remove(m._shader.contexts, sc);
			array_remove(m._shader._contexts, sc);
		}
		let con = MakeParticle.run({ name: "MaterialParticle", canvas: null });
		if (sc != null) MakeMaterial.deleteContext(sc);
		shader_context_create(con.data, (_sc: shader_context_t) => { sc = _sc; });
		m._shader.contexts.push(sc);
		m._shader._contexts.push(sc);
	}

	static parseMeshPreviewMaterial = (md: material_data_t = null) => {
		if (!MakeMaterial.getMOut()) return;

		let m = md == null ? Project.materials[0].data : md;
		let scon: shader_context_t = null;
		for (let c of m._shader._contexts) {
			if (c.name == "mesh") {
				scon = c;
				break;
			}
		}

		array_remove(m._shader.contexts, scon);
		array_remove(m._shader._contexts, scon);

		let mcon: material_context_t = { name: "mesh", bind_textures: [] };

		let sd: TMaterial = { name: "Material", canvas: null };
		let con = MakeMeshPreview.run(sd, mcon);

		for (let i = 0; i < m._contexts.length; ++i) {
			if (m._contexts[i].name == "mesh") {
				material_context_create(mcon, (self: material_context_t) => { m._contexts[i] = self; });
				break;
			}
		}

		if (scon != null) MakeMaterial.deleteContext(scon);

		let compileError = false;
		shader_context_create(con.data, (_scon: shader_context_t) => {
			if (_scon == null) compileError = true;
			scon = _scon;
		});
		if (compileError) return;

		m._shader.contexts.push(scon);
		m._shader._contexts.push(scon);
	}

	///if arm_voxels
	static makeVoxel = (m: material_data_t) => {
		let rebuild = MakeMaterial.heightUsed;
		if (Config.raw.rp_gi != false && rebuild) {
			let scon: shader_context_t = null;
			for (let c of m._shader._contexts) {
				if (c.name == "voxel") {
					scon = c;
					break;
				}
			}
			if (scon != null) MakeVoxel.run(scon);
		}
	}
	///end

	static parsePaintMaterial = (bakePreviews = true) => {
		if (!MakeMaterial.getMOut()) return;

		if (bakePreviews) {
			let current = _g2_current;
			if (current != null) g2_end();
			MakeMaterial.bakeNodePreviews();
			if (current != null) g2_begin(current, false);
		}

		let m = Project.materials[0].data;
		// let scon: TShaderContext = null;
		// let mcon: TMaterialContext = null;
		for (let c of m._shader._contexts) {
			if (c.name == "paint") {
				array_remove(m._shader.contexts, c);
				array_remove(m._shader._contexts, c);
				if (c != MakeMaterial.defaultScon) MakeMaterial.deleteContext(c);
				break;
			}
		}
		for (let c of m._contexts) {
			if (c.name == "paint") {
				array_remove(m.contexts, c);
				array_remove(m._contexts, c);
				break;
			}
		}

		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let tmcon: material_context_t = { name: "paint", bind_textures: [] };
		let con = MakePaint.run(sdata, tmcon);

		let compileError = false;
		let scon: shader_context_t;
		shader_context_create(con.data, (_scon: shader_context_t) => {
			if (_scon == null) compileError = true;
			scon = _scon;
		});
		if (compileError) return;
		scon._override_context = {};
		scon._override_context.addressing = "repeat";
		let mcon: material_context_t;
		material_context_create(tmcon, (_mcon: material_context_t) => { mcon = _mcon; });

		m._shader.contexts.push(scon);
		m._shader._contexts.push(scon);
		m.contexts.push(mcon);
		m._contexts.push(mcon);

		if (MakeMaterial.defaultScon == null) MakeMaterial.defaultScon = scon;
		if (MakeMaterial.defaultMcon == null) MakeMaterial.defaultMcon = mcon;
	}

	static bakeNodePreviews = () => {
		Context.raw.nodePreviewsUsed = [];
		if (Context.raw.nodePreviews == null) Context.raw.nodePreviews = new Map();
		MakeMaterial.traverseNodes(UINodes.getCanvasMaterial().nodes, null, []);
		for (let key of Context.raw.nodePreviews.keys()) {
			if (Context.raw.nodePreviewsUsed.indexOf(key) == -1) {
				let image = Context.raw.nodePreviews.get(key);
				Base.notifyOnNextFrame(function() { image_unload(image); });
				Context.raw.nodePreviews.delete(key);
			}
		}
	}

	static traverseNodes = (nodes: zui_node_t[], group: zui_node_canvas_t, parents: zui_node_t[]) => {
		for (let node of nodes) {
			MakeMaterial.bakeNodePreview(node, group, parents);
			if (node.type == "GROUP") {
				for (let g of Project.materialGroups) {
					if (g.canvas.name == node.name) {
						parents.push(node);
						MakeMaterial.traverseNodes(g.canvas.nodes, g.canvas, parents);
						parents.pop();
						break;
					}
				}
			}
		}
	}

	static bakeNodePreview = (node: zui_node_t, group: zui_node_canvas_t, parents: zui_node_t[]) => {
		if (node.type == "BLUR") {
			let id = ParserMaterial.node_name(node, parents);
			let image = Context.raw.nodePreviews.get(id);
			Context.raw.nodePreviewsUsed.push(id);
			let resX = Math.floor(Config.getTextureResX() / 4);
			let resY = Math.floor(Config.getTextureResY() / 4);
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY);
				Context.raw.nodePreviews.set(id, image);
			}

			ParserMaterial.blur_passthrough = true;
			UtilRender.makeNodePreview(UINodes.getCanvasMaterial(), node, image, group, parents);
			ParserMaterial.blur_passthrough = false;
		}
		else if (node.type == "DIRECT_WARP") {
			let id = ParserMaterial.node_name(node, parents);
			let image = Context.raw.nodePreviews.get(id);
			Context.raw.nodePreviewsUsed.push(id);
			let resX = Math.floor(Config.getTextureResX());
			let resY = Math.floor(Config.getTextureResY());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY);
				Context.raw.nodePreviews.set(id, image);
			}

			ParserMaterial.warp_passthrough = true;
			UtilRender.makeNodePreview(UINodes.getCanvasMaterial(), node, image, group, parents);
			ParserMaterial.warp_passthrough = false;
		}
		else if (node.type == "BAKE_CURVATURE") {
			let id = ParserMaterial.node_name(node, parents);
			let image = Context.raw.nodePreviews.get(id);
			Context.raw.nodePreviewsUsed.push(id);
			let resX = Math.floor(Config.getTextureResX());
			let resY = Math.floor(Config.getTextureResY());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY, tex_format_t.R8);
				Context.raw.nodePreviews.set(id, image);
			}

			if (RenderPathPaint.liveLayer == null) {
				RenderPathPaint.liveLayer = SlotLayer.create("_live");
			}

			let _space = UIHeader.worktab.position;
			let _tool = Context.raw.tool;
			let _bakeType = Context.raw.bakeType;
			UIHeader.worktab.position = SpaceType.Space3D;
			Context.raw.tool = WorkspaceTool.ToolBake;
			Context.raw.bakeType = BakeType.BakeCurvature;

			ParserMaterial.bake_passthrough = true;
			ParserMaterial.start_node = node;
			ParserMaterial.start_group = group;
			ParserMaterial.start_parents = parents;
			MakeMaterial.parsePaintMaterial(false);
			ParserMaterial.bake_passthrough = false;
			ParserMaterial.start_node = null;
			ParserMaterial.start_group = null;
			ParserMaterial.start_parents = null;
			Context.raw.pdirty = 1;
			RenderPathPaint.useLiveLayer(true);
			RenderPathPaint.commandsPaint(false);
			RenderPathPaint.dilate(true, false);
			RenderPathPaint.useLiveLayer(false);
			Context.raw.pdirty = 0;

			UIHeader.worktab.position = _space;
			Context.raw.tool = _tool;
			Context.raw.bakeType = _bakeType;
			MakeMaterial.parsePaintMaterial(false);

			let rts = render_path_render_targets;
			let texpaint_live = rts.get("texpaint_live");

			g2_begin(image, false);
			g2_draw_image(texpaint_live.image, 0, 0);
			g2_end();
		}
	}

	static parseNodePreviewMaterial = (node: zui_node_t, group: zui_node_canvas_t = null, parents: zui_node_t[] = null): { scon: shader_context_t, mcon: material_context_t } => {
		if (node.outputs.length == 0) return null;
		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let mcon_raw: material_context_t = { name: "mesh", bind_textures: [] };
		let con = MakeNodePreview.run(sdata, mcon_raw, node, group, parents);
		let compileError = false;
		let scon: shader_context_t;
		shader_context_create(con.data, (_scon: shader_context_t) => {
			if (_scon == null) compileError = true;
			scon = _scon;
		});
		if (compileError) return null;
		let mcon: material_context_t;
		material_context_create(mcon_raw, (_mcon: material_context_t) => { mcon = _mcon; });
		return { scon: scon, mcon: mcon };
	}

	static parseBrush = () => {
		ParserLogic.parse(Context.raw.brush.canvas);
	}

	static blendMode = (frag: NodeShaderRaw, blending: i32, cola: string, colb: string, opac: string): string => {
		if (blending == BlendType.BlendMix) {
			return `mix(${cola}, ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendDarken) {
			return `mix(${cola}, min(${cola}, ${colb}), ${opac})`;
		}
		else if (blending == BlendType.BlendMultiply) {
			return `mix(${cola}, ${cola} * ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendBurn) {
			return `mix(${cola}, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - ${cola}) / ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendLighten) {
			return `max(${cola}, ${colb} * ${opac})`;
		}
		else if (blending == BlendType.BlendScreen) {
			return `(vec3(1.0, 1.0, 1.0) - (vec3(1.0 - ${opac}, 1.0 - ${opac}, 1.0 - ${opac}) + ${opac} * (vec3(1.0, 1.0, 1.0) - ${colb})) * (vec3(1.0, 1.0, 1.0) - ${cola}))`;
		}
		else if (blending == BlendType.BlendDodge) {
			return `mix(${cola}, ${cola} / (vec3(1.0, 1.0, 1.0) - ${colb}), ${opac})`;
		}
		else if (blending == BlendType.BlendAdd) {
			return `mix(${cola}, ${cola} + ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendOverlay) {
			return `mix(${cola}, vec3(
				${cola}.r < 0.5 ? 2.0 * ${cola}.r * ${colb}.r : 1.0 - 2.0 * (1.0 - ${cola}.r) * (1.0 - ${colb}.r),
				${cola}.g < 0.5 ? 2.0 * ${cola}.g * ${colb}.g : 1.0 - 2.0 * (1.0 - ${cola}.g) * (1.0 - ${colb}.g),
				${cola}.b < 0.5 ? 2.0 * ${cola}.b * ${colb}.b : 1.0 - 2.0 * (1.0 - ${cola}.b) * (1.0 - ${colb}.b)
			), ${opac})`;
		}
		else if (blending == BlendType.BlendSoftLight) {
			return `((1.0 - ${opac}) * ${cola} + ${opac} * ((vec3(1.0, 1.0, 1.0) - ${cola}) * ${colb} * ${cola} + ${cola} * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - ${colb}) * (vec3(1.0, 1.0, 1.0) - ${cola}))))`;
		}
		else if (blending == BlendType.BlendLinearLight) {
			return `(${cola} + ${opac} * (vec3(2.0, 2.0, 2.0) * (${colb} - vec3(0.5, 0.5, 0.5))))`;
		}
		else if (blending == BlendType.BlendDifference) {
			return `mix(${cola}, abs(${cola} - ${colb}), ${opac})`;
		}
		else if (blending == BlendType.BlendSubtract) {
			return `mix(${cola}, ${cola} - ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendDivide) {
			return `vec3(1.0 - ${opac}, 1.0 - ${opac}, 1.0 - ${opac}) * ${cola} + vec3(${opac}, ${opac}, ${opac}) * ${cola} / ${colb}`;
		}
		else if (blending == BlendType.BlendHue) {
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${colb}).r, rgb_to_hsv(${cola}).g, rgb_to_hsv(${cola}).b)), ${opac})`;
		}
		else if (blending == BlendType.BlendSaturation) {
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${cola}).r, rgb_to_hsv(${colb}).g, rgb_to_hsv(${cola}).b)), ${opac})`;
		}
		else if (blending == BlendType.BlendColor) {
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${colb}).r, rgb_to_hsv(${colb}).g, rgb_to_hsv(${cola}).b)), ${opac})`;
		}
		else { // BlendValue
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${cola}).r, rgb_to_hsv(${cola}).g, rgb_to_hsv(${colb}).b)), ${opac})`;
		}
	}

	static blendModeMask = (frag: NodeShaderRaw, blending: i32, cola: string, colb: string, opac: string): string => {
		if (blending == BlendType.BlendMix) {
			return `mix(${cola}, ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendDarken) {
			return `mix(${cola}, min(${cola}, ${colb}), ${opac})`;
		}
		else if (blending == BlendType.BlendMultiply) {
			return `mix(${cola}, ${cola} * ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendBurn) {
			return `mix(${cola}, 1.0 - (1.0 - ${cola}) / ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendLighten) {
			return `max(${cola}, ${colb} * ${opac})`;
		}
		else if (blending == BlendType.BlendScreen) {
			return `(1.0 - ((1.0 - ${opac}) + ${opac} * (1.0 - ${colb})) * (1.0 - ${cola}))`;
		}
		else if (blending == BlendType.BlendDodge) {
			return `mix(${cola}, ${cola} / (1.0 - ${colb}), ${opac})`;
		}
		else if (blending == BlendType.BlendAdd) {
			return `mix(${cola}, ${cola} + ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendOverlay) {
			return `mix(${cola}, ${cola} < 0.5 ? 2.0 * ${cola} * ${colb} : 1.0 - 2.0 * (1.0 - ${cola}) * (1.0 - ${colb}), ${opac})`;
		}
		else if (blending == BlendType.BlendSoftLight) {
			return `((1.0 - ${opac}) * ${cola} + ${opac} * ((1.0 - ${cola}) * ${colb} * ${cola} + ${cola} * (1.0 - (1.0 - ${colb}) * (1.0 - ${cola}))))`;
		}
		else if (blending == BlendType.BlendLinearLight) {
			return `(${cola} + ${opac} * (2.0 * (${colb} - 0.5)))`;
		}
		else if (blending == BlendType.BlendDifference) {
			return `mix(${cola}, abs(${cola} - ${colb}), ${opac})`;
		}
		else if (blending == BlendType.BlendSubtract) {
			return `mix(${cola}, ${cola} - ${colb}, ${opac})`;
		}
		else if (blending == BlendType.BlendDivide) {
			return `(1.0 - ${opac}) * ${cola} + ${opac} * ${cola} / ${colb}`;
		}
		else { // BlendHue, BlendSaturation, BlendColor, BlendValue
			return `mix(${cola}, ${colb}, ${opac})`;
		}
	}

	static getDisplaceStrength = (): f32 => {
		let sc = Context.mainObject().base.transform.scale.x;
		return Config.raw.displace_strength * 0.02 * sc;
	}

	static voxelgiHalfExtents = (): string => {
		let ext = Context.raw.vxaoExt;
		return `const vec3 voxelgiHalfExtents = vec3(${ext}, ${ext}, ${ext});`;
	}

	static deleteContext = (c: shader_context_t) => {
		Base.notifyOnNextFrame(() => { // Ensure pipeline is no longer in use
			shader_context_delete(c);
		});
	}
}
