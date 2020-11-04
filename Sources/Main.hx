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
	static function embedRaw(handle: String, name: String, file: String) {
		iron.data.Data.cachedBlobs.set(name, kha.Blob.fromBytes(haxe.Resource.getBytes(file)));
		iron.data.Data.getSceneRaw(handle, function(_) {});
		iron.data.Data.cachedBlobs.remove(name);
	}
	static function embedBlob(name: String, file: String) {
		iron.data.Data.cachedBlobs.set(name, kha.Blob.fromBytes(haxe.Resource.getBytes(file)));
	}
	static function embedFont(name: String, file: String) {
		iron.data.Data.cachedFonts.set(name, new kha.Kravur(kha.Blob.fromBytes(haxe.Resource.getBytes(file))));
	}
	#end

	public static function main() {
		#if arm_snapshot

		js.Syntax.code("globalThis.kickstart = Main.kickstart");

		BuildMacros.embed("../Assets/common/Scene.arm");
		embedRaw("Scene", "Scene.arm", "../Assets/common/Scene.arm");

		BuildMacros.embed("../Assets/common/shader_datas.arm");
		embedRaw("shader_datas", "shader_datas.arm", "../Assets/common/shader_datas.arm");

		BuildMacros.embed("../Assets/common/font.ttf");
		embedFont("font.ttf", "../Assets/common/font.ttf");

		BuildMacros.embed("../Assets/common/font_mono.ttf");
		embedFont("font_mono.ttf", "../Assets/common/font_mono.ttf");

		BuildMacros.embed("../Assets/common/font13.bin");
		embedBlob("font13.bin", "../Assets/common/font13.bin");

		BuildMacros.embed("../Assets/common/ltc_mag.arm");
		embedBlob("ltc_mag.arm", "../Assets/common/ltc_mag.arm");

		BuildMacros.embed("../Assets/common/ltc_mat.arm");
		embedBlob("ltc_mat.arm", "../Assets/common/ltc_mat.arm");

		BuildMacros.embed("../Assets/common/World_irradiance.arm");
		embedBlob("World_irradiance.arm", "../Assets/common/World_irradiance.arm");

		BuildMacros.embed("../Assets/common/default_brush.arm");
		embedBlob("default_brush.arm", "../Assets/common/default_brush.arm");

		BuildMacros.embed("../Assets/common/default_material.arm");
		embedBlob("default_material.arm", "../Assets/common/default_material.arm");

		BuildMacros.embed("../Assets/common/embed/World_radiance.k");
		embedBlob("World_radiance.k", "../Assets/common/embed/World_radiance.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_0.k");
		embedBlob("World_radiance_0.k", "../Assets/common/embed/World_radiance_0.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_1.k");
		embedBlob("World_radiance_1.k", "../Assets/common/embed/World_radiance_1.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_2.k");
		embedBlob("World_radiance_2.k", "../Assets/common/embed/World_radiance_2.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_3.k");
		embedBlob("World_radiance_3.k", "../Assets/common/embed/World_radiance_3.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_4.k");
		embedBlob("World_radiance_4.k", "../Assets/common/embed/World_radiance_4.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_5.k");
		embedBlob("World_radiance_5.k", "../Assets/common/embed/World_radiance_5.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_6.k");
		embedBlob("World_radiance_6.k", "../Assets/common/embed/World_radiance_6.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_7.k");
		embedBlob("World_radiance_7.k", "../Assets/common/embed/World_radiance_7.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_8.k");
		embedBlob("World_radiance_8.k", "../Assets/common/embed/World_radiance_8.k");

		BuildMacros.embed("../Assets/common/embed/World_radiance_9.k");
		embedBlob("World_radiance_9.k", "../Assets/common/embed/World_radiance_9.k");

		BuildMacros.embed("../Assets/common/embed/brdf.k");
		embedBlob("brdf.k", "../Assets/common/embed/brdf.k");

		BuildMacros.embed("../Assets/common/embed/color_wheel.k");
		embedBlob("color_wheel.k", "../Assets/common/embed/color_wheel.k");

		BuildMacros.embed("../Assets/common/embed/cursor.k");
		embedBlob("cursor.k", "../Assets/common/embed/cursor.k");

		BuildMacros.embed("../Assets/common/embed/icons.k");
		embedBlob("icons.k", "../Assets/common/embed/icons.k");

		BuildMacros.embed("../Assets/common/embed/icons2x.k");
		embedBlob("icons2x.k", "../Assets/common/embed/icons2x.k");

		BuildMacros.embed("../Assets/common/embed/noise256.k");
		embedBlob("noise256.k", "../Assets/common/embed/noise256.k");

		BuildMacros.embed("../Assets/common/embed/smaa_search.k");
		embedBlob("smaa_search.k", "../Assets/common/embed/smaa_search.k");

		BuildMacros.embed("../Assets/common/embed/smaa_area.k");
		embedBlob("smaa_area.k", "../Assets/common/embed/smaa_area.k");

		@:privateAccess haxe.Resource.content = null;

		#else

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
