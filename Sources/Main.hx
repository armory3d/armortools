package ;

import kha.Window;
import kha.WindowOptions;
import kha.WindowMode;
import kha.System;
import iron.Scene;
import iron.RenderPath;
import iron.object.Object;
import arm.render.Inc;
import arm.render.RenderPathDeferred;
import arm.Config;

class Main {
	
	public static inline var voxelgiVoxelSize = 2.0 / 256;
	public static inline var voxelgiHalfExtents = 1;
	
	static var tasks:Int;

	public static function main() {
		tasks = 1;
		#if (arm_config) tasks++; Config.load(function() { tasks--; start(); }); #end
		tasks--; start();
	}

	static function start() {
		if (tasks > 0) return;
		
		if (Config.raw == null) Config.raw = {};
		var c = Config.raw;
		if (c.window_mode == null) c.window_mode = 0;
		if (c.window_resizable == null) c.window_resizable = true;
		if (c.window_minimizable == null) c.window_minimizable = true;
		if (c.window_maximizable == null) c.window_maximizable = true;
		if (c.window_w == null) c.window_w = 1600;
		if (c.window_h == null) c.window_h = 900;
		if (c.window_scale == null) c.window_scale = 1.0;
		if (c.window_msaa == null) c.window_msaa = 1;
		if (c.window_vsync == null) c.window_vsync = true;
		
		var windowMode = c.window_mode == 0 ? WindowMode.Windowed : WindowMode.Fullscreen;
		var windowFeatures = None;
		if (c.window_resizable) windowFeatures |= FeatureResizable;
		if (c.window_maximizable) windowFeatures |= FeatureMaximizable;
		if (c.window_minimizable) windowFeatures |= FeatureMinimizable;

		System.start({title: "untitled - ArmorPaint", width: c.window_w, height: c.window_h, window: {mode: windowMode, windowFeatures: windowFeatures}, framebuffer: {samplesPerPixel: c.window_msaa, verticalSync: c.window_vsync}}, function(window:Window) {
			iron.App.init(function() {
				Scene.setActive("Scene", function(object:Object) {
					var path = new RenderPath();
					Inc.init(path);
					RenderPathDeferred.init(path);
					path.commands = RenderPathDeferred.commands;
					RenderPath.setActive(path);
					new arm.App();
				});
			});
		});
	}
}
