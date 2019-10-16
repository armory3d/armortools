package ;

import kha.Window;
import kha.WindowOptions;
import kha.WindowMode;
import kha.System;
import iron.object.Object;
import iron.Scene;
import iron.RenderPath;
import arm.render.Inc;
import arm.render.RenderPathDeferred;
import arm.render.Uniforms;
import arm.Config;
using StringTools;

class Main {

	static var tasks:Int;

	public static function main() {
		tasks = 1;
		tasks++; Config.load(function() { tasks--; start(); });
		tasks--; start();
	}

	static function start() {
		if (tasks > 0) return;

		Config.create();
		var c = Config.raw;
		var windowMode = c.window_mode == 0 ? WindowMode.Windowed : WindowMode.Fullscreen;
		var windowFeatures = None;
		if (c.window_resizable) windowFeatures |= FeatureResizable;
		if (c.window_maximizable) windowFeatures |= FeatureMaximizable;
		if (c.window_minimizable) windowFeatures |= FeatureMinimizable;

		#if arm_player
		var title = Krom.getArg(0);
		title = title.replace("\\", "/");
		var lasti = title.lastIndexOf("/");
		if (lasti >= 0) title = title.substr(lasti + 1);
		if (title.endsWith(".exe")) title = title.substr(0, title.length - 4);
		#else
		var title = "untitled - ArmorPaint";
		#end

		System.start({title: title, window: {width: c.window_w, height: c.window_h, x: c.window_x, y: c.window_y, mode: windowMode, windowFeatures: windowFeatures}, framebuffer: {samplesPerPixel: 1, verticalSync: c.window_vsync}}, function(window:Window) {
			iron.App.init(function() {
				Scene.setActive("Scene", function(o:Object) {
					Uniforms.init();
					var path = new RenderPath();
					Inc.init(path);
					RenderPathDeferred.init(path);
					path.commands = RenderPathDeferred.commands;
					RenderPath.setActive(path);
					#if arm_player
					new arm.Player();
					#else
					new arm.App();
					#end
				});
			});
		});
	}
}
