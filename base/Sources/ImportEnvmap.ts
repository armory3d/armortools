
class ImportEnvmap {

	static pipeline: pipeline_t = null;
	static paramsLocation: kinc_const_loc_t;
	static params = vec4_create();
	static n = vec4_create();
	static radianceLocation: kinc_tex_unit_t;
	static radiance: image_t = null;
	static radianceCpu: image_t = null;
	static mips: image_t[] = null;
	static mipsCpu: image_t[] = null;

	static run = (path: string, image: image_t) => {

		// Init
		if (ImportEnvmap.pipeline == null) {
			ImportEnvmap.pipeline = pipeline_create();
			ImportEnvmap.pipeline.vertexShader = sys_get_shader("pass.vert");
			ImportEnvmap.pipeline.fragmentShader = sys_get_shader("prefilter_envmap.frag");
			let vs = vertex_struct_create();
			vertex_struct_add(vs, "pos", VertexData.F32_2X);
			ImportEnvmap.pipeline.inputLayout = [vs];
			ImportEnvmap.pipeline.colorAttachmentCount = 1;
			ImportEnvmap.pipeline.colorAttachments[0] = TextureFormat.RGBA128;
			pipeline_compile(ImportEnvmap.pipeline);
			ImportEnvmap.paramsLocation = pipeline_get_const_loc(ImportEnvmap.pipeline, "params");
			ImportEnvmap.radianceLocation = pipeline_get_tex_unit(ImportEnvmap.pipeline, "radiance");

			ImportEnvmap.radiance = image_create_render_target(1024, 512, TextureFormat.RGBA128);

			ImportEnvmap.mips = [];
			let w = 512;
			for (let i = 0; i < 10; ++i) {
				ImportEnvmap.mips.push(image_create_render_target(w, w > 1 ? Math.floor(w / 2) : 1, TextureFormat.RGBA128));
				w = Math.floor(w / 2);
			}

			if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		}

		// Down-scale to 1024x512
		g2_begin(ImportEnvmap.radiance.g2, false);
		ImportEnvmap.radiance.g2.pipeline = Base.pipeCopy128;
		g2_draw_scaled_image(image, 0, 0, 1024, 512);
		ImportEnvmap.radiance.g2.pipeline = null;
		g2_end(ImportEnvmap.radiance.g2);

		let radiancePixels = image_get_pixels(ImportEnvmap.radiance);
		if (ImportEnvmap.radianceCpu != null) {
			let _radianceCpu = ImportEnvmap.radianceCpu;
			Base.notifyOnNextFrame(() => {
				image_unload(_radianceCpu);
			});
		}
		ImportEnvmap.radianceCpu = image_from_bytes(radiancePixels, ImportEnvmap.radiance.width, ImportEnvmap.radiance.height, TextureFormat.RGBA128, Usage.DynamicUsage);

		// Radiance
		if (ImportEnvmap.mipsCpu != null) {
			for (let mip of ImportEnvmap.mipsCpu) {
				let _mip = mip;
				Base.notifyOnNextFrame(() => {
					///if (!krom_direct3d12) // TODO: crashes after 50+ imports
					image_unload(_mip);
					///end
				});
			}
		}
		ImportEnvmap.mipsCpu = [];
		for (let i = 0; i < ImportEnvmap.mips.length; ++i) {
			ImportEnvmap.getRadianceMip(ImportEnvmap.mips[i], i, ImportEnvmap.radiance);
			ImportEnvmap.mipsCpu.push(image_from_bytes(image_get_pixels(ImportEnvmap.mips[i]), ImportEnvmap.mips[i].width, ImportEnvmap.mips[i].height, TextureFormat.RGBA128, Usage.DynamicUsage));
		}
		image_set_mipmaps(ImportEnvmap.radianceCpu, ImportEnvmap.mipsCpu);

		// Irradiance
		scene_world._irradiance = ImportEnvmap.getSphericalHarmonics(radiancePixels, ImportEnvmap.radiance.width, ImportEnvmap.radiance.height);

		// World
		scene_world.strength = 1.0;
		scene_world.radiance_mipmaps = ImportEnvmap.mipsCpu.length - 2;
		scene_world._envmap = image;
		scene_world.envmap = path;
		scene_world._radiance = ImportEnvmap.radianceCpu;
		scene_world._radianceMipmaps = ImportEnvmap.mipsCpu;
		Context.raw.savedEnvmap = image;
		if (Context.raw.showEnvmapBlur) {
			scene_world._envmap = scene_world._radianceMipmaps[0];
		}
		Context.raw.ddirty = 2;
		Project.raw.envmap = path;
	}

	static getRadianceMip = (mip: image_t, level: i32, radiance: image_t) => {
		g4_begin(mip.g4);
		g4_set_vertex_buffer(ConstData.screenAlignedVB);
		g4_set_index_buffer(ConstData.screenAlignedIB);
		g4_set_pipeline(ImportEnvmap.pipeline);
		ImportEnvmap.params.x = 0.1 + level / 8;
		g4_set_float4(ImportEnvmap.paramsLocation, ImportEnvmap.params.x, ImportEnvmap.params.y, ImportEnvmap.params.z, ImportEnvmap.params.w);
		g4_set_tex(ImportEnvmap.radianceLocation, radiance);
		g4_draw();
		g4_end();
	}

	static reverseEquirect = (x: f32, y: f32): vec4_t => {
		let theta = x * Math.PI * 2 - Math.PI;
		let phi = y * Math.PI;
		// return n.set(Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)), Math.cos(phi));
		return vec4_set(ImportEnvmap.n, -Math.cos(phi), Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)));
	}

	// https://ndotl.wordpress.com/2015/03/07/pbr-cubemap-filtering
	// https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering
	static getSphericalHarmonics = (source: ArrayBuffer, sourceWidth: i32, sourceHeight: i32): Float32Array => {
		let sh = new Float32Array(9 * 3 + 1); // Align to mult of 4 - 27->28
		let accum = 0.0;
		let weight = 1.0;
		let weight1 = weight * 4 / 17;
		let weight2 = weight * 8 / 17;
		let weight3 = weight * 15 / 17;
		let weight4 = weight * 5 / 68;
		let weight5 = weight * 15 / 68;
		let view = new DataView(source);

		for (let x = 0; x < sourceWidth; ++x) {
			for (let y = 0; y < sourceHeight; ++y) {
				ImportEnvmap.n = ImportEnvmap.reverseEquirect(x / sourceWidth, y / sourceHeight);

				for (let i = 0; i < 3; ++i) {
					let value = view.getFloat32(((x + y * sourceWidth) * 16 + i * 4), true);
					value = Math.pow(value, 1.0 / 2.2);

					sh[0 + i] += value * weight1;
					sh[3 + i] += value * weight2 * ImportEnvmap.n.x;
					sh[6 + i] += value * weight2 * ImportEnvmap.n.y;
					sh[9 + i] += value * weight2 * ImportEnvmap.n.z;

					sh[12 + i] += value * weight3 * ImportEnvmap.n.x * ImportEnvmap.n.z;
					sh[15 + i] += value * weight3 * ImportEnvmap.n.z * ImportEnvmap.n.y;
					sh[18 + i] += value * weight3 * ImportEnvmap.n.y * ImportEnvmap.n.x;

					sh[21 + i] += value * weight4 * (3.0 * ImportEnvmap.n.z * ImportEnvmap.n.z - 1.0);
					sh[24 + i] += value * weight5 * (ImportEnvmap.n.x * ImportEnvmap.n.x - ImportEnvmap.n.y * ImportEnvmap.n.y);

					accum += weight;
				}
			}
		}

		for (let i = 0; i < sh.length; ++i) {
			sh[i] /= accum / 16;
		}

		return sh;
	}
}
