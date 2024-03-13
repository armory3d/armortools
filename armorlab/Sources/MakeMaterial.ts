
class MakeMaterial {

	static defaultScon: shader_context_t = null;
	static defaultMcon: material_context_t = null;
	static heightUsed = false;

	static parseMeshMaterial = () => {
		let m = project_materialData;

		for (let c of m._.shader.contexts) {
			if (c.name == "mesh") {
				array_remove(m._.shader.contexts, c);
				array_remove(m._.shader._.contexts, c);
				MakeMaterial.deleteContext(c);
				break;
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
		scon._.override_context.addressing = "repeat";
		m._.shader.contexts.push(scon);
		m._.shader._.contexts.push(scon);

		context_raw.ddirty = 2;

		///if arm_voxels
		MakeMaterial.makeVoxel(m);
		///end

		///if (krom_direct3d12 || krom_vulkan)
		render_path_raytrace_dirty = 1;
		///end
	}

	///if arm_voxels
	static makeVoxel = (m: material_data_t) => {
		let rebuild = true; // heightUsed;
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

	static parsePaintMaterial = () => {
		let m = project_materialData;
		let scon: shader_context_t = null;
		let mcon: material_context_t = null;
		for (let c of m._.shader.contexts) {
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

		let sdata: TMaterial = { name: "Material", canvas: null };
		let mcon2: material_context_t = { name: "paint", bind_textures: [] };
		let con = MakePaint.run(sdata, mcon2);

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
