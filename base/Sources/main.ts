
function main() {
	///if arm_snapshot

	///if (is_paint || is_sculpt)
	main_embed(["default_material.arm"]);
	///end
	///if is_lab
	main_embed(["placeholder.k"]);
	///end

	///if (krom_direct3d12 || krom_vulkan || krom_metal)
	main_embed_raytrace();
	///if is_paint
	main_embed_raytrace_bake();
	///end
	///end

	///else

	kickstart();

	///end
}

function kickstart() {
	// Used to locate external application data folder
	krom_set_app_name(manifest_title);
	Config.load(main_start);
}

function main_start() {
	app_on_resize = base_on_resize;
	app_on_w = base_w;
	app_on_h = base_h;
	app_on_x = base_x;
	app_on_y = base_y;

	Config.init();
	sys_start(Config.get_options());
	if (Config.raw.layout == null) base_init_layout();
	krom_set_app_name(manifest_title);
	app_init(function() {
		let o: object_t = scene_set_active("Scene");
		UniformsExt.init();
		RenderPathBase.init();

		if (Context.raw.render_mode == render_mode_t.FORWARD) {
			RenderPathDeferred.init(); // Allocate gbuffer
			RenderPathForward.init();
			render_path_commands = RenderPathForward.commands;
		}
		else {
			RenderPathDeferred.init();
			render_path_commands = RenderPathDeferred.commands;
		}

		base_init();
	});
}

///if arm_snapshot

function main_embed(additional: string[]) {
	let global: any = globalThis;

	Res.embed_raw("Scene", "Scene.arm", global["data/Scene.arm"]);
	global["data/Scene.arm"] = null;

	Res.embed_raw("shader_datas", "shader_datas.arm", global["data/shader_datas.arm"]);
	global["data/shader_datas.arm"] = null;

	Res.embed_font("font.ttf", global["data/font.ttf"]);
	global["data/font.ttf"] = null;

	Res.embed_font("font_mono.ttf", global["data/font_mono.ttf"]);
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
	for (let add of additional) files.push(add);
	for (let file of files) {
		Res.embed_blob(file, global["data/" + file]);
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
		"raytrace_brute_core" + RenderPathRaytrace.ext,
		"raytrace_brute_full" + RenderPathRaytrace.ext
	];
	for (let file of files) {
		Res.embed_blob(file, global["data/" + file]);
		global["data/" + file] = null;
	}
}

function main_embed_raytrace_bake() {
	let global: any = globalThis;
	let files: string[] = [
		"raytrace_bake_ao" + RenderPathRaytrace.ext,
		"raytrace_bake_bent" + RenderPathRaytrace.ext,
		"raytrace_bake_light" + RenderPathRaytrace.ext,
		"raytrace_bake_thick" + RenderPathRaytrace.ext
	];
	for (let file of files) {
		Res.embed_blob(file, global["data/" + file]);
		global["data/" + file] = null;
	}
}

///end

///end

main();
