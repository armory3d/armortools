package arm;

import haxe.io.Bytes;
import haxe.Json;
import kha.Display;
import kha.WindowOptions;
import kha.WindowMode;
import kha.System;
import iron.data.Data;
import zui.Zui;
import arm.ui.UIBase;
import arm.render.RenderPathBase;
import arm.sys.File;
import arm.sys.Path;
import arm.ConfigFormat;

class Config {

	public static var raw: TConfig = null;
	public static var keymap: Dynamic;
	public static var configLoaded = false;
	public static var buttonAlign = zui.Zui.Align.Left;
	public static var buttonSpacing = "      ";

	public static function load(done: Void->Void) {
		try {
			Data.getBlob((Path.isProtected() ? Krom.savePath() : "") + "config.json", function(blob: kha.Blob) {
				configLoaded = true;
				raw = Json.parse(blob.toString());

				done();
			});
		}
		catch (e: Dynamic) {
			#if krom_linux
			try { // Protected directory
				Data.getBlob(Krom.savePath() + "config.json", function(blob: kha.Blob) {
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
		var path = (Path.isProtected() ? Krom.savePath() : Path.data() + Path.sep) + "config.json";
		var bytes = Bytes.ofString(Json.stringify(raw));
		Krom.fileSaveBytes(path, bytes.getData());

		#if krom_linux // Protected directory
		if (!File.exists(path)) Krom.fileSaveBytes(Krom.savePath() + "config.json", bytes.getData());
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
			raw.rp_vignette = 0.2;
			raw.rp_motionblur = false;
			#if (krom_android || krom_ios)
			raw.rp_ssao = false;
			#else
			raw.rp_ssao = true;
			#end
			raw.rp_ssr = false;
			raw.rp_supersample = 1.0;
			raw.version = Manifest.version;
			raw.sha = Main.sha;
			App.initConfig();
		}
		else {
			// Upgrade config format created by older ArmorPaint build
			// if (raw.version != Manifest.version) {
			// 	raw.version = Manifest.version;
			// 	save();
			// }
			if (raw.sha != Main.sha) {
				configLoaded = false;
				init();
				return;
			}
		}

		Zui.touchScroll = Zui.touchHold = Zui.touchTooltip = Config.raw.touch_ui;
		App.resHandle.position = raw.layer_res;
		loadKeymap();
	}

	public static function getOptions(): kha.SystemOptions {
		var windowMode = raw.window_mode == 0 ? WindowMode.Windowed : WindowMode.Fullscreen;
		var windowFeatures = None;
		if (raw.window_resizable) windowFeatures |= FeatureResizable;
		if (raw.window_maximizable) windowFeatures |= FeatureMaximizable;
		if (raw.window_minimizable) windowFeatures |= FeatureMinimizable;
		var title = "untitled - " + Manifest.title;
		return {
			title: title,
			window: {
				width: raw.window_w,
				height: raw.window_h,
				x: raw.window_x,
				y: raw.window_y,
				mode: windowMode,
				windowFeatures: windowFeatures
			},
			framebuffer: {
				samplesPerPixel: 1,
				verticalSync: raw.window_vsync,
				frequency: raw.window_frequency
			}
		};
	}

	public static function restore() {
		zui.Zui.Handle.global = new zui.Zui.Handle(); // Reset ui handles
		configLoaded = false;
		var _layout = raw.layout;
		init();
		raw.layout = _layout;
		App.initLayout();
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
		App.initLayout();
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
		iron.object.Uniforms.defaultFilter = Config.raw.rp_supersample < 1.0 ? kha.graphics4.TextureFilter.PointFilter : kha.graphics4.TextureFilter.LinearFilter;
		save();
		Context.raw.ddirty = 2;

		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();
		RenderPathBase.applyConfig();
		if (current != null) current.begin(false);
	}

	public static function loadKeymap() {
		if (raw.keymap == "default.json") { // Built-in default
			keymap = App.defaultKeymap;
		}
		else {
			Data.getBlob("keymap_presets/" + raw.keymap, function(blob: kha.Blob) {
				keymap = Json.parse(blob.toString());
				// Fill in undefined keys with defaults
				for (field in Reflect.fields(App.defaultKeymap)) {
					if (!Reflect.hasField(keymap, field)) {
						Reflect.setField(keymap, field, Reflect.field(App.defaultKeymap, field));
					}
				}
			});
		}
	}

	public static function saveKeymap() {
		if (raw.keymap == "default.json") return;
		var path = Data.dataPath + "keymap_presets/" + raw.keymap;
		var bytes = Bytes.ofString(Json.stringify(keymap));
		Krom.fileSaveBytes(path, bytes.getData());
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
		return Context.raw.projectAspectRatio == 2 ? Std.int(getTextureRes() / 2) : getTextureRes();
	}

	public static function getTextureResY(): Int {
		return Context.raw.projectAspectRatio == 1 ? Std.int(getTextureRes() / 2) : getTextureRes();
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
			for (ui in App.getUIs()) ui.t = App.theme;
			UIBase.inst.tagUIRedraw();
		}
		if (Config.raw.touch_ui) {
			// Enlarge elements
			App.theme.FULL_TABS = true;
			App.theme.ELEMENT_H = 24 + 4;
			App.theme.BUTTON_H = 22 + 4;
			App.theme.FONT_SIZE = 13 + 2;
			App.theme.ARROW_SIZE = 5 + 2;
			App.theme.CHECK_SIZE = 15 + 4;
			App.theme.CHECK_SELECT_SIZE = 8 + 2;
			buttonAlign = zui.Zui.Align.Center;
			buttonSpacing = "";
		}
		else {
			App.theme.FULL_TABS = false;
			buttonAlign = zui.Zui.Align.Left;
			buttonSpacing = "      ";
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
