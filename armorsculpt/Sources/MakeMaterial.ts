
class MakeMaterial {

	static defaultScon: TShaderContext = null;
	static defaultMcon: TMaterialContext = null;

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
			while (i < m.contexts.length) {
				let c = m.contexts[i];
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
		let scon: TShaderContext;
		ShaderContext.create(con.data, (_scon: TShaderContext) => { scon = _scon; });
		scon._overrideContext = {};
		if (con.frag.sharedSamplers.length > 0) {
			let sampler = con.frag.sharedSamplers[0];
			scon._overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!Context.raw.textureFilter) {
			scon._overrideContext.filter = "point";
		}
		m._shader.contexts.push(scon);
		m._shader._contexts.push(scon);

		for (let i = 1; i < MakeMesh.layerPassCount; ++i) {
			let con = MakeMesh.run({ name: "Material", canvas: null }, i);
			let scon: TShaderContext;
			ShaderContext.create(con.data, (_scon: TShaderContext) => { scon = _scon; });
			scon._overrideContext = {};
			if (con.frag.sharedSamplers.length > 0) {
				let sampler = con.frag.sharedSamplers[0];
				scon._overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
			}
			if (!Context.raw.textureFilter) {
				scon._overrideContext.filter = "point";
			}
			m._shader.contexts.push(scon);
			m._shader._contexts.push(scon);

			let mcon: TMaterialContext;
			MaterialContext.create({ name: "mesh" + i, bind_textures: [] }, (self: TMaterialContext) => { mcon = self; });
			m.contexts.push(mcon);
			m._contexts.push(mcon);
		}

		Context.raw.ddirty = 2;

		///if arm_voxels
		MakeMaterial.makeVoxel(m);
		///end
	}

	static parseParticleMaterial = () => {
		let m = Context.raw.particleMaterial;
		let sc: TShaderContext = null;
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
		ShaderContext.create(con.data, (_sc: TShaderContext) => { sc = _sc; });
		m._shader.contexts.push(sc);
		m._shader._contexts.push(sc);
	}

	static parseMeshPreviewMaterial = () => {
		if (!MakeMaterial.getMOut()) return;

		let m = Project.materials[0].data;
		let scon: TShaderContext = null;
		for (let c of m._shader._contexts) {
			if (c.name == "mesh") {
				scon = c;
				break;
			}
		}
		array_remove(m._shader.contexts, scon);
		array_remove(m._shader._contexts, scon);

		let mcon: TMaterialContext = { name: "mesh", bind_textures: [] };

		let sd: TMaterial = { name: "Material", canvas: null };
		let con = MakeMeshPreview.run(sd, mcon);

		for (let i = 0; i < m.contexts.length; ++i) {
			if (m.contexts[i].name == "mesh") {
				MaterialContext.create(mcon, (self: TMaterialContext) => { m.contexts[i] = self; });
				break;
			}
		}

		if (scon != null) MakeMaterial.deleteContext(scon);

		let compileError = false;
		ShaderContext.create(con.data, (_scon: TShaderContext) => {
			if (_scon == null) compileError = true;
			scon = _scon;
		});
		if (compileError) return;

		m._shader.contexts.push(scon);
		m._shader._contexts.push(scon);
	}

	///if arm_voxels
	static makeVoxel = (m: TMaterialData) => {
		let rebuild = MakeMaterial.heightUsed;
		if (Config.raw.rp_gi != false && rebuild) {
			let scon: TShaderContext = null;
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
			let current = Graphics2.current;
			if (current != null) Graphics2.end(current);
			MakeMaterial.bakeNodePreviews();
			if (current != null) Graphics2.begin(current, false);
		}

		let m = Project.materials[0].data;
		let scon: TShaderContext = null;
		let mcon: TMaterialContext = null;
		for (let c of m._shader._contexts) {
			if (c.name == "paint") {
				array_remove(m._shader.contexts, c);
				array_remove(m._shader._contexts, c);
				if (c != MakeMaterial.defaultScon) MakeMaterial.deleteContext(c);
				break;
			}
		}
		for (let c of m.contexts) {
			if (c.name == "paint") {
				array_remove(m.contexts, c);
				array_remove(m._contexts, c);
				break;
			}
		}

		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let mcon2: TMaterialContext = { name: "paint", bind_textures: [] };
		let con = MakeSculpt.run(sdata, mcon2);

		let compileError = false;
		let scon2: TShaderContext;
		ShaderContext.create(con.data, (_scon: TShaderContext) => {
			if (_scon == null) compileError = true;
			scon2 = _scon;
		});
		if (compileError) return;
		scon2._overrideContext = {};
		scon2._overrideContext.addressing = "repeat";
		let mcon3: TMaterialContext;
		MaterialContext.create(mcon2, (_mcon: TMaterialContext) => { mcon3 = _mcon; });

		m._shader.contexts.push(scon2);
		m._shader._contexts.push(scon2);
		m.contexts.push(mcon3);
		m._contexts.push(mcon3);

		if (MakeMaterial.defaultScon == null) MakeMaterial.defaultScon = scon2;
		if (MakeMaterial.defaultMcon == null) MakeMaterial.defaultMcon = mcon3;
	}

	static bakeNodePreviews = () => {
		Context.raw.nodePreviewsUsed = [];
		if (Context.raw.nodePreviews == null) Context.raw.nodePreviews = new Map();
		MakeMaterial.traverseNodes(UINodes.getCanvasMaterial().nodes, null, []);
		for (let key of Context.raw.nodePreviews.keys()) {
			if (Context.raw.nodePreviewsUsed.indexOf(key) == -1) {
				let image = Context.raw.nodePreviews.get(key);
				Base.notifyOnNextFrame(function() { Image.unload(image); });
				Context.raw.nodePreviews.delete(key);
			}
		}
	}

	static traverseNodes = (nodes: TNode[], group: TNodeCanvas, parents: TNode[]) => {
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

	static bakeNodePreview = (node: TNode, group: TNodeCanvas, parents: TNode[]) => {
		if (node.type == "BLUR") {
			let id = ParserMaterial.node_name(node, parents);
			let image = Context.raw.nodePreviews.get(id);
			Context.raw.nodePreviewsUsed.push(id);
			let resX = Math.floor(Config.getTextureResX() / 4);
			let resY = Math.floor(Config.getTextureResY() / 4);
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) Image.unload(image);
				image = Image.createRenderTarget(resX, resY);
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
				if (image != null) Image.unload(image);
				image = Image.createRenderTarget(resX, resY);
				Context.raw.nodePreviews.set(id, image);
			}

			ParserMaterial.warp_passthrough = true;
			UtilRender.makeNodePreview(UINodes.getCanvasMaterial(), node, image, group, parents);
			ParserMaterial.warp_passthrough = false;
		}
	}

	static parseNodePreviewMaterial = (node: TNode, group: TNodeCanvas = null, parents: TNode[] = null): { scon: TShaderContext, mcon: TMaterialContext } => {
		if (node.outputs.length == 0) return null;
		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let mcon_raw: TMaterialContext = { name: "mesh", bind_textures: [] };
		let con = MakeNodePreview.run(sdata, mcon_raw, node, group, parents);
		let compileError = false;
		let scon: TShaderContext;
		ShaderContext.create(con.data, (_scon: TShaderContext) => {
			if (_scon == null) compileError = true;
			scon = _scon;
		});
		if (compileError) return null;
		let mcon: TMaterialContext;
		MaterialContext.create(mcon_raw, (_mcon: TMaterialContext) => { mcon = _mcon; });
		return { scon: scon, mcon: mcon };
	}

	static parseBrush = () => {
		ParserLogic.parse(Context.raw.brush.canvas);
	}

	static getDisplaceStrength = (): f32 => {
		let sc = Context.mainObject().base.transform.scale.x;
		return Config.raw.displace_strength * 0.02 * sc;
	}

	static voxelgiHalfExtents = (): string => {
		let ext = Context.raw.vxaoExt;
		return `const vec3 voxelgiHalfExtents = vec3(${ext}, ${ext}, ${ext});`;
	}

	static deleteContext = (c: TShaderContext) => {
		Base.notifyOnNextFrame(() => { // Ensure pipeline is no longer in use
			ShaderContext.delete(c);
		});
	}
}
