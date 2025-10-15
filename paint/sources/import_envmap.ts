
let import_envmap_pipeline: gpu_pipeline_t = null;
let import_envmap_params_loc: i32;
let import_envmap_params: vec4_t = vec4_create();
let import_envmap_n: vec4_t      = vec4_create();
let import_envmap_radiance_loc: i32;
let import_envmap_radiance: gpu_texture_t = null;
let import_envmap_mips: gpu_texture_t[]   = null;

function import_envmap_run(path: string, image: gpu_texture_t) {
	// Init
	if (import_envmap_pipeline == null) {
		import_envmap_pipeline                 = gpu_create_pipeline();
		import_envmap_pipeline.vertex_shader   = sys_get_shader("prefilter_envmap.vert");
		import_envmap_pipeline.fragment_shader = sys_get_shader("prefilter_envmap.frag");
		let vs: gpu_vertex_structure_t         = {};
		gpu_vertex_struct_add(vs, "pos", vertex_data_t.F32_2X);
		import_envmap_pipeline.input_layout                      = vs;
		import_envmap_pipeline.color_attachment_count            = 1;
		ARRAY_ACCESS(import_envmap_pipeline.color_attachment, 0) = tex_format_t.RGBA128;

		gpu_pipeline_compile(import_envmap_pipeline);
		import_envmap_params_loc   = 0;
		import_envmap_radiance_loc = 0;

		import_envmap_radiance = gpu_create_render_target(1024, 512, tex_format_t.RGBA128);

		import_envmap_mips = [];
		let w: i32         = 512;
		for (let i: i32 = 0; i < 5; ++i) {
			array_push(import_envmap_mips, gpu_create_render_target(w, w > 1 ? math_floor(w / 2) : 1, tex_format_t.RGBA128));
			w = math_floor(w / 2);
		}
	}

	// Down-scale to 1024x512
	draw_begin(import_envmap_radiance);
	draw_set_pipeline(pipes_copy128);
	draw_scaled_image(image, 0, 0, 1024, 512);
	draw_set_pipeline(null);
	draw_end();

	// Radiance
	for (let i: i32 = 0; i < import_envmap_mips.length; ++i) {
		import_envmap_get_radiance_mip(import_envmap_mips[i], i, import_envmap_radiance);
	}

	// Irradiance
	let radiance_pixels: buffer_t = gpu_get_texture_pixels(import_envmap_radiance);
	scene_world._.irradiance      = import_envmap_get_spherical_harmonics(radiance_pixels, import_envmap_radiance.width, import_envmap_radiance.height);

	// World
	scene_world.strength           = 1.0;
	scene_world.radiance_mipmaps   = import_envmap_mips.length - 2;
	scene_world._.envmap           = image;
	scene_world.envmap             = path;
	scene_world._.radiance         = import_envmap_radiance;
	scene_world._.radiance_mipmaps = import_envmap_mips;
	context_raw.saved_envmap       = image;
	context_raw.show_envmap        = true;
	if (context_raw.show_envmap_blur) {
		scene_world._.envmap = scene_world._.radiance_mipmaps[0];
	}
	context_raw.ddirty = 2;
	project_raw.envmap = path;
}

function import_envmap_get_radiance_mip(mip: gpu_texture_t, level: i32, radiance: gpu_texture_t) {
	_gpu_begin(mip);
	gpu_set_vertex_buffer(const_data_screen_aligned_vb);
	gpu_set_index_buffer(const_data_screen_aligned_ib);
	gpu_set_pipeline(import_envmap_pipeline);
	import_envmap_params.x = 0.1 + level / 8;
	/// if (arm_macos || arm_ios)
	import_envmap_params.y = 1024 * 2; // Prevent gpu hang
	/// else
	import_envmap_params.y = 1024 * 16;
	/// end
	gpu_set_float4(import_envmap_params_loc, import_envmap_params.x, import_envmap_params.y, import_envmap_params.z, import_envmap_params.w);
	gpu_set_texture(import_envmap_radiance_loc, radiance);
	gpu_draw();
	gpu_end();
}

function import_envmap_reverse_equirect(x: f32, y: f32): vec4_t {
	let theta: f32 = x * math_pi() * 2 - math_pi();
	let phi: f32   = y * math_pi();
	// return n.set(math_sin(phi) * math_cos(theta), -(math_sin(phi) * math_sin(theta)), math_cos(phi));
	import_envmap_n = vec4_create(-math_cos(phi), math_sin(phi) * math_cos(theta), -(math_sin(phi) * math_sin(theta)));
	return import_envmap_n;
}

// https://ndotl.wordpress.com/2015/03/07/pbr-cubemap-filtering
// https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering
function import_envmap_get_spherical_harmonics(source: buffer_t, source_width: i32, source_height: i32): f32_array_t {
	let sh: f32_array_t = f32_array_create(9 * 3 + 1); // Align to mult of 4 - 27->28
	let accum: f32      = 0.0;
	let weight: f32     = 1.0;
	let weight1: f32    = weight * 4 / 17;
	let weight2: f32    = weight * 8 / 17;
	let weight3: f32    = weight * 15 / 17;
	let weight4: f32    = weight * 5 / 68;
	let weight5: f32    = weight * 15 / 68;

	for (let x: i32 = 0; x < source_width; ++x) {
		for (let y: i32 = 0; y < source_height; ++y) {
			import_envmap_n = import_envmap_reverse_equirect(x / source_width, y / source_height);

			for (let i: i32 = 0; i < 3; ++i) {
				let value: f32 = buffer_get_f32(source, ((x + y * source_width) * 16 + i * 4));
				value          = math_pow(value, 1.0 / 2.2);

				sh[0 + i] += value * weight1;
				sh[3 + i] += value * weight2 * import_envmap_n.x;
				sh[6 + i] += value * weight2 * import_envmap_n.y;
				sh[9 + i] += value * weight2 * import_envmap_n.z;

				sh[12 + i] += value * weight3 * import_envmap_n.x * import_envmap_n.z;
				sh[15 + i] += value * weight3 * import_envmap_n.z * import_envmap_n.y;
				sh[18 + i] += value * weight3 * import_envmap_n.y * import_envmap_n.x;

				sh[21 + i] += value * weight4 * (3.0 * import_envmap_n.z * import_envmap_n.z - 1.0);
				sh[24 + i] += value * weight5 * (import_envmap_n.x * import_envmap_n.x - import_envmap_n.y * import_envmap_n.y);

				accum += weight;
			}
		}
	}

	for (let i: i32 = 0; i < sh.length; ++i) {
		sh[i] /= accum / 16;
	}

	return sh;
}
