
class MakeMaterial {

	static defaultScon: ShaderContext = null;
	static defaultMcon: MaterialContext = null;
	static heightUsed = false;

	static parseMeshMaterial = () => {
		let m = Project.materialData;

		for (let c of m.shader.contexts) {
			if (c.raw.name == "mesh") {
				array_remove(m.shader.raw.contexts, c.raw);
				array_remove(m.shader.contexts, c);
				MakeMaterial.deleteContext(c);
				break;
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
		scon.overrideContext.addressing = "repeat";
		m.shader.raw.contexts.push(scon.raw);
		m.shader.contexts.push(scon);

		Context.raw.ddirty = 2;

		///if arm_voxels
		MakeMaterial.makeVoxel(m);
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

		let sdata: TMaterial = { name: "Material", canvas: null };
		let mcon2: TMaterialContext = { name: "paint", bind_textures: [] };
		let con = MakePaint.run(sdata, mcon2);

		let compileError = false;
		let scon2 = new ShaderContext(con.data, (scon: ShaderContext) => {
			if (scon == null) compileError = true;
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
