
class ImportEnvmap {

	static pipeline: pipeline_t = null;
	static params_loc: kinc_const_loc_t;
	static params: vec4_t = vec4_create();
	static n: vec4_t = vec4_create();
	static radiance_loc: kinc_tex_unit_t;
	static radiance: image_t = null;
	static radiance_cpu: image_t = null;
	static mips: image_t[] = null;
	static mips_cpu: image_t[] = null;

	static run = (path: string, image: image_t) => {

		// Init
		if (ImportEnvmap.pipeline == null) {
			ImportEnvmap.pipeline = g4_pipeline_create();
			ImportEnvmap.pipeline.vertex_shader = sys_get_shader("pass.vert");
			ImportEnvmap.pipeline.fragment_shader = sys_get_shader("prefilter_envmap.frag");
			let vs: vertex_struct_t = g4_vertex_struct_create();
			g4_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
			ImportEnvmap.pipeline.input_layout = [vs];
			ImportEnvmap.pipeline.color_attachment_count = 1;
			ImportEnvmap.pipeline.color_attachments[0] = tex_format_t.RGBA128;
			g4_pipeline_compile(ImportEnvmap.pipeline);
			ImportEnvmap.params_loc = g4_pipeline_get_const_loc(ImportEnvmap.pipeline, "params");
			ImportEnvmap.radiance_loc = g4_pipeline_get_tex_unit(ImportEnvmap.pipeline, "radiance");

			ImportEnvmap.radiance = image_create_render_target(1024, 512, tex_format_t.RGBA128);

			ImportEnvmap.mips = [];
			let w: i32 = 512;
			for (let i: i32 = 0; i < 10; ++i) {
				ImportEnvmap.mips.push(image_create_render_target(w, w > 1 ? Math.floor(w / 2) : 1, tex_format_t.RGBA128));
				w = Math.floor(w / 2);
			}

			if (const_data_screen_aligned_vb == null) const_data_create_screen_aligned_data();
		}

		// Down-scale to 1024x512
		g2_begin(ImportEnvmap.radiance);
		g2_set_pipeline(base_pipe_copy128);
		g2_draw_scaled_image(image, 0, 0, 1024, 512);
		g2_set_pipeline(null);
		g2_end();

		let radiance_pixels: buffer_t = image_get_pixels(ImportEnvmap.radiance);
		if (ImportEnvmap.radiance_cpu != null) {
			let _radiance_cpu: image_t = ImportEnvmap.radiance_cpu;
			base_notify_on_next_frame(() => {
				image_unload(_radiance_cpu);
			});
		}
		ImportEnvmap.radiance_cpu = image_from_bytes(radiance_pixels, ImportEnvmap.radiance.width, ImportEnvmap.radiance.height, tex_format_t.RGBA128);

		// Radiance
		if (ImportEnvmap.mips_cpu != null) {
			for (let mip of ImportEnvmap.mips_cpu) {
				let _mip: image_t = mip;
				base_notify_on_next_frame(() => {
					///if (!krom_direct3d12) // TODO: crashes after 50+ imports
					image_unload(_mip);
					///end
				});
			}
		}
		ImportEnvmap.mips_cpu = [];
		for (let i: i32 = 0; i < ImportEnvmap.mips.length; ++i) {
			ImportEnvmap.get_radiance_mip(ImportEnvmap.mips[i], i, ImportEnvmap.radiance);
			ImportEnvmap.mips_cpu.push(image_from_bytes(image_get_pixels(ImportEnvmap.mips[i]), ImportEnvmap.mips[i].width, ImportEnvmap.mips[i].height, tex_format_t.RGBA128));
		}
		image_set_mipmaps(ImportEnvmap.radiance_cpu, ImportEnvmap.mips_cpu);

		// Irradiance
		scene_world._.irradiance = ImportEnvmap.get_spherical_harmonics(radiance_pixels, ImportEnvmap.radiance.width, ImportEnvmap.radiance.height);

		// World
		scene_world.strength = 1.0;
		scene_world.radiance_mipmaps = ImportEnvmap.mips_cpu.length - 2;
		scene_world._.envmap = image;
		scene_world.envmap = path;
		scene_world._.radiance = ImportEnvmap.radiance_cpu;
		scene_world._.radiance_mipmaps = ImportEnvmap.mips_cpu;
		Context.raw.saved_envmap = image;
		if (Context.raw.show_envmap_blur) {
			scene_world._.envmap = scene_world._.radiance_mipmaps[0];
		}
		Context.raw.ddirty = 2;
		Project.raw.envmap = path;
	}

	static get_radiance_mip = (mip: image_t, level: i32, radiance: image_t) => {
		g4_begin(mip);
		g4_set_vertex_buffer(const_data_screen_aligned_vb);
		g4_set_index_buffer(const_data_screen_aligned_ib);
		g4_set_pipeline(ImportEnvmap.pipeline);
		ImportEnvmap.params.x = 0.1 + level / 8;
		g4_set_float4(ImportEnvmap.params_loc, ImportEnvmap.params.x, ImportEnvmap.params.y, ImportEnvmap.params.z, ImportEnvmap.params.w);
		g4_set_tex(ImportEnvmap.radiance_loc, radiance);
		g4_draw();
		g4_end();
	}

	static reverse_equirect = (x: f32, y: f32): vec4_t => {
		let theta: f32 = x * Math.PI * 2 - Math.PI;
		let phi: f32 = y * Math.PI;
		// return n.set(Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)), Math.cos(phi));
		return vec4_set(ImportEnvmap.n, -Math.cos(phi), Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)));
	}

	// https://ndotl.wordpress.com/2015/03/07/pbr-cubemap-filtering
	// https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering
	static get_spherical_harmonics = (source: ArrayBuffer, sourceWidth: i32, sourceHeight: i32): Float32Array => {
		let sh: Float32Array = new Float32Array(9 * 3 + 1); // Align to mult of 4 - 27->28
		let accum: f32 = 0.0;
		let weight: f32 = 1.0;
		let weight1: f32 = weight * 4 / 17;
		let weight2: f32 = weight * 8 / 17;
		let weight3: f32 = weight * 15 / 17;
		let weight4: f32 = weight * 5 / 68;
		let weight5: f32 = weight * 15 / 68;
		let view: DataView = new DataView(source);

		for (let x: i32 = 0; x < sourceWidth; ++x) {
			for (let y: i32 = 0; y < sourceHeight; ++y) {
				ImportEnvmap.n = ImportEnvmap.reverse_equirect(x / sourceWidth, y / sourceHeight);

				for (let i: i32 = 0; i < 3; ++i) {
					let value: f32 = view.getFloat32(((x + y * sourceWidth) * 16 + i * 4), true);
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

		for (let i: i32 = 0; i < sh.length; ++i) {
			sh[i] /= accum / 16;
		}

		return sh;
	}
}
