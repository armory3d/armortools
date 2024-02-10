
class Config {

	static raw: TConfig = null;
	static keymap: any;
	static configLoaded = false;
	static buttonAlign = Align.Left;
	static defaultButtonSpacing = "       ";
	static buttonSpacing = Config.defaultButtonSpacing;

	static load = (done: ()=>void) => {
		try {
			data_get_blob((Path.isProtected() ? krom_save_path() : "") + "config.json", (blob: ArrayBuffer) => {
				Config.configLoaded = true;
				Config.raw = JSON.parse(sys_buffer_to_string(blob));

				done();
			});
		}
		catch (e: any) {
			///if krom_linux
			try { // Protected directory
				data_get_blob(krom_save_path() + "config.json", (blob: ArrayBuffer) => {
					Config.configLoaded = true;
					Config.raw = JSON.parse(sys_buffer_to_string(blob));
					done();
				});
			}
			catch (e: any) {
				done();
			}
			///else
			done();
			///end
		}
	}

	static save = () => {
		// Use system application data folder
		// when running from protected path like "Program Files"
		let path = (Path.isProtected() ? krom_save_path() : Path.data() + Path.sep) + "config.json";
		let buffer = sys_string_to_buffer(JSON.stringify(Config.raw));
		krom_file_save_bytes(path, buffer);

		///if krom_linux // Protected directory
		if (!File.exists(path)) krom_file_save_bytes(krom_save_path() + "config.json", buffer);
		///end
	}

	static init = () => {
		if (!Config.configLoaded || Config.raw == null) {
			Config.raw = {};
			Config.raw.locale = "system";
			Config.raw.window_mode = 0;
			Config.raw.window_resizable = true;
			Config.raw.window_minimizable = true;
			Config.raw.window_maximizable = true;
			Config.raw.window_w = 1600;
			Config.raw.window_h = 900;
			///if krom_darwin
			Config.raw.window_w *= 2;
			Config.raw.window_h *= 2;
			///end
			Config.raw.window_x = -1;
			Config.raw.window_y = -1;
			Config.raw.window_scale = 1.0;
			if (sys_display_width() >= 2560 && sys_display_height() >= 1600) {
				Config.raw.window_scale = 2.0;
			}
			///if (krom_android || krom_ios || krom_darwin)
			Config.raw.window_scale = 2.0;
			///end
			Config.raw.window_vsync = true;
			Config.raw.window_frequency = sys_display_frequency();
			Config.raw.rp_bloom = false;
			Config.raw.rp_gi = false;
			Config.raw.rp_vignette = 0.2;
			Config.raw.rp_grain = 0.09;
			Config.raw.rp_motionblur = false;
			///if (krom_android || krom_ios)
			Config.raw.rp_ssao = false;
			///else
			Config.raw.rp_ssao = true;
			///end
			Config.raw.rp_ssr = false;
			Config.raw.rp_supersample = 1.0;
			Config.raw.version = manifest_version;
			Config.raw.sha = Config.getSha();
			Base.initConfig();
		}
		else {
			// Upgrade config format created by older ArmorPaint build
			// if (Config.raw.version != manifest_version) {
			// 	Config.raw.version = manifest_version;
			// 	save();
			// }
			if (Config.raw.sha != Config.getSha()) {
				Config.configLoaded = false;
				Config.init();
				return;
			}
		}

		zui_set_touch_scroll(Config.raw.touch_ui);
		zui_set_touch_hold(Config.raw.touch_ui);
		zui_set_touch_tooltip(Config.raw.touch_ui);
		Base.resHandle.position = Config.raw.layer_res;
		Config.loadKeymap();
	}

	static getSha = (): string => {
		let sha = "";
		data_get_blob("version.json", (blob: ArrayBuffer) => {
			sha = JSON.parse(sys_buffer_to_string(blob)).sha;
		});
		return sha;
	}

	static getDate = (): string => {
		let date = "";
		data_get_blob("version.json", (blob: ArrayBuffer) => {
			date = JSON.parse(sys_buffer_to_string(blob)).date;
		});
		return date;
	}

	static getOptions = (): kinc_sys_ops_t => {
		let windowMode = Config.raw.window_mode == 0 ? window_mode_t.WINDOWED : window_mode_t.FULLSCREEN;
		let windowFeatures = window_features_t.NONE;
		if (Config.raw.window_resizable) windowFeatures |= window_features_t.RESIZABLE;
		if (Config.raw.window_maximizable) windowFeatures |= window_features_t.MAXIMIZABLE;
		if (Config.raw.window_minimizable) windowFeatures |= window_features_t.MINIMIZABLE;
		let title = "untitled - " + manifest_title;
		return {
			title: title,
			width: Config.raw.window_w,
			height: Config.raw.window_h,
			x: Config.raw.window_x,
			y: Config.raw.window_y,
			mode: windowMode,
			features: windowFeatures,
			vsync: Config.raw.window_vsync,
			frequency: Config.raw.window_frequency
		};
	}

	static restore = () => {
		zui_children = new Map(); // Reset ui handles
		Config.configLoaded = false;
		let _layout = Config.raw.layout;
		Config.init();
		Config.raw.layout = _layout;
		Base.initLayout();
		Translator.loadTranslations(Config.raw.locale);
		Config.applyConfig();
		Config.loadTheme(Config.raw.theme);
	}

	static importFrom = (from: TConfig) => {
		let _sha = Config.raw.sha;
		let _version = Config.raw.version;
		Config.raw = from;
		Config.raw.sha = _sha;
		Config.raw.version = _version;
		zui_children = new Map(); // Reset ui handles
		Config.loadKeymap();
		Base.initLayout();
		Translator.loadTranslations(Config.raw.locale);
		Config.applyConfig();
		Config.loadTheme(Config.raw.theme);
	}

	static applyConfig = () => {
		Config.raw.rp_ssao = Context.raw.hssao.selected;
		Config.raw.rp_ssr = Context.raw.hssr.selected;
		Config.raw.rp_bloom = Context.raw.hbloom.selected;
		Config.raw.rp_gi = Context.raw.hvxao.selected;
		Config.raw.rp_supersample = Config.getSuperSampleSize(Context.raw.hsupersample.position);
		Config.save();
		Context.raw.ddirty = 2;

		let current = _g2_current;
		if (current != null) g2_end();
		RenderPathBase.applyConfig();
		if (current != null) g2_begin(current, false);
	}

	static loadKeymap = () => {
		if (Config.raw.keymap == "default.json") { // Built-in default
			Config.keymap = Base.defaultKeymap;
		}
		else {
			data_get_blob("keymap_presets/" + Config.raw.keymap, (blob: ArrayBuffer) => {
				Config.keymap = JSON.parse(sys_buffer_to_string(blob));
				// Fill in undefined keys with defaults
				for (let field in Base.defaultKeymap) {
					if (!(field in Config.keymap)) {
						let adefaultKeymap: any = Base.defaultKeymap;
						Config.keymap[field] = adefaultKeymap[field];
					}
				}
			});
		}
	}

	static saveKeymap = () => {
		if (Config.raw.keymap == "default.json") return;
		let path = data_path() + "keymap_presets/" + Config.raw.keymap;
		let buffer = sys_string_to_buffer(JSON.stringify(Config.keymap));
		krom_file_save_bytes(path, buffer);
	}

	static getSuperSampleQuality = (f: f32): i32 => {
		return f == 0.25 ? 0 :
			   f == 0.5 ? 1 :
			   f == 1.0 ? 2 :
			   f == 1.5 ? 3 :
			   f == 2.0 ? 4 : 5;
	}

	static getSuperSampleSize = (i: i32): f32 => {
		return i == 0 ? 0.25 :
			   i == 1 ? 0.5 :
			   i == 2 ? 1.0 :
			   i == 3 ? 1.5 :
			   i == 4 ? 2.0 : 4.0;
	}

	static getTextureRes = (): i32 => {
		let res = Base.resHandle.position;
		return res == TextureRes.Res128 ? 128 :
			   res == TextureRes.Res256 ? 256 :
			   res == TextureRes.Res512 ? 512 :
			   res == TextureRes.Res1024 ? 1024 :
			   res == TextureRes.Res2048 ? 2048 :
			   res == TextureRes.Res4096 ? 4096 :
			   res == TextureRes.Res8192 ? 8192 :
			   res == TextureRes.Res16384 ? 16384 : 0;
	}

	static getTextureResX = (): i32 => {
		return Context.raw.projectAspectRatio == 2 ? Math.floor(Config.getTextureRes() / 2) : Config.getTextureRes();
	}

	static getTextureResY = (): i32 => {
		return Context.raw.projectAspectRatio == 1 ? Math.floor(Config.getTextureRes() / 2) : Config.getTextureRes();
	}

	static getTextureResBias = (): f32 => {
		let res = Base.resHandle.position;
		return res == TextureRes.Res128 ? 16.0 :
			   res == TextureRes.Res256 ? 8.0 :
			   res == TextureRes.Res512 ? 4.0 :
			   res == TextureRes.Res1024 ? 2.0 :
			   res == TextureRes.Res2048 ? 1.5 :
			   res == TextureRes.Res4096 ? 1.0 :
			   res == TextureRes.Res8192 ? 0.5 :
			   res == TextureRes.Res16384 ? 0.25 : 1.0;
	}

	static getTextureResPos = (i: i32): i32 => {
		return i == 128 ? TextureRes.Res128 :
			   i == 256 ? TextureRes.Res256 :
			   i == 512 ? TextureRes.Res512 :
			   i == 1024 ? TextureRes.Res1024 :
			   i == 2048 ? TextureRes.Res2048 :
			   i == 4096 ? TextureRes.Res4096 :
			   i == 8192 ? TextureRes.Res8192 :
			   i == 16384 ? TextureRes.Res16384 : 0;
	}

	static loadTheme = (theme: string, tagRedraw = true) => {
		if (theme == "default.json") { // Built-in default
			Base.theme = zui_theme_create();
		}
		else {
			data_get_blob("themes/" + theme, (b: ArrayBuffer) => {
				let parsed = JSON.parse(sys_buffer_to_string(b));
				Base.theme = zui_theme_create();
				for (let key in Base.theme) {
					if (key == "theme_") continue;
					if (key.startsWith("set_")) continue;
					if (key.startsWith("get_")) key = key.substr(4);
					let atheme: any = Base.theme;
					atheme[key] = parsed[key];
				}
			});
		}
		Base.theme.FILL_WINDOW_BG = true;
		if (tagRedraw) {
			for (let ui of Base.getUIs()) ui.t = Base.theme;
			UIBase.tagUIRedraw();
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
			Config.buttonAlign = Align.Left;
			Config.buttonSpacing = "";
		}
		else {
			Base.theme.FULL_TABS = false;
			Config.buttonAlign = Align.Left;
			Config.buttonSpacing = Config.defaultButtonSpacing;
		}
	}

	static enablePlugin = (f: string) => {
		Config.raw.plugins.push(f);
		Plugin.start(f);
	}

	static disablePlugin = (f: string) => {
		array_remove(Config.raw.plugins, f);
		Plugin.stop(f);
	}
}
