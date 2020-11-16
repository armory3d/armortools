package;

import kha.Window;
import kha.WindowOptions;
import kha.WindowMode;
import kha.System;
import iron.object.Object;
import iron.Scene;
import iron.RenderPath;
import arm.render.Inc;
import arm.render.RenderPathForward;
import arm.render.RenderPathDeferred;
import arm.render.Uniforms;
import arm.util.BuildMacros;
import arm.Config;
import arm.Context;
#if arm_vr
import arm.render.RenderPathForwardVR;
#end

class Main {

	public static var version = "0.8";
	public static var sha = BuildMacros.sha().substr(1, 7);
	public static var date = BuildMacros.date().split(" ")[0];
	static var tasks: Int;

	#if arm_snapshot
	static function embedRaw(handle: String, name: String, file: js.lib.ArrayBuffer) {
		iron.data.Data.cachedBlobs.set(name, kha.Blob.fromBytes(haxe.io.Bytes.ofData(file)));
		iron.data.Data.getSceneRaw(handle, function(_) {});
		iron.data.Data.cachedBlobs.remove(name);
	}
	static function embedBlob(name: String, file: js.lib.ArrayBuffer) {
		iron.data.Data.cachedBlobs.set(name, kha.Blob.fromBytes(haxe.io.Bytes.ofData(file)));
	}
	static function embedFont(name: String, file: js.lib.ArrayBuffer) {
		iron.data.Data.cachedFonts.set(name, new kha.Font(kha.Blob.fromBytes(haxe.io.Bytes.ofData(file))));
	}
	#end

	public static function main() {
		#if arm_snapshot

		var global = js.Syntax.code("globalThis");
		global.kickstart = kickstart;

		embedRaw("Scene", "Scene.arm", untyped global['data/Scene.arm']);
		untyped global['data/Scene.arm'] = null;

		embedRaw("shader_datas", "shader_datas.arm", untyped global['data/shader_datas.arm']);
		untyped global['data/shader_datas.arm'] = null;

		embedFont("font.ttf", untyped global['data/font.ttf']);
		untyped global['data/font.ttf'] = null;

		embedFont("font_mono.ttf", untyped global['data/font_mono.ttf']);
		untyped global['data/font_mono.ttf'] = null;

		var files = [
			"font13.bin",
			"ltc_mag.arm",
			"ltc_mat.arm",
			"default_brush.arm",
			"default_material.arm",
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
			"cursor.k",
			"icons.k",
			"icons2x.k",
			"noise256.k",
			"smaa_search.k",
			"smaa_area.k"
		];

		for (file in files) {
			embedBlob(file, untyped global['data/' + file]);
			untyped global['data/' + file] = null;
		}

		#if (kha_direct3d12 || kha_vulkan)

		var ext = #if kha_direct3d12 ".cso" #else ".spirv" #end ;

		var files_renderlib = [
			"bnoise_rank.k",
			"bnoise_scramble.k",
			"bnoise_sobol.k",
			"raytrace_bake_ao" + ext,
			"raytrace_bake_bent" + ext,
			"raytrace_bake_light" + ext,
			"raytrace_bake_thick" + ext,
			"raytrace_brute_core" + ext,
			"raytrace_brute_full" + ext
		];

		for (file in files_renderlib) {
			embedBlob(file, untyped global['data/' + file]);
			untyped global['data/' + file] = null;
		}

		#end

		#end // arm_snapshot

		#if (!arm_snapshot)
		iron.data.ShaderData.shaderPath = ""; // Use arm_data_dir
		kickstart();
		#end
	}

	@:keep
	public static function kickstart() {
		// Used to locate external application data folder
		Krom.setApplicationName("ArmorPaint");

		tasks = 1;
		tasks++; Config.load(function() { tasks--; start(); });
		#if arm_physics
		tasks++; arm.plugin.PhysicsWorld.load();
		#end
		tasks--; start();
	}

	static function start() {
		if (tasks > 0) return;

		Config.init();
		var c = Config.raw;

		var windowMode = c.window_mode == 0 ? WindowMode.Windowed : WindowMode.Fullscreen;
		var windowFeatures = None;
		if (c.window_resizable) windowFeatures |= FeatureResizable;
		if (c.window_maximizable) windowFeatures |= FeatureMaximizable;
		if (c.window_minimizable) windowFeatures |= FeatureMinimizable;
		var title = "untitled - ArmorPaint";
		var options: kha.SystemOptions = {
			title: title,
			window: {
				width: c.window_w,
				height: c.window_h,
				x: c.window_x,
				y: c.window_y,
				mode: windowMode,
				windowFeatures: windowFeatures
			},
			framebuffer: {
				samplesPerPixel: 1,
				verticalSync: c.window_vsync,
				frequency: c.window_frequency
			}
		};

		System.start(options, function(window: Window) {
			if (Config.raw.layout == null) Config.initLayout();
			Krom.setApplicationName("ArmorPaint");
			iron.App.init(function() {
				Scene.setActive("Scene", function(o: Object) {
					Uniforms.init();
					var path = new RenderPath();
					Inc.init(path);

					#if arm_vr
					RenderPathDeferred.init(path); // Allocate gbuffer
					RenderPathForward.init(path);
					RenderPathForwardVR.init(path);
					path.commands = RenderPathForwardVR.commands;
					#else
					if (Context.renderMode == RenderForward) {
						RenderPathDeferred.init(path); // Allocate gbuffer
						RenderPathForward.init(path);
						path.commands = RenderPathForward.commands;
					}
					else {
						RenderPathDeferred.init(path);
						path.commands = RenderPathDeferred.commands;
					}
					#end

					RenderPath.setActive(path);
					new arm.App();
					#if arm_physics
					o.addTrait(new arm.plugin.PhysicsWorld());
					#end
				});
			});
		});
	}
}
