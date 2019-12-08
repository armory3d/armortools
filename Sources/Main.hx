package;

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
#if arm_player
using StringTools;
#end

class Main {

	static var tasks: Int;

	public static function main() {
		tasks = 1;
		tasks++; Config.load(function() { tasks--; start(); });
		#if arm_physics
		tasks++; loadPhysics();
		#end
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
				verticalSync: c.window_vsync
			}
		};

		System.start(options, function(window: Window) {
			iron.App.init(function() {
				Scene.setActive("Scene", function(o: Object) {
					Config.init();
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
					#if arm_physics
					o.addTrait(new arm.plugin.PhysicsWorld());
					#end
				});
			});
		});
	}

	#if arm_physics
	static function loadPhysics() {
		var b = haxe.io.Bytes.ofData(Krom.loadBlob("data/ammo.wasm.js"));
		var print = function(s: String) { trace(s); };
		var loaded = function() { tasks--; start(); };
		untyped __js__("(1, eval)({0})", b.toString());
		var instantiateWasm = function(imports, successCallback) {
			var wasmbin = Krom.loadBlob("data/ammo.wasm.wasm");
			var module = new js.lib.webassembly.Module(wasmbin);
			var inst = new js.lib.webassembly.Instance(module, imports);
			successCallback(inst);
			return inst.exports;
		};
		untyped __js__("Ammo({print:print, instantiateWasm:instantiateWasm}).then(loaded)");
	}
	#end
}
