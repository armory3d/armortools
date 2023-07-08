package;

import kha.Window;
import kha.System;
import iron.object.Object;
import iron.Scene;
import iron.RenderPath;
import arm.render.RenderPathBase;
import arm.render.RenderPathForward;
import arm.render.RenderPathDeferred;
import arm.render.RenderPathRaytrace;
import arm.render.Uniforms;
import arm.sys.BuildMacros;
import arm.Config;
import arm.Context;
import arm.Res;

class Main {

	public static var sha = BuildMacros.sha().substr(1, 7);
	public static var date = BuildMacros.date().split(" ")[0];
	public static var tasks: Int;

	public static function main() {
		#if arm_snapshot

		#if (is_paint || is_sculpt)
		embed(["default_material.arm"]);
		#end
		#if is_lab
		embed(["placeholder.k"]);
		#end

		#if (kha_direct3d12 || kha_vulkan || kha_metal)
		embedRaytrace();
		#if is_paint
		embedRaytraceBake();
		#end
		#end

		#else

		kickstart();

		#end
	}

	@:keep
	public static function kickstart() {
		#if (!arm_snapshot)
		iron.data.ShaderData.shaderPath = ""; // Use arm_data_dir
		#end

		// Used to locate external application data folder
		Krom.setApplicationName(Manifest.title);

		tasks = 1;
		tasks++; Config.load(function() { tasks--; start(); });
		tasks--; start();
	}

	public static function start() {
		if (tasks > 0) return;

		iron.App.onResize = arm.App.onResize;
		iron.App.w = arm.App.w;
		iron.App.h = arm.App.h;
		iron.App.x = arm.App.x;
		iron.App.y = arm.App.y;

		Config.init();
		System.start(Config.getOptions(), function(window: Window) {
			if (Config.raw.layout == null) arm.App.initLayout();
			Krom.setApplicationName(Manifest.title);
			iron.App.init(function() {
				Scene.setActive("Scene", function(o: Object) {
					Uniforms.init();
					var path = new RenderPath();
					RenderPathBase.init(path);

					if (Context.raw.renderMode == RenderForward) {
						RenderPathDeferred.init(path); // Allocate gbuffer
						RenderPathForward.init(path);
						path.commands = RenderPathForward.commands;
					}
					else {
						RenderPathDeferred.init(path);
						path.commands = RenderPathDeferred.commands;
					}

					RenderPath.setActive(path);
					new arm.App();
				});
			});
		});
	}

	#if arm_snapshot

	public static function embed(additional: Array<String>) {
		var global = js.Syntax.code("globalThis");
		global.kickstart = Main.kickstart;

		Res.embedRaw("Scene", "Scene.arm", untyped global["data/Scene.arm"]);
		untyped global["data/Scene.arm"] = null;

		Res.embedRaw("shader_datas", "shader_datas.arm", untyped global["data/shader_datas.arm"]);
		untyped global["data/shader_datas.arm"] = null;

		Res.embedFont("font.ttf", untyped global["data/font.ttf"]);
		untyped global["data/font.ttf"] = null;

		Res.embedFont("font_mono.ttf", untyped global["data/font_mono.ttf"]);
		untyped global["data/font_mono.ttf"] = null;

		var files = [
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
			"text_coloring.json"
		];
		for (add in additional) files.push(add);
		for (file in files) {
			Res.embedBlob(file, untyped global["data/" + file]);
			untyped global["data/" + file] = null;
		}
	}

	#if (kha_direct3d12 || kha_vulkan || kha_metal)

	public static function embedRaytrace() {
		var global = js.Syntax.code("globalThis");
		var files = [
			"bnoise_rank.k",
			"bnoise_scramble.k",
			"bnoise_sobol.k",
			"raytrace_brute_core" + RenderPathRaytrace.ext,
			"raytrace_brute_full" + RenderPathRaytrace.ext
		];
		for (file in files) {
			Res.embedBlob(file, untyped global["data/" + file]);
			untyped global["data/" + file] = null;
		}
	}

	public static function embedRaytraceBake() {
		var global = js.Syntax.code("globalThis");
		var files = [
			"raytrace_bake_ao" + RenderPathRaytrace.ext,
			"raytrace_bake_bent" + RenderPathRaytrace.ext,
			"raytrace_bake_light" + RenderPathRaytrace.ext,
			"raytrace_bake_thick" + RenderPathRaytrace.ext
		];
		for (file in files) {
			Res.embedBlob(file, untyped global["data/" + file]);
			untyped global["data/" + file] = null;
		}
	}

	#end

	#end
}
