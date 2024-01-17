
class Main {

	// @:keep static snapshotHelper = globalThis.Krom = {};
	static tasks: i32;

	static main = () => {
		///if arm_snapshot

		///if (is_paint || is_sculpt)
		Main.embed(["default_material.arm"]);
		///end
		///if is_lab
		Main.embed(["placeholder.k"]);
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		Main.embedRaytrace();
		///if is_paint
		Main.embedRaytraceBake();
		///end
		///end

		///else

		Main.kickstart();

		///end
	}

	// @:keep
	static kickstart = () => {
		// Used to locate external application data folder
		Krom.setApplicationName(Manifest.title);

		Main.tasks = 1;
		Main.tasks++; Config.load(() => { Main.tasks--; Main.start(); });
		Main.tasks--; Main.start();
	}

	static start = () => {
		if (Main.tasks > 0) return;

		App.onResize = Base.onResize;
		App.w = Base.w;
		App.h = Base.h;
		App.x = Base.x;
		App.y = Base.y;

		Config.init();
		System.start(Config.getOptions(), () => {
			if (Config.raw.layout == null) Base.initLayout();
			Krom.setApplicationName(Manifest.title);
			App.init(() => {
				Scene.setActive("Scene", (o: BaseObject) => {
					UniformsExt.init();
					let path = new RenderPath();
					RenderPathBase.init(path);

					if (Context.raw.renderMode == RenderMode.RenderForward) {
						RenderPathDeferred.init(path); // Allocate gbuffer
						RenderPathForward.init(path);
						path.commands = RenderPathForward.commands;
					}
					else {
						RenderPathDeferred.init(path);
						path.commands = RenderPathDeferred.commands;
					}

					RenderPath.setActive(path);
					new Base();
				});
			});
		});
	}

	///if arm_snapshot

	static embed = (additional: string[]) => {
		let global: any = globalThis;
		global.kickstart = Main.kickstart;

		Res.embedRaw("Scene", "Scene.arm", global["data/Scene.arm"]);
		global["data/Scene.arm"] = null;

		Res.embedRaw("shader_datas", "shader_datas.arm", global["data/shader_datas.arm"]);
		global["data/shader_datas.arm"] = null;

		Res.embedFont("font.ttf", global["data/font.ttf"]);
		global["data/font.ttf"] = null;

		Res.embedFont("font_mono.ttf", global["data/font_mono.ttf"]);
		global["data/font_mono.ttf"] = null;

		let files = [
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
			Res.embedBlob(file, global["data/" + file]);
			global["data/" + file] = null;
		}
	}

	///if (krom_direct3d12 || krom_vulkan || krom_metal)

	static embedRaytrace = () => {
		let global: any = globalThis;
		let files = [
			"bnoise_rank.k",
			"bnoise_scramble.k",
			"bnoise_sobol.k",
			"raytrace_brute_core" + RenderPathRaytrace.ext,
			"raytrace_brute_full" + RenderPathRaytrace.ext
		];
		for (let file of files) {
			Res.embedBlob(file, global["data/" + file]);
			global["data/" + file] = null;
		}
	}

	static embedRaytraceBake = () => {
		let global: any = globalThis;
		let files = [
			"raytrace_bake_ao" + RenderPathRaytrace.ext,
			"raytrace_bake_bent" + RenderPathRaytrace.ext,
			"raytrace_bake_light" + RenderPathRaytrace.ext,
			"raytrace_bake_thick" + RenderPathRaytrace.ext
		];
		for (let file of files) {
			Res.embedBlob(file, global["data/" + file]);
			global["data/" + file] = null;
		}
	}

	///end

	///end
}
