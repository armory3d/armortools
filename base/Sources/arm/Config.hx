package arm;

import haxe.Json;
import iron.System;
import iron.Data;
import zui.Zui;
import arm.ui.UIBase;
import arm.ui.UIHeader;
import arm.render.RenderPathBase;
import arm.sys.File;
import arm.sys.Path;
import arm.ConfigFormat;

class Config {

	public static var raw: TConfig = null;
	public static var keymap: Dynamic;
	public static var configLoaded = false;
	public static var buttonAlign = zui.Zui.Align.Left;
	public static inline var defaultButtonSpacing = "       ";
	public static var buttonSpacing = defaultButtonSpacing;

	public static function load(done: Void->Void) {
		try {
			Data.getBlob((Path.isProtected() ? Krom.savePath() : "") + "config.json", function(blob: js.lib.ArrayBuffer) {
				configLoaded = true;
				raw = Json.parse(System.bufferToString(blob));

				done();
			});
		}
		catch (e: Dynamic) {
			#if krom_linux
			try { // Protected directory
				Data.getBlob(Krom.savePath() + "config.json", function(blob: js.lib.ArrayBuffer) {
					configLoaded = true;
					raw = Json.parse(System.bufferToString(blob));
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
		var path = (Path.isProtected() ? Krom.savePath() : Path.data() + Path.sep) + "config.json";
		var buffer = System.stringToBuffer(Json.stringify(raw));
		Krom.fileSaveBytes(path, buffer);

		#if krom_linux // Protected directory
		if (!File.exists(path)) Krom.fileSaveBytes(Krom.savePath() + "config.json", buffer);
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
			if (System.displayWidth() >= 2560 && System.displayHeight() >= 1600) {
				raw.window_scale = 2.0;
			}
			#if (krom_android || krom_ios || krom_darwin)
			raw.window_scale = 2.0;
			#end
			raw.window_vsync = true;
			raw.window_frequency = System.displayFrequency();
			raw.rp_bloom = false;
			raw.rp_gi = false;
			raw.rp_vignette = 0.2;
			raw.rp_grain = 0.09;
			raw.rp_motionblur = false;
			#if (krom_android || krom_ios)
			raw.rp_ssao = false;
			#else
			raw.rp_ssao = true;
			#end
			raw.rp_ssr = false;
			raw.rp_supersample = 1.0;
			raw.version = Manifest.version;
			raw.sha = getSha();
			Base.initConfig();
		}
		else {
			// Upgrade config format created by older ArmorPaint build
			// if (raw.version != Manifest.version) {
			// 	raw.version = Manifest.version;
			// 	save();
			// }
			if (raw.sha != getSha()) {
				configLoaded = false;
				init();
				return;
			}
		}

		Zui.touchScroll = Zui.touchHold = Zui.touchTooltip = Config.raw.touch_ui;
		Base.resHandle.position = raw.layer_res;
		loadKeymap();
	}

	public static function getSha(): String {
		var sha = "";
		Data.getBlob("version.json", function(blob: js.lib.ArrayBuffer) {
			sha = Json.parse(System.bufferToString(blob)).sha;
		});
		return sha;
	}

	public static function getDate(): String {
		var date = "";
		Data.getBlob("version.json", function(blob: js.lib.ArrayBuffer) {
			date = Json.parse(System.bufferToString(blob)).date;
		});
		return date;
	}

	public static function getOptions(): SystemOptions {
		var windowMode = raw.window_mode == 0 ? WindowMode.Windowed : WindowMode.Fullscreen;
		var windowFeatures = WindowFeatures.FeatureNone;
		if (raw.window_resizable) windowFeatures |= FeatureResizable;
		if (raw.window_maximizable) windowFeatures |= FeatureMaximizable;
		if (raw.window_minimizable) windowFeatures |= FeatureMinimizable;
		var title = "untitled - " + Manifest.title;
		return {
			title: title,
			width: raw.window_w,
			height: raw.window_h,
			x: raw.window_x,
			y: raw.window_y,
			mode: windowMode,
			features: windowFeatures,
			vsync: raw.window_vsync,
			frequency: raw.window_frequency
		};
	}

	public static function restore() {
		zui.Zui.children = []; // Reset ui handles
		configLoaded = false;
		var _layout = raw.layout;
		init();
		raw.layout = _layout;
		Base.initLayout();
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
		zui.Zui.children = []; // Reset ui handles
		loadKeymap();
		Base.initLayout();
		Translator.loadTranslations(raw.locale);
		applyConfig();
		loadTheme(raw.theme);
	}

	public static function applyConfig() {
		Config.raw.rp_ssao = Context.raw.hssao.selected;
		Config.raw.rp_ssr = Context.raw.hssr.selected;
		Config.raw.rp_bloom = Context.raw.hbloom.selected;
		Config.raw.rp_gi = Context.raw.hvxao.selected;
		Config.raw.rp_supersample = getSuperSampleSize(Context.raw.hsupersample.position);
		save();
		Context.raw.ddirty = 2;

		var current = Graphics2.current;
		if (current != null) current.end();
		RenderPathBase.applyConfig();
		if (current != null) current.begin(false);
	}

	public static function loadKeymap() {
		if (raw.keymap == "default.json") { // Built-in default
			keymap = Base.defaultKeymap;
		}
		else {
			Data.getBlob("keymap_presets/" + raw.keymap, function(blob: js.lib.ArrayBuffer) {
				keymap = Json.parse(System.bufferToString(blob));
				// Fill in undefined keys with defaults
				for (field in Reflect.fields(Base.defaultKeymap)) {
					if (!Reflect.hasField(keymap, field)) {
						Reflect.setField(keymap, field, Reflect.field(Base.defaultKeymap, field));
					}
				}
			});
		}
	}

	public static function saveKeymap() {
		if (raw.keymap == "default.json") return;
		var path = Data.dataPath + "keymap_presets/" + raw.keymap;
		var buffer = System.stringToBuffer(Json.stringify(keymap));
		Krom.fileSaveBytes(path, buffer);
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

	static function getTextureRes(): Int {
		var res = Base.resHandle.position;
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
		return Context.raw.projectAspectRatio == 2 ? Std.int(getTextureRes() / 2) : getTextureRes();
	}

	public static function getTextureResY(): Int {
		return Context.raw.projectAspectRatio == 1 ? Std.int(getTextureRes() / 2) : getTextureRes();
	}

	public static function getTextureResBias(): Float {
		var res = Base.resHandle.position;
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

	public static function loadTheme(theme: String, tagRedraw = true) {
		if (theme == "default.json") { // Built-in default
			Base.theme = new zui.Zui.Theme();
		}
		else {
			Data.getBlob("themes/" + theme, function(b: js.lib.ArrayBuffer) {
				var parsed = Json.parse(System.bufferToString(b));
				Base.theme = new zui.Zui.Theme();
				for (key in Type.getInstanceFields(zui.Zui.Theme)) {
					if (key == "theme_") continue;
					if (key.startsWith("set_")) continue;
					if (key.startsWith("get_")) key = key.substr(4);
					Reflect.setProperty(Base.theme, key, Reflect.getProperty(parsed, key));
				}
			});
		}
		Base.theme.FILL_WINDOW_BG = true;
		if (tagRedraw) {
			for (ui in Base.getUIs()) ui.t = Base.theme;
			UIBase.inst.tagUIRedraw();
		}
		if (Config.raw.touch_ui) {
			// Enlarge elements
			Base.theme.FULL_TABS = true;
			Base.theme.ELEMENT_H = 24 + 6;
			Base.theme.BUTTON_H = 22 + 6;
			Base.theme.FONT_SIZE = 13 + 2;
			Base.theme.ARROW_SIZE = 5 + 2;
			Base.theme.CHECK_SIZE = 15 + 4;
			Base.theme.CHECK_SELECT_SIZE = 8 + 2;
			buttonAlign = zui.Zui.Align.Left;
			buttonSpacing = "";
		}
		else {
			Base.theme.FULL_TABS = false;
			buttonAlign = zui.Zui.Align.Left;
			buttonSpacing = defaultButtonSpacing;
		}
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
