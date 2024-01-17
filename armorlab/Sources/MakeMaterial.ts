
class MakeMaterial {

	static defaultScon: ShaderContext = null;
	static defaultMcon: MaterialContext = null;
	static heightUsed = false;

	static parseMeshMaterial = () => {
		let m = Project.materialData;

		for (let c of m.shader.contexts) {
			if (c.raw.name == "mesh") {
				m.shader.raw.contexts.remove(c.raw);
				m.shader.contexts.remove(c);
				deleteContext(c);
				break;
			}
		}

		let con = MakeMesh.run(new NodeShaderData({ name: "Material", canvas: null }));
		let scon = new ShaderContext(con.data, (scon: ShaderContext) => {});
		scon.overrideContext = {};
		if (con.frag.sharedSamplers.length > 0) {
			let sampler = con.frag.sharedSamplers[0];
			scon.overrideContext.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!Context.raw.textureFilter) {
			scon.overrideContext.filter = "point";
		}
		scon.overrideContext.addressing = "repeat";
		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);

		Context.raw.ddirty = 2;

		///if arm_voxels
		makeVoxel(m);
		///end

		///if (krom_direct3d12 || krom_vulkan)
		RenderPathRaytrace.dirty = 1;
		///end
	}

	///if arm_voxels
	static makeVoxel = (m: MaterialData) => {
		let rebuild = true; // heightUsed;
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

	static parsePaintMaterial = () => {
		let m = Project.materialData;
		let scon: ShaderContext = null;
		let mcon: MaterialContext = null;
		for (let c of m.shader.contexts) {
			if (c.raw.name == "paint") {
				m.shader.raw.contexts.remove(c.raw);
				m.shader.contexts.remove(c);
				if (c != defaultScon) deleteContext(c);
				break;
			}
		}
		for (let c of m.contexts) {
			if (c.raw.name == "paint") {
				m.raw.contexts.remove(c.raw);
				m.contexts.remove(c);
				break;
			}
		}

		let sdata = new NodeShaderData({ name: "Material", canvas: null });
		let mcon: TMaterialContext = { name: "paint", bind_textures: [] };
		let con = MakePaint.run(sdata, mcon);

		let compileError = false;
		let scon = new ShaderContext(con.data, (scon: ShaderContext) => {
			if (scon == null) compileError = true;
		});
		if (compileError) return;
		scon.overrideContext = {};
		scon.overrideContext.addressing = "repeat";
		let mcon = new MaterialContext(mcon, (mcon: MaterialContext) => {});

		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);
		m.raw.contexts.push(mcon.raw);
		m.contexts.push(mcon);

		if (defaultScon == null) defaultScon = scon;
		if (defaultMcon == null) defaultMcon = mcon;
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
