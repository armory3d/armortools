package arm;

import haxe.Json;
import haxe.io.Bytes;
import kha.Display;
import iron.data.Data;
import arm.ui.UISidebar;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIStatus;
import arm.render.Inc;
import arm.sys.File;
import arm.sys.Path;
import arm.Enums;
import arm.ConfigFormat;
import arm.KeymapFormat;

class Config {

	public static var raw: TConfig = null;
	public static var keymap: TKeymap;
	public static var configLoaded = false;
	#if (krom_android || krom_ios)
	public static inline var buttonAlign = zui.Zui.Align.Center;
	public static inline var buttonSpacing = "";
	#else
	public static inline var buttonAlign = zui.Zui.Align.Left;
	public static inline var buttonSpacing = "      ";
	#end

	public static function load(done: Void->Void) {
		try {
			Data.getBlob((Path.isProtected() ? Krom.savePath() : "") + "config.arm", function(blob: kha.Blob) {
				configLoaded = true;
				raw = Json.parse(blob.toString());
				done();
			});
		}
		catch (e: Dynamic) {
			#if krom_linux
			try { // Protected directory
				Data.getBlob(Krom.savePath() + "config.arm", function(blob: kha.Blob) {
					configLoaded = true;
					raw = Json.parse(blob.toString());
					done();
				});
			}
			catch (e: Dynamic) {
				done();
			}
			#else
			done();
			#end
		}
	}

	public static function save() {
		// Use system application data folder
		// when running from protected path like "Program Files"
		var path = (Path.isProtected() ? Krom.savePath() : Path.data() + Path.sep) + "config.arm";
		var bytes = Bytes.ofString(Json.stringify(raw));
		Krom.fileSaveBytes(path, bytes.getData());

		#if krom_linux // Protected directory
		if (!File.exists(path)) Krom.fileSaveBytes(Krom.savePath() + "config.arm", bytes.getData());
		#end
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
			if (disp.width >= 2560 && disp.height >= 1600) {
				raw.window_scale = 2.0;
			}
			#if (krom_android || krom_ios || krom_darwin)
			raw.window_scale = 2.0;
			#end
			raw.window_vsync = true;
			raw.window_frequency = disp.frequency;
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
			#if kha_vulkan
			raw.material_live = false;
			#else
			raw.material_live = true;
			#end
			raw.brush_3d = true;
			raw.brush_depth_reject = true;
			raw.brush_angle_reject = true;
			raw.brush_live = false;
			raw.camera_speed = 1.0;
			raw.zoom_direction = ZoomVertical;
			raw.displace_strength = 0.0;
			raw.show_asset_names = false;
			raw.node_preview = true;
			raw.workspace = 0;
			raw.layer_res = Res2048;
			raw.dilate = DilateInstant;
			raw.dilate_radius = 2;
			raw.server = "https://armorpaint.fra1.digitaloceanspaces.com";
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

		App.resHandle.position = raw.layer_res;
		loadKeymap();
	}

	public static function restore() {
		zui.Zui.Handle.global = new zui.Zui.Handle(); // Reset ui handles
		configLoaded = false;
		var _layout = raw.layout;
		init();
		raw.layout = _layout;
		initLayout();
		Translator.loadTranslations(raw.locale);
		applyConfig();
		loadTheme(raw.theme);
	}

	public static function importFrom(from: TConfig) {
		var _sha = raw.sha;
		var _version = raw.version;
		raw = from;
		raw.sha = _sha;
		raw.version = _version;
		zui.Zui.Handle.global = new zui.Zui.Handle(); // Reset ui handles
		loadKeymap();
		initLayout();
		Translator.loadTranslations(raw.locale);
		applyConfig();
		loadTheme(raw.theme);
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

	public static function applyConfig() {
		Config.raw.rp_ssgi = Context.hssgi.selected;
		Config.raw.rp_ssr = Context.hssr.selected;
		Config.raw.rp_bloom = Context.hbloom.selected;
		Config.raw.rp_gi = Context.hvxao.selected;
		Config.raw.rp_supersample = getSuperSampleSize(Context.hsupersample.position);
		iron.object.Uniforms.defaultFilter = Config.raw.rp_supersample < 1.0 ? kha.graphics4.TextureFilter.PointFilter : kha.graphics4.TextureFilter.LinearFilter;
		save();
		Context.ddirty = 2;

		var current = @:privateAccess kha.graphics2.Graphics.current;
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

	public static function initLayout() {
		var show2d = (UINodes.inst != null && UINodes.inst.show) || (UIView2D.inst != null && UIView2D.inst.show);
		raw.layout = [
			Std.int(UISidebar.defaultWindowW * raw.window_scale),
			Std.int(kha.System.windowHeight() / 3),
			Std.int(kha.System.windowHeight() / 3),
			Std.int(kha.System.windowHeight() / 3),
			show2d ? Std.int((iron.App.w() + raw.layout[LayoutNodesW]) / 2) : Std.int(iron.App.w() / 2),
			Std.int(iron.App.h() / 2),
			Std.int(UIStatus.defaultStatusH * raw.window_scale)
		];
	}

	public static function loadTheme(theme: String, tagRedraw = true) {
		if (theme == "default.json") { // Built-in default
			App.theme = zui.Themes.dark;
		}
		else {
			Data.getBlob("themes/" + theme, function(b: kha.Blob) {
				App.theme = Json.parse(b.toString());
			});
		}
		App.theme.FILL_WINDOW_BG = true;
		if (tagRedraw) {
			App.uiBox.t = App.theme;
			App.uiMenu.t = App.theme;
			UISidebar.inst.ui.t = App.theme;
			UINodes.inst.ui.t = App.theme;
			UIView2D.inst.ui.t = App.theme;
			UISidebar.inst.tagUIRedraw();
		}
		#if (krom_android || krom_ios)
		App.theme.FULL_TABS = true;
		#end
	}

	public static function enablePlugin(f: String) {
		raw.plugins.push(f);
		Plugin.start(f);
	}

	public static function disablePlugin(f: String) {
		raw.plugins.remove(f);
		Plugin.stop(f);
	}
}
