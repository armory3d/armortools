package arm;

import haxe.Json;
import haxe.io.Bytes;
import kha.Display;
import iron.data.Data;
#if arm_painter
import arm.ui.UISidebar;
import arm.render.Inc;
import arm.sys.Path;
import arm.Enums;
#end
import arm.ConfigFormat;

class Config {

	public static var raw: TConfig = null;
	public static var keymap: Dynamic;
	public static var configLoaded = false;

	public static function load(done: Void->Void) {
		try {
			Data.getBlob((Path.isProtected() ? Krom.savePath() : "") + "config.arm", function(blob: kha.Blob) {
				configLoaded = true;
				raw = Json.parse(blob.toString());
				done();
			});
		}
		catch (e: Dynamic) { done(); }
	}

	public static function save() {
		// Use system application data folder
		// when running from protected path like "Program Files"
		var path = (Path.isProtected() ? Krom.savePath() : Path.data() + Path.sep) + "config.arm";
		var bytes = Bytes.ofString(Json.stringify(raw));
		Krom.fileSaveBytes(path, bytes.getData());
	}

	public static function init() {
		if (!configLoaded || raw == null) {
			raw = {};
			raw.locale = "system";
			raw.window_mode = 0;
			raw.window_resizable = true;
			raw.window_minimizable = true;
			raw.window_maximizable = true;
			raw.window_w = 1600;
			raw.window_h = 900;
			#if krom_darwin
			raw.window_w *= 2;
			raw.window_h *= 2;
			#end
			raw.window_x = -1;
			raw.window_y = -1;
			raw.window_scale = 1.0;
			var disp = Display.primary;
			if (disp != null && disp.width >= 2560 && disp.height >= 1600) {
				raw.window_scale = 2.0;
			}
			#if (krom_android || krom_ios || krom_darwin)
			raw.window_scale = 2.0;
			#end
			raw.window_vsync = true;
			raw.window_frequency = 60;
			if (disp != null) {
				raw.window_frequency = disp.frequency;
			}
			raw.rp_bloom = false;
			raw.rp_gi = false;
			raw.rp_vignette = 0.4;
			raw.rp_motionblur = false;
			#if (krom_android || krom_ios)
			raw.rp_ssgi = false;
			#else
			raw.rp_ssgi = true;
			#end
			raw.rp_ssr = false;
			raw.rp_supersample = 1.0;
			#if krom_darwin
			raw.rp_supersample = 0.5;
			#end

			#if arm_painter
			raw.version = Main.version;
			raw.sha = Main.sha;
			raw.recent_projects = [];
			raw.bookmarks = [];
			raw.plugins = [];
			raw.keymap = "default.json";
			raw.theme = "default.json";
			raw.undo_steps = 4;
			raw.pressure_radius = true;
			raw.pressure_hardness = true;
			raw.pressure_angle = false;
			raw.pressure_opacity = false;
			raw.pressure_sensitivity = 1.0;
			raw.material_live = true;
			#if kha_vulkan
			raw.brush_3d = false; // TODO
			#else
			raw.brush_3d = true;
			#end
			raw.brush_live = false;
			raw.camera_speed = 1.0;
			raw.displace_strength = 1.0;
			#if krom_android
			raw.native_file_browser = false;
			#else
			raw.native_file_browser = true;
			#end
			raw.show_asset_names = false;
			#end

			#if arm_creator
			// raw.displace_strength = 100.0;
			#end
		}
		else {
			// Upgrade config format created by older ArmorPaint build
			// if (raw.version != Main.version) {
			// 	raw.version = Main.version;
			// 	save();
			// }
			if (raw.sha != Main.sha) {
				configLoaded = false;
				init();
				return;
			}
		}

		#if arm_painter
		loadKeymap();
		#end
	}

	public static function restore() {
		zui.Zui.Handle.global = new zui.Zui.Handle(); // Reset ui handles
		configLoaded = false;
		init();
		Translator.loadTranslations(raw.locale);
		#if arm_painter
		applyConfig();
		arm.ui.BoxPreferences.loadTheme(raw.theme);
		#end
	}

	public static inline function getSuperSampleQuality(f: Float): Int {
		return f == 0.25 ? 0 :
			   f == 0.5 ? 1 :
			   f == 1.0 ? 2 :
			   f == 1.5 ? 3 :
			   f == 2.0 ? 4 : 5;
	}

	public static inline function getSuperSampleSize(i: Int): Float {
		return i == 0 ? 0.25 :
			   i == 1 ? 0.5 :
			   i == 2 ? 1.0 :
			   i == 3 ? 1.5 :
			   i == 4 ? 2.0 : 4.0;
	}

	#if arm_painter
	public static function applyConfig() {
		Config.raw.rp_ssgi = Context.hssgi.selected;
		Config.raw.rp_ssr = Context.hssr.selected;
		Config.raw.rp_bloom = Context.hbloom.selected;
		Config.raw.rp_gi = Context.hvxao.selected;
		Config.raw.rp_supersample = getSuperSampleSize(Context.hsupersample.position);
		iron.object.Uniforms.defaultFilter = Config.raw.rp_supersample < 1.0 ? kha.graphics4.TextureFilter.PointFilter : kha.graphics4.TextureFilter.LinearFilter;
		save();
		Context.ddirty = 2;

		var current = @:privateAccess kha.graphics4.Graphics2.current;
		if (current != null) current.end();
		Inc.applyConfig();
		if (current != null) current.begin(false);
	}

	public static function loadKeymap() {
		Data.getBlob("keymap_presets/" + raw.keymap, function(blob: kha.Blob) {
			keymap = Json.parse(blob.toString());
		});
	}

	public static function saveKeymap() {
		var path = Data.dataPath + "keymap_presets/" + raw.keymap;
		var bytes = Bytes.ofString(Json.stringify(keymap));
		Krom.fileSaveBytes(path, bytes.getData());
	}

	static function getTextureRes(): Int {
		var res = App.resHandle.position;
		return res == Res128 ? 128 :
			   res == Res256 ? 256 :
			   res == Res512 ? 512 :
			   res == Res1024 ? 1024 :
			   res == Res2048 ? 2048 :
			   res == Res4096 ? 4096 :
			   res == Res8192 ? 8192 :
			   res == Res16384 ? 16384 : 0;
	}

	public static function getTextureResX(): Int {
		return Context.projectAspectRatio == 2 ? Std.int(getTextureRes() / 2) : getTextureRes();
	}

	public static function getTextureResY(): Int {
		return Context.projectAspectRatio == 1 ? Std.int(getTextureRes() / 2) : getTextureRes();
	}

	public static function getTextureResBias(): Float {
		var res = App.resHandle.position;
		return res == Res128 ? 16.0 :
			   res == Res256 ? 8.0 :
			   res == Res512 ? 4.0 :
			   res == Res1024 ? 2.0 :
			   res == Res2048 ? 1.5 :
			   res == Res4096 ? 1.0 :
			   res == Res8192 ? 0.5 :
			   res == Res16384 ? 0.25 : 1.0;
	}

	public static function getTextureResPos(i: Int): Int {
		return i == 128 ? Res128 :
			   i == 256 ? Res256 :
			   i == 512 ? Res512 :
			   i == 1024 ? Res1024 :
			   i == 2048 ? Res2048 :
			   i == 4096 ? Res4096 :
			   i == 8192 ? Res8192 :
			   i == 16384 ? Res16384 : 0;
	}
	#end
}
