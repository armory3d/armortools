
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
		let m = project_materials[0].data;

		for (let c of m._.shader._.contexts) {
			if (c.name == "mesh") {
				array_remove(m._.shader.contexts, c);
				array_remove(m._.shader._.contexts, c);
				MakeMaterial.deleteContext(c);
				break;
			}
		}

		if (MakeMesh.layerPassCount > 1) {
			let i = 0;
			while (i < m._.shader._.contexts.length) {
				let c = m._.shader._.contexts[i];
				for (let j = 1; j < MakeMesh.layerPassCount; ++j) {
					if (c.name == "mesh" + j) {
						array_remove(m._.shader.contexts, c);
						array_remove(m._.shader._.contexts, c);
						MakeMaterial.deleteContext(c);
						i--;
						break;
					}
				}
				i++;
			}

			i = 0;
			while (i < m.contexts.length) {
				let c = m.contexts[i];
				for (let j = 1; j < MakeMesh.layerPassCount; ++j) {
					if (c.name == "mesh" + j) {
						array_remove(m.contexts, c);
						array_remove(m._.contexts, c);
						i--;
						break;
					}
				}
				i++;
			}
		}

		let con = MakeMesh.run({ name: "Material", canvas: null });
		let scon: shader_context_t = shader_context_create(con.data);
		scon._.override_context = {};
		if (con.frag.sharedSamplers.length > 0) {
			let sampler = con.frag.sharedSamplers[0];
			scon._.override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!context_raw.textureFilter) {
			scon._.override_context.filter = "point";
		}
		m._.shader.contexts.push(scon);
		m._.shader._.contexts.push(scon);

		for (let i = 1; i < MakeMesh.layerPassCount; ++i) {
			let con = MakeMesh.run({ name: "Material", canvas: null }, i);
			let scon: shader_context_t = shader_context_create(con.data);
			scon._.override_context = {};
			if (con.frag.sharedSamplers.length > 0) {
				let sampler = con.frag.sharedSamplers[0];
				scon._.override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
			}
			if (!context_raw.textureFilter) {
				scon._.override_context.filter = "point";
			}
			m._.shader.contexts.push(scon);
			m._.shader._.contexts.push(scon);

			let mcon: material_context_t = material_context_create({ name: "mesh" + i, bind_textures: [] });
			m.contexts.push(mcon);
			m._.contexts.push(mcon);
		}

		context_raw.ddirty = 2;

		///if arm_voxels
		MakeMaterial.makeVoxel(m);
		///end
	}

	static parseParticleMaterial = () => {
		let m = context_raw.particleMaterial;
		let sc: shader_context_t = null;
		for (let c of m._.shader._.contexts) {
			if (c.name == "mesh") {
				sc = c;
				break;
			}
		}
		if (sc != null) {
			array_remove(m._.shader.contexts, sc);
			array_remove(m._.shader._.contexts, sc);
		}
		let con = MakeParticle.run({ name: "MaterialParticle", canvas: null });
		if (sc != null) MakeMaterial.deleteContext(sc);
		sc = shader_context_create(con.data);
		m._.shader.contexts.push(sc);
		m._.shader._.contexts.push(sc);
	}

	static parseMeshPreviewMaterial = () => {
		if (!MakeMaterial.getMOut()) return;

		let m = project_materials[0].data;
		let scon: shader_context_t = null;
		for (let c of m._.shader._.contexts) {
			if (c.name == "mesh") {
				scon = c;
				break;
			}
		}
		array_remove(m._.shader.contexts, scon);
		array_remove(m._.shader._.contexts, scon);

		let mcon: material_context_t = { name: "mesh", bind_textures: [] };

		let sd: TMaterial = { name: "Material", canvas: null };
		let con = MakeMeshPreview.run(sd, mcon);

		for (let i = 0; i < m.contexts.length; ++i) {
			if (m.contexts[i].name == "mesh") {
				m.contexts[i] = material_context_create(mcon);
				break;
			}
		}

		if (scon != null) MakeMaterial.deleteContext(scon);

		let compileError = false;
		let _scon: shader_context_t = shader_context_create(con.data);
		if (_scon == null) compileError = true;
		scon = _scon;
		if (compileError) return;

		m._.shader.contexts.push(scon);
		m._.shader._.contexts.push(scon);
	}

	///if arm_voxels
	static makeVoxel = (m: material_data_t) => {
		let rebuild = MakeMaterial.heightUsed;
		if (config_raw.rp_gi != false && rebuild) {
			let scon: shader_context_t = null;
			for (let c of m._.shader._.contexts) {
				if (c.name == "voxel") {
					scon = c;
					break;
				}
			}
			if (scon != null) make_voxel_run(scon);
		}
	}
	///end

	static parsePaintMaterial = (bakePreviews = true) => {
		if (!MakeMaterial.getMOut()) return;

		if (bakePreviews) {
			let current = _g2_current;
			if (current != null) g2_end();
			MakeMaterial.bakeNodePreviews();
			if (current != null) g2_begin(current);
		}

		let m = project_materials[0].data;
		let scon: shader_context_t = null;
		let mcon: material_context_t = null;
		for (let c of m._.shader._.contexts) {
			if (c.name == "paint") {
				array_remove(m._.shader.contexts, c);
				array_remove(m._.shader._.contexts, c);
				if (c != MakeMaterial.defaultScon) MakeMaterial.deleteContext(c);
				break;
			}
		}
		for (let c of m.contexts) {
			if (c.name == "paint") {
				array_remove(m.contexts, c);
				array_remove(m._.contexts, c);
				break;
			}
		}

		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let mcon2: material_context_t = { name: "paint", bind_textures: [] };
		let con = MakeSculpt.run(sdata, mcon2);

		let compileError = false;
		let scon2: shader_context_t;
		let _scon: shader_context_t = shader_context_create(con.data);
		if (_scon == null) compileError = true;
		scon2 = _scon;

		if (compileError) return;
		scon2._.override_context = {};
		scon2._.override_context.addressing = "repeat";
		let mcon3: material_context_t = material_context_create(mcon2);

		m._.shader.contexts.push(scon2);
		m._.shader._.contexts.push(scon2);
		m.contexts.push(mcon3);
		m._.contexts.push(mcon3);

		if (MakeMaterial.defaultScon == null) MakeMaterial.defaultScon = scon2;
		if (MakeMaterial.defaultMcon == null) MakeMaterial.defaultMcon = mcon3;
	}

	static bakeNodePreviews = () => {
		context_raw.nodePreviewsUsed = [];
		if (context_raw.nodePreviews == null) context_raw.nodePreviews = new Map();
		MakeMaterial.traverseNodes(UINodes.getCanvasMaterial().nodes, null, []);
		for (let key of context_raw.nodePreviews.keys()) {
			if (context_raw.nodePreviewsUsed.indexOf(key) == -1) {
				let image = context_raw.nodePreviews.get(key);
				base_notifyOnNextFrame(function() { image_unload(image); });
				context_raw.nodePreviews.delete(key);
			}
		}
	}

	static traverseNodes = (nodes: zui_node_t[], group: zui_node_canvas_t, parents: zui_node_t[]) => {
		for (let node of nodes) {
			MakeMaterial.bakeNodePreview(node, group, parents);
			if (node.type == "GROUP") {
				for (let g of project_materialGroups) {
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
			let image = context_raw.nodePreviews.get(id);
			context_raw.nodePreviewsUsed.push(id);
			let resX = Math.floor(config_getTextureResX() / 4);
			let resY = Math.floor(config_getTextureResY() / 4);
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY);
				context_raw.nodePreviews.set(id, image);
			}

			ParserMaterial.blur_passthrough = true;
			UtilRender.makeNodePreview(UINodes.getCanvasMaterial(), node, image, group, parents);
			ParserMaterial.blur_passthrough = false;
		}
		else if (node.type == "DIRECT_WARP") {
			let id = ParserMaterial.node_name(node, parents);
			let image = context_raw.nodePreviews.get(id);
			context_raw.nodePreviewsUsed.push(id);
			let resX = Math.floor(config_getTextureResX());
			let resY = Math.floor(config_getTextureResY());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY);
				context_raw.nodePreviews.set(id, image);
			}

			ParserMaterial.warp_passthrough = true;
			UtilRender.makeNodePreview(UINodes.getCanvasMaterial(), node, image, group, parents);
			ParserMaterial.warp_passthrough = false;
		}
	}

	static parseNodePreviewMaterial = (node: zui_node_t, group: zui_node_canvas_t = null, parents: zui_node_t[] = null): { scon: shader_context_t, mcon: material_context_t } => {
		if (node.outputs.length == 0) return null;
		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let mcon_raw: material_context_t = { name: "mesh", bind_textures: [] };
		let con = MakeNodePreview.run(sdata, mcon_raw, node, group, parents);
		let compileError = false;
		let scon: shader_context_t;
		let _scon: shader_context_t = shader_context_create(con.data);
		if (_scon == null) compileError = true;
		scon = _scon;

		if (compileError) return null;
		let mcon: material_context_t = material_context_create(mcon_raw);
		return { scon: scon, mcon: mcon };
	}

	static parseBrush = () => {
		ParserLogic.parse(context_raw.brush.canvas);
	}

	static getDisplaceStrength = (): f32 => {
		let sc = context_mainObject().base.transform.scale.x;
		return config_raw.displace_strength * 0.02 * sc;
	}

	static voxelgiHalfExtents = (): string => {
		let ext = context_raw.vxaoExt;
		return `const vec3 voxelgiHalfExtents = vec3(${ext}, ${ext}, ${ext});`;
	}

	static deleteContext = (c: shader_context_t) => {
		base_notifyOnNextFrame(() => { // Ensure pipeline is no longer in use
			shader_context_delete(c);
		});
	}
}
