
function main() {

	// ///if (is_paint || is_sculpt)
	// main_embed(["default_material.arm"]);
	// ///end
	// ///if is_lab
	// main_embed(["placeholder.k"]);
	// ///end

	// ///if (krom_direct3d12 || krom_vulkan || krom_metal)
	// main_embed_raytrace();
	// ///if is_paint
	// main_embed_raytrace_bake();
	// ///end

	// Used to locate external application data folder
	krom_set_app_name(manifest_title);
	config_load();

	app_on_resize = base_on_resize;
	app_on_w = base_w;
	app_on_h = base_h;
	app_on_x = base_x;
	app_on_y = base_y;

	context_init();
	config_init();
	sys_start(config_get_options());
	if (config_raw.layout == null) {
		base_init_layout();
	}
	krom_set_app_name(manifest_title);
	app_init();
	scene_set_active("Scene");
	uniforms_ext_init();
	render_path_base_init();

	if (context_raw.render_mode == render_mode_t.FORWARD) {
		render_path_deferred_init(); // Allocate gbuffer
		render_path_forward_init();
		render_path_commands = render_path_forward_commands;
	}
	else {
		render_path_deferred_init();
		render_path_commands = render_path_deferred_commands;
	}

	base_init();
}

///if arm_embed

function main_embed(additional: string[]) {
	let global: any = globalThis;

	resource_embed_raw("Scene", "Scene.arm", global["data/Scene.arm"]);
	global["data/Scene.arm"] = null;

	resource_embed_raw("shader_datas", "shader_datas.arm", global["data/shader_datas.arm"]);
	global["data/shader_datas.arm"] = null;

	resource_embed_font("font.ttf", global["data/font.ttf"]);
	global["data/font.ttf"] = null;

	resource_embed_font("font_mono.ttf", global["data/font_mono.ttf"]);
	global["data/font_mono.ttf"] = null;

	let files: string[] = [
		"ltc_mag.arm",
		"ltc_mat.arm",
		"default_brush.arm",
		"World_irradiance.arm",
		"World_radiance.k",
		"World_radiance_0.k",
		"World_radiance_1.k",
		"World_radiance_2.k",
		"World_radiance_3.k",
		"World_radiance_4.k",
		"World_radiance_5.k",
		"World_radiance_6.k",
		"World_radiance_7.k",
		"World_radiance_8.k",
		"brdf.k",
		"color_wheel.k",
		"color_wheel_gradient.k",
		"cursor.k",
		"icons.k",
		"icons2x.k",
		"badge.k",
		"noise256.k",
		"smaa_search.k",
		"smaa_area.k",
		"text_coloring.json",
		"version.json"
	];
	for (let i: i32 = 0; i < additional.length; ++i) {
		let add: string = additional[i];
		array_push(files, add);
	}
	for (let i: i32 = 0; i < files.length; ++i) {
		let file: string = files[i];
		resource_embed_blob(file, global["data/" + file]);
		global["data/" + file] = null;
	}
}

///if (krom_direct3d12 || krom_vulkan || krom_metal)

function main_embed_raytrace() {
	let global: any = globalThis;
	let files: string[] = [
		"bnoise_rank.k",
		"bnoise_scramble.k",
		"bnoise_sobol.k",
		"raytrace_brute_core" + render_path_raytrace_ext,
		"raytrace_brute_full" + render_path_raytrace_ext
	];
	for (let i: i32 = 0; i < files.length; ++i) {
		let file: string = files[i];
		resource_embed_blob(file, global["data/" + file]);
		global["data/" + file] = null;
	}
}

function main_embed_raytrace_bake() {
	let global: any = globalThis;
	let files: string[] = [
		"raytrace_bake_ao" + render_path_raytrace_ext,
		"raytrace_bake_bent" + render_path_raytrace_ext,
		"raytrace_bake_light" + render_path_raytrace_ext,
		"raytrace_bake_thick" + render_path_raytrace_ext
	];
	for (let i: i32 = 0; i < files.length; ++i) {
		let file: string = files[i];
		resource_embed_blob(file, global["data/" + file]);
		global["data/" + file] = null;
	}
}

///end

///end

main();
