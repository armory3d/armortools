
class MakeMaterial {

	static defaultScon: ShaderContext = null;
	static defaultMcon: MaterialContext = null;

	static heightUsed = false;
	static emisUsed = false;
	static subsUsed = false;

	static getMOut = (): bool => {
		for (let n of UINodes.getCanvasMaterial().nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
		return false;
	}

	static parseMeshMaterial = () => {
		let m = Project.materials[0].data;

		for (let c of m.shader.contexts) {
			if (c.raw.name == "mesh") {
				array_remove(m.shader.raw.contexts, c.raw);
				array_remove(m.shader.contexts, c);
				MakeMaterial.deleteContext(c);
				break;
			}
		}

		if (MakeMesh.layerPassCount > 1) {
			let i = 0;
			while (i < m.shader.contexts.length) {
				let c = m.shader.contexts[i];
				for (let j = 1; j < MakeMesh.layerPassCount; ++j) {
					if (c.raw.name == "mesh" + j) {
						array_remove(m.shader.raw.contexts, c.raw);
						array_remove(m.shader.contexts, c);
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
					if (c.raw.name == "mesh" + j) {
						array_remove(m.raw.contexts, c.raw);
						array_remove(m.contexts, c);
						i--;
						break;
					}
				}
				i++;
			}
		}

		let con = MakeMesh.run({ name: "Material", canvas: null });
		let scon = new ShaderContext(con.data, (scon: ShaderContext) => {});
		scon.overrideContext = {};
		if (con.frag.sharedSamplers.length > 0) {
			let sampler = con.frag.sharedSamplers[0];
			scon.overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!Context.raw.textureFilter) {
			scon.overrideContext.filter = "point";
		}
		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);

		for (let i = 1; i < MakeMesh.layerPassCount; ++i) {
			let con = MakeMesh.run({ name: "Material", canvas: null }, i);
			let scon = new ShaderContext(con.data, (scon: ShaderContext) => {});
			scon.overrideContext = {};
			if (con.frag.sharedSamplers.length > 0) {
				let sampler = con.frag.sharedSamplers[0];
				scon.overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
			}
			if (!Context.raw.textureFilter) {
				scon.overrideContext.filter = "point";
			}
			m.shader.raw.contexts.push(scon.raw);
			m.shader.contexts.push(scon);

			let mcon = new MaterialContext({ name: "mesh" + i, bind_textures: [] }, (self: MaterialContext) => {});
			m.raw.contexts.push(mcon.raw);
			m.contexts.push(mcon);
		}

		Context.raw.ddirty = 2;

		///if arm_voxels
		MakeMaterial.makeVoxel(m);
		///end
	}

	static parseParticleMaterial = () => {
		let m = Context.raw.particleMaterial;
		let sc: ShaderContext = null;
		for (let c of m.shader.contexts) {
			if (c.raw.name == "mesh") {
				sc = c;
				break;
			}
		}
		if (sc != null) {
			array_remove(m.shader.raw.contexts, sc.raw);
			array_remove(m.shader.contexts, sc);
		}
		let con = MakeParticle.run({ name: "MaterialParticle", canvas: null });
		if (sc != null) MakeMaterial.deleteContext(sc);
		sc = new ShaderContext(con.data, (sc: ShaderContext) => {});
		m.shader.raw.contexts.push(sc.raw);
		m.shader.contexts.push(sc);
	}

	static parseMeshPreviewMaterial = () => {
		if (!MakeMaterial.getMOut()) return;

		let m = Project.materials[0].data;
		let scon: ShaderContext = null;
		for (let c of m.shader.contexts) {
			if (c.raw.name == "mesh") {
				scon = c;
				break;
			}
		}
		array_remove(m.shader.raw.contexts, scon.raw);
		array_remove(m.shader.contexts, scon);

		let mcon: TMaterialContext = { name: "mesh", bind_textures: [] };

		let sd: TMaterial = { name: "Material", canvas: null };
		let con = MakeMeshPreview.run(sd, mcon);

		for (let i = 0; i < m.contexts.length; ++i) {
			if (m.contexts[i].raw.name == "mesh") {
				m.contexts[i] = new MaterialContext(mcon, (self: MaterialContext) => {});
				break;
			}
		}

		if (scon != null) MakeMaterial.deleteContext(scon);

		let compileError = false;
		scon = new ShaderContext(con.data, (scon: ShaderContext) => {
			if (scon == null) compileError = true;
		});
		if (compileError) return;

		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);
	}

	///if arm_voxels
	static makeVoxel = (m: MaterialData) => {
		let rebuild = MakeMaterial.heightUsed;
		if (Config.raw.rp_gi != false && rebuild) {
			let scon: ShaderContext = null;
			for (let c of m.shader.contexts) {
				if (c.raw.name == "voxel") {
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
			if (current != null) current.end();
			MakeMaterial.bakeNodePreviews();
			if (current != null) current.begin(false);
		}

		let m = Project.materials[0].data;
		let scon: ShaderContext = null;
		let mcon: MaterialContext = null;
		for (let c of m.shader.contexts) {
			if (c.raw.name == "paint") {
				array_remove(m.shader.raw.contexts, c.raw);
				array_remove(m.shader.contexts, c);
				if (c != MakeMaterial.defaultScon) MakeMaterial.deleteContext(c);
				break;
			}
		}
		for (let c of m.contexts) {
			if (c.raw.name == "paint") {
				array_remove(m.raw.contexts, c.raw);
				array_remove(m.contexts, c);
				break;
			}
		}

		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let mcon2: TMaterialContext = { name: "paint", bind_textures: [] };
		let con = MakeSculpt.run(sdata, mcon2);

		let compileError = false;
		let scon2 = new ShaderContext(con.data, (scon: ShaderContext) => {
			if (scon2 == null) compileError = true;
		});
		if (compileError) return;
		scon2.overrideContext = {};
		scon2.overrideContext.addressing = "repeat";
		let mcon3 = new MaterialContext(mcon2, (mcon: MaterialContext) => {});

		m.shader.raw.contexts.push(scon2.raw);
		m.shader.contexts.push(scon2);
		m.raw.contexts.push(mcon3.raw);
		m.contexts.push(mcon3);

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
				Base.notifyOnNextFrame(image.unload);
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
				if (image != null) image.unload();
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
				if (image != null) image.unload();
				image = Image.createRenderTarget(resX, resY);
				Context.raw.nodePreviews.set(id, image);
			}

			ParserMaterial.warp_passthrough = true;
			UtilRender.makeNodePreview(UINodes.getCanvasMaterial(), node, image, group, parents);
			ParserMaterial.warp_passthrough = false;
		}
	}

	static parseNodePreviewMaterial = (node: TNode, group: TNodeCanvas = null, parents: TNode[] = null): { scon: ShaderContext, mcon: MaterialContext } => {
		if (node.outputs.length == 0) return null;
		let sdata: TMaterial = { name: "Material", canvas: UINodes.getCanvasMaterial() };
		let mcon_raw: TMaterialContext = { name: "mesh", bind_textures: [] };
		let con = MakeNodePreview.run(sdata, mcon_raw, node, group, parents);
		let compileError = false;
		let scon = new ShaderContext(con.data, (scon: ShaderContext) => {
			if (scon == null) compileError = true;
		});
		if (compileError) return null;
		let mcon = new MaterialContext(mcon_raw, (mcon: MaterialContext) => {});
		return { scon: scon, mcon: mcon };
	}

	static parseBrush = () => {
		ParserLogic.parse(Context.raw.brush.canvas);
	}

	static getDisplaceStrength = (): f32 => {
		let sc = Context.mainObject().transform.scale.x;
		return Config.raw.displace_strength * 0.02 * sc;
	}

	static voxelgiHalfExtents = (): string => {
		let ext = Context.raw.vxaoExt;
		return `const vec3 voxelgiHalfExtents = vec3(${ext}, ${ext}, ${ext});`;
	}

	static deleteContext = (c: ShaderContext) => {
		Base.notifyOnNextFrame(() => { // Ensure pipeline is no longer in use
			c.delete();
		});
	}
}
